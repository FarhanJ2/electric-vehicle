#include <cstdio>
#include <string>

#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#include "periodic.h"
#include "constant.h"

#include "hardware/imu_hw.h"
#include "hardware/oled_hw.h"
#include "hardware/button_hw.h"
#include "hardware/motor_hw.h"

int main() {
    stdio_init_all();
    sleep_ms(2000);
    
    printf("\n=== MPU9250 Data Read Test (0x70 chip) ===\n");
    
    gpio_init(3);
    gpio_set_dir(3, GPIO_OUT);
    gpio_put(3, 1);
    
    // Use Mode 0 (CPOL=0, CPHA=0) since it gave 0x70
    spi_init(spi0, 1000 * 1000);
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(16, GPIO_FUNC_SPI);
    gpio_set_function(2, GPIO_FUNC_SPI);
    gpio_set_function(19, GPIO_FUNC_SPI);
    
    sleep_ms(100);
    
    // Wake up and configure
    printf("Configuring sensor...\n");
    uint8_t config_cmds[][2] = {
        {0x6B, 0x00},  // PWR_MGMT_1: Wake up
        {0x6B, 0x01},  // PWR_MGMT_1: Use PLL with X-axis gyro reference
        {0x1C, 0x00},  // ACCEL_CONFIG: ±2g
        {0x1B, 0x00},  // GYRO_CONFIG: ±250°/s
    };
    
    for (int i = 0; i < 4; i++) {
        gpio_put(3, 0);
        spi_write_blocking(spi0, config_cmds[i], 2);
        gpio_put(3, 1);
        sleep_ms(10);
    }
    
    sleep_ms(100);
    printf("Reading data (move the sensor!)...\n\n");
    
    for (int i = 0; i < 20; i++) {
        // Read accel registers (0x3B-0x40)
        uint8_t reg = 0x3B | 0x80;
        uint8_t accel_buf[6];
        
        gpio_put(3, 0);
        spi_write_blocking(spi0, &reg, 1);
        spi_read_blocking(spi0, 0, accel_buf, 6);
        gpio_put(3, 1);
        
        int16_t ax = (accel_buf[0] << 8) | accel_buf[1];
        int16_t ay = (accel_buf[2] << 8) | accel_buf[3];
        int16_t az = (accel_buf[4] << 8) | accel_buf[5];
        
        // Read gyro registers (0x43-0x48)
        reg = 0x43 | 0x80;
        uint8_t gyro_buf[6];
        
        gpio_put(3, 0);
        spi_write_blocking(spi0, &reg, 1);
        spi_read_blocking(spi0, 0, gyro_buf, 6);
        gpio_put(3, 1);
        
        int16_t gx = (gyro_buf[0] << 8) | gyro_buf[1];
        int16_t gy = (gyro_buf[2] << 8) | gyro_buf[3];
        int16_t gz = (gyro_buf[4] << 8) | gyro_buf[5];
        
        float ax_g = ax / 16384.0f;
        float ay_g = ay / 16384.0f;
        float az_g = az / 16384.0f;
        
        float gx_dps = gx / 131.0f;
        float gy_dps = gy / 131.0f;
        float gz_dps = gz / 131.0f;
        
        printf("A[g]: %6.3f %6.3f %6.3f | G[dps]: %7.2f %7.2f %7.2f\n", 
               ax_g, ay_g, az_g, gx_dps, gy_dps, gz_dps);
        
        sleep_ms(100);
    }
    
    printf("\n");
    printf("If you see changing values when you move the sensor,\n");
    printf("then your chip works fine despite WHO_AM_I = 0x70!\n");
    
    while(1) {
        sleep_ms(1000);
    }
}