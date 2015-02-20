#!/usr/bin/python

import Adafruit_BBIO.GPIO as GPIO
import time

pin = "P8_12"
GPIO.setup(pin, GPIO.OUT)

while True:
    GPIO.output(pin, GPIO.HIGH)
    time.sleep(1)
    GPIO.output(pin, GPIO.LOW)
    time.sleep(1)
