/*****************************************************************************/
/* �ե��������˴ؤ������						     */
/*									     */
/*	���ͤξܺ٤ϡ��إå��ե����� file-op.h ����			     */
/*									     */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>	/* _fullpath */
#include <string.h>
#include <sys/stat.h>	/* _stat */
#include <direct.h>	/* _getcwd, _getdrives */
#include <io.h>		/* _findfirst */
#include <errno.h>
#include <mbstring.h>	/* _mbsicmp */

#include "quasi88.h"
#include "initval.h"
#include "file-op.h"


/* ���ե�JIS�����ɤ��ɤ�����Ƚ�� */
#define	IS_SJIS(h, l)							\
	((BETWEEN(0x81, (unsigned char)(h), 0x9F) ||			\
	  BETWEEN(0xE0, (unsigned char)(h), 0xFC))   &&			\
	 (BETWEEN(0x40, (unsigned char)(l), 0x7E) ||			\
	  BETWEEN(0x80, (unsigned char)(l), 0xFC)))

#define	IS_SJIS_H(h)							\
	(BETWEEN(0x81, (h), 0x9F) || BETWEEN(0xE0, (h), 0xFC))



/*****************************************************************************/

static char *dir_cwd;	/* �ǥե���ȤΥǥ��쥯�ȥ� (������)		*/
static char *dir_rom;	/* ROM���᡼���ե�����θ����ǥ��쥯�ȥ�	*/
static char *dir_disk;	/* DISK���᡼���ե�����θ����ǥ��쥯�ȥ�	*/
static char *dir_tape;	/* TAPE���᡼���ե�����δ��ǥ��쥯�ȥ�	*/
static char *dir_snap;	/* ���̥��ʥåץ���åȥե��������¸��		*/
static char *dir_state;	/* �����ڥ�ɥե��������¸��			*/
static char *dir_home;	/* ��������ե�������֤��Ƥ�ǥ��쥯�ȥ�	*/
static char *dir_ini;	/* ��������ե�������֤��Ƥ�ǥ��쥯�ȥ�	*/



/****************************************************************************
 * �Ƽ�ǥ��쥯�ȥ�μ���	(osd_dir_cwd �� NULL���֤��ƤϤ��� !)
 *****************************************************************************/
const char *osd_dir_cwd  (void) { return dir_cwd;   }
const char *osd_dir_rom  (void) { return dir_rom;   }
const char *osd_dir_disk (void) { return dir_disk;  }
const char *osd_dir_tape (void) { return dir_tape;  }
const char *osd_dir_snap (void) { return dir_snap;  }
const char *osd_dir_state(void) { return dir_state; }
const char *osd_dir_gcfg (void) { return dir_home;  }
const char *osd_dir_lcfg (void) { return dir_ini;   }

