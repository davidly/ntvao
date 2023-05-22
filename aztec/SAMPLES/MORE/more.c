/* -------------------------------------------------------------------
 System       : ProDOS 8 or DOS 3.3
 Environment  : Aztec C Shell
 Program      : more.c
 Description  : paged or line oriented text file viewer

                This program is a filter that displays a text file
                either page by page if the spacebar is pressed
                or line by line if enter is pressed.

                If the [ESCAPE] key or the letter 'Q' is pressed
                the program ends.

                It will optionally accept a filename to read input
                from. All output is to the standard output device
                unless redirected.

                This does a better job displaying text files like
                readme files than the built-in shell command cat,
                but is nonetheless primitive.

                It is oriented to the 80 column x 24 row display.

                Example:
                  more abc.txt
                      read input from abc.txt,
                      output goes to screen.
                  more <abc.txt
                      same as previous example

 Written by   : Bill Buckels
 Date Written : Aug 2008
 Revision     : 1.0 First Release
 ------------------------------------------------------------------ */
#include <stdio.h>

main(argc, argv)
int argc;
char **argv;
{
    int c, d,ctr = 0;
    FILE *fp = NULL;

    if (argc > 1)
      fp = fopen(argv[1], "r");

    for (;;) {
      if (NULL == fp)
        c = getchar();
      else
        c = fgetc(fp);

      if (c==EOF)break;


      putchar(c);
      if(c == 13 || c==10) {
	  	  ctr++;
	  	  if (ctr > 22) {
		      d = getch();
		      if (d == 27 || d=='q' || d == 'Q')break;
		      else if (d == 32)ctr = 0;
		      else if (d == 13)ctr = 22;
		  }

	  }

    }

    if (NULL != fp) {
      fclose(fp);
      exit(0);
    }
}

getch()
{
  /* return the last key press */
  char *key_press = (char*)0xC000;
  /* clear the last key press */
  char *key_clear = (char*)0xC010;
  char c;

	/* clear stragglers from the keyboard buffer */
	while((c=key_press[0]) > 127)key_clear[0]=0;

	/* read the keyboard buffer */
	/* and return the character */
	do{
	   c = key_press[0];
	   }while(c < 128);

	c-=128;
	key_clear[0]=0;
	return (int )c;
}