#! /usr/bin/python2

from subprocess import Popen, PIPE
from argparse import ArgumentParser
from outputs.textlcd import LCDTextFilter
from time import sleep
import abc
from abc.button import ABCButton, ABCButtonManager

class ABCOrchestration(object):
    lexer_command = {'args': ['lexer.py']}
    error_command = [
        {'args': ['python2', '-u', 'abc/error.py']}
    ]
    output_commands = [
        {'args': ['python2', '-u', 'outputs/textlcd.py', '1'], 'name': 'lcd'},
        {'args': ['python2', '-u', 'outputs/lcdrgb.py', '2'], 'name': 'rgb'},
        {'args': ['python2', '-u', 'outputs/motor.py', '3'], 'name': 'motor'},
        {'args': ['python2', '-u', 'outputs/buzzer.py', '4'], 'name': 'buzzer'},
        {'args': ['python2', '-u', 'outputs/print.py', '5'], 'name': 'print'}
    ]
    null_command = 'outputs/null.py'
    INTERPRETER = 'abc/interpreter/abc'

    def __init__(self):
        self.commands = []
        self.parse_args()
        self._disable_outputs()
        self._setup_commands()
        self.lcd = LCDTextFilter(filter=False)
        self.buttonmgr = ABCButtonManager([
            ABCButton(port='P8_10', callback=self.lexer_run),
            ABCButton(port='P8_12', callback=(self.lexer_display() if self.args.use_lexer else None))
        ])

    def parse_args(self):
        parser = ArgumentParser()
        parser.add_argument("-f", "--file", help="Run a test script instead of using the blocks")
        parser.add_argument("-d", "--daemon", help="When run as a script, run in daemon mode",
                action="store_true")
        for x in range(0,5):
            parser.add_argument("-{0}".format(x+1), "--disable-{0}".format(self.output_commands[x]['name']),
                help="Disable output #{1}: {0}".format(self.output_commands[x]['name'], x+1),
                action="store_true")
        args = parser.parse_args()
        if not args.file:
            args.use_lexer = True
        else:
            args.use_lexer = False
        self.args = args

    def _disable_outputs(self):
        for command in self.output_commands:
            if vars(self.args)['disable_'+command['name']]:
                command['args'][2] = self.null_command

    def _setup_commands(self):
        assert not (self.args.use_lexer and self.args.file)
        if self.args.use_lexer:
            self.commands.append(self.lexer_command)
            self.commands.append({'args': [self.INTERPRETER]})
        else:
            self.commands.append({'args': [self.INTERPRETER, self.args.file]})
        self.commands += self.error_command
        self.commands += self.output_commands

    def _setup_pipes(self):
        if self.args.use_lexer:
            self.commands[0]['proc'] = Popen(self.commands[0]['args'], stdout=PIPE, stdin=PIPE, stderr=PIPE)
            self.lexer_control = self.commands[0]['proc'].stdin
            self.lexer_results = self.commands[0]['proc'].stderr
        else:
            self.commands[0]['proc'] = Popen(self.commands[0]['args'], stdout=PIPE)

        for needle in range(1, len(self.commands) - 1):
            self.commands[needle]['proc'] = Popen(self.commands[needle]['args'],
                stdout=PIPE, stdin=self.commands[needle-1]['proc'].stdout)

        self.commands[-1]['proc'] = Popen(self.commands[-1]['args'],
            stdin=self.commands[-2]['proc'].stdout)

    def _wait_procs(self):
        for command in self.commands:
            command['proc'].wait()

    def run_once(self):
        self._setup_pipes()
        if self.args.use_lexer:
            self.lexer_run()
        self._wait_procs()

    def read_lexer(self):
        lines = ['', '']
        while lines[1] != 'endprogram':
            self.lcd.set_text(line[0] + '\n' + line[1])
            sleep(1)
            lines[0] = lines[1]
            lines[1] = self.lexer_results.readline()

    def lexer_run(self):
        self.lexer_control.write('r')

    def lexer_display(self):
        self.lexer_control.write('d')
        self.read_lexer()

    def run_loop(self):
        self.buttonmgr.poll()

if __name__ == '__main__':
    o = ABCOrchestration()
    if o.args.daemon:
        o.run_loop()
    else:
        o.run_once()
