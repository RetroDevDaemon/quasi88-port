/************************************************************************/
/*									*/
/* ���̤�ɽ��								*/
/*									*/
/************************************************************************/

#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "screen.h"
#include "screen-func.h"
#include "graph.h"

#include "crtcdmac.h"
#include "pc88main.h"

#include "status.h"
#include "suspend.h"

#include "intr.h"
#include "q8tk.h"

#include "pause.h"			/* pause_event_focus_in_when_pause() */



PC88_PALETTE_T	vram_bg_palette;	/* OUT[52/54-5B]		*/
PC88_PALETTE_T	vram_palette[8];	/*		�Ƽ�ѥ�å�	*/

byte	sys_ctrl;			/* OUT[30] SystemCtrl		*/
byte	grph_ctrl;			/* OUT[31] GraphCtrl		*/
byte	grph_pile;			/* OUT[53] �Ť͹�碌		*/



char	screen_dirty_flag[ 0x4000*2 ];		/* �ᥤ���ΰ� ��ʬ����	*/
int	screen_dirty_all = TRUE;		/* �ᥤ���ΰ� ���蹹��	*/
int	screen_dirty_palette = TRUE;		/* ������ ����		*/
int	screen_dirty_status = FALSE;		/* ���ơ������ΰ� ����	*/
int	screen_dirty_status_hide = FALSE;	/* ���ơ������ΰ� �õ�	*/
int	screen_dirty_status_show = FALSE;	/* ���ơ������ΰ� �����*/
int	screen_dirty_frame = TRUE;		/* ���ΰ� ���� 		*/



int	frameskip_rate	= DEFAULT_FRAMESKIP;	/* ����ɽ���ι����ֳ�	*/
int	monitor_analog	= TRUE;			/* ���ʥ���˥���     */
int	use_auto_skip	= TRUE;			/* ��ư�ե졼�ॹ���å�	*/


static	int	do_skip_draw = FALSE;		/* ���󥹥��åפ��뤫?	*/
static	int	already_skip_draw = FALSE;	/* ���󥹥��åפ�����?	*/

static	int	skip_counter = 0;	/* Ϣ³���󥹥��åפ�����	*/
static	int	skip_count_max = 15;	/* ����ʾ�Ϣ³�����åפ�����
						��ö������Ū�����褹��	*/

static	int	frame_counter = 0;	/* �ե졼�ॹ���å��ѤΥ�����	*/



static	int	blink_ctrl_cycle   = 1;	/* ��������ɽ���ѤΥ�����	*/
static	int	blink_ctrl_counter = 0;	/*              ��		*/





/*CFG*/	int	hide_mouse = FALSE;	/* �ޥ����򱣤����ɤ���		*/
/*CFG*/	int	grab_mouse = FALSE;	/* ����֤��뤫�ɤ���		*/

/*CFG*/	int	use_swcursor = FALSE;	/* ��˥塼���ѥ�������ɽ��̵ͭ	*/
	int	now_swcursor;		/* �������ѥ�������ɽ����?	*/




/*CFG*/	int	use_interlace = 0;		/* ���󥿡��졼��ɽ��	*/

static	int	enable_half_interp = FALSE;	/* HALF��������ֲ�ǽ���ݤ� */
/*CFG*/	int	use_half_interp = TRUE;		/* HALF��������֤���	    */
static	int	now_half_interp = FALSE;	/* ���ߡ����䴰��ʤ鿿	    */



typedef	struct{				/* ���̥������Υꥹ��		*/
    int		w,  h;
} SCREEN_SIZE_TABLE;

static const SCREEN_SIZE_TABLE screen_size_tbl[ SCREEN_SIZE_END ] =
{
    /*  w     h   */
    {  320,  200, },		/* SCREEN_SIZE_HALF	320x200	*/
    {  640,  400, },		/* SCREEN_SIZE_FULL	640x400	*/
#ifdef	SUPPORT_DOUBLE
    { 1280,  800, },		/* SCREEN_SIZE_DOUBLE	1280x800*/
#endif
};


static	int	screen_size_max = SCREEN_SIZE_END - 1; /*�ѹ���ǽ�ʺ��祵����*/
static	int	screen_size_min = SCREEN_SIZE_HALF;    /*�ѹ���ǽ�ʺǾ�������*/
/*CFG*/	int	screen_size	= SCREEN_SIZE_FULL;    /*���̥���������      */
static	int	now_screen_size;		       /*�ºݤΡ����̥�����  */


static	int	enable_fullscreen = 0;	/* ������ɽ����ǽ���ɤ���	*/
/*CFG*/	int	use_fullscreen = FALSE;	/* ������ɽ������		*/
static	int	now_fullscreen = FALSE;	/* ���ߡ�������ɽ����ʤ鿿	*/


/*CFG*/	double	mon_aspect = 0.0;	/* ��˥����Υ����ڥ�����	*/


int	WIDTH  = 0;			/* ����Хåե���������		*/
int	HEIGHT = 0;			/* ����Хåե��ĥ�����		*/
int	DEPTH  = 8;			/* ���ӥåȿ�	(8 or 16 or 32)	*/
int	SCREEN_W = 0;			/* ���̲������� (320/640/1280)	*/
int	SCREEN_H = 0;			/* ���̽ĥ����� (200/400/800)	*/

int	SCREEN_DX = 0;			/* ������ɥ�����ȡ�		*/
int	SCREEN_DY = 0;			/* ���̥��ꥢ����ȤΥ��ե��å�	*/

char	*screen_buf;			/* ����Хåե���Ƭ		*/
char	*screen_start;			/* ������Ƭ			*/

static	int	screen_bx;		/* �ܡ�����(��)���� x (�ɥå�)	*/
static	int	screen_by;		/*	   ��	    y (�ɥå�)	*/



/*CFG*/	int	status_fg = 0x000000;	/* ���ơ��������ʿ�		*/
/*CFG*/	int	status_bg = 0xd6d6d6;	/* ���ơ������طʿ�		*/

static	int	enable_status = TRUE;	/* ���ơ�����ɽ����ǽ���ɤ���	*/
/*CFG*/	int	show_status = TRUE;	/* ���ơ�����ɽ��̵ͭ		*/
static	int	now_status = FALSE;	/* ���ߡ����ơ�����ɽ����ʤ鿿	*/

char	*status_buf;			/* ���ơ��������� ��Ƭ		*/
char	*status_start[3];		/* ���ơ��������� ��Ƭ		*/
int	status_sx[3];			/* ���ơ��������襵����		*/
int	status_sy[3];


Ulong	color_pixel[16];			/* ��������		*/
Ulong	color_half_pixel[16][16];		/* ���䴰���ο�������	*/
Ulong	black_pixel;				/* ���ο�������		*/
Ulong	status_pixel[ STATUS_COLOR_END ];	/* ���ơ������ο�������	*/



static	int	screen_write_only;	/* ���̥Хåե��ɽ��ԲĤʤ顢��	*/

static	void	(*draw_start)(void);	/* �������Υ�����Хå��ؿ�	*/
static	void	(*draw_finish)(void);	/* �����Υ�����Хå��ؿ�	*/
static	int	dont_frameskip;		/* �ե졼�ॹ���å׶ػߤʤ顢��	*/



static	int	drawn_count;		/* fps�׻��Ѥ�������������	*/

static	int	broken_mouse;		/* �����ƥ�Υޥ���ɽ�����۾�	*/
static	int	auto_mouse;		/* ���ǡ��ޥ�����ư�Ǳ����⡼��   */
static	int	auto_mouse_timer;	/* ��ư�Ǳ����ޤǤλĤ���֥����ޡ� */
static	int	auto_grab;		/* ���ǡ��ޥ�����ư����֥⡼��     */


enum {
    SETUP_START,
    SETUP_MOVE,
    SETUP_TIMEUP,
    SETUP_CLICK
};
static	void	screen_attr_setup(int stat);

static	void	check_half_interp(void);
static	void	trans_palette(PC88_PALETTE_T syspal[]);
static	void	set_vram2screen_list(void);
static	void	clear_all_screen(void);
static	void	put_image_all(void);

/***********************************************************************
 * ���̽����ν��������λ
 ************************************************************************/
static	const T_GRAPH_SPEC *spec;

static	int	open_window(void);
static	void	open_window_or_exit(void);

