/* Copyright (C) 1985 by Manx Software Systems, Inc. */

#include	<sgtty.h>
#include	<device.h>
#define BASE (char *)0x212 /*jd 6/25/86...ProDOS close writes on 0x210-211 */

static char maptab[96] = {
	0x40,0x7b,0x02,0x7f,0x04,0x7c,0x06,0x07,
	0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x7e,0x0f,
	0x60,0x1b,0x7d,0x13,0x14,0x09,0x16,0x17,
	0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x5e,0x1f,
	0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
	0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,
	0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
	0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,
	0x70,0x61,0x62,0x63,0x64,0x65,0x66,0x67,
	0x68,0x69,0x6a,0x6b,0x6c,0x6d,0x6e,0x6f,
	0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,
	0x78,0x79,0x7a,0x7b,0x7c,0x6d,0x6e,0x7f };


_kb_get(buf, cnt)
char *buf;
{
	static char *endp = BASE;
	static char needmore;
	register unsigned char c;
	register unsigned short flags;

	flags = _dev_info->tty.sg_flags;
	if (endp == BASE || needmore) {
		for (;;) {
			if ((c = _cget((flags&NODELAY)!=0)) < 0) {
				needmore = 1;
				return(0);
			}
			*endp++ = c;
			if (flags&RAW)
				break;
			if ((flags & CRMOD) && c == '\r') {
				c = '\n';
				*(endp-1) = '\n';
			}
			if (_dev_info->con_flags&CON_IMAP) {
				if (c < 0x60)
					c = maptab[c];
				if (c >= 0x60 && (*(char *)0xc063&0x80) == 0x00)
					c ^= 0x20;
				*(endp-1) = c;
			}
			if (flags&ECHO)
				if (c == 0x15) {
					--endp;
					continue;
				} else
					chput(c);
			if (flags&CBREAK)
				break;
			if (c == 0x04 && endp == BASE+1) { /* ^D */
				endp = BASE;
				break;
			}
			if (c == 3) /* ^C */
				_exit(2);
			if (c == _dev_info->tty.sg_erase) {
				if (--endp > BASE)
					endp--;
				if (flags&ECHOE) {
					if (c == 0x08)
						_kb_put(" \x8", 2);
					else {
						_kb_put("\x8 \x8", 3);
						if (c >= ' ')
							_kb_put("\x8 \x8", 3);
					}
				}
			}
			if (c == _dev_info->tty.sg_kill) {
				chput('\n');
				endp = BASE;
			}
			if (c == '\n')
				break;
		}
	}
	if ((endp-BASE) < cnt)
		cnt = endp - BASE;
	movmem(BASE, buf, cnt);
	endp -= cnt;
	movmem(BASE+cnt, BASE, endp-BASE);
	needmore = 0;
	return(cnt);
}

#asm
	instxt	<zpage.h>

_cget_
	stx	R0+1
	ldy	#2
	lda	(SP),Y
	beq	x1
	lda	$c000
	bmi	x1
	lda	#$ff
	sta	R0+1
	bne	x2
x1
	jsr	$fd0c
	and	#$7f
x2
	sta	R0
	rts
#endasm

_kb_put(buf, cnt)
char *buf;
{
	register unsigned short i;

	for (i=0;i<cnt;i++)
		chput(*buf++);
	return(cnt);
}

static unsigned char pos;

#define	WNDWDTH	(*(char *)0x21)
#define	WNDTOP	(*(char *)0x22)
#define	WNDBTM	(*(char *)0x23)
#define	CH		(*(char *)0x24)
#define	CV		(*(char *)0x25)
#define	CH80	(*(char *)0x57b)
#define	VTAB	(*((void (*)())0xfc22))
#define	CLREOP	(*((void (*)())0xfc42))
#define	HOME	(*((void (*)())0xfc58))
#define	CLREOL	(*((void (*)())0xfc9c))

