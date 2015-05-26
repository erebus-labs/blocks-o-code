# abc_local.py

import Adafruit_BBIO.GPIO as GPIO
from time import sleep

class ABC_Local_Bus(object):
    """docstring for ABC_Handshake"""
    def __init__(self, spiClockPin="P9_11", slaveSelectPin="P9_12", mosiPin="P9_23", debugFlag=False):
        super(ABC_Local_Bus, self).__init__()
        self.spiClock    = spiClockPin
        self.slaveSelect = slaveSelectPin
        self.mosi        = mosiPin
        self.debug       = debugFlag

    def setupHandshake(self):
        GPIO.setup(self.spiClock, GPIO.OUT)
        GPIO.setup(self.mosi, GPIO.OUT)
        GPIO.setup(self.slaveSelect, GPIO.IN)

    def setupSpi(self):
        # SPI mode 0: CPOL = 0, CPHA = 0. Slave Select (SS) lines idle low.
        GPIO.setup(self.spiClock, GPIO.OUT)
        GPIO.setup(self.slaveSelect, GPIO.OUT)
        GPIO.setup(self.mosi, GPIO.OUT)

    def handshake(self):
        if self.debug:
            print "Starting handshake..."

        pause = 0.02
        self.setupHandshake()

        if not GPIO.input(self.slaveSelect):
            if self.debug:
                print "slave low"
            GPIO.output(self.mosi, GPIO.LOW)
            GPIO.output(self.spiClock, GPIO.LOW)
        else:
            if self.debug:
                print "wait for slave low"
            GPIO.wait_for_edge(self.slaveSelect, GPIO.FALLING)
            GPIO.output(self.mosi, GPIO.LOW)
            GPIO.output(self.spiClock, GPIO.LOW)

        if self.debug:
            print "START"
        GPIO.wait_for_edge(self.slaveSelect, GPIO.RISING)
        GPIO.output(self.mosi, GPIO.HIGH)
        GPIO.output(self.spiClock, GPIO.HIGH)

        if self.debug:
            print "A"
        GPIO.wait_for_edge(self.slaveSelect, GPIO.FALLING)
        GPIO.output(self.mosi, GPIO.LOW)
        GPIO.output(self.spiClock, GPIO.LOW)

        if self.debug:
            print "B"
        GPIO.wait_for_edge(self.slaveSelect, GPIO.RISING)
        GPIO.setup(self.slaveSelect, GPIO.OUT)
        GPIO.output(self.slaveSelect, GPIO.LOW)

        if self.debug:
            print "C"
        sleep(pause)
        GPIO.output(self.spiClock, GPIO.HIGH)

        if self.debug:
            print "END"
        sleep(pause)
        GPIO.output(self.spiClock, GPIO.LOW)
        if self.debug:
            print "Handshake completed."


    def sendVector(self, vector):

        self.setupSpi()

        if self.debug:
            print "Writing " + "{0:b}".format(vector) + "..."
        GPIO.output(self.slaveSelect, GPIO.HIGH)
        num = int(vector)
        dataIn = 0

        for i in xrange(7, -1, -1):
            level = (num & (1 << i)) >> i
            if (level == 1):
                GPIO.output(self.mosi, GPIO.HIGH)
            else:
                GPIO.output(self.mosi, GPIO.LOW)

            # pulse clock
            GPIO.output(self.spiClock, GPIO.HIGH)
            GPIO.output(self.spiClock, GPIO.LOW)


        GPIO.output(self.slaveSelect, GPIO.LOW)
        if self.debug:
            print "Wrote " + str(vector) + "!"
        return dataIn
