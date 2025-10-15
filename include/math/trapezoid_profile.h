#pragma once

class trapezoid_profile {
    private:
        int direction;
        float max_velocity;
        float max_acceleration;
        float curr_position;
        float curr_velocity;
        
        float end_accel;
        float end_full_speed;
        float end_decel;

        float result_pos;
        float result_vel;

    public:
        trapezoid_profile(float max_vel, float max_accel);

        void compute(float t, 
            float target_position, float target_velocity,
            float goal_position, float goal_velocity);

        float get_output_pos();
        float get_output_vel();

        float time_until_left(float target);

        // Flip the sign of the position based on direction
        float direct_position(float currPosition) {
            return currPosition * direction;
        }

        // Flip the sign of the velocity based on direction
        float direct_velocity(float currVelocity) {
            return currVelocity * direction;
        }

};