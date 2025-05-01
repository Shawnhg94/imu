#ifndef __CPHIDGET
#define __CPHIDGET

#ifndef EXTERNALPROTO
/*
 * This file is part of libphidget22
 *
 * Copyright (c) 2015-2022 Phidgets Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
#endif

#include "types.gen.h"
#include "phidget.gen.h"
#include "object.h"

typedef void(CCONV *Phidget_AsyncCallback)(PhidgetHandle phid, void *ctx, PhidgetReturnCode returnCode);

/* Channel agnostic delete */
API_PRETURN_HDR Phidget_delete(PhidgetHandle *phid);

/* Returns VINT devices attached to a VINT hub */
API_PRETURN_HDR Phidget_getChildDevices(PhidgetHandle phid, PhidgetHandle *arr, size_t *arrCnt);
API_PRETURN_HDR Phidget_releaseDevices(PhidgetHandle *arr, size_t arrCnt);

/* Used by the network server */
API_IRETURN_HDR Phidget_validDictionaryKey(const char *);
typedef void(CCONV *PhidgetDictionary_OnChangeCallback)(int, const char *, void *, int, const char *, const char *);
API_PRETURN_HDR PhidgetDictionary_setOnChangeCallbackHandler(PhidgetDictionary_OnChangeCallback, void *);

#ifdef _MACOSX
/* macOS system sleep / wake handlers */
API_PRETURN_HDR Phidget_setOnWillSleepHandler(void(CCONV *fptr)(void *ctx), void *ctx);
API_PRETURN_HDR Phidget_setOnWakeupHandler(void(CCONV *fptr)(void *ctx), void *ctx);
#endif

#ifndef EXTERNALPROTO

	#ifdef NDEBUG
		#define chlog(...)
	#else
		#define chlog(...) PhidgetLog_loge(NULL, 0, __func__, "_phidget22channel", PHIDGET_LOG_INFO, __VA_ARGS__)
	#endif

	#include "devices.h"
	#include "constantsinternal.h"
	#include "constants.h"
	#include "bridge.h"
	#include "mos/bsdqueue.h"
	#include "mos/mos_rwrlock.h"

	#include "util/packettracker.h"
	#include "util/packing.h"
	#include "util/utils.h"

	#include "network/network.h"
	#include "object.h"
	#include "enumutil.gen.h"
	#include "errorstrings.gen.h"

	#ifdef CHK_FORMAT_STRINGS
		#undef PRIphid
		#define PRIphid "p"
	#endif

	#define CHANNELID_ISHUBPORT	   0x01
	#define CHANNELID_ISVINTDEVICE 0x02
struct _channelid_parts {
	uint32_t serial;
	uint8_t flags;
	uint8_t class;
	uint8_t port;
	uint8_t index;
};

typedef union {
	#define c_flags	 parts.flags
	#define c_type	 parts.type
	#define c_serial parts.serial
	#define c_port	 parts.port
	#define c_index	 parts.index
	#define c_class	 parts.class
	struct _channelid_parts parts;
	uint64_t c_id;
} channelid_t;

typedef enum {
	EVENTMODE_DATARATE = 1,
	EVENTMODE_CHANGETRIGGER = 2
} Phidget_EventMode;

typedef struct _PhidgetChannel PhidgetChannel;
typedef struct _PhidgetDevice PhidgetDevice;

	#include "vint.h"
	#include "usb.h"
	#include "spi.h"
	#include "lightning.h"
	#include "mesh.h"
	#include "virtual.h"

typedef MTAILQ_HEAD(phidgetchannnelnetconnlist, _PhidgetChannelNetConn) phidgetchannelnetconnlist_t;

typedef struct {
	Phidget_DeviceClass class;

	const PhidgetUniqueDeviceDef *UDD;
	int version;

	// Properties which only exist for a USB Device
	char label[MAX_LABEL_STORAGE];
	int serialNumber;

	// Properties which only exist for a VINT Device
	unsigned char isHubPort;
	int hubPort;

	// Properties which exist only for mesh devices
	Phidget_MeshMode meshMode;

	// Properties which exist only for remote devices
	char serverName[256];
	char serverUniqueName[256];
	char serverPeerName[256];
	char serverHostName[256];

	int uniqueIndex; // firmware index (index into ->children[] array of parent)
} PhidgetDeviceInfo, *PhidgetDeviceInfoHandle;

