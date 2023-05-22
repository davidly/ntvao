/* Copyright (C) 1981,1982 by Manx Software Systems */
#include "stdio.h"

aputc(c,ptr)
register int c; register FILE *ptr;
{
	if (c == '\n')
		c = '\r';
	return putc(c,ptr);
}

