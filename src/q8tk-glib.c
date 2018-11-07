/************************************************************************/
/*									*/
/* QUASI88 ��˥塼�� Tool Kit						*/
/*				Graphic lib				*/
/*									*/
/*	Q8TK �� API ( q8tk_XXX() �ؿ� ) �����ǸƤФ��ؿ���		*/
/*	menu_screen[][]������٤��ͤ򥻥åȤ��롣			*/
/*									*/
/************************************************************************/
#include <stdio.h>
#include <string.h>

#include "quasi88.h"
#include "memory.h"		/* has_kanji_rom	*/

#include "q8tk.h"
#include "q8tk-glib.h"

#ifdef	SUPPORT_UTF8
#include "utf16.c"
#endif

/********************************************************/
/* ���						*/
/********************************************************/
int		menu_screen_current;
T_Q8GR_SCREEN	menu_screen[2][ Q8GR_SCREEN_Y ][ Q8GR_SCREEN_X ];
static	int	menu_mouse_x;
static	int	menu_mouse_y;


/********************************************************/
/* �ץ�ȥ�����						*/
/********************************************************/
static	void	q8gr_putchar(int x, int y, int fg, int bg,
			     int reverse, int underline, int c);

static	int	q8gr_strings(int x, int y, int fg, int bg,
			     int reverse, int underline,
			     int cursor_pos, 
			     int code, const char *str, int start, int width);

INLINE	void	q8gr_putc(int x, int y, int fg, int bg, int c)
{
    q8gr_putchar(x, y, fg, bg, FALSE, FALSE,  c);
}

INLINE	int	q8gr_puts(int x, int y, int fg, int bg,
			  int code, const char *str)
{
    return q8gr_strings(x, y, fg, bg, FALSE, FALSE, -1, code, str, 0, 0);
}


/********************************************************/
/*							*/
/********************************************************/
void	q8gr_init(void)
{

    memset(menu_screen, 0, sizeof(menu_screen));
#if 0
    {
	int i, j;
	for (j=0; j<Q8GR_SCREEN_Y; j++) {
	    for (i=0; i<Q8GR_SCREEN_X; i++) {
		menu_screen[ 1 ][ j ][ i ].font_type = FONT_UNUSED;
	    }
	}
    }
#endif
    menu_screen_current = 0;

    q8gr_clear_screen();
}



/********************************************************/
/* menu_screen[][]�򥯥ꥢ				*/
/********************************************************/
void	q8gr_clear_screen(void)
{
    int i, j;
    T_Q8GR_SCREEN *p = &menu_screen[ menu_screen_current ][0][0];

    for (j=0; j<Q8GR_SCREEN_Y; j++) {
	for (i=0; i<Q8GR_SCREEN_X; i++) {
	    p->background = Q8GR_PALETTE_BACKGROUND;
	    p->foreground = Q8GR_PALETTE_FOREGROUND;
	    p->mouse      = FALSE;
	    p->reverse    = FALSE;
	    p->underline  = FALSE;
	    p->font_type  = FONT_ANK;
	    p->addr       = 0;
	    p ++;
	}
    }
    menu_mouse_x = -1;
    menu_mouse_y = -1;

    q8gr_reset_screen_mask();

    q8gr_set_cursor_exist(FALSE);
}



/********************************************************/
/* �����꡼��Υޥ�����				*/
/********************************************************/
static	int	screen_mask_x0, screen_mask_x1;
static	int	screen_mask_y0, screen_mask_y1;



#define	CHECK_MASK_X_FOR(x)	if      ((x) < screen_mask_x0) continue; \
				else if ((x) >=screen_mask_x1) break
#define	CHECK_MASK_Y_FOR(y)	if      ((y) < screen_mask_y0) continue; \
				else if ((y) >=screen_mask_y1) break
#define	CHECK_MASK_X(x)		((x)<screen_mask_x0 || (x)>=screen_mask_x1)
#define	CHECK_MASK_Y(x)		((y)<screen_mask_y0 || (y)>=screen_mask_y1)


void	q8gr_set_screen_mask(int x, int y, int sx, int sy)
{
    screen_mask_x0 = x;
    screen_mask_y0 = y;
    screen_mask_x1 = x + sx;
    screen_mask_y1 = y + sy;
}
void	q8gr_reset_screen_mask(void)
{
    screen_mask_x0 = 0;
    screen_mask_y0 = 0;
    screen_mask_x1 = Q8GR_SCREEN_X;
    screen_mask_y1 = Q8GR_SCREEN_Y;
}




/********************************************************/
/* �ե��������ѤΥ����꡼�����				*/
/********************************************************/
Q8tkWidget	dummy_widget_window; /* ���ߡ���̤���ѥ������å� */
static	void	*focus_screen[ Q8GR_SCREEN_Y ][ Q8GR_SCREEN_X ];


void	q8gr_clear_focus_screen(void)
{
    int i, j;

    for (j=0; j<Q8GR_SCREEN_Y; j++)
	for (i=0; i<Q8GR_SCREEN_X; i++)
	    focus_screen[j][i] = NULL;
}

void	q8gr_set_focus_screen(int x, int y, int sx, int sy, void *p)
{
    int i, j;

    if (p) {
	for (j=y; j<y+sy; j++) {
	    CHECK_MASK_Y_FOR(j);
	    for (i=x; i<x+sx; i++) {
		CHECK_MASK_X_FOR(i);
		focus_screen[j][i] = p;
	    }
	}
    }
}
void	*q8gr_get_focus_screen(int x, int y)
{
    if (0 <= x && x < Q8GR_SCREEN_X &&
	0 <= y && y < Q8GR_SCREEN_Y) {
	return focus_screen[y][x];
    } else {
	return NULL;
    }
}
int	q8gr_scan_focus_screen(void *p)
{
    int i, j;

    for (j=0; j<Q8GR_SCREEN_Y; j++) {
	for (i=0; i<Q8GR_SCREEN_X; i++) {
	    if (focus_screen[j][i] == p) return TRUE;
	}
    }
    return FALSE;
}





/*----------------------------------------------------------------------
 * (���������)������ɥ���(�ȥ���)�ܥ��󡦥ե졼�ࡦ���ץ�����˥塼
 *----------------------------------------------------------------------
 *        ��
 *   ��  �������������������������� 	���夫����������äƤ���褦��
 *       ��                    ���� 	Ω�δ��������
 *       ��                    ��sy 
 *       ��                    ���� 
 *       �������������������������� 
 *        ����������sx����������
 *----------------------------------------------------------------------*/
static	void	draw_normal_box(int x, int y, int sx, int sy, int shadow_type)
{
    int i,j,c,fg;
    int light, shadow;

    switch (shadow_type) {
    case Q8TK_SHADOW_NONE:			/* �����ʤ� */
#if 0	/* �����ʤ����뤿�ᡢ�طʤ�Ʊ���������� */
	light  = Q8GR_PALETTE_BACKGROUND;
	shadow = Q8GR_PALETTE_BACKGROUND;
	break;
#else	/* ���ä��Τ��ȡ����褷�ʤ� */
	return;
#endif
    case Q8TK_SHADOW_IN:			/* ���Τ��ؤ���Ǥ��� */
	light  = Q8GR_PALETTE_SHADOW;
	shadow = Q8GR_PALETTE_LIGHT;
	break;
    case Q8TK_SHADOW_OUT:			/* ���Τ�����夬�äƤ��� */
	light  = Q8GR_PALETTE_LIGHT;
	shadow = Q8GR_PALETTE_SHADOW;
	break;
    case Q8TK_SHADOW_ETCHED_IN:			/* �Ȥ����ؤ���Ǥ��� */
	light  = Q8GR_PALETTE_LIGHT;
	shadow = Q8GR_PALETTE_LIGHT;
	break;
    case Q8TK_SHADOW_ETCHED_OUT:		/* �Ȥ�������夬�äƤ��� */
    default:
	light  = Q8GR_PALETTE_SHADOW;
	shadow = Q8GR_PALETTE_SHADOW;
	break;
    }

    for (j=y; j<y+sy; j++) {
	CHECK_MASK_Y_FOR(j);
	if (j == y || j == y+sy-1) {
	    for (i=x; i<x+sx; i++) {
		CHECK_MASK_X_FOR(i);

		if        (i == x      && j == y     ) {	/* ���� �� */
		    c  = Q8GR_G_7;
		    fg = light;
		} else if (i == x+sx-1 && j == y     ) {	/* ���� �� */
		    c  = Q8GR_G_9;
		    fg = shadow;
		} else if (i == x      && j == y+sy-1) {	/* ���� �� */
		    c  = Q8GR_G_1;
		    fg = light;
		} else if (i == x+sx-1 && j == y+sy-1) {	/* ���� �� */
		    c  = Q8GR_G_3;
		    fg = shadow;
		} else {
		    c  = Q8GR_G__;
		    if (j == y) fg = light;			/* ��   �� */
		    else        fg = shadow;			/* ��   �� */
		}

		q8gr_putc(i, j, fg, Q8GR_PALETTE_BACKGROUND, c);
	    }
	} else {
	    for (i=x; i<x+sx; i++) {
		CHECK_MASK_X_FOR(i);

		if (i == x || i == x+sx-1) {
		    c  = Q8GR_G_I;
		    if (i == x) fg = light;			/* ��   �� */
		    else        fg = shadow;			/* ��   �� */
		} else {					/* ��      */
		    c  = (Uint)' ';
		    fg = Q8GR_PALETTE_FOREGROUND;
		}

		q8gr_putc(i, j, fg, Q8GR_PALETTE_BACKGROUND, c);
	    }
	}
    }
}



