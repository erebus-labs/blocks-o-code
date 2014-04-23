#!/usr/bin/env python
#
# GrovePi Example for using the Grove - LCD RGB Backlight (http://www.seeedstudio.com/wiki/Grove_-_LCD_RGB_Backlight)
#
# The GrovePi connects the Raspberry Pi and Grove sensors.  You can learn more about GrovePi here:  http://www.dexterindustries.com/GrovePi
#
# Have a question about this example?  Ask on the forums here:  http://www.dexterindustries.com/forum/?forum=grovepi
#
# LICENSE: 
# These files have been made available online through a [Creative Commons Attribution-ShareAlike 3.0](http://creativecommons.org/licenses/by-sa/3.0/) license.
#
# NOTE:
# 	Just supports setting the backlight colour, and
# 	putting a single string of text onto the display
# 	Doesn't support anything clever, cursors or anything

import time,sys
import smbus

I2C_CHANNEL = 1

# this device has two I2C addresses
DISPLAY_RGB_ADDR = 0x62
DISPLAY_TEXT_ADDR = 0x3e

# commands
LCD_CLEARDISPLAY = 0x01
LCD_RETURNHOME = 0x02
LCD_ENTRYMODESET = 0x04
LCD_DISPLAYCONTROL = 0x08
LCD_CURSORSHIFT = 0x10
#LCD_FUNCTIONSET = 0x20
LCD_FUNCTIONSET = 0x30
LCD_SETCGRAMADDR = 0x40
LCD_SETDDRAMADDR = 0x80

# flags for display entry mode
LCD_ENTRYRIGHT = 0x00
LCD_ENTRYLEFT = 0x02
LCD_ENTRYSHIFTINCREMENT = 0x01
LCD_ENTRYSHIFTDECREMENT = 0x00

# flags for display on/off control
LCD_DISPLAYON = 0x04
LCD_DISPLAYOFF = 0x00
LCD_CURSORON = 0x02
LCD_CURSOROFF = 0x00
LCD_BLINKON = 0x01
LCD_BLINKOFF = 0x00

# flags for display/cursor shift
LCD_DISPLAYMOVE = 0x08
LCD_CURSORMOVE = 0x00
LCD_MOVERIGHT = 0x04
LCD_MOVELEFT = 0x00

# flags for function set
LCD_8BITMODE = 0x10
LCD_4BITMODE = 0x00
LCD_2LINE = 0x08
LCD_1LINE = 0x00
LCD_5x10DOTS = 0x04
LCD_5x8DOTS = 0x00

bus = smbus.SMBus(I2C_CHANNEL)

# set backlight to (R,G,B) (values from 0..255 for each)
def setRGB(r,g,b):
    bus.write_byte_data(DISPLAY_RGB_ADDR,0,0)
    bus.write_byte_data(DISPLAY_RGB_ADDR,1,0)
    bus.write_byte_data(DISPLAY_RGB_ADDR,0x08,0xaa)
    bus.write_byte_data(DISPLAY_RGB_ADDR,4,r)
    bus.write_byte_data(DISPLAY_RGB_ADDR,3,g)
    bus.write_byte_data(DISPLAY_RGB_ADDR,2,b)

# send command to display (no need for external use)    
def textCommand(cmd, text="[default]"):
    print("%s: %s" % (text, "{0:b}".format(cmd)))
    bus.write_byte_data(DISPLAY_TEXT_ADDR,0x80,cmd)

# set display text \n for second line(or auto wrap)     
def setText(text):
    textCommand(LCD_FUNCTIONSET | LCD_DISPLAYON | LCD_2LINE, text='functionset');
    time.sleep(0.05)

    textCommand(LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKOFF, text='displayset')
    time.sleep(0.02)

    textCommand(LCD_CLEARDISPLAY, text='clear')
    time.sleep(0.02)

    textCommand(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT, text='entryset')
    time.sleep(.05)

    count = 0
    row = 0
    for c in text:
        textCommand(count | 0x80 if row == 0 else count | 0xc0, text=('rowset'))
        if c == '\n' or count == 16:
            count = 0
            row += 1
            if row == 2:
                break
            if c == '\n':
                continue
        count += 1
        print('Writing [%d: %c], Returnvalue: %s' %
            (ord(c), c, str(
            bus.write_byte_data(DISPLAY_TEXT_ADDR,0x40,ord(c))
            )))


def interpolate(c1, c2, percent):
    red = c1[0] - ((c1[0] - c2[0]) * percent)
    green = c1[1] - ((c1[1] - c2[1]) * percent)
    blue = c1[2] - ((c1[2] - c2[2]) * percent)
    return int(red), int(green), int(blue)


def transition(c, oldval, holdtime = 0.25, transittime=0.1, steptime = 0.01):
            nsteps = transittime / steptime
            for y in range(1, int(nsteps)):
                red, green, blue = interpolate(oldval, c, float(y)/nsteps)
                setRGB(red, green, blue)
                time.sleep(steptime)
            red = c[0]
            green = c[1]
            blue = c[2]
            setRGB(red, green, blue)
            time.sleep(holdtime)


if __name__=="__main__":
    setRGB(255, 255, 255)
    setText("Hello world!")
    time.sleep(5)

    c = [(255, 0, 0), (255, 63, 0), (255, 255, 0),
         (0, 255, 0), (0, 0, 255), (63, 0, 255),
         (127, 0, 255)]
    old_c = (0, 0, 0)
    while True:
        for x in range(0,7):
            transition(c[x], old_c, holdtime=2, transittime=0.5)
            old_c = c[x]

