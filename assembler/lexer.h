#pragma once


#include "common.h"


#define LEXER_BUF_CAP 8


enum toktype {
    T_EOF,
    T_CHAR,
    T_TEXT,
    T_NUM,
};


struct lexer {
    int fd;
    char buf[LEXER_BUF_CAP + 1];
    char* cursor;
    char* limit;
    int line;
    int col;

    char* tok;
};


struct token {
    enum toktype type;
    char* s;
    int n;
};


void print_token(struct token* t);
void lexer_init(struct lexer* lexer);
struct token lexer_next(struct lexer* lexer);
