/* Copyright (C) 1985 by Manx Software Systems, Inc. */
#include	<sysfunc.h>
#include	<errno.h>

unlink(nam)
register char *nam;
{
	register int err;
	char name[128];

	if (_fixnam(nam, name))
		return(-1);
	ctop(name);
	_sys_parm[0] = 1;
	*(char **)(_sys_parm + 1) = name;
	err = _system(SYS_DSTRY);
	if (err) {
		errno = err;
		return(-1);
	}
	return(0);
}

