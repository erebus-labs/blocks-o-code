#! /usr/bin/python2.7

from Adafruit_I2C import Adafruit_I2C

reserved = [0,1,2,44,45,46,47,56,57,58,59,60,61,62]

blocks = []

for address in range(0,128):
    if address in reserved:
        continue

    block = {}

    i2caddr = Adafruit_I2C(address)
    block['addr'] = address
    block['func'] = i2caddr.readU8(0)
    if block['func'] < 0:
        continue
    block['x'] = i2caddr.readU8(5)
    block['y'] = i2caddr.readU8(4)

    # print(block)
    blocks.append(block)

func_to_str = {
    36: 'A',
    12: ':',
    117: '3',
    19: 'p',
    0:' '
}

source_code = ""

for x in reversed(range(0,16)):
    for y in range(0,16):
        for block in blocks:
            if block['x'] == x and block['y'] == y:
                source_code += ' %s ' % func_to_str[block['func']]
                break
    source_code += '\n'

print(source_code)
