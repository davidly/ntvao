/*	Copyright (C) 1985 by Manx Software Systems, Inc. */

#include	<prodos.h>
#include	<sysfunc.h>

extern struct shvar {
	char			vects[6];
	int				retflg;
} *_Sp;

_exit(n)
{
	_sys_parm[0] = 1;
	_sys_parm[1] = 0;				/* close all files */
	_system(SYS_CLOSE);
	if (_Sp)						/* if called from SHELL */
		_Sp->retflg = n;
	(*((void (*)())0x03d0))();		/* ProDOS warm boot */
}

