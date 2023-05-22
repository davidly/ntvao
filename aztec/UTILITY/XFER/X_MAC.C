/*	Copyright (C) 1985 by Manx Software Systems, Inc.		*/

#include	<sgtty.h>
#include	<serial.h>

/*
 *	This routine is designed for use on an Apple Macintosh port A.
 */

struct bauds {
	short baud;
	short val;
} Bauds[] = {
  300, baud300,
  600, baud600,
 1200, baud1200,
 1800, baud1800,
 2400, baud2400,
 3600, baud3600,
 4800, baud4800,
 7200, baud7200,
 9600, baud9600,
19200, baud19200,
00000, 0x000 };

short Refout, Refin;
struct sgttyb Tty_state, Term;

cinit(baud)
int baud;
{
	register int i;
	static char inbuf[1024];

	i = -1;
	while (Bauds[++i].baud)
		if (Bauds[i].baud == baud)
			break;
	OpenDriver("\P.ain", &Refin);
	OpenDriver("\P.aout", &Refout);
	SerReset(Refin, Bauds[i].val | data8 | stop20 | noParity);
	SerReset(Refout, Bauds[i].val | data8 | stop20 | noParity);
	SerSetBuf(Refin, inbuf, 1024);
	ioctl(0, TIOCGETP, &Tty_state);
	Term = Tty_state;
	Term.sg_flags &= ~ECHO;
	Term.sg_flags |= CBREAK | NODELAY;
	ioctl(0, TIOCSETP, &Term);
}

cflush()
{
	SerSetBuf(Refin, 0L, 0);
	ioctl(0, TIOCSETP, &Tty_state);
}

cput(c)
char c;
{
	long i = 1;

	FSWrite(Refout, &i, &c);
}

cget()
{
	register int j;
	char c;
	long l;

	for (j=0;j<100;j++) {
		SerGetBuf(Refin, &l);
		if (l > 0) {
			l = 1;
			FSRead(Refin, &l, &c);
			return(c&0xff);
		}
		if (read(0, &c, 1) == 1) {
			if (c == 3)
				abort();
		}
	}
	return(-1);
}

clrchan()
{
	char c = 0;
	long l;

	for (;;) {
		SerGetBuf(Refin, &l);
		if (l > 0) {
			l = 1;
			FSRead(Refin, &l, &c);
		}
		else
			return(c&0xff);
	}
}

