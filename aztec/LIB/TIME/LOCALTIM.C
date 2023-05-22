/* Copyright (C) 1984 by Manx Software Systems */
#include <time.h>

struct tm *
gmtime(clock)
long *clock;
{
	struct tm *localtime();

	return localtime(clock);
}

struct tm *
localtime(clock)
long *clock;
{
	register long l;
	static struct tm tm;
	static int days[12] = {
		-1, 30, 58, 89, 119, 150, 180, 211, 242, 272, 303, 333
	};

	l = *clock;
	tm.tm_sec = 0;
	tm.tm_min = l&0xff;
	tm.tm_hour = (l >> 8) & 0xff;
	tm.tm_mday = (l >> 16) & 0x1f;
	tm.tm_mon = ((l >> 21) & 0x0f) - 1;
	tm.tm_year = ((l >> 25) & 0x7f);
	tm.tm_yday = days[tm.tm_mon] + tm.tm_mday +
					(tm.tm_mon > 1 && (tm.tm_year&3) == 0);
	tm.tm_wday = (tm.tm_yday + tm.tm_year + ((tm.tm_year-1)>>2) + 1) % 7;
	return &tm;
}

