UNAME=$(shell uname)
CC=gcc
CFLAGS=-std=gnu99
CDEBUGFLAGS=-g -DPRINTDEBUGINFO
XXD=xxd -i
INSTALL=install
UNINSTALL=rm -f
BINDIR=/usr/local/bin

FLEX=flex
ifeq ($(UNAME), Darwin)
BISON=/usr/local/opt/bison/bin/bison
ifneq (,$(wildcard $(BISON)))
$(info Using BREW PORT of BISON)
else
$(info OSX Requires bison ver > 3.0)
$(info use: sudo brew tap homebrew/dupes && brew install bison)
$(error Bison 3.0 Not Found)
endif
else
BISON=bison
endif

LIBS=-lm -lfl

TARGETS=abc

PARSERS=src/parse.y
PARSEC=$(PARSERS:.y=.tab.c)
PARSEH=$(PARSERS:.y=.tab.h)
PARSEO=$(PARSERS:.y=.tab.o)
PARSESRC=$(PARSEC) $(PARSEH)

LEXS=src/parse.l
LEXC=$(LEXS:.l=.lex.c)
LEXH=$(LEXS:.l=.lex.h)
LEXO=$(LEXS:.l=.lex.o)
LEXSRC=$(LEXC) $(LEXH)

READMESRC=readme.md
README=src/readme.xxd

SRC=$(wildcard src/*.c) $(PARSEC) $(LEXC)
OBJ=$(filter-out $(PARSEO) $(LEXO), $(patsubst %.c, %.o, $(SRC) $(LEXO))) $(PARSEO) $(LEXO)
DEP=$(SRC:.c=.d)

DOC=readme.md
HTML=$(DOC:.md=.html)

all: $(TARGETS)

debug: CFLAGS += $(CDEBUGFLAGS)
debug: clean all

install: all
	$(INSTALL) $(TARGETS) '$(BINDIR)/'

uninstall:
	-$(UNINSTALL) '$(BINDIR)/$(TARGETS)'

abc: $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGETS) $(OBJ) $(LIBS)

%.o: %.c $(PARSESRC) $(LEXSRC) $(README)
	$(CC) $(CFLAGS) -c $< -o $@
	$(CC) $(CFLAGS) -c -o $*.d -MT $@ -MM $<

$(PARSESRC): $(PARSERS)
	$(BISON) -v -o src/parse.tab.c --defines=src/parse.tab.h $<

$(LEXSRC): $(LEXS)
	$(FLEX) -o $(LEXC) --header-file=$(LEXH) $<

$(README):
	$(XXD) $(READMESRC) > $(README)

clean:
	rm -f $(OBJ) $(DEP) $(PARSESRC) $(LEXSRC) $(TARGETS) $(README) $(PARSERS:.y=.output)

doc:
	pandoc -f markdown_github -o $(HTML) $(DOC)

cleandoc:
	rm -f $(HTML)