typedef struct {
	int openFlags;
	PhidgetHub_PortMode hubPortMode;

	int channel;
	BOOL isLocal;

	// USB Phidget options
	int serialNumber;
	char *label;

	// VINT Device options
	int hubPort;
	unsigned char isHubPort;

	// whether to reset channel on attach/close
	unsigned char openCloseReset;

	// Open type, timeout
	unsigned char async;
	uint32_t timeout;

	// Network options
	char *serverName;
	BOOL isRemote;
	uint64_t devid;

	// open attempt feedback
	int openAttempts;
	PhidgetReturnCode lastOpenRes;
	PhidgetDeviceHandle lastOpenDevice;
} PhidgetOpenInfo, *PhidgetOpenInfoHandle;

typedef struct _PhidgetChannelNetConn {
	PhidgetNetConnHandle nc;
	MTAILQ_ENTRY(_PhidgetChannelNetConn)
	link;
	uint16_t setstatusrep; /* reply seq for setstatus (0 if sent already) */
} PhidgetChannelNetConn, *PhidgetChannelNetConnHandle;

struct _PhidgetChannel {
	PHIDGET_STRUCT_START

	Phidget_ChannelClass class;
	const PhidgetUniqueChannelDef *UCD;
	int uniqueIndex; /* firmware index (index into ->channels[] array of parent) */
	int index;		 /* channel id */

	MTAILQ_ENTRY(_PhidgetChannel)
	link; /* open channel  linkage */
	MTAILQ_ENTRY(_PhidgetChannel)
	match; /* attach matching list linkage */

	phidgetchannelnetconnlist_t netconns; /* list of network connections to channel */
	mos_mutex_t netconnslk;				  /* lock for network connections */
	int netconnscnt;

	PhidgetOpenInfoHandle openInfo;
	mosiop_t iop;

	void *private; /* private pointer for extending code */

	PhidgetReturnCode (*initAfterOpen)(PhidgetChannelHandle);
	PhidgetReturnCode (*setDefaults)(PhidgetChannelHandle);
	PhidgetReturnCode (*bridgeInput)(PhidgetChannelHandle, BridgePacket *);
	void (*errorHandler)(PhidgetChannelHandle, Phidget_ErrorEventCode);
	PhidgetReturnCode (*getStatus)(PhidgetChannelHandle, BridgePacket **);
	PhidgetReturnCode (*setStatus)(PhidgetChannelHandle, BridgePacket *);
	void (*fireInitialEvents)(PhidgetChannelHandle);
	int (*hasInitialState)(PhidgetChannelHandle);

	/* User events */
	volatile Phidget_OnAttachCallback Attach;
	volatile void *AttachCtx;
	volatile Phidget_OnDetachCallback Detach;
	volatile void *DetachCtx;
	volatile Phidget_OnErrorCallback Error;
	volatile void *ErrorCtx;
	volatile Phidget_OnPropertyChangeCallback PropertyChange;
	volatile void *PropertyChangeCtx;
	PhidgetReturnCode (*_closing)(PhidgetChannelHandle);

	/* Error event tracking */
	Phidget_ErrorEventCode lastErrorEventCode;
	char *lastErrorEventDesc;
	mostime_t lastErrorEventTime;
};

	#define PHIDGET_DEVICE_LAST_ERROR_STR_LEN 256
struct _PhidgetDevice {
	PHIDGET_STRUCT_START

	mos_tlock_t *__memberlock;

	MTAILQ_ENTRY(_PhidgetDevice)
	link; /* phidgetDevices linkage */
	PhidgetConnectionType connType;
	PhidgetHandle conn;

	#define devudd_id deviceInfo.UDD->id
	#define dev_hub	  deviceInfo.UDD->channelCnts.hub

	PhidgetDeviceInfo deviceInfo;

	PhidgetDeviceHandle child[PHIDGET_MAXCHILDREN];	   /* RUNLOCK */
	PhidgetChannelHandle channel[PHIDGET_MAXCHANNELS]; /* RUNLOCK */
	uint32_t readCount;

