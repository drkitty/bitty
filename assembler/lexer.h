#pragma once


#include "common.h"


enum toktype {
    T_EOF,
    T_REG,
    T_CHAR,
    T_TEXT,
    T_NUM,
};


struct token {
    enum toktype type;
    char* s;
    int n;
};


void print_token(struct token* t);
struct lexer* lexer_init(int fd);
struct token lexer_next(struct lexer* lexer);
