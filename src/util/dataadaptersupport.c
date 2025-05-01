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

#include "phidgetbase.h"
#include "util/dataadaptersupport.h"

// Access the PhidgetIRSupport struct via the channel private pointer
#define DATAADAPTER_SUPPORT(ch) ((PhidgetDataAdapterSupportHandle)(((PhidgetChannelHandle)(ch))->private))

static PhidgetReturnCode
SetNAK(PhidgetChannelHandle ch) {

	PhidgetLock(ch);
	DATAADAPTER_SUPPORT(ch)->nakFlag = 1;
	PhidgetBroadcast(ch);
	PhidgetUnlock(ch);
	return (EPHIDGET_OK);
}

#if (PHIDUID_ADP0001_USB_SUPPORTED || PHIDUID_ADP0002_USB_SUPPORTED)
static PhidgetReturnCode
ClearNAK(PhidgetChannelHandle ch) {

	PhidgetLock(ch);
	DATAADAPTER_SUPPORT(ch)->nakFlag = 0;
	PhidgetBroadcast(ch);
	PhidgetUnlock(ch);
	return (EPHIDGET_OK);
}

// === Internal Functions === //
static PhidgetReturnCode
processDataPackets(PhidgetChannelHandle ch, const uint8_t *buffer, size_t length) {
	PhidgetDataAdapterSupportHandle dataAdapterSupport = DATAADAPTER_SUPPORT(ch);
	uint16_t receivedPacketCount;

	size_t usedPacketLength = 0;
	int endPacket = 0;
	int error = 0;
	int packetOverhead = USB_IN_PACKET_OVERHEAD;
	PhidgetReturnCode errorCode = EPHIDGET_OK;
	int newResponsePacket = 0;
	uint16_t packetInfo;
	uint16_t packetID;

	switch (buffer[0]) {
	case VINT_PACKET_TYPE_DATAADAPTER_PACKET_SYNC:
		dataAdapterSupport->usbInPacketCount = unpack16(&buffer[1]);
		// This clears all packets received before the reset packet was able to make it to the device
		//  - Needed for when the device wasn't properly closed
		dataAdapterSupport->storedPacketLength = 0;

		dataAdapterSupport->rxPacketError = 0;
		dataAdapterSupport->droppedPacketID = NO_ACTIVE_PACKET;
		dataAdapterSupport->rxPacketID = NEW_RX_READY;
		return (EPHIDGET_OK);
	case VINT_PACKET_TYPE_DATAADAPTER_PACKET_DATA_ERROR:
		error = 1; // we intend to fall through to the next here
		errorCode = buffer[5];
		packetOverhead++;
	case VINT_PACKET_TYPE_DATAADAPTER_PACKET_DATA_END:
		endPacket = 1;
	case VINT_PACKET_TYPE_DATAADAPTER_PACKET_DATA:
		// Verify the validity of the packet
		receivedPacketCount = unpack16(&buffer[1]);
		packetInfo = unpack16(&buffer[3]);
		packetID = packetInfo & 0x3FFF;
		newResponsePacket = ((packetInfo & NEW_PACKET_FLAG) != 0);

		if (dataAdapterSupport->usbInPacketCount != receivedPacketCount || (newResponsePacket && (dataAdapterSupport->storedPacketLength != 0))) {
			// Send old data to channel
			if (!newResponsePacket) { // we missed the start of this packet, it is in error
				error = 1;
				errorCode = PACKET_ERROR_CORRUPT;
			}
			dataAdapterSupport->storedPacketLength = 0;
		}

		if (dataAdapterSupport->rxPacketID == NEW_RX_READY) {
			dataAdapterSupport->rxPacketID = packetID;
			dataAdapterSupport->rxPacketError = 0;
		}

		dataAdapterSupport->rxPacketError |= error;

		if (packetID != dataAdapterSupport->rxPacketID && !newResponsePacket) {
			// MOS_PANIC("TODO: Something got out of sequence");
			// Send old data to channel with an error flag
			error = 1;
			errorCode = PACKET_ERROR_CORRUPT;
			dataAdapterSupport->storedPacketLength = 0;
		}

		// handle the packet
		dataAdapterSupport->usbInPacketCount = receivedPacketCount + 1;
		dataAdapterSupport->lastDataLength = length - packetOverhead;
		memcpy(dataAdapterSupport->lastData, buffer + packetOverhead, dataAdapterSupport->lastDataLength);

		// Keep accumulating response data until the full response is received, or the maximum bridge packet length is reached
		if (dataAdapterSupport->storedPacketLength + dataAdapterSupport->lastDataLength <= DATAADAPTER_MAX_PACKET_LENGTH) {
			memcpy(&dataAdapterSupport->storedPacket[dataAdapterSupport->storedPacketLength], dataAdapterSupport->lastData, dataAdapterSupport->lastDataLength);
			dataAdapterSupport->storedPacketLength += (uint16_t)dataAdapterSupport->lastDataLength;
			if ((endPacket == 0))
				return EPHIDGET_OK;
		} else {
			usedPacketLength = (DATAADAPTER_MAX_PACKET_LENGTH - dataAdapterSupport->storedPacketLength);
			memcpy(&dataAdapterSupport->storedPacket[dataAdapterSupport->storedPacketLength], dataAdapterSupport->lastData, usedPacketLength);
			dataAdapterSupport->storedPacketLength = DATAADAPTER_MAX_PACKET_LENGTH;
		}
		// Release response data
		dataAdapterSupport->responseLength = dataAdapterSupport->storedPacketLength;
		memcpy(dataAdapterSupport->responsePacket, dataAdapterSupport->storedPacket, dataAdapterSupport->storedPacketLength);
		dataAdapterSupport->storedPacketLength = 0;
		dataAdapterSupport->responseError = errorCode;

		dataAdapterSupport->responsePacketID = dataAdapterSupport->rxPacketID;
		dataAdapterSupport->rxPacketID = NEW_RX_READY;
		return (EPHIDGET_OK);
	case VINT_PACKET_TYPE_DATAADAPTER_PACKET_DROPPED:
		PhidgetLock(ch);
		dataAdapterSupport->droppedPacketID = unpack16(&buffer[1]);
		dataAdapterSupport->droppedPacketReason = buffer[3];
		PhidgetBroadcast(ch);
		PhidgetUnlock(ch);
		// Could add packet ID here for faster acknowledgement of rejection, but for now the corresponding send will time out.
		// PhidgetChannel_sendErrorEvent(ch, EEPHIDGET_PACKETLOST, "One or more of the transmitted packets were lost.");
		return (EPHIDGET_OK);
	case VINT_PACKET_TYPE_DATAADAPTER_PACKET_ACK:
		PhidgetLock(ch);
		dataAdapterSupport->ackID = (unpack16(&buffer[1])) & 0x3FFF;
		PhidgetBroadcast(ch);
		PhidgetUnlock(ch);
		ClearNAK(ch);
		return (EPHIDGET_OK);
	default:
		MOS_PANIC("Unexpected packet type");
	}
}
#endif

