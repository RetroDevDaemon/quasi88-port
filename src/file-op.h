/*****************************************************************************/
/* �ե��������								     */
/*									     */
/*	QUASI88 �Υե��������ϡ����٤Ƥ��Υإå��ˤ���������ؿ�(�ޥ���)�� */
/*	��ͳ���ƹԤ��롣						     */
/*									     */
/*****************************************************************************/

#ifndef FILE_OP_H_INCLUDED
#define FILE_OP_H_INCLUDED


/****************************************************************************
 * ������ѥ�̾�κ���Ĺ
 *****************************************************************************/

#ifndef	OSD_MAX_FILENAME
#define	OSD_MAX_FILENAME	(1024)
#endif


/****************************************************************************
 * �ե��������Ķ��ν����
 *
 * int	osd_file_config_init(void)
 *	�Ƽ�ե���������˻��Ѥ��롢�ǥ��쥯�ȥ�̾�ʤɤ������Ԥ������δؿ���
 *	�ƤӽФ��ʹߡ����Υإå��˽񤫤�Ƥ���ؿ����ѿ������Ѳ�ǽ�Ȥʤ롣
 *
 *	���δؿ��ϡ� config_init() �����ˤƺǽ�˸ƤӽФ���롣
 *	���ｪλ���Ͽ��򡢰۾ｪλ���ϵ����֤���
 *	(�����֤�����硢 config_init() �⥨�顼�Ȥʤ�)
 *
 * void	osd_file_config_exit(void)
 *	osd_file_config_init() �θ����դ���Ԥ���(���ݤ����꥽�����β����ʤ�)
 *	���δؿ��ϡ���λ����1������ƤӽФ���롣
 *
 *****************************************************************************/
int	osd_file_config_init(void);
void	osd_file_config_exit(void);



/****************************************************************************
 * �Ƽ�ե�����̾ (�����Х��ѿ�)
 *
 *	��ư�塢�ؿ� quasi88() ��ƤӽФ����ˡ������ץ󤷤����ե�����̾��
 *	���ꤷ�Ƥ������������פξ��ϡ���ʸ���� (�ޤ��� NULL) �򥻥åȡ�
 *	�ʤ����ؿ� quasi88() �θƤӽФ��ʸ�ϡ��ѹ����ʤ����� !
 *
 *	char file_disk[2][QUASI88_MAX_FILENAME]
 *		�ɥ饤�� 1: / 2: �˥��åȤ���ǥ��������᡼���ե�����̾��
 *		Ʊ��ʸ���� (�ե�����̾) �����åȤ���Ƥ�����ϡ�ξ�ɥ饤��
 *		�ˡ�Ʊ���ǥ��������᡼���ե�����򥻥åȤ��뤳�Ȥ��̣���롣
 *
 *	int image_disk[2]
 *		�ǥ��������᡼�������åȤ���Ƥ����硢���᡼�����ֹ档
 *		0��31 �ʤ� 1��32���ܡ� -1 �ʤ�С���ưŪ���ֹ���꿶�롣
 *
 *	int readonly_disk[2]
 *		���ʤ顢�꡼�ɥ���꡼�ǥǥ��������᡼���ե�����򳫤���
 *
 *	char *file_compatrom
 *		���Υե������ P88SR.exe �� ROM���᡼���ե�����Ȥ��Ƴ�����
 *		NULL �ξ����̾��̤�� ROM���᡼���ե�����򳫤���
 *
 *	char file_tape[2][QUASI88_MAX_FILENAME]
 *		�ơ��ץ�����/�������ѥե�����̾��
 *
 *	char file_prn[QUASI88_MAX_FILENAME]
 *		�ץ�󥿽����ѥե�����̾��
 *
 *	char file_sin[QUASI88_MAX_FILENAME]
 *	char file_sout[QUASI88_MAX_FILENAME]
 *		���ꥢ��������/�����ѥե�����̾��
 *
 *	char file_state[QUASI88_MAX_FILENAME]
 *		���ơ��ȥ�����/�����ѤΥե�����̾��
 *		��ʸ����ξ��ϡ��ǥե���ȤΥե�����̾�����Ѥ���롣
 *
 *	char file_snap[QUASI88_MAX_FILENAME]
 *		���̥��ʥåץ���å���¸�ѤΥե�����Υ١���̾��
 *		�ºݤ˥��ʥåץ���åȤ���¸������ϡ����Υե�����̾��
 *		4��ο��� + ��ĥ�� ( 0000.bmp �ʤ� ) ��Ϣ�뤷���ե�����̾�ˤʤ�
 *		��ʸ����ξ��ϡ��ǥե���ȤΥե�����̾�����Ѥ���롣
 *
 *	char file_wav[QUASI88_MAX_FILENAME]
 *		������ɽ�����¸�ѤΥե�����Υ١���̾��
 *		�ºݤ˥�����ɽ��Ϥ���¸������ϡ����Υե�����̾��
 *		4��ο��� + ��ĥ�� ( 0000.wav ) ��Ϣ�뤷���ե�����̾�ˤʤ�
 *		��ʸ����ξ��ϡ��ǥե���ȤΥե�����̾�����Ѥ���롣
 *
 *	char *file_rec / char *file_pb
 *		�������Ϥε�Ͽ��/�����ѥե�����̾��
 *		���Ѥ��ʤ��ʤ顢NULL �ˤ��Ƥ�����
 *****************************************************************************/

