/***********************************************************************
 *			�ե�����̾���桿����
 ************************************************************************/
#include "getconf.h"

char	file_disk[2][QUASI88_MAX_FILENAME];	/*�ǥ��������᡼���ե�����̾*/
int	image_disk[2];	 	  		/*���᡼���ֹ�0��31,-1�ϼ�ư*/
int	readonly_disk[2];			/*�꡼�ɥ���꡼�ǳ����ʤ鿿*/

char	file_tape[2][QUASI88_MAX_FILENAME];	/* �ơ��������ϤΥե�����̾ */
char	file_prn[QUASI88_MAX_FILENAME];		/* �ѥ�����ϤΥե�����̾ */
char	file_sin[QUASI88_MAX_FILENAME];		/* ���ꥢ����ϤΥե�����̾ */
char	file_sout[QUASI88_MAX_FILENAME];	/* ���ꥢ�����ϤΥե�����̾ */

int	file_coding = 0;			/* �ե�����̾�δ���������   */
int	filename_synchronize = TRUE;		/* �ե�����̾��ƱĴ������   */



static char *assemble_filename(const char *imagename,
			       const char *basedir,
			       const char *suffix);

/*----------------------------------------------------------------------
 * �ѿ� file_XXX[] �����ꤵ��Ƥ���ե�����򤹤٤Ƴ�����
 *	�̾�ϡ��ե�����򳫤��ݤ� file_XXX[] �����ꤹ��Τ�����
 *	�ʲ��ξ��ϡ���� file_XXX[] �����ꤵ��Ƥ��ޤäƤ���Τǡ�
 *	���δؿ���ƤӽФ�ɬ�פ����롣
 *
 *	����ư��          �ĥ��ץ����� file_XXX[] �����ꤵ���
 *	�����ơ��ȥ��ɻ����ѿ� file_XXX[] ����������롣
 *			    (��ư���Υ��ץ����ǥ��ơ��ȥ��ɤ�������Ʊ��)
 *
 *	���ơ��ȥ��ɻ��ϡ� stateload �򿿤ˤ��ơ��ƤӽФ���
 *----------------------------------------------------------------------*/
static	void	imagefile_all_open(int stateload)
{
    int err0 = TRUE;
    int err1 = TRUE;
    int err2 = TRUE;
    int err3 = TRUE;

    if (stateload == FALSE) {
	int i;
	for (i=0; i<NR_DRIVE; i++) {
	    memset(file_disk[i], 0, QUASI88_MAX_FILENAME);
	    if (config_image.d[i]) {
		strcpy(file_disk[i], config_image.d[i]);
	    }
	    image_disk[i]    = config_image.n[i];
	    readonly_disk[i] = config_image.ro[i];
	}

	for (i=0; i<NR_TAPE; i++) {
	    memset(file_tape[i], 0, QUASI88_MAX_FILENAME);
	    if (config_image.t[i]) {
		strcpy(file_tape[i], config_image.t[i]);
	    }
	}
    }
    /* ���ơ��ȥ��ɻ��ϡ� file_XXX ������Ѥ� */
    /* �����ʲ��ϸ��ߥ��ơ��ȥ����֤��Ƥ��ʤ��Τǡ�̵�������� */
    {
	memset(file_prn, 0, QUASI88_MAX_FILENAME);
	if (config_image.prn) {
	    strcpy(file_prn, config_image.prn);
	}

	memset(file_sin, 0, QUASI88_MAX_FILENAME);
	if (config_image.sin) {
	    strcpy(file_sin, config_image.sin);
	}

	memset(file_sout, 0, QUASI88_MAX_FILENAME);
	if (config_image.sout) {
	    strcpy(file_sout, config_image.sout);
	}
    }


    if (file_disk[0][0] &&	/* �ɥ饤��1,2 �Ȥ⥤�᡼������Ѥߤξ�� */
	file_disk[1][0]) {		/*	% quasi88 file file       */
					/*	% quasi88 file m m        */
					/*	% quasi88 file n file     */
					/*	% quasi88 file file m     */
					/*	% quasi88 file n file m   */
	int same = (strcmp(file_disk[0], file_disk[1]) == 0) ? TRUE : FALSE;

	err0 = disk_insert(DRIVE_1,		/* �ɥ饤�� 1 �򥻥å� */
			   file_disk[0],
			   (image_disk[0] < 0) ? 0 : image_disk[0],
			   readonly_disk[0]);

	if (same) {				/* Ʊ��ե�����ξ��� */

	    if (err0 == FALSE) {			/* 1: �� 2: ž�� */
		err1 = disk_insert_A_to_B(DRIVE_1, DRIVE_2, 
					  (image_disk[1] < 0) ? 0
							      : image_disk[1]);
	    }

	} else {				/* �̥ե�����ξ��� */

	    err1 = disk_insert(DRIVE_2,			/* �ɥ饤��2 ���å� */
			       file_disk[1],
			       (image_disk[1] < 0) ? 0 : image_disk[1],
			       readonly_disk[1]);
	}

	/* ξ�ɥ饤�֤�Ʊ���ե����� ���� ���᡼�����꼫ư�ξ��ν��� */
	if (err0 == FALSE &&
	    err1 == FALSE &&
	    drive[DRIVE_1].fp == drive[DRIVE_2].fp && 
	    image_disk[0] < 0 && image_disk[1] < 0) {
	    disk_change_image(DRIVE_2, 1);		/* 2: �� ���᡼��2�� */
	}

    } else if (file_disk[0][0]) {/* �ɥ饤��1 ���� ���᡼������Ѥߤξ�� */
					/*	% quasi88 file		 */
					/*	% quasi88 file num       */
	err0 = disk_insert(DRIVE_1,
			   file_disk[0],
			   (image_disk[0] < 0) ? 0 : image_disk[0],
			   readonly_disk[0]);

	if (err0 == FALSE) {
	    if (image_disk[0] < 0 &&		/* ���᡼���ֹ����ʤ��ʤ� */
		disk_image_num(DRIVE_1) >= 2) {	/* �ɥ饤��2�ˤ⥻�å�      */

		err1 = disk_insert_A_to_B(DRIVE_1, DRIVE_2, 1);
		if (err1 == FALSE) {
		    memcpy(file_disk[1], file_disk[0], QUASI88_MAX_FILENAME);
		}
	    }
	}

    } else if (file_disk[1][0]) {/* �ɥ饤��2 ���� ���᡼������Ѥߤξ�� */
					/*	% quasi88 noexist file	 */
	err1 = disk_insert(DRIVE_2,
			   file_disk[1],
			   (image_disk[1] < 0) ? 0 : image_disk[1],
			   readonly_disk[1]);
    }



    /* �����ץ󤷤ʤ��ä�(����ʤ��ä�)���ϡ��ե�����̾�򥯥ꥢ */
    if (err0) memset(file_disk[ 0 ], 0, QUASI88_MAX_FILENAME);
    if (err1) memset(file_disk[ 1 ], 0, QUASI88_MAX_FILENAME);


    /* ����¾�Υ��᡼���ե�����⥻�å� */
    if (file_tape[CLOAD][0]) { err2 = sio_open_tapeload(file_tape[CLOAD]); }
    if (file_tape[CSAVE][0]) { err3 = sio_open_tapesave(file_tape[CSAVE]); }
    if (file_sin[0])         {        sio_open_serialin(file_sin);         }
    if (file_sout[0])        {        sio_open_serialout(file_sout);       }
    if (file_prn[0])         {        printer_open(file_prn);              }
	    /* �����ϡ����ơ��ȥ��ɤǤ�SEEK���Ƥʤ����ɤ����褦�� */


    /* �ե�����̾�ˤ��碌�ơ����ʥåץ���åȥե�����̾������ */
    if (filename_synchronize) {
	if (err0 == FALSE || err1 == FALSE /*|| err2 == FALSE*/) {
	    if (stateload == FALSE) {
		filename_init_state(TRUE);
	    }
	    filename_init_snap(TRUE);
	    filename_init_wav(TRUE);
	}
    }


    if (verbose_proc) {
	int i;
	for (i=0; i<2; i++) {
	    if (disk_image_exist(i)) {
		printf("DRIVE %d: <= %s [%d]\n", i+1,
		       file_disk[i], disk_image_selected(i)+1);
	    } else {
		printf("DRIVE %d: <= (empty)\n", i+1);
	    }
	}
    }
}

