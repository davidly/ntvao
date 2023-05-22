/* Copyright (C) 1985 by Manx Software Systems, Inc. */
#include	<errno.h>

#define NULL 0

extern char *_Top;
extern unsigned short _Stksiz;

brk(ptr)
register void *ptr;
{
	register void *tmp;

	if (ptr > *((char **) 2) - _Stksiz) {
		errno = ENOMEM;
		return(-1);
	}
	_Top = ptr;
	return(0);
}

