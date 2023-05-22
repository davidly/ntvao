/* Copyright (C) 1985 by Manx Software Systems, Inc. */

#include	<sgtty.h>
#include	<device.h>

#define INITLEN 64 /* 64 bytes for init-string space */

struct _dev_info _devinfo = {
	"DEV_INFO04/86",
	CON_HIGH, CRMOD | ECHO | ECHOE | XTABS | 4, 0x08, 0x18,
	"CON:", 0x40,
	"PR:", 1,
	"SER:", 2,
	0, 0, 0, 0xc0, 0x00, 0, SLOT_LFCR|SLOT_TABS|SLOT_HIGH, 0, 0, INIT_VEC, 0,
	0, 0, 0, 0xc1, 0x10, 0, SLOT_TABS, 0, 0, INIT_VEC|INIT_CAL, 0,
	0, 0, 0, 0xc2, 0x20, 0, SLOT_TABS, 0, 0, INIT_VEC, 0,
	0, 0, 0, 0xc3, 0x30, 0, SLOT_TABS, 0, 0, INIT_VEC|INIT_CAL, 0,
	0, 0, 0, 0xc4, 0x40, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0xc5, 0x50, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0xc6, 0x60, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0xc7, 0x70, 0, 0, 0, 0, 0, 0,
	INITLEN,
	0
};

static char _devinfobuf[INITLEN]={0};

