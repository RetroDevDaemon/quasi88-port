/*****************************************************************************/
/* �ե��������˴ؤ������						     */
/*									     */
/*	���ͤξܺ٤ϡ��إå��ե����� file-op.h ����			     */
/*									     */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#include "quasi88.h"
#include "initval.h"
#include "file-op.h"
#include "menu.h"


/*****************************************************************************/

/* �ʲ��Υǥ��쥯�ȥ�̾�ϡ�ͽ�� OSD_MAX_FILENAME �Х��Ȥ�
   ����Ĺ�Хåե�����ݤ��ơ�����ˤ�ꤢ�Ƥ롣
	(malloc/free ��ưŪ�˴��������������ޡ��Ȥ��⤷��ʤ����ɡ�)	*/

static char *dir_cwd;	/* �ǥե���ȤΥǥ��쥯�ȥ� (������)		*/
static char *dir_rom;	/* ROM���᡼���ե�����θ����ǥ��쥯�ȥ�	*/
static char *dir_disk;	/* DISK���᡼���ե�����θ����ǥ��쥯�ȥ�	*/
static char *dir_tape;	/* TAPE���᡼���ե�����δ��ǥ��쥯�ȥ�	*/
static char *dir_snap;	/* ���̥��ʥåץ���åȥե��������¸��		*/
static char *dir_state;	/* �����ڥ�ɥե��������¸��			*/
static char *dir_g_cfg;	/* ��������ե�����Υǥ��쥯�ȥ�		*/
static char *dir_l_cfg;	/* ��������ե�����Υǥ��쥯�ȥ�		*/



/****************************************************************************
 * �Ƽ�ǥ��쥯�ȥ�μ���	(osd_dir_cwd �� NULL���֤��ƤϤ��� !)
 *****************************************************************************/
const char *osd_dir_cwd  (void) { return dir_cwd;   }
const char *osd_dir_rom  (void) { return dir_rom;   }
const char *osd_dir_disk (void) { return dir_disk;  }
const char *osd_dir_tape (void) { return dir_tape;  }
const char *osd_dir_snap (void) { return dir_snap;  }
const char *osd_dir_state(void) { return dir_state; }
const char *osd_dir_gcfg (void) { return dir_g_cfg[0] ? dir_g_cfg : NULL; }
const char *osd_dir_lcfg (void) { return dir_l_cfg[0] ? dir_l_cfg : NULL; }

