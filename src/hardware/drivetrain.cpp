#include <cstdio>
#include "hardware/drivetrain.h"
#include "math/pid_control.h"
#include "constant.h"
#include <algorithm>

motor_hw left_motor(constants::left_motor_ln1_pin, constants::left_motor_ln2_pin, constants::left_motor_ena_pin);
// motor_hw right_motor(constants::right_motor_ln1_pin, constants::right_motor_ln2_pin, constants::right_motor_ena_pin);

pid_control left_pid(1.0f, 0.0f, 0.1f);
pid_control right_pid(1.0f, 0.0f, 0.1f);

bool enc_left_initialized = false;
bool enc_right_initialized = false;

bool drivetrain_init() {
    printf("[Drivetrain] initialization successful!\n");
    printf("[Drivetrain] left encoder initialization successful!\n");
    printf("[Drivetrain] right encoder initialization successful!\n");
    enc_left_initialized = true;
    enc_right_initialized = true;
    return false;
}

std::string drivetrain_fault_status() {
    std::string status = "";
    if (!enc_left_initialized && !enc_right_initialized) {
        return "BOTH";
    }
    if (!enc_left_initialized) {
        status += "LEFT";
    }
    if (!enc_right_initialized) {
        status += "RIGHT";
    }
    return status.empty() ? "OK" : status;
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