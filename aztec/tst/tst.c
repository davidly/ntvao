/*
   Test app for the Apple 1.
   This version builds with Aztec CG65 v3.2c 10-2-89 on MS-DOS targeting 6502.
   That compiler expects an Apple II, so there are hacks here for the Apple 1.
   There are dependencies on start.a, which initialzes for the Apple 1.
   Aztec C treats the char type as unsigned, and expands to 16 bits for any expression (slow!)
*/

#define LINT_ARGS

#include <stdio.h>

#define A1_SUPPORT_SHOW    0
#define A1_SUPPORT_LONG    1
#define A1_SUPPORT_KBD     1
#define A1_SUPPORT_PR_SXX  1
#include "a1.c"

#define bufsize 80
static char buf[ bufsize ];

int main()
{
    int len;
    char ch;

    pr_s( "hello woz\n" );

    pr_s( "type a string then <ENTER>: " );
    len = get_input( buf, bufsize );

    pr_s( "string entered: '" );
    pr_s( buf );
    pr_s( "'\n" );

    pr_s( "enter a single key (not echoed): " );
    ch = getch();
    pr_s( "\n" );
    pr_sis( "integer value of key: ", ch, "\n" );
    pr_s( "char form of key: " );
    pr_c( ch );
    pr_s( "\n" );

    pr_s( "enter a single key (echoed): " );
    ch = getche();
    pr_s( "\n" );
    pr_sis( "integer value of key: ", ch, "\n" );
    pr_s( "char form of key: " );
    pr_c( ch );
    pr_s( "\n" );

    pr_sis( "integer value -6543: ", -6543, "\n" );
    pr_sls( "long value -6543210: ", -6543210, "\n" );

    pr_s( "press any key to exit\n" );
    while ( !kbhit() );

    bye();
    return 0;
} /*main*/