// dataInput - parses device packets
PhidgetReturnCode CCONV
PhidgetDataAdapterSupport_dataInput(PhidgetChannelHandle ch, const uint8_t *buffer, size_t length) {

	assert(ch);
	assert(buffer);

	// Parse device packets - store data locally
	switch (ch->UCD->uid) {
#if (PHIDUID_ADP0001_USB_SUPPORTED || PHIDUID_ADP0002_USB_SUPPORTED)
	#if PHIDUID_ADP0001_USB_SUPPORTED
	case PHIDCHUID_ADP0001_DATAADAPTER_100_USB:
	#endif
	#if PHIDUID_ADP0001_VINT_SUPPORTED
	case PHIDCHUID_ADP0001_DATAADAPTER_100_VINT:
	#endif
	#if PHIDUID_ADP0002_USB_SUPPORTED
	case PHIDCHUID_ADP0002_DATAADAPTER_100_USB:
	#endif
	#if PHIDUID_ADP0002_VINT_SUPPORTED
	case PHIDCHUID_ADP0002_DATAADAPTER_100_VINT:
	#endif
		if (length > 0) {
			switch (buffer[0]) {
			default:
				return processDataPackets(ch, buffer, length);
			}
		}
		MOS_PANIC("Unexpected packet type");
#endif
	default:
		MOS_PANIC("Unexpected device");
	}
}

PhidgetReturnCode CCONV
PhidgetDataAdapterSupport_bridgeInput(PhidgetChannelHandle ch, BridgePacket *bp) {
#if (PHIDUID_ADP0001_USB_SUPPORTED || PHIDUID_ADP0002_USB_SUPPORTED)
	PhidgetDeviceHandle device = (PhidgetDeviceHandle)ch->parent;
	unsigned char buffer[MAX_OUT_PACKET_SIZE] = {0};
	PhidgetReturnCode ret;
	int32_t commFrequency;
	size_t len;
	PhidgetDataAdapterSupportHandle dataAdapterSupport = DATAADAPTER_SUPPORT(ch);
#endif

	switch (ch->UCD->uid) {
#if PHIDUID_ADP0001_USB_SUPPORTED
	case PHIDCHUID_ADP0001_DATAADAPTER_100_USB:
		switch (ch->class) {
		case PHIDCHCLASS_DATAADAPTER:
			switch (bp->vpkt) {
			/*case BP_DATAOUT:
			case BP_DATAEXCHANGE:*/
				
			case BP_I2CDATAEXCHANGE:
				dataAdapterSupport->address = getBridgePacketUInt32(bp, 1);
				ret = parseI2CFormat(ch, getBridgePacketString(bp, 2));
				if (ret != EPHIDGET_OK)
					return ret;
				ret = sendI2CData(ch, bp, 1);
				return ret;
			case BP_SETBAUDRATE:
				len = 4;
				switch (getBridgePacketInt32(bp, 0)) {
				case FREQUENCY_10kHz:
					commFrequency = 10000;
					break;
				case FREQUENCY_100kHz:
					commFrequency = 100000;
					break;
				case FREQUENCY_400kHz:
					commFrequency = 400000;
					break;
				default:
					return EPHIDGET_INVALIDARG;
				}
				pack32(buffer, commFrequency);
				ret = PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_DATAADAPTER_BAUD_RATE, 0, buffer, &len, 100);
				if (ret == EPHIDGET_OK)
					dataAdapterSupport->baudRate = commFrequency;
				return ret;
			case BP_OPENRESET:
				len = 0;
				dataAdapterSupport->enabled = 0;
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_PHIDGET_RESET, 0, buffer, &len, 100);
			case BP_CLOSERESET:
				len = 0;
				dataAdapterSupport->enabled = 0;
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_PHIDGET_RESET, 0, buffer, &len, 100);
			case BP_ENABLE:
				len = 0;
				dataAdapterSupport->enabled = 1;
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_PHIDGET_ENABLE, 0, buffer, &len, 100);
			default:
				MOS_PANIC("Unexpected packet type");
			}
		default:
			MOS_PANIC("Unexpected Channel Class");
		}
