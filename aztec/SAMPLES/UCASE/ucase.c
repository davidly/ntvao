/* -------------------------------------------------------------------
 System       : ProDOS 8 or DOS 3.3
 Environment  : Aztec C Shell
 Program      : ucase.c
 Description  : Lowercase to Uppercase filter.

                This program is a filter that translates all lowercase
                characters to uppercase.  It will optionally accept a
                filename to read input from.  All output goes to the
                standard output device unless redirected.

                Examples:
                  Ucase abc.txt
                      read input from abc.txt,
                      output goes to screen.
                  Ucase abc.txt >abcnew.txt
                      read input from abc.txt,
                      output goes to abcnew.txt.
                  Ucase <abc.txt >abcnew.txt
                      same as previous example

 Written by   : Bill Buckels
 Based On     : upper.asm
                used with permission
 Date Written : Aug 2008
 Revision     : 3.0 Third Release
 ------------------------------------------------------------------ */
#include <stdio.h>

main(argc, argv)
int argc;
char **argv;
{
    int c;
    FILE *fp = NULL;

    if (argc > 1)
      fp = fopen(argv[1], "r");

    for (;;) {
      if (NULL == fp)
        c = getchar();
      else
        c = fgetc(fp);

      if (c==EOF)break;

      if(c>96 && c<123)c-=32;

      putchar(c);
    }

    if (NULL != fp) {
      fclose(fp);
      exit(0);
    }
}
