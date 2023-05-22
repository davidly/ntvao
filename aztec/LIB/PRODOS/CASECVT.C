/*	Copyright (C) 1985 by Manx Software Systems	*/

toupper(c)
register int c;
{
	if (c >= 'a' && c <= 'z')
		return(c - 'a' + 'A');
	return(c);
}

tolower(c)
register int c;
{
	if (c >= 'A' && c <= 'Z')
		return(c - 'A' + 'a');
	return(c);
}