int	screen_init(void)
{
    int i;
    int w, h, max, min;

    status_init();			/* ���ơ�����ɽ���Υ�������	*/

    spec = graph_init();
    if (spec == NULL) {
	return FALSE;
    }


    /* spec �ˤ�äơ�������ɥ��κ��硦�Ǿ������������ */

    min = -1;
    for (i = 0; i < SCREEN_SIZE_END; i++) {
	if (i == SCREEN_SIZE_HALF && spec->forbid_half) { continue; }

	if (spec->window_max_width  >= screen_size_tbl[ i ].w &&
	    spec->window_max_height >= screen_size_tbl[ i ].h) {
	    min = i;
	    break;
	}
    }
    max = -1;
    for (i = SCREEN_SIZE_END - 1; i >= 0; i--) {
	if (spec->window_max_width  >= screen_size_tbl[ i ].w &&
	    spec->window_max_height >= screen_size_tbl[ i ].h) {
	    max = i;
	    break;
	}
    }
    if (min < 0 || max < 0 || max < min) {
	if (verbose_proc) printf("  Not found drawable window size (bug?)\n");
	return FALSE;
    }
    screen_size_max = max;
    screen_size_min = min;


    /* spec �ˤ�äơ������̲�ǽ���ɤ�������� */

    if (spec->forbid_half) { i = SCREEN_SIZE_FULL; }
    else                   { i = SCREEN_SIZE_HALF; }

    if (spec->fullscreen_max_width  >= screen_size_tbl[ i ].w &&
	spec->fullscreen_max_height >= screen_size_tbl[ i ].h) {
	enable_fullscreen = TRUE;
    } else {
	enable_fullscreen = FALSE;
    }



    /* screen_size, WIDTH, HEIGHT �˥��ޥ�ɥ饤��ǻ��ꤷ��������ɥ���������
       ���åȺѤߤʤΤǡ�������Ȥ˥ܡ�����(��)�Υ������򻻽Ф��� */

    w = screen_size_tbl[ screen_size ].w;
    h = screen_size_tbl[ screen_size ].h;

    screen_bx = ((MAX(WIDTH,  w) - w) / 2) & ~7;	/* 8���ܿ� */
    screen_by = ((MAX(HEIGHT, h) - h) / 2);

    if (open_window()) {
	clear_all_screen();
	put_image_all();
	return TRUE;
    } else {
	return FALSE;
    }
}


void	screen_exit(void)
{
    graph_exit();
}



/*----------------------------------------------------------------------
 * ������ɥ�������
 *----------------------------------------------------------------------*/
static int added_color;
static unsigned long added_pixel[120+16];

#define	SET_N_COLOR(n, rgb)					\
		color[ n ].red   = ((rgb) >> 16) & 0xff;	\
		color[ n ].green = ((rgb) >>  8) & 0xff;	\
		color[ n ].blue  = ((rgb)      ) & 0xff;	\

/*
  �ʲ����ѿ��˴�Ť������̤����������ʲ����ѿ��򥻥åȤ���
	use_fullscreen, enable_fullscreen		�� now_fullscreen
	screen_size   , screen_size_max, screen_size_min
	show_status					�� now_status
	screen_bx     , screen_by
 */

static	int	open_window(void)
{
    int i, size, found = FALSE;
    int w = 0, h = 0, status_displayable = FALSE;
    const T_GRAPH_INFO *info;


    added_color = 0;

    if (enable_fullscreen == FALSE) {	/* �������ԲĤʤ顢�����̻ؼ��ϵѲ� */
	use_fullscreen = FALSE;
    }


    /* �ե륹���꡼��ɽ������ǽ�ʥ��������礭���ۤ�����õ���Ф� */

    if (use_fullscreen) {
	for (size = screen_size; size >= screen_size_min; size--) {

	    for (i = 0; i < 3; i++) {	/* 3�ѥ�����Υ������ˤƳ�ǧ */

		w = screen_size_tbl[ size ].w;
		h = screen_size_tbl[ size ].h;

		switch (i) {
		case 0:		/* ���ꥵ���� (���ơ������ԲĤ���) */
		    if (screen_bx == 0 && screen_by == 0) { continue; }
		    w += screen_bx * 2;
		    h += screen_by * 2;
		    status_displayable = FALSE;
		    break;

		case 1:		/* ��Ŭ������ + ���ơ����� */
		    if (spec->forbid_status) { continue; }
		    w += (0) * 2;
		    h += STATUS_HEIGHT * 2;
		    status_displayable = TRUE;
		    break;

		case 2:		/* ��Ŭ������ (���ơ������Բ�) */
		    status_displayable = FALSE;
		    break;
		}

		if (w <= spec->fullscreen_max_width &&
		    h <= spec->fullscreen_max_height) {
		    found = TRUE;
		    break;
		}
	    }
	    if (found) break;
	}

	if (found == FALSE) {		/* ɽ����ǽ������̵���ʤ��������Բ� */
	    use_fullscreen = FALSE;
	}
    }


    /* ������ɥ�ɽ����ɽ���ʲ�ǽ���������礭���ۤ�����õ���Ф� */

    if (use_fullscreen == FALSE) {
	for (size = screen_size; size >= screen_size_min; size--) {

	    for (i = 0; i < 4; i++) {	/* 4�ѥ�����Υ������ˤƳ�ǧ */

		w = screen_size_tbl[ size ].w;
		h = screen_size_tbl[ size ].h;

		switch (i) {
		case 0:		/* ���ꥵ���� + ���ơ����� */
		    if (screen_bx == 0 && screen_by == 0) { continue; }
		    w += screen_bx * 2;
		    h += screen_by * 2 + STATUS_HEIGHT;
		    status_displayable = TRUE;
		    break;

		case 1:		/* ���ꥵ���� (���ơ������Բ�) */
		    if (screen_bx == 0 && screen_by == 0) { continue; }
		    w += screen_bx * 2;
		    h += screen_by * 2;
		    status_displayable = FALSE;
		    break;

		case 2:		/* ��Ŭ������ + ���ơ����� */
		    w += 0;
		    h += STATUS_HEIGHT;
		    status_displayable = TRUE;
		    break;

		case 3:		/* ��Ŭ������ (���ơ������Բ�) */
		    status_displayable = FALSE;
		    break;
		}

		if (w <= spec->window_max_width &&
		    h <= spec->window_max_height) {
		    found = TRUE;
		    break;
		}
	    }
	    if (found) break;
	}

	/* ���ơ�����ɽ������ǽ�ʥ����������Ĥ��ä����Ǥ⡢
	   ���ơ�����ɽ�����ʤ��ʤ�С�����ʬ�Υ������򸺤餷�Ƥ��� */
	if (status_displayable) {
	    if (spec->forbid_status || show_status == FALSE) {
		h -= STATUS_HEIGHT;
	    }
	}
    }


    /* ����������ޤä��Τǡ����褤����̤��������� */

    if (found == FALSE) {	/* ����Ϥ��ꤨ�ʤ��ϥ�������ǰ�Τ���� */
	size = screen_size_min;
	w = screen_size_tbl[ size ].w;
	h = screen_size_tbl[ size ].h;
	status_displayable = FALSE;
    }
    now_screen_size = size;

    info = graph_setup(w, h, use_fullscreen, (float)mon_aspect);

    if (info) {

	/* �ե륹���꡼��ǡ����ơ�����ɽ������ǽ�ʥ����������ݤǤ�������ǧ */
	if ((info->fullscreen) &&
	    (info->height >= screen_size_tbl[ size ].h + STATUS_HEIGHT * 2)) {
	    status_displayable = TRUE;
	}

	/* �����˥��ơ�����ɽ������ǽ���ɤ����򡢺ǽ�Ƚ�� */
	if (status_displayable && (spec->forbid_status == FALSE)) {
	    enable_status = TRUE;
	    /* show_status �ϸ����ͤΤޤ� */
	} else {
	    enable_status = FALSE;
	    show_status   = FALSE;
	}

	/* �������ν�����׻� */
	WIDTH      = info->byte_per_line / info->byte_per_pixel;
	if (info->fullscreen) {
	    HEIGHT = info->height;
	} else {
	    HEIGHT = info->height - ((show_status) ? STATUS_HEIGHT : 0);
	}

	SCREEN_W  = screen_size_tbl[ size ].w;
	SCREEN_H  = screen_size_tbl[ size ].h;
	SCREEN_DX = (info->width  - SCREEN_W) / 2;
	SCREEN_DY = (HEIGHT       - SCREEN_H) / 2;

	if (info->fullscreen) {
	    if (enable_status) {
		HEIGHT -= STATUS_HEIGHT;
	    }
	    now_fullscreen = TRUE;
	} else {
	    now_fullscreen = FALSE;
	}

	DEPTH     = info->byte_per_pixel * 8;


	/* �Ȥ��뿧�ο�������å� */
	if        (info->nr_color >= 144) {	/* ���äѤ��Ȥ��� */
	    enable_half_interp = TRUE;

	} else if (info->nr_color >= 24) {	/* Ⱦʬ�⡼�ɤο���֤Ϥ��� */
	    enable_half_interp = FALSE;

	} else if (info->nr_color >= 16) {	/* ���ơ�����ɽ����ޤޤʤ��*/
	    enable_half_interp = FALSE;

	} else {				/* ���󤼤󿧤�­��ʤ� */
	    return FALSE;
	}

	/* HALF���������ο��䴰̵ͭ������ */
	check_half_interp();


	/* �����꡼��Хåե��Ρ����賫�ϰ��֤����� */
	screen_buf = (char *)info->buffer;
	screen_start = &screen_buf[ (WIDTH*SCREEN_DY + SCREEN_DX)
						* info->byte_per_pixel ];

	/* ���ơ������ѤΥХåե��ʤɤ򻻽� */
	status_sx[0] = info->width / 5;
	status_sx[1] = info->width - status_sx[0]*2;
	status_sx[2] = info->width / 5;

	status_sy[0] = 
	status_sy[1] = 
	status_sy[2] = STATUS_HEIGHT - 2;

	status_buf = &screen_buf[ WIDTH * HEIGHT * info->byte_per_pixel ];

	/* ���ơ����������賫�ϰ��֤ϡ��Хåե��� 2�饤�� */
	status_start[0]= status_buf + 2*(WIDTH * info->byte_per_pixel);
	status_start[1]= status_start[0] + (status_sx[0]*info->byte_per_pixel);
	status_start[2]= status_start[1] + (status_sx[1]*info->byte_per_pixel);


	/* ���ơ������Ѥο��ԥ��������� */
	if (info->nr_color >= 24) {
	    PC88_PALETTE_T color[8];
	    unsigned long pixel[8];

	    SET_N_COLOR( STATUS_BG,    status_bg);
	    SET_N_COLOR( STATUS_FG,    status_fg);
	    SET_N_COLOR( STATUS_BLACK, 0x000000 );
	    SET_N_COLOR( STATUS_WHITE, 0xffffff );
	    SET_N_COLOR( STATUS_RED,   0xff0000 );
	    SET_N_COLOR( STATUS_GREEN, 0x00ff00 );
	    SET_N_COLOR( 6, 0x000000 );
	    SET_N_COLOR( 7, 0xffffff );

	    graph_add_color(color, 8, pixel);

	    status_pixel[STATUS_BG   ] = pixel[0];
	    status_pixel[STATUS_FG   ] = pixel[1];
	    status_pixel[STATUS_BLACK] = pixel[2];
	    status_pixel[STATUS_WHITE] = pixel[3];
	    status_pixel[STATUS_RED  ] = pixel[4];
	    status_pixel[STATUS_GREEN] = pixel[5];
	    black_pixel                = pixel[6];

	} else {
	    PC88_PALETTE_T syspal[16];
	    unsigned long black, white;

	    /* syspal[0]��[14] �Ϲ��� syspal[15] ���� */
	    memset(&syspal[0],  0x00, sizeof(PC88_PALETTE_T) * 15);
	    memset(&syspal[15], 0xff, sizeof(PC88_PALETTE_T));

	    /* ���ơ������Ѥο������ݤǤ��ʤ��Τǡ��Ȥꤢ����Ŭ���˿������ */
	    /* syspal[8] �ϥƥ����Ȥι��� syspal[15] �ϥƥ����Ȥ��������   */
	    trans_palette(syspal);

	    black = color_pixel[ 8];
	    white = color_pixel[15];

	    status_pixel[STATUS_BG   ] = black;
	    status_pixel[STATUS_FG   ] = white;
	    status_pixel[STATUS_BLACK] = black;
	    status_pixel[STATUS_WHITE] = white;
	    status_pixel[STATUS_RED  ] = white;
	    status_pixel[STATUS_GREEN] = white;

	    black_pixel                = black;
	}

	now_status = show_status;

	screen_write_only = info->write_only;

	draw_start  = info->draw_start;
	draw_finish = info->draw_finish;
	dont_frameskip = info->dont_frameskip;


	/* ž����ɽ���ؿ��Υꥹ�Ȥ����� */
	set_vram2screen_list();


	/* �����ƥ�Υޥ���ɽ������ */
	broken_mouse = info->broken_mouse;

	screen_switch();

	/* ���ơ����������� */
	status_setup(now_status);

	return TRUE;
    } else {
	return FALSE;
    }
}

