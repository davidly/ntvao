/* -------------------------------------------------------------------
 System       : ProDOS 8
 Environment  : Aztec C Shell
 Program      : sort.c
 Description  : Sort filter.

                This program is a sort filter. It optionally accepts
                a filename to read input from. All output goes to the
                standard output device unless redirected.

                Examples:
                  Sort abc.txt
                      read input from abc.txt,
                      output goes to screen.
                  Sort abc.txt >abcnew.txt
                      read input from abc.txt,
                      output goes to abcnew.txt.
                  Sort <abc.txt >abcnew.txt
                      same as previous example

 Written by   : Bill Buckels
 Date Written : Aug 2008
 Revision     : 1.0 First Release
 ------------------------------------------------------------------ */
#include <stdio.h>

char *malloc();
char *gets();
int cmp();

#define MAXLINE   80
#define FEEDTEST  78
#define MAXLINES  2048

FILE *fp = NULL;
char *lines[MAXLINES];
char buf[MAXLINE+1];


main(argc, argv)
int argc;
char **argv;
{
	int idx, c, len=0, numlines = 0;
	char *ptr;

    if (argc > 1)
      fp = fopen(argv[1], "r");

	for (;;)
	{
      if (NULL == fp)
        c = getchar();
      else
        c = fgetc(fp);

	  /* nobody should be sorting text lines longer than 80 */
	  /* what would be the point? */
	  if (c == '\n' || c == '\r' || len > FEEDTEST || c == EOF) {
		if (c != EOF && c != '\n' && c!='\r') {
			buf[len] = c;
			len++;
		}
		/* terminate the buffer */
		if (len != 0) {
			buf[len] = 0;
			len++;
			/* transfer the line into storage */
			ptr = malloc (len);
			if (NULL == ptr)break;
			lines[numlines] = ptr;
			strcpy (lines[numlines], buf);
			numlines++;
			len = 0;
		}
		if (numlines == MAXLINES || c == EOF) {
			break;
		}
	  }
	  else {
		buf[len] = c;
		len++;
	  }
	}
	if (numlines > 1)
	  qsort (lines, numlines, 2, cmp);
	for (idx=0; idx<numlines; idx++) printf ("%s\n", lines[idx]);

	if (numlines == 0)putchar('\n');

	if (NULL != fp) {
	      fclose(fp);
	      exit(0);
    }
}

cmp (a, b)
char **a, **b;
{
	return strcmp (*a, *b);
}
