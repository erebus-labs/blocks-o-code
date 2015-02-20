#!/usr/bin/python

import Adafruit_BBIO.GPIO as GPIO
from Adafruit_BBIO.SPI import SPI
import time

pause = 0.1

chipSelect = "P9_12"
GPIO.setup(chipSelect, GPIO.OUT)
GPIO.output(chipSelect, GPIO.HIGH)

sclk = "P9_11"
GPIO.setup(sclk, GPIO.IN)

spi = SPI(0,0)
spi.mode = 0

spi.msh = 500000
spi.open(0,0)

# global count
# count = 0;
#
# def callback_function_print(input_pin):
#   count = count + 1
#   print "Input on pin", input_pin
#
# GPIO.add_event_detect(sclk, GPIO.BOTH, callback=callback_function_print)

def spi_write(num):
    print "Writing " + num + "..."
    GPIO.output(chipSelect, GPIO.LOW)
    print str(spi.xfer2([int(num)]))
    time.sleep(pause)
    # print str(count) + " messages"
    GPIO.output(chipSelect, GPIO.HIGH)
    print "Wrote " + num + "!"

stop = False
while True != stop:
    response = str(raw_input("enter num to send over spi, x to exit: "))
    if response != "x":
        spi_write(response)
    else:
        stop = True

print "Thanks."
# spi.close()
