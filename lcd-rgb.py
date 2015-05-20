#!/usr/bin/env python

from time import sleep
import smbus

I2C_CHANNEL = 1
RGB_ADDR = 0x62

ROYGBIV = [
    (255, 0, 0),
    (255, 63, 0),
    (255, 255, 0),
    (0, 255, 0),
    (0, 0, 255),
    (63, 0, 255),
    (127, 0, 255)
]

def setRGB(red ,green, blue):
    i2c.write_byte_data(RGB_ADDR, 0, 0)
    i2c.write_byte_data(RGB_ADDR, 1, 0)
    i2c.write_byte_data(RGB_ADDR, 0x08, 0xaa)
    i2c.write_byte_data(RGB_ADDR, 4, red)
    i2c.write_byte_data(RGB_ADDR, 3, green)
    i2c.write_byte_data(RGB_ADDR, 2, blue)

def interpolate(c1, c2, percent):
    red = c1[0] - ((c1[0]-c2[0]) * percent)
    green = c1[1] - ((c1[1]-c2[1]) * percent)
    blue = c1[2] - ((c1[2]-c2[2]) * percent)
    return int(red), int(green), int(blue)

def transition(c, oldval, holdtime = 0.25, transittime=0.1, steptime = 0.01):
    nsteps = transittime / steptime
    for y in range(1, int(nsteps)):
        red, green, blue = interpolate(oldval, c, float(y)/nsteps)
        setRGB(red, green, blue)
        sleep(steptime)
    red, green, blue = (c[0], c[1], c[2])
    setRGB(red, green, blue)
    sleep(holdtime)


if __name__=="__main__":
    i2c = smbus.SMBus(I2C_CHANNEL)
    old_c = (255, 255, 255)

    while True:
        for x in ROYGBIV:
            transition(x, old_c, holdtime=2, transittime=0.5)
            old_c = x
