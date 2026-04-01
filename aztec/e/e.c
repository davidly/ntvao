/*
   Find e for the Apple 1.
   This version builds with Aztec CG65 v3.2c 10-2-89 on MS-DOS targeting 6502.
   That compiler expects an Apple II, so there are hacks here for the Apple 1.
   There are dependencies on start.a, which initialzes for the Apple 1.
   Aztec C treats the char type as unsigned, and expands to 16 bits for any expression (slow!)
*/

#define LINT_ARGS

#include <stdio.h>

#define A1_SUPPORT_SHOW    0
#define A1_SUPPORT_LONG    0
#define A1_SUPPORT_KBD     0
#define A1_SUPPORT_PR_SXX  0
#include "a1.c"

#define true 1
#define false 0

typedef char ttype;  /* 8-bit and 16-bit cpus do best with char aside from register in locals */

#define DIGITS_TO_FIND 200 /*9009;*/

int a[ DIGITS_TO_FIND ];

int main()
{
    int N, x, n;

    N = DIGITS_TO_FIND;
    x = 0;

    for (n = N - 1; n > 0; --n)
        a[n] = 1;

    a[1] = 2, a[0] = 0;
    while (N > 9)
    {
        n = N--;
        while (--n)
        {
            a[n] = x % n;
            x = 10 * a[n-1] + x/n;
        }
        pr_i( x );
    }

    pr_c( '\r' );

    /* The C runtime doesn't know how to exit or even return to the entry proc
       on an Apple 1, so exit with bye() */

    bye();

    return 0;
} /*main*/

