#!/usr/bin/python

from abc_local import ABC_Local_Bus
from abc_global import ABC_Global_Bus
import time
import sys

value_tokens = [
	'0',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	'.',
	'X',
	'Y',
	'Z',
	'SUM',
	'COUNT'
]

operator_tokens = [
    '+',
    '-',
    '/',
    '*',
    '^',
    '%',
    '=',
    '!=',
    '>',
    '<',
    '>=',
    '<=',
    '!'
]

control_tokens = [
    'while',
    'endwhile',
    'if',
    'else',
    'endif',
    '(',
    ')'
]

statement_tokens = [
    'OUT1',
    'OUT2',
    'OUT3',
    'OUT4',
    'OUT5',
    'Print'
]

abc_tokens = [
    value_tokens,
    operator_tokens,
    control_tokens,
    statement_tokens
]

class Block(object):
    """docstring for Block"""
    def __init__(self, new_x, new_y, block_data, debug_flag=False):
        self.x = new_x
        self.y = new_y
        self.data = int(block_data)
        self.category = ((self.data >> 4) & 0b00000011)
        self.token = self.token_match(self.category, self.data & 0b00001111)
        self.right = (self.data & 0b01000000) > 0
        self.above = (self.data & 0b10000000) > 0
        self.debug = debug_flag

    def adc_map(self, val, out_min, out_max, in_min=0, in_max=15):
        return (val - in_min) * (out_max - out_min) // (in_max - in_min) + out_min

    def token_match(self, category, adc_val):
        subset = abc_tokens[category]
        return subset[self.adc_map(adc_val, 0, len(subset)-1)]

    def __repr__(self):
		if self.debug:
			desc = '[ '
			desc += self.token
			desc += ' R' if self.right else ' r'
			desc += ' A' if self.above else ' a'
			desc += ' ]'
		else:
			desc = self.token
		return desc

    def __eq__(self, other):
        return isinstance(other, self.__class__) and self.data==other.data

    def __ne__(self, other):
        return not self==other

    def __cmp__(self, other):
        return self==other


class ABC_Lexer(object):
    """docstring for ABC_Lexer"""
    def __init__(self, debug_flag=False):
        super(ABC_Lexer, self).__init__()
        self.global_bus = ABC_Global_Bus()
        self.local_bus = ABC_Local_Bus(debugFlag=debug_flag)
        self.topology = []
        self.debug = debug_flag
        self.scan_delay = 0.75

    def placeBlock(self, x, y, new_block):
        if y > len(self.topology):
            row = []
            row.append(new_block)
            self.topology.append(row)
        else:
            row = self.topology[y-1]
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
        new_block = self.global_bus.readData(x, y)    # read from i2c line (0)
        if self.debug:
        	print x, y, new_block
        if new_block == -1:
            return 0

        time.sleep(0.05)
        new_block = Block(x, y, new_block)
        self.placeBlock(x, y, new_block)

        if x == 0 and new_block.above == True:
            # wait = 10
            read_block = self.scan(x, y+1)
            if read_block == 0: #and wait > 0:
        		if self.debug:
        			print "Sending up"
        		new_block = self.global_bus.sendVertical(x, y)      # read current block - sending up (1)
        		new_block = Block(x, y, new_block)
        		self.placeBlock(x, y, new_block)
        		time.sleep(self.scan_delay)
        		read_block = self.scan(x, y+1)
        		# time.sleep(0.01)
        		# wait -= 1

        if new_block.right == True:
            # wait = 10
            read_block = self.scan(x+1, y)
            if read_block == 0: #and wait > 0:
        		if self.debug:
        			print "Sending right"
        		new_block = self.global_bus.sendHorizontal(x, y)     # read current block - sending right (2)
        		new_block = Block(x, y, new_block)
        		self.placeBlock(x, y, new_block)
        		time.sleep(self.scan_delay)
        		read_block = self.scan(x+1, y)
        		# time.sleep(0.01)
        		# wait -= 1

        if self.debug:
        	print 'found block:', new_block
        return 1

    def showTopology(self, stderr=False):
        if self.debug:
        	print '++++++++++++++++++++++++++'
        for row in reversed(self.topology):
            for block in row:
                if block != 0:
                    print block,
        	if stderr:
        		sys.stderr.write(block.token)
            print
        if stderr:
        	sys.stderr.write('\n')
        if self.debug:
            print '++++++++++++++++++++++++++'

    def runOnce(self):
        self.local_bus.resetChain()
        self.local_bus.handshake()
        self.local_bus.sendVector(self.global_bus.formatAddress(0,0))

        self.topology = []
        block = self.scan(0, 1)

        if block==1 and len(self.topology)>0:
            self.showTopology()

    def displayCode(self):
        self.showTopology(stderr=True)
