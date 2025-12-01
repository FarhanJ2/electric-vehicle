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

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart1
#define BAUD_RATE 115200

// Move UART1 off the IMU pins to avoid conflicts with I2C on 4/5
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_TX_PIN 8
#define UART_RX_PIN 9

int main() {
    stdio_init_all();

    // Initialise the Wi-Fi chip
    if (cyw43_arch_init()) {
        std::printf("Wi-Fi init failed\n");
        return -1;
    }

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS, GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    // Example to turn on the Pico W LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART

    // Send out a string, with CR/LF conversions
    uart_puts(UART_ID, " Hello, UART!\n");

    // For more examples of UART use see https://github.com/raspberrypi/pico-examples/tree/master/uart

    sleep_ms(1000); // wait for a second

    // Initialize hardware
    bool imu_fault = true;
    bool oled_fault = true;
    bool lmotor_fault = true;
    bool rmotor_fault = true;
    bool has_fault = false;
    // if (imu_hw_init()) {
    //     printf("[IMU] initialization failed!\n");
    //     imu_failed = true;
    // }

    if (oled_hw_init()) {
        printf("[OLED] initialization failed!\n");
        oled_fault = false;
    }

    has_fault = imu_fault || lmotor_fault || rmotor_fault || oled_fault;
    uint8_t fault_count =
        (imu_fault ? 1 : 0) +
        (lmotor_fault ? 1 : 0) +
        (rmotor_fault ? 1 : 0);

    oled_hw_clear();
    play_animation(10);
    oled_hw_clear();
    oled_hw_print(0, 0, "Nebula Runner [Alpha]");
    oled_hw_print(0, 20, ("[IMU] " + std::string(imu_fault ? "FAULT" : "READY")).c_str());
    oled_hw_print(0, 30, ("[MOTORLEFT] " + std::string(lmotor_fault ? "FAULT" : "READY")).c_str());
    oled_hw_print(0, 40, ("[MOTORRIGHT] " + std::string(rmotor_fault ? "FAULT" : "READY")).c_str());
    oled_hw_print(0, 55, ("[System " + std::string(has_fault == 0 ? "READY" : "FAILED") + std::string("]")).c_str());
    oled_hw_update();

    button_hw start_prod(13);

    bool led_on = true;
    /* 
    * Blink the onboard LED according to fault status:
    * No Fault: Solid On
    * IMU Fault: 500ms blink
    * Motor Fault: 100ms blink
    * Other Faults (i.e. OLED): 800ms blink
    */
    const uint32_t blink_ms =
        fault_count > 1 
            ? 50 
            : has_fault ?
                imu_fault ? 500 : 
                    (lmotor_fault || rmotor_fault) ? 100 : 800
                : 0;

    absolute_time_t next_blink = make_timeout_time_ms(blink_ms);
    while (true) {
        periodic();
        start_prod.update();

        if (start_prod.just_pressed()) {
            printf("Start button pressed.\n");
            sleep_ms(10);
        }

        imu_hw_poll();
        if (absolute_time_diff_us(get_absolute_time(), next_blink) <= 0) {
            led_on = !led_on;
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
            next_blink = make_timeout_time_ms(blink_ms);
        }
        sleep_ms(int(constants::dt * 1000));
    }
}
