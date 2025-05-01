/* Generated: Mon Mar 24 2025 13:18:57 GMT-0600 (Mountain Daylight Time) */

static void CCONV PhidgetDataAdapter_errorHandler(PhidgetChannelHandle ch, Phidget_ErrorEventCode code);
static void CCONV PhidgetDataAdapter_free(PhidgetChannelHandle *ch);
static PhidgetReturnCode CCONV PhidgetDataAdapter_bridgeInput(PhidgetChannelHandle phid,
  BridgePacket *bp);
static PhidgetReturnCode CCONV PhidgetDataAdapter_setStatus(PhidgetChannelHandle phid,
  BridgePacket *bp);
static PhidgetReturnCode CCONV PhidgetDataAdapter_getStatus(PhidgetChannelHandle phid,
  BridgePacket **bp);
static PhidgetReturnCode CCONV PhidgetDataAdapter_initAfterOpen(PhidgetChannelHandle phid);
static PhidgetReturnCode CCONV PhidgetDataAdapter_setDefaults(PhidgetChannelHandle phid);
static void CCONV PhidgetDataAdapter_fireInitialEvents(PhidgetChannelHandle phid);
static int CCONV PhidgetDataAdapter_hasInitialState(PhidgetChannelHandle phid);

struct _PhidgetDataAdapter {
	struct _PhidgetChannel phid;
	uint8_t lastData[8192];
	uint32_t lastDataIndex;
	uint8_t eventData[8193];
	uint32_t eventDataLen;
	uint32_t eventDataError;
	uint32_t lastDataLen;
	PhidgetReturnCode lastDataError;
	int lastDataRead;
	char endOfLine[8];
	Phidget_DataAdapterVoltage dataAdapterVoltage;
	uint32_t dataBits;
	uint32_t minDataBits;
	uint32_t maxDataBits;
	PhidgetDataAdapter_Frequency frequency;
	PhidgetDataAdapter_Endianness endianness;
	uint32_t maxReceivePacketLength;
	uint32_t maxSendPacketLength;
	PhidgetDataAdapter_SPIChipSelect SPIChipSelect;
	PhidgetDataAdapter_SPIMode SPIMode;
};

static PhidgetReturnCode CCONV
_setStatus(PhidgetChannelHandle phid, BridgePacket *bp) {
	PhidgetDataAdapterHandle ch;
	int nsversion;

	ch = (PhidgetDataAdapterHandle)phid;

	nsversion = getBridgePacketUInt32ByName(bp, "_class_version_");
	if (nsversion != 3) {
		loginfo("%"PRIphid": server/client class version mismatch: %d != 3 - functionality may be limited.", phid, nsversion);
	}

	if(hasBridgePacketByName(bp, "lastDataIndex"))
		ch->lastDataIndex = getBridgePacketUInt32ByName(bp, "lastDataIndex");
	if(hasBridgePacketByName(bp, "eventDataLen"))
		ch->eventDataLen = getBridgePacketUInt32ByName(bp, "eventDataLen");
	if(hasBridgePacketByName(bp, "eventDataError"))
		ch->eventDataError = getBridgePacketUInt32ByName(bp, "eventDataError");
	if(hasBridgePacketByName(bp, "lastDataLen"))
		ch->lastDataLen = getBridgePacketUInt32ByName(bp, "lastDataLen");
	if(hasBridgePacketByName(bp, "lastDataRead"))
		ch->lastDataRead = getBridgePacketInt32ByName(bp, "lastDataRead");
	if (hasBridgePacketByName(bp, "dataAdapterVoltage"))
		ch->dataAdapterVoltage = getBridgePacketInt32ByName(bp, "dataAdapterVoltage");
	if (hasBridgePacketByName(bp, "dataBits"))
		ch->dataBits = getBridgePacketUInt32ByName(bp, "dataBits");
	if (hasBridgePacketByName(bp, "minDataBits"))
		ch->minDataBits = getBridgePacketUInt32ByName(bp, "minDataBits");
	if (hasBridgePacketByName(bp, "maxDataBits"))
		ch->maxDataBits = getBridgePacketUInt32ByName(bp, "maxDataBits");
	if (hasBridgePacketByName(bp, "frequency"))
		ch->frequency = getBridgePacketInt32ByName(bp, "frequency");
	if (hasBridgePacketByName(bp, "endianness"))
		ch->endianness = getBridgePacketInt32ByName(bp, "endianness");
	if (hasBridgePacketByName(bp, "maxReceivePacketLength"))
		ch->maxReceivePacketLength = getBridgePacketUInt32ByName(bp, "maxReceivePacketLength");
	if (hasBridgePacketByName(bp, "maxSendPacketLength"))
		ch->maxSendPacketLength = getBridgePacketUInt32ByName(bp, "maxSendPacketLength");
	if (hasBridgePacketByName(bp, "SPIChipSelect"))
		ch->SPIChipSelect = getBridgePacketInt32ByName(bp, "SPIChipSelect");
	if (hasBridgePacketByName(bp, "SPIMode"))
		ch->SPIMode = getBridgePacketInt32ByName(bp, "SPIMode");

	return (EPHIDGET_OK);
}

static PhidgetReturnCode CCONV
_getStatus(PhidgetChannelHandle phid, BridgePacket **bp) {
	PhidgetDataAdapterHandle ch;

	ch = (PhidgetDataAdapterHandle)phid;

	return (createBridgePacket(bp, BP_SETSTATUS, 16, "_class_version_=%u"
	  ",lastDataIndex=%u"
	  ",eventDataLen=%u"
	  ",eventDataError=%u"
	  ",lastDataLen=%u"
	  ",lastDataRead=%d"
	  ",dataAdapterVoltage=%d"
	  ",dataBits=%u"
	  ",minDataBits=%u"
	  ",maxDataBits=%u"
	  ",frequency=%d"
	  ",endianness=%d"
	  ",maxReceivePacketLength=%u"
	  ",maxSendPacketLength=%u"
	  ",SPIChipSelect=%d"
	  ",SPIMode=%d"
	  ,3 /* class version */
	  ,ch->lastDataIndex
	  ,ch->eventDataLen
	  ,ch->eventDataError
	  ,ch->lastDataLen
	  ,ch->lastDataRead
	  ,ch->dataAdapterVoltage
	  ,ch->dataBits
	  ,ch->minDataBits
	  ,ch->maxDataBits
	  ,ch->frequency
	  ,ch->endianness
	  ,ch->maxReceivePacketLength
	  ,ch->maxSendPacketLength
	  ,ch->SPIChipSelect
	  ,ch->SPIMode
	));
}

