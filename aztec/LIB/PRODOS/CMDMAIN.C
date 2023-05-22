#define NULL (char *) 0
#define BUFSIZ 80
static char buf[BUFSIZ];

_cmdline()
{
	register char *bp=buf;
	register int i=1;
	register int c;
	register int argc=1;
	char *argv[20];
	int cnt;

	argv[0] = "";
	write(1,"\ncmd? ",6);
	if ((cnt=read(0,buf,BUFSIZ)) <=0){
		argv[1]=NULL;
		return main(argc, argv);
	}
	buf[cnt-1]=0;

	while (*bp){
		while ((c=*bp) && (c==' ' || c=='\t'))
			++bp;
		if (!*bp)
			break;
		argv[i++] = bp;
		++argc;
		while ((c=*++bp) && c != ' ' && c != '\t')
			;
		*bp++ = 0;
	}
	argv[i] = NULL;
	return main(argc, argv);
}
#asm
*:ts=8
*
* Copyright (c) 1982,83,84,85 by Manx Software Systems, Inc.
*
	instxt	<zpage.h>

	global	_dev_info_,2
	global	_devinfo_,0
	global	_fil_tab_,2
	global	_filtab_,0
	global	errno_,2
	global	_Sp_,2

	public	_main_
_main_
	lda	#<_devinfo_
	sta	_dev_info_
	lda	#>_devinfo_
	sta	_dev_info_+1
	lda	#<_filtab_
	sta	_fil_tab_
	lda	#>_filtab_
	sta	_fil_tab_+1
	sec
	lda	SP
	sbc	#6
	sta	SP
	bcs	.1x
	dec	SP+1
.1x
	ldy	#5
	lda	#0
.2x
	sta	(SP),Y
	dey
	bpl	.2x
	jsr	_cmdline_
	ldy	#0
	tya
	sta	(SP),Y
	iny
	sta	(SP),Y
	jsr	exit_
	brk
;
	public	main_
	public	exit_

#endasm
