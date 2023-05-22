/* Copyright (C) 1985 by Manx Software Systems */
#include	<prodos.h>
#include	<sysfunc.h>
#include	<errno.h>

getprefix(buf)
char *buf;
{
	register int err;

	*_sys_parm = 1;
	*(char **)(_sys_parm + 1) = buf;
	if ((err = _system(SYS_GTPFX)) == 0)
		ptoc(buf);
	if (err) {
		errno = err;
		return(-1);
	}
	return(0);
}

