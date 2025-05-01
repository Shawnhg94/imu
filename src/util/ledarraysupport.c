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
#include "util/ledarraysupport.h"

// Access the PhidgetIRSupport struct via the channel private pointer
#define LEDARRAY_SUPPORT(ch) ((PhidgetLEDArraySupportHandle)(((PhidgetChannelHandle)(ch))->private))

// === Internal Functions === //
static PhidgetReturnCode
processDataPackets(PhidgetChannelHandle ch, const uint8_t *buffer, size_t length) {
	PhidgetLEDArraySupportHandle ledArraySupport = LEDARRAY_SUPPORT(ch);
	//	uint16_t packetInfo;
	//	uint16_t packetID;

	switch (buffer[0]) {
	case VINT_PACKET_TYPE_LEDARRAY_PACKET_DATA_END:
		return (EPHIDGET_OK);
	case VINT_PACKET_TYPE_LEDARRAY_PACKET_DROPPED:
		PhidgetLock(ch);
		ledArraySupport->droppedPacketID = unpack16(&buffer[1]);
		ledArraySupport->droppedPacketReason = buffer[3];
		PhidgetBroadcast(ch);
		PhidgetUnlock(ch);
		// Could add packet ID here for faster acknowledgement of rejection, but for now the corresponding send will time out.
		// PhidgetChannel_sendErrorEvent(ch, EEPHIDGET_PACKETLOST, "One or more of the transmitted packets were lost.");
		return (EPHIDGET_OK);
	case VINT_PACKET_TYPE_LEDARRAY_PACKET_ACK:
		PhidgetLock(ch);
		ledArraySupport->ackID = (unpack16(&buffer[1])) & MAX_PACKET_ID;
		PhidgetBroadcast(ch);
		PhidgetUnlock(ch);
		return (EPHIDGET_OK);
	case VINT_PACKET_TYPE_LEDARRAY_USB_FAULT:
		switch (ch->UCD->uid) {
#if PHIDUID_LED0100_VINT_SUPPORTED
		case PHIDCHUID_LED0100_LEDARRAY_100_VINT:
			PhidgetChannel_sendErrorEvent(ch, EEPHIDGET_OVERCURRENT, "This device requires an external supply when connected through VINT. Connect an external supply and cycle powerEnabled to try again.");
			return (EPHIDGET_OK);
#endif
		default:
			PhidgetChannel_sendErrorEvent(ch, EEPHIDGET_OVERCURRENT, "USB Overcurrent, disabling USB power to the LEDs. Consider running at a lower brightness or using an external power supply. Cycle powerEnabled to resume power.");
			return (EPHIDGET_OK);
		}
	case VINT_PACKET_TYPE_LEDARRAY_OVERCURRENT:
		PhidgetChannel_sendErrorEvent(ch, EEPHIDGET_OVERCURRENT, "External supply overcurrent, disabling power to the LEDs. Consider running at a lower brightness or powering LEDs offboard. Cycle powerEnabled to resume power.");
		return (EPHIDGET_OK);
	default:
		MOS_PANIC("Unexpected packet type");
	}
}

