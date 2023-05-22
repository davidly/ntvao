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
