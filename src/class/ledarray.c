/* Generated: Mon Dec 09 2024 15:18:07 GMT-0700 (Mountain Standard Time) */
/* Will not be regenerated. */

#include "phidgetbase.h"
#include "util/ledarraysupport.h"
#include "class/ledarray.gen.h"
#include "class/ledarray.gen.c"

static void CCONV
PhidgetLEDArray_errorHandler(PhidgetChannelHandle phid, Phidget_ErrorEventCode code) {
	PhidgetLEDArrayHandle ch = ((PhidgetLEDArrayHandle)phid);
	switch (code) {
	case EEPHIDGET_OVERCURRENT:
		ch->powerEnabled = 0;
		break;
	}
}

static void CCONV
PhidgetLEDArray_free(PhidgetChannelHandle *ch) {

	if (ch && *ch)
		PhidgetLEDArraySupport_free((PhidgetLEDArraySupportHandle *)&(*ch)->private);
	_free(ch);
}

API_PRETURN
PhidgetLEDArray_create(PhidgetLEDArrayHandle *phidp) {
	PhidgetReturnCode res;

	res = _create(phidp);
	if (res == EPHIDGET_OK)
		res = PhidgetLEDArraySupport_create((PhidgetLEDArraySupportHandle *)&(*phidp)->phid.private);

	return (res);
}

static PhidgetReturnCode CCONV
PhidgetLEDArray_setStatus(PhidgetChannelHandle phid, BridgePacket *bp) {
	return (_setStatus(phid, bp));
}

static PhidgetReturnCode CCONV
PhidgetLEDArray_getStatus(PhidgetChannelHandle phid, BridgePacket **bp) {
	return (_getStatus(phid, bp));
}

static PhidgetReturnCode CCONV
PhidgetLEDArray_initAfterOpen(PhidgetChannelHandle phid) {
	if (phid)
		PhidgetLEDArraySupport_init((PhidgetLEDArraySupportHandle)phid->private);
	return (_initAfterOpen(phid));
}

static PhidgetReturnCode CCONV
PhidgetLEDArray_setDefaults(PhidgetChannelHandle phid) {
	return (_setDefaults(phid));
}

static PhidgetReturnCode
PhidgetLEDArray_bridgeInput(PhidgetChannelHandle phid, BridgePacket *bp) {
	PhidgetReturnCode res;
	PhidgetLEDArrayHandle ch = ((PhidgetLEDArrayHandle)phid);

	switch (bp->vpkt) {
	case BP_SETPROTOCOL:
		res = _bridgeInput(phid, bp);
		if (res != EPHIDGET_OK)
			return res;
		switch (getBridgePacketInt32(bp, 0)) {
		case LED_PROTOCOL_RGB:
		case LED_PROTOCOL_GRB:
			ch->maxLEDCount = 2048;
			ch->maxAnimationPatternCount = 128;
			break;
		case LED_PROTOCOL_RGBW:
		case LED_PROTOCOL_GRBW:
			ch->maxLEDCount = 1536;
			ch->maxAnimationPatternCount = 96;
			break;
		}
		break;
	default:
		res = _bridgeInput(phid, bp);
		break;
	}

	return (res);
}

static void CCONV
PhidgetLEDArray_fireInitialEvents(PhidgetChannelHandle phid) {
	_fireInitialEvents(phid);
}

static int CCONV
PhidgetLEDArray_hasInitialState(PhidgetChannelHandle phid) {
	return (_hasInitialState(phid));
}

API_PRETURN
PhidgetLEDArray_setLEDs(PhidgetLEDArrayHandle ch, uint32_t offset, const PhidgetLEDArray_RGBW *leds, size_t ledCount, uint32_t fadeTime) {
	uint8_t buffer[BPE_MAXARRAY_LEN];
	size_t i;

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_LEDARRAY);
	TESTATTACHED_PR(ch);

	TESTRANGE_PR(fadeTime, "%u", ch->minFadeTime, ch->maxFadeTime);

	if (ledCount+offset > ch->maxLEDCount)
		return (PHID_RETURN_ERRSTR(EPHIDGET_INVALIDARG, "Too many LEDs specified"));

	if (ledCount > BPE_MAXARRAY_LEN / 4)
		return (PHID_RETURN_ERRSTR(EPHIDGET_INVALIDARG, "Too many LEDs specified"));

	for (i = 0; i < ledCount; i++) {
		buffer[(i * 4) + 0] = leds[i].r;
		buffer[(i * 4) + 1] = leds[i].g;
		buffer[(i * 4) + 2] = leds[i].b;
		buffer[(i * 4) + 3] = leds[i].w;
	}

	return bridgeSendToDevice((PhidgetChannelHandle)ch, BP_DATAOUT, NULL, NULL, 3, "%u%u%*R", offset, fadeTime, (ledCount * 4), buffer);
}

API_VRETURN_HDR
PhidgetLEDArray_setLEDs_async(PhidgetLEDArrayHandle ch, uint32_t offset, const PhidgetLEDArray_RGBW *leds, size_t ledCount, uint32_t fadeTime,
										Phidget_AsyncCallback fptr, void *ctx) {
	uint8_t buffer[BPE_MAXARRAY_LEN];
	PhidgetReturnCode res;
	uint32_t i;

	if (ch == NULL) {
		if (fptr)
			fptr((PhidgetHandle)ch, ctx, EPHIDGET_INVALIDARG);
		return;
	}
	if (ch->phid.class != PHIDCHCLASS_LEDARRAY) {
		if (fptr)
			fptr((PhidgetHandle)ch, ctx, EPHIDGET_WRONGDEVICE);
		return;
	}
	if (!ISATTACHED(ch)) {
		if (fptr)
			fptr((PhidgetHandle)ch, ctx, EPHIDGET_NOTATTACHED);
		return;
	}

	if ((fadeTime < ch->minFadeTime) ||  (fadeTime > ch->maxFadeTime)) {
		if (fptr)
			fptr((PhidgetHandle)ch, ctx, EPHIDGET_INVALIDARG);
		return;
	}

	if (ledCount+offset > ch->maxLEDCount) {
		if (fptr)
			fptr((PhidgetHandle)ch, ctx, EPHIDGET_INVALIDARG);
		return;
	}

	if (ledCount > BPE_MAXARRAY_LEN / 4) {
		if (fptr)
			fptr((PhidgetHandle)ch, ctx, EPHIDGET_INVALIDARG);
		return;
	}

	for (i = 0; i < ledCount; i++) {
		buffer[(i * 4) + 0] = leds[i].r;
		buffer[(i * 4) + 1] = leds[i].g;
		buffer[(i * 4) + 2] = leds[i].b;
		buffer[(i * 4) + 3] = leds[i].w;
	}

	res = bridgeSendToDevice((PhidgetChannelHandle)ch, BP_DATAOUT, fptr, ctx, 3, "%u%u%*R", offset, fadeTime, (ledCount * 4), buffer);
	if (res != EPHIDGET_OK && fptr != NULL)
		fptr((PhidgetHandle)ch, ctx, res);
}

API_PRETURN
PhidgetLEDArray_setAnimation(PhidgetLEDArrayHandle ch, int32_t animationID, const PhidgetLEDArray_RGBW *pattern, size_t patternCount, PhidgetLEDArray_AnimationDescription *animation) {

	size_t i;

	TESTPTR_PR(ch);
	TESTPTR_PR(animation);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_LEDARRAY);
	TESTATTACHED_PR(ch);

	if (patternCount > ch->maxAnimationPatternCount)
		return (PHID_RETURN_ERRSTR(EPHIDGET_INVALIDARG, "Too many RGBW values specified"));

	if (patternCount > BPE_MAXARRAY_LEN / 4)
		return (PHID_RETURN_ERRSTR(EPHIDGET_INVALIDARG, "Too many RGBW values specified"));

	if (animation->startAddress > ch->maxLEDCount)
		return (PHID_RETURN_ERRSTR(EPHIDGET_INVALIDARG, "Animation start address is too high"));
	if ((uint32_t)(animation->startAddress + animation->span) > ch->maxLEDCount)
		return (PHID_RETURN_ERRSTR(EPHIDGET_INVALIDARG, "Animation does not fit within the LED string"));
	
	TESTRANGE_PR(animationID, "%d", ch->minAnimationID, ch->maxAnimationID);
	TESTRANGE_PR((int32_t)animation->animationType, "%d", ANIMATION_TYPE_FORWARD_SCROLL, ANIMATION_TYPE_REVERSE_SCROLL_MIRROR);
	TESTRANGE_PR(animation->time, "%d", ch->minFadeTime, ch->maxFadeTime);

	uint8_t buffer[BPE_MAXARRAY_LEN];
	for (i = 0; i < patternCount; i++) {
		buffer[(i * 4) + 0] = pattern[i].r;
		buffer[(i * 4) + 1] = pattern[i].g;
		buffer[(i * 4) + 2] = pattern[i].b;
		buffer[(i * 4) + 3] = pattern[i].w;
	}

	return bridgeSendToDevice((PhidgetChannelHandle)ch, BP_SETANIMATION, NULL, NULL, 6, "%d%d%d%d%d%*R", animationID, animation->startAddress,
							  animation->span, animation->time, animation->animationType, (patternCount * 4), buffer);
}