	PhidgetPacketTrackersHandle packetTracking;

	uint8_t GPPResponse;

	PhidgetReturnCode(CCONV *bridgeInput)(PhidgetChannelHandle, BridgePacket *);
	mos_mutex_t bridgeInputLock;

	PhidgetReturnCode(CCONV *initAfterCreate)(PhidgetDeviceHandle);
	PhidgetReturnCode(CCONV *initAfterOpen)(PhidgetDeviceHandle);
	PhidgetReturnCode(CCONV *_closing)(PhidgetDeviceHandle);
	PhidgetReturnCode(CCONV *dataInput)(PhidgetDeviceHandle, uint8_t *buffer, size_t length);
	const VINTIO_t *vintIO;

	char fwstr[64];
	char firmwareUpgradeName[128];

	// open attempt feedback
	int openAttempts;
	PhidgetReturnCode lastOpenRes;
	char *lastOpenErrstr;
};

typedef struct _AttachDetachEntry {
	int flags;
	int index;
	PhidgetDeviceHandle dev;
	MTAILQ_ENTRY(_AttachDetachEntry)
	link; /* netAttachDetachQueue linkage */
} AttachDetachEntry, *AttachDetachEntryHandle;

typedef struct _PhidetErrorDetail {
	PhidgetReturnCode code;
	char *detail;
} PhidgetErrorDetail, *PhidgetErrorDetailHandle;

/* Globals */

extern const char *LibraryVersion;
extern const char *LibraryVersionNumber;
extern const char *LibrarySystem;

extern const PhidgetUniqueDeviceDef Phidget_Unique_Device_Def[];

void phidget_init(PhidgetHandle, PhidgetStructType, PhidgetDelete_t);

PhidgetChannelHandle PhidgetChannelCast(void *);
PhidgetDeviceHandle PhidgetDeviceCast(void *);

void PhidgetWriteLockChannels(void);
void PhidgetReadLockChannels(void);
void PhidgetUnlockChannels(void);

void PhidgetWriteLockDevices(void);
void PhidgetReadLockDevices(void);
void PhidgetUnlockDevices(void);

void PhidgetLockAttachDetachQueue(void);
void PhidgetUnlockAttachDetachQueue(void);

PhidgetReturnCode dispatchUserRequest(PhidgetChannelHandle, BridgePacket *, Phidget_AsyncCallback, void *);
PhidgetReturnCode dispatchBridgePacket(void *_phid, BridgePacket *);
PhidgetReturnCode dispatchVintData(void *_phid, const uint8_t *, size_t);
void clearPhidgetDispatch(PhidgetHandle);
void freeDispatchHandle(PhidgetHandle);
void startDispatch(PhidgetHandle);
void stopDispatch(PhidgetHandle, int);
void wakeDispatch(void);

PhidgetChannelHandle getAttachedChannel(void *_device, int index);
PhidgetChannelHandle getChannel(void *_device, int index);
PhidgetReturnCode setChannel(PhidgetDeviceHandle, int index, void *_channel);
PhidgetDeviceHandle getParent(void *phid);
void setParent(void *_phid, void *_parent);
PhidgetDeviceHandle getChild(PhidgetDeviceHandle, int index);
void setChild(PhidgetDeviceHandle device, int index, void *_childdevice);

const PhidgetChannelAttributeDef *getPhidgetChannelAttributesByClass(Phidget_ChannelClass);
const PhidgetChannelAttributeDef *getPhidgetChannelAttributes(PhidgetChannelHandle);

PhidgetReturnCode PhidgetDevice_usbDataInput(PhidgetDeviceHandle device, uint8_t *buffer, size_t length);

PhidgetReturnCode PhidgetDevice_read(PhidgetDeviceHandle device);

BOOL isVintChannel(void *);
BOOL isNetworkPhidget(void *);
uint64_t PhidgetDeviceGetNetId(PhidgetDeviceHandle device);

typedef PhidgetReturnCode (*deviceChannelVisitor_t)(PhidgetDeviceHandle dev,
													const PhidgetUniqueChannelDef *ucd, int index, int uniqueIndex, void *ctx);

void matchOpenChannels(void);

