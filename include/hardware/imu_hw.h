#pragma once
#include <cstdint>
#include <string>
#include "pico/stdlib.h"

#define PIN_INTERRUPT 5 // physical pin 5
#define DEVICE_ADDRESS 0x70 // 0x70 not 0x71

extern float pitch, roll, yaw;

int imu_hw_init();
void imu_hw_poll();
void gpio_callback(uint gpio, uint32_t events);

std::string get_imu_status(void);

void imu_get_accel(float* ax, float* ay, float* az);
void imu_get_gyro(float* gx, float* gy, float* gz);

void imu_get_angles(float* pitch, float* roll, float* yaw);
void imu_reset_yaw(void);
