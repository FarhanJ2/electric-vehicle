import matplotlib.pyplot as plt
import vehicle_params as p
import math

def simulate(voltage, duration=3.0, dt=0.01):
    t = 0
    x = 0
    v = 0

    time_log = []
    pos_log = []
    vel_log = []

    while t < duration:
        # linear to angular velocity
        omega = v / p.wheel_radius

        # motor equations
        back_emf = p.motor_kV * omega
        current = (voltage - back_emf) / p.motor_R

        torque = p.motor_kT * current

        # apply static friction sim
        if abs(torque) < p.static_friction_torque:
            torque = 0

        # motor torque to linear force
        force = torque / p.wheel_radius

        a = force / p.mass

        # integrate dynamics
        v += a * dt
        x += v * dt
        t += dt

        # Log
        time_log.append(t)
        pos_log.append(x)
        vel_log.append(v)

    return time_log, pos_log, vel_log


if __name__ == "__main__":
    time, pos, vel = simulate(voltage=p.max_voltage)

    plt.subplot(2,1,1)
    plt.plot(time, pos)
    plt.ylabel("Position (m)")

    plt.subplot(2,1,2)
    plt.plot(time, vel)
    plt.ylabel("Velocity (m/s)")
    plt.xlabel("Time (s)")

    plt.show()
