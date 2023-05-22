#include <stdio.h>

#define	TMPEXT	".TMP"
#define	PFX_SIZE	5

static char work[] = "aaaa";

char *
tmpnam (s)
char *s;
{
	static char tmpbuf[L_tmpnam];
	void bump ();

	if (s==NULL)
		s = tmpbuf;
	strcpy (s,P_tmpdir);
	strcat (s,work);
	strcat (s,TMPEXT);
	if ( access (s,0) == 0 )  {
		bump();
		return s;
	}
	return NULL;
}

/* the temp name will be of the form:  dir/pfxTEMPNAM.EXT,
	where TEMPNAM.EXT is determined as above.  The given
	directory is checked for validity.
	if dir is NULL, check env for TMPDIR, then use P_tmpdir */

char *
tempnam (dir, pfx)
char *dir, *pfx;
{
	char state, slash;
	int len;
	register char *tbuf;
	struct stat sbuf;
	char *getenv ();
	void bump ();

	for (state=0, tbuf = getenv ("TMPDIR"); 
							stat (tbuf, &sbuf) == -1; state++)  {
retry:
		if (dir == NULL || state == 1)
			tbuf = P_tmpdir;
		else if (state == 0)
			tbuf = dir;
		else
			return NULL;
	}

	slash = 0;
	if ( (tbuf = strrchr (dir, '/')) || (tbuf = strrchr (dir,'\\')) ) {
		if (*(tbuf+1))	/* if slash is not last char of dir */
			slash = 1;
	} else
		slash = 1;
	len = strlen (dir);
	if (pfx) {
		int i;
		if ((i = strlen (pfx)) > PFX_SIZE)
			i = PFX_SIZE;
		len += i;
	}
	len += 10;		/* this is enough for aaaa.TMP\0 and possible slash */
	if ((tbuf = malloc (len)) != NULL)  {
		strcpy (tbuf, dir);
		if (slash)
			strcat (tbuf, "/");
		if (pfx)
			strncat (tbuf, pfx, PFX_SIZE);	/* still null-term'd */
		strcat (tbuf, work);
		strcat (tbuf, TMPEXT);
		if ( access (tbuf,0) == 0 )
			bump ();
		else
			goto retry;
	}
	return tbuf;
}

static void
bump ()
{
	int carry = 1;
	register char *cp = work;

	for (; carry && *cp; cp++) {
		if (*cp == 'z') {
			*cp = 'a';
		} else {
			(*cp)++;
			carry = 0;
		}
	}
}
