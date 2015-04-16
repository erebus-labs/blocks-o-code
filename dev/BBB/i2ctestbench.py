# Testbench to test the i2c read limits and collect data for A Block of Code


from Adafruit_I2C import Adafruit_I2C
import time
import sys
import csv

def addrFvect(x,y):
	if y<16 and x<8:
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


KNOWN_BLOCK_X = 2
KNOWN_BLOCK_Y = 5
KNOWN_BLOCK_FUNC = 36
NO_BLOCK_FUNC = 0
NUM_OF_SCANS = 2000
STALL_CUTOFF = 0.4

scan_duration = 0
current_scan_num = 0
response_list = []
response_correct = 0
response_error = 0
data_correct = 0
data_error = 0
stall_total = 0
scan_list = []

# fp = open('scan_results.csv', 'wt')
try:
    # writer = csv.writer(fp)
    # writer.writerow( ('Scan', 'Response Correct %', 'Data Correct %', 'Stall %', 'Ave Time / Read', 'Ave Time / Scan') )
    for scan in range(NUM_OF_SCANS):
        current_scan_num += 1
        for y in range(1,16):
            for x in range(0,8):
                # time.sleep(0.01)
                read_start = time.time()
                block_func = I2cLex(x,y) #block_func will be returned value from i2c
                read_end = time.time()

                response_time = read_end - read_start
                response_list.append(response_time)

                if response_time > STALL_CUTOFF:
                    stall_total += 1

                if (x, y) == (KNOWN_BLOCK_X, KNOWN_BLOCK_Y):
                    if block_func == KNOWN_BLOCK_FUNC:
                        response_correct += 1
                        data_correct += 1
                    elif block_func == NO_BLOCK_FUNC:
                        response_error += 1
                    else:
                        data_error += 1
                        print current_scan_num, "Known block data error", (x, y), block_func
                else:
                    if block_func == NO_BLOCK_FUNC:
                        data_correct += 1
                        response_correct += 1
                    else:
                        response_error +=1
                        data_error += 1
                        print current_scan_num, "Unknown block data error", (x, y), block_func
        if scan % 100 == 0:
            print scan, 'out of', NUM_OF_SCANS, 'completed.'
        # time.sleep(1)

        #         print block_func,
        #     print
        # print '+++++++++++++++++++++++++++++++++++++++++++++'


    response_total = len(response_list)
    print 'Total Responses', response_total
    print 'Responses Correct', response_correct
    print 'Response Errors', response_error
    print 'Percent Correct', (response_correct / response_total) * 100.0
    print 'Correct Data', data_correct
    print 'Data Errors', data_error
    print 'Percent Correct', (data_correct / response_total) * 100.0
    print 'Stalls', stall_total
    print 'Percent Stalled', (stall_total / response_total) * 100.0
    print 'Average Time / Read', sum(response_list) / response_total

    # get average time for full scan
    for scan in range(NUM_OF_SCANS):
        scan_start = time.time()
        for y in range(1,15):
            for x in range(0,8):
                block_func = I2cLex(x,y)
        scan_end = time.time()
        scan_list.append(scan_end - scan_start)
    print 'Average Time / Scan', sum(scan_list) / len(scan_list)
finally:
#     fp.close()
    print 'thanks'
