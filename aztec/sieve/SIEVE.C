/*  sieve.c */

/* Eratosthenes Sieve Prime Number Program in C from Byte Jan 1983
   to compare the speed. */

#include <stdio.h>

#define A1_SUPPORT_SHOW    0
#define A1_SUPPORT_LONG    0
#define A1_SUPPORT_KBD     0
#define A1_SUPPORT_PR_SXX  0
#include "a1.c"

#define TRUE 1
#define FALSE 0
#define SIZE 8190

char flags[SIZE+1];

int main()
        {
        int i,k;
        int prime,count,iter;

        for (iter = 1; iter <= 10; iter++) {    /* do program 10 times */
                count = 0;                      /* initialize prime counter */
                for (i = 0; i <= SIZE; i++)     /* set all flags true */
                        flags[i] = TRUE;
                for (i = 0; i <= SIZE; i++) {
                        if (flags[i]) {         /* found a prime */
                                prime = i + i + 3;      /* twice index + 3 */
                                for (k = i + prime; k <= SIZE; k += prime)
                                        flags[k] = FALSE;       /* kill all multiples */
                                count++;                /* primes found */
                                }
                        }
                }
        pr_i( count );
        pr_s(" primes.\n");
        bye();
        return 0;
        }