/*
 * ������ɥ��κ����������Ԥ����� exit
 */
static	void	open_window_or_exit(void)
{
    if (open_window() == FALSE) {
	fprintf(stderr,"Sorry : Graphic System Fatal Error !!!\n");

	quasi88_exit(1);
    }
}



/*----------------------------------------------------------------------
 * HALF����������֤�̵ͭ���������
 *----------------------------------------------------------------------*/
static	void	check_half_interp(void)
{
    if (now_screen_size == SCREEN_SIZE_HALF &&	/* ���� HALF��������    */
	enable_half_interp  &&			/* �ե��륿��󥰲�ǽ��	*/
	use_half_interp) {			/* ���䴰���Ƥ���ʤ�   */

	now_half_interp = TRUE;

    } else {
	now_half_interp = FALSE;
    }
}







/***********************************************************************
 * �⡼���ڤ��ؤ����Ρ��Ƽ������
 ************************************************************************/
void	screen_switch(void)
{
    char *title;

    /* �����ꥢ��������ν��� */

    screen_set_dirty_frame();		/* ���ΰ� �����(==����) */
    screen_set_dirty_palette();		/* ������ �����(==����) */

    if (now_status) {			/* ���ơ�����ɽ�� */

	screen_set_dirty_status_show();		/* ���ơ������ΰ� ����� */

    } else {				/* ���ơ�������ɽ�� */

	if (now_fullscreen && enable_status) {	/* �����̤ʤ� */
	    screen_set_dirty_status_hide();	/* ���ơ������ΰ� �õ� */
	}					/* ������ɥ��ʤ�����ʤ� */

    }

    frameskip_counter_reset();		/* �������� */


    /* �ޥ�����������ԡ��Ȥ����� */

    screen_attr_setup(SETUP_START);


    /* �����ƥ�Υޥ���ɽ�����۾�ޤ��ϥ�˥塼���ѥ���������Ѥξ��	*/

    if ((broken_mouse) ||
	(! quasi88_is_exec() && /*now_fullscreen &&*/ use_swcursor)) {
	now_swcursor = TRUE;
    } else {
	now_swcursor = FALSE;
    }


    /* ������ɥ������ȥ��ɽ�� */

    if      (quasi88_is_exec())    title = Q_TITLE " ver " Q_VERSION;
    else if (quasi88_is_menu())    title = Q_TITLE " ver " Q_VERSION;
    else if (quasi88_is_pause())   title = Q_TITLE " (PAUSE)";
    else if (quasi88_is_monitor()) title = Q_TITLE " (MONITOR)";
    else                           title = "";

    graph_set_window_title(title);


    /*
      SDL���

      �ե륹���꡼��������� (Win�ˤ�ȯ��)
      ���֥�Хåե��ξ�硢�ޥ����������ɽ������ʤ���
      ���󥰥�Хåե��ǥϡ��ɥ����������ե����ξ�硢
      �ޥ�����ON�ˤ����ִ֡��ޥ�����ɽ�����٤����֤˥��ߤ��Ĥ롩

      ��
      ��˥塼�������ܤ���������ʤΤǡ����եȥ�������������Ǥ��ޤ�����
    */
}

/*-----------------------------------------------------------------------------
  �ޥ���ɽ��������֡�������ԡ��Ȥ����ꤹ�롣

			EMU		MENU/PAUSE
	�ޥ���ɽ��	�� ����ˤ��	����
	�����		�� ����ˤ��	���ʤ�
	������ԡ���	����		����

  �� EMU �ˤ����롢�ޥ���ɽ��������֤ϰʲ��Τ褦�ˤʤ롣

	A �� ����֤��ʤ����ޥ���ɽ������
	B �� ����֤��ʤ����ޥ���ɽ�����ʤ�
	C �� ����֤���	 ���ޥ���ɽ�����ʤ�

	D �� ����֤��ʤ����ޥ���ɽ���ϡ���ư��Ƚ��

    ������ɥ���
	            |�ޥ���ɽ������|�ޥ���ɽ�����ʤ�||�ޥ�����ưɽ��
	------------+--------------+----------------++--------------
	����֤��ʤ�|       A      |        B       ||       D
	------------+--------------+----------------++--------------
	����֤���  |       C   �� |        C       ||       C

	�� ����֤��롿�ޥ���ɽ������ ���ȹ礻��̵��
	   ������ɥ�ü�ǥޥ�������ߤ���Τ��������ޤ����Ω�������ˤʤ���

    �ե륹���꡼���
	            |�ޥ���ɽ������|�ޥ���ɽ�����ʤ�||�ޥ�����ưɽ��
	------------+--------------+----------------++--------------
	����֤��ʤ�|       A   �� |        C    �� ||       C
	------------+--------------+----------------++--------------
	����֤���  |       C   �� |        C       ||       C

	�� ����֤��롿�ޥ���ɽ������ ���ȹ礻��̵��
	   ����ü�ǥޥ�������ߤ���Τ��������ޤ����Ω�������ˤʤ���
	�� ����֤��ʤ����ޥ���ɽ������ ���ȹ礻�ϡ�ͭ��
	   ����ü�ǥޥ�������ߤ���Τ�����Win+SDL �ǥޥ���ǥ����ץ쥤�ξ��
	   �ޥ�������ߤ������٤Υǥ����ץ쥤�˰�ư���뤳�Ȥ�����餷����
	�� ����֤��ʤ����ޥ���ɽ�����ʤ� ���ȹ礻��̵��
	   ����ü�ǥޥ�������ߤ��뤬���ޥ�����ɽ������ʤ��Τǡ��狼��ˤ���

-----------------------------------------------------------------------------*/

