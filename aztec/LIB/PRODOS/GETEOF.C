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
get_eof(fd)
register int fd;
{
	register int err;

	if (fd < 0 || fd > MAXFILES || (seek->ref_num=_fil_tab[fd].unit) == 0) {
		errno = EBADF;
		return(-1);
	}
	seek->param_cnt = 2;
	if (err=_system(SYS_GTEOF)) {
		errno = err;
		return(-1);
	}
	_sys_parm[5] = 0;
	return(seek->pos);
}

