MAKEFLAGS += --no-builtin-rules
.SUFFIXES:

.SECONDEXPANSION:


EXE_SRC := test.c
SRC := $(EXE_SRC)

OBJ := $(SRC:%.c=%.o)
EXE := $(EXE_SRC:%.c=%)

CC := gcc
CFLAGS := -std=c99 -pedantic -g -Wall -Wextra -Werror -Wno-unused-function

RE2C_DIR := lib/re2c/re2c/


all: $(EXE) $(EXTRA_EXE)

$(OBJ): $$(patsubst %.o,%.c,$$@)
	$(CC) $(CFLAGS) -c -o $@ $<

$(EXE) $(EXTRA_EXE):
	$(CC) -o $@ $^

$(EXE): $$@.o

clean:
	rm -f $(OBJ) $(EXE)


test.c: test.re.c $(RE2C_DIR)re2c
	$(RE2C_DIR)re2c -8 -o $@ $<


cpic.o: common.h
dict.o: common.h dict.h fail.h
fail.o: fail.h common.h
arch_emr.o: arch_emr.h common.h fail.h cpic.h utils.h

cpic: dict.o fail.o arch_emr.o


# Some of these prereq lists are incomplete, but I don't care.

$(RE2C_DIR)configure: $(RE2C_DIR)autogen.sh
	cd $(RE2C_DIR) && ./autogen.sh

$(RE2C_DIR)Makefile: $(RE2C_DIR)configure
	cd $(RE2C_DIR) && ./configure

$(RE2C_DIR)re2c: $(RE2C_DIR)Makefile
	make -C $(RE2C_DIR)


.DEFAULT_GOAL := all
.PHONY: all clean




