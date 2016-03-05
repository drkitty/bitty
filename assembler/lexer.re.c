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

#define BUFCAP 128


/*!max:re2c*/
enum { MAXFILL = YYMAXFILL };
#undef YYMAXFILL


struct lexer {
    int fd;
    char buf[BUFCAP + MAXFILL + 1];
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
    case T_NL: print("T_NL"); break;
    case T_REG: printf("r%d", t->n); break;
    case T_CHAR: printf("T_CHAR('%c')", t->n); break;
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


static void _lexer_fill(struct lexer* lxr, int want)
{
    v2("want = %d", want);
    if (lxr->eof)
        return;

    if (lxr->limit + want > lxr->buf + BUFCAP) {
        int diff = lxr->tok - lxr->buf;
        if (lxr->limit + want - diff > lxr->buf + BUFCAP)
            fatal(E_RARE, "Buffer won't fit requested data");
        memmove(lxr->buf, lxr->tok, lxr->limit - lxr->tok);
        lxr->tok = lxr->buf;
        lxr->cursor -= diff;
        lxr->limit -= diff;
        lxr->marker -= diff;
        lxr->ctxmarker -= diff;
    }

    while (want > 0) {
        ssize_t got = read(lxr->fd, lxr->limit, want);

        v2("got = %zd", got);

        if (got == -1) {
            fatal_e(E_RARE, "Can't read from input file");
        } else if (got == 0) {
            lxr->eof = true;
            for (char* c = lxr->limit; want > 0; --want, ++c)
                *c = '\0';
            return;
        }

        lxr->limit += got;
        want -= got;
    }
}


int _lexer_lex_int(char* str, char* end, int base)
{
    errno = 0;
    char c = *end;
    *end = '\0';
    int n = strtol(str, NULL, base);
    if (errno != 0)
        fatal_e(E_COMMON, "Invalid base-%d integer", base);
    else if (base != 10 && n < 0)
        fatal(E_COMMON, "Base-%d integer cannot be negative", base);
    *end = c;
    return n;
}


struct lexer* lexer_init(int fd)
{
    struct lexer* lxr = malloc(sizeof(struct lexer));
    if (lxr != NULL) {
        lxr->fd = fd;
        lxr->line = 1;
        lxr->col = 1;
        lxr->limit = lxr->cursor = lxr->buf;
        lxr->eof = false;
    }
    return lxr;
}


struct token lexer_next(struct lexer* lxr)
{
    struct token t;

    while (true) {
        lxr->tok = lxr->cursor;
        v2("cursor = %d, limit = %d", lxr->cursor - lxr->buf,
            lxr->limit - lxr->buf);
#define TOKLEN (lxr->cursor - lxr->tok)
#define YYFILL(n) _lexer_fill(lxr, (n))
        /*!re2c
            re2c:define:YYCTYPE = "char";
            re2c:define:YYCURSOR = "lxr->cursor";
            re2c:define:YYLIMIT = "lxr->limit";
            re2c:define:YYMARKER = "lxr->marker";
            re2c:define:YYCTXMARKER = "lxr->ctxmarker";

            [ \t]+ {
                lxr->col += TOKLEN;
                continue;
            }

            "\n"+ {
                t.type = T_NL;
                lxr->line += TOKLEN;
                lxr->col = 1;
                return t;
            }

            [:,] {
                t.type = T_CHAR;
                t.n = *lxr->tok;
                ++lxr->col;
                return t;
            }

            "0"|("-"?[1-9][0-9]*) / [,: \t\n] {
                t.type = T_NUM;
                t.n = _lexer_lex_int(lxr->tok, lxr->cursor, 10);
                lxr->col += TOKLEN;
                return t;
            }

            "0x"[0-9A-Fa-f]+ / [,: \t\n] {
                t.type = T_NUM;
                t.n = _lexer_lex_int(lxr->tok + 2, lxr->cursor, 16);
                lxr->col += TOKLEN;
                return t;
            }

            "0"[nr][01]+ / [,: \t\n] {
                t.type = T_NUM;
                t.n = _lexer_lex_int(lxr->tok + 2, lxr->cursor, 2);
                lxr->col += TOKLEN;
                return t;
            }

            "r"([0-9]|("1"[0-5])) / [,: \t\n] {
                t.type = T_REG;
                t.n = _lexer_lex_int(lxr->tok + 1, lxr->cursor, 10);
                lxr->col += TOKLEN;
                return t;
            }

            [a-zA-Z_][0-9a-zA-Z_]* {
                t.type = T_TEXT;
                t.s = lxr->tok;
                t.n = TOKLEN;
                lxr->col += TOKLEN;
                return t;
            }

            "\x00" {
                t.type = T_EOF;
                return t;
            }

            * {
                fatal(
                    E_COMMON, "%s:%d:%d: Invalid character",
                    "STDIN", lxr->line, lxr->col
                );
            }
        */
#undef TOKLEN
#undef YYFILL
    }
}
