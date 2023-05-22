/* Copyright (C) 1983,84,85 by Manx Software Systems */

#define OVMAGIC	0xf1

struct header {
	int magic;
	unsigned ovaddr;
	unsigned ovsize;
	unsigned ovbss;
	int (*ovbgn)();
};

static char *ovname;

#asm
	instxt	<zpage.h>

	public	ovloader_
ovloader_
	ldy	#2
	lda	(SP),Y
	sta	ovname_
	iny
	lda	(SP),Y
	sta	ovname_+1
;
	jsr	_ovld_
	jmp	(R0)
#endasm

static
_ovld()
{
	int fd;
	auto struct header hdr;
	extern char *_mbot;
	auto char filename[64];
	
	strcpy(filename, ovname);
	for (;;) {
		strcat(filename, ".ovr");
#ifdef PATH
		if (path(filename) && (fd = open(filename, 0)) >= 0)
			break;
#else
		if ((fd = open(filename, 0)) >= 0)
			break;
#endif
		loadabort(10);
	}

	if (read(fd, &hdr, sizeof hdr) != sizeof hdr)
		loadabort(20);

	/* check magic number on overlay file */
	if (hdr.magic != OVMAGIC)
		loadabort(30);

	if (_mbot < (char *)(hdr.ovaddr+hdr.ovsize+hdr.ovbss))
		loadabort(40);

	if (read(fd, hdr.ovaddr, hdr.ovsize) < hdr.ovsize)
		loadabort(50);
	close(fd);
	return hdr.ovbgn;
}

static
loadabort(code)
{
	printf("Error %d loading overlay: %s\n", code, ovname);
	exit(10);
}

#ifdef PATH
#include	<prodos.h>

static
path(file)
char *file;
{
	register char *cp, *tp;
	register int i;
	struct finfo f;
	char buf[64];
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
		if (getfinfo(buf, &f) == 0) {
			strcpy(file, buf);
			return(1);
		}
		cp = tp;
		if (*cp)
			cp++;
	}
	return(0);
}
#endif

