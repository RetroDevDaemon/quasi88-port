/************************************************************************/
/*									*/
/* �ǥ��������᡼���Υإå����ν��� (�ɤ߽�)				*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "drive.h"
#include "image.h"
#include "file-op.h"





/***********************************************************************
 * ���ꤵ�줿���ե��åȰ��֤��顢�إå����� 32�Х��Ȥ��ɤ߽Ф���
 *	�إå�������Ȥˡ����Υ��᡼�����������Х��Ȥ򥢥���������
 *	�����������Ԥ����饤�᡼��������Ƥ����Ƚ�Ǥ��롣
 *	�ե�����θ��߰��֤ϴؿ��ƤӽФ��������ѹ�����Ƥ���Τ���դ��뤳��
 *
 *	����	FILE *fp	�ե�����ݥ���
 *		long offset	���᡼������Ƭ�Υե��������
 *		Uchar header[32] �إå�����γ�Ǽ��
 *
 *	�֤���	D88_SUCCESS	���ｪλ
 *		D88_NO_IMAGE	����ʾ奤�᡼����̵��
 *		D88_BAD_IMAGE	���᡼���۾�(�إå���»����������­)
 *		D88_ERR_SEEK	���������顼
 *		D88_ERR_READ	�꡼�ɥ��顼
 *	header[32] �ˤϡ����᡼���Υإå������Ƥ����롣
 *	â�����֤��ͤ� D88_SUCCESS �ʳ��λ��ϡ�header[]�����Ƥ����ꡣ
 ************************************************************************/

int	d88_read_header( OSD_FILE *fp, long offset, unsigned char header[32] )
{
  long	size;
  char	c;
  long	current;
  int	result = D88_SUCCESS;


  if( (current = osd_ftell( fp )) < 0 ) result = D88_ERR_SEEK;

  if( result == D88_SUCCESS ){

    if( osd_fseek( fp, offset,  SEEK_SET ) ) result = D88_NO_IMAGE;

    if( result == D88_SUCCESS ){
      size = osd_fread( header, sizeof(char), 32, fp );
      if     ( size == 0 ) result = D88_NO_IMAGE;
      else if( size < 32 ) result = D88_BAD_IMAGE;

      if( result == D88_SUCCESS ){

	size = READ_SIZE_IN_HEADER( header );

	if( osd_fseek( fp, offset+size-1, SEEK_SET ) ) result = D88_BAD_IMAGE;
	if( result == D88_SUCCESS ){
	  if( osd_fread( &c, sizeof(char), 1, fp )!=1 ) result = D88_BAD_IMAGE;
	}

      }
    }

    if( (osd_fseek( fp, current, SEEK_SET )) ) result = D88_ERR_SEEK;
  }

  return result;
}



/***********************************************************************
 * ���ꤵ�줿���᡼���κǸ�ˡ��֥�󥯥ǥ������Υ��᡼�����ɲä��롣
 *	�ɲä����Τϡ�2D �Υǥ�������2DD/2HD ��̤�б�
 *
 *	����	FILE *fp	�ե�����ݥ���			
 *		int  drv	�ɥ饤�֤˥��åȤ���Ƥ���ե�����ξ�硢
 *				���Υɥ饤���ֹ档( 0 or 1 )
 *				�̤Υե�����ξ��� -1
 ************************************************************************/

int	d88_append_blank( OSD_FILE *fp, int drv )
{
  char *s, c[256];
  int	i, j, result = D88_SUCCESS;
  int	type = DISK_TYPE_2D;
  int	trk_nr, trk_size, img_size;
  long	current;

  switch( type ){
  case DISK_TYPE_2HD:
    trk_nr   = 164;
    trk_size = 0x2300;
    break;
  case DISK_TYPE_2DD:
    trk_nr   = 164;
    trk_size = 0x1600;
    break;
  case DISK_TYPE_2D:
  default:
    type     = DISK_TYPE_2D;
    trk_nr   = 84;
    trk_size = 0x1600;
    break;
  }
  img_size = (32) + (164*4) + (trk_nr*trk_size);


	/* ���ߤΥե�������֤�Ф��Ƥ�����(���Ȥǡ��᤹����) */

  if( (current = osd_ftell( fp )) < 0 ) { result = D88_ERR_SEEK; }

	/* �ե�������֤򡢥ե��������ü�˰�ư */

  if( result == D88_SUCCESS ){
    if( (osd_fseek( fp, 0,  SEEK_END )) ){ result = D88_ERR_SEEK; }
  }

	/* �إå����񤭹��� */

  if( result == D88_SUCCESS ){
    s = &c[0];
    for(i=0;i<17;i++) *s++ = 0;		/* filename */
    for(i=0;i<9;i++)  *s++ = 0;		/* reserved */
    *s++ = DISK_PROTECT_FALSE;		/* protect  */
    *s++ = type;			/* type     */
    *s++ = (img_size >>  0) & 0xff;	/* size     */
    *s++ = (img_size >>  8) & 0xff;
    *s++ = (img_size >> 16) & 0xff;
    *s++ = (img_size >> 24) & 0xff;
    if( osd_fwrite( c, sizeof(char), 32, fp )!=32 ){
      result = D88_ERR_WRITE;
    }
  }

	/* �ȥ�å����񤭹��� */

  if( result == D88_SUCCESS ){
    j = 0x2b0;				/* size of track data */
    for(i=0;i<164;i++){
      if( i<trk_nr ){
	c[0] = (j >>  0)&0xff;
	c[1] = (j >>  8)&0xff;
	c[2] = (j >> 16)&0xff;
	c[3] = (j >> 24)&0xff;
	j += trk_size;
      }else{
	c[0] = c[1] = c[2] = c[3] = 0;
      }
      if( osd_fwrite( c, sizeof(char), 4, fp )!=4 ) break;
    }
    if( i!=164 ){
      result = D88_ERR_WRITE;
    }
  }

	/* ���������񤭹��� */

  if( result == D88_SUCCESS ){
    for( i=0; i<256; i++ ) c[i] = 0;	/* sec data */
    for( i=0; i<trk_nr*trk_size; ){
      if( trk_nr*trk_size - i < 256 ) j = trk_nr*trk_size - i;
      else                            j = 256;
      if( osd_fwrite( c, sizeof(char), j, fp )!=(size_t)j ) break;
      i += j;
    }
    if( i!=trk_nr*trk_size ){
      result = D88_ERR_WRITE;
    }
  }

  osd_fflush( fp );


	/* �ե�������֤�ǽ�ΰ��֤��᤹ */

  if( current >= 0 ){
    if( (osd_fseek( fp, current, SEEK_SET )) ){ result = D88_ERR_SEEK; }
  }

	/* ���ｪλ ���� �ɥ饤�֤˥��åȤ��줿�ե����빹�����ϥ���⹹�� */

  if( result == D88_SUCCESS &&
      drv >= 0 ){

    int img, img_size;
    int type = DISK_TYPE_2D;

    switch( type ){
    case DISK_TYPE_2HD:
      img_size = 164 * 0x2300;
      break;
    case DISK_TYPE_2DD:
      img_size = 164 * 0x1600;
      break;
    case DISK_TYPE_2D:
    default:
      type     = DISK_TYPE_2D;
      img_size = 84 * 0x1600;
      break;
    }
    img_size += (32) + (164*4);


    for( i=0; i < 2; i++, drv ^= 1 ){
      if( !drive[ drv ].detect_broken_image ){
	if( drive[ drv ].image_nr + 1 <= MAX_NR_IMAGE ){
	  img = drive[ drv ].image_nr;
	  drive[drv].image[img].name[0] = '\0';
	  drive[drv].image[img].protect = DISK_PROTECT_FALSE;
	  drive[drv].image[img].type    = type;
	  drive[drv].image[img].size    = img_size;
	  drive[ drv ].image_nr ++;
	}else{
	  drive[ drv ].over_image = TRUE;
	}
      }
      if( drive[0].fp != drive[1].fp ) break;
    }

  }

  return result;
}



/*----------------------------------------------------------------------
 * �ǥ���������Ƭ������ꤵ�줿���᡼����õ���Ф������Υ��ե��åȰ��֤��֤�
 *	�ե�����θ��߰��֤ϴؿ��ƤӽФ��������ѹ�����Ƥ���Τ���դ��뤳��
 *
 *	����	FILE *fp	�ե�����ݥ���
 *		ing  img	õ�����᡼�����ֹ� 0��
 *		long *img_top	���Ĥ��ä���硢���Υե�������֤򥻥å�
 *
 *	�֤���	D88_SUCCESS	���ｪλ
 *----------------------------------------------------------------------*/

static	int	d88_search_image( OSD_FILE *fp, int img, long *image_top )
{
  unsigned char c[32];
  long offset = 0;
  int	num = 0, result;

  if( img >= MAX_NR_IMAGE ) return D88_MANY_IMAGE;

  while( (result = d88_read_header( fp, offset, c ) ) == D88_SUCCESS ){

    if( img == num ){
      *image_top = offset;
      return result;
    }

    num ++;
    offset += READ_SIZE_IN_HEADER( c );

    if( offset < 0 ){
      return D88_MANY_IMAGE;
    }

  }

  return result;
}


/***********************************************************************
 * ���ꤵ�줿���᡼���ֹ�Ρ��ץ�ƥ��Ⱦ����񤭴����롣
 *
 *	����	FILE *fp	�ե�����ݥ���			
 *		int  drv	�ɥ饤�֤˥��åȤ���Ƥ���ե�����ξ�硢
 *				���Υɥ饤���ֹ档( 0 or 1 )
 *				�̤Υե�����ξ��� -1
 *		int  img	���᡼���ֹ�
 *		char *protect   �ץ�ƥ��Ⱦ���
 ************************************************************************/
int	d88_write_protect( OSD_FILE *fp, int drv, int img, const char *protect)
{
  long	current;
  long	offset = 0;
  int	i, result;


	/* ���ߤΥե�������֤�Ф��Ƥ�����(���Ȥǡ��᤹����) */

  if( (current = osd_ftell( fp )) < 0 ) { return D88_ERR_SEEK; }

	/* img ���ܤΥ��᡼���򸡺�  */

  result = d88_search_image( fp, img, &offset );

	/* img ���ܤΥ��᡼�������Ĥ��ä���ץ�ƥ��Ⱦ����񤭴����ƽ�λ */

  if( result == D88_SUCCESS ){

    if( osd_fseek( fp, offset + DISK_PROTECT, SEEK_SET )==0 ){
      if( osd_fwrite( protect, sizeof(char), 1, fp )==1 ){
	   osd_fflush( fp );
	   result = D88_SUCCESS;
      }
      else result = D88_ERR_WRITE;
    }else  result = D88_ERR_SEEK;

  }

	/* �ե�������֤��Ȥΰ��֤��᤹ */

  if( (osd_fseek( fp, current, SEEK_SET )) ){ return D88_ERR_SEEK; }


	/* ���ｪλ ���� �ɥ饤�֤˥��åȤ��줿�ե����빹�����ϥ���⹹�� */

  if( result == D88_SUCCESS &&
      drv >= 0 ){

    for( i=0; i < 2; i++, drv ^= 1 ){

      drive[ drv ].image[ img ].protect = *protect;

      if( drive[ drv ].selected_image == img ){
	drive[ drv ].protect = drive[ drv ].image[ img ].protect;

	if( drive[ drv ].read_only ) drive[ drv ].protect = DISK_PROTECT_TRUE;
      }

      if( drive[0].fp != drive[1].fp ) break;
    }

  }

  return result;
}



/***********************************************************************
 * ���ꤵ�줿���᡼���ֹ�Ρ����᡼��̾��񤭴����롣
 *
 *	����	FILE *fp	�ե�����ݥ���			
 *		int  drv	�ɥ饤�֤˥��åȤ���Ƥ���ե�����ξ�硢
 *				���Υɥ饤���ֹ档( 0 or 1 )
 *				�̤Υե�����ξ��� -1
 *		int  img	���᡼���ֹ�
 *		char *name      ���᡼��̾(��Ƭ��16ʸ���ޤ�)
 ************************************************************************/
int	d88_write_name( OSD_FILE *fp, int drv, int img, const char *name )
{
  long	current;
  long	offset = 0;
  int	i, result;
  char	c[17];

  strncpy( c, name, 16 );
  c[16] = '\0';


	/* ���ߤΥե�������֤�Ф��Ƥ�����(���Ȥǡ��᤹����) */

  if( (current = osd_ftell( fp )) < 0 ) { return D88_ERR_SEEK; }

	/* img ���ܤΥ��᡼���򸡺�  */

  result = d88_search_image( fp, img, &offset );

	/* img ���ܤΥ��᡼�������Ĥ��ä��饤�᡼��̾��񤭴����ƽ�λ */

  if( result == D88_SUCCESS ){

    if( osd_fseek( fp, offset + DISK_FILENAME, SEEK_SET )==0 ){
      if( osd_fwrite( c, sizeof(char), 17, fp )==17 ){
	   osd_fflush( fp );
	   result = D88_SUCCESS;
      }
      else result = D88_ERR_WRITE;
    }else  result = D88_ERR_SEEK;

  }

	/* �ե�������֤��Ȥΰ��֤��᤹ */

  if( (osd_fseek( fp, current, SEEK_SET )) ){ return D88_ERR_SEEK; }


	/* ���ｪλ ���� �ɥ饤�֤˥��åȤ��줿�ե����빹�����ϥ���⹹�� */

  if( result == D88_SUCCESS &&
      drv >= 0 ){

    for( i=0; i < 2; i++, drv ^= 1 ){
      strncpy( drive[ drv ].image[ img ].name, name, 17 );

      if( drive[0].fp != drive[1].fp ) break;
    }

  }

  return result;
}



/***********************************************************************
 * ���ꤵ�줿���᡼���ֹ�Ρ����᡼���򥢥�ե����ޥåȾ��֤ˤ��롣
 *
 *	����	FILE *fp	�ե�����ݥ���			
 *		int  drv	�ɥ饤�֤˥��åȤ���Ƥ���ե�����ξ�硢
 *				���Υɥ饤���ֹ档( 0 or 1 )
 *				�̤Υե�����ξ��� -1
 *		int  img	���᡼���ֹ�
 ************************************************************************/
int	d88_write_unformat( OSD_FILE *fp, int drv, int img )
{
  unsigned char c[256];
  long	current;
  long	offset = 0;
  int	result;
  long	st, sz, len;


	/* ���ߤΥե�������֤�Ф��Ƥ�����(���Ȥǡ��᤹����) */

  if( (current = osd_ftell( fp )) < 0 ) { return D88_ERR_SEEK; }

	/* img ���ܤΥ��᡼���򸡺�  */

  result = d88_search_image( fp, img, &offset );

	/* img ���ܤΥ��᡼�������Ĥ��ä��饢��ե����ޥåȤ��ƽ�λ */

  if( result == D88_SUCCESS ){

    if( osd_fseek( fp, offset, SEEK_SET )==0 ){		/* �إå�����       */
      if( osd_fread( c, sizeof(char), 32, fp )==32 ){	/* ���ΤΥ��������� */
	sz = READ_SIZE_IN_HEADER( c );			/* (�Ϥ߽Ф�Ƚ����) */

	if( osd_fseek( fp, offset + DISK_TRACK, SEEK_SET )==0 ){
	  if( osd_fread( c, sizeof(char), 4, fp )==4 ){

	    st =((long)c[0]+((long)c[1]<<8)+((long)c[2]<<16)+((long)c[3]<<24));

	    memset( c, 0, 256 );		  /* 0�ȥ�å��ܤ���Ƭ����   */
	    if( DISK_TRACK+4 <= st && st < sz ){  /* ���᡼�������ޤ�0�饤�� */

	      if( osd_fseek( fp, offset+st, SEEK_SET )==0 ){

		while( st < sz ){
		  if( sz - st < 256 ) len = sz - st;
		  else                len = 256;
		  if( osd_fwrite( c, sizeof(char), len, fp ) != (size_t)len ){
		    result = D88_ERR_WRITE;
		    break;
		  }
		  st += len;
		}
		osd_fflush( fp );

  	      } else result = D88_ERR_SEEK;

	    } else   result = D88_BAD_IMAGE;

	  } else result = D88_ERR_READ;
	} else   result = D88_ERR_SEEK;

      } else   result = D88_ERR_READ;
    } else     result = D88_ERR_SEEK;

  }

	/* �ե�������֤��Ȥΰ��֤��᤹ */

  if( (osd_fseek( fp, current, SEEK_SET )) ){ return D88_ERR_SEEK; }


  return result;
}





/***********************************************************************
 * ���ꤵ�줿���᡼���ֹ�Ρ����᡼����ե����ޥåȤ��롣
 *	2D �Ρ�N88-BASIC �ǡ����ǥ������Ȥ��ƥե����ޥåȤ�Ԥʤ���
 *
 *	����	FILE *fp	�ե�����ݥ���			
 *		int  drv	�ɥ饤�֤˥��åȤ���Ƥ���ե�����ξ�硢
 *				���Υɥ饤���ֹ档( 0 or 1 )
 *				�̤Υե�����ξ��� -1
 *		int  img	���᡼���ֹ�
 ************************************************************************/


#define	GENERATE_SECTOR_DATA()					\
	do{							\
	  if( trk==37 && sec==13 ){				\
	    i = 1;						\
	  }else if( trk==37 && (sec==14||sec==15||sec==16) ){	\
	    i = 2;						\
	  }else{						\
	    i = 0;						\
	  }							\
	  d[i][ DISK_C ] = trk >> 1;				\
	  d[i][ DISK_H ] = trk & 1;				\
	  d[i][ DISK_R ] = sec;					\
	  d[i][ DISK_N ] = 1;					\
	  d[i][ DISK_SEC_NR +0 ] = 16 >> 0;			\
	  d[i][ DISK_SEC_NR +1 ] = 16 >> 8;			\
	  d[i][ DISK_DENSITY ] = DISK_DENSITY_DOUBLE;		\
	  d[i][ DISK_DELETED ] = DISK_DELETED_FALSE;		\
	  d[i][ DISK_STATUS ] = 0;				\
	  d[i][ DISK_SEC_SZ +0 ] = ((256) >> 0) & 0xff;		\
	  d[i][ DISK_SEC_SZ +1 ] = ((256) >> 8) & 0xff;		\
	}while(0)


int	d88_write_format( OSD_FILE *fp, int drv, int img )
{
  unsigned char c[32];
  unsigned char d[3][ 16 + 256 ];
  long	current;
  long	offset, st, sz;
  int	i, result, trk, sec;

	/* ��������������� */

  for( i=0; i<16;  i++ ) d[ 0 ][ i ] = 0x00;	/* �����ʳ����������� */
  for( i=0; i<256; i++ ) d[ 0 ][ 16 +i ] = 0xff;

  for( i=0; i<16;  i++ ) d[ 1 ][ i ] = 0x00;	/* 37�ȥ�å� ������13 */
  for( i=0; i<256; i++ ) d[ 1 ][ 16 +i ] = 0x00;

  for( i=0; i<16;  i++ ) d[ 2 ][ i ] = 0x00;	/* 37�ȥ�å� ������14��16 */
  for( i=0; i<160; i++ ) d[ 2 ][ 16 +i ] = 0xff;
  for(    ; i<256; i++ ) d[ 2 ][ 16 +i ] = 0x00;
  d[ 2 ][ 16 +37*2    ] = 0xfe;
  d[ 2 ][ 16 +37*2 +1 ] = 0xfe;


	/* ���ߤΥե�������֤�Ф��Ƥ�����(���Ȥǡ��᤹����) */

  if( (current = osd_ftell( fp )) < 0 ) { return D88_ERR_SEEK; }

	/* img ���ܤΥ��᡼���򸡺�  */

  result = d88_search_image( fp, img, &offset );

	/* img ���ܤΥ��᡼�������Ĥ��ä���ե����ޥåȤ��ƽ�λ */

  if( result == D88_SUCCESS ){

    if( osd_fseek( fp, offset, SEEK_SET )==0 ){		/* �إå�����       */
      if( osd_fread( c, sizeof(char), 32, fp )==32 ){	/* ���ΤΥ��������� */
	sz = READ_SIZE_IN_HEADER( c );			/* (�Ϥ߽Ф�Ƚ����) */

	c[ 0 ] = DISK_TYPE_2D;				/* 2D���᡼�������� */
	if( osd_fseek( fp, offset + DISK_TYPE, SEEK_SET )==0 ){
	  if( osd_fwrite( c, sizeof(char), 1, fp )==1 ){

	    for( trk=0; trk<80; trk++ ){		/* 80�ȥ�å��񴹤� */

	      if( osd_fseek( fp, offset+(DISK_TRACK+trk*4), SEEK_SET )==0 ){
		if( osd_fread( c, sizeof(char), 4, fp )==4 ){

		  st =  (long)c[0]     +((long)c[1]<<8)
		       +((long)c[2]<<16)+((long)c[3]<<24);

		  if( osd_fseek( fp, offset+st, SEEK_SET )==0 ){

		    if( DISK_TRACK+80*4 <= st && st+(16+256)*16 <= sz ){

		      for( sec=1; sec<=16; sec++ ){	/* 16�������񴹤� */
			GENERATE_SECTOR_DATA();
			if( osd_fwrite( d[i], sizeof(char), 16+256, fp )
			    				 != 16+256 ){
			  result = D88_ERR_WRITE;
			  break;
			}
		      }

		    }

		  }else result = D88_BAD_IMAGE;

		}else result = D88_ERR_READ;
	      }else   result = D88_ERR_SEEK;

	      if( result != D88_SUCCESS ){
		break;
	      }
	    }
	    osd_fflush( fp );

	  }else result = D88_ERR_WRITE;
	}else   result = D88_ERR_SEEK;

      }else result = D88_ERR_READ;
    }else   result = D88_ERR_SEEK;

  }

	/* �ե�������֤��Ȥΰ��֤��᤹ */

  if( (osd_fseek( fp, current, SEEK_SET )) ){ return D88_ERR_SEEK; }


	/* ���ｪλ ���� �ɥ饤�֤˥��åȤ��줿�ե����빹�����ϥ���⹹�� */

  if( result == D88_SUCCESS &&
      drv >= 0 ){

    for( i=0; i<2; i++, drv ^= 1 ){
      drive[ drv ].image[ img ].type = DISK_TYPE_2D;

      if( drive[0].fp != drive[1].fp ) break;
    }

  }

  return result;
}
