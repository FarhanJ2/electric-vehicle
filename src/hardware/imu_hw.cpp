#include "hardware/imu_hw.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <cstdio>
#include <string>
#include <cmath>
extern "C" {
    #include "mpu9250.h"
}

float accel[3], gyro[3], temp;
float accel_g[3];
float gyro_dps[3];
float temperature_c;

// angle tracking
float pitch = 0.0f;  // Rotation around Y-axis (forward/backward tilt)
float roll = 0.0f;   // Rotation around X-axis (left/right tilt)
float yaw = 0.0f;    // Rotation around Z-axis (heading)

int16_t gyro_bias[3] = {0};
static volatile bool imu_data_ready = false;

// calibration offsets
int16_t gyroCal[3] = {0, 0, 0};

// timing for integration
static absolute_time_t last_update_time = nil_time;

// interupt callback
void gpio_callback(uint gpio, uint32_t events) {
    if (gpio == PIN_INTERRUPT) {
        imu_data_ready = true;
    }
}

int imu_hw_init() {
    printf("[IMU] Starting SPI...\n");
    start_spi();
    
    uint8_t who_am_i;
    read_registers(0x75, &who_am_i, 1);
    printf("[IMU] WHO_AM_I register: 0x%02X\n", who_am_i);

    if (who_am_i != 0x71 && who_am_i != 0x70 && who_am_i != 0x73) {
        printf("[IMU] ERROR: Unexpected WHO_AM_I value!\n");
        return -1;
    }
    
    printf("[IMU] Waiting for sensor to stabilize...\n");
    sleep_ms(1000);
    
    printf("[IMU] Calibrating gyro (keep IMU VERY still)...\n");
    calibrate_gyro(gyroCal, 2000);
    printf("[IMU] Gyro calibrated: X=%d Y=%d Z=%d\n", gyroCal[0], gyroCal[1], gyroCal[2]);
    
    // enable data ready interrupt on MPU9250
    printf("[IMU] Configuring interrupt...\n");
    mpu9250_enable_interrupt();
    
    // setup interrupt pin on Pico
    gpio_init(PIN_INTERRUPT);
    gpio_set_dir(PIN_INTERRUPT, GPIO_IN);
    gpio_pull_up(PIN_INTERRUPT);
    gpio_set_irq_enabled_with_callback(PIN_INTERRUPT, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    
    // init timing
    last_update_time = get_absolute_time();
    imu_reset_yaw();
    
    printf("[IMU] Initialization done.\n");
    return 0;
}

void imu_hw_poll(void) {
    if (!imu_data_ready) {
        return;
    }
    
    imu_data_ready = false;
    
    // calculate dt
    absolute_time_t current_time = get_absolute_time();
    float dt = absolute_time_diff_us(last_update_time, current_time) / 1000000.0f;  // Convert to seconds
    last_update_time = current_time;
    
    int16_t accel_raw[3];
    int16_t gyro_raw[3];

    mpu9250_read_raw_accel(accel_raw);
    mpu9250_read_raw_gyro(gyro_raw);

    // convert to G's and deg/s
    for (int i = 0; i < 3; i++) {
        accel_g[i] = accel_raw[i] / 16384.0f;  // +-2G range
        gyro_dps[i] = (gyro_raw[i] - gyroCal[i]) / 131.0f;  // +-250°/s range
    }
    // === Complementary Filter ===
    // Combines gyro (short-term accuracy) with accel (long-term stability)
    
    // Step 1: Get angle from accelerometer (using gravity direction)
    float accel_pitch = atan2(accel_g[1], accel_g[2]) * 57.2958f;  // radians to degrees
    float accel_roll = atan2(-accel_g[0], accel_g[2]) * 57.2958f;
    
    // Step 2: Integrate gyro
    float gyro_pitch = pitch + gyro_dps[0] * dt;
    float gyro_roll = roll + gyro_dps[1] * dt;
    
    // Step 3: Complementary filter (98% gyro, 2% accel)
    // This corrects gyro drift while keeping fast response
    pitch = 0.98f * gyro_pitch + 0.02f * accel_pitch;
    roll = 0.98f * gyro_roll + 0.02f * accel_roll;
    
    // Yaw can only come from gyro (accel cant measure rotation around vertical axis)
    yaw += gyro_dps[2] * dt; // TODO FIX WEIRD DRIFT, maybe add magnetometer later, Gy consistently is at -1.5, -2.0 dps when still
    
    // wrap yaw to [0, 360)
    if (yaw > 360.0f) yaw -= 360.0f;
    if (yaw < 0.0f) yaw += 360.0f;
}

std::string get_imu_status(void) {
    char buffer[150];
    snprintf(buffer, sizeof(buffer),
             "Pitch: %.1f° Roll: %.1f° Yaw: %.1f° | Ax: %.2fg Ay: %.2fg Az: %.2fg",
             pitch, roll, yaw,
             accel_g[0], accel_g[1], accel_g[2]);
    return std::string(buffer);
}

void imu_get_accel(float* ax, float* ay, float* az) {
    if (ax) *ax = accel_g[0];
    if (ay) *ay = accel_g[1];
    if (az) *az = accel_g[2];
}

void imu_get_gyro(float* gx, float* gy, float* gz) {
    if (gx) *gx = gyro_dps[0];
    if (gy) *gy = gyro_dps[1];
    if (gz) *gz = gyro_dps[2];
}

void imu_get_angles(float* p, float* r, float* y) {
    if (p) *p = pitch;
    if (r) *r = roll;
    if (y) *y = yaw;
}

void imu_reset_yaw(void) {
    yaw = 0.0f;
}

bool imu_data_available(void) {
    return imu_data_ready;
}