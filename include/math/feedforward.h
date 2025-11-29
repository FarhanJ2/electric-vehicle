#pragma once

#include "constant.h"

class feedforward {
    private:
        float kS; // Static gain
        float kV; // Velocity gain
        float kA; // Acceleration gain

    public:
        feedforward(float s, float v, float a)
            : kS(s), kV(v), kA(a) {}

        double compute(double velocity, double acceleration, double dt) {
            return (kS * (velocity != 0 ? (velocity > 0 ? 1 : -1) : 0)) + (kV * velocity) + (kA * acceleration);
        }

        double compute(double velocity, double acceleration) {
            float dt = constants::dt;
            return (kS * (velocity != 0 ? (velocity > 0 ? 1 : -1) : 0)) + (kV * velocity) + (kA * acceleration);
        }
};
