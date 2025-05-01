/* Generated: Wed Jun 22 2016 14:15:21 GMT-0600 (Mountain Daylight Time) */
/* Will not be regenerated. */

#include "phidgetbase.h"
#include "util/dataadaptersupport.h"
#include "class/dataadapter.gen.h"
#include "class/dataadapter.gen.c"

// Access the PhidgetDataAdapterSupport struct via the channel private pointer
#define DATAADAPTER_SUPPORT(ch) ((PhidgetDataAdapterSupportHandle)(((PhidgetChannelHandle)(ch))->private))

static PhidgetReturnCode PhidgetDataAdapter_setI2CFormat(PhidgetDataAdapterHandle ch, const char *format);
static PhidgetReturnCode PhidgetDataAdapter_setDeviceAddress(PhidgetDataAdapterHandle ch, uint32_t deviceAddress);

static void CCONV
PhidgetDataAdapter_errorHandler(PhidgetChannelHandle phid, Phidget_ErrorEventCode code) {
}

static void CCONV
PhidgetDataAdapter_free(PhidgetChannelHandle *ch) {

	if (ch && *ch)
		PhidgetDataAdapterSupport_free((PhidgetDataAdapterSupportHandle *)&(*ch)->private);
	_free(ch);
}

API_PRETURN
PhidgetDataAdapter_create(PhidgetDataAdapterHandle *phidp) {
	PhidgetReturnCode res;

	res = _create(phidp);
	if (res == EPHIDGET_OK)
		res = PhidgetDataAdapterSupport_create((PhidgetDataAdapterSupportHandle *)&(*phidp)->phid.private);

	return (res);
}

static PhidgetReturnCode CCONV
PhidgetDataAdapter_setStatus(PhidgetChannelHandle phid, BridgePacket *bp) {
	PhidgetDataAdapterHandle ch;
	PhidgetReturnCode res;

	res = _setStatus(phid, bp);
	if (res != EPHIDGET_OK)
		return (res);

	ch = (PhidgetDataAdapterHandle)phid;

	return (EPHIDGET_OK);
}

static PhidgetReturnCode CCONV
PhidgetDataAdapter_getStatus(PhidgetChannelHandle phid, BridgePacket **bp) {
	PhidgetDataAdapterHandle ch;
	PhidgetReturnCode res;

	res = _getStatus(phid, bp);
	if (res != EPHIDGET_OK)
		return (res);

	ch = (PhidgetDataAdapterHandle)phid;
	if (res != EPHIDGET_OK)
		return (res);

	return (EPHIDGET_OK);
}

static PhidgetReturnCode CCONV
PhidgetDataAdapter_initAfterOpen(PhidgetChannelHandle phid) {
	PhidgetDataAdapterSupport_init(DATAADAPTER_SUPPORT(phid));
	return (_initAfterOpen(phid));
}

static PhidgetReturnCode CCONV
PhidgetDataAdapter_setDefaults(PhidgetChannelHandle phid) {
	return (_setDefaults(phid));
}

static PhidgetReturnCode
PhidgetDataAdapter_bridgeInput(PhidgetChannelHandle phid, BridgePacket *bp) {
	PhidgetDataAdapterHandle ch;
	PhidgetReturnCode res;

	ch = (PhidgetDataAdapterHandle)phid;

	switch (bp->vpkt) {
	case BP_IOVOLTAGECHANGE:
		ch->dataAdapterVoltage = getBridgePacketInt32(bp, 0);
		FIRE_PROPERTYCHANGE(ch, "DataAdapterVoltage");
		return (EPHIDGET_OK);
	case BP_SETI2CFORMAT:
	case BP_DATAEXCHANGE:
	case BP_SETADDRESS:
		switch (ch->phid.UCD->uid) {
#if (PHIDUID_ADP_I2C_USB_SUPPORTED || PHIDUID_ADP0001_VINT_SUPPORTED)
	#if PHIDUID_ADP0001_USB_SUPPORTED
		case PHIDCHUID_ADP0001_DATAADAPTER_100_USB:
	#endif
	#if PHIDUID_ADP0001_VINT_SUPPORTED
		case PHIDCHUID_ADP0001_DATAADAPTER_100_VINT:
	#endif
			return (DEVBRIDGEINPUT(phid, bp));
#endif
		default:
			break;
		}
		break;
	default:
		break;
	}
	res = _bridgeInput(phid, bp);
	return (res);
}

static void
PhidgetDataAdapter_fireInitialEvents(PhidgetChannelHandle phid) {
	_fireInitialEvents(phid);
}

static int
PhidgetDataAdapter_hasInitialState(PhidgetChannelHandle phid) {
	return (_hasInitialState(phid));
}

API_PRETURN
PhidgetDataAdapter_sendPacket(PhidgetDataAdapterHandle ch, const uint8_t *data, size_t length) {
	PhidgetReturnCode res = EPHIDGET_OK;

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);
	TESTATTACHED_PR(ch);

	if (length == 0)
		return EPHIDGET_OK;

	mos_mutex_lock(&DATAADAPTER_SUPPORT(ch)->sendLock);
	if (length > ch->maxSendPacketLength) {
		mos_mutex_unlock(&DATAADAPTER_SUPPORT(ch)->sendLock);
		return (PHID_RETURN_ERRSTR(EPHIDGET_INVALIDARG, "Packet length too long."));
	}

	res = bridgeSendToDevice((PhidgetChannelHandle)ch, BP_DATAOUT, NULL, NULL, 1, "%*R", length, data);
	if (res != EPHIDGET_OK) {
		mos_mutex_unlock(&DATAADAPTER_SUPPORT(ch)->sendLock);
		if (res == EPHIDGET_TIMEOUT)
			return (PHID_RETURN_ERRSTR(EPHIDGET_TIMEOUT, "The packet timed out while waiting to be transmitted. Check that your system can keep up with the data being sent."));
		return res;
	}

	mos_mutex_unlock(&DATAADAPTER_SUPPORT(ch)->sendLock);
	if (res == EPHIDGET_TIMEOUT)
		return (PHID_RETURN_ERRSTR(EPHIDGET_TIMEOUT, "The packet timed out while waiting to be transmitted. Check that your system can keep up with the data being sent."));
	return res;
}

API_VRETURN
PhidgetDataAdapter_sendPacket_async(PhidgetDataAdapterHandle ch, const uint8_t *data, size_t length,
									Phidget_AsyncCallback fptr, void *ctx) {
	PhidgetReturnCode res = EPHIDGET_OK;
	//	uint32_t maxBridgeLength;
	//	uint32_t i;

	if (ch == NULL) {
		if (fptr)
			fptr((PhidgetHandle)ch, ctx, EPHIDGET_INVALIDARG);
		return;
	}
	if (ch->phid.class != PHIDCHCLASS_DATAADAPTER) {
		if (fptr)
			fptr((PhidgetHandle)ch, ctx, EPHIDGET_WRONGDEVICE);
		return;
	}
	if (!ISATTACHED(ch)) {
		if (fptr)
			fptr((PhidgetHandle)ch, ctx, EPHIDGET_NOTATTACHED);
		return;
	}
	if (length > ch->maxSendPacketLength) {
		if (fptr)
			fptr((PhidgetHandle)ch, ctx, EPHIDGET_INVALIDARG);
		return;
	}

	// Could error out, but technically we did send 0 bytes
	if (length == 0)
		return;

	res = bridgeSendToDevice((PhidgetChannelHandle)ch, BP_DATAOUT, fptr, ctx, 1, "%*R", length, data);

	if (res != EPHIDGET_OK && fptr != NULL)
		fptr((PhidgetHandle)ch, ctx, res);
}

API_PRETURN
PhidgetDataAdapter_i2cSendReceive(PhidgetDataAdapterHandle ch, int address, const uint8_t *data, size_t length, uint8_t *recvData, size_t recvDataLen) {
	TESTPTR_PR(ch);
	TESTPTR_PR(data);
	TESTPTR_PR(recvData);
	TESTRANGE_PR((int)recvDataLen, "%d", 0, ch->maxReceivePacketLength);
	TESTRANGE_PR((int)length, "%d", 0, ch->maxSendPacketLength);
	TESTRANGE_PR(address, "%d", 0, 127);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);

	TESTATTACHED_PR(ch);

	char formatString[128];

	if (length == 0 && recvDataLen == 0)
		return EPHIDGET_OK;
	else if (recvDataLen == 0)
		sprintf(formatString, "sT%dp", (int)length);
	else if (length == 0)
		sprintf(formatString, "sR%dp", (int)recvDataLen);
	else
		sprintf(formatString, "sT%dsR%dp", (int)length, (int)(recvDataLen));

	return PhidgetDataAdapter_i2cComplexTransaction(ch, address, formatString, data, length, recvData, &recvDataLen);
}

