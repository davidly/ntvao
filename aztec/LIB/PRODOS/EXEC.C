/*	Copyright (C) 1985 by Manx Software Systems, Inc. */
#include	<prodos.h>
#include	<sgtty.h>
#include	<device.h>

#define INITLEN 64

extern struct shvar {
	char			vects[6];
	int				retflg;
	char *			env;
	int				envlen;
	int				envmax;
	char *			argstk;
	unsigned char	flags;
	char			shdir[32];
	char			batch_file[32];
	unsigned short	batch_pos;
	struct _fil_buf fil_tab[MAXFILES];
	struct _dev_info dev_info;
	char			ibuf[INITLEN];  /* space for slot init strings */
	char *			iobbase;	/* ptr to base of iobs */
	struct args *	batch_args; /* base of batch args */
} *_Sp;

execl(file, arg0)
char *file, *arg0;
{
	return(execv(file, &arg0));
}

execv(arg, argv)
register char *arg, **argv;
{
	register char **p;
	register char *cp;
	register unsigned short i, argc;
	register int *ap;
	register int j;
	unsigned short mch_env;
	unsigned int iobspace;
	int (*func)();

	if (_Sp == 0)
		return(-1);
	strcpy(0x280, arg);
	ctop(0x280);

	mch_env = (unsigned short) _Sp;
	iobspace = mch_env - (unsigned short) _Sp->iobbase;
	cp = (char *)mch_env - 8;
	for (argc=0,p=argv;*p;argc++,p++)
		cp -= strlen(*p) + 1;;
	p = (char **) (cp - ((argc+1) * sizeof(char *)));
	ap = (int *)(p - 3);
	ap[0] = 0;
	ap[1] = argc;
	*((char **)(ap+2)) = (char *)p - iobspace;
	for (i=0;i<argc;i++) {
		strcpy(cp, *argv++);
		*p++ = cp - iobspace;
		cp += strlen(cp) + 1;
	}
	*p++ = 0;
	_Sp->argstk = (char *)ap;

	*(int *)0x73 = mch_env - iobspace-8;
	*(int *)0x4c = mch_env - iobspace-8;
	*(int *)0x3f2 = mch_env;
	*(char *)0x3f4 = (mch_env>>8) ^ 0xa5;

	for (i=0;i<MAXFILES;i++)
		if (((j=_fil_tab[i].unit) == 0 || (j&0x80)) &&
									_fil_tab[i].iob < _Sp->iobbase)
			_fil_tab[i].iob = 0;
	func = (char *) (mch_env + 3);
	return((*func)());
}

