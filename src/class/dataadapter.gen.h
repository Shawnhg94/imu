#ifndef _DATAADAPTER_H_
#define _DATAADAPTER_H_
#ifdef INCLUDE_PRIVATE
typedef struct _PhidgetDataAdapter *PhidgetDataAdapterHandle;

/* Methods */
API_PRETURN_HDR PhidgetDataAdapter_create(PhidgetDataAdapterHandle *ch);
API_PRETURN_HDR PhidgetDataAdapter_delete(PhidgetDataAdapterHandle *ch);
API_PRETURN_HDR PhidgetDataAdapter_i2cComplexTransaction(PhidgetDataAdapterHandle ch, int32_t address,
  char *I2CPacketString, const uint8_t *data, size_t sendDataLen, uint8_t *recvData, size_t *recvDataLen);
API_PRETURN_HDR PhidgetDataAdapter_i2cSendReceive(PhidgetDataAdapterHandle ch, int32_t address,
  const uint8_t *data, size_t sendDataLen, uint8_t *recvData, size_t recvDataLen);
API_PRETURN_HDR PhidgetDataAdapter_sendPacket(PhidgetDataAdapterHandle ch, const uint8_t *data,
  size_t dataLen);
API_PRETURN_HDR PhidgetDataAdapter_sendPacketWaitResponse(PhidgetDataAdapterHandle ch,
  const uint8_t *data, size_t sendDataLen, uint8_t *recvData, size_t *recvDataLen);

/* Properties */
API_PRETURN_HDR PhidgetDataAdapter_setDataAdapterVoltage(PhidgetDataAdapterHandle ch,
  Phidget_DataAdapterVoltage dataAdapterVoltage);
API_PRETURN_HDR PhidgetDataAdapter_getDataAdapterVoltage(PhidgetDataAdapterHandle ch,
  Phidget_DataAdapterVoltage *dataAdapterVoltage);
API_PRETURN_HDR PhidgetDataAdapter_setDataBits(PhidgetDataAdapterHandle ch, uint32_t dataBits);
API_PRETURN_HDR PhidgetDataAdapter_getDataBits(PhidgetDataAdapterHandle ch, uint32_t *dataBits);
API_PRETURN_HDR PhidgetDataAdapter_getMinDataBits(PhidgetDataAdapterHandle ch, uint32_t *minDataBits);
API_PRETURN_HDR PhidgetDataAdapter_getMaxDataBits(PhidgetDataAdapterHandle ch, uint32_t *maxDataBits);
API_PRETURN_HDR PhidgetDataAdapter_setFrequency(PhidgetDataAdapterHandle ch,
  PhidgetDataAdapter_Frequency frequency);
API_PRETURN_HDR PhidgetDataAdapter_getFrequency(PhidgetDataAdapterHandle ch,
  PhidgetDataAdapter_Frequency *frequency);
API_PRETURN_HDR PhidgetDataAdapter_setEndianness(PhidgetDataAdapterHandle ch,
  PhidgetDataAdapter_Endianness endianness);
API_PRETURN_HDR PhidgetDataAdapter_getEndianness(PhidgetDataAdapterHandle ch,
  PhidgetDataAdapter_Endianness *endianness);
API_PRETURN_HDR PhidgetDataAdapter_getMaxReceivePacketLength(PhidgetDataAdapterHandle ch,
  uint32_t *maxReceivePacketLength);
API_PRETURN_HDR PhidgetDataAdapter_getMaxSendPacketLength(PhidgetDataAdapterHandle ch,
  uint32_t *maxSendPacketLength);
API_PRETURN_HDR PhidgetDataAdapter_setSPIChipSelect(PhidgetDataAdapterHandle ch,
  PhidgetDataAdapter_SPIChipSelect SPIChipSelect);
API_PRETURN_HDR PhidgetDataAdapter_getSPIChipSelect(PhidgetDataAdapterHandle ch,
  PhidgetDataAdapter_SPIChipSelect *SPIChipSelect);
API_PRETURN_HDR PhidgetDataAdapter_setSPIMode(PhidgetDataAdapterHandle ch,
  PhidgetDataAdapter_SPIMode SPIMode);
API_PRETURN_HDR PhidgetDataAdapter_getSPIMode(PhidgetDataAdapterHandle ch,
  PhidgetDataAdapter_SPIMode *SPIMode);

/* Events */

#endif /* INCLUDE_PRIVATE */
#endif /* _DATAADAPTER_H_ */