#endif /* PHIDUID_ADP0001_USB_SUPPORTED */
#if PHIDUID_ADP0001_VINT_SUPPORTED
	case PHIDCHUID_ADP0001_DATAADAPTER_100_VINT:
		switch (ch->class) {
		case PHIDCHCLASS_DATAADAPTER:
			switch (bp->vpkt) {
			case BP_I2CDATAEXCHANGE:
				dataAdapterSupport->address = getBridgePacketUInt32(bp, 1);
				ret = parseI2CFormat(ch, getBridgePacketString(bp, 2));
				if (ret != EPHIDGET_OK)
					return ret;
				ret = sendI2CData(ch, bp, 1);
				return ret;
			case BP_SETBAUDRATE:
				len = 4;
				switch (getBridgePacketInt32(bp, 0)) {
				case FREQUENCY_10kHz:
					commFrequency = 10000;
					break;
				case FREQUENCY_100kHz:
					commFrequency = 100000;
					break;
				case FREQUENCY_400kHz:
					commFrequency = 400000;
					break;
				default:
					return EPHIDGET_INVALIDARG;
				}
				pack32(buffer, commFrequency);
				ret = sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_DATAADAPTER_BAUD_RATE, buffer, len);
				if (ret == EPHIDGET_OK)
					dataAdapterSupport->baudRate = commFrequency;
				return ret;
			case BP_OPENRESET:
				len = 0;
				dataAdapterSupport->enabled = 0;
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_PHIDGET_RESET, buffer, len);
			case BP_CLOSERESET:
				len = 0;
				dataAdapterSupport->enabled = 0;
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_PHIDGET_RESET, buffer, len);
			case BP_ENABLE:
				len = 0;
				dataAdapterSupport->enabled = 1;
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_PHIDGET_ENABLE, buffer, len);
			default:
				MOS_PANIC("Unexpected packet type");
			}
		default:
			MOS_PANIC("Unexpected Channel Class");
		}
#endif /* PHIDUID_ADP0001_VINT_SUPPORTED */
#if PHIDUID_ADP0002_USB_SUPPORTED
	case PHIDCHUID_ADP0002_DATAADAPTER_100_USB:
		switch (ch->class) {
		case PHIDCHCLASS_DATAADAPTER:
			switch (bp->vpkt) {
			case BP_DATAOUT:
				ret = sendData(ch, bp, 0);
				return ret;
			case BP_DATAEXCHANGE:
				ret = sendData(ch, bp, 1);
				return ret;
			case BP_SETBAUDRATE:
				len = 4;
				switch (getBridgePacketInt32(bp, 0)) {
				case FREQUENCY_188kHz:
					commFrequency = 187500;
					break;
				case FREQUENCY_375kHz:
					commFrequency = 375000;
					break;
				case FREQUENCY_750kHz:
					commFrequency = 750000;
					break;
				case FREQUENCY_1500kHz:
					commFrequency = 1500000;
					break;
				case FREQUENCY_3MHz:
					commFrequency = 3000000;
					break;
				case FREQUENCY_6MHz:
					commFrequency = 6000000;
					break;
				default:
					return EPHIDGET_INVALIDARG;
				}
				pack32(buffer, commFrequency);
				ret = PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_DATAADAPTER_BAUD_RATE, 0, buffer, &len, 100);
				if (ret == EPHIDGET_OK)
					dataAdapterSupport->baudRate = commFrequency;
				return ret;
			case BP_OPENRESET:
				len = 0;
				dataAdapterSupport->enabled = 0;
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_PHIDGET_RESET, 0, buffer, &len, 100);
			case BP_CLOSERESET:
				len = 0;
				dataAdapterSupport->enabled = 0;
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_PHIDGET_RESET, 0, buffer, &len, 100);
			case BP_ENABLE:
				len = 0;
				dataAdapterSupport->enabled = 1;
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_PHIDGET_ENABLE, 0, buffer, &len, 100);
			case BP_SETDATABITS:
				len = 1;
				buffer[0] = (uint8_t)getBridgePacketUInt32(bp, 0);
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_DATAADAPTER_DATA_BITS, 0, buffer, &len, 100);
			case BP_SETSPIMODE:
				len = 1;
				buffer[0] = (uint8_t)getBridgePacketInt32(bp, 0);
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_DATAADAPTER_SPIMODE, 0, buffer, &len, 100);
			case BP_SETSPICHIPSELECT:
				len = 1;
				buffer[0] = (uint8_t)getBridgePacketInt32(bp, 0);
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_DATAADAPTER_SPICHIPSELECT, 0, buffer, &len, 100);
			case BP_SETIGNORERESPONSE:
				len = 1;
				buffer[0] = (uint8_t)getBridgePacketInt32(bp, 0);
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_DATAADAPTER_IGNORERESPONSE, 0, buffer, &len, 100);
			case BP_SETENDIANNESS:
				len = 1;
				buffer[0] = (uint8_t)getBridgePacketInt32(bp, 0);
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_DATAADAPTER_ENDIANNESS, 0, buffer, &len, 100);
			default:
				MOS_PANIC("Unexpected packet type");
			}
		default:
			MOS_PANIC("Unexpected Channel Class");
		}
