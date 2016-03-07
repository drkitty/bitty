MAKEFLAGS += --no-builtin-rules
.SUFFIXES:

.SECONDEXPANSION:
.DELETE_ON_ERROR:

.DEFAULT:
	make -C .. assembler/$@


EXE_SRC := asm.c
SRC := $(EXE_SRC) dict.c fail.c lexer.c parser.c

OBJ := $(SRC:%.c=%.o)
EXE := $(EXE_SRC:%.c=%)

CC := gcc
CFLAGS := -std=c99 -pedantic -g -Wall -Wextra -Werror \
	-Wno-unused-function -Wno-unused-parameter

RE2C_DIR := lib/re2c/re2c/


all: $(EXE) $(EXTRA_EXE)

$(OBJ): $$(patsubst %.o,%.c,$$@) common.h
	$(CC) $(CFLAGS) -c -o $@ $<

$(EXE) $(EXTRA_EXE):
	$(CC) -o $@ $^

$(EXE): $$@.o


asm.o: ast.h fail.h insn.h lexer.h parser.h
dict.o: dict.h fail.h
fail.o: fail.h
lexer.o: fail.h lexer.h
parser.o: ast.h fail.h lexer.h parser.h

asm: asm.o dict.o fail.o lexer.o parser.o


# Some of these prereq lists are incomplete, but I don't care.

$(RE2C_DIR)configure: $(RE2C_DIR)autogen.sh
	cd $(RE2C_DIR) && ./autogen.sh

$(RE2C_DIR)Makefile: $(RE2C_DIR)configure
	cd $(RE2C_DIR) && ./configure

$(RE2C_DIR)re2c: $(RE2C_DIR)Makefile
	make -C $(RE2C_DIR)


lexer.c: lexer.re.c $(RE2C_DIR)re2c
	$(RE2C_DIR)re2c -8 -W -Werror -o $@ $<


clean:
	rm -f $(OBJ) $(EXE) lexer.c


.DEFAULT_GOAL := all
.PHONY: all clean
