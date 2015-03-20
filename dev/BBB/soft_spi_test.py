#!/usr/bin/python

import Adafruit_BBIO.GPIO as GPIO
# from Adafruit_BBIO.SPI import SPI
import time

pause = 1

spiClock = "P9_11"
slaveSelect = "P9_12"
MISO = "P9_21"
MOSI = "P9_22"

global dataIn

def setupSpi():
    GPIO.setup(spiClock, GPIO.OUT)
    GPIO.output(spiClock, GPIO.LOW)
    GPIO.setup(slaveSelect, GPIO.OUT)
    GPIO.output(slaveSelect, GPIO.HIGH)
    GPIO.setup(MOSI, GPIO.OUT)
    GPIO.output(MOSI, GPIO.LOW)
    GPIO.setup(MISO, GPIO.IN)

def spi_transfer(number):
    print "Writing " + number + "..."
    GPIO.output(slaveSelect, GPIO.LOW)
    num = int(number)
    dataIn = 0

    for i in xrange(7, -1, -1):
        level = (num & (1 << i)) >> i
        print "Bit " + str(i) + ": " + str(level)
        if (level == 1):
            GPIO.output(MOSI, GPIO.HIGH)
        else:
            GPIO.output(MOSI, GPIO.LOW)

        GPIO.output(spiClock, GPIO.HIGH)

        if GPIO.input(MISO):
            dataIn |= (1 << i)
        else:
            dataIn &= (255 ^ (1 << i))

        GPIO.output(spiClock, GPIO.LOW)

    # time.sleep(pause)
    # print str(count) + " messages"
    GPIO.output(slaveSelect, GPIO.HIGH)
    print "Wrote " + number + "!"

    return dataIn


setupSpi()

stop = False
while True != stop:
    response = str(raw_input("enter num to send over spi, x to exit: "))
    if response != "x":
        rcvd = str(spi_transfer(response))
        print "Received: ", rcvd
    else:
        stop = True

print "Thanks."
# spi.close()
