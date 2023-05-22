extern int errno;

/* ProDOS return codes */
#define EINVAL	1
#define ENOTDIR	3
#define EMFILE	0x42
#define EBADF	0x43
#define ENOENT	0x46
#define EEXIST	0x47
#define EACCES	0x4e
#define ENOTVOL	0x52
#define	ENOSPC	0x55
#define EARENA	7
#define ENOMEM	8
#define EFAULT	9
#define EINVENV	10
#define EBADFMT	11
#define EINVACC	12
#define EINVDAT	13
#define ENODEV	15
#define ERMCD	16
#define EXDEV	17
#define ENOMORE	18

/* additional codes used by Aztec C */
#define ENOTTY	20
#define EBADPRM	0x101
#define ENOFUNC	0x102
/* used by the math library */
#define ERANGE	21
#define EDOM	22