#endif /* PHIDUID_ADP0002_USB_SUPPORTED */
#if PHIDUID_ADP0002_VINT_SUPPORTED
	case PHIDCHUID_ADP0002_DATAADAPTER_100_VINT:
		switch (ch->class) {
		case PHIDCHCLASS_DATAADAPTER:
			switch (bp->vpkt) {
			case BP_DATAOUT:
				ret = sendData(ch, bp, 0);
				return ret;
			case BP_DATAEXCHANGE:
				ret = sendData(ch, bp, 1);
				return ret;
			case BP_SETBAUDRATE:
				len = 4;
				switch (getBridgePacketInt32(bp, 0)) {
				case FREQUENCY_188kHz:
					commFrequency = 187500;
					break;
				case FREQUENCY_375kHz:
					commFrequency = 375000;
					break;
				case FREQUENCY_750kHz:
					commFrequency = 750000;
					break;
				case FREQUENCY_1500kHz:
					commFrequency = 1500000;
					break;
				case FREQUENCY_3MHz:
					commFrequency = 3000000;
					break;
				case FREQUENCY_6MHz:
					commFrequency = 6000000;
					break;
				default:
					return EPHIDGET_INVALIDARG;
				}
				pack32(buffer, commFrequency);
				ret = sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_DATAADAPTER_BAUD_RATE, buffer, len);
				if (ret == EPHIDGET_OK)
					dataAdapterSupport->baudRate = commFrequency;
				return ret;
			case BP_OPENRESET:
				len = 0;
				dataAdapterSupport->enabled = 0;
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_PHIDGET_RESET, buffer, len);
			case BP_CLOSERESET:
				len = 0;
				dataAdapterSupport->enabled = 0;
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_PHIDGET_RESET, buffer, len);
			case BP_ENABLE:
				len = 0;
				dataAdapterSupport->enabled = 1;
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_PHIDGET_ENABLE, buffer, len);
			case BP_SETDATABITS:
				len = 1;
				buffer[0] = (uint8_t)getBridgePacketUInt32(bp, 0);
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_DATAADAPTER_DATA_BITS, buffer, len);
			case BP_SETSPIMODE:
				len = 1;
				buffer[0] = (uint8_t)getBridgePacketInt32(bp, 0);
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_DATAADAPTER_SPIMODE, buffer, len);
			case BP_SETSPICHIPSELECT:
				len = 1;
				buffer[0] = (uint8_t)getBridgePacketInt32(bp, 0);
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_DATAADAPTER_SPICHIPSELECT, buffer, len);
			case BP_SETIGNORERESPONSE:
				len = 1;
				buffer[0] = (uint8_t)getBridgePacketInt32(bp, 0);
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_DATAADAPTER_IGNORERESPONSE, buffer, len);
			case BP_SETENDIANNESS:
				len = 1;
				buffer[0] = (uint8_t)getBridgePacketInt32(bp, 0);
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_DATAADAPTER_ENDIANNESS, buffer, len);
			default:
				MOS_PANIC("Unexpected packet type");
			}
		default:
			MOS_PANIC("Unexpected Channel Class");
		}
#endif /* PHIDUID_ADP0002_VINT_SUPPORTED */
	default:
		MOS_PANIC("Unexpected device");
	}
}

PhidgetReturnCode sendData(PhidgetChannelHandle ch, BridgePacket *bp, int waitResponse) {

	return sendDataBuffer(ch, bp->entry[0].len, (const uint8_t *)getBridgePacketUInt8Array(bp, 0), bp, waitResponse);
}

