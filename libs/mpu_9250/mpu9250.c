#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <math.h>
#include "mpu9250.h"

/* MPU9250 SPI Configuration for GY-9250 module
   
   Connections:
   GPIO 16 (pin 21) MISO/spi0_rx -> ADO on MPU9250 board
   GPIO 3 (pin 5) Chip select -> NCS on MPU9250 board
   GPIO 2 (pin 4) SCK/spi0_sclk -> SCL on MPU9250 board
   GPIO 19 (pin 25) MOSI/spi0_tx -> SDA on MPU9250 board
   3.3v (pin 36) -> VCC on MPU9250 board
   GND (pin 38) -> GND on MPU9250 board
*/

#define PIN_MISO 16
#define PIN_CS 3
#define PIN_SCK 2
#define PIN_MOSI 19

#define SPI_PORT spi0
#define READ_BIT 0x80

void cs_select()
{
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 0); // Active low
    asm volatile("nop \n nop \n nop");
}

void cs_deselect()
{
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}

void mpu9250_reset()
{
    printf("[MPU] Sending reset/wake command...\n");
    
    // Wake up from sleep mode
    uint8_t wake_buf[] = {0x6B, 0x00};
    cs_select();
    spi_write_blocking(SPI_PORT, wake_buf, 2);
    cs_deselect();
    sleep_ms(100);
    
    // Configure to use PLL with X-axis gyro reference
    uint8_t pll_buf[] = {0x6B, 0x01};
    cs_select();
    spi_write_blocking(SPI_PORT, pll_buf, 2);
    cs_deselect();
    sleep_ms(10);
    
    // Configure accelerometer (±2g)
    uint8_t accel_buf[] = {0x1C, 0x00};
    cs_select();
    spi_write_blocking(SPI_PORT, accel_buf, 2);
    cs_deselect();
    sleep_ms(10);
    
    // Configure gyroscope (±250°/s)
    uint8_t gyro_buf[] = {0x1B, 0x00};
    cs_select();
    spi_write_blocking(SPI_PORT, gyro_buf, 2);
    cs_deselect();
    sleep_ms(10);
    
    printf("[MPU] Configuration complete.\n");
}

void read_registers(uint8_t reg, uint8_t *buf, uint16_t len)
{
    reg |= READ_BIT;
    cs_select();
    spi_write_blocking(SPI_PORT, &reg, 1);
    spi_read_blocking(SPI_PORT, 0, buf, len);
    cs_deselect();
}

