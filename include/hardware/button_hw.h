#pragma once
#include "pico/stdlib.h"

class button_hw {
public:
    button_hw(uint pin, bool pullUp = true);

    bool is_pressed();
    void update();
    bool just_pressed();
    bool just_released();

private:
    uint _pin;
    bool _pullUp;

    bool _lastState;
    bool _currentState;
};
