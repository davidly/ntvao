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
