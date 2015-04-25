#a script for a-block'o-code
#Jacob Mickiewicz

from Adafruit_I2C import Adafruit_I2C

def addrFvect(x,y):
	if((y<16)&(x<8)):
		return (y<<3|x)
	else:
		return (0xff)

def I2cLex(z):
	#address = addrFvect(x,y)
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

lex = enum(noblock=" ",plus=1,minus=2,one=3,two=4,X=5,assign=6,ifblock=7,lessthan=8,"3"=117,x=36,":"=12,say=19)

###################################################################
#start of run
###################################################################

A = I2cLex(6) # will be returned value from i2c
B = I2cLex(32)
C = I2cLex(42)
D = I2cLex(4)
E = I2cLex(17)
print lex.reverse_mapping[D], lex.reverse_mapping[E], lex.reverse_mapping[B]
print lex.reverse_mapping[C], lex.reverse_mapping[A]