/*#define DEBUG_ALL_MOUSE_PATTERN*/  /* �ǥХå�(���ޥ���������ȹ礻�򸡾�) */

#ifdef	DEBUG_ALL_MOUSE_PATTERN
int	screen_attr_mouse_debug(void) { return TRUE; }
#else
int	screen_attr_mouse_debug(void) { return FALSE; }
#endif


#define	AUTO_MOUSE_TIMEOUT	(2 * 60)

/* �ޥ�����ư������ƤӽФ���� */
void	screen_attr_mouse_move(void)
{
    if (auto_mouse) {
	if (auto_mouse_timer == 0) {
	    screen_attr_setup(SETUP_MOVE);
	} else {
	    auto_mouse_timer = AUTO_MOUSE_TIMEOUT;
	}
    }
}

/* �ޥ����Υ���å����˸ƤӽФ���� */
void	screen_attr_mouse_click(void)
{
    if (auto_grab) {
	screen_attr_setup(SETUP_CLICK);
	auto_grab = FALSE;
    }
}

/* 1/60sec��˸ƤӽФ���� */
static	void	screen_attr_update(void)
{
    if (auto_mouse) {
	if (auto_mouse_timer > 0) {
	    if (--auto_mouse_timer == 0) {
		screen_attr_setup(SETUP_TIMEUP);
	    }
	}
    }
}

/* �ޥ���ɽ��������֡�������ԡ��Ȥ����ꤹ�� */
static	void	screen_attr_setup(int stat)
{
    int repeat;		/* �����ȥ�ԡ��Ȥ�̵ͭ	*/
    int mouse;		/* �ޥ���ɽ����̵ͭ	*/
    int grab;		/* ����֤�̵ͭ		*/

    if (stat == SETUP_START) {
	auto_mouse = FALSE;
	auto_mouse_timer = 0;
	auto_grab  = FALSE;
    }


    if (quasi88_is_exec()) {

	repeat = FALSE;

#ifdef	DEBUG_ALL_MOUSE_PATTERN	/* �ǥХå���:���ƤΥޥ���������ȹ礻�򸡾� */
	if      (grab_mouse == UNGRAB_MOUSE){ grab = FALSE; }
	else if (grab_mouse == GRAB_MOUSE)  { grab = TRUE;  }
	else {
	    if      (stat == SETUP_START)   { grab = FALSE; auto_grab = TRUE; }
	    else if (stat == SETUP_CLICK)   { grab = TRUE;  auto_grab = FALSE;}
	    else {
		if (auto_grab)              { grab = FALSE; }
		else                        { grab = TRUE;  }
	    }
	}
	goto DEBUG;
#endif

	if (now_fullscreen) {	/* ������ɽ���ξ�� -------- */

	    if (grab_mouse == UNGRAB_MOUSE &&
		hide_mouse == SHOW_MOUSE) {

		/* ����֤ʤ� && �ޥ������� �ξ��Τߡ������̤�ˤ��� */
		mouse = TRUE;
		grab  = FALSE;

	    } else {

		/* ����֤���or��ư || �ޥ����ʤ�or��ư �ʤ�С��ʲ��Ǹ��� */
		mouse = FALSE;
		grab  = TRUE;

	    }

	} else {		/* ������ɥ�ɽ���ξ�� -------- */

	    if (grab_mouse == GRAB_MOUSE) {

		/* ����֤���ʤ顢�ޥ����Ͼä� */
		grab  = TRUE;
		mouse = FALSE;

	    } else {

		if (grab_mouse == AUTO_MOUSE &&
		    stat == SETUP_CLICK) {

		    /* ��ư����֤ǡ��ܥ��󥯥�å����ϡ��ޥ����ä� */
		    grab  = TRUE;
		    mouse = FALSE;

		    /* �ʲ��ϥ��ꥢ */
		    auto_mouse = FALSE;
		    auto_mouse_timer = 0;
		    auto_grab  = FALSE;

		} else {

		    /* ����֤ʤ��ʤ顢�ޥ�����̵ͭ������ˤ�� */
		    if (grab_mouse == AUTO_MOUSE) {
			auto_grab = TRUE;
		    }

		    grab  = FALSE;

DEBUG:
		    switch (hide_mouse) {
		    case AUTO_MOUSE:
			auto_mouse = TRUE;
			if (stat == SETUP_START ||
			    stat == SETUP_MOVE) {
			    auto_mouse_timer = AUTO_MOUSE_TIMEOUT;
			    mouse = TRUE;
			} else {
			    auto_mouse_timer = 0;
			    mouse = FALSE;
			}
			break;

		    case SHOW_MOUSE:
			mouse = TRUE;
			break;

		    case HIDE_MOUSE:
		    default:
			mouse = FALSE;
			break;
		    }
		}
	    }
	}

    } else {

	repeat = TRUE;
	mouse  = TRUE;
	grab   = FALSE;

	/* �����̥⡼�ɤǡ����եȥ��������������Ȥ��ʤ顢�ޥ����Ͼä� */
	if (/*now_fullscreen &&*/ use_swcursor) mouse = FALSE;
    }

    graph_set_attribute(mouse, grab, repeat);
}






/***********************************************************************
 * HALF���������ο��䴰��ͭ����̵����Ϣ�δؿ�
 ***********************************************************************/

/* ���䴰�β��ݤ��֤� */
int	quasi88_cfg_can_interp(void) { return enable_half_interp; }

/* ���䴰�θ��߾��֤��֤� */
int	quasi88_cfg_now_interp(void) { return use_half_interp; }

/* ���䴰��̵ͭ�����ꤹ�� */
void	quasi88_cfg_set_interp(int enable)
{
    use_half_interp = enable;
    check_half_interp();
    set_vram2screen_list();
    screen_set_dirty_all();		/* �ᥤ���ΰ� �����(==����) */
    frameskip_counter_reset();		/* �������� */
}



/***********************************************************************
 * INTERLACE�������Ϣ�δؿ�
 ***********************************************************************/

/* ���ߤΥ��󥿥졼�����֤��֤� */
int	quasi88_cfg_now_interlace(void) { return use_interlace; }

/* ���󥿥졼�����֤����ꤹ�� */
void	quasi88_cfg_set_interlace(int interlace_mode)
{
    use_interlace = interlace_mode;
    set_vram2screen_list();
    screen_set_dirty_frame();		/* ���ΰ� �����(==����) */
    frameskip_counter_reset();		/* �������� */
}



/***********************************************************************
 * ���ơ�����ɽ�������Ϣ�δؿ�
 ***********************************************************************/

/* ���ơ�����ɽ���β��ݤ��֤� */
int	quasi88_cfg_can_showstatus(void) { return enable_status; }

/* ���ơ�����ɽ���θ��߾��֤��֤� */
int	quasi88_cfg_now_showstatus(void) { return now_status; }

/* ���ơ�������ɽ�������ꤹ�� */
void	quasi88_cfg_set_showstatus(int show)
{
    if (now_status != show) {		/* ���ơ�����ɽ��̵ͭ���Ѥ�ä� */
	show_status = show;

	if (now_fullscreen == FALSE) {	/* ������ɥ��ξ�硦���� */

	    open_window_or_exit();		/* ���̥���������     */

	} else {			/* �����̤ξ�硦���� */

	    now_status = show_status;

	    if (now_status) {
		screen_set_dirty_status_show();	/* ���ơ������ΰ� ����� */
		screen_set_dirty_status();	/* ���ơ������ΰ� ���� */
	    } else {
		screen_set_dirty_status_hide();	/* ���ơ������ΰ� �õ� */
	    }

	}

	status_setup(now_status);		/* ���ơ������ѿ�������� */
    }
}



/***********************************************************************
 * ���������ꡦ���̥����������Ϣ�δؿ�
 ***********************************************************************/

/* �����̤β��ݤ��֤� */
int	quasi88_cfg_can_fullscreen(void) { return enable_fullscreen; }

/* �����̤θ��߾��֤��֤� */
int	quasi88_cfg_now_fullscreen(void) { return now_fullscreen; }

