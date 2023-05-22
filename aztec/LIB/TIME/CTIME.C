/* Copyright (C) 1984 by Manx Software Systems */
#include <time.h>

char *
ctime(clock)
long *clock;
{
	struct tm *tm;

	tm = localtime(clock);
	return asctime(tm);
}
