#include	<prodos.h>
#include	<errno.h>

execlp(file, arg0)
char *file, *arg0;
{
	return(execvp(file, &arg0));
}

execvp(file, argv)
char *file, **argv;
{
	register char *cp, *tp;
	register int i;
	struct finfo f;
	char buf[128];
	char pbuf[128];
	char *getenv(), *index();

	tp = pbuf;
	*tp = 0;

	if (*file == '/') {		/* don't use path	*/
		strcpy(buf, file);
		goto skip;
	}

	if (cp = getenv("PATH"))		/* get the path	*/
		strcpy(pbuf, cp);
	else {
		*tp = ':';
		tp[1] = 0;
	}
	cp = pbuf;
	while (*cp) {
		tp = index(cp, ':');
		if (tp == 0)
			tp = cp + strlen(cp);
		i = tp - cp;
		strncpy(buf, cp, i);			/* construct a path name */
		buf[i] = 0;
		if (i && buf[i-1] != '/')
			strcat(buf, "/");
		strcat(buf, file);
skip:
		if (_getfinfo(buf, &f) == 0) {
			if ((i=f.file_type) == 0xff || i == 0xf8 || i == 0x06) {
				return(execv(buf, argv));
			}
		}
		cp = tp;
		if (*cp)
			cp++;
	}
	errno = ENOENT;
	return(-1);
}

