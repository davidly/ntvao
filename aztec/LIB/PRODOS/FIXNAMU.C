/* Copyright (C) 1985 by Manx Software Systems */
#include	<errno.h>

_fixnam(was, buf)
register char *was, *buf;
{
	register char *cp, *from, *ep;
	char *index();

	cp = buf;
	if (*was != '/') {
		getprefix(buf);
		strcat(buf, "/");
	}
	else
		*buf = 0;
	strcat(buf, was);
	from = buf;
	for (;;) {
		while (*from == '/')
			from++;
		*cp++ = '/';
		if (ep = index(from, '/'))
			*ep = 0;
		else {
			if (*from == '.' && (from[1] == 0 ||
										(from[1] == '.' && from[2] == 0)))
				goto skip;
			strcpy(cp, from);
			cp += strlen(cp);
			break;
		}

skip:
		if (strcmp(from, "..") == 0) {
			if (cp == buf+1) {
				errno = EINVAL;
				return(-1);
			}
			--cp;
			while (--cp > buf && *cp != '/')
				;
		}
		else if (strcmp(from, ".") == 0)
			--cp;
		else {
			cp--;
			while (*++cp = *from++)
				;
		}
		if (ep == 0)
			break;
		*ep++ = '/';
		from = ep;
	}
	if (cp == buf)
		*cp++ = '/';
	*cp = 0;
	return(0);
}

