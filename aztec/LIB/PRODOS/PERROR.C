#include <stdio.h>
#include <errno.h>

struct errlist {
	int code;
	char *msg;
} errlist[] = {
	0x00, "No error",
	0x01, "Invalid number for system call",
	0x04, "Invalid param count for system call",
	0x25, "Interrupt vector table full",
	0x27, "I/O Error",
	0x28, "No device connected/detected",
	0x2b, "Disk write protected",
	0x2e, "Disk switched",
	0x40, "Invalid characters in pathname",
	0x42, "File control block table full",
	0x43, "Invalid reference number",
	0x44, "Directory not found",
	0x45, "Volume not found",
	0x46, "File not found",
	0x47, "Duplicate file name",
	0x48, "Volume Full",
	0x49, "Volume directory full",
	0x4a, "Incompatible file format",
	0x4b, "Unsupported storage type",
	0x4c, "End of file encountered",
	0x4d, "Position out of range",
	0x4e, "File Access error; eg, file locked",
	0x50, "File is open",
	0x51, "Directory structure damaged",
	0x52, "Not a ProDOS disk",
	0x53, "Invalid system call parameter",
	0x55, "Volume control block table full",
	0x56, "Bad buffer address",
	0x57, "Duplicate volume",
	0x5a, "Invalid address in bit map",
	-1, "Unknown Error Code"
};

#define MAXERR 0x5a

perror (s)
char *s;
{
	register struct errlist * sp;

	if (errno < 0 || errno > MAXERR)
		return -1;
	if (s)
		fprintf (stderr, "%s: ", s);
	for (sp=errlist;sp->code>=0;sp++)
		if (sp->code == errno)
			break;
	fprintf (stderr, "%s\n", sp->msg);
	return 0;
}
