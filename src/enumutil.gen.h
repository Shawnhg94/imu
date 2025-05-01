#ifndef _ENUM_UTIL_H_
#define _ENUM_UTIL_H_

API_CRETURN_HDR Phidget_enumString(const char *, int);
API_IRETURN_HDR Phidget_enumFromString(const char *);

#ifndef EXTERNALPROTO
int supportedEncoderIOMode(PhidgetChannelHandle ch, Phidget_EncoderIOMode val);
int supportedDeviceID(PhidgetChannelHandle ch, Phidget_DeviceID val);
int supportedDeviceClass(PhidgetChannelHandle ch, Phidget_DeviceClass val);
int supportedChannelClass(PhidgetChannelHandle ch, Phidget_ChannelClass val);
int supportedChannelSubclass(PhidgetChannelHandle ch, Phidget_ChannelSubclass val);
int supportedMeshMode(PhidgetChannelHandle ch, Phidget_MeshMode val);
int supportedPowerSupply(PhidgetChannelHandle ch, Phidget_PowerSupply val);
int supportedDataAdapterVoltage(PhidgetChannelHandle ch, Phidget_DataAdapterVoltage val);
int supportedRTDWireSetup(PhidgetChannelHandle ch, Phidget_RTDWireSetup val);
int supportedInputMode(PhidgetChannelHandle ch, Phidget_InputMode val);
int supportedFanMode(PhidgetChannelHandle ch, Phidget_FanMode val);
int supportedDriveMode(PhidgetChannelHandle ch, Phidget_DriveMode val);
int supportedPositionType(PhidgetChannelHandle ch, Phidget_PositionType val);
int supportedSpatialPrecision(PhidgetChannelHandle ch, Phidget_SpatialPrecision val);
int supportedUnit(PhidgetChannelHandle ch, Phidget_Unit val);
int supportedBridgeGain(PhidgetChannelHandle ch, PhidgetVoltageRatioInput_BridgeGain val);
int supportedVoltageRatioSensorType(PhidgetChannelHandle ch, PhidgetVoltageRatioInput_SensorType val);
int supportedLEDForwardVoltage(PhidgetChannelHandle ch, PhidgetDigitalOutput_LEDForwardVoltage val);
int supportedRCServoVoltage(PhidgetChannelHandle ch, PhidgetRCServo_Voltage val);
int supportedVoltageOutputRange(PhidgetChannelHandle ch, PhidgetVoltageOutput_VoltageOutputRange val);
int supportedVoltageRange(PhidgetChannelHandle ch, PhidgetVoltageInput_VoltageRange val);
int supportedVoltageSensorType(PhidgetChannelHandle ch, PhidgetVoltageInput_SensorType val);
int supportedRFIDProtocol(PhidgetChannelHandle ch, PhidgetRFID_Protocol val);
int supportedRFIDChipset(PhidgetChannelHandle ch, PhidgetRFID_Chipset val);
int supportedSpatialAlgorithm(PhidgetChannelHandle ch, Phidget_SpatialAlgorithm val);
int supportedRTDType(PhidgetChannelHandle ch, PhidgetTemperatureSensor_RTDType val);
int supportedThermocoupleType(PhidgetChannelHandle ch, PhidgetTemperatureSensor_ThermocoupleType val);
int supportedFilterType(PhidgetChannelHandle ch, PhidgetFrequencyCounter_FilterType val);
int supportedEncoding(PhidgetChannelHandle ch, PhidgetIR_Encoding val);
int supportedLength(PhidgetChannelHandle ch, PhidgetIR_Length val);
int supportedControlMode(PhidgetChannelHandle ch, PhidgetStepper_ControlMode val);
int supportedLCDFont(PhidgetChannelHandle ch, PhidgetLCD_Font val);
int supportedLCDScreenSize(PhidgetChannelHandle ch, PhidgetLCD_ScreenSize val);
int supportedLCDPixelState(PhidgetChannelHandle ch, PhidgetLCD_PixelState val);
int supportedDataAdapterSPIMode(PhidgetChannelHandle ch, PhidgetDataAdapter_SPIMode val);
int supportedDataAdapterFrequency(PhidgetChannelHandle ch, PhidgetDataAdapter_Frequency val);
int supportedPacketErrorCode(PhidgetChannelHandle ch, PhidgetDataAdapter_PacketErrorCode val);
int supportedDataAdapterSPIChipSelect(PhidgetChannelHandle ch, PhidgetDataAdapter_SPIChipSelect val);
int supportedDataAdapterEndianness(PhidgetChannelHandle ch, PhidgetDataAdapter_Endianness val);
int supportedLEDArrayProtocol(PhidgetChannelHandle ch, PhidgetLEDArray_Protocol val);
int supportedLEDArrayAnimationType(PhidgetChannelHandle ch, PhidgetLEDArray_AnimationType val);
int supportedSPLRange(PhidgetChannelHandle ch, PhidgetSoundSensor_SPLRange val);
int supportedPortMode(PhidgetChannelHandle ch, PhidgetHub_PortMode val);
#endif
#endif /* _ENUM_UTIL_H_ */
