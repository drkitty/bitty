MAKEFLAGS += --no-builtin-rules
.SUFFIXES:

.SECONDEXPANSION:
.DELETE_ON_ERROR:


EXE_SRC := test.c
SRC := $(EXE_SRC) fail.c

OBJ := $(SRC:%.c=%.o)
EXE := $(EXE_SRC:%.c=%)

CC := gcc
CFLAGS := -std=c99 -pedantic -g -Wall -Wextra -Werror \
	-Wno-unused-function -Wno-unused-parameter

RE2C_DIR := lib/re2c/re2c/


all: $(EXE) $(EXTRA_EXE)

$(OBJ): $$(patsubst %.o,%.c,$$@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(EXE) $(EXTRA_EXE):
	$(CC) -o $@ $^

$(EXE): $$@.o


test.c: test.re.c $(RE2C_DIR)re2c
	$(RE2C_DIR)re2c -8 -W -Werror -o $@ $<


fail.o: common.h fail.h

test: test.o fail.o


# Some of these prereq lists are incomplete, but I don't care.

$(RE2C_DIR)configure: $(RE2C_DIR)autogen.sh
	cd $(RE2C_DIR) && ./autogen.sh

$(RE2C_DIR)Makefile: $(RE2C_DIR)configure
	cd $(RE2C_DIR) && ./configure

$(RE2C_DIR)re2c: $(RE2C_DIR)Makefile
	make -C $(RE2C_DIR)


clean:
	rm -f $(OBJ) $(EXE) test.c


.DEFAULT_GOAL := all
.PHONY: all clean
