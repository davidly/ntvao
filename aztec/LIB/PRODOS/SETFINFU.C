/* Copyright (C) 1985 by Manx Software Systems, Inc. */
#include	<prodos.h>
#include	<sysfunc.h>
#include	<errno.h>

_setfinfo(nam, fp)
register char *nam;
struct finfo *fp;
{
	register int err;
	char name[128];

	if (_fixnam(nam, name))
		return(-1);
	_sys_parm[0] = 7;
	ctop(name);
	*(char **)(_sys_parm + 1) = name;
	*(struct finfo *)(_sys_parm + 3) = *fp;
	err = _system(SYS_SINFO);
	if (err) {
		errno = err;
		return(-1);
	}
	return(0);
}

