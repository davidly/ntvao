/* note: 
 *    if you're compiling config for execution on UNIX, 
 *    use the sgtty.h and device.h that come with the Aztec C package
 */
#include "sgtty.h"
#include "device.h"

char id[]="DEV_INFO04/86";
extern int fd;
extern struct _dev_info ds;
extern char initbuf[];
extern unsigned int loc;
extern int memflag;
long cseek(), lseek();

#if MPU6502 | MPUINT
#define DEVINFO 0xbc73
extern int memflag;
#endif

find()
{
	register int c;
	register char *cp;
	unsigned tloc, holdloc;

#if MPU6502 | MPUINT
	if (memflag)
		return 0;
#endif
	cseek(fd, 0L, 0);
	for (tloc=0;;++tloc){
		if((c=cget(fd)) == -1)
			return -1;
		if (c != id[0])
			continue;
		/* found first char, now see if rest match: */ 
		for (cp=id+1;*cp; ++cp){
			if ((c=cget(fd)) == -1)
				return -1;
			if (*cp != c)
				break;
		}
		if (*cp){
			tloc += cp-id;
			continue;
		} else
			break;
	}
	printf("found table at %x\n", tloc);
	loc = tloc;
	return 0;
}

readconfig()
{
#if MPU6502 | MPUINT
	register struct _dev_info *sp=DEVINFO;
	if (memflag){
		movmem(sp, &ds, sizeof(struct _dev_info));
		movmem(sp->init_buf, initbuf, ds.init_max);
		return 0;
	}
#endif
	if (cseek(fd, (long) loc, 0) < 0){
		perror("seek error");
		return 1;
	}
	if (read(fd, &ds, sizeof(struct _dev_info)) != sizeof(struct _dev_info)){
		perror("read error");
		return 1;
	}
	if (ds.init_len && read(fd, initbuf, ds.init_max) != ds.init_max){
		perror("error reading init buf");
		return 1;
	}
	return 0;
}

wrconfig()
{
#if MPU6502 | MPUINT
	register struct _dev_info *sp=DEVINFO;
	if (memflag){
		movmem(&ds, sp, sizeof(struct _dev_info));
		movmem(initbuf, sp->init_buf, ds.init_max);
		return;
	}
#endif
	if (cseek(fd, (long) loc, 0)<0){
		perror("seek error");
		return;
	}
	if (write(fd, &ds, sizeof(struct _dev_info)) != sizeof(struct _dev_info))
		perror("write error");
	if (ds.init_max && write(fd, initbuf, ds.init_max) != ds.init_max)
		perror("error writing init space");
}

#define FBUFSIZE 1024
char *fbuf, *fpos, *fmax;

cget()
{
	register int size;
	char *malloc();

	if (fpos == 0 || fpos >= fmax){
		if (fbuf == 0 && (fbuf = malloc(FBUFSIZE)) == (char *)0){
				printf("can't allocate file buffer\n");
				return -1;
		}
		if ((size=read(fd, fbuf, FBUFSIZE)) <= 0)
			return -1;
		fmax = fbuf+size;
		fpos=fbuf;
	}
	return 255 & *fpos++;
}

long
cseek(fd, how, origin)
long how;
{
	fpos=0;
	return lseek(fd, how, origin);
}
