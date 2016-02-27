#include "common.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "fail.h"


#define endof(p) ((char *)p + sizeof(*p))

enum toktype {
    T_EOF,
    T_CHAR,
    T_TEXT,
    T_NUM,
};


struct lexer {
    char* cursor;
    char* limit;
    int line;
    char* line_start;
};


struct token {
    enum toktype type;
    char* s;
    int n;
};


static void print_token(struct token* t)
{
    switch (t->type) {
    case T_EOF: print("T_EOF"); break;
    case T_CHAR: printf("T_CHAR(%d)", t->n); break;
    case T_TEXT: {
        char c = t->s[t->n];
        t->s[t->n] = '\0';
        printf("T_TEXT(\"%s\")", t->s);
        t->s[t->n] = c;
        break;
    }
    case T_NUM: printf("T_NUM(%d)", t->n); break;
    default: fatal(E_RARE, "BUG");
    }
}


static void _lexer_fill(int n)
{
}


void lexer_init(struct lexer* lexer)
{
    lexer->line = 1;
    lexer->line_start = lexer->cursor;
}


struct token lexer_next(struct lexer* lexer)
{
    struct token t;

    while (true) {
        char* start = lexer->cursor;
#define COL (start - lexer->line_start + 1)
        /*!re2c
            re2c:define:YYFILL = "_lexer_fill";
            re2c:define:YYCTYPE = "char";
            re2c:define:YYCURSOR = "lexer->cursor";
            re2c:define:YYLIMIT = "lexer->limit";

            "\n" {
                t.type = T_CHAR;
                t.n = '\n';
                ++lexer->line;
                lexer->line_start = lexer->cursor;
                return t;
            }

            [a-zA-Z_][0-9a-zA-Z_]* {
                t.type = T_TEXT;
                t.s = start;
                t.n = lexer->cursor - start;
                return t;
            }

            [ \t]+ { continue; }

            "\x00" {
                t.type = T_EOF;
                return t;
            }

            * {
                fatal(
                    E_COMMON, "%s:%d:%d: Invalid character",
                    "STDIN", lexer->line, COL
                );
            }
        */
#undef COL
    }
}


int main(int argc, char** argv)
{
    if (argc != 2)
        fatal(E_USAGE, "Usage:  test STR");

    size_t len = strlen(argv[1]) + 2;
    char buf[len];
    memcpy(buf, argv[1], len);

    struct lexer lexer = {
        .cursor = buf,
        .limit = endof(buf) - 1,
    };
    lexer_init(&lexer);

    while (true) {
        struct token t = lexer_next(&lexer);
        if (t.type == T_EOF)
            break;
        print_token(&t);
        putchar('\n');
    }

    return 0;
}
