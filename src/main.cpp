#include <cstdio>
#include <string>
#include <cmath>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "hardware/uart.h"

#include "lwip/udp.h"
#include "lwip/ip_addr.h"

#include "periodic.h"
#include "telemetry.h"
#include "constant.h"

#include "hardware/imu_hw.h"
#include "hardware/oled_hw.h"
#include "hardware/button_hw.h"
#include "hardware/motor_hw.h"

void button_bindings();
void update_display();

enum DisplayMode {
    DISPLAY_STATUS,
    DISPLAY_IMU_ROC, // rate of change
    DISPLAY_IMU_ANGLES
};

DisplayMode current_display = DISPLAY_STATUS;

// initialize hardware faults
bool imu_fault = false;
bool oled_fault = false;
bool lmotor_fault = false;
bool rmotor_fault = false;
bool has_fault = false;

bool wifi_fault = false;
bool run_telemetry = true;

button_hw start_prod(1);
button_hw btn_forward(13);
button_hw btn_backward(28);

// hw motor instances
motor_hw left_motor(constants::left_motor_ln1_pin, constants::left_motor_ln2_pin, constants::left_motor_ena_pin);

int main() {
    stdio_init_all();
    sleep_ms(2000); // give time to connect to serial console
    
    printf("\n\n=== PROGRAM START ===\n");
    fflush(stdout);

    // Initialise the Wi-Fi chip
    printf("Initializing Wi-Fi...\n");
    fflush(stdout);

    if (telemetry_chip_init()) {
        std::printf("Wi-Fi init failed\n");
        wifi_fault = true;
    }
    printf("Wi-Fi initialized.\n");
    fflush(stdout);

    // turn on the Pico W LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);

    sleep_ms(1000);

    if (imu_hw_init()) {
        printf("[IMU] initialization failed!\n");
        imu_fault = true;
    }

    if (oled_hw_init()) {
        printf("[OLED] initialization failed!\n");
        oled_fault = true;
    }

    if (run_telemetry) {
        const int max_retries = 3;
        int attempt = 0;

        while (attempt < max_retries) {
            if (telemetry_init() == 0) {
                printf("[Telemetry] Initialized successfully.\n");
                wifi_fault = false;
                break;
            } else {
                wifi_fault = true;
                printf("[Telemetry] Initialization failed! Attempt %d/%d\n", attempt + 1, max_retries);
                attempt++;

                if (attempt < max_retries) {
                    printf("[Telemetry] Retrying...\n");
                    sleep_ms(1000); // wait 1 second before retrying
                }
            }
        }

        if (wifi_fault) {
            printf("[Telemetry] Failed to initialize telemetry after %d attempts.\n", max_retries);
        }
    } else {
        printf("[Telemetry] Skipping Wi-Fi connection at competition.\n");
        wifi_fault = true;
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
    oled_hw_print(0, 30, ("[Telemetry] " + std::string(wifi_fault ? "FAULT" : "READY")).c_str());
    oled_hw_print(0, 55, ("[System " + std::string(has_fault == 0 ? "READY" : "FAILED") + std::string("]")).c_str());
    oled_hw_update();

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
    absolute_time_t next_telemetry = make_timeout_time_ms(50); // 20Hz

    while (true) {
        cyw43_arch_poll();
        periodic();
        button_bindings();
        imu_hw_poll();
        update_display();
        
        // Send telemetry at regular intervals
        if (absolute_time_diff_us(get_absolute_time(), next_telemetry) <= 0) {
            float ax, ay, az, gx, gy, gz;
            imu_get_accel(&ax, &ay, &az);
            imu_get_gyro(&gx, &gy, &gz);
            float pitch = atan2(ay, sqrt(ax*ax + az*az)) * 180.0f / M_PI;
            float roll  = atan2(-ax, az) * 180.0f / M_PI; 
            float yaw   = 0;
            telemetry_send(pitch, roll, yaw, ax, ay, az, gx, gy, gz);
            next_telemetry = make_timeout_time_ms(50); // 20Hz
        }
        
        if (absolute_time_diff_us(get_absolute_time(), next_blink) <= 0) {
            led_on = !led_on;
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
            next_blink = make_timeout_time_ms(blink_ms);
        }
        
        sleep_ms(int(constants::dt * 1000));
    }
}

void button_bindings() {
    start_prod.update();
    btn_forward.update();
    btn_backward.update();

    if (start_prod.is_pressed()) {
        left_motor.forward(1000);
    }

    if (start_prod.just_released()) {
        left_motor.stop();
    }

    if (btn_forward.just_pressed()) {
        if (current_display == DISPLAY_STATUS) {
            current_display = DISPLAY_IMU_ANGLES;
        } else if (current_display == DISPLAY_IMU_ANGLES) {
            current_display = DISPLAY_IMU_ROC;
        } else if (current_display == DISPLAY_IMU_ROC) {
            current_display = DISPLAY_STATUS;
        }
    }
}

void update_display() {
    static absolute_time_t last_update = nil_time;
    
    if (!is_nil_time(last_update)) {
        int64_t elapsed_us = absolute_time_diff_us(last_update, get_absolute_time());
        if (elapsed_us < 100000) {
            return;
        }
    }
    last_update = get_absolute_time();
    
    oled_hw_clear();
    
    if (current_display == DISPLAY_STATUS) {
        oled_hw_clear();
        oled_hw_print(0, 0, "Nebula Runner [Alpha]");
        oled_hw_print(0, 20, ("[IMU] " + std::string(imu_fault ? "FAULT" : "READY")).c_str());
        oled_hw_print(0, 30, ("[Telemetry] " + std::string(wifi_fault ? (run_telemetry ? "FAULT" : "DISABLED") : "READY")).c_str());
        oled_hw_print(0, 55, ("[System " + std::string(has_fault == 0 ? "READY" : "FAILED") + std::string("]")).c_str());
    } else if (current_display == DISPLAY_IMU_ANGLES) {
        oled_hw_print(0, 0, "IMU Angles:");
        
        char buf[32];
        snprintf(buf, sizeof(buf), "Pitch: %.1f", pitch);
        oled_hw_print(0, 20, buf);
        
        snprintf(buf, sizeof(buf), "Roll:  %.1f", roll);
        oled_hw_print(0, 30, buf);
        
        snprintf(buf, sizeof(buf), "Yaw:   %.1f", yaw);
        oled_hw_print(0, 45, buf);
    } else if (current_display == DISPLAY_IMU_ROC) {
        float ax, ay, az, gx, gy, gz;
        imu_get_accel(&ax, &ay, &az);
        imu_get_gyro(&gx, &gy, &gz);
        
        oled_hw_print(0, 0, "IMU Live Data:");
        
        char buf[32];
        snprintf(buf, sizeof(buf), "Ax:%.2f Ay:%.2f", ax, ay);
        oled_hw_print(0, 20, buf);
        
        snprintf(buf, sizeof(buf), "Az:%.2f", az);
        oled_hw_print(0, 30, buf);
        
        snprintf(buf, sizeof(buf), "Gx:%.1f Gy:%.1f", gx, gy);
        oled_hw_print(0, 45, buf);
    }
    
    oled_hw_update();
}