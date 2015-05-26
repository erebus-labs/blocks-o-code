#!/usr/bin/env python

from time import sleep
from filter import ABCFilter
import Adafruit_BBIO.GPIO as GPIO

class MotorFilter(ABCFilter):
    def __init__(self, outpin = 'P8_7'):
        ABCFilter.__init__(self)
        self.pin = outpin
        GPIO.setup(self.pin, GPIO.OUT)
        GPIO.output(self.pin, GPIO.LOW)

    def action(self, arg):
        num = float(arg)
        num = 2.5 if num > 2.5 else num
        GPIO.output(self.pin, GPIO.HIGH)
        sleep(num)
        GPIO.output(self.pin, GPIO.LOW)
        sleep(0.1)

if __name__=="__main__":
    f = MotorFilter()
    f._run()