static	void	imagefile_all_close(void)
{
    disk_eject(0);          memset(file_disk[0],     0, QUASI88_MAX_FILENAME);
    disk_eject(1);          memset(file_disk[1],     0, QUASI88_MAX_FILENAME);

    sio_close_tapeload();   memset(file_tape[CLOAD], 0, QUASI88_MAX_FILENAME);
    sio_close_tapesave();   memset(file_tape[CSAVE], 0, QUASI88_MAX_FILENAME);
    sio_close_serialin();   memset(file_sin,         0, QUASI88_MAX_FILENAME);
    sio_close_serialout();  memset(file_sout,        0, QUASI88_MAX_FILENAME);
    printer_close();        memset(file_prn,         0, QUASI88_MAX_FILENAME);

#if 0	/* �Ĥ�����ϡ��ե�����̾Ʊ���Ϲ�θ���� �� ��? */
    if (filename_synchronize) {
	filename_init_state(TRUE);
	filename_init_snap(TRUE);
	filename_init_wav(TRUE);
    }
#endif
}




/***********************************************************************
 *
 *
 ************************************************************************/
const char	*filename_get_disk(int drv)
{
    if (file_disk[drv][0] != '\0') return file_disk[drv];
    else                           return NULL;
}
const char	*filename_get_tape(int mode)
{
    if (file_tape[mode][0] != '\0') return file_tape[mode];
    else                            return NULL;
}
const char	*filename_get_prn(void)
{
    if (file_prn[0] != '\0') return file_prn;
    else                     return NULL;
}
const char	*filename_get_sin(void)
{
    if (file_sin[0] != '\0') return file_sin;
    else                     return NULL;
}
const char	*filename_get_sout(void)
{
    if (file_sout[0] != '\0') return file_sout;
    else                      return NULL;
}

/* ����ɥ饤�֤ʤ���ȿ�Хɥ饤�֤˥ǥ�����������С����Υե�����̾��
   �ʤ���С��ǥ������ѥǥ��쥯�ȥ���֤� */
const char	*filename_get_disk_or_dir(int drv)
{
    const char *p;

    if      (file_disk[drv  ][0] != '\0') p = file_disk[drv  ];
    else if (file_disk[drv^1][0] != '\0') p = file_disk[drv^1];
    else {
	p = osd_dir_disk();
	if (p == NULL) p = osd_dir_cwd();
    }

    return p;
}
/* ���ꤵ�줿��ʬ�Υơ��פ����åȤ���Ƥ���С����Υե�����̾��
   �ʤ���С��ơ����ѥǥ��쥯�ȥ���֤� */
const char	*filename_get_tape_or_dir(int mode)
{
    const char *p;

    if (file_tape[ mode ][0] != '\0') p = file_tape[ mode ];
    else {
	p = osd_dir_tape();
	if (p == NULL) p = osd_dir_cwd();
    }

    return p;
}

