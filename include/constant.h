#pragma once

#include <cstdint>

extern bool run_telemetry;

namespace constants {
    inline constexpr uint8_t left_motor_ln1_pin = 7;
    inline constexpr uint8_t left_motor_ln2_pin = 6;
    inline constexpr uint8_t left_motor_ena_pin = 8;

    inline constexpr float dt = 0.01f; // 10ms loop
}