void	q8gr_draw_window(int x, int y, int sx, int sy, int shadow_type,
			 void *p)
{
    draw_normal_box(x, y, sx, sy, shadow_type);

    if (p) {
	q8gr_set_focus_screen(x, y, sx, sy, p);
    }
}

void	q8gr_draw_button(int x, int y, int sx, int sy, int condition, void *p)
{
    int shadow_type;
    if (condition == Q8TK_BUTTON_OFF) shadow_type = Q8TK_SHADOW_OUT;
    else                              shadow_type = Q8TK_SHADOW_IN;

    draw_normal_box(x, y, sx, sy, shadow_type);

    q8gr_set_focus_screen(x, y, sx, sy, p);
}

void	q8gr_draw_frame(int x, int y, int sx, int sy, int shadow_type,
			int code, const char *str, void *p)
{
    int fg = (p) ? Q8GR_PALETTE_FOREGROUND : Q8GR_PALETTE_WHITE;
    int bg =       Q8GR_PALETTE_BACKGROUND;

    draw_normal_box(x, y, sx, sy, shadow_type);

    q8gr_puts(x+1, y, fg, bg, code, str);
}

void	q8gr_draw_scrolled_window(int x, int y, int sx, int sy,
				  int shadow_type, void *p)
{
    draw_normal_box(x, y, sx, sy, shadow_type);

#if 0
    q8gr_set_focus_screen(x, y, sx, sy, p);
    if (sx >= 2  &&  sy >= 2) {
	q8gr_set_focus_screen(x+1, y+1, sx-2, sy-2, p);
    }
#endif
}



/*----------------------------------------------------------------------
 * �����å��ܥ���
 *----------------------------------------------------------------------
 *        ��
 *   ��     ��  ʸ�������ʬ
 *        ������
 *	  �ܥ���
 *----------------------------------------------------------------------*/
void	q8gr_draw_check_button(int x, int y, int condition, void *p)
{
    int fg_edge  = (p) ? Q8GR_PALETTE_FOREGROUND : Q8GR_PALETTE_WHITE;
    int bg_edge  =       Q8GR_PALETTE_BACKGROUND;
    int fg_check = (p) ? Q8GR_PALETTE_FONT_FG    : Q8GR_PALETTE_WHITE;
    int bg_check = (p) ? Q8GR_PALETTE_FONT_BG    : Q8GR_PALETTE_SCALE_BAR;

    q8gr_putc(x  , y, fg_edge, bg_edge, Q8GR_L_RIGHT);

#if 1	/* �����å����򢣤�ɽ�� */
    q8gr_putc(x+1, y, fg_check, bg_check,
		 ((condition == Q8TK_BUTTON_OFF) ? Q8GR_B_UL : Q8GR_B_BOX));
#else	/* �����å�����ߤ�ɽ�� */
    q8gr_putchar(x+1, y, fg_check, bg_check, FALSE, TRUE,
		 ((condition == Q8TK_BUTTON_OFF) ? Q8GR_B_UL : Q8GR_B_X));
#endif
    
    q8gr_putc(x+2, y, fg_edge, bg_edge, Q8GR_L_LEFT);



    if (p) {
#if 0	/* �����å��ܥ������Τߡ�����å���ȿ������ */

	q8gr_set_focus_screen(x+1, y, 1, 1, p);

#else	/* ��٥��Ҥ˻��ľ�硢���Υ�٥�򥯥�å����Ƥ�ȿ������ */

	if (((Q8tkWidget *)p)->child &&
	    ((Q8tkWidget *)p)->child->type == Q8TK_TYPE_LABEL &&
	    ((Q8tkWidget *)p)->child->visible &&
	    ((Q8tkWidget *)p)->child->sensitive &&
	    ((Q8tkWidget *)p)->child->name) {

	    int len = q8gr_strlen(((Q8tkWidget *)p)->child->code,
				  ((Q8tkWidget *)p)->child->name);

	    q8gr_set_focus_screen(x+1, y, 1+1+len, 1, p);
	} else {
	    q8gr_set_focus_screen(x+1, y, 1, 1, p);
	}
#endif
    }
}


/*----------------------------------------------------------------------
 * �饸���ܥ���
 *----------------------------------------------------------------------
 *        ��
 *   ��     ��  ʸ�������ʬ
 *        ������
 *	  �ܥ���
 *----------------------------------------------------------------------*/
void	q8gr_draw_radio_button(int x, int y, int condition, void *p)
{
    int fg = (p) ? Q8GR_PALETTE_FOREGROUND : Q8GR_PALETTE_WHITE;
    int bg =       Q8GR_PALETTE_BACKGROUND;

    q8gr_putc(x  , y, fg, bg, ' ');

    q8gr_putc(x+1, y, fg, bg,
	      ((condition == Q8TK_BUTTON_OFF) ? Q8GR_B_OFF : Q8GR_B_ON));

    q8gr_putc(x+2, y, fg, bg, ' ');

    if (p) {
#if 0	/* �饸���ܥ������Τߡ�����å���ȿ������ */

	q8gr_set_focus_screen(x+1, y, 1, 1, p);

#else	/* ��٥��Ҥ˻��ľ�硢���Υ�٥�򥯥�å����Ƥ�ȿ������ */

	if (((Q8tkWidget *)p)->child &&
	    ((Q8tkWidget *)p)->child->type == Q8TK_TYPE_LABEL &&
	    ((Q8tkWidget *)p)->child->visible &&
	    ((Q8tkWidget *)p)->child->sensitive &&
	    ((Q8tkWidget *)p)->child->name) {

	    int len = q8gr_strlen(((Q8tkWidget *)p)->child->code,
				  ((Q8tkWidget *)p)->child->name);

	    q8gr_set_focus_screen(x+1, y, 1+1+len, 1, p);
	} else {
	    q8gr_set_focus_screen(x+1, y, 1, 1, p);
	}
#endif
    }
}



/*----------------------------------------------------------------------
 * �Ρ��ȥ֥å�
 *----------------------------------------------------------------------
 *        ��
 *   ��  ��������������������    ��
 *       �����֨����֨����֨�	 ��
 *       ��    ��������������������
 *       ��                    ��sy
 *       ��                    ����
 *       ��                    ����
 *       ��������������������������
 *        ����������sx����������
 *----------------------------------------------------------------------*/
void	q8gr_draw_notebook(int x, int y, int sx, int sy,
			   Q8tkWidget *notebook, void *p)
{
    struct notebook_draw *draw = &notebook->stat.notebook.draw;

    if (p) {
	draw->drawing = TRUE;
	draw->x  = x;
	draw->y  = y;
	draw->x0 = x;
	draw->x1 = x+sx-1;
	draw->selected = FALSE;
    } else {
	draw->drawing = FALSE;
    }

    /* �����Υ�����ʬ�ʳ��Ρ�������ɥ������� */
    q8gr_draw_button(x, y+2, sx, sy-2, Q8TK_BUTTON_OFF, NULL);
}
void	q8gr_draw_notepage(int code, const char *tag,
			   int select_flag, int active_flag,
			   Q8tkWidget *notebook, void *p)
{
    int i,len;
    int fg = (p) ? Q8GR_PALETTE_FOREGROUND : Q8GR_PALETTE_WHITE;
    int bg =       Q8GR_PALETTE_BACKGROUND;
    int light  = Q8GR_PALETTE_LIGHT;
    int shadow = Q8GR_PALETTE_SHADOW;
    int focus_x;
    struct notebook_draw *draw = &notebook->stat.notebook.draw;


    if (draw->drawing == FALSE) return;

    if (select_flag) p = NULL;	/* ������ڡ����Υ����ϡ��ե��������ʤ� */

    focus_x = draw->x;

    len = q8gr_strlen(code, tag);

    if (select_flag) {

	/* ���򤷤��Ρ��ȥڡ����Υ�����ʬ��ɽ�� */

	q8gr_strings(draw->x+1, draw->y+1, fg, bg,
		     FALSE, active_flag, -1, code, tag, 0, 0);

	q8gr_putc(draw->x, draw->y,   light, bg, Q8GR_C_7);
	q8gr_putc(draw->x, draw->y+1, light, bg, Q8GR_G_I);
	if (draw->x == draw->x0)
	    q8gr_putc(draw->x, draw->y+2, light, bg, Q8GR_G_I);
	else
	    q8gr_putc(draw->x, draw->y+2, light, bg, Q8GR_G_3);

	draw->x ++;
	for (i=0; i<len; i++, draw->x++) {
	    q8gr_putc(draw->x, draw->y,   light, bg, Q8GR_G__);
	    q8gr_putc(draw->x, draw->y+2, light, bg, ' ');
	}

	q8gr_putc(draw->x, draw->y,   shadow, bg, Q8GR_C_9);
	q8gr_putc(draw->x, draw->y+1, shadow, bg, Q8GR_G_I);
	if (draw->x == draw->x1)
	    q8gr_putc(draw->x, draw->y+2, shadow, bg, Q8GR_G_I);
	else
	    q8gr_putc(draw->x, draw->y+2, light,  bg, Q8GR_G_1);
	draw->x ++;

	draw->selected = TRUE;

    } else if (draw->selected == FALSE) {

	/* ���򤷤��Ρ��ȥڡ�����꺸�Υڡ����Ρ�������ʬ��ɽ�� */

	q8gr_strings(draw->x+1, draw->y+1, fg, bg,
		     FALSE, active_flag, -1, code, tag, 0, 0);

	q8gr_putc(draw->x, draw->y,   shadow, bg, Q8GR_C_7);
	q8gr_putc(draw->x, draw->y+1, shadow, bg, Q8GR_G_I);
	if (draw->x == draw->x0)
	    q8gr_putc(draw->x, draw->y+2, light, bg, Q8GR_G_4);
	else
	    q8gr_putc(draw->x, draw->y+2, light, bg, Q8GR_G_2);

	draw->x ++;
	for (i=0; i<len; i++, draw->x++) {
	    q8gr_putc(draw->x, draw->y, shadow, bg, Q8GR_G__);
	}

    } else {

	/* ���򤷤��Ρ��ȥڡ�����걦�Υڡ����Ρ�������ʬ��ɽ�� */

	q8gr_strings(draw->x, draw->y+1, fg, bg,
		     FALSE, active_flag, -1, code, tag, 0, 0);

	for (i=0; i<len; i++, draw->x++) {
	    q8gr_putc(draw->x, draw->y, shadow, bg, Q8GR_G__);
	}

	q8gr_putc(draw->x, draw->y,   shadow, bg, Q8GR_C_9);
	q8gr_putc(draw->x, draw->y+1, light,  bg, Q8GR_G_I);
	if (draw->x == draw->x1)
	    q8gr_putc(draw->x, draw->y+2, shadow, bg, Q8GR_G_6);
	else
	    q8gr_putc(draw->x, draw->y+2, light,  bg, Q8GR_G_2);
	draw->x ++;

    }


    q8gr_set_focus_screen(focus_x, draw->y, draw->x - focus_x, 2, p);
}


/*----------------------------------------------------------------------
 * ��ľ����ʿ���ڤ���
 *----------------------------------------------------------------------
 *        ��             ��
 *   ��   ����        �� ��������������������
 *        ����           ��������width ������
 *        ��height
 *        ����
 *        ����
 *----------------------------------------------------------------------*/
void	q8gr_draw_vseparator(int x, int y, int height)
{
    int j;
    if (! CHECK_MASK_X(x)) {
	for (j=y; j<y+height; j++) {
	    CHECK_MASK_Y_FOR(j);
	    q8gr_putc(x, j, Q8GR_PALETTE_FOREGROUND, Q8GR_PALETTE_BACKGROUND,
		      Q8GR_G_I);
	}
    }
}
void	q8gr_draw_hseparator(int x, int y, int width)
{
    int	i;
    if (! CHECK_MASK_Y(y)) {
	for (i=x; i<x+width; i++) {
	    CHECK_MASK_X_FOR(i);
	    q8gr_putc(i, y, Q8GR_PALETTE_FOREGROUND, Q8GR_PALETTE_BACKGROUND,
		      Q8GR_G__);
	}
    }
}

/*----------------------------------------------------------------------
 * ����ȥ�
 *----------------------------------------------------------------------
 *          ��
 *          ����width ��
 *   y      ʸ�������ʬ
 *
 *----------------------------------------------------------------------*/
void	q8gr_draw_entry(int x, int y, int width, int code, const char *text,
			int disp_pos, int cursor_pos, void *p)
{
    int fg = (p) ? Q8GR_PALETTE_FONT_FG : Q8GR_PALETTE_BACKGROUND;
    int bg = (p) ? Q8GR_PALETTE_FONT_BG : Q8GR_PALETTE_SCALE_BAR;

    q8gr_strings(x, y, fg, bg, 
		 FALSE, FALSE,
		 cursor_pos, code, text, disp_pos, width);

    /* �Խ���ǽ�ʾ��Τ�ȿ������ */
    if (p && (((Q8tkWidget*)p)->stat.entry.editable)) {
	q8gr_set_focus_screen(x, y, width, 1, p);
    }
}


/*----------------------------------------------------------------------
 * �����
 *----------------------------------------------------------------------
 *        ��
 *   ��   ʸ�������ʬ  ��		���������ɽ�����롣
 *        ����width ��������		ʸ����� entry �Ȥ�������ɽ������
 *                     ���
 *----------------------------------------------------------------------*/
void	q8gr_draw_combo(int x, int y, int width, int active, void *p)
{
#if 0	/* �����������ʸ��(����ȿž)��ɽ�� */
    int fg = (active) ? Q8GR_PALETTE_FONT_FG : Q8GR_PALETTE_FOREGROUND;
    int bg =            Q8GR_PALETTE_BACKGROUND;

    if (p == NULL) fg = Q8GR_PALETTE_SCALE_BAR;

    q8gr_putchar(x+width,  y, fg, bg, TRUE, FALSE, ' ');
    q8gr_putchar(x+width+1,y, fg, bg, TRUE, FALSE, Q8GR_A_D);
    q8gr_putchar(x+width+2,y, fg, bg, TRUE, FALSE, ' ');

    q8gr_set_focus_screen(x+width, y, 3, 1, p);


#else	/* ������򥰥�ե��å�ʸ��2ʸ��(��)��ɽ�� */
    int fg = (active) ? Q8GR_PALETTE_FONT_FG : Q8GR_PALETTE_FOREGROUND;
    int bg =            Q8GR_PALETTE_BACKGROUND;

    if (p == NULL) fg = Q8GR_PALETTE_SCALE_BAR;

    q8gr_putc(x+width,  y, fg, bg, 0xe6);
    q8gr_putc(x+width+1,y, fg, bg, 0xe7);

    q8gr_set_focus_screen(x+width, y, 2, 1, p);
#endif


    /* ����ȥ������Խ��ԲĤξ�硢����ȥ����򥯥�å����Ƥ�ȿ������ */
    if (p &&
	(((Q8tkWidget*)p)->stat.combo.entry->stat.entry.editable == FALSE)) {
	q8gr_set_focus_screen(x, y, width, 1, p);
    }
}


/*----------------------------------------------------------------------
 * �ꥹ�ȥ����ƥ�
 *----------------------------------------------------------------------
 *        ��
 *   ��   ʸ�������ʬ
 *        ����width ��
 *----------------------------------------------------------------------*/
void	q8gr_draw_list_item(int x, int y, int width, int active,
			    int reverse, int underline,
			    int code, const char *text, void *p)
{
    int fg = (active) ? Q8GR_PALETTE_FONT_FG : Q8GR_PALETTE_FOREGROUND;
    int bg = (p)      ? Q8GR_PALETTE_FONT_BG : Q8GR_PALETTE_BACKGROUND;

    if (p == NULL) fg = Q8GR_PALETTE_WHITE;

    q8gr_strings(x, y, fg, bg,
		 reverse, underline,
		 -1, code, text, 0, width);

    q8gr_set_focus_screen(x, y, width, 1, p);
}


/*----------------------------------------------------------------------
 * ��ʿ�������롦��ľ��������
 *----------------------------------------------------------------------
 *        ��             ��
 *   ��   ����        �� ��������������������
 *        ����           ��������length������
 *        ��height
 *        ����
 *        ����
 *----------------------------------------------------------------------*/
