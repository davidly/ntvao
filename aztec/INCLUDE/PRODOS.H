#define MAXFILES 8
struct _fil_buf {
		char 	unit,			/* DOS return or device nr | 0x80 */
				flags;			/* r/w flags */
		char 	*iob;			/* alloc'd area for io buffer */
	};

extern struct _fil_buf *_fil_tab;

#define	RDWRT	2

/* file information structure */
struct finfo {
	unsigned char	access;
	unsigned char	file_type;
	unsigned short	aux_type;
	unsigned char	storage_type;
	unsigned short	blocks_used;
	unsigned short	mod_date;
	unsigned short	mod_time;
	unsigned short	create_date;
	unsigned short	create_time;
};

