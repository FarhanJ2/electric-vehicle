class PID:
    def __init__(self, kP, kI, kD):
        self.kP = kP
        self.kI = kI
        self.kD = kD
        self.i = 0
        self.last_error = 0

    def update(self, target, current, dt):
        err = target - current
        self.i += err * dt
        d = (err - self.last_error) / dt
        self.last_error = err
        return self.kP*err + self.kI*self.i + self.kD*d


pid = PID(100.0, 0.0, 0.2)

def voltage_command(time, pos, dt):
    v = pid.update(1.0, pos, dt)
    return max(min(v, 8.0), -8.0)
