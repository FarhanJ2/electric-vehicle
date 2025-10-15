#pragma once

#include <constants.h>

class pid_control {
    private:
        float kp;
        float ki;
        float kd;

        float previous_error;
        float integral;

    public:
        pid_control(float p, float i, float d)
            : kp(p), ki(i), kd(d), previous_error(0), integral(0) {}

        double compute(double setpoint, double measured_value, double dt) {
            double error = setpoint - measured_value;
            integral += error * dt;
            double derivative = (error - previous_error) / dt;
            previous_error = error;

            return (kp * error) + (ki * integral) + (kd * derivative);
        }

        double compute(double setpoint, double measured_value) {
            float dt = constants::dt;
            double error = setpoint - measured_value;
            integral += error * dt;
            double derivative = (error - previous_error) / dt;
            previous_error = error;

            return (kp * error) + (ki * integral) + (kd * derivative);
        }

        void reset() {
            previous_error = 0;
            integral = 0;
        }
};