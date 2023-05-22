/*	Copyright (C) 1985 by Manx Software Systems, Inc. */

extern struct shvar {
	char	vects[6];
	int		retflg;
	char *	env;
	int		envlen;
	int		envmax;
} *_Sp;

char *
getenv(name)
char *name;
{
	register int i;
	register char *cp, *ep;
	static char *buf;
	char *index(), *malloc();

	if (_Sp == 0)
		return(0);
	cp = _Sp->env;
	i = strlen(name);
	while (*cp) {
		if (cp[i] == '=' && strncmp(cp, name, i) == 0) {
			cp += i + 1;
			if (buf)
				free(buf);
			buf = malloc(strlen(cp)+1);
			if (buf)
				strcpy(buf, cp);
			return(buf);
		}
		cp += strlen(cp) + 1;
	}
	return((char *)0);
}

