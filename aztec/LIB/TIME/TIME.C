/* Copyright (C) 1985 by Manx Software Systems */
#include	<prodos.h>
#include	<sysfunc.h>
#include	<errno.h>
#include	<time.h>

/* tloc format

	00-07	minute
	08-15	hour
	16-20	day
	21-24	month
	25-31	year

*/

time_t
time(tloc)
time_t *tloc;
{
	register int err;
	register time_t l, save;

	save = *(long *)0xbf90;
	*(long *)0xbf90 = 0;
	if (err = _system(SYS_TIME)) {
		errno = err;
		return(-1L);
	}
	l = *(unsigned short *)0xbf90;
	l = (l<<16) | *(unsigned short *)0xbf92;
	if (l == 0) {
		l = (save>>16) | (save<<16);
		*(long *)0xbf90 = save;
	}
	if (tloc)
		*tloc = l;
	return(l);
} 