static int set_new_dir(const char *newdir, char **dir)
{
    char *p;
    p = malloc(strlen(newdir) + 1);
    if (p) {
	free(*dir);
	*dir = p;
	strcpy(*dir, newdir);
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
int osd_set_dir_gcfg (const char *d) { return set_new_dir(d, &dir_home);  }
int osd_set_dir_lcfg (const char *d) { return set_new_dir(d, &dir_ini);   }







/****************************************************************************
 * �ե�����̾�˻��Ѥ���Ƥ�����������ɤ����
 *		0 �� ASCII �Τ�
 *		1 �� ���ܸ�EUC
 *		2 �� ���ե�JIS
 *****************************************************************************/
int	osd_kanji_code(void)
{
    return 2;			/* WIN �ʤΤǤ��ᤦ���� SJIS */
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
 * ������ˡ���Τ�ʤ��Τǡ��ե�����̾�Ƕ��̤��뤳�Ȥˤ��褦��
 *
 * osd_fopen ���ƤӽФ��줿�Ȥ��ˡ��ե�����̾���ݻ����Ƥ�����
 * ���Ǥ˳����Ƥ���ե�����Υե�����̾�Ȱ��פ��ʤ���������å����롣
 * �����ǡ��ǥ��������᡼���ե�����ξ��ϡ����Ǥ˳����Ƥ���ե������
 * �ե�����ݥ��󥿤��֤���¾�ξ��ϥ����ץ��ԤȤ��� NULL ���֤���
 */


/*
 * �ե�����̾ f1 �� f2 ��Ʊ���ե�����Ǥ���п����֤�
 */
static int file_cmp(const char *f1, const char *f2);

#if 0

/*
 * WinAPI ��Ȥ���ˡ����ľ���褯�狼��ʤ���
 */

#include <windows.h>
static int file_cmp(const char *f1, const char *f2)
{
    HANDLE h1, h2;
    BY_HANDLE_FILE_INFORMATION fi1, fi2;

    if (f1 == NULL || f2 == NULL) return FALSE;
    if (f1 == f2) return TRUE;

    h1 = CreateFile(f1, GENERIC_READ, FILE_SHARE_READ, NULL,
		    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h1 == INVALID_HANDLE_VALUE) {
	return FALSE;
    }

    h2 = CreateFile(f2, GENERIC_READ, FILE_SHARE_READ, NULL,
		    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h2 == INVALID_HANDLE_VALUE) {
	CloseHandle(h1); return FALSE; 
    }

    if (! GetFileInformationByHandle(h1, &fi1)) {
	CloseHandle(h1);
	CloseHandle(h2);
	return FALSE;
    }
    if (! GetFileInformationByHandle(h2, &fi2)) {
	CloseHandle(h1);
	CloseHandle(h2);
	return FALSE;
    }

    return (fi1.dwVolumeSerialNumber == fi2.dwVolumeSerialNumber &&
	    fi1.nFileIndexHigh == fi2.nFileIndexHigh && 
	    fi1.nFileIndexLow  == fi2.nFileIndexLow)     ? TRUE : FALSE;
}
#elif 0

/*
 * �ե�����̾����Ӥ�����ˡ���ѻ�����ʸ����ʸ����̵�����������������Ƥ��롣
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
	    if (_strnicmp(f1, f2, 1) != 0) return FALSE;
	}

	f1 ++;
	f2 ++;
    }

    if (*f2 == '\0') return TRUE;
    else             return FALSE;
}
#else

/*
 * _mbsicmp ��ȤäƤ���ڤˡ�
 */

static int file_cmp(const char *f1, const char *f2)
{
    if (f1 == NULL || f2 == NULL) return FALSE;
    if (f1 == f2) return TRUE;

    return (_mbsicmp(f1, f2) == 0) ? TRUE : FALSE;
}
#endif







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
    char	*fullname;

    st = NULL;
    for (i=0; i<MAX_STREAM; i++) {	/* �����Хåե���õ�� */
	if (osd_stream[i].fp == NULL) {		/* fp �� NULL �ʤ���� */
	    st = &osd_stream[i];
	    break;
	}
    }
    if (st == NULL) return NULL;		/* �������ʤ���� NG */
    st->path = NULL;


    fullname = _fullpath(NULL, path, 0);	/* �ե�����̾��������� */
    if (fullname == NULL) return NULL;



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
		if (file_cmp(osd_stream[i].path, fullname)) {

		    free(fullname);

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
	st->path = fullname;		/* �ե�����̾���ݻ����� */
	/* FALLTHROUGH */


    default:
	st->fp = fopen(fullname, mode);	/* �ե�����򳫤� */

	if (st->fp) {

	    st->type = type;
	    strncpy(st->mode, mode, sizeof(st->mode));
	    return st;

	} else {

	    free(fullname);
	    return NULL;
	}
    }
}



int	osd_fclose(OSD_FILE *stream)
{
    FILE *fp = stream->fp;

    stream->fp = NULL;
    if (stream->path) {
	free(stream->path);
	stream->path = NULL;
    }

    return fclose(fp);
}



int	osd_fflush(OSD_FILE *stream)
{
    if (stream == NULL) return fflush(NULL);
    else                return fflush(stream->fp);
}



int	osd_fseek(OSD_FILE *stream, long offset, int whence)
{
    return fseek(stream->fp, offset, whence);
}



long	osd_ftell(OSD_FILE *stream)
{
    return ftell(stream->fp);
}



void	osd_rewind(OSD_FILE *stream)
{
    (void)osd_fseek(stream, 0L, SEEK_SET);
    osd_fflush(stream);
}



size_t	osd_fread(void *ptr, size_t size, size_t nobj, OSD_FILE *stream)
{
    return fread(ptr, size, nobj, stream->fp);
}



size_t	osd_fwrite(const void *ptr, size_t size, size_t nobj, OSD_FILE *stream)
{
    return fwrite(ptr, size, nobj, stream->fp);
}



int	osd_fputc(int c, OSD_FILE *stream)
{
    return fputc(c, stream->fp);
}


int	osd_fgetc(OSD_FILE *stream)
{
    return fgetc(stream->fp);
}


char	*osd_fgets(char *str, int size, OSD_FILE *stream)
{
    return fgets(str, size, stream->fp);
}


int	osd_fputs(const char *str, OSD_FILE *stream)
{
    return fputs(str, stream->fp);
}



/*---------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/
/* �ѥ����롼�ȥǥ��쥯�ȥ꤫�ɤ�����Ƚ�ꡣ path ������������Ƥ������� */
static	int	is_root_dir(const char *path)
{
    int len = strlen(path);

    if (len == 1 && strcmp(&path[0], "\\")  == 0) {	    /* \   �� �� */
	return TRUE;
    }
    if (len == 3 && strcmp(&path[1], ":\\") == 0) {	    /* x:\ �� �� */
	return TRUE;
    }
    if (len >= 4 && strncmp(&path[0], "\\\\", 2) == 0) {
	int i, j;

	if (path[2] == '\\') { return FALSE; }		    /* \\\ �� �� */

	/* �ͥåȥ���ǥ��쥯�ȥ�������顢���Ϥ���	      (\\�� �ǻϤޤ�)*/
	for (i = 3; i < len-1; i++) {

	    /* SJISʸ���ϥ����åפ��� */
	    if (IS_SJIS(path[i], path[i+1])) { i++; continue; }

	    /* \ �����Ĥ���С���³����ѥ������Ƥ�����å� */

	    if (path[i] == '\\') {

		/* �����ʤ�ѥ�����ü�ξ��ϡ��ۥ���̾����
		   �ʤΤǡ��롼�ȤȤߤʤ��ʤ� */

		if (path[i+1] == '\0') { return FALSE; }    /* \\x��\ �� �� */

		/* ��³����ѥ��� \ �ǽ���뤫�����å� */

		for (j = i+1; j < len-1; j++) {

		    if (IS_SJIS(path[j], path[j+1])) { i++; continue; }

		    if (path[j] == '\\') {
			if (path[j+1] == '\0') {	/* \\x��\y��\   �� ��*/
			    return TRUE;
			} else {			/* \\x��\y��\�� �� ��*/
			    return FALSE;
			}
		    }
		}

		/* ��³����ѥ��� \ ���ޤޤ�ʤ����ϡ�
		   \ �ǽ���äƤʤ����롼�ȤȤߤʤ� */

		return TRUE;				/* \\x��\y��    �� ��*/
	    }
	}

	/* �ۥ���̾�����ʤΤǡ��롼�ȤȤߤʤ��ʤ� */
	return FALSE;					    /* \\x��  �� �� */
    }

    return FALSE;
}

/* �ѥ��������� \ ���ɤ�����Ƚ�� */
static	int	backslash_terminated(const char *path)
{
    int i, len = strlen(path);

    for (i = 0; i < len; i++) {

	if (i < len-1 ){
	    if (IS_SJIS(path[i], path[i+1])) { i++; continue; }
	}

	if (path[i] == '\\' &&
	    path[i+1] == '\0') {
	    return TRUE;
	}
    }

    return FALSE;
}

/****************************************************************************
 * �ǥ��쥯�ȥ����
 *****************************************************************************/

struct	T_DIR_INFO_STRUCT
{
    int		cur_entry;		/* ��̤�������������ȥ��	*/
    int		nr_entry;		/* ����ȥ������		*/
    int		nr_total;		/* ����ȥ������ + �ɥ饤�ֿ�	*/
    T_DIR_ENTRY	*entry;			/* ����ȥ���� (entry[0]��)	*/
};



/*
 * �ǥ��쥯�ȥ���Υե�����̾�Υ����ƥ��󥰤˻Ȥ��ؿ�
 */

	/* ��ʸ����ʸ����̵�뤷�ƥե�����̾������  */
static int namecmp(const void *p1, const void *p2)
{
    T_DIR_ENTRY *s1 = (T_DIR_ENTRY *)p1;
    T_DIR_ENTRY *s2 = (T_DIR_ENTRY *)p2;
#if 0
    return _stricmp(s1->name, s2->name);
#else
    return _mbsicmp(s1->name, s2->name);
#endif
}
	/* ��ʸ����ʸ����̵�뤷�ƥǥ��쥯�ȥ�̾������  */
static int dircmp(const void *p1, const void *p2)
{
    T_DIR_ENTRY *s1 = (T_DIR_ENTRY *)p1;
    T_DIR_ENTRY *s2 = (T_DIR_ENTRY *)p2;
    if (s1->str[0] == '<' && s2->str[0] != '<') return -1; /* <��>�ϥɥ饤�� */
    if (s1->str[0] != '<' && s2->str[0] == '<') return +1; /* �ʤΤǹ�ͥ��   */
#if 0
    return _stricmp(s1->name, s2->name);
#else
    return _mbsicmp(s1->name, s2->name);
#endif
}
	/* �ե�����ȥǥ��쥯�ȥ�Ȥ�ʬΥ */
static int typecmp(const void *p1, const void *p2)
{
    T_DIR_ENTRY *s1 = (T_DIR_ENTRY *)p1;
    T_DIR_ENTRY *s2 = (T_DIR_ENTRY *)p2;

    if (s1->type == s2->type) return 0;
    if (s1->type == FILE_STAT_DIR) return -1;
    else                           return +1;
}



/*---------------------------------------------------------------------------
 * T_DIR_INFO *osd_opendir(const char *filename)
 *	_findfirst(), _findnext(), _findclose() ���Ȥ���
 *	�ǥ��쥯�ȥ�����ƤΥ���ȥ�� �ե�����̾��°�������˥��åȤ��롣
 *	����� malloc �ǳ��ݤ��뤬�����Ի��Ϥ����ǥ���ȥ�μ������Ǥ��ڤ롣
 *	������ϡ����Υ����ե�����̾�ǥ����Ȥ��Ƥ�����
 *	�ޤ����ǥ��쥯�ȥ�ξ��ϡ�ɽ���Ѥ�̾������� [ �� ] ���ղä��Ƥ�����
 *---------------------------------------------------------------------------*/
T_DIR_INFO	*osd_opendir(const char *filename)
{
    int i;
    T_DIR_INFO *dir;

    long dirp;
    struct _finddata_t dp;

    int len;
    char *p;
    char *fname;
    long drv_list    = _getdrives();
    char drv_name[4] = "A:\\";
    char drv_str[5]  = "<A:>";
    int top_dir = FALSE;

				/* T_DIR_INFO ����� 1�ĳ��� */
    if ((dir = (T_DIR_INFO *)malloc(sizeof(T_DIR_INFO))) == NULL) {
	return NULL;
    }

    if (filename == NULL || filename[0] == '\0') {
	filename = ".";
    }

				/* �롼�ȥǥ��쥯�ȥ꤫�ɤ�����Ƚ�� */
    top_dir = is_root_dir(filename);


				/* �ǥ��쥯�ȥ긡��̾ "filename\\*" �򥻥å� */
    len = strlen(filename) + sizeof("\\*");
    if (len >= OSD_MAX_FILENAME ||
	((fname = (char*)malloc(len)) == NULL)) {	/* �Хåե����� */
	free(dir);
	return NULL;
    }
    strcpy(fname, filename);
    if (backslash_terminated(fname) == FALSE) {
	strcat(fname, "\\");				/* ������ \ ���ղ� */
    }
    strcat(fname, "*");					/* ����� * ���ղ� */


				/* �ǥ��쥯�ȥ���Υե������������� */
    dir->nr_entry = 0;
    dirp = _findfirst(fname, &dp);
    if (dirp != -1) {
	do {
	    dir->nr_entry ++;
	} while (_findnext(dirp, &dp) == 0);
	_findclose(dirp);
    }


				/* T_DIR_ENTRY ����� �ե������ʬ ���� */
    dir->nr_total = dir->nr_entry + 26;		/* +26 �ϥɥ饤��̾��ʬ */
    dir->entry = (T_DIR_ENTRY *)malloc(dir->nr_total * sizeof(T_DIR_ENTRY));
    if (dir->entry == NULL) {
	free(dir);
	free(fname);
	return NULL;
    }
    for (i=0; i<dir->nr_total; i++) {
	dir->entry[i].name = NULL;
	dir->entry[i].str  = NULL;
    }


				/* �ե������ʬ�������롼�� (������Ǽ) */
    dirp = -1;
    for (i=0; i<dir->nr_entry; i++) {

	if (i == 0) {			/* �ե�����̾����(���) */
	    dirp = _findfirst(fname, &dp);
	    if (dirp == -1) {
		dir->nr_entry = i;		/* �����˼��Ԥ����顢����  */
		break;
	    }
	} else {			/* �ե�����̾����(����ʹ�) */
	    if (_findnext(dirp, &dp) != 0) {
		dir->nr_entry = i;		/* �����˼��Ԥ����顢����  */
		break;
	    }
	}

					/* �ե�����μ���򥻥å� */
	if (dp.attrib & _A_SUBDIR) {
	    dir->entry[i].type = FILE_STAT_DIR;
	} else {
	    dir->entry[i].type = FILE_STAT_FILE;
	}

					/* �ե�����̾�Хåե����� */
	len = strlen(dp.name) + 1;
	p = (char *)malloc(( len )   +  ( len + 2 ));
	if (p == NULL) { /* ���ե�����̾ �� ��ɽ��̾ �ΥХåե���쵤�˳��� */
	    dir->nr_entry = i;
	    break;				/* malloc �˼��Ԥ��������� */
	}

					/* �ե�����̾��ɽ��̾���å� */
	dir->entry[i].name = &p[0];
	dir->entry[i].str  = &p[len];

	strcpy(dir->entry[i].name, dp.name);

	if (dir->entry[i].type == FILE_STAT_DIR) {
	    sprintf(dir->entry[i].str, "[%s]", dp.name);
	} else {
	    sprintf(dir->entry[i].str, "%s",   dp.name);
	}

    }


    free(fname);
    if (dirp != -1) {
	_findclose(dirp);		/* �ǥ��쥯�ȥ���Ĥ��� */
    }


	/* ����ȥ꤬�ʤ�(��������)���䡢�롼�ȥǥ��쥯�ȥ�ξ��ϡ�
	   �ɥ饤�֤򥨥�ȥ���ɲä��Ƥ����褦 */

    if (dir->nr_entry == 0 || top_dir) {
	for (i=0; i<26; i++) {
	    if (drv_list & (1L<<i)) {

		p = (char *)malloc(sizeof(drv_name) + sizeof(drv_str));
		if (p) {
		    dir->entry[ dir->nr_entry ].name = &p[0];
		    dir->entry[ dir->nr_entry ].str  = &p[sizeof(drv_name)];

		    strcpy(dir->entry[ dir->nr_entry ].name, drv_name);
		    strcpy(dir->entry[ dir->nr_entry ].str,  drv_str);

		    dir->entry[ dir->nr_entry ].type = FILE_STAT_DIR;
		    dir->nr_entry ++;
		}
	    }
	    drv_name[0] ++;	/* "x:\\" �� x �� A��Z���ִ����Ƥ��� */
	    drv_str[1] ++;	/* "<x:>" �� x �� A��Z���ִ����Ƥ��� */
	}
    }


				/* �ե�����̾�򥽡��� */

					/* �ޤ��ե�����ȥǥ��쥯�ȥ��ʬΥ */
    qsort(dir->entry, dir->nr_entry, sizeof(T_DIR_ENTRY), typecmp);
    {
	T_DIR_ENTRY *p = dir->entry;
	for (i=0; i<dir->nr_entry; i++, p++) {
	    if (p->type == FILE_STAT_FILE) break;
	}
					/* �ơ���ե�����̾�ǥ����� */
	qsort(&dir->entry[0], i, sizeof(T_DIR_ENTRY), dircmp);
	qsort(&dir->entry[i], dir->nr_entry-i, sizeof(T_DIR_ENTRY), namecmp);
    }


				/* osd_readdir �������� */
    dir->cur_entry = 0;
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

    for (i=0; i<dirp->nr_entry; i++) {
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
 *	��������:
 *		_fullpath()�η�̤򤽤Τޤ��֤���
 *			������ \ ���Ĥä���硢����Ϻ�����롣
 *		_fullpath()�η�̤� NULL �ʤ顢pathname �򤽤Τޤ��֤�
 *---------------------------------------------------------------------------*/
int	osd_path_normalize(const char *path, char resolved_path[], int size)
{
    if (_fullpath(resolved_path, path, size) != NULL) {

	int i = strlen(resolved_path);
	if (i) {
	    if (is_root_dir(resolved_path)) {
		/* �롼�ȥǥ��쥯�ȥ�ʤΤ������� \ �Ǥʤ��ʤ顢�ղ� */
		if (backslash_terminated(resolved_path) == FALSE) {
		    strcat(resolved_path, "\\");
		}
	    } else {
		/* �롼�ȥǥ��쥯�ȥ�ʳ��ϡ������� \ �Ϻ�� */
		if (backslash_terminated(resolved_path)) {
		    resolved_path[i - 1] = '\0';
		}
	    }
	}
	/* resolved_path �ˤ����Хѥ���Ǽ�Ѥ� */
	/*printf("NORM:\"%s\" => \"%s\"\n",path,resolved_path);*/
	return TRUE;
    } else {
	return FALSE;
    }
}



/*---------------------------------------------------------------------------
 * int	osd_path_split(const char *path, char dir[], char file[], int size)
 *
 *	��������:
 *		path �κǸ�� \ ������� dir������ file �˥��åȤ���
 *			dir �������� \ �ϤĤ��ʤ���
 *		path �������� \ �ʤ顢ͽ�������Ƥ����������
 *			��äơ� file �������ˤ� \ �ϤĤ��ʤ���
 *		path ��ͽ�ᡢ����������Ƥ����ΤȤ��롣
 *---------------------------------------------------------------------------*/
int	osd_path_split(const char *path, char dir[], char file[], int size)
{
    int skip = FALSE;
    int i, backslash_pos;
    int len = strlen(path);

    /* dir, file �Ͻ�ʬ�ʥ���������ݤ��Ƥ���Ϥ��ʤΤǡ��ڤ������å� */
    if (len == 0 || size <= len) {
	dir[0]  = '\0';
	file[0] = '\0';
	strncat(file, path, size-1);
	if (len) fprintf(stderr, "internal overflow %d\n", __LINE__);
	return FALSE;
    }


					/* �롼�ȥǥ��쥯�ȥ�ξ�硢�̽��� */
    if (is_root_dir(path)) {		/* (������ \ ��Ĥ�����)            */
	strcpy(dir, path);			/* �ǥ��쥯�ȥ�� path	*/
	strcpy(file, "");			/* �ե������ ""	*/
	return TRUE;
    }

    if (backslash_terminated(path)) {	/* path ������ \ �ʤ�ͽ�᥹���å� */
	skip = TRUE;
	len --;
    }


    backslash_pos = 0;			/* ��Ƭ���顢�Ǹ�� \ ��õ�� */
    for (i = 0; i < len; i++) {

	if (path[i] == '\\') { backslash_pos = i+1; }

	if (i < len-1 ){
	    if (IS_SJIS(path[i], path[i+1])) { i++; continue; }
	}
    }

    if (backslash_pos) {		/* \ �����Ĥ��ä���	*/
	strncpy(dir, path, backslash_pos);	/* ��Ƭ�� \ �ޤǤ򥳥ԡ�*/
	dir[backslash_pos] = '\0';		/* \ ��ޤޤ�ޤ�	*/

	/* dir �� ������ \ �ʤ顢��� */
	if (is_root_dir(dir)) {		/* �롼�ȥǥ��쥯�ȥ�ʤ������ʤ� */
	    /* DO NOTHING */
	} else {
	    if (backslash_terminated(dir)) {
		dir[backslash_pos - 1] = '\0';
	    }
	}

	strcpy(file, &path[backslash_pos]);

    } else {				/* \ �����Ĥ���ʤ��ä�	*/
	strcpy(dir,  "");			/* �ǥ��쥯�ȥ�� ""	*/
	strcpy(file, path);			/* �ե������ path����	*/
    }

    if (skip) {				/* �ե����������� / �Ϻ�� */
	file[ strlen(file) - 1 ] = '\0';
    }

    /*printf("SPLT:\"%s\" = \"%s\" + \"%s\")\n",path,dir,file);*/
    return TRUE;
}



/*---------------------------------------------------------------------------
 * int	osd_path_join(const char *dir, const char *file, char path[], int size)
 *
 *	��������:
 *		file �� \\ �ǻϤޤäƤ����顢���Τޤ� path �˥��å�
 *		file �� x:\\ �ξ��⡢      ���Τޤ� path �˥��å�
 *		�����Ǥʤ���С�"dir" + "\\" + "file" �� path �˥��å�
 *---------------------------------------------------------------------------*/
int	osd_path_join(const char *dir, const char *file, char path[], int size)
{
    int len = strlen(file);

    if (file[0] == '\\' ||			/* �ե�����̾�������Хѥ� */
	file[1] == ':') {

	if ((size_t)size <= strlen(file)) { return FALSE; }
	strcpy(path, file);

    } else {					/* �ե�����̾�ϡ����Хѥ� */

	path[0] = '\0';
	strncat(path, dir, size - 1);

	if (backslash_terminated(path) == FALSE) {	/* �ǥ��쥯�ȥ�����  */
	    len = strlen(path);				/* ��'\\' �Ǥʤ��ʤ� */
	    strncat(path, "\\", size - len - 1);	/* �ղä���          */
	}

	len = strlen(path);
	strncat(path, file, size - len - 1);

    }

    /*printf("JOIN:\"%s\" + \"%s\" = \"%s\"\n",dir,file,path);*/
    return TRUE;
}



/****************************************************************************
 * �ե�����°���μ���
 ****************************************************************************/
int	osd_file_stat(const char *pathname)
{
    struct _stat sb;

    if (_stat(pathname, &sb)) {
	return FILE_STAT_NOEXIST;
    }

    if (sb.st_mode & _S_IFDIR) {
	return FILE_STAT_DIR;
    } else {
	return FILE_STAT_FILE;
    }
}








/****************************************************************************
 * int	osd_file_config_init(void)
 *
 *	���δؿ��ϡ���ư���1�٤����ƤӽФ���롣
 *	���ｪλ���Ͽ��� malloc �˼��Ԥ����ʤɰ۾ｪλ���ϵ����֤���
 *
 ****************************************************************************/

static int make_dir(const char *dname);
static int check_dir(const char *dname);

/*
 * �Ķ��ѿ� *env_dir �˥��åȤ��줿�ǥ��쥯�ȥ�� **dir �˥��åȤ��롣
 * �Ķ��ѿ���̤����ʤ顢�����ȥǥ��쥯�ȥ� + *alt_dir ��ɽ�����
 * �ǥ��쥯�ȥ�� **dir �˥��åȤ��롣
 * ������ξ��⡢ **dir �ˤ� malloc ���줿�ΰ� (�ʤ��� NULL) �����åȤ����
 */

static void set_dir(char **dir, char *env_dir, char *alt_dir)
{
    char *s;

    s = getenv(env_dir);
    if (s) {

	*dir = _fullpath(NULL, s, 0);

    } else {

	if (alt_dir) {
	    if (dir_home) {

		s = (char*)malloc(strlen(dir_home) + strlen(alt_dir) + 2);

		if (s) {
		    s[0] = '\0';
		    if (dir_home[0]) {
			strcat(s, dir_home);
			strcat(s, "\\");
		    }
		    strcat(s, alt_dir);

		    *dir = _fullpath(NULL, s, 0);

		    free(s);
		} else {
		    *dir = NULL;
		}

		if (*dir) {
#if 0
		    if (make_dir(*dir)) return;  /* �ǥ��쥯�ȥ�ʤ���к�� */
#else
		    if (check_dir(*dir)) return; /* �ǥ��쥯�ȥꤢ������� */
#endif
		    free(*dir);
		}
	    }
	}

	*dir = _getcwd(NULL, 0);
    }
}


int	osd_file_config_init(void)
{
    char *s;


	/* �����ȥ���󥰥ǥ��쥯�ȥ�̾ (CWD) ��������� */

    dir_cwd = _getcwd(NULL, 0);


	/* �ۡ���ǥ��쥯�ȥ� $(QUASI88_HOME) ��������� */

    s = getenv("QUASI88_HOME");
    if (s) {
	dir_home = _fullpath(NULL, s, 0);
    } else {
	dir_home = _getcwd(NULL, 0);
    }


    /* �����ʥǥ��쥯�ȥ�����ꤹ��				*/
    /*	��2�����δĶ��ѿ������ꤷ�Ƥ���С����Υǥ��쥯�ȥꡣ	*/
    /*	̤����ʤ顢��3�����Υǥ��쥯�ȥ꤬ dir_home �ʲ���	*/
    /*	���뤫�����å���������Ф��졣�ʤ���� dir_cwd		*/


	/* ����ǥ��쥯�ȥ� */

    set_dir(&dir_ini, "QUASI88_INI_DIR", "INI");


	/* ROM�ǥ��쥯�ȥ� */

    set_dir(&dir_rom, "QUASI88_ROM_DIR", "ROM");


	/* DISK�ǥ��쥯�ȥ� */

    set_dir(&dir_disk, "QUASI88_DISK_DIR", "DISK");


	/* TAPE�ǥ��쥯�ȥ� */

    set_dir(&dir_tape, "QUASI88_TAPE_DIR", "TAPE");


	/* SNAP�ǥ��쥯�ȥ� */

    set_dir(&dir_snap, "QUASI88_SNAP_DIR", "SNAP");


	/* STATE�ǥ��쥯�ȥ� */

    set_dir(&dir_state, "QUASI88_STATE_DIR", "STATE");



	/* �ƥǥ��쥯�ȥ꤬����Ǥ��ʤ���а۾ｪλ */

    if (! dir_cwd  || ! dir_home || ! dir_ini  || ! dir_rom  ||
	! dir_disk || ! dir_tape || ! dir_snap || ! dir_state)  return FALSE;


    return TRUE;
}



/*
 *	�ǥ��쥯�ȥ� dname �����뤫�����å���̵����к�롣
 *		���������顢�����֤�
 */
static int make_dir(const char *dname)
{
    struct _stat sb;

    if (_stat(dname, &sb)) {

	if (errno == ENOENT) {			/* �ǥ��쥯�ȥ�¸�ߤ��ʤ� */

	    if (_mkdir(dname)) {
		fprintf(stderr, "error: can't make dir %s\n", dname);
		return FALSE;
	    } else {
		printf("make dir \"%s\"\n", dname);
	    }

	} else {				/* ����¾�ΰ۾� */
	    return FALSE;
	}

    } else {					/* �ǥ��쥯�ȥꤢ�ä� */

	if (! (sb.st_mode & _S_IFDIR)) {		/* �Ȼפä���ե�����*/
	    fprintf(stderr, "error: not exist dir %s\n", dname);
	    return FALSE;
	}

    }

    return TRUE;
}



/*
 *	�ǥ��쥯�ȥ� dname �����뤫�����å�������� ��
 */
static int check_dir(const char *dname)
{
    struct _stat sb;

    if (_stat(dname, &sb)) {

	return FALSE;				/* �����å����� */

    } else {					/* �ǥ��쥯�ȥꤢ�ä� */

	if (! (sb.st_mode & _S_IFDIR)) {		/* �Ȼפä���ե�����*/
	    return FALSE;
	}

    }

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
    if (dir_home)  free(dir_home);
    if (dir_ini)   free(dir_ini);
    if (dir_rom)   free(dir_rom);
    if (dir_disk)  free(dir_disk);
    if (dir_tape)  free(dir_tape);
    if (dir_snap)  free(dir_snap);
    if (dir_state) free(dir_state);
}
