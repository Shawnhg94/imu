#include <phidget22.h>
#include <stdio.h>

double ts = 0.0;
double heading = 0.0;

static void CCONV onAngularRateUpdate(PhidgetGyroscopeHandle ch, void * ctx, const double angularRate[3], double timestamp) {
	printf("AngularRate: \t%lf  |  %lf  |  %lf\n", angularRate[0], angularRate[1], angularRate[2]);
	printf("Timestamp: %lf\n", timestamp);
	printf("----------\n");


}

double normalize_angle(double angle_degrees) {
    // Using fmodf (requires math.h)
    // angle_degrees = fmodf(angle_degrees, 360.0f);
    // if (angle_degrees < 0.0f) {
    //     angle_degrees += 360.0f;
    // }
    // return angle_degrees;

    // Alternative using while loops (often better for embedded without FPU)
    while (angle_degrees >= 360.0) {
        angle_degrees -= 360.0;
    }
    while (angle_degrees < 0.0) {
        angle_degrees += 360.0;
    }
    return angle_degrees;
}

static void CCONV onSpatialData(PhidgetSpatialHandle ch, void * ctx, const double acceleration[3], const double angularRate[3], const double magneticField[3], double timestamp) {
	double dt_sec = 0.0;
	printf("Acceleration: \t%lf  |  %lf  |  %lf\n", acceleration[0], acceleration[1], acceleration[2]);
	printf("AngularRate: \t%lf  |  %lf  |  %lf\n", angularRate[0], angularRate[1], angularRate[2]);
	printf("MagneticField: \t%lf  |  %lf  |  %lf\n", magneticField[0], magneticField[1], magneticField[2]);
	printf("Timestamp: %lf\n", timestamp);
	//printf("----------\n");

	if (ts == 0.0) {
		ts = timestamp;
		return;
	}

	dt_sec = (timestamp - ts) / 1000.0;
	heading = heading + angularRate[2] * dt_sec;
	heading = heading * 180 / 3.14;
	heading = normalize_angle(heading);
	printf("Heading: %lf\n", heading);
	printf("----------\n");

}

void zeroGyro(PhidgetSpatialHandle ch) {
	printf("PhidgetSpatial_zeroGyronw\n");
	PhidgetSpatial_zeroGyro(ch);
}

int main() {
	PhidgetGyroscopeHandle gyroscope0;
	PhidgetSpatialHandle spatial0;
	char ch = 0;

	PhidgetGyroscope_create(&gyroscope0);
	PhidgetSpatial_create(&spatial0);

	PhidgetGyroscope_setOnAngularRateUpdateHandler(gyroscope0, onAngularRateUpdate, NULL);
	PhidgetSpatial_setOnSpatialDataHandler(spatial0, onSpatialData, NULL);

	Phidget_openWaitForAttachment((PhidgetHandle)gyroscope0, 5000);
	Phidget_openWaitForAttachment((PhidgetHandle)spatial0, 5000);

	PhidgetSpatial_zeroGyro(spatial0);

	//Wait until Enter has been pressed before exiting
	while(1) {
		ch = getchar();
		if (ch == 'e') {
			break;
		}
		
	}

	Phidget_close((PhidgetHandle)gyroscope0);
	Phidget_close((PhidgetHandle)spatial0);

	PhidgetGyroscope_delete(&gyroscope0);
	PhidgetSpatial_delete(&spatial0);
}

