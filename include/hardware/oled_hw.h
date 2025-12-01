#pragma once
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define OLED_PIN_SCL 11
#define OLED_PIN_SDA 10
#define ADDR 0x3C
static inline i2c_inst_t* I2C_PORT = i2c1;

int oled_hw_init();
void oled_hw_clear();
void oled_hw_print(int x, int y, const char* text);
void oled_hw_update();
void play_animation(uint32_t delay_ms);