static	void	draw_adjustment(int x, int y, int active, 
				Q8Adjust *adj, void *p)
{
    int i, fg = Q8GR_PALETTE_FOREGROUND;
    if (active) fg = Q8GR_PALETTE_SCALE_ACT;

    adj->x = x;
    adj->y = y;

    if (adj->horizontal) {			/* HORIZONTAL */

	if (adj->arrow) {
	    if (p) {
		q8gr_putc(x, y, fg, Q8GR_PALETTE_SCALE_SLD, Q8GR_A_L);
	    } else {
		q8gr_putc(x, y, fg, Q8GR_PALETTE_SCALE_SLD, ' ');
	    }
	    x++;
	}
	for (i=0; i<adj->length; i++) {
	    if (p && i == adj->pos) {
		q8gr_putc(x+i, y, fg, Q8GR_PALETTE_SCALE_SLD, Q8GR_B_B);
	    } else {
		q8gr_putc(x+i, y, fg, Q8GR_PALETTE_SCALE_BAR, ' ');
	    }
	}
	if (adj->arrow) {
	    if (p) {
		q8gr_putc(x+i, y, fg, Q8GR_PALETTE_SCALE_SLD, Q8GR_A_R);
	    } else {
		q8gr_putc(x+i, y, fg, Q8GR_PALETTE_SCALE_SLD, ' ');
	    }
	    x--;
	}
	q8gr_set_focus_screen(x, y, adj->length +(adj->arrow?2:0), 1, p);

    } else {				/* Virtival */

	if (adj->arrow) {
	    if (p) {
		q8gr_putc(x, y, fg, Q8GR_PALETTE_SCALE_SLD, Q8GR_A_U);
	    } else {
		q8gr_putc(x, y, fg, Q8GR_PALETTE_SCALE_SLD, ' ');
	    }
	    y++;
	}
	for (i=0; i<adj->length; i++) {
	    if (p && i == adj->pos) {
		q8gr_putc(x, y+i, fg, Q8GR_PALETTE_SCALE_SLD, Q8GR_B_B);
	    } else {
		q8gr_putc(x, y+i, fg, Q8GR_PALETTE_SCALE_BAR, ' ');
	    }
	}
	if (adj->arrow) {
	    if (p) {
		q8gr_putc(x, y+i, fg, Q8GR_PALETTE_SCALE_SLD, Q8GR_A_D);
	    } else {
		q8gr_putc(x, y+i, fg, Q8GR_PALETTE_SCALE_SLD, ' ');
	    }
	    y--;
	}
	q8gr_set_focus_screen(x, y, 1, adj->length +(adj->arrow?2:0), p);
    }

}



void	q8gr_draw_hscale(int x, int y, Q8Adjust *adj, int active,
			 int draw_value, int value_pos, void *p)
{
    if (draw_value) {
	int  fg = (active) ? Q8GR_PALETTE_FONT_FG : Q8GR_PALETTE_FOREGROUND;
	int  bg =            Q8GR_PALETTE_BACKGROUND;
	int  vx, vy;
	char valstr[8];
	int  len = adj->length + (adj->arrow ? 2 : 0);

	if (p == NULL) fg = Q8GR_PALETTE_WHITE;

	if      (adj->value < -99) strcpy(valstr, "-**");
	else if (adj->value > 999) strcpy(valstr, "***");
	else                       sprintf(valstr, "%3d", adj->value);

	switch (value_pos) {
	case Q8TK_POS_LEFT:
	    vx = x;
	    vy = y;
	    x += 4;
	    break;
	case Q8TK_POS_RIGHT:
	    vx = x + len+1;
	    vy = y;
	    break;
	case Q8TK_POS_TOP:
	    vx = x + ((adj->pos+3 > len) ? (len-3) : adj->pos);
	    vy = y;
	    y += 1;
	    break;
	case Q8TK_POS_BOTTOM:
	default:
	    vx = x + ((adj->pos+3 > len) ? (len-3) : adj->pos);
	    vy = y +1;
	    break;
	}

	q8gr_puts(vx, vy, fg, bg, Q8TK_KANJI_ANK, valstr);

    }

    draw_adjustment(x, y, active, adj, p);
}


void	q8gr_draw_vscale(int x, int y, Q8Adjust *adj, int active,
			 int draw_value, int value_pos, void *p)
{
    if (draw_value) {
	int  fg = (active) ? Q8GR_PALETTE_FONT_FG : Q8GR_PALETTE_FOREGROUND;
	int  bg =            Q8GR_PALETTE_BACKGROUND;
	int  vx, vy;
	char valstr[8];

	if (p == NULL) fg = Q8GR_PALETTE_WHITE;

	if      (adj->value < -99) strcpy(valstr, "-**");
	else if (adj->value > 999) strcpy(valstr, "***");
	else                       sprintf(valstr, "%3d", adj->value);

	switch (value_pos) {
	case Q8TK_POS_LEFT:
	    vx = x;
	    vy = y + adj->pos + (adj->arrow ? 1 : 0);
	    x += 4;
	    break;
	case Q8TK_POS_RIGHT:
	    vx = x+1;
	    vy = y + adj->pos + (adj->arrow ? 1 : 0);
	    break;
	case Q8TK_POS_TOP:
	    vx = x;
	    vy = y;
	    x += 1;
	    y += 1;
	    break;
	case Q8TK_POS_BOTTOM:
	default:
	    vx = x;
	    vy = y + adj->length + (adj->arrow ? 2 : 0);
	    x += 1;
	    break;
	}

	q8gr_puts(vx, vy, fg, bg, Q8TK_KANJI_ANK, valstr);

    }

    draw_adjustment(x, y, active, adj, p);
}





/************************************************************************/
/* ��� menu_screen[][] ��ºݤ�����ؿ�				*/
/*	�ʤ���ɽ���Х��ȤȤϡ�Ⱦ��1�Х��ȡ�����2�Х��Ȥȿ�����		*/
/************************************************************************/


/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
 * ASCII�����ɡ�JIS���������ɤ���¢ROM���ɥ쥹���Ѵ�
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

static void kanji2addr(int code, unsigned int *addr, unsigned int *type)
{
    if (has_kanji_rom == FALSE) {	/* ---- ����ROM�ʤ���� ---- */

	if     (code < 0x100)			/* Ⱦ��ʸ�� */
	{
	    *addr = (code << 3);
	    *type = FONT_ANK;
	}
	else {					/* ����ʸ�� */
	    *addr = 0;
	    *type = FONT_KNJXL;
	}

    } else {				/* ---- ����ROM������ ---- */

	if      (code < 0x100)			/* Ⱦ��ʸ�� */
	{
	    if ((0x20 <= code && code <= 0x7f) ||	/* ASCII    */
		(0xa0 <= code && code <= 0xdf)) {	/* �������� */

		*addr = (code << 3);
		*type = FONT_HALF;

	    } else {			/* ����ȥ���ʸ��,����ե��å�ʸ�� */
		*addr = (code << 2) | 0x0800;
		*type = FONT_QUART;
	    }
	}
	else if (code < 0x3000)			/* ����� */
	{
	    *addr = (((code & 0x0060) << 7) | ((code & 0x0700) << 1)
					    | ((code & 0x001f) << 4));
	    *type = FONT_KNJ1L;
	}
	else if (code < 0x5000)			/* �������� */
	{
	    *addr = (((code & 0x0060) << 9) | ((code & 0x1f00) << 1)
					    | ((code & 0x001f) << 4));
	    *type = FONT_KNJ1L;
	}
	else if (code < 0x7000)			/* ��������� ����1 */
	{
	    *addr = (((code & 0x0060) << 9) | ((code & 0x0f00) << 1)
		   | ((code & 0x001f) << 4) |  (code & 0x2000));
	    *type = FONT_KNJ2L;
	}
	else				/* ��������� ����2 */
	{
	    *addr = (((code & 0x0060) << 7) | ((code & 0x0700) << 1)
					    | ((code & 0x001f) << 4));
	    *type = FONT_KNJ2L;
	}
    }
}



