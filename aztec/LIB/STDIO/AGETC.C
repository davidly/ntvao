/* Copyright (C) 1981,1982 by Manx Software Systems */
#include "stdio.h"

agetc(ptr)
register FILE *ptr;
{
	register int c;

	if ((c = getc(ptr)) == '\r')
		c = '\n';
	return c;
}

