#include <phidget22.h>
#include <stdio.h>
#include <stdint.h> // Required for Madgwick (if using high-res timers elsewhere)
#include <math.h>   // Required for Madgwick (atan2f, sqrtf, etc.)

// Define PI if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

//-------------------------------------------------------------------------------------------
// Madgwick AHRS Definitions & Variables

#define SAMPLE_FREQ_DEFAULT 100.0f // Adjust to your PhidgetSpatial Data Rate in Hz
#define BETA_DEFAULT 0.1f         // Madgwick filter gain (tune as needed)

volatile float beta = BETA_DEFAULT;
volatile float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f; // Quaternion of sensor frame relative to auxiliary frame
volatile float sampleFreq = SAMPLE_FREQ_DEFAULT;

//-------------------------------------------------------------------------------------------
// Madgwick Function Prototypes

void MadgwickAHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz);
void MadgwickAHRSupdateIMU(float gx, float gy, float gz, float ax, float ay, float az);
float invSqrt(float x);
void getEulerAngles(float *roll, float *pitch, float *yaw);

//-------------------------------------------------------------------------------------------
// Helper Functions

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

    // --- Convert Phidget Data to format needed by Madgwick ---

    // Gyroscope: Convert degrees/sec (Phidget) to radians/sec (Madgwick)
    float gx_rad = (float)angularRate[0] * (M_PI / 180.0f);
    float gy_rad = (float)angularRate[1] * (M_PI / 180.0f);
    float gz_rad = (float)angularRate[2] * (M_PI / 180.0f);

    // Accelerometer: Cast to float
    float ax = (float)acceleration[0];
    float ay = (float)acceleration[1];
    float az = (float)acceleration[2];

    // Magnetometer: Cast to float
    // !!! IMPORTANT: Apply calibration offsets/scaling here before passing !!!
    // Example (replace with your actual calibration):
    // float mx_calibrated = (float)magneticField[0] - mag_offset_x;
    // float my_calibrated = (float)magneticField[1] - mag_offset_y;
    // float mz_calibrated = (float)magneticField[2] - mag_offset_z;
    // For now, using raw data (cast to float):
    float mx = (float)magneticField[0];
    float my = (float)magneticField[1];
    float mz = (float)magneticField[2];


    // --- Update the Madgwick AHRS filter ---
    // WARNING: Ensure sensor axes match Madgwick expectation (NED by default).
    // You might need to swap/negate axes here, e.g., MadgwickAHRSupdate(gy_rad, gx_rad, -gz_rad, ...)
    MadgwickAHRSupdate(gx_rad, gy_rad, gz_rad, ax, ay, az, mx, my, mz);

    // --- Get Euler Angles from the filter's quaternion ---
    float roll_deg, pitch_deg, yaw_deg;
    getEulerAngles(&roll_deg, &pitch_deg, &yaw_deg); // Yaw is magnetic heading

    // --- (Optional) Apply Magnetic Declination for True Heading ---
    // float declination_deg = -0.8f; // Example for Perth, WA - GET LOCAL VALUE!
    // float true_heading_deg = yaw_deg + declination_deg;
    // true_heading_deg = normalize_angle(true_heading_deg);


    // --- Print Results ---
    // Print less frequently if needed, e.g., using a counter or checking timestamp interval
    printf("Timestamp: %.3f\n", timestamp);
    // Raw data (optional)
    // printf(" Accel(raw): %.3f | %.3f | %.3f\n", acceleration[0], acceleration[1], acceleration[2]);
    // printf(" Gyro(raw):  %.3f | %.3f | %.3f\n", angularRate[0], angularRate[1], angularRate[2]);
    // printf(" Mag(raw):   %.3f | %.3f | %.3f\n", magneticField[0], magneticField[1], magneticField[2]);
    // Calculated Angles
    printf(" AHRS Angles: Roll=%.2f, Pitch=%.2f, Yaw(Mag)=%.2f\n", roll_deg, pitch_deg, yaw_deg);
    // printf("              True Heading (Est)=%.2f\n", true_heading_deg);
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


//-------------------------------------------------------------------------------------------
// AHRS Algorithm Implementation (Madgwick)
// (Copied from ahrs_madgwick_c immersive, ensure it's complete and correct)