static PhidgetReturnCode CCONV
_bridgeInput(PhidgetChannelHandle phid, BridgePacket *bp) {
	PhidgetDataAdapterHandle ch;
	PhidgetReturnCode res;

	ch = (PhidgetDataAdapterHandle)phid;
	res = EPHIDGET_OK;

	switch (bp->vpkt) {
	case BP_I2CDATAEXCHANGE:
		res = DEVBRIDGEINPUT(phid, bp);
		break;
	case BP_DATAOUT:
		res = DEVBRIDGEINPUT(phid, bp);
		break;
	case BP_DATAEXCHANGE:
		res = DEVBRIDGEINPUT(phid, bp);
		break;
	case BP_SETIOVOLTAGE:
		if (!supportedDataAdapterVoltage(phid, (Phidget_DataAdapterVoltage)getBridgePacketInt32(bp,
		  0)))
			return (MOS_ERROR(bp->iop, EPHIDGET_INVALIDARG,
			  "Specified DataAdapterVoltage is unsupported by this device."));
		res = DEVBRIDGEINPUT(phid, bp);
		if (res != EPHIDGET_OK)
			break;
		ch->dataAdapterVoltage = getBridgePacketInt32(bp, 0);
		if (bridgePacketIsFromNet(bp)) {
			FIRE_PROPERTYCHANGE(ch, "DataAdapterVoltage");
		}
		break;
	case BP_SETDATABITS:
		TESTRANGE_IOP(bp->iop, "%"PRIu32, getBridgePacketUInt32(bp, 0), ch->minDataBits,
		  ch->maxDataBits);
		res = DEVBRIDGEINPUT(phid, bp);
		if (res != EPHIDGET_OK)
			break;
		ch->dataBits = getBridgePacketUInt32(bp, 0);
		if (bridgePacketIsFromNet(bp)) {
			FIRE_PROPERTYCHANGE(ch, "DataBits");
		}
		break;
	case BP_SETBAUDRATE:
		if (!supportedDataAdapterFrequency(phid, (PhidgetDataAdapter_Frequency)getBridgePacketInt32(bp,
		  0)))
			return (MOS_ERROR(bp->iop, EPHIDGET_INVALIDARG,
			  "Specified DataAdapterFrequency is unsupported by this device."));
		res = DEVBRIDGEINPUT(phid, bp);
		if (res != EPHIDGET_OK)
			break;
		ch->frequency = getBridgePacketInt32(bp, 0);
		if (bridgePacketIsFromNet(bp)) {
			FIRE_PROPERTYCHANGE(ch, "Frequency");
		}
		break;
	case BP_SETENDIANNESS:
		if (!supportedDataAdapterEndianness(phid,
		  (PhidgetDataAdapter_Endianness)getBridgePacketInt32(bp, 0)))
			return (MOS_ERROR(bp->iop, EPHIDGET_INVALIDARG,
			  "Specified DataAdapterEndianness is unsupported by this device."));
		res = DEVBRIDGEINPUT(phid, bp);
		if (res != EPHIDGET_OK)
			break;
		ch->endianness = getBridgePacketInt32(bp, 0);
		if (bridgePacketIsFromNet(bp)) {
			FIRE_PROPERTYCHANGE(ch, "Endianness");
		}
		break;
	case BP_SETSPICHIPSELECT:
		if (!supportedDataAdapterSPIChipSelect(phid,
		  (PhidgetDataAdapter_SPIChipSelect)getBridgePacketInt32(bp, 0)))
			return (MOS_ERROR(bp->iop, EPHIDGET_INVALIDARG,
			  "Specified DataAdapterSPIChipSelect is unsupported by this device."));
		res = DEVBRIDGEINPUT(phid, bp);
		if (res != EPHIDGET_OK)
			break;
		ch->SPIChipSelect = getBridgePacketInt32(bp, 0);
		if (bridgePacketIsFromNet(bp)) {
			FIRE_PROPERTYCHANGE(ch, "SPIChipSelect");
		}
		break;
	case BP_SETSPIMODE:
		if (!supportedDataAdapterSPIMode(phid, (PhidgetDataAdapter_SPIMode)getBridgePacketInt32(bp,
		  0)))
			return (MOS_ERROR(bp->iop, EPHIDGET_INVALIDARG,
			  "Specified DataAdapterSPIMode is unsupported by this device."));
		res = DEVBRIDGEINPUT(phid, bp);
		if (res != EPHIDGET_OK)
			break;
		ch->SPIMode = getBridgePacketInt32(bp, 0);
		if (bridgePacketIsFromNet(bp)) {
			FIRE_PROPERTYCHANGE(ch, "SPIMode");
		}
		break;
	default:
		logerr("%"PRIphid": unsupported bridge packet:0x%x", phid, bp->vpkt);
		res = EPHIDGET_UNSUPPORTED;
	}

	return (res);
}

static PhidgetReturnCode CCONV
_initAfterOpen(PhidgetChannelHandle phid) {
	PhidgetDataAdapterHandle ch;
	PhidgetReturnCode ret;

	TESTPTR(phid);
	ch = (PhidgetDataAdapterHandle)phid;

	ret = EPHIDGET_OK;


	switch (phid->UCD->uid) {
	default:
		MOS_PANIC("Unsupported Channel");
	}

	memset(ch->lastData, 0, sizeof (uint8_t) * 8192);
	ch->lastDataIndex = 0;
	memset(ch->eventData, 0, sizeof (uint8_t) * 8193);
	ch->eventDataLen = 0;
	ch->eventDataError = 0;
	ch->lastDataLen = 0;
	ch->lastDataError = 0;
	ch->lastDataRead = 0;
	memset(ch->endOfLine, 0, sizeof (char) * 8);

	return (ret);
}

static PhidgetReturnCode CCONV
_setDefaults(PhidgetChannelHandle phid) {
	PhidgetReturnCode ret;

	TESTPTR(phid);

	ret = EPHIDGET_OK;

	switch (phid->UCD->uid) {
	default:
		MOS_PANIC("Unsupported Channel");
	}

	return (ret);
}

