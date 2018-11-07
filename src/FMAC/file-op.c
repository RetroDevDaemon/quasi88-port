/*****************************************************************************/
/* �ե��������˴ؤ������						     */
/*									     */
/*	���ͤξܺ٤ϡ��إå��ե����� file-op.h ����			     */
/*									     */
/*****************************************************************************/

/*----------------------------------------------------------------------*/
/* �ǥ��쥯�ȥ�ΰ����ʤɡ� mac ��ͭ��API��Ȥä������ϡ� 		*/
/* apaslothy �����Υ����ɤ�Ȥ碌�Ƥ�餤�ޤ�����			*/
/*							 (c) apaslothy	*/
/*----------------------------------------------------------------------*/


/* �Ǥ⡢���ޤ����褯�狼�äƤʤ�������					*/
/* �Ȥꤢ�������ʲ������¤Ǽ������Ƥߤ롣				*/
/*									*/
/*	�ѥ��ζ��ڤ�� ':'						*/
/*	: ��ʣ���Ҥ������Хѥ�ɽ���϶ػߡ�ʣ���� : ��ñ��� : �Ȥߤʤ�	*/
/*	":"  ��1ʸ���ϡ��롼�ȥǥ��쥯�ȥ�Ȥߤʤ���			*/
/*	"::" ��2ʸ���ϡ��ƥǥ��쥯�ȥ�Ȥߤʤ�				*/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <MacTypes.h>
#include <Files.h>

#include "quasi88.h"
#include "initval.h"
#include "file-op.h"


/*****************************************************************************/

static char dir_cwd[256];	/* �ǥե���ȤΥǥ��쥯�ȥ� (������)	  */
static char dir_rom[256];	/* ROM���᡼���ե�����θ����ǥ��쥯�ȥ�  */
static char dir_disk[256];	/* DISK���᡼���ե�����θ����ǥ��쥯�ȥ� */
static char dir_tape[256];	/* TAPE���᡼���ե�����δ��ǥ��쥯�ȥ� */
static char dir_snap[256];	/* ���̥��ʥåץ���åȥե��������¸��	  */
static char dir_state[256];	/* �����ڥ�ɥե��������¸��		  */
static char dir_g_cfg[256];	/* ��������ե�����Υǥ��쥯�ȥ�	  */
static char dir_l_cfg[256];	/* ��������ե�����Υǥ��쥯�ȥ�	  */

/*------------------------------------------------------------------------*/

/* �ܥ�塼���ֹ�ȥǥ��쥯�ȥ�ID����ե�ѥ���������� */
static OSErr GetFullPath(short vRefNum, long dirID, UInt8 *pathname);

/* �ե�ѥ�����ܥ�塼���ֹ�ȥǥ��쥯�ȥ�ID��������� */
static OSErr PathToSpec(const char *pathname, short *vRefNum, long *dirID,
			Boolean *is_dir);


/****************************************************************************
 * �Ƽ�ǥ��쥯�ȥ�μ���	(osd_dir_cwd �� NULL���֤��ƤϤ��� !)
 *****************************************************************************/
const char *osd_dir_cwd  (void) { return dir_cwd;   }
const char *osd_dir_rom  (void) { return dir_rom;   }
const char *osd_dir_disk (void) { return dir_disk;  }
const char *osd_dir_tape (void) { return dir_tape;  }
const char *osd_dir_snap (void) { return dir_snap;  }
const char *osd_dir_state(void) { return dir_state; }
const char *osd_dir_gcfg (void) { return dir_g_cfg; }
const char *osd_dir_lcfg (void) { return dir_l_cfg; }

static int set_new_dir(const char *newdir, char *dir)
{
    if (strlen(newdir) < 256) {
	strcpy(dir, newdir);
	return TRUE;
    }
    return FALSE;
}

int osd_set_dir_cwd  (const char *d) { return set_new_dir(d, dir_cwd);   }
int osd_set_dir_rom  (const char *d) { return set_new_dir(d, dir_rom);   }
int osd_set_dir_disk (const char *d) { return set_new_dir(d, dir_disk);  }
int osd_set_dir_tape (const char *d) { return set_new_dir(d, dir_tape);  }
int osd_set_dir_snap (const char *d) { return set_new_dir(d, dir_snap);  }
int osd_set_dir_state(const char *d) { return set_new_dir(d, dir_state); }
int osd_set_dir_gcfg (const char *d) { return set_new_dir(d, dir_g_cfg); }
int osd_set_dir_lcfg (const char *d) { return set_new_dir(d, dir_l_cfg); }