void MadgwickAHRSupdate(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz) {
	float recipNorm;
	float s0, s1, s2, s3;
	float qDot1, qDot2, qDot3, qDot4;
	float hx, hy;
	float _2q0mx, _2q0my, _2q0mz, _2q1mx, _2q1my, _2q1mz, _2q2mx, _2q2my, _2q2mz, _2q3mx, _2q3my, _2q3mz;
	float _2q0, _2q1, _2q2, _2q3, _2q0q2, _2q2q3;
	float q0q0, q0q1, q0q2, q0q3, q1q1, q1q2, q1q3, q2q2, q2q3, q3q3;
    float _4bx = 0.0f, _4bz = 0.0f; // Initialize to avoid potential uninitialized use warnings


	// Use IMU algorithm if magnetometer measurement invalid (avoids NaN in magnetometer normalisation)
	if((mx == 0.0f) && (my == 0.0f) && (mz == 0.0f)) {
		MadgwickAHRSupdateIMU(gx, gy, gz, ax, ay, az);
		return;
	}

	// Rate of change of quaternion from gyroscope
	qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
	qDot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);
	qDot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);
	qDot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

		// Normalise accelerometer measurement
		recipNorm = invSqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;

		// Normalise magnetometer measurement
		recipNorm = invSqrt(mx * mx + my * my + mz * mz);
		mx *= recipNorm;
		my *= recipNorm;
		mz *= recipNorm;

        // Auxiliary variables to avoid repeated arithmetic
        _2q0mx = 2.0f * q0 * mx;
        _2q0my = 2.0f * q0 * my;
        _2q0mz = 2.0f * q0 * mz;
        _2q1mx = 2.0f * q1 * mx;
        _2q1my = 2.0f * q1 * my;
        _2q1mz = 2.0f * q1 * mz;
        _2q2mx = 2.0f * q2 * mx;
        _2q2my = 2.0f * q2 * my;
        _2q2mz = 2.0f * q2 * mz;
        _2q3mx = 2.0f * q3 * mx;
        _2q3my = 2.0f * q3 * my;
        _2q3mz = 2.0f * q3 * mz;
        _2q0 = 2.0f * q0;
        _2q1 = 2.0f * q1;
        _2q2 = 2.0f * q2;
        _2q3 = 2.0f * q3;
        _2q0q2 = 2.0f * q0 * q2;
        _2q2q3 = 2.0f * q2 * q3;
        q0q0 = q0 * q0;
        q0q1 = q0 * q1;
        q0q2 = q0 * q2;
        q0q3 = q0 * q3;
        q1q1 = q1 * q1;
        q1q2 = q1 * q2;
        q1q3 = q1 * q3;
        q2q2 = q2 * q2;
        q2q3 = q2 * q3;
        q3q3 = q3 * q3;

		// Reference direction of Earth's magnetic field
        hx = mx * (q0q0 + q1q1 - q2q2 - q3q3) + my * (2.0f * q1q2 - _2q0q3) + mz * (2.0f * q1q3 + _2q0q2);
        hy = mx * (2.0f * q1q2 + _2q0q3) + my * (q0q0 - q1q1 + q2q2 - q3q3) + mz * (2.0f * q2q3 - _2q0q2); // Original had _2q0q1 typo?
        float bz = mx * (2.0f * q1q3 - _2q0q2) + my * (2.0f * q2q3 + _2q0q1) + mz * (q0q0 - q1q1 - q2q2 + q3q3); // Renamed from _2bz to avoid conflict

		// Estimated direction of magnetic field
        float _2bx = sqrtf(hx * hx + hy * hy); // Horizontal magnetic field strength
        float _2bz = bz; // Vertical magnetic field strength

        // Correct potential typos in gradient descent terms (using _2bx, _2bz)
        _4bx = 2.0f * _2bx; // If needed below
        _4bz = 2.0f * _2bz; // If needed below


		// Gradient decent algorithm corrective step (carefully check against original paper if issues arise)
        s0 = -_2q2 * (2.0f * q1q3 - _2q0q2 - ax) + _2q1 * (2.0f * q0q1 + _2q2q3 - ay) - _2bz * q2 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q3 + _2bz * q1) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q2 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s1 = _2q3 * (2.0f * q1q3 - _2q0q2 - ax) + _2q0 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q1 * (1.0f - 2.0f * q1q1 - 2.0f * q2q2 - az) + _2bz * q3 * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q2 + _2bz * q0) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q3 - _4bz * q1) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s2 = -_2q0 * (2.0f * q1q3 - _2q0q2 - ax) + _2q3 * (2.0f * q0q1 + _2q2q3 - ay) - 4.0f * q2 * (1.0f - 2.0f * q1q1 - 2.0f * q2q2 - az) + (-_4bx * q2 - _2bz * q0) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (_2bx * q1 + _2bz * q3) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + (_2bx * q0 - _4bz * q2) * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);
        s3 = _2q1 * (2.0f * q1q3 - _2q0q2 - ax) + _2q2 * (2.0f * q0q1 + _2q2q3 - ay) + (-_4bx * q3 + _2bz * q1) * (_2bx * (0.5f - q2q2 - q3q3) + _2bz * (q1q3 - q0q2) - mx) + (-_2bx * q0 + _2bz * q2) * (_2bx * (q1q2 - q0q3) + _2bz * (q0q1 + q2q3) - my) + _2bx * q1 * (_2bx * (q0q2 + q1q3) + _2bz * (0.5f - q1q1 - q2q2) - mz);


		recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
		s0 *= recipNorm;
		s1 *= recipNorm;
		s2 *= recipNorm;
		s3 *= recipNorm;

		// Apply feedback step
		qDot1 -= beta * s0;
		qDot2 -= beta * s1;
		qDot3 -= beta * s2;
		qDot4 -= beta * s3;
	}

	// Integrate rate of change of quaternion to yield quaternion
    float dt = 1.0f / sampleFreq;
	q0 += qDot1 * dt;
	q1 += qDot2 * dt;
	q2 += qDot3 * dt;
	q3 += qDot4 * dt;

	// Normalise quaternion
	recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 *= recipNorm;
	q1 *= recipNorm;
	q2 *= recipNorm;
	q3 *= recipNorm;
}

