/*****************************************************************************/
/* �ե��������˴ؤ������						     */
/*									     */
/*	���ͤξܺ٤ϡ��إå��ե����� file-op.h ����			     */
/*									     */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "file-op.h"


/*****************************************************************************/

static char *dir_cwd;	/* �ǥե���ȤΥǥ��쥯�ȥ� (������)		*/
static char *dir_rom;	/* ROM���᡼���ե�����θ����ǥ��쥯�ȥ�	*/
static char *dir_disk;	/* DISK���᡼���ե�����θ����ǥ��쥯�ȥ�	*/
static char *dir_tape;	/* TAPE���᡼���ե�����δ��ǥ��쥯�ȥ�	*/
static char *dir_snap;	/* ���̥��ʥåץ���åȥե��������¸��		*/
static char *dir_state;	/* �����ڥ�ɥե��������¸��			*/
static char *dir_g_cfg;	/* ��������ե�����Υǥ��쥯�ȥ�		*/
static char *dir_l_cfg;	/* ��������ե�����Υǥ��쥯�ȥ�		*/



/****************************************************************************
 * �Ƽ�ǥ��쥯�ȥ�μ���������
 *****************************************************************************/
const char *osd_dir_cwd  (void) { return dir_cwd;   }
const char *osd_dir_rom  (void) { return dir_rom;   }
const char *osd_dir_disk (void) { return dir_disk;  }
const char *osd_dir_tape (void) { return dir_tape;  }
const char *osd_dir_snap (void) { return dir_snap;  }
const char *osd_dir_state(void) { return dir_state; }
const char *osd_dir_gcfg (void) { return dir_g_cfg; }
const char *osd_dir_lcfg (void) { return dir_l_cfg; }

static int set_new_dir(const char *newdir, char **dir)
{
    char *p = malloc(strlen(newdir) + 1);
    if (p) {
	strcpy(p, newdir);
	free(*dir);
	*dir = p;
	return TRUE;
    }
    return FALSE;
}

int osd_set_dir_cwd  (const char *d) { return set_new_dir(d, &dir_cwd);   }
int osd_set_dir_rom  (const char *d) { return set_new_dir(d, &dir_rom);   }
int osd_set_dir_disk (const char *d) { return set_new_dir(d, &dir_disk);  }
int osd_set_dir_tape (const char *d) { return set_new_dir(d, &dir_tape);  }
int osd_set_dir_snap (const char *d) { return set_new_dir(d, &dir_snap);  }
int osd_set_dir_state(const char *d) { return set_new_dir(d, &dir_state); }
int osd_set_dir_gcfg (const char *d) { return set_new_dir(d, &dir_g_cfg); }
int osd_set_dir_lcfg (const char *d) { return set_new_dir(d, &dir_l_cfg); }







/****************************************************************************
 * �ե�����̾�˻��Ѥ���Ƥ�����������ɤ����
 *		0 �� ASCII �Τ�
 *		1 �� ���ܸ�EUC
 *		2 �� ���ե�JIS
 *****************************************************************************/
int	osd_kanji_code(void)
{
    return 0;	/* ASCII�Τ� */
}



/****************************************************************************
 * �ե��������
 *
 * OSD_FILE *osd_fopen(int type, const char *path, const char *mode)
 * int	osd_fclose(OSD_FILE *stream)
 * int	osd_fflush(OSD_FILE *stream)
 * int	osd_fseek(OSD_FILE *stream, long offset, int whence)
 * long	osd_ftell(OSD_FILE *stream)
 * void	osd_rewind(OSD_FILE *stream)
 * size_t osd_fread(void *ptr, size_t size, size_t nobj, OSD_FILE *stream)
 * size_t osd_fwrite(const void *ptr,size_t size,size_t nobj,OSD_FILE *stream)
 * int	osd_fputc(int c, OSD_FILE *stream)
 * int	osd_fgetc(OSD_FILE *stream)
 * char	*osd_fgets(char *str, int size, OSD_FILE *stream)
 * int	osd_fputs(const char *str, OSD_FILE *stream)
 *****************************************************************************/


