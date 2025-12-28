#include "hardware/imu_hw.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <cstdio>
#include <string>
extern "C" {
    #include "mpu9250.h"
}

float accel[3], gyro[3], temp;
float accel_g[3];
float gyro_dps[3];
float temperature_c;

int16_t gyro_bias[3] = {0};
static volatile bool imu_data_ready = false;

// Calibration offsets
int16_t gyroCal[3] = {0, 0, 0};

// Interrupt callback
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
    
    printf("[IMU] Calibrating gyro (keep IMU still)...\n");
    calibrate_gyro(gyroCal, 500);
    printf("[IMU] Gyro calibrated: X=%d Y=%d Z=%d\n", gyroCal[0], gyroCal[1], gyroCal[2]);
    
    // Enable data ready interrupt on MPU9250
    printf("[IMU] Configuring interrupt...\n");
    mpu9250_enable_interrupt();
    
    // Setup interrupt pin on Pico
    gpio_init(PIN_INTERRUPT);
    gpio_set_dir(PIN_INTERRUPT, GPIO_IN);
    gpio_pull_up(PIN_INTERRUPT);  // MPU9250 INT is active HIGH
    gpio_set_irq_enabled_with_callback(PIN_INTERRUPT, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    
    printf("[IMU] Initialization done.\n");
    return 0;
}

void imu_hw_poll(void) {
    // only read if new data
    if (!imu_data_ready) {
        return;
    }
    
    imu_data_ready = false;    
    int16_t accel_raw[3];
    int16_t gyro_raw[3];

    mpu9250_read_raw_accel(accel_raw);
    mpu9250_read_raw_gyro(gyro_raw);

    // Convert to G's and degrees/sec
    for (int i = 0; i < 3; i++) {
        accel_g[i] = accel_raw[i] / 16384.0f;  // ±2G range
        gyro_dps[i] = (gyro_raw[i] - gyroCal[i]) / 131.0f;  // ±250°/s range
    }
}

std::string get_imu_status(void) {
    char buffer[100];
    snprintf(buffer, sizeof(buffer),
             "Accel [g]: X=%.3f Y=%.3f Z=%.3f | Gyro [dps]: X=%.2f Y=%.2f Z=%.2f",
             accel_g[0], accel_g[1], accel_g[2],
             gyro_dps[0], gyro_dps[1], gyro_dps[2]);
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

bool imu_data_available(void) {
    return imu_data_ready;
}