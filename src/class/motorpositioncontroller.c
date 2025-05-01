/* Generated: Thu Nov 24 2016 13:44:03 GMT-0700 (Mountain Standard Time) */
/* Will not be regenerated. */

#include "phidgetbase.h"
#include "class/motorpositioncontroller.gen.h"
#include "class/motorpositioncontroller.gen.c"

#define OLD_KP_VELOCITY_SCALER 2097151

int64_t PhidgetMotorPositionController_getLastPosition(PhidgetMotorPositionControllerHandle);

static void CCONV
PhidgetMotorPositionController_errorHandler(PhidgetChannelHandle phid, Phidget_ErrorEventCode code) {
	PhidgetMotorPositionControllerHandle ch;

	ch = (PhidgetMotorPositionControllerHandle)phid;

	switch (code) {
	case EEPHIDGET_MOTORSTALL:
		ch->engaged = 0;
		break;
	default:
		break;
	}
}

static void CCONV
PhidgetMotorPositionController_free(PhidgetChannelHandle *ch) {
	_free(ch);
}

API_PRETURN
PhidgetMotorPositionController_create(PhidgetMotorPositionControllerHandle *phidp) {
	return (_create(phidp));
}

static PhidgetReturnCode CCONV
PhidgetMotorPositionController_setStatus(PhidgetChannelHandle phid, BridgePacket *bp) {
	return (_setStatus(phid, bp));
}

static PhidgetReturnCode CCONV
PhidgetMotorPositionController_getStatus(PhidgetChannelHandle phid, BridgePacket **bp) {
	return (_getStatus(phid, bp));
}

static PhidgetReturnCode CCONV
PhidgetMotorPositionController_initAfterOpen(PhidgetChannelHandle phid) {
	return (_initAfterOpen(phid));
}

static PhidgetReturnCode CCONV
PhidgetMotorPositionController_setDefaults(PhidgetChannelHandle phid) {
	PhidgetReturnCode ret;

	ret = _setDefaults(phid);

	switch (phid->UCD->uid) {
#if (PHIDUID_DCC1010_SUPPORTED || PHIDUID_DCC1020_SUPPORTED || PHIDUID_DCC1030_SUPPORTED || PHIDUID_DCC1110_SUPPORTED || PHIDUID_DCC1120_SUPPORTED || PHIDUID_DCC1130_SUPPORTED)
	#if PHIDUID_DCC1010_SUPPORTED
	case PHIDCHUID_DCC1010_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1020_SUPPORTED
	case PHIDCHUID_DCC1020_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1030_SUPPORTED
	case PHIDCHUID_DCC1030_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1110_SUPPORTED
	case PHIDCHUID_DCC1110_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1120_SUPPORTED
	case PHIDCHUID_DCC1120_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1130_SUPPORTED
	case PHIDCHUID_DCC1130_MOTORPOSITIONCONTROLLER_100:
	#endif
		switch (ret) {
		case EPHIDGET_FAILSAFE:
			FIRE_ERROR(phid, EEPHIDGET_ESTOP, "External stop procedure initiated.");
			if (phid->iop != NULL) {
				mos_iop_addnotev(phid->iop, "ESTOP Button Pressed.");
			}
			break;
		case EPHIDGET_BADPOWER:
			FIRE_ERROR(phid, EEPHIDGET_BADPOWER, "Your power supply voltage is too high for the motor controller to begin operation.");
			if (phid->iop != NULL) {
				mos_iop_addnotev(phid->iop, "Bad Power Supply");
			}
			break;
		case EPHIDGET_POWERCYCLE:
			FIRE_ERROR(phid, EEPHIDGET_BADPOWER, "An overvoltage fault has triggered. Power your device off and on to resume operation. "
												 "We recommend a PowerGuard Phidget to prevent this in future.");
			if (phid->iop != NULL) {
				mos_iop_addnotev(phid->iop, "Overvoltage Fault. Power cycle required.");
			}
			break;
		case EPHIDGET_HALLSENSOR:
			FIRE_ERROR(phid, EEPHIDGET_BADCONNECTION, "The hall sensor of your BLDC motor is not reporting a valid input. "
													  "The cable is likely damaged or unplugged.");
			if (phid->iop != NULL) {
				mos_iop_addnotev(phid->iop, "Hall Sensor Error. Likely damaged or unplugged.");
			}
			break;
		case EPHIDGET_BADCURRENT:
			FIRE_ERROR(phid, EPHIDGET_BADCURRENT, "The current sensor is seeing an unacceptable offset in its readings. "
												  "Move the controller away from magnetic fields and try again.");
			if (phid->iop != NULL) {
				mos_iop_addnotev(phid->iop, "Current sensor error. Move away from magnets.");
			}
			break;
		default:
			break;
		}
		break;
#endif
	default:
		break;
	}

	return ret;
}

static PhidgetReturnCode
PhidgetMotorPositionController_bridgeInput(PhidgetChannelHandle phid, BridgePacket *bp) {
	PhidgetMotorPositionControllerHandle ch;
	PhidgetReturnCode res;
	double tmpDbl;

	ch = (PhidgetMotorPositionControllerHandle)phid;

	switch (bp->vpkt) {
	case BP_POSITIONCHANGE:
		ch->position = getBridgePacketInt64(bp, 0);
		FIRECH(ch, PhidgetMotorPositionController, PositionChange, (ch->position + ch->positionOffset) * ch->rescaleFactor);
		res = EPHIDGET_OK;
		break;
	case BP_SETFAILSAFETIME:
		TESTRANGE_IOP(bp->iop, "%u", getBridgePacketUInt32(bp, 0), ch->minFailsafeTime, ch->maxFailsafeTime);
		res = _bridgeInput(phid, bp);
		break;
	case BP_INDUCTANCECHANGE:
		ch->inductance = getBridgePacketDouble(bp, 0);
		FIRE_PROPERTYCHANGE(ch, "Inductance");
		res = EPHIDGET_OK;
		break;
	case BP_DATAINTERVALCHANGE:
		if (bp->entrycnt > 1)
			ch->dataInterval = getBridgePacketDouble(bp, 1);
		else
			ch->dataInterval = (double)getBridgePacketUInt32(bp, 0);
		FIRE_PROPERTYCHANGE(ch, "DataInterval");
		FIRE_PROPERTYCHANGE(ch, "DataRate");
		res = EPHIDGET_OK;
		break;
	case BP_CURRENTLIMITCHANGE:
		ch->currentLimit = getBridgePacketDouble(bp, 0);
		FIRE_PROPERTYCHANGE(ch, "CurrentLimit");
		res = EPHIDGET_OK;
		break;
	case BP_SURGECURRENTLIMITCHANGE:
		ch->surgeCurrentLimit = getBridgePacketDouble(bp, 0);
		FIRE_PROPERTYCHANGE(ch, "SurgeCurrentLimit");
		res = EPHIDGET_OK;
		break;
	case BP_ACTIVECURRENTLIMITCHANGE:
		tmpDbl = getBridgePacketDouble(bp, 0);
		if (ch->activeCurrentLimit != tmpDbl) {
			ch->activeCurrentLimit = tmpDbl;
			FIRE_PROPERTYCHANGE(ch, "ActiveCurrentLimit");
		}
		res = EPHIDGET_OK;
		break;
	case BP_EXPECTEDPOSITIONCHANGE:
		ch->expectedPosition = getBridgePacketInt64(bp, 0);
		FIRECH(ch, PhidgetMotorPositionController, ExpectedPositionChange, (ch->expectedPosition + ch->positionOffset) * ch->rescaleFactor);
		res = EPHIDGET_OK;
		break;
	default:
		res = _bridgeInput(phid, bp);
		break;
	}

	return (res);
}

