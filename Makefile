CC=gcc
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
	$(CC) -o $(TARGETS) $(OBJ) $(LIBS)

%.o: %.c $(PARSESRC) $(README)
	$(CC) -c $< -o $@
	$(CC) -c -o $*.d -MT $@ -MM $<

$(PARSESRC): $(PARSERS)
	$(BISON) -o src/parse.tab.c --defines=src/parse.tab.h $<

$(README):
	$(XXD) -i $(READMESRC) > $(README)

clean:
	rm -f $(OBJ) $(DEP) $(PARSESRC) $(TARGETS) $(README)

doc:
	pandoc -f markdown_github -o $(HTML) $(DOC)

cleandoc:
	rm -f $(HTML)
