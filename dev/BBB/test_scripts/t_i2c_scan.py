# Testbench to test the i2c read limits and collect data for A Block of Code


from Adafruit_I2C import Adafruit_I2C
import time
import sys
import csv

reserved = [(0,0), (0,1), (0,2),
			(5,4), (5,5), (5,6), (5,7),
			(7,8), (7,9), (7,10), (7,11), (7,12), (7,13), (7,14), (7,15)]

def addrFvect(x,y):
	if y>16 and x>8:
		return (0xff)
	elif (x,y) in reserved:
		return (0xff)
	else:
		return (y<<3|x)


def I2cLex(x ,y):
	address = addrFvect(x,y)
	if(address == 0xff):
		return lex.noblock
	else:
		i2caddr = Adafruit_I2C(address, busnum=i2c_busnum)
		val = i2caddr.readU8(0)
		if (val == -1):
			return lex.noblock
		else:
			return val # lex.reverse_mapping[val]

def enum(*sequential, **named):
    enums = dict(zip(sequential, range(len(sequential))), **named)
    reverse = dict((value, key) for key, value in enums.iteritems())
    enums['reverse_mapping'] = reverse
    return type('Enum', (), enums)

lex = enum(noblock=0,plus=1,minus=2,one=3,two=4,X=5,assign=6,ifblock=7,lessthan=8)

scan_list = []
for scan in range(10):
	scan_start = time.time()
	for y in range(0,15):
		for x in range(0,8):
			block_func = I2cLex(x,y)
			print block_func,
			time.sleep(0.1)
		print
	print '++++++++++++++++++++'
	scan_end = time.time()
	scan_list.append(scan_end - scan_start)
print 'Average Time per Scan', sum(scan_list) / len(scan_list)
