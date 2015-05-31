#!/usr/bin/python

from abc_lexer import ABC_Lexer
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

lex = ABC_Lexer(debug)

go = True
while go:
    #time.sleep(delay)
    lex.runOnce()

    if not continuous:
        go = False

# 00 => No Neighbor / No Changes
# 01 => New Right Block
# 10 => New Above Block
# 11 => New Above and Right Blocks
