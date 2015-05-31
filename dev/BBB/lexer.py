#!/usr/bin/python

from abc_lexer import ABC_Lexer
import time
import sys

lex = ABC_Lexer()

def once(lexer):
	lexer.runOnce()
	sys.stdout.write('g')
	sys.stdout.flush()

def display(lexer):
	lexdr.displayCode()

ControlCommands = {
	'r' : once,
	'd' : display
}

go = True
while go:
 	ControlCommands[sys.stdin.read(1)]()