static int set_new_dir(const char *newdir, char *dir)
{
    if (strlen(newdir) < OSD_MAX_FILENAME) {
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
 *		3 �� UTF-8
 *****************************************************************************/
int	osd_kanji_code(void)
{
    if      (file_coding == 2) return 3;
    else if (file_coding == 1) return 2;
    else                       return 1;
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
 *
 * osd_fopen ���ƤӽФ��줿�Ȥ��ˡ��ե�����ξ���� stat �ˤƼ�������
 * ���Ǥ˳����Ƥ���ե������ stat �Ȱ��פ��ʤ���������å����롣
 * �����ǡ��ǥ��������᡼���ե�����ξ��ϡ����Ǥ˳����Ƥ���ե������
 * �ե�����ݥ��󥿤��֤���¾�ξ��ϥ����ץ��ԤȤ��� NULL ���֤���
 */

struct OSD_FILE_STRUCT {

    FILE	*fp;			/* !=NULL �ʤ������	*/
    struct stat	sb;			/* �������ե�����ξ���	*/
    int		type;			/* �ե��������		*/
    char	mode[4];		/* �������ݤΡ��⡼��	*/

};

#define	MAX_STREAM	8
static	OSD_FILE	osd_stream[ MAX_STREAM ];



OSD_FILE *osd_fopen(int type, const char *path, const char *mode)
{
    int i;
    struct stat	sb;
    OSD_FILE	*st;
    int		stat_ok;

    st = NULL;
    for (i=0; i<MAX_STREAM; i++) {	/* �����Хåե���õ�� */
	if (osd_stream[i].fp == NULL) {		/* fp �� NULL �ʤ���� */
	    st = &osd_stream[i];
	    break;
	}
    }
    if (st == NULL) return NULL;		/* �������ʤ���� NG */


    if (stat(path, &sb) != 0) {		/* �ե�����ξ��֤�������� */
	if (mode[0] == 'r') return NULL;
	stat_ok = FALSE;
    } else {
	stat_ok = TRUE;
    }



    switch (type) {

    case FTYPE_DISK:		/* "r+b" , "rb"	*/
    case FTYPE_TAPE_LOAD:	/* "rb" 	*/
    case FTYPE_TAPE_SAVE:	/* "ab"		*/
    case FTYPE_PRN:		/* "ab"		*/
    case FTYPE_COM_LOAD:	/* "rb"		*/
    case FTYPE_COM_SAVE:	/* "ab"		*/

	if (stat_ok) {
	    /* ���Ǥ˳����Ƥ���ե����뤫�ɤ���������å����� */
	    for (i=0; i<MAX_STREAM; i++) {
		if (osd_stream[i].fp) {
		    if (osd_stream[i].sb.st_dev == sb.st_dev &&
			osd_stream[i].sb.st_ino == sb.st_ino) {

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
	}
	/* FALLTHROUGH */


    default:
	st->fp = fopen(path, mode);	/* �ե�����򳫤� */

	if (st->fp) {

	    if (stat_ok == FALSE) {		/* �⤦���٥ե�������֤� */
		fflush(st->fp);			/* �������Ƥߤ褦��       */
		if (stat(path, &sb) != 0) {	/* ɬ����������Ϥ��ġġ� */
		    sb.st_dev = 0;
		    sb.st_ino = 0;
		}
	    }

	    st->type = type;
	    st->sb   = sb;
	    strncpy(st->mode, mode, sizeof(st->mode));
	    return st;

	} else {

	    return NULL;
	}
    }
}



int	osd_fclose(OSD_FILE *stream)
{
    FILE *fp = stream->fp;

    stream->fp = NULL;
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



/****************************************************************************
 * �ǥ��쥯�ȥ����
 *****************************************************************************/

struct	T_DIR_INFO_STRUCT
{
    int		cur_entry;		/* ��̤�������������ȥ��	*/
    int		nr_entry;		/* ����ȥ������		*/
    T_DIR_ENTRY	*entry;			/* ����ȥ���� (entry[0]��)	*/
};



/*
 * �ǥ��쥯�ȥ���Υե�����̾�Υ����ƥ��󥰤˻Ȥ��ؿ�
 */
static int namecmp(const void *p1, const void *p2)
{
    T_DIR_ENTRY *s1 = (T_DIR_ENTRY *)p1;
    T_DIR_ENTRY *s2 = (T_DIR_ENTRY *)p2;

    return strcmp(s1->name, s2->name);
}




/*---------------------------------------------------------------------------
 * T_DIR_INFO *osd_opendir(const char *filename)
 *	opendir()��rewinddir()��readdir()��closedir() ���Ȥ���
 *	�ǥ��쥯�ȥ�����ƤΥ���ȥ�Υե�����̾�����˥��åȤ��롣
 *	����� malloc �ǳ��ݤ��뤬�����Ի��Ϥ����ǥ���ȥ�μ������Ǥ��ڤ롣
 *	������ϡ����Υ����ե�����̾�ǥ����Ȥ��Ƥ�����
 *---------------------------------------------------------------------------*/
T_DIR_INFO	*osd_opendir(const char *filename)
{
    int  len;
    char *p;
    int  i;
    T_DIR_INFO *dir;

    DIR *dirp;
    struct dirent *dp;

				/* T_DIR_INFO ����� 1�ĳ��� */
    if ((dir = (T_DIR_INFO *)malloc(sizeof(T_DIR_INFO))) == NULL) {
	return NULL;
    }

    if (filename == NULL || filename[0] == '\0') {
	filename = ".";
    }

    dirp = opendir(filename);	/* �ǥ��쥯�ȥ�򳫤� */
    if (dirp == NULL) {
	free(dir);
	return NULL;
    }


    dir->nr_entry = 0;		/* �ե������������� */
    while (readdir(dirp)) {
	dir->nr_entry ++;
    }
    rewinddir(dirp);

				/* T_DIR_ENTRY ����� �ե������ʬ ���� */
    dir->entry = (T_DIR_ENTRY *)malloc(dir->nr_entry * sizeof(T_DIR_ENTRY));
    if (dir->entry == NULL) {
	closedir(dirp);
	free(dir);
	return NULL;
    }
    for (i=0; i<dir->nr_entry; i++) {
	dir->entry[i].name = NULL;
	dir->entry[i].str  = NULL;
    }

				/* �ե������ʬ�������롼�� (������Ǽ) */
    for (i=0; i<dir->nr_entry; i++) {

	dp = readdir(dirp);		/* �ե�����̾���� */

	if (dp == NULL) {			/* �����˼��Ԥ����顢����  */
	    dir->nr_entry = i;			/* (��������ﰷ���Ȥ��롣 */
	    break;				/*  �����餯����ǥե����� */
	}					/*  ��������줿�Τ���)  */

					/* �ե�����μ���򥻥å� */
	{
	    char *fullname;			/* �ǥ��쥯�ȥ�̾(filename) */
	    struct stat sb;			/* �ȥե�����̾(dp->d_name) */
						/* ����stat�ؿ���°�������� */
						/* ���Ԥ��Ƥⵤ�ˤ��ʤ�     */

	    dir->entry[i].type = FILE_STAT_FILE; /*  (���Ԥ����� FILE ����) */

	    fullname = (char*)malloc(strlen(filename)+1 +strlen(dp->d_name)+1);

	    if (fullname) {
		sprintf(fullname, "%s%s%s", filename, "/", dp->d_name);

		if (stat(fullname, &sb) == 0) {
#if 1
		    if (S_ISDIR(sb.st_mode))
#else
		    if ((sb.st_mode & S_IFMT) == S_IFDIR)
#endif
		    {
			dir->entry[i].type = FILE_STAT_DIR;
		    } else {
			dir->entry[i].type = FILE_STAT_FILE;
		    }
		}
		free(fullname);
	    }
	}

					/* �ե�����̾�Хåե����� */

	len = strlen(dp->d_name) + 1;
	p = (char *)malloc(( len + 1 )  +  ( len + 1 ));
	if (p == NULL) { /* ���ե�����̾ �� ��ɽ��̾ �ΥХåե���쵤�˳��� */
	    dir->nr_entry = i;
	    break;				/* malloc �˼��Ԥ��������� */
	}

					/* �ե�����̾��ɽ��̾���å� */
	dir->entry[i].name = &p[0];
	dir->entry[i].str  = &p[len+1];

	strcpy(dir->entry[i].name, dp->d_name);
	strcpy(dir->entry[i].str,  dp->d_name);

	if (dir->entry[i].type == FILE_STAT_DIR) { /* �ǥ��쥯�ȥ�ξ�硢 */
	    strcat(dir->entry[i].str, "/");	   /* ɽ��̾�� / ���ղ�    */
	}

    }


    closedir(dirp);		/* �ǥ��쥯�ȥ���Ĥ��� */


				/* �ե�����̾�򥽡��� */
    qsort(dir->entry, dir->nr_entry, sizeof(T_DIR_ENTRY), namecmp);


#if 0	/* ���ν�����̵����ɽ��̾ abc/ �Ǥ� �ե�����̾�� abc �Τޤ� */
    /* �ǥ��쥯�ȥ�ξ�硢�ե�����̾�� / ���ղ� (�����ȸ�˹Ԥ�) */
    for (i=0; i<dir->nr_entry; i++) {
	if (dir->entry[i].type == FILE_STAT_DIR) {
	    strcat(dir->entry[i].name, "/");
	}
    }
#endif


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
    T_DIR_ENTRY	*ret_value = NULL;

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
    int	i;

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
 *		./ �Ϻ���� ../ �Ͽƥǥ��쥯�ȥ���֤������� /��/ �� / ���ִ���
 *		���ݤʤΤǡ���󥯤䥫���ȥǥ��쥯�ȥ��Ÿ�����ʤ���
 *		������ / ���Ĥä���硢����Ϻ�����롣
 *	��:
 *		"../dir1/./dir2///dir3/../../file" �� "../dir1/file"
 *---------------------------------------------------------------------------*/
int	osd_path_normalize(const char *path, char resolved_path[], int size)
{
    char *buf, *s, *d, *p;
    int is_abs, is_dir, success = FALSE;
    size_t len = strlen(path);

    if (len == 0) {
	if (size) { resolved_path[0] = '\0';  success = TRUE; }
    } else {

	is_abs = (path[0]     == '/') ? TRUE : FALSE;
	is_dir = (path[len-1] == '/') ? TRUE : FALSE;

	buf = (char *)malloc((len+3) * 2);	/* path ��Ʊ�������̤�	*/
	if (buf) {				/* �Хåե���2��ʬ ����	*/
	    strcpy(buf, path);
	    d = &buf[ len + 3 ];
	    d[0] = '\0';

	    s = strtok(buf, "/");		/* / �� ���ڤäƤ���	*/

	    if (s == NULL) {			/* ���ڤ�ʤ��ʤ顢	*/
						/* ����� / ���Τ�Τ�  */
		if (size > 1) {
		    strcpy(resolved_path, "/");
		    success = TRUE;
		}

	    } else {				/* ���ڤ줿�ʤ顢ʬ��	*/

		for ( ; s ;  s = strtok(NULL, "/")) {

		    if        (strcmp(s, ".")  == 0) {	/* . ��̵��	*/
			;

		    } else if (strcmp(s, "..") == 0) {	/* .. ��ľ������ */

			p = strrchr(d, '/');		    /* ľ����/��õ�� */

			if (p && strcmp(p, "/..") != 0) {   /* ���Ĥ����    */
			    *p = '\0';			    /*    ������ʬ�� */
			} else {                            /* ���Ĥ���ʤ�  */
			    if (p == NULL && is_abs) {	    /*   ���Хѥ��ʤ�*/
				;			    /*     ̵�뤹��  */
			    } else {			    /*   ���Хѥ��ʤ�*/
				strcat(d, "/..");	    /*     .. �ˤ��� */
			    }
			}

		    } else {				/* �嵭�ʳ���Ϣ�� */
			strcat(d, "/");			    /* ��� / ������ */
			strcat(d, s);
		    }
		}

		if (d[0] == '\0') {		/* ��̤���ʸ����ˤʤä��� */
		    if (is_abs) strcpy(d, "/");	/*   �������Хѥ��ʤ� /     */
		    /* else         ;		 *   �������Хѥ����� ��    */

		} else {
		    if (is_abs == FALSE) {	/* �������Хѥ��ʤ� */
			d ++;			/* ��Ƭ�� / ����  */
		    }
#if 0	/* ���ν�����̵�������� a/b/c/ �Ǥ� a/b/c �Ȥ��� */
		    if (is_dir) {		/* ���������� / �ʤ� */
			strcat(d, "/");		/* ������ / ���ղ�   */
		    }
#endif
		}

		if (strlen(d) < (size_t)size) {
		    strcpy(resolved_path, d);
		    success = TRUE;
		}
	    }

	    free(buf);
	}
    }

    /*printf("NORM:\"%s\" => \"%s\"\n",path,resolved_path);*/
    return success;
}



/*---------------------------------------------------------------------------
 * int	osd_path_split(const char *path, char dir[], char file[], int size)
 *
 *	��������:
 *		path �κǸ�� / ������� dir������ file �˥��åȤ���
 *			dir �������� / �ϤĤ��ʤ���
 *		path �������� / �ʤ顢ͽ�������Ƥ����������
 *			��äơ� file �������ˤ� / �ϤĤ��ʤ���
 *		path ��ͽ�ᡢ����������Ƥ����ΤȤ��롣
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


    if (strcmp(path, "/") == 0) {	/* "/" �ξ�硢�̽���	*/
	strcpy(dir, "/");			/* �ǥ��쥯�ȥ�� "/"	*/
	strcpy(file, "");			/* �ե������ ""	*/
	return TRUE;
    }

    if (path[ pos - 1 ] == '/') {	/* path ������ / ��̵��	*/
	pos --;
    }

    do {				/* / ����������õ��	*/
	if (path[ pos - 1 ] == '/') { break; }
	pos --;
    } while (pos);

    if (pos) {				/* / �����Ĥ��ä���	*/
	strncpy(dir, path, pos);		/* ��Ƭ�� / �ޤǤ򥳥ԡ�*/
	if (pos > 1)
	    dir[ pos - 1 ] = '\0';		/* ������ / �Ϻ������	*/
	else					/* ������		*/ 
	    dir[ pos ] = '\0';			/* "/"�ξ��� / �ϻĤ� */

	strcpy(file, &path[pos]);

    } else {				/* / �����Ĥ���ʤ��ä�	*/
	strcpy(dir,  "");			/* �ǥ��쥯�ȥ�� ""	*/
	strcpy(file, path);			/* �ե������ path����	*/
    }

    pos = strlen(file);			/* �ե����������� / �Ϻ�� */
    if (pos && file[ pos - 1 ] == '/') { 
	file[ pos - 1 ] = '\0';
    }

    /*printf("SPLT:\"%s\" = \"%s\" + \"%s\")\n",path,dir,file);*/
    return TRUE;
}



/*---------------------------------------------------------------------------
 * int	osd_path_join(const char *dir, const char *file, char path[], int size)
 *
 *	��������:
 *		file �� / �ǻϤޤäƤ����顢���Τޤ� path �˥��å�
 *		�����Ǥʤ���С�"dir" + "/" + "file" �� path �˥��å�
 *		����夬�ä� path �����������Ƥ���
 *---------------------------------------------------------------------------*/
int	osd_path_join(const char *dir, const char *file, char path[], int size)
{
    int len;
    char *p;

    if (dir == NULL    ||
	dir[0] == '\0' ||			/* �ǥ��쥯�ȥ�̾�ʤ� or  */
	file[0] == '/') {			/* �ե�����̾�������Хѥ� */

	if ((size_t)size <= strlen(file)) { return FALSE; }
	strcpy(path, file);

    } else {					/* �ե�����̾�ϡ����Хѥ� */

	path[0] = '\0';
	strncat(path, dir, size - 1);

	len = strlen(path);				/* �ǥ��쥯�ȥ�����  */
	if (len && path[ len - 1 ] != '/') {		/* �� '/' �Ǥʤ��ʤ� */
	    strncat(path, "/", size - len - 1);		/* �ղä���          */
	}

	len = strlen(path);
	strncat(path, file, size - len - 1);

    }


    p = (char *)malloc(size);			/* ���������Ƥ����� */
    if (p) {
	strcpy(p, path);
	if (osd_path_normalize(p, path, size) == FALSE) {
	    strcpy(path, p);
	}
	free(p);
    }

    /*printf("JOIN:\"%s\" + \"%s\" = \"%s\"\n",dir,file,path);*/
    return TRUE;
}



/****************************************************************************
 * �ե�����°���μ���
 ****************************************************************************/
#if 1

int	osd_file_stat(const char *pathname)
{
    struct stat sb;

    if (stat(pathname, &sb)) {
	return FILE_STAT_NOEXIST;
    }

    if (S_ISDIR(sb.st_mode)) {
	return FILE_STAT_DIR;
    } else {
	return FILE_STAT_FILE;
    }
}

#else
int	osd_file_stat(const char *pathname)
{
    DIR  *dirp;
    FILE *fp;

    if ((dirp = opendir(pathname))) {		/* �ǥ��쥯�ȥ�Ȥ��Ƴ��� */
	closedir(dirp);				/* ����������ǥ��쥯�ȥ� */
	return FILE_STAT_DIR;
    } else {
	if ((fp = fopen(pathname, "r"))) {	/* �ե�����Ȥ��Ƴ���     */
	    fclose(fp);				/* ����������ե�����	  */
	    return FILE_STAT_FILE;
	} else {
	    return FILE_STAT_NOEXIST;		/* �ɤ���Ȥ⼺��	  */
	}
    }
}
#endif






/****************************************************************************
 * int	osd_file_config_init(void)
 *
 *	���δؿ��ϡ���ư���1�٤����ƤӽФ���롣
 *	���ｪλ���Ͽ��� malloc �˼��Ԥ����ʤɰ۾ｪλ���ϵ����֤���
 *
 ****************************************************************************/
static int parse_tilda(const char *home, const char *path,
		       char *result_path, int result_size);
static int make_dir(const char *dname);

int	osd_file_config_init(void)
{
    char *s;
    char *home  = NULL;
    char *g_cfg = NULL;
    char *l_cfg = NULL;
    char *state = NULL;

	/* �������� (����Ĺ�ǽ�������ͽ��ʤΤ���Ū���ݤǤ⤤���������) */

    dir_cwd   = (char *)malloc(OSD_MAX_FILENAME);
    dir_rom   = (char *)malloc(OSD_MAX_FILENAME);
    dir_disk  = (char *)malloc(OSD_MAX_FILENAME);
    dir_tape  = (char *)malloc(OSD_MAX_FILENAME);
    dir_snap  = (char *)malloc(OSD_MAX_FILENAME);
    dir_state = (char *)malloc(OSD_MAX_FILENAME);
    dir_g_cfg = (char *)malloc(OSD_MAX_FILENAME);
    dir_l_cfg = (char *)malloc(OSD_MAX_FILENAME);


    if (! dir_cwd  || ! dir_rom   || ! dir_disk  || ! dir_tape || 
	! dir_snap || ! dir_state || ! dir_g_cfg || ! dir_l_cfg) return FALSE;



	/* �����ȥ���󥰥ǥ��쥯�ȥ�̾ (CWD) ��������� */

    if (getcwd(dir_cwd, OSD_MAX_FILENAME-1)) {
	dir_cwd[ OSD_MAX_FILENAME-1 ] = '\0';
    } else {
	fprintf(stderr, "error: can't get CWD\n");
	strcpy(dir_cwd, "");
    }


	/* �ۡ���ǥ��쥯�ȥ� $(HOME) ��������� */

    home = getenv("HOME");

    if (home    == NULL ||			/* ̤����Ȥ����Хѥ���	*/
	home[0] != '/') {			/* �ʤ����� NG	*/
	fprintf(stderr, "error: can't get HOME\n");
	home = NULL;

    } else {

	/* $(HOME)/.quasi88/�ʲ��Υǥ��쥯�ȥ����� */

#define	HOME_QUASI88		"/.quasi88"
#define	HOME_QUASI88_RC		"/.quasi88/rc"
#define	HOME_QUASI88_STATE	"/.quasi88/state"

	s = malloc(strlen(home) + sizeof(HOME_QUASI88) + 1);
	if (s) {
	    sprintf(s, "%s%s", home, HOME_QUASI88);

	    if (make_dir(s)) {
		g_cfg = s;
	    } else {
		free(s);
	    }
	}

	s = malloc(strlen(home) + sizeof(HOME_QUASI88_RC) + 1);
	if (s) {
	    sprintf(s, "%s%s", home, HOME_QUASI88_RC);

	    if (make_dir(s)) {
		l_cfg = s;
	    } else {
		free(s);
	    }
	}

	s = malloc(strlen(home) + sizeof(HOME_QUASI88_STATE) + 1);
	if (s) {
	    sprintf(s, "%s%s", home, HOME_QUASI88_STATE);

	    if (make_dir(s)) {
		state = s;
	    } else {
		free(s);
	    }
	}

    }



	/* ROM�ǥ��쥯�ȥ�����ꤹ�� */

    s = getenv("QUASI88_ROM_DIR");		/* $(QUASI88_ROM_DIR)	*/
    if (s && strlen(s) < OSD_MAX_FILENAME) {
	strcpy(dir_rom, s);
    } else {
	if (parse_tilda(home, ROM_DIR, dir_rom, OSD_MAX_FILENAME) == 0) {
	    strcpy(dir_rom, dir_cwd);
	}
    }


	/* DISK�ǥ��쥯�ȥ�����ꤹ�� */

    s = getenv("QUASI88_DISK_DIR");		/* $(QUASI88_DISK_DIR) */
    if (s && strlen(s) < OSD_MAX_FILENAME) {
	strcpy(dir_disk, s);
    } else {
	if (parse_tilda(home, DISK_DIR, dir_disk, OSD_MAX_FILENAME) == 0) {
	    strcpy(dir_disk, dir_cwd);
	}
    }


	/* TAPE�ǥ��쥯�ȥ�����ꤹ�� */

    s = getenv("QUASI88_TAPE_DIR");		/* $(QUASI88_TAPE_DIR) */
    if (s && strlen(s) < OSD_MAX_FILENAME) {
	strcpy(dir_tape, s);
    } else {
	if (parse_tilda(home, TAPE_DIR, dir_tape, OSD_MAX_FILENAME) == 0) {
	    strcpy(dir_tape, dir_cwd);
	}
    }


	/* SNAP�ǥ��쥯�ȥ�����ꤹ�� */

    s = getenv("QUASI88_SNAP_DIR");		/* $(QUASI88_SNAP_DIR) */
    if (s && strlen(s) < OSD_MAX_FILENAME) {
	strcpy(dir_snap, s);
    } else {
	strcpy(dir_snap, dir_cwd);
    }


	/* STATE�ǥ��쥯�ȥ�����ꤹ�� */

    s = getenv("QUASI88_STATE_DIR");		/* $(QUASI88_STATE_DIR) */
    if (s && strlen(s) < OSD_MAX_FILENAME) {
	strcpy(dir_state, s);
    } else {
	if (state && strlen(state) < OSD_MAX_FILENAME) {
	    strcpy(dir_state, state);
	} else {
	    strcpy(dir_state, dir_cwd);
	}
    }


	/* ��������ǥ��쥯�ȥ�����ꤹ�� */

    s = g_cfg;
    if (s && strlen(s) < OSD_MAX_FILENAME) {
	strcpy(dir_g_cfg, s);
    } else {
	strcpy(dir_g_cfg, "");
    }


	/* ��������ǥ��쥯�ȥ�����ꤹ�� */

    s = l_cfg;
    if (s && strlen(s) < OSD_MAX_FILENAME) {
	strcpy(dir_l_cfg, s);
    } else {
	strcpy(dir_l_cfg, "");
    }



    if (g_cfg) free(g_cfg);
    if (l_cfg) free(l_cfg);
    if (state) free(state);

    return TRUE;
}


/*
 * path �� ~ �ǻϤޤäƤ����顢 home ���֤������� result_path �˳�Ǽ���롣
 *	���餫����ͳ�ǳ�Ǽ�Ǥ��ʤ��ä��顢�����֤���
 */

static int parse_tilda(const char *home, const char *path,
		       char *result_path, int result_size)
{
    int  i;
    char *buf;

    if (home           &&
	home[0] == '/' &&	/* home �� / �ǻϤޤäƤ��ơ�   */
	path[0] == '~') {	/* path �� ~ �ǻϤޤäƤ�����	*/

	buf = (char *)malloc(strlen(home) + strlen(path) + 2);
	if (buf == NULL)
	    return FALSE;

	if (path[1] == '/'  ||		/* path �� ~/ �� ~/xxx �� ~ �ξ�� */
	    path[1] == '\0') {

	    sprintf(buf, "%s%s%s", home, "/", &path[1]);

	} else {			/* path �� ~xxx �� ~xxx/ �ξ�� */

	    strcpy(buf, home);			/* home ����Ǹ�Υǥ��쥯 */
	    i = strlen(buf) - 1;		/* �ȥ��������ڤ���  */

	    while (0<=i && buf[i] == '/') {i--;}  /* ������ / �����ƥ����å� */
	    while (0<=i && buf[i] != '/') {i--;}  /* / �ʳ������ƥ����å�    */
	    while (0<=i && buf[i] == '/') {i--;}  /* ����� / �����ƥ����å� */
						  /*   (���������åפ���     */
	    buf[i+1] = '\0';			  /*    ���ޤä��� / �ˤʤ�) */

	    strcat(buf, "/");
	    strcat(buf, &path[1]);
	}

	osd_path_normalize(buf, result_path, result_size);

	free(buf);
	return TRUE;

    } else {			/* home �� / �ǻϤޤ�ʤ��� path �� ~ �ǡġ� */

	if (strlen(path) < (size_t)result_size) {

	    strcpy(result_path, path);
	    return TRUE;

	} else {
	    return FALSE;
	}
    }
}



/*
 *	�ǥ��쥯�ȥ� dname �����뤫�����å���̵����к�롣
 *		���������顢�����֤�
 */
static int make_dir(const char *dname)
{
    struct stat sb;

    if (stat(dname, &sb)) {

	if (errno == ENOENT) {			/* �ǥ��쥯�ȥ�¸�ߤ��ʤ� */

	    if (mkdir(dname, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH)) { /*mode==0775*/
		fprintf(stderr, "error: can't make dir %s\n", dname);
		return FALSE;
	    } else {
		printf("make dir \"%s\"\n", dname);
	    }

	} else {				/* ����¾�ΰ۾� */
	    return FALSE;
	}

    } else {					/* �ǥ��쥯�ȥꤢ�ä� */

	if (! S_ISDIR(sb.st_mode)) {			/* �Ȼפä���ե�����*/
	    fprintf(stderr, "error: not exist dir %s\n", dname);
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
    if (dir_rom)   free(dir_rom);
    if (dir_disk)  free(dir_disk);
    if (dir_tape)  free(dir_tape);
    if (dir_snap)  free(dir_snap);
    if (dir_state) free(dir_state);
    if (dir_g_cfg) free(dir_g_cfg);
    if (dir_l_cfg) free(dir_l_cfg);
}
