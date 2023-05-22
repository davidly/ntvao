/*	Copyright (C) 1985 by Manx Software Systems, Inc. */

#include	<prodos.h>

struct _fil_buf _filtab[MAXFILES] = {
	0xc0, RDWRT, 0,		/* stdin	*/
	0xc0, RDWRT, 0,		/* stdout	*/
	0xc0, RDWRT, 0		/* stderr	*/
};