/* �����̤����ؤ����ꤹ�� */
void	quasi88_cfg_set_fullscreen(int fullscreen)
{
    use_fullscreen = fullscreen;

    if (now_fullscreen != use_fullscreen) {
	open_window_or_exit();			/* ���̥���������     */
    }
}

/* ���̥������κ�����֤� */
int	quasi88_cfg_max_size(void) { return screen_size_max; }

/* ���̥������κǾ����֤� */
int	quasi88_cfg_min_size(void) { return screen_size_min; }

/* ���̥������θ��߾��֤��֤� */
int	quasi88_cfg_now_size(void) { return now_screen_size; }

/* ���̥����������ꤹ�� */
void	quasi88_cfg_set_size(int new_size)
{
    screen_size = new_size;

    if (now_screen_size != screen_size) {
	open_window_or_exit();			/* ���̥���������     */
    }
}

/* ���̥��������礭������ */
void	quasi88_cfg_set_size_large(void)
{
    if (++screen_size > screen_size_max) screen_size = screen_size_min;

    open_window_or_exit();			/* ���̥���������     */
}

/* ���̥������򾮤������� */
void	quasi88_cfg_set_size_small(void)
{
    if (--screen_size < screen_size_min) screen_size = screen_size_max;

    open_window_or_exit();			/* ���̥���������     */
}













/*----------------------------------------------------------------------
 * �ѥ�å�����
 *----------------------------------------------------------------------*/
static	void	trans_palette(PC88_PALETTE_T syspal[])
{
    PC88_PALETTE_T color[120+16];
    int i, j, num;

    if (added_color) {
	graph_remove_color(added_color, added_pixel);
	added_color = 0;
    }

		/* 88�Υѥ�å�16��ʬ������ */

    for (i = 0; i < 16; i++) {
	color[i].red   = syspal[i].red;
	color[i].green = syspal[i].green;
	color[i].blue  = syspal[i].blue;
    }
    num = 16;

	/* HALF�������ե��륿��󥰲�ǽ���ϥե��륿�ѥ�å��ͤ�׻� */
	/* (�ե��륿����Ѥˤϡ�120��������) */

    if (now_half_interp) {
	for (i = 0; i < 16; i++) {
	    for (j = i+1; j < 16; j++) {
	       color[num].red   = (color[i].red  >>1) + (color[j].red  >>1);
	       color[num].green = (color[i].green>>1) + (color[j].green>>1);
	       color[num].blue  = (color[i].blue >>1) + (color[j].blue >>1);
	       num++;
	    }
	}
    }

    graph_add_color(color, num, added_pixel);
    added_color = num;

    for (i = 0; i < 16; i++) {
	color_pixel[i] = added_pixel[i];
    }

    if (now_half_interp) {
	for (i = 0; i < 16; i++) {
	    color_half_pixel[i][i] = color_pixel[i];
	}
	num = 16;
	for (i = 0; i < 16; i++) {
	    for (j = i+1; j < 16; j++) {
		color_half_pixel[i][j] = added_pixel[num];
		color_half_pixel[j][i] = color_half_pixel[i][j];
		num++;
	    }
	}
    }
}


/*----------------------------------------------------------------------
 * GVRAM/TVRAM������Хåե���ž������ؿ��Ρ��ꥹ�Ȥ����
 *	���δؿ��ϡ� bpp �� ������ �� ���ե����� ���ѹ����˸ƤӽФ���
 *----------------------------------------------------------------------*/
static	int	(*vram2screen_list[4][4][2])(void);
static	void	(*screen_buf_init_p)(void);

static	int	(*menu2screen_p)(void);

static	void	(*status2screen_p)(int kind, byte pixmap[], int w, int h);
/*static	void	(*status_buf_init_p)(void);*/
static	void	(*status_buf_clear_p)(void);

static	void	set_vram2screen_list(void)
{
    typedef	int		(*V2S_FUNC_TYPE)(void);
    typedef	V2S_FUNC_TYPE	V2S_FUNC_LIST[4][4][2];
    V2S_FUNC_LIST *list = NULL;


    if (DEPTH <= 8) {		/* ----------------------------------------- */

#ifdef	SUPPORT_8BPP
	switch (now_screen_size) {
	case SCREEN_SIZE_FULL:
	    if      (use_interlace == 0) {
		if (screen_write_only)   { list = &vram2screen_list_F_N__8_d; }
		else                     { list = &vram2screen_list_F_N__8; }
	    }
	    else if (use_interlace >  0) { list = &vram2screen_list_F_I__8; }
	    else                         { list = &vram2screen_list_F_S__8; }
	    menu2screen_p = menu2screen_F_N__8;
	    break;
	case SCREEN_SIZE_HALF:
	    if (now_half_interp) { list = &vram2screen_list_H_P__8;
				menu2screen_p = menu2screen_H_P__8; }
	    else                 { list = &vram2screen_list_H_N__8;
				menu2screen_p = menu2screen_H_N__8; }
	    break;
#ifdef	SUPPORT_DOUBLE
	case SCREEN_SIZE_DOUBLE:
	    if (screen_write_only) {
		if      (use_interlace == 0) { list=&vram2screen_list_D_N__8_d; }
		else if (use_interlace >  0) { list=&vram2screen_list_D_I__8_d; }
		else                         { list=&vram2screen_list_D_S__8_d; }
	    } else {
		if      (use_interlace == 0) { list=&vram2screen_list_D_N__8; }
		else if (use_interlace >  0) { list=&vram2screen_list_D_I__8; }
		else                         { list=&vram2screen_list_D_S__8; }
	    }
	    menu2screen_p = menu2screen_D_N__8;
	    break;
#endif
	}
	screen_buf_init_p  = screen_buf_init__8;
	status2screen_p    = status2screen__8;
/*	status_buf_init_p  = status_buf_init__8;*/
	status_buf_clear_p = status_buf_clear__8;
#else
	fprintf(stderr, "Error! This version is not support %dbpp !\n", DEPTH);
	exit(1);
#endif

    } else if (DEPTH <= 16) {	/* ----------------------------------------- */

#ifdef	SUPPORT_16BPP
	switch (now_screen_size) {
	case SCREEN_SIZE_FULL:
	    if      (use_interlace == 0) {
		if (screen_write_only)   { list = &vram2screen_list_F_N_16_d; }
		else                     { list = &vram2screen_list_F_N_16; }
	    }
	    else if (use_interlace >  0) { list = &vram2screen_list_F_I_16; }
	    else                         { list = &vram2screen_list_F_S_16; }
	    menu2screen_p = menu2screen_F_N_16;
	    break;
	case SCREEN_SIZE_HALF:
	    if (now_half_interp) { list = &vram2screen_list_H_P_16;
				menu2screen_p = menu2screen_H_P_16; }
	    else                 { list = &vram2screen_list_H_N_16;
				menu2screen_p = menu2screen_H_N_16; }
	    break;
#ifdef	SUPPORT_DOUBLE
	case SCREEN_SIZE_DOUBLE:
	    if (screen_write_only) {
		if      (use_interlace == 0) { list=&vram2screen_list_D_N_16_d; }
		else if (use_interlace >  0) { list=&vram2screen_list_D_I_16_d; }
		else                         { list=&vram2screen_list_D_S_16_d; }
	    } else {
		if      (use_interlace == 0) { list=&vram2screen_list_D_N_16; }
		else if (use_interlace >  0) { list=&vram2screen_list_D_I_16; }
		else                         { list=&vram2screen_list_D_S_16; }
	    }
	    menu2screen_p = menu2screen_D_N_16;
	    break;
#endif
	}
	screen_buf_init_p  = screen_buf_init_16;
	status2screen_p    = status2screen_16;
/*	status_buf_init_p  = status_buf_init_16;*/
	status_buf_clear_p = status_buf_clear_16;
#else
	fprintf(stderr, "Error! This version is not support %dbpp !\n", DEPTH);
	exit(1);
#endif

    } else if (DEPTH <= 32) {	/* ----------------------------------------- */

#ifdef	SUPPORT_32BPP
	switch (now_screen_size) {
	case SCREEN_SIZE_FULL:
	    if      (use_interlace == 0) {
		if (screen_write_only)   { list = &vram2screen_list_F_N_32_d; }
		else                     { list = &vram2screen_list_F_N_32; }
	    }
	    else if (use_interlace >  0) { list = &vram2screen_list_F_I_32; }
	    else                         { list = &vram2screen_list_F_S_32; }
	    menu2screen_p = menu2screen_F_N_32;
	    break;
	case SCREEN_SIZE_HALF:
	    if (now_half_interp) { list = &vram2screen_list_H_P_32;
				menu2screen_p = menu2screen_H_P_32; }
	    else                 { list = &vram2screen_list_H_N_32;
				menu2screen_p = menu2screen_H_N_32; }
	    break;
#ifdef	SUPPORT_DOUBLE
	case SCREEN_SIZE_DOUBLE:
	    if (screen_write_only) {
		if      (use_interlace == 0) { list=&vram2screen_list_D_N_32_d; }
		else if (use_interlace >  0) { list=&vram2screen_list_D_I_32_d; }
		else                         { list=&vram2screen_list_D_S_32_d; }
	    } else {
		if      (use_interlace == 0) { list=&vram2screen_list_D_N_32; }
		else if (use_interlace >  0) { list=&vram2screen_list_D_I_32; }
		else                         { list=&vram2screen_list_D_S_32; }
	    }
	    menu2screen_p = menu2screen_D_N_32;
	    break;
#endif
	}
	screen_buf_init_p  = screen_buf_init_32;
	status2screen_p    = status2screen_32;
/*	status_buf_init_p  = status_buf_init_32;*/
	status_buf_clear_p = status_buf_clear_32;
#else
	fprintf(stderr, "Error! This version is not support %dbpp !\n", DEPTH);
	exit(1);
#endif

    }

    memcpy(vram2screen_list, list, sizeof(vram2screen_list));
}