static
chput(c)
{
	register unsigned short flags, end;
	static char escflg;

	flags = _dev_info->tty.sg_flags;
	if (escflg == 0) {
		if (c == '\t' && (flags&XTABS)) {
			end = 0;
			while (pos >= end)
				end += flags & TABSIZ;
			end -= pos;
			while (end--) {
				_cput(' ');
				pos++;
			}
			return;
		}
		if ((c == '\n' || c == '\r') && (flags&CRMOD)) {
			_cput('\r');
			pos = 0;
			return;
		}
	}
#ifndef NOCURS
	if (escflg == 1 && c == 0x3d) {
		escflg = 2;
		return;
	}
	if (escflg == 2) {
		pos = c - 0x20;
		escflg = 3;
		return;
	}
	if (WNDWDTH <= 40) {
		if (escflg == 3) {
			CV = pos;
			CH = pos = c - 0x20;
			VTAB();
			escflg = 0;
			return;
		}
		if (escflg) {
			switch(c) {
			case 0x45:			/* insert blank line at cursor		*/
				WNDTOP = CV;
				_rscrl();
				WNDTOP = 0;
				break;
			case 0x51:			/* insert blank character at cursor	*/
				break;
			case 0x52:			/* delete line at cursor			*/
				WNDTOP = CV;
				CV = WNDBTM - 1;
				VTAB();
				_cput(0x0a);
				CV = WNDTOP;
				WNDTOP = 0;
				VTAB();
				break;
			case 0x54:			/* clear to end of line from cursor	*/
				CLREOL();
				break;
			case 0x57:			/* delete character at cursor		*/
				break;
			case 0x59:			/* clear to end of screen from curs	*/
				CLREOP();
				break;
			}
			escflg = 0;
			return;
		}
		switch(c) {
		case 0x0b:			/* cursor up		*/
			if (CV > 0)
				CV -= 1;
			VTAB();
			return;
		case 0x0c:			/* cursor right		*/
			if (++CH >= WNDWDTH) {
				if (CV >= WNDBTM-1)
					_cput(0x0a);
				else
					CV++;
				CH = 0;
			}
			VTAB();
			pos++;
			return;
		case 0x0d:			/* begin of line	*/
			CH = pos = 0;
			VTAB();
			return;
		case 0x1a:			/* home and clear	*/
			HOME();
			pos = 0;
			break;
		case 0x1e:			/* home				*/
			CH = CV = pos = 0;
			VTAB();
			break;
		}
	}
	else {
		if (escflg == 3) {
			CV = pos;
			CH80 = pos = c - 0x20;
			VTAB();
			escflg = 0;
			return;
		}
		if (escflg) {
			switch(c) {
			case 0x45:			/* insert blank line at cursor		*/
				WNDTOP = CV;
				_cput(0x16);
				WNDTOP = 0;
				break;
			case 0x51:			/* insert blank character at cursor	*/
				break;
			case 0x52:			/* delete line at cursor			*/
				WNDTOP = CV;
				_cput(0x17);
				WNDTOP = 0;
				break;
			case 0x54:			/* clear to end of line from cursor	*/
				_cput(0x1d);
				break;
			case 0x57:			/* delete character at cursor		*/
				break;
			case 0x59:			/* clear to end of screen from curs	*/
				_cput(0x0b);
				break;
			}
			escflg = 0;
			return;
		}
		switch(c) {
		case 0x0b:			/* cursor up		*/
			if (CV > 0)
				CV -= 1;
			VTAB();
			return;
		case 0x0c:			/* cursor right		*/
			c = 0x1c;
			pos++;
			break;
		case 0x0d:			/* begin of line	*/
			CH80 = pos = 0;
			VTAB();
			return;
		case 0x1a:			/* home and clear	*/
			pos = 0;
			c = 0x0c;
			break;
		case 0x1e:			/* home				*/
			pos = 0;
			c = 0x19;
			break;
		}
	}
	switch(c) {
	case 0x08:			/* cursor left		*/
		pos--;
		break;
	case 0x1b:
		escflg = 1;
		return;
	default:
		if (c >= 0x20)
			pos++;
		break;
	}
#endif
	_cput(c);
}

#asm
CON_FLAGS	equ	14
TTY_FLAGS	equ	16

_cput_
	lda	_dev_info_
	sta	R0
	lda	_dev_info_+1
	sta	R0+1
	ldy	#CON_FLAGS		;get console flags
	lda	(R0),Y
	and	#$80			;isolate HIGH flag bit
	ldy	#2
	ora	(SP),Y			;get character to output
	jsr	$fded			;send it
	ldy	#TTY_FLAGS+1	;get tty flag high byte
	lda	(R0),Y
	and	#4				;check for raw mode
	bne	skip			;yes, don't detect ^C
	lda	$c000			;get kbd char
	cmp	#$83			;is it ^C
	bne	skip			;no, skip
	ldx	$c010			;clear kbd strobe
	ldy	#2
	sta	(SP),Y			;do _exit(0x83)
	lda	#0
	iny
	sta	(SP),Y
	jsr	_exit_#
skip
	rts

WNDLFT	equ	$20
WNDWTH	equ	$21
WNDTOP	equ	$22
WNDBTM	equ	$23
CH		equ	$24		;cursor horizontal memory location
CV		equ	$25		;cursor vertical memory lockation
BASL	equ	$28		;pointer to beginning of line
BASH	equ	$29
BAS2L	equ	$2a
BAS2H	equ	$2b
VTAB	equ	$fc22
VTABZ	equ	$fc24
CLEOLZ	equ	$fc9e
*
_rscrl_
	ldx	WNDBTM
	dex
	txa
	pha
	jsr	VTABZ
RSCRL1	lda	BASL
	sta	BAS2L
	lda	BASH
	sta	BAS2H
	ldy	WNDWTH
	dey
	pla
	tax
	dex
	txa
	bmi	RSCRL3
	cmp	WNDTOP
	bcc	RSCRL3
	pha
	jsr	VTABZ
RSCRL2	lda	(BASL),Y
	sta	(BAS2L),Y
	dey
	bpl	RSCRL2
	bmi	RSCRL1
RSCRL3	ldy	#0
	jsr	CLEOLZ
	jmp	VTAB
#endasm

