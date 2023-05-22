/* Copyright (C) 1985 by Manx Software Systems, Inc. */
#include	<sysfunc.h>
#include	<errno.h>

rename(o, n)
register char *o, *n;
{
	register int err;
	char old[128], new[128];

	if (_fixnam(n, new) || _fixnam(o, old))
		return(-1);
	ctop(old);
	ctop(new);
	_sys_parm[0] = 2;
	*(char **)(_sys_parm + 1) = old;
	*(char **)(_sys_parm + 3) = new;
	err = _system(SYS_RENAM);
	if (err) {
		errno = err;
		return(-1);
	}
	return(0);
}

