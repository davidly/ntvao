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

