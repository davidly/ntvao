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

