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

#ifndef __LEDARRAYSUPPORT
#define __LEDARRAYSUPPORT

#include "types.gen.h"

#define LEDARRAY_MAX_LED_COUNT		4096
#define USB_OUT_PACKET_LENGTH		64

#define USB_OUT_PACKET_OVERHEAD		5
#define USB_IN_PACKET_OVERHEAD		5
#define NEW_PACKET_FLAG				0x8000
#define IS_ANIMATION_FLAG			0x4000
#define RGBW_FLAG					0x2000
#define MAX_PACKET_ID				0x00FF

#define NEW_RX_READY				0xFFFF
#define NO_ACTIVE_PACKET			0xFFFF

PhidgetReturnCode ledArray_sendData(PhidgetChannelHandle ch, BridgePacket *bp);
PhidgetReturnCode ledArray_sendSegment(PhidgetChannelHandle ch, BridgePacket *bp);
PhidgetReturnCode ledArray_sendAnimation(PhidgetChannelHandle ch, BridgePacket *bp);
PhidgetReturnCode ledArray_sendDataBuffer(PhidgetChannelHandle ch, size_t len, const uint8_t *buffer, BridgePacket *bp);

typedef enum LEDArray_TXDroppedReason {
	LED_TX_DROPPED_UNKNOWN = 0,
	LED_TX_DROPPED_TIMEOUT = 1,
	LED_TX_DROPPED_NOT_CONFIGURED = 4,
	LED_TX_DROPPED_TOOBIG = 5,
	LED_TX_DROPPED_INVALID = 6,
	LED_TX_DROPPED_TRYAGAIN = 7
} LEDArray_TXDroppedReason;

typedef struct {

	/* Public Members */

	/* Private Members */
	uint16_t usbInPacketCount;
	uint32_t packetID;

	uint16_t ackID;

	mos_mutex_t sendLock;
	mos_mutex_t receiveLock;

	uint32_t protocolLocked;

	uint16_t droppedPacketID;
	LEDArray_TXDroppedReason droppedPacketReason;

	PhidgetLEDArray_Protocol ledProtocol;

} PhidgetLEDArraySupport, *PhidgetLEDArraySupportHandle;

void PhidgetLEDArraySupport_free(PhidgetLEDArraySupportHandle *arg);
PhidgetReturnCode PhidgetLEDArraySupport_create(PhidgetLEDArraySupportHandle *ir);
void PhidgetLEDArraySupport_init(PhidgetLEDArraySupportHandle ir);
PhidgetReturnCode PhidgetLEDArraySupport_bridgeInput(PhidgetChannelHandle ch, BridgePacket *bp);
PhidgetReturnCode PhidgetLEDArraySupport_dataInput(PhidgetChannelHandle ch, const uint8_t *buf, size_t len);

#endif