extern char file_disk[2][QUASI88_MAX_FILENAME];	/*�ǥ��������᡼���ե�����̾*/
extern int  image_disk[2];	   		/*���᡼���ֹ�0��31,-1�ϼ�ư*/
extern int  readonly_disk[2];			/*�꡼�ɥ���꡼�ǳ����ʤ鿿*/

extern char file_tape[2][QUASI88_MAX_FILENAME];	/* �ơ��������ϤΥե�����̾ */
extern char file_prn[QUASI88_MAX_FILENAME];	/* �ѥ�����ϤΥե�����̾ */
extern char file_sin[QUASI88_MAX_FILENAME];	/* ���ꥢ����ϤΥե�����̾ */
extern char file_sout[QUASI88_MAX_FILENAME];	/* ���ꥢ�����ϤΥե�����̾ */

extern char file_state[QUASI88_MAX_FILENAME];	/* ���ơ��ȥե�����̾	    */
extern char file_snap[QUASI88_MAX_FILENAME];	/* ���ʥåץ���åȥ١����� */
extern char file_wav[QUASI88_MAX_FILENAME];	/* ������ɽ��ϥ١�����     */

extern char *file_compatrom;		/* P88SR.exe��ROM��Ȥ��ʤ�ե�����̾*/
extern char *file_rec;			/* �������ϵ�Ͽ����ʤ顢�ե�����̾  */
extern char *file_pb;			/* �������Ϻ�������ʤ顢�ե�����̾  */



/****************************************************************************
 * �Ƽ�ǥ��쥯�ȥ�μ���������
 *	QUASI88 �ϥե��������κݤˡ������Ǽ��������ǥ��쥯�ȥ�̾��
 *	osd_path_join ��Ȥäơ��ѥ�̾���������롣
 *	osd_dir_cwd() �� NULL ���֤��ʤ�����!  ¾�ϡġĤޤ� NULL �Ǥ⤤���䡣
 *
 * const char *osd_dir_cwd  (void)	�ǥե���ȤΥǥ��쥯�ȥ���֤�
 * const char *osd_dir_rom  (void)	ROM���᡼���Τ���ǥ��쥯�ȥ���֤�
 * const char *osd_dir_disk (void)	DISK���᡼���δ��ǥ��쥯�ȥ���֤�
 * const char *osd_dir_tape (void)	TAPE���᡼���δ��ǥ��쥯�ȥ���֤�
 * const char *osd_dir_snap (void)	���ʥåץ���å���¸�ǥ��쥯�ȥ���֤�
 * const char *osd_dir_state(void)	���ơ��ȥե�������¸�ǥ��쥯�ȥ���֤�
 * const char *osd_dir_gcfg (void)	��������ե�����Υǥ��쥯�ȥ���֤�
 * const char *osd_dir_lcfg (void)	��������ե�����Υǥ��쥯�ȥ���֤�
 *
 * int osd_set_dir_???(const char *new_dir)
 *					���줾�졢�ƥǥ��쥯�ȥ�� new_dir ��
 *					���ꤹ�롣���Ԥ������ϵ����֤롣
 *					( ���λ����ƥǥ��쥯�ȥ�ϸ��Τޤ� )
 *****************************************************************************/

const char *osd_dir_cwd  (void);
const char *osd_dir_rom  (void);
const char *osd_dir_disk (void);
const char *osd_dir_tape (void);
const char *osd_dir_snap (void);
const char *osd_dir_state(void);
const char *osd_dir_gcfg (void);
const char *osd_dir_lcfg (void);

int	osd_set_dir_cwd  (const char *new_dir);
int	osd_set_dir_rom  (const char *new_dir);
int	osd_set_dir_disk (const char *new_dir);
int	osd_set_dir_tape (const char *new_dir);
int	osd_set_dir_snap (const char *new_dir);
int	osd_set_dir_state(const char *new_dir);
int	osd_set_dir_gcfg (const char *new_dir);
int	osd_set_dir_lcfg (const char *new_dir);



