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
#include "device/dataadapterdevice.h"
#include "class/dataadapter.gen.h"

// === Internal Functions === //

// initAfterOpen - sets up the initial state of an object, reading in packets from the device if needed
//				  used during attach initialization - on every attach
static PhidgetReturnCode CCONV
PhidgetDataAdapterDevice_initAfterOpen(PhidgetDeviceHandle device) {
	PhidgetDataAdapterDeviceHandle phid = (PhidgetDataAdapterDeviceHandle)device;
#if (PHIDUID_ADP0001_USB_SUPPORTED || PHIDUID_ADP0002_USB_SUPPORTED)
	uint8_t buffer[MAX_IN_PACKET_SIZE];
	PhidgetReturnCode ret;
	size_t len;
#endif
	assert(phid);

	switch (phid->phid.deviceInfo.UDD->uid) {
#if PHIDUID_ADP0001_USB_SUPPORTED
	case PHIDUID_ADP0001_USB:
		len = sizeof(buffer);
		ret = PhidgetDevice_transferpacket(NULL, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_DEVICE_READ, DATAADAPTER_PACKET_GETIOSTATUS, 0, buffer, &len, 100);
		if (ret != EPHIDGET_OK)
			return ret;
		phid->inputsEnabled = buffer[0];
		phid->outputsEnabled = buffer[1];
		phid->stateInvalidSent = 0;
		break;
#endif /* PHIDUID_ADP0002_USB_SUPPORTED */
#if PHIDUID_ADP0002_USB_SUPPORTED
	case PHIDUID_ADP0002_USB:
		len = sizeof(buffer);
		ret = PhidgetDevice_transferpacket(NULL, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_DEVICE_READ, DATAADAPTER_PACKET_GETIOSTATUS, 0, buffer, &len, 100);
		if (ret != EPHIDGET_OK)
			return ret;
		phid->inputsEnabled = buffer[0];
		phid->outputsEnabled = buffer[1];
		phid->stateInvalidSent = 0;
		break;
#endif /* PHIDUID_ADP0002_USB_SUPPORTED */
	default:
		MOS_PANIC("Unexpected device");
	}

	return (EPHIDGET_OK);
}

// dataInput - parses device packets
static PhidgetReturnCode CCONV
PhidgetDataAdapterDevice_dataInput(PhidgetDeviceHandle device, uint8_t *buffer, size_t length) {
	PhidgetDataAdapterDeviceHandle phid = (PhidgetDataAdapterDeviceHandle)device;
#if (PHIDUID_ADP0001_USB_SUPPORTED || PHIDUID_ADP0002_USB_SUPPORTED)
	PhidgetChannelHandle channel;
	PhidgetReturnCode ret;
	uint8_t lastInputState[DATAADAPTER_MAXINPUTS];
	int i, j;
#endif

	assert(phid);
	assert(buffer);

	// Parse device packets - store data locally
	switch (phid->phid.deviceInfo.UDD->uid) {
#if PHIDUID_ADP0001_USB_SUPPORTED
	case PHIDUID_ADP0001_USB:
		if (length > 0) {
			switch (buffer[0]) {
			case VINT_PACKET_TYPE_DATAADAPTER_PACKET_DATA_ERROR:
			case VINT_PACKET_TYPE_DATAADAPTER_PACKET_DATA_END:
			case VINT_PACKET_TYPE_DATAADAPTER_PACKET_DATA:
			case VINT_PACKET_TYPE_DATAADAPTER_PACKET_DROPPED:
			case VINT_PACKET_TYPE_DATAADAPTER_PACKET_ACK:
				if ((channel = getChannel(phid, 0)) != NULL) {
					ret = PhidgetDataAdapterSupport_dataInput(channel, buffer, length);
					PhidgetRelease(&channel);
					return ret;
				}
				return EPHIDGET_OK;
			case VINT_PACKET_TYPE_DATAADAPTER_VOLTAGE_ERROR:
				if ((channel = getChannel(phid, 0)) != NULL) {
					PhidgetChannel_sendErrorEvent(channel, EEPHIDGET_VOLTAGEERROR, "Voltage Error Detected");
					PhidgetRelease(&channel);
				}
				return (EPHIDGET_OK);
			case STATE_CHANGE:
				for (j = 0; j < phid->devChannelCnts.numInputs; j++) {
					//				inputState[j] = PUNK_BOOL;
					lastInputState[j] = phid->inputState[j];
					phid->inputState[j] = ((buffer[1] & (1 << j)) != 0);
				}

				for (i = 0; i < phid->devChannelCnts.numInputs; i++) {
					if ((phid->inputsEnabled & (1 << i)) == 0)
						continue;
					int chIndex = i + phid->devChannelCnts.numDataAdapters;
					if ((channel = getChannel(phid, chIndex)) != NULL) {
						if (phid->inputState[i] != PUNK_BOOL && phid->inputState[i] != lastInputState[i]) {
							bridgeSendToChannel(channel, BP_STATECHANGE, 1, "%d", (int)(phid->inputState[i]));
						}
						PhidgetRelease(&channel);
					}
				}
				return (EPHIDGET_OK);
			case STATE_INVALID:
				for (j = 0; j < phid->devChannelCnts.numInputs; j++) {
					if ((buffer[1] & (1 << j)) != 0) {
						int chIndex = j + phid->devChannelCnts.numDataAdapters;
						lastInputState[j] = PUNK_BOOL;
						phid->inputState[j] = PUNK_BOOL;
						phid->inputsEnabled &= ~(1 << j);
						if ((channel = getChannel(phid, chIndex)) != NULL) {
							PhidgetChannel_sendErrorEvent(channel, EEPHIDGET_INVALIDSTATE, "Channel Invalidated. This means some other aspect of the device is making use of the channel.");
							PhidgetRelease(&channel);
						}
					}
					if ((buffer[2] & (1 << j)) != 0) {
						int chIndex = j + phid->devChannelCnts.numDataAdapters + phid->devChannelCnts.numInputs;
						phid->outputsEnabled &= ~(1 << j);
						if ((channel = getChannel(phid, chIndex)) != NULL) {
							PhidgetChannel_sendErrorEvent(channel, EEPHIDGET_INVALIDSTATE, "Channel Invalidated. This means some other aspect of the device is making use of the channel.");
							PhidgetRelease(&channel);
						}
					}
				}

				return (EPHIDGET_OK);
			default:
				MOS_PANIC("Unexpected packet type");
			}
		}
		MOS_PANIC("Unexpected packet type");
#endif /* PHIDUID_ADP0001_USB_SUPPORTED */
#if PHIDUID_ADP0002_USB_SUPPORTED
	case PHIDUID_ADP0002_USB:
		if (length > 0) {
			switch (buffer[0]) {
			case VINT_PACKET_TYPE_DATAADAPTER_PACKET_DATA_ERROR:
			case VINT_PACKET_TYPE_DATAADAPTER_PACKET_DATA_END:
			case VINT_PACKET_TYPE_DATAADAPTER_PACKET_DATA:
			case VINT_PACKET_TYPE_DATAADAPTER_PACKET_DROPPED:
			case VINT_PACKET_TYPE_DATAADAPTER_PACKET_ACK:
				if ((channel = getChannel(phid, 0)) != NULL) {
					ret = PhidgetDataAdapterSupport_dataInput(channel, buffer, length);
					PhidgetRelease(&channel);
					return ret;
				}
				return EPHIDGET_OK;
			case VINT_PACKET_TYPE_DATAADAPTER_VOLTAGE_ERROR:
				if ((channel = getChannel(phid, 0)) != NULL) {
					PhidgetChannel_sendErrorEvent(channel, EEPHIDGET_VOLTAGEERROR, "Voltage Error Detected");
					PhidgetRelease(&channel);
				}
				return (EPHIDGET_OK);
			case STATE_CHANGE:
				for (j = 0; j < phid->devChannelCnts.numInputs; j++) {
					//				inputState[j] = PUNK_BOOL;
					lastInputState[j] = phid->inputState[j];
					phid->inputState[j] = ((buffer[1] & (1 << j)) != 0);
				}

				for (i = 0; i < phid->devChannelCnts.numInputs; i++) {
					if ((phid->inputsEnabled & (1 << i)) == 0)
						continue;
					int chIndex = i + phid->devChannelCnts.numDataAdapters;
					if ((channel = getChannel(phid, chIndex)) != NULL) {
						if (phid->inputState[i] != PUNK_BOOL && phid->inputState[i] != lastInputState[i]) {
							bridgeSendToChannel(channel, BP_STATECHANGE, 1, "%d", (int)(phid->inputState[i]));
						}
						PhidgetRelease(&channel);
					}
				}
				return (EPHIDGET_OK);
			case STATE_INVALID:
				for (j = 0; j < phid->devChannelCnts.numInputs; j++) {
					if ((buffer[1] & (1 << j)) != 0) {
						int chIndex = j + phid->devChannelCnts.numDataAdapters;
						lastInputState[j] = PUNK_BOOL;
						phid->inputState[j] = PUNK_BOOL;
						phid->inputsEnabled &= ~(1 << j);
						if ((channel = getChannel(phid, chIndex)) != NULL) {
							PhidgetChannel_sendErrorEvent(channel, EEPHIDGET_INVALIDSTATE, "Channel Invalidated. This means some other aspect of the device is making use of the channel.");
							PhidgetRelease(&channel);
						}
					}
					if ((buffer[2] & (1 << j)) != 0) {
						int chIndex = j + phid->devChannelCnts.numDataAdapters + phid->devChannelCnts.numInputs;
						phid->outputsEnabled &= ~(1 << j);
						if ((channel = getChannel(phid, chIndex)) != NULL) {
							PhidgetChannel_sendErrorEvent(channel, EEPHIDGET_INVALIDSTATE, "Channel Invalidated. This means some other aspect of the device is making use of the channel.");
							PhidgetRelease(&channel);
						}
					}
				}

				return (EPHIDGET_OK);
			default:
				MOS_PANIC("Unexpected packet type");
			}
		}
		MOS_PANIC("Unexpected packet type");
#endif /* PHIDUID_ADP0002_USB_SUPPORTED */
	default:
		MOS_PANIC("Unexpected device");
	}
}

