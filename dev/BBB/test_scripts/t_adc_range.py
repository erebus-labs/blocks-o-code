#!/usr/bin/python

from abc_global import ABC_Global_Bus
from abc_lexer import Block
import time
import sys

category = 0
if len(sys.argv)>1:
	if '-v' in sys.argv:
		category = 0
	if '-a' in sys.argv:
		category = 1
	if '-c' in sys.argv:
		category = 2
	if '-s' in sys.argv:
		category = 3

delay = 0.75
go = True

i2c = ABC_Global_Bus()

while go:
	time.sleep(delay)
	data = i2c.readData(2, 5) & 0b00001111
	if len(sys.argv)>1:
		block = Block(2, 5, data)
		print block.token_match(category, data)
	else:
		print data
