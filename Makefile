CC=gcc
CFLAGS=-std=c99 -g
XXD=xxd -i
BISON=bison

LIBS=-lm

TARGETS=abc

PARSERS=src/parse.y
PARSEC=$(PARSERS:.y=.tab.c)
PARSEH=$(PARSERS:.y=.tab.h)
PARSEO=$(PARSERS:.y=.tab.o)
PARSESRC=$(PARSEC) $(PARSEH)

READMESRC=readme.md
README=src/readme.xxd

SRC=$(wildcard src/*.c) $(PARSEC)
OBJ=$(filter-out $(PARSEO), $(patsubst %.c, %.o, $(SRC))) $(PARSEO)
DEP=$(SRC:.c=.d)

DOC=readme.md
HTML=$(DOC:.md=.html)

all: $(TARGETS)

abc: $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGETS) $(OBJ) $(LIBS)

%.o: %.c $(PARSESRC) $(README)
	$(CC) $(CFLAGS) -c $< -o $@
	$(CC) $(CFLAGS) -c -o $*.d -MT $@ -MM $<

$(PARSESRC): $(PARSERS)
	$(BISON) -v -o src/parse.tab.c --defines=src/parse.tab.h $<

$(README):
	$(XXD) -i $(READMESRC) > $(README)

clean:
	rm -f $(OBJ) $(DEP) $(PARSESRC) $(TARGETS) $(README) $(PARSERS:.y=.output)

doc:
	pandoc -f markdown_github -o $(HTML) $(DOC)

cleandoc:
	rm -f $(HTML)
