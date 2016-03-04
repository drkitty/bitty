#include "common.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "fail.h"


#define endof(p) ((char *)p + sizeof(*p))

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


static void _lexer_fill(struct lexer* lexer, int want)
{
    if ((lexer->limit - lexer->buf) + want >= LEXER_BUF_CAP) {
        ssize_t keep_len = lexer->limit - lexer->tok;
        memmove(lexer->buf, lexer->tok, keep_len);
        lexer->limit -= lexer->tok - lexer->buf;
        lexer->cursor -= lexer->tok - lexer->buf;
        lexer->tok = lexer->buf;
    }

    if (want > lexer->buf + LEXER_BUF_CAP - lexer->limit)
        fatal(E_COMMON, "Token too long");

    while (want > 0) {
        ssize_t got = read(lexer->fd, lexer->limit, want);
        if (got == -1) {
            fatal_e(E_RARE, "Can't read from input file");
        } else if (got == 0) {
            *lexer->limit = 0;
            ++lexer->limit;
            break;
        }

        lexer->limit += got;
        want -= got;
    }
}


void lexer_init(struct lexer* lexer)
{
    lexer->line = 1;
    lexer->col = 1;
}


struct token lexer_next(struct lexer* lexer)
{
    struct token t;

    while (true) {
        lexer->tok = lexer->cursor;
#define TOKLEN (lexer->cursor - lexer->tok)
#define YYFILL(n) _lexer_fill(lexer, (n))
        /*!re2c
            re2c:define:YYCTYPE = "char";
            re2c:define:YYCURSOR = "lexer->cursor";
            re2c:define:YYLIMIT = "lexer->limit";

            "\n" {
                t.type = T_CHAR;
                t.n = '\n';
                ++lexer->line;
                lexer->col = 1;
                return t;
            }

            [a-zA-Z_][0-9a-zA-Z_]* {
                t.type = T_TEXT;
                t.s = lexer->tok;
                t.n = TOKLEN;
                lexer->col += TOKLEN;
                return t;
            }

            [ \t]+ {
                lexer->col += TOKLEN;
                continue;
            }

            "\x00" {
                t.type = T_EOF;
                return t;
            }

            * {
                fatal(
                    E_COMMON, "%s:%d:%d: Invalid character",
                    "STDIN", lexer->line, lexer->col
                );
            }
        */
#undef TOKLEN
#undef YYFILL
    }
}


int main()
{
    struct lexer* lexer = malloc(sizeof(*lexer));
    if (lexer == NULL)
        fatal_e(E_RARE, "Can't allocate memory");

    lexer->fd = STDIN_FILENO;
    lexer->limit = lexer->cursor = lexer->buf;
    lexer_init(lexer);

    while (true) {
        struct token t = lexer_next(lexer);
        if (t.type == T_EOF)
            break;
        print_token(&t);
        putchar('\n');
    }

    free(lexer);

    return 0;
}
