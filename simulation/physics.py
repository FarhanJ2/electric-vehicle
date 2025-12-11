import vehicle_params as c

def motor_torque(voltage, velocity):
    # convert linear velocity to angular vel
    omega = velocity / c.wheel_radius

    back_emf = c.motor_kV * omega
    current = (voltage - back_emf) / c.motor_R

    return c.motor_kT * current * c.efficiency


def forces(voltage, velocity):
    torque = motor_torque(voltage, velocity)
    F_motor = torque / c.wheel_radius

    F_rr = c.C_rr * c.mass * c.gravity
    F_k = c.mu_k * c.mass * c.gravity
    F_static = c.mu_s * c.mass * c.gravity

    return F_motor, F_rr, F_k, F_static


def acceleration(voltage, velocity):
    F_motor, F_rr, F_k, F_static = forces(voltage, velocity)

    # static friction
    if abs(velocity) < 0.0001 and abs(F_motor) < F_static:
        return 0

    # kinetic friction
    F_total = F_motor - F_k - F_rr
    a = F_total / c.mass

    return a