void mpu9250_read_raw_accel(int16_t accel[3]) {
    uint8_t buffer[6];
    read_registers(0x3B, buffer, 6);

    for (int i = 0; i < 3; i++) {
        accel[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
    }
}

void mpu9250_read_raw_gyro(int16_t gyro[3]) {
    uint8_t buffer[6];
    read_registers(0x43, buffer, 6);

    for (int i = 0; i < 3; i++) {
        gyro[i] = (buffer[i * 2] << 8 | buffer[(i * 2) + 1]);
    }
}

void mpu9250_enable_interrupt(void)
{
    printf("[MPU] Enabling data ready interrupt...\n");
    
    // INT Pin / Bypass Enable Configuration (Register 0x37)
    // Bit 7: INT level (0=active high)
    // Bit 6: INT open drain (0=push-pull)
    // Bit 5: Latch INT (0=50us pulse, 1=latch until read)
    // Bit 4: INT read clear (1=any read clears)
    // Other bits: 0
    uint8_t int_pin_cfg[] = {0x37, 0x10};  // Push-pull, 50us pulse, clear on any read
    cs_select();
    spi_write_blocking(SPI_PORT, int_pin_cfg, 2);
    cs_deselect();
    sleep_ms(10);
    
    // INT Enable (Register 0x38)
    // Bit 0: DATA_RDY_EN (1=enable data ready interrupt)
    uint8_t int_enable[] = {0x38, 0x01};  // Enable data ready interrupt
    cs_select();
    spi_write_blocking(SPI_PORT, int_enable, 2);
    cs_deselect();
    sleep_ms(10);
    
    printf("[MPU] Data ready interrupt enabled.\n");
}

void calibrate_gyro(int16_t gyroCal[3], int loop)
{
    int16_t temp[3];
    for (int i = 0; i < loop; i++)
    {
        mpu9250_read_raw_gyro(temp);
        gyroCal[0] += temp[0];
        gyroCal[1] += temp[1];
        gyroCal[2] += temp[2];
    }
    gyroCal[0] /= loop;
    gyroCal[1] /= loop;
    gyroCal[2] /= loop;
}

void calculate_angles(int16_t eulerAngles[2], int16_t accel[3], int16_t gyro[3], uint64_t usSinceLastReading)
{
    long hertz = 1000000/usSinceLastReading;
    
    if (hertz < 200)
    {
        calculate_angles_from_accel(eulerAngles, accel);
        return;
    }

    long temp = 1.l/(hertz * 65.5l);  

    eulerAngles[0] += gyro[0] * temp;
    eulerAngles[1] += gyro[1] * temp;

    eulerAngles[0] += eulerAngles[1] * sin(gyro[2] * temp * 0.1f);
    eulerAngles[1] -= eulerAngles[0] * sin(gyro[2] * temp * 0.1f);

    int16_t accelEuler[2];
    calculate_angles_from_accel(accelEuler, accel);

    eulerAngles[0] = eulerAngles[0] * 0.9996 + accelEuler[0] * 0.0004;
    eulerAngles[1] = eulerAngles[1] * 0.9996 + accelEuler[1] * 0.0004;
}

void calculate_angles_from_accel(int16_t eulerAngles[2], int16_t accel[3])
{
    float accTotalVector = sqrt((accel[0] * accel[0]) + (accel[1] * accel[1]) + (accel[2] * accel[2]));

    float anglePitchAcc = asin(accel[1] / accTotalVector) * 57.296;
    float angleRollAcc = asin(accel[0] / accTotalVector) * -57.296;

    eulerAngles[0] = anglePitchAcc;
    eulerAngles[1] = angleRollAcc;
}

void convert_to_full(int16_t eulerAngles[2], int16_t accel[3], int16_t fullAngles[2])
{
    if (accel[1] > 0 && accel[2] > 0) fullAngles[0] = eulerAngles[0];
    if (accel[1] > 0 && accel[2] < 0) fullAngles[0] = 180 - eulerAngles[0];
    if (accel[1] < 0 && accel[2] < 0) fullAngles[0] = 180 - eulerAngles[0];
    if (accel[1] < 0 && accel[2] > 0) fullAngles[0] = 360 + eulerAngles[0];

    if (accel[0] < 0 && accel[2] > 0) fullAngles[1] = eulerAngles[1];
    if (accel[0] < 0 && accel[2] < 0) fullAngles[1] = 180 - eulerAngles[1];
    if (accel[0] > 0 && accel[2] < 0) fullAngles[1] = 180 - eulerAngles[1];
    if (accel[0] > 0 && accel[2] > 0) fullAngles[1] = 360 + eulerAngles[1];
}

void start_spi()
{
    // Initialize SPI at 1MHz with Mode 0 (CPOL=0, CPHA=0)
    spi_init(SPI_PORT, 1000 * 1000);
    spi_set_format(SPI_PORT, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    sleep_ms(100);  // Give SPI time to initialize
    
    mpu9250_reset();

    // Verify WHO_AM_I (should be 0x71, 0x70, or 0x73)
    uint8_t id;
    read_registers(0x75, &id, 1);
    printf("[MPU] WHO_AM_I: 0x%02X ", id);
    
    if (id == 0x71) {
        printf("(MPU9250)\n");
    } else if (id == 0x70) {
        printf("(MPU9250 compatible/clone)\n");
    } else if (id == 0x73) {
        printf("(MPU9255)\n");
    } else {
        printf("(Unknown - may not work properly)\n");
    }
}
