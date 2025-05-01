/* Generated: Fri Feb 05 2016 11:41:33 GMT-0700 (Mountain Standard Time) */
/* Will not be regenerated. */

#include "phidgetbase.h"
#include "class/powerguard.gen.h"
#include "class/powerguard.gen.c"

static void CCONV
PhidgetPowerGuard_errorHandler(PhidgetChannelHandle phid, Phidget_ErrorEventCode code) {
}

static void CCONV
PhidgetPowerGuard_free(PhidgetChannelHandle *ch) {
	_free(ch);
}

API_PRETURN
PhidgetPowerGuard_create(PhidgetPowerGuardHandle *phidp) {
	return (_create(phidp));
}

static PhidgetReturnCode CCONV
PhidgetPowerGuard_setStatus(PhidgetChannelHandle phid, BridgePacket *bp) {
	return (_setStatus(phid, bp));
}

static PhidgetReturnCode CCONV
PhidgetPowerGuard_getStatus(PhidgetChannelHandle phid, BridgePacket **bp) {
	return (_getStatus(phid, bp));
}

static PhidgetReturnCode CCONV
PhidgetPowerGuard_initAfterOpen(PhidgetChannelHandle phid) {
	return (_initAfterOpen(phid));
}

static PhidgetReturnCode CCONV
PhidgetPowerGuard_setDefaults(PhidgetChannelHandle phid) {
	return (_setDefaults(phid));
}

static PhidgetReturnCode
PhidgetPowerGuard_bridgeInput(PhidgetChannelHandle phid, BridgePacket *bp) {
	PhidgetPowerGuardHandle ch;
	PhidgetReturnCode res;
	
	ch = (PhidgetPowerGuardHandle)phid;
	switch (bp->vpkt) {
	case BP_SETFAILSAFETIME:
		TESTRANGE_IOP(bp->iop, "%u", getBridgePacketUInt32(bp, 0), ch->minFailsafeTime, ch->maxFailsafeTime);
		res = _bridgeInput(phid, bp);
		break;
	default:
		res = _bridgeInput(phid, bp);
		break;
	}

	return res;
}

static void
PhidgetPowerGuard_fireInitialEvents(PhidgetChannelHandle phid) {
	_fireInitialEvents(phid);
}

static int
PhidgetPowerGuard_hasInitialState(PhidgetChannelHandle phid) {
	return (_hasInitialState(phid));
}
