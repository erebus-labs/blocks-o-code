#!/usr/bin/python

from abc_global import ABC_Global_Bus
import time
import sys

delay = 0.75
go = True

i2c = ABC_Global_Bus()

while go:
	time.sleep(delay)
	print i2c.readData(2, 5) & 0b00001111

# 00 => No Neighbor / No Changes
# 01 => New Right Block
# 10 => New Above Block
# 11 => New Above and Right Blocks
