/* Copyright (C) 1985 by Manx Software Systems, Inc. */

#asm
	instxt	<zpage.h>
#endasm

_kb_get(buf, cnt)
register char *buf;
register int cnt;
{
	static char xbuf[256];
	static char *xp = 0;
	register unsigned short i;
	char *index();

	if (xp == 0) {
		*(char *)0x33 = 0xa0;
		(*((void (*)())0xfd6f))();			/* call the GETLN routine */
		movmem((char *)0x200, xbuf, 256);	/* copy to our buffer */
		for (xp=xbuf;*xp!=0x8d;xp++)		/* get rid of high bits */
			*xp &= 0x7f;
		*xp++ = '\n';						/* replace CR with LF */
		*xp = 0;							/* null terminate */
		xp = xbuf;
	}
	if ((i=strlen(xp)) < cnt)
		cnt = i;
	movmem(xp, buf, cnt);
	if (cnt == i)
		xp = 0;
	else
		xp += cnt;
	return(cnt);
}

_kb_put(buf, cnt)
register char *buf;
register int cnt;
{
#asm
	lda	$c000
	cmp	#$83
	bne	nobreak
	lda	$c010
	ldy	#2
	lda	#-1
	sta	(SP),Y
	iny
	sta	(SP),Y
	public	_exit_
	jmp	_exit_
nobreak
	ldy	#0
	lda	REGS+3
	sta	R0+1
	ldx	REGS+2
	stx	R0
	beq	skip
loop
	lda	(REGS),Y
	cmp	#$a
	bne	nolf
	lda	#$d
nolf
	cmp	#$9
	bne	notab
	lda	#$20
notab
	ora	#$80
	jsr	$fded
	iny
	bne	nobump
	inc	REGS+1
nobump
	dex
	bne	loop
skip
	dec	REGS+3
	bpl	loop
#endasm
}