PhidgetReturnCode parseI2CFormat(PhidgetChannelHandle ch, const char *string) {
	int index = 0;
	int count = 0;
	int stopped = 0;
	int mode = 0; // 0 transmit, 1 receive
	int totalFormatCount = 0;
	int transmitCount = 0;
	int numberTracker = 0;
	size_t i;

	uint8_t tmpFormatList[128];

	for (i = 0; i < mos_strlen(string); i++) {
		if (stopped)
			return EPHIDGET_INVALIDARG;
		switch (string[i]) {
		case 's':
			if (i == 0)
				continue;
			if (count != 0) {
				if (numberTracker != 0) {
					if (numberTracker > 127 || numberTracker < 1)
						return EPHIDGET_INVALIDARG;
					count--;
					count += numberTracker;
					totalFormatCount--;
					totalFormatCount += numberTracker;
					if (mode == 0) {
						transmitCount--;
						transmitCount += numberTracker;
					}
				}
				tmpFormatList[index] = count;
				if (mode)
					tmpFormatList[index] |= 0x80;

				index++;
				count = 0;
				numberTracker = 0;
			} else
				return EPHIDGET_INVALIDARG;
			break;
		case 'T':
			if (i == 0 || numberTracker != 0)
				return EPHIDGET_INVALIDARG;
			if (count == 0) {
				mode = 0;
			}
			if (mode != 0) {
				return EPHIDGET_INVALIDARG;
			}
			count++;
			totalFormatCount++;
			transmitCount++;
			if (count > 127 || totalFormatCount > 256)
				return EPHIDGET_INVALIDARG;
			break;
		case 'R':
			if (i == 0 || numberTracker != 0)
				return EPHIDGET_INVALIDARG;
			if (count == 0) {
				mode = 1;
			}
			if (mode != 1) {
				return EPHIDGET_INVALIDARG;
			}
			count++;
			totalFormatCount++;
			if (count > 127 || totalFormatCount > 256)
				return EPHIDGET_INVALIDARG;
			break;
		case 'p':
			if (i == 0)
				return EPHIDGET_INVALIDARG;
			if (count != 0) {
				if (numberTracker != 0) {
					count--;
					count += numberTracker;
					totalFormatCount--;
					totalFormatCount += numberTracker;
					if (mode == 0) {
						transmitCount--;
						transmitCount += numberTracker;
					}
				}
				if (totalFormatCount > 256)
					return EPHIDGET_INVALIDARG;
				tmpFormatList[index] = count;
				if (mode)
					tmpFormatList[index] |= 0x80;

			} else
				return EPHIDGET_INVALIDARG;
			stopped = 1;
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			if (count != 1)
				return EPHIDGET_INVALIDARG;
			numberTracker *= 10;
			numberTracker += string[i] - '0';
			break;
		default:
			return EPHIDGET_INVALIDARG;
		}
	}
	if (!stopped)
		return EPHIDGET_INVALIDARG;

	if ((index + 1 + transmitCount + 2) > 512) // Total Transmitted bytes given the proposed format
		return EPHIDGET_INVALIDARG;

	DATAADAPTER_SUPPORT(ch)->i2cFormatCount = index + 1;
	memcpy(DATAADAPTER_SUPPORT(ch)->i2cFormatList, tmpFormatList, DATAADAPTER_SUPPORT(ch)->i2cFormatCount);

	return EPHIDGET_OK;
}

PhidgetReturnCode sendI2CData(PhidgetChannelHandle ch, BridgePacket *bp, int waitResponse) {
	uint8_t buffer[1024];
	int transmitCount = 0;
	int i;
	uint8_t dataSize = bp->entry[0].len;

	if (DATAADAPTER_SUPPORT(ch)->i2cFormatCount == 0)
		return (MOS_ERROR(bp->iop, EPHIDGET_INVALIDARG, "I2C Format must be set brfore data can be exchanged"));

	uint8_t totalCount = dataSize + DATAADAPTER_SUPPORT(ch)->i2cFormatCount + 2;

	if (totalCount > 512)
		return (MOS_ERROR(bp->iop, EPHIDGET_INVALIDARG, "Packet is too large"));

	for (i = 0; i < DATAADAPTER_SUPPORT(ch)->i2cFormatCount; i++) {
		if (!(DATAADAPTER_SUPPORT(ch)->i2cFormatList[i] & 0x80)) // if transmit segment
			transmitCount += DATAADAPTER_SUPPORT(ch)->i2cFormatList[i] & 0x7F;
	}

	if (transmitCount != dataSize)
		return (MOS_ERROR(bp->iop, EPHIDGET_INVALIDARG, "Transmit array length does not match the length specified in I2CFormat"));

	buffer[0] = DATAADAPTER_SUPPORT(ch)->address;
	buffer[1] = DATAADAPTER_SUPPORT(ch)->i2cFormatCount;
	memcpy(&buffer[2], DATAADAPTER_SUPPORT(ch)->i2cFormatList, DATAADAPTER_SUPPORT(ch)->i2cFormatCount);
	memcpy(&buffer[DATAADAPTER_SUPPORT(ch)->i2cFormatCount + 2], (const uint8_t *)getBridgePacketUInt8Array(bp, 0), dataSize);

	return sendDataBuffer(ch, totalCount, (const uint8_t *)buffer, bp, waitResponse);
}

static PhidgetReturnCode sendTXDataVINT(mosiop_t iop, PhidgetChannelHandle ch, uint8_t *buf, size_t packetLen, PhidgetTransaction *trans) {
	PhidgetReturnCode ret;
	SetNAK(ch);
	ret = sendVINTDataPacketTransaction(iop, ch, VINT_PACKET_TYPE_DATAADAPTER_TX_DATA, buf, packetLen, trans);

	if (ret != EPHIDGET_OK)
		return ret;

	return EPHIDGET_OK;
}

