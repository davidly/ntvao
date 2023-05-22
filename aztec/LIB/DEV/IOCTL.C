/* Copyright (C) 1985 by Manx Software Systems, Inc. */

#include	<prodos.h>
#include	<errno.h>
#include	<sgtty.h>
#include	<device.h>


ioctl(fd, cmd, ttp)
struct sgttyb *ttp;
{
	register int err;
	register struct _fil_buf *b;

	if (fd < 0 || fd >= MAXFILES || (b = _fil_tab+fd)->unit == 0 ||
												(b->unit&0xc0) != 0xc0) {
		errno = EBADF;
		return(-1);
	}
	if (cmd == TIOCGETP)
		*ttp = _dev_info->tty;
	else if (cmd == TIOCSETP || cmd == TIOCSETN)
		_dev_info->tty = *ttp;
	else
		return(-1);
	return(0);
}