// dataInput - parses device packets
PhidgetReturnCode CCONV
PhidgetLEDArraySupport_dataInput(PhidgetChannelHandle ch, const uint8_t *buffer, size_t length) {

	assert(ch);
	assert(buffer);

	// Parse device packets - store data locally
	switch (ch->UCD->uid) {
#if (PHIDUID_LED0100_USB_SUPPORTED || PHIDUID_LED0100_VINT_SUPPORTED)
	#if PHIDUID_LED0100_USB_SUPPORTED
	case PHIDCHUID_LED0100_LEDARRAY_100_USB:
	#endif
	#if PHIDUID_LED0100_VINT_SUPPORTED
	case PHIDCHUID_LED0100_LEDARRAY_100_VINT:
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
PhidgetLEDArraySupport_bridgeInput(PhidgetChannelHandle ch, BridgePacket *bp) {
#if (PHIDUID_LED0100_USB_SUPPORTED || PHIDUID_LED0100_VINT_SUPPORTED)
	PhidgetLEDArraySupportHandle ledArraySupport = LEDARRAY_SUPPORT(ch);
	PhidgetDeviceHandle device = (PhidgetDeviceHandle)ch->parent;
	unsigned char buffer[MAX_OUT_PACKET_SIZE] = {0};
	PhidgetReturnCode ret;
	size_t len;
#endif

	switch (ch->UCD->uid) {
#if (PHIDUID_LED0100_USB_SUPPORTED || PHIDUID_LED0100_VINT_SUPPORTED)
	#if PHIDUID_LED0100_USB_SUPPORTED
	case PHIDCHUID_LED0100_LEDARRAY_100_USB:
	#endif
		switch (ch->class) {
		case PHIDCHCLASS_LEDARRAY:
			switch (bp->vpkt) {
			case BP_DATAOUT:
				ret = ledArray_sendData(ch, bp);
				return ret;
			case BP_SETANIMATION:
				ret = ledArray_sendAnimation(ch, bp);
				if (ret == EPHIDGET_OK)
					ledArraySupport->protocolLocked = 1;
				return ret;
			case BP_STOPANIMATION:
				len = 1;
				buffer[0] = getBridgePacketInt32(bp, 0);
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_LEDARRAY_STOPANIMATION, 0, buffer, &len, 100);
			case BP_SYNCHRONIZE:
				len = 0;
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_LEDARRAY_SYNCHRONIZE, 0, buffer, &len, 100);
			case BP_SETENABLED:
				len = 1;
				buffer[0] =(getBridgePacketInt32(bp, 0) != 0);
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_LEDARRAY_ENABLED, 0, buffer, &len, 100);
			case BP_SETPROTOCOL:
				if(ledArraySupport->protocolLocked && (ledArraySupport->ledProtocol != getBridgePacketInt32(bp, 0)))
					return (MOS_ERROR(bp->iop, EPHIDGET_NOTCONFIGURED, "LEDArray Protocol cannot be changed after an animation has been set"));
				ledArraySupport->ledProtocol = getBridgePacketInt32(bp, 0);
				return EPHIDGET_OK;
			case BP_SETGAMMA:
				len = 4;
				packfloat(&buffer[0], (float)getBridgePacketDouble(bp, 0));
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_LEDARRAY_SETGAMMA, 0, buffer, &len, 100);
			case BP_SETBRIGHTNESS:
				len = 4;
				packfloat(&buffer[0], (float)getBridgePacketDouble(bp, 0));
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_LEDARRAY_SETBRIGHTNESS, 0, buffer, &len, 100);
			case BP_CLEAR:
				len = 0;
				ledArraySupport->protocolLocked = 0;
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_LEDARRAY_CLEAR, 0, buffer, &len, 100);
			case BP_OPENRESET:
				len = 0;
				ledArraySupport->protocolLocked = 0;
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_PHIDGET_RESET, 0, buffer, &len, 100);
			case BP_CLOSERESET:
				len = 0;
				ledArraySupport->protocolLocked = 0;
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_PHIDGET_RESET, 0, buffer, &len, 100);
			case BP_ENABLE:
				len = 0;
				return PhidgetDevice_transferpacket(bp->iop, device, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_PHIDGET_ENABLE, 0, buffer, &len, 100);
			default:
				MOS_PANIC("Unexpected packet type");
			}
		default:
			MOS_PANIC("Unexpected Channel Class");
		}
	#if PHIDUID_LED0100_VINT_SUPPORTED
	case PHIDCHUID_LED0100_LEDARRAY_100_VINT:
	#endif
		switch (ch->class) {
		case PHIDCHCLASS_LEDARRAY:
			switch (bp->vpkt) {
			case BP_DATAOUT:
				ret = ledArray_sendData(ch, bp);
				return ret;
			case BP_SETANIMATION:
				ret = ledArray_sendAnimation(ch, bp);
				if (ret == EPHIDGET_OK)
					ledArraySupport->protocolLocked = 1;
				return ret;
			case BP_STOPANIMATION:
				len = 1;
				buffer[0] = getBridgePacketInt32(bp, 0);
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_LEDARRAY_STOPANIMATION, buffer, len);
			case BP_SYNCHRONIZE:
				len = 0;
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_LEDARRAY_SYNCHRONIZE, buffer, len);
			case BP_SETENABLED:
				len = 1;
				buffer[0] = (getBridgePacketInt32(bp, 0) != 0);
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_LEDARRAY_ENABLED, buffer, len);
			case BP_SETPROTOCOL:
				if (ledArraySupport->protocolLocked && (ledArraySupport->ledProtocol != getBridgePacketInt32(bp, 0)))
					return (MOS_ERROR(bp->iop, EPHIDGET_NOTCONFIGURED, "LEDArray Protocol cannot be changed after an animation has been set"));
				ledArraySupport->ledProtocol = getBridgePacketInt32(bp, 0);
				return EPHIDGET_OK;
			case BP_SETGAMMA:
				len = 4;
				packfloat(&buffer[0], (float)getBridgePacketDouble(bp, 0));
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_LEDARRAY_SETGAMMA, buffer, len);
			case BP_SETBRIGHTNESS:
				len = 4;
				packfloat(&buffer[0], (float)getBridgePacketDouble(bp, 0));
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_LEDARRAY_SETBRIGHTNESS, buffer, len);
			case BP_CLEAR:
				len = 0;
				ledArraySupport->protocolLocked = 0;
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_LEDARRAY_CLEAR, buffer, len);
			case BP_OPENRESET:
				len = 0;
				ledArraySupport->protocolLocked = 0;
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_PHIDGET_RESET, buffer, len);
			case BP_CLOSERESET:
				len = 0;
				ledArraySupport->protocolLocked = 0;
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_PHIDGET_RESET, buffer, len);
			case BP_ENABLE:
				len = 0;
				return sendVINTDataPacket(bp->iop, ch, VINT_PACKET_TYPE_PHIDGET_ENABLE, buffer, len);
			default:
				MOS_PANIC("Unexpected packet type");
			}
		default:
			MOS_PANIC("Unexpected Channel Class");
		}
