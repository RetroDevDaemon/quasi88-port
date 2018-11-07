#ifndef SCREEN_H_INCLUDED
#define SCREEN_H_INCLUDED



typedef struct {
    unsigned	char	blue;			/* B�̵��� (0��7/255)	*/
    unsigned	char	red;			/* R�̵��� (0��7/255)	*/
    unsigned 	char	green;			/* G�̵��� (0��7/255)	*/
    unsigned 	char	padding;
} PC88_PALETTE_T;


/*
 *	PC-88 Related
 */

extern	PC88_PALETTE_T	vram_bg_palette;	/* �طʥѥ�å�	*/
extern	PC88_PALETTE_T	vram_palette[8];	/* �Ƽ�ѥ�å�	*/

extern	byte	sys_ctrl;			/* OUT[30] SystemCtrl     */
extern	byte	grph_ctrl;			/* OUT[31] GraphCtrl      */
extern	byte	grph_pile;			/* OUT[53] �Ť͹�碌     */

#define	SYS_CTRL_80		(0x01)		/* TEXT COLUMN80 / COLUMN40*/
#define	SYS_CTRL_MONO		(0x02)		/* TEXT MONO     / COLOR   */

#define	GRPH_CTRL_200		(0x01)		/* VRAM-MONO 200 / 400 line*/
#define	GRPH_CTRL_64RAM		(0x02)		/* RAM   64K-RAM / ROM-RAM */
#define	GRPH_CTRL_N		(0x04)		/* BASIC       N / N88     */
#define GRPH_CTRL_VDISP		(0x08)		/* VRAM  DISPLAY / UNDISP  */
#define GRPH_CTRL_COLOR		(0x10)		/* VRAM  COLOR   / MONO    */
#define	GRPH_CTRL_25		(0x20)		/* TEXT  LINE25  / LINE20  */

#define	GRPH_PILE_TEXT		(0x01)		/* �Ť͹�碌 ��ɽ�� TEXT  */
#define	GRPH_PILE_BLUE		(0x02)		/*		       B   */
#define	GRPH_PILE_RED		(0x04)		/*		       R   */
#define	GRPH_PILE_GREEN		(0x08)		/*		       G   */



/*
 *	��������ѥ��
 */

	/* ���躹ʬ���� */

extern	char	screen_dirty_flag[ 0x4000*2 ];	/* �ᥤ���ΰ� ��ʬ����	*/
extern	int	screen_dirty_all;		/* �ᥤ���ΰ� ���蹹��	*/
extern	int	screen_dirty_palette;		/* ������ ����		*/
extern	int	screen_dirty_status;		/* ���ơ������ΰ� ����	*/
extern	int	screen_dirty_status_hide;	/* ���ơ������ΰ� �õ�	*/
extern	int	screen_dirty_status_show;	/* ���ơ������ΰ� �����*/
extern	int	screen_dirty_frame;		/* ���ΰ� ����		*/

#define	screen_set_dirty_flag(x)	screen_dirty_flag[x] = 1
#define	screen_set_dirty_all()		screen_dirty_all = TRUE
#define	screen_set_dirty_palette()	do {				\
					  screen_dirty_palette = TRUE;	\
					  screen_dirty_all = TRUE;	\
					} while(0)
#define	screen_set_dirty_status()	screen_dirty_status = 0xff
#define	screen_set_dirty_status_hide()	screen_dirty_status_hide = TRUE
#define	screen_set_dirty_status_show()	screen_dirty_status_show = TRUE
#define	screen_set_dirty_frame()	screen_dirty_frame = TRUE;


	/* ����¾ */

extern	int	frameskip_rate;		/* ����ɽ���ι����ֳ�		*/
extern	int	monitor_analog;		/* ���ʥ���˥���		*/
extern	int	use_auto_skip;		/* ��ư�ե졼�ॹ���å�		*/



/*
 *	ɽ������
 */

enum {
    SCREEN_INTERLACE_NO = 0,		/* ���󥿡��쥹ɽ�����ʤ�	*/
    SCREEN_INTERLACE_YES = 1,		/* ���󥿡��쥹ɽ������		*/
    SCREEN_INTERLACE_SKIP = -1		/* 1�饤�󤪤�ɽ������		*/
};
extern	int	use_interlace;		/* ���󥿡��졼��ɽ��		*/

extern	int	use_half_interp;	/* ���̥�����Ⱦʬ��������֤��� */