static PhidgetReturnCode CCONV
PhidgetDataAdapterDevice_bridgeInput(PhidgetChannelHandle ch, BridgePacket *bp) {
	PhidgetDataAdapterDeviceHandle phid = (PhidgetDataAdapterDeviceHandle)ch->parent;
#if (PHIDUID_ADP0001_USB_SUPPORTED || PHIDUID_ADP0002_USB_SUPPORTED)
	PhidgetChannelHandle channel;
	PhidgetReturnCode ret;
	unsigned char buffer[MAX_OUT_PACKET_SIZE] = {0};
	double dutyCycle;
	int32_t state;
	size_t len;
#endif

	assert(phid->phid.deviceInfo.class == PHIDCLASS_DATAADAPTER);

	switch (((PhidgetDeviceHandle)phid)->deviceInfo.UDD->uid) {
#if PHIDUID_ADP0001_USB_SUPPORTED
	case PHIDUID_ADP0001_USB:
		switch (ch->class) {
		case PHIDCHCLASS_DATAADAPTER:
			// Handle parameters not handled by DataAdapterSupport
			switch (bp->vpkt) {
			case BP_SETIOVOLTAGE:
				len = 1;
				buffer[0] = (uint8_t)getBridgePacketInt32(bp, 0);
				ret = PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_DATAADAPTER_IOVOLTAGE, 0, buffer, &len, 100);
				// Need to update the other channels because power supply is board-wide
				if (ret == EPHIDGET_OK) {
					phid->dataAdapterVoltage = getBridgePacketInt32(bp, 0);
					bridgeSendToChannelSiblings(ch, BP_IOVOLTAGECHANGE, 1, "%d", phid->dataAdapterVoltage);
				}
				return ret;
			default:
				break;
			}

			// Send the packet
			if ((channel = getChannel(phid, 0)) != NULL) {
				ret = PhidgetDataAdapterSupport_bridgeInput(channel, bp);
				PhidgetRelease(&channel);
				if (ret)
					return ret;
			}

			// If the packet succeeds, set various local settings to keep track of device state
			switch (bp->vpkt) {
			case BP_DATAOUT:
			case BP_DATAEXCHANGE:
			case BP_SETBAUDRATE:
			case BP_SETDATABITS:
			case BP_SETSPIMODE:
			case BP_SETIGNORERESPONSE:
			case BP_SETENDIANNESS:
			case BP_SETI2CFORMAT:
			case BP_OPENRESET:
			case BP_CLOSERESET:
				break;
			case BP_ENABLE:
				break;
			case BP_SETADDRESS:
				phid->address = getBridgePacketUInt32(bp, 0);
				break;
			default:
				break;
			}
			return EPHIDGET_OK;
		case PHIDCHCLASS_DIGITALINPUT:
			switch (bp->vpkt) {
			case BP_SETIOVOLTAGE:
				len = 1;
				buffer[0] = (uint8_t)getBridgePacketInt32(bp, 0);
				ret = PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_DATAADAPTER_IOVOLTAGE, 0, buffer, &len, 100);
				// Need to update the other channels because power supply is board-wide
				if (ret == EPHIDGET_OK) {
					phid->dataAdapterVoltage = getBridgePacketInt32(bp, 0);
					bridgeSendToChannelSiblings(ch, BP_IOVOLTAGECHANGE, 1, "%d", phid->dataAdapterVoltage);
				}
				return ret;
			case BP_SETINPUTMODE:
				len = 1;
				buffer[0] = getBridgePacketInt32(bp, 0);
				return PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_CHANNEL_WRITE, VINT_PACKET_TYPE_DIGITALINPUT_SETINPUTMODE, ch->uniqueIndex, buffer, &len, 100);
			case BP_OPENRESET:
			case BP_CLOSERESET:

				phid->inputsEnabled &= ~(1 << ch->index);
				len = 0;
				return PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_CHANNEL_WRITE, VINT_PACKET_TYPE_PHIDGET_RESET, ch->uniqueIndex, buffer, &len, 100);
			case BP_ENABLE:
				// Latest enable will override
				phid->inputsEnabled |= (1 << ch->index);
				phid->outputsEnabled &= ~(1 << ch->index);
				len = 0;
				// Send the Enable packet regardless of if it will succeed, so the device has a chance to send a State Error indicating to the user the channel is no longer valid.
				return PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_CHANNEL_WRITE, VINT_PACKET_TYPE_PHIDGET_ENABLE, ch->uniqueIndex, buffer, &len, 100);
			default:
				MOS_PANIC("Unexpected packet type");
			}
		case PHIDCHCLASS_DIGITALOUTPUT:
			switch (bp->vpkt) {
			case BP_SETIOVOLTAGE:
				len = 1;
				buffer[0] = (uint8_t)getBridgePacketInt32(bp, 0);
				ret = PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_DATAADAPTER_IOVOLTAGE, 0, buffer, &len, 100);
				// Need to update the other channels because power supply is board-wide
				if (ret == EPHIDGET_OK) {
					phid->dataAdapterVoltage = getBridgePacketInt32(bp, 0);
					bridgeSendToChannelSiblings(ch, BP_IOVOLTAGECHANGE, 1, "%d", phid->dataAdapterVoltage);
				}
				return ret;
			case BP_SETDUTYCYCLE:
				dutyCycle = getBridgePacketDouble(bp, 0);
				if (dutyCycle != 0 && dutyCycle != 1)
					return EPHIDGET_INVALIDARG;
				buffer[0] = (uint8_t)dutyCycle;
				len = 1;
				return PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_CHANNEL_WRITE, VINT_PACKET_TYPE_DIGITALOUTPUT_SETDUTYCYCLE, ch->uniqueIndex, buffer, &len, 100);
			case BP_SETSTATE:
				state = getBridgePacketInt32(bp, 0);
				if (state != 0 && state != 1)
					return EPHIDGET_INVALIDARG;
				buffer[0] = (uint8_t)state;
				len = 1;
				return PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_CHANNEL_WRITE, VINT_PACKET_TYPE_DIGITALOUTPUT_SETDUTYCYCLE, ch->uniqueIndex, buffer, &len, 100);
			case BP_OPENRESET:
			case BP_CLOSERESET:

				phid->outputsEnabled &= ~(1 << ch->index);
				len = 0;
				return PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_CHANNEL_WRITE, VINT_PACKET_TYPE_PHIDGET_RESET, ch->uniqueIndex, buffer, &len, 100);
			case BP_ENABLE:
				// Latest enable will override
				phid->outputsEnabled |= (1 << ch->index);
				phid->inputsEnabled &= ~(1 << ch->index);
				len = 0;
				// Send the Enable packet regardless of if it will succeed, so the device has a chance to send a State Error indicating to the user the channel is no longer valid.
				return PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_CHANNEL_WRITE, VINT_PACKET_TYPE_PHIDGET_ENABLE, ch->uniqueIndex, buffer, &len, 100);
			default:
				MOS_PANIC("Unexpected packet type");
			}
		default:
			MOS_PANIC("Unexpected Channel Class");
		}