#endif
	default:
		MOS_PANIC("Unexpected device");
	}
}

PhidgetReturnCode ledArray_sendData(PhidgetChannelHandle ch, BridgePacket *bp) {

	uint8_t buffer[16384 + 4];
	size_t dataSize = bp->entry[2].len;
	size_t totalCount = dataSize + 4;

	pack16(&buffer[0], (uint16_t)getBridgePacketUInt32(bp, 0)); // offset
	pack16(&buffer[2], (uint16_t)getBridgePacketUInt32(bp, 1)); // fadeTime
	memcpy(&buffer[4], (const uint8_t *)getBridgePacketUInt8Array(bp, 2), dataSize);

	return ledArray_sendDataBuffer(ch, totalCount, (const uint8_t *)buffer, bp);
}

PhidgetReturnCode ledArray_sendAnimation(PhidgetChannelHandle ch, BridgePacket *bp) {

	uint8_t buffer[1024 + 8];
	size_t dataSize = bp->entry[5].len;
	size_t totalCount = dataSize + 8;

	buffer[0] = (uint8_t)getBridgePacketInt32(bp, 0);		   // animation number
	pack16(&buffer[1], (uint16_t)getBridgePacketInt32(bp, 1)); // start Addr
	pack16(&buffer[3], (uint16_t)getBridgePacketInt32(bp, 2)); // span
	pack16(&buffer[5], (uint16_t)getBridgePacketInt32(bp, 3)); // time
	buffer[7] = (uint8_t)getBridgePacketInt32(bp, 4);		   // animation type

	memcpy(&buffer[8], (const uint8_t *)getBridgePacketUInt8Array(bp, 5), dataSize);

	return ledArray_sendDataBuffer(ch, totalCount, (const uint8_t *)buffer, bp);
}

static PhidgetReturnCode sendTXDataVINT(mosiop_t iop, PhidgetChannelHandle ch, uint8_t *buf, size_t packetLen, PhidgetTransaction *trans) {
	PhidgetReturnCode ret;

	ret = sendVINTDataPacketTransaction(iop, ch, VINT_PACKET_TYPE_LEDARRAY_TX_DATA, buf, packetLen, trans);

	if (ret != EPHIDGET_OK)
		return ret;

	return EPHIDGET_OK;
}