static	void	clear_all_screen(void)
{
    if (draw_start) { (draw_start)(); }		/* �����ƥ��¸������������ */

    (screen_buf_init_p)();			/* ���������ꥢ(�ܡ�������) */
    if (now_status) {
	(status_buf_clear_p)();			/* ���ơ������ΰ� �õ� */
    }

    if (draw_finish) { (draw_finish)(); }	/* �����ƥ��¸���������� */
}



/*----------------------------------------------------------------------
 * GVRAM/TVRAM �� screen_buf ��ž������
 *
 *	int method == V_DIF �� screen_dirty_flag �˴�Ť�����ʬ������ž��
 *		   == V_ALL �� ���̤��٤Ƥ�ž��
 *
 *	�����     == -1    �� ž���ʤ� (���̤��Ѳ��ʤ�)
 *		   != -1    �� ��̤��� 8�ӥåȤ��Ĥˡ�x0, y0, x1, y1 ��
 *			       4�Ĥ� unsigned �ͤ����åȤ���롣�����ǡ�
 *				    (x0 * 8, y0 * 2) - (x1 * 8, y1 * 2)
 *			       ��ɽ������ϰϤ���ž�������ΰ�Ȥʤ롣
 *
 *	ͽ�ᡢ set_vram2screen_list �Ǵؿ��ꥹ�Ȥ��������Ƥ�������
 *----------------------------------------------------------------------*/
static	int	vram2screen(int method)
{
    int vram_mode, text_mode;

    if (sys_ctrl & SYS_CTRL_80) {		/* �ƥ����Ȥιԡ��� */
	if (CRTC_SZ_LINES == 25) { text_mode = V_80x25; }
	else                     { text_mode = V_80x20; }
    } else {
	if (CRTC_SZ_LINES == 25) { text_mode = V_40x25; }
	else                     { text_mode = V_40x20; }
    }

    if (grph_ctrl & GRPH_CTRL_VDISP) {		/* VRAM ɽ������ */

	if (grph_ctrl & GRPH_CTRL_COLOR) {		/* ���顼 */
	    vram_mode = V_COLOR;
	} else {
	    if (grph_ctrl & GRPH_CTRL_200) {		/* ��� */
		vram_mode = V_MONO;
	    } else {					/* 400�饤�� */
		vram_mode = V_HIRESO;
	    }
	}

    } else {					/* VRAM ɽ�����ʤ� */

	vram_mode = V_UNDISP;
    }

#if 0
{ /*���ߤɤΥ⡼�ɤ�ɽ����? */
  static int vram_mode0=-1, text_mode0=-1;
  if (text_mode0 != text_mode) {
    if      (text_mode==V_80x25) printf("      80x25\n");
    else if (text_mode==V_80x20) printf("      80x20\n");
    else if (text_mode==V_40x25) printf("      40x25\n");
    else if (text_mode==V_40x20) printf("      40x20\n");
    text_mode0 = text_mode;
  }
  if (vram_mode0 != vram_mode) {
    if      (vram_mode==V_COLOR)  printf("COLOR\n");
    else if (vram_mode==V_MONO)   printf("mono \n");
    else if (vram_mode==V_HIRESO) printf("H=400\n");
    else if (vram_mode==V_UNDISP) printf("-----\n");
    vram_mode0 = vram_mode;
  }
}
#endif

    return (vram2screen_list[ vram_mode ][ text_mode ][ method ])();
}



/*----------------------------------------------------------------------
 * ����ɽ��	�ܡ�����(��)�ΰ衢�ᥤ���ΰ衢���ơ������ΰ�����Ƥ�ɽ��
 *----------------------------------------------------------------------*/
static	void	put_image_all(void)
{
    T_GRAPH_RECT rect[1];

    rect[0].x = 0;
    rect[0].y = 0;
    rect[0].width  = WIDTH;
    rect[0].height = HEIGHT + ((now_status || now_fullscreen) ? STATUS_HEIGHT
			       				      : 0);

    graph_update(1, &rect[0]);
}


/*----------------------------------------------------------------------
 * ����ɽ��	�ᥤ���ΰ�� (x0,y0)-(x1,y1) �� ���ꤵ�줿���ơ������ΰ��ɽ��
 *----------------------------------------------------------------------*/
static	void	put_image(int x0, int y0, int x1, int y1,
			  int st0, int st1, int st2)
{
    int n = 0;
    T_GRAPH_RECT rect[4];

    if (x0 >= 0) {
	if        (now_screen_size == SCREEN_SIZE_FULL) {
	    ;
	} else if (now_screen_size == SCREEN_SIZE_HALF) {
	    x0 /= 2;  x1 /= 2;  y0 /= 2;  y1 /= 2;
	} else  /* now_screen_size == SCREEN_SIZE_DOUBLE */ {
	    x0 *= 2;  x1 *= 2;  y0 *= 2;  y1 *= 2;
	}

	rect[n].x = SCREEN_DX + x0;
	rect[n].y = SCREEN_DY + y0;
	rect[n].width  = x1 - x0;
	rect[n].height = y1 - y0;
	n ++;
    }

    if (now_status || now_fullscreen ){	/* �����̻��ϡ����ơ������õ���� */
	if (st0) {
	    rect[n].x = 0;
	    rect[n].y = HEIGHT;
	    rect[n].width  = status_sx[0];
	    rect[n].height = STATUS_HEIGHT;
	    n ++;
	}
	if (st1) {
	    rect[n].x = status_sx[0];
	    rect[n].y = HEIGHT;
	    rect[n].width  = status_sx[1];
	    rect[n].height = STATUS_HEIGHT;
	    n ++;
	}
	if (st2) {
	    rect[n].x = status_sx[0] + status_sx[1];
	    rect[n].y = HEIGHT;
	    rect[n].width  = status_sx[2];
	    rect[n].height = STATUS_HEIGHT;
	    n ++;
	}
    }

    graph_update(n, &rect[0]);
}








/***********************************************************************
 * ����κݤ˻��Ѥ��롢�ºݤΥѥ�åȾ������� syspal �˥��åȤ���
 ************************************************************************/
void	screen_get_emu_palette(PC88_PALETTE_T pal[16])
{
    int i;

	/* VRAM �� ���顼�ѥ�å�����   pal[0]��[7] */

    if (grph_ctrl & GRPH_CTRL_COLOR) {		/* VRAM ���顼 */

	if (monitor_analog) {
	    for (i = 0; i < 8; i++) {
		pal[i].red   = vram_palette[i].red   * 73 / 2;
		pal[i].green = vram_palette[i].green * 73 / 2;
		pal[i].blue  = vram_palette[i].blue  * 73 / 2;
	    }
	} else {
	    for (i = 0; i < 8; i++) {
		pal[i].red   = vram_palette[i].red   ? 0xff : 0;
		pal[i].green = vram_palette[i].green ? 0xff : 0;
		pal[i].blue  = vram_palette[i].blue  ? 0xff : 0;
	    }
	}

    } else {					/* VRAM ��� */

	if (monitor_analog) {
	    pal[0].red   = vram_bg_palette.red   * 73 / 2;
	    pal[0].green = vram_bg_palette.green * 73 / 2;
	    pal[0].blue  = vram_bg_palette.blue  * 73 / 2;
	} else {
	    pal[0].red   = vram_bg_palette.red   ? 0xff : 0;
	    pal[0].green = vram_bg_palette.green ? 0xff : 0;
	    pal[0].blue  = vram_bg_palette.blue  ? 0xff : 0;
	}
	for (i = 1; i < 8; i++) {
	    pal[i].red   = 0;
	    pal[i].green = 0;
	    pal[i].blue  = 0;
	}

    }


	/* TEXT �� ���顼�ѥ�å�����   pal[8]��[15] */

    if (grph_ctrl & GRPH_CTRL_COLOR) {		/* VRAM ���顼 */

	for (i = 8; i < 16; i++) {			/* TEXT ����ξ��� */
	    pal[i].red   = (i & 0x02) ? 0xff : 0;	/* ��=[8],��=[15] �� */
	    pal[i].green = (i & 0x04) ? 0xff : 0;	/* �Ȥ��Τ�����ʤ�  */
	    pal[i].blue  = (i & 0x01) ? 0xff : 0;
	}

    } else {					/* VRAM ���   */

	if (misc_ctrl & MISC_CTRL_ANALOG) {		/* ���ʥ��ѥ�åȻ�*/

	    if (monitor_analog) {
		for (i = 8; i < 16; i++) {
		    pal[i].red   = vram_palette[i & 0x7].red   * 73 / 2;
		    pal[i].green = vram_palette[i & 0x7].green * 73 / 2;
		    pal[i].blue  = vram_palette[i & 0x7].blue  * 73 / 2;
		}
	    } else {
		for (i = 8; i < 16; i++) {
		    pal[i].red   = vram_palette[i & 0x7].red   ? 0xff : 0;
		    pal[i].green = vram_palette[i & 0x7].green ? 0xff : 0;
		    pal[i].blue  = vram_palette[i & 0x7].blue  ? 0xff : 0;
		}
	    }

	} else {					/* �ǥ�����ѥ�åȻ�*/
	    for (i = 8; i < 16; i++) {
		pal[i].red   = (i & 0x02) ? 0xff : 0;
		pal[i].green = (i & 0x04) ? 0xff : 0;
		pal[i].blue  = (i & 0x01) ? 0xff : 0;
	    }
	}

    }
}



