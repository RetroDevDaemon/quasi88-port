#ifndef IMAGE_H_INCLUDED
#define IMAGE_H_INCLUDED


enum {
  D88_SUCCESS,		/* ���ｪλ				*/
  D88_NO_IMAGE,		/* �����ֹ�Υ��᡼��̵��		*/
  D88_BAD_IMAGE,	/* ���᡼���۾�(�إå���»����������­) */
  D88_MANY_IMAGE,	/* ���᡼���������С�			*/
  D88_ERR_SEEK,		/* ���������顼				*/
  D88_ERR_WRITE,	/* �饤�ȥ��顼				*/
  D88_ERR_READ,		/* �꡼�ɥ��顼				*/
  D88_ERR		/*					*/
};

int	d88_read_header( OSD_FILE *fp, long offset, unsigned char header[32] );
int	d88_append_blank( OSD_FILE *fp, int drv );
int	d88_write_protect(OSD_FILE *fp, int drv, int img, const char *protect);
int	d88_write_name( OSD_FILE *fp, int drv, int img, const char *name );
int	d88_write_format( OSD_FILE *fp, int drv, int img );
int	d88_write_unformat( OSD_FILE *fp, int drv, int img );


#define	READ_SIZE_IN_HEADER( header )					     \
	 (long)header[DISK_SIZE  ]      + ((long)header[DISK_SIZE+1]<<8)  +  \
	((long)header[DISK_SIZE+2]<<16) + ((long)header[DISK_SIZE+3]<<24)


#endif	/* IMAGE_H_INCLUDED */