PhidgetReturnCode addDictionary(int, const char *);

PhidgetOpenInfoHandle mallocPhidgetOpenInfo(void);
void freePhidgetOpenInfo(PhidgetOpenInfoHandle item);

size_t getMaxOutPacketSize(PhidgetDeviceHandle device);
PhidgetReturnCode PhidgetDevice_sendpacket(mosiop_t iop, PhidgetDeviceHandle device, const unsigned char *buf, size_t length);
PhidgetReturnCode PhidgetDevice_sendpacketTransaction(mosiop_t iop, PhidgetDeviceHandle device,
													  const unsigned char *bufferIn, size_t bufferInLen, PhidgetTransactionHandle trans);
PhidgetReturnCode PhidgetDevice_beginTransaction(PhidgetDeviceHandle device, PhidgetTransactionHandle trans);
PhidgetReturnCode PhidgetChannel_endTransaction(PhidgetChannelHandle channel, PhidgetTransactionHandle trans);
PhidgetReturnCode PhidgetChannel_beginTransaction(PhidgetChannelHandle channel, PhidgetTransactionHandle trans);
PhidgetReturnCode PhidgetDevice_transferpacket(mosiop_t iop, PhidgetDeviceHandle device, int transferType, int packetType, int index,
											   unsigned char *buffer, size_t *bufferLen, int timeout);

PhidgetReturnCode StartCentralThread(PhidgetChannelHandle channel);
void NotifyCentralThread(void);

PhidgetReturnCode VINTPacketStatusCode_to_PhidgetReturnCode(VINTPacketStatusCode code, const PhidgetUniqueDeviceDef *udd);
PhidgetReturnCode VINTPacketStatusCodeDeviceCode_to_PhidgetReturnCode(VINTPacketStatusCode code, const PhidgetUniqueDeviceDef *udd);

PhidgetReturnCode _addDevice(PhidgetDeviceHandle); /* no lock */
PhidgetReturnCode addDevice(PhidgetDeviceHandle);
PhidgetReturnCode _removeDevice(PhidgetDeviceHandle); /* no lock */
PhidgetReturnCode getNetworkDevice(PhidgetNetConnHandle, uint64_t, PhidgetDeviceHandle *);
PhidgetDeviceHandle getDeviceById(uint64_t);
PhidgetReturnCode addChannel(PhidgetChannelHandle);
PhidgetReturnCode removeChannel(PhidgetChannelHandle);
PhidgetChannelHandle getChannelById(uint64_t);
uint64_t mkChannelId(int chindex, int chclass, int serialNumber, int vint, int port, int hubport);
uint64_t getChannelId(PhidgetChannelHandle);

PhidgetHandle getPhidgetConnection(void *phid);

int getTimeout(PhidgetDeviceHandle device);
PhidgetReturnCode attachChannel(PhidgetDeviceHandle, int index, PhidgetChannelHandle);
PhidgetReturnCode openDevice(PhidgetDeviceHandle attachedDevice);
void closeDevice(PhidgetDeviceHandle device, int forceClose);
PhidgetReturnCode PhidgetChannel_create(PhidgetChannelHandle *phid);

int phidgetdevice_compare(PhidgetDeviceHandle, PhidgetDeviceHandle);
typedef MTAILQ_HEAD(PhidgetDevices, _PhidgetDevice) phidgetdevices_t;
extern phidgetdevices_t phidgetDevices;
extern mos_tlock_t *devicesLock;
	#define FOREACH_DEVICE_SAFE(dev, tmp) MTAILQ_FOREACH_SAFE(dev, &phidgetDevices, link, tmp)
	#define FOREACH_DEVICE(dev)			  MTAILQ_FOREACH(dev, &phidgetDevices, link)
	#define DEVICES_EMPTY				  MTAILQ_EMPTY(&phidgetDevices)

typedef MTAILQ_HEAD(PhidgetChannels, _PhidgetChannel) phidgetchannels_t;
extern phidgetchannels_t phidgetChannels;
extern mos_rwrlock_t channelsLock;
	#define FOREACH_CHANNEL_SAFE(ch, tmp) MTAILQ_FOREACH_SAFE(ch, &phidgetChannels, link, tmp)
	#define FOREACH_CHANNEL(ch)			  MTAILQ_FOREACH(ch, &phidgetChannels, link)
	#define CHANNELS_EMPTY				  MTAILQ_EMPTY(&phidgetChannels)

	#define ATTACHQUEUE_FLAG			  0x00002000
	#define DETACHQUEUE_FLAG			  0x00004000
	#define DETACHVINTQUEUE_FLAG		  0x00008000
	#define USBERRQUEUE_FLAG			  0x00010000
typedef MTAILQ_HEAD(AttachDetachEntries, _AttachDetachEntry) attachdetachentries_t;
extern attachdetachentries_t attachDetachQueue;
	#define FOREACH_ATTACHDETACH_QUEUE_DEVICE(dev) MTAILQ_FOREACH(dev, &attachDetachQueue, link)
	#define ATTACHDETACH_QUEUE_DEVICE_EMPTY		   MTAILQ_EMPTY(&attachDetachQueue)

typedef MTAILQ_HEAD(phidgets, _Phidget) phidgets_t;

PhidgetReturnCode matchUniqueDevice(PhidgetUniqueDeviceType, int, int, int, int, int *);

PhidgetReturnCode createPhidgetDevice(PhidgetConnectionType connType, const PhidgetUniqueDeviceDef *pdd,
									  int version, const char *label, int serialNumber, PhidgetDeviceHandle *device);

PhidgetReturnCode createPhidgetVirtualDevice(const PhidgetUniqueDeviceDef *, int, const char *, int,
											 PhidgetDeviceHandle *);

PhidgetReturnCode createPhidgetHIDUSBDevice(const PhidgetUniqueDeviceDef *, int, const char *, int,
											const void *, const char *, PhidgetDeviceHandle *);

PhidgetReturnCode createPhidgetPHIDUSBDevice(const PhidgetUniqueDeviceDef *pdd, int version, const char *label, int serialNumber,
											 const void *devpath, const char *skuString, PhidgetDeviceHandle *device);

PhidgetReturnCode createPhidgetSPIDevice(const PhidgetUniqueDeviceDef *, int version, const char *label,
										 int serialNumber, const char *skuString, PhidgetDeviceHandle *);

PhidgetReturnCode createPhidgetVINTDevice(const PhidgetUniqueDeviceDef *, int, const char *, int,
										  PhidgetDeviceHandle *);

PhidgetReturnCode createPhidgetMeshDevice(const PhidgetUniqueDeviceDef *pdd, int version,
										  const char *label, int serialNumber, PhidgetDeviceHandle *device);

PhidgetReturnCode createPhidgetNetDevice(const PhidgetUniqueDeviceDef *, int, const char *, int,
										 const char *, PhidgetNetConnHandle, uint64_t, PhidgetDeviceHandle *);

// Exported but never appears in phidget22.h
const char *CCONV deviceInfo(PhidgetDeviceHandle, char *, uint32_t);
const char *CCONV channelInfo(PhidgetChannelHandle, char *, uint32_t);

PhidgetReturnCode waitForReads(PhidgetDeviceHandle, uint32_t numReads, uint32_t ms);

PhidgetReturnCode PhidgetChannel_bridgeInput(PhidgetChannelHandle channel, BridgePacket *bp);

void PhidgetInit(void);
void PhidgetFini(void);

const char *clientGetHostName(PhidgetNetConnHandle nc);

uint32_t HANDLE_DATAINTERVAL_PKT(BridgePacket *bp, uint32_t interruptRate);

PhidgetReturnCode Phidget_setLastError(PhidgetReturnCode code, const char *fmt, ...);

const char *getPhidgetServerName(PhidgetDeviceHandle);

PhidgetReturnCode PhidgetChannel_sendErrorEvent(PhidgetChannelHandle channel, Phidget_ErrorEventCode code, const char *desc);
PhidgetReturnCode PhidgetChannel_sendErrorEventThrottled(PhidgetChannelHandle channel, Phidget_ErrorEventCode code, const char *desc);
void PhidgetChannel_clearErrorEvent(PhidgetChannelHandle channel);

#endif // #ifndef EXTERNALPROTO

#endif // #ifndef __CPHIDGET
