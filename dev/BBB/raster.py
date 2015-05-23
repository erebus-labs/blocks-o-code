#!/usr/bin/python

import Adafruit_BBIO.GPIO as GPIO
from Adafruit_I2C import Adafruit_I2C
import time
from sys import argv

delay = 0.75
pause = 0.02

arg = False
if len(argv)>1:
	file, arg = argv
	if arg=='-c':
		arg = True

# i2cClock 19
# i2cData 20
# spiClock 21
# slaveSelect 22
# slaveDataOut 23
# reset? 24

spiClock = "P9_11"
slaveSelect = "P9_12"
MOSI = "P9_23"

# open('/sys/devices/bone_capemgr.9/slots', 'r+').write('BB-I2C1')

def addrFvect(x,y):
	if((y<16)&(x<8)):
		return (y<<3|x)
	else:
		return (0xff)

def I2cLex(x, y, reg):
	address = addrFvect(x,y)
	if(address == 0xff):
		return lex.noblock
	else:
		before = time.time()
		i2caddr = Adafruit_I2C(address, busnum=2)
		after = time.time()

		if (after-before) > 0.4:
			print "Stall"

		val = i2caddr.readU8(reg)
		# print reg, x, y, functions[val & 0b00111111]
		if (val == -1):
			return 0 #lex.noblock
		else:
			return val # lex.reverse_mapping[val]

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
    if not GPIO.input(slaveSelect):
        # print "slave low"
        GPIO.output(MOSI, GPIO.LOW)
        GPIO.output(spiClock, GPIO.LOW)
    else:
        # print "wait for slave low"
        GPIO.wait_for_edge(slaveSelect, GPIO.FALLING)
        GPIO.output(MOSI, GPIO.LOW)
        GPIO.output(spiClock, GPIO.LOW)

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

def enum(*sequential, **named):
    enums = dict(zip(sequential, range(len(sequential))), **named)
    reverse = dict((value, key) for key, value in enums.iteritems())
    enums['reverse_mapping'] = reverse
    return type('Enum', (), enums)

lex = enum(noblock=0,plus=1,minus=2,one=3,two=4,X=5,assign=6,ifblock=7,lessthan=8)

functions = {-1: 'NOP', 1: "A", 2: ":", 3: "3", 4:"p", 42:"Life", 63:"ERR"}

class Block(object):
    """asdf
    """

    def __init__(self, new_x, new_y, block_data):
        self.x = new_x
        self.y = new_y
        self.data = int(block_data)
        self.func = functions[self.data & 0b00111111]
        self.right = (self.data & 0b01000000) > 0
        self.above = (self.data & 0b10000000) > 0

    def __repr__(self):
        desc = self.func + ":"
        desc += ' R' if self.right else ' r'
        desc += ' A' if self.above else ' a'
        return desc

    def __eq__(self, other):
        return isinstance(other, self.__class__) and self.data==other.data

    def __ne__(self, other):
        return not self==other

    def __cmp__(self, other):
        return self==other

    # def __hash__(self):
        # print 'hash'
        # return hash(self.data, self.func)



def placeBlock(x, y, new_block):

    if y > len(topo):
        row = []
        row.append(new_block)
        topo.append(row)
    else:
        row = topo[y-1]
        if x > (len(row) - 1):
            row.append(new_block)
        else:
            # block exists already
            old_block = row[x]
            row[x] = new_block
            # existing = True
            # new_block = old_block
            # print 'existing:', old_block
            # print 'new', new_block

def scan(x, y):
    new_block = I2cLex(x, y, 0)     # read from i2c line (0)
    # print x, y, new_block
    if new_block == 0:
        return 0

    time.sleep(0.05)
    new_block = Block(x, y, new_block)
    placeBlock(x, y, new_block)

    if x == 0 and new_block.above == True:
        # wait = 10
        read_block = scan(x, y+1)
        if read_block == 0: #and wait > 0:
			new_block = I2cLex(x, y, 1)      # read current block - sending up (1)
			new_block = Block(x, y, new_block)
			placeBlock(x, y, new_block)
			time.sleep(delay)
			read_block = scan(x, y+1)
			# time.sleep(0.01)
			# wait -= 1

    if new_block.right == True:
        # wait = 10
        read_block = scan(x+1, y)
        if read_block == 0: #and wait > 0:
			new_block = I2cLex(x, y, 2)      # read current block - sending right (2)
			new_block = Block(x, y, new_block)
			placeBlock(x, y, new_block)
			time.sleep(delay)
			read_block = scan(x+1, y)
			# time.sleep(0.01)
			# wait -= 1

    # print 'found block:', new_block
    return 1

def showTopo(topology):
    print '++++++++++++++++++++++++++'
    for row in reversed(topology):
        for block in row:
            if block != 0:
                print '[', block, ']',
        print
    print '++++++++++++++++++++++++++'

setupHandshake()
print "Starting handshake..."
handshake()
print "Handshake completed."
# time.sleep(pause)
setupSpi()
spi_transfer(i2c_addr(0, 0))

go = True
old = []
while go:
	time.sleep(delay)
	topo = []
	block = scan(0, 1)

	if block==1 and len(topo)>0 and topo!=old:
		showTopo(topo)
		old = topo

	if not arg:
		go = False

# 00 => No Neighbor / No Changes
# 01 => New Right Block
# 10 => New Above Block
# 11 => New Above and Right Blocks
