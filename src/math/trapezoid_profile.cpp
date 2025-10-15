#include "trapezoid_profile.h"
#include <cmath>

trapezoid_profile::trapezoid_profile(float max_vel, float max_accel)
    : max_velocity(max_vel),
      max_acceleration(max_accel),
      curr_position(0),
      curr_velocity(0),
      direction(1) {}

void trapezoid_profile::compute(float t,
                                 float target_position, float target_velocity,
                                 float goal_position, float goal_velocity) {
    direction = (curr_position > goal_position) ? 1 : -1;
    curr_position = directPosition(curr_position);
    curr_velocity = directVelocity(curr_velocity);
    goal_position = directPosition(goal_position);
    goal_velocity = directVelocity(goal_velocity);

    if (curr_velocity > max_velocity) {
        curr_velocity = max_velocity;
    } else if (curr_velocity < -max_velocity) {
        curr_velocity = -max_velocity;
    }

    float cutoff_begin = curr_velocity / max_acceleration;
    float cutoff_dist_begin = cutoff_begin * cutoff_begin * max_acceleration / 2.0f;

    float cutoff_end = goal_velocity / max_acceleration;
    float cutoff_dist_end = cutoff_end * cutoff_end * max_acceleration / 2.0f;

    float full_trapezoid_dist = cutoff_dist_begin + (goal_position - curr_position) + cutoff_dist_end;
    float accel_time = max_velocity / max_acceleration;

    float full_speed_dist = full_trapezoid_dist - accel_time * accel_time * max_acceleration;
    if (full_speed_dist < 0) {
        accel_time = std::sqrt(full_trapezoid_dist / max_acceleration);
        full_speed_dist = 0.0f;
    }

    end_accel = accel_time - cutoff_begin;
    end_full_speed = end_accel + full_speed_dist / max_velocity;
    end_decel = end_full_speed + accel_time - cutoff_end;

    result_pos = curr_position;
    result_vel = curr_velocity;

    if (t < end_accel) {
        result_vel += t * max_acceleration;
        result_pos += (curr_velocity + t * max_acceleration / 2.0f) * t;
    } else if (t < end_full_speed) {
        result_vel = max_velocity;
        result_pos += (curr_velocity + end_accel * max_acceleration / 2.0f) * end_accel + max_velocity * (t - end_accel);
    } else if (t <= end_decel) {
        result_vel = goal_velocity + (end_decel - t) * max_acceleration;
        float time_left = end_decel - t;
        result_pos = goal_position - (goal_velocity + time_left * max_acceleration / 2.0f) * time_left;
    } else {
        result_pos = goal_position;
        result_vel = goal_velocity;
    }
}

float trapezoid_profile::get_output_pos() {
    return directPosition(result_pos);
}

float trapezoid_profile::get_output_vel() {
    return directVelocity(result_vel);
}


float trapezoid_profile::timeUntilLeft(float target) {
    float distance_left = target - curr_position;
    if (curr_velocity == 0) return INFINITY; // avoid divide by zero
    return distance_left / curr_velocity;
}
