#!/usr/bin/env python

from filter import ABCFilter

class NullFilter(ABCFilter):
    def __init__(self):
        ABCFilter.__init__(self)

    def action(self, arg):
        pass

if __name__=="__main__":
    f = NullFilter()
    f._run()
