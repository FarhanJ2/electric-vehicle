#include "trapezoid_profile.h"
#include <cmath>
#include <algorithm>

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
    curr_position = direct_position(curr_position);
    curr_velocity = direct_velocity(curr_velocity);
    goal_position = direct_position(goal_position);
    goal_velocity = direct_velocity(goal_velocity);

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
    return direct_position(result_pos);
}

float trapezoid_profile::get_output_vel() {
    return direct_velocity(result_vel);
}


float trapezoid_profile::time_until_left(float target) {
    float pos = curr_position * direction;
    float vel = curr_velocity * direction;

    float end_accel = max_acceleration * direction;
    float end_decel = end_full_speed * direction - end_accel;

    if (target < pos) {
        end_accel = -end_accel;
        end_full_speed = -end_full_speed;
        vel = -vel;
    }

    end_accel = std::max(end_accel, 0.0f);
    end_full_speed = std::max(end_full_speed, 0.0f);

    const float accel = max_acceleration;
    const float decel = -max_acceleration;

    float dist_to_target = std::abs(target - pos);
    if (dist_to_target < 0.001f) {
        return 0.0f;
    }

    float accel_dist = vel * end_accel + 0.5f * accel * end_accel * end_accel;
    float decel_vel;

    if (end_accel > 0) {
        decel_vel = std::sqrt(std::abs(vel * vel + 2.0f * accel * accel_dist));
    } else {
        decel_vel = vel;
    }

    float full_speed_dist = max_velocity * end_full_speed;
    float decel_dist;
    if (accel_dist > dist_to_target) {
        accel_dist = dist_to_target;
        full_speed_dist = 0.0f;
        decel_dist = 0.0f;
    } else if (accel_dist + full_speed_dist > dist_to_target) {
        full_speed_dist = dist_to_target - accel_dist;
        decel_dist = 0.0f;
    } else {
        decel_dist = dist_to_target - full_speed_dist - accel_dist;
    }

    float accel_time = (-vel + std::sqrt(std::abs(vel * vel + 2.0f * accel * accel_dist))) / accel;
    float decel_time = (-decel_vel * std::sqrt(std::abs(decel_vel * decel_vel + 2.0f * decel * decel_dist))) / decel;
    float full_speed_time = full_speed_dist / max_velocity;
    return accel_time + decel_time + full_speed_time;
}
