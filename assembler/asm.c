#include "common.h"

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>

#include "fail.h"
#include "lexer.h"


int verbosity = 0;


int main()
{
    struct lexer* lexer = lexer_init(STDIN_FILENO);
    if (lexer == NULL)
        fatal_e(E_RARE, "Can't allocate memory");

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