const char	*filename_get_disk_name(int drv)
{
           char  dir[ OSD_MAX_FILENAME ];
    static char file[ OSD_MAX_FILENAME ];

    if (file_disk[drv][0]) {
	if (osd_path_split(file_disk[drv], dir, file, OSD_MAX_FILENAME)) {
	    return file;
	}
    }
    return NULL;
}
const char	*filename_get_tape_name(int mode)
{
           char  dir[ OSD_MAX_FILENAME ];
    static char file[ OSD_MAX_FILENAME ];

    if (file_tape[mode][0]) {
	if (osd_path_split(file_tape[mode], dir, file, OSD_MAX_FILENAME)) {
	    return file;
	}
    }
    return NULL;
}













/***********************************************************************
 * ���ơ��ȥե����롢���ʥåץ���åȥե�����Υե�����̾�ˡ�
 * ���ʸ����򥻥åȤ��롣
 *
 *	���� set_default
 *		���ʤ顢����ͤ򥻥åȤ��롣
 *		���ʤ顢���᡼���ե�����̾�˱������ͤ򥻥åȤ��롣
 ************************************************************************/

void	filename_init_state(int synchronize)
{
    char *s, *buf;
    const char *dir;

    dir = osd_dir_state();
    if (dir == NULL) dir = osd_dir_cwd();

    memset(file_state, 0, QUASI88_MAX_FILENAME);

    if (synchronize) {
	if      (file_disk[0][0]     != '\0') s = file_disk[0];
	else if (file_disk[1][0]     != '\0') s = file_disk[1];
/*	else if (file_tape[CLOAD][0] != '\0') s = file_tape[CLOAD];*/
/*	else if (file_tape[CSAVE][0] != '\0') s = file_tape[CSAVE];*/
	else                                  s = STATE_FILENAME;
    } else {
	s = STATE_FILENAME;
    }

    buf = assemble_filename(s, dir, STATE_SUFFIX);

    if (buf) {
	if (strlen(buf) < QUASI88_MAX_FILENAME) {
	    strcpy(file_state, buf);
	    return;
	}
    }

    /* ����ǥ��顼�ˤʤä��顢Ŭ�����ͤ򥻥å� */
    strcpy(file_state, STATE_FILENAME STATE_SUFFIX);
}



void	filename_init_snap(int synchronize)
{
    char *s, *buf;
    const char *dir;

    dir = osd_dir_snap();
    if (dir == NULL) dir = osd_dir_cwd();

    memset(file_snap, 0, QUASI88_MAX_FILENAME);

    if (synchronize) {
	if      (file_disk[0][0]     != '\0') s = file_disk[0];
	else if (file_disk[1][0]     != '\0') s = file_disk[1];
/*	else if (file_tape[CLOAD][0] != '\0') s = file_tape[CLOAD];*/
/*	else if (file_tape[CSAVE][0] != '\0') s = file_tape[CSAVE];*/
	else                                  s = SNAPSHOT_FILENAME;
    } else {
	s = SNAPSHOT_FILENAME;
    }

    buf = assemble_filename(s, dir, "");

    if (buf) {
	if (strlen(buf) < QUASI88_MAX_FILENAME) {
	    strcpy(file_snap, buf);
	    return;
	}
    }

    /* ����ǥ��顼�ˤʤä��顢Ŭ�����ͤ򥻥å� */
    strcpy(file_snap, SNAPSHOT_FILENAME);
}



void	filename_init_wav(int synchronize)
{
    char *s, *buf;
    const char *dir;

    dir = osd_dir_snap();
    if (dir == NULL) dir = osd_dir_cwd();

    memset(file_wav, 0, QUASI88_MAX_FILENAME);

    if (synchronize) {
	if      (file_disk[0][0]     != '\0') s = file_disk[0];
	else if (file_disk[1][0]     != '\0') s = file_disk[1];
/*	else if (file_tape[CLOAD][0] != '\0') s = file_tape[CLOAD];*/
/*	else if (file_tape[CSAVE][0] != '\0') s = file_tape[CSAVE];*/
	else                                  s = WAVEOUT_FILENAME;
    } else {
	s = WAVEOUT_FILENAME;
    }

    buf = assemble_filename(s, dir, "");

    if (buf) {
	if (strlen(buf) < QUASI88_MAX_FILENAME) {
	    strcpy(file_wav, buf);
	    return;
	}
    }

    /* ����ǥ��顼�ˤʤä��顢Ŭ�����ͤ򥻥å� */
    strcpy(file_wav, WAVEOUT_FILENAME);
}




/***********************************************************************
 * �Ƽ�ե�����Υե�ѥ������
 *	�����ν����ʵ������¸�ܰ�¸�ˤ���ӡ�
 *	�Ƽ�����ν��� (�����¸��) ����ƤӽФ���롦�����ϥ�
 *
 *		����� �ǥ��������᡼��̾�Ρ��ե�ѥ������
 *		����� ROM���᡼��̾�Ρ�     �ե�ѥ������
 *		����� ��������ե�����̾�Ρ��ե�ѥ������
 *		����� ��������ե�����̾�Ρ��ե�ѥ������
 *
 *	�������ϡ� char * (malloc���줿�ΰ�)�����Ի��� NULL
 ************************************************************************/

/*
 * ���륤�᡼���Υե�����̾ imagename �Υǥ��쥯�ȥ����ȳ�ĥ������
 * ���������١���̾����Ф���
 * basedir �� �١���̾ �� suffix ���礷���ե�����̾���֤���
 *
 *	��)
 *	imagename �� /my/disk/dir/GAMEDISK.d88
 *                                ^^^^^^^^	������Ф��ơ�
 *	basedir   ��     /new/dir		�����
 *	suffix    ��                      .dat	����򤯤äĤ���
 *	�֤���    ��     /new/dir/GAMEDISK.dat	������֤�
 *
 * �����֤äƤ����ΰ�ϡ���Ū���ΰ�ʤΤ���� !
 */
