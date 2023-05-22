/* Copyright (C) 1985 by Manx Software Systems */
#include	<errno.h>
#include	<prodos.h>
#include	<sysfunc.h>

struct seek_parms {
	char	param_cnt,
			ref_num;
	long	pos;				/* only lower three bytes are significant */
} ;
#define seek  ((struct seek_parms *)_sys_parm)

long
lseek (fd, pos, how)
long pos;
{
	register int err;

	if (fd < 0 || fd > MAXFILES || (seek->ref_num=_fil_tab[fd].unit) == 0) {
		err = EBADF;
xerr:
		errno = err;
		return(-1);
	}
	seek->param_cnt = 2;
	switch (how)  {
	case 0:
		seek->pos = pos;
		break;
	case 1:
		if (err = _system(SYS_GTMRK))
			goto xerr;
		seek->pos += pos;
		break;
	case 2:
		if (err=_system(SYS_GTEOF))
			goto xerr;
		seek->pos += pos;
		break;
	}

	if (err=_system(SYS_STMRK))
		goto xerr;
	_sys_parm[5] = 0;
	return(seek->pos);
}