#endif /* PHIDUID_ADP0001_USB_SUPPORTED */
#if PHIDUID_ADP0002_USB_SUPPORTED
	case PHIDUID_ADP0002_USB:
		switch (ch->class) {
		case PHIDCHCLASS_DATAADAPTER:
			// Handle parameters not handled by DataAdapterSupport
			switch (bp->vpkt) {
			case BP_SETIOVOLTAGE:
				len = 1;
				buffer[0] = (uint8_t)getBridgePacketInt32(bp, 0);
				ret = PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_DATAADAPTER_IOVOLTAGE, 0, buffer, &len, 100);
				// Need to update the other channels because power supply is board-wide
				if (ret == EPHIDGET_OK) {
					phid->dataAdapterVoltage = getBridgePacketInt32(bp, 0);
					bridgeSendToChannelSiblings(ch, BP_IOVOLTAGECHANGE, 1, "%d", phid->dataAdapterVoltage);
				}
				return ret;
			default:
				break;
			}

			// Send the packet
			if ((channel = getChannel(phid, 0)) != NULL) {
				ret = PhidgetDataAdapterSupport_bridgeInput(channel, bp);
				PhidgetRelease(&channel);
				if (ret)
					return ret;
			}

			// If the packet succeeds, set various local settings to keep track of device state
			switch (bp->vpkt) {
			case BP_DATAOUT:
			case BP_DATAEXCHANGE:
			case BP_SETBAUDRATE:
			case BP_SETDATABITS:
			case BP_SETSPIMODE:
			case BP_SETIGNORERESPONSE:
			case BP_SETENDIANNESS:
			case BP_SETI2CFORMAT:
				break;
			case BP_OPENRESET:
			case BP_CLOSERESET:
				break;
			case BP_ENABLE:
				break;
			case BP_SETADDRESS:
				phid->address = getBridgePacketUInt32(bp, 0);
				break;
			default:
				break;
			}
			return EPHIDGET_OK;
		case PHIDCHCLASS_DIGITALINPUT:
			switch (bp->vpkt) {
			case BP_SETIOVOLTAGE:
				len = 1;
				buffer[0] = (uint8_t)getBridgePacketInt32(bp, 0);
				ret = PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_DATAADAPTER_IOVOLTAGE, 0, buffer, &len, 100);
				// Need to update the other channels because power supply is board-wide
				if (ret == EPHIDGET_OK) {
					phid->dataAdapterVoltage = getBridgePacketInt32(bp, 0);
					bridgeSendToChannelSiblings(ch, BP_IOVOLTAGECHANGE, 1, "%d", phid->dataAdapterVoltage);
				}
				return ret;
			case BP_OPENRESET:
			case BP_CLOSERESET:

				phid->inputsEnabled &= ~(1 << ch->index);
				len = 0;
				return PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_CHANNEL_WRITE, VINT_PACKET_TYPE_PHIDGET_RESET, ch->uniqueIndex, buffer, &len, 100);
			case BP_ENABLE:
				// Latest enable will override
				phid->inputsEnabled |= (1 << ch->index);
				phid->outputsEnabled &= ~(1 << ch->index);
				len = 0;
				// Send the Enable packet regardless of if it will succeed, so the device has a chance to send a State Error indicating to the user the channel is no longer valid.
				return PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_CHANNEL_WRITE, VINT_PACKET_TYPE_PHIDGET_ENABLE, ch->uniqueIndex, buffer, &len, 100);
			default:
				MOS_PANIC("Unexpected packet type");
			}
		case PHIDCHCLASS_DIGITALOUTPUT:
			switch (bp->vpkt) {
			case BP_SETIOVOLTAGE:
				len = 1;
				buffer[0] = (uint8_t)getBridgePacketInt32(bp, 0);
				ret = PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_DEVICE_WRITE, VINT_PACKET_TYPE_DATAADAPTER_IOVOLTAGE, 0, buffer, &len, 100);
				// Need to update the other channels because power supply is board-wide
				if (ret == EPHIDGET_OK) {
					phid->dataAdapterVoltage = getBridgePacketInt32(bp, 0);
					bridgeSendToChannelSiblings(ch, BP_IOVOLTAGECHANGE, 1, "%d", phid->dataAdapterVoltage);
				}
				return ret;
			case BP_SETDUTYCYCLE:
				dutyCycle = getBridgePacketDouble(bp, 0);
				if (dutyCycle != 0 && dutyCycle != 1)
					return EPHIDGET_INVALIDARG;
				buffer[0] = (uint8_t)dutyCycle;
				len = 1;
				return PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_CHANNEL_WRITE, VINT_PACKET_TYPE_DIGITALOUTPUT_SETDUTYCYCLE, ch->uniqueIndex, buffer, &len, 100);
			case BP_SETSTATE:
				state = getBridgePacketInt32(bp, 0);
				if (state != 0 && state != 1)
					return EPHIDGET_INVALIDARG;
				buffer[0] = (uint8_t)state;
				len = 1;
				return PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_CHANNEL_WRITE, VINT_PACKET_TYPE_DIGITALOUTPUT_SETDUTYCYCLE, ch->uniqueIndex, buffer, &len, 100);
			case BP_OPENRESET:
			case BP_CLOSERESET:

				phid->outputsEnabled &= ~(1 << ch->index);
				len = 0;
				return PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_CHANNEL_WRITE, VINT_PACKET_TYPE_PHIDGET_RESET, ch->uniqueIndex, buffer, &len, 100);
			case BP_ENABLE:
				// Latest enable will override
				phid->outputsEnabled |= (1 << ch->index);
				phid->inputsEnabled &= ~(1 << ch->index);
				len = 0;
				// Send the Enable packet regardless of if it will succeed, so the device has a chance to send a State Error indicating to the user the channel is no longer valid.
				return PhidgetDevice_transferpacket(bp->iop, (PhidgetDeviceHandle)phid, PHIDGETUSB_REQ_CHANNEL_WRITE, VINT_PACKET_TYPE_PHIDGET_ENABLE, ch->uniqueIndex, buffer, &len, 100);
			default:
				MOS_PANIC("Unexpected packet type");
			}
		default:
			MOS_PANIC("Unexpected Channel Class");
		}
#endif /* PHIDUID_ADP0002_USB_SUPPORTED */
	default:
		MOS_PANIC("Unexpected device");
	}
}

static void CCONV
PhidgetDataAdapterDevice_free(PhidgetDeviceHandle *phid) {

	mos_free(*phid, sizeof(struct _PhidgetDataAdapterDevice));
	*phid = NULL;
}

PhidgetReturnCode
PhidgetDataAdapterDevice_create(PhidgetDataAdapterDeviceHandle *phidp) {
	DEVICECREATE_BODY(DataAdapterDevice, PHIDCLASS_DATAADAPTER);
	return (EPHIDGET_OK);
}
