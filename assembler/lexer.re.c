#include "common.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fail.h"
#include "lexer.h"


#define endof(p) ((char *)p + sizeof(*p))


/*!max:re2c*/
enum { lexer_maxfill = YYMAXFILL };
#undef YYMAXFILL


struct lexer {
    int fd;
    char buf[2 * lexer_maxfill + 1];
    char* cursor;
    char* limit;
    int line;
    int col;
    bool eof;

    char* tok;
    char* marker;
    char* ctxmarker;
};


void print_token(struct token* t)
{
    switch (t->type) {
    case T_EOF: print("T_EOF"); break;
    case T_REG: printf("r%d", t->n); break;
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
    if ((lexer->limit - lexer->buf) + want >= lexer_maxfill) {
        ssize_t keep_len = lexer->limit - lexer->tok;
        memmove(lexer->buf, lexer->tok, keep_len);
        lexer->limit -= lexer->tok - lexer->buf;
        lexer->cursor -= lexer->tok - lexer->buf;
        lexer->marker -= lexer->tok - lexer->buf;
        lexer->ctxmarker -= lexer->tok - lexer->buf;
        lexer->tok = lexer->buf;
    }

    if (want > lexer->buf + lengthof(lexer->buf) - lexer->limit)
        fatal(E_COMMON, "Token too long");

    while (want > 0) {
        ssize_t got = read(lexer->fd, lexer->limit, want);
        if (got == -1) {
            fatal_e(E_RARE, "Can't read from input file");
        } else if (got == 0) {
            for (char* c = lexer->limit; want > 0; --want, ++c)
                *c = '\0';
            return;
        }

        lexer->limit += got;
        want -= got;
    }
}


int _lexer_lex_int(char* str, char* end, int base)
{
    errno = 0;
    char c = *end;
    *end = '\0';
    int n = strtol(str, NULL, 10);
    if (errno != 0)
        fatal_e(E_COMMON, "Invalid base-%d integer", base);
    else if (base != 10 && n < 0)
        fatal(E_COMMON, "Base-%d integer cannot be negative", base);
    *end = c;
    return n;
}


struct lexer* lexer_init(int fd)
{
    v0("lexer_maxfill = %d", lexer_maxfill);

    struct lexer* lexer = malloc(sizeof(struct lexer));
    if (lexer != NULL) {
        lexer->fd = fd;
        lexer->line = 1;
        lexer->col = 1;
        lexer->limit = lexer->cursor = lexer->buf;
        lexer->eof = false;
    }
    return lexer;
}


struct token lexer_next(struct lexer* lexer)
{
    struct token t;

    while (true) {
        lexer->tok = lexer->cursor;
        v0("cursor = %d, limit = %d", lexer->cursor - lexer->buf,
            lexer->limit - lexer->buf);
#define TOKLEN (lexer->cursor - lexer->tok)
#define YYFILL(n) _lexer_fill(lexer, (n))
        /*!re2c
            re2c:define:YYCTYPE = "char";
            re2c:define:YYCURSOR = "lexer->cursor";
            re2c:define:YYLIMIT = "lexer->limit";
            re2c:define:YYMARKER = "lexer->marker";
            re2c:define:YYCTXMARKER = "lexer->ctxmarker";

            [ \t]+ {
                lexer->col += TOKLEN;
                continue;
            }

            "\n" {
                t.type = T_CHAR;
                t.n = '\n';
                ++lexer->line;
                lexer->col = 1;
                return t;
            }

            "0"|("-"?[1-9][0-9]*) / [ \t\n] {
                t.type = T_NUM;
                t.n = _lexer_lex_int(lexer->tok, lexer->cursor, 10);
                lexer->col += TOKLEN;
                return t;
            }

            "0x"[0-9A-Fa-f]+ / [ \t\n] {
                t.type = T_NUM;
                t.n = _lexer_lex_int(lexer->tok + 2, lexer->cursor, 16);
                lexer->col += TOKLEN;
                return t;
            }

            "0"[nr][01]+ / [ \t\n] {
                t.type = T_NUM;
                t.n = _lexer_lex_int(lexer->tok + 2, lexer->cursor, 2);
                lexer->col += TOKLEN;
                return t;
            }

            "r"([0-9]|("1"[0-5])) / [ \t\n] {
                t.type = T_REG;
                t.n = _lexer_lex_int(lexer->tok + 1, lexer->cursor, 10);
                lexer->col += TOKLEN;
                return t;
            }

            [a-zA-Z_][0-9a-zA-Z_]* {
                t.type = T_TEXT;
                t.s = lexer->tok;
                t.n = TOKLEN;
                lexer->col += TOKLEN;
                return t;
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