/****************************************************************************
 * �ե�����̾�˻��Ѥ���Ƥ�����������ɤ����
 *		0 �� ASCII �Τ�
 *		1 �� ���ܸ�EUC
 *		2 �� ���ե�JIS
 *	���ξ���ϡ���˥塼���̤ǥե�����̾��ɽ������ݤˤΤ����Ѥ��롣
 *****************************************************************************/
int	osd_kanji_code(void);



/****************************************************************************
 * �ե��������
 *	QUASI88 �Υե����륢�������ϡ����ưʲ��δؿ����ͳ���ƹԤ��롣
 *	�ºݤ��ɤ߽񤭤ϡ������¸��¦�ˤơ������ˤ��Ƥ�褤��
 *	(��¾���椷���ꡢ���̥ե�����Ȥ����ɤ߽񤭤�����ġĤʤ�)
 *	���ݤʤ顢���Τޤ�ɸ��ؿ��Υ�åѤˤ���Ф褤��
 *	���ʤߤ� errno �ϻ��Ȥ��ʤ���ferror��feof �ʤɤ�ƤӽФ��ʤ���
 *
 *	���δؿ��ˤư����ե�����μ��प��ӥ⡼�ɤϰʲ��ΤȤ��ꡣ
 *	()��ϥե�����η����ǡ����ܤ��Ƥʤ���Τ����ΥХ��ʥ�ǡ����ǰ�����
 *
 *	FTYPE_ROM		ROM���᡼��				"rb"
 *	FTYPE_DISK		�ǥ��������᡼��(D88����)	"r+b" , "rb"
 *	FTYPE_TAPE_LOAD		�����ѥơ��ץ��᡼��(T88/CMT����)	"rb"
 *	FTYPE_TAPE_SAVE		�������ѥơ��ץ��᡼��(CMT����)		"ab"
 *	FTYPE_PRN		�ץ�󥿽��ϥ��᡼��			"ab"
 *	FTYPE_COM_LOAD		���ꥢ�����ϥ��᡼��			"rb"
 *	FTYPE_COM_SAVE		���ꥢ����ϥ��᡼��			"ab"
 *	FTYPE_SNAPSHOT_RAW	���̥��ʥåץ���å�			"wb"
 *	FTYPE_SNAPSHOT_PPM	���̥��ʥåץ���å�(PPM����)		"wb"
 *	FTYPE_SNAPSHOT_BMP	���̥��ʥåץ���å�(BMP����)		"wb"
 *	FTYPE_KEY_PB		�������Ϻ���				"rb"
 *	FTYPE_KEY_REC		�������ϵ�Ͽ				"wb"
 *	FTYPE_STATE_LOAD	�쥸�塼���ѥ��᡼��			"rb"
 *	FTYPE_STATE_SAVE	�����ڥ���ѥ��᡼��			"wb"
 *	FTYPE_CFG		����ե�����			"r", "w","a"
 *	FTYPE_READ		�Х��ʥ�꡼������			"rb"
 *	FTYPE_WRITE		�Х��ʥ�꡼������			"wb"
 *	---------------------------------------------------------------------
 *
 * OSD_FILE *osd_fopen(int type, const char *path, const char *mode)
 *	fopen ��Ʊ�����������ϥե�����ݥ��󥿤������Ի��� NULL ���֤���
 *	type �ϡ��嵭�� FTYPE_xxxx ����ꤹ�롣
 *	type �� FTYPE_DISK �ξ��ǡ����Ǥ�Ʊ���ե����뤬�����Ƥ�����ϡ�
 *	�㤦�⡼�ɤǳ������Ȥ������� NULL ��Ʊ���⡼�ɤǳ������Ȥ�������
 *	���γ����Ƥ���ե�����Υե�����ݥ��󥿤��֤���
 *	(Ʊ���ե����뤬�����Ƥ��뤫�ɤ������ΤǤ�����Τ�)
 *
 * int	osd_fclose(OSD_FILE *stream)
 *	fclose ��Ʊ�������Ի��Ǥ� EOF ���֤��ʤ��Ƥ⤫�ޤ�ʤ���
 *
 * int	osd_fflush(OSD_FILE *stream)
 *	fflush ��Ʊ�������Ի��Ǥ� EOF ���֤��ʤ��Ƥ⤫�ޤ�ʤ���
 *
 * int	osd_fseek(OSD_FILE *stream, long offset, int whence)
 *	fseek ��Ʊ�������Ի��ˤ� -1 ���֤���
 *
 * long	osd_ftell(OSD_FILE *stream)
 *	ftell ��Ʊ�������Ի��ˤ� -1 ���֤���
 *
 * void	osd_rewind(OSD_FILE *stream)
 *	rewind ��Ʊ����
 *
 * size_t osd_fread(void *ptr, size_t size, size_t nobj, OSD_FILE *stream)
 *	fread ��Ʊ�����ɤ߹��ߤ����������֥�å������֤���
 *
 * size_t osd_fwrite(const void *ptr,size_t size,size_t nobj,OSD_FILE *stream)
 *	fwrite ��Ʊ�����񤭹��ߤ����������֥�å������֤���
 *
 * int	osd_fputc(int c, OSD_FILE *stream)
 *	fputc ��Ʊ�������Ի��ˤ� EOF ���֤���
 *
 * int	osd_fgetc(OSD_FILE *stream)
 *	fgetc ��Ʊ�������Ի��ˤ� EOF ���֤���
 *
 * char	*osd_fgets(char *str, int size, OSD_FILE *stream)
 *	fgets ��Ʊ�������Ի��ˤ� NULL ���֤���
 *
 * int	osd_fputs(const char *str, OSD_FILE *stream)
 *	fputs ��Ʊ�������Ի��ˤ� NULL ���֤���
 *
 *****************************************************************************/

