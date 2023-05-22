/* Copyright (C) 1985 by Manx Software Systems */
#include	<prodos.h>
#include	<errno.h>

access(file, mode)
char *file;
{
	register unsigned short i;
	struct finfo f;

	if (_getfinfo(file, &f))
		return(-1);
	switch(mode) {
	case 0:
		return(0);
	case 1:
		if ((i=f.storage_type) == 0x0d || i == 0x0f) {
			if ((f.access&0xc3) == 0xc3)
				return(0);
		}
		else if ((i=f.file_type) == 0x06 || i == 0xf8 || i == 0xff)
			return(0);
		errno = EACCES;
		return(-1);
	case 2:
		if (f.access & 0x02)
			return(0);
		errno = EACCES;
		return(-1);
	case 4:
		if (f.access & 0x01)
			return(0);
		errno = EACCES;
		return(-1);
	}
	errno = EACCES;
	return(-1);
}

