# abc_global.py

from Adafruit_I2C import Adafruit_I2C

class Command:
    (
    ReadData,
    SendVertical,
    SendHorizontal,
    StatusLedOn,
    StatusLedBlink,
    StatusLedOff,
    ErrorLedOn,
    ErrorLedBlink,
    ErrorLedOff,
    Reset
    ) = range(10)


class ABC_Global_Bus(object):
    """docstring for ABC_Global_Bus"""
    def __init__(self):
        super(ABC_Global_Bus, self).__init__()
        f = open('/sys/devices/bone_capemgr.9/slots', 'w')
        f.write('BB-I2C1')
        try:
            f.close()
        except IOError as e:
            if e.errno != 17:
                raise

    def formatAddress(self, x, y):
        return (int(x) & 7) | ((int(y) & 15) << 3)

    # returns function value stored in the block's function register
    def serviceCommand(self, x, y, i2c_busnum=2, register=0):
        return Adafruit_I2C(self.formatAddress(x, y), busnum=i2c_busnum).readU8(register)

    def readData(self, x, y, i2c_busnum=2):
        return self.serviceCommand(x, y, i2c_busnum, 0) # Commands.ReadData)

    def sendVertical(self, x, y, i2c_busnum=2):
        return self.serviceCommand(x, y, i2c_busnum, 1) # Commands.SendVertical)

    def sendHorizontal(self, x, y, i2c_busnum=2):
        return self.serviceCommand(x, y, i2c_busnum, 2) # Commands.SendHorizontal)

    def statusLedOn(self, x, y, i2c_busnum=2):
        return self.serviceCommand(x, y, i2c_busnum, 3) # Commands.StatusLedOn)

    def statusLedBlink(self, x, y, i2c_busnum=2):
        return self.serviceCommand(x, y, i2c_busnum, 4) # Commands.StatusLedBlink)

    def statusLedOff(self, x, y, i2c_busnum=2):
        return self.serviceCommand(x, y, i2c_busnum, 5) # Commands.StatusLedOff)

    def errorLedOn(self, x, y, i2c_busnum=2):
        return self.serviceCommand(x, y, i2c_busnum, 6) # Commands.ErrorLedOn)

    def errorLedBlink(self, x, y, i2c_busnum=2):
        return self.serviceCommand(x, y, i2c_busnum, 7) # Commands.ReadData)

    def errorLedOff(self, x, y, i2c_busnum=2):
        return self.serviceCommand(x, y, i2c_busnum, 8) # Commands.ErrorLedOn)

    def reset(self, x, y, i2c_busnum=2):
        return self.serviceCommand(x, y, i2c_busnum, 9) # Commands.Reset)
