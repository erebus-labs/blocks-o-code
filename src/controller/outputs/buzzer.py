#!/usr/bin/env python

from time import sleep
from filter import ABCFilter
import Adafruit_BBIO.GPIO as GPIO

class BuzzerFilter(ABCFilter):
    def __init__(self, outpin = 'P8_8'):
        ABCFilter.__init__(self)
        self.pin = outpin
        GPIO.setup(self.pin, GPIO.OUT)
        GPIO.output(self.pin, GPIO.LOW)

    def action(self, arg):
        num = int(arg)
        num = 0 if num < 0 else num
        num = 5 if num > 5 else num
        for x in range(0, num):
            GPIO.output(self.pin, GPIO.HIGH)
            sleep(0.05)
            GPIO.output(self.pin, GPIO.LOW)
            sleep(0.25)
        sleep(1)

if __name__=="__main__":
    f = BuzzerFilter()
    f._run()
