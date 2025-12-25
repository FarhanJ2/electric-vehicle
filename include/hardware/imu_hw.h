#pragma once
#include <cstdint>
#include "pico/stdlib.h"

#define PIN_INTERRUPT 15
#define DEVICE_ADDRESS 0x68

int imu_hw_init();
void imu_hw_read(float* ax, float* ay, float* az,
                 float* gx, float* gy, float* gz,
                 float* mx, float* my, float* mz);

// Call regularly from the main loop to service IMU data without doing I2C in the IRQ.
void imu_hw_poll();

void gpio_callback(uint gpio, uint32_t events);
