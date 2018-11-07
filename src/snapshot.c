/************************************************************************/
/*									*/
/* �����꡼�� ���ʥåץ���å�						*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "quasi88.h"
#include "screen.h"
#include "screen-func.h"
#include "graph.h"

#include "crtcdmac.h"
#include "memory.h"
#include "file-op.h"
#include "snapshot.h"
#include "initval.h"



char	file_snap[QUASI88_MAX_FILENAME];/* ���ʥåץ���åȥ١�����	*/
int	snapshot_format  = 0;		/* ���ʥåץ���åȥե����ޥå�	*/

char	snapshot_cmd[ SNAPSHOT_CMD_SIZE ];/* ���ʥåץ���åȸ女�ޥ��	*/
char	snapshot_cmd_do  = FALSE;	/* ���ޥ�ɼ¹Ԥ�̵ͭ		*/

#ifdef	USE_SSS_CMD
char	snapshot_cmd_enable = TRUE;	/* ���ޥ�ɼ¹Ԥβ���		*/
#else
char	snapshot_cmd_enable = FALSE;	/* ���ޥ�ɼ¹Ԥβ���		*/
#endif


/* ���ʥåץ���åȤ�����������Хåե��ȡ����Υ���ǥå��� */

char			screen_snapshot[ 640*400 ];
static PC88_PALETTE_T	pal[16 +1];





/***********************************************************************
 * ���ʥåץ���åȥե�����̾�ʤɤ�����
 ************************************************************************/
void	screen_snapshot_init(void)
{
    const char *s;

    if (file_snap[0] == '\0') {
	filename_init_snap(FALSE);
    }


    memset(snapshot_cmd, 0, SNAPSHOT_CMD_SIZE);
    s = getenv("QUASI88_SSS_CMD");			/* �¹ԥ��ޥ�� */
    if (s  &&  (strlen(s) < SNAPSHOT_CMD_SIZE)) {
	strcpy(snapshot_cmd, s);
    }

    snapshot_cmd_do = FALSE;	/* ����ͤϡ����ޥ�ɼ¹ԡؤ��ʤ��٤ˤ��� */



    if (file_wav[0] == '\0') {
	filename_init_wav(FALSE);
    }
}
void	screen_snapshot_exit(void)
{
    waveout_save_stop();
}




/*----------------------------------------------------------------------*/
/* ���̥��᡼������������						*/
/*----------------------------------------------------------------------*/

typedef	int		( *SNAPSHOT_FUNC )( void );

static	void	make_snapshot( void )
{
  int vram_mode, text_mode;
  SNAPSHOT_FUNC		(*list)[4][2];


  /* skipline �ξ��ϡ�ͽ�� snapshot_clear() ��ƤӽФ��Ƥ��� */

  if     ( use_interlace == 0 ){ list = snapshot_list_normal; }
  else if( use_interlace >  0 ){ list = snapshot_list_itlace; }
  else                         { snapshot_clear();
				 list = snapshot_list_skipln; }



	/* VRAM/TEXT �����Ƥ� screen_snapshot[] ��ž�� */

  if( sys_ctrl & SYS_CTRL_80 ){
    if( CRTC_SZ_LINES == 25 ){ text_mode = V_80x25; }
    else                     { text_mode = V_80x20; }
  }else{
    if( CRTC_SZ_LINES == 25 ){ text_mode = V_40x25; }
    else                     { text_mode = V_40x20; }
  }

  if( grph_ctrl & GRPH_CTRL_VDISP ){
    if( grph_ctrl & GRPH_CTRL_COLOR ){		/* ���顼 */
        vram_mode = V_COLOR;
    }else{
      if( grph_ctrl & GRPH_CTRL_200 ){		/* ��� */
	vram_mode = V_MONO;
      }else{					/* 400�饤�� */
	vram_mode = V_HIRESO;
      }
    }
  }else{					/* ��ɽ�� */
        vram_mode = V_UNDISP;
  }

  (list[ vram_mode ][ text_mode ][ V_ALL ])();


	/* �ѥ�åȤ����Ƥ� pal[] ��ž�� */

  screen_get_emu_palette( pal );

  pal[16].red   = 0;		/* pal[16] �Ϲ�����ǻ��Ѥ��� */
  pal[16].green = 0;
  pal[16].blue  = 0;
}






#if 0		/* XPM �ϥ��ݡ����оݳ� */
/*----------------------------------------------------------------------*/
/* ����ץ��㤷�����Ƥ�xpm �����ǥե�����˽���			*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_xpm( OSD_FILE *fp )
{
  unsigned char buf[80];
  int i, j, c;
  char *p = screen_snapshot;

  if( fp==NULL ) return 0;

  sprintf( buf,
	   "/* XPM */\n"
	   "static char * quasi88_xpm[] = {\n"
	   "\"640 400 16 1\",\n" );
  osd_fwrite( buf, sizeof(char), strlen(buf), fp );

  for( i=0; i<16; i++ ){
    sprintf( buf, "\"%1X      c #%04X%04X%04X\",\n",
	     i,
	     (unsigned short)pal[i].red   << 8,
	     (unsigned short)pal[i].green << 8,
	     (unsigned short)pal[i].blue  << 8 );
    osd_fwrite( buf, sizeof(char), strlen(buf), fp );
  }


  for( i=0; i<400; i++ ){

    osd_fputc( '\"', fp );

    for( j=0; j<640; j++ ){
      c = *p++;
      if( c < 10 ) c += '0';
      else         c += 'A' - 10;
      osd_fputc( c, fp );
    }

    sprintf( buf, "\",\n" );
    osd_fwrite( buf, sizeof(char), strlen(buf), fp );

  }

  sprintf( buf, "};\n" );
  osd_fwrite( buf, sizeof(char), strlen(buf), fp );

  return 1;
}
#endif



/*----------------------------------------------------------------------*/
/* ����ץ��㤷�����Ƥ�ppm ����(raw)�ǥե�����˽���			*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_ppm( OSD_FILE *fp )
{
  unsigned char buf[32];
  int i, j;
  char *p = screen_snapshot;

  if( fp==NULL ) return 0;

  strcpy( (char *)buf, 
	  "P6\n"
	  "# QUASI88\n"
	  "640 400\n"
	  "255\n" );
  osd_fwrite( buf, sizeof(char), strlen((char *)buf), fp );


  for( i=0; i<400; i++ ){
    for( j=0; j<640; j++ ){
      buf[0] = pal[ (int)*p ].red;
      buf[1] = pal[ (int)*p ].green;
      buf[2] = pal[ (int)*p ].blue;
      osd_fwrite( buf, sizeof(char), 3, fp );
      p++;
    }
  }

  return 1;
}



#if 0		/* PPM(ascii)  �ϥ��ݡ����оݳ� */
/*----------------------------------------------------------------------*/
/* ����ץ��㤷�����Ƥ�ppm ����(ascii)�ǥե�����˽���		*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_ppm_ascii( OSD_FILE *fp )
{
  unsigned char buf[32];
  int i, j, k;
  char *p = screen_snapshot;

  if( fp==NULL ) return 0;

  strcpy( buf, 
	  "P3\n"
	  "# QUASI88\n"
	  "640 400\n"
	  "255\n" );
  osd_fwrite( buf, sizeof(char), strlen(buf), fp );

  
  for( i=0; i<400; i++ ){
    for( j=0; j<640; j+=5 ){
      for( k=0; k<5; k++ ){
	sprintf( buf, "%3d %3d %3d ",
		 pal[ (int)*p ].red,
		 pal[ (int)*p ].green,
		 pal[ (int)*p ].blue );
	osd_fwrite( buf, sizeof(char), strlen(buf), fp );
	p++;
      }
      osd_fputc( '\n', fp );
    }

  }

  return 1;
}
#endif



/*----------------------------------------------------------------------*/
/* ����ץ��㤷�����Ƥ�bmp ����(win)�ǥե�����˽���			*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_bmp( OSD_FILE *fp )
{
  static const unsigned char header[] =
  {
    'B', 'M',			/* BM */
    0x36, 0xb8, 0x0b, 0x00,	/* �ե����륵���� 0xbb836 */
    0x00, 0x00,
    0x00, 0x00,
    0x36, 0x00, 0x00, 0x00,	/* �����ǡ������ե��å� 0x36 */

    0x28, 0x00, 0x00, 0x00,	/* ���󥵥��� 0x28 */
    0x80, 0x02, 0x00, 0x00,	/* ��	0x280 */
    0x90, 0x01, 0x00, 0x00,	/* �⤵	0x190 */
    0x01, 0x00,
    0x18, 0x00,			/* ������ */
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,	/* ����������?	0xbb800 */
    0x00, 0x00, 0x00, 0x00,	/* ������������?	*/
    0x00, 0x00, 0x00, 0x00,	/* ������������?	*/
    0x00, 0x00, 0x00, 0x00,	/* ���ѥѥ�åȿ�	*/
    0x00, 0x00, 0x00, 0x00,	/* ����?		*/
  };

  unsigned char buf[4];
  int i, j;
  char *p;

  if( fp==NULL ) return 0;

  osd_fwrite( header, sizeof(char), sizeof(header), fp );


  for( i=0; i<400; i++ ){
    p = screen_snapshot + (399-i)*640;
    for( j=0; j<640; j++ ){
      buf[0] = pal[ (int)*p ].blue;
      buf[1] = pal[ (int)*p ].green;
      buf[2] = pal[ (int)*p ].red;
      osd_fwrite( buf, sizeof(char), 3, fp );
      p++;
    }
  }

  return 1;
}



/*----------------------------------------------------------------------*/
/* ����ץ��㤷�����Ƥ�raw�����ǥե�����˽���			*/
/*----------------------------------------------------------------------*/
static int	save_snapshot_raw( OSD_FILE *fp )
{
  unsigned char buf[4];
  int i, j;
  char *p = screen_snapshot;

  if( fp==NULL ) return 0;

  for( i=0; i<400; i++ ){
    for( j=0; j<640; j++ ){
      buf[0] = pal[ (int)*p ].red;
      buf[1] = pal[ (int)*p ].green;
      buf[2] = pal[ (int)*p ].blue;
      osd_fwrite( buf, sizeof(char), 3, fp );
      p++;
    }
  }

  return 1;
}



/***********************************************************************
 * ���̤Υ��ʥåץ���åȤ򥻡��֤���
 *	��������VRAM���Ȥ˥��ʥåץ���åȤ�������롣
 *	����ɽ������Ƥ�����̤򥻡��֤���櫓�ǤϤʤ���
 *
 *	�Ķ��ѿ� ${QUASI88_SSS_CMD} ���������Ƥ����硢�����ָ��
 *	�������Ƥ򥳥ޥ�ɤȤ��Ƽ¹Ԥ��롣���κݡ�%a ���ե�����̾�ˡ�
 *	%b ���ե�����̾���饵�ե��å�������������Τˡ��֤�����롣
 *
 *	��) setenv QUASI88_SSS_CMD 'ppmtopng %a > %b.png'
 *
 ************************************************************************/

/*
  screen_snapshot_save() �ǡ������֤����ե�����̾�ϡ�
  ��ưŪ�����ꤵ���Τǡ������ֻ��˻��ꤹ��ɬ�פϤʤ���
  (�ǥ��������᡼����̾���ʤɤ˴�Ť������ꤵ���)

	�ե�����̾�ϡ� /my/snap/dir/save0001.bmp �Τ褦�ˡ�Ϣ�� + ��ĥ�� ��
	�ղä���롣Ϣ�֤ϡ� 0000 �� 9999 �ǡ���¸�Υե�����Ƚ�ʣ���ʤ�
	�褦��Ŭ�����ꤵ��롣

  ----------------------------------------------------------------------
  �ե�����̾���ѹ����������ϡ��ʲ��δؿ���Ȥ���

  �ե�����̾�μ��� �� filename_get_snap_base()
	�����Ǥ���ե�����̾�ϡ�/my/snap/dir/save �Τ褦�ˡ�Ϣ�֤ȳ�ĥ�Ҥ�
	�������Ƥ��롣

  �ե�����̾������ �� filename_set_snap_base()
	�㤨�С�/my/snap/dir/save0001.bmp �����ꤷ�Ƥ⡢������ Ϣ�֤� ��ĥ�Ҥ�
	�������롣���Τ��ᡢfilename_set_snap_base() �����ꤷ���ե�����̾��
	filename_get_snap_base() �Ǽ��������ե�����̾�ϰ��פ��ʤ����Ȥ����롣
	�ʤ���NULL ����ꤹ��ȡ�����ͤ����åȤ���롣

*/


/* file_snap[] �������� NNNN.suffix �ȤʤäƤ����硢�����������롣
   NNNN ��Ϣ�֤� 0000��9999��
   suffix �ϳ�ĥ�Ҥǡ��ʲ��Τ�Τ��оݤȤ��롣*/
static const char *snap_suffix[] = { ".ppm",  ".PPM", 
				     ".xpm",  ".XPM", 
				     ".png",  ".PNG", 
				     ".bmp",  ".BMP", 
				     ".rgb",  ".RGB", 
				     ".raw",  ".RAW", 
				     ".gif",  ".GIF", 
				     ".xwd",  ".XWD", 
				     ".pict", ".PICT",
				     ".tiff", ".TIFF",
				     ".tif",  ".TIF", 
				     ".jpeg", ".JPEG",
				     ".jpg",  ".JPG", 
				     NULL
};
static void truncate_filename(char filename[], const char *suffix[])
{
    int i;
    char *p;

    for (i=0; suffix[i]; i++) {

	if (strlen(filename) > strlen(suffix[i]) + 4) {

	    p = &filename[ strlen(filename) - strlen(suffix[i]) - 4 ];
	    if (isdigit(*(p+0)) &&
		isdigit(*(p+1)) &&
		isdigit(*(p+2)) &&
		isdigit(*(p+3)) &&
		strcmp(p+4, suffix[i]) == 0) {

		*p = '\0';
/*	printf("screen-snapshot : filename truncated (%s)\n", filename);*/
		break;
	    }
	}
    }
}



void		filename_set_snap_base(const char *filename)
{
    if (filename) {
	strncpy(file_snap, filename, QUASI88_MAX_FILENAME - 1);
	file_snap[ QUASI88_MAX_FILENAME - 1 ] = '\0';

	truncate_filename(file_snap, snap_suffix);
    } else {
	filename_init_snap(FALSE);
    }
}
const char	*filename_get_snap_base(void)
{
    return file_snap;
}



int	screen_snapshot_save(void)
{
    static char filename[ QUASI88_MAX_FILENAME + sizeof("NNNN.suffix") ];
    static int snapshot_no = 0;		/* Ϣ�� */

    static const char *suffix[] = { ".bmp", ".ppm", ".raw", };

    OSD_FILE *fp;
    int i, j, len, success;

    if (snapshot_format >= COUNTOF(suffix)) return FALSE;


	/* �ե�����̾��̤����ξ�硢����ե�����̾�ˤ��� */

    if (file_snap[0] == '\0') {
	filename_init_snap(FALSE);
    }


	/* file_snap[] ����ü�� NNNN.suffix �ʤ��� */

    truncate_filename(file_snap, snap_suffix);


	/* ¸�ߤ��ʤ��ե�����̾��õ������ (0000.suffix�� 9999.suffix) */

    success = FALSE;
    for (j=0; j<10000; j++) {

	len = sprintf(filename, "%s%04d", file_snap, snapshot_no);
	if (++ snapshot_no > 9999) snapshot_no = 0;

	for (i=0; snap_suffix[i]; i++) {
	    filename[ len ] = '\0';
	    strcat(filename, snap_suffix[ i ]);
	    if (osd_file_stat(filename) != FILE_STAT_NOEXIST) break;
	}
	if (snap_suffix[i] == NULL) {	    /* ���Ĥ��ä� */
	    filename[ len ] = '\0';
	    strcat(filename, suffix[ snapshot_format ]);
	    success = TRUE;
	    break;
	}
    }


	/* �ե�����򳫤��ơ����ʥåץ���åȥǡ�����񤭹��� */

    if (success) {

	success = FALSE;
	if ((fp = osd_fopen(FTYPE_SNAPSHOT_PPM, filename, "wb"))) {

	    make_snapshot();

	    switch (snapshot_format) {
	    case 0:	success = save_snapshot_bmp(fp);	break;
	    case 1:	success = save_snapshot_ppm(fp);	break;
	    case 2:	success = save_snapshot_raw(fp);	break;
	    }

	    osd_fclose(fp);
	}
/*
	printf("screen-snapshot : %s ... %s\n", 
				filename, (success) ? "OK" : "FAILED");
*/
    }


	/* �񤭹��������塢���ޥ�ɤ�¹Ԥ��� */

#ifdef	USE_SSS_CMD

    if (success &&

	snapshot_cmd_enable &&
	snapshot_cmd_do     &&
	snapshot_cmd[0]) {

	int  a_len, b_len;
	char *cmd, *s, *d;

	a_len = strlen(filename);
	b_len = a_len - 4;	/* ���ե��å��� ".???" ��4ʸ��ʬ���� */

	len = 0;
	s = snapshot_cmd;	/* ���ޥ�ɤ� %a, %b ���ִ�����Τ�   */
	while (*s) {		/* ���ޥ�ɤ�ʸ��Ĺ���ɤ��ʤ뤫������ */
	    if (*s == '%') {
		switch (*(s+1)) {
		case '%': len ++;	s++;	break; 
		case 'a': len += a_len;	s++;	break; 
		case 'b': len += b_len;	s++;	break; 
		default:  len ++;		break; 
		}
	    } else {      len ++; }

	    s++;
	}
				/* ������ʸ����ʬ��malloc ���� */
	cmd = (char *)malloc(len + 1);
	if (cmd) {

	    s = snapshot_cmd;
	    d = cmd;
	    while (*s) {	/* ���ޥ�ɤ� %a, %b ���ִ����Ƴ�Ǽ���Ƥ��� */
		if (*s == '%') {
		    switch (*(s+1)) {
		    case '%': *d++ = *s;			    s++; break;
		    case 'a': memcpy(d, filename, a_len); d+=a_len; s++; break;
		    case 'b': memcpy(d, filename, b_len); d+=b_len; s++; break;
		    default:  *d++ = *s;
		    }
		} else {      *d++ = *s; }
		s++;
	    }
	    *d = '\0';
				/* ����夬�ä����ޥ�ɤ�¹� */
	    printf("[SNAPSHOT command]%% %s\n", cmd);
	    system(cmd);

	    free(cmd);
	}
    }
#endif	/* USE_SSS_CMD */

    return success;
}





/***********************************************************************
 * ������ɽ��Ϥ򥻡��֤���
 *
 ************************************************************************/
#include "snddrv.h"

char	file_wav[QUASI88_MAX_FILENAME];		/* ������ɽ��ϥ١�����	*/

static const char *wav_suffix[] = { ".wav",  ".WAV", 
				     NULL
};


void		filename_set_wav_base(const char *filename)
{
    if (filename) {
	strncpy(file_wav, filename, QUASI88_MAX_FILENAME - 1);
	file_wav[ QUASI88_MAX_FILENAME - 1 ] = '\0';

	truncate_filename(file_wav, wav_suffix);
    } else {
	filename_init_wav(FALSE);
    }
}
const char	*filename_get_wav_base(void)
{
    return file_wav;
}



int	waveout_save_start(void)
{
    static char filename[ QUASI88_MAX_FILENAME + sizeof("NNNN.suffix") ];
    static int waveout_no = 0;		/* Ϣ�� */

    static const char *suffix = ".wav";

    int i, j, len, success;


	/* �ե�����̾��̤����ξ�硢����ե�����̾�ˤ��� */

    if (file_wav[0] == '\0') {
	filename_init_wav(FALSE);
    }


	/* file_wav[] ����ü�� NNNN.suffix �ʤ��� */

    truncate_filename(file_wav, wav_suffix);


	/* ¸�ߤ��ʤ��ե�����̾��õ������ (0000.suffix�� 9999.suffix) */

    success = FALSE;
    for (j=0; j<10000; j++) {

	len = sprintf(filename, "%s%04d", file_wav, waveout_no);
	if (++ waveout_no > 9999) waveout_no = 0;

	for (i=0; wav_suffix[i]; i++) {
	    filename[ len ] = '\0';
	    strcat(filename, wav_suffix[ i ]);
	    if (osd_file_stat(filename) != FILE_STAT_NOEXIST) break;
	}
	if (wav_suffix[i] == NULL) {	    /* ���Ĥ��ä� */
	    filename[ len ] = '\0';
	    strcat(filename, suffix);
	    success = TRUE;
	    break;
	}
    }


	/* �ե�����̾����ޤä��Τǡ��ե������򳫤� */

    if (success) {
	success = xmame_wavout_open(filename);
    }

    return success;
}

void	waveout_save_stop(void)
{
/*  if (xmame_wavout_opened())*/
	xmame_wavout_close();
}