PhidgetReturnCode ledArray_sendDataBuffer(PhidgetChannelHandle ch, size_t len, const uint8_t *rawBuffer, BridgePacket *bp) {
	PhidgetLEDArraySupportHandle ledArraySupport = LEDARRAY_SUPPORT(ch);
	PhidgetReturnCode ret;
	size_t packetLen;
	uint32_t packetCount = 0;
	PhidgetTransaction trans;
	uint16_t packetID;
	uint16_t packetInfo;
	PhidgetReturnCode res1;
	size_t i = 0, j = 0;
	size_t lenCapture = len;
	uint8_t outBuffer[BPE_MAXARRAY_LEN];
	uint16_t requestRGBW = 0;

	switch (ch->UCD->uid) {
#if PHIDUID_LED0100_USB_SUPPORTED
	case PHIDCHUID_LED0100_LEDARRAY_100_VINT:
	case PHIDCHUID_LED0100_LEDARRAY_100_USB:
		if (bp->vpkt == BP_DATAOUT) {
			for (i = 0; i < 4; i++)
				outBuffer[i] = rawBuffer[i];
		} else if (bp->vpkt == BP_SETANIMATION) {
			for (i = 0; i < 8; i++)
				outBuffer[i] = rawBuffer[i];
		}

		switch (ledArraySupport->ledProtocol) {
		case LED_PROTOCOL_GRB:
		case LED_PROTOCOL_RGB:
			requestRGBW = 0;
			break;
		case LED_PROTOCOL_GRBW:
		case LED_PROTOCOL_RGBW:
			requestRGBW = 1;
			break;
		}

		for (j = i; i < lenCapture; i += 4) {
			switch (ledArraySupport->ledProtocol) {
			case LED_PROTOCOL_GRB:
				outBuffer[j + 1] = rawBuffer[i + 0]; // r
				outBuffer[j + 0] = rawBuffer[i + 1]; // g
				outBuffer[j + 2] = rawBuffer[i + 2]; // b
				j += 3;
				len--;
				break;
			case LED_PROTOCOL_RGB:
				outBuffer[j + 0] = rawBuffer[i + 0]; // r
				outBuffer[j + 1] = rawBuffer[i + 1]; // g
				outBuffer[j + 2] = rawBuffer[i + 2]; // b
				j += 3;
				len--;
				break;
			case LED_PROTOCOL_GRBW:
				outBuffer[i + 1] = rawBuffer[i + 0]; // r
				outBuffer[i + 0] = rawBuffer[i + 1]; // g
				outBuffer[i + 2] = rawBuffer[i + 2]; // b
				outBuffer[i + 3] = rawBuffer[i + 3]; // w
				break;
			case LED_PROTOCOL_RGBW:
				outBuffer[i + 0] = rawBuffer[i + 0]; // r
				outBuffer[i + 1] = rawBuffer[i + 1]; // g
				outBuffer[i + 2] = rawBuffer[i + 2]; // b
				outBuffer[i + 3] = rawBuffer[i + 3]; // w
				break;
			}
		}
		break;
#endif
	default:
		MOS_PANIC("Invalid Channel UID");
	}

	uint8_t buf[USB_OUT_PACKET_LENGTH];

	mostime_t duration;
	mostime_t start;
	mostime_t maxDuration;
	mostime_t remainingTime;

	maxDuration = 10000000; // we want the FW to indicate a timeout before the sendData call times out

	start = mos_gettime_usec();

retransmit:
	packetCount = 0;
	// Assign unique packet ID
	PhidgetLock(ch);
	LEDARRAY_SUPPORT(ch)->packetID++;
	LEDARRAY_SUPPORT(ch)->packetID &= MAX_PACKET_ID; // 8-bit packet ID
	if (LEDARRAY_SUPPORT(ch)->packetID == 0)
		LEDARRAY_SUPPORT(ch)->packetID = 0x0001;
	packetID = LEDARRAY_SUPPORT(ch)->packetID;
	PhidgetUnlock(ch);

	// Wait until previous packets are dealt with, ensures the device will accept the packet

	ret = EPHIDGET_OK;

	// Transfer the packet
	if (ch->parent->deviceInfo.class != PHIDCLASS_VINT) {
		if (len > 0) {
			if (len <= (USB_OUT_PACKET_LENGTH - USB_OUT_PACKET_OVERHEAD)) {
				packetLen = len + USB_OUT_PACKET_OVERHEAD;
				packetInfo = NEW_PACKET_FLAG | packetID;
				if (bp->vpkt == BP_SETANIMATION)
					packetInfo |= IS_ANIMATION_FLAG;
				if (requestRGBW)
					packetInfo |= RGBW_FLAG;
				pack16(&buf[0], packetInfo);
				buf[2] = (uint8_t)(len >> 16); // pack 24
				buf[3] = (uint8_t)(len >> 8);
				buf[4] = (uint8_t)(len & 0xFF);
				memcpy(&buf[USB_OUT_PACKET_OVERHEAD], outBuffer, len);
				remainingTime = maxDuration - (mos_gettime_usec() - start);
				ret = PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)ch->parent, PHIDGETUSB_REQ_BULK_WRITE, 0, 0, buf, &packetLen, remainingTime / 1000);
				if (ret != EPHIDGET_OK) {
					goto processReturnCode;
				}

			} else {
				for (i = 0; i + (USB_OUT_PACKET_LENGTH - USB_OUT_PACKET_OVERHEAD) < len; i += (USB_OUT_PACKET_LENGTH - USB_OUT_PACKET_OVERHEAD)) {
					packetLen = USB_OUT_PACKET_LENGTH;
					if (i == 0) {
						packetInfo = NEW_PACKET_FLAG | packetID;
						if (bp->vpkt == BP_SETANIMATION)
							packetInfo |= IS_ANIMATION_FLAG;
						if (requestRGBW)
							packetInfo |= RGBW_FLAG;
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

					if (LEDARRAY_SUPPORT(ch)->droppedPacketID == packetID) {
						LEDARRAY_SUPPORT(ch)->droppedPacketID = NO_ACTIVE_PACKET;
						ret = EPHIDGET_INTERRUPTED;
						goto processReturnCode;
					}
					memcpy(&buf[USB_OUT_PACKET_OVERHEAD], outBuffer + i, (packetLen - USB_OUT_PACKET_OVERHEAD));
					remainingTime = maxDuration - (mos_gettime_usec() - start);
					ret = PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)ch->parent, PHIDGETUSB_REQ_BULK_WRITE, 0, 0, buf, &packetLen, remainingTime / 1000);
					if (ret != EPHIDGET_OK) {
						goto processReturnCode;
					}
					packetCount++;
				}
				if (i != len) {
					packetLen = len - i + USB_OUT_PACKET_OVERHEAD;
					pack16(&buf[0], packetID);
					buf[2] = (uint8_t)(packetCount >> 16); // pack 24
					buf[3] = (uint8_t)(packetCount >> 8);
					buf[4] = (uint8_t)(packetCount & 0xFF);

					if (LEDARRAY_SUPPORT(ch)->droppedPacketID == packetID) {
						LEDARRAY_SUPPORT(ch)->droppedPacketID = NO_ACTIVE_PACKET;
						ret = EPHIDGET_INTERRUPTED;
						goto processReturnCode;
					}
					memcpy(&buf[USB_OUT_PACKET_OVERHEAD], outBuffer + i, (packetLen - USB_OUT_PACKET_OVERHEAD));
					remainingTime = maxDuration - (mos_gettime_usec() - start);
					ret = PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)ch->parent, PHIDGETUSB_REQ_BULK_WRITE, 0, 0, buf, &packetLen, remainingTime / 1000);
					if (ret != EPHIDGET_OK) {
						goto processReturnCode;
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
				if (bp->vpkt == BP_SETANIMATION)
					packetInfo |= IS_ANIMATION_FLAG;
				if (requestRGBW)
					packetInfo |= RGBW_FLAG;
				pack16(&buf[0], packetInfo);
				buf[2] = (uint8_t)(len >> 16); // pack 24
				buf[3] = (uint8_t)(len >> 8);
				buf[4] = (uint8_t)(len & 0xFF);
				memcpy(&buf[USB_OUT_PACKET_OVERHEAD], outBuffer, len);
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
						if (bp->vpkt == BP_SETANIMATION)
							packetInfo |= IS_ANIMATION_FLAG;
						if (requestRGBW)
							packetInfo |= RGBW_FLAG;
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

					if (LEDARRAY_SUPPORT(ch)->droppedPacketID == packetID) {
						LEDARRAY_SUPPORT(ch)->droppedPacketID = NO_ACTIVE_PACKET;
						ret = EPHIDGET_INTERRUPTED;
						goto done;
					}
					memcpy(&buf[USB_OUT_PACKET_OVERHEAD], outBuffer + i, (packetLen - USB_OUT_PACKET_OVERHEAD));
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

					if (LEDARRAY_SUPPORT(ch)->droppedPacketID == packetID) {
						LEDARRAY_SUPPORT(ch)->droppedPacketID = NO_ACTIVE_PACKET;
						ret = EPHIDGET_INTERRUPTED;
						goto done;
					}
					memcpy(&buf[USB_OUT_PACKET_OVERHEAD], outBuffer + i, (packetLen - USB_OUT_PACKET_OVERHEAD));
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
			duration = (mos_gettime_usec() - start);
			if (duration >= maxDuration)
				return (EPHIDGET_TIMEOUT);
			if (!(ISATTACHED(ch))) {
				return (EPHIDGET_CLOSED);
			}
			goto retransmit;
		}
	}

	////Wait to see if the device accepted the packet, this makes the return code from sendPacket mean something
	// start = mos_gettime_usec();

	for (;;) {
		// Wait for acknowledgement that the packet finished transmitting, even if we're not reporting response data.
		// This helps keep the user's program in sync
		if (LEDARRAY_SUPPORT(ch)->ackID == packetID) { // ackID

			return (EPHIDGET_OK);
		}

		if (LEDARRAY_SUPPORT(ch)->droppedPacketID == packetID) {
processReturnCode:
			LEDARRAY_SUPPORT(ch)->droppedPacketID = NO_ACTIVE_PACKET;

			if (LEDARRAY_SUPPORT(ch)->droppedPacketReason == LED_TX_DROPPED_TOOBIG)
				return (MOS_ERROR(bp->iop, EPHIDGET_INVALIDARG, "The packet is too large to be accepted by the device"));

			if (LEDARRAY_SUPPORT(ch)->droppedPacketReason == LED_TX_DROPPED_INVALID)
				return (MOS_ERROR(bp->iop, EPHIDGET_INVALIDARG, "Animations cannot be configured to overlap"));

			if (LEDARRAY_SUPPORT(ch)->droppedPacketReason == LED_TX_DROPPED_NOT_CONFIGURED)
				return (MOS_ERROR(bp->iop, EPHIDGET_INVALIDARG, "Fade times cannot be used when animations are active"));

			if (LEDARRAY_SUPPORT(ch)->droppedPacketReason == LED_TX_DROPPED_TRYAGAIN)
				goto retransmit;

			/*duration = (mos_gettime_usec() - start);
			if (duration >= maxDuration)
				return (EPHIDGET_TIMEOUT);*/

			goto retransmit;

			/*if (LEDARRAY_SUPPORT(ch)->droppedPacketReason == LED_TX_DROPPED_TIMEOUT) {
				return (MOS_ERROR(bp->iop, EPHIDGET_TIMEOUT, "The packet timed out while waiting to be transmitted. Check that your system can keep up with the data being sent."));
			}*/

			// return EPHIDGET_INTERRUPTED;
		}

		if (!(ISATTACHED(ch))) {
			return (EPHIDGET_CLOSED);
		}

		duration = (mos_gettime_usec() - start);
		if (duration >= maxDuration) {
			//	phid->activePacket = NO_ACTIVE_PACKET; // transmission failed, the packet is no longer active
			return (EPHIDGET_TIMEOUT);
		}
	}
}