enum {
  FTYPE_ROM,
  FTYPE_DISK,
  FTYPE_TAPE_LOAD,
  FTYPE_TAPE_SAVE,
  FTYPE_PRN,
  FTYPE_COM_LOAD,
  FTYPE_COM_SAVE,
  FTYPE_SNAPSHOT_RAW,
  FTYPE_SNAPSHOT_PPM,
  FTYPE_SNAPSHOT_BMP,
  FTYPE_KEY_PB,
  FTYPE_KEY_REC,
  FTYPE_STATE_LOAD,
  FTYPE_STATE_SAVE,
  FTYPE_CFG,
  FTYPE_READ,
  FTYPE_WRITE,
  FTYPE_END
};


/* �ե��������ι�¤�Ρ����Τϡ� file-op.c �ˤ�������� */

typedef struct OSD_FILE_STRUCT		OSD_FILE;



OSD_FILE *osd_fopen(int type, const char *path, const char *mode);
int	osd_fclose (OSD_FILE *stream);
int	osd_fflush (OSD_FILE *stream);
int	osd_fseek  (OSD_FILE *stream, long offset, int whence);
long	osd_ftell  (OSD_FILE *stream);
void	osd_rewind (OSD_FILE *stream);
size_t	osd_fread  (void *ptr, size_t size, size_t nobj, OSD_FILE *stream);
size_t	osd_fwrite (const void *ptr,size_t size,size_t nobj,OSD_FILE *stream);
int	osd_fputc  (int c, OSD_FILE *stream);
int	osd_fgetc  (OSD_FILE *stream);
char	*osd_fgets (char *str, int size, OSD_FILE *stream);
int	osd_fputs  (const char *str, OSD_FILE *stream);



/****************************************************************************
 * �ե�����°���μ���
 *	filename ��Ϳ����줿�ե������°�����֤�
 *
 *	�ƤӽФ����ϡ��֤ä��褿���Ƥˤ��ʲ��ν�����Ԥ���
 *
 *		FILE_STAT_NOEXIST �� �ե�����ο����������ߤ롣
 *						( fopen("filename", "w") ) 
 *		FILE_STAT_DIR     �� �ǥ��쥯�ȥ���ɤ߹��ߤ��ߤ롣
 *						( osd_opendir ) 
 *		FILE_STAT_FILE    �� �ե�������ɤ߽񤭤��ߤ롣
 *				 ( fopen("filename", "r" or "r+" or "a") )
 *
 *	°���������ξ��ϡ������ɤ����褦������??
 *	�Ȥꤢ������ FILE_STAT_FILE ���֤����Ȥˤ��Ƥ������礭������Ϥʤ� ?
 *****************************************************************************/
#define	FILE_STAT_NOEXIST	(0)	/* ¸�ߤ��ʤ�			*/
#define	FILE_STAT_DIR		(1)	/* �ǥ��쥯�ȥ�			*/
#define	FILE_STAT_FILE		(2)	/* �ե�����			*/
int	osd_file_stat(const char *filename);



