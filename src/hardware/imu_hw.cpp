#include "hardware/imu_hw.h"

#include <cstdint>
#include <cstdio>

#include "mp92plus.h"

mpu9250_t mpu92;

float accel[3], gyro[3], temp;

int imu_hw_init() {
    bool is_ok = true;
    uint8_t activate = mpu9250_reset(
        &mpu92,
        PIN_SDA,
        PIN_SCL,
        DEVICE_ADDRESS,
        i2c0,
        100 * 1000 // frequency 100kHz
    );
    if (activate) {
        printf("IMU init failed\n");
        is_ok = false;
    }
    mpu_settings_t set = {
        .accel_range = ACCEL_RANGE_8G,
        .gyro_range = GYRO_RANGE_1000DPS,
        .dlpf_filter = DLPF_41HZ,
        .sample_rate_divider = 249};
    int16_t correct_gyro[] = {0, 0, 0};
    uint8_t settings = mpu9250_setup(&mpu92, &set, correct_gyro);
    if (settings) {
        printf("IMU setup failed\n");
        is_ok = false;
    }

    mpu9250_attach_interrupt(&mpu92, PIN_INTERRUPT, gpio_callback);
    return !is_ok;
}

void gpio_callback(uint gpio, uint32_t events) {
    mpu9250_read_motion(&mpu92, accel, gyro);
    mpu9250_read_temperature(&mpu92, &temp);
    printf("Acceleration in G     X = %10.4f,  Y = %10.4f,  Z = %10.4f\n", accel[0], accel[1], accel[2]);
    printf("Gyroscope in Deg/s    X = %10.4f,  Y = %10.4f,  Z = %10.4f\n", gyro[0], gyro[1], gyro[2]);
    printf("Temperature in C      %10.4f\n\n", temp);
    mpu9250_continue_interrupt(&mpu92);
}
