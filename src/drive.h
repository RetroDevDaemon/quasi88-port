#ifndef DRIVE_H_INCLUDED
#define DRIVE_H_INCLUDED

#include <stdio.h>
#include "file-op.h"

#include "initval.h"
/*	�ɥ饤�֤ο� NR_DRIVE �ϡ�initval.h ������� */
/*	���᡼���ο� NR_IMAGE �ϡ�initval.h ������� */




	/**** �ɥ饤�������� ****/

typedef	struct{

  OSD_FILE *fp;			/* FILE �ݥ���			*/

  char	read_only;		/* �꡼�ɥ���꡼�ǥե�����򳫤����鿿	*/
  char	over_image;		/* ���᡼������¿�᤮����˿�		*/
  char	detect_broken_image;	/* ���줿���᡼�������Ĥ��ä��鿿	*/
  char	empty;			/* �ɥ饤�֤˶�����ꤷ�Ƥ���ʤ鿿	*/

  int	selected_image;		/* ���򤷤Ƥ��륤�᡼���ֹ� (0��)	*/
  int	image_nr;		/* �ե�������Υ��᡼����   (1��)	*/

  struct{			/* �ե�������������᡼���ξ���		*/
    char	name[17];	/*	���᡼��̾			*/
    char	protect;	/*	�ץ�ƥ���			*/
    char	type;		/*	�ǥ�����������			*/
    long	size;		/*	������				*/
  }image[ MAX_NR_IMAGE ];

				/* �������顢�����楤�᡼���Υ��	*/

  int	track;			/* ���ߤΥȥ�å��ֹ�			*/
  int	sec_nr;			/* �ȥ�å���Υ�������			*/
  int	sec;			/* ���ߤΥ������ֹ�			*/

  long	sec_pos;		/* ������  �θ��߰���			*/
  long	track_top;		/* �ȥ�å�����Ƭ����			*/
  long	disk_top;		/* �ǥ���������Ƭ����			*/
  long	disk_end;		/* ���᡼���ν�ü����			*/

  char	protect;		/* �饤�ȥץ�ƥ���			*/
  char	type;			/* �ǥ�����������			*/

				/* �ե�����̾				*/

  /* char	filename[ QUASI88_MAX_FILENAME ];*/

} PC88_DRIVE_T;



extern	PC88_DRIVE_T	drive[ NR_DRIVE ];




	/**** �ǥ��������� ���ե��å� ****/
#define	DISK_FILENAME	(0)		/* char x [17]	*/
#define	DISK_PROTECT	(26)		/* char		*/
#define	DISK_TYPE	(27)		/* char		*/
#define	DISK_SIZE	(28)		/* long		*/
#define	DISK_TRACK	(32)		/* long x [164]	*/

	/**** ID���� ���ե��å� ****/
#define	DISK_C		(0)		/* char		*/
#define	DISK_H		(1)		/* char		*/
#define	DISK_R		(2)		/* char		*/
#define	DISK_N		(3)		/* char		*/
#define	DISK_SEC_NR	(4)		/* short	*/
#define	DISK_DENSITY	(6)		/* char		*/
#define	DISK_DELETED	(7)		/* char		*/
#define	DISK_STATUS	(8)		/* char		*/
#define	DISK_RESERVED	(9)		/* char	x [5]	*/
#define	DISK_SEC_SZ	(14)		/* char	x [???]	*/

#define	SZ_DISK_ID	(16)		/* ID���� 16Byte*/


	/**** �ǥ�����/ID���� ��� ****/
#define DISK_PROTECT_TRUE	(0x10)
#define	DISK_PROTECT_FALSE	(0x00)

#define	DISK_TYPE_2D		(0x00)
#define	DISK_TYPE_2DD		(0x10)
#define	DISK_TYPE_2HD		(0x20)

#define	DISK_DENSITY_DOUBLE	(0x00)
#define	DISK_DENSITY_SINGLE	(0x40)

#define	DISK_DELETED_TRUE	(0x10)
#define	DISK_DELETED_FALSE	(0x00)



	/**** �ؿ� ****/


void	drive_init( void );
void	drive_reset( void );
int	disk_insert( int drv, const char *filename, int img, int readonly );
int	disk_change_image( int drv, int img );
void	disk_eject( int drv );
int	disk_insert_A_to_B( int src, int dst, int img );

void	drive_set_empty( int drv );
void	drive_unset_empty( int drv );
void	drive_change_empty( int drv );
int	drive_check_empty( int drv );

int	get_drive_ready( int drv );




#define	disk_same_file()		(drive[ 0 ].fp == drive[ 1 ].fp)
#define	disk_image_exist( drv )		(drive[ drv ].fp)
#define	disk_image_num( drv )		(drive[ drv ].image_nr)
#define	disk_image_selected( drv )	(drive[ drv ].selected_image)


#endif	/* DRIVE_H_INCLUDED */
