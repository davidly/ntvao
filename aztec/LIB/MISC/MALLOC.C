/* Copyright (C) 1984 by Manx Software Systems */

typedef struct freelist {
	unsigned f_size;
	struct freelist *f_chain;
} FREE;

#define NULL	(FREE *)0
#define GRAIN 1024

static FREE head, *last;

char *
realloc(area, size)
register char *area; unsigned size;
{
	register char *cp;
	unsigned osize;
	char *malloc();

	osize = (((FREE *)area-1)->f_size - 1) * sizeof(FREE);
	free(area);
	if ((cp = malloc(size)) != 0 && cp != area)
		movmem(area, cp, size>osize ? osize : size);
	return cp;
}

char *
malloc(size)
unsigned size;
{
	register FREE *tp, *prev;
	char *sbrk();
	int units;

	units = (size+sizeof(FREE)-1)/sizeof(FREE) + 1;
	if ((prev = last) == NULL)
		last = head.f_chain = prev = &head;

	for (tp = prev->f_chain ; ; prev = tp, tp = tp->f_chain) {
		while (tp != tp->f_chain && tp+tp->f_size == tp->f_chain) {
			if (last == tp->f_chain)
				last = tp->f_chain->f_chain;
			tp->f_size += tp->f_chain->f_size;
			tp->f_chain = tp->f_chain->f_chain;
		}

		if (tp->f_size >= units) {
			if (tp->f_size == units)
				prev->f_chain = tp->f_chain;
			else {
				last = tp + units;
				prev->f_chain = last;
				last->f_chain = tp->f_chain;
				last->f_size = tp->f_size - units;
				tp->f_size = units;
			}
			last = prev;
			tp->f_chain = NULL;
			return (char *)(tp+1);
		}
		if (tp == last) {
			if ((tp = (FREE *)sbrk(GRAIN)) == (FREE *)-1)
				return (char *)NULL;
			tp->f_size = GRAIN/sizeof(FREE);
			tp->f_chain = NULL;
			free(tp+1);
			tp = last;
		}
	}
}

free(area)
char *area;
{
	register FREE *tp, *hole, *tmp;

	hole = (FREE *)area - 1;
	if (area == NULL || hole->f_chain != NULL)
		return -1;
	for (tp = last ; tp > hole || hole > tp->f_chain ; tp = tp->f_chain) {
		tmp = tp;
		if (tmp >= tp->f_chain && (hole > tp || hole < tp->f_chain))
			break;
	}

	hole->f_chain = tp->f_chain;
	tp->f_chain = hole;
	last = tp;
	return 0;
}
