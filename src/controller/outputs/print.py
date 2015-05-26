#!/usr/bin/env python

from filter import ABCFilter

class PrintFilter(ABCFilter):
    def __init__(self):
        ABCFilter.__init__(self)

    def action(self, arg):
        print("output = %f (on output channel #%s)" % (float(arg), int(self.channel)))

if __name__=="__main__":
    f = PrintFilter()
    f._run()
