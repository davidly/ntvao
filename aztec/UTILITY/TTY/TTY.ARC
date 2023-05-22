tty.c
#include <errno.h>
int slot = 2;

#ifdef DOS33
extern int _DDRIVE;
#endif

#define getu() u6551get(slot)

#define putu(c) u6551put(slot, c)

char *index();

main(argc, argv)
int argc;
char **argv;
{
	int fp;
	char buf[40];
	int c;
	int baud = 9600;
	char *cp;
	extern char Signon[];

		/* note that term driver inits the uart	*/
	outinit();
	printf(Signon);
	while (--argc > 0) {
		cp = *++argv;
		if (*cp == '-') {
			switch(tolower(*++cp)) {
			case 's':
				slot = atoi(++cp);
				break;
			case 'b':
				baud = atoi(++cp);
				break;
#ifdef DOS33
			case 'd':
				_DDRIVE = atoi(++cp);
				break;
			default:
				printf("bad option %c\n",*cp);
				break;
#endif
			}
		}
	}
	slot <<= 4;
	dumb(getbaud(baud));
}

struct {
	int baud, bits;
} bauds[] = {
   50,	0x01,
   75,	0x02,
  110, 	0x03,
  150, 	0x05,
  300, 	0x06,
  600, 	0x07,
 1200, 	0x08,
 1800, 	0x09,
 2400, 	0x0a,
 3600, 	0x0b,
 4800, 	0x0c,
 7200, 	0x0d,
 9600, 	0x0e,
19200, 	0x0f,
};

getbaud(baud)
{
	int i;

	for (i=0;bauds[i].baud;i++) {
		if (bauds[i].baud == baud)
			return(bauds[i].bits);
	}
}

apple2e.a65
*:ts=8
*
*	APPLE //e TERMINAL
*
*

	instxt	<zpage.h>

HEAD	EQU	$65
TAIL	EQU	$67
STACK	EQU	$68
*
*
*
DATREG	EQU	$C088
STAREG	EQU	$C089
CMDREG	EQU	$C08A
CTLREG	EQU	$C08B
RMASK	EQU	$08
TMASK	EQU	$10
*
*
*
	global	slot_,2
*
	cseg
	public	dumb_
dumb_	tsx
	stx	STACK
	jsr	INIT
;	jsr	outinit_
	cli
LOOP	jsr	GETC
	jsr	PUTC
	jmp	LOOP
*
	public	EXIT
EXIT	sei
	sta	R0
	lda	#0
	sta	R0+1
	ldx	slot_
	lda	#$0B
	sta	CMDREG,X
	ldx	STACK
	txs
	rts
*
INIT	sei
	ldx	slot_
	lda	#$09		;transmit, recieve enabled, irq on recv enabled
	sta	CMDREG,X
	ldy	#2
	lda	(SP),Y		;get baud rate
	ora	#$10		;1 stop, 8 data, internal clock
	sta	CTLREG,X
	lda	DATREG,X
	lda	#<SERVICE
	sta	$3fe
	lda	#>SERVICE
	sta	$3ff
	lda	#0
	sta	HEAD
	sta	TAIL
	rts
*
SERVICE	tya
	pha
	txa
	pha
	ldx	slot_
	lda	STAREG,X
	and	#RMASK
	beq	SKIP
	lda	DATREG,X
	and	#$7f
	ldy	HEAD
	sta	$200,y
	inc	HEAD
SKIP	pla
	tax
	pla
	tay
	lda	$45
	rti
*
*
GETC	ldx	$C000
	bpl	GETC24
GETC20	cpx	=$80	;CTRL 2 _ -> exit
	bne	GETC21
	jsr	curs
	lda	$C010
	lda	=1
	jmp	EXIT
GETC21	cpx	=$ff
	bne	GETC22
	ldx	=$88
	bne	GETC23
GETC22	cpx	=$88
	bne	GETC23
	ldx	=$ff
GETC23	ldy	slot_
	lda	STAREG,Y
	and	=TMASK
	beq	GETC24
	sta	$C010
	txa
	and	=$7F
	sta	DATREG,Y
GETC24	rts
*
*
PUTC	ldx	TAIL
	cpx	HEAD
	bne	PUTC0
	rts
PUTC0	ldy	TAIL
	lda	$200,Y
	inc	TAIL
	jmp	outch
*
*
*
ALTCHRON	EQU	$C00F
ALTCHROFF	EQU	$C00E
ALTCHRRD	EQU	$C01E
PAGE2ON		EQU	$C055
PAGE2OFF	EQU	$C054
PAGE2RD		EQU	$C01C
COL80ON		EQU	$C00D
COL80OFF	EQU	$C00C
COL80RD		EQU	$C01F
STOR80ON	EQU	$C001
STOR80OFF	EQU	$C000
STOR80RD	EQU	$C018
SPKR		EQU	$C030
*
*
TP		EQU	$EA
BP		EQU	$EC
CHX		EQU	$FA
CHY		EQU	$FB
TMPY		EQU	$FC
TMPA		EQU	$FD
INESC		EQU	$FE
TMPC		EQU	$FF
*
	cseg
	public	outinit_
outinit_
	sta	COL80ON
	sta	STOR80ON
	sta	ALTCHRON
	lda	#0
	sta	INESC
	sta	CHX
	sta	CHY
	jsr	clear
	jmp	curs
*
*
	public	putchar_
putchar_
	lda	#0
	sta	INESC
	ldy	#2
	lda	(SP),Y
	cmp	#$0a
	bne	outch
	lda	#$0d
	jsr	outch
	lda	#$0a
*
	public	outch
outch
	jsr	curs
	lda	INESC
	beq	outch1
	jsr	doesc
	jmp	curs
outch1
	lda	TMPA
	cmp	#$20
	bcc	outch2
	jsr	store
	jsr	advanc
	jmp	curs
outch2
	jsr	chk
	jmp	curs
*
*
chk
	cmp	#$1a	;^Z
	bne	chk1
	jmp	clear
chk1
	cmp	#$08	;^H
	bne	chk2
	dec	CHX
	bpl	chkd
	lda	#79
	sta	CHX
	dec	CHY
	bpl	chkd
	lda	#0
	sta	CHY
	rts
chk2
	cmp	#$0C	;^L
	bne	chk3
advanc
	inc	CHX
	lda	CHX
	cmp	#80
	bne	chkd
	lda	#0
	sta	CHX
	inc	CHY
	lda	CHY
	cmp	#24
	bne	chkd
	jmp	lf
chk3
	cmp	#$0A	;^J
	bne	chk4
	inc	CHY
	lda	CHY
	cmp	#24
	bne	chkd
	dec	CHY
	jmp	lf
chk4	cmp	#$0B	;^K
	bne	chk5
	dec	CHY
	bpl	chkd
	inc	CHY
	rts
chk5
	cmp	#$1B	;ESC
	bne	chk6
	lda	#1
	sta	INESC
	rts
chk6
	cmp	#$0D	;^M
	bne	chk7
	lda	#$0
	sta	CHX
	rts
chk7
	cmp	#$07	;^G
	bne	chk8
	jmp	bell
chk8
	cmp	#$1E	;^^
	bne	chkd
	lda	#0
	sta	CHX
	sta	CHY
chkd
	rts
*
*
doesc
	tax
	lda	TMPA
	cpx	#$4
	bne	doesc1
	sec
	sbc	#$20
	sta	CHX
	lda	TMPC
	sta	CHY
	lda	#0
	sta	INESC
	rts
doesc1
	cpx	#$2
	bne	doesc2
	sec
	sbc	#$20
	sta	TMPC
	lda	#$4
	sta	INESC
	rts
doesc2
	cmp	#'=
	bne	doesc3
	lda	#$2
	sta	INESC
	rts
doesc3
	cmp	#'T		;clear to EOL
	bne	doesc6
	lda	CHX
	pha
doesc4
	lda	#$20
	jsr	store
	lda	CHX
	cmp	#79
	beq	doesc5
	jsr	advanc
	jmp	doesc4
doesc5
	pla
	sta	CHX
	jmp	escdon
doesc6
	cmp	#'Q		;char insert
	bne	doesc9
	lda	CHX
	sta	TMPC
	lda	#79
	sta	CHX
doesc7
	lda	CHX
	cmp	TMPC
	beq	doesc8
	dec	CHX
	jsr	calc
	lda	CHX
	lsr	A
	tay
	lda	(TP),Y
	eor	#$80
	inc	CHX
	jsr	store
	dec	CHX
	bpl	doesc7
doesc8
	lda	#$20
	jsr	store
	jmp	escdon
doesc9
	cmp	#'W		;char delete
	bne	doesc12
	lda	CHX
	pha
doesc10
	lda	CHX
	cmp	#79
	beq	doesc11
	inc	CHX
	jsr	calc
	lda	CHX
	lsr	A
	tay
	lda	(TP),Y
	eor	#$80
	dec	CHX
	jsr	store
	inc	CHX
	bne	doesc10
doesc11
	lda	#$20
	jsr	store
	pla
	sta	CHX
	jmp	escdon
doesc12
	cmp	#'E		;line insert
	bne	doesc13
	jsr	inslin
	jmp	escdon
doesc13
	cmp	#'R		;line delete
	bne	doesc14
	jsr	dellin
	jmp	escdon
doesc14
	cmp	#$02
	beq	doesc15
	cmp	#$03
	bne	escdon
doesc15
	lda	#0
	jmp	EXIT
escdon
	lda	#0
	sta	INESC
	rts
*
*
clear
	ldx	#47
clear1
	lda	STAB,X
	sta	TP+1
	dex
	lda	STAB,X
	sta	TP
	jsr	clrlin
	dex
	bpl	clear1
	lda	#0
	sta	CHX
	sta	CHY
	rts
*
clrlin
	sta	PAGE2OFF
clrlin1
	ldy	#39
	lda	#$a0
clrlin2
	sta	(TP),Y
	dey
	bpl	clrlin2
	lda	PAGE2RD
	bmi	clrlin3
	sta	PAGE2ON
	jmp	clrlin1
clrlin3
	rts
*
store
	pha
	jsr	calc
	lda	CHX
	lsr	A
	tay
	pla
	eor	#$80
	sta	(TP),Y
	rts
*
lf
	lda	CHX
	pha
	lda	#0
	sta	CHX
	sta	CHY
	jsr	dellin
	pla
	sta	CHX
	lda	#23
	sta	CHY
	jsr	calc
	rts
*
inslin
	lda	#0
	sta	CHX
	lda	CHY
	sta	TMPC
	lda	#23
	sta	CHY
inslin1
	lda	CHY
	cmp	TMPC
	beq	inslin2
	jsr	calc
	lda	TP
	sta	BP
	lda	TP+1
	sta	BP+1
	dec	CHY
	jsr	calc
	jsr	cpylin
	jmp	inslin1
inslin2
	jsr	calc
	jsr	clrlin
	rts
*
dellin
	lda	#0
	sta	CHX
	lda	CHY
	pha
dellin1
	lda	CHY
	cmp	#23
	beq	dellin2
	jsr	calc
	lda	TP
	sta	BP
	lda	TP+1
	sta	BP+1
	inc	CHY
	jsr	calc
	jsr	cpylin
	jmp	dellin1
dellin2
	jsr	calc
	jsr	clrlin
	pla
	sta	CHY
	rts
*
cpylin
	sta	PAGE2OFF
cpylin1
	ldy	#39
cpylin2
	lda	(TP),Y
	sta	(BP),Y
	dey
	bpl	cpylin2
	lda	PAGE2RD
	bmi	cpylin3
	sta	PAGE2ON
	jmp	cpylin1
cpylin3
	rts
*
curs
	sty	TMPY
	sta	TMPA
	jsr	calc
	lda	CHX
	lsr	A
	tay
	lda	(TP),Y
	cmp	#$20
	bcs	curs1
	ora	#$40
curs1
	eor	#$80
	cmp	#$40
	bcc	curs2
	cmp	#$60
	bcs	curs2
	and	#$bf
curs2
	sta	(TP),Y
	ldy	TMPY
	lda	TMPA
	rts
*
calc
	lda	CHY
	asl	A
	tay
	lda	STAB,Y
	sta	TP
	lda	STAB+1,Y
	sta	TP+1
	lda	CHX
	and	#1
	beq	calc1
	sta	PAGE2OFF
	rts
calc1
	sta	PAGE2ON
	rts
*
bell
	lda	#$20
	sta	TMPC
bell1
	lda	#$2
	jsr	delay
	sta	SPKR
	lda	#$24
	jsr	delay
	sta	SPKR
	dec	TMPC
	bne	bell1
	rts
*
delay
	sec
delay1
	pha
delay2
	sbc	#1
	bne	delay2
	pla
	sbc	#1
	bne	delay1
	rts
*
STAB	fdb	$400,$480,$500,$580,$600,$680,$700,$780
	fdb	$428,$4a8,$528,$5a8,$628,$6a8,$728,$7a8
	fdb	$450,$4d0,$550,$5d0,$650,$6d0,$750,$7d0
*
acumcrc.a65
*:ts=8

	instxt	<zpage.h>

AL	EQU	12
B	EQU	13
C	EQU	14
H	EQU	15
L	EQU	16

	global	crclow_,2
	global	crchigh_,2
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
	eor	crchigh_
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
	eor	crclow_
	sta	crchigh_
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
	sta	crclow_
	rts

u6551.a65
*:ts=8

	instxt	<zpage.h>

*
DATREG	EQU	$C088
STAREG	EQU	$C089
*
*		u6551get(slot);
*

CNT	equ	VAL

	public	u6551get_
u6551get_
	ldy	#2
	lda	(SP),Y
	tay
	ldx	#0
	stx	CNT
	lda	#$40
	sta	CNT+1
uget1	lda	CNT
	bne	uget2
	dec	CNT+1
	bpl	uget2
	dex
	txa
	bne	uget3
uget2	dec	CNT
	lda	STAREG,Y
	and	#$08
	beq	uget1
	lda	DATREG,Y
uget3	sta	R0
	stx	R0+1
	rts
*
*
*	u6551put(slot, char);
*
	public	u6551put_
u6551put_
	ldy	#4
	lda	(SP),Y
	sta	R0
	lda	#0
	sta	R0+1
	ldy	#2
	lda	(SP),Y
	tay
put1	lda	STAREG,Y
	and	#$10
	beq	put1
	tya
	ora	#DATREG+1
	tay
	lda	R0
	sta	$BFFF,Y
	rts
*
signon.c
char Signon[] = "TTY 1.3 5-23-86\n";
makefile
.SUFFIXES:.a65 .r
CFLAGS=

.c.r:
	c65 $(CFLAGS) $*.c

.a65.r:
	as65 $*.a65

TERM=apple2e.r u6551.r acumcrc.r
SRC= ttymake tty.c apple2e.a65 u6551.a65 acumcrc.a65 signon.c

tty.bin:	tty.r signon.r $(TERM) 
	ln65 -mt tty.r $(TERM) signon.i -lc
	strip65 <tty >tty.bin
tty.doc
NAME

 tty - terminal emulation program

SYNOPSIS

 tty -syy -bxx

DESCRIPTION

 tty is a terminal emulation program that allows an Apple //
operator to talk to another computer. To the other system, the Apple
// will appear to be a terminal that supports some of the special
features of the ADM-3A terminal.

 tty reads characters from the keyboard and writes them to a serial
interface. It also reads characters from this interface and writes them
to the console. This interface must be compatible with the Super
Serial Card.

 The -s option defines the number of the slot containing the
interface. The number immediately follows the -s, with no
intervening spaces. If this option isn't specified, the interface is
assumed to be in slot 2.

 The -b option defines the baud rate of the serial interface. The
baud rate immediately follows the -b option, with no intervening
spaces. If this option isn't specified, the baud rate is assumed to be
9600.

 To exit tty, type control-2; ie, type the '2' key while holding down
the control key.
