#include "hardware/button_hw.h"

button_hw::button_hw(uint pin, bool pullUp)
    : _pin(pin), _pullUp(pullUp), _lastState(false), _currentState(false) {

    gpio_init(_pin);
    gpio_set_dir(_pin, GPIO_IN);

    if (_pullUp)
        gpio_pull_up(_pin);
    else
        gpio_pull_down(_pin);

    // initialize state
    _currentState = gpio_get(_pin);
    _lastState = _currentState;
}

bool button_hw::is_pressed() {
    
    return _pullUp ? !gpio_get(_pin) : gpio_get(_pin);
}

void button_hw::update() {
    _lastState = _currentState;
    _currentState = gpio_get(_pin);
}

bool button_hw::just_pressed() {
    if (_pullUp)
        return (_lastState == 1 && _currentState == 0);
    else
        return (_lastState == 0 && _currentState == 1);
}

bool button_hw::just_released() {
    if (_pullUp)
        return (_lastState == 0 && _currentState == 1);
    else
        return (_lastState == 1 && _currentState == 0);
}
