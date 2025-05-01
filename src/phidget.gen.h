#ifndef _PHIDGET_H_
#define _PHIDGET_H_
typedef struct _Phidget *PhidgetHandle;

/* Methods */
API_PRETURN_HDR Phidget_getErrorDescription(PhidgetReturnCode errorCode, const char **errorString);
API_PRETURN_HDR Phidget_finalize(int flags);
API_PRETURN_HDR Phidget_getLastError(PhidgetReturnCode *errorCode, const char **errorString,
  char *errorDetail, size_t *errorDetailLen);
API_PRETURN_HDR Phidget_release(PhidgetHandle *phid);
API_PRETURN_HDR Phidget_resetLibrary(void);
API_PRETURN_HDR Phidget_retain(PhidgetHandle phid);
#ifdef INCLUDE_PRIVATE
API_PRETURN_HDR Phidget_getClientVersion(PhidgetHandle ch, int *major, int *minor);
#endif
API_PRETURN_HDR Phidget_close(PhidgetHandle ch);
API_PRETURN_HDR Phidget_getDeviceChannelCount(PhidgetHandle ch, Phidget_ChannelClass cls,
  uint32_t *count);
API_PRETURN_HDR Phidget_open(PhidgetHandle ch);
API_PRETURN_HDR Phidget_openWaitForAttachment(PhidgetHandle ch, uint32_t timeout);
#ifdef INCLUDE_PRIVATE
API_PRETURN_HDR Phidget_reboot(PhidgetHandle ch);
#endif
#ifdef INCLUDE_PRIVATE
API_PRETURN_HDR Phidget_rebootFirmwareUpgrade(PhidgetHandle ch, uint32_t upgradeTimeout);
#endif
#ifdef INCLUDE_PRIVATE
API_PRETURN_HDR Phidget_getServerVersion(PhidgetHandle ch, int *major, int *minor);
#endif
API_PRETURN_HDR Phidget_writeDeviceLabel(PhidgetHandle ch, const char *deviceLabel);
#ifdef INCLUDE_PRIVATE
API_PRETURN_HDR Phidget_writeFlash(PhidgetHandle ch);
#endif

/* Properties */
API_PRETURN_HDR Phidget_getLibraryVersion(const char **libraryVersion);
API_PRETURN_HDR Phidget_getLibraryVersionNumber(const char **libraryVersionNumber);
API_PRETURN_HDR Phidget_getAttached(PhidgetHandle ch, int *attached);
API_PRETURN_HDR Phidget_setChannel(PhidgetHandle ch, int channel);
API_PRETURN_HDR Phidget_getChannel(PhidgetHandle ch, int *channel);
API_PRETURN_HDR Phidget_getIsChannel(PhidgetHandle ch, int *isChannel);
API_PRETURN_HDR Phidget_getChannelClass(PhidgetHandle ch, Phidget_ChannelClass *channelClass);
API_PRETURN_HDR Phidget_getChannelClassName(PhidgetHandle ch, const char **channelClassName);
API_PRETURN_HDR Phidget_getChannelName(PhidgetHandle ch, const char **channelName);
API_PRETURN_HDR Phidget_getChannelSubclass(PhidgetHandle ch, Phidget_ChannelSubclass *channelSubclass);
API_PRETURN_HDR Phidget_setDataInterval(PhidgetHandle ch, uint32_t dataInterval);
API_PRETURN_HDR Phidget_getDataInterval(PhidgetHandle ch, uint32_t *dataInterval);
API_PRETURN_HDR Phidget_getMinDataInterval(PhidgetHandle ch, uint32_t *minDataInterval);
API_PRETURN_HDR Phidget_getMaxDataInterval(PhidgetHandle ch, uint32_t *maxDataInterval);
API_PRETURN_HDR Phidget_setDataRate(PhidgetHandle ch, double dataRate);
API_PRETURN_HDR Phidget_getDataRate(PhidgetHandle ch, double *dataRate);
API_PRETURN_HDR Phidget_getMinDataRate(PhidgetHandle ch, double *minDataRate);
API_PRETURN_HDR Phidget_getMaxDataRate(PhidgetHandle ch, double *maxDataRate);
API_PRETURN_HDR Phidget_getDeviceClass(PhidgetHandle ch, Phidget_DeviceClass *deviceClass);
API_PRETURN_HDR Phidget_getDeviceClassName(PhidgetHandle ch, const char **deviceClassName);
#ifdef INCLUDE_PRIVATE
API_PRETURN_HDR Phidget_getDeviceFirmwareUpgradeString(PhidgetHandle ch,
  const char **deviceFirmwareUpgradeString);
