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
#include "util/rfidsupport.h"

// Access the PhidgetRFIDSupport struct via the channel private pointer
#define RFID_SUPPORT(ch) ((PhidgetRFIDSupportHandle)(((PhidgetChannelHandle)(ch))->private))

/*
 * Public API
 */

const uint8_t PhidgetRFIDSupport_hexChar[] = {
	'0', '1', '2', '3',
	'4', '5', '6', '7',
	'8', '9', 'a', 'b',
	'c', 'd', 'e', 'f'};

PhidgetReturnCode
RFIDSupport_setLatestTagString(mosiop_t iop, PhidgetChannelHandle ch, const char *tagString) {

	PhidgetRFIDSupportHandle rfidSupport = RFID_SUPPORT(ch);

	mos_strncpy(rfidSupport->latestTagString, tagString, sizeof(rfidSupport->latestTagString) - 1);

	return EPHIDGET_OK;
}

PhidgetReturnCode
RFIDSupport_waitForTag(mosiop_t iop, PhidgetChannelHandle ch, const char *tagString, int timeout, mos_mutex_t *tagLock) {

	PhidgetRFIDSupportHandle rfidSupport = RFID_SUPPORT(ch);

	// Wait for this tag to show up
	while (timeout > 0) {
		if (tagLock == NULL)
			PhidgetLock(ch);
		else
			mos_mutex_lock(tagLock);

		if (!strncmp(rfidSupport->latestTagString, tagString, RFIDDevice_MAX_TAG_STRING_LEN)) {
			if (tagLock == NULL)
				PhidgetUnlock(ch);
			else
				mos_mutex_unlock(tagLock);
			return (EPHIDGET_OK);
		}
		if (tagLock == NULL)
			PhidgetUnlock(ch);
		else
			mos_mutex_unlock(tagLock);
		mos_usleep(50000);
		timeout -= 50;
	}

	return (MOS_ERROR(iop, EPHIDGET_TIMEOUT, "Timed out waiting for tag to appear after writing. Try again."));
}

void PhidgetRFIDSupport_free(PhidgetRFIDSupportHandle *arg) {

	if (arg == NULL || *arg == NULL)
		return;

	mos_free(*arg, sizeof(PhidgetRFIDSupport));
	*arg = NULL;
}

PhidgetReturnCode
PhidgetRFIDSupport_create(PhidgetRFIDSupportHandle *rfid) {

	TESTPTR_PR(rfid);
	*rfid = mos_zalloc(sizeof(PhidgetRFIDSupport));

	return (EPHIDGET_OK);
}

void PhidgetRFIDSupport_init(PhidgetRFIDSupportHandle rfid) {

	assert(rfid);
}

void PhidgetRFIDSupport_setExpectedTag(PhidgetRFID_Protocol protocol, const char *tagString, char *expectedTagString) {
	int oddParity = 0, evenParity = 1;
	uint64_t tagNumber = 0;
	uint32_t tagID;
	uint32_t tagFacilityID;
	int i, j;

	if (protocol == PROTOCOL_HID_GENERIC) {
		for (i = 0; (uint32_t)i < strlen(tagString); i++) {
			tagNumber <<= 4; // load the next hex-char at the end
			for (j = 0; j < 16; j++) {
				if (tagString[i] == PhidgetRFIDSupport_hexChar[j]) {
					tagNumber |= j;
					break;
				}
			}
		}

		for (i = 0; i <= 12; i++) {
			oddParity ^= (tagNumber >> i) & 1;
		}

		for (i = 13; i <= 25; i++) {
			evenParity ^= (tagNumber >> i) & 1;
		}

		// Check for H10301 pattern for additional decoding
		if (oddParity && evenParity && ((tagNumber & 0xFFFFC000000) == 0x02004000000)) {
			tagNumber >>= 1;
			tagID = tagNumber & 0xFFFF;
			tagFacilityID = (tagNumber >> 16) & 0xFF;

			expectedTagString[8] = 0; /* NUL trminator */
			// uint64_t decimalPlaceholder = 1;
			for (i = 7; i >= 3; i--) {
				expectedTagString[i] = (tagID % 10) + '0';
				tagID /= 10;
			}

			for (i = 2; i >= 0; i--) {
				expectedTagString[i] = (tagFacilityID % 10) + '0';
				tagFacilityID /= 10;
			}
			return;
		}
	}

	// copy tagString to expected string, including null terminator
	memcpy(expectedTagString, tagString, strlen(tagString) + 1);

	return;
}
