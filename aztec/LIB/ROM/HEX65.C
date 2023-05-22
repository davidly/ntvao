#include <stdio.h> 
#include <ctype.h>

#if MPU6502 | MPUINT
#define OFFSET 0L
#else
#define OFFSET 4L
#endif

#define	RECSIZ	16
#define RECMARK  ':'

#ifdef NOCTYPE
#define isspace(c)	(c == '\t' || c == ' ')
#define isalnum(c)	(c>='a'&&c<='z'||c>='A'&&c<='Z'||c=='_')
#endif

char lower, space;      			/* options flags */

unsigned laddr,      	                /* load address */
	base, baseflg;
unsigned Corg,
	Dorg,
	Cend,
	Dend,
	vectab[3];		/* nmi, reset, irq vectors */
unsigned part_size=2*1024; /* ROM size is 2k, by default */
int vectors=1;	/* generate interrupt & reset vectors by default */

char filename[20], *cp, *xp;

int infil;
FILE *fopen(), *outfil;
char *index(), *getval(), *getint();

main(ac,av)
char **av;
{
	init (ac, av);
	outinit();
	if (base == Corg-3)
		doseg(base, base+3);
	doseg (Corg, Cend);
	doseg (Dorg, Dend);
	outflsh();
	if (vectors)
		dovectors();
	puteof();
}

init(argc, argv)
char **argv;
{
	char buf[30];

	if (argc < 2)  {
		printf("usage: hx [-options] infile\n");
		exit(1);
	}

	while (--argc)  {
		if (**++argv == '-')
			setopt(*argv+1);
		else if (infil == 0)  {
			if ((infil = open(*argv, 0)) == -1)  {
				printf("Can't open input file %s!\n", *argv);
				exit(1);
			}
			strcpy(filename, *argv);
			if (cp=index(filename, '.'))
				*cp=0;
		} else  {
			printf("Bad arg %s\n",*argv);
			exit(1);
		}
	}

	strcpy(buf, filename);
	strcat(buf, ".x00");
	if ((outfil = fopen(buf, "w")) == NULL)  {
		printf("Can't open output file %s!\n", buf);
		exit(1);
	}

	getsyms();  
	
	printf("ROM size = %d kb\n", part_size/1024);
	printf("Base: %4x  Corg: %4x  Cend: %4x  Dorg: %4x  Dend: %4x\n",
					base, Corg, Cend, Dorg, Dend);
	if (vectors)
		printf("Vector addrs: nmi=%4x  reset=%4x  irq=%4x\n", 
			vectab[0], vectab[1], vectab[2]);
}

setopt(str)
char *str;
{
	char buf[20];
	char *xp;

	while (*str)  {
		switch (makelow(*str)) {
			case 'b':
				str = getval(&base, ++str);
				baseflg=1;
				break;
			case 'p':
				str = getint(&part_size, ++str);
				part_size *= 1024;
				break;
			case 'l':
				lower = 1;
				break;
			case 's':
				space = 1;
				break;
			case '?':
				puts("bxxxx -- base address = xxxx");
				puts("px -- rom part size = x kilobytes");
				puts("z -- don't generate reset & interrupt vectors");
				puts("l -- lower case hex digits");
				puts("s -- spaced format for legibility");
				exit(0);
			case 'z':
				vectors=0;
				break;
			default:
				printf("Invalid option <%c>\n", *str);
				break;
		}
		str++;
	}
}

doseg (org, end)
unsigned org, end;
{
	register int i;
	int siz;
	char buf[1024], *sp;

	lseek (infil, (long)(org - base) + OFFSET, 0);

	while ( (siz = read (infil, buf, sizeof(buf))) > 0) {
		for (sp=buf; org<end && sp < buf+siz; org++, sp++)
			outbyte(*sp);
		if (org > end) 
			break;
	}
}

unsigned char data[RECSIZ];
int reclen;                             /* record length */
unsigned char *dataptr;
int rectyp;	                             /* record type */

outinit()
{
	dataptr = data;
}

outbyte(c)
{
	*dataptr++ = c; 
	++reclen;
	if (laddr >= part_size){
		puteof();
		bumpext(1);
		laddr=0;
	}
	if (reclen == RECSIZ || reclen+laddr >= part_size) {
		putrec();
		laddr += reclen;
		dataptr = data;
		reclen = 0;
	}
}

outflsh()
{
	if (laddr >= part_size){
		puteof();
		bumpext(1);
		laddr=0;
	}
	if (reclen > 0)
		putrec();
	reclen=0;
}

putrec()      /* ":llaaaattdddd...ddddccCRLF" */
{
	putc (RECMARK, outfil);
	puth (reclen, 1, outfil);
	puth (laddr, 2, outfil);
	puth (rectyp, 1, outfil);

	putdata( data, reclen );

	puth ( check (data, reclen), 1, outfil);
	putc('\r',outfil);
	putc( '\n', outfil );
}

bumpext(incr)
{
	register char *cp;
	static int ext;
	char name[30], Ext[10];

	fclose(outfil);

	strcpy(name, filename);
	ext += incr;
	sprintf(Ext, ".x%02d", ext);
	strcat(name, Ext);

	if ((outfil = fopen(name, "w")) == NULL){
		printf("Can't open %s\n", name);
		exit(1);
	}
}

dovectors()
{
	int memaddr = Cend + Dend - Dorg + 2;
	int more = ((unsigned)0xfffa - memaddr)/part_size;
	if (more){
		puteof();
		bumpext(more);
	}
	putc (RECMARK, outfil);
	puth (6, 1, outfil);	/* 6 bytes for vectors **/
	puth (part_size-6, 2, outfil); /* vectors begin 6 bytes before end */
	puth (rectyp, 1, outfil);

	putdata( vectab, 6 );

	puth ( check (vectab, 6), 1, outfil);
	putc('\r',outfil);
	putc( '\n', outfil );
}

puteof()
{
	putc (RECMARK, outfil);
	puth(0, 1, outfil);
	puth(0, 2, outfil);
	puth(1, 1, outfil);
	puth(255, 1, outfil);
	putc('\r',outfil);
	putc('\n',outfil);
	return;
}

check(buf, size)                       /* compute checksum */
char *buf;
{
	int c = 0;
	while (size--)  {
		c += *buf++;
	}
	c += reclen + laddr + (laddr>>8) + rectyp;      /* laddr is unsigned */
	return((0-c) & 0xff);
}

putdata(dbuf, done)
char *dbuf;
{
	while (done--) {
		puth(*dbuf++, 1, outfil);
	}
}

#define	STRSIZ	10

puth(val, width, fil)
unsigned val;
FILE *fil;
{
	char *convert();
	char str[STRSIZ];
	char *s = str + STRSIZ - 1;   /* end of str */
	int i;

	if (space == 1)
		putc(' ', fil);
	if (width == 1)
		val &= 0xff;

	*(s) = '\0';
	s = convert(val, 16, s);
	if ((i = s - (str + STRSIZ - 1 - 2 * width)) > 0)
		while (i--)
			putc('0', fil);
	while (*s) 
		putc(*s++, fil);
}

static char digits[]="0123456789ABCDEF",
			lowers[]="0123456789abcdef";

char *
convert(n, base, s)
register int base; register char *s; register unsigned n;
{
	if (lower)  {
		do {
			*--s = lowers[(int)(n%base)];
		} while ( (n /= base) != 0 );
	} else {
		do {
			*--s = digits[(int)(n%base)];
		} while ( (n /= base) != 0 );
	}
	return s;
}

char *
getval(ip,s)
int *ip;
char *s;
{
	int val, i, j;
	int dig = 1;

	val = i = 0;

#ifdef DEBUG
	printf("getval: %s\n", s);
#endif
	while ( isxdigit(*s) )  {
		s++;
		i++;
	}
	for (j = i; j; j--)  {
		val += dig * htoi((int)*--s);
		dig *= 16;
	}
	*ip = val;
	return (s + i - 1);
}

char *
getint(ip,s)
int *ip;
char *s;
{
	int val=0;

	while ( *s && isdigit(*s) ){
		val = val*10 + *s-'0';
		++s;
	}
	*ip = val;
	return --s;
}

htoi(c)
int c;
{
	if (isdigit(c))
		return(c  - '0');
	c = makelow(c);
	return(10 + c - 'a');
}

getsyms()
{
	FILE *fp;
	char sym[80], buf[80];
	char *getval();

	strcpy(sym, filename);
	strcat(sym, ".sym");
	if ((fp = fopen(sym, "r")) == NULL)  {
		printf("Can't open symbol table file, %s\n", sym);
		exit(1);
	}
	while (fgets(buf, sizeof(buf), fp))  {
		getsym(sym, buf);
#ifdef DEBUG
		printf ("Examine sym: '%s' line: %s", sym, buf);
#endif
		if (Cend == 0 || Dend == 0)  {
			if (strcmp(sym, "_Corg_") == 0)
				getval(&Corg, buf);
			else if (strcmp(sym, "_Cend_") == 0)
				getval(&Cend, buf);
			else if (strcmp(sym, "_Dorg_") == 0)
				getval(&Dorg, buf);
			else if (strcmp(sym, "_Dend_") == 0)
				getval(&Dend, buf);
			else if (strcmp(sym, ".nmi") == 0)
				getval(&vectab[0], buf);
			else if (strcmp(sym, ".begin") == 0)
				getval(&vectab[1], buf);
			else if (strcmp(sym, ".irq") == 0)
				getval(&vectab[2], buf);
		}
		else
			break;
	}
	if (!baseflg) /* if base option not used, set base to min(Corg, Dorg) */
		base = Corg<Dorg? Corg:Dorg;

	fclose(fp);
}

getsym(d,s)
char *s, *d;
{
	while (!isspace(*s))
		s++;
	while (isspace(*s))
		s++;
	while (!isspace(*s))
		*d++ = *s++;
	*d = '\0';
}

char *
index(s, c)
register char *s; register int c;
{
	while (*s && *s != c)
		++s;
	return (*s? s: NULL);
}

makelow(c)
register int c;
{
	return (c >= 'A' && c <= 'Z' ? c-'A'+'a' : c);
}
