#! /usr/bin/python2

import sys

class ABCFilter(object):
    """
    A filter that performs an action when
    an output pattern is outputted by the abc
    interpreter

    The output pattern is
        #OUT\([1-5], [0-9]+[\.[0-9]+]\)
    where the first parameter is a number 1-5
    for the output channel and the second parameter
    is a float.
    """

    def __init__(self, channel):
        self.channel = int(channel)

    def _run(self):
        """
        Filter abc interpreter output and call
        the associated callback self.action()
        """
        while True:
            try:
                line = sys.stdin.readline()
            except KeyboardInterrupt:
                break

            if not line:
                break

            if not '#OUT(' + str(self.channel) in line:
                sys.stdout.write(line)
            else:
                second_arg_with_paren = line.split(',')[1]
                secondarg = second_arg_with_paren.split(')')[0]
                self.action(secondarg)


    def action(self, arg):
        """
        Perform an action when an output statement is
        executed by the abc interpreter. Subclass to
        define your own callback.
        """
        print('[Default Output]: "%f" on channel [%d]' % (
              float(arg), self.channel))


def main():
    if len(sys.argv) < 2:
        raise ValueError('An output channel is required')

    try:
        dummy = int(sys.argv[1])
        if dummy > 5 or dummy < 0:
            raise RuntimeError('The output channel must be between 1-5')
    except ValueError:
        raise ValueError('Could not parse output channel')

    abcfilter = ABCFilter(sys.argv[1])
    abcfilter._run()

if __name__ == '__main__':
    main()
