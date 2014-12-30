# A Block of Code Emulator
```
usage:
    scripted mode:
    ./abc /path/to/file
```
Note: Interactive mode currently broken.

## Semantics
There are two 'things' currently implemented: statements and expressions
* ~~Statements do things~~ Statements are implemented as expressions with side-effects
* Expressions say things

Expressions can contain operators, implemented as pure functions. These
functions take number(s) and return numbers. Algebraic order-of-
operations apply.

### Variables
There are 6 variables, [A-F], representing the 6 sides of a cube.
Variables hold numbers

In the spirit of `golfscript`, variables are preinitialized to some
useful values:
* A=0
* B=1
* C=2
* D=-1
* E=pi
* F=10

### Statements
There are 3 statements currently implemented:
* `VARIABLE: EXPRESSION` Assigns the results of evaluating EXPRESSION
    to the indicated variable, prints debug info
* ~~`EXPRESSION` prints the results of evaluating EXPRESSION, with debug
    info~~ Removed
* `p EXPRESSION` explicitly prints the results of evaluating EXPRESSION

### Functions
* `+`, `-`, `*`, `/`, `^`, `%`: Binary arithmetic operators
* `=`, `<`, `>`: Comparison operators, returns 1 for TRUE, 0 for FALSE
    * `l`: Less than or equal to
    * `g`: Greater than or equal to
    * `~`: Not equal to
* `-`: Unary operator. Arithmetic negation.
* `!`: Unary operator. Logical negation.


## Comments
End-of-line comments can be included with `#`.


## TODO
* Statement Blocks
* Control Flow

