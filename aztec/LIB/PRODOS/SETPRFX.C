/* Copyright (C) 1985 by Manx Software Systems */
#include	<prodos.h>
#include	<sysfunc.h>
#include	<errno.h>

setprefix(buf)
register char *buf;
{
	register int err;

	_sys_parm[0] = 1;
	ctop(buf);
	*(char **)(_sys_parm + 1) = buf;
	err = _system(SYS_STPFX);
	ptoc(buf);
	if (err) {
		errno = err;
		return(-1);
	}
	return(0);
}