PhidgetReturnCode sendDataBuffer(PhidgetChannelHandle ch, size_t len, const uint8_t *buffer, BridgePacket *bp, int waitResponse) {
	PhidgetReturnCode ret;
	size_t packetLen;
	uint32_t packetCount = 0;
	PhidgetTransaction trans;
	uint16_t packetID;
	uint16_t packetInfo;
	PhidgetReturnCode res1;
	uint32_t expectedDurationUS;
	size_t i = 0;

	uint8_t buf[USB_OUT_PACKET_LENGTH];

	mostime_t duration;
	mostime_t start;
	mostime_t maxDuration;
	mostime_t remainingTime;

	PhidgetDataAdapterSupportHandle dataAdapterSupport = DATAADAPTER_SUPPORT(ch);

	if ((dataAdapterSupport->enabled == 0) && (ch->parent->deviceInfo.class != PHIDCLASS_VINT))
		return (MOS_ERROR(bp->iop, EPHIDGET_NOTCONFIGURED, "Data cannot be exchanged from within the attach handler"));

	if (dataAdapterSupport->txTimeout == 0)
		maxDuration = ((double)len / DATAADAPTER_SUPPORT(ch)->baudRate) * 2000000 * 10 + 100000; // 2 * transmission time in us + 100ms
	else
		maxDuration = (dataAdapterSupport->txTimeout + 50) * 1000; // we want the FW to indicate a timeout before the sendData call times out

#if PHIDUID_ADP0001_USB_SUPPORTED || PHIDUID_ADP0001_VINT_SUPPORTED
	switch (ch->UCD->uid) {
	case PHIDUID_ADP0001_USB:
	case PHIDUID_ADP0001_VINT:
		if (dataAdapterSupport->address == PUNK_INT32)
			return (MOS_ERROR(bp->iop, EPHIDGET_NOTCONFIGURED, "I2C Requires a Device Address to be set before sending data"));
		break;
	}
#endif

	if (DATAADAPTER_SUPPORT(ch)->baudRate == 0)
		return (MOS_ERROR(bp->iop, EPHIDGET_NOTCONFIGURED, "Baud Rate Not Configured"));

	start = mos_gettime_usec();

	// PhidgetRunLock(ch);
	// Assign unique packet ID
	PhidgetLock(ch);
	DATAADAPTER_SUPPORT(ch)->packetID++;
	DATAADAPTER_SUPPORT(ch)->packetID &= 0x3FFF; // 14-bit packet ID
	if (DATAADAPTER_SUPPORT(ch)->packetID == ANONYMOUS_PACKET_ID)
		DATAADAPTER_SUPPORT(ch)->packetID = 0x0001;
	packetID = DATAADAPTER_SUPPORT(ch)->packetID;
	PhidgetUnlock(ch);
	// PhidgetRunUnlock(ch);

	// Wait until previous packets are dealt with, ensures the device will accept the packet

	ret = EPHIDGET_OK;

	// Transfer the packet
	if (ch->parent->deviceInfo.class != PHIDCLASS_VINT) {
		if (len > 0) {
			if (len <= (USB_OUT_PACKET_LENGTH - USB_OUT_PACKET_OVERHEAD)) {
				packetLen = len + USB_OUT_PACKET_OVERHEAD;
				packetInfo = NEW_PACKET_FLAG | packetID;
				if (waitResponse)
					packetInfo |= WAIT_RESP_FLAG;
				pack16(&buf[0], packetInfo);
				buf[2] = (uint8_t)(len >> 16); // pack 24
				buf[3] = (uint8_t)(len >> 8);
				buf[4] = (uint8_t)(len & 0xFF);
				memcpy(&buf[USB_OUT_PACKET_OVERHEAD], buffer, len);
				remainingTime = maxDuration - (mos_gettime_usec() - start);
				ret = PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)ch->parent, PHIDGETUSB_REQ_BULK_WRITE, 0, 0, buf, &packetLen, remainingTime / 1000);
				if (ret != EPHIDGET_OK) {
					return ret;
				}

			} else {
				for (i = 0; i + (USB_OUT_PACKET_LENGTH - USB_OUT_PACKET_OVERHEAD) < len; i += (USB_OUT_PACKET_LENGTH - USB_OUT_PACKET_OVERHEAD)) {
					packetLen = USB_OUT_PACKET_LENGTH;
					if (i == 0) {
						packetInfo = NEW_PACKET_FLAG | packetID;
						if (waitResponse)
							packetInfo |= WAIT_RESP_FLAG;
						pack16(&buf[0], packetInfo);
						buf[2] = (uint8_t)(len >> 16); // pack 24
						buf[3] = (uint8_t)(len >> 8);
						buf[4] = (uint8_t)(len & 0xFF);
					} else {
						pack16(&buf[0], packetID);
						buf[2] = (uint8_t)(packetCount >> 16); // pack 24
						buf[3] = (uint8_t)(packetCount >> 8);
						buf[4] = (uint8_t)(packetCount & 0xFF);
					}

					if (DATAADAPTER_SUPPORT(ch)->droppedPacketID == packetID) {
						DATAADAPTER_SUPPORT(ch)->droppedPacketID = NO_ACTIVE_PACKET;
						return EPHIDGET_INTERRUPTED;
					}
					memcpy(&buf[USB_OUT_PACKET_OVERHEAD], buffer + i, (packetLen - USB_OUT_PACKET_OVERHEAD));
					remainingTime = maxDuration - (mos_gettime_usec() - start);
					ret = PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)ch->parent, PHIDGETUSB_REQ_BULK_WRITE, 0, 0, buf, &packetLen, remainingTime / 1000);
					if (ret != EPHIDGET_OK) {
						return ret;
					}
					packetCount++;
				}
				if (i != len) {
					packetLen = len - i + USB_OUT_PACKET_OVERHEAD;
					pack16(&buf[0], packetID);
					buf[2] = (uint8_t)(packetCount >> 16); // pack 24
					buf[3] = (uint8_t)(packetCount >> 8);
					buf[4] = (uint8_t)(packetCount & 0xFF);

					if (DATAADAPTER_SUPPORT(ch)->droppedPacketID == packetID) {
						DATAADAPTER_SUPPORT(ch)->droppedPacketID = NO_ACTIVE_PACKET;
						return EPHIDGET_INTERRUPTED;
					}
					memcpy(&buf[USB_OUT_PACKET_OVERHEAD], buffer + i, (packetLen - USB_OUT_PACKET_OVERHEAD));
					remainingTime = maxDuration - (mos_gettime_usec() - start);
					ret = PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)ch->parent, PHIDGETUSB_REQ_BULK_WRITE, 0, 0, buf, &packetLen, remainingTime / 1000);
					if (ret != EPHIDGET_OK) {
						return ret;
					}
				}
			}
		}
	} else {
		ret = PhidgetChannel_beginTransaction(ch, &trans);
		if (ret != EPHIDGET_OK)
			goto done;

		if (len > 0) {
			if (len <= (VINT_MAX_OUT_PACKETSIZE - USB_OUT_PACKET_OVERHEAD)) {
				packetLen = len + USB_OUT_PACKET_OVERHEAD;
				packetInfo = NEW_PACKET_FLAG | packetID;
				if (waitResponse)
					packetInfo |= WAIT_RESP_FLAG;
				pack16(&buf[0], packetInfo);
				buf[2] = (uint8_t)(len >> 16); // pack 24
				buf[3] = (uint8_t)(len >> 8);
				buf[4] = (uint8_t)(len & 0xFF);
				memcpy(&buf[USB_OUT_PACKET_OVERHEAD], buffer, len);
				// remainingTime = maxDuration - (mos_gettime_usec() - start);
				ret = sendTXDataVINT(bp->iop, ch, buf, packetLen, &trans);
				if (ret != EPHIDGET_OK) {
					goto done;
				}

			} else {
				for (i = 0; i + (VINT_MAX_OUT_PACKETSIZE - USB_OUT_PACKET_OVERHEAD) < len; i += (VINT_MAX_OUT_PACKETSIZE - USB_OUT_PACKET_OVERHEAD)) {
					packetLen = VINT_MAX_OUT_PACKETSIZE;
					if (i == 0) {
						packetInfo = NEW_PACKET_FLAG | packetID;
						if (waitResponse)
							packetInfo |= WAIT_RESP_FLAG;
						pack16(&buf[0], packetInfo);
						buf[2] = (uint8_t)(len >> 16); // pack 24
						buf[3] = (uint8_t)(len >> 8);
						buf[4] = (uint8_t)(len & 0xFF);
					} else {
						pack16(&buf[0], packetID);
						buf[2] = (uint8_t)(packetCount >> 16); // pack 24
						buf[3] = (uint8_t)(packetCount >> 8);
						buf[4] = (uint8_t)(packetCount & 0xFF);
					}

					if (DATAADAPTER_SUPPORT(ch)->droppedPacketID == packetID) {
						DATAADAPTER_SUPPORT(ch)->droppedPacketID = NO_ACTIVE_PACKET;
						ret = EPHIDGET_INTERRUPTED;
						goto done;
					}
					memcpy(&buf[USB_OUT_PACKET_OVERHEAD], buffer + i, (packetLen - USB_OUT_PACKET_OVERHEAD));
					// remainingTime = maxDuration - (mos_gettime_usec() - start);
					ret = sendTXDataVINT(bp->iop, ch, buf, packetLen, &trans);
					if (ret != EPHIDGET_OK) {
						goto done;
					}
					packetCount++;
				}
				if (i != len) {
					packetLen = len - i + USB_OUT_PACKET_OVERHEAD;
					pack16(&buf[0], packetID);
					buf[2] = (uint8_t)(packetCount >> 16); // pack 24
					buf[3] = (uint8_t)(packetCount >> 8);
					buf[4] = (uint8_t)(packetCount & 0xFF);

					if (DATAADAPTER_SUPPORT(ch)->droppedPacketID == packetID) {
						DATAADAPTER_SUPPORT(ch)->droppedPacketID = NO_ACTIVE_PACKET;
						ret = EPHIDGET_INTERRUPTED;
						goto done;
					}
					memcpy(&buf[USB_OUT_PACKET_OVERHEAD], buffer + i, (packetLen - USB_OUT_PACKET_OVERHEAD));
					// remainingTime = maxDuration - (mos_gettime_usec() - start);
					ret = sendTXDataVINT(bp->iop, ch, buf, packetLen, &trans);
					if (ret != EPHIDGET_OK) {
						goto done;
					}
				}
			}
		}

	done:
		res1 = PhidgetChannel_endTransaction(ch, &trans);
		if (res1 != EPHIDGET_OK) {
			if(res1 == EPHIDGET_NOTCONFIGURED)
				return (MOS_ERROR(bp->iop, EPHIDGET_NOTCONFIGURED, "Data cannot be exchanged from within the attach handler"));
			return (res1);
		}
		//	if(ret)
		//		return (ret);
	}

	////Wait to see if the device accepted the packet, this makes the return code from sendPacket mean something
	// start = mos_gettime_usec();

	for (;;) {
		// Wait for acknowledgement that the packet finished transmitting, even if we're not reporting response data.
		// This helps keep the user's program in sync
		if (DATAADAPTER_SUPPORT(ch)->responsePacketID == packetID) {
			if (waitResponse != 0) {
				// Bridge packet reply is entire response
				bp->reply_bpe = mos_malloc(sizeof(BridgePacketEntry));
				memset(bp->reply_bpe, 0, sizeof(BridgePacketEntry));

				bp->reply_bpe->type = BPE_UI8ARRAY;
				bp->reply_bpe->bpe_len = (uint16_t)(dataAdapterSupport->responseLength + 4);
				bp->reply_bpe->bpe_ptr = mos_malloc(dataAdapterSupport->responseLength + 4);
				bp->reply_bpe->bpe_ui8array = bp->reply_bpe->bpe_ptr;
				bp->reply_bpe->bpe_cnt = (uint16_t)1;
				memcpy(bp->reply_bpe->bpe_ui8array, dataAdapterSupport->responsePacket, dataAdapterSupport->responseLength);
				pack32(&(bp->reply_bpe->bpe_ui8array[dataAdapterSupport->responseLength]), dataAdapterSupport->responseError);
				dataAdapterSupport->responseLength = 0;
				dataAdapterSupport->responseError = PACKET_ERROR_OK;
			}
			return (EPHIDGET_OK);
		}

		if (DATAADAPTER_SUPPORT(ch)->droppedPacketID == packetID) {
			DATAADAPTER_SUPPORT(ch)->droppedPacketID = NO_ACTIVE_PACKET;
			if (DATAADAPTER_SUPPORT(ch)->droppedPacketReason == TX_DROPPED_TIMEOUT) {
				return (MOS_ERROR(bp->iop, EPHIDGET_TIMEOUT, "The packet timed out while waiting to be transmitted. Check that your system can keep up with the data being sent."));
			}
			if (DATAADAPTER_SUPPORT(ch)->droppedPacketReason == TX_DROPPED_TOOBIG) {
				return (MOS_ERROR(bp->iop, EPHIDGET_INVALIDARG, "The packet is too large to be accepted by the device"));
			}
			return EPHIDGET_INTERRUPTED;
		}

		if (!(ISATTACHED(ch))) {
			return (EPHIDGET_CLOSED);
		}

		duration = (mos_gettime_usec() - start);
		if (duration >= maxDuration) {
			//	phid->activePacket = NO_ACTIVE_PACKET; // transmission failed, the packet is no longer active
			return (EPHIDGET_TIMEOUT);
		}
		PhidgetLock(ch);
		if (DATAADAPTER_SUPPORT(ch)->baudRate != 0) {
			expectedDurationUS = ((len * 8) * 1000000) / DATAADAPTER_SUPPORT(ch)->baudRate;
			if (expectedDurationUS > ((uint32_t)duration)) {
				if (waitResponse || (expectedDurationUS - ((uint32_t)duration)) > 20000) {
					PhidgetTimedWait(ch, (expectedDurationUS - ((uint32_t)duration)) / 1000);
				} else {
					PhidgetTimedWait(ch, 1);
				}
			}
		} else {
			PhidgetUnlock(ch);
			return (MOS_ERROR(bp->iop, EPHIDGET_NOTCONFIGURED, "Baud Rate Not Configured"));
		}
		PhidgetUnlock(ch);
	}
}

