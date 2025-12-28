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
    sleep_ms(2000); // give time to connect to serial console
    
    printf("\n\n=== PROGRAM START ===\n");
    fflush(stdout);

    // Initialise the Wi-Fi chip
    printf("Initializing Wi-Fi...\n");
    fflush(stdout);
    
    if (cyw43_arch_init()) {
        std::printf("Wi-Fi init failed\n");
        return -1;
    }
    
    printf("Wi-Fi initialized.\n");
    fflush(stdout);

    // turn on the Pico W LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    sleep_ms(1000); // wait for a second

    // Initialize hardware
    bool imu_fault = false;
    bool oled_fault = false;
    bool lmotor_fault = false;
    bool rmotor_fault = false;
    bool has_fault = false;
    if (imu_hw_init()) {
        printf("[IMU] initialization failed!\n");
        imu_fault = true;
    }

    if (oled_hw_init()) {
        printf("[OLED] initialization failed!\n");
        oled_fault = true;
    }

    motor_hw_init();

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

        if (start_prod.is_pressed()) {
            motor_forward(1000);
        }

        if (start_prod.just_released()) {
            motor_stop();
        }

        imu_hw_poll();
        if (absolute_time_diff_us(get_absolute_time(), next_blink) <= 0) {
            led_on = !led_on;
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
            next_blink = make_timeout_time_ms(blink_ms);
        }
        sleep_ms(int(constants::dt * 1000));

        // motor_test();
        // sleep_ms(1000);

    }
}