static	int	get_letter(int code, const char **str)
{
    const unsigned char *p = (const unsigned char *)(*str);
    unsigned int h, c = 0;

    switch (code) {
    case Q8TK_KANJI_EUC:
	while ((h = *p ++)) {
	    if (h < 0x80) {			    /* ASCII */
		c = h;
	    } else if (h == 0x8e) {		    /* Ⱦ�ѥ��� */
		if ((c = *p++) == '\0') break;
		/* 0xa0��0xdf�ʳ����ͤǤⵤ�ˤ��ʤ� */
	    } else {				    /* ���� */
		if ((c = *p++) == '\0') break;
		/* �ϰϳ����ͤǤⵤ�ˤ��ʤ� */

		/* EUC �� JIS */
		c = ((h & 0x7f) << 8) | (c & 0x7f);
	    }
	    *str = (const char *)p;
	    return c;
	}
	break;

    case Q8TK_KANJI_SJIS:
	while ((h = *p ++)) {
	    if ((h >= 0x81 && h <= 0x9f) ||	     /* ���� */ 
		(h >= 0xe0 && h <= 0xfc)) {
		if ((c = *p++) == '\0') break;
		/* �ϰϳ����ͤǤⵤ�ˤ��ʤ� */

		/* SJIS �� JIS */
		c = (h << 8) | c;
		if (0xe000 <= c) c -= 0x4000;
		c = ((((c & 0xff00) - 0x8100) << 1) | (c & 0x00ff)) & 0xffff;
		if ((c & 0x00ff) >= 0x80) c -= 1;
		if ((c & 0x00ff) >= 0x9e) c += 0x100 - 0x9e;
		else                      c -= 0x40;
		c += 0x2121;
	    } else {					/* ANK */
		c = h;
	    }
	    *str = (const char *)p;
	    return c;
	}
	break;

    case Q8TK_KANJI_UTF8:
	while ((h = *p ++)) {
	    if (h < 0x80) {			    /* ASCII    0xxxxxxx */
		c = h;
	    } else {
		if (h < 0xc0) {			    /* ��³     10xxxxxx */
		    continue;
		} else if (h < 0xe0) {		    /* 2byte    110xxxxx */
		    c = (h & 0x1f) << 6;
		    if ((h = *p++) == '\0') break;
		    c |= (h & 0x3f);
		} else if (h < 0xf0) {		    /* 3byte    1110xxxx */
		    c = (h & 0x0f) << 12;
		    if ((h = *p++) == '\0') break;
		    c |= (h & 0x3f) << 6;
		    if ((h = *p++) == '\0') break;
		    c |= (h & 0x3f);
		} else if (h < 0xf8) {		    /* 4byte    11110xxx */
		    c = (h & 0x07) << 18;
		    if ((h = *p++) == '\0') break;
		    c |= (h & 0x3f) << 12;
		    if ((h = *p++) == '\0') break;
		    c |= (h & 0x3f) << 6;
		    if ((h = *p++) == '\0') break;
		    c |= (h & 0x3f);
		} else {			    /* 5��6byte          */
		    continue;
		}

		/* Unicode �� JIS */
		if (0xff61 <= c && c <= 0xff9f) {
		    c = c - 0xff61 + 0xa1;
		} else {
#ifdef	SUPPORT_UTF8
		    int i;
		    int found = FALSE;
		    for (i=0; jis2uft16[i][0]; i++) {
			if (jis2uft16[i][1] == c) {
			    found = TRUE;
			    break;
			}
		    }
		    if (found) {
			c = jis2uft16[i][0];
		    } else {
			c = 0x2121;
		    }
#else
		    c = 0x2121;
#endif
		}
	    }
	    *str = (const char *)p;
	    return c;
	}
	break;

    default:
	while ((c = *p ++)) {
	    *str = (const char *)p;
	    return c;
	}
	break;

    }
    return 0;
}




/*----------------------------------------------------------------------
 * ����ʸ����ɽ�� (EUC/SJIS/UTF8/ANK)
 *
 *	x, y, bg, fg       �� ɽ����ɸ ����ӡ����ʿ����طʿ�
 *	reverse, underline �� ���ʤ�ȿž ����� ��������饤��
 *	cursor_pos	   �� ��������ɽ���Х��Ȱ��� (-1�ʤ�̵��)
 *	code, str          �� ʸ�������ɡ�ʸ����
 *	start              �� ɽ�����ϥХ��Ȱ��� (0��)
 *	width              �� ɽ���Х��ȿ��� width==0 �ʤ�����ɽ������
 *	                      ʸ����ɽ���Х��ȿ��������ʤ����϶����ɽ��
 *	�����             �� �ºݤ�ɽ������ʸ���Х��ȿ�
 *			      (width!=0 �ʤ顢width��Ʊ���Ϥ�)
 *----------------------------------------------------------------------*/
static	int	q8gr_strings(int x, int y, int fg, int bg,
			     int reverse, int underline,
			     int cursor_pos, 
			     int code, const char *str, int start, int width)
{
    const char *p = str;
    unsigned int c, type, addr, rev;
    int w, pos = 0;
    int count = 0;


    /* -------- width �Х���ʬ��ɽ�� (width==0�ʤ�����ɽ��) -------- */

    while (*p) {

	c = get_letter(code, &p);
	if (c == '\0') break;


	/* ======== ���賫�ϥ����å�  ======== */

	if (c < 0x100) w = 1;		/* 1�Х���ʸ�� */
	else           w = 2;		/* 2�Х���ʸ�� */
      
	if (count == 0) {		/* ̤���� */

	    if (pos < start && pos + w <= start) {	/* �����ΰ�̤ã */
		pos += w;
		continue;
	    }
	    else if (pos < start) {			/* �����ΰ贳�� */
		c = 0x8e;	/* ���ߡ�ʸ���� */
		w = 1;		/* 1�Х�������  */
	    }
	    else {					/* �����ΰ���ã */
		;
	    }
	}
    
#if 0	/* select_start �� select_end ��ȿž������ (ʸ������ϰϻ���)	*/
	if (0 <= select_start) {
	    if (pos == select_start) {
		reverse = !(reverse);
	    }
	    if (0 <= select_end) {
		if( pos == select_end) {
		    reverse = !(reverse);
		}
	    }
	}
#endif

	rev = reverse;
	if (0 <= cursor_pos) {		/* ����������� */
	    if (pos == cursor_pos) {
		q8gr_set_cursor_exist(TRUE);
		if (q8gr_get_cursor_blink()) {
		    rev = !(rev);
		}
	    }
	}

	pos += w;


	/* ======== ʸ�������ɤ�ե���ȼ��̡����ɥ쥹���Ѵ�  ======== */

	kanji2addr(c, &addr, &type);



	/* ======== menu_screen[][] �˥��å�  ======== */

	if (! CHECK_MASK_Y(y)) {
	    if (! CHECK_MASK_X(x)) {
		menu_screen[menu_screen_current][y][x].background = bg;
		menu_screen[menu_screen_current][y][x].foreground = fg;
		menu_screen[menu_screen_current][y][x].reverse    = rev;
		menu_screen[menu_screen_current][y][x].underline  = underline;
		menu_screen[menu_screen_current][y][x].font_type  = type;
		menu_screen[menu_screen_current][y][x].addr       = addr;
	    }
	    if (type >= FONT_2_BYTE) {
		x++;
		count ++;
		if (width && width <= count)  break;
		if (! CHECK_MASK_X(x)) {
		    menu_screen[menu_screen_current][y][x].background = bg;
		    menu_screen[menu_screen_current][y][x].foreground = fg;
		    menu_screen[menu_screen_current][y][x].reverse    = rev;
		    menu_screen[menu_screen_current][y][x].underline  = underline;
		    menu_screen[menu_screen_current][y][x].font_type  = type + 1;
		    menu_screen[menu_screen_current][y][x].addr       = addr;
		}
	    }
	}
	x++;
	count ++;
	if (width && width <= count) break;
    }


    /* -------- width ������� ;�ä���ʬ�ϥ��ڡ�����ɽ�� -------- */

    if (width) {
	for ( ; count < width; ) {

#if 0	/* select_start �� select_end ��ȿž������ (ʸ������ϰϻ���)	*/
	    if (0 <= select_start) {
		if (pos == select_start) {
		    reverse = !(reverse);
		}
		if (0 <= select_end) {
		    if (pos == select_end) {
			reverse = !(reverse);
		    }
		}
	    }
#endif

	    rev = reverse;
	    if (0 <= cursor_pos) {	/* ����������� */
		if (pos == cursor_pos) {
		    q8gr_set_cursor_exist(TRUE);
		    if (q8gr_get_cursor_blink()) {
			rev = !(rev);
		    }
		}
	    }

	    q8gr_putchar(x, y, fg, bg, rev, underline, ' ');

	    pos ++;
	    x ++;
	    count ++;
	}
    }
    return count;
}

/*----------------------------------------------------------------------
 * ANKʸ�� putc
 *	x, y, bg, fg  �� ɽ����ɸ ����ӡ����ʿ����طʿ�
 *	reverse, unsigned �� ���ʤ�ȿž ����� ��������饤��
 *	c �� ʸ�������� (0x00��0xff)
 *----------------------------------------------------------------------*/
static	void	q8gr_putchar(int x, int y, int fg, int bg,
			     int reverse, int underline, int c)
{
    if (! CHECK_MASK_X(x) && ! CHECK_MASK_Y(y)) {
	menu_screen[menu_screen_current][y][x].background = bg;
	menu_screen[menu_screen_current][y][x].foreground = fg;
	menu_screen[menu_screen_current][y][x].reverse    = reverse;
	menu_screen[menu_screen_current][y][x].underline  = underline;
	menu_screen[menu_screen_current][y][x].font_type  = FONT_ANK;
	menu_screen[menu_screen_current][y][x].addr       = (c&0xff) << 3;
    }
}


/*----------------------------------------------------------------------
 * ʸ�����ɽ���Х���Ĺ���֤�
 *	code, str �� ʸ��������, ʸ����,
 *----------------------------------------------------------------------*/
int	q8gr_strlen(int code, const char *str)
{
    const char *p = str;
    unsigned int h;
    int count = 0;

    while (*p) {

	h = get_letter(code, &p);
	if (h == '\0') break;

	if (h < 0x100) count ++;
	else           count += 2;
    }
    return count;
}


/*----------------------------------------------------------------------
 * ʸ����� ɽ���Х��� pos ����ʸ���Τɤ���ʬ�ˤ����뤫���֤�
 *	code, str, pos �� ʸ��������, ʸ����, �����å�����ɽ���Х��Ȱ���
 *	�����	0 = 1�Х���ʸ��
 *		1 = 2�Х���ʸ������Ⱦ
 *		2 = 2�Х���ʸ���θ�Ⱦ
 *----------------------------------------------------------------------*/
int	q8gr_strchk(int code, const char *str, int pos)
{
    const char *p = str;
    unsigned int h;
    int count = 0;
    int type = 0;

    while (*p) {

	h = get_letter(code, &p);
	if (h == '\0') break;

	if (h < 0x100) type = 0;
	else           type = 1;


	if (pos == count) {
	    return type;
	}
	if (type == 1) {
	    count ++;
	    if (pos == count) {
		return 2;
	    }
	}
	count ++;
    }
    return 0;
}


/*--------------------------------------------------------------
 * ����ȥ���ʸ������
 *	code �� ʸ��������
 *	str  �� ʸ���󡣤��Υ����ľ�ܽ񤭴�����
 *	del_pos �� �������ɽ���Х��Ȱ��֡��ʹߤ�ʸ��������˵ͤ�롣
 *	����� �� ��������Х��ȿ�(Ⱦ��=1������=2������ʤ�=0)
 *--------------------------------------------------------------*/
int	q8gr_strdel(int code, char *str, int del_pos)
{
    char *p = str;
    char *q = p;
    int count = 0;
    unsigned int h;


    if (del_pos < 0) return 0;

    /* ɽ���Х��Ȱ��֤�������ʸ��������ʤ顢����ʸ������Ƭ�Ȥ��� */
    if (q8gr_strchk(code, str, del_pos) == 2) del_pos --;


    while (*p) {

	h = get_letter(code, (const char **)&p);
	if (h == '\0') return 0;

	if (count == del_pos) {
	    memmove(q, p, strlen(p) + 1);
	    if (h < 0x100) return 1;
	    else           return 2;
	}

	if (h < 0x100) count ++;
	else           count += 2;

	q = p;
    }
    return 0;
}



/*--------------------------------------------------------------
 * ����ȥ���ʸ��������
 *	code �� ʸ��������
 *	str  �� ʸ���󡣤��Υ����ľ�ܽ񤭴����� (�������Ͻ�ʬ�ˤ��뤳��)
 *	add_pos �� ��������ɽ���Х��Ȱ��֡��ʹߤ�ʸ����ϸ��ˤ��餹��
 *	add_chr �� ��������ʸ�� (�������Ǥϡ�ASCII�Τ��б�)
 *	����� �� ʸ�������������顢�������ʤ��ä��顢��
 *--------------------------------------------------------------*/
int	q8gr_stradd(int code, char *str, int add_pos, int add_chr)
{
    char *p = str;
    int count = 0;
    unsigned int h;


    if (add_pos < 0) return FALSE;

    /* ɽ���Х��Ȱ��֤�������ʸ��������ʤ顢����ʸ������Ƭ�Ȥ��� */
    if (q8gr_strchk(code, str, add_pos) == 2) add_pos --;


    for (;;) {
	if (count == add_pos) {
	    memmove(p+1, p, strlen(p) + 1);
	    *p = (char)add_chr;
	    return TRUE;
	}

	if (*p == '\0') break;

	h = get_letter(code, (const char **)&p);
	if (h == '\0') break;

	if (h < 0x100) count ++;
	else           count += 2;
    }
    return FALSE;
}



/*--------------------------------------------------------------
 * ʸ����� size �Х���ʬ�����ԡ�
 *	code �� ʸ��������
 *	dst  �� ���ԡ���ʸ����
 *	src  �� ���ԡ���ʸ����
 *	size �� ���ԡ����륵����
 *		¿�Х���ʸ��������ǥ�������ã������硢
 *		����ʸ���ϥ��ԡ�����ʤ���
 *	�� ����ǻ��ꥵ������ã����������ü�� \0 �ˤʤ�ʤ���
 *	   �ޤ������ꥵ�����������ʤ����ꥢ�� \0 �����롣 (strncpy ��Ʊ��)
 *--------------------------------------------------------------*/
void	q8gr_strncpy(int code, char *dst, const char *src, int size)
{
    const char *p = src;
    const char *q = p;
    int len = strlen(src);
    int esize;

    if (size < 0) return;

    /* ��ʬ�ʥ�����������С��ޤ뤴�ȥ��ԡ� */
    if (size >= len) {
	strncpy(dst, src, size);
	return;
    }

    /* ��ʬ�ʥ��������ʤ���С�¿�Х���ʸ���ζ��ڤ��ܤ�õ������ */
    for (;;) {

	if (get_letter(code, &p) == '\0') {
	    /* \0 ���֤���ϡ� p �ϲ���Ƥ���Τǡ�ʸ����������Ȥߤʤ� */
	    p = &src[ len ];
	}

	/* p �� size ��Ķ����С������������С���¾�Х���ʸ������������� */
	if ((p - src) > size) { break; }

	q = p;

	/* size �˰��פ��Ƥ⡢�����������С���¾�Х���ʸ�����ڤ��ܤ����� */
	if ((p - src) == size) { break; }
    }


    esize = q - src;				/* ͭ�������� */
    strncpy(dst, src, esize);			/* ͭ��������ʬ���ԡ� */
    if (esize < size) {
	memset(&dst[esize], 0, size - esize);	/* ͭ���������ʹߤ� 0 ��� */
    }
}




/*--------------------------------------------------------------
 * ʸ�������ɤ�ʤ�Ȥʤ�Ƚ�Ǥ���
 *	buffer �� ʸ���򥻥åȤ����Хåե�
 *	size   �� �Хåե��Υ�����
 *	����� �� ʸ��������
 *	�� buffer �ˤ���ǡ�����size �Х���ʬ�����å����롣
 *	�� buffer �ˤϡ� \0 ��ޤह�٤Ƥ�ʸ�������åȲ�ǽ
 *--------------------------------------------------------------*/
/*
  ASCII
	          | 1�Х�����
	----------+-----------------
	����      | 0x00��0x1F, 0x7F
	ʸ��      | 0x00��0x7E

	    �� ESC(0x1B) ������� JIS ���Ȼפ����������̵��

  EUC-JP
	          | 1�Х�����  | 2�Х�����  | 3�Х����� 
	----------+------------+------------+-----------
	Ⱦ�ѥ���  | 0x8E       | 0xA1��0xDF |
	����      | 0xA1��0xFE | 0xA1��0xFE |
	�������  | 0x8F       | 0xA1��0xFE | 0xA1��0xFE

	    ����������ϡ������̵��

  SJIS
	          | 1�Х�����              | 2�Х�����
        ----------+------------------------+-----------------------
	Ⱦ�ѥ���  | 0xA1��0xDF             |
	����      | 0x81��0x9F��0xE0��0xFC | 0x40��0x7E��0x80��0xFC


  UTF-8
	          | 1�Х�����  | 2�Х�����  | 3�Х�����  | 4�Х�����
	----------+------------+------------+------------+-----------
	��U+07FF  | 0xC2��0xDF | 0x80��0xBF |            |
	��U+FFFF  | 0xE0��0xEF | 0x80��0xBF | 0x80��0xBF |
	��U+1FFFFF| 0xF0��0xF7 | 0x80��0xBF | 0x80��0xBF | 0x80��0xBF

	    �� �ե�������Ƭ�� EF BB BF ���ղä������⤢��
	       0xC0��0xC1�ϡ��������ƥ��塢���Ѷػߤ餷��
	       0xF4��0xFD�ϡ��Ȥ��ʤ��ʤä��餷�� (5��6�Х���Ĺ���ѻ�)
	       0xFE��0xFF�ϡ����Ѥ��ʤ�
*/
int	q8gr_strcode(const char *buffer, int size)
{
    const unsigned char *p;
    unsigned char c;
    int sz;

    int found_not_ascii = FALSE;


    /* UTF-8 ���ͤ����󤬤���Τǡ����곰�Ϥ����ˤ狼�롣�ޤ���
       ���ܸ�ξ��� 3�Х���Ĺ�ǡ�EUC �� SJIS ���ͤ��Ťʤ��ϰϤ�������
       ��äơ��ǽ�˥����å����Ƥ��ǧ�����ˤ����������Ⱦ�������� */


    /* ASCII ���� UTF-8 ��������ʳ�����Ƚ��
       ��0x00��0x7f �����и����ʤ� �� ASCII
       ��UTF-8 �ε�§�˽��äƤ���  �� UTF-8 (¿ʬ)
       ��������Ǥ�ʤ�            �� ����ʳ� */

    p = (const unsigned char *)buffer;
    sz = size;

    while (sz) {
	c = *p;
	if (c <= 0x7f) {			/* ASCII */
	    p++;
	    sz--;
	    continue;
	} else {				/* UTF8 ? */
	    found_not_ascii = TRUE;
	    if (c < 0xc0) {			    /* ????     10xxxxxx */
		break;		/* NG */
	    } else {
		int len;
		if      (c < 0xe0) { len = 2; }	    /* 2byte    110xxxxx */
		else if (c < 0xf0) { len = 3; }	    /* 3byte    1110xxxx */
		else if (c < 0xf8) { len = 4; }	    /* 4byte    11110xxx */
		else { break; }	/* NG */

		p++;
		sz--;
		len--;
		for ( ; len && sz ; len--, sz--) {
		    c = *p++;
		    if ((c & 0xc0) == 0x80) {	    /* lead     10xxxxxx */
			/* OK */
		    } else {
			break;	/* NG */
		    }
		}
	    }
	}
    }
    /* ASCII���ʤ��� UTF-8 �ξ��ϡ� sz == 0 �Ȥʤ� */
    if (sz == 0) {
	if (found_not_ascii) return Q8TK_KANJI_UTF8;
	else                 return Q8TK_KANJI_ANK;
    }
    /* sz > 0 �ʤ顢 EUC �� SJIS */


    /* EUC �� SJIS ����Ƚ��
       ��EUC Ⱦ�� ���ϰϤ��鳰��Ƥ��� �� SJIS
       ��0xfd��0xfe ���ޤޤ��         �� EUC
       ��EUC �������ϰ���Τ�          �� EUC  (¿ʬ)
       ������ʳ�                      �� SJIS �� ����¾ */

    p = (const unsigned char *)buffer;
    sz = size;

    while (sz) {
	c = *p++;
	if (c <= 0x7f) {			    /* ASCII */
	    sz --;
	    continue;
	} else {
	    if (c == 0x8e) {			    /* EUCȾ��? */
		sz --;
		if (sz == 0) break;
		c = *p++;
		if (0xa1 <= c && c <= 0xdf) {		/* yes, �Ǥ�SJIS���� */
		    sz --;
		    continue;
		} else {				/* EUC����ʤ� */
		    break;
		}

	    } else if (0xa1 <= c && c <= 0xfc) {    /* EUC����? */
		sz --;
		if (sz == 0) break;
		c = *p++;
		if (0xa1 <= c && c <= 0xfc) {		/* yes, �Ǥ�SJIS���� */
		    sz --;
		    continue;
		} else if (0xfd <= c && c <= 0xfe) {    /* EUC���� */
		    sz = 0;
		    break;
		} else {				/* EUC����ʤ� */
		    break;
		}
	    } else if (0xfd <= c && c <= 0xfe) {    /* EUC���� */
		sz = 0;
		break;
	    } else {				    /* EUC����ʤ� */
		break;
	    }
	}
    }
    /* EUC �ξ��ϡ� sz == 0 �Ȥʤ� */
    if (sz == 0) return Q8TK_KANJI_EUC;
    else         return Q8TK_KANJI_SJIS;

    /* ���ν������ȡ�ʸ�������ɤ������ʾ��� SJIS �ˤʤ롣�ޡ������� */
}





void	q8gr_draw_mouse(int x, int y)
{
    if (0 <= menu_mouse_x) {
	menu_screen[menu_screen_current][menu_mouse_y][menu_mouse_x].mouse = FALSE;
    }


    if (0 <= x && x < Q8GR_SCREEN_X &&
	0 <= y && y < Q8GR_SCREEN_Y) {

	menu_screen[menu_screen_current][y][x].mouse = TRUE;
	menu_mouse_x = x;
	menu_mouse_y = y;

    } else {
	menu_mouse_x = -1;
	menu_mouse_y = -1;
    }

}








/*----------------------------------------------------------------------
 * �����ȥ��
 *----------------------------------------------------------------------
 *          ��
 *   y      ������ɽ������������ 24ʸ��x3�� ��ͽ��
 *
 *----------------------------------------------------------------------*/
void	q8gr_draw_logo(int x, int y)
{
    int i, j;
    int fg = Q8GR_PALETTE_LOGO_FG;
    int bg = Q8GR_PALETTE_LOGO_BG;
    int  c = 0, addr;

    for (j = 0; j < Q8GR_LOGO_H; j ++) {
	for (i = 0; i < Q8GR_LOGO_W; i ++) {

	    addr = ((c / Q8GR_LOGO_W) * Q8GR_LOGO_W * 16) + (c % Q8GR_LOGO_W);

	    if (! CHECK_MASK_Y(y+j)) {
		if (! CHECK_MASK_X(x+i)) {
		    menu_screen[menu_screen_current][y+j][x+i].background = bg;
		    menu_screen[menu_screen_current][y+j][x+i].foreground = fg;
		    menu_screen[menu_screen_current][y+j][x+i].reverse    = FALSE;
		    menu_screen[menu_screen_current][y+j][x+i].underline  = FALSE;
		    menu_screen[menu_screen_current][y+j][x+i].font_type  = FONT_LOGO;
		    menu_screen[menu_screen_current][y+j][x+i].addr       = addr;
		}
	    }

	    c++;
	}
    }
}
    
/* ���μ��ΤϤ��졣 192x48�ɥå� == 24ʸ��x3�� */