/*
 * Public API
 */

void PhidgetDataAdapterSupport_free(PhidgetDataAdapterSupportHandle *arg) {

	if (arg == NULL || *arg == NULL)
		return;

	assert(arg);
	mos_mutex_destroy(&((*arg)->sendLock));
	mos_mutex_destroy(&((*arg)->receiveLock));

	mos_free(*arg, sizeof(PhidgetDataAdapterSupport));
	*arg = NULL;
}

PhidgetReturnCode
PhidgetDataAdapterSupport_create(PhidgetDataAdapterSupportHandle *arg) {

	TESTPTR_PR(arg);
	*arg = mos_zalloc(sizeof(PhidgetDataAdapterSupport));

	assert(arg);
	mos_mutex_init(&((*arg)->sendLock));
	mos_mutex_init(&((*arg)->receiveLock));

	return (EPHIDGET_OK);
}

void PhidgetDataAdapterSupport_init(PhidgetDataAdapterSupportHandle dataAdapter) {

	assert(dataAdapter);

	dataAdapter->usbInPacketCount = 0;
	dataAdapter->packetID = 0;
	dataAdapter->rxPacketID = NEW_RX_READY;
	dataAdapter->responsePacketID = NO_ACTIVE_PACKET;
	dataAdapter->baudRate = 9600;

	dataAdapter->lastDataLength = 0;

	dataAdapter->ackID = NO_ACTIVE_PACKET;

	dataAdapter->nakFlag = 0;

	dataAdapter->rxPacketError = 0;

	dataAdapter->droppedPacketID = NO_ACTIVE_PACKET;

	dataAdapter->storedPacketLength = 0;
	dataAdapter->i2cFormatCount = 0;
	memset(dataAdapter->i2cFormatList, 0, sizeof(dataAdapter->i2cFormatList));
	dataAdapter->address = PUNK_INT32;

	dataAdapter->enabled = 0;

	dataAdapter->txTimeout = 0;
}
