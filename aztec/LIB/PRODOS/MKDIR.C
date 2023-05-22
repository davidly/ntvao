/* Copyright (C) 1985 by Manx Software Systems */
#include	<prodos.h>
#include	<errno.h>
#include	<sysfunc.h>

mkdir(uname)
char *uname;
{
	register int err;
	char name[64];
	

	strcpy(name, uname);
	ctop(name);
	*_sys_parm = 7;
	*(char **)(_sys_parm + 1) = name;
	*(_sys_parm + 3) = 0xc1;	/* enable everything but write */
	*(_sys_parm + 4) = 0x0f;	/* type directory file */
	*(int *)(_sys_parm + 5) = 0x0100;
	*(_sys_parm + 7) = 0x0d;		/* directory, not file */
	*(long *)(_sys_parm + 8) = 0;
	if (err = _system(SYS_CREAT)) {
		errno = err;
		return(-1);
	}
	return(0);
}