/****************************************************************************
 * �ե�����̾�˻��Ѥ���Ƥ�����������ɤ����
 *		0 �� ASCII �Τ�
 *		1 �� ���ܸ�EUC
 *		2 �� ���ե�JIS
 *****************************************************************************/
int	osd_kanji_code(void)
{
    return 2;
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
 * mac �ǤϤɤ�����Ф����Τ��Τ�ʤ��Τǡ��ե�����̾�Ƕ��̤��뤳�Ȥˤ��褦��
 *
 * osd_fopen ���ƤӽФ��줿�Ȥ��ˡ��ե�����̾���ݻ����Ƥ�����
 * ���Ǥ˳����Ƥ���ե�����Υե�����̾�Ȱ��פ��ʤ���������å����롣
 * �����ǡ��ǥ��������᡼���ե�����ξ��ϡ����Ǥ˳����Ƥ���ե������
 * �ե�����ݥ��󥿤��֤���¾�ξ��ϥ����ץ��ԤȤ��� NULL ���֤���
 */


/*
 * �ե�����̾ f1 �� f2 ��Ʊ���ե�����Ǥ���п����֤�
 *
 *	�Ȥꤢ�������ե�����̾��������ӡ���ʸ����ʸ���ΰ㤤��̵�롣
 *	(�Ĥޤꡢ�����ꥢ������Ʊ���ե�����Ǥ��԰��פˤʤäƤ��ޤ��ʡ�����)
 */

static int file_cmp(const char *f1, const char *f2)
{
    int is_sjis = FALSE;
    int c;

    if (f1 == NULL || f2 == NULL) return FALSE;
    if (f1 == f2) return TRUE;

    while ((c = (int)*f1)) {

	if (is_sjis) {				/* ���ե�JIS��2�Х�����	*/
	    if (*f1 != *f2) return FALSE;
	    is_sjis = FALSE;
	}
	else if ((c >= 0x81 && c <= 0x9f) ||	/* ���ե�JIS��1�Х����� */
		 (c >= 0xe0 && c <= 0xfc)) {
	    if (*f1 != *f2) return FALSE;
	    is_sjis = TRUE;
	}
	else {					/* �ѿ���Ⱦ�ѥ���ʸ��	*/
	    if (my_strcmp(f1, f2) != 0) return FALSE;
	}

	f1 ++;
	f2 ++;
    }

    if (*f2 == '\0') return TRUE;
    else             return FALSE;
}







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
		if (file_cmp(osd_stream[i].path, path)) {

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

struct	T_DIR_INFO_STRUCT
{
    int		cur_entry;		/* ��̤�������������ȥ��	*/
    int		nr_entry;		/* ����ȥ������		*/
    T_DIR_ENTRY	*entry;			/* ����ȥ���� (entry[0]��)	*/
};


/*---------------------------------------------------------------------------
 * �ȥåץ�٥� (�ܥ�塼��ΰ���)
 *---------------------------------------------------------------------------*/
static	T_DIR_INFO	*openroot(void)
{
    char  *p;
    short i;
    OSErr err;
    int   num;
    T_DIR_INFO	*dir;
    Str63	temp;
    HParamBlockRec pbr;

				/* T_DIR_INFO ����� 1�ĳ��� */
    if ((dir = (T_DIR_INFO *)malloc(sizeof(T_DIR_INFO))) == NULL) {
	return NULL;
    }

    /* ���ܤο���������� */
    dir->nr_entry = 0;
    i = 1;
    pbr.volumeParam.ioNamePtr = temp;
    do {
	pbr.volumeParam.ioVolIndex = i;
	err = PBHGetVInfoSync(&pbr);
	if (err == noErr) {
	    dir->nr_entry += 1;
	}
	i++;
    } while (err == noErr);

    if (dir->nr_entry == 0) {
	free(dir);
	return NULL;
    }
				/* T_DIR_ENTRY ����� ���ܿ�ʬ ���� */
    dir->entry = (T_DIR_ENTRY *)malloc(dir->nr_entry * sizeof(T_DIR_ENTRY));
    if (dir->entry == NULL) {
	free(dir);
	return NULL;
    }
    for (i=0; i<dir->nr_entry; i++) {
	dir->entry[i].name = NULL;
      /*dir->entry[i].str  = NULL;*/
    }


    dir->cur_entry = 0;

    /* �ե������ι��ܤ��ɲ� */
    i = 1;
    num = 0;
    pbr.volumeParam.ioNamePtr = temp;
    do {
	pbr.volumeParam.ioVolIndex = i;
	err = PBHGetVInfoSync(&pbr);
	if (err == noErr) {
	    temp[temp[0] + 1] = 0;

	    p = (char *)malloc( (temp[0] + 1) + (temp[0] + 1) );
	    if (p == NULL) { /* ���ե�����̾ �� ��ɽ��̾ �ΥХåե������ */
		dir->nr_entry = num;
		break;				/* malloc �˼��Ԥ��������� */
	    }

	    dir->entry[num].name = &p[0];
	    dir->entry[num].str  = &p[(temp[0] + 1)];

	    strcpy(dir->entry[num].name, (char*)&temp[1]); /* �ե�����̾ */
	    strcpy(dir->entry[num].str,  (char*)&temp[1]); /* ɽ��̾     */

	    dir->entry[num].type = FILE_STAT_DIR;
	    num++;
	}
	i++;
    } while (err == noErr && num < dir->nr_entry);

    dir->nr_entry = num;

    return dir;
}


/*---------------------------------------------------------------------------
 * T_DIR_INFO *osd_opendir(const char *filename)
 *---------------------------------------------------------------------------*/
T_DIR_INFO	*osd_opendir(const char *filename)
{
    char  *p;
    short i;
    OSErr err;
    short vRefNum;
    int   num;
    long  dirID;
    T_DIR_INFO	*dir;
    Str63	temp;
    CInfoPBRec	pbr;

    /* : �ʤ顢�롼��(�ȥåץ�٥�) �Ȥߤʤ� */
    if (strcmp(filename, ":") == 0) {
	return openroot();
    }

    /* filename��̵���Ȥ��⡢�롼��(�ȥåץ�٥�) �Ȥߤʤ� */
    if (filename == NULL || filename[0] == '\0') {
	return openroot();
    }


				/* T_DIR_INFO ����� 1�ĳ��� */
    if ((dir = (T_DIR_INFO *)malloc(sizeof(T_DIR_INFO))) == NULL) {
	return NULL;
    }

    err = PathToSpec(filename, &vRefNum, &dirID, NULL);
    if (noErr != err) {
	free(dir);
	return NULL;
    }

    /* ���ܤο���������� */
    dir->nr_entry = 2;		/* ����Ǥ�2����ȥ� (TOP��parent) */
    i = 1;
    pbr.hFileInfo.ioNamePtr = temp;
    pbr.hFileInfo.ioVRefNum = vRefNum;
    do {
	pbr.hFileInfo.ioFDirIndex = i;
	pbr.hFileInfo.ioDirID     = dirID;
	pbr.hFileInfo.ioACUser    = 0;
	err = PBGetCatInfoSync(&pbr);
	if (err == noErr && !(pbr.hFileInfo.ioFlFndrInfo.fdFlags & 0x4000)) {
	    dir->nr_entry += 1;
	}
	i++;
    } while (err == noErr);
				/* T_DIR_ENTRY ����� ���ܿ�ʬ ���� */
    dir->entry = (T_DIR_ENTRY *)malloc(dir->nr_entry * sizeof(T_DIR_ENTRY));
    if (dir->entry == NULL) {
	free(dir);
	return NULL;
    }
    for (i=0; i<dir->nr_entry; i++) {
	dir->entry[i].name = NULL;
      /*dir->entry[i].str  = NULL;*/
    }


    dir->cur_entry = 0;

    /* ��Ƭ�� <TOP> �� <parent> ���ɲ� */

#define	TOP_NAME	":"
#define	TOP_STR		"<< TOP >>"
#define	PAR_NAME	"::"
#define	PAR_STR		"<< parent >>"

    num = 0;
    {
	p = (char *)malloc(sizeof(TOP_NAME) + sizeof(TOP_STR));
	if (p == NULL) {    /* ���ե�����̾ �� ��ɽ��̾ �ΥХåե������ */
	    dir->nr_entry = num;
	    return dir;
	}
	dir->entry[num].name = &p[0];
	dir->entry[num].str  = &p[sizeof(TOP_NAME)];

	strcpy(dir->entry[num].name, TOP_NAME);
	strcpy(dir->entry[num].str,  TOP_STR);

	dir->entry[num].type = FILE_STAT_DIR;
    }
    num = 1;
    {
	p = (char *)malloc(sizeof(PAR_NAME) + sizeof(PAR_STR));
	if (p == NULL) {    /* ���ե�����̾ �� ��ɽ��̾ �ΥХåե������ */
	    dir->nr_entry = num;
	    return dir;
	}
	dir->entry[num].name = &p[0];
	dir->entry[num].str  = &p[sizeof(PAR_NAME)];

	strcpy(dir->entry[num].name, PAR_NAME);
	strcpy(dir->entry[num].str,  PAR_STR);

	dir->entry[num].type = FILE_STAT_DIR;
    }

    /* �ե������ι��ܤ��ɲ� */
    i = 1;
    num = 2;
    pbr.hFileInfo.ioNamePtr = temp;
    pbr.hFileInfo.ioVRefNum = vRefNum;
    do {
	pbr.hFileInfo.ioFDirIndex = i;
	pbr.hFileInfo.ioDirID     = dirID;
	pbr.hFileInfo.ioACUser    = 0;
	err = PBGetCatInfo(&pbr, 0);
	if (err == noErr &&		/* �� �ԲĻ�°���Υե�����Ͻ��� */
	    !(pbr.hFileInfo.ioFlFndrInfo.fdFlags & 0x4000)) {
	    temp[temp[0] + 1] = 0;

	    p = (char *)malloc((temp[0] + 1) + (temp[0] + 3));
	    if (p == NULL) { /* ���ե�����̾ �� ��ɽ��̾ �ΥХåե������ */
		dir->nr_entry = num;
		break;				/* malloc �˼��Ԥ��������� */
	    }

	    dir->entry[num].name = &p[0];
	    dir->entry[num].str  = &p[(temp[0] + 1)];

	    if (pbr.hFileInfo.ioFlAttrib & 16) {
		sprintf(dir->entry[num].name, "%s",   temp + 1);
		sprintf(dir->entry[num].str,  "[%s]", temp + 1);
		dir->entry[num].type = FILE_STAT_DIR;
	    } else {
		sprintf(dir->entry[num].name, "%s", temp + 1);
		sprintf(dir->entry[num].str,  "%s", temp + 1);
		dir->entry[num].type = FILE_STAT_FILE;
	    }
	    num++;
	}
	i++;
    } while (err == noErr && num < dir->nr_entry);

    dir->nr_entry = num;

    return dir;
}



/*---------------------------------------------------------------------------
 * T_DIR_ENTRY *osd_readdir(T_DIR_INFO *dirp)
 *	osd_opendir() �λ��˳��ݤ���������ȥ�������ؤΥݥ��󥿤�
 *	�缡���֤��Ƥ�����
 *---------------------------------------------------------------------------*/
T_DIR_ENTRY	*osd_readdir(T_DIR_INFO *dirp)
{
    T_DIR_ENTRY *ret_value = NULL;

    if (dirp->cur_entry != dirp->nr_entry) {
	ret_value = &dirp->entry[ dirp->cur_entry ];
	dirp->cur_entry ++;
    }
    return ret_value;
}



/*---------------------------------------------------------------------------
 * void osd_closedir(T_DIR_INFO *dirp)
 *	osd_opendir() ���˳��ݤ������ƤΥ���������롣
 *---------------------------------------------------------------------------*/
void		osd_closedir(T_DIR_INFO *dirp)
{
    int i;

    for (i=dirp->nr_entry -1; i>=0; i--) {
	if (dirp->entry[i].name) {
	    free(dirp->entry[i].name);
	}
    }
    free(dirp->entry);
    free(dirp);
}



/****************************************************************************
 * �ѥ�̾�����
 *****************************************************************************/

/*---------------------------------------------------------------------------
 * int	osd_path_normalize(const char *path, char resolved_path[], int size)
 *
 *	Ϣ³���� : �ϡ�ñ��� : ���֤������롣�Ĥޤ����Хѥ��ϻȤ��ʤ���
 *---------------------------------------------------------------------------*/
int	osd_path_normalize(const char *path, char resolved_path[], int size)
{
    int i, del = FALSE;

    while (*path) {			/* ��Ƭ�� : �����ƺ�� */
	if (*path == ':') { path ++; }
	else              { break; }
    }


    i = 0;
    while (--size && *path) {		/* path �򥳥ԡ�		  */
	if (*path == ':') {		/* â�� : ��Ϣ³�����顢1�Ĥˤ��� */
	    if (del == FALSE) {
		resolved_path[i++] = *path;
		del = TRUE;
	    }
	} else {
	    resolved_path[i++] = *path;
	    del = FALSE;
	}
	path ++;
    }

    if (size == 0 && *path) {
	return FALSE;
    }

    if (i == 0 && size) {		/* ��̤�����Ĥ�ʤ���� : �Ȥ��� */
	resolved_path[i++] = ':';	/* (:�ϡ��롼��(�ȥåץ�٥�)����) */
    } else {
	if (resolved_path[i-1] == ':') {	/* ������ : �Ϻ�� */
	    i--;
	}
    }

    resolved_path[i] = '\0';

    return TRUE;
}



/*---------------------------------------------------------------------------
 * int	osd_path_split(const char *path, char dir[], char file[], int size)
 *---------------------------------------------------------------------------*/
int	osd_path_split(const char *path, char dir[], char file[], int size)
{
    int pos = strlen(path);

    /* dir, file �Ͻ�ʬ�ʥ���������ݤ��Ƥ���Ϥ��ʤΤǡ��ڤ������å� */
    if (pos == 0 || size <= pos) {
	dir[0]  = '\0';
	file[0] = '\0';
	strncat(file, path, size-1);
	if (pos) fprintf(stderr, "internal overflow %d\n", __LINE__);
	return FALSE;
    }


    do {				/* : ����������õ�� 	*/
	if (path[ pos-1 ] == ':') { break; }
	pos --;
    } while (pos);

    if (pos) {				/* : �����Ĥ��ä���	*/
	strncpy(dir, path, pos);		/* ��Ƭ�� : �ޤǤ򥳥ԡ�*/
	dir[pos-1] = '\0';			/* : �ϴޤޤ�ޤ���	*/
	strcpy(file, &path[pos]);

    } else {				/* : �����Ĥ���ʤ��ä�	*/
	strcpy(dir,  "");			/* �ǥ��쥯�ȥ�� ""	*/
	strcpy(file, path);			/* �ե������ path����	*/
    }

    return TRUE;
}



/*---------------------------------------------------------------------------
 * int	osd_path_join(const char *dir, const char *file, char path[], int size)
 *
 *	file �� ":"  �ʤ顢�ȥåץ�٥�Ȥ��롣
 *
 *	file �� "::" �ʤ顢�ƥǥ��쥯�ȥ��귫�롣
 *		dir �� : ���ޤޤ����ϡ��Ǹ�� : �μ�����ʬ����Ф���
 *		dir �� : ���ޤޤ�ʤ����ϡ� ":" (������) �Ȥ��롣
 *
 *	file �� ����ʳ��ʤ顢 dir �� file ���礹�롣
 *		dir == ":" �ʤ顢 "file" ���Τ�ΤȤ��롣
 *		dir != ":" �ʤ顢 "dir:file" �Ȥ��롣
 *		(file �� :: �ǻϤޤ����Ʊ�ͤ˷�礹�롣
 *		 ��������ʣ���� : �� ñ��� : �Ȥ��ƽ��������)
 *---------------------------------------------------------------------------*/
int	osd_path_join(const char *dir, const char *file, char path[], int size)
{
    char buf[257];

    if (strcmp(file, ":") == 0) {	/* ":" �ϥ롼��(�ȥåץ�٥�)����) */
	strcpy(buf, ":");
    } else {
	if (strcmp(file, "::") == 0) {	/* "::" �ϡ��ƥǥ��쥯�ȥ����� */
	    char *p;
	    strcpy(buf, dir);
	    if (buf[ strlen(buf) - 1 ] == ':') {
		buf[ strlen(buf) - 1 ] = '\0';
	    }
	    p = strrchr(buf, ':');
	    if (p) { *p = '\0'; }
	    else   { strcpy(buf, ":"); }

	} else {			/* ����ʳ��� */
	    if (strcmp(dir, ":") == 0) {	/* dir==":" �ʤ� file �Ȥ��� */
		strcpy(buf, file);
	    } else {				/* �ʳ��ϡ�  dir:file �Ȥ��� */
		sprintf(buf, "%s:%s", dir, file);
	    }
	}
    }

    return osd_path_normalize(buf, path, size);
}



/****************************************************************************
 * �ե�����°���μ���
 ****************************************************************************/

int	osd_file_stat(const char *pathname)
{
    if (pathname == NULL || pathname[0] == '\0') {
	return FILE_STAT_NOEXIST;
    }

    if (strcmp(pathname, ":") == 0) {	/* ":" �ϥ롼��(�ȥåץ�٥�)����) */
	return FILE_STAT_DIR;

    } else {				/* osd_opendir��Ʊ�����Ȥ��äƤߤ� */
	short vRefNum;
	long dirID;
	Boolean is_dir;

	if (PathToSpec(pathname, &vRefNum, &dirID, &is_dir) == noErr) {
	    if (is_dir) return FILE_STAT_DIR;
	    else        return FILE_STAT_FILE;
	} else {
	    return FILE_STAT_NOEXIST;
	}
    }
}






/****************************************************************************
 * int	osd_file_config_init(void)
 *
 *	���δؿ��ϡ���ư���1�٤����ƤӽФ���롣
 *	���ｪλ���Ͽ��� malloc �˼��Ԥ����ʤɰ۾ｪλ���ϵ����֤���
 *
 ****************************************************************************/

int	osd_file_config_init(void)
{
    char buffer[257];

    OSErr err;
    short vRefNum;
    long  dirID;

	/* �����ȥǥ��쥯�ȥ������ */

    dir_cwd[0] = '\0';

    err = HGetVol((UInt8*)buffer, &vRefNum, &dirID);
    if (noErr != err) {
	;
    } else {
	err = GetFullPath(vRefNum, dirID, (UInt8*)buffer);
	if (noErr == err) {
	    buffer[ buffer[0]+1 ] = '\0';
	    strcpy(dir_cwd, &buffer[1]);
	} else {
	    ;
	}
    }

	/* ROM�ǥ��쥯�ȥ�����ꤹ�� */

    if (strlen(dir_rom) + sizeof(":ROM") < 256) {
	dir_rom[0] = '\0';
	strcat(dir_rom, dir_cwd);
	strcat(dir_rom, ":ROM");
    }

	/* DISK�ǥ��쥯�ȥ�����ꤹ�� */

    if (strlen(dir_disk) + sizeof(":DISK") < 256) {
	dir_disk[0] = '\0';
	strcat(dir_disk, dir_cwd);
	strcat(dir_disk, ":DISK");
    }

	/* TAPE�ǥ��쥯�ȥ�����ꤹ�� */

    if (strlen(dir_tape) + sizeof(":TAPE") < 256) {
	dir_tape[0] = '\0';
	strcat(dir_tape, dir_cwd);
	strcat(dir_tape, ":TAPE");
    }

	/* SNAP�ǥ��쥯�ȥ�����ꤹ�� */

    if (strlen(dir_snap) + sizeof(":SNAP") < 256) {
	dir_snap[0] = '\0';
	strcat(dir_snap, dir_cwd);
	strcat(dir_snap, ":SNAP");
    }

	/* STATE�ǥ��쥯�ȥ�����ꤹ�� */

    if (strlen(dir_state) + sizeof(":STATE") < 256) {
	dir_state[0] = '\0';
	strcat(dir_state, dir_cwd);
	strcat(dir_state, ":STATE");
    }


	/* ��������ǥ��쥯�ȥ�����ꤹ�� */

    if (strlen(dir_g_cfg) + sizeof(":Prefs") < 256) {
	dir_g_cfg[0] = '\0';
	strcat(dir_g_cfg, dir_cwd);
/*	strcat(dir_g_cfg, ":Prefs");*/
    }

	/* ��������ǥ��쥯�ȥ�����ꤹ�� */

    if (strlen(dir_l_cfg) + sizeof(":Prefs") < 256) {
	dir_l_cfg[0] = '\0';
	strcat(dir_l_cfg, dir_cwd);
/*	strcat(dir_l_cfg, ":Prefs");*/
    }

    return TRUE;
}



/*------------------------------------------------------------------------
 *
 *
 *------------------------------------------------------------------------*/

/* �ܥ�塼���ֹ�ȥǥ��쥯�ȥ�ID����ե�ѥ���������� */
static OSErr GetFullPath(short vRefNum, long dirID, UInt8 *pathname)
{
    OSErr  err;
    FSSpec spec;
    char   tmp[257];
    int    i = 0;
    UInt8  nullstr[2] = { 0, 0 };

    pathname[0] = pathname[1] = 0;
	
    spec.vRefNum = vRefNum;
    spec.parID   = dirID;
    do {
	err = FSMakeFSSpec(spec.vRefNum, spec.parID, nullstr, &spec);
	if (noErr == err) {
	    memcpy(tmp, &spec.name[1], spec.name[0]);
	    tmp[spec.name[0]] = 0;
	    if (i > 0) 
		strcat((char*)tmp, ":");
	    strcat(tmp, (char*)&pathname[1]);
	    pathname[0] = strlen(tmp);
	    strcpy((char*)&pathname[1], tmp);
	}
	i++;
    } while (spec.parID != 1 && noErr == err);

    return err;
}

/* �ե�ѥ�����ܥ�塼���ֹ�ȥǥ��쥯�ȥ�ID��������� */
static OSErr PathToSpec(const char *pathname, short *vRefNum, long *dirID,
			Boolean *is_dir)
{
    FSSpec spec;
    OSErr  err;
    UInt8  temp[257];
    int    i;
    CInfoPBRec pbr;
    HParamBlockRec hpbr;

    if (is_dir) *is_dir = FALSE;

    temp[0] = strlen(pathname);
    strcpy((char*)&temp[1], pathname);
    if (! strchr(pathname, ':')) {	/* : ���ޤޤ�ʤ���� (�ܥ�塼��?) */
	hpbr.volumeParam.ioNamePtr = temp;
	i = 1;
	do {
	    hpbr.volumeParam.ioVolIndex = i;
	    err = PBHGetVInfoSync(&hpbr);
	    temp[temp[0] + 1] = 0;
	    if (err == noErr && !strcmp((char*)&temp[1], pathname)) {
		*vRefNum = hpbr.volumeParam.ioVRefNum;
		*dirID   = fsRtDirID;
		if (is_dir) *is_dir = TRUE;
		return err;
	    }
	    i++;
	} while (err == noErr);
	return err;
    }

    err = FSMakeFSSpec(-1, 1, temp, &spec);
    if (err != noErr) return err;

    /* dirID��������� */
    pbr.hFileInfo.ioNamePtr   = spec.name;
    pbr.hFileInfo.ioVRefNum   = spec.vRefNum;
    pbr.hFileInfo.ioFDirIndex = 0;
    pbr.hFileInfo.ioDirID     = spec.parID;
    pbr.hFileInfo.ioACUser    = 0;
    err = PBGetCatInfoSync(&pbr);
    if (err == noErr) {
	*vRefNum = pbr.hFileInfo.ioVRefNum;
	*dirID   = pbr.hFileInfo.ioDirID;
	if (pbr.hFileInfo.ioFlAttrib & 16) {
	    if (is_dir) *is_dir = TRUE;
	}
	return noErr;
    }
    return err;
}


/****************************************************************************
 * int	osd_file_config_exit(void)
 *
 *	���δؿ��ϡ���λ���1�٤����ƤӽФ���롣
 *
 ****************************************************************************/
void	osd_file_config_exit(void)
{
    /* �äˤ��뤳�Ȥʤ� */
}