API_PRETURN
PhidgetDataAdapter_i2cComplexTransaction(PhidgetDataAdapterHandle ch, int address, char *formatString, const uint8_t *data, size_t length, uint8_t *recvData, size_t *recvDataLen) {
	uint8_t response[DATAADAPTER_MAX_PACKET_LENGTH];
	size_t responseLen = DATAADAPTER_MAX_PACKET_LENGTH;
	PhidgetReturnCode res;
	TESTPTR_PR(ch);
	TESTPTR_PR(data);
	TESTPTR_PR(recvData);
	TESTPTR_PR(formatString);
	TESTRANGE_PR((int)*recvDataLen, "%u", 0, ch->maxReceivePacketLength);
	TESTRANGE_PR((int)length, "%u", 0, ch->maxSendPacketLength);
	TESTRANGE_PR(address, "%d", 0, 127);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);

	TESTATTACHED_PR(ch);

	mos_mutex_lock(&DATAADAPTER_SUPPORT(ch)->sendLock);
	res = bridgeSendToDeviceWithReply((PhidgetChannelHandle)ch, BP_I2CDATAEXCHANGE, NULL, NULL, (uint8_t *)response, (uint32_t *)&responseLen, 1, "%*R%u%s", length, data, address, formatString);

	if (res) {
		mos_mutex_unlock(&DATAADAPTER_SUPPORT(ch)->sendLock);
		if (res == EPHIDGET_TIMEOUT)
			return (PHID_RETURN_ERRSTR(EPHIDGET_TIMEOUT, "The packet timed out while waiting to be transmitted. Check that your system can keep up with the data being sent."));
		return res;
	}

	if (responseLen >= 4)
		responseLen -= 4;
	else
		MOS_PANIC("Malformed response");

	if (res == EPHIDGET_OK) {
		switch (unpack32(&response[responseLen])) {
		case PACKET_ERROR_OK:
			break;
		case PACKET_ERROR_TIMEOUT:
			mos_mutex_unlock(&DATAADAPTER_SUPPORT(ch)->sendLock);
			return (PHID_RETURN_ERRSTR(EPHIDGET_TIMEOUT, "Packet timed out"));
		case PACKET_ERROR_UNKNOWN:
		case PACKET_ERROR_FORMAT:
		case PACKET_ERROR_INVALID:
		case PACKET_ERROR_OVERRUN:
		case PACKET_ERROR_CORRUPT:
			mos_mutex_unlock(&DATAADAPTER_SUPPORT(ch)->sendLock);
			return (PHID_RETURN_ERRSTR(EPHIDGET_INTERRUPTED, "Something happened to corrupt the response data."));
		case PACKET_ERROR_NACK:
			mos_mutex_unlock(&DATAADAPTER_SUPPORT(ch)->sendLock);
			return (PHID_RETURN_ERRSTR(EPHIDGET_NACK, "Packet was NACKed by the external device"));
		}
	}

	if (*recvDataLen < responseLen) {
		mos_mutex_unlock(&DATAADAPTER_SUPPORT(ch)->sendLock);
		*recvDataLen = 0;
		return (PHID_RETURN_ERRSTR(EPHIDGET_INVALIDARG, "Receive array length too short."));
	}

	*recvDataLen = responseLen;
	memcpy(recvData, response, responseLen);
	if (res == EPHIDGET_OK)
		unpack32(&response[responseLen]);

	mos_mutex_unlock(&DATAADAPTER_SUPPORT(ch)->sendLock);
	return (res);
}

