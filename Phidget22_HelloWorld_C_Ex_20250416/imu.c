#include <phidget22.h>
#include <stdio.h>

static void CCONV onAngularRateUpdate(PhidgetGyroscopeHandle ch, void * ctx, const double angularRate[3], double timestamp) {
	double heading_rad;
	double heading_degree;
	printf("AngularRate: \t%lf  |  %lf  |  %lf\n", angularRate[0], angularRate[1], angularRate[2]);
	printf("Timestamp: %lf\n", timestamp);
	printf("----------\n");
	heading_rad = atan2(2.0 * (ch->quaternion.w * ch->quaternion.z + ch->quaternion.x * ch->quaternion.y), 1 - 2.0 * (ch->quaternion.y * ch->quaternion.y + ch->quaternion.z * ch->quaternion.z));
	heading_degree = heading_rad * 180.0 / 3.14159265358979;
	printf('Heand: \t%lf', heading_degree);

}

static void CCONV onSpatialData(PhidgetSpatialHandle ch, void * ctx, const double acceleration[3], const double angularRate[3], const double magneticField[3], double timestamp) {
	printf("Acceleration: \t%lf  |  %lf  |  %lf\n", acceleration[0], acceleration[1], acceleration[2]);
	printf("AngularRate: \t%lf  |  %lf  |  %lf\n", angularRate[0], angularRate[1], angularRate[2]);
	printf("MagneticField: \t%lf  |  %lf  |  %lf\n", magneticField[0], magneticField[1], magneticField[2]);
	printf("Timestamp: %lf\n", timestamp);
	printf("----------\n");
}

int main() {
	PhidgetGyroscopeHandle gyroscope0;
	PhidgetSpatialHandle spatial0;

	PhidgetGyroscope_create(&gyroscope0);
	PhidgetSpatial_create(&spatial0);

	PhidgetGyroscope_setOnAngularRateUpdateHandler(gyroscope0, onAngularRateUpdate, NULL);
	PhidgetSpatial_setOnSpatialDataHandler(spatial0, onSpatialData, NULL);

	Phidget_openWaitForAttachment((PhidgetHandle)gyroscope0, 5000);
	Phidget_openWaitForAttachment((PhidgetHandle)spatial0, 5000);

	//Wait until Enter has been pressed before exiting
	getchar();

	Phidget_close((PhidgetHandle)gyroscope0);
	Phidget_close((PhidgetHandle)spatial0);

	PhidgetGyroscope_delete(&gyroscope0);
	PhidgetSpatial_delete(&spatial0);
}

