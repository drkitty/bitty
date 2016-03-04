#include "common.h"

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "fail.h"
#include "lexer.h"


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
