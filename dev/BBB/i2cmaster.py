#a script for a-block'o-code
#Jacob Mickiewicz

from Adafruit_I2C import Adafruit_I2C

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
			return val # lex.reverse_mapping[val]

def enum(*sequential, **named):
    enums = dict(zip(sequential, range(len(sequential))), **named)
    reverse = dict((value, key) for key, value in enums.iteritems())
    enums['reverse_mapping'] = reverse
    return type('Enum', (), enums)

lex = enum(noblock=0,plus=1,minus=2,one=3,two=4,X=5,assign=6,ifblock=7,lessthan=8)

###################################################################
#start of run
###################################################################
for y in range(1,5):
    for x in range(0,4):
        z = I2cLex(x,y) #z will be returned value from i2c
        print lex.reverse_mapping[z],
    print