static char *assemble_filename(const char *imagename,
			       const char *basedir,
			       const char *suffix)
{
    static char buf[ OSD_MAX_FILENAME ];
           char file[ OSD_MAX_FILENAME ];

    if (osd_path_split(imagename, buf, file, OSD_MAX_FILENAME)) {

	size_t len = strlen(file);

	if (len >= 4) {
	    if (strcmp(&file[ len-4 ], ".d88") == 0 ||
		strcmp(&file[ len-4 ], ".D88") == 0 ||
		strcmp(&file[ len-4 ], ".t88") == 0 ||
		strcmp(&file[ len-4 ], ".T88") == 0 ||
		strcmp(&file[ len-4 ], ".cmt") == 0 ||
		strcmp(&file[ len-4 ], ".CMT") == 0) {

		file[ len-4 ] = '\0';
	    }
	}

	if (strlen(file) + strlen(suffix) + 1 < OSD_MAX_FILENAME) {

	    strcat(file, suffix);

	    if (osd_path_join(basedir, file, buf, OSD_MAX_FILENAME)) {
		return buf;
	    }
	}
    }

    return NULL;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/*
 * �ǥ��������᡼���ե�����Υե�����̾���������
 *	���� filename ��Ϳ����줿�ǥ��������᡼���ե�����̾��
 *	�䴰���ơ��ǽ�Ū�ʥե�����̾�֤������Υե�����̾�� malloc ���줿
 *	�ΰ�ʤΤǡ�ɬ�פ˱����ƸƤӽФ����� free ���뤳�ȡ�
 */
char	*filename_alloc_diskname(const char *filename)
{
    char *p;
    char dir [ OSD_MAX_FILENAME ];
    char file[ OSD_MAX_FILENAME ];
    const char *base;
    OSD_FILE *fp;
    int step;

		/* filename �� dir �� file ��ʬ���� */

    if (osd_path_split(filename, dir, file, OSD_MAX_FILENAME)) {

	if (dir[0] == '\0') {
		/* dir �������Ĥޤ� filename �˥ѥ��ζ��ڤ꤬�ޤޤ�ʤ� */

	    step = 0;	/* dir_disk + filename �� �ե�����̵ͭ��Ƚ��	*/

	} else {
		/* filename �˥ѥ����ڤ꤬�ޤޤ��			*/

	    step = 1;	/* dir_cwd + filename �� �ե�����̵ͭ������å�	*/
			/*	(filename�����Хѥ��ʤ顢 filename	*/
			/*	 ���Τ�Τǥե�����̵ͭ�����å��Ȥʤ�)	*/
	}

    } else {
	return NULL;
    }


		/* step 0 �� step 1 �ν�ˡ��ե�����̵ͭ�����å� */

    for ( ; step < 2; step ++) {

	if (step == 0) base = osd_dir_disk();
	else           base = osd_dir_cwd();

	if (base == NULL) continue;

	if (osd_path_join(base, filename, file, OSD_MAX_FILENAME) == FALSE) {
	    return NULL;
	}

			/* �ºݤ� open �Ǥ��뤫������å����� */
	fp = osd_fopen(FTYPE_DISK, file, "rb");
	if (fp) {
	    osd_fclose(fp);

	    p = (char *)malloc(strlen(file) + 1);
	    if (p) {
		strcpy(p, file);
		return p;
	    }
	    break;
	}
    }

    return NULL;
}





char	*filename_alloc_romname(const char *filename)
{
  char *p;
  char buf[ OSD_MAX_FILENAME ];
  OSD_FILE *fp;
  int step;
  const char *dir = osd_dir_rom(); 

	/* step 0 �� filename�����뤫�����å�			*/
	/* step 1 �� dir_rom �ˡ� filename �����뤫�����å�	*/

  for( step=0; step<2; step++ ){

    if( step==0 ){

      if( OSD_MAX_FILENAME <= strlen(filename) ) return NULL;
      strcpy( buf, filename );

    }else{

      if( dir == NULL ||
	  osd_path_join( dir, filename, buf, OSD_MAX_FILENAME ) == FALSE ){

	return NULL;
      }
    }

		/* �ºݤ� open �Ǥ��뤫������å����� */
    fp = osd_fopen( FTYPE_ROM, buf, "rb" );
    if( fp ){
      osd_fclose( fp );

      p = (char *)malloc( strlen(buf) + 1 );
      if( p ){
	strcpy( p, buf );
	return p;
      }
      break;
    }
  }
  return NULL;
}





char	*filename_alloc_global_cfgname(void)
{
    const char *dir  = osd_dir_gcfg();
    const char *file = CONFIG_FILENAME  CONFIG_SUFFIX;
    char *p;
    char buf[ OSD_MAX_FILENAME ];

    if (dir == NULL ||
	osd_path_join(dir, file, buf, OSD_MAX_FILENAME) == FALSE) {

	return NULL;
    }

    p = (char *)malloc(strlen(buf) + 1);
    if (p) {
	strcpy(p, buf);
    }
    return p;
}

char	*filename_alloc_local_cfgname(const char *imagename)
{
    char *p   = NULL;
    char *buf;
    const char *dir = osd_dir_lcfg();

    if (dir == NULL) return NULL;

    buf = assemble_filename(imagename, dir, CONFIG_SUFFIX);

    if (buf) {
	p = (char *)malloc(strlen(buf) + 1);
	if (p) {
	    strcpy(p, buf);
	}
    }
    return p;
}




char	*filename_alloc_keyboard_cfgname(void)
{
  const char *dir  = osd_dir_gcfg();
  const char *file = KEYCONF_FILENAME  CONFIG_SUFFIX;
  char *p;
  char buf[ OSD_MAX_FILENAME ];


  if( dir == NULL ||
      osd_path_join( dir, file, buf, OSD_MAX_FILENAME ) == FALSE )

    return NULL;

  p = (char *)malloc( strlen(buf) + 1 );
  if( p ){
    strcpy( p, buf );
    return p;
  }

  return NULL;
}