/***********************************************************************
 * ���᡼��ž�� (ɽ��)
 *
 *	���δؿ��ϡ�ɽ�������ߥ� (�� 1/60����) �˸ƤӽФ���롣
 ************************************************************************/
void	screen_update(void)
{
    int i;
    int skip = FALSE;
    int all_area  = FALSE;	/* �����ꥢž���ե饰	*/
    int rect = -1;		/* ����ž���ե饰	*/
    int flag = 0;		/* ���ơ�����ž���ե饰	*/
    PC88_PALETTE_T syspal[16];
    int is_exec = (quasi88_is_exec()) ? TRUE : FALSE;


    screen_attr_update();	/* �ޥ�����ư�Ǳ����ĸƤӽФ���꤬���ޤ��� */


    if (is_exec) {
	profiler_lapse( PROF_LAPSE_BLIT );
    }

    status_update();		/* ���ơ������ΰ�β����ǡ����򹹿� */


    /* �ᥤ���ΰ�ϡ�����򥹥��åפ����礬����Τǡ��ʲ���Ƚ�� */
    /* (��˥塼�ʤɤϡ���� frame_counter==0 �ʤΤǡ��������)   */

    if ((frame_counter % frameskip_rate) == 0) { /* ����λ����褿��       */
						 /* �ʲ��Τ����줫�ʤ����� */
	if (no_wait ||				   /* �������Ȥʤ������     */
	    use_auto_skip == FALSE || 		   /* ��ư�����åפʤ������ */
	    do_skip_draw  == FALSE) {		   /* ���󥹥��å��оݤǤʤ� */

	    skip = FALSE;

	} else {				 /* �ʳ��ϥ����å� */

	    skip = TRUE;

	    /* ���西���ߥ󥰤ʤΤ˥����åפ������ϡ����Τ��Ȥ�Ф��Ƥ��� */
	    already_skip_draw = TRUE;
	}

	/* �����������ǤΥ������ */
	if (is_exec) {
	    if (--blink_ctrl_counter == 0) {
		blink_ctrl_counter = blink_ctrl_cycle;
		blink_counter ++;
	    }
	}
    } else {
	skip = TRUE;
    }


    /* �ᥤ���ΰ�����褹�� (�����åפ��ʤ�) ���ν��� */

    if (skip == FALSE) {

	/* ��ž�� */

	if (screen_dirty_palette) {		/* ���򥷥��ƥ��ž�� */
	    if (quasi88_is_menu()) {
		screen_get_menu_palette(syspal);
	    } else {
		screen_get_emu_palette(syspal);
	    }
	    trans_palette(syspal);
	}

	/* ɬ�פ˱����ơ��ե饰�򥻥å� */

	if (screen_dirty_frame) {
	    screen_set_dirty_all();
	    screen_set_dirty_status();
	    all_area = TRUE;
	}

	if (screen_dirty_palette) {
	    screen_set_dirty_all();
	    screen_dirty_palette = FALSE; 
	}

	/* �ե饰�˱����ơ����� */

	if (draw_start) { (draw_start)(); }	/* �����ƥ��¸������������ */

	if (screen_dirty_frame) {
	    (screen_buf_init_p)();		/* ���������ꥢ(�ܡ�������) */
	    screen_dirty_frame = FALSE;
	    /* �ܡ��������Ϲ����ꡣ���ѹ��ĤȤ���ʤ顢��˿�ž����ɬ�ס� */
	}

	/* VRAMž�� */

	if (quasi88_is_menu()) {

	    if (screen_dirty_all ||
		screen_dirty_flag[0]) {

		if (screen_dirty_all) {
		    /* ΢���̤򥯥ꥢ������Ǻ�ʬ�������ˤʤ� */
		    memset(&menu_screen[menu_screen_current^1],
			   0,
			   sizeof(menu_screen[0]));
		}

		rect = (menu2screen_p)();

		/* ɽ���̤�΢���̤���פ����Ƥ����ʤ��Ȥ��������ʤ�� */
		memcpy(&menu_screen[menu_screen_current^1],
		       &menu_screen[menu_screen_current],
		       sizeof(menu_screen[0]));

		menu_screen_current ^= 1;
		screen_dirty_flag[0] = 0;
		screen_dirty_all = FALSE;
	    }

	} else {

	    /* VRAM�����ե饰 screen_dirty_flag ���㳰����		     */
	    /*		VRAM��ɽ���ξ�硢�����ե饰�ϰ�̵̣���Τǥ��ꥢ���� */
	    /*		400�饤��ξ�硢�����ե饰����̲�Ⱦʬ�ˤ��ĥ����  */

	    if (screen_dirty_all == FALSE) {
		if (! (grph_ctrl & GRPH_CTRL_VDISP)) {
		    /* ��ɽ�� */
		    memset(screen_dirty_flag, 0, sizeof(screen_dirty_flag) / 2);
		}
		if (! (grph_ctrl & (GRPH_CTRL_COLOR|GRPH_CTRL_200))) {
		    /* 400�饤�� */
		    memcpy(&screen_dirty_flag[80*200], screen_dirty_flag, 80*200);
		}
	    }

	    crtc_make_text_attr();	/* TVRAM �� °����������	  */
					/* VRAM/TEXT �� screen_buf ž��	  */
	    rect = vram2screen(screen_dirty_all ? V_ALL : V_DIF);

	    text_attr_flipflop ^= 1;
	    memset(screen_dirty_flag, 0, sizeof(screen_dirty_flag));
	    screen_dirty_all = FALSE;
	}

	if (draw_finish) { (draw_finish)(); }	/* �����ƥ��¸���������� */
    }


    /* ���ơ��������ꥢ�ν��� (���ơ������ϡ�ɽ������¤ꥹ���åפ��ʤ�) */

    if (draw_start) { (draw_start)(); }		/* �����ƥ��¸������������ */

    if (screen_dirty_status_hide) {
	(status_buf_clear_p)();			/* ���ơ������ΰ� �õ� */
	screen_dirty_status_hide = FALSE;
	all_area = TRUE;
    }

    if (screen_dirty_status_show) {
	(status_buf_clear_p)();			/* ���ơ������ΰ� ����� */
	screen_dirty_status_show = FALSE;
	all_area = TRUE;
    }

    if (now_status) {
	if (screen_dirty_status) {
	    flag = screen_dirty_status;
	    for (i = 0; i < 3; i++) {
		if (flag & (1 << i)) {
		    (status2screen_p)(i, status_info[i].pixmap,
				      status_info[i].w, status_info[i].h);
		}
	    }
	    screen_dirty_status = 0;
	}
    }

    if (draw_finish) { (draw_finish)(); }	/* �����ƥ��¸���������� */

    if (is_exec) {
	profiler_video_output(((frame_counter % frameskip_rate) == 0),
			      skip, (all_area || rect != -1));
    }


    if ((is_exec) && (dont_frameskip == FALSE)) {
	++ frame_counter;
    } else {	/* menu,pause,monitor */
	frame_counter = 0;
    }

    if (is_exec) {
	profiler_lapse( PROF_LAPSE_VIDEO );
    }

    if (all_area) {

	put_image_all();
	drawn_count ++;

    } else {
	if (rect != -1) {
	    put_image(((rect >> 24)       ) * 8, ((rect >> 16) & 0xff) * 2,
		      ((rect >>  8) & 0xff) * 8, ((rect      ) & 0xff) * 2,
		      (flag & 1), (flag & 2), (flag & 4));
	    drawn_count ++;
	}
	else if (flag) {
	    put_image(-1, -1, -1, -1,
		      (flag & 1), (flag & 2), (flag & 4));
	}
    }
}

void	screen_update_immidiate(void)
{
    screen_set_dirty_frame();		/* ���ΰ� ���� */
    frameskip_counter_reset();		/* �������� */

    screen_update();			/* ������� */
}

int	quasi88_info_draw_count(void)
{
    return drawn_count;
}













/***********************************************************************
 * �����ƥ।�٥�Ƚ��� EXPOSE / FOCUS-IN / FOCUS-OUT
 ***********************************************************************/

/*======================================================================
 *
 *======================================================================*/
void	quasi88_expose(void)
{
    screen_set_dirty_frame();		/* ���ΰ� ���� */

    frameskip_counter_reset();		/* �������� */
}


/*======================================================================
 *
 *======================================================================*/
void	quasi88_focus_in(void)
{
    if (quasi88_is_pause()) {

	pause_event_focus_in_when_pause();

    }
}

void	quasi88_focus_out(void)
{
    if (quasi88_is_exec()) {

	pause_event_focus_out_when_exec();

    }
}













/***********************************************************************
 * �ƥ��������� (�������뤪��ӡ�ʸ��°��������) �����Υ������
 *	CRTC �� frameskip_rate, blink_cycle ���ѹ�����뤿�Ӥ˸ƤӽФ�
 ************************************************************************/
void	frameskip_blink_reset(void)
{
    int wk;

    wk = blink_cycle / frameskip_rate;

    if (wk == 0 ||
	! (blink_cycle-wk*frameskip_rate < (wk+1)*frameskip_rate-blink_cycle))
	wk++;
  
    blink_ctrl_cycle = wk;
    blink_ctrl_counter = blink_ctrl_cycle;
}



/***********************************************************************
 * �ե졼�५���󥿽����
 *	���ե졼��ϡ�ɬ��ɽ������롣(�����åפ���ʤ�)
 ************************************************************************/
void	frameskip_counter_reset(void)
{
    frame_counter = 0;
    do_skip_draw = FALSE;
    already_skip_draw = FALSE;
}



/***********************************************************************
 * ��ư�ե졼�ॹ���å׽���		( by floi, thanks ! )
 ************************************************************************/
void	frameskip_check(int on_time)
{
    if (use_auto_skip) {

	if (on_time) {			/* ������˽����Ǥ��� */

	    skip_counter = 0;
	    do_skip_draw = FALSE;	    /* ��������Ȥ��� */

	    if (already_skip_draw) {	    /* ����������������åפ��Ƥ��� */
		frameskip_counter_reset();	/* ����VSYNC�Ƕ������� */
	    }

	} else {			/* ������˽����Ǥ��Ƥ��ʤ� */

	    do_skip_draw = TRUE;	    /* �������襹���å� */

	    skip_counter++;		    /* â���������åפ������ʤ� */
	    if (skip_counter >= skip_count_max) {
		frameskip_counter_reset();	/* ����VSYNC�Ƕ������� */
	    }
	}
    }
}



/***********************************************************************
 * �ե졼�ॹ���åץ졼�Ȥμ���������
 ************************************************************************/
int	quasi88_cfg_now_frameskip_rate(void)
{
    return frameskip_rate;
}
void	quasi88_cfg_set_frameskip_rate(int rate)
{
    char str[32];

    if (rate <= 0) rate = 1;

    if (rate != frameskip_rate) {
	frameskip_rate = rate;

	frameskip_blink_reset();
	frameskip_counter_reset();

	sprintf(str, "FRAME RATE = %2d/sec", 60/rate);
	status_message(1, STATUS_INFO_TIME, str);
	/* �ѹ�������ϡ����Ф餯���̤˥ե졼��졼�Ȥ�ɽ�������� */
    }
}





/***********************************************************************
 * ���ơ��ȥ��ɡ����ơ��ȥ�����
 ************************************************************************/

#define	SID	"SCRN"

static	T_SUSPEND_W	suspend_screen_work[]=
{
    { TYPE_CHAR,	&vram_bg_palette.blue,	},
    { TYPE_CHAR,	&vram_bg_palette.red,	},
    { TYPE_CHAR,	&vram_bg_palette.green,	},

    { TYPE_CHAR,	&vram_palette[0].blue,	},
    { TYPE_CHAR,	&vram_palette[0].red,	},
    { TYPE_CHAR,	&vram_palette[0].green,	},
    { TYPE_CHAR,	&vram_palette[1].blue,	},
    { TYPE_CHAR,	&vram_palette[1].red,	},
    { TYPE_CHAR,	&vram_palette[1].green,	},
    { TYPE_CHAR,	&vram_palette[2].blue,	},
    { TYPE_CHAR,	&vram_palette[2].red,	},
    { TYPE_CHAR,	&vram_palette[2].green,	},
    { TYPE_CHAR,	&vram_palette[3].blue,	},
    { TYPE_CHAR,	&vram_palette[3].red,	},
    { TYPE_CHAR,	&vram_palette[3].green,	},
    { TYPE_CHAR,	&vram_palette[4].blue,	},
    { TYPE_CHAR,	&vram_palette[4].red,	},
    { TYPE_CHAR,	&vram_palette[4].green,	},
    { TYPE_CHAR,	&vram_palette[5].blue,	},
    { TYPE_CHAR,	&vram_palette[5].red,	},
    { TYPE_CHAR,	&vram_palette[5].green,	},
    { TYPE_CHAR,	&vram_palette[6].blue,	},
    { TYPE_CHAR,	&vram_palette[6].red,	},
    { TYPE_CHAR,	&vram_palette[6].green,	},
    { TYPE_CHAR,	&vram_palette[7].blue,	},
    { TYPE_CHAR,	&vram_palette[7].red,	},
    { TYPE_CHAR,	&vram_palette[7].green,	},

    { TYPE_BYTE,	&sys_ctrl,		},
    { TYPE_BYTE,	&grph_ctrl,		},
    { TYPE_BYTE,	&grph_pile,		},

    { TYPE_INT,		&frameskip_rate,	},
    { TYPE_INT,		&monitor_analog,	},
    { TYPE_INT,		&use_auto_skip,		},
/*  { TYPE_INT,		&frame_counter,		}, ����ͤǤ�����ʤ����� */
/*  { TYPE_INT,		&blink_ctrl_cycle,	}, ����ͤǤ�����ʤ����� */
/*  { TYPE_INT,		&blink_ctrl_counter,	}, ����ͤǤ�����ʤ����� */

    { TYPE_INT,		&use_interlace,		},
    { TYPE_INT,		&use_half_interp,	},

    { TYPE_END,		0			},
};


int	statesave_screen(void)
{
    if (statesave_table(SID, suspend_screen_work) == STATE_OK) return TRUE;
    else                                                       return FALSE;
}

int	stateload_screen(void)
{
    if (stateload_table(SID, suspend_screen_work) == STATE_OK) return TRUE;
    else                                                       return FALSE;
}
























/* �ǥХå��Ѥδؿ� */
void attr_misc(int line)
{
int i;

  text_attr_flipflop ^= 1;    
  for(i=0;i<80;i++){
    printf("%02X[%02X] ",
    text_attr_buf[text_attr_flipflop][line*80+i]>>8,
    text_attr_buf[text_attr_flipflop][line*80+i]&0xff );
  }
return;
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][9*80+i]>>8,
    text_attr_buf[text_attr_flipflop][9*80+i]&0xff );
  }
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][10*80+i]>>8,
    text_attr_buf[text_attr_flipflop][10*80+i]&0xff );
  }
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][11*80+i]>>8,
    text_attr_buf[text_attr_flipflop][11*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][12*80+i]>>8,
    text_attr_buf[text_attr_flipflop][12*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][13*80+i]>>8,
    text_attr_buf[text_attr_flipflop][13*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[text_attr_flipflop][14*80+i]>>8,
    text_attr_buf[text_attr_flipflop][14*80+i]&0xff );
  }
#if 0
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][15*80+i]>>8,
    text_attr_buf[0][15*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][16*80+i]>>8,
    text_attr_buf[0][16*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][17*80+i]>>8,
    text_attr_buf[0][17*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][18*80+i]>>8,
    text_attr_buf[0][18*80+i]&0xff );
  }
  printf("\n");
  for(i=0;i<80;i++){
    printf("%c[%02X] ",
    text_attr_buf[0][19*80+i]>>8,
    text_attr_buf[0][19*80+i]&0xff );
  }
  printf("\n");
#endif
}
