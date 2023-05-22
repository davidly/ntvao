/* Copyright (C) 1985 by Manx Software Systems */
#include	<prodos.h>
#include	<fcntl.h>
#include	<errno.h>
#include	<sysfunc.h>
#include	<sgtty.h>
#include	<device.h>

struct rd_info {
	char	param_cnt,
			ref_num;
	char	*data_buf;
	int		count,
			trans_cnt;
} ;
#define io_buf  ((struct rd_info *)_sys_parm)

read (fd, buf, cnt)
register int fd, cnt;
register char *buf;
{
	register int err;
	register struct _fil_buf *b;

	if (fd < 0 || fd >= MAXFILES || (b = _fil_tab+fd)->unit == 0 || 
														b->flags == O_WRONLY) {
		errno = EBADF;
		return(-1);
	}
	if ((b->unit)&0x80) {
		if (b->unit&0x40)
			return(_kb_get(buf, cnt));
		return(_dev_read(&_dev_info->slots[b->unit&7], buf, cnt));
	}
	io_buf->param_cnt = 4;
	io_buf->ref_num = b->unit;
	io_buf->data_buf = buf;
	io_buf->count = cnt;
	if ((err = _system(SYS_READ)) && err != 0x4c) {
		errno = err;
		return(-1);
	}
	return(io_buf->trans_cnt);
}

_dev_read(s, buf, cnt)
struct _slot_dev *s;
unsigned char *buf;
{
	int i;

	for (i=0;i<cnt;i++)
		if ((*buf++ = _slot_read(s)) < 0)
			break;
	return(i);
}

