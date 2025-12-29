#include "hardware/motor_hw.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <algorithm>

motor_hw::motor_hw(uint8_t pin_ln1, uint8_t pin_ln2, uint8_t pin_ena)
    : ln1_pin(pin_ln1), ln2_pin(pin_ln2), ena_pin(pin_ena), pwm_slice(0)
{
    init();
}

void motor_hw::init() {
    gpio_init(ln1_pin);
    gpio_set_dir(ln1_pin, GPIO_OUT);

    gpio_init(ln2_pin);
    gpio_set_dir(ln2_pin, GPIO_OUT);

    gpio_set_function(ena_pin, GPIO_FUNC_PWM);
    pwm_slice = pwm_gpio_to_slice_num(ena_pin);

    pwm_set_clkdiv(pwm_slice, 125.0f);
    pwm_set_wrap(pwm_slice, 1000);
    pwm_set_chan_level(pwm_slice, pwm_gpio_to_channel(ena_pin), 0);

    pwm_set_enabled(pwm_slice, true);

    stop();
}

void motor_hw::pwm_set_duty(uint16_t duty) {
    duty = std::min<uint16_t>(duty, 1000);
    pwm_set_chan_level(pwm_slice, pwm_gpio_to_channel(ena_pin), duty);
}

void motor_hw::run(int16_t speed) {
    bool forward = abs(speed) == speed;
    if (forward) {
        gpio_put(ln1_pin, 1);
        gpio_put(ln2_pin, 0);
    } else {
        gpio_put(ln1_pin, 0);
        gpio_put(ln2_pin, 1);
    }
    pwm_set_duty(abs(speed));
}

void motor_hw::stop() {
    gpio_put(ln1_pin, 0);
    gpio_put(ln2_pin, 0);
    pwm_set_duty(0);
}

void motor_hw::test() {
    run(800);
    sleep_ms(1500);

    stop();
    sleep_ms(500);

    run(-800);
    sleep_ms(1500);

    stop();
}
