/* Copyright (C) 1984 by Manx Software Systems */
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

write (fd, buf, cnt)
register int fd, cnt;
register char *buf;
{
	register int err;
	register struct _fil_buf *b;

	if (fd < 0 || fd >= MAXFILES || (b = _fil_tab+fd)->unit == 0 || 
													b->flags == O_RDONLY) {
		errno = EBADF;
		return(-1);
	}
	if (cnt == 0)
		return(0);
	if (b->unit & 0x80) {
		if (b->unit & 0x40)
			return(_kb_put(buf, cnt));
		return(_dev_write(&_dev_info->slots[b->unit&7], buf, cnt));
	}
	io_buf->param_cnt = 4;
	io_buf->ref_num = b->unit;
	io_buf->data_buf = buf;
	io_buf->count = cnt;
	io_buf->trans_cnt = 0;
	if (err=_system(SYS_WRITE)) {
		errno = err;
		return(-1);
	}
	return(io_buf->trans_cnt);
}

_dev_write(s, buf, cnt)
char *buf;
{
	int i = cnt;

	while (cnt--)
		_slot_write(s, *buf++);
	return(i);
}

