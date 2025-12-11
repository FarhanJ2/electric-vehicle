import pygame
import time
from vehicle import Vehicle
from controller import voltage_command

pygame.init()
screen = pygame.display.set_mode((800, 200))
pygame.display.set_caption("SciOly Electric Vehicle Simulator")
clock = pygame.time.Clock()

car = Vehicle()

running = True
sim_time = 0
dt = 0.001  # 1000 Hz physics

while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

    # run physics faster than graphics
    for _ in range(10):  # 10 physics steps per frame
        voltage = voltage_command(sim_time, pos=car.x, dt=dt)
        car.update(voltage, dt)
        sim_time += dt

    # draw
    screen.fill((30, 30, 30))
    
    # convert meters to pixels (1m = 500px)
    px = int(car.x * 500)

    pygame.draw.rect(screen, (0, 200, 255), (50 + px, 80, 40, 20))

    pygame.display.flip()
    clock.tick(60)

pygame.quit()
