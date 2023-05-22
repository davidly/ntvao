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