static void CCONV
_fireInitialEvents(PhidgetChannelHandle phid) {

}

static int CCONV
_hasInitialState(PhidgetChannelHandle phid) {

	return (PTRUE);
}

static void CCONV
_free(PhidgetChannelHandle *ch) {

	mos_free(*ch, sizeof (struct _PhidgetDataAdapter));
}

static PhidgetReturnCode CCONV
_create(PhidgetDataAdapterHandle *phidp) {

	CHANNELCREATE_BODY(DataAdapter, PHIDCHCLASS_DATAADAPTER);
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetDataAdapter_delete(PhidgetDataAdapterHandle *phidp) {

	return (Phidget_delete((PhidgetHandle *)phidp));
}

API_PRETURN
PhidgetDataAdapter_setDataAdapterVoltage(PhidgetDataAdapterHandle ch,
  Phidget_DataAdapterVoltage dataAdapterVoltage) {

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);
	TESTATTACHED_PR(ch);

	return (bridgeSendToDevice((PhidgetChannelHandle)ch, BP_SETIOVOLTAGE, NULL, NULL, 1, "%d",
	  dataAdapterVoltage));
}

API_PRETURN
PhidgetDataAdapter_getDataAdapterVoltage(PhidgetDataAdapterHandle ch,
  Phidget_DataAdapterVoltage *dataAdapterVoltage) {

	TESTPTR_PR(ch);
	TESTPTR_PR(dataAdapterVoltage);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);
	TESTATTACHED_PR(ch);

	*dataAdapterVoltage = ch->dataAdapterVoltage;
	if (ch->dataAdapterVoltage == (Phidget_DataAdapterVoltage)PUNK_ENUM)
		return (PHID_RETURN(EPHIDGET_UNKNOWNVAL));
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetDataAdapter_setDataBits(PhidgetDataAdapterHandle ch, uint32_t dataBits) {

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);
	TESTATTACHED_PR(ch);

	return (bridgeSendToDevice((PhidgetChannelHandle)ch, BP_SETDATABITS, NULL, NULL, 1, "%u",
	  dataBits));
}

API_PRETURN
PhidgetDataAdapter_getDataBits(PhidgetDataAdapterHandle ch, uint32_t *dataBits) {

	TESTPTR_PR(ch);
	TESTPTR_PR(dataBits);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);
	TESTATTACHED_PR(ch);

	*dataBits = ch->dataBits;
	if (ch->dataBits == (uint32_t)PUNK_UINT32)
		return (PHID_RETURN(EPHIDGET_UNKNOWNVAL));
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetDataAdapter_getMinDataBits(PhidgetDataAdapterHandle ch, uint32_t *minDataBits) {

	TESTPTR_PR(ch);
	TESTPTR_PR(minDataBits);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);
	TESTATTACHED_PR(ch);

	*minDataBits = ch->minDataBits;
	if (ch->minDataBits == (uint32_t)PUNK_UINT32)
		return (PHID_RETURN(EPHIDGET_UNKNOWNVAL));
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetDataAdapter_getMaxDataBits(PhidgetDataAdapterHandle ch, uint32_t *maxDataBits) {

	TESTPTR_PR(ch);
	TESTPTR_PR(maxDataBits);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);
	TESTATTACHED_PR(ch);

	*maxDataBits = ch->maxDataBits;
	if (ch->maxDataBits == (uint32_t)PUNK_UINT32)
		return (PHID_RETURN(EPHIDGET_UNKNOWNVAL));
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetDataAdapter_setFrequency(PhidgetDataAdapterHandle ch, PhidgetDataAdapter_Frequency frequency) {

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);
	TESTATTACHED_PR(ch);

	return (bridgeSendToDevice((PhidgetChannelHandle)ch, BP_SETBAUDRATE, NULL, NULL, 1, "%d",
	  frequency));
}

