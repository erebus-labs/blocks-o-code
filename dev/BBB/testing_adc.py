#!/usr/bin/python

from abc_local import ABC_Local_Bus
from abc_global import ABC_Global_Bus
import time
import sys


delay = 0.75

continuous = False
debug = False
stderr = True
if len(sys.argv)>1:
	if '-c' in sys.argv:
		continuous = True
	if '-debug' in sys.argv:
		debug = True
	if '-E' in sys.argv:
		stderr = False


functions = {
	-1: 'NOP',
	1: "A",
	2: " ",
	3: "4",
	4: "output 1 ",
	42:"Life",
	63:"ERR"
}

class Block(object):
    """docstring for Block"""

    def __init__(self, new_x, new_y, block_data):
        self.x = new_x
        self.y = new_y
        self.data = int(block_data)
        self.func = functions[self.data & 0b00111111]
        self.right = (self.data & 0b01000000) > 0
        self.above = (self.data & 0b10000000) > 0

    def __repr__(self):
		if debug:
			desc = '[ '
			desc += self.func
			desc += ' R' if self.right else ' r'
			desc += ' A' if self.above else ' a'
			desc += ' ]'
		else:
			desc = self.data
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


class Lexer(object):
	"""docstring for Lexer"""
	def __init__(self, global_bus):
		super(Lexer, self).__init__()
		self.bus = global_bus

	def placeBlock(self, x, y, new_block):

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

	def scan(self, x, y):
	    new_block = self.bus.readData(x, y)    # read from i2c line (0)
	    if debug:
			print x, y, new_block
	    if new_block == 0 or new_block == -1:
	        return 0

	    time.sleep(0.05)
	    new_block = Block(x, y, new_block)
	    self.placeBlock(x, y, new_block)

	    if x == 0 and new_block.above == True:
	        # wait = 10
	        read_block = self.scan(x, y+1)
	        if read_block == 0: #and wait > 0:
				if debug:
					print "Sending up"
				new_block = self.bus.sendVertical(x, y)      # read current block - sending up (1)
				new_block = Block(x, y, new_block)
				self.placeBlock(x, y, new_block)
				time.sleep(delay)
				read_block = self.scan(x, y+1)
				# time.sleep(0.01)
				# wait -= 1

	    if new_block.right == True:
	        # wait = 10
	        read_block = self.scan(x+1, y)
	        if read_block == 0: #and wait > 0:
				if debug:
					print "Sending right"
				new_block = self.bus.sendHorizontal(x, y)     # read current block - sending right (2)
				new_block = Block(x, y, new_block)
				self.placeBlock(x, y, new_block)
				time.sleep(delay)
				read_block = self.scan(x+1, y)
				# time.sleep(0.01)
				# wait -= 1

	    if debug:
			print 'found block:', new_block
	    return 1

	def showTopo(self, topology):
	    if debug:
			print '++++++++++++++++++++++++++'
	    for row in reversed(topology):
	        for block in row:
	            if block != 0:
	                print block,
			if stderr:
				sys.stderr.write(block.func)
	        print
		if stderr:
			sys.stderr.write('\n')
	    if debug:
		print '++++++++++++++++++++++++++'


spi = ABC_Local_Bus(debugFlag=debug)
i2c = ABC_Global_Bus()
lex = Lexer(i2c)

spi.handshake()
spi.sendVector(i2c.formatAddress(0,0))

go = True
old = []
while go:
	time.sleep(delay)
	topo = []
	block = lex.scan(0, 1)

	if block==1 and len(topo)>0 and topo!=old:
		lex.showTopo(topo)
		old = topo

	if not continuous:
		go = False

# 00 => No Neighbor / No Changes
# 01 => New Right Block
# 10 => New Above Block
# 11 => New Above and Right Blocks
