#a script for a-block'o-code
#Jacob Mickiewicz

#!/usr/bin/python

import Adafruit_BBIO.GPIO as GPIO
from Adafruit_I2C import Adafruit_I2C
# from Adafruit_BBIO.SPI import SPI
import time

pause = 1

spiClock = "P9_11"
slaveSelect = "P9_12"
MOSI = "P9_23"
MISO = "P9_24"

global dataIn

def addrFvect(x,y):
	if((y<16)&(x<8)):
		return (y<<3|x)
	else:
		return (0xff)

def I2cLex(x ,y):
	address = addrFvect(x,y)
	if(address == 0xff):
		return lex.noblock
	else:
		i2caddr = Adafruit_I2C(address)
		val = i2caddr.readU8(0)
		if (val == -1):
			return lex.noblock
		else:
			return lex.reverse_mapping[val]

def enum(*sequential, **named):
    enums = dict(zip(sequential, range(len(sequential))), **named)
    reverse = dict((value, key) for key, value in enums.iteritems())
    enums['reverse_mapping'] = reverse
    return type('Enum', (), enums)

lex = enum(noblock=0,plus=1,minus=2,one=3,two=4,X=5,assign=6,ifblock=7,lessthan=8)

def i2c_addr(x, y):
    return (int(x) & 7) | ((int(y) & 15) << 3)

def setupHandshake():
    GPIO.setup(spiClock, GPIO.OUT)
    # GPIO.output(spiClock, GPIO.LOW)
    GPIO.setup(MOSI, GPIO.OUT)
    # GPIO.output(MOSI, GPIO.LOW)
    GPIO.setup(slaveSelect, GPIO.IN)

def handshake():
    GPIO.wait_for_edge(slaveSelect, GPIO.RISING)
    GPIO.output(MOSI, GPIO.HIGH)
    GPIO.output(spiClock, GPIO.HIGH)
    time.sleep(0.01)
    GPIO.output(spiClock, GPIO.LOW)
    GPIO.output(MOSI, GPIO.LOW)
    time.sleep(0.01)

    # GPIO.wait_for_edge(slaveSelect, GPIO.RISING)
    GPIO.output(spiClock, GPIO.HIGH)
    time.sleep(0.01)
    GPIO.output(spiClock, GPIO.LOW)
    time.sleep(0.01)

    GPIO.output(MOSI, GPIO.HIGH)
    GPIO.output(spiClock, GPIO.HIGH)
    GPIO.setup(slaveSelect, GPIO.OUT)
    GPIO.output(slaveSelect, GPIO.HIGH)
    time.sleep(0.01)
    GPIO.output(spiClock, GPIO.LOW)
    GPIO.output(MOSI, GPIO.LOW)
    time.sleep(0.01)


def setupSpi():
    GPIO.setup(spiClock, GPIO.OUT)
    # GPIO.output(spiClock, GPIO.LOW)
    GPIO.setup(slaveSelect, GPIO.OUT)
    # GPIO.output(slaveSelect, GPIO.HIGH)
    GPIO.setup(MOSI, GPIO.OUT)
    GPIO.setup(MISO, GPIO.IN)

def spi_transfer(number):
    print "Writing " + "{0:b}".format(number) + "..."
    GPIO.output(slaveSelect, GPIO.LOW)
    num = int(number)
    dataIn = 0

    for i in xrange(7, -1, -1):
        level = (num & (1 << i)) >> i
        # print "Bit " + str(i) + ": " + str(level)
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
    # print "Wrote " + str(number) + "!"
    return dataIn

while True:
    setupHandshake()
    print "Starting handshake..."
    handshake()
    print "Handshake completed."
    setupSpi()
    spi_transfer(i2c_addr(0, 0))

	###################################################################
	#start of run
	###################################################################
	for y in range(1,5):
		# line = ()
        for x in range(0,4):
		    # z = I2cLex(x,y) #z will be returned value from i2c
		    # line.append(I2cLex(x,y))
			print I2cLex(x,y), # lex.reverse_mapping[z],
		print

# stop = False
# while True != stop:
#     response = str(raw_input("Enter 'x, y' coord to send, q to quit: "))
#     if response != "q":
#         x, y = response.split(',')
#         rcvd = str(spi_transfer(i2c_addr(x, y)))
#         print "Received: ", rcvd
#     else:
#         stop = True

print "Thanks."