/*
 * Public API
 */

void PhidgetLEDArraySupport_free(PhidgetLEDArraySupportHandle *arg) {

	if (arg == NULL || *arg == NULL)
		return;

	assert(arg);
	mos_mutex_destroy(&((*arg)->sendLock));
	mos_mutex_destroy(&((*arg)->receiveLock));

	mos_free(*arg, sizeof(PhidgetLEDArraySupport));
	*arg = NULL;
}

PhidgetReturnCode
PhidgetLEDArraySupport_create(PhidgetLEDArraySupportHandle *arg) {

	TESTPTR_PR(arg);
	*arg = mos_zalloc(sizeof(PhidgetLEDArraySupport));

	assert(arg);
	mos_mutex_init(&((*arg)->sendLock));
	mos_mutex_init(&((*arg)->receiveLock));

	return (EPHIDGET_OK);
}

void PhidgetLEDArraySupport_init(PhidgetLEDArraySupportHandle ledArray) {

	assert(ledArray);

	ledArray->usbInPacketCount = 0;
	ledArray->packetID = 0;

	ledArray->ackID = NO_ACTIVE_PACKET;

	ledArray->droppedPacketID = NO_ACTIVE_PACKET;
	ledArray->ledProtocol = LED_PROTOCOL_GRB;
	ledArray->protocolLocked = 0;
}
