#include "hardware/imu_hw.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <cstdio>
extern "C" {
    #include "mpu9250.h"
}


float accel[3], gyro[3], temp;
static volatile bool imu_data_ready = false;

// Calibration offsets
int16_t gyroCal[3] = {0, 0, 0};

// Initialize SPI and MPU
int imu_hw_init() {
    printf("[IMU] Starting SPI...\n");
    start_spi();  // This resets and initializes the MPU

    // Optional: Calibrate gyro (must keep IMU still)
    printf("[IMU] Calibrating gyro...\n");
    calibrate_gyro(gyroCal, 500); // Adjust number of samples if needed
    printf("[IMU] Gyro calibrated: X=%d Y=%d Z=%d\n", gyroCal[0], gyroCal[1], gyroCal[2]);

    // Setup interrupt pin
    gpio_init(PIN_INTERRUPT);
    gpio_set_dir(PIN_INTERRUPT, GPIO_IN);
    gpio_pull_up(PIN_INTERRUPT);
    gpio_set_irq_enabled_with_callback(PIN_INTERRUPT, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    printf("[IMU] Initialization done.\n");
    return 0; // Success
}

// Poll IMU for new data if interrupt triggered
void imu_hw_poll() {
    if (!imu_data_ready) return;
    imu_data_ready = false;

    int16_t rawAccel[3], rawGyro[3];
    mpu9250_read_raw_accel(rawAccel);
    mpu9250_read_raw_gyro(rawGyro);

    // Apply gyro calibration
    for (int i = 0; i < 3; i++) {
        rawGyro[i] -= gyroCal[i];
    }

    // Convert to floats in G / deg/s (assuming +/- 8G accel, +/-1000 dps gyro)
    for (int i = 0; i < 3; i++) {
        accel[i] = rawAccel[i] / 4096.0f; // 8G scale
        gyro[i] = rawGyro[i] / 32.8f;     // 1000 dps scale
    }

    // Read temperature
    uint8_t tempRaw[2];
    read_registers(0x41, tempRaw, 2); // TEMP_OUT_H/L
    int16_t t = (tempRaw[0] << 8) | tempRaw[1];
    temp = (t / 333.87f) + 21.0f;

    // Print values
    printf("Accel (G)  X=%6.3f Y=%6.3f Z=%6.3f\n", accel[0], accel[1], accel[2]);
    printf("Gyro (dps) X=%6.3f Y=%6.3f Z=%6.3f\n", gyro[0], gyro[1], gyro[2]);
    printf("Temp (C)   %6.2f\n\n", temp);
}

// Interrupt callback
void gpio_callback(uint gpio, uint32_t events) {
    (void)gpio;
    (void)events;
    imu_data_ready = true;
}
