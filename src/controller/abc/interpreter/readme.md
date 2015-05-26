# A Block of Code Emulator
```
usage:
    abc                  Start interactive mode
    abc /path/to/file    Run file
    abc -h               Display this help
```

Alternatively, do a full install and use a hashbang on the first line of
your script:

```
#! /usr/local/bin/abc

<your code goes here>
```

Then make your script executable with `chmod +x /path/to/your/script`. It
will be interpreted just like any other script.

## Building
Building relies on `bison` (>3.0 for '%precendence' directive)  and `xxd`
AND `flex` (and of course `make` and `gcc`, along with gnu `libc`. If
you've ever used a Makefile on your machine, you probably already have
these.

Just run `make` to build, then `sudo make install` to install. The default
path is `/usr/local/bin/abc`

### OSX Specific
Default `bison` version = 2.5 < 3.0. This will require manual update, or
installation through favorite add on package management tool (port, brew,
etc). The following assumes the use of Brew, which installs to
`/usr/local/opt/bison/lib`

```
brew tap homebrew/dupes && brew install bison
```

## Syntax
#### Blocks
* ~~**Line Start** - has an LED indicating if the statements on the line produce
an error, could also provide power.~~

* **Variable** - a list of 8 named variables (`count`, `sum`, `a`, `b`, `c`,
`x`, `y`, and `z`), selectable by pressing a button on the block to cycle
through values, with an LED showing which is currently selected.

* **Number Selection** - a small LCD screen shows a value that can go from 0.0
to 9.9 decimal, then from 10 to 99 integer by rotating an adjacent knob.

* **Operator** - a list of 6 operators (`+`, `-`, `*`, `/`, `^`, and `%`) and
their 6 compound assignment counterparts (`+=`, `-=`, `*=`, `/=`, `^=`, and
`%=`) selectable by pressing a button on the block to cycle through values, with
an LED showing which is currently selected.

* **Integer Constant** - a constant integer block for common use numbers (`1`,
`2`, `5`, and `10`).

* **IF** - starts an if-statement, must be followed by a conditional statement,
some statements, and END IF.

* **END IF** - ends the statement list for an if-statement.

* **Condition** - a list of 6 comparison operators (`==`, `!=`, `>`, `<`, `≥`,
and `≤`), selectable by pressing a button on the block to cycle through values,
with an LED showing which is currently selected.

* **WHILE** - starts a while-statement, must be followed by a conditional
statement, some statements, and END WHILE.

* **END WHILE** - ends the statement list for an while-statement.

* **Actuators** - syntactically similar to functions: take parameters
(right-adjacent blocks) and perform a task. Shown in picture:

  * `say(number)` - ~~takes a number as a parameter, uses speech synthesis to
say the number through the on-block speaker.~~ Prints to stdout.

  * ~~`blink(integer)` - takes a positive integer as a parameter, blinks the
larger on-block LED that number of times~~

* ~~**Output** - larger LCD screen can show `stdout`-type print statements,
errors and hints, or other messages after pressing the `GO` button.~~


**Total Blocks**: 35 - could remove the 8 Line Start blocks to get to 27 total blocks.

#### Connection Indicators
Blocks have connection established LEDs indicating power and/or data connection.
Some only have left and right side connections available (operators, constants,
etc) while others have left, right, top, bottom connections available
(variables, control flow) which helps guide students to start lines with the
correct type of block. This prevents lines such as:

```
+ 5 * 9 = x
```
Also, while the following is valid in some programming languages:
```
3 + y
```
the resultant value is ignored, and this layout can help prevent such
unnecessary statements.

#### Terminators
There are no semicolons or terminating whitespace (but there are curly braces to
help with proper nesting). Statements can be determined by the block with the
right connector open (o).

#### Immediate Feedback
More verbose error checking for the software implementation is a TODO item - *Tylo*
