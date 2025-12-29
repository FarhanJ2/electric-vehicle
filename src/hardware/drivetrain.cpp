#include "hardware/drivetrain.h"
#include "constant.h"
#include <algorithm>

motor_hw left_motor(constants::left_motor_ln1_pin, constants::left_motor_ln2_pin, constants::left_motor_ena_pin);
// motor_hw right_motor(constants::right_motor_ln1_pin, constants::right_motor_ln2_pin, constants::right_motor_ena_pin);



void drivetrain_init() {
    
}

void drivetrain_test() {
    left_motor.test();
    // right_motor.test();
}

void drivetrain_tank_drive(float left_speed, float right_speed) {
    // left_speed and right_speed are in range -1.0 to 1.0
    left_speed = std::clamp(left_speed, -1.0f, 1.0f);
    right_speed = std::clamp(right_speed, -1.0f, 1.0f);
    left_motor.run(static_cast<int16_t>(left_speed * 1000.0f));
    // right_motor.run(static_cast<int16_t>(right_speed * 1000.0f));
}

void drivetrain_stop() {
    left_motor.stop();
    // right_motor.stop();
}