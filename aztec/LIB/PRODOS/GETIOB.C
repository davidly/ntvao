/* Copyright (C) 1985 by Manx Software Systems */
#include	<prodos.h>
#include	<errno.h>

#define NULL 0

char *
_get_iob(fd)
{
	register int i, j;
	register char *cp;
	char *sbrk();

	if ((cp=_fil_tab[fd].iob) != 0)
found:
		return(_fil_tab[fd].iob = cp);
	for (i=0;i<MAXFILES;i++) {
		if (((j=_fil_tab[i].unit) == 0 || (j&0x80)) && (cp=_fil_tab[i].iob)) {
			_fil_tab[i].iob = 0;
			goto found;
		}
	}
	cp = sbrk(0);
	if (sbrk((((unsigned short)cp + 0x4ff)&0xff00)-cp) != -1) {
		cp = (char *)(((unsigned short)cp+0xff) & 0xff00);
		goto found;
	}
	errno = ENOMEM;
	return NULL;
}

