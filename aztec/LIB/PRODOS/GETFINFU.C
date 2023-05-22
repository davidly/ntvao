/* Copyright (C) 1985 by Manx Software Systems */
#include	<prodos.h>
#include	<sysfunc.h>
#include	<errno.h>

_getfinfo(nam, fp)
register char *nam;
struct finfo *fp;
{
	register int err;
	char name[128];

	if (_fixnam(nam, name))
		return(-1);
	*_sys_parm = 10;
	ctop(name);
	*(char **)(_sys_parm + 1) = name;
	if ((err = _system(SYS_GINFO)) == 0)
		*fp = *(struct finfo *)(_sys_parm + 3);
	if (err) {
		errno = err;
		return(-1);
	}
	return(0);
} 

