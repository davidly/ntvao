
struct _slot_dev {
	short	outvec;			/* CSW vector ($36-37)					*/
	short	invec;			/* KSW vector ($38-39)					*/
	short 	init;			/* offset of init str into init_buf 	*/
	char	slot;			/* slot number							*/
	char	hi_slot;		/* $s0									*/
	char	type;			/* -1=BASIC, 0=Pascal1.0, 1=Pascal1.1	*/
	char	flags;			/* flag byte for simple mappings		*/
	char	tabp;			/* head position for tab mapping		*/
	char	tabw;			/* tab width							*/
	char	iflags;			/* initialization flags					*/
	char	xtra;			/* unused 								*/
};

/*
 *	Bit definition for "flags" field.
 */

#define	SLOT_LFCR	0x01	/* map lf to cr/lf				*/
#define	SLOT_TABS	0x02	/* map tabs to spaces			*/
#define	SLOT_UPPR	0x04	/* map lower case to upper		*/
#define	SLOT_CRLF	0x08	/* follow cr with lf			*/
#define	SLOT_HIGH	0x80	/* turn on high bit of chars	*/

/*
 *	Bit definition for "iflags" field.
 */

#define	INIT_VEC	0x01	/* init the vector and type		*/
#define	INIT_CAL	0x02	/* call the init function		*/
#define	INIT_STR	0x04	/* send the init string			*/
#define	INIT_ONCE	0x08	/* only do it once				*/
#define	BACK_VEC	0x10
#define	BACK_CAL	0x20
#define	BACK_STR	0x40

struct _name_dev	{
	char	dev_nam[5];
	char	dev_num;
};

/* bits for con_flags */

#define	CON_IMAP	0x01
#define	CON_UPPR	0x02
#define	CON_HIGH	0x80

struct _dev_info {
	char 				fnd_str[14];	/* signature string for config	*/

	unsigned short		con_flags;		/* console flags			*/
	struct sgttyb		tty;			/* console tty structure	*/

	struct _name_dev	dev_con,		/* the named slots			*/
						dev_pr,
						dev_ser;

	struct _slot_dev	slots[8];		/* the slots themselves		*/
	int					init_max;		/* no. bytes of init space */
	int					init_len;		/* no bytes used of init space */
	char				init_buf[];		/* the init space */
};

extern struct _dev_info *_dev_info;

