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
#include "gpp.h"
#include "device/ledarraydevice.h"
#include "class/ledarray.gen.h"

// === Internal Functions === //

// initAfterOpen - sets up the initial state of an object, reading in packets from the device if needed
//				  used during attach initialization - on every attach
static PhidgetReturnCode CCONV
PhidgetLEDArrayDevice_initAfterOpen(PhidgetDeviceHandle device) {
	PhidgetLEDArrayDeviceHandle phid = (PhidgetLEDArrayDeviceHandle)device;
#if PHIDUID_LED0100_USB_SUPPORTED
	uint8_t buffer[MAX_IN_PACKET_SIZE];
	PhidgetReturnCode ret;
	size_t len;
#endif
	assert(phid);

	switch (phid->phid.deviceInfo.UDD->uid) {
#if PHIDUID_LED0100_USB_SUPPORTED
	case PHIDUID_LED0100_USB:
		len = 3;
		ret = PhidgetDevice_transferpacket(NULL, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_DEVICE_READ, 0, 0, buffer, &len, 100);
		if (ret != EPHIDGET_OK)
			return ret;
		phid->inputsEnabled = buffer[0];
		phid->outputsEnabled = buffer[1];
		phid->stateInvalidSent = 0;
		break;
#endif
	default:
		MOS_PANIC("Unexpected device");
	}

	return (EPHIDGET_OK);
}

// dataInput - parses device packets
static PhidgetReturnCode CCONV
PhidgetLEDArrayDevice_dataInput(PhidgetDeviceHandle device, uint8_t *buffer, size_t length) {
	PhidgetLEDArrayDeviceHandle phid = (PhidgetLEDArrayDeviceHandle)device;
#if PHIDUID_LED0100_USB_SUPPORTED
	PhidgetChannelHandle channel;
	PhidgetReturnCode ret;
#endif

	assert(phid);
	assert(buffer);

	// Parse device packets - store data locally
	switch (phid->phid.deviceInfo.UDD->uid) {
#if PHIDUID_LED0100_USB_SUPPORTED
	case PHIDUID_LED0100_USB:
		if (length > 0) {
			switch (buffer[0]) {				
			default:
				if ((channel = getChannel(phid, 0)) != NULL) {
					ret = PhidgetLEDArraySupport_dataInput(channel, buffer, length);
					PhidgetRelease(&channel);
					return ret;
				}
				return EPHIDGET_OK;
			}
		}
		MOS_PANIC("Unexpected packet type");
#endif
	default:
		MOS_PANIC("Unexpected device");
	}
}

static PhidgetReturnCode CCONV
PhidgetLEDArrayDevice_bridgeInput(PhidgetChannelHandle ch, BridgePacket *bp) {
	PhidgetLEDArrayDeviceHandle phid = (PhidgetLEDArrayDeviceHandle)ch->parent;
#if PHIDUID_LED0100_USB_SUPPORTED
	PhidgetChannelHandle channel;
	PhidgetReturnCode ret;
#endif
	assert(phid->phid.deviceInfo.class == PHIDCLASS_LEDARRAY);

	switch (((PhidgetDeviceHandle)phid)->deviceInfo.UDD->uid) {
#if PHIDUID_LED0100_USB_SUPPORTED
	case PHIDUID_LED0100_USB:
		switch (ch->class) {
		case PHIDCHCLASS_LEDARRAY:
			// Handle parameters not handled by LEDArraySupport
			switch (bp->vpkt) {
			
			default:
				break;
			}

			// Send the packet
			if ((channel = getChannel(phid, 0)) != NULL) {
				ret = PhidgetLEDArraySupport_bridgeInput(channel, bp);
				PhidgetRelease(&channel);
				if (ret)
					return ret;
			}

			// If the packet succeeds, set various local settings to keep track of device state
			switch (bp->vpkt) {
			case BP_DATAOUT:
			case BP_SETANIMATION:
				break;
			case BP_OPENRESET:
			case BP_CLOSERESET:
				break;
			case BP_ENABLE:
				break;
			default:
				break;
			}
			return EPHIDGET_OK;
		default:
			MOS_PANIC("Unexpected Channel Class");
		}
#endif
	default:
		MOS_PANIC("Unexpected device");
	}
}

static void CCONV
PhidgetLEDArrayDevice_free(PhidgetDeviceHandle *phid) {

	mos_free(*phid, sizeof(struct _PhidgetLEDArrayDevice));
	*phid = NULL;
}

PhidgetReturnCode
PhidgetLEDArrayDevice_create(PhidgetLEDArrayDeviceHandle *phidp) {
	DEVICECREATE_BODY(LEDArrayDevice, PHIDCLASS_LEDARRAY);
	return (EPHIDGET_OK);
}