byte	q8gr_logo[ Q8GR_LOGO_W * Q8GR_LOGO_H * 16 ] =
{
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x07,0xf0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x1f,0xfc,0x00,0x7f,0xc0,0x0f,0xf8,0x00,0x7f,0xc0,0x00,0x00,0x0f,0xf1,0xff,0x00,0x01,0xf8,0x00,0x00,0x07,0xe0,0x00,
0x00,0x7f,0xff,0x00,0x7f,0xc0,0x0f,0xf8,0x01,0xff,0xf0,0x00,0x00,0x7f,0xf1,0xff,0x00,0x07,0xfe,0x00,0x00,0x1f,0xf8,0x00,
0x00,0xff,0xff,0x80,0x7f,0xc0,0x0f,0xf8,0x07,0xff,0xfc,0x00,0x01,0xff,0xf1,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x01,0xff,0xff,0xc0,0x7f,0xc0,0x0f,0xf8,0x0f,0xff,0xfe,0x00,0x03,0xff,0xf1,0xff,0x00,0x7f,0xff,0xe0,0x01,0xff,0xff,0x80,
0x03,0xff,0xff,0xe0,0x7f,0xc0,0x0f,0xf8,0x1f,0xff,0xff,0x00,0x07,0xff,0xf1,0xff,0x00,0xff,0xff,0xf0,0x03,0xff,0xff,0xc0,
0x07,0xff,0xff,0xf0,0x7f,0xc0,0x0f,0xf8,0x3f,0xff,0xff,0x80,0x0f,0xff,0xf1,0xff,0x00,0xff,0xff,0xf0,0x03,0xff,0xff,0xc0,
0x07,0xff,0xff,0xf0,0x7f,0xc0,0x0f,0xf8,0x3f,0xff,0xff,0x80,0x0f,0xff,0xf1,0xff,0x01,0xff,0xff,0xf8,0x07,0xff,0xff,0xe0,
0x0f,0xff,0xff,0xf8,0x7f,0xc0,0x0f,0xf8,0x7f,0xff,0xff,0xc0,0x1f,0xff,0xf1,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x0f,0xff,0xff,0xf8,0x7f,0xc0,0x0f,0xf8,0x7f,0xff,0xff,0xc0,0x1f,0xff,0xf1,0xff,0x03,0xff,0xff,0xfc,0x0f,0xff,0xff,0xf0,
0x0f,0xfe,0x3f,0xf8,0x7f,0xc0,0x0f,0xf8,0x7f,0xf1,0xff,0xc0,0x1f,0xfe,0x01,0xff,0x03,0xff,0x9f,0xfc,0x0f,0xfe,0x7f,0xf0,
0x1f,0xfc,0x1f,0xfc,0x7f,0xc0,0x0f,0xf8,0xff,0xe0,0xff,0xe0,0x3f,0xf8,0x01,0xff,0x07,0xfe,0x07,0xfe,0x1f,0xf8,0x1f,0xf8,
0x1f,0xfc,0x1f,0xfc,0x7f,0xc0,0x0f,0xf8,0xff,0xc0,0x7f,0xe0,0x3f,0xf0,0x01,0xff,0x07,0xfc,0x03,0xfe,0x1f,0xf0,0x0f,0xf8,
0x1f,0xf8,0x0f,0xfc,0x7f,0xc0,0x0f,0xf8,0xff,0xc0,0x7f,0xe0,0x3f,0xe0,0x01,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x3f,0xf0,0x07,0xfe,0x7f,0xc0,0x0f,0xf9,0xff,0x80,0x3f,0xf0,0x3f,0xe0,0x01,0xff,0x07,0xfc,0x03,0xfe,0x1f,0xf0,0x0f,0xf8,
0x3f,0xf0,0x07,0xfe,0x7f,0xc0,0x0f,0xf9,0xff,0x80,0x3f,0xf0,0x3f,0xe0,0x01,0xff,0x07,0xfc,0x03,0xfe,0x1f,0xf0,0x0f,0xf8,
0x3f,0xf0,0x07,0xfe,0x7f,0xc0,0x0f,0xf9,0xff,0x80,0x3f,0xf0,0x3f,0xe0,0x01,0xff,0x07,0xfe,0x07,0xfe,0x1f,0xf8,0x1f,0xf8,
0x3f,0xe0,0x03,0xfe,0x7f,0xc0,0x0f,0xf9,0xff,0x00,0x1f,0xf0,0x3f,0xe0,0x01,0xff,0x03,0xff,0x9f,0xfc,0x0f,0xfe,0x7f,0xf0,
0x3f,0xe0,0x03,0xfe,0x7f,0xc0,0x0f,0xf9,0xff,0x00,0x1f,0xf0,0x3f,0xf0,0x01,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x3f,0xe0,0x03,0xfe,0x7f,0xc0,0x0f,0xf9,0xff,0x00,0x1f,0xf0,0x3f,0xf0,0x01,0xff,0x03,0xff,0xff,0xfc,0x0f,0xff,0xff,0xf0,
0x3f,0xe0,0x03,0xfe,0x7f,0xc0,0x0f,0xf9,0xff,0x00,0x1f,0xf0,0x1f,0xf8,0x01,0xff,0x01,0xff,0xff,0xf8,0x07,0xff,0xff,0xe0,
0x3f,0xe0,0x03,0xfe,0x7f,0xc0,0x0f,0xf9,0xff,0x00,0x1f,0xf0,0x1f,0xfc,0x01,0xff,0x01,0xff,0xff,0xf8,0x07,0xff,0xff,0xe0,
0x3f,0xe0,0x03,0xfe,0x7f,0xc0,0x0f,0xf9,0xff,0x00,0x1f,0xf0,0x0f,0xfe,0x01,0xff,0x01,0xff,0xff,0xf8,0x07,0xff,0xff,0xe0,
0x3f,0xe0,0x03,0xfe,0x7f,0xc0,0x0f,0xf9,0xff,0x00,0x1f,0xf0,0x07,0xff,0x01,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x3f,0xe0,0x03,0xfe,0x7f,0xc0,0x0f,0xf9,0xff,0x3f,0xff,0xf0,0x07,0xff,0x81,0xff,0x03,0xff,0xff,0xfc,0x0f,0xff,0xff,0xf0,
0x3f,0xe0,0x03,0xfe,0x7f,0xc0,0x0f,0xf9,0xff,0x3f,0xff,0xf0,0x03,0xff,0xc1,0xff,0x07,0xff,0xff,0xfe,0x1f,0xff,0xff,0xf8,
0x3f,0xe0,0x13,0xfe,0x7f,0xc0,0x0f,0xf9,0xff,0x3f,0xff,0xf0,0x01,0xff,0xc1,0xff,0x07,0xff,0x9f,0xfe,0x1f,0xfe,0x7f,0xf8,
0x3f,0xe0,0x3b,0xfe,0x7f,0xc0,0x0f,0xf9,0xff,0x3f,0xff,0xf0,0x00,0xff,0xe1,0xff,0x0f,0xfe,0x07,0xff,0x3f,0xf8,0x1f,0xfc,
0x3f,0xe0,0x7f,0xfe,0x7f,0xc0,0x0f,0xf9,0xff,0x3f,0xff,0xf0,0x00,0x7f,0xe1,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x3f,0xf0,0xff,0xfe,0x7f,0xc0,0x0f,0xf9,0xff,0x3f,0xff,0xf0,0x00,0x3f,0xf1,0xff,0x0f,0xfc,0x03,0xff,0x3f,0xf0,0x0f,0xfc,
0x3f,0xf1,0xff,0xfe,0x7f,0xc0,0x0f,0xf9,0xff,0x3f,0xff,0xf0,0x00,0x3f,0xf1,0xff,0x0f,0xf8,0x01,0xff,0x3f,0xe0,0x07,0xfc,
0x3f,0xf1,0xff,0xfe,0x7f,0xc0,0x0f,0xf9,0xff,0x3f,0xff,0xf0,0x00,0x1f,0xf1,0xff,0x0f,0xf8,0x01,0xff,0x3f,0xe0,0x07,0xfc,
0x1f,0xf8,0xff,0xfc,0x7f,0xc0,0x0f,0xf9,0xff,0x3f,0xff,0xf0,0x00,0x1f,0xf1,0xff,0x0f,0xf8,0x01,0xff,0x3f,0xe0,0x07,0xfc,
0x1f,0xfc,0x7f,0xfc,0x7f,0xe0,0x1f,0xf9,0xff,0x00,0x1f,0xf0,0x00,0x3f,0xf1,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x1f,0xfc,0x7f,0xfc,0x7f,0xf8,0x7f,0xf9,0xff,0x00,0x1f,0xf0,0x00,0x7f,0xf1,0xff,0x0f,0xfc,0x03,0xff,0x3f,0xf0,0x0f,0xfc,
0x0f,0xfe,0x3f,0xf8,0x7f,0xff,0xff,0xf9,0xff,0x00,0x1f,0xf0,0x01,0xff,0xf1,0xff,0x0f,0xfe,0x07,0xff,0x3f,0xf8,0x1f,0xfc,
0x0f,0xff,0xff,0xf8,0x7f,0xff,0xff,0xf9,0xff,0x00,0x1f,0xf0,0x0f,0xff,0xf1,0xff,0x0f,0xff,0x9f,0xff,0x3f,0xfe,0x7f,0xfc,
0x0f,0xff,0xff,0xfc,0x3f,0xff,0xff,0xf1,0xff,0x00,0x1f,0xf3,0xff,0xff,0xe1,0xff,0x07,0xff,0xff,0xfe,0x1f,0xff,0xff,0xf8,
0x07,0xff,0xff,0xfe,0x3f,0xff,0xff,0xf1,0xff,0x00,0x1f,0xf3,0xff,0xff,0xe1,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x07,0xff,0xff,0xff,0x1f,0xff,0xff,0xe1,0xff,0x00,0x1f,0xf3,0xff,0xff,0xc1,0xff,0x03,0xff,0xff,0xfc,0x0f,0xff,0xff,0xf0,
0x03,0xff,0xff,0xff,0x1f,0xff,0xff,0xe1,0xff,0x00,0x1f,0xf3,0xff,0xff,0x81,0xff,0x01,0xff,0xff,0xf8,0x07,0xff,0xff,0xe0,
0x01,0xff,0xff,0xfe,0x0f,0xff,0xff,0xc1,0xff,0x00,0x1f,0xf3,0xff,0xff,0x01,0xff,0x00,0xff,0xff,0xf0,0x03,0xff,0xff,0xc0,
0x00,0xff,0xff,0xfc,0x07,0xff,0xff,0x81,0xff,0x00,0x1f,0xf3,0xff,0xfe,0x01,0xff,0x00,0x7f,0xff,0xe0,0x01,0xff,0xff,0x80,
0x00,0x7f,0xff,0x78,0x01,0xff,0xfe,0x01,0xff,0x00,0x1f,0xf3,0xff,0xf8,0x01,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x1f,0xfc,0x30,0x00,0xff,0xfc,0x01,0xff,0x00,0x1f,0xf3,0xff,0xc0,0x01,0xff,0x00,0x0f,0xff,0x00,0x00,0x3f,0xfc,0x00,
0x00,0x07,0xf0,0x00,0x00,0x1f,0xe0,0x01,0xff,0x00,0x1f,0xf3,0xfe,0x00,0x01,0xff,0x00,0x03,0xfc,0x00,0x00,0x0f,0xf0,0x00,
};



int	q8gr_draw_label(int x, int y, int fg, int bg,
			int reverse, int underline, int code, const char *str,
			void *p)
{
    if (p == NULL) fg = Q8GR_PALETTE_WHITE;

    return q8gr_strings(x, y, fg, bg, reverse, underline, -1, code, str, 0, 0);
}
