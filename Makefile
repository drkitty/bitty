MAKEFLAGS += --no-builtin-rules
.SUFFIXES:

.SECONDEXPANSION:


EXE_SRC := bitty.c
SRC := $(EXE_SRC) fail.c

OBJ := $(SRC:%.c=%.o)
EXE := $(EXE_SRC:%.c=%)

CC := gcc
CFLAGS := -std=c99 -pedantic -g -Wall -Wextra -Werror -Wno-unused-function


all: $(EXE) $(EXTRA_EXE)

$(OBJ): $$(patsubst %.o,%.c,$$@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(EXE) $(EXTRA_EXE):
	$(CC) -o $@ $^

$(EXE): $$@.o

clean:
	rm -f $(OBJ) $(EXE)


fail.o: common.h fail.h
bitty.o: common.h fail.h

bitty: fail.o


.DEFAULT_GOAL := all
.PHONY: all clean
