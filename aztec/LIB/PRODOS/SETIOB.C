/* Copyright (C) 1985 by Manx Software Systems */
#include	<prodos.h>
#include	<errno.h>

setiob(cnt)
{
	register int i, tally;
	register char *cp;
	char *sbrk();

	tally = 0;
	for (i=0;i<MAXFILES;i++)
		if (_fil_tab[i].iob)
			tally++;
	for (i=0;i<MAXFILES && tally < cnt;i++) {
		if (_fil_tab[i].iob || _fil_tab[i].unit)
			continue;
		cp = sbrk(0);
		sbrk((((unsigned short)cp + 0x4ff)&0xff00)-cp);
		_fil_tab[i].iob = (char *)(((unsigned short)cp+0xff) & 0xff00);
		tally++;
	}
}