API_PRETURN
PhidgetDataAdapter_getFrequency(PhidgetDataAdapterHandle ch, PhidgetDataAdapter_Frequency *frequency) {

	TESTPTR_PR(ch);
	TESTPTR_PR(frequency);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);
	TESTATTACHED_PR(ch);

	*frequency = ch->frequency;
	if (ch->frequency == (PhidgetDataAdapter_Frequency)PUNK_ENUM)
		return (PHID_RETURN(EPHIDGET_UNKNOWNVAL));
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetDataAdapter_setEndianness(PhidgetDataAdapterHandle ch,
  PhidgetDataAdapter_Endianness endianness) {

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);
	TESTATTACHED_PR(ch);

	return (bridgeSendToDevice((PhidgetChannelHandle)ch, BP_SETENDIANNESS, NULL, NULL, 1, "%d",
	  endianness));
}

API_PRETURN
PhidgetDataAdapter_getEndianness(PhidgetDataAdapterHandle ch,
  PhidgetDataAdapter_Endianness *endianness) {

	TESTPTR_PR(ch);
	TESTPTR_PR(endianness);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);
	TESTATTACHED_PR(ch);

	*endianness = ch->endianness;
	if (ch->endianness == (PhidgetDataAdapter_Endianness)PUNK_ENUM)
		return (PHID_RETURN(EPHIDGET_UNKNOWNVAL));
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetDataAdapter_getMaxReceivePacketLength(PhidgetDataAdapterHandle ch,
  uint32_t *maxReceivePacketLength) {

	TESTPTR_PR(ch);
	TESTPTR_PR(maxReceivePacketLength);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);
	TESTATTACHED_PR(ch);

	*maxReceivePacketLength = ch->maxReceivePacketLength;
	if (ch->maxReceivePacketLength == (uint32_t)PUNK_UINT32)
		return (PHID_RETURN(EPHIDGET_UNKNOWNVAL));
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetDataAdapter_getMaxSendPacketLength(PhidgetDataAdapterHandle ch, uint32_t *maxSendPacketLength) {

	TESTPTR_PR(ch);
	TESTPTR_PR(maxSendPacketLength);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);
	TESTATTACHED_PR(ch);

	*maxSendPacketLength = ch->maxSendPacketLength;
	if (ch->maxSendPacketLength == (uint32_t)PUNK_UINT32)
		return (PHID_RETURN(EPHIDGET_UNKNOWNVAL));
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetDataAdapter_setSPIChipSelect(PhidgetDataAdapterHandle ch,
  PhidgetDataAdapter_SPIChipSelect SPIChipSelect) {

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);
	TESTATTACHED_PR(ch);

	return (bridgeSendToDevice((PhidgetChannelHandle)ch, BP_SETSPICHIPSELECT, NULL, NULL, 1, "%d",
	  SPIChipSelect));
}

API_PRETURN
PhidgetDataAdapter_getSPIChipSelect(PhidgetDataAdapterHandle ch,
  PhidgetDataAdapter_SPIChipSelect *SPIChipSelect) {

	TESTPTR_PR(ch);
	TESTPTR_PR(SPIChipSelect);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);
	TESTATTACHED_PR(ch);

	*SPIChipSelect = ch->SPIChipSelect;
	if (ch->SPIChipSelect == (PhidgetDataAdapter_SPIChipSelect)PUNK_ENUM)
		return (PHID_RETURN(EPHIDGET_UNKNOWNVAL));
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetDataAdapter_setSPIMode(PhidgetDataAdapterHandle ch, PhidgetDataAdapter_SPIMode SPIMode) {

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);
	TESTATTACHED_PR(ch);

	return (bridgeSendToDevice((PhidgetChannelHandle)ch, BP_SETSPIMODE, NULL, NULL, 1, "%d", SPIMode));
}

API_PRETURN
PhidgetDataAdapter_getSPIMode(PhidgetDataAdapterHandle ch, PhidgetDataAdapter_SPIMode *SPIMode) {

	TESTPTR_PR(ch);
	TESTPTR_PR(SPIMode);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_DATAADAPTER);
	TESTATTACHED_PR(ch);

	*SPIMode = ch->SPIMode;
	if (ch->SPIMode == (PhidgetDataAdapter_SPIMode)PUNK_ENUM)
		return (PHID_RETURN(EPHIDGET_UNKNOWNVAL));
	return (EPHIDGET_OK);
}
