/* Copyright (C) 1985 by Manx Software Systems */
#include	<time.h>

get_time(tm)
register struct tm *tm;
{
	long t=time();
	register struct tm *ptr;
	ptr=localtime(&t);
	*tm = *ptr;
} 

