#include "hardware/motor_hw.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"

static uint slice;

// Set PWM duty in range 0–1000 (because wrap = 1000)
static inline void pwm_set_duty(uint16_t duty) {
    if (duty > 1000) duty = 1000;
    pwm_set_chan_level(slice, pwm_gpio_to_channel(MOTOR_PIN_ENA), duty);
}

void motor_hw_init() {
    // Direction pins
    gpio_init(MOTOR_PIN_LN_1);
    gpio_set_dir(MOTOR_PIN_LN_1, GPIO_OUT);

    gpio_init(MOTOR_PIN_LN_2);
    gpio_set_dir(MOTOR_PIN_LN_2, GPIO_OUT);

    // ENA pin for PWM
    gpio_set_function(MOTOR_PIN_ENA, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(MOTOR_PIN_ENA);

    // Set PWM to ~1 kHz (L298N safe)
    pwm_set_clkdiv(slice, 125.0f); // slow main clock
    pwm_set_wrap(slice, 1000);     // 0–1000 resolution
    pwm_set_chan_level(slice, pwm_gpio_to_channel(MOTOR_PIN_ENA), 0);

    pwm_set_enabled(slice, true);

    motor_stop();
}

void motor_forward(uint16_t speed) {
    gpio_put(MOTOR_PIN_LN_1, 1);
    gpio_put(MOTOR_PIN_LN_2, 0);
    pwm_set_duty(speed);    // 0–1000
}

void motor_backward(uint16_t speed) {
    gpio_put(MOTOR_PIN_LN_1, 0);
    gpio_put(MOTOR_PIN_LN_2, 1);
    pwm_set_duty(speed);
}

void motor_stop() {
    gpio_put(MOTOR_PIN_LN_1, 0);
    gpio_put(MOTOR_PIN_LN_2, 0);
    pwm_set_duty(0);
}

void motor_test() {
    motor_forward(800);  // strong forward
    sleep_ms(1500);

    motor_stop();
    sleep_ms(500);

    motor_backward(800); // strong backward
    sleep_ms(1500);

    motor_stop();
}
