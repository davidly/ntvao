config.c
/* note: 
 *    if you're compiling config for execution on UNIX, 
 *    use the sgtty.h, device.h, and config.h that come with the Aztec C package
 *    and use the fcntl.h that comes with UNIX
 */
#include "sgtty.h"
#include "device.h"
#include "config.h"
#include <fcntl.h>

#define SHELLENV (char *)0xbc00
#define NUMSLOTS 8

extern char tokbuf[80];

int dumpdev(), mod(), wrconfig(), readconfig(), opencmd();
extern int exit();

struct cmd {
	char *name;
	int (*func)();
	long arg;
} cmdlist[] = {
	"dump", dumpdev, 0,
	"mod", mod, 0,
	"read", readconfig, 0,
	"write", wrconfig, 0,
	"open", opencmd, 0,
	"quit", exit, 0,
	0,0,0,
};

unsigned int loc;
struct _dev_info ds;
char initbuf[80];
int fd;
long cseek();

#if MPU6502 | MPUINT
int memflag = 0;
#endif

main(argc, argv)
char **argv;
{
	struct cmd *cp;

	if (argc>=2){
#if MPU6502 | MPUINT
		if (strcmp(argv[1], "mem:")==0){
			memflag = 1;
			goto domem;
		}
#endif
		if ((fd=open(argv[1], O_RDWR)) < 0)
			printf("can't open file %s\n", argv[1]);
		else if (find())
			printf("can't find device info structure\n");
		else
domem:
			readconfig();
	}
	while (newline() != EOF){
		if (token() != WORD)
			continue;
		for (cp=cmdlist;cp->name;++cp)
			if (strcmp(tokbuf, cp->name) == 0)
				break;
		if (cp->name){
			(*cp->func)();
		} else
			printf("invalid cmd <%s>\n", tokbuf);
	}
}

opencmd()
{
	if (token() != WORD){
		printf("invalid argument\n");
		return;
	}
	if (fd)
		close(fd);
#if MPU6502 | MPUINT
	if (strcmp(tokbuf, "mem:")==0){
		memflag = 1;
		return;
	}
	memflag = 0;
#endif
	if ((fd=open(tokbuf, O_RDWR)) < 0)
		printf("can't open file %s\n", tokbuf);
	else if (find()){
		printf("can't find device info structure\n");
		close(fd);
		return;
	}
}

int dumpcon(), dumppr(), dumpser(), dumpinit();

struct cmd dumplist[]={
	"con:", dumpcon, 0,
	"pr:", dumppr, 0,
	"ser:", dumpser, 0,
	"init", dumpinit, 0,
	0,0,0,
};

#define ISBIT 0x08000000L
#define SLOTFLAGS 0x80000000L
#define SLOTIFLAGS 0x40000000L
#define SLOTTABW 0x20000000L
#define SLOTHEX 0x10000000L

int slotfield(), slotbit(), slotstring();

struct cmd slotlist[]={
	"flags", slotfield, SLOTHEX | SLOTFLAGS,
	"iflags", slotfield, SLOTHEX | SLOTIFLAGS,
	"tabw", slotfield, SLOTTABW,
	"lfcr", slotbit, ISBIT | SLOTFLAGS | SLOT_LFCR,
	"tabs", slotbit, ISBIT | SLOTFLAGS | SLOT_TABS,
	"uppr", slotbit, ISBIT | SLOTFLAGS | SLOT_UPPR,
	"crlf", slotbit, ISBIT | SLOTFLAGS | SLOT_CRLF,
	"high", slotbit, ISBIT | SLOTFLAGS | SLOT_HIGH,
	"vec", slotbit, ISBIT | SLOTIFLAGS | INIT_VEC,
	"cal", slotbit, ISBIT | SLOTIFLAGS | INIT_CAL,
	"once", slotbit, ISBIT | SLOTIFLAGS | INIT_ONCE,
	"str", slotstring, ISBIT | SLOTIFLAGS | INIT_STR,
	0,0,0,
};

int modconflags(), modconbit();

struct cmd conlist[]={
	"flags", modconflags, 0,
	"imap", modconbit, ISBIT | CON_IMAP,
	"uppr", modconbit, ISBIT | CON_UPPR,
	"high", modconbit, ISBIT | CON_HIGH,
	0,0,0,
};

dumpdev()
{
	register int i;
	int tokval;
	struct cmd *dp;

	if ((tokval=token()) == EOL){
		dumpcon();
		dumppr();
		dumpser();
		dumpinit();
		for(i=0;i<NUMSLOTS;++i)
			dumpslot(i);
	} else if (tokval == WORD){
		for (dp=dumplist;dp->name;++dp)
			if (strcmp(tokbuf, dp->name) == 0)
				break;
		if (dp->name)
			(*dp->func)();
		else if (tokbuf[0]=='s' && tokbuf[2]==':')
			dumpslot(tokbuf[1]-'0');
		else
			printf("invalid device\n");
	} else
		printf("invalid device\n");
}

dumpcon()
{
	printf("console slot: %d\n", 0x3f & ds.dev_con.dev_num);
	printf("  flags: %x", ds.con_flags);
	dumpnames( ds.con_flags, 0L, conlist);

}

