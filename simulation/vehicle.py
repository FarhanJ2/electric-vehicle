from physics import acceleration

class Vehicle:
    def __init__(self):
        self.x = 0
        self.v = 0

    def update(self, voltage, dt):
        a = acceleration(voltage, self.v)
        self.v += a * dt
        self.x += self.v * dt
