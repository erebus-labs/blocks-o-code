#!/usr/bin/python

import Adafruit_BBIO.GPIO as GPIO
# from Adafruit_BBIO.SPI import SPI
import time

pause = 0.02

# i2cClock 19
# i2cData 20
# spiClock 21
# slaveSelect 22
# slaveDataOut 23
# reset? 24

spiClock = "P9_11"
slaveSelect = "P9_12"
MOSI = "P9_23"

global dataIn

def i2c_addr(x, y):
    return (int(x) & 7) | ((int(y) & 15) << 3)

def setupHandshake():
    GPIO.setup(spiClock, GPIO.OUT)
    GPIO.setup(MOSI, GPIO.OUT)
    GPIO.setup(slaveSelect, GPIO.IN)

def setupSpi():
    # SPI mode 0: CPOL = 0, CPHA = 0. Slave Select (SS) lines idle low.
    GPIO.setup(spiClock, GPIO.OUT)
    GPIO.setup(slaveSelect, GPIO.OUT)
    GPIO.setup(MOSI, GPIO.OUT)

def handshake():
    # print "START"
    GPIO.wait_for_edge(slaveSelect, GPIO.RISING)
    GPIO.output(MOSI, GPIO.HIGH)
    GPIO.output(spiClock, GPIO.HIGH)

    # print "A"
    GPIO.wait_for_edge(slaveSelect, GPIO.FALLING)
    GPIO.output(MOSI, GPIO.LOW)
    GPIO.output(spiClock, GPIO.LOW)

    # print "B"
    GPIO.wait_for_edge(slaveSelect, GPIO.RISING)
    GPIO.setup(slaveSelect, GPIO.OUT)
    GPIO.output(slaveSelect, GPIO.LOW)

    # print "C"
    time.sleep(pause)
    GPIO.output(spiClock, GPIO.HIGH)

    # print "END"
    time.sleep(pause)
    GPIO.output(spiClock, GPIO.LOW)


def spi_transfer(number):
    print "Writing " + "{0:b}".format(number) + "..."
    GPIO.output(slaveSelect, GPIO.HIGH)
    num = int(number)
    dataIn = 0

    for i in xrange(7, -1, -1):
        level = (num & (1 << i)) >> i
        # print "Bit " + str(i) + ": " + str(level)
        if (level == 1):
            GPIO.output(MOSI, GPIO.HIGH)
        else:
            GPIO.output(MOSI, GPIO.LOW)

        # pulse clock
        GPIO.output(spiClock, GPIO.HIGH)
        GPIO.output(spiClock, GPIO.LOW)


    # time.sleep(pause)
    GPIO.output(slaveSelect, GPIO.LOW)
    # print "Wrote " + str(number) + "!"
    return dataIn

#while True:
setupHandshake()
print "Starting handshake..."
handshake()
print "Handshake completed."
    # time.sleep(pause)
setupSpi()
spi_transfer(i2c_addr(0, 0))
