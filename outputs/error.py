#! /usr/bin/python2

import sys

class ErrorFilter(object):
    def __init__(self):
        pass

    def _run(self):
        while True:
            try:
                line = sys.stdin.readline()
            except KeyboardInterrupt:
                break

            if not line:
                break

            if not '#SYNTAXERROR(' in line:
                sys.stdout.write(line)
                sys.stdout.flush()
            else:
                args_w_comma = line.split('(')[1].split(')')[0]
                args = args_w_comma.split(',')
                self.action(args[0], args[1])

    def action(self, line, column):
        print('[ERROR]: line %d, column %d' % (
              int(line), int(column)))


def main():
    f = ErrorFilter()
    f._run()

if __name__ == '__main__':
    main()
