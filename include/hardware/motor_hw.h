#pragma once
#include <cstdint>

class motor_hw {
public:
    motor_hw(uint8_t pin_ln1, uint8_t pin_ln2, uint8_t pin_ena);

    void init();
    void run(int16_t speed);   // -1000–1000
    void stop();
    void test();

private:
    uint8_t ln1_pin;
    uint8_t ln2_pin;
    uint8_t ena_pin;
    uint8_t pwm_slice;

    void pwm_set_duty(uint16_t duty);
};