void MadgwickAHRSupdateIMU(float gx, float gy, float gz, float ax, float ay, float az) {
	float recipNorm;
	float s0, s1, s2, s3;
	float qDot1, qDot2, qDot3, qDot4;
	float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2 ,_8q1, _8q2, q0q0, q1q1, q2q2, q3q3;

	// Rate of change of quaternion from gyroscope
	qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz);
	qDot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy);
	qDot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx);
	qDot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx);

	// Compute feedback only if accelerometer measurement valid (avoids NaN in accelerometer normalisation)
	if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {

		// Normalise accelerometer measurement
		recipNorm = invSqrt(ax * ax + ay * ay + az * az);
		ax *= recipNorm;
		ay *= recipNorm;
		az *= recipNorm;

		// Auxiliary variables to avoid repeated arithmetic
		_2q0 = 2.0f * q0;
		_2q1 = 2.0f * q1;
		_2q2 = 2.0f * q2;
		_2q3 = 2.0f * q3;
		_4q0 = 4.0f * q0;
		_4q1 = 4.0f * q1;
		_4q2 = 4.0f * q2;
		_8q1 = 8.0f * q1;
		_8q2 = 8.0f * q2;
		q0q0 = q0 * q0;
		q1q1 = q1 * q1;
		q2q2 = q2 * q2;
		q3q3 = q3 * q3;

		// Gradient decent algorithm corrective step
		s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
		s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q1 - _2q0 * ay - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
		s2 = 4.0f * q0q0 * q2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
		s3 = 4.0f * q1q1 * q3 - _2q1 * ax + 4.0f * q2q2 * q3 - _2q2 * ay;
		recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3); // normalise step magnitude
		s0 *= recipNorm;
		s1 *= recipNorm;
		s2 *= recipNorm;
		s3 *= recipNorm;

		// Apply feedback step
		qDot1 -= beta * s0;
		qDot2 -= beta * s1;
		qDot3 -= beta * s2;
		qDot4 -= beta * s3;
	}

	// Integrate rate of change of quaternion to yield quaternion
    float dt = 1.0f / sampleFreq;
	q0 += qDot1 * dt;
	q1 += qDot2 * dt;
	q2 += qDot3 * dt;
	q3 += qDot4 * dt;

	// Normalise quaternion
	recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
	q0 *= recipNorm;
	q1 *= recipNorm;
	q2 *= recipNorm;
	q3 *= recipNorm;
}

float invSqrt(float x) {
    #if defined(__ARM_ARCH_7EM__) && (__FPU_PRESENT == 1)
        return 1.0f / sqrtf(x);
    #else
	float halfx = 0.5f * x;
	float y = x;
	long i = *(long*)&y;
	i = 0x5f3759df - (i>>1);
	y = *(float*)&i;
	y = y * (1.5f - (halfx * y * y));
    y = y * (1.5f - (halfx * y * y));
	return y;
    #endif
}

void getEulerAngles(float *roll, float *pitch, float *yaw) {
    // Ensure quaternion is normalized
    float norm = sqrtf(q0*q0 + q1*q1 + q2*q2 + q3*q3);
    if (norm == 0.0f) return; // Should not happen with normalized q
    float q0_n = q0 / norm;
    float q1_n = q1 / norm;
    float q2_n = q2 / norm;
    float q3_n = q3 / norm;

    // Roll (x-axis rotation)
    float sinr_cosp = 2.0f * (q0_n * q1_n + q2_n * q3_n);
    float cosr_cosp = 1.0f - 2.0f * (q1_n * q1_n + q2_n * q2_n);
    *roll = atan2f(sinr_cosp, cosr_cosp);

    // Pitch (y-axis rotation)
    float sinp = 2.0f * (q0_n * q2_n - q3_n * q1_n);
    if (fabsf(sinp) >= 1.0f)
        *pitch = copysignf(M_PI / 2.0f, sinp); // Use 90 degrees if out of range
    else
        *pitch = asinf(sinp);

    // Yaw (z-axis rotation)
    float siny_cosp = 2.0f * (q0_n * q3_n + q1_n * q2_n);
    float cosy_cosp = 1.0f - 2.0f * (q2_n * q2_n + q3_n * q3_n);
    *yaw = atan2f(siny_cosp, cosy_cosp);

    // Convert radians to degrees
    *roll = radians_to_degrees(*roll);
    *pitch = radians_to_degrees(*pitch);
    *yaw = radians_to_degrees(*yaw);
}

int main() {
	//PhidgetGyroscopeHandle gyroscope0;
	PhidgetSpatialHandle spatial0;
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
		
	}

	//Phidget_close((PhidgetHandle)gyroscope0);
	Phidget_close((PhidgetHandle)spatial0);

	//PhidgetGyroscope_delete(&gyroscope0);
	PhidgetSpatial_delete(&spatial0);
}

