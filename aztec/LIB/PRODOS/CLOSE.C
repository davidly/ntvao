/* Copyright (C) 1985 by Manx Software Systems */

#include	<prodos.h>
#include	<errno.h>
#include	<sysfunc.h>

close (fd)
{
	register struct _fil_buf *fb;
	register int err;

	if (fd >= MAXFILES || (fb = _fil_tab+fd)->unit == 0) {
		errno = EBADF;
		return(-1);
	}
	_sys_parm[0] = 1;
	_sys_parm[1] = err = fb->unit;
	fb->unit = 0;
	if ((err&0x80) == 0) {
		if (err=_system(SYS_CLOSE)) {
			errno = err;
			return(-1);
		}
	}
	return(0);
}