/*
 * ���ƤΥե�������Ф�����¾���椷���ۤ��������Ȼפ����ɡ����ݤʤΤǡ�
 * �ǥ��������ơ��פΥ��᡼���˴ؤ��ƤΤߡ�¿�Ť˥����ץ󤷤ʤ��褦�ˤ��롣
 * �������¸����ˡ�Ϥʤ��Τǡ������Ǥϥե�����̾�Ƕ��̤��뤳�Ȥˤ��롣(��?)
 *
 * osd_fopen ���ƤӽФ��줿�Ȥ��ˡ��ե�����̾���ݻ����Ƥ�����
 * ���Ǥ˳����Ƥ���ե�����Υե�����̾�Ȱ��פ��ʤ���������å����롣
 * �����ǡ��ǥ��������᡼���ե�����ξ��ϡ����Ǥ˳����Ƥ���ե������
 * �ե�����ݥ��󥿤��֤���¾�ξ��ϥ����ץ��ԤȤ��� NULL ���֤���
 */

struct OSD_FILE_STRUCT {

    FILE	*fp;			/* !=NULL �ʤ������	*/
    int		type;			/* �ե��������		*/
    char	*path;			/* �ե�����̾		*/
    char	mode[4];		/* �������ݤΡ��⡼��	*/

};

#define	MAX_STREAM	8
static	OSD_FILE	osd_stream[ MAX_STREAM ];



OSD_FILE *osd_fopen(int type, const char *path, const char *mode)
{
    int i;
    OSD_FILE	*st;

    st = NULL;
    for (i=0; i<MAX_STREAM; i++) {	/* �����Хåե���õ�� */
	if (osd_stream[i].fp == NULL) {		/* fp �� NULL �ʤ���� */
	    st = &osd_stream[i];
	    break;
	}
    }
    if (st == NULL) return NULL;		/* �������ʤ���� NG */
    st->path = NULL;



    switch (type) {

    case FTYPE_DISK:		/* "r+b" , "rb"	*/
    case FTYPE_TAPE_LOAD:	/* "rb" 	*/
    case FTYPE_TAPE_SAVE:	/* "ab"		*/
    case FTYPE_PRN:		/* "ab"		*/
    case FTYPE_COM_LOAD:	/* "rb"		*/
    case FTYPE_COM_SAVE:	/* "ab"		*/

	/* ���Ǥ˳����Ƥ���ե����뤫�ɤ���������å����� */
	for (i=0; i<MAX_STREAM; i++) {
	    if (osd_stream[i].fp) {
		if (osd_stream[i].path   &&
		    strcmp(osd_stream[i].path, path) == 0) {

		    /* DISK�ξ�礫��Ʊ���⡼�ɤʤ�Ф�����֤� */
		    if (type == FTYPE_DISK                   &&
			osd_stream[i].type == type           &&
			strcmp(osd_stream[i].mode, mode) == 0) {

			return &osd_stream[i];

		    } else {
			/* DISK�ʳ����ʤ����⡼�ɤ��㤦�ʤ��NG */
			return NULL;
		    }
		}
	    }
	}
					/* �ե�����̾�ݻ��ѤΥХåե������ */
	st->path = malloc(strlen(path) + 1);
	if (st->path == NULL) {
	    return NULL;
	}
	/* FALLTHROUGH */


    default:
	st->fp = fopen(path, mode);	/* �ե�����򳫤� */

	if (st->fp) {

	    st->type = type;
	    if (st->path)
		strcpy(st->path, path);
	    strncpy(st->mode, mode, sizeof(st->mode));
	    return st;

	} else {

	    if (st->path) {
		free(st->path);
		st->path = NULL;
	    }
	    return NULL;
	}
    }
}



int	osd_fclose(OSD_FILE *stream)
{
    if (stream->fp) {

	FILE *fp = stream->fp;

	stream->fp = NULL;
	if (stream->path) {
	    free(stream->path);
	    stream->path = NULL;
	}

	return fclose(fp);

    }
    return EOF;
}



int	osd_fflush(OSD_FILE *stream)
{
    if (stream == NULL) return fflush(NULL);

    if (stream->fp) {
	return fflush(stream->fp);
    }
    return EOF;
}



int	osd_fseek(OSD_FILE *stream, long offset, int whence)
{
    if (stream->fp) {
	return fseek(stream->fp, offset, whence);
    }
    return -1;
}



long	osd_ftell(OSD_FILE *stream)
{
    if (stream->fp) {
	return ftell(stream->fp);
    }
    return -1;
}



void	osd_rewind(OSD_FILE *stream)
{
    (void)osd_fseek(stream, 0L, SEEK_SET);
    osd_fflush(stream);
}



size_t	osd_fread(void *ptr, size_t size, size_t nobj, OSD_FILE *stream)
{
    if (stream->fp) {
	return fread(ptr, size, nobj, stream->fp);
    }
    return 0;
}



size_t	osd_fwrite(const void *ptr, size_t size, size_t nobj, OSD_FILE *stream)
{
    if (stream->fp) {
	return fwrite(ptr, size, nobj, stream->fp);
    }
    return 0;
}



int	osd_fputc(int c, OSD_FILE *stream)
{
    if (stream->fp) {
	return fputc(c, stream->fp);
    }
    return EOF;
}


int	osd_fgetc(OSD_FILE *stream)
{
    if (stream->fp) {
	return fgetc(stream->fp);
    }
    return EOF;
}


char	*osd_fgets(char *str, int size, OSD_FILE *stream)
{
    if (stream->fp) {
	return fgets(str, size, stream->fp);
    }
    return NULL;
}


int	osd_fputs(const char *str, OSD_FILE *stream)
{
    if (stream->fp) {
	return fputs(str, stream->fp);
    }
    return EOF;
}



/****************************************************************************
 * �ǥ��쥯�ȥ����
 *****************************************************************************/

/*---------------------------------------------------------------------------
 * T_DIR_INFO *osd_opendir(const char *filename)
 *	�ǥ��쥯�ȥ�����ϵ����¸�ʤΤǡ������Ǥ� NULL (���顼) ���֤���
 *---------------------------------------------------------------------------*/
T_DIR_INFO	*osd_opendir(const char *filename)
{
    return NULL;
}



/*---------------------------------------------------------------------------
 * T_DIR_ENTRY *osd_readdir(T_DIR_INFO *dirp)
 *	�ǥ��쥯�ȥ�����ϵ����¸�ʤΤǡ������Ǥ� NULL (���顼) ���֤���
 *---------------------------------------------------------------------------*/
T_DIR_ENTRY	*osd_readdir(T_DIR_INFO *dirp)
{
    return NULL;
}


/*---------------------------------------------------------------------------
 * void osd_closedir(T_DIR_INFO *dirp)
 *	�ǥ��쥯�ȥ�����ϵ����¸�ʤΤǡ����δؿ��ϥ��ߡ�
 *---------------------------------------------------------------------------*/
void		osd_closedir(T_DIR_INFO *dirp)
{
}



/****************************************************************************
 * �ѥ�̾�����
 *****************************************************************************/

/*---------------------------------------------------------------------------
 * int	osd_path_normalize(const char *path, char resolved_path[], int size)
 *	�ե�����̾��ѥ��ˤĤ��Ƥν����ϵ����¸�ʤΤǡ�
 *	path �򤽤Τޤ� resolved_path �˥��åȤ����֤�
 *---------------------------------------------------------------------------*/
int	osd_path_normalize(const char *path, char resolved_path[], int size)
{
    if (strlen(path) < size) {
	strcpy(resolved_path, path);
	return TRUE;
    }

    return FALSE;
}



/*---------------------------------------------------------------------------
 * int	osd_path_split(const char *path, char dir[], char file[], int size)
 *	�ե�����̾��ѥ��ˤĤ��Ƥν����ϵ����¸�ʤΤǡ�
 *	path �򤽤Τޤ� file �˥��åȤ����֤�
 *---------------------------------------------------------------------------*/
int	osd_path_split(const char *path, char dir[], char file[], int size)
{
    if (strlen(path) < size) {
	dir[0] = '\0';
	strcpy(file, path);
	return TRUE;
    }

    return FALSE;
}



/*---------------------------------------------------------------------------
 * int	osd_path_join(const char *dir, const char *file, char path[], int size)
 *	�ե�����̾��ѥ��ˤĤ��Ƥν����ϵ����¸�ʤΤǡ�
 *	file �򤽤Τޤ� path �˥��åȤ����֤�
 *---------------------------------------------------------------------------*/
int	osd_path_join(const char *dir, const char *file, char path[], int size)
{
    if (strlen(file) < size) {
	strcpy(path, file);
	return TRUE;
    }

    return FALSE;
}



/****************************************************************************
 * �ե�����°���μ���
 ****************************************************************************/

int	osd_file_stat(const char *pathname)
{
    FILE *fp;

    if ((fp = fopen(pathname, "r"))) {	/* �ե�����Ȥ��Ƴ���	*/

	fclose(fp);				/* ����������ե�����	*/
	return FILE_STAT_FILE;

    } else {				/* ���Ԥ�����¸�ߤ��ʤ�	*/

	return FILE_STAT_NOEXIST;

	/* �ǥ��쥯�ȥ꤫�⤷��ʤ����������������Ĥ�̵���Τ����Τ�ʤ� */
	/* �Ǥ⡢�����ϵ����¸�ʤΤǤȤꤢ���� '¸�ߤ��ʤ�' ���֤���	*/

	/* �ʤ���'¸�ߤ��ʤ�' ���֤��ȡ����θ�˥ե�����򿷵��˺���	*/
	/* ���褦�Ȥ��뤫�⤷��ʤ���(fopen(pathname, "w") �ʤɤ�)	*/
	/* �Ǥ⡢�ǥ��쥯�ȥ꤬¸�ߤ����ꡢ�����������Ĥ�̵���ե����뤬	*/
	/* ¸�ߤ�����ϡ����������˼��Ԥ���Τ�����ס�����͡�	*/
    }
}






/****************************************************************************
 * int	osd_environment(void)
 *
 *	���δؿ��ϡ���ư���1�٤����ƤӽФ���롣
 *	���ｪλ���Ͽ��� malloc �˼��Ԥ����ʤɰ۾ｪλ���ϵ����֤���
 *
 ****************************************************************************/

int	osd_file_config_init(void)
{
	/* ��������*/

    dir_cwd   = (char *)malloc(1);
    dir_rom   = (char *)malloc(1);
    dir_disk  = (char *)malloc(1);
    dir_tape  = (char *)malloc(1);
    dir_snap  = (char *)malloc(1);
    dir_state = (char *)malloc(1);
    dir_g_cfg = (char *)malloc(1);
    dir_l_cfg = (char *)malloc(1);


    if (! dir_cwd  || ! dir_rom   || ! dir_disk  || ! dir_tape ||
	! dir_snap || ! dir_state || ! dir_g_cfg || ! dir_l_cfg)  return FALSE;


	/* �����ȥ���󥰥ǥ��쥯�ȥ�̾ (CWD) �����ꤹ�� */

    dir_cwd[0] = '\0';

	/* ROM�ǥ��쥯�ȥ�����ꤹ�� */

    dir_rom[0] = '\0';

	/* DISK�ǥ��쥯�ȥ�����ꤹ�� */

    dir_disk[0] = '\0';

	/* TAPE�ǥ��쥯�ȥ�����ꤹ�� */

    dir_tape[0] = '\0';

	/* SNAP�ǥ��쥯�ȥ�����ꤹ�� */

    dir_snap[0] = '\0';

	/* STATE�ǥ��쥯�ȥ�����ꤹ�� */

    dir_state[0] = '\0';

	/* ��������ǥ��쥯�ȥ�����ꤹ�� */

    dir_g_cfg[0] = '\0';

	/* ��������ǥ��쥯�ȥ�����ꤹ�� */

    dir_l_cfg[0] = '\0';



    return TRUE;
}


/****************************************************************************
 * int	osd_file_config_exit(void)
 *
 *	���δؿ��ϡ���λ���1�٤����ƤӽФ���롣
 *
 ****************************************************************************/
void	osd_file_config_exit(void)
{
    if (dir_cwd)   free(dir_cwd);
    if (dir_rom)   free(dir_rom);
    if (dir_disk)  free(dir_disk);
    if (dir_tape)  free(dir_tape);
    if (dir_snap)  free(dir_snap);
    if (dir_state) free(dir_state);
    if (dir_g_cfg) free(dir_g_cfg);
    if (dir_l_cfg) free(dir_l_cfg);
}
