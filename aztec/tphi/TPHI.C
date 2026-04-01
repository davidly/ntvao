#include <stdio.h>

#define A1_SUPPORT_SHOW    0
#define A1_SUPPORT_LONG    1
#define A1_SUPPORT_KBD     0
#define A1_SUPPORT_PR_SXX  1
#include "a1.c"

typedef unsigned long uint32_t;

static s_ac[ 100 ];

int main()
{
    uint32_t limit = 40;
    uint32_t prev2 = 1;
    uint32_t prev1 = 1;
    uint32_t next, i, last_shown = 0;
    int len;
    double d;

    pr_s( "should tend towards 1.61803398874989484820458683436563811772030\n" );

    for ( i = 1; i <= limit; i++ )
    {
        next = prev1 + prev2;
        prev2 = prev1;
        prev1 = next;

        if ( i == ( last_shown + 5 ) )
        {
            last_shown = i;
            d = (double) prev1 / (double) prev2;
            pr_sls( "  at ", i, " iterations: " );
            ftoa( d, s_ac, 50, 1 );
            pr_s( s_ac );
            pr_s( "\n" );
        }
    }

    pr_s( "done\n" );
    bye();
    return 0;
}
