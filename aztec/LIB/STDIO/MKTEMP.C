char *
mktemp(template)
char *template;
{
	register char *cp;
	register unsigned val;
	extern unsigned _main;

	cp = template;
	cp += strlen(cp);
	for (val = _main ; ; val /= 10)
		if (*--cp == 'X')
			*cp = val%10 + '0';
		else if (*cp != '.')
			break;

	if (*++cp != 0) {
		*cp = 'A';
		while (access(template, 0) == 0) {
			if (*cp == 'Z') {
				*template = 0;
				break;
			}
			++*cp;
		}
	} else {
		if (access(template, 0) == 0)
			*template = 0;
	}
	return template;
}
