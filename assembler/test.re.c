#include "common.h"

#include <stdbool.h>
#include <stdio.h>

#include "fail.h"


#define endof(p) ((char *)p + sizeof(*p))

enum toktype {
    T_EOF,
    T_CHAR,
    T_TEXT,
    T_NUM,
};


struct lexer {
    char *cursor;
    char *limit;
};


struct token {
    enum toktype type;
    char* s;
    int n;
};


void print_token(struct token* t)
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


void _lexer_fill(int n)
{
    (void)n;
}


struct token lexer_next(struct lexer* lexer)
{
    struct token t;

    while (true) {
        char* start = lexer->cursor;
        /*!re2c
            re2c:define:YYFILL = "_lexer_fill";
            re2c:define:YYCTYPE = "char";
            re2c:define:YYCURSOR = "lexer->cursor";
            re2c:define:YYLIMIT = "lexer->limit";

            "\n" {
                t.type = T_CHAR;
                t.n = '\n';
                return t;
            }

            [a-zA-Z_][0-9a-zA-Z_]* {
                t.type = T_TEXT;
                t.s = start;
                t.n = lexer->cursor - start;
                return t;
            }

            [1-9][0-9]* {
                t.type = T_NUM;
                t.s = start;
                t.n = lexer->cursor - start;
                return t;
            }

            [ \t]+ { continue; }

            "\x00" {
                t.type = T_EOF;
                return t;
            }

            * { fatal(E_COMMON, "Invalid character"); }
        */
    }
}


int main()
{
    char buf[] = "abc ABC\n\n\n_0189_\n    _ _ _  \n\na1a1\x00";
    struct lexer lexer = {
        .cursor = buf,
        .limit = endof(buf) - 1,
    };

    while (true) {
        struct token t = lexer_next(&lexer);
        if (t.type == T_EOF)
            break;
        print_token(&t);
        putchar('\n');
    }

    return 0;
}