enum {					/* ���̥�����			*/
    SCREEN_SIZE_HALF,			/*		320 x 200	*/
    SCREEN_SIZE_FULL,			/*		640 x 400	*/
#ifdef	SUPPORT_DOUBLE
    SCREEN_SIZE_DOUBLE,			/*		1280x 800	*/
#endif
    SCREEN_SIZE_END
};
extern	int	screen_size;		/* ���̥���������		*/

extern	int	use_fullscreen;		/* ������ɽ������		*/

extern	double	mon_aspect;		/* ��˥����Υ����ڥ�����	*/

extern	int	status_fg;		/* ���ơ��������ʿ�		*/
extern	int	status_bg;		/* ���ơ������طʿ�		*/

extern	int	show_status;		/* ���ơ�����ɽ��̵ͭ		*/


/*
 *
 */

enum {
    SHOW_MOUSE = 0,
    HIDE_MOUSE = 1,
    AUTO_MOUSE = 2
};
extern	int	hide_mouse;		/* �ޥ����򱣤����ɤ���		*/
enum {
    UNGRAB_MOUSE = 0,
    GRAB_MOUSE   = 1
};
extern	int	grab_mouse;		/* ����֤��뤫�ɤ���		*/

extern	int	use_swcursor;		/* ��˥塼���ѥ�������ɽ�����롩*/
extern	int	now_swcursor;		/* �������ѥ�������ɽ����?	*/


/*
 *	ɽ���ǥХ����ѥ��
 */

#define	STATUS_HEIGHT	(20)

extern	int	WIDTH;			/* ����Хåե���������		*/
extern	int	HEIGHT;			/* ����Хåե��ĥ�����		*/
extern	int	DEPTH;			/* ���ӥåȿ�	(8/16/32)	*/
extern	int	SCREEN_W;		/* ���̲������� (320/640/1280)	*/
extern	int	SCREEN_H;		/* ���̽ĥ����� (200/400/800)	*/

extern	int	SCREEN_DX;		/* ������ɥ�����ȡ�		*/
extern	int	SCREEN_DY;		/* ���̥��ꥢ����ȤΥ��ե��å�	*/

extern	char	*screen_buf;		/* ����Хåե���Ƭ		*/
extern	char	*screen_start;		/* ������Ƭ			*/

extern	char	*status_buf;		/* ���ơ��������� ��Ƭ		*/
extern	char	*status_start[3];	/* ���ơ��������� ��Ƭ		*/
extern	int	status_sx[3];		/* ���ơ��������襵����		*/
extern	int	status_sy[3];




extern	Ulong	color_pixel[16];		/* ��������		*/
extern	Ulong	color_half_pixel[16][16];	/* ���䴰���ο�������	*/
extern	Ulong	black_pixel;			/* ���ο�������		*/
enum {						/* ���ơ������˻Ȥ���	*/
    STATUS_BG,					/*	�طʿ�(��)	*/
    STATUS_FG,					/*	���ʿ�(��)	*/
    STATUS_BLACK,				/*	����		*/
    STATUS_WHITE,				/*	��		*/
    STATUS_RED,					/*	�ֿ�		*/
    STATUS_GREEN,				/*	�п�		*/
    STATUS_COLOR_END
};
extern	Ulong	status_pixel[STATUS_COLOR_END];	/* ���ơ������ο�������	*/



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
                          WIDTH
	 ������������������������������������������
	�������������������������������������������� ��
	��              ��                        �� ��
	��              ��SCREEN_DY               �� ��
	��              ��                        �� ��
	��������������������������������������    �� ��
	��   SCREEN_DX  ��  ��              ��    �� ��
	��              ����������������������    �� ��HEIGHT
	��              ��  ��   SCREEN_W   ��    �� ��
	��              ��  ��              ��    �� ��
	��              ��  ��SCREEN_H      ��    �� ��
	��              ��  ��              ��    �� ��
	��              ����������������������    �� ��
	��                                        �� ��
	�������������������������������������������� ��
	�����ơ�����0 �����ơ�����1 �����ơ�����2 �� ��STATUS_HEIGHT
	�������������������������������������������� ��
	    ���ơ�����0��2�Υ�������Ψ�ϡ� 1:3:1

	screen_buf	����Хåե�����Ρ���Ƭ�ݥ���
	WIDTH		����Хåե�����Ρ����ԥ������
	HEIGHT		        ��          �ĥԥ������

	screen_size	���̥�����
	screen_start	���̥Хåե��Ρ���Ƭ�ݥ���
	SCREEN_W	���̥Хåե��Ρ����ԥ������ (320/640/1280)
	SCREEN_H	      ��        �ĥԥ������ (200/400/800)

	DEPTH		������ (�Хåե��Υӥå�����8/16/32)

	status_buf	���ơ������Хåե�����Ρ���Ƭ�ݥ���
	status_start[3]	���ơ����� 0��2 �ΥХåե��Ρ���Ƭ�ݥ���
	status_sx[3]		��		      ���ԥ������
	status_sy[3]		��		      �ĥԥ������


	�� ������ɥ�ɽ���ξ�硢
		WIDTH * (HEIGHT + STATUS_HEIGHT) �Υ������ǡ�
		������ɥ����������ޤ���
			(���ơ�������ɽ���ʤ顢 WIDTH * HEIGHT)

	�� ������ɽ���ξ�硢
		SCREEN_SX * (SCREEN_SY + STATUS_HEIGHT) �ʾ�Υ�������
		�����̲����ޤ���
			(���ơ�������ɽ���ʤ顢������ʬ�Ϲ����ɤ�Ĥ֤�)

   - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