/****************************************************************************
 * �ǥ��쥯�ȥ����
 *
 * T_DIR_INFO *osd_opendir(const char *filename)
 *	filename �ǻ��ꤵ�줿�ǥ��쥯�ȥ�򳫤������ξ���򥻥åȤ������
 *	�ؤΥݥ��󥿤��֤������Υݥ��󥿤ϡ�osd_readdir��osd_closedir �ˤƻ���
 *	�ƤӽФ�¦��ľ����Ȥ򻲾Ȥ��뤳�ȤϤʤ���
 *
 * const T_DIR_ENTRY *osd_readdir(T_DIR_INFO *dirp)
 *	osd_opendir �ˤƳ������ǥ��쥯�ȥ꤫�饨��ȥ�����ɤ߼�äơ����Ƥ�
 *	T_DIR_ENTRY���Υ���˥��åȤ������Υݥ��󥿤��֤���
 *
 *		typedef	struct {
 *		  int	 type; 		�ե�����μ��� (������)
 *		  char	*name;		�ե�����̾ (����������)
 *		  char	*str;		�ե�����̾ (ɽ����)
 *		} T_DIR_ENTRY;
 *
 *	�ե�����μ���ϡ��ʲ��Τ����줫
 *		FILE_STAT_DIR		osd_opendir �ǳ������Ȥ���ǽ
 *						(�ǥ��쥯�ȥ�ʤ�)
 *		FILE_STAT_FILE		osd_opendir �ǳ������Ȥ�����ʤ�
 *						(�ե�����ʤ�)
 *
 *	name �ϡ�osd_fopen �ǥե�����򳫤��ݤ˻��Ѥ��롢�ե�����̾��
 *	str  �ϡ����̤�ɽ������ݤ�ʸ����ǡ��㤨�Хǥ��쥯�ȥ�̾�� <> �ǰϤࡢ
 *	�Ȥ��ä�������ܤ���ʸ����Ǥ⤫�ޤ�ʤ���(EUC or SJIS �δ������)
 *	�ʤ������̤�ɽ���ϡ� osd_readdir �ˤƼ���������˹Ԥ��Τǡ�ͽ��Ŭ�ڤ�
 *	����ǥ����ƥ��󥰤���Ƥ���Τ��褤��
 *
 * void osd_closedir(T_DIR_INFO *dirp)
 *	�ǥ��쥯�ȥ���Ĥ��롣�ʹ� dirp �ϻȤ�ʤ���
 *
 *****************************************************************************/

/* �ǥ��쥯�ȥ����¤�Ρ����Τϡ�file-op.c �ˤ�������� */

typedef	struct	T_DIR_INFO_STRUCT	T_DIR_INFO;



/* �ǥ��쥯�ȥꥨ��ȥ깽¤�� */

typedef	struct {
  int	 type; 		/* �ե�����μ��� (������)	*/
  char	*name;		/* �ե�����̾ (����������)	*/
  char	*str;		/* �ե�����̾ (ɽ����)		*/
} T_DIR_ENTRY;


T_DIR_INFO	*osd_opendir(const char *filename);
T_DIR_ENTRY	*osd_readdir(T_DIR_INFO *dirp);
void		osd_closedir(T_DIR_INFO *dirp);



/****************************************************************************
 * �ѥ�̾�����
 *	�ѥ�̾�򡢥ǥ��쥯�ȥ�̾�ȥե�����̾��ʬΥ�����ꡢ�Ĥʤ����ꤹ�롣
 *	������δؿ��⡢�����˼��Ԥ������ϵ����֤���
 *
 * int	osd_path_normalize(const char *path, char resolved_path[], int size)
 *	path ��Ϳ����줿�ѥ�̾��Ÿ������ resolved_path �˥��åȤ��롣
 *	resolved_path �ΥХåե��������ϡ� size �Х��ȡ�
 *
 * int	osd_path_split(const char *path, char dir[], char file[], int size)
 *	path ��Ϳ����줿�ѥ�̾��ǥ��쥯�ȥ�̾�ȥե�����̾��ʬ�䤷�ơ�
 *	dir, file �˥��åȤ��롣 dir, file �ΥХåե��������ϤȤ��size �Х���
 *
 * int	osd_path_join(const char *dir, const char *file, char path[], int size)
 *	dir, file Ϳ����줿�ǥ��쥯�ȥ�̾�ȥե�����̾���礷���ѥ�̾��
 *	path �˥��åȤ��롣 path �ΥХåե��������ϡ�size �Х���
 *
 *****************************************************************************/
int osd_path_normalize(const char *path, char resolved_path[], int size);
int osd_path_split(const char *path, char dir[], char file[], int size);
int osd_path_join(const char *dir, const char *file, char path[], int size);



#endif	/* FILE_OP_H_INCLUDED */
