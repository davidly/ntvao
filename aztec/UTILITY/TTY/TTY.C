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