/***********************************************************************
 * ���̽����ν��������λ
 *	screen_init()	���̤��������롣��ư���˸ƤФ�롣
 *	screen_exit()	�夫���Ť����롣��λ���˸ƤФ�롣
 ************************************************************************/
int	screen_init(void);
void	screen_exit(void);


/***********************************************************************
 * �⡼���ڤ��ؤ����Ρ��Ƽ������
 *	�����ꥢ�������褹��ɬ�פ�����Τǡ����ν����򤹤롣
 * 	grab_mouse �� hide_mouse �ʤɤ˴�Ť����ޥ���������򤹤롣
 *	������ԡ��Ȥ䡢���ơ����������ꤹ�롣
 ************************************************************************/
void	screen_switch(void);


/***********************************************************************
 *
 ************************************************************************/
void	screen_attr_mouse_move(void);
void	screen_attr_mouse_click(void);
int	screen_attr_mouse_debug(void);


/***********************************************************************
 * PC-8801�κǽ�Ū�ʿ����������
 ************************************************************************/
void	screen_get_emu_palette(PC88_PALETTE_T pal[16]);
void	screen_get_menu_palette(PC88_PALETTE_T pal[16]);


/***********************************************************************
 * ����
 ************************************************************************/
void	screen_update(void);		/* ����   (1/60sec��)  */
void	screen_update_immidiate(void);	/* ¨���� (��˥�����) */


/***********************************************************************
 * �ե졼�ॹ���å�
 ************************************************************************/
void	frameskip_blink_reset(void);	/* ���ǽ��� �ƽ����		*/
void	frameskip_counter_reset(void);	/* �ե졼�ॹ���å� �ƽ����	*/
void	frameskip_check(int on_time);	/* �ե졼�ॹ���å� Ƚ��	*/

int	quasi88_cfg_now_frameskip_rate(void);
void	quasi88_cfg_set_frameskip_rate(int rate);



/***********************************************************************
 * HALF���������ο��䴰��ͭ����̵����Ϣ�δؿ�
 ***********************************************************************/
int	quasi88_cfg_can_interp(void);
int	quasi88_cfg_now_interp(void);
void	quasi88_cfg_set_interp(int enable);

/***********************************************************************
 * INTERLACE�������Ϣ�δؿ�
 ***********************************************************************/
int	quasi88_cfg_now_interlace(void);
void	quasi88_cfg_set_interlace(int interlace_mode);

/***********************************************************************
 * ���ơ�����ɽ�������Ϣ�δؿ�
 ***********************************************************************/
int	quasi88_cfg_can_showstatus(void);
int	quasi88_cfg_now_showstatus(void);
void	quasi88_cfg_set_showstatus(int show);

/***********************************************************************
 * ���������ꡦ���̥����������Ϣ�δؿ�
 ***********************************************************************/
int	quasi88_cfg_can_fullscreen(void);
int	quasi88_cfg_now_fullscreen(void);
void	quasi88_cfg_set_fullscreen(int fullscreen);
int	quasi88_cfg_max_size(void);
int	quasi88_cfg_min_size(void);
int	quasi88_cfg_now_size(void);
void	quasi88_cfg_set_size(int new_size);
void	quasi88_cfg_set_size_large(void);
void	quasi88_cfg_set_size_small(void);

/***********************************************************************
 * ???
 ***********************************************************************/
int	quasi88_info_draw_count(void);

#endif	/* SCREEN_H_INCLUDED */