static void CCONV
PhidgetMotorPositionController_fireInitialEvents(PhidgetChannelHandle phid) {
	_fireInitialEvents(phid);
}

static int
PhidgetMotorPositionController_hasInitialState(PhidgetChannelHandle phid) {

	if (supportedBridgePacket(phid, BP_INDUCTANCECHANGE) && ((PhidgetMotorPositionControllerHandle)phid)->inductance == PUNK_DBL)
		return (PFALSE);

	return (_hasInitialState(phid));
}

API_PRETURN
PhidgetMotorPositionController_addPositionOffset(PhidgetMotorPositionControllerHandle ch,
												 double positionOffset) {

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	ch->positionOffset += roundl(positionOffset / ch->rescaleFactor);
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_setAcceleration(PhidgetMotorPositionControllerHandle ch,
											   double acceleration) {

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);
	TESTRANGE_PR(acceleration, "%lf", (ch->minAcceleration * fabs(ch->rescaleFactor)), (ch->maxAcceleration * fabs(ch->rescaleFactor)));

	return (bridgeSendToDevice((PhidgetChannelHandle)ch, BP_SETACCELERATION, NULL, NULL, 1, "%g",
							   acceleration / fabs(ch->rescaleFactor)));
}

API_PRETURN
PhidgetMotorPositionController_getAcceleration(PhidgetMotorPositionControllerHandle ch,
											   double *acceleration) {

	TESTPTR_PR(ch);
	TESTPTR_PR(acceleration);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	*acceleration = ch->acceleration * fabs(ch->rescaleFactor);
	if (ch->acceleration == (double)PUNK_DBL)
		return (EPHIDGET_UNKNOWNVAL);
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_getMinAcceleration(PhidgetMotorPositionControllerHandle ch,
												  double *minAcceleration) {

	TESTPTR_PR(ch);
	TESTPTR_PR(minAcceleration);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	*minAcceleration = ch->minAcceleration * fabs(ch->rescaleFactor);
	if (ch->minAcceleration == (double)PUNK_DBL)
		return (EPHIDGET_UNKNOWNVAL);
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_getMaxAcceleration(PhidgetMotorPositionControllerHandle ch,
												  double *maxAcceleration) {

	TESTPTR_PR(ch);
	TESTPTR_PR(maxAcceleration);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	*maxAcceleration = ch->maxAcceleration * fabs(ch->rescaleFactor);
	if (ch->maxAcceleration == (double)PUNK_DBL)
		return (EPHIDGET_UNKNOWNVAL);
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_setDeadBand(PhidgetMotorPositionControllerHandle ch, double deadBand) {

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	return (bridgeSendToDevice((PhidgetChannelHandle)ch, BP_SETDEADBAND, NULL, NULL, 1, "%u",
							   roundu((deadBand / fabs(ch->rescaleFactor)))));
}

API_PRETURN
PhidgetMotorPositionController_getDeadBand(PhidgetMotorPositionControllerHandle ch, double *deadBand) {

	TESTPTR_PR(ch);
	TESTPTR_PR(deadBand);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	*deadBand = ch->deadBand * fabs(ch->rescaleFactor);
	if (ch->deadBand == PUNK_UINT32)
		return (EPHIDGET_UNKNOWNVAL);
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_getMaxVelocityLimit(PhidgetMotorPositionControllerHandle ch,
												   double *maxVelocityLimit) {

	TESTPTR_PR(ch);
	TESTPTR_PR(maxVelocityLimit);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	*maxVelocityLimit = ch->maxVelocityLimit * fabs(ch->rescaleFactor);
	if (ch->maxVelocityLimit == (double)PUNK_DBL)
		return (EPHIDGET_UNKNOWNVAL);
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_getPosition(PhidgetMotorPositionControllerHandle ch, double *position) {

	TESTPTR_PR(ch);
	TESTPTR_PR(position);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	*position = (ch->position + ch->positionOffset) * ch->rescaleFactor;
	if (ch->position == PUNK_INT64)
		return (EPHIDGET_UNKNOWNVAL);
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_getMinPosition(PhidgetMotorPositionControllerHandle ch,
											  double *minPosition) {

	TESTPTR_PR(ch);
	TESTPTR_PR(minPosition);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	*minPosition = (ch->minPosition + ch->positionOffset) * fabs(ch->rescaleFactor);
	if (ch->minPosition == PUNK_INT64)
		return (EPHIDGET_UNKNOWNVAL);
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_getMaxPosition(PhidgetMotorPositionControllerHandle ch,
											  double *maxPosition) {

	TESTPTR_PR(ch);
	TESTPTR_PR(maxPosition);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	*maxPosition = (ch->maxPosition + ch->positionOffset) * fabs(ch->rescaleFactor);
	if (ch->maxPosition == PUNK_INT64)
		return (EPHIDGET_UNKNOWNVAL);
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_setRescaleFactor(PhidgetMotorPositionControllerHandle ch,
												double rescaleFactor) {

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	if (rescaleFactor == 0)
		return EPHIDGET_INVALIDARG;

	ch->rescaleFactor = rescaleFactor;
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_getMinVelocityLimit(PhidgetMotorPositionControllerHandle ch,
												   double *minVelocityLimit) {

	TESTPTR_PR(ch);
	TESTPTR_PR(minVelocityLimit);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	*minVelocityLimit = ch->minVelocityLimit;
	if (ch->minVelocityLimit == (double)PUNK_DBL)
		return (EPHIDGET_UNKNOWNVAL);
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_setTargetPosition(PhidgetMotorPositionControllerHandle ch,
												 double targetPosition) {

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	return (bridgeSendToDevice((PhidgetChannelHandle)ch, BP_SETTARGETPOSITION, NULL, NULL, 1, "%l",
							   roundl((targetPosition / ch->rescaleFactor) - ch->positionOffset)));
}

API_VRETURN
PhidgetMotorPositionController_setTargetPosition_async(PhidgetMotorPositionControllerHandle ch,
													   double targetPosition, Phidget_AsyncCallback fptr, void *ctx) {
	PhidgetReturnCode res;

	if (ch == NULL) {
		if (fptr)
			fptr((PhidgetHandle)ch, ctx, EPHIDGET_INVALIDARG);
		return;
	}
	if (ch->phid.class != PHIDCHCLASS_MOTORPOSITIONCONTROLLER) {
		if (fptr)
			fptr((PhidgetHandle)ch, ctx, EPHIDGET_WRONGDEVICE);
		return;
	}
	if (!ISATTACHED(ch)) {
		if (fptr)
			fptr((PhidgetHandle)ch, ctx, EPHIDGET_NOTATTACHED);
		return;
	}

	res = bridgeSendToDevice((PhidgetChannelHandle)ch, BP_SETTARGETPOSITION, fptr, ctx, 1, "%l", roundl((targetPosition / ch->rescaleFactor) - ch->positionOffset));
	if (res != EPHIDGET_OK && fptr != NULL)
		fptr((PhidgetHandle)ch, ctx, res);
}

API_PRETURN
PhidgetMotorPositionController_getTargetPosition(PhidgetMotorPositionControllerHandle ch,
												 double *targetPosition) {

	TESTPTR_PR(ch);
	TESTPTR_PR(targetPosition);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	*targetPosition = (ch->targetPosition + ch->positionOffset) * ch->rescaleFactor;
	if (ch->targetPosition == PUNK_INT64)
		return (EPHIDGET_UNKNOWNVAL);
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_getExpectedPosition(PhidgetMotorPositionControllerHandle ch,
												   double *expectedPosition) {

	TESTPTR_PR(ch);
	TESTPTR_PR(expectedPosition);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	*expectedPosition = (ch->expectedPosition + ch->positionOffset) * ch->rescaleFactor;
	if (ch->expectedPosition == PUNK_INT64)
		return (EPHIDGET_UNKNOWNVAL);
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_setNormalizePID(PhidgetMotorPositionControllerHandle ch, int normalize) {

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);
	ch->normalizePID = (normalize != 0) ? PTRUE : PFALSE;
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_setVelocityLimit(PhidgetMotorPositionControllerHandle ch, double velocityLimit) {

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);
	TESTRANGE_PR(velocityLimit, "%lf", (ch->minVelocityLimit * fabs(ch->rescaleFactor)), (ch->maxVelocityLimit * fabs(ch->rescaleFactor)));

	return (bridgeSendToDevice((PhidgetChannelHandle)ch, BP_SETDUTYCYCLE, NULL, NULL, 1, "%g",
							   velocityLimit / fabs(ch->rescaleFactor)));
}

API_PRETURN
PhidgetMotorPositionController_getVelocityLimit(PhidgetMotorPositionControllerHandle ch, double *velocityLimit) {

	TESTPTR_PR(ch);
	TESTPTR_PR(velocityLimit);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	*velocityLimit = ch->velocityLimit * fabs(ch->rescaleFactor);
	if (ch->velocityLimit == (double)PUNK_DBL)
		return (EPHIDGET_UNKNOWNVAL);
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_setStallVelocity(PhidgetMotorPositionControllerHandle ch,
												double stallVelocity) {

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);
	TESTRANGE_PR(stallVelocity, "%lf", (ch->minStallVelocity * fabs(ch->rescaleFactor)), (ch->maxStallVelocity * fabs(ch->rescaleFactor)));

	return (bridgeSendToDevice((PhidgetChannelHandle)ch, BP_SETSTALLVELOCITY, NULL, NULL, 1, "%g",
							   stallVelocity / fabs(ch->rescaleFactor)));
}

API_PRETURN
PhidgetMotorPositionController_getStallVelocity(PhidgetMotorPositionControllerHandle ch, double *stallVelocity) {

	TESTPTR_PR(ch);
	TESTPTR_PR(stallVelocity);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	*stallVelocity = ch->stallVelocity * fabs(ch->rescaleFactor);
	if (ch->stallVelocity == (double)PUNK_DBL)
		return (EPHIDGET_UNKNOWNVAL);
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_getMinStallVelocity(PhidgetMotorPositionControllerHandle ch, double *minStallVelocity) {

	TESTPTR_PR(ch);
	TESTPTR_PR(minStallVelocity);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	*minStallVelocity = ch->minStallVelocity * fabs(ch->rescaleFactor);
	if (ch->minStallVelocity == (double)PUNK_DBL)
		return (EPHIDGET_UNKNOWNVAL);
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_getMaxStallVelocity(PhidgetMotorPositionControllerHandle ch, double *maxStallVelocity) {

	TESTPTR_PR(ch);
	TESTPTR_PR(maxStallVelocity);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	*maxStallVelocity = ch->maxStallVelocity * fabs(ch->rescaleFactor);
	if (ch->maxStallVelocity == (double)PUNK_DBL)
		return (EPHIDGET_UNKNOWNVAL);
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_setKp(PhidgetMotorPositionControllerHandle ch, double Kp) {

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	switch (ch->phid.UCD->uid) {
#if (PHIDUID_DCC1010_SUPPORTED || PHIDUID_DCC1020_SUPPORTED || PHIDUID_DCC1030_SUPPORTED || PHIDUID_DCC1110_SUPPORTED || PHIDUID_DCC1120_SUPPORTED || PHIDUID_DCC1130_SUPPORTED)
	#if PHIDUID_DCC1010_SUPPORTED
	case PHIDCHUID_DCC1010_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1020_SUPPORTED
	case PHIDCHUID_DCC1020_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1030_SUPPORTED
	case PHIDCHUID_DCC1030_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1110_SUPPORTED
	case PHIDCHUID_DCC1110_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1120_SUPPORTED
	case PHIDCHUID_DCC1120_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1130_SUPPORTED
	case PHIDCHUID_DCC1130_MOTORPOSITIONCONTROLLER_100:
	#endif
		return (bridgeSendToDevice((PhidgetChannelHandle)ch, BP_SETKP, NULL, NULL, 1, "%g", Kp * fabs(ch->rescaleFactor)));
#endif
	default:
		if (ch->normalizePID == PTRUE) {
			Kp *= OLD_KP_VELOCITY_SCALER;
			Kp *= fabs(ch->rescaleFactor);
		}
		return (bridgeSendToDevice((PhidgetChannelHandle)ch, BP_SETKP, NULL, NULL, 1, "%g", Kp));
	}
}

API_PRETURN
PhidgetMotorPositionController_getKp(PhidgetMotorPositionControllerHandle ch,
									 double *Kp) {

	TESTPTR_PR(ch);
	TESTPTR_PR(Kp);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	switch (ch->phid.UCD->uid) {
#if (PHIDUID_DCC1010_SUPPORTED || PHIDUID_DCC1020_SUPPORTED || PHIDUID_DCC1030_SUPPORTED || PHIDUID_DCC1110_SUPPORTED || PHIDUID_DCC1120_SUPPORTED || PHIDUID_DCC1130_SUPPORTED)
	#if PHIDUID_DCC1010_SUPPORTED
	case PHIDCHUID_DCC1010_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1020_SUPPORTED
	case PHIDCHUID_DCC1020_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1030_SUPPORTED
	case PHIDCHUID_DCC1030_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1110_SUPPORTED
	case PHIDCHUID_DCC1110_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1120_SUPPORTED
	case PHIDCHUID_DCC1120_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1130_SUPPORTED
	case PHIDCHUID_DCC1130_MOTORPOSITIONCONTROLLER_100:
	#endif
		*Kp = ch->kp / fabs(ch->rescaleFactor);
		break;
#endif
	default:
		*Kp = ch->kp;
		if (ch->normalizePID == PTRUE) {
			*Kp /= OLD_KP_VELOCITY_SCALER;
			*Kp /= fabs(ch->rescaleFactor);
		}
		break;
	}

	if (ch->kp == (double)PUNK_DBL)
		return (EPHIDGET_UNKNOWNVAL);
	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_setKi(PhidgetMotorPositionControllerHandle ch,
									 double Ki) {

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	switch (ch->phid.UCD->uid) {
#if (PHIDUID_DCC1010_SUPPORTED || PHIDUID_DCC1020_SUPPORTED || PHIDUID_DCC1030_SUPPORTED || PHIDUID_DCC1110_SUPPORTED || PHIDUID_DCC1120_SUPPORTED || PHIDUID_DCC1130_SUPPORTED)
	#if PHIDUID_DCC1010_SUPPORTED
	case PHIDCHUID_DCC1010_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1020_SUPPORTED
	case PHIDCHUID_DCC1020_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1030_SUPPORTED
	case PHIDCHUID_DCC1030_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1110_SUPPORTED
	case PHIDCHUID_DCC1110_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1120_SUPPORTED
	case PHIDCHUID_DCC1120_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1130_SUPPORTED
	case PHIDCHUID_DCC1130_MOTORPOSITIONCONTROLLER_100:
	#endif
		return (bridgeSendToDevice((PhidgetChannelHandle)ch, BP_SETKI, NULL, NULL, 1, "%g",
								   Ki * fabs(ch->rescaleFactor)));
#endif
	default:
		if (ch->normalizePID == PTRUE) {
			Ki *= (OLD_KP_VELOCITY_SCALER / 1000.0);
			Ki *= fabs(ch->rescaleFactor);
		}
		return (bridgeSendToDevice((PhidgetChannelHandle)ch, BP_SETKI, NULL, NULL, 1, "%g", Ki));
	}
}

API_PRETURN
PhidgetMotorPositionController_getKi(PhidgetMotorPositionControllerHandle ch,
									 double *Ki) {

	TESTPTR_PR(ch);
	TESTPTR_PR(Ki);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	switch (ch->phid.UCD->uid) {
#if (PHIDUID_DCC1010_SUPPORTED || PHIDUID_DCC1020_SUPPORTED || PHIDUID_DCC1030_SUPPORTED || PHIDUID_DCC1110_SUPPORTED || PHIDUID_DCC1120_SUPPORTED || PHIDUID_DCC1130_SUPPORTED)
	#if PHIDUID_DCC1010_SUPPORTED
	case PHIDCHUID_DCC1010_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1020_SUPPORTED
	case PHIDCHUID_DCC1020_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1030_SUPPORTED
	case PHIDCHUID_DCC1030_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1110_SUPPORTED
	case PHIDCHUID_DCC1110_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1120_SUPPORTED
	case PHIDCHUID_DCC1120_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1130_SUPPORTED
	case PHIDCHUID_DCC1130_MOTORPOSITIONCONTROLLER_100:
	#endif
		*Ki = ch->ki / fabs(ch->rescaleFactor);
		break;
#endif
	default:
		*Ki = ch->ki;
		if (ch->normalizePID == PTRUE) {
			*Ki /= (OLD_KP_VELOCITY_SCALER / 1000.0);
			*Ki /= fabs(ch->rescaleFactor);
		}
		break;
	}

	if (ch->ki == (double)PUNK_DBL)
		return (EPHIDGET_UNKNOWNVAL);

	return (EPHIDGET_OK);
}

API_PRETURN
PhidgetMotorPositionController_setKd(PhidgetMotorPositionControllerHandle ch,
									 double Kd) {

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	switch (ch->phid.UCD->uid) {
#if (PHIDUID_DCC1010_SUPPORTED || PHIDUID_DCC1020_SUPPORTED || PHIDUID_DCC1030_SUPPORTED || PHIDUID_DCC1110_SUPPORTED || PHIDUID_DCC1120_SUPPORTED || PHIDUID_DCC1130_SUPPORTED)
	#if PHIDUID_DCC1010_SUPPORTED
	case PHIDCHUID_DCC1010_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1020_SUPPORTED
	case PHIDCHUID_DCC1020_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1030_SUPPORTED
	case PHIDCHUID_DCC1030_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1110_SUPPORTED
	case PHIDCHUID_DCC1110_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1120_SUPPORTED
	case PHIDCHUID_DCC1120_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1130_SUPPORTED
	case PHIDCHUID_DCC1130_MOTORPOSITIONCONTROLLER_100:
	#endif
		return (bridgeSendToDevice((PhidgetChannelHandle)ch, BP_SETKD, NULL, NULL, 1, "%g",
								   Kd * fabs(ch->rescaleFactor)));
#endif
	default:
		if (ch->normalizePID == PTRUE) {
			Kd *= (OLD_KP_VELOCITY_SCALER * 1000.0);
			Kd *= fabs(ch->rescaleFactor);
		}
		return (bridgeSendToDevice((PhidgetChannelHandle)ch, BP_SETKD, NULL, NULL, 1, "%g", Kd));
	}
}

API_PRETURN
PhidgetMotorPositionController_getKd(PhidgetMotorPositionControllerHandle ch,
									 double *Kd) {

	TESTPTR_PR(ch);
	TESTPTR_PR(Kd);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	switch (ch->phid.UCD->uid) {
#if (PHIDUID_DCC1010_SUPPORTED || PHIDUID_DCC1020_SUPPORTED || PHIDUID_DCC1030_SUPPORTED || PHIDUID_DCC1110_SUPPORTED || PHIDUID_DCC1120_SUPPORTED || PHIDUID_DCC1130_SUPPORTED)
	#if PHIDUID_DCC1010_SUPPORTED
	case PHIDCHUID_DCC1010_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1020_SUPPORTED
	case PHIDCHUID_DCC1020_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1030_SUPPORTED
	case PHIDCHUID_DCC1030_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1110_SUPPORTED
	case PHIDCHUID_DCC1110_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1120_SUPPORTED
	case PHIDCHUID_DCC1120_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1130_SUPPORTED
	case PHIDCHUID_DCC1130_MOTORPOSITIONCONTROLLER_100:
	#endif
		*Kd = ch->kd / fabs(ch->rescaleFactor);
		break;
#endif
	default:
		*Kd = ch->kd;
		if (ch->normalizePID == PTRUE) {
			*Kd /= (OLD_KP_VELOCITY_SCALER * 1000.0);
			*Kd /= fabs(ch->rescaleFactor);
		}
		break;
	}

	if (ch->kd == (double)PUNK_DBL)
		return (EPHIDGET_UNKNOWNVAL);

	return (EPHIDGET_OK);
}

int64_t
PhidgetMotorPositionController_getLastPosition(PhidgetMotorPositionControllerHandle ch) {
	return (ch->position);
}

API_PRETURN
PhidgetMotorPositionController_setPositionType(PhidgetMotorPositionControllerHandle ch,
											   Phidget_PositionType positionType) {
	PhidgetReturnCode ret;

	TESTPTR_PR(ch);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	ret = (bridgeSendToDevice((PhidgetChannelHandle)ch, BP_POSITIONTYPE, NULL, NULL, 1, "%d",
							  positionType));

	if (ret != EPHIDGET_OK)
		return ret;

	switch (ch->phid.UCD->uid) {
#if (PHIDUID_DCC1110_SUPPORTED || PHIDUID_DCC1120_SUPPORTED || PHIDUID_DCC1130_SUPPORTED)
	#if PHIDUID_DCC1110_SUPPORTED
	case PHIDCHUID_DCC1110_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1120_SUPPORTED
	case PHIDCHUID_DCC1120_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1130_SUPPORTED
	case PHIDCHUID_DCC1130_MOTORPOSITIONCONTROLLER_100:
	#endif
		switch (positionType) {
		case POSITION_TYPE_ENCODER:
			ch->maxVelocityLimit = 500000;
			break;
		case POSITION_TYPE_HALL_SENSOR:
			ch->maxVelocityLimit = 2000;
			break;
		default:
			break;
		}
#endif
	default:
		break;
	}
	return ret;
}

API_PRETURN
PhidgetMotorPositionController_getPositionType(PhidgetMotorPositionControllerHandle ch,
											   Phidget_PositionType *positionType) {

	TESTPTR_PR(ch);
	TESTPTR_PR(positionType);
	TESTCHANNELCLASS_PR(ch, PHIDCHCLASS_MOTORPOSITIONCONTROLLER);
	TESTATTACHED_PR(ch);

	switch (ch->phid.UCD->uid) {
#if (PHIDUID_DCC1110_SUPPORTED || PHIDUID_DCC1120_SUPPORTED || PHIDUID_DCC1130_SUPPORTED)
	#if PHIDUID_DCC1110_SUPPORTED
	case PHIDCHUID_DCC1110_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1120_SUPPORTED
	case PHIDCHUID_DCC1120_MOTORPOSITIONCONTROLLER_100:
	#endif
	#if PHIDUID_DCC1130_SUPPORTED
	case PHIDCHUID_DCC1130_MOTORPOSITIONCONTROLLER_100:
	#endif
		break;
#endif
	default:
		return (PHID_RETURN(EPHIDGET_UNSUPPORTED));
	}

	*positionType = ch->positionType;
	if (ch->positionType == (Phidget_PositionType)PUNK_ENUM)
		return (PHID_RETURN(EPHIDGET_UNKNOWNVAL));
	return (EPHIDGET_OK);
}
