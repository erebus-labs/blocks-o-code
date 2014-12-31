# A Block of Code Emulator
```
usage:
    ./abc /path/to/file    Run file
    ./abc -h               Display this help
```

## Building
Building relies on `bison` and `xxd` (and of course `make` and `gcc`, along
with gnu `libc`. If you've ever used a Makefile on your machine, you probably
already have these.

Just run `make` to build.

There is no `install` rule to add it to your path yet.

## Semantics
Everything is an expression, meaning everything returns a value

A code block is defined as a series of expressions separated by semicolons
(';') or newlines ('\n'). The result of the last expression is returned by
a block.

### Variables
There are 6 variables, [A-F], representing the 6 sides of a cube.
Variables hold numbers

Variables are preinitialized to some useful values:
* A=0
* B=1
* C=2
* D=-1
* E=pi
* F=10

### Statements
Although everything is an expression, statements and functions are considered
differently since the following have side-effects:
* *var* `:` *e*: Assigns the results of evaluating *e* to the indicated
    variable *var*
* `p` *e*: prints the results of evaluating *e*
* `c` *e*: prints the ascii encoding of evaluating *e*

### Functions
* Binary operators:
    * `+`, `-`, `*`, `/`, `^`, `%`: Arithmetic operators
    * `=`, `<`, `>`: Comparison operators, returns 1 for TRUE, 0 for FALSE
        * `l`: Less than or equal to
        * `g`: Greater than or equal to
        * `~`: Not equal to
* `-`: Arithmetic negation. Unary operator.
* `!`: Logical negation. Unary operator
* `{` *e* `}` Code Block. Returns the last expression in the list.
* `(` *e* `)` Precedence Specifier. Can only contain one expression.

### Control Flow
* *e* `?` *s1*: If. Evaluates and returns *s1* when *e* evaluates to
    non-zero. Otherwise returns 0.
* *e* `?` *s1* `:` `s2`: If.. Else. Evaluates and returns *s1* when *e*
    evaluates non-zero. Otherwise evaluates and returns *s2*.
* *e* `@` *s1*: While Loop. Evaluates *e* and then *s1* until *e* returns
    zero.
* *e* `d` *s1*: Do.. While Loop. Evaluates *s1* and then *e* until *e*
    returns zero.
* *e* `$` *s1*: Repeat Loop. Evaluates *e* and repeats *s1* *e* times.


## Comments
* `#` *Comment*: `#` and *comment* are ignored until the next newline

## TODO
* Error Checking
