#!/usr/bin/env python

from time import sleep
import smbus
from filter import ABCFilter

class RGBFilter(ABCFilter):
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

    def __init__(self):
        ABCFilter.__init__(self)
        self.i2c = smbus.SMBus(self.I2C_CHANNEL)

        self.oldcolor = (255, 255, 255)
        self.setRGB(255, 255, 255)

    def action(self, arg):
        color = int(arg) % 7
        self.transition(self.ROYGBIV[color], holdtime=2, transittime=0.5)
        self.oldcolor = self.ROYGBIV[color]

    def transition(self, c, holdtime = 0.25, transittime=0.1, steptime = 0.01):
        nsteps = transittime / steptime
        for y in range(1, int(nsteps)):
            red, green, blue = self.interpolate(self.oldcolor, c, float(y)/nsteps)
            self.setRGB(red, green, blue)
            sleep(steptime)
        red, green, blue = (c[0], c[1], c[2])
        self.setRGB(red, green, blue)
        sleep(holdtime)

    def setRGB(self, red, green, blue):
        self.i2c.write_byte_data(self.RGB_ADDR, 0, 0)
        self.i2c.write_byte_data(self.RGB_ADDR, 1, 0)
        self.i2c.write_byte_data(self.RGB_ADDR, 0x08, 0xaa)
        self.i2c.write_byte_data(self.RGB_ADDR, 4, red)
        self.i2c.write_byte_data(self.RGB_ADDR, 3, green)
        self.i2c.write_byte_data(self.RGB_ADDR, 2, blue)

    @staticmethod
    def interpolate(c1, c2, percent):
        red = c1[0] - ((c1[0]-c2[0]) * percent)
        green = c1[1] - ((c1[1]-c2[1]) * percent)
        blue = c1[2] - ((c1[2]-c2[2]) * percent)
        return int(red), int(green), int(blue)


if __name__=="__main__":
    f = RGBFilter()
    f._run()