API_PRETURN
PhidgetDataAdapter_sendPacketWaitResponse(PhidgetDataAdapterHandle ch, const uint8_t *data, size_t length, uint8_t *recvData, size_t *recvDataLen) {
	PhidgetReturnCode res;
	TESTPTR_PR(ch);
	TESTPTR_PR(data);
	TESTPTR_PR(recvData);
	TESTPTR_PR(recvDataLen);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);
	TESTATTACHED_PR(ch);
	uint8_t response[DATAADAPTER_MAX_PACKET_LENGTH];
	size_t responseLen = DATAADAPTER_MAX_PACKET_LENGTH;
	uint32_t maxSendReceiveLength;

	switch (ch->phid.parent->deviceInfo.UDD->uid) {
#if (PHIDUID_ADP0001_USB_SUPPORTED || PHIDUID_ADP0001_VINT_SUPPORTED)
	#if PHIDUID_ADP0001_USB_SUPPORTED
	case PHIDUID_ADP0001_USB:
	#endif
	#if PHIDUID_ADP0001_VINT_SUPPORTED
	case PHIDUID_ADP0001_VINT:
	#endif
		// send 0 and expecting a response is valid for I2C
		break;
#endif
	default:
		// Sending nothing is technically valid, and already done.
		if (length == 0)
			return EPHIDGET_OK;
		break;
	}

	switch (ch->phid.UCD->uid) {
#if (PHIDUID_ADP0002_USB_SUPPORTED || PHIDUID_ADP0002_VINT_SUPPORTED)
	#if PHIDUID_ADP0002_VINT_SUPPORTED
	case PHIDCHUID_ADP0002_DATAADAPTER_100_VINT:
	#endif
	#if PHIDUID_ADP0002_USB_SUPPORTED
	case PHIDCHUID_ADP0002_DATAADAPTER_100_USB:
	#endif
		maxSendReceiveLength = ch->maxReceivePacketLength;
		break;
#endif
	default:
		maxSendReceiveLength = ch->maxSendPacketLength;
		break;
	}
	if ((uint32_t)length > maxSendReceiveLength) {
		*recvDataLen = 0;
		return (PHID_RETURN_ERRSTR(EPHIDGET_INVALIDARG, "Packet length too long."));
	}

	mos_mutex_lock(&DATAADAPTER_SUPPORT(ch)->sendLock);
	res = bridgeSendToDeviceWithReply((PhidgetChannelHandle)ch, BP_DATAEXCHANGE, NULL, NULL, (uint8_t *)response, (uint32_t *)&responseLen, 1, "%*R", length, data);

	if (res) {
		mos_mutex_unlock(&DATAADAPTER_SUPPORT(ch)->sendLock);
		*recvDataLen = 0;
		if (res == EPHIDGET_TIMEOUT)
			return (PHID_RETURN_ERRSTR(EPHIDGET_TIMEOUT, "The packet timed out while waiting to be transmitted. Check that your system can keep up with the data being sent."));
		return res;
	}

	if (responseLen >= 4)
		responseLen -= 4;
	else
		MOS_PANIC("Malformed response");

	if (*recvDataLen < responseLen) {
		mos_mutex_unlock(&DATAADAPTER_SUPPORT(ch)->sendLock);
		*recvDataLen = 0;
		return (PHID_RETURN_ERRSTR(EPHIDGET_INVALIDARG, "Receive array length too short."));
	}

	// Check if we can null terminate
	if (*recvDataLen >= (responseLen + 1))
		recvData[ch->eventDataLen] = 0;

	if (res == EPHIDGET_OK) {
		switch (unpack32(&response[responseLen])) {
		case PACKET_ERROR_OK:
			break;
		case PACKET_ERROR_TIMEOUT:
			mos_mutex_unlock(&DATAADAPTER_SUPPORT(ch)->sendLock);
			return (PHID_RETURN_ERRSTR(EPHIDGET_TIMEOUT, "Packet timed out"));
		case PACKET_ERROR_UNKNOWN:
		case PACKET_ERROR_FORMAT:
		case PACKET_ERROR_INVALID:
		case PACKET_ERROR_OVERRUN:
		case PACKET_ERROR_CORRUPT:
			mos_mutex_unlock(&DATAADAPTER_SUPPORT(ch)->sendLock);
			return (PHID_RETURN_ERRSTR(EPHIDGET_INTERRUPTED, "Something happened to corrupt the response data."));
		case PACKET_ERROR_NACK:
			mos_mutex_unlock(&DATAADAPTER_SUPPORT(ch)->sendLock);
			return (PHID_RETURN_ERRSTR(EPHIDGET_NACK, "Packet was NACKed by the external device"));
		}
	}

	memcpy(recvData, response, responseLen);
	*recvDataLen = responseLen;
	if (res == EPHIDGET_OK)
		unpack32(&response[responseLen]);

	mos_mutex_unlock(&DATAADAPTER_SUPPORT(ch)->sendLock);
	return (res);
}
