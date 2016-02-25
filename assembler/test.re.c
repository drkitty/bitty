#include <stdbool.h>
#include <stdio.h>


#define endof(p) ((char *)p + sizeof(*p))

#define T_EOF 0
#define T_A 1
#define T_Z 2


void YYFILL(int n)
{
    (void)n;
}


#define YYCURSOR *cursor
#define YYCTYPE char
int scan(char** cursor, char* YYLIMIT)
{
    /*static char* YYMARKER;*/

/*!re2c

   "a" { return T_A; }
   "z" { return T_Z; }
   "\x00" { return T_EOF; }

*/

}
#undef YYCURSOR
#undef YYCTYPE


int main()
{
    char buf[] = "aazazz\x00";

    char* c = buf;
    while (true) {
        int t = scan(&c, endof(buf) - 1);
        if (t == T_EOF)
            break;
        printf("%d\n", t);
    }

    return 0;
}
