#!/usr/bin/python

# Tests the full range of the i2c address space on a given BeagleBone Black port1_reserved
# by changing the function data and addresses of a single connected block.

from Adafruit_I2C import Adafruit_I2C
from sys import argv

# starting values for block match with uC test structure named "i2ctest"
addr = 42           # starting i2c address of block
func = 19           # starting function value of block
read_func_reg   = 0 # (virtual) i2c register to read function
change_func_reg = 1 # (virtual) i2c register to change function
change_addr_reg = 2 # (virtual) i2c register to change address
i2c_busnum      = 2 # i2c port number on BBB (default to 2)
error_count     = 0 # running total of errors encountered
error_addrs     = set() # set of erroring addresses
verbose         = False # command-line flag for

# reserved addresses for i2c ports
port1_reserved  = [84, 85, 86, 87]

# capture command line args
if len(argv) == 2:
    filename, i2c_busnum = argv
    i2c_busnum = int(i2c_busnum)
if len(argv) == 3:
    filename, i2c_busnum, v = argv
    i2c_busnum = int(i2c_busnum)
    if v == "-v":
        verbose = True

# returns function value stored in the block's function register
def readFunction(addr):
    return Adafruit_I2C(addr, busnum=i2c_busnum).readU8(read_func_reg)

# changes a block by writing a given byte to a given register
def writeToChip(addr, reg, byte):
    Adafruit_I2C(addr, busnum=i2c_busnum).write8(reg, byte)

# performs the given test by comparing expected data with returned data
def confirmFunction(test_name, addr, exp_func):
    global error_count
    ret = readFunction(addr)
    if ret != exp_func:
        print test_name, "error! Read addr:", addr, "returned", ret, ". Expected: ", exp_func
        error_count += 1
        error_addrs.add(addr)
    else:
        if verbose:
            print test_name, "for addr:", addr, "succeeded!"

# tests for a new function data from a read after it has been written
def testFunctionChange(addr, new_func):
    test = "Testing function change"
    writeToChip(addr, change_func_reg, new_func)
    confirmFunction(test, addr, new_func)

# tests for a given function from a new address after it has been changed
def testAddrChange(addr, new_addr, func):
    test = "Testing address change"
    writeToChip(addr, change_addr_reg, new_addr)
    readFunction(new_addr) # address change requires follow up read
    confirmFunction(test, new_addr, func)

max_num = 130   # intentially exceed i2c addres space to confirm boundary
offset = 200    # arbitrarily adjust function value as it's written and read
read_test = "Testing function read"
for x in range(max_num):
    # skip over known reserved values
    if i2c_busnum == 1 and x in port1_reserved:
        print x, "is reserved on port", i2c_busnum
        continue

    # reduced printing for each address only
    if not verbose:
        print "Testing addr:", addr

    # test simple read
    confirmFunction(read_test, addr, func)

    # test change of function data
    func = offset-x
    testFunctionChange(addr, func)

    # test change of address
    testAddrChange(addr, x, func)
    addr = x

# final read after full range
confirmFunction(read_test, addr, func)

# print results
print "Errors encountered:", error_count, "at addresses:", error_addrs
print "Thanks!"
