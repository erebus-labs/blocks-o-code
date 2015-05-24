#! /usr/bin/python2

from subprocess import Popen, PIPE
from argparse import ArgumentParser

class ABCOrchestration(object):
    lexer_command = [
    ]
    error_command = [
        {'args': ['python2', 'outputs/error.py']}
    ]
    output_commands = [
        {'args': ['python2', 'outputs/print.py', '1'], 'name': 'lcd'},
        {'args': ['python2', 'outputs/print.py', '2'], 'name': 'rgb'},
        {'args': ['python2', 'outputs/print.py', '3'], 'name': 'motor'},
        {'args': ['python2', 'outputs/print.py', '4'], 'name': 'buzzer'},
        {'args': ['python2', 'outputs/print.py', '5'], 'name': 'print'}
    ]
    null_command = 'outputs/null.py'
    
    def __init__(self):
        self.commands = []
        self.parse_args()
        self._disable_outputs()
        self._setup_commands()

    def parse_args(self):
        parser = ArgumentParser()
        parser.add_argument("-f", "--file", help="Run a test script instead of using the blocks")
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
                command['args'][1] = self.null_command

    def _setup_commands(self):
        assert not (self.args.use_lexer and self.args.file)
        if self.args.use_lexer:
            self.commands.append(self.lexer_command)
            self.commands.append({'args': ['./abc']})
        else:
            self.commands.append({'args': ['./abc', self.args.file]})
        self.commands += self.error_command
        self.commands += self.output_commands

    def _setup_pipes(self):
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
        self._wait_procs()

if __name__ == '__main__':
    o = ABCOrchestration()
    o.run_once()
