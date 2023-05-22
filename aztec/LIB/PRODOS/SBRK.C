/* Copyright (C) 1985 by Manx Software Systems, Inc. */
#include	<errno.h>

#define NULL 0

extern char *_Top, *_End;
extern unsigned short _Stksiz;

char *
sbrk(size)
register unsigned size;
{
	register char *tmp;

	if (_Top+size > _End-_Stksiz || (((long)_Top+size) & 0xffff0000L)) {
		errno = ENOMEM;
		return(-1);
	}
	tmp = _Top;
	_Top += size;
	return(tmp);
}

