#pragma once

#include <cstdint>

#define MOTOR_PIN_LN_1 7
#define MOTOR_PIN_LN_2 6
#define MOTOR_PIN_ENA  8

void motor_hw_init();
void motor_forward(uint16_t speed);   // 0–1000
void motor_backward(uint16_t speed);  // 0–1000
void motor_stop();
void motor_test();
