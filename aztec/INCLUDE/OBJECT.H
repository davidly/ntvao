#define SPEC2	0x00
#define THEEND	0x00
#define ABSDATA	0x10
#define EXPR	0x20
#define SMLINT	0x30
#define SMLNEG	0x40
#define MEDINT	0x50
#define LCLSYM	0x60
#define GBLSYM	0x70
#define SPACE	0x80
#define USECOMN	0x90
#define CODEREF	0xa0
#define DATAREF 0xb0
#define BSSREF	0xc0
#define SEGREL	0xd0		/* segment relative reference */
#define SPECIAL	0xf0
#define DATASEG	0xf0		/* para # of data segment (8086 only) */
#define BEXPR	0xf1		/* byte expression */
#define BREL	0xf2		/* 8-bit PC relative */
#define LRGINT	0xf3
#define USECODE	0xf4
#define USEDATA	0xf5
#define STARTAD	0xf6
#define LRGCODE 0xf7
#define LRGDATA 0xf8
#define INTJSR	0xf9		/* interpreter call instruction */
#define WREL	0xfa		/* 16-bit PC relative */
#define LRGBSS	0xfb
#define HUGINT	0xfc		/* 32-bit constant */
#define LEXPR	0xfd		/* 32-bit address expression */
#define CODESEG	0xfe		/* para # of code segment (8086 only) */
#define NLINE	0xff

#define ADD	1
#define SUB	2
#define MUL	3
#define DIV	4
#define MOD	5
#define AND	6
#define OR	7
#define XOR	8
#define RSH	9
#define LSH	10
#define NOT	11 /* must be the first unary op, and only unarys may follow */
#define NEG	12
#define OHIGH 13
#define OLOW 14
#define OSEG 15

#define M_MAGIC	0x5466
#define M_LMAGIC	0x020c
#define M_OVROOT ((int)0xcf56)
#define M_LIBRARY ((int)0xec62)

struct module {
	short m_magic;			/* magic number for object files */
	char m_name[8];			/* module name */
	unsigned short m_code;	/* size of code in this module */
	unsigned short m_data;	/* size of data in this module */
	unsigned short m_static;/* size of static storage (bss) in this module */
	unsigned short m_global;/* offset of start of global symbols */
	short m_nglobal;		/* # of global symbols */
	unsigned short m_local;	/* offset of start of local symbols */
	short m_nlocal;			/* # of local symbols */
	unsigned short m_end;	/* end of local symbols */
	unsigned short m_next;	/* offset of next module in this file */
	unsigned short m_nfix;	/* number of segment fixes required */
};

#define LBSIZE (2048-2*sizeof(short))

struct newlib {
	short nl_magic;		/* magic number for library files */
	unsigned short nl_next;	/* location of next dictionary block */
	char nl_dict[LBSIZE];		/* dictionary buffer */
	/* the format of the dictionary is:
		two bytes - (position of module defining symbol) / 128
		nul terminated string - name of symbol
	*/
};

#define S_ABS		1
#define S_CODE		2
#define S_DATA		3
#define S_COMN		4		/* common segment (obsolete) */
#define S_CMNREF	5
#define S_ROOTSYM	6		/* used by linker to tag root symbols */
#define S_EXPR		6
#define S_UND		7
#define S_WEAK		8
#define S_BSS		9
#define S_GROUP		10
#define S_PSEG		11		/* public segment */
#define S_SSEG		12		/* stack segment */
#define S_MSEG		13		/* memory segment */
#define S_CSEG		14		/* common segment (new form) */
#define S_CLASS		15		/* segment class name */

#define ST_TYPE		0x0f	/* type part of symbol type */

#define ST_ALIGN	0x70		/* alignment type (for segments) */
#define ST_BYTE		0x00		/* alignment boundarys (byte) */
#define ST_WORD		0x10		/* even */
#define ST_PARA		0x20		/* 16-byte */
#define ST_PAGE		0x30		/* 256-byte */
#define ST_INPAGE	0x40		/* within 256 byte page */

/* symbol flags */
#define SF_DATAREF	0x02		/* used for externs in dataseg */
#define SF_HAVCLAS	0x04		/* symbol is followed by class index */
#define SF_GLOBL 	0x08		/* symbol is public/extern */
#define SF_HAVSEG	0x10		/* symbol is followed by segment/group # */
#define SF_OVSYM	0x20		/* overlay symbol */
#define S_FIXED		0x40
#define S_UNNAMED	0x80

struct symtab {
	char s_type,s_flags;
	unsigned short s_value;
};

/* symbol table entry is followed by a segment or group # if the SF_HAVSEG
	flag is set.

	Global symbols are followed by a nul terminated string which is the
	name of the symbol.
*/
