#pragma once

#include "hardware/motor_hw.h"
#include <cstdint>

void drivetrain_init();
void drivetrain_test();
void drivetrain_tank_drive(float left_speed, float right_speed);
void drivetrain_arc_drive(float forward_speed, float turn_rate);
void drivetrain_stop();