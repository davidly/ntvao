x_amiga.c
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

x_apple.c
/* Copyright (C) 1984,85 by Manx Software Systems, Inc. */

struct bauds {
	short baud;
	short val;
} Bauds[] = {
   50,	1,
   75,	2,
  110,	3,
  150,	5,
  300,	6,
  600,	7,
 1200,	8,
 1800,	9,
 2400,	10,
 3600,	11,
 4800,	12,
 7200,	13,
 9600,	14,
19200,	15,
00000, 0x000 };

cinit(baud)
int baud;
{
	register int i;

	i = -1;
	while (Bauds[++i].baud)
		if (Bauds[i].baud == baud)
			goto found;
	return(-1);

found:
	_cinit(Bauds[i].val);
	return(0);
}

#asm
;
	instxt	<zpage.h>
*
DATREG	EQU	$C088
STAREG	EQU	$C089
CMDREG	EQU	$C08A
CTLREG	EQU	$C08B
*
SLOT	EQU	$20		;slot 2
*
	cseg
*
*	cflush();
*
	public	cflush_
cflush_	rts

*
*	cinit(baud);
*
	public	_cinit_
_cinit_
	ldy	#2
	ldx	#SLOT
	lda	#$0b		;transmit, recieve enabled
	sta	CMDREG,X
	lda	(SP),Y		;get baud rate
	ora	#$10		;1 stop, 8 data, internal clock *jd 6 Mar 86 (use 1 stop)
	sta	CTLREG,X
	lda	DATREG,X	;get character
	rts
*
*		cget();
*
	public	cget_
cget_	
	ldy	#$0
	ldx	#SLOT
	lda	#$40
	sta	CNT
	sta	CNT+1
cget1	lda	CNT
	bne	cget2
	dec	CNT+1
	bpl	cget2
	lda	#$ff
	tay
	bne	cget3
cget2	lda	$c000
	cmp	#$83
	bne	cget2a
	lda	$c010
	func	abort_
	jmp	abort_
cget2a	dec	CNT
	lda	STAREG,X
	and	#$08
	beq	cget1
	lda	DATREG,X
cget3	sta	R0
	sty	R0+1
	rts
*
	public	clrchan_
clrchan_
	ldy	#0
	sty	R0+1
	ldx	#SLOT
	lda	STAREG,X
	and	#$08
	beq	clrch1
	lda	DATREG,X
clrch1	sta	R0
	rts
*
CNT	fdb	0
*
*	cput(char);
*
	public	cput_
cput_	ldy	#2
	lda	(SP),Y
	sta	R0
	ldy	#0
	sty	R0+1
	ldx	#SLOT
cput1	lda	STAREG,X
	and	#$10
	beq	cput1
	lda	R0
	sta	DATREG,X
	rts
*
#endasm
x_godbou.a86
;:ts=8
;Copyright (C) 1984 by Manx Software Systems, Inc.
;

codeseg segment para public 'code'
dataseg segment para public 'data'
dataseg ends
	assume cs:codeseg,ds:dataseg,es:dataseg,ss:dataseg
	extrn $cswt:near
	extrn $begin:near
dataseg segment para public 'data'
	public	Bauds_
Bauds_ equ this word
	dw 50,0
	dw 75,1
	dw 110,2
	dw 150,4
	dw 300,5
	dw 600,6
	dw 1200,7
	dw 1800,8
	dw 2000,9
	dw 2400,10
	dw 3600,11
	dw 4800,12
	dw 7200,13
	dw 9600,14
	dw 19200,15
	dw 0,0
dataseg ends
;
;	Interfacer 3/4 Routines.
;

I3_DATA		equ	10h		;data register
I3_STATUS	equ	11h		;status register
I3_MODE		equ	12h		;mode register
I3_CMD		equ	13h		;command register
I3_USER		equ	17h		;user select register

AUX		equ	5

	extrn	quit_:near
	public cinit_
cinit_ proc	near
	mov bx,sp
	push	si
	mov	si, offset Bauds_
$7:
	cmp	 word ptr  [si],0
	beq 	$6
	mov	ax, word ptr  [si]
	cmp	ax,word ptr 2[bx]
	beq 	$6
$5:
	add	si,4
	jmp $7
$6:
	mov	ax,word ptr 2[si]	;get baud value
	pop si
	mov	ah,al
	mov	al,AUX			;init the console
	out	I3_USER,al		;set the user number
	mov	al,0ceh			;async, 16x, 8 bits, no parity, 2 stops
	out	I3_MODE,al		;set up mode register 1
	mov	al,ah			;baud rate
	or	al,070h			;internal clocks (was 30)
	out	I3_MODE,al		;set up mode register 2
	mov	al,027h			;transmitter/recvr on, dtr low
	out	I3_CMD,al		;set the command port
	ret

	public	cget_
cget_:
	mov	al,AUX			;console device
	out	I3_USER,al
cget0:
	mov	cx,07fffh
cget1:
	in	al,I3_STATUS		;no, get the status
	and	al,2			;check for receiver full
	jnz	cget3			;yes, get char
	loop	cget1			;try again
	push	es
	mov	ah,6			;console i/o call
	mov	dx,0ffh
	int	21h
	pop	es
	cmp	al,03
	jnz	cget2
	mov	ax,5
	push	ax
	call	quit_
cget2:
	mov	ax,-1			;return -1
	ret
cget3:
	in	al,I3_DATA		;yes, get the byte
	sub	ah,ah
	ret

	public	cput_
cput_:
	push	bx
	mov	bx,sp
	mov	ah,4[bx]
	mov	al,AUX			;console device
	out	I3_USER,al
cput1:
	in	al,I3_STATUS		;get the status
	test	al,1			;is the transmitter ready?
	jz	cput1			;not yet
	mov	al,ah
	out	I3_DATA,al		;output the data
	pop	bx
	ret

	public	cflush_
cflush_:
	ret

	public	clrchan_
clrchan_:
	mov	al,AUX			;console device
	out	I3_USER,al
	in	al,I3_STATUS		;get the status
	and	al,2			;check for receiver full
	jz	clrch0			;no, return
	in	al,I3_DATA		;yes, get the byte
clrch0:
	sub	ah,ah
	ret

cinit_	endp
CODESEG	ends
	end

x_ibmpc.a86
;:ts=8
;Copyright (C) 1984 by Manx Software Systems, Inc.
;

codeseg segment para public 'code'
dataseg segment para public 'data'
dataseg ends
	assume cs:codeseg,ds:dataseg,es:dataseg,ss:dataseg
	extrn $cswt:near
	extrn $begin:near
dataseg segment para public 'data'
	public	Bauds_
Bauds_ equ this word
	dw   50,0900h
	dw   75,0600h
	dw  110,0417h
	dw  150,0300h
	dw  300,0180h
	dw  600,00c0h
	dw 1200,0060h
	dw 1800,0040h
	dw 2000,003ah
	dw 2400,0030h
	dw 3600,0020h
	dw 4800,0018h
	dw 7200,0010h
	dw 9600,000ch
	dw    0,   0
dataseg ends

TXBUF	equ	03f8h
RXBUF	equ	03f8h
DIVLO	equ	03f8h
DIVHI	equ	03f9h
INTEN	equ	03f9h
INTID	equ	03fah
LINCT	equ	03fbh
MDMCT	equ	03fch
LINST	equ	03fdh
MDMST	equ	03feh

DR	equ	001h
THRE	equ	020h

	extrn	quit_:near
	public cinit_
cinit_ proc	near
	mov	bx,sp
	push	si
	mov	si, offset Bauds_
$7:
	cmp	 word ptr  [si],0
	beq 	$6
	mov	ax, word ptr  [si]
	cmp	ax,word ptr 2[bx]
	beq 	$6
$5:
	add	si,4
	jmp	$7
$6:
	mov	bx,si			;copy pointer
	pop	si
	mov	al,087h			;8 bits, no parity, 2 stop
	mov	dx,LINCT
	out	dx,al
	mov	al,2[bx]		;baud rate low byte
	mov	dx,DIVLO
	out	dx,al
	mov	al,3[bx]		;baud rate high byte
	mov	dx,DIVHI
	out	dx,al
	mov	al,03h			;now talk to other registers
	mov	dx,LINCT
	out	dx,al
	mov	al,0			;disable all interrupts
	mov	dx,INTEN
	out	dx,al
	ret

	public	cget_
cget_:
	push	dx
	mov	cx,02fffh
	mov	dx,LINST
cget1:
	in	al,dx			;get the status
	and	al,DR			;check for receiver full
	jnz	cget3			;yes, get char
	loop	cget1			;try again
	push	es
	mov	ah,6			;console i/o call
	mov	dx,0ffh
	int	21h
	pop	es
	cmp	al,03
	jnz	cget2
	mov	ax,5
	push	ax
	call	quit_
cget2:
	mov	ax,-1			;return -1
	pop	dx
	ret
cget3:
	mov	dx,RXBUF
	in	al,dx			;yes, get the byte
	sub	ah,ah
	pop	dx
	ret

	public	cput_
cput_:
	push	bx
	mov	bx,sp
	push	dx
	mov	dx,LINST
cput1:
	in	al,dx			;get the status
	test	al,THRE			;is the transmitter ready?
	jz	cput1			;not yet
	mov	al,4[bx]
	mov	dx,TXBUF
	out	dx,al		;output the data
	pop	dx
	pop	bx
	ret

	public	cflush_
cflush_:
	ret

	public	clrchan_
clrchan_:
	push	dx
	mov	dx,LINST
	in	al,dx			;get the status
	and	al,DR			;check for receiver full
	jz	clrch1			;yes, get char
	mov	dx,RXBUF
	in	al,dx			;yes, get the byte
clrch1:
	sub	ah,ah
	pop	dx
	ret

cinit_	endp
codeseg	ends
	end

x_mac.c
/*	Copyright (C) 1985 by Manx Software Systems, Inc.		*/

#include	<sgtty.h>
#include	<serial.h>

/*
 *	This routine is designed for use on an Apple Macintosh port A.
 */

struct bauds {
	short baud;
	short val;
} Bauds[] = {
  300, baud300,
  600, baud600,
 1200, baud1200,
 1800, baud1800,
 2400, baud2400,
 3600, baud3600,
 4800, baud4800,
 7200, baud7200,
 9600, baud9600,
19200, baud19200,
00000, 0x000 };

short Refout, Refin;
struct sgttyb Tty_state, Term;

cinit(baud)
int baud;
{
	register int i;
	static char inbuf[1024];

	i = -1;
	while (Bauds[++i].baud)
		if (Bauds[i].baud == baud)
			break;
	OpenDriver("\P.ain", &Refin);
	OpenDriver("\P.aout", &Refout);
	SerReset(Refin, Bauds[i].val | data8 | stop20 | noParity);
	SerReset(Refout, Bauds[i].val | data8 | stop20 | noParity);
	SerSetBuf(Refin, inbuf, 1024);
	ioctl(0, TIOCGETP, &Tty_state);
	Term = Tty_state;
	Term.sg_flags &= ~ECHO;
	Term.sg_flags |= CBREAK | NODELAY;
	ioctl(0, TIOCSETP, &Term);
}

cflush()
{
	SerSetBuf(Refin, 0L, 0);
	ioctl(0, TIOCSETP, &Tty_state);
}

cput(c)
char c;
{
	long i = 1;

	FSWrite(Refout, &i, &c);
}

cget()
{
	register int j;
	char c;
	long l;

	for (j=0;j<100;j++) {
		SerGetBuf(Refin, &l);
		if (l > 0) {
			l = 1;
			FSRead(Refin, &l, &c);
			return(c&0xff);
		}
		if (read(0, &c, 1) == 1) {
			if (c == 3)
				abort();
		}
	}
	return(-1);
}

clrchan()
{
	char c = 0;
	long l;

	for (;;) {
		SerGetBuf(Refin, &l);
		if (l > 0) {
			l = 1;
			FSRead(Refin, &l, &c);
		}
		else
			return(c&0xff);
	}
}

xfer.c
/*	Copyright (C) 1984,1985 by Manx Software Systems, Inc.		*/

#include	<stdio.h>
#include	<fcntl.h>

#define	VERSION	0x17

#define	START	0x01
#define	SENDDAT	0x02
#define	RECVDAT	0x03
#define	ALLDONE	0x04

#define	SOH		0x01
#define	ACK		0x06
#define	NACK	0x15
#define	CANCEL	0x18

unsigned short Addr;
unsigned Crchigh, Crclow;
int Master, Delay_time;
int Sendflg, Typeflg;
int Abort;

#ifndef BAUD
#define	BAUD	9600
#endif

FILE *openfile();

#define	ASCII		0x01
#define	RESOURCE	0x02
#define	PRO_BIN		0x04
#define	PRO_PRG		0x08

main(argc, argv)
int argc;
char **argv;
{
	char *hname = 0, *tname = 0;
	char *cp, buf1[40], buf2[40];

	if (argc <= 1) {
		slave();
		exit();
	}
	print("\nFILE TRANSFER UTILITY V1.7a - MASTER\n");
	print("(C) 1985 Manx Software Systems, Inc.\n");
	print("%d Baud\n", BAUD);
	Master = 1;
	cinit(BAUD);
#ifdef DELAY
	Delay_time = DELAY;
#else
	Delay_time = 0;
#endif

	Sendflg = 1;
	Typeflg = 0;

	while (--argc) {
		cp = *++argv;
		if (*cp == '-') {
			while (*++cp) {
				switch(tolower(*cp)) {
				case 'r':
					Sendflg = 0;
					break;
				default:
					fatal("Unknown option %c!\n", *cp);
				}
			}
		}
		else if (*cp == '+') {
			while (*++cp) {
				switch(tolower(*cp)) {
				case 'a':
					Typeflg |= ASCII;
					break;
				case 'b':
					Typeflg |= PRO_BIN;
					break;
				case 'p':
					Typeflg |= PRO_PRG;
					break;
				case 'r':
					Typeflg |= RESOURCE;
					break;
				default:
					fatal("Unknown option %c!\n", *cp);
				}
			}
		}
		else {
			if (hname && tname) {
				printf("Too many file names specified.\n");
				usage();
			}
			if (hname == 0)
				hname = cp;
			else
				tname = cp;
		}
	}
	if (hname == 0)
		usage();
	if (tname == 0)
		tname = hname;
	if (Sendflg)
		sendfile(hname, tname);
	else
		recvfile(tname, hname);
	cflush();
}

usage()
{
	fatal("usage: xfer [-r] [+abpr] source [destination]\n");
}

fatal(msg, a1, a2, a3)
char *msg;
{
	print(msg, a1, a2, a3);
	quit();
}

slave()
{
	int typ, id, len;
	char buf[50];
#ifdef C64
	long b;

	b = BAUD;
	if (*((char *)0x2a6))
		b = 985250L / b;
	else
		b = 1022730L / b;
	cinit((int)b);
#else
	cinit(BAUD);
#endif

	print("\nFILE TRANSFER UTILITY V1.7a - SLAVE\n");
	print("(C) 1985 Manx Software Systems, Inc.\n");
	print("%d Baud\n", BAUD);
	Master = Delay_time = 0;
	for (;;) {
		Abort = 0;
		print("\nWaiting ...  ( Control-C to exit )");
		precv(buf, &typ, &id, &len);
		if (typ == START) {
			if (buf[0] != VERSION)
				fatal("Incompatible versions of XFER!\n");
			Sendflg = buf[1];
			Typeflg = buf[2] | (buf[3] << 8);
			if (Sendflg)
				recvfile(buf+4);
			else
				sendfile(buf+4);
		}
	}
}

char mbuf[5*1024];

sendfile(hname, tname)
char *hname, *tname;
{
	FILE *fp;
	register char *cp;
	register int i, len, l, id;
	int retry = 0;

	id = 1;
	fp = openfile(hname, 0);
	if (fp == NULL) {
		print("\nFailure opening <%s>.\n", hname);
		if (Master == 0)
			abort();
		return(-1);
	}
	if (Master) {
		cp = mbuf;
		*cp++ = VERSION;
		*cp++ = Sendflg;
		*cp++ = Typeflg;
		*cp++ = Typeflg >> 8;
		while (*cp++ = tolower(*tname++))
			;
		if ((i=psend(mbuf, START, 0, cp-mbuf)) == -1) {
			print("Unable to start communication.\n");
			abort();
			quit();
		}
		if (i == -2) {
			print("Slave aborted.\n");
			quit();
		}
	}
	print("\nSending <%s>.\n", hname);
	for (;;) {
		if (Typeflg & ASCII) {
			for (len=0;len<5*1024;len++) {
				if ((i = agetc(fp)) == EOF)
					break;
				mbuf[len] = i;
			}
		}
#ifdef C64
		else if (id == 1 && Master && (Typeflg & PRO_PRG) {
			len = fread(mbuf, 1, 4, fp);
			len += fread(mbuf+2, 1, 5*1024-2, fp) - 2;
		}
#endif
		else
			len = fread(mbuf, 1, 5*1024, fp);

		if (len == 0) {
			if ((i = psend(mbuf, ALLDONE, id, 0)) == -1) {
				fclose(fp);
				print("\nFailure sending EOT packet.\n");
				abort();
				return(-1);
			}
			if (i == -2) {
				fclose(fp);
				print("\nAbort by receiver.\n");
				return(-1);
			}
			retry += i;
			print(".");
			fclose(fp);
			print("\nSent %d packets, %d retries.\n", id, retry);
			return(0);
		}
		if (len < 0) {
			fclose(fp);
			print("\nFailure reading file.\n" );
			abort();
			return(-1);
		}
		cp = mbuf;
		while (len) {
			if (len > 128)
				l = 128;
			else
				l = len;
			if ((i = psend(cp, SENDDAT, id, l)) == -1) {
				fclose(fp);
				print("\nFailure sending data packet %d.\n", id);
				abort();
				return(-1);
			}
			if (i == -2) {
				fclose(fp);
				print("\nAbort by receiver.\n");
				return(-1);
			}
			retry += i;
			print(".");
			cp += l;
			len -= l;
			id++;
		}
	}
}

recvfile(hname, tname)
char *hname, *tname;
{
	FILE *fp;
	char *cp;
	int typ, id, len, l, packet;
	char time = 0;

	if (Master) {
		cp = mbuf;
		*cp++ = VERSION;
		*cp++ = Sendflg;
		*cp++ = Typeflg;
		*cp++ = Typeflg >> 8;
		while (*cp++ = tolower(*tname++))
			;
		if ((typ=psend(mbuf, START, 0, cp-mbuf)) == -1) {
			print("Unable to start communication.\n");
			abort();
			quit();
		}
		if (typ == -2) {
			print("Slave aborted.\n");
			quit();
		}
	}
	fp = openfile(hname, 1);
	if (fp == NULL) {
		print("\nFailure creating file <%s>.\n", hname);
		abort();
		return(-1);
	}
	print("\nReceiving <%s>.\n", hname);
	packet = typ = 0;
	while (typ != ALLDONE) {
		len = 0;
		while (len+128 <= 5*1024 && typ != ALLDONE) {
			precv(mbuf+len, &typ, &id, &l);
			if (++packet != id) {
				fclose(fp);
				print("\nBad packet id - wanted %d, got %d.\n", packet, id);
				abort();
				return(-1);
			}
			print(".");
			len += l;
		}
		cp = mbuf;
#if PRODOS
		if ((Typeflg & (PRO_BIN|PRO_PRG)) && time == 0) {
			time = 1;
			cp += 4;
			Addr = *(short *)mbuf;
			len -= 4;
		}
#endif
		if (Typeflg & ASCII) {
			while (cp<mbuf+len) {
				if (aputc(*cp++, fp) == EOF) {
					fclose(fp);
					print("\nFailure writing file.\n");
					abort();
					return(-1);
				}
			}
		}
		else if (fwrite(cp, 1, len, fp) != len) {
			fclose(fp);
			print("\nFailure writing file.\n");
			if (typ != ALLDONE)
				abort();
			return(-1);
		}
	}
	closefile(fp, hname);
	print("\nReceived %d packets.\n", id);
	return(0);
}

FILE *
openfile(name, flg)
char *name;
{
	FILE *fp, *fdopen();
	int i, fd;

	if (flg == 0)
		fp = fopen(name, "r");
	else
#ifdef DOS33
		{
		fd=open(name, O_RDWR | O_CREAT | O_TRUNC, (Typeflg&ASCII)? 0:4);
		fp = fdopen(fd, "w+");
		}
#else
		fp = fopen(name, "w+");
#endif
#ifdef MACINTOSH
	if (Typeflg & RESOURCE) {
		close(fp->_unit);
		if (flg == 0)
			i = openrf(name, O_RDONLY);
		else
			i = openrf(name, O_RDWR | O_CREAT | O_TRUNC);
		if (i < 0) {
			fclose(fp);
			fp = NULL;
		}
		else
			fp->_unit = i;
	}
#endif
	return(fp);
}

closefile(fp, name)
FILE *fp;
char *name;
{
	int i;
#ifdef PRODOS
	struct finfo {
		char			access;
		char			file_type;
		unsigned short	aux_type;
		char			storage_type;
		unsigned short	blocks_used;
		unsigned short	mod_date;
		unsigned short	mod_time;
		unsigned short	create_date;
		unsigned short	create_time;
	} finfo;
	extern char _sys_parm[];
#endif

	i = fclose(fp);
#ifdef PRODOS
	getfinfo(name, &finfo);
	if (Typeflg & ASCII) {
		finfo.file_type = 4;
		finfo.aux_type = 0;
	}
	else if (Typeflg & (PRO_BIN | PRO_PRG)) {
		if (Addr == 0x2000)
			finfo.file_type = 0xff;
		else if (Typeflg & PRO_PRG)
			finfo.file_type = 0xf8;
		else
			finfo.file_type = 0x06;
		finfo.aux_type = (unsigned short) Addr;
	}
	else
		finfo.file_type = 0;
	setfinfo(name, &finfo);
#endif
#ifdef MACINTOSH
	if (Typeflg & ASCII)
		settyp(name, 'TEXT', 'Manx');
	if (Typeflg & PRO_PRG)
		settyp(name, 'AZTC', 'Manx');
#endif
	return(i);
}

/*
 *	packet send and receive protocol
 */

psend(msg, type, id, len)
char *msg;
int len, type, id;
{
	int retry, c, i, cnt;
	char *cp;
	char buf[134];

	cp = buf;
	*cp++ = type;
	*cp++ = id;
	*cp++ = id >> 8;
	*cp++ = len;
	for (c=0;c<128;c++)
		*cp++ = msg[c];
	Crclow = Crchigh = 0;
	for (c=0;c<132;c++) {
		acumcrc(buf[c]);
#ifdef SDEBUG
		debug("%02x ", buf[c]&0xff);
#endif
	}
#ifdef SDEBUG
	debug("\n%hi, lo=%02x %02x\n", Crchigh&0xff, Crclow&0xff);
#endif
	*cp++ = Crchigh;
	*cp++ = Crclow;
	setvec();
	for (retry=0;retry<10;retry++) {
		cnt = 134;
		cp = buf;
#ifdef SDEBUG
		debug("retry - %d\n", retry);
#endif
		if (clrchan() != SOH)
			while (cancel(1) != SOH)
				;
#ifdef SDEBUG
		debug("got SOH\n");
#endif
		dput(0);
		dput(0);
		dput(0);
		dput(0);
		dput(0);
		clrchan();
		while (cnt--)
			dput(*cp++);
		if ((c=cancel(100)) == ACK) {
			clrvec();
			return(retry);
		}
#ifdef SDEBUG
		debug("reply is %x\n", c);
#endif
	}
	clrvec();
	return(-1);
}

precv(msg, type, id, len)
char *msg;
int *len, *type, *id;
{
	int l, c, cnt;
	unsigned char *cp;
	unsigned char buf[134];

	setvec();
	clrchan();
	for (;;) {
		cnt = 133;
		cp = buf;
		do {
#ifdef RDEBUG
			debug("SOH\n");
#endif
			dput(SOH);
		} while ((c = cancel(1)) == -1);
		while ((c = cget()) == 0)
			;
#ifdef RDEBUG
		debug("receiving packet\n");
#endif
		*cp++ = c;
		while (cnt--) {
			*cp++ = cget();
		}
		Crclow = Crchigh = 0;
		for (l=0;l<134;l++) {
			acumcrc(buf[l]);
#ifdef RDEBUG
			debug("%02x ", buf[l]&0xff);
#endif
		}
#ifdef RDEBUG
		debug("\n");
#endif
		if (Crclow || Crchigh) {
#ifdef RDEBUG
			debug("lo=%x hi=%x\n", Crclow, Crchigh);
#endif
			dput(NACK);
			continue;
		}
#ifdef RDEBUG
		debug("packet okay\n");
#endif
		cp = buf;
		*type = *cp++ & 0xff;
		*id = *cp++ & 0xff;
		*id |= *cp++ << 8;
		*len = *cp++ & 0xff;
		for (l=0;l<*len;l++)
			msg[l] = *cp++;
		dput(ACK);
		clrvec();
		return;
	}
}

cancel(cnt)
{
	register int c;

	while (cnt--) {
		if ((c = cget()) == CANCEL)
			abort();
		if (c != -1)
			break;
	}
	return(c);
}

abort()
{
	setvec();
	dput(CANCEL);
	clrvec();
	printf("ABORTING\n");
	quit();
}

dput(c)
int c;
{
	int i;

	for (i=0;i<Delay_time;i++)
		;
	cput(c);
}

quit()
{
	cflush();
	putchar('\n');
	exit(1);
}

pchar(c)
char c;
{
#ifdef UPPER
	if (c >= 'a' && c <= 'z')
		c = c - 'a' + 'A';
#endif
	write(1, &c, 1);
}

print(msg, a1, a2, a3)
char *msg;
{
	format(pchar, msg, &a1);
}

#ifndef C64
setvec()
{
}
clrvec()
{
}
#endif

debug(str, a1, a2, a3, a4, a5)
char *str;
{
	clrvec();
	print(str, a1, a2, a3, a4, a5);
	setvec();
}

#ifndef MPU6502
/*	CRC16	*/

acumcrc(v)
{
	register unsigned a,p,x;

	x = a = (Crchigh ^ v) & 0xff;
	for (p = 0 ; x ; ) {
		p ^= x&1;
		x >>= 1;
	}
	if (p) {
		Crchigh = ((a&1)?0x80:0) ^ ((a<<6)&0xc0) ^ 0x01 ^ Crclow;
		a >>= 1;
		Crclow = (0x80>>1) ^ (a>>1) ^ a ^ 0x80;
	} else {
		Crchigh = ((a&1)?0x80:0) ^ ((a<<6)&0xc0) ^ Crclow;
		a >>= 1;
		Crclow = (a>>1) ^ a;
	}
}
#else
#asm
	instxt	<zpage.h>

AL	EQU	12
B	EQU	13
C	EQU	14
H	EQU	15
L	EQU	16

	global	Crclow_,2
	global	Crchigh_,2
*
	public acumcrc_
acumcrc_
	ldy	#2
	lda	(SP),Y
	sta	AL
	ldy	#0
	sty	B
	sty	C
	lda	AL
	eor	Crchigh_
	sta	AL
X0	asl	A
	bcc	X1
	iny
X1	tax
	bne	X0
	tya
	and	#1
	beq	X2
	sta	C
	lda	#$80
	sta	B
X2	lsr	AL
	txa
	ror	A
	ora	AL
	sta	L
	lsr	A
	stx	AL
	ror	AL
	ora	AL
	and	#$C0
	eor	C
	sta	H
	lda	L
	and	#$80
	eor	H
	eor	Crclow_
	sta	Crchigh_
	lda	L
	and	#$7F
	sta	L
	lsr	A
	eor	L
	eor	B
	sta	L
	lda	B
	lsr	A
	eor	L
	sta	Crclow_
	rts
#endasm
#endif

makefile
.SUFFIXES: .c .a86 .r68 .r86 .r65 
CFLAGS=
C68FLAGS=
C86FLAGS=

.c.r68:
	c68 $(C68FLAGS) -o $*.r68 $*.c

.c.r65:
	c65 -DPRODOS -o $*.r65 $*.c

.c.r86:
	cc86 $(C86FLAGS) -o $*.r86 $*.c

.a86.r86:
	as86 -o $*.r86 $*.a86

godbout:	xfer.r86 x_godbou.r86
	ln86 -o godbout.exe xfer.r86 x_godbou.r86 -lc

amiga:	xfer.r68 x_amiga.r68
	ln68 xfer.r68 x_amiga.r68 -lc
	ren xfer xfer.amiga

apple:	xfer.r65 x_apple.r65
	ln65 xfer.r65 x_apple.r65 -lc
	strip65 <xfer >xfer.apl

apple33: xfer33.r65 x_apple.r65
	ln65 -o xfer33 xfer33.r65 x_apple.r65 -ld
	strip65 <xfer33 >xfer33.apl

pc:	xfer.r86 x_ibmpc.r86
	ln86 xfer.r86 x_ibmpc.r86 -lc

xfer33.r65: xfer.c
	c65 -DDOS33 -o xfer33.r65 xfer.c
xfer.doc
NAME

 xfer - file transfer utility

SYNOPSIS

 xfer [-r] [+abpr] srcfile [destfile]

DESCRIPTION

 xfer transfers a file from one machine to another over a serial link.
To do so, xfer must be run on each of the two machines and a cable
connected between the two serial ports. One machine is considered to
be the master and the other the slave, with the master telling the slave
what to do. Whether a machine is the master or slave is defined by
specifying or not specifying arguments, respectively, when its xfer is
started.

 srcfile is the name of the file that is to be sent, and destfile is the
name of the file that is to be received. if destfile isn't specified, it's
assumed to be the same as srcfile.


1.  Options

 The arguments to xfer are as follows:

 +a Specifies that the file being transferred is an ASCII
    text file. When this option is used, line endings are
    converted as necessary. For example, if the sending
    machine uses a carriage return-line feed combination
    to terminate a line of text, while the receiving
    machine uses a newline character for this, this
    translation will be made when the file is transferred.

 +b When a file is being transferred to ProDOS, +b
    specifies that on ProDOS the file type should be set to
    BIN. For more information, see below.

 +r When a file is being transferred to or from the
    Macintosh, +r specifies that on the Macintosh the
    file's resource fork is to be accessed.

 +p Specifies that a program is being transferred.

 -r Specifies that the file is to go from the slave to the
    master. If this option isn't used, the file is sent from
    the master to the slave.

 When the +a, +b, and +p options are used, the file is assumed to
contain arbitrary binary data.


2.  Machine-dependent Information

2.1  Information for Macintosh

 * Data is sent through Macintosh port A.

 * ASCII text is sent to or from a Macintosh using the +a option.
   This option causes line endings to be converted as described
   above. When data is sent to the Macintosh, the +a option also
   causes the Macintosh file's type and creator to be set to TEXT
   and Manx.

 * A program is sent to or read from a Macintosh using the +p
   option. When a program is received by the Macintosh, the +p
   option causes the type and creator fields of the Macintosh file
   to which the program is written to be set to AZTC and Manx.
   When a program is sent from the Macintosh, the +p option
   has no effect on the Macintosh (but it might, of course, have
   an effect on the other computer).

 * The +r option causes the resource fork of the specified
   Macintosh file to be accessed. If this option isn't specified,
   the Macintosh file's data fork will be accessed.

 * When data is sent to or read from a Macintosh without using
   the +a +p options, the Macintosh file's type and creator are
   set to TEXT and ????.

 * For example, to send ASCII text to the data fork of a
   Macintosh file, you would use the option +a. To send a
   program to the resource fork of a Macintosh file, you would
   use the options +pr.

2.2  Information for Apple //

 There are two versions of xfer for use on an Apple //, one which
runs under ProDOS in the SHELL environment, and one which runs
under DOS 3.3. In both cases, the Apple // serial card must be in slot
2 and must be compatible with the Super Serial card.

 The following paragraphs first discuss the special features of the
ProDOS version and then of the DOS 3.3 version.

2.2.1  ProDOS Information

 * The +a option is used to send ASCII text. When data sent
   using the +a option is received by an Apple //, line endings
   are converted as decribed above, the type of the file to which
   the received data is written is set to TXT, and the file's AUX
   TYPE field is set to 0.

 * To send to ProDOS a program that can only be executed in
   the SHELL environment, the +p option is used. In this case,
   the data, less the first four bytes (which contain control
   information), is written to a file of type PRG. The program's
   load address, which is in the data's first two bytes, is set in the
   file's AUX TYPE field. (Note: if the program's load address
   is 0x2000, the file's type is set to SYS instead of PRG).

 * To send to ProDOS a program that can be executed either
   inside or outside the SHELL environment, the +b option is
   used. In this case, the data, less its first four bytes (which
   contain control information), is written to a file of type BIN.
   The program's load address, which is in the data's first two
   bytes, is set in the file's AUX TYPE field. (Note: if the
   program's load address is 0x2000, the file's type is set to SYS
   instead of BIN).

 * To send arbitrary binary data (ie, data other than ASCII text
   and programs) to ProDOS, don't specify the +a, +p, or +b
   options. In this case, the type of the file to which the data is
   written is set to 0.

2.2.2  DOS 3.3 Information

 * The DOS 3.3 version of xfer can only be used in 'slave' mode.

 * When a file is sent to Apple // DOS 3.3, the type of the file is
   set to TXT if the +a option was specified; otherwise, the type
   is set to BIN.

2.3  Information for Commodore 64

 * When data is sent to a Comodore 64 using the +p option, the
   second two bytes of data are not placed in the Commodore file.

2.4  Information for Amiga

 * Data is sent through the serial port on the Amiga.


3.  Examples

 For all of the following examples, the slave machine's xfer is
started by typing

 xfer

In fact, the absence of arguments when starting xfer defines the
machine on which this xfer is started as being the slave machine.

 The following command, when executed on the master machine
transfers the file datafile from the master machine to the slave
machine. The file is assumed to contain arbitrary binary data, and no
translations or manipulations are done to it.

 xfer datafile

 The next command, when executed on the master machine,
transfers the source file test.c from the master machine to the slave
machine. The name of the file that's created on the slave machine is
also test.c.

 xfer +a test.c

 The next command, when executed on the master machine,
transfers a program from the master to the slave. The name of the file
on the master is test. On the slave, the program is placed in the /bin
directory in a file called tprog.

 xfer +p test /bin/tprog 

 The next command, when executed on the master machine,
transfers text from the file named test.out on the slave to the file
named output on the master.

 xfer -r +a test.out output


4.  Baud rate

 The default baud rate is 9600, but can be changed by recompiling
with the macro BAUD defined to the appropriate value.


5.  Source

 The source to the XFER program is provided with the package for
adaptation to different hardware configurations. Included in the
source are drivers for the Macintosh, Apple // Super Serial Card,
Amiga, IBM PC Asynchronous Communications Adapter, and the
Interfacer 4 from Godbout.

 xfer consists of a system-independent module, whose source is in
xfer.c, and a machine-dependent module. To generate a version of
xfer for a particular machine, compile xfer.c and the machine-
dependent module and link the two together.


6.  Problems

 When xfer is used between two machines whose speeds greatly
differ, you may need to put a timing delay in the faster machine's
xfer. To do this, rebuild the faster machine's xfer; when you compile
its xfer.c, specify a value for the DELAY macro using the compiler's
-D option. Begin with a value of 50 for DELAY, and work up from
there. For example,

 cc -DDELAY=50 xfer.c
