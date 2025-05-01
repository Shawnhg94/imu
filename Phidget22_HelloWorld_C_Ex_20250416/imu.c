#include <phidget22.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>   // Required for Madgwick (atan2f, sqrtf, etc.)

// Define PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif


float radians_to_degrees(float radians) {
    return radians * (180.0f / M_PI);
}

// Using float to match Madgwick implementation
float normalize_angle(float angle_degrees) {
    while (angle_degrees >= 360.0f) {
        angle_degrees -= 360.0f;
    }
    while (angle_degrees < 0.0f) {
        angle_degrees += 360.0f;
    }
    return angle_degrees;
}

//-------------------------------------------------------------------------------------------
// Phidget Callback Function

// Note: PhidgetSpatial provides data as 'double', Madgwick uses 'float'.
static void CCONV onSpatialData(PhidgetSpatialHandle ch, void * ctx, const double acceleration[3], const double angularRate[3], const double magneticField[3], double timestamp) {

    // Raw data (optional)
    printf(" Accel(raw): %.3f | %.3f | %.3f\n", acceleration[0], acceleration[1], acceleration[2]);
    printf(" Gyro(raw):  %.3f | %.3f | %.3f\n", angularRate[0], angularRate[1], angularRate[2]);
    printf(" Mag(raw):   %.3f | %.3f | %.3f\n", magneticField[0], magneticField[1], magneticField[2]);
    printf("----------\n");
}

//-------------------------------------------------------------------------------------------
// Phidget Initialization and Main Loop

void zeroGyro(PhidgetSpatialHandle ch) {
    printf("PhidgetSpatial_zeroGyro now...\n");
    PhidgetReturnCode ret = PhidgetSpatial_zeroGyro(ch);
     if (ret != EPHIDGET_OK) {
        fprintf(stderr, "Error calling PhidgetSpatial_zeroGyro: %d\n", ret);
    } else {
        printf("Gyro zeroing requested.\n");
    }
}

int main() {
	//PhidgetGyroscopeHandle gyroscope0;
	PhidgetSpatialHandle spatial0;
	PhidgetSpatial_SpatialEulerAngles angles;
	char ch = 0;

	//PhidgetGyroscope_create(&gyroscope0);
	PhidgetSpatial_create(&spatial0);

	//PhidgetGyroscope_setOnAngularRateUpdateHandler(gyroscope0, onAngularRateUpdate, NULL);
	PhidgetSpatial_setOnSpatialDataHandler(spatial0, onSpatialData, NULL);

	//Phidget_openWaitForAttachment((PhidgetHandle)gyroscope0, 5000);
	Phidget_openWaitForAttachment((PhidgetHandle)spatial0, 5000);

	PhidgetSpatial_zeroGyro(spatial0);

	//Wait until Enter has been pressed before exiting
	while(1) {
		ch = getchar();
		if (ch == 'e') {
			break;
		}
		else if(ch == 'g') {
			PhidgetSpatial_getEulerAngles(ch, &angles);
			printf('Heading %lf\n', angles.heading);
		}
		else {
			zeroGyro(spatial0);
		}
		
	}

	//Phidget_close((PhidgetHandle)gyroscope0);
	Phidget_close((PhidgetHandle)spatial0);

	//PhidgetGyroscope_delete(&gyroscope0);
	PhidgetSpatial_delete(&spatial0);
}

