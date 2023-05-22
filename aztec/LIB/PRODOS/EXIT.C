/*	Copyright (C) 1985 by Manx Software Systems, Inc. */

#include	<prodos.h>

static
noper()
{
	return(0);
}

int (*cls_)() = noper;

exit(n)
int n;
{
	register unsigned i;

	(*cls_)();
	for (i=0;i<MAXFILES;i++)
		if (_fil_tab[i].unit)
			close(i);     /* close all remaining "unbuffered" streams */
	_exit(n);
}

