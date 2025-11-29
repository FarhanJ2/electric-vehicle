#pragma once
#include <cstdint>
#include "pico/stdlib.h"

#define PIN_SCL 1
#define PIN_SDA 2
#define PIN_INTERRUPT 3
#define DEVICE_ADDRESS 0x68

int imu_hw_init();
void imu_hw_read(float* ax, float* ay, float* az,
                 float* gx, float* gy, float* gz,
                 float* mx, float* my, float* mz);

void gpio_callback(uint gpio, uint32_t events);
