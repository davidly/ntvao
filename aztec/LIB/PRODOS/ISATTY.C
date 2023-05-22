/* Copyright (C) 1985 by Manx Software Systems */
#include	<prodos.h>

isatty(fd)
register int fd;
{
	register unsigned i;

	if (fd < 0 || fd > MAXFILES || ((i = _fil_tab[fd].unit) & 0x80) == 0)
		return 0;
	return ((i&0x40) != 0);
}

