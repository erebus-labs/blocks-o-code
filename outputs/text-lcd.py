#!/usr/bin/env python

import time
import smbus
from filter import ABCFilter

class LCDTextFilter(ABCFilter):
    def __init__(self):
        ABCFilter.__init__(self)
        self.bus = smbus.SMBus(self.I2C_CHANNEL)
 
    def action(self, arg): 
        self.setText("Yay! Good job :)\nOutput=%f" % float(arg))
   
    # send command to display (no need for external use)    
    def textCommand(self, cmd):
        self.bus.write_byte_data(self.DISPLAY_TEXT_ADDR,0x80,cmd)
    
    # set display text \n for second line(or auto wrap)     
    def setText(self, text):
        self.textCommand(self.LCD_FUNCTIONSET | self.LCD_DISPLAYON |
                         self.LCD_2LINE)
        time.sleep(0.05)
    
        self.textCommand(self.LCD_DISPLAYCONTROL | self.LCD_DISPLAYON |
                         self.LCD_CURSORON | self.LCD_BLINKON)
        time.sleep(0.02)
    
        self.textCommand(self.LCD_CLEARDISPLAY)
        time.sleep(0.02)
    
        self.textCommand(self.LCD_ENTRYMODESET | self.LCD_ENTRYLEFT |
                         self.LCD_ENTRYSHIFTDECREMENT)
        time.sleep(.05)
    
        count = 0
        row = 0
    
        for c in text:
            if c == '\n' or count == 16:
                count = 0
                row += 1
                if row == 2:
                    break
                self.textCommand(0xc0)
                if c == '\n':
                    continue
            count += 1
            self.bus.write_byte_data(self.DISPLAY_TEXT_ADDR,0x40,ord(c))
    
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
    LCD_FUNCTIONSET = 0x20
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

if __name__=="__main__":
    f = LCDTextFilter()
    f._run()
