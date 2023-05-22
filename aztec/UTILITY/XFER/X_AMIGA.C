#include	<exec/types.h>
#include	<devices/serial.h>

static struct IOExtSer s, p;

cinit(baud)
{
	unsigned long OpenDevice();
	struct MsgPort *CreatePort();
	register char *cp;
	extern int Enable_Abort;

	Enable_Abort = 0;
	if (OpenDevice(SERIALNAME, 0L, &s, 0L)) {
		printf("SERIAL device not available\n");
		exit(1);
	}
	s.IOSer.io_Message.mn_ReplyPort = CreatePort("xfer", 0);
	s.IOSer.io_Command = SDCMD_SETPARAMS;
	s.io_Baud = baud;
	s.io_SerFlags = SERF_XDISABLED;
	s.io_ReadLen = 8;
	s.io_WriteLen = 8;
	s.io_StopBits = 2;
	DoIO(&s);
	p = s;

/*
 *	The following kludge is here to keep the Xon from being sent all the
 *	time when we switch from recv to send. We are zapping the internal
 *	copy of the Xon and Xoff characters.
 */

	cp = s.IOSer.io_Device;
	cp += 0x22;
	*cp++ = 0;
	*cp++ = 0;
}

cflush()
{
	s.IOSer.io_Command = CMD_FLUSH;
	DoIO(&s);
	CloseDevice(&s);
	DeletePort(s.IOSer.io_Message.mn_ReplyPort);
}

cput(c)
char c;
{
	p.IOSer.io_Command = CMD_WRITE;
	p.IOSer.io_Data = (APTR) &c;
	p.IOSer.io_Length = 1;
	DoIO(&p);
}

cget()
{
	register int j, c;
	long Chk_Abort();

	for (j=0;j<200;j++) {
		if ((c = serget()) != -1)
			return(c);
		if (Chk_Abort())
			abort();
	}
	return(-1);
}

clrchan()
{
	register int c, oc = 0;

	while ((c = serget()) != -1)
		oc = c;
	return(oc);
}

static int flg;

static
serget()
{
	static unsigned char c;
	register int i;
	long CheckIO();

	if (flg == 0) {
		s.IOSer.io_Command = CMD_READ;
		s.IOSer.io_Data = (APTR) &c;
		s.IOSer.io_Length = 1;
		SendIO(&s);
		flg = 1;
	}
	if (CheckIO(&s) == FALSE)
		return(-1);
	WaitIO(&s);
	flg = 0;
	return(c);
}