dumppr()
{
	printf("printer slot: %d\n", ds.dev_pr.dev_num);
}

dumpser()
{
	printf("serial slot: %d\n", ds.dev_ser.dev_num);
}

dumpinit()
{
	printf("init space: max = %d  cur = %d\n", ds.init_max, ds.init_len);
}

dumpslot(i)
{
	register struct _slot_dev *sp = &ds.slots[i];
	register int c;
	register char *cp;
	char *dumpbits();

	printf("slot %d info:\n", i);
	printf("  slot #%x  hi_slot=%x  type = %d outvec: %x invec: %x\n",
		0xff & sp->slot, sp->hi_slot, sp->type, sp->outvec, sp->invec);
	printf("  flags: %s", dumpbits(sp->flags));
	dumpnames(sp->flags, SLOTFLAGS, slotlist);
	printf("  iflags: %s", dumpbits(sp->iflags));
	dumpnames(sp->iflags, SLOTIFLAGS, slotlist);
	printf("  tabw: %d tabp: %d\n", sp->tabw, sp->tabp);
	if (sp->iflags&INIT_STR){
		printf("  init string <");
		for (cp=ds.init_buf+sp->init;*cp;++cp)
			unxlat(*cp);
		printf(">\n");
	}
}

char *
dumpbits(i)
register int i;
{
	register int cnt=8;
	static char buf[9];
	for (cnt=8;cnt;--cnt){
		buf[8-cnt] = (i&0x80) ? '1' : '0';
		i <<= 1;
	}
	buf[8]=0;
	return buf;
}

dumpnames(f, mask, cmdptr)
register int f; long mask; struct cmd *cmdptr;
{
	register int x;
	register long flags;
	register struct cmd *cp;

	for (x=0x80;x;x>>=1)
		if (x&f)
			for (cp=cmdptr;cp->name;cp++)
				if (((flags=cp->arg)&ISBIT) && (mask==0 || (flags&mask))
					&& (flags&x))
						printf(" %s", cp->name);
	printf("\n");
}

unxlat(c)
register int c;
{
	if (isprint(c))
		putchar(c);
	else
		switch(c){
		case '\n':
			printf("\\n");
			break;
		case '\t':
			printf("\\t");
			break;
		case '\b':
			printf("\\b");
			break;
		case '\r':
			printf("\\r");
			break;
		case '\f':
			printf("\\f");
			break;
		default:
			printf("\\x%x%x",c>>4&15, c&15);
			break;
		}
}

fatal(s, args)
char *s; unsigned int args;
{
	register char *cp;

	for (cp="fatal error: ";*cp;++cp)
		puterr(*cp);
	format (puterr, s, &args);
	puterr('\n');
	exit(1);
}

int modcon(), moddev(), modinit();

#define DEVCON 1
#define DEVPR 2
#define DEVSER 4

struct cmd modlist[] = {
	"con:", modcon, 0,
	"pr:", moddev, DEVPR,
	"ser:", moddev, DEVSER,
	"init", modinit, 0,
	0,0,0,
};

mod()
{
	struct cmd *mp;

	if (token() != WORD){
		printf("invalid device name\n");
		return;
	}
	for (mp=modlist;mp->name;++mp)
		if (strcmp(tokbuf, mp->name) == 0)
			break;
	if (mp->name)
		(*mp->func)(mp->arg);
	else if (tokbuf[0]='s' && tokbuf[2]==':' && tokbuf[3]==0)
		modslot(tokbuf[1]-'0');
	else
		printf("invalid device\n");
}

moddev(dev)
long dev;
{
	register int slot;

	if (token() != WORD || tokbuf[0] != 's' || tokbuf[2] != ':'){
		printf("invalid device\n");
		return;
	}
	slot=tokbuf[1]-'0';
	if (dev&DEVPR)
		ds.dev_pr.dev_num = slot;
	else
		ds.dev_ser.dev_num = slot;
}

modinit()
{
	register int i;

	ds.init_len = 0;
	for (i=0;i<8;++i){
		ds.slots[i].init = 0;
		ds.slots[i].iflags &= ~INIT_STR;
	}
}

modcon()
{
	struct cmd *mp;
	int tokval, notflag;

	while ((tokval=token()) != EOL){
		if (tokval == NOT){
			notflag = 1;
			tokval=token();
		} else
			notflag = 0;
		if (tokval != WORD){
			printf("bad syntax\n");
			return;
		}
		if (tokbuf[0] == 's' && tokbuf[2] == ':'){
			ds.dev_con.dev_num = (tokbuf[1]-'0')| 0x40;
			continue;
		}
		for (mp=conlist;mp->name;++mp)
			if (strcmp(tokbuf, mp->name) == 0)
				break;
		if (mp->name)
			(*mp->func)(mp->arg, notflag);
		else
			printf("bad argument\n");
	}
}

modconflags()
{
	if(token() != EQUALS){
		printf("need equal sign here\n");
		return;
	}
	if (token() != WORD){
		printf("need hex value\n");
		return;
	}
	ds.con_flags = atoh(tokbuf);
}

modconbit(arg, notflag)
long arg;
{
	unsigned int val=arg;

	if(notflag)
		ds.con_flags &= ~val;
	else
		ds.con_flags |= val;
}


modslot(slotno)
{
	struct cmd *sp;
	int tokval, notflag;

	while((tokval=token()) != EOL){
		if (tokval == NOT){
			notflag=1;
			tokval=token();
		} else
			notflag=0;
		if (tokval != WORD){
			printf("bad syntax\n");
			return;
		}
		for (sp=slotlist;sp->name;++sp)
			if (strcmp(tokbuf, sp->name)==0)
				break;
		if (sp->name)
			(*sp->func)(slotno, sp->arg, notflag);
		else
			printf("invalid slot field\n");
	}
}

slotfield(slotno, arg)
long arg;
{
	int val;
	struct cmd *sp;

	if (token() != EQUALS){
		printf("bad syntax: need equals after field name\n");
		return;
	}
	if (token() != WORD){
		printf("bad syntax: need hex value after equals\n");
		return;
	}
	if (arg & SLOTHEX)
		val=atoh(tokbuf);
	else
		val=atoi(tokbuf);
	if (arg & SLOTFLAGS)
		ds.slots[slotno].flags = val;
	else if (arg & SLOTIFLAGS)
		ds.slots[slotno].iflags = val;
	else
		ds.slots[slotno].tabw = val;
}

slotbit(slotno, arg, notflag)
long arg;
{
	unsigned int val=arg;

	if (arg & SLOTFLAGS)
		if (notflag)
			ds.slots[slotno].flags &= ~val;
		else
			ds.slots[slotno].flags |= val;
	else
		if (notflag)
			ds.slots[slotno].iflags &= ~val;
		else
			ds.slots[slotno].iflags |= val;
}

slotstring(slotno, arg, notflag)
long arg;
{
	if (notflag){
		ds.slots[slotno].iflags &= ~INIT_STR;
		return;
	}
	if (token() != EQUALS){
		printf("need equals sign\n");
		return;
	}
	if (token() != WORD){
		printf("need string\n");
		return;
	}
	if (strlen(tokbuf)+1 >= ds.init_max-ds.init_len){
		printf("not enough init space for string\n");
		return;
	}
	ds.slots[slotno].init = ds.init_len;
	strcpy(ds.init_buf+ds.init_len, tokbuf);
	ds.init_len += strlen(tokbuf)+1;
	ds.slots[slotno].iflags |= INIT_STR;
}

cffile.c
/* note: 
 *    if you're compiling config for execution on UNIX, 
 *    use the sgtty.h and device.h that come with the Aztec C package
 */
#include "sgtty.h"
#include "device.h"

char id[]="DEV_INFO04/86";
extern int fd;
extern struct _dev_info ds;
extern char initbuf[];
extern unsigned int loc;
extern int memflag;
long cseek(), lseek();

#if MPU6502 | MPUINT
#define DEVINFO 0xbc73
extern int memflag;
#endif

find()
{
	register int c;
	register char *cp;
	unsigned tloc, holdloc;

#if MPU6502 | MPUINT
	if (memflag)
		return 0;
#endif
	cseek(fd, 0L, 0);
	for (tloc=0;;++tloc){
		if((c=cget(fd)) == -1)
			return -1;
		if (c != id[0])
			continue;
		/* found first char, now see if rest match: */ 
		for (cp=id+1;*cp; ++cp){
			if ((c=cget(fd)) == -1)
				return -1;
			if (*cp != c)
				break;
		}
		if (*cp){
			tloc += cp-id;
			continue;
		} else
			break;
	}
	printf("found table at %x\n", tloc);
	loc = tloc;
	return 0;
}

readconfig()
{
#if MPU6502 | MPUINT
	register struct _dev_info *sp=DEVINFO;
	if (memflag){
		movmem(sp, &ds, sizeof(struct _dev_info));
		movmem(sp->init_buf, initbuf, ds.init_max);
		return 0;
	}
#endif
	if (cseek(fd, (long) loc, 0) < 0){
		perror("seek error");
		return 1;
	}
	if (read(fd, &ds, sizeof(struct _dev_info)) != sizeof(struct _dev_info)){
		perror("read error");
		return 1;
	}
	if (ds.init_len && read(fd, initbuf, ds.init_max) != ds.init_max){
		perror("error reading init buf");
		return 1;
	}
	return 0;
}

wrconfig()
{
#if MPU6502 | MPUINT
	register struct _dev_info *sp=DEVINFO;
	if (memflag){
		movmem(&ds, sp, sizeof(struct _dev_info));
		movmem(initbuf, sp->init_buf, ds.init_max);
		return;
	}
#endif
	if (cseek(fd, (long) loc, 0)<0){
		perror("seek error");
		return;
	}
	if (write(fd, &ds, sizeof(struct _dev_info)) != sizeof(struct _dev_info))
		perror("write error");
	if (ds.init_max && write(fd, initbuf, ds.init_max) != ds.init_max)
		perror("error writing init space");
}

#define FBUFSIZE 1024
char *fbuf, *fpos, *fmax;

cget()
{
	register int size;
	char *malloc();

	if (fpos == 0 || fpos >= fmax){
		if (fbuf == 0 && (fbuf = malloc(FBUFSIZE)) == (char *)0){
				printf("can't allocate file buffer\n");
				return -1;
		}
		if ((size=read(fd, fbuf, FBUFSIZE)) <= 0)
			return -1;
		fmax = fbuf+size;
		fpos=fbuf;
	}
	return 255 & *fpos++;
}

long
cseek(fd, how, origin)
long how;
{
	fpos=0;
	return lseek(fd, how, origin);
}
cflex.c
#include "config.h"
char linebuf[80];
char *lineptr;
char tokbuf[80];

newline()
{
	printf("enter cmd: ");
	if (gets(linebuf) == 0)
		return EOF;
	lineptr=linebuf;
	return 0;
}

token()
{
		register int c;
		register char *tp;
		while ((c = *lineptr) && isspace(c))
			++lineptr;
		if (!c)
			return EOL;
		if (isalnum(c) || c == '/' || c == '.' /* for files */){
			for (tp=tokbuf; (c = *lineptr) && 
				(isalnum(c) || c == ':' /* for slot devices */
				|| c == '/' /* for files */ || c == '.' /* for files */);
			++tp, ++lineptr)
				*tp=tolower(c);
			*tp=0;
			return WORD;
		} else if (c == '='){
			++lineptr;
			return EQUALS;
		} else if (c == '~'){
			++lineptr;
			return NOT;
		} else if (c == '"'){
			for (tp=tokbuf, ++lineptr; (c= *lineptr) && c != '"';
				++lineptr, ++tp)
					*tp = xlat(c);
			*tp=0;
			if (c=='"')
				++lineptr;
			return WORD;
		} else {
			printf("invalid character at pos %d\n", lineptr-linebuf);
			return BADTOKEN;
		}
}

atoh(s)
char *s;
{
	int val=0, c;
	while(c=tolower(*s++)){
		if (c >= '0' && c <= '9')
			c -= '0';
		else if (c >= 'a' && c <= 'f')
			c =  c-'a'+10;
		else
			printf("bad hex digit");
		val = (val<<4)|c;
	}
	return val;
}

xlat(c)
register int c;
{
	register int cnt, val;

	if (c == '\\') {
		switch (c = *++lineptr) {
		case 'n':
			return '\n';
		case 't':
			return '\t';
		case 'b':
			return '\b';
		case 'r':
			return '\r';
		case 'f':
			return '\f';
		case 'x': case 'X':
			val = 0;
			for (cnt=0,++lineptr ; cnt<2 && (c = *lineptr) && isxdigit(c) 
				; ++cnt, ++lineptr)
					val = (val<<4) + ishex(c);
			--lineptr;
			c=val;
			break;

		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
			val=0;
			for (cnt=0, ++lineptr; cnt<2 && (c = *lineptr) && c>='0' && c<='7' 
				;++cnt, ++lineptr)
					val = (val<<3) + c - '0';
			c=val;
			break;
		}
	}
	return c;
}

ishex(c)
register int c;
{
	if (isdigit(c))
		return c-'0';
	else if (c >= 'a' && c <= 'f')
		return c-'a'+10;
	else if (c >= 'A' && c <= 'F')
		return c-'A'+10;
	else
		return -1;
}
cfsubs.c
isprint(c)
register int c;
{
	if (c >= 0x20 && c <= 0x7e)
		return 1;
	return 0;
}

isspace(c)
register int c;
{
	switch (c) {
	case 0x20:
	case '\t':
	case '\r':
	case '\n':
	case 0x0c:
		return 1;
	}
	return 0;
}

isalnum(c)
register int c;
{
	if ((c=tolower(c)) >= '0' && c <='9' || c >= 'a' && c <= 'z')
		return 1;
	return 0;
}

isxdigit(c)
register int c;
{
	if ((c=tolower(c)) >= '0' && c <= '9' || c >= 'a' && c <= 'f')
		return 1;
	return 0;
}

isdigit(c)
register int c;
{
	if (c >= '0' && c <= '9')
		return 1;
	return 0;
}
config.h
#define BADTOKEN -3
#define EOL -2
#define WORD 1
#define EQUALS 2
#define NOT 3

#define EOF -1
makefile
.SUFFIXES: .c .o .r86 .r .i 

.c.r:
	c65 -o $*.r $*.c

.c.i:
	cci -o $*.i $*.c

.c.r86:
	cc -o $*.r86 $*.c

.c.o:
	cc -c -o $*.o $*.c

config: config.o cffile.o cflex.o cfsubs.o
	cc -o config config.o cffile.o cflex.o cfsubs.o 

config.exe: config.r86 cffile.r86 cflex.r86 cfsubs.r86
	ln config.r86 cffile.r86 cflex.r86 cfsubs.r86 -lc

config.bin: config.r cffile.r cflex.r cfsubs.r
	ln65 -t -o xx config.r cffile.r cflex.r cfsubs.r -lc
	strip65 <xx >config.bin
config.doc
NAME

 config - Define device attributes

SYNOPSIS

 config [file]

DESCRIPTION

 config defines device attributes to a program that was created using
Aztec C. It does this by modifying, as directed by you, the table that
defines these attributes.

 This description of config first describes the device table, then how
to use config, and finally gives some examples of config usage.


1.  The Device Table

 The device table used by the SHELL and by programs of type PRG
(ie, programs that can only be run in the SHELL environment, because
they have been linked with the shmain module instead of the samain
module), resides in the SHELL's memory-resident environment area.
This area, which contains information that the SHELL needs to
maintain between program executions, is loaded from the file that
contains the SHELL. It is loaded only when necessary: when the
SHELL is first loaded and when the SHELL detects that its
environment area has been corrupted. PRG programs and BIN
programs created using Aztec C won't corrupt the SHELL's
environment area unless they go out of their way to do so, and the
SHELL tries to keep programs that were created without using Aztec C
from corrupting this area by setting the HIMEM field to the base of
this area. So when you're doing standard development activities under
the SHELL, the SHELL's memory-resident environment area (and
hence the SHELL's memory-resident device table) is normally only
loaded once: when the SHELL is first loaded.

 A program created using Aztec C that either runs under ProDOS
and is of type BIN and SYS (ie, that can run outside the SHELL
environment) or that runs under DOS 3.3 uses a device table that
resides in the program's memory space. This table is loaded along with
the program from a copy in the file from which the program is loaded.

 config can modify the copy of a program's device table that is in
the file that contains the program. For example, the SHELL is just a
program created using Aztec C, and you can modify the device table
that is within the file that contains the SHELL, shell.system. The
modifications that you make to this copy of the SHELL's device table
will take affect (ie, affect executing SHELL and PRG-type programs)
when the SHELL next reloads this table into its memory-resident
environment area.

 config can also directly modify the SHELL's memory-resident
device table, without modifying a file-resident device table. This
feature allows you to temporarily redefine device attributes; you can
modify a program's file-resident table when you want to make a more
permanent redefinition of device attributes.

 Before getting into the description on config usage, we are going to
define the names by which you will refer to devices. Then, since
config lets you access a device table at a low level, allowing you to
examine and modify specific fields and bits, we define the device table
in detail.

1.1  Device Names

 The following list defines the names of devices:

 con: The console device
 pr: The printer
 ser: The serial device
 sx: The device that's in slot x. For example s3: is the
 name of the device in slot 3.

1.2  Device Table Organization

 The include file device.h contains definitions related to devices.
This section defines information that is in that file.

 A device table has the following structure:

 struct _dev_info {
  char fnd_str[14]; /* signature */
  short con_flags; /* console flags */
  struct sgttyb tty; /* con info for ioctl */
  struct _name_dev
   dev_con, /* con: info */
   dev_pr, /* pr: info */
   dev_ser; /* ser: info */
  struct _slot_dev
   slots[8]; /* slot info */
  int init_max; /* init space size */
  int init_len; /* init space used */
  char init_buf[]; /* init space */
 };

1.2.1  The fnd_str Field

 The fnd_str field contains a (hopefully) unique character string;
when told to find a file's device table, config looks for this string.

1.2.2  The con_flags Field

 The con_flags field contains bits that define console attributes. the
following tables lists for each bit its symbolic name, a value in
parentheses that defines its location in the field, and the meaning of
the bit.

 CON_IMAP (0x01) 
    When this bit is set, the console driver will perform
    keyboard mapping. This mapping is needed needed
    for Apples whose keyboards don't support the full
    ASCII character set.
 CON_UPPR (0x02) 
    When this bit is set, the console driver will turn lower
    case characters into upper on output.
 CON_HIGH (0x80) 
    When this bit is set, the console driver will set the
    high order bit of each character it outputs.

 The config commands dump con: and mod con: are used to display
and modify the con_flags field.

1.2.3  The tty Field

 The tty field contains those fields related to a program's console I/O
that the program modifies by calling the ioctl function. For example,
this field defines whether the console is in line-oriented or character-
oriented mode. config doesn't modify this field, so we won't discuss it
further here. For a complete discussion of console I/O and ioctl, see
the Console I/O section of the Library Overview chapter.

1.2.4  The dev_con, dev_pr, and dev_ser Fields

 The dev_con, dev_pr, and dev_ser fields define the slot devices
that are associated with the con:, pr:, and ser: devices, respectively.
These three devices are just generic names that allow a program to
access the console, printer or serial card without having to know the
exact slot that contains the card. When a program performs I/O to one
of these generic devices, the I/O operation is simply passed on to its
associated slot device. For example, if a program writes to pr:, and
during one execution the device table used by the program specifies
that pr: is associated with slot device s1:, then the data will be written
to the card in slot 1. If during another execution the table specifies
that pr: is associated with slot device s2:, then the data will be written
to the card in slot 2.

 The config commands dump con:, dump pr:, and dump ser: display
the slot device that's associated with a generic device. And the
commands mod con:, mod pr:, and mod ser: define the slot device that's
associated with a generic device.

 The console device driver does not currently look at the dev_con
field; it always performs console input by calling the RDKEY ROM
routine that begins at 0xfd0c, which in turn calls the console input
routine whose address is in the KSW zero-page field. It performs
console output by calling the COUT ROM routine that begins at
0xfded, which in turn calls the console output routine whose address is
in the CSW zero-page field. If the Apple on which the SHELL is
running has an 80 column card, the SHELL will automatically set up
the CSW and KSW zero page fields to use it.


1.2.5  The slot Fields

 The slot field is an array of eight structures of type struct
_slot_dev, each of which defines the attributes of one slot device.
This structure will be defined after we finish describing the fields in
the device table structure.

1.2.6  The init_buf, init_max, and init_len Fields

 When a program opens a slot device, the slot device driver can
optionally send an initialization string to the device. The following
_dev_info fields define information related to device initialization
strings:

 * init_buf contains the strings;
 * init_max defines the length of the init_buf space;
 * init_len defines the number of bytes in init_buf that have
   been allocated to initialization strings.

 config allocates an initialization string by placing the string at the
current location defined by init_len and then incrementing init_len
past the string.

 The size of the SHELL's init_buf area is hard-coded into the
SHELL to be 64 bytes; this limit can't be exceeded. The size of a BIN,
SYS, or DOS 3.3 program's init_buf area is also hard-coded to be 64
bytes, but you can change this by modifying and replacing the devtab
module in the various versions of c.lib (c.lib, ci.lib, d.lib, and di.lib).

 The following config commands access the string initialization
fields:

 * The dump init command displays the contents of the init_max
   and init_len fields.
 * The mod init command resets the init_len field and turns off
   each slot device's INIT_STR bit, thus freeing all of the string
   initialization space.
 * The commands that display and modify information about slot
   devices (dump sx: and mod sx:) access the device table's string
   initialization fields.

1.3  The _slot_dev Structure

 A slot device's struct _slot_dev structure has the following form:

 struct _slot_dev {
  short outvec; /* CSW vector ($36-37) */
  short invec; /* KSW vector ($38-39) */
  short  init; /* init str offset in initbuf */
  char slot; /* $s0 */
  char hi_slot; /* slot number */
  char type; 
    /* -1=BASIC, 0=Pascal1.0, 1=Pascal1.1 */
  char flags; /* slot attr flags*/
  char tabp; /* line pos for tab mapping */
  char tabw; /* tab width */
  char iflags; /* slot init flags */
  char xtra; /* unused */
 };

1.3.1  The outvec and invec Fields

 The outvec and invec fields in a slot device's _slot_dev structure
contains the addresses of the routines that the slot device driver will
call to transfer each character to or from the slot device, respectively.
Normally, the slot device driver sets these addresses to locations within
the associated card's ROM space at the time the device is opened, by
determining the type of protocol (Basic, Pascal 1.0, or Pascal 1.1) used
by the ROM routines. As discussed below, the setting of these fields
for a particular slot device by the slot device driver can be disabled by
turning off its INIT_VEC bit in the iflags field of its _slot_dev
structure.

 config doesn't have commands to modify these fields, although the
dump sx: commands display their contents.

1.3.2  The init Field

 The init field in a slot device's _slot_dev structure contains the
offset within the _dev_info structure's init_buf area at which the
device's initialization string begins.

 As discussed below, when a slot device is opened, the INIT_STR
bit in the iflags field of the device's _slot_dev structure determines
whether the device's initialization string should be sent to it.

 The dump sx: command will display the initialization string that's
associated with slot device sx:, and the mod sx: str=... command will
assign an initialization string to sx:.

1.3.3  The type Field

 The type field in a slot device's _slot_dev structure defines the
type of protocol used by the device's ROM routines. It can have the
following values:

 Value Meaning
   -1 Basic
   0  Pascal 1.0
   1  Pascal 1.1

 This field is normally set when the device is opened, but if the
INIT_VEC bit in the iflags field of the device's structure is off, the
type field is not set.

 The dump sx: command will display sx:'s type field, but config
doesn't have a command to set it.

1.3.4  The slot and hi_slot fields

 The slot field in a slot device's _slot_dev structure defines the
high-order byte of the address of the device's ROM space. The hi_slot
field defines the number of the slot.

 The dump sx: command will display sx:'s slot and hi_slot fields, but
config doesn't have commands that will modify these fields.

1.3.5  The flags Field

 The flags field in a slot device's _slot_dev structure contains bits
defining various attributes of the slot. The following table lists for
each bit the symbolic name of the bit, a value in parentheses that
defines the location of the bit, and the bit's meaning.

 SLOT_LFCR (0x01) 
    When this bit is set, an carriage return character that
    is read from the device will be translated to a linefeed,
    and a linefeed character that is written to the device
    will be translated to a carriage return.
 SLOT_TABS (0x02) 
    When this bit is set, an output tab character will be
    replaced by enough space characters to position the
    device at the next "tab stop". For more information
    on tab stops, see the descriptions of the tabw and tabp
    fields.
 SLOT_UPPR (0x04) 
    When this bit is set, an output alphabetic character
    will be converted to upper case.
 SLOT_CRLF (0x08) 
    When this bit is set, and a program writes a carriage
    return character to the device, the device driver will
    also write a line feed character to the device.
 SLOT_HIGH (0x80) 
    When this bit is set, the high order bit will be set on
    all output characters.

 The dump sx: command will display in binary the contents of sx:'s
flags field, and the mod sx: command can be used to modify it.

1.3.6  The iflags Field

 The iflags field in a slot device's _slot_dev structure contains bits
that define attributes of the device. The following table lists for each
bit the symbolic name of the bit, a value in parentheses that defines
the location of the bit, and the bit's meaning.

 INIT_VEC (0x01) 
    When this bit is set and the slot device is opened, the
    addresses of its input and output vectors will be
    determined and set in the structure's invec and outvec
    fields based on the type of protocol used by the card,
    and the type field will be set.
 INIT_CAL (0x02) 
    When this bit is set and the slot device is opened, the
    card will be initialialized by issuing a call to the first
    byte of its ROM space.
 INIT_STR (0x04) 
    When this bit is set and the slot device is opened, the
    device's initialization string (which is pointed at by
    the init field of the device's _slot_dev structure) will
    be written to the device.
 INIT_ONCE (0x08) 
    When this bit is set, the initialization activities
    described above will only be performed once for the
    slot device.

 The dump sx: command will display in binary the contents of sx:'s
iflags field, and the mod sx: command can be used to modify it.

1.3.7  The tabw Field

 The tabw field in a slot device's _slot_dev structure defines the
number of characters between each of the device's tab stops. If 0, it's
assumed to be eight.

 When a tab character is written to a slot device, the slot device
driver can optionally output spaces in its place until the next tab stop is
reached. This replacement is enabled by setting the SLOT_TABS bit
in the flags fields of the device's _slot_dev structure.

 The dump sx: and mod sx: commands can be used to display and
modify the contents of sx:'s tabw and tabp fields.

1.3.8  The tabp Field

 The tabp field in a slot device's _slot_dev structure defines the
number of characters that have been send to it since the last carriage
return or linefeed. When a tab character is sent to a device, the device
driver uses the device's tabp field to decide how many spaces it should
output to reach the next tab stop.

1.3.9  The xtra Field

 This field is unused.


2.  Using config

 config is an interactive program: you enter commands to it to
examine device information, make modifications, and so on. These
commands access a copy of the table that resides in an internal buffer
within config: when you define the file whose device table you want to
modify (or the table that's in the SHELL's memory-resident
environment area), config finds the table and reads it into its internal
buffer; your examination and modification commands then access the
table that is in this internal buffer. When you've completed the
modifications, you type the write command, which causes config to
write the table back to the location from which it was obtained.

 The location of a table to be modified can be specified to config as
an argument when it is started. This argument can be the name of a
file containing a program whose device table is to be modified. It can
also be the word mem:, which specified that the SHELL's memory-
resident device table is to be modified. When config is started with
this optional argument, it automatically searches for the table in the
specified location and, if found, reads it into its table; if config doesn't
find the table, it will tell you.

 Alternatively, the location of a device information table that's to be
modified can be specified once config is active, by typing the open
command. This command takes a single argument: the name of the
file whose device table is to be modified; or mem:, if the SHELL's
memory-resident table is to be modified. The open command searches
for the table in the specified location, but doesn't load it into config's
internal buffer; if you want the table read, you must explicitly say so,
using config's read command. This allows you to make changes to one
program's device table and then write the modified table to several
different programs.


3.  Commands

 config has just a few basic commands, most of which have
arguments. We'll first list the commands and give a brief description.
The following paragraphs will then discuss the commands in detail.

 command Description
  dump Display information
  mod     Make modifications
  open    Prepare to examine/modify a device table
  read    Read a device table into config's internal buffer
  write   Write a device table from config's internal buffer
  quit    Halt config

3.1  The dump Command

 The dump command displays information about one device, about
all devices, or about the device initialization string space. The
command has the following format:

 dump [dev]

The optional argument dev is the name of the device about which you
want to get information, or the word init if you want information
about string space. If an argument isn't specified, information about
all devices and about string space is displayed.

3.2  The mod Command

 The mod command is used to modify device attributes and to reset
the device initialization string space. The command has the following
format:

 mod dev [args]

where dev is the name of the device whose attributes are to be
changed, or init if string space is to be initialized; [args] are arguments
defining the attributes that are to be changed. The arguments to the
mod command depend on the device being modified. The following
paragraphs discuss the modification of each device.

3.2.1  Modifying con:

 The command for modifying con: has the following format:

 mod con: [flags=xx] [imap] [uppr] [high]

where square brackets surround optional arguments. imap, uppr, and
high usually cause a bit to be turned on in the con_flags field. If
preceded by a ~ character, they cause the designated bit to instead be
turned off.

 The arguments have the following meanings.

 Argument       Meaning

 flags=xx      sets the field in the device table that defines console
               attributes, con_flags, to the hex value xx.

 imap          Set (or reset, if preceded by ~) the CON_IMAP bit in
               con_flags.

 uppr          Set or reset the CON_UPPR bit in con_flags.

 high          Set or reset the CON_HIGH bit in con_flags.

3.2.2  Modifying pr: and ser:

 The commands that modify the pr: and ser: attributes are similar:

   mod pr: sx:
   mod ser: sx:

where sx: is the name of the slot device that is to be associated with
pr: or ser:.

3.2.3  Modifying Slot Devices

 The command for modifying the attributes of a slot divice has the
following form:

 mod sx: [flags=xx] [iflags=xx] [tabw=dd]
  [lfcr] [crlf] [tabs] [uppr] [high] [cal] [once] [vec]
  [str=string]

where sx: is the name of the slot device.

3.2.3.1  The flags and iflags Arguments

 The flags=xx argument sets the device's flags field to the hex value
xx.

 Similarly, the iflags=xx argument set the device's iflags field to the
hex value xx.

3.2.3.2  The lfcr, ... Arguments

 The arguments specified on the second line (lfcr, ...) usually cause a
bit in the device's flags or iflags field to be turned on; if preceded by a
~ character, they instead cause the designated bit to be reset. The
symbolic name of the bit represented by these arguments can be
derived by appending "SLOT_" (for a flags bit) or "INIT_" (for an
iflags bit). For example, the command "mod s2: lfcr" sets the bit
SLOT_LFCR in the flags field in the _slot_dev structure for the s2:
device.

3.2.3.3  The str=string Argument

 The argument str=string usually sets the initialization string for the
specified slot device to string and turns on the INIT_STR bit for the
device. If the argument has the form ~str (ie, preceded by a ~
character and not followed by =string), the INIT_STR bit is instead
turned off.

 Strings are usually specified by surrounding them with double-
quote characters, although if a string contains just printable characters
with no spaces it can be specified without the surrounding quotes.

 In a quoted string, a printable character is represented by itself.
Unprintable characters are represented by a sequence that begins with
a backslash character, as defined in the following table.

 Sequence Meaning

  \n Newline
  \t Horizontal tab
  \b Backspace
  \r Carriage return
  \f Form Feed
  \\ Backslash
  \xyy The hex value yy

 For example, the command to set the initialization string for slot
device s2: to a string consisting of an escape character (hex value 1b),
followed by the character Q would be:

 mod s2: str="\x1bQ"

3.2.4  Reseting Device Initialization String Space

 The following command resets the use of the space used for device
initialization strings:

 mod init

 It resets the field that points to the top of allocated string space
(init_len in the device information structure), and turns off the
INIT_STR bit and clears the init field for each slot device.

3.3  The open Command

 The open command prepares config for accessing a device table that
is in a file or in the SHELL's memory-resident environment area. The
command has the following form:

 open file

where file is the name of the file whose device table is to be accessed,
or mem: to access the table in the SHELL's memory-resident
environment area.

 If a file is specified, the open command causes config to open the
file and search for the file's device information table. If the table is
not found, config will say so.

 Note that the open command does not read the specified device
information table into config's internal buffer; you must explicitly tell
config to do that, using the read command.

3.4  The read Command

 The read command causes config to read the device information
table from the currently open file or SHELL environment area into
config's internal buffer.

 Note that when a file or the SHELL's environment area is specified
as a command-line argument, config automatically reads the device
information table into its internal buffer, making it unnecessary to
issue a read command.

3.5  The write Command

 The write command causes config to write the device information
table that's in its internal buffer to the currently open file or SHELL
environment area.

3.6  The quit Command

 The quit command causes config to halt.


4.  Examples

 In this example, the device table in shell.system is modified for use
with an Image Writer printer that's connected to a Super Serial card in
slot 2. The changes are also written to the SHELL's memory-resident
table. First, we get config started by entering the following command
to the SHELL:

 config shell.system

Once started, config finds the device table that's in the file shell.system
and reads it into config's internal buffer.

 We next display the current settings of s2:'s flags and fields by
entering to config:

 dump s2:

We next set and reset flags and fields that define how i/o to s2: is to be
performed. These changes are made just to the device table that's in
config's internal buffer; they won't be made to the device table that's
in shell.system until the write command is issued.

 mod s2: vec cal ~lfcr tabs tabw=4 str="\x1bQ"

The operands to the mod command have the following meanings:

 * vec tells the device driver to determine the addresses of s2:'s
 ROM routines when s2: is opened;
 * cal tells the driver to call the device's initialization code,
 which begins at the first byte of its ROM, when the device is
 opened;
 * ~lfcr tells the driver not to send a linefeed after a carriage
   return;
 * tabs tells the driver to output spaces in place of tabs;
 * tabw=4 says that there are 4 spaces between tab stops;
 * str="\x1bQ" tells the driver to send the specified character
   string to the device when it is opened.

 We next display the modified settings of s2:'s flags and fields, which
are in the device table that's in config's internal buffer:

 dump s2:

 Everything's OK, so we write the modified device table back to
shell.system:

 write

 The changes that have just been made won't affect an executing
SHELL or PRG programs until the SHELL detects that its
environment area has been corrupted, or until the SHELL is loaded
following system power-up. To make these changes take effect
immediately, we need to write them to the SHELL's memory-resident
device table. To do this, we first tell config that we want to access this
memory-resident table by entering:

 open mem:

This doesn't affect the device table that's in config's internal buffer, so
we can immediately issue the following command to overwrite the
SHELL's memory-resident table:

 write

 We're all done, so we exit config by entering:

 quit