#endif
API_PRETURN_HDR Phidget_getDeviceID(PhidgetHandle ch, Phidget_DeviceID *deviceID);
API_PRETURN_HDR Phidget_setDeviceLabel(PhidgetHandle ch, const char * deviceLabel);
API_PRETURN_HDR Phidget_getDeviceLabel(PhidgetHandle ch, const char **deviceLabel);
API_PRETURN_HDR Phidget_getDeviceName(PhidgetHandle ch, const char **deviceName);
API_PRETURN_HDR Phidget_setDeviceSerialNumber(PhidgetHandle ch, int32_t deviceSerialNumber);
API_PRETURN_HDR Phidget_getDeviceSerialNumber(PhidgetHandle ch, int32_t *deviceSerialNumber);
API_PRETURN_HDR Phidget_getDeviceSKU(PhidgetHandle ch, const char **deviceSKU);
API_PRETURN_HDR Phidget_getDeviceVersion(PhidgetHandle ch, int *deviceVersion);
#ifdef INCLUDE_PRIVATE
API_PRETURN_HDR Phidget_getDeviceVINTID(PhidgetHandle ch, uint32_t *deviceVINTID);
#endif
API_PRETURN_HDR Phidget_getHub(PhidgetHandle ch, PhidgetHandle *hub);
API_PRETURN_HDR Phidget_setHubPort(PhidgetHandle ch, int hubPort);
API_PRETURN_HDR Phidget_getHubPort(PhidgetHandle ch, int *hubPort);
API_PRETURN_HDR Phidget_getHubPortCount(PhidgetHandle ch, int *hubPortCount);
API_PRETURN_HDR Phidget_setIsHubPortDevice(PhidgetHandle ch, int isHubPortDevice);
API_PRETURN_HDR Phidget_getIsHubPortDevice(PhidgetHandle ch, int *isHubPortDevice);
API_PRETURN_HDR Phidget_setHubPortSpeed(PhidgetHandle ch, uint32_t hubPortSpeed);
API_PRETURN_HDR Phidget_getHubPortSpeed(PhidgetHandle ch, uint32_t *hubPortSpeed);
API_PRETURN_HDR Phidget_getMaxHubPortSpeed(PhidgetHandle ch, uint32_t *maxHubPortSpeed);
API_PRETURN_HDR Phidget_getHubPortSupportsAutoSetSpeed(PhidgetHandle ch,
  int *hubPortSupportsAutoSetSpeed);
API_PRETURN_HDR Phidget_getHubPortSupportsSetSpeed(PhidgetHandle ch, int *hubPortSupportsSetSpeed);
API_PRETURN_HDR Phidget_setIsLocal(PhidgetHandle ch, int isLocal);
API_PRETURN_HDR Phidget_getIsLocal(PhidgetHandle ch, int *isLocal);
#ifdef INCLUDE_PRIVATE
API_PRETURN_HDR Phidget_setMeshMode(PhidgetHandle ch, Phidget_MeshMode meshMode);
API_PRETURN_HDR Phidget_getMeshMode(PhidgetHandle ch, Phidget_MeshMode *meshMode);
#endif
API_PRETURN_HDR Phidget_getIsOpen(PhidgetHandle ch, int *isOpen);
API_PRETURN_HDR Phidget_getParent(PhidgetHandle ch, PhidgetHandle *parent);
API_PRETURN_HDR Phidget_setIsRemote(PhidgetHandle ch, int isRemote);
API_PRETURN_HDR Phidget_getIsRemote(PhidgetHandle ch, int *isRemote);
API_PRETURN_HDR Phidget_getServerHostname(PhidgetHandle ch, const char **serverHostname);
API_PRETURN_HDR Phidget_setServerName(PhidgetHandle ch, const char * serverName);
API_PRETURN_HDR Phidget_getServerName(PhidgetHandle ch, const char **serverName);
API_PRETURN_HDR Phidget_getServerPeerName(PhidgetHandle ch, const char **serverPeerName);
API_PRETURN_HDR Phidget_getServerUniqueName(PhidgetHandle ch, const char **serverUniqueName);
API_PRETURN_HDR Phidget_getMaxVINTDeviceSpeed(PhidgetHandle ch, uint32_t *maxVINTDeviceSpeed);
API_PRETURN_HDR Phidget_getVINTDeviceSupportsAutoSetSpeed(PhidgetHandle ch,
  int *VINTDeviceSupportsAutoSetSpeed);
API_PRETURN_HDR Phidget_getVINTDeviceSupportsSetSpeed(PhidgetHandle ch,
  int *VINTDeviceSupportsSetSpeed);

/* Events */
typedef void (CCONV *Phidget_OnAttachCallback)(PhidgetHandle ch, void *ctx);

API_PRETURN_HDR Phidget_setOnAttachHandler(PhidgetHandle ch, Phidget_OnAttachCallback fptr, void *ctx);
typedef void (CCONV *Phidget_OnDetachCallback)(PhidgetHandle ch, void *ctx);

API_PRETURN_HDR Phidget_setOnDetachHandler(PhidgetHandle ch, Phidget_OnDetachCallback fptr, void *ctx);
typedef void (CCONV *Phidget_OnErrorCallback)(PhidgetHandle ch, void *ctx, Phidget_ErrorEventCode code,
  const char *description);

API_PRETURN_HDR Phidget_setOnErrorHandler(PhidgetHandle ch, Phidget_OnErrorCallback fptr, void *ctx);
typedef void (CCONV *Phidget_OnPropertyChangeCallback)(PhidgetHandle ch, void *ctx,
  const char *propertyName);

API_PRETURN_HDR Phidget_setOnPropertyChangeHandler(PhidgetHandle ch,
  Phidget_OnPropertyChangeCallback fptr, void *ctx);

#endif /* _PHIDGET_H_ */
