/************************************************************************/
/*									*/
/* QUASI88 ��˥塼�� Tool Kit						*/
/*									*/
/*	GTK+ �� API �򿿻��ƺ��ޤ�������䤳�������ơ��ɥ�����Ȥ�	*/
/*	�񤱤ޤ���ġġġ�						*/
/*									*/
/************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "quasi88.h"

#include "q8tk.h"
#include "q8tk-glib.h"

#include "menu-screen.h"

#include "file-op.h"
#include "wait.h"

#include "keyboard.h"	/* mouse_x, mouse_y */
#include "event.h"
#include "menu.h"
#include "screen.h"


/*#define OLD_FOCUS_CHANGE*/


/* ������Хå��ؿ��ΰ��� (Q8tkWidget*, void*) ��̤���Ѥξ�硢
 * ��˥󥰤��Ф�ݵƫ�����Τǡ� gcc �����ǵ����Ƥ�餦�� */
#if defined(__GNUC__)
#define	UNUSED_WIDGET	__attribute__((__unused__)) Q8tkWidget *dummy_0
#define	UNUSED_PARM	__attribute__((__unused__)) void *dummy_1
#else
#define	UNUSED_WIDGET	Q8tkWidget *dummy_0
#define	UNUSED_PARM	void *dummy_1
#endif



/************************************************************************/
/* �ǥХå������顼����							*/
/************************************************************************/

#define	Q8tkAssert(e,s)		\
		((e) ? (void)0 : _Q8tkAssert(__FILE__, __LINE__, #e, s))

static	void	_Q8tkAssert(char *file, int line, char *exp, const char *s)
{
    fprintf(stderr, "Fatal Error | %s <%s:%d>\n", exp, file, line);
    fprintf(stderr, "message = %s\n",(s)?s:"---");
    quasi88_exit(-1);
}

static const char *debug_type(int type) {
    switch (type) {
    case Q8TK_TYPE_WINDOW:		return "window  :";
    case Q8TK_TYPE_BUTTON:		return "button  :";
    case Q8TK_TYPE_TOGGLE_BUTTON:	return "t-button:";
    case Q8TK_TYPE_CHECK_BUTTON:	return "c-button:";
    case Q8TK_TYPE_RADIO_BUTTON:	return "r-button:";
    case Q8TK_TYPE_FRAME:		return "frame   :";
    case Q8TK_TYPE_LABEL:		return "label   :";
    case Q8TK_TYPE_LOGO:		return "logo    :";
    case Q8TK_TYPE_NOTEBOOK:		return "notebook:";
    case Q8TK_TYPE_NOTEPAGE:		return "page    :";
    case Q8TK_TYPE_VBOX:		return "vbox    :";
    case Q8TK_TYPE_HBOX:		return "hbox    :";
    case Q8TK_TYPE_VSEPARATOR:		return "vsep    :";
    case Q8TK_TYPE_HSEPARATOR:		return "hsep    :";
    case Q8TK_TYPE_COMBO:		return "combo   :";
    case Q8TK_TYPE_LISTBOX:		return "listbox :";
    case Q8TK_TYPE_LIST_ITEM:		return "listitem:";
    case Q8TK_TYPE_ADJUSTMENT:		return "adjust  :";
    case Q8TK_TYPE_HSCALE:		return "hscale  :";
    case Q8TK_TYPE_VSCALE:		return "vscale  :";
    case Q8TK_TYPE_SCROLLED_WINDOW:	return "scrolled:";
    case Q8TK_TYPE_ENTRY:		return "entry   :";
    case Q8TK_TYPE_DIALOG:		return "dialog  :";
    case Q8TK_TYPE_FILE_SELECTION:	return "f-select:";
    }
    return "UNDEF TYPE:";
}



/************************************************************************/
/* ���								*/
/************************************************************************/

static	int		MAX_WIDGET;
static	Q8tkWidget	**widget_table;


static	int		MAX_LIST;
static	Q8List		**list_table;


#define	MAX_WINDOW_LAYER	(8)
static	Q8tkWidget		*window_layer[ MAX_WINDOW_LAYER ];
static	int			window_layer_level;
static	Q8tkWidget		*focus_widget[ MAX_WINDOW_LAYER ];
#define	get_focus_widget()	(focus_widget[ window_layer_level ])



static	struct {
    int	x, y;
    int	x_old, y_old;
} mouse;


static	int	kanji_code = Q8TK_KANJI_ANK;
static	int	disp_cursor = FALSE;

static	int	now_shift_on;
static	int	now_mouse_on;

/*----------------------------------------------------------------------*/

static	void	widget_map(Q8tkWidget *widget);
static	void	widget_construct(void);
static	void	widget_signal_do(Q8tkWidget *widget, const char *name);



/************************************************************************/
/* ưŪ����γ��ݡ�����						*/
/************************************************************************/
/*--------------------------------------------------------------
 *	Widget
 *--------------------------------------------------------------*/
static	Q8tkWidget	*malloc_widget(void)
{
    int		i;
    Q8tkWidget	**t, *w;
    int		add;

    t = NULL;
    for (i=0; i<MAX_WIDGET; i++) {
	if (widget_table[i] == NULL) {
	    t = &widget_table[i];
	    break;
	}
    }

    if (t == NULL) {
	add = 512;
	if (MAX_WIDGET == 0) {
	    widget_table = malloc((MAX_WIDGET + add) * sizeof(Q8tkWidget*));
	} else {
	    widget_table = realloc(widget_table,
				   (MAX_WIDGET + add) * sizeof(Q8tkWidget*));
	}
	if (widget_table) {
	    for (i=0; i<add; i++) {
		widget_table[ MAX_WIDGET + i ] = NULL;
	    }
	    t = &widget_table[ MAX_WIDGET ];
	    MAX_WIDGET += add;
	}
    }

    if (t) {
	w = (Q8tkWidget *)calloc(1, sizeof(Q8tkWidget));
	if (w) {
	    /* ����ͤ� �����ФȤ� 0 */
	    *t = w;
	    return w;
	} else {
	    Q8tkAssert(FALSE, "memory exhoused");
	}
    }

    Q8tkAssert(FALSE, "work 'widget' exhoused");
    return NULL;
}
static	void	free_widget(Q8tkWidget *w)
{
    int i;
    for (i=0; i<MAX_WIDGET; i++) {
	if (widget_table[i] == w) {
	    free(w);
	    widget_table[i] = NULL;
	    return;
	}
    }
    Q8tkAssert(FALSE, "pointer is not malloced widget");
}
/*--------------------------------------------------------------
 *	List
 *--------------------------------------------------------------*/
static	Q8List	*malloc_list(void)
{
    int		i;
    Q8List	**t, *l;
    int		add;

    t = NULL;
    for (i=0; i<MAX_LIST; i++) {
	if (list_table[i] == NULL) {
	    t = &list_table[i];
	    break;
	}
    }

    if (t == NULL) {
	add = 512;
	if (MAX_LIST == 0) {
	    list_table = malloc((MAX_LIST + add) * sizeof(Q8List*));
	} else {
	    list_table = realloc(list_table,
				 (MAX_LIST + add) * sizeof(Q8List*));
	}
	if (list_table) {
	    for (i=0; i<add; i++) {
		list_table[ MAX_LIST + i ] = NULL;
	    }
	    t = &list_table[ MAX_LIST ];
	    MAX_LIST += add;
	}
    }

    if (t) {
	l = (Q8List *)calloc(1, sizeof(Q8List));
	if (l) {
	    *t = l;
	    return l;
	} else {
	    Q8tkAssert(FALSE, "memory exhoused");
	}
    }

    Q8tkAssert(FALSE, "work 'list' exhoused");
    return NULL;
}
static	void	free_list(Q8List *l)
{
    int	i;
    for (i=0; i<MAX_LIST; i++) {
	if (list_table[i] == l) {
	    free(l);
	    list_table[i] = NULL;
	    return;
	}
    }
    Q8tkAssert(FALSE, "pointer is not malloced list");
}



#if 0
/************************************************************************/
/* Q8TK ���� ʸ�������							*/
/*	q8_strncpy(s, ct, n)						*/
/*		ʸ���� ct �� ʸ���� s �� ���ԡ����롣			*/
/*		s ��ʸ����ü�ϡ�ɬ�� '\0' �Ȥʤꡢs ��Ĺ���� n-1 ʸ��	*/
/*		�ʲ��˼��ޤ롣						*/
/*	q8_strncat(s, ct, n)						*/
/*		ʸ���� ct �� ʸ���� s �� �ղä��롣			*/
/*		s ��ʸ����ü�ϡ�ɬ�� '\0' �Ȥʤꡢs ��Ĺ���� n-1 ʸ��	*/
/*		�ʲ��˼��ޤ롣						*/
/************************************************************************/
static	void	q8_strncpy(char *s, const char *ct, size_t n)
{
    strncpy(s, ct, n-1);
    s[ n-1 ] = '\0';
}
static	void	q8_strncat(char *s, const char *ct, size_t n)
{
    if (n > strlen(s) + 1)
	strncat(s, ct, n - strlen(s) -1);
}
#endif


/************************************************************************/
/* �ꥹ�Ƚ���								*/
/************************************************************************/
/*--------------------------------------------------------------
 * �ꥹ�Ȥ������ˤ����餷�����Ǥ��ɲ� (�ޤ��ϥꥹ�Ȥ򿷵�����)
 *	����ͤϡ��ꥹ�Ȥ���Ƭ
 *--------------------------------------------------------------*/
Q8List	*q8_list_append(Q8List *list, void *data)
{
    Q8List	*new_list;

    new_list  = malloc_list();		/* ������� */

    if (list) {				/* ��¸�Υꥹ�ȤˤĤʤ����� */
	list = q8_list_last(list);		/* �����ȥ�󥯤��� */
	list->next     = new_list;
	new_list->prev = list;
	new_list->next = NULL;
    }else{				/* �����Υꥹ�Ȥξ�� */
	new_list->prev = NULL;
	new_list->next = NULL;
    }
    new_list->data   = data;

    return q8_list_first(new_list);	/* ������Υꥹ����Ƭ���֤�   */
}
/*--------------------------------------------------------------
 * �ꥹ�Ȥ�����ˤ����餷�����Ǥ��ɲ� (position==0����Ƭ)
 *	����ͤϡ��ꥹ�Ȥ���Ƭ
 *--------------------------------------------------------------*/
#if 0
Q8List	*q8_list_insert(Q8List *list, void *data, int position)
{
    /* ̤���� */
    return NULL;
}
#endif
/*--------------------------------------------------------------
 * �ꥹ�Ȥ����Ǥ�ҤȤĤ������
 *	����ͤϡ��ꥹ�Ȥ���Ƭ
 *--------------------------------------------------------------*/
Q8List	*q8_list_remove(Q8List *list, void *data)
{
    list = q8_list_first(list);

    while (list) {			/* �ꥹ����Ƭ�����ˤ��ɤä� */
	if (list->data == data) {	/* data �ΰ��פ����Τ�õ��  */
	    list->prev->next = list->next;	/* �ߤĤ���Х�󥯤� */
	    list->next->prev = list->prev;	/* �Ĥʤ��Ѥ���       */
	    free_list(list);			/* ���ȤϺ��         */
	    break;
	}
	list = list->next;
    }

    return q8_list_first(list);		/* ������Υꥹ����Ƭ���֤�   */
}
/*--------------------------------------------------------------
 * �ꥹ�Ȥξõ�
 *--------------------------------------------------------------*/
void	q8_list_free(Q8List *list)
{
    Q8List *l;

    list = q8_list_first(list);		/* �ꥹ�Ȥ���Ƭ���� */

    while (list) {			/* ���ɤ�ʤ������ */
	l = list->next;
	free_list(list);
	list = l;
    }
}
/*--------------------------------------------------------------
 * �ꥹ����Ƭ��
 *--------------------------------------------------------------*/
Q8List	*q8_list_first(Q8List *list)
{
    if (list) {
	while (list->prev) {		/* �ꥹ�Ȥ����ˤ��ɤ� */
	    list = list->prev;
	}				/* ���ɤ�ʤ��ʤä��� */
    }					/* ���줬��Ƭ         */
    return list;
}
/*--------------------------------------------------------------
 * �ꥹ��������
 *--------------------------------------------------------------*/
Q8List	*q8_list_last(Q8List *list)
{
    if (list) {
	while (list->next) {		/* �ꥹ�Ȥ��ˤ��ɤ� */
	    list = list->next;
	}				/* ���ɤ�ʤ��ʤä��� */
    }					/* ���줬����         */
    return list;
}
/*--------------------------------------------------------------
 * �ꥹ�ȸ���
 *--------------------------------------------------------------*/
Q8List	*q8_list_find(Q8List *list, void *data)
{
    list = q8_list_first(list);		/* �ꥹ�Ȥ���Ƭ����   */

    while (list) {			/* ���ɤ�ʤ���       */
	if (list->data == data) break;	/* ���פ����Τ�õ�� */
	list = list->next;
    }
    return list;
}


/************************************************************************/
/* ����������								*/
/************************************************************************/
int	q8tk_set_kanjicode(int code)
{
    int ret = kanji_code;

    if (code != Q8TK_KANJI_ANK &&
	code != Q8TK_KANJI_EUC &&
	code != Q8TK_KANJI_SJIS &&
	code != Q8TK_KANJI_UTF8) {

	code = Q8TK_KANJI_ANK;
    }

    kanji_code = code;

    return ret;
}
/************************************************************************/
/* ��������ɽ����̵ͭ							*/
/************************************************************************/
void	q8tk_set_cursor(int enable)
{
    disp_cursor = (enable) ? TRUE : FALSE;
}





/************************************************************************/
/* ������Ƚ�λ								*/
/************************************************************************/

static	int		q8tk_disp_init_flag;
static	int		q8tk_main_loop_flag;
static	int		q8tk_construct_flag;

#define	set_main_loop_flag(f)	q8tk_main_loop_flag = (f)
#define	get_main_loop_flag()	q8tk_main_loop_flag
#define	set_construct_flag(f)	q8tk_construct_flag = (f)
#define	get_construct_flag()	q8tk_construct_flag


static	Q8tkWidget	*q8tk_drag_widget;
#define	set_drag_widget(w)	q8tk_drag_widget = (w)
#define	get_drag_widget()	q8tk_drag_widget



static	void		widget_focus_list_init(void);
static	Q8tkWidget	*widget_focus_list_get_next(Q8tkWidget *widget,
						    int back);



static	void	widget_scrollin_init(void);
static	void	widget_scrollin_register(Q8tkWidget *widget);
static	void	widget_scrollin_drawn(Q8tkWidget *widget);


static	int		delay_timer;
static	Q8tkWidget	*q8tk_delay_widget;
#define	set_delay_widget(w, n)	do {				\
				    if (w == NULL) {		\
					delay_timer = 0;	\
				    } else {			\
					delay_timer = n;	\
				    }				\
				    q8tk_delay_widget = w;	\
				} while (0)
#define	get_delay_widget()	q8tk_delay_widget




/*--------------------------------------------------------------
 *	�����
 *--------------------------------------------------------------*/
void	q8tk_init(void)
{
    int	i;

    MAX_WIDGET = 0;
    widget_table = NULL;

    MAX_LIST = 0;
    list_table = NULL;

    for (i=0; i<MAX_WINDOW_LAYER; i++) {
	window_layer[i] = NULL;
	focus_widget[i] = NULL;
    }
    window_layer_level = -1;

    set_main_loop_flag(TRUE);
    set_construct_flag(FALSE);

    set_drag_widget(NULL);
    widget_focus_list_init();
    widget_scrollin_init();


    q8gr_init();


    mouse.x_old = mouse.x = mouse_x;
    mouse.y_old = mouse.y = mouse_y;

    now_shift_on = FALSE;
    now_mouse_on = FALSE;

    q8tk_disp_init_flag = TRUE;
}
/*--------------------------------------------------------------
 *	��λ
 *--------------------------------------------------------------*/
static	void	q8tk_term(void)
{
    int	i;
    for (i=0; i<MAX_WIDGET; i++) {
	if (widget_table[i]) {
	    if (widget_table[i]->name) {
		free(widget_table[i]->name);
	    }
	    free(widget_table[i]);
	}
    }
    if (widget_table) {
	free(widget_table);
    }
    widget_table = NULL;
    MAX_WIDGET = 0;

    for (i=0; i<MAX_LIST; i++) {
	if (list_table[i]) {
	    free(list_table[i]);
	}
    }
    if (list_table) {
	free(list_table);
    }
    list_table = NULL;
    MAX_LIST = 0;
}



/************************************************************************/
/* �⡼���������							*/
/************************************************************************/
/*--------------------------------------------------------------
 *	�⡼��������
 *--------------------------------------------------------------*/
void	q8tk_grab_add(Q8tkWidget *widget)
{
    int i;
    Q8tkAssert(widget->type == Q8TK_TYPE_WINDOW, "grab add not window");

    for (i=0; i<MAX_WINDOW_LAYER; i++) {
	if (window_layer[i] == NULL) {
	    window_layer[i] = widget;
	    window_layer_level = i;
	    set_construct_flag(TRUE);
	    return;
	}
    }
    Q8tkAssert(FALSE, "overflow window level");
}
/*--------------------------------------------------------------
 *	�⡼������
 *--------------------------------------------------------------*/
void	q8tk_grab_remove(Q8tkWidget *widget)
{
    int i;

    for (i=0; i<MAX_WINDOW_LAYER; i++) {
	if (window_layer[i] == widget) {
	    break;
	}
    }
    Q8tkAssert(i < MAX_WINDOW_LAYER, "grab remove not widget");
    for (   ; i<MAX_WINDOW_LAYER-1; i++) {
	window_layer[i] = window_layer[i+1];
	focus_widget[i] = focus_widget[i+1];
    }
    window_layer[i] = NULL;
    focus_widget[i] = NULL;
    window_layer_level --;
    set_construct_flag(TRUE);
}



/********************************************************/
/* �������åȤκ���					*/
/********************************************************/

/*---------------------------------------------------------------------------
 * ������ɥ� (WINDOW)
 *---------------------------------------------------------------------------
 *  �����ƤΥ������åȤκǤ����Ĥˤʤ륳��ƥʥ������åȡ�
 *  ���Ҥ��Ļ��Ƥ롣
 *  ���Ҥ���Ĥˤϡ�q8tk_container_add() ����Ѥ��롣
 *  �����Υ������åȤ�ɽ������ˤϡ�q8tk_grab_add()�ˤ�����Ū��ɽ����ؼ����롣
 *  ��WINDOW �Ϻ��硢MAX_WINDOW_LAYER�ĺ����Ǥ��뤬�������ʥ������դ���Τϡ�
 *    �Ǹ�� q8tk_grab_add() ��ȯ�Ԥ���������ɥ��λ�¹�ΤߤǤ��롣
 *  ��Q8TK_WINDOW_TOPLEVEL �Υ�����ɥ���ɬ��ɬ�ס�
 *  �������ʥ�
 *	�ʤ�
 *  �����������ʥ� (�桼�������Բ�)
 *	"inactivate"	�ݥåץ��åץ�����ɥ��ˤƥ�����ɥ����򥯥�å���ȯ��
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_window_new(int window_type)
 *	������ɥ�������
 *	�����ˤ�ꡢ������ɥ��μ��ब��ޤ�
 *		Q8TK_WINDOW_TOPLEVEL �� ���ִ��äΥ�����ɥ��ǡ�ͣ��ĤΤ�
 *					������ǽ��������ɥ��Ȥ�����ʤ�
 *		Q8TK_WINDOW_DIALOG   �� �ФäѤä�������ɥ��Ȥ���
 *		Q8TK_WINDOW_POPUP    �� ñ��ʥ�����ɥ��Ȥ���
 *  -------------------------------------------------------------------------
 *	��WINDOW�ۢ��� [xxxx]		���ޤ��ޤʥ������åȤ�Ҥ˻��Ƥ�
 *
 *---------------------------------------------------------------------------*/
/* �ݥåץ��åץ�����ɥ����ϰϳ��ˤƥޥ�������å������顢�����ʥ�ȯ�� */
static	void	window_event_button_on(Q8tkWidget *widget)
{
    widget_signal_do(widget, "inactivate");
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Q8tkWidget	*q8tk_window_new(int window_type)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_WINDOW;
    w->attr = Q8TK_ATTR_CONTAINER;
    w->sensitive = TRUE;

    w->stat.window.type = window_type;

    switch (window_type) {
    case Q8TK_WINDOW_TOPLEVEL:
	Q8tkAssert(window_layer[0] == NULL, "Redefine TOP window");
	q8tk_grab_add(w);
	w->stat.window.no_frame = TRUE;
	break;

    case Q8TK_WINDOW_DIALOG:
	w->stat.window.no_frame = FALSE;
	w->stat.window.shadow_type = Q8TK_SHADOW_OUT;
	break;

    case Q8TK_WINDOW_POPUP:
	w->stat.window.no_frame = FALSE;
	w->stat.window.shadow_type = Q8TK_SHADOW_ETCHED_OUT;
	w->event_button_on = window_event_button_on;
	break;
    }

    return w;
}




/*---------------------------------------------------------------------------
 * �ܥ��� (BUTTON)
 *---------------------------------------------------------------------------
 *  ���ޥ����ʤɤǥܥ���򲡤��Ƥ���֤ϰ��ù���ǡ��ޥ�����Υ������롣
 *  ���Ҥ��Ļ��Ƥ롣 (â������٥�ʳ��λҤ���ä�����ư���̤�ݾ�)
 *  ���Ҥ���Ĥˤϡ�q8tk_container_add() ����Ѥ��롣
 *  �������ʥ�
 *	"clicked"	�ܥ��󤬲����줿����ȯ��
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_button_new(void)
 *	�ܥ����������
 *
 *  Q8tkWidget	*q8tk_button_new_with_label(const char *label)
 *	ʸ���� label �������줿�ܥ����������
 *	����Ū�ˤϡ��ܥ���ȥ�٥���ꡢ�ƻҤˤ��롣
 *  -------------------------------------------------------------------------
 *	��BUTTON�ۢ��� [LABEL]		��٥��Ҥ˻���
 *
 *---------------------------------------------------------------------------*/
/* �꥿���󡦥��ڡ����򲡤����顢�����ʥ�ȯ�� */
static	void	button_event_key_on(Q8tkWidget *widget, int key)
{
    if (key == Q8TK_KEY_RET || key == Q8TK_KEY_SPACE) {

#if 0		/* ���Τޤޤ��ȡ���������֤����褹��ˤ��ʤ� */
	widget_signal_do(widget, "clicked");

#else		/* ��������֤����褵�������Τǡ����٥�Ȥ��٤餻�� */
	widget->stat.button.active = Q8TK_BUTTON_ON;
	set_construct_flag(TRUE);
	set_delay_widget(widget, 3);		/* 3 �ե졼��ʬ�ٱ� */
#endif
    }
}

/* �ܥ����ޥ�������å������顢�ܥ������ޤ��ơ��ɥ�å����ϤȤ��� */
static	void	button_event_button_on(Q8tkWidget *widget)
{
    set_drag_widget(widget);

    widget->stat.button.active = Q8TK_BUTTON_ON;
    set_construct_flag(TRUE);
}

/* �ɥ�å���ϡ��ޥ����ΰ��֤˱����ơ��ܥ�����̱������� */
static	void	button_event_dragging(Q8tkWidget *widget)
{
    if (q8gr_get_focus_screen(mouse.x/8, mouse.y/16) == widget) {
	if (widget->stat.button.active == Q8TK_BUTTON_OFF) {
	    widget->stat.button.active = Q8TK_BUTTON_ON;
	    set_construct_flag(TRUE);
	}
    } else {
	if (widget->stat.button.active == Q8TK_BUTTON_ON) {
	    widget->stat.button.active = Q8TK_BUTTON_OFF;
	    set_construct_flag(TRUE);
	}
    }
}

/* �ɥ�å���λ�ǡ��ܥ��󤬱���Ǥ��顢�����ʥ�ȯ�� */
static	void	button_event_drag_off(Q8tkWidget *widget)
{
    if (widget->stat.button.active == Q8TK_BUTTON_ON) {

#if 0	/* �ޥ���Ϣ�ǤǤΡ����̤�Ϣ³ɽ��������äȸ��줷�� */
	widget->stat.button.active = Q8TK_BUTTON_OFF;
	widget_signal_do(widget, "clicked");
	set_construct_flag(TRUE);

#else	/* ���٥�Ȥ��٤餻�뤳�Ȥǡ�Ϣ³ɽ�����ʤ��褦�ˤ��� */
	widget->stat.button.active = Q8TK_BUTTON_OFF;
	set_delay_widget(widget, 3);		/* 3 �ե졼��ʬ�ٱ� */
	set_construct_flag(TRUE);
#endif
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Q8tkWidget	*q8tk_button_new(void)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_BUTTON;
    w->attr = Q8TK_ATTR_CONTAINER;
    w->sensitive = TRUE;

    w->event_button_on = button_event_button_on;
    w->event_key_on    = button_event_key_on;
    w->event_dragging  = button_event_dragging;
    w->event_drag_off  = button_event_drag_off;

    return w;
}

Q8tkWidget	*q8tk_button_new_with_label(const char *label)
{
    Q8tkWidget *b = q8tk_button_new();
    Q8tkWidget *l = q8tk_label_new(label);

    q8tk_widget_show(l);
    q8tk_container_add(b, l);

    b->with_label = TRUE;

    return b;
}




/*---------------------------------------------------------------------------
 * �ȥ���ܥ��� (TOGGLE BUTTON)
 *---------------------------------------------------------------------------
 *  ���ޥ����ʤɤǥܥ���򲡤��Ȱ��ù��ࡣ�⤦���٥ܥ���򲡤�����롣
 *  ���Ҥ��Ļ��Ƥ롣 (â������٥�ʳ��λҤ���ä�����ư���̤�ݾ�)
 *  ���Ҥ���Ĥˤϡ�q8tk_container_add() ����Ѥ��롣
 *  �������ʥ�
 *	"clicked"	�ܥ���򲡤��ư��ù��������ȯ��
 *	"toggled"	�ܥ���ξ��֤��Ѳ���������ȯ��
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_toggle_button_new(void)
 *	�ȥ���ܥ����������
 *
 *  Q8tkWidget	*q8tk_toggle_button_new_with_label(const char *label)
 *	ʸ���� label �������줿����ܥ����������
 *	����Ū�ˤϡ��ȥ���ܥ���ȥ�٥���ꡢ�ƻҤˤ��롣
 *
 *  void	q8tk_toggle_button_set_state(Q8tkWidget *widget, int status)
 *	�ܥ���ξ��֤��ѹ����롣
 *	status �����ʤ�ܥ���򲡤������֤ˤ��� (�����ʥ뤬ȯ������)��
 *	status �����ʤ�ʤˤ⤷�ʤ���
 *  -------------------------------------------------------------------------
 *	��TOGGLE BUTTON�ۢ��� [LABEL]		��٥��Ҥ˻���
 *
 *---------------------------------------------------------------------------*/
/* �ܥ����ޥ�������å������顢�����ʥ�ȯ�� */
static	void	toggle_button_event_button_on(Q8tkWidget *widget)
{
    if (widget->stat.button.active == Q8TK_BUTTON_ON) {
	widget->stat.button.active = Q8TK_BUTTON_OFF;
	widget_signal_do(widget, "toggled");
    } else {
	widget->stat.button.active = Q8TK_BUTTON_ON;
	widget_signal_do(widget, "clicked");
	widget_signal_do(widget, "toggled");
    }
    set_construct_flag(TRUE);
}

/* �꥿���󡦥��ڡ����򲡤����顢�ޥ�������å���Ʊ�������򤹤� */
static	void	toggle_button_event_key_on(Q8tkWidget *widget, int key)
{
    if (key == Q8TK_KEY_RET || key == Q8TK_KEY_SPACE) {
	toggle_button_event_button_on(widget);
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Q8tkWidget	*q8tk_toggle_button_new(void)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_TOGGLE_BUTTON;
    w->attr = Q8TK_ATTR_CONTAINER;
    w->sensitive = TRUE;

    w->event_button_on = toggle_button_event_button_on;
    w->event_key_on    = toggle_button_event_key_on;

    return w;
}

Q8tkWidget	*q8tk_toggle_button_new_with_label(const char *label)
{
    Q8tkWidget *b = q8tk_toggle_button_new();
    Q8tkWidget *l = q8tk_label_new(label);

    q8tk_widget_show(l);
    q8tk_container_add(b, l);

    b->with_label = TRUE;

    return b;
}

void		q8tk_toggle_button_set_state(Q8tkWidget *widget, int status)
{
    if (status) {
	if (widget->event_key_on) {
	    (*widget->event_key_on)(widget, Q8TK_KEY_SPACE);
	}
    } else {
	widget->stat.button.active = Q8TK_BUTTON_OFF;
	set_construct_flag(TRUE);
    }
}




/*---------------------------------------------------------------------------
 * �����å��ܥ��� (CHECK BUTTON)
 *---------------------------------------------------------------------------
 *  ���ܥ���򲡤��ȡ������å��ܥå������ɤ�Ĥ֤���롣
 *    �⤦���٥ܥ���򲡤�����롣
 *  ���Ҥ��Ļ��Ƥ롣 (â������٥�ʳ��λҤ���ä�����ư���̤�ݾ�)
 *  ���Ҥ���Ĥˤϡ�q8tk_container_add() ����Ѥ��롣
 *  �������ʥ�
 *	"clicked"	�ܥ��󤬲����줿����ȯ��
 *	"toggled"	�ܥ���ξ��֤��Ѳ���������ȯ��
 *		����OFF���֤ξ��ϡ� "toggled" ��ȯ��
 *		���� ON���֤ξ��ϡ� "clicked" �� "toggled" �ν��ȯ��
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_check_button_new(void)
 *	�����å��ܥ����������
 *
 *  Q8tkWidget	*q8tk_check_button_new_with_label(const char *label)
 *	ʸ���� label �������줿�����å��ܥ����������
 *	����Ū�ˤϡ������å��ܥ���ȥ�٥���ꡢ�ƻҤˤ��롣
 *  -------------------------------------------------------------------------
 *	�����ܤϰ㤦������ǽŪ�ˤϥȥ���ܥ����Ʊ���Ǥ��롣
 *	�ȥ���ܥ����Ʊ�͡�q8tk_toggle_button_set_state() �ˤ�ꡢ
 *	�ܥ���򲡤������֤ˤ��뤳�Ȥ��Ǥ��� (�����ʥ뤬ȯ������)��
 *  -------------------------------------------------------------------------
 *	��CHECK BUTTON�ۢ��� [LABEL]		��٥��Ҥ˻���
 *
 *---------------------------------------------------------------------------*/
static	void	check_button_event_do(Q8tkWidget *widget)
{
    if (widget->stat.button.active == Q8TK_BUTTON_ON) {
	widget->stat.button.active = Q8TK_BUTTON_OFF;
	widget_signal_do(widget, "toggled");
    } else {
	widget->stat.button.active = Q8TK_BUTTON_ON;
	widget_signal_do(widget, "clicked");
	widget_signal_do(widget, "toggled");
    }
    set_construct_flag(TRUE);
}

/* �꥿���󡦥��ڡ����򲡤����顢�����ʥ�ȯ�� */
static	void	check_button_event_key_on(Q8tkWidget *widget, int key)
{
    if (key == Q8TK_KEY_RET || key == Q8TK_KEY_SPACE) {
	check_button_event_do(widget);
    }
}

/* �ܥ����ޥ�������å������顢�ɥ�å����ϤȤ��� */
static	void	check_button_event_button_on(Q8tkWidget *widget)
{
    set_drag_widget(widget);
    set_construct_flag(TRUE);
}

/* �ɥ�å���λ�ǡ������ʥ�ȯ�� (�ɥ�å���ϡ��ʤˤ⤷�ʤ�) */
static	void	check_button_event_drag_off(Q8tkWidget *widget)
{
    if (q8gr_get_focus_screen(mouse.x/8, mouse.y/16) == widget) {
	check_button_event_do(widget);
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Q8tkWidget	*q8tk_check_button_new(void)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_CHECK_BUTTON;
    w->attr = Q8TK_ATTR_CONTAINER;
    w->sensitive = TRUE;

    w->event_button_on = check_button_event_button_on;
    w->event_key_on    = check_button_event_key_on;
    w->event_drag_off  = check_button_event_drag_off;

    return w;
}

Q8tkWidget	*q8tk_check_button_new_with_label(const char *label)
{
    Q8tkWidget *b = q8tk_check_button_new();
    Q8tkWidget *l = q8tk_label_new(label);

    q8tk_widget_show(l);
    q8tk_container_add(b, l);

    b->with_label = TRUE;

    return b;
}




/*---------------------------------------------------------------------------
 * �饸���ܥ��� (RADIO BUTTON)
 *---------------------------------------------------------------------------
 *  �������Ĥ��Υ饸���ܥ���Ʊ�Τǥ��롼�ԥ󥰤Ǥ��롣
 *  ���ܥ���򲡤��ȥ����å�����뤬��Ʊ�����롼�ԥ󥰤��줿
 *    ¾�Υ饸���ܥ���ϡ������å�������롣
 *  ���Ҥ��Ļ��Ƥ롣 (â������٥�ʳ��λҤ���ä�����ư���̤�ݾ�)
 *  ���Ҥ���Ĥˤϡ�q8tk_container_add() ����Ѥ��롣
 *  �������ʥ�
 *	"clicked"	�ܥ��󤬲����줿����ȯ��
 *	"toggled"	�ܥ���ξ��֤��Ѳ���������ȯ��
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_radio_button_new(Q8tkWidget *group)
 *	�饸���ܥ����������
 *	Ʊ�����롼�פȤ���饸���ܥ���������Ϳ���롣(�ǽ�Υܥ���ʤ� NULL)
 *	�ǽ�Υܥ��󤬥����å�����롣���ʹߤΥܥ���ϥ����å�������롣
 *	�ʲ��ϡ�3�ĤΥ饸���ܥ��������������
 *		button[0] = q8tk_radio_button_new(NULL);
 *		button[1] = q8tk_radio_button_new(button[0]);
 *		button[2] = q8tk_radio_button_new(button[1]);
 *
 *  Q8tkWidget	*q8tk_radio_button_new_with_label(Q8List *list,
 *						  const char *label)
 *	ʸ���� label �������줿�饸���ܥ����������
 *	����Ū�ˤϡ��饸���ܥ���ȥ�٥���ꡢ�ƻҤˤ��롣
 *  -------------------------------------------------------------------------
 *	���롼�ԥ󥰤���Ƥ��뤬����ǽŪ�ˤϥȥ���ܥ����Ʊ���Ǥ��롣
 *	�ȥ���ܥ����Ʊ�͡�q8tk_toggle_button_set_state() �ˤ�ꡢ
 *	�ܥ���򲡤������֤ˤ��뤳�Ȥ��Ǥ��롣�ʤ������λ����롼�ԥ󥰤��줿
 *	���٤ƤΥ饸���ܥ���� "toggled" �����ʥ뤬ȯ�����롣
 *  -------------------------------------------------------------------------
 *	��RADIO BUTTON�ۢ��� [LABEL]			��٥��Ҥ˻���
 *		��					LIST ��Ƭ���������ݻ�
 *		����������������������	LIST
 *					����
 *	��RADIO BUTTON�ۢ��� [LABEL]	�á�
 *		��			�â�
 *		����������������������	LIST
 *
 *	�ƥ饸���ܥ������Ω�����������åȤǤ��뤬���ꥹ�Ⱦ���ˤ�ꥰ�롼�פ�
 *	�ʤ��Ƥ��롣�ƥ饸���ܥ���ϡ����Υꥹ�Ȥ���Ƭ���������ݻ����Ƥ��롣
 *	�ܥ���򲡤��ȥ��롼�פ��������Ƥ������������åȤ˥����ʥ뤬ȯ�����롣
 *
 *---------------------------------------------------------------------------*/
static	void	radio_button_event_do(Q8tkWidget *widget)
{
    Q8List *list;

					/* ��ʬ���Ȥ� clicked �����ʥ��ȯ�� */
    widget_signal_do(widget, "clicked");
    if (widget->stat.button.active == Q8TK_BUTTON_ON) return;

					/* �ꥹ�Ȥ򤿤ɤäƼ�ʬ�ʳ��Υܥ���  */
    list = widget->stat.button.list;	/* �򥪥դˤ��� toggled �����ʥ�ȯ�� */
    while (list) {
	if (list->data != widget) {
	    ((Q8tkWidget *)(list->data))->stat.button.active = Q8TK_BUTTON_OFF;
	    widget_signal_do((Q8tkWidget *)(list->data), "toggled");
	}
	list = list->next;
    }
						/* ���Ȥ�ܥ�������Ѳ��ʤ� */
						/* toggled �����ʥ�ȯ��     */
    widget->stat.button.active = Q8TK_BUTTON_ON;

    widget_signal_do(widget, "toggled");

    set_construct_flag(TRUE);
}

/* �꥿���󡦥��ڡ����򲡤����顢�����ʥ�ȯ�� */
/* ����/���������򲡤����顢��/��Υ饸���ܥ���ˡ��ե���������ư */
static	void	radio_button_event_key_on(Q8tkWidget *widget, int key)
{
    if (key == Q8TK_KEY_RET || key == Q8TK_KEY_SPACE) {
	radio_button_event_do(widget);
    }

#ifdef	OLD_FOCUS_CHANGE
	/* DO NOTHING */
#else	/* �����ʥե���������ư���� */
    if (key == Q8TK_KEY_LEFT  ||
	key == Q8TK_KEY_UP    ||
	key == Q8TK_KEY_RIGHT ||
	key == Q8TK_KEY_DOWN) {

#if 0	/* OFF���֤ǥե�����������ξ�硢�������륭���ǥܥ��󲡲��Ȥߤʤ� */
	if (widget->stat.button.active == FALSE) {
	    radio_button_event_do(widget);
	}
	else
#endif
	{
	    Q8List *list = q8_list_find(widget->stat.button.list, widget);

	    if (list) {
		if (key == Q8TK_KEY_LEFT ||
		    key == Q8TK_KEY_UP) {
		    if (list->prev == NULL) list = q8_list_last(list);
		    else                    list = list->prev;
		} else {
		    if (list->next == NULL) list = q8_list_first(list);
		    else                    list = list->next;
		}
		if (list) {
		    q8tk_widget_set_focus((Q8tkWidget *)(list->data));
#if 0	/* ����/���������򲡤����顢��/��Υ饸���ܥ���򲡲��Ȥߤʤ� */
		    radio_button_event_do((Q8tkWidget *)(list->data));
#endif
		}
	    }
	}
    }
#endif
}

/* �ܥ����ޥ�������å������顢�ɥ�å����ϤȤ��� */
static	void	radio_button_event_button_on(Q8tkWidget *widget)
{
    set_drag_widget(widget);
    set_construct_flag(TRUE);
}

/* �ɥ�å���λ�ǡ������ʥ�ȯ�� (�ɥ�å���ϡ��ʤˤ⤷�ʤ�) */
static	void	radio_button_event_drag_off(Q8tkWidget *widget)
{
    if (q8gr_get_focus_screen(mouse.x/8, mouse.y/16) == widget) {
	radio_button_event_do(widget);
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

Q8tkWidget	*q8tk_radio_button_new(Q8tkWidget *group)
{
    Q8tkWidget *w;
    Q8List *list;

    w = malloc_widget();
    w->type = Q8TK_TYPE_RADIO_BUTTON;
    w->attr = Q8TK_ATTR_CONTAINER;
    w->sensitive = TRUE;

    w->key_up_used    = TRUE;
    w->key_down_used  = TRUE;
    w->key_left_used  = TRUE;
    w->key_right_used = TRUE;

    w->event_button_on = radio_button_event_button_on;
    w->event_key_on    = radio_button_event_key_on;
    w->event_drag_off  = radio_button_event_drag_off;

    if (group == NULL) {				/* �ǽ��1���� */
	list = NULL;
	w->stat.button.active = TRUE;
    } else {						/* 2���ܰʹ�   */
	list = group->stat.button.list;
	w->stat.button.active = FALSE;
    }
    w->stat.button.list = q8_list_append(list, w);

    return w;
}

Q8tkWidget	*q8tk_radio_button_new_with_label(Q8tkWidget *group,
						  const char *label)
{
    Q8tkWidget *b = q8tk_radio_button_new(group);
    Q8tkWidget *l = q8tk_label_new(label);

    q8tk_widget_show(l);
    q8tk_container_add(b, l);

    b->with_label = TRUE;

    return b;
}

Q8List	*q8tk_radio_button_get_list(Q8tkWidget *group)
{
    return group->stat.button.list;
}




/*---------------------------------------------------------------------------
 * ����ܥܥå��� (COMBO BOX)
 *---------------------------------------------------------------------------
 *  ������ȥ��ΰ����ġ�
 *  ���Ҥϻ��Ƥʤ���
 *  ��q8tk_combo_append_popdown_strings() �ˤ�ꡢ����ʸ������٥�˻��ä�
 *    �ꥹ�ȥ����ƥ���������������롣����ϰ�������Υݥåץ��åץ�����ɥ�
 *    �ˤ�ɽ������롣
 *  �������ʥ�
 *	"activate"	�ꥹ�ȥ����ƥब���򤵤줿���Ȥˤ�ꡢ
 *			����ȥ��ΰ��ʸ������ѹ������ä�����ȯ��
 *			�ʤ���������ȥ����ˤƥ꥿���󥭡����Ϥ����ä�����ȯ��
 *	"changed"	����ȥ����ˤ�ʸ�����ϡ�ʸ����������ä�����ȯ��
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_combo_new(void)
 *	����ܥܥå������������������ϡ����Ϥ��ԲĤˤʤäƤ��롣
 *
 *  void	q8tk_combo_append_popdown_strings(Q8tkWidget *combo, 
 *						  const char *entry_str,
 *						  const char *disp_str)
 *	����������Υݥåץ��åץ�����ɥ��������뤿��Ρ��ꥹ�ȥ����ƥ��
 *	�������롣���Υꥹ�ȥ����ƥ�ϡ� disp_str ��ʸ������٥�˻��ġ�
 *		����������Υݥåץ��åץ�����ɥ��ˤƤ��Υꥹ�ȥ����ƥब����
 *		���줿��硢 ����ܥܥå����Υ���ȥ��ΰ�� entry_str �����å�
 *		����롣("activate"�����ʥ뤬ȯ������)
 *	disp_str == NULL �ξ��ϡ� disp_str �� entry_str ��Ʊ���ˤʤ롣
 *
 *  const char	*q8tk_combo_get_text(Q8tkWidget *combo)
 *	����ȥ��ΰ�����Ϥ���Ƥ���ʸ������֤�
 *
 *  void	q8tk_combo_set_text(Q8tkWidget *combo, const char *text)
 *	ʸ����򥨥�ȥ��ΰ�����ꤹ�� (�����ʥ��ȯ�����ʤ�)��
 *
 *  void	q8tk_combo_set_editable(Q8tkWidget *combo, int editable)
 *	����ȥ��ΰ�����ϲġ��ԲĤ����ꤹ�롣 editable �� ���ʤ����ϲġ�
 *
 *  void	q8tk_misc_set_size(Q8tkWidget *widget, int width, int height)
 *	����ܥܥå�����ɽ�������� width ����ꤹ�롣 height ��̵����
 *  -------------------------------------------------------------------------
 *	��COMBO BOX��
 *		��
 *		��������ENTRY��
 *
 *	q8tk_combo_append_popdown_strings �ˤ� LIST ITEM �� LABEL ���������롣
 *
 *	��COMBO BOX��
 *	    ��  ��
 *	    ��  ��������ENTRY��
 *	    ��
 *	    ����������  LIST  ����LIST ITEM�ۢ�����LABEL��
 *			����
 *			LIST  ����LIST ITEM�ۢ�����LABEL��
 *			����
 *			LIST  ����LIST ITEM�ۢ�����LABEL��
 *			����
 *
 *	����ܥܥå������ޥ����ǥ���å������ȡ��ʲ��Τ褦�ʥ�����ɥ�
 *	(Q8TK_WINDOW_POPUP) ��ưŪ�������������Υ�����ɥ��� q8tk_grab_add()
 *	����롣�Ĥޤꡢ���Υ�����ɥ��ؤ����ʳ��ϡ�����ʤ��ʤ롣
 *
 *	��WINDOW�ۢ�����LIST BOX�ۢ�����LIST ITEM�ۢ�����LABEL��
 *	    ��				  ����
 *	    ��			  ��  ��LIST ITEM�ۢ�����LABEL��
 *	    ��				  ����
 *	    ��			  ��  ��LIST ITEM�ۢ�����LABEL��
 *	    ��				  ����
 *	    ��
 *	    ������ACCEL GROUP�ۢ�����ACCEL KEY�� �� ��BUTTON(���ߡ�)��
 *
 *	���� LIST ITEM �� LABEL �� q8tk_combo_append_popdown_strings() �ˤ�
 *	��ư�������줿�ꥹ�ȥ����ƥ� (�ȥ�٥�) �Ǥ��롣
 *
 *	�ꥹ�ȥ����ƥ�Τ����줫��Ĥ����򤵤��ȡ������Ǽ�ư��������
 *	�������å�(�ꥹ�ȥ����ƥ�Ͻ���)�����ƺ�����졢��Ȥ�ɽ������롣
 *	���λ������򤷤��ꥹ�ȥ����ƥ�λ���ʸ���󤬡�����ȥ��ΰ��
 *	���åȤ��졢Ʊ���˥����ʥ뤬ȯ�����롣
 *
 *	ESC�����������줿���⡢����ɽ������롣(���ξ������������ʥ�ʤ�)
 *	�����¸����뤿��˥�������졼�����������Ѥ��Ƥ��뤬����������졼��
 *	�����ϡ��������åȤ˥����ʥ�����뤳�Ȥ�������ʤ��Τǡ����ߡ��Υܥ���
 *	(��ɽ��) ���������ơ����Υܥ���˥����ʥ������褦�ˤ��Ƥ��롣
 *	���Υܥ���ϥ����ʥ�ȼ�����ȡ����ƤΥ������åȤ������롣
 *
 *	�ꥹ�Ȥ�Ĺ���ʤä���硢�ꥹ�ȥܥå�����ɽ�������̤˼��ޤ�ʤ����Ȥ�
 *	����Τǡ����ξ��� WINDOW �� LIST BOX �δ֤˥�������ɥ�����ɥ�
 *	���������롣(���̤˼��ޤ뤫�ɤ�����Ƚ�ǤϷ빽�����ø�)
 *
 *---------------------------------------------------------------------------*/

/* �ݥåץ��åץ�����ɥ��ǡ�LIST ITEM ���� or ESC�������� or ������ɥ�����
   �ޥ�������å��������Υ�����Хå��ؿ����ݥåץ��åץ�����ɥ��������� */
static	void	combo_fake_callback(UNUSED_WIDGET, Q8tkWidget *parent)
{
    Q8List *l;

    l = parent->stat.combo.list;		/* l->data �� LIST ITEM */
    while (l) {
	q8tk_signal_handlers_destroy((Q8tkWidget *)(l->data));
	l = l->next;
    }

    q8tk_grab_remove(parent->stat.combo.popup_window);

    if (parent->stat.combo.popup_scrolled_window)
	q8tk_widget_destroy(parent->stat.combo.popup_scrolled_window);

    /* LIST BOX �Ϻ�����뤬�� LIST ITEM �Ϻ�������˻Ĥ� */
    q8tk_widget_destroy(parent->stat.combo.popup_list);
    q8tk_widget_destroy(parent->stat.combo.popup_window);
    q8tk_widget_destroy(parent->stat.combo.popup_fake);
    q8tk_widget_destroy(parent->stat.combo.popup_accel_group);
}

/* �ݥåץ��åץ�����ɥ��� LIST ITEM ��ޥ�������å��������Υ�����Хå� */
static	void	combo_event_list_callback(Q8tkWidget *list_item,
					  Q8tkWidget *parent)
{
    if (strcmp(parent->stat.combo.entry->name, list_item->child->name) == 0) {
	/* ���ߤ����Ϥȡ����򤷤��ꥹ�Ȥ�ʸ����Ʊ���ʤΤǡ������ʥ��̵�� */
    } else {
	/* ���ߤ����Ϥȡ����򤷤��ꥹ�Ȥ�ʸ����Ʊ���ʤΤǡ������ʥ��̵�� */
	q8tk_entry_set_text(parent->stat.combo.entry,
			    list_item->child->name);
	widget_signal_do(parent, "activate");
    }

    combo_fake_callback(NULL, parent);
}

/* ����ܤΰ���ɽ���ܥ����ޥ�������å������顢�ݥåץ��åץ�����ɥ����� */
static	void	combo_event_button_on(Q8tkWidget *widget)
{
    int selected = FALSE;
    Q8List *l;
    int scrolled = FALSE;
    int y_pos = widget->y + 1; /* �ݥåץ��åץ�����ɥ�ɽ������ */

    widget->stat.combo.popup_window = q8tk_window_new(Q8TK_WINDOW_POPUP);
    widget->stat.combo.popup_scrolled_window = NULL;
    widget->stat.combo.popup_list = q8tk_listbox_new();

    l = widget->stat.combo.list;		/* l->data �� LIST ITEM */
    while (l) {
	q8tk_container_add(widget->stat.combo.popup_list,
			   (Q8tkWidget *)(l->data));

    /* ����ȥ��ʸ����ΰ��פ���ꥹ�ȥ����ƥ��������֤Ȥ��롣          */
    /* q8tk_listbox_select_child()�ϥ����ʥ�ȯ������Τǥ����ʥ���Ͽ���˽��� */
	if (selected == FALSE) {		/* l->data->child �� LABEL */
	    if (strcmp(widget->stat.combo.entry->name,
		       ((Q8tkWidget*)(l->data))->child->name) == 0) {
		q8tk_listbox_select_child(widget->stat.combo.popup_list,
					  (Q8tkWidget*)(l->data));
		selected = TRUE;
	    }
	}

	q8tk_signal_connect((Q8tkWidget *)(l->data), "select",
			    combo_event_list_callback, widget);
	l = l->next;
    }


    q8tk_widget_show(widget->stat.combo.popup_list);


#if 1
    if (widget->y + widget->stat.combo.nr_items + 2 <= (24+1)) {
	/* ������ɽ�����Ƥ⡢���̤���Ϥ߽Ф��ʤ� (�̾�� WINDOW ����)*/

	scrolled = FALSE;
	y_pos    = widget->y + 1;

    } else if (widget->y - widget->stat.combo.nr_items - 2 >= (0-1)) {
	/* ������ɽ������С����̤���Ϥ߽Ф��ʤ� (�̾�� WINDOW ����)*/

	scrolled = FALSE;
	y_pos    = widget->y - widget->stat.combo.nr_items - 2;

    } else {
	/* �ɤä���ɽ�����Ƥ⡢���̤���Ϥ߽Ф� (SCROLLED WINDOW ����)*/

	if (widget->y <= 25/2) {	/* �����Τۤ��������Τǡ�������ɽ�� */
	    scrolled = TRUE;
	    y_pos    = widget->y + 1;
	} else {			/* �����Τۤ��������Τǡ�������ɽ�� */
	    scrolled = TRUE;
	    y_pos    = 0-1;
	}
    }

    if (scrolled) {	/* SCROLLED WINDOW �ξ�� */
	int height;

	if (y_pos > widget->y) height = (24+1) - 2 - widget->y;	/* ���� */
	else                   height = widget->y - 2 +1;	/* ���� */
	if (height < 3) height = 3;		/* ���ꤨ�ʤ�����ǰ�ΰ� */

	widget->stat.combo.popup_scrolled_window = 
					q8tk_scrolled_window_new(NULL, NULL);
	q8tk_container_add(widget->stat.combo.popup_scrolled_window, 
			   widget->stat.combo.popup_list);

	/* �岼�ǡ������ƥ��֤�ITEM��ɽ�����֤��Ѥ��Ƥ⤤�����⡣�������� */
	q8tk_listbox_set_placement(widget->stat.combo.popup_list, +1, +1);

	q8tk_scrolled_window_set_policy(
				widget->stat.combo.popup_scrolled_window,
				Q8TK_POLICY_AUTOMATIC, Q8TK_POLICY_ALWAYS);

	if (widget->stat.combo.width) {
	    q8tk_misc_set_size(widget->stat.combo.popup_scrolled_window, 
			       widget->stat.combo.width +3, height);
	} else {
	    q8tk_misc_set_size(widget->stat.combo.popup_scrolled_window, 
			       widget->stat.combo.length +3, height);
	}

	q8tk_widget_show(widget->stat.combo.popup_scrolled_window);
	q8tk_container_add(widget->stat.combo.popup_window, 
			   widget->stat.combo.popup_scrolled_window);

    } else {		/* �Ρ��ޥ� WINDOW �ξ�� */

	q8tk_container_add(widget->stat.combo.popup_window, 
			   widget->stat.combo.popup_list);

    }

#else
		/* ���̤���Ϥ߽Ф����ʻ��ϡ�SCROLLED WINDOW ������ */
    if (widget->y + widget->stat.combo.nr_items + 2 > (24+1)) {

	int height = (24+1) - 2 - widget->y;
	if (height < 3) height = 3;

	widget->stat.combo.popup_scrolled_window = 
					q8tk_scrolled_window_new(NULL, NULL);
	q8tk_container_add(widget->stat.combo.popup_scrolled_window, 
			   widget->stat.combo.popup_list);

	q8tk_scrolled_window_set_policy(
				widget->stat.combo.popup_scrolled_window,
				Q8TK_POLICY_AUTOMATIC, Q8TK_POLICY_ALWAYS);

	if (widget->stat.combo.width) {
	    q8tk_misc_set_size(widget->stat.combo.popup_scrolled_window, 
			       widget->stat.combo.width +3, height);
	} else {
	    q8tk_misc_set_size(widget->stat.combo.popup_scrolled_window, 
			       widget->stat.combo.length +3, height);
	}

	q8tk_widget_show(widget->stat.combo.popup_scrolled_window);
	q8tk_container_add(widget->stat.combo.popup_window, 
			   widget->stat.combo.popup_scrolled_window);

    } else {	/* �̾�ϥ���ץ�� WINDOW ������ */

	q8tk_container_add(widget->stat.combo.popup_window, 
			   widget->stat.combo.popup_list);

    }
#endif

    q8tk_widget_show(widget->stat.combo.popup_window);
    q8tk_grab_add(widget->stat.combo.popup_window);
    q8tk_widget_set_focus(widget->stat.combo.popup_list);

    if (widget->stat.combo.width) {
	q8tk_misc_set_size(widget->stat.combo.popup_list,
			   widget->stat.combo.width, 0);
    }

    widget->stat.combo.popup_window->stat.window.set_position = TRUE;
    widget->stat.combo.popup_window->stat.window.x = 
		widget->x -1 -((widget->stat.combo.popup_scrolled_window)?1:0);
    widget->stat.combo.popup_window->stat.window.y = 
		y_pos;

    q8tk_signal_connect(widget->stat.combo.popup_window,
			"inactivate", combo_fake_callback,widget);


	/* ESC �����򲡤������ꥹ�Ȥ�õ�뤿��Ρ����ߡ������� */

    widget->stat.combo.popup_fake = q8tk_button_new();
    q8tk_signal_connect(widget->stat.combo.popup_fake, "clicked",
			combo_fake_callback, widget);

    widget->stat.combo.popup_accel_group = q8tk_accel_group_new();
    q8tk_accel_group_attach(widget->stat.combo.popup_accel_group,
			    widget->stat.combo.popup_window);
    q8tk_accel_group_add(widget->stat.combo.popup_accel_group, Q8TK_KEY_ESC,
			 widget->stat.combo.popup_fake, "clicked");
}

/* �꥿���󡦥��ڡ����򲡤����顢����ɽ���ܥ���ޥ�������å���Ʊ�������򤹤�*/
/* ���������򲡤�������Ʊ�� */
static	void	combo_event_key_on(Q8tkWidget *widget, int key)
{
    if (key == Q8TK_KEY_RET || key == Q8TK_KEY_SPACE) {
	combo_event_button_on(widget);
    }

#ifdef	OLD_FOCUS_CHANGE
	/* DO NOTHING */
#else	/* �����ʥե���������ư���� */
    if (key == Q8TK_KEY_UP || key == Q8TK_KEY_DOWN) {
	combo_event_button_on(widget);
    }
#endif
}

/* ����ȥ��������Ϥ����ä��Ȥ��������ʥ�ȯ�� */
static	void	combo_event_entry_activate(UNUSED_WIDGET, Q8tkWidget *parent)
{
    widget_signal_do(parent, "activate");
}
static	void	combo_event_entry_changed(UNUSED_WIDGET, Q8tkWidget *parent)
{
    widget_signal_do(parent, "changed");
}


Q8tkWidget	*q8tk_combo_new(void)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_COMBO;
    w->sensitive = TRUE;

    w->key_up_used    = TRUE;
    w->key_down_used  = TRUE;

    w->stat.combo.entry  = q8tk_entry_new();
    w->stat.combo.list   = NULL;
    w->stat.combo.width  = 0;
    w->stat.combo.length = 0;

    w->stat.combo.entry->stat.entry.combo = w;

    q8tk_combo_set_editable(w, FALSE);
    q8tk_widget_show(w->stat.combo.entry);
    q8tk_signal_connect(w->stat.combo.entry, "activate",
			combo_event_entry_activate, w);
    q8tk_signal_connect(w->stat.combo.entry, "changed",
			combo_event_entry_changed, w);

    w->event_button_on = combo_event_button_on;
    w->event_key_on    = combo_event_key_on;

    return w;
}
void		q8tk_combo_append_popdown_strings(Q8tkWidget *combo, 
						  const char *entry_str,
						  const char *disp_str)
{
    int l0, l1, l2;
    Q8tkWidget *list_item;

    if (combo->stat.combo.list == NULL) {	/* ǰ�Τ������� */
	combo->stat.combo.length   = 0;
	combo->stat.combo.nr_items = 0;
    }

		/* ʸ����Ĺ������å� */

    if (entry_str == NULL) entry_str = "";
    if (disp_str  == NULL) disp_str  = entry_str;

    l0 = combo->stat.combo.length;
    l1 = (disp_str) ? q8gr_strlen(kanji_code, disp_str) : 0;
    l2 = q8gr_strlen(kanji_code, entry_str)
		+ ((combo->stat.combo.entry->stat.entry.editable) ? +1 : 0);

    combo->stat.combo.length = Q8TKMAX(Q8TKMAX(l0, l1), l2);


		/* ��٥��դ� LIST ITEM �������������Υݥ��󥿤�ꥹ�Ȥ��ݻ� */

    list_item = q8tk_list_item_new_with_label(disp_str);
    q8tk_widget_show(list_item);
    q8tk_list_item_set_string(list_item, entry_str);

    combo->stat.combo.list = q8_list_append(combo->stat.combo.list, list_item);
    combo->stat.combo.nr_items ++;


		/* ����ܤ��������ˡ�����ʸ����򥻥å� */

    q8tk_entry_set_text(combo->stat.combo.entry, entry_str);

    if (combo->stat.combo.width) {
	q8tk_misc_set_size(combo->stat.combo.entry,
			   combo->stat.combo.width, 0);
    } else {
	q8tk_misc_set_size(combo->stat.combo.entry,
			   combo->stat.combo.length, 0);
    }
}

#if 0
void		q8tk_combo_clear_popdown_strings(Q8tkWidget *combo, 
						 int start, int end)
{
    /* ̤���� */
}
#endif

const	char	*q8tk_combo_get_text(Q8tkWidget *combo)
{
    return combo->stat.combo.entry->name;
}

void		q8tk_combo_set_text(Q8tkWidget *combo, const char *text)
{
    q8tk_entry_set_text(combo->stat.combo.entry, text);
}

void		q8tk_combo_set_editable(Q8tkWidget *combo, int editable)
{
    q8tk_entry_set_editable(combo->stat.combo.entry, editable);
}




/*---------------------------------------------------------------------------
 * �ꥹ�ȥܥå��� (LIST BOX)
 *---------------------------------------------------------------------------
 *  ����ľ�ܥå����Τ褦�ʥ������åȡ�
 *  ��ʣ���λҤ��Ƥ롣  â�����ꥹ�ȥ����ƥ�ʳ��λҤ���äƤϤ����ʤ���
 *  ���Ҥ���Ĥˤϡ�q8tk_container_add() ����Ѥ��롣
 *    �Ҥϡ�q8tk_container_add() ��ƤӽФ������֤˲��ز��ؤ��¤�Ǥ�����
 *  ���ꥹ�ȥ����ƥ�Τ����줫��Ĥ�������֤ˤ��뤳�Ȥ��Ǥ��롣
 *    (����ʤ��ˤ��뤳�Ȥ�Ǥ��뤬��ʣ����������֤ˤ��뤳�ȤϤǤ��ʤ�)
 *  �������ʥ�
 *	"selection_change"	�ꥹ�ȥ����ƥ��ޥ�������å���������
 *				����������֤ȤʤäƤ���ꥹ�ȥ����ƥब
 *				�Ѳ�������ȯ�����롣
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_listbox_new(void)
 *	�ꥹ�ȥܥå�����������
 *
 *  void    q8tk_listbox_clear_items(Q8tkWidget *wlist, int start, int end)
 *	�ҤΥꥹ�ȥ����ƥ�������롣 
 *	�ꥹ�ȥ����ƥ�ϡ�q8tk_container_add()��ƤӽФ������ 0 ������ֹ椬
 *	������Τǡ������ֹ�ǡ��������ꥹ�ȥ����ƥ����ꤹ�롣
 *	end �� start ���⾮�������ϡ����ƤΥꥹ�ȥ����ƥब�������롣
 *	��������ꥹ�ȥ����ƥ�Ͼ��Ǥ���ΤǸ夫������Ѥ��뤳�ȤϤǤ��ʤ���
 *	������Ѥ��������ϤҤȤĤ��� q8tk_container_remove() �ˤƺ�����٤���
 *
 *  void    q8tk_listbox_select_item(Q8tkWidget *wlist, int item)
 *	item ���ֹ�Υꥹ�ȥ����ƥ��������֤ˤ��� (�����ʥ뤬ȯ������)��
 *
 *  void    q8tk_listbox_select_child(Q8tkWidget *wlist, Q8tkWidget *child)
 *	child �Υꥹ�ȥ����ƥ��������֤ˤ��� (�����ʥ뤬ȯ������)��
 *
 *  void    q8tk_misc_set_size(Q8tkWidget *widget, int width, int height)
 *	�ꥹ�ȥܥå�����ɽ�������� width ����ꤹ�롣 height ��̵����
 *
 *  void    q8tk_listbox_set_placement(Q8tkWidget *widget,
 *				       int top_pos, int left_pos)
 *	�ꥹ�ȥܥå�������������ɥ�����ɥ��λҤǤ����硢�ꥹ�ȥܥå�����
 *	�ҤΥꥹ�ȥ����ƥ�Τ����������ƥ��֤ʤ�Τ��������륤�󤹤�褦��
 *	ɽ�����֤�Ĵ������뤬������Ĵ�����֤���ꤹ�롣
 *		top_pos �����ξ��ϡ������ƥ��֤ʤ�Τ�������ɥ������ˡ�
 *		��ξ��ϡ������ƥ��֤ʤ�Τ�������ɥ������ˡ�ɽ�������
 *		�褦��Ĵ������롣�ͤ������ͤ��礭���ʤ�ۤɡ�������ɥ����
 *		����ɽ������롣
 *		left_pos �����ξ��ϡ��ꥹ�ȥ����ƥ����Ƭ������ξ��ϡ�
 *		�ꥹ�ȥ����ƥ��������ɽ�������褦��Ĵ������롣�ͤ������ͤ�
 *		�ط��ʤ���
 *		�ͤ� 0 �ξ�� (�ǥե����) �ϡ� -1 �Ȥߤʤ���롣(�Ĥޤꡢ
 *		������ɥ������˥����ƥ��������ɽ�������褦��Ĵ�������)
 *  -------------------------------------------------------------------------
 *	��LIST BOX�ۢ��� [LIST ITEM] ���� [LABEL]	�ꥹ�ȥ����ƥ� (�ȡ�
 *	   ��		    ����			��٥�) ��Ҥ˻���
 *	   ��	    ��   [LIST ITEM] ���� [LABEL]
 *	   ��		    ����
 *	   ����->   ��   [LIST ITEM] ���� [LABEL]
 *	��������	    ����
 *	�ˤ�        ��   [LIST ITEM] ���� [LABEL]
 *	�����줫��	    ����
 *	������֤�
 *	����
 *	    �ġġ� �� ���򤵤줿 LIST ITEM �� �ޥ�������å����줿���Ȥˤʤ�
 *
 *
 *	�ꥹ�ȥܥå�������������ɥ�����ɥ��λҤǤ����硢��������
 *	������ɥ��˥ե�������������ľ���˥ꥹ�ȥܥå����˥ե���������
 *	�ܤ롣����ˤ�ꡢ�ꥹ�ȥܥå����ϥ������Ϥ�����Ĥ���褦�ˤʤ롣
 *
 *	�������Ϥˤơ�����ꥹ�ȥ����ƥ�����򤷤���硢
 *	���Υꥹ�ȥ����ƥब�ޥ����ǥ���å����줿�Τ�Ʊ��ư��򤹤롣
 *	
 *	q8tk_listbox_select_item() ��q8tk_listbox_select_child() ��
 *	�ꥹ�ȥ����ƥ�����򤷤����Ȥ�ư��㤦������
 *
 *---------------------------------------------------------------------------*/

static void list_scroll_adjust(Q8tkWidget *widget);

/* ʸ�������򲡤����顢����ʸ������Ƭ�ˤ�ĥꥹ�ȥ����ƥब�����ƥ��֤ˤ���  */
/* ���������򲡤����顢�岼�Υꥹ�ȥ����ƥ�򥢥��ƥ��֤ˤ���                */
/* ���ڡ������꥿����򲡤����顢���Υꥹ�ȥ����ƥ�����򤹤� (�����ʥ�ȯ��) */
static	void	list_event_key_on(Q8tkWidget *widget, int key)
{
    Q8tkWidget *w = widget->stat.listbox.active;

    if (w) {
	if (key <= 0xff &&
	    isgraph(key)) {	       /* ���ڡ����ʳ���ʸ�������Ϥ��줿��� */

	    Q8tkWidget *active = w;
	    for (;;) {		       /* �����ƥ��֤ʥꥹ�ȥ����ƥ����� */
		w = w->next;	       /* �Υꥹ�ȥ����ƥ�Ρ���(��٥�)��   */
		if (w == NULL) {       /* ��Ƭʸ���ȥ������ϤΤ����פ����� */
		    w = widget->child; /* ��缡õ���Ƥ������ꥹ�ȥ����ƥ�� */
		}		       /* �����μ�����Ƭ����ꡢ���٤Ƥ�õ�� */

		if (w == active || w == NULL) break;   /* �ʤ���Ф������� */

		if (w->child) {
		    if ((w->name        && w->name[0]        == key) ||
			(w->child->name && w->child->name[0] == key)) {
							/* ���Ĥ��ä���      */
			widget->stat.listbox.active = w;/* ����򥢥��ƥ��֤�*/

			list_scroll_adjust(widget);	/* ɽ�����֤�Ĵ�� */
			break;
		    }
		}
	    }

	} else {		       /* ���ڡ���������ʸ�������Ϥ��줿��� */
	    switch (key) {

	    case Q8TK_KEY_RET:
	    case Q8TK_KEY_SPACE:
		(*w->event_button_on)(w);
		break;

	    case Q8TK_KEY_UP:
		if (w->prev) {
		    widget->stat.listbox.active = w->prev;
							/* 1�����򥢥��ƥ��� */
		    list_scroll_adjust(widget);		/* ɽ�����֤�Ĵ�� */
		}
		break;
	    case Q8TK_KEY_DOWN:
		if (w->next) {
		    widget->stat.listbox.active = w->next;
							/* 1�ĸ�򥢥��ƥ��� */
		    list_scroll_adjust(widget);		/* ɽ�����֤�Ĵ�� */
		}
		break;

	    case Q8TK_KEY_PAGE_UP:
		if (widget->parent &&
		    widget->parent->type == Q8TK_TYPE_SCROLLED_WINDOW &&
		    widget->parent->stat.scrolled.vscrollbar) {

		    int h = widget->parent->stat.scrolled.height -1;
		    if (widget->stat.scrolled.hscrollbar) h -= 3;
		    else                                  h -= 2;
		    for ( ; h>0; h--) {
			if (w->prev == NULL) break;
			w = w->prev;
		    }
		    widget->stat.listbox.active = w;
							/* 1�����򥢥��ƥ��� */
		    list_scroll_adjust(widget);		/* ɽ�����֤�Ĵ�� */
		}
		break;
	    case Q8TK_KEY_PAGE_DOWN:
		if (widget->parent &&
		    widget->parent->type == Q8TK_TYPE_SCROLLED_WINDOW &&
		    widget->parent->stat.scrolled.vscrollbar) {

		    int h = widget->parent->stat.scrolled.height -1;
		    if (widget->stat.scrolled.hscrollbar) h -= 3;
		    else                                  h -= 2;
		    for ( ; h>0; h--) {
			if (w->next == NULL) break;
			w = w->next;
		    }
		    widget->stat.listbox.active = w;
							/* 1�Ǹ�򥢥��ƥ��� */
		    list_scroll_adjust(widget);		/* ɽ�����֤�Ĵ�� */
		}
		break;
	    }
	}
    }
}



static void list_scroll_adjust(Q8tkWidget *widget)
{
    int i;
    Q8tkWidget *c;

    if (widget->parent &&
	widget->parent->type == Q8TK_TYPE_SCROLLED_WINDOW) {

	c = widget->child;
	for (i=0; c ;i++) {
	    if (c == widget->stat.listbox.active) break;
	    c = c->next;
	}

	if (i < widget->parent->stat.scrolled.child_y0) {
	    Q8TK_ADJUSTMENT(widget->parent->stat.scrolled.vadj)->value = i;
	}
	else if (i >=   widget->parent->stat.scrolled.child_y0
		      + widget->parent->stat.scrolled.child_sy) {
	    Q8TK_ADJUSTMENT(widget->parent->stat.scrolled.vadj)->value
				= i -widget->parent->stat.scrolled.child_sy +1;
	}
    }
    set_construct_flag(TRUE);
}





Q8tkWidget	*q8tk_listbox_new(void)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_LISTBOX;
    w->attr = Q8TK_ATTR_CONTAINER;
    w->sensitive = TRUE;

    w->key_up_used    = TRUE;
    w->key_down_used  = TRUE;

    w->event_key_on = list_event_key_on;

    w->stat.listbox.selected = NULL;
    w->stat.listbox.active   = NULL;

    return w;
}

void		q8tk_listbox_clear_items(Q8tkWidget *wlist, int start, int end)
{
    int i, count;
    Q8tkWidget *wk, *c = wlist->child;

    if (c == NULL) return;
    if (start < 0) return;

    if (end < start) count = -1;
    else             count = end - start +1;

    for (i=0; i<start; i++) {			/* �Ҥ� LIST ITEM �� start�� */
	if ((c = c->next) == NULL) return;	/* �ܤޤǽ�ˤ��ɤ�          */
    }
    while (count) {				/* �������� end���ܤޤǤ�    */
	wk = c->next;				/* �ڤ�Υ��                  */
	q8tk_container_remove(wlist, c);		/* LIST ITEM �� LABEL*/
	q8tk_widget_destroy(c);				/* ��쵤�˺������  */
	if ((c = wk) == NULL) break;
	if (count > 0) count --;
    }

    set_construct_flag(TRUE);
}

static	/* LIST BOX �˷Ҥ��ä� LIST �Ρ� name �� s �˰��פ����Τ�õ�� */
Q8tkWidget	*q8tk_listbox_search_items(Q8tkWidget *wlist, const char *s)
{
    Q8tkWidget *c = wlist->child;

    for (;;) {
	if (c == NULL) return NULL;

	if (c->name &&
	    strcmp(c->name, s) == 0) return c;

	c = c->next;
    }
}

void		q8tk_listbox_select_item(Q8tkWidget *wlist, int item)
{
    int i;
    Q8tkWidget *c = wlist->child;

    if (c == NULL) return;

    if (item < 0) c = NULL;
    else {
	for (i=0; i<item; i++) {
	    if ((c = c->next) == NULL) break;
	}
    }

    q8tk_listbox_select_child(wlist, c);
}

void		q8tk_listbox_select_child(Q8tkWidget *wlist, Q8tkWidget *child)
{
    if (wlist->stat.listbox.selected != child) {
	wlist->stat.listbox.selected = child;
	wlist->stat.listbox.active   = child;
	widget_signal_do(wlist, "selection_changed");
	set_construct_flag(TRUE);
    }
    if (child) {
	widget_signal_do(child, "select");
	widget_scrollin_register(child);
    }
}

void		q8tk_listbox_set_placement(Q8tkWidget *widget,
					   int top_pos, int left_pos)
{
    widget->stat.listbox.scrollin_top  = top_pos;
    widget->stat.listbox.scrollin_left = left_pos;
}


/*
 * ����Ϥ���ä��ü������
 *	LIST BOX �� SCROLLED WINDOW �λҤξ��ǡ�SCROLLED WINDOW �Υ�������
 *	�С�(������) ��ư�����줿�Ȥ������δؿ����ƤФ�롣
 *	�����Ǥϡ�SCROLLED WINDOW ��ɽ���ϰϤ˱����ơ�LIST BOX �� active
 *	�������åȤ��ѹ����Ƥ��롣
 */
static	void	list_event_window_scrolled(Q8tkWidget *swin, int sy)
{
    Q8tkWidget *widget = swin->child;	/* == LISTBOX */

    Q8tkWidget *c = widget->child;
    int        nth = 0;

    if (c == NULL) return;

    while (c) {
	if (c == widget->stat.listbox.active) break;
	nth ++;
	c = c->next;
    }
    nth = nth - swin->stat.scrolled.vadj->stat.adj.value;

    if (0 <= nth && nth < sy) {
	/* Ok, No Adjust */
    } else {

	if (nth < 0) nth = swin->stat.scrolled.vadj->stat.adj.value;
	else         nth = swin->stat.scrolled.vadj->stat.adj.value + sy -1;

	c = widget->child;
	while (nth--) {
	    if ((c = c->next) == NULL) return;
	}
	widget->stat.listbox.active   = c;
	set_construct_flag(TRUE);

    }
}




/*---------------------------------------------------------------------------
 * �ꥹ�ȥ����ƥ� (LIST ITEM)
 *---------------------------------------------------------------------------
 *  ���ꥹ�ȥܥå����λҤˤʤ�롣
 *  ���Ҥ��Ļ��Ƥ롣 (â������٥�ʳ��λҤ���ä�����ư���̤�ݾ�)
 *  ���Ҥ���Ĥˤϡ�q8tk_container_add() ����Ѥ��롣
 *  ��������ʸ������ݻ��Ǥ��롣
 *  �������ʥ�
 *	"select"	����å������� (���Ǥ����򤵤줿���֤Ǥ�ȯ��)
 *			�ƤΥꥹ�ȥܥå�����������֤Ȥ��Ƥ���Τ����Ȥ�̵��
 *			��硢��˿Ƥ� "selection_changed" ���٥�Ȥ�ȯ��������
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_list_item_new(void)
 *	�ꥹ�ȥ����ƥ��������
 *
 *  Q8tkWidget	*q8tk_list_item_new_with_label(const char *label)
 *	ʸ���� label �������줿�ꥹ�ȥ����ƥ��������
 *	����Ū�ˤϡ��ꥹ�ȥ����ƥ�ȥ�٥���ꡢ�ƻҤˤ��롣
 *
 *  void	q8tk_list_item_set_string(Q8tkWidget *w, const char *str)
 *	������ʸ���� str ���ݻ����롣
 *  -------------------------------------------------------------------------
 *	[LIST BOX] ���� ��LIST ITEM�ۢ��� [LABEL]	��٥��Ҥ˻���
 *							�Ƥϡ��ꥹ�ȥܥå���
 *	   ��		    ��
 *	   ��		    ��
 *   "selection_change"     ��
 *	   ��		  "select"
 *	   ���ġġġġġ�   ��
 *	��˿Ƥ�	    ��
 *	���Ť���        �ܥ��������
 *
 *---------------------------------------------------------------------------*/
/* �ꥹ�ȥ����ƥ��ޥ�������å������顢�����ʥ�ȯ�� */
static	void	list_item_event_button_on(Q8tkWidget *widget)
{
    if (widget->parent) {
	q8tk_widget_set_focus(widget->parent);

	if (widget->parent->stat.listbox.selected != widget) {
	    widget->parent->stat.listbox.selected = widget;
	    widget->parent->stat.listbox.active   = widget;

#if 0       /* ���Τޤޤ��ȡ����򤷤����֤����褹��ˤ��ʤ� */
	    widget_signal_do(widget->parent, "selection_changed");

#else	    /* ���򤷤����֤����褵�������Τǡ����٥�Ȥ��٤餻�� */
	    set_delay_widget(widget, 6);	/* 6 �ե졼��ʬ�ٱ� */
	    return;
#endif
	}
    }
    widget_signal_do(widget, "select");
    set_construct_flag(TRUE);
}

Q8tkWidget	*q8tk_list_item_new(void)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_LIST_ITEM;
    w->attr = Q8TK_ATTR_CONTAINER | Q8TK_ATTR_LABEL_CONTAINER;
    w->sensitive = TRUE;

    w->event_button_on = list_item_event_button_on;

    return w;
}

Q8tkWidget	*q8tk_list_item_new_with_label(const char *label)
{
    Q8tkWidget *i = q8tk_list_item_new();
    Q8tkWidget *l = q8tk_label_new(label);

    q8tk_widget_show(l);
    q8tk_container_add(i, l);

    i->with_label = TRUE;

    return i;
}

void		q8tk_list_item_set_string(Q8tkWidget *w, const char *str)
{
    if (w->name) free(w->name);
    w->name = (char *)malloc(strlen(str)+1);

    Q8tkAssert(w->name, "memory exhoused");

    strcpy(w->name, str);
    w->code = kanji_code;
}




/*---------------------------------------------------------------------------
 * ��٥� (LABEL)
 *---------------------------------------------------------------------------
 *  ��(ɽ���Ѥ�)ʸ������ݻ��Ǥ���
 *  �����ޤ��ޤʥ���ƥʤλҤˤʤ롣
 *  �������ʥ� �� ̵��
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_label_new(const char *label)
 *	ʸ���� label ���ĥ�٥��������
 *
 *  void	q8tk_label_set(Q8tkWidget *w, const char *label)
 *	��٥��ʸ����� label ���ѹ����롣
 *
 *  void	q8tk_label_set_reverse(Q8tkWidget *w, int reverse)
 *	��٥��ʸ�����ȿžɽ�������ꤹ�롣 reverse �����ʤ�ȿž���롣
 *
 *  void	q8tk_label_set_color(Q8tkWidget *w, int foreground)
 *	��٥��ʸ����ο������ꤹ�롣���� foreground �����ꤹ�롣�ͤ�
 *	Q8GR_PALETTE_xxxx �ǻ��ꤹ�롣�����ͤ���ʤ顢�ǥե���Ȥο����᤹��
 *  -------------------------------------------------------------------------
 *	��LABEL��			�Ҥϻ��Ƥʤ�
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_label_new(const char *label)
{
    Q8tkWidget *w;

    if (label == NULL) label = "";

    w = malloc_widget();
    w->type = Q8TK_TYPE_LABEL;
    w->name = (char *)malloc(strlen(label) + 1);
    w->sensitive = TRUE;

    Q8tkAssert(w->name, "memory exhoused");

    strcpy(w->name, label);
    w->code = kanji_code;

    w->stat.label.foreground = Q8GR_PALETTE_FOREGROUND;
    w->stat.label.background = Q8GR_PALETTE_BACKGROUND;
    w->stat.label.reverse    = FALSE;

    return w;
}
void		q8tk_label_set(Q8tkWidget *w, const char *label)
{
    if (label == NULL) label = "";

    if (w->name) free(w->name);
    w->name = (char *)malloc(strlen(label) + 1);

    Q8tkAssert(w->name, "memory exhoused");

    strcpy(w->name, label);
    w->code = kanji_code;

    set_construct_flag(TRUE);
}
void		q8tk_label_set_reverse(Q8tkWidget *w, int reverse)
{
    if (w->stat.label.reverse != reverse) {
	w->stat.label.reverse = reverse;
	set_construct_flag(TRUE);
    }
}
void		q8tk_label_set_color(Q8tkWidget *w, int foreground)
{
    if (foreground < 0) foreground = Q8GR_PALETTE_FOREGROUND;

    if (w->stat.label.foreground != foreground) {
	w->stat.label.foreground = foreground;
	set_construct_flag(TRUE);
    }
}


/*---------------------------------------------------------------------------
 * �� (LOGO)
 *---------------------------------------------------------------------------
 *  �������ȥ����ɽ���Ǥ���(����,�������ϸ���)
 *  �����ޤ��ޤʥ���ƥʤλҤˤʤ롣
 *  �������ʥ� �� ̵��
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_logo_new(void)
 *	����������
 *
 *  -------------------------------------------------------------------------
 *	��LOGO��			�Ҥϻ��Ƥʤ�
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_logo_new(void)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_LOGO;
    w->sensitive = TRUE;

    return w;
}


/*---------------------------------------------------------------------------
 * �ե졼�� (FRAME)
 *---------------------------------------------------------------------------
 *  ���Ҥ�ҤȤĤ�Ƥ롣
 *  ���Ҥ���Ĥˤϡ�q8tk_container_add() ����Ѥ��롣
 *  ��(���Ф���)ʸ������ݻ��Ǥ��롣
 *  �������ʥ� �� ̵��
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_frame_new(const char *label)
 *	ʸ���� label �򸫽Ф��ˤ�ĥե졼���������
 *  void       q8tk_frame_set_shadow_type(Q8tkWidget *frame, int shadow_type)
 *	�ե졼��μ��������Ǥ��롣
 *		Q8TK_SHADOW_NONE	�����Τʤ��ե졼��
 *		Q8TK_SHADOW_IN		���Τ��ؤ�����ե졼��
 *		Q8TK_SHADOW_OUT		���Τ�����夬�ä��ե졼��
 *		Q8TK_SHADOW_ETCHED_IN	�Ȥ��ؤ�����ե졼��
 *		Q8TK_SHADOW_ETCHED_OUT	�Ȥ�����夬�ä��ե졼��
 *  -------------------------------------------------------------------------
 *	��FRAME�� ���� [xxxx]		�����ʻҤ����Ƥ�
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_frame_new(const char *label)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_FRAME;
    w->attr = Q8TK_ATTR_CONTAINER;
    w->sensitive = TRUE;

    w->stat.frame.shadow_type = Q8TK_SHADOW_OUT;

    w->name = (char *)malloc(strlen(label) + 1);

    Q8tkAssert(w->name, "memory exhoused");

    strcpy(w->name, label);
    w->code = kanji_code;

    return w;
}

void		q8tk_frame_set_shadow_type(Q8tkWidget *frame, int shadow_type)
{
    frame->stat.frame.shadow_type = shadow_type;
}




/*---------------------------------------------------------------------------
 * ��ʿ�ܥå��� (HBOX)
 *---------------------------------------------------------------------------
 *  ��ʣ���λҤ��Ƥ롣
 *  ���Ҥ���Ĥˤϡ�q8tk_box_pack_start() / _end ()����Ѥ��롣
 *  �������ʥ� �� ̵��
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_hbox_new(void)
 *	��ʿ�ܥå�����������
 *  -------------------------------------------------------------------------
 *	��HBOX�ۢ��� [xxxx]		�����ʻҤ����Ƥ�
 *		      ����
 *		��   [xxxx]
 *		      ����
 *		��   [xxxx]
 *		      ����
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_hbox_new(void)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_HBOX;
    w->attr = Q8TK_ATTR_CONTAINER;
    w->sensitive = TRUE;

    return w;
}




/*---------------------------------------------------------------------------
 * ��ľ�ܥå��� (VBOX)
 *---------------------------------------------------------------------------
 *  ��ʣ���λҤ��Ƥ롣
 *  ���Ҥ���Ĥˤϡ�q8tk_box_pack_start() / _end ()����Ѥ��롣
 *  �������ʥ� �� ̵��
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_vbox_new(void)
 *	��ľ�ܥå�����������
 *  -------------------------------------------------------------------------
 *	��VBOX�ۢ��� [xxxx]		�����ʻҤ����Ƥ�
 *		      ����
 *		��   [xxxx]
 *		      ����
 *		��   [xxxx]
 *		      ����
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_vbox_new(void)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_VBOX;
    w->attr = Q8TK_ATTR_CONTAINER;
    w->sensitive = TRUE;

    return w;
}




/*---------------------------------------------------------------------------
 * �Ρ��ȥ֥å� (NOTE BOOK / NOTE PAGE)
 *---------------------------------------------------------------------------
 *  ��ʣ���ΥΡ��ȥڡ������Ƥ롣
 *  ���ƥΡ��ȥڡ����ϥ���ƥʤǤ��ꡢ�Ҥ��Ļ��Ƥ롣
 *  ��q8tk_notebook_append() �ǻҤ���Ĥ��������٤������ǥΡ��ȥڡ�����������
 *    ���줬�Ҥ���Ĥ��Ȥˤʤ롣
 *  ��NOTE PAGE �ϡ�(���Ф���)ʸ������ݻ��Ǥ��롣
 *  �������ʥ�
 *	"switch_page"	�̤Υڡ������ڤ��ؤ�ä�����ȯ��
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_notebook_new(void)
 *	�Ρ��ȥܥå�������
 *
 *  void	q8tk_notebook_append(Q8tkWidget *notebook,
 *				     Q8tkWidget *widget, const char *label)
 *	�Ρ��ȥ֥å� noteboot  �˿����ʥڡ�����������롣
 *	���Υڡ����� widget ���ɲä��롣���Ф���ʸ����� label��
 *	����Ū�ˤϡ��Ρ��ȥڡ������ꡢ�ƻҤˤ��������Ǥ���ˡ�
 *	�Ρ��ȥڡ����� wiget ��ƻҤˤ��롢
 *
 *  int	q8tk_notebook_current_page(Q8tkWidget *notebook)
 *	���ߤΥڡ����ֹ��������롣
 *	�ʤ����Ρ��ȥ֥å��˥ڡ������ɲä�����ˡ�0 ������ֹ椬�����롣
 *	�����ڡ������ɲä��Ƥ��ʤ����� -1 ��
 *
 *  void	q8tk_notebook_set_page(Q8tkWidget *notebook, int page_num)
 *	�Ρ��ȥ֥å��Υڡ�������ꤹ��(�����ʥ뤬ȯ������)��
 *
 *  void	q8tk_notebook_next_page(Q8tkWidget *notebook)
 *	�Ρ��ȥ֥å��Υڡ������Ǥˤ���(�����ʥ뤬ȯ������)��
 *
 *  void	q8tk_notebook_prev_page(Q8tkWidget *notebook)
 *	�Ρ��ȥ֥å��Υڡ��������Ǥˤ���(�����ʥ뤬ȯ������)��
 *  -------------------------------------------------------------------------
 *	��NOTE BOOK�ۢ�����NOTE PAGE�ۢ��� [xxxx]	�Ρ��ȥڡ�����Ҥ˻���
 *			      ����			����˥Ρ��ȥڡ�����
 *		     ��  ��NOTE PAGE�ۢ��� [xxxx]	�����ʻҤ����Ƥ�
 *			      ����
 *		     ��  ��NOTE PAGE�ۢ��� [xxxx]
 *			      ����
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_notebook_new(void)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_NOTEBOOK;
    w->attr = Q8TK_ATTR_CONTAINER;
    w->sensitive = TRUE;

    w->stat.notebook.page = NULL;

    return w;
}

/* �Ρ��ȥڡ����Υ�����ޥ�������å������顢�����ʥ�ȯ�� */
static	void	notepage_event_button_on(Q8tkWidget *widget)
{
    if ((widget->parent)->stat.notebook.page != widget) {
	(widget->parent)->stat.notebook.page = widget;
	widget_signal_do(widget->parent, "switch_page");

	/* �Ρ��ȥڡ�������ľ��ϡ��ե�������(��������饤��)ɽ�����ʤ� */
	/*    �������ʤ�Ȥʤ���ɽ�������å�������������Τǡ����� */
	if ((widget->parent)->stat.notebook.lost_focus) {
	    q8tk_widget_set_focus(NULL);
	}

	set_construct_flag(TRUE);
    }
}

/* �꥿���󡦥��ڡ����򲡤����顢�ޥ�������å���Ʊ�������򤹤� */
/* ���������򲡤����顢����Υڡ����ǤΡ��ޥ�������å���Ʊ�������򤹤� */
static	void	notepage_event_key_on(Q8tkWidget *widget, int key)
{
    if (key == Q8TK_KEY_RET || key == Q8TK_KEY_SPACE) {
	notepage_event_button_on(widget);
    }

#ifdef	OLD_FOCUS_CHANGE
	/* DO NOTHING */
#else	/* �����ʥե���������ư���� */
    {
	Q8tkWidget *w = NULL;

	if (key == Q8TK_KEY_LEFT) {
	    if (widget->prev == NULL) {
		w = widget->next;
		if (w) {
		    while (w->next) {
			w = w->next;
		    }
		}
	    } else {
		w = widget->prev;
	    }
	}
	if (key == Q8TK_KEY_RIGHT) {
	    if (widget->next == NULL) {
		w = widget->prev;
		if (w) {
		    while (w->prev) {
			w = w->prev;
		    }
		}
	    } else {
		w = widget->next;
	    }
	}

	if (w) {
	    q8tk_widget_set_focus(w);
	    notepage_event_button_on(w);
	}
    }
#endif
}

void		q8tk_notebook_append(Q8tkWidget *notebook,
				     Q8tkWidget *widget, const char *label)
{
    Q8tkWidget *w;

    Q8tkAssert(notebook->type == Q8TK_TYPE_NOTEBOOK, NULL);

    w = malloc_widget();
    w->type = Q8TK_TYPE_NOTEPAGE;
    w->attr = Q8TK_ATTR_CONTAINER;
    w->sensitive = TRUE;

    w->key_left_used  = TRUE;
    w->key_right_used = TRUE;

    w->name = (char *)malloc(strlen(label) + 1);

    Q8tkAssert(w->name, "memory exhoused");

    strcpy(w->name, label);
    w->code = kanji_code;

    if (notebook->child == NULL) {		/* �ǽ�� NOTE PAGE �ξ���*/
	notebook->stat.notebook.page = w;	/* ����� ������֤ˤ���    */
    }
    q8tk_box_pack_start(notebook, w);		/* ����ƥʽ����� BOX��Ʊ�� */

    q8tk_container_add(w, widget);
    q8tk_widget_show(w);

    w->event_button_on = notepage_event_button_on;
    w->event_key_on    = notepage_event_key_on;
}

int		q8tk_notebook_current_page(Q8tkWidget *notebook)
{
    Q8tkWidget *child = notebook->child;
    int        i = 0;

    while (child) {
	if (child == notebook->stat.notebook.page) {
	    return i;
	}
	child = child->next;
	i++;
    }
    return -1;
}

void		q8tk_notebook_set_page(Q8tkWidget *notebook, int page_num)
{
    Q8tkWidget *child = notebook->child;
    int        i = 0;

    while (child) {
	if (i == page_num) {
	    if (notebook->stat.notebook.page != child) {
		q8tk_widget_set_focus(child);
		notepage_event_button_on(child);
	    }
	    break;
	} else {
	    child = child->next;
	    i++;
	}
    }
}

void		q8tk_notebook_next_page(Q8tkWidget *notebook)
{
    if (notebook->child) {
	Q8tkWidget *page = notebook->stat.notebook.page;
	if (page && page->next) {
	    q8tk_widget_set_focus(page->next);
	    notepage_event_button_on(page->next);
	}
    }
}

void		q8tk_notebook_prev_page(Q8tkWidget *notebook)
{
    if (notebook->child) {
	Q8tkWidget *page = notebook->stat.notebook.page;
	if (page && page->prev) {
	    q8tk_widget_set_focus(page->prev);
	    notepage_event_button_on(page->prev);
	}
    }
}

void		q8tk_notebook_hook_focus_lost(Q8tkWidget *notebook, 
					      int focus_lost)
{
    notebook->stat.notebook.lost_focus = focus_lost;
}



/*---------------------------------------------------------------------------
 * ��ľ���ѥ졼�� (VSEPARATOR)
 *---------------------------------------------------------------------------
 *  ���Ҥϻ��Ƥʤ���
 *  ��Ĺ�������ƥ������åȤ��礭���ˤ�ꡢưŪ���Ѥ�롣
 *  �������ʥ� �� �ʤ�
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_vseparator_new(void)
 *	��ľ���ѥ졼��������
 *  -------------------------------------------------------------------------
 *	��VSEPARATOR��			�Ҥϻ��Ƥʤ�
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_vseparator_new(void)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_VSEPARATOR;
    w->sensitive = TRUE;

    return w;
}




/*---------------------------------------------------------------------------
 * ��ʿ���ѥ졼�� (HSEPARATOR)
 *---------------------------------------------------------------------------
 *  ���Ҥϻ��Ƥʤ���
 *  ��Ĺ�������ƥ������åȤ��礭���ˤ�ꡢưŪ���Ѥ�롣
 *  �������ʥ� �� �ʤ�
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_hseparator_new(void)
 *	��ʿ���ѥ졼��������
 *  -------------------------------------------------------------------------
 *	��VSEPARATOR��			�Ҥϻ��Ƥʤ�
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_hseparator_new(void)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_HSEPARATOR;
    w->sensitive = TRUE;

    return w;
}




/*---------------------------------------------------------------------------
 * �����㥹�ȥ��� (ADJUSTMENT)
 *---------------------------------------------------------------------------
 *  ��ɽ���Ǥ��ʤ�
 *  ���Ҥϻ��Ƥʤ����Ҥˤʤ뤳�Ȥ�Ǥ��ʤ���
 *  ����������䥹������ɥ�����ɥ�����������ݤˡ�ɬ�פȤʤ롣
 *    ñ�ȤǤϻ��Ѥ��뤳�ȤϤʤ���
 *  �����(�ͤ��ϰ�)����ʬ(��������ʬ���礭����ʬ��2����) ����Ĥ���
 *    ������������˸¤롣
 *  ���������������������ϡ�����äƥ����㥹�ȥ��Ȥ��������Ƥ����Τ�
 *    �����Τ�����Ū��
 *  ����������ɥ�����ɥ�������������ϡ����λ��˥����㥹�ȥ��Ȥ�ư
 *    ����������Τ�����Ū��
 *  �������ʥ�
 *	"value_changed"		�ͤ��Ѥ�ä�����ȯ��
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_adjustment_new(int value, int lower, int upper,
 *				     int step_increment, int page_increment)
 *	�����㥹�ȥ��Ȥ�������
 *	�ͤ��ϰϤ� lower �� upper �������ͤ� value ��
 *	�礭����ʬ�� step_increment ����������ʬ�� page_increment ��
 *
 *  void  q8tk_adjustment_set_value(Q8tkWidget *adj, int value)
 *	�����㥹�ȥ��Ȥθ����ͤ����ꤹ�롣�����ʥ��ȯ�����ʤ���
 *
 *  void  q8tk_adjustment_clamp_page(Q8tkWidget *adj, int lower, int upper)
 *	�ͤ��ϰϤ� lower �� upper �����ꤷ�ʤ��������λ��������ͤ��ϰϳ��ξ��
 *	�����ͤʤ����Ǿ��ͤ����ꤵ��롣�����ʥ��ȯ�����ʤ���
 *
 *  void  q8tk_adjustment_set_increment(Q8tkWidget *adj,
 *				        int step_increment, int page_increment)
 *	�礭����ʬ�ȡ���������ʬ�����ꤷ�ʤ�����
 *
 *  void	q8tk_adjustment_set_arrow(Q8tkWidget *adj, int arrow)
 *	arrow �����ξ�硢�����ɽ�����롣
 *	�ʤ������������å����줿���� �� step_increment ʬ���������롣
 *	�ط���ʬ������å����줿���� �� page_increment ʬ���������롣
 *
 *  void	q8tk_adjustment_set_length(Q8tkWidget *adj, int length)
 *
 *   ------------------------------------------------------------------------
 *	��ADJUSTMENT��			�Ҥϻ��Ƥʤ������Ҥˤ�ʤ�ʤ�
 *
 *---------------------------------------------------------------------------*/
/* �����㥹�ȥ��Ȥ�Ĵ��
   �����㥹�ȥ��Ȥ�ޥ�������å����ɥ�å������ݤ˸ƤӽФ���� */
enum {
    ADJ_STEP_DEC,
    ADJ_STEP_INC,
    ADJ_PAGE_DEC,
    ADJ_PAGE_INC,
    ADJ_SLIDER_DRAG
};
static	void	adjustment_do(Q8tkWidget *widget, int mode)
{
    Q8Adjust *adj = &widget->stat.adj;
    int value_old = adj->value;
    int repeat = TRUE;
    static int old = -1;	/* �ޥ����ܥ��󲡤��äѤʤ��ˤ������ˡ�
				   ���饤�������Ԥä����褿�ꤹ��Τ��ɤ� */

    if (now_mouse_on == FALSE) old = -1;

    switch (mode) {
    case ADJ_STEP_DEC:
	adj->value -= adj->step_increment;
	if (adj->value < adj->lower) {
	    adj->value = adj->lower;
	    repeat = FALSE;
	}
	break;
    case ADJ_STEP_INC:
	adj->value += adj->step_increment;
	if (adj->value > adj->upper) {
	    adj->value = adj->upper;
	    repeat = FALSE;
	}
	break;
    case ADJ_PAGE_DEC:
	if (old == ADJ_PAGE_INC) {
	    mode = -1;
	    repeat = FALSE;
	    break;
	}
	adj->value -= adj->page_increment;
	if (adj->value < adj->lower) {
	    adj->value = adj->lower;
	    repeat = FALSE;
	}
	break;
    case ADJ_PAGE_INC:
	if (old == ADJ_PAGE_DEC) {
	    mode = -1;
	    repeat = FALSE;
	    break;
	}
	adj->value += adj->page_increment;
	if (adj->value > adj->upper) {
	    adj->value = adj->upper;
	    repeat = FALSE;
	}
	break;
    case ADJ_SLIDER_DRAG:
	set_drag_widget(widget);
	return;
    }

    if (value_old != adj->value) {
	widget_signal_do(widget, "value_changed");
	/* �����㥹�������ǡ�LISTBOX�����줿�����㳰�����ե饰ON */
	adj->listbox_changed = TRUE;
    }
    set_construct_flag(TRUE);

    if (repeat) now_mouse_on = TRUE;
    else        now_mouse_on = FALSE;

    old = mode;
}

/* �����㥹�ȥ��Ȥξ�ˤƥޥ�������å������顢��Ĵ���������ʥ�ȯ�� */
static	void	adjustment_event_button_on(Q8tkWidget *widget)
{
    Q8Adjust *adj = &widget->stat.adj;
    int slider_x, slider_y;
    int arrow_lower_x, arrow_lower_y;
    int arrow_upper_x, arrow_upper_y;
    int m_x = mouse.x / 8;
    int m_y = mouse.y / 16;

    if (adj->horizontal) {			/* HORIZONTAL ADJUSTMENT */

	slider_x = adj->x + adj->pos;
	slider_y = adj->y;

	if (m_y == slider_y) {

	    if (adj->arrow) {
		arrow_lower_x = adj->x;		slider_x ++;
		arrow_upper_x = adj->x + adj->length +1;

		if        (m_x == arrow_lower_x) {
		    adjustment_do(widget, ADJ_STEP_DEC);	return;
		} else if (m_x == arrow_upper_x) {
		    adjustment_do(widget, ADJ_STEP_INC);	return;
		}
	    }

	    if        (m_x == slider_x) {
		adjustment_do(widget, ADJ_SLIDER_DRAG);		return;
	    } else if (m_x < slider_x) {
		adjustment_do(widget, ADJ_PAGE_DEC);		return;
	    } else {
		adjustment_do(widget, ADJ_PAGE_INC);		return;
	    }

	} else {
	    return;
	}

    } else {					/* VIRTICAL ADJUSTMENT */

	slider_x = adj->x;
	slider_y = adj->y + adj->pos;

	if (m_x == slider_x) {

	    if (adj->arrow) {
		arrow_lower_y = adj->y;		slider_y ++;
		arrow_upper_y = adj->y + adj->length +1;
		if        (m_y == arrow_lower_y) {
		    adjustment_do(widget, ADJ_STEP_DEC);	return;
		} else if (m_y == arrow_upper_y) {
		    adjustment_do(widget, ADJ_STEP_INC);	return;
		}
	    }

	    if        (m_y == slider_y) {
		adjustment_do(widget, ADJ_SLIDER_DRAG);		return;
	    } else if (m_y < slider_y) {
		adjustment_do(widget, ADJ_PAGE_DEC);		return;
	    } else {
		adjustment_do(widget, ADJ_PAGE_INC);		return;
	    }

	} else {
	    return;
	}

    }
}

/* �����㥹�ȥ��ȤΥ��饤������ɥ�å������顢��Ĵ���������ʥ�ȯ�� */
static	void	adjustment_event_dragging(Q8tkWidget *widget)
{
    Q8Adjust *adj = &widget->stat.adj;
    int m_x = mouse.x / 8;
    int m_y = mouse.y / 16;
    int adj_x = adj->x;
    int adj_y = adj->y;
    int slider = -1;

    if (adj->upper <= adj->lower)  return;

    if (adj->horizontal) {
	if (adj->arrow) adj_x ++;
	if (adj_x <= m_x && m_x < adj_x+adj->length) {
	    slider = m_x - adj_x;
	}
    }else{
	if (adj->arrow) adj_y ++;
	if (adj_y <= m_y && m_y < adj_y+adj->length) {
	    slider = m_y - adj_y;
	}
    }

    if (slider >= 0) {
	float val0 = (slider -1) * adj->scale   + adj->lower;
	float val1 = (slider   ) * adj->scale   + adj->lower;
	int   val;

	if      (slider <= 0)             val = adj->lower;
	else if (slider >= adj->length-1) val = adj->upper;
	else {
	    float base = (float)(adj->upper-adj->lower) / (adj->length-1);
	    int   val2 = (int) ((int)(val1 / base) * base);
	    if (val0 < val2 && val2 <= val1) val =         val2;
	    else                             val = (int) ((val0 + val1) / 2);
	}

	if (adj->value != val) {
	    adj->value = val;
	    widget_signal_do(widget, "value_changed");
	    /* �����㥹�������ǡ�LISTBOX�����줿�����㳰�����ե饰ON */
	    adj->listbox_changed = TRUE;
	}
	set_construct_flag(TRUE);
    }
}

/* �������륭���ʤɤ򲡤����顢�ޥ�������å���Ʊ�������򤹤� */
static	void	adjustment_event_key_on(Q8tkWidget *widget, int key)
{
    Q8Adjust *adj = &widget->stat.adj;

    if (adj->horizontal) {			/* HORIZONTAL ADJUSTMENT */
	if      (key == Q8TK_KEY_LEFT)     adjustment_do(widget, ADJ_STEP_DEC);
	else if (key == Q8TK_KEY_RIGHT)    adjustment_do(widget, ADJ_STEP_INC);
    } else {					/* VIRTICAL ADJUSTMENT */
	if      (key == Q8TK_KEY_UP)       adjustment_do(widget, ADJ_STEP_DEC);
	else if (key == Q8TK_KEY_DOWN)     adjustment_do(widget, ADJ_STEP_INC);
	else if (key == Q8TK_KEY_PAGE_UP)  adjustment_do(widget, ADJ_PAGE_DEC);
	else if (key == Q8TK_KEY_PAGE_DOWN)adjustment_do(widget, ADJ_PAGE_INC);
    }

    /* �������륭���򲡤��äѤʤ��ˤ���ȡ�
       �ޥ����ܥ���β����äѤʤ����ν��������ܤ��Ƥ��ޤ��Τǡ�������ɤ� */
    now_mouse_on = FALSE;
}

Q8tkWidget	*q8tk_adjustment_new(int value, int lower, int upper,
				     int step_increment, int page_increment)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_ADJUSTMENT;
    w->sensitive = TRUE;

    w->event_button_on = adjustment_event_button_on;
    w->event_dragging  = adjustment_event_dragging;
    w->event_key_on    = adjustment_event_key_on;

    w->stat.adj.value = value;
    w->stat.adj.lower = lower;
    w->stat.adj.upper = upper;
    w->stat.adj.step_increment = step_increment;
    w->stat.adj.page_increment = page_increment;

    return w;
}

void		q8tk_adjustment_set_value(Q8tkWidget *adj, int value)
{
    if (value < adj->stat.adj.lower) value = adj->stat.adj.lower;
    if (value > adj->stat.adj.upper) value = adj->stat.adj.upper;

    if (adj->stat.adj.value != value) {
	adj->stat.adj.value = value;
	/* ���Υ����㥹�������ǡ�LISTBOX�����뤳�Ȥ�̵���ϥ�
	  adj->stat.adj.listbox_changed = TRUE;
	*/

	/* �����ʥ��ȯ�������ʤ� */
	/*
	  widget_signal_do(adj, "value_changed");
	*/

	set_construct_flag(TRUE);
    }
}

void		q8tk_adjustment_clamp_page(Q8tkWidget *adj,
					   int lower, int upper)
{
    if (upper < lower) upper = lower;

    adj->stat.adj.lower = lower;
    adj->stat.adj.upper = upper;
    if (adj->stat.adj.value < adj->stat.adj.lower)
				adj->stat.adj.value = adj->stat.adj.lower;
    if (adj->stat.adj.value > adj->stat.adj.upper)
				adj->stat.adj.value = adj->stat.adj.upper;

    /* �����ʥ��ȯ�������ʤ� */

    set_construct_flag(TRUE);
}

void		q8tk_adjustment_set_arrow(Q8tkWidget *adj, int arrow)
{
    if (adj->stat.adj.arrow != arrow) {
	adj->stat.adj.arrow = arrow;
	set_construct_flag(TRUE);
    }
}

void		q8tk_adjustment_set_length(Q8tkWidget *adj, int length)
{
    if (adj->stat.adj.max_length != length) {
	adj->stat.adj.max_length = length;
	set_construct_flag(TRUE);
    }
}

void		q8tk_adjustment_set_increment(Q8tkWidget *adj,
					      int step_increment,
					      int page_increment)
{
    adj->stat.adj.step_increment = step_increment;
    adj->stat.adj.page_increment = page_increment;
}



/*---------------------------------------------------------------------------
 * ��ʿ�������� (HSCALE)
 *---------------------------------------------------------------------------
 *  ��new()���ˡ������ǥ����㥹�ȥ��Ȥ���ꤹ�롣
 *    ��������Υ��(�ϰ�)����ʬ�ϡ����Υ����㥹�ȥ��Ȥ˰�¸���롣
 *  ��new()���ΰ����� NULL �ξ��ϡ���ưŪ�˥����㥹�ȥ��Ȥ���������뤬��
 *    ���λ��Υ�󥸤� 0��10 ����ʬ�� 1 �� 2 �˸���Ǥ��롣(�ѹ���ǽ)
 *  ���Ҥϻ��Ƥʤ�
 *  �������ʥ� �� �ʤ���
 *	�������������㥹�ȥ��Ȥϥ����ʥ������롣
 *	�����㥹�ȥ��Ȥ�ư����������硢�ɤ���äƥ����ʥ�򥻥åȤ��롩
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_hscale_new(Q8tkWidget *adjustment)
 *	��ʿ���������������
 *	ͽ������ѤߤΥ����㥹�ȥ��� adjustment ����ꤹ�롣 
 *	NULL �ʤ������Ǽ�ư��������롣
 *
 *  void	q8tk_scale_set_draw_value(Q8tkWidget *scale, int draw_value)
 *	draw_value �����ʤ顢�����ͤ򥹥饤�����β���ɽ�����롣
 *
 *  void	q8tk_scale_set_value_pos(Q8tkWidget *scale, int pos)
 *	�����ͤ�ɽ��������֤���롣
 *		Q8TK_POS_LEFT	���饤�����κ���ɽ��
 *		Q8TK_POS_RIGHT	���饤�����α���ɽ��
 *		Q8TK_POS_TOP	���饤�����ξ��ɽ��
 *		Q8TK_POS_BOTTOM	���饤�����β���ɽ��
 *  -------------------------------------------------------------------------
 *	��HSCALE��				�Ҥϻ��Ƥʤ�
 *	     ����������	��ADJUSTMENT��
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_hscale_new(Q8tkWidget *adjustment)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_HSCALE;
    w->sensitive = TRUE;

    if (adjustment) {
	w->stat.scale.adj = adjustment;
    } else {
	w->stat.scale.adj = q8tk_adjustment_new(0, 0, 10, 1, 2);
    }

    w->stat.scale.adj->stat.adj.horizontal = TRUE;
    w->stat.scale.adj->parent = w;

    w->stat.scale.adj->key_up_used    = FALSE;
    w->stat.scale.adj->key_down_used  = FALSE;
    w->stat.scale.adj->key_left_used  = TRUE;
    w->stat.scale.adj->key_right_used = TRUE;

    return w;
}




/*---------------------------------------------------------------------------
 * ��ľ�������� (HSCALE)
 *---------------------------------------------------------------------------
 *  ��new()���ˡ������ǥ����㥹�ȥ��Ȥ���ꤹ�롣
 *    ��������Υ��(�ϰ�)����ʬ�ϡ����Υ����㥹�ȥ��Ȥ˰�¸���롣
 *  ��new()���ΰ����� NULL �ξ��ϡ���ưŪ�˥����㥹�ȥ��Ȥ���������뤬��
 *    ���λ��Υ�󥸤� 0��10 ����ʬ�� 1 �� 2 �˸���Ǥ��롣(�ѹ���ǽ)
 *  ���Ҥϻ��Ƥʤ�
 *  �������ʥ� �� �ʤ���
 *	�������������㥹�ȥ��Ȥϥ����ʥ������롣
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_hscale_new(Q8tkWidget *adjustment)
 *	��ľ���������������
 *	ͽ������ѤߤΥ����㥹�ȥ��� adjustment ����ꤹ�롣 
 *	NULL �ʤ������Ǽ�ư��������롣
 *
 *  void	q8tk_scale_set_draw_value(Q8tkWidget *scale, int draw_value)
 *	��ʿ���������Ʊ����
 *
 *  void	q8tk_scale_set_value_pos(Q8tkWidget *scale, int pos)
 *	��ʿ���������Ʊ����
 *  -------------------------------------------------------------------------
 *	��VSCALE��				�Ҥϻ��Ƥʤ�
 *	     ����������	��ADJUSTMENT��
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_vscale_new(Q8tkWidget *adjustment)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_VSCALE;
    w->sensitive = TRUE;

    w->key_up_used    = TRUE;
    w->key_down_used  = TRUE;

    if (adjustment) {
	w->stat.scale.adj = adjustment;
    } else {
	w->stat.scale.adj = q8tk_adjustment_new(0, 0, 10, 1, 2);
    }

    w->stat.scale.adj->stat.adj.horizontal = FALSE;
    w->stat.scale.adj->parent = w;

    w->stat.scale.adj->key_up_used    = TRUE;
    w->stat.scale.adj->key_down_used  = TRUE;
    w->stat.scale.adj->key_left_used  = FALSE;
    w->stat.scale.adj->key_right_used = FALSE;

    return w;
}

void		q8tk_scale_set_value_pos(Q8tkWidget *scale, int pos)
{
    scale->stat.scale.value_pos = pos;
    set_construct_flag(TRUE);
}

void		q8tk_scale_set_draw_value(Q8tkWidget *scale, int draw_value)
{
    scale->stat.scale.draw_value = draw_value;
    set_construct_flag(TRUE);
}



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * ɽ�����Υ������׻� (widget_size()�⤫��ƤФ��)
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
static	void	adjustment_size(Q8Adjust *adj, int *sx, int *sy)
{
    int i, limit;
    int range = (adj->upper - adj->lower + 1);

    if (adj->horizontal) limit = (int) (Q8GR_SCREEN_X * 0.8);
    else                 limit = (int) (Q8GR_SCREEN_Y * 0.8);

    if (adj->max_length <= 2) {
	for (i=1; ; i++) if (range / i < limit) break;
	adj->length = range / i;
    } else {
	adj->length = adj->max_length;
    }
    adj->scale = (float)(range - 1) / (adj->length - 2);
    if      (adj->value == adj->lower) adj->pos = 0;
    else if (adj->value == adj->upper) adj->pos = adj->length-1;
    else {
	adj->pos = (int) ((adj->value - adj->lower) / adj->scale) + 1;
	if (adj->pos >= adj->length-1) adj->pos = adj->length-2;
    }

    if (adj->horizontal) {
	*sx = adj->length + ((adj->arrow) ? +2 : 0);
	*sy = 1;
    }else{
	*sx = 1;
	*sy = adj->length + ((adj->arrow) ? +2 : 0);
    }
}




/*---------------------------------------------------------------------------
 * ��������ɥ�����ɥ� (SCROLLED WINDOW)
 *---------------------------------------------------------------------------
 *  ��new()���ˡ������ǥ����㥹�ȥ��Ȥ���ꤹ�뤬����������Υ��(�ϰ�)��
 *    ���Υ�������ɥ�����ɥ��λҤ��礭���ˤ�äơ�ưŪ���Ѳ����롣
 *     (��ʬ�ϰ����Ѥ����)
 *  ��new()���ΰ����� NULL �ξ��ϡ���ưŪ�˥����㥹�ȥ��Ȥ���������롣
 *    ���λ�����ʬ�� 1 �� 10 �Ǥ��롣
 *    �ä���ͳ���ʤ���С�NULL �ˤ�뼫ư������������ñ�������Ǥ��롣
 *    �ʤ������������������� NULL �ˤ���ȥХ��ꤽ����ͽ�������롣
 *    �ʤΤǡ����� NULL �ޤ��� ���˥����㥹�ȥ��Ȥ����ꡢ�Ȥ��뤳�ȡ�
 *  ���Ҥ��Ļ��Ƥ롣
 *    (â�����Ҥ�¹���������륦����ɥ�����Ĥ褦�ʾ���ư���̤�ݾ�)
 *  �������ʥ� �� �ʤ�
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_scrolled_window_new(Q8tkWidget *hadjustment,
 *					  Q8tkWidget *vadjustment)
 *	��������ɥ�����ɥ���������
 *	ͽ������ѤߤΥ����㥹�ȥ��� adjustment ����ꤹ�롣 
 *	NULL �ʤ������Ǽ�ư��������롣
 *
 *  void	q8tk_scrolled_window_set_policy(Q8tkWidget *scrolledw,
 *						int hscrollbar_policy,
 *						int vscrollbar_policy)
 *	��������С���ɽ����ˡ��ġ������̤����ꤹ�롣
 *		Q8TK_POLICY_ALWAYS	���ɽ������
 *		Q8TK_POLICY_AUTOMATIC	ɽ�����������礭�����Τ�ɽ������
 *		Q8TK_POLICY_NEVER	ɽ�����ʤ�
 *
 *  void	q8tk_misc_set_size(Q8tkWidget *widget, int width, int height)
 *	������ɥ��Υ����� width �� height ����ꤹ�롣
 *  -------------------------------------------------------------------------
 *	��SCROLLED WINDOW�ۢ��� [xxxx]		�����ʻҤ����Ƥ�
 *		�á�
 *		�è���	��ADJUSTMENT��
 *		������	��ADJUSTMENT��
 *
 *
 *
 *	��SCROLLED WINDOW�ۢ��� [LISTBOX]
 *
 *	��������ɥ�����ɥ��λҤ��ꥹ�ȥܥå����λ���
 *	��������ɥ�����ɥ��򥯥�å�����ȡ��ꥹ�ȥܥå�����
 *	�ե������������ꤵ��롣�Ĥޤꡢ�������Ϥ����ƥꥹ�ȥܥå���������롣
 *
 *---------------------------------------------------------------------------*/
/* ������ɥ����ޥ�������å�������
   ľ���λҤ��ꥹ�ȥܥå����ξ��˸¤äơ��ꥹ�ȥܥå����򥢥��ƥ��֤ˤ��� */
static	void	scrolled_window_event_button_on(Q8tkWidget *widget)
{
    if (widget->child && widget->child->type == Q8TK_TYPE_LISTBOX) {
	q8tk_widget_set_focus(widget->child);
    }
}
Q8tkWidget	*q8tk_scrolled_window_new(Q8tkWidget *hadjustment,
					  Q8tkWidget *vadjustment)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_SCROLLED_WINDOW;
    w->attr = Q8TK_ATTR_CONTAINER;
    w->sensitive = TRUE;

    w->stat.scrolled.width  = 10;
    w->stat.scrolled.height = 10;
    w->stat.scrolled.hpolicy = Q8TK_POLICY_ALWAYS;
    w->stat.scrolled.vpolicy = Q8TK_POLICY_ALWAYS;
    w->stat.scrolled.hscrollbar = TRUE;
    w->stat.scrolled.vscrollbar = TRUE;

    if (hadjustment)
	w->stat.scrolled.hadj = hadjustment;
    else {
	w->stat.scrolled.hadj = q8tk_adjustment_new(0, 0, 7, 1, 10);
	w->with_label = TRUE; 
    }
    q8tk_adjustment_set_arrow(w->stat.scrolled.hadj, TRUE);
    q8tk_adjustment_set_length(w->stat.scrolled.hadj, 7);
    w->stat.scrolled.hadj->stat.adj.horizontal = TRUE;
    w->stat.scrolled.hadj->parent = w;
    w->stat.scrolled.hadj->key_up_used    = FALSE;
    w->stat.scrolled.hadj->key_down_used  = FALSE;
    w->stat.scrolled.hadj->key_left_used  = TRUE;
    w->stat.scrolled.hadj->key_right_used = TRUE;

    if (vadjustment)
	w->stat.scrolled.vadj = vadjustment;
    else {
	w->stat.scrolled.vadj = q8tk_adjustment_new(0, 0, 7, 1, 10);
	w->with_label = TRUE;
    }
    q8tk_adjustment_set_arrow(w->stat.scrolled.vadj, TRUE);
    q8tk_adjustment_set_length(w->stat.scrolled.vadj, 7);
    w->stat.scrolled.vadj->stat.adj.horizontal = FALSE;
    w->stat.scrolled.vadj->parent = w;
    w->stat.scrolled.vadj->key_up_used    = TRUE;
    w->stat.scrolled.vadj->key_down_used  = TRUE;
    w->stat.scrolled.vadj->key_left_used  = FALSE;
    w->stat.scrolled.vadj->key_right_used = FALSE;

    w->event_button_on = scrolled_window_event_button_on;

    return w;
}
void		q8tk_scrolled_window_set_policy(Q8tkWidget *scrolledw,
						int hscrollbar_policy,
						int vscrollbar_policy)
{
    scrolledw->stat.scrolled.hpolicy = hscrollbar_policy;
    scrolledw->stat.scrolled.vpolicy = vscrollbar_policy;

    if        (scrolledw->stat.scrolled.hpolicy == Q8TK_POLICY_ALWAYS) {
	scrolledw->stat.scrolled.hscrollbar = TRUE;
    } else if (scrolledw->stat.scrolled.hpolicy == Q8TK_POLICY_NEVER)  {
	scrolledw->stat.scrolled.hscrollbar = FALSE;
    }

    if        (scrolledw->stat.scrolled.vpolicy == Q8TK_POLICY_ALWAYS) {
	scrolledw->stat.scrolled.vscrollbar = TRUE;
    } else if (scrolledw->stat.scrolled.vpolicy == Q8TK_POLICY_NEVER)  {
	scrolledw->stat.scrolled.vscrollbar = FALSE;
    }

    set_construct_flag(TRUE);
}
#if 0
void		q8tk_scrolled_window_set_width_height(Q8tkWidget *w, 
						      int width, int height)
{
    w->stat.scrolled.width  = width;
    w->stat.scrolled.height = height;

    set_construct_flag(TRUE);
}
#endif






/*---------------------------------------------------------------------------
 * ����ȥ꡼ (ENTRY)
 *---------------------------------------------------------------------------
 *  ��ʸ�������Ϥ���ǽ��â����ʸ���� ASCII�Τߡ�ɽ��������ϴ������
 *  ���Ҥϻ��Ƥʤ���
 *  �������ʥ�
 *	"activate"	�꥿���󥭡����Ϥ����ä�����ȯ��
 *	"changed"	ʸ�����ϡ�ʸ����������ä�����ȯ��
 *  ------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_entry_new(void)
 *	����ȥ�����������ϲ�ǽ��ʸ������̵���¡�
 *
 *  Q8tkWidget	*q8tk_entry_new_with_max_length(int max)
 *	����ȥ�������� ���ϲ�ǽ��ʸ�����ϡ�max ʸ���ޤǡ�(max==0��̵����)
 *
 *  void	q8tk_entry_set_text(Q8tkWidget *entry, const char *text)
 *	����ȥ�ˡ�ʸ���� text �����ꤹ�롣�����ʥ��ȯ�����ʤ���
 *	�����������ɽ���Ȥʤꡢɽ�����֤�ʸ������Ƭ�Ȥʤ롣
 *	���������������åȤ������ƥ��֤����Խ���ǽ�ʾ�硢���������ʸ������ü
 *	�˰�ư�����������뤬ɽ�������褦��ɽ�����֤��Ѳ����롣
 *
 *  void	q8tk_entry_set_position(Q8tkWidget *entry, int position)
 *	����ȥ�������ʸ����Υ���������֤� position �����ꤹ�롣
 *	position == -1 �ξ��ϡ����������ɽ������ʤ���
 *	position ��ʸ�������������ξ�硢���������ʸ����������ɽ������롣
 *	��������ɽ�������硢�������뤬ɽ�������褦��ɽ�����֤��Ѳ����롣
 *
 *  void	q8tk_entry_set_max_length(Q8tkWidget *entry, int max)
 *	����ȥ�����ϲ�ǽ��ʸ������ max ���ѹ����롣(max==0��̵����)
 *
 *  void	q8tk_entry_set_editable(Q8tkWidget *entry, int editable)
 *	����ȥ��ΰ�����ϲġ��ԲĤ����ꤹ�롣 editable �� ���ʤ����ϲġ�
 *
 *  void	q8tk_misc_set_size(Q8tkWidget *widget, int width, int height)
 *	����ȥ��ɽ�������� width ����ꤹ�롣 height ��̵����
 *	��������ɽ�������硢�������뤬ɽ�������褦��ɽ�����֤��Ѳ����롣
 *  -------------------------------------------------------------------------
 *	��ENTRY��			�Ҥϻ��Ƥʤ�
 *
 *---------------------------------------------------------------------------*/
/*
 * strlen()�η�̤� strsize �Ǥ���褦��ʸ����򥻥åȲ�ǽ�ʥ�������
 */
static	void	q8tk_entry_malloc(Q8tkWidget *entry, int strsize)
{
#if 0
	/* strsize ��� 1�Х���;ʬ�˳��� ('\0' ��ʬ��;ʬ��) */
    int malloc_size = strsize + 1;
#else
	/* 1�Х���ñ�̤Ǥʤ���512�Х���ñ�̤ǳ��ݤ���褦�ˤ��Ƥߤ褦 */
    int malloc_size = (((strsize + 1)/512) +1) * 512;
#endif

    if (entry->name == NULL ||
	entry->stat.entry.malloc_length < malloc_size) {
	if (entry->name == NULL) {
	    entry->name = (char *)malloc(malloc_size);
	} else {
	    entry->name = (char *)realloc(entry->name, malloc_size);
	}

	Q8tkAssert(entry->name, "memory exhoused");

	entry->stat.entry.malloc_length = malloc_size;
    }
}

/* ����ȥ��ޥ�������å������顢���������ư (�Խ���ǽ���Τ�) */
static	void	entry_event_button_on(Q8tkWidget *widget)
{
    int m_x = mouse.x / 8;

    if (widget->stat.entry.editable == FALSE) return;

    if (widget->stat.entry.cursor_pos < 0) {	/* ����������ɽ���ʤ�       */
						/* ʸ���������˥��������ư */
	q8tk_entry_set_position(widget,
				q8gr_strlen(widget->code, widget->name));

    } else					/* ��������ɽ������ʤ�     */
    {						/* �ޥ������֤˥��������ư */
	q8tk_entry_set_position(widget,		
				m_x - widget->x + widget->stat.entry.disp_pos);
    }
    set_construct_flag(TRUE);
}

/* �������������򲡤����顢���������ư           (�Խ���ǽ���Τ�) */
/* ʸ�������򲡤����顢ʸ�����Խ����������ʥ�ȯ�� (�Խ���ǽ���Τ�) */
/* �꥿����򲡤����顢�����ʥ�ȯ��               (�Խ���ǽ���Τ�) */
static	void	entry_event_key_on(Q8tkWidget *widget, int key)
{
    if (widget->stat.entry.editable == FALSE) return;
    if (widget->stat.entry.cursor_pos < 0) return;

    switch (key) {
    case Q8TK_KEY_LEFT:
	if (widget->stat.entry.cursor_pos) {
	    q8tk_entry_set_position(widget, widget->stat.entry.cursor_pos - 1);
	}
	break;

    case Q8TK_KEY_RIGHT:
	{
	    int w = 1;
	    if (q8gr_strchk(widget->code, widget->name, 
			    widget->stat.entry.cursor_pos) == 1) { w = 2; }
	    q8tk_entry_set_position(widget, widget->stat.entry.cursor_pos + w);
	}
	break;

    case Q8TK_KEY_UP:
    case Q8TK_KEY_DOWN:
#ifdef	OLD_FOCUS_CHANGE
	/* DO NOTHING */
#else	/* �����ʥե���������ư���� */
	if (widget->stat.entry.combo) {
	    Q8List *l = widget->stat.entry.combo->stat.combo.list;
	    /* ����ܤ��۲��ˤ����硢�����ǥ���ܤΥꥹ�Ȥ�귫�� */
	    while (l) {
		if (strcmp(widget->name,
			   ((Q8tkWidget*)(l->data))->child->name) == 0) {
		    if (key == Q8TK_KEY_UP) l = l->prev;
		    else                    l = l->next;
		    if (l) {
			q8tk_entry_set_text(widget,
					((Q8tkWidget*)(l->data))->child->name);
			widget_signal_do(widget, "activate");
			return;
		    } else {
			break;
		    }
		}
		l = l->next;
	    }
	    /* �귫���оݤ��ʤ���С��ꥹ�Ȥ���Ƭ(����)�򥻥å� */
	    l = widget->stat.entry.combo->stat.combo.list;
	    if (l) {
		if (key == Q8TK_KEY_UP) l = q8_list_first(l);
		else                    l = q8_list_last(l);
		if (l) {
		    q8tk_entry_set_text(widget,
					((Q8tkWidget*)(l->data))->child->name);
		    widget_signal_do(widget, "activate");
		    return;
		}
	    }
	}
#endif
	if (key == Q8TK_KEY_UP)
	    q8tk_entry_set_position(widget, 0);
	else                   
	    q8tk_entry_set_position(widget,
				    q8gr_strlen(widget->code, widget->name));
	break;

    case Q8TK_KEY_PAGE_UP:
	q8tk_entry_set_position(widget, 0);
	break;

    case Q8TK_KEY_PAGE_DOWN:
	q8tk_entry_set_position(widget,
				q8gr_strlen(widget->code, widget->name));
	break;

    case Q8TK_KEY_RET:
	widget_signal_do(widget, "activate");
	break;

    case Q8TK_KEY_BS:
	if (widget->stat.entry.cursor_pos > 0) {
	    int w = q8gr_strdel(widget->code, widget->name,
				widget->stat.entry.cursor_pos - 1);
	    /* ���������1ʸ��ʬ�����˰�ư */
	    q8tk_entry_set_position(widget, widget->stat.entry.cursor_pos - w);
	    widget_signal_do(widget, "changed");
	}
	break;

    case Q8TK_KEY_DEL:
	if (widget->stat.entry.cursor_pos < (int)q8gr_strlen(widget->code,
							     widget->name)) {
	    q8gr_strdel(widget->code, widget->name,
			widget->stat.entry.cursor_pos);
	    /* ��������ΰ�ư������ */
	    set_construct_flag(TRUE);
	    widget_signal_do(widget, "changed");
	}
	break;

    default:
	if (key <= 0xff && isprint(key)) {
	    int len = (int)strlen(widget->name) + 1;	/* 1�Х���ʸ�����ɲ� */
	    if (widget->stat.entry.max_length &&	/* ʸ�����¥����� */
		widget->stat.entry.max_length < len) {	/* ��Ķ������̵��   */
		;
	    } else {
		q8tk_entry_malloc(widget, len);	/* ���꤬­��ʤ��ʤ���� */

		if (q8gr_stradd(widget->code, widget->name,
				widget->stat.entry.cursor_pos, key)) {
		    /* ���������1ʸ��ʬ�����˰�ư */
		    q8tk_entry_set_position(widget,
					    widget->stat.entry.cursor_pos + 1);
		    widget_signal_do(widget, "changed");
		}
	    }
	}
	break;
    }
}

Q8tkWidget	*q8tk_entry_new(void)
{
    return q8tk_entry_new_with_max_length(0);
}

Q8tkWidget	*q8tk_entry_new_with_max_length(int max)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_ENTRY;
    w->sensitive = TRUE;

    w->key_up_used    = TRUE;
    w->key_down_used  = TRUE;
    w->key_left_used  = TRUE;
    w->key_right_used = TRUE;

    q8tk_entry_malloc(w, max);

    w->name[0] = '\0';
    w->code = kanji_code;

    w->stat.entry.max_length    = max;
/*  w->stat.entry.malloc_length = q8tk_entry_malloc()�ˤ�����Ѥ� */
    w->stat.entry.disp_pos      = 0;
    w->stat.entry.cursor_pos    = 0;
    w->stat.entry.width         = 8;
    w->stat.entry.editable      = TRUE;
    w->stat.entry.combo         = NULL;

    w->event_button_on = entry_event_button_on;
    w->event_key_on    = entry_event_key_on;

    return w;
}

const	char	*q8tk_entry_get_text(Q8tkWidget *entry)
{
    return entry->name;
}

void		q8tk_entry_set_text(Q8tkWidget *entry, const char *text)
{
    int len = strlen(text);

    if (entry->stat.entry.max_length == 0) {	/* ��¤ʤ� */

	q8tk_entry_malloc(entry, len);	/* ���꤬­��ʤ��ʤ顢���� */
	strcpy(entry->name, text);

    } else {					/* ��¤��� */

	q8gr_strncpy(entry->code,
		     entry->name, text, entry->stat.entry.max_length - 1);
	entry->name[ entry->stat.entry.max_length - 1 ] = '\0';

    }

    entry->stat.entry.disp_pos = 0;		/* ʸ������Ƭ����ɽ�� */
    q8tk_entry_set_position(entry, -1);		/* ����������ɽ��     */
}

void		q8tk_entry_set_position(Q8tkWidget *entry, int position)
{
    int disp_pos;
    int tail = q8gr_strlen(entry->code, entry->name);

    if (position < 0) {
	entry->stat.entry.cursor_pos = -1;
	set_construct_flag(TRUE);
	return;
    }

    if (tail < position) position = tail;
    if (q8gr_strchk(entry->code, entry->name, position) == 2) {
	position -= 1;
    }

	/* ɽ���ϰϤ�����˥������륻�å� */
    if (position < entry->stat.entry.disp_pos) {

	entry->stat.entry.disp_pos   = position;
	entry->stat.entry.cursor_pos = position;

    }	/* ɽ���ϰϤ����˥������륻�å� */
    else if (entry->stat.entry.disp_pos + entry->stat.entry.width <= position){

	disp_pos = position - entry->stat.entry.width + 1;
	if (q8gr_strchk(entry->code, entry->name, disp_pos) == 2) {
	    disp_pos += 1;
	}
	entry->stat.entry.disp_pos   = disp_pos;
	entry->stat.entry.cursor_pos = position;

    }	/* ɽ���ϰϤ����� (����ʸ����2�Х�����)�˥������륻�å� */
    else if ((entry->stat.entry.disp_pos + entry->stat.entry.width - 1
								== position) &&
	     q8gr_strchk(entry->code, entry->name, position) == 1) {

	disp_pos = entry->stat.entry.disp_pos +1;
	if (q8gr_strchk(entry->code, entry->name, disp_pos) == 2) {
	    disp_pos += 1;
	}
	entry->stat.entry.disp_pos   = disp_pos;
	entry->stat.entry.cursor_pos = position;
    
    }	/* ����ʳ� (ɽ���ϰ���˥������륻�å�) */
    else {

	entry->stat.entry.cursor_pos = position;

    }
    q8gr_set_cursor_blink();
    set_construct_flag(TRUE);
}

void		q8tk_entry_set_max_length(Q8tkWidget *entry, int max)
{
    q8tk_entry_malloc(entry, max);	/* ���꤬­��ʤ��ʤ顢���� */

    entry->stat.entry.max_length = max;
}

void		q8tk_entry_set_editable(Q8tkWidget *entry, int editable)
{
    if (entry->stat.entry.editable != editable) {
	entry->stat.entry.editable = editable;
	set_construct_flag(TRUE);
    }
}





/*---------------------------------------------------------------------------
 * ��������졼�����롼�� (ACCEL GROUP)
 * ��������졼��������   (ACCEL KEY)
 *---------------------------------------------------------------------------
 *  ���Ҥϻ��Ƥʤ�����ɽ����Ǥ��ʤ���
 *  ������Ū�ˤϡ���������졼�����롼�פ��ơ���������졼���������Ҥ�
 *    ���֤�Ȥ�
 *  ����������졼�����롼�פ򥦥���ɥ��˴�Ϣ�Ť��뤳�Ȥǡ���������졼��
 *    �����򤽤Υ�����ɥ��ˤ�ͭ���ˤ��뤳�Ȥ��Ǥ��롣
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_accel_group_new(void)
 *	��������졼�����롼�פ�������
 *
 *  void	q8tk_accel_group_attach(Q8tkWidget *accel_group, 
 *					Q8tkWidget *window)
 *	��������졼�����롼�פ򡢥�����ɥ� window �˴�Ϣ�Ť��롣
 *
 *  void	q8tk_accel_group_detach(Q8tkWidget *accel_group, 
 *					Q8tkWidget *window)
 *	������ɥ� window �˴�Ϣ�Ť�����������졼�����롼�פ��ڤ�Υ����
 *
 *  void	q8tk_accel_group_add(Q8tkWidget *accel_group, int accel_key,
 *				     Q8tkWidget *widget, const char *signal)
 *	��������졼�����������ꤹ�롣
 *	��������졼�����롼�� accel_group �ˡ����� accel_key �����ꤹ�롣
 *	���Υ������������줿�顢 �������å� widget �� �����ʥ� signal �����롣
 *  -------------------------------------------------------------------------
 *	[WINDOW]
 *	   ��
 *	   ��������ACCEL GROUP�ۢ�����ACCEL KEY��
 *					����
 *				��  ��ACCEL KEY��
 *					����
 *				��  ��ACCEL KEY��
 *					����
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_accel_group_new(void)
{
    Q8tkWidget *w;

    w = malloc_widget();
    w->type = Q8TK_TYPE_ACCEL_GROUP;
    w->sensitive = TRUE;

    return w;
}

void		q8tk_accel_group_attach(Q8tkWidget *accel_group,
					Q8tkWidget *window)
{
    Q8tkAssert(window->type == Q8TK_TYPE_WINDOW, NULL);
    window->stat.window.accel = accel_group;
}

void		q8tk_accel_group_detach(Q8tkWidget *accel_group,
					Q8tkWidget *window)
{
    Q8tkAssert(window->type == Q8TK_TYPE_WINDOW, NULL);

    if (window->stat.window.accel == accel_group) {
	window->stat.window.accel = NULL;
    }
}

void		q8tk_accel_group_add(Q8tkWidget *accel_group, int accel_key,
				     Q8tkWidget *widget, const char *signal)
{
    Q8tkWidget *w;
    w = malloc_widget();

    if (accel_group->child) {
	Q8tkWidget *c = accel_group->child;
	while (c->next) {
	    c = c->next;
	}
	c->next = w;
	w->prev = c;
	w->next = NULL;
    } else {
	accel_group->child = w;
	w->prev = NULL;
	w->next = NULL;
    }
    w->parent = accel_group;

    w->type = Q8TK_TYPE_ACCEL_KEY;
    w->sensitive = TRUE;
    w->name = (char *)malloc(strlen(signal) + 1);

    Q8tkAssert(w->name, "memory exhoused");

    strcpy(w->name, signal);
    w->code = Q8TK_KANJI_ANK;

    w->stat.accel.key    = accel_key;
    w->stat.accel.widget = widget;
}




/*---------------------------------------------------------------------------
 * �������� (DIALOG)
 *---------------------------------------------------------------------------
 *  ����������������ɥ��򳫤���
 *		+-------------------+
 *		| WARING !          |��
 *		| Quit, really?     |������ʬ�ϡ�vbox (��ľ�ܥå���)
 *		| press button.     |��
 *		| +------+ +------+ |
 *		| |  OK  | |CANCEL| |
 *		| +------+ +------+ |
 *		+-------------------+
 *		  �� ������ʬ��  ��
 *		     action_area (��ʿ�ܥå���)
 *  ����������������������ϡ� vbox �� action_area ��Ǥ�դΥ������åȤ�
 *    ����Ǥ��롣q8tk_box_pack_start() / end() �ˤ����ꤹ�롣
 *  �����Υ�����ɥ��ϥ⡼���쥹�ʤΤ���ա�
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_dialog_new(void)
 *	����������������
 *
 *	������������������������ vbox �˥������åȤ��ɲä�������㡣
 *		d = q8tk_dialog_new();
 *		q8tk_box_pack_start(Q8TK_DIALOGE(d)->vbox, label);
 *
 *	������������������������ action_area �˥������åȤ��ɲä�������㡣
 *		d = q8tk_dialog_new();
 *		q8tk_box_pack_start(Q8TK_DIALOGE(d)->action_area, button);
 *  -------------------------------------------------------------------------
 *	    ���ġĢ���DIALOG��
 *	    ��
 *	��WINDOW�ۢ�����FRAME�ۢ�����VBOX�ۢ�����VBOX(vbox)��
 *			 	                    ����
 *				           ��  ��HBOX(action_area)��
 *
 *	q8tk_dialog_new()���֤��ͤ� ��WINDOW�� �Υ������åȤǤ��롣
 *	�⡼����ˤ�����ϡ����Υ������åȤ� q8tk_grab_add() ���롣
 *
 *	vbox��action_area �ϥ桼����Ǥ�դ˥������åȤ��ɲäǤ��롣
 *
 *	q8tk_widget_destroy() �κݤϡ��������ƤΥ������åȤ��������뤬��
 *	�桼���� vbox��action_area ���ɲä����������åȤϺ������ʤ���
 *
 *---------------------------------------------------------------------------*/
Q8tkWidget	*q8tk_dialog_new(void)
{
    Q8tkWidget *dialog, *window, *vbox, *frame;

    dialog = malloc_widget();
    dialog->type = Q8TK_TYPE_DIALOG;
    dialog->sensitive = TRUE;

    window = q8tk_window_new(Q8TK_WINDOW_DIALOG);
    window->stat.window.work = dialog;

    frame = q8tk_frame_new("");
    q8tk_container_add(window, frame);
    q8tk_widget_show(frame);
    q8tk_frame_set_shadow_type(frame, Q8TK_SHADOW_NONE);

    vbox = q8tk_vbox_new();
    q8tk_container_add(frame, vbox);
    q8tk_widget_show(vbox);

    dialog->stat.dialog.vbox = q8tk_vbox_new();
    q8tk_box_pack_start(vbox, dialog->stat.dialog.vbox);
    q8tk_widget_show(dialog->stat.dialog.vbox);

    dialog->stat.dialog.action_area = q8tk_hbox_new();
    q8tk_box_pack_start(vbox, dialog->stat.dialog.action_area);
    q8tk_widget_show(dialog->stat.dialog.action_area);

    return window;
}




/*---------------------------------------------------------------------------
 * FILE SELECTION
 *---------------------------------------------------------------------------
 *  ���������åȤ��Ȥ߹�碌�ơ��ե����륻�쥯��������Ф���
 *  ��������ե�����̾��Ĺ���ϡ��ѥ���ޤ�ƺ����
 *    Q8TK_MAX_FILENAME �ޤǤǤ��롣(ʸ����ü��\0��ޤ�)
 *  ���ե�����̾�ΰ����ϡ������ƥ��¸�ؿ��ˤƽ������롣
 *  -------------------------------------------------------------------------
 *  Q8tkWidget	*q8tk_file_selection_new(const char *title, int select_ro)
 *	�ե����륻�쥯������������
 *	���Ф�ɽ���Ѥ�ʸ���� title ����ꤹ�롣
 *	select_ro �� 1 �ʤ顢��Read Only�ץ����å��ܥ���ɽ�����졢�����å�
 *	����롣0 �ʤ顢��Read Only�ץ����å��ܥ���ɽ������뤬�������å���
 *	����ʤ���-1 �ʤ顢��Read Only�ץ����å��ܥ����ɽ������ʤ���
 *	 (�����å����줿���ɤ����ϡ� q8tk_file_selection_get_readonly �ˤ�
 *	  �����Ǥ���Τǡ��ƤӽФ����ǥ꡼�ɥ���꡼�ǳ������ɤ�������ꤹ��)
 *
 *  const char	*q8tk_file_selection_get_filename(Q8tkWidget *fselect)
 *	����(����)����Ƥ���ե�����̾��������롣
 *
 *  void	q8tk_file_selection_set_filename(Q8tkWidget *fselect,
 *						 const char *filename)
 *	�ե�����̾ (�ǥ��쥯�ȥ�̾�Ǥ��) ����ꤹ�롣
 *
 *  int		q8tk_file_selection_get_readonly(Q8tkWidget *fselect)
 *	�� �ʤ顢�֥꡼�ɥ���꡼�ǳ����פ������å�����Ƥ��롣
 *  -------------------------------------------------------------------------
 *	    ���ġĢ���FILE SELECTION��
 *	    ��
 *	��WINDOW�ۢ���
 *	��������������
 *	������VBOX�ۢ�����LABEL��	q8tk_file_selection_new()�ΰ���
 *			  ����
 *		    ��  ��HSEPARATOR��
 *			  ����
 *		    ��  ��LABEL��	�ǥ��쥯�ȥ�̾
 *			  ����
 *		    ��  ��HBOX�ۢ�����LABEL�� (������)
 *			  ����	      ����
 *			  �á�  ��  ��SCROLLED WINDOW�ۢ�����LIST BOX�ۢ���(A)
 *			  �á�	      ����    �á�
 *			  �á�	      �á�    �è���  ��ADJUSTMENT��
 *			  �á�	      �á�    ������  ��ADJUSTMENT��
 *			  �á�	      �â�
 *			  �á�  ��  ��LABEL�� (������)
 *			  �á�	      ����
 *			  �á�  ��  ��VBOX�ۢ�����LABEL�� (������)
 *			  �á�			  ����
 *			  �á�		    ��  ��LABEL�� �ե������
 *			  �á�			  ����
 *			  �á�		    ��  ��LABEL�� (�����ޡ�4)
 *			  �á�			  ����
 *			  �á�		    ��  ��BUTTON�ۢ�����LABEL��
 *			  �á�			  ����		(view_button)
 *			  �á�			  �â�
 *			  �á�		    ��  ��LABEL�� (�����ޡ�2)
 *			  �á�			  ����
 *			  �á�		    ��  ��CHECK BUTTON�ۢ�����LABEL��
 *			  �á�			  ����		      Read only
 *			  �á�			  �â�
 *			  �á�		    ��  ��BUTTON�ۢ�����LABEL��
 *			  �á�			  ����		(ok_button)
 *			  �á�			  �â�
 *			  �á�		    ��  ��BUTTON�ۢ�����LABEL��
 *			  �á�					(cancel_button)
 *			  �â�
 *		    ��  ��LABEL��	(����)
 *			  ����
 *		    ��  ��HBOX�ۢ�����LABEL��	Filename(���Ф�)
 *				      ����
 *				��  ��ENTRY��	�ե�����̾����
 *
 *	q8tk_file_selection_new() ���֤��ͤ� ��WINDOW�� �Υ������åȤǤ��롣
 *	�⡼����ˤ�����ϡ����Υ������åȤ� q8tk_grab_add() ���롣
 *
 *	cancel_button��ok_button �ϥ桼����Ǥ�դ˥����ʥ������Ǥ��롣
 *
 *		fsel = q8tk_file_selection_new();
 *		q8tk_widget_show(fsel);
 *		q8tk_grab_add(fsel);
 *		q8tk_signal_connect(Q8TK_FILE_SELECTION(fsel)->ok_button,
 *		 		    "clicked", �桼���ؿ����桼������);
 *		q8tk_signal_connect(Q8TK_FILE_SELECTION(fsel)->cancel_button,
 *		 		    "clicked", �桼���ؿ����桼������);
 *
 *	---------------------------------------------------------------------
 *		(A)  ������LIST ITEM�ۢ�����LABEL��
 *		     	   ����
 *		     ��  ��LIST ITEM�ۢ�����LABEL��
 *		     	   ����
 *		     ��  ��LIST ITEM�ۢ�����LABEL��
 *		     	   ����
 *
 *	q8tk_file_selection_set_filename() �ˤƥǥ��쥯�ȥ�̾�򥻥åȤ���ȡ�
 *	���Υǥ��쥯�ȥ�Υե�����������������(���ν����ϥ����ƥ��¸)��
 *	���ƤΥե�����̾��ꥹ�ȥ����ƥ�ˤ��ơ��ꥹ�ȥܥå����˾褻�롣
 *	�ե�����̾�򥻥åȤ������⡢��������ǥ��쥯�ȥ�̾���ڤ�Ф���
 *	(�����ν����⥷���ƥ��¸)��Ʊ�ͤ˥ǥ��쥯�ȥ�Υե��������������
 *	���롣���ξ�硢����ȥ�ˤ�ե�����̾�򥻥åȤ��롣
 *
 *	---------------------------------------------------------------------
 *	�ʲ��ξ�硢ok_button �� "clicked" �����ʥ뤬ȯ�����롣
 *		��OK �Υܥ���򥯥�å��������
 *		���ե�����̾�����Υꥹ�ȥ����ƥ�Ρ������ƥ��֤ʤΤ򥯥�å���
 *		  ���줬�ǥ��쥯�ȥ�Ǥʤ��ä���硣
 *		������ȥ�ˤ����Ϥ��ƥ꥿���󥭡��򲡲�����
 *		  ���줬�ǥ��쥯�ȥ�Ǥʤ��ä���硣
 *
 *	�Ĥޤꡢok_button �� "clicked" �����ʥ�����ꤷ�Ƥ����С�
 *	����Ū�����ˤ��ե���������(����)���ΤǤ��롣
 *
 *	---------------------------------------------------------------------
 *	q8tk_widget_destroy() �κݤϡ����٤ƤΥ������åȤ��˲����롣
 *	(������� LIST ITEM �������åȤ�ޤ�)
 *
 *---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------
 *	�ʲ��δؿ��ΰ��� fselect �� q8tk_file_selection_new() ������ͤΡ�
 *	������ɥ��������åȤǤ��롣�ե����륻�쥯����󥦥����åȤ��ѿ�
 *	(fselect->stat.fselect.�ѿ�̾) �˥�����������ˤϡ��ʲ��Τ褦�ˤ��롣
 *	Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->�ѿ�̾
 *---------------------------------------------------------------------------*/

/* �ե����륻�쥯�����Υꥹ�ȥܥå�����Υꥹ�ȥ����ƥब����å����줿���

   ��ľ�������򤷤Ƥ����ꥹ�ȥܥå����Ȱ㤦�Τ�����å����줿���
	fsel_list_selection_changed() ("selection_changed"���٥�Ȥˤ��)
	fsel_item_selected()          ("select"           ���٥�Ȥˤ��)
     �ν�˸ƤӽФ���롣

   ��ľ�������򤷤Ƥ����ꥹ�ȥܥå�����Ʊ���Τ�����å����줿���
	fsel_item_selected()          ("select"           ���٥�Ȥˤ��)
     �������ƤӽФ���롣 ("selection_changed" ���٥�Ȥ�ȯ�����ʤ��Τ�)     */


static void fsel_update(Q8tkWidget *fselect, const char *filename, int type);
static int fsel_opendir(Q8tkWidget *fselect, const char *filename, int type);


/* �ե�����̾���� LIST BOX �ˤơ�������Υե����뤬�Ѥ�ä����Υ�����Хå� */
static void fsel_selection_changed_callback(UNUSED_WIDGET, void *fselect)
{
    Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->selection_changed = TRUE;
}


/* �ե�����̾�� LIST ITEM �ˤơ� ����å��������Υ�����Хå� */
static void fsel_selected_callback(Q8tkWidget *item, void *fselect)
{
    const char *name;
    if      (item->name)                       name = item->name;
    else if (item->child && item->child->name) name = item->child->name;
    else                                       name = "";


    if (Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->selection_changed) {

	/* ľ���� fsel_selection_changed_callback() ���ƤӽФ���Ƥ����硢
	   �Ĥޤ�����Ȥϰ㤦�٥ꥹ�ȥ����ƥब����å����줿���ϡ�
	   ���Υꥹ�ȥ����ƥ���ݻ�����ե�����̾�򡢥���ȥ�˥��åȤ��� */

	Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->selection_changed = FALSE;

	q8tk_entry_set_text(Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)
						      ->selection_entry, name);

    } else {

	/* ���� fsel_selected_callback() �������ƤӽФ��줿��硢
	   �Ĥޤ������Ʊ���٥ꥹ�ȥ����ƥब����å����줿���ϡ�
	   ���Υꥹ�ȥ����ƥ���ݻ�����ե�����̾�ǡ��ե������������ */

	fsel_update((Q8tkWidget*)fselect, name, item->stat.any.data[0]);
    }
}


/* �ե�����̾���Ϥ� ENTRY �ˤơ��꥿���󥭡��������Υ�����Хå� */
static void fsel_activate_callback(Q8tkWidget *entry, void *fselect)
{
    const char *input_filename = q8tk_entry_get_text(entry);
    int   type = -1;	/* �ǥ��쥯�ȥ꤫�ɤ��������� */

    /* ���Ϥ����ե�����̾�����ꥹ�Ȥ� name �˰��פ���С�
       �ե����륿���פ���ꤹ�뤳�Ȥ��Ǥ��롣
       (�����Ƴ��ꤵ����ɬ�פϤʤ��Τǡ�Ŭ���˰��ץ����å�����) */

    Q8tkWidget *match_list;
    match_list = q8tk_listbox_search_items(
			 Q8TK_FILE_SELECTION((Q8tkWidget*)fselect)->file_list,
			 input_filename);

    if (match_list) type = match_list->stat.any.data[0];


    /* ���Ϥ����ե�����̾�ǡ��ե������������ */
    fsel_update((Q8tkWidget*)fselect, input_filename, type);
}


/* �ե�������� LIST BOX �򹹿����롣 �ǥ��쥯�ȥ�̾�ʤ�ɽ��������
   �ե�����̾�����ꤷ�����ϡ�OK �ܥ���򲡤������Ȥˤ��� */
static void fsel_update(Q8tkWidget *fselect, const char *filename, int type)
{
    char wk[ Q8TK_MAX_FILENAME ];

    if (osd_path_join(Q8TK_FILE_SELECTION(fselect)->pathname, filename,
		      wk, Q8TK_MAX_FILENAME) == FALSE) {
	strcpy(wk, "");
    }

    if (fsel_opendir(fselect, wk, type) != FILE_STAT_DIR) {
	widget_signal_do(Q8TK_FILE_SELECTION(fselect)->ok_button, "clicked");
    }
}


/* ���ꤵ�줿�ѥ�̾�򡢥ǥ��쥯�ȥ�̾�ȥե�����̾��ʬ���� �ǥ��쥯�ȥ�򳫤���
   �ꥹ�Ȥ˥ե���������򥻥åȡ� �ե�����̾�򥨥�ȥ�˥��åȤ��� */
static int fsel_opendir_sub(Q8tkWidget *fselect, const char *dirname);
static int fsel_opendir(Q8tkWidget *fselect, const char *filename, int type)
{
    int save_code = q8tk_set_kanjicode(osd_kanji_code());

    char path[ Q8TK_MAX_FILENAME ];
    char dir [ Q8TK_MAX_FILENAME ];
    char file[ Q8TK_MAX_FILENAME ];


    if (filename == NULL || filename[0] == '\0') { /* filename��̵���ʾ��� */
	filename = osd_dir_cwd();		   /* �����ȥǥ��쥯�ȥ�� */
	type = FILE_STAT_DIR;			   /* ����˻Ȥ����Ȥˤ��� */
    }

    if (osd_path_normalize(filename, path, Q8TK_MAX_FILENAME)	/* ������ */
							== FALSE) {
	strcpy(path, "");
	type = FILE_STAT_DIR;
    }


		/* filename ��ǥ��쥯�ȥ�̾�ȥե�����̾��ʬΥ���� */

    if (type < 0) {				/* �ǥ��쥯�ȥ꤫�ɤ������� */
	type = osd_file_stat(path);		/* �ʤ顢�ޤ���°�������   */
    }

    if (type == FILE_STAT_DIR) {		/* filename �ϥǥ��쥯�ȥ� */
	strcpy(dir,  path);
	strcpy(file, "");				/* �ե�����̾�϶� */

    } else {					/* filename �ϥե�����餷�� */
						/* �ǥ��쥯�ȥ�̾��ʬΥ����  */
	if (osd_path_split(path, dir, file, Q8TK_MAX_FILENAME) == FALSE) {
	    strcpy(dir,  "");				/* ���Ԥ�����     */
	    strcpy(file, path);				/* Ŭ���ˤ��ޤ��� */
	}
    }

		/* �ǥ��쥯�ȥ�̾�ǥǥ��쥯�ȥ�򳫤� */

    if (fsel_opendir_sub(fselect, dir) >= 0) {		/* �����Τ��������� */

	Q8TK_FILE_SELECTION(fselect)->pathname[0] = '\0';
	strncat(Q8TK_FILE_SELECTION(fselect)->pathname, dir,
		Q8TK_MAX_FILENAME-1);

	q8tk_entry_set_text(Q8TK_FILE_SELECTION(fselect)->selection_entry,
			    file);
    } else {						/* �����ʤ��ä��ġ� */

	q8tk_entry_set_text(Q8TK_FILE_SELECTION(fselect)->selection_entry,
			    path);
    }


    q8tk_set_kanjicode(save_code);

		/* ���ꤵ�줿�ѥ�̾���ǥ��쥯�ȥ���ä����ɤ������֤� */
    return type;
}


static	int	fsel_opendir_sub(Q8tkWidget *fselect, const char *dirname)
{
    T_DIR_INFO	*dirp;
    T_DIR_ENTRY	*dirent;
    char	wk[ Q8TK_MAX_FILENAME ];
    int		nr, i, width;
    char	old_path[ Q8TK_MAX_FILENAME ];
    char	*old_dirname = NULL;

		/* ��¸�� LIST ITEM ��� */

    if (Q8TK_FILE_SELECTION(fselect)->file_list->child) {
	q8tk_listbox_clear_items(Q8TK_FILE_SELECTION(fselect)->file_list,0,-1);
    }
    q8tk_adjustment_set_value(Q8TK_FILE_SELECTION(fselect)->scrolled_window
						      ->stat.scrolled.hadj, 0);
    q8tk_adjustment_set_value(Q8TK_FILE_SELECTION(fselect)->scrolled_window
						      ->stat.scrolled.vadj, 0);


		/* �ǥ��쥯�ȥ��Ĵ�١��ե�����̾�� LIST ITEM �Ȥ�����Ͽ */
		/* ����������ǽ�ʥե�����̾��ʻ���� LIST ITEM ���ݻ����Ƥ��� */
		/* �Ĥ��ǤʤΤǡ��ե�����Υ����פ� LIST ITEM ���ݻ����Ƥ��� */

    if (dirname[0] != '\0' &&
	(dirp = osd_opendir(dirname))) {

	/* ��ѥ�̾���̥ѥ��ȥǥ��쥯�ȥ�̾��ʬΥ���롣
	   ���ξ�̥ѥ�̾�ȡ������Υѥ�̾�����פ���С�
	   ����ʬΥ�����ǥ��쥯�ȥ�̾��Ф��Ƥ��� */
	if (Q8TK_FILE_SELECTION(fselect)->pathname[0] != '\0' &&
	    osd_path_split(Q8TK_FILE_SELECTION(fselect)->pathname,
			   wk,
			   old_path,
			   Q8TK_MAX_FILENAME)) {
	    if (strcmp(wk, dirname) == 0) {
		old_dirname = old_path;
	    }
	}

	nr = 0;
	while ((dirent = osd_readdir(dirp))) {
	    Q8tkWidget *item;

	    wk[0] = '\0';
	    strncat(wk, dirent->str, Q8TK_MAX_FILENAME-1);

	    item = q8tk_list_item_new_with_label(wk);
	    q8tk_list_item_set_string(item, dirent->name);
	    item->stat.any.data[0] = dirent->type;
	    q8tk_container_add(Q8TK_FILE_SELECTION(fselect)->file_list, item);
	    q8tk_widget_show(item);

	    /* �Ф����ǥ��쥯�ȥ�̾�˰��פ��륢���ƥब����С������ƥ��֤� */
	    if (old_dirname && (strcmp(old_dirname, dirent->name) == 0)) {
		q8tk_listbox_select_child(
				Q8TK_FILE_SELECTION(fselect)->file_list, item);
		old_dirname = NULL;
	    }

	    q8tk_signal_connect(item, "select",
				fsel_selected_callback, fselect);
	    nr ++;
	}
	osd_closedir(dirp);


	i = strlen(dirname);
	width = Q8TK_FILE_SELECTION(fselect)->width;
	if (i + 6 > width) {			/* 6 == sizeof("DIR = ") */
	    i = i - (width - 6 -3);		/* 3 == sizeof("...")    */
	    if (q8gr_strchk(Q8TK_FILE_SELECTION(fselect)->dir_name->code, 
			    dirname, i) == 2) { i ++; }	/* ������������򤱤�*/
	} else {
	    i = 0;
	}
	wk[0] = '\0';
	strncat(wk, "DIR = ", Q8TK_MAX_FILENAME-1);
	if (i) {
	    strncat(wk, "...", Q8TK_MAX_FILENAME-1 -strlen(wk));
	}
	strncat(wk, &dirname[i], Q8TK_MAX_FILENAME-1 -strlen(wk));


    } else {

	nr = -1;

	wk[0] = '\0';
	strncat(wk, "DIR = non existant", Q8TK_MAX_FILENAME-1);

    }

    q8tk_label_set(Q8TK_FILE_SELECTION(fselect)->dir_name, wk);

    sprintf(wk, "%4d file(s)", (nr < 0) ? 0 : nr);
    q8tk_label_set(Q8TK_FILE_SELECTION(fselect)->nr_files, wk);


    return nr;
}




static void fsel_selection_view_callback(UNUSED_WIDGET, void *fselect)
{
    q8tk_utility_view(q8tk_file_selection_get_filename(fselect));
}



#if 0	/* ver 0.5.x �ޤǤ�ɽ������ */

Q8tkWidget	*q8tk_file_selection_new(const char *title, int select_ro)
{
    Q8tkWidget *fselect, *window, *vbox, *wk;
    int save_code;

    fselect = malloc_widget();
    fselect->type = Q8TK_TYPE_FILE_SELECTION;
    fselect->sensitive = TRUE;

    fselect->stat.fselect.pathname = (char *)calloc(Q8TK_MAX_FILENAME, 1);
    fselect->stat.fselect.filename = (char *)calloc(Q8TK_MAX_FILENAME, 1);
    Q8tkAssert(fselect->stat.fselect.pathname, "memory exhoused");
    Q8tkAssert(fselect->stat.fselect.filename, "memory exhoused");

    window = q8tk_window_new(Q8TK_WINDOW_DIALOG);	/* ������ɥ����� */
    window->stat.window.work = fselect;

    vbox = q8tk_vbox_new();				/* �ʲ� ��ľ�˵ͤ�� */
    q8tk_container_add(window, vbox);
    q8tk_widget_show(vbox);

    wk = q8tk_label_new(title);				/* ���Ф�(�������) */
    q8tk_box_pack_start(vbox, wk);
    q8tk_widget_show(wk);
    q8tk_misc_set_placement(wk, Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_TOP);
    fselect->stat.fselect.width = MAX(q8gr_strlen(wk->code, wk->name),
				      Q8GR_SCREEN_X * 0.5);
  
    wk = q8tk_hseparator_new();				/* ---------------- */
    q8tk_box_pack_start(vbox, wk);
    q8tk_widget_show(wk);
							/* DIR = /...       */
    fselect->stat.fselect.dir_name = q8tk_label_new("DIR =");
    q8tk_box_pack_start(vbox, fselect->stat.fselect.dir_name);
    q8tk_widget_show(fselect->stat.fselect.dir_name);
    q8tk_misc_set_placement(fselect->stat.fselect.dir_name,
			    Q8TK_PLACEMENT_X_LEFT, Q8TK_PLACEMENT_Y_TOP);

							/*     99 file(s)   */
    fselect->stat.fselect.nr_files = q8tk_label_new("0 file(s)");
    q8tk_box_pack_start(vbox, fselect->stat.fselect.nr_files);
    q8tk_widget_show(fselect->stat.fselect.nr_files);
    q8tk_misc_set_placement(fselect->stat.fselect.nr_files,
			    Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_TOP);

							/* [�ꥹ�ȥܥå���] */
    fselect->stat.fselect.scrolled_window=q8tk_scrolled_window_new(NULL, NULL);
    q8tk_box_pack_start(vbox, fselect->stat.fselect.scrolled_window);
    q8tk_widget_show(fselect->stat.fselect.scrolled_window);

    fselect->stat.fselect.file_list = q8tk_listbox_new();
    q8tk_container_add(fselect->stat.fselect.scrolled_window,
		       fselect->stat.fselect.file_list);
    q8tk_widget_show(fselect->stat.fselect.file_list);
    q8tk_signal_connect(fselect->stat.fselect.file_list, "selection_changed",
			fsel_selection_changed_callback, window);

    q8tk_misc_set_placement(fselect->stat.fselect.scrolled_window,
			    Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_CENTER);
    q8tk_scrolled_window_set_policy(fselect->stat.fselect.scrolled_window,
				    Q8TK_POLICY_AUTOMATIC, Q8TK_POLICY_ALWAYS);
    q8tk_misc_set_size(fselect->stat.fselect.scrolled_window, 40, 14);
    q8tk_misc_set_size(fselect->stat.fselect.file_list, 40-3, 0);
    q8tk_listbox_set_placement(fselect->stat.fselect.file_list, -2, +1);

							/* File name:       */
    wk = q8tk_label_new("File name:");
    q8tk_box_pack_start(vbox, wk);
    q8tk_widget_show(wk);
    q8tk_misc_set_placement(wk, Q8TK_PLACEMENT_X_LEFT, Q8TK_PLACEMENT_Y_TOP);

							/* [����ȥ���]     */
    save_code = q8tk_set_kanjicode(osd_kanji_code());
    fselect->stat.fselect.selection_entry = q8tk_entry_new();
    q8tk_box_pack_start(vbox, fselect->stat.fselect.selection_entry);
    q8tk_widget_show(fselect->stat.fselect.selection_entry);
    q8tk_misc_set_size(fselect->stat.fselect.selection_entry,
		       fselect->stat.fselect.width, 0);
    q8tk_signal_connect(fselect->stat.fselect.selection_entry, "activate",
			fsel_activate_callback, window);
    q8tk_set_kanjicode(save_code);

    wk = q8tk_hbox_new();				/*     [OK][CANCEL] */
    q8tk_box_pack_start(vbox, wk);
    q8tk_widget_show(wk);
    q8tk_misc_set_placement(wk, Q8TK_PLACEMENT_X_RIGHT, Q8TK_PLACEMENT_Y_TOP);

    fselect->stat.fselect.ro_button =
				q8tk_check_button_new_with_label("Read only");
    q8tk_box_pack_start(wk, fselect->stat.fselect.ro_button);
    if (select_ro >= 0) {
	if (select_ro > 0) {
	    q8tk_toggle_button_set_state(fselect->stat.fselect.ro_button,TRUE);
	}
	q8tk_widget_show(fselect->stat.fselect.ro_button);
    }
    q8tk_misc_set_placement(fselect->stat.fselect.ro_button,
			    Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_CENTER);

    fselect->stat.fselect.ok_button = q8tk_button_new_with_label("  OK  ");
    q8tk_box_pack_start(wk, fselect->stat.fselect.ok_button);
    q8tk_widget_show(fselect->stat.fselect.ok_button);

    fselect->stat.fselect.cancel_button = q8tk_button_new_with_label("CANCEL");
    q8tk_box_pack_start(wk, fselect->stat.fselect.cancel_button);
    q8tk_widget_show(fselect->stat.fselect.cancel_button);

/*  q8tk_file_selection_set_filename(window, Q8TK_FILE_SELECTION(fselect)->pathname);*/

    return window;
}

#else	/* ver 0.6.0 �ʹߤ�ɽ������ */

Q8tkWidget	*q8tk_file_selection_new(const char *title, int select_ro)
{
    Q8tkWidget *fselect, *window, *vbox, *hbox, *vv, *wk;
    int i, save_code;

    fselect = malloc_widget();
    fselect->type = Q8TK_TYPE_FILE_SELECTION;
    fselect->sensitive = TRUE;

    fselect->stat.fselect.pathname = (char *)calloc(Q8TK_MAX_FILENAME, 1);
    fselect->stat.fselect.filename = (char *)calloc(Q8TK_MAX_FILENAME, 1);
    Q8tkAssert(fselect->stat.fselect.pathname, "memory exhoused");
    Q8tkAssert(fselect->stat.fselect.filename, "memory exhoused");

    window = q8tk_window_new(Q8TK_WINDOW_DIALOG);	/* ������ɥ����� */
    window->stat.window.work = fselect;

    vbox = q8tk_vbox_new();				/* �ʲ� ��ľ�˵ͤ�� */
    q8tk_container_add(window, vbox);
    q8tk_widget_show(vbox);

    wk = q8tk_label_new(title);				/* ���Ф�(�������) */
    q8tk_box_pack_start(vbox, wk);
    q8tk_widget_show(wk);
    q8tk_misc_set_placement(wk, Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_TOP);
    fselect->stat.fselect.width = MAX(q8gr_strlen(wk->code, wk->name), 60);
  
    wk = q8tk_hseparator_new();				/* ---------------- */
    q8tk_box_pack_start(vbox, wk);
    q8tk_widget_show(wk);
							/* DIR = /...       */
    fselect->stat.fselect.dir_name = q8tk_label_new("DIR =");
    q8tk_box_pack_start(vbox, fselect->stat.fselect.dir_name);
    q8tk_widget_show(fselect->stat.fselect.dir_name);
    q8tk_misc_set_placement(fselect->stat.fselect.dir_name,
			    Q8TK_PLACEMENT_X_LEFT, Q8TK_PLACEMENT_Y_TOP);

    hbox = q8tk_hbox_new();
    q8tk_box_pack_start(vbox, hbox);
    q8tk_widget_show(hbox);
    {
	wk = q8tk_label_new("    ");				/* ������ */
	q8tk_box_pack_start(hbox, wk);
	q8tk_widget_show(wk);

								/* [�ꥹ��] */
	fselect->stat.fselect.scrolled_window
					= q8tk_scrolled_window_new(NULL, NULL);
	q8tk_box_pack_start(hbox, fselect->stat.fselect.scrolled_window);
	q8tk_widget_show(fselect->stat.fselect.scrolled_window);

	fselect->stat.fselect.file_list = q8tk_listbox_new();
	q8tk_container_add(fselect->stat.fselect.scrolled_window,
			   fselect->stat.fselect.file_list);
	q8tk_widget_show(fselect->stat.fselect.file_list);
	q8tk_signal_connect(fselect->stat.fselect.file_list,
			    "selection_changed",
			    fsel_selection_changed_callback, window);

	q8tk_misc_set_placement(fselect->stat.fselect.scrolled_window,
				Q8TK_PLACEMENT_X_CENTER,
				Q8TK_PLACEMENT_Y_CENTER);
	q8tk_scrolled_window_set_policy(fselect->stat.fselect.scrolled_window,
					Q8TK_POLICY_AUTOMATIC,
					Q8TK_POLICY_ALWAYS);
	q8tk_misc_set_size(fselect->stat.fselect.scrolled_window, 40, 18);
	q8tk_misc_set_size(fselect->stat.fselect.file_list, 40-3, 0);
	q8tk_listbox_set_placement(fselect->stat.fselect.file_list, -2, +1);


	wk = q8tk_label_new("  ");				/* ������ */
	q8tk_box_pack_start(hbox, wk);
	q8tk_widget_show(wk);


	vv = q8tk_vbox_new();
	q8tk_box_pack_start(hbox, vv);
	q8tk_widget_show(vv);
	{

	    wk = q8tk_label_new("");				/* ���� */
	    q8tk_box_pack_start(vv, wk);
	    q8tk_widget_show(wk);

								/* n file(s) */
	    fselect->stat.fselect.nr_files = q8tk_label_new("0000 file(s)");
	    q8tk_box_pack_start(vv, fselect->stat.fselect.nr_files);
	    q8tk_widget_show(fselect->stat.fselect.nr_files);
	    q8tk_misc_set_placement(fselect->stat.fselect.nr_files,
				    Q8TK_PLACEMENT_X_CENTER,
				    Q8TK_PLACEMENT_Y_CENTER);
#if 0
	    for (i=0; i<9; i++) {				/* ���� */
		wk = q8tk_label_new("");
		q8tk_box_pack_start(vv, wk);
		q8tk_widget_show(wk);
	    }
#else
	    for (i=0; i<4; i++) {				/* ���� */
		wk = q8tk_label_new("");
		q8tk_box_pack_start(vv, wk);
		q8tk_widget_show(wk);
	    }
								/* [VIEW] */
	    fselect->stat.fselect.view_button =
					q8tk_button_new_with_label(" VIEW ");
	    q8tk_box_pack_start(vv, fselect->stat.fselect.view_button);
	    q8tk_widget_show(fselect->stat.fselect.view_button);
	    q8tk_misc_set_placement(fselect->stat.fselect.view_button,
				    Q8TK_PLACEMENT_X_CENTER,
				    Q8TK_PLACEMENT_Y_CENTER);
	    q8tk_signal_connect(fselect->stat.fselect.view_button,  "clicked",
				fsel_selection_view_callback, window);

	    for (i=0; i<2; i++) {				/* ���� */
		wk = q8tk_label_new("");
		q8tk_box_pack_start(vv, wk);
		q8tk_widget_show(wk);
	    }
#endif

								/* Read only */
	    fselect->stat.fselect.ro_button =
				q8tk_check_button_new_with_label("Read only");
	    q8tk_box_pack_start(vv, fselect->stat.fselect.ro_button);
	    q8tk_misc_set_placement(fselect->stat.fselect.ro_button,
				    Q8TK_PLACEMENT_X_CENTER,
				    Q8TK_PLACEMENT_Y_CENTER);
	    if (select_ro >= 0) {
		if (select_ro > 0) {
		    q8tk_toggle_button_set_state(
					fselect->stat.fselect.ro_button, TRUE);
		}
		q8tk_widget_show(fselect->stat.fselect.ro_button);
	    }

								/* [OK] */
	    fselect->stat.fselect.ok_button = 
					q8tk_button_new_with_label("  OK  ");
	    q8tk_box_pack_start(vv, fselect->stat.fselect.ok_button);
	    q8tk_widget_show(fselect->stat.fselect.ok_button);
	    q8tk_misc_set_placement(fselect->stat.fselect.ok_button,
				    Q8TK_PLACEMENT_X_CENTER,
				    Q8TK_PLACEMENT_Y_CENTER);

								/* [CANCEL] */
	    fselect->stat.fselect.cancel_button =
					q8tk_button_new_with_label("CANCEL");
	    q8tk_box_pack_start(vv, fselect->stat.fselect.cancel_button);
	    q8tk_widget_show(fselect->stat.fselect.cancel_button);
	    q8tk_misc_set_placement(fselect->stat.fselect.cancel_button,
				    Q8TK_PLACEMENT_X_CENTER,
				    Q8TK_PLACEMENT_Y_CENTER);
	}

#if 0
	wk = q8tk_label_new(" ");				/* ������ */
	q8tk_box_pack_start(hbox, wk);
	q8tk_widget_show(wk);
#endif
    }


    wk = q8tk_label_new("");				/* ���� */
    q8tk_box_pack_start(vbox, wk);
    q8tk_widget_show(wk);

    hbox = q8tk_hbox_new();
    q8tk_box_pack_start(vbox, hbox);
    q8tk_widget_show(hbox);
    {
							/* File name:       */
	wk = q8tk_label_new("Filename ");
	q8tk_box_pack_start(hbox, wk);
	q8tk_widget_show(wk);

							/* [����ȥ���]     */
	save_code = q8tk_set_kanjicode(osd_kanji_code());
	fselect->stat.fselect.selection_entry = q8tk_entry_new();
	q8tk_box_pack_start(hbox, fselect->stat.fselect.selection_entry);
	q8tk_widget_show(fselect->stat.fselect.selection_entry);
	q8tk_misc_set_size(fselect->stat.fselect.selection_entry, 50, 0);
	q8tk_signal_connect(fselect->stat.fselect.selection_entry, "activate",
			    fsel_activate_callback, window);
	q8tk_set_kanjicode(save_code);
    }

/*  q8tk_file_selection_set_filename(window, Q8TK_FILE_SELECTION(fselect)->pathname);*/

    return window;
}

#endif

const	char	*q8tk_file_selection_get_filename(Q8tkWidget *fselect)
{
    if (osd_path_join(Q8TK_FILE_SELECTION(fselect)->pathname,
		      q8tk_entry_get_text(Q8TK_FILE_SELECTION(fselect)
							->selection_entry),
		      Q8TK_FILE_SELECTION(fselect)->filename,
		      Q8TK_MAX_FILENAME)
	== FALSE) {
	return "";
    }

    return Q8TK_FILE_SELECTION(fselect)->filename;
}

void		q8tk_file_selection_set_filename(Q8tkWidget *fselect,
						 const char *filename)
{
    fsel_opendir(fselect, filename, -1);
}

int		q8tk_file_selection_get_readonly(Q8tkWidget *fselect)
{
    return Q8TK_TOGGLE_BUTTON(Q8TK_FILE_SELECTION(fselect)->ro_button)->active;
}




/*---------------------------------------------------------------------------
 * FILE VIEWER
 *---------------------------------------------------------------------------
 *  �����������Ǻ�ä����ե���������äȤ����ߤ�桼�ƥ���ƥ�
 *  ���ƥ����ȥե������Ƚ�Ǥϡ���ĥ�ҤΤ�
 *    SJIS �� EUC �� UTF8 ����ۣ���Ƚ��
 *  �����Τ����ˤ����Ȥ�����Τ���Ȥ��褦�ġġ�
 *    �Ȥ����������褬�褯�ʤ뵤�ۤ��ʤ���оä����ġġ�
 *  �����ʤߤ˥������åȤǤϤʤ��������ʤ�ե�����ɽ�����롣
 *---------------------------------------------------------------------------*/
#define	VIEW_MAX_LINE	(1000)				/* 1000�Ԥ���ɽ�� */
#define	VIEW_MAX_COLUMN	(80)				/* 80��ʹߤ���ɽ�� */
static	Q8tkWidget	*view_widget_win;
static	Q8tkWidget	*view_widget_vbox;
static	Q8tkWidget	*view_widget_swin;
static	Q8tkWidget	*view_widget_board;
static	Q8tkWidget	*view_widget_quit;
static	Q8tkWidget	*view_widget_accel;
static	Q8tkWidget	*view_string[ VIEW_MAX_LINE ];
static	int		view_string_cnt;

static	void	q8tk_utility_view_end(UNUSED_WIDGET, UNUSED_PARM);
int		q8tk_utility_view(const char *filename)
{
    int save_code, file_code, file_text;
    OSD_FILE *fp;
    Q8tkWidget *w, *swin, *x, *b, *z;

    if (filename == NULL || *filename == '\0' ||
	osd_file_stat(filename) != FILE_STAT_FILE) {
	return 0;
    }

    fp = osd_fopen(FTYPE_READ, filename, "rb");

    if (fp == NULL) {
	return 0;
    }

    {
	int l = strlen(filename);
	if (l >= 4 && ((filename[l-4] == '.') &&
		       (filename[l-3] == 'T' || filename[l-3] == 't') &&
		       (filename[l-2] == 'X' || filename[l-2] == 'x') &&
		       (filename[l-1] == 'T' || filename[l-1] == 't'))) {
	    file_text = TRUE;
	} else {
	    file_text = FALSE;
	}
    }

    if (file_text == FALSE) {		/* �Х��ʥ�ե����� */

	file_code = Q8TK_KANJI_ANK;

    } else {				/* �ƥ����ȥե����� */
					/* �ե�������Ƭ 1KB ��Ŭ���˸���  */
	int  sz;			/* ���������ɤ�Ƚ�ꡣ�ۤ�Ȥ�Ŭ�� */
	char buf[1024];

	sz = (int)osd_fread(buf, sizeof(char), sizeof(buf), fp);
	if (osd_fseek(fp, 0, SEEK_SET) == -1) {
	    osd_fclose(fp);
	    return 0;
	}
	file_code = q8gr_strcode(buf, sz);
    }



    {						/* �ᥤ��Ȥʤ륦����ɥ� */
	w = q8tk_window_new(Q8TK_WINDOW_DIALOG);
	view_widget_accel = q8tk_accel_group_new();
	q8tk_accel_group_attach(view_widget_accel, w);
    }
    {						/* ����˥ܥå�����褻�� */
	x = q8tk_vbox_new();
	q8tk_container_add(w, x);
	q8tk_widget_show(x);
							/* �ܥå����ˤ�     */
	{						/* SCRL������ɥ��� */
	    swin  = q8tk_scrolled_window_new(NULL, NULL);
	    q8tk_widget_show(swin);
	    q8tk_scrolled_window_set_policy(swin, Q8TK_POLICY_AUTOMATIC,
						  Q8TK_POLICY_AUTOMATIC);
	    q8tk_misc_set_size(swin, 78, 20);
	    q8tk_box_pack_start(x, swin);
	}
	{						/* ��λ�ܥ�������� */
	    b = q8tk_button_new_with_label(" O K ");
	    q8tk_box_pack_start(x, b);
	    q8tk_widget_show(b);

	    q8tk_misc_set_placement(b, Q8TK_PLACEMENT_X_CENTER,
				       Q8TK_PLACEMENT_Y_CENTER);
	    q8tk_signal_connect(b, "clicked", q8tk_utility_view_end, NULL);

	    q8tk_accel_group_add(view_widget_accel, Q8TK_KEY_ESC,
				 b, "clicked");
	}
    }

    {							/* SCRL������ɥ��� */
	z = q8tk_vbox_new();				/* VBOX���ä�     */
	q8tk_container_add(swin, z);
	q8tk_widget_show(z);

	save_code = q8tk_set_kanjicode(file_code);	/* �ե��������Ƥ�   */
    							/* ��٥�Ȥ������� */
	{
	    int c1, c2;
	    int column, line;
	    int end_of_line, end_of_file;
	    char src[ VIEW_MAX_COLUMN + 1 ];

	    c2 = osd_fgetc(fp);

	    line = 0;
	    end_of_file = FALSE;
	    memset(src, 0, sizeof(src));

	    while (end_of_file == FALSE) {

		if (file_text) {		/* �ƥ����ȥե����� */
		    column = 0;
		    end_of_line = FALSE;
		    memset(src, 0, sizeof(src));

		    while (end_of_line == FALSE) {

			c1 = c2;
			c2 = osd_fgetc(fp);
			if (c1 == '\r' &&  c2 == '\n') c2 = osd_fgetc(fp);

			if (c1 == EOF) {
			    c1 = '\0';
			    end_of_line = TRUE;
			    end_of_file = TRUE;
			}
			else if ((c1 == '\r') || (c1 == '\n')) {
			    c1 = '\0';
			    end_of_line = TRUE;
			}

			
			if (c1 == '\t') {			/* �����ִ� */
			    int len = q8gr_strlen(file_code, src);
			    int pad = ((len + 8) / 8) * 8 - len;
			    int i;
			    for (i=0; i<pad; i++) {
				if (column < (int)sizeof(src)-1) {
				    src[ column ] = ' ';
				    column ++;
				}
			    }
			}
			else if (column < (int)sizeof(src)-1) {	/* ���ְʳ� */
			    src[ column ] = c1;
			    column ++;
			}
		    }

		} else {			/* �Х��ʥ�ե����� */
		    int c[16];
		    char fmt[128];

		    if (c2 == EOF) {
			strcpy(src, "[EOF]");
			end_of_file = TRUE;
		    } else {
			for (column = 0; column < 16 && c2 != EOF; column ++) {
			    c[column] = c2;
			    c2 = osd_fgetc(fp);
			}

			strcpy(fmt, "%04X : ");
			while (column--) { strcat(fmt, "%02X "); }

			sprintf(src, fmt, line * 0x10,
				c[ 0], c[ 1], c[ 2], c[ 3],
				c[ 4], c[ 5], c[ 6], c[ 7],
				c[ 8], c[ 9], c[10], c[11],
				c[12], c[13], c[14], c[15]);
		    }
		}


		if (line == COUNTOF(view_string)-1)
		    strcpy(src, "........To be continued.");

		view_string[ line ] = q8tk_label_new(src);
		q8tk_widget_show(view_string[ line ]);
		q8tk_box_pack_start(z, view_string[ line ]);

		line ++;
		if (line >= COUNTOF(view_string)) break;
	    }

	    view_string_cnt = line;
	}
	q8tk_set_kanjicode(save_code);
    }

    osd_fclose(fp);


    q8tk_widget_show(w);
    q8tk_grab_add(w);

    q8tk_widget_set_focus(b);


    view_widget_win   = w;	/* �����������Ĥ����Ȥ��������� */
    view_widget_vbox  = x;	/* �������åȤ�Ф��Ƥ����ޤ�     */
    view_widget_swin  = swin;
    view_widget_board = z;
    view_widget_quit  = b;

    return 1;
}

static	void	q8tk_utility_view_end(UNUSED_WIDGET, UNUSED_PARM)
{
    int i;
    for (i=0; i<view_string_cnt; i++)
	q8tk_widget_destroy(view_string[ i ]);

    q8tk_widget_destroy(view_widget_quit);
    q8tk_widget_destroy(view_widget_board);
    q8tk_widget_destroy(view_widget_swin);
    q8tk_widget_destroy(view_widget_vbox);

    q8tk_grab_remove(view_widget_win);
    q8tk_widget_destroy(view_widget_win);
    q8tk_widget_destroy(view_widget_accel);
}




/************************************************************************/
/* ��¿�ʽ���								*/
/************************************************************************/

/*---------------------------------------------------------------------------
 * ɽ�����֤��ѹ�
 *---------------------------------------------------------------------------
 *  ��HBOX��VBOXľ���λҤΥ������åȤΤ�ͭ����¾��̵�� (̵��)
 *  -------------------------------------------------------------------------
 *  void	q8tk_misc_set_placement(Q8tkWidget *widget,
 *					int placement_x, int placement_y)
 *---------------------------------------------------------------------------*/
void	q8tk_misc_set_placement(Q8tkWidget *widget,
				int placement_x, int placement_y)
{
    widget->placement_x = placement_x;
    widget->placement_y = placement_y;
}

/*---------------------------------------------------------------------------
 * ɽ�����������ѹ�
 *---------------------------------------------------------------------------
 *  ���ʲ��Υ������åȤΤ�ͭ����¾��̵�� (̵��)
 *	COMBO		�� ʸ������ʬ��ɽ����
 *	LIST		�� ʸ�����ɽ����
 *	SCROLLED WINDOW	�� ������ɥ��������⤵
 *	ENTRY		�� ʸ�����ɽ����
 *  -------------------------------------------------------------------------
 *  void	q8tk_misc_set_size(Q8tkWidget *widget, int width, int height)
 *---------------------------------------------------------------------------*/
void	q8tk_misc_set_size(Q8tkWidget *widget, int width, int height)
{
    switch (widget->type) {

    case Q8TK_TYPE_COMBO:
	if (width > 0) {
	    widget->stat.combo.width = width;
	    q8tk_misc_set_size(widget->stat.combo.entry,
			       widget->stat.combo.width, 0);
	} else {
	    widget->stat.combo.width = 0;
	    q8tk_misc_set_size(widget->stat.combo.entry,
			       widget->stat.combo.length, 0);
	}
	break;

    case Q8TK_TYPE_LISTBOX:
	if (width > 0) widget->stat.listbox.width = width;
	else           widget->stat.listbox.width = 0;
	break;

    case Q8TK_TYPE_SCROLLED_WINDOW:
	widget->stat.scrolled.width  = width;
	widget->stat.scrolled.height = height;

	if (widget->with_label) {
	    q8tk_adjustment_set_increment(widget->stat.scrolled.hadj,
					  1, MAX(10, width - 3));
	    q8tk_adjustment_set_increment(widget->stat.scrolled.vadj,
					  1, MAX(10, height - 3));
	}
	break;

    case Q8TK_TYPE_ENTRY:
	widget->stat.entry.width = width;
	q8tk_entry_set_position(widget, widget->stat.entry.cursor_pos);
	break;

    default:
	fprintf(stderr, "Cant resize widget=%s\n", debug_type(widget->type));
	Q8tkAssert(FALSE, NULL);
	return;
    }
    set_construct_flag(TRUE);
}




/************************************************************************/
/* ����ƥʴط�								*/
/************************************************************************/
/*---------------------------------------------------------------------------
 * ����ƥ����
 *---------------------------------------------------------------------------
 *  ������ƥʥ������åȤȥ������åȤ�ƻҤˤ���
 *  -------------------------------------------------------------------------
 *  void    q8tk_container_add(Q8tkWidget *container, Q8tkWidget *widget)
 *	����ƥʥ������å� container �� �������å� widget ��ƻҤˤ��롣
 *
 *  void    q8tk_box_pack_start(Q8tkWidget *box, Q8tkWidget *widget)
 *	��ʿ����ľ�ܥå��� box �θ����ˡ��������å� widget ��ͤ�롣
 *
 *  void    q8tk_box_pack_end(Q8tkWidget *box, Q8tkWidget *widget)
 *	��ʿ����ľ�ܥå��� box �������ˡ��������å� widget ��ͤ�롣
 *
 *  void    q8tk_container_remove(Q8tkWidget *container, Q8tkWidget *widget)
 *	����ƥʥ������å� container ���� �������å� widget ���ڤ�Υ����
 *---------------------------------------------------------------------------*/
void	q8tk_container_add(Q8tkWidget *container, Q8tkWidget *widget)
{
    Q8tkAssert(container->attr & Q8TK_ATTR_CONTAINER, NULL);
    if (container->attr & Q8TK_ATTR_LABEL_CONTAINER) {
	Q8tkAssert(widget->type == Q8TK_TYPE_LABEL, NULL);
    }

    switch (container->type) {
    case Q8TK_TYPE_LISTBOX:			/* LIST BOX �㳰���� */
	if (container->child == NULL) {			/* �ǽ�� LIST ITEM */
	    container->stat.listbox.selected = widget;	/* �ξ��ϡ������ */
	    container->stat.listbox.active   = widget;	/* ������֤ˤ���   */
	}
	q8tk_box_pack_start(container, widget);		/* ������VBOX��Ʊ�� */
	return;

    default:					/* �̾�ν��� */
	container->child = widget;
	widget->parent = container;
	widget->prev   = NULL;
	widget->next   = NULL;
	break;
    }

    if (widget->visible) {
	widget_map(widget);
    }
}

void	q8tk_box_pack_start(Q8tkWidget *box, Q8tkWidget *widget)
{
    Q8tkAssert(box->attr & Q8TK_ATTR_CONTAINER, NULL);

    if (box->child == NULL) {
	box->child   = widget;
	widget->prev = NULL;
	widget->next = NULL;
    } else {
	Q8tkWidget *c = box->child;
	while (c->next) {
	    c = c->next;
	}
	c->next      = widget;
	widget->prev = c;
	widget->next = NULL;
    }
    widget->parent = box;

    if (widget->visible) {
	widget_map(widget);
    }
}

void	q8tk_box_pack_end(Q8tkWidget *box, Q8tkWidget *widget)
{
    Q8tkAssert(box->attr & Q8TK_ATTR_CONTAINER, NULL);

    if (box->child == NULL) {
	widget->prev = NULL;
	widget->next = NULL;
    } else {
	Q8tkWidget *c = box->child;
	Q8tkAssert(c->prev == NULL, NULL);
	c->prev      = widget;
	widget->next = c;
	widget->prev = NULL;
    }
    box->child     = widget;
    widget->parent = box;

    if (widget->visible) {
	widget_map(widget);
    }
}

void	q8tk_container_remove(Q8tkWidget *container, Q8tkWidget *widget)
{
    Q8tkAssert(container->attr & Q8TK_ATTR_CONTAINER, NULL);
    Q8tkAssert(widget->parent == container, NULL);

    if (widget->prev == NULL) {			/* ��ʬ���Ƥ�ľ���λ� */
	Q8tkWidget *n = widget->next;
	if (n) n->prev = NULL;
	container->child = n;

    } else {					/* ��������ʤ��� */
	Q8tkWidget *p = widget->prev;
	Q8tkWidget *n = widget->next;
	if (n) n->prev = p;
	p->next = n;
    }

    widget->parent = NULL;
    widget->prev = NULL;
    widget->next = NULL;

    switch (container->type) {

    case Q8TK_TYPE_LISTBOX:			/* LIST BOX �㳰���� */
	if (container->stat.listbox.selected == widget) {
	    container->stat.listbox.selected = NULL;
	    container->stat.listbox.active   = NULL;
	}
	break;

    }


    if (container->visible) {
	widget_map(container);
    }
}


/************************************************************************/
/* ɽ���ط�								*/
/************************************************************************/
/*---------------------------------------------------------------------------
 * �Ļ벽���ԲĻ벽
 *---------------------------------------------------------------------------
 *  ���������åȤ�ɽ������ɽ��
 *  -------------------------------------------------------------------------
 *  void	q8tk_widget_show(Q8tkWidget *widget)
 *	�������åȤ�ɽ�����롣
 *
 *  void	q8tk_widget_hide(Q8tkWidget *widget)
 *	�������åȤ���ɽ���ˤ��롣�Ҥ�������ɽ���ˤʤ롣
 *---------------------------------------------------------------------------*/
void	q8tk_widget_show(Q8tkWidget *widget)
{
    widget->visible = TRUE;
    widget_map(widget);
}
void	q8tk_widget_hide(Q8tkWidget *widget)
{
    widget->visible = FALSE;
    widget_map(widget);
}


/*---------------------------------------------------------------------------
 * ������
 *---------------------------------------------------------------------------
 *  ���������åȤ�������
 *  -------------------------------------------------------------------------
 *  void	q8tk_widget_set_sensitive(Q8tkWidget *widget, int sensitive)
 *	�������åȤ�����ǽ�������ǽ�ˤ��롣
 *	---------------------------------------------------------------------
 *	�����ݤϡ��Ҥ�����롣
 *	�����ǽ�ʥ������åȤϡ��̾�Ȥϰ㤦��(��Ϥο�)��ɽ���Ȥʤ롣
 *	��Ȥ�����Ǥ��ʤ��������å�(��٥�ʤ�)���Ф��Ƥ⡢����Ǥ��롣
 *	---------------------------------------------------------------------
 *	LOGO / VSEPARATOR / HSEPARATOR �ϡ������ǽ�ˤ��Ƥ⿧���Ѳ����ʤ���
 *	(�Ѳ��������ۤ�����������?)
 *
 *	NOTEPAGE �ϡ������ǥ���ƥ�(NOTEPAGE)��ư����������ȤߤʤΤǡ�
 *	�����ݤ�����Ǥ��ʤ��� (̵����ꤵ��������С�
 *	q8tk_widget_set_sensitive(widget->parent, FALSE) �ǤǤ��뤬��)
 *
 *	ADJUSTMENT �������ݤ�����Ǥ��뤬����������ʤɤΤۤ������ꤹ��
 *	�������顢ľ�ܻ��ꤹ�뤳�ȤϤʤ�������
 *---------------------------------------------------------------------------*/
void	q8tk_widget_set_sensitive(Q8tkWidget *widget, int sensitive)
{
    widget->sensitive = sensitive;
    widget_map(widget);
}


/************************************************************************/
/* �������åȤξõ�							*/
/************************************************************************/
/*---------------------------------------------------------------------------
 * �������åȾõ�
 *---------------------------------------------------------------------------
 *  ���������åȤ�õ��
 *	��������	     ��q8tk_dialog_new �ˤ����������������å����ƾõ
 *				vbox �� action_area �˺ܤ����������åȤϾõ�
 *				���ʤ��Τơ��ƤӽФ����Ǿõ�롣
 *
 *	�ե����륻�쥯����� �� �����������������ƤΥ������åȤ�õ
 *				�ե�����������������������������åȤ����ƾõ�
 *
 *	����¾		     �� �����Ǽ�ư���������������åȤ⤢�碌�ƾõ
 *				q8tk_xxx_new_with_label ������������٥�䡢
 *				q8tk_scrolled_window_new �Ǽ�ư��������
 *				�����㥹�ȥ��Ȥʤ�
 *  -------------------------------------------------------------------------
 *  void	q8tk_widget_destroy(Q8tkWidget *widget)
 *	�������åȤξõ�ʹߡ������� widget �ϻ��Ѷػ�
 *
 *---------------------------------------------------------------------------*/
static	void	widget_destroy_all(Q8tkWidget *widget);

void	q8tk_widget_destroy(Q8tkWidget *widget)
{
    Q8tkWidget *work;

    if (widget->type == Q8TK_TYPE_WINDOW &&	/* DIALOG �� FILE SELECTION */
	(work = widget->stat.window.work)) {	/* ���㳰����		    */

	switch (work->type) {
	case Q8TK_TYPE_DIALOG:
	    Q8tkAssert(widget->child->type == Q8TK_TYPE_FRAME, NULL);
	    Q8tkAssert(widget->child->child->type == Q8TK_TYPE_VBOX, NULL);
	    q8tk_widget_destroy(Q8TK_DIALOG(widget)->action_area);
	    q8tk_widget_destroy(Q8TK_DIALOG(widget)->vbox);
	    q8tk_widget_destroy(widget->child->child);
	    q8tk_widget_destroy(widget->child);
	    q8tk_widget_destroy(work);
	    break;

	case Q8TK_TYPE_FILE_SELECTION:
	    {
		q8tk_widget_destroy(
			(Q8TK_FILE_SELECTION(widget)->scrolled_window)
							->stat.scrolled.hadj);
		q8tk_widget_destroy(
			(Q8TK_FILE_SELECTION(widget)->scrolled_window)
							->stat.scrolled.vadj);

		free(Q8TK_FILE_SELECTION(widget)->pathname);
		free(Q8TK_FILE_SELECTION(widget)->filename);

		/* �ط����Ϥä��ꤷ�Ƥ���Τǡ�����̵�Ѥ����Ƥ�õ� */
		widget_destroy_all(widget->child);
	    }
	    q8tk_widget_destroy(work);
	    break;
	}
    }

    if (widget->type == Q8TK_TYPE_COMBO) {	/* COMBO ���㳰���� */
	Q8List *l = widget->stat.combo.list;
	while (l) {
	    q8tk_widget_destroy((Q8tkWidget *)(widget->stat.combo.list->data));
	    l = l->next;
	}
	q8_list_free(widget->stat.combo.list);
	q8tk_widget_destroy(widget->stat.combo.entry);
    }

    if (widget->type == Q8TK_TYPE_ACCEL_GROUP &&    /* ACCEL �λҤ����ƾõ� */
	widget->child) {
	/* �ط����Ϥä��ꤷ�Ƥ���Τǡ�����̵�Ѥ����Ƥ�õ� */
	widget_destroy_all(widget->child);
    }


    if (widget->with_label &&			/* XXX_new_with_label()�� */
	widget->child      &&			/* �������줿 LABEL�ν��� */
	widget->child->type == Q8TK_TYPE_LABEL) {
	q8tk_widget_destroy(widget->child);
    }
    if (widget->with_label &&			    /* ���� NULL ���������줿*/
	widget->type == Q8TK_TYPE_SCROLLED_WINDOW) {/* SCROLLED WINDOW �ν���*/
	q8tk_widget_destroy(widget->stat.scrolled.hadj);
	q8tk_widget_destroy(widget->stat.scrolled.vadj);
    }

    if (widget->name) {
	free(widget->name);
	widget->name = NULL;
    }
    free_widget(widget);
}


/* ���Ȥȡ��Ҥȡ���֤����ƺ����
   ���Ȱʲ��Υ������åȤκ���ˤ��㳰������̵�����Ȥ��狼�äƤ�����Τ߲� */

static	void	widget_destroy_all(Q8tkWidget *widget)
{
    if (widget->next)  widget_destroy_all(widget->next);
    if (widget->child) widget_destroy_all(widget->child);

    if (widget->name) {
	free(widget->name);
	widget->name = NULL;
    }
    free_widget(widget);
}




/************************************************************************/
/* ����Υ������åȤ˥ե������������Ƥ롣				*/
/*	�̾� �ե��������ϡ�ľ���˥ܥ���䥭�����Ϥ��ʤ��줿�������å�	*/
/*	�ˤʤꡢTAB �ˤ�����ؤ���ǽ���������δؿ�������Υ������å�	*/
/*	�˥ե������������ꤹ�뤳�Ȥ��Ǥ��롣				*/
/*	�����������Υ������åȤΰ������Ĥ� WINDOW ����q8tk_grab_add()��	*/
/*	������ʤ��줿���ȤǤʤ����̵���Ǥ��롣			*/
/************************************************************************/
void	q8tk_widget_set_focus(Q8tkWidget *widget)
{
    focus_widget[ window_layer_level ] = widget;
    set_construct_flag(TRUE);
}


/************************************************************************/
/* �����ʥ�ط�								*/
/************************************************************************/
/*
 *	Ǥ�դΥ������åȤˡ�Ǥ�դΥ����ʥ������
 */
static	void	widget_signal_do(Q8tkWidget *widget, const char *name)
{
    switch (widget->type) {
    case Q8TK_TYPE_WINDOW:			/* ������ɥ�		*/
	if (strcmp(name, "inactivate") == 0) {
	    if (widget->user_event_0) {
		(*widget->user_event_0)(widget, widget->user_event_0_parm);
	    }
	    return;
	}
	break;
    case Q8TK_TYPE_BUTTON:			/* �ܥ���		*/
	if (strcmp(name, "clicked") == 0) {
	    if (widget->user_event_0) {
		(*widget->user_event_0)(widget, widget->user_event_0_parm);
	    }
	    return;
	}
	break;
    case Q8TK_TYPE_TOGGLE_BUTTON:		/* �ȥ���ܥ���		*/
    case Q8TK_TYPE_CHECK_BUTTON:		/* �����å��ܥ���	*/
    case Q8TK_TYPE_RADIO_BUTTON:		/* �饸���ܥ���		*/
	if        (strcmp(name, "clicked") == 0) {
	    if (widget->user_event_0) {
		(*widget->user_event_0)(widget, widget->user_event_0_parm);
	    }
	    return;
	} else if (strcmp(name, "toggled") == 0) {
	    if (widget->user_event_1) {
		(*widget->user_event_1)(widget, widget->user_event_1_parm);
	    }
	    return;
	}
	break;

    case Q8TK_TYPE_NOTEBOOK:			/* �Ρ��ȥ֥å�		*/
	if (strcmp(name, "switch_page") == 0) {
	    if (widget->user_event_0) {
		(*widget->user_event_0)(widget, widget->user_event_0_parm);
	    }
	    return;
	}
	break;

    case Q8TK_TYPE_COMBO:			/* ����ܥܥå���	*/
	if        (strcmp(name, "activate") == 0) {
	    if (widget->user_event_0) {
		(*widget->user_event_0)(widget, widget->user_event_0_parm);
	    }
	    return;
	} else if (strcmp(name, "changed") == 0) {
	    if (widget->user_event_1) {
		(*widget->user_event_1)(widget, widget->user_event_1_parm);
	    }
	    return;
	}
	break;

    case Q8TK_TYPE_LISTBOX:			/* �ꥹ�ȥܥå���	*/
	if (strcmp(name, "selection_changed") == 0) {
	    if (widget->user_event_0) {
		(*widget->user_event_0)(widget, widget->user_event_0_parm);
	    }
	    return;
	}
	break;

    case Q8TK_TYPE_LIST_ITEM:			/* �ꥹ�ȥ����ƥ�	*/
	if (strcmp(name, "select") == 0) {
	    if (widget->user_event_0) {
		(*widget->user_event_0)(widget, widget->user_event_0_parm);
	    }
	    return;
	}
	break;

    case Q8TK_TYPE_ADJUSTMENT:			/* �����㥹�ȥ��� */
	if (strcmp(name, "value_changed") == 0) {
	    if (widget->user_event_0) {
		(*widget->user_event_0)(widget, widget->user_event_0_parm);
	    }
	    return;
	}
	break;

    case Q8TK_TYPE_ENTRY:			/* ����ȥ꡼		*/
	if        (strcmp(name, "activate") == 0) {
	    if (widget->user_event_0) {
		(*widget->user_event_0)(widget, widget->user_event_0_parm);
	    }
	    return;
	} else if (strcmp(name, "changed") == 0) {
	    if (widget->user_event_1) {
		(*widget->user_event_1)(widget, widget->user_event_1_parm);
	    }
	    return;
	}
	break;

    }

    fprintf(stderr, "BAD signal %s '%s'\n", debug_type(widget->type), name);
    Q8tkAssert(FALSE, NULL);
    return;
}


/*---------------------------------------------------------------------------
 * �����ʥ�����ꡦ���
 *---------------------------------------------------------------------------
 *  ���������åȤΥ����ʥ�ˡ�������Хå��ؿ������ꤹ�롣
 *    �ƥ������åȤ��Ȥ�ȯ�����륷���ʥ뤬��ޤäƤ���Τǡ����Υ����ʥ뤬
 *    ȯ���������˸ƤӽФ����ؿ������ꤹ�롣
 *  �����������ꤹ��ؿ������Ǽ�ʬ���ȤΥ������åȤ�������褦�ʤ��Ȥ�
 *    ���ƤϤ����ʤ��������Ǹ�ư����ǽ�������롣
 *     (���������ܥ��󥦥����åȤ˸¤ꡢ��ǽ�Ȥ��롣�ʤΤǡ�����������
 *	�ե����륻�쥯�����Υܥ���ˤơ����Ȥ�������Τ�����פʤϤ�)
 *
 *  -------------------------------------------------------------------------
 * int	q8tk_signal_connect(Q8tkWidget *widget, const char *name,
 *			    Q8tkSignalFunc func, void *func_data)
 *	widget �� name �Ǽ�����륷���ʥ���Ф��륳����Хå��ؿ� func ��
 *	���ꤹ�롣�����ʥ�ȯ�����ˤϡ��ؿ� func �� ���� widget, func_data ��
 *	ȼ�äƸƤӽФ���롣
 *	�ʤ�������ͤϾ�� 0 �ˤʤ롣
 *
 * void	q8tk_signal_handlers_destroy(Q8tkWidget *widget)
 *	widget �����ꤵ�줿���ƤΥ����ʥ�Υ�����Хå��ؿ���̵���ˤ��롣
 *
 *---------------------------------------------------------------------------*/
void	q8tk_signal_handlers_destroy(Q8tkWidget *widget)
{
    widget->user_event_0 = NULL;
    widget->user_event_1 = NULL;
}

int	q8tk_signal_connect(Q8tkWidget *widget, const char *name,
			    Q8tkSignalFunc func, void *func_data)
{
    switch (widget->type) {
    case Q8TK_TYPE_WINDOW:			/* ������ɥ�		*/
	if (strcmp(name, "inactivate") == 0) {
	    widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
	    widget->user_event_0_parm = func_data;
	    return 0;
	}
	break;
    case Q8TK_TYPE_BUTTON:			/* �ܥ���		*/
	if (strcmp(name, "clicked") == 0) {
	    widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
	    widget->user_event_0_parm = func_data;
	    return 0;
	}
	break;
    case Q8TK_TYPE_TOGGLE_BUTTON:		/* �ȥ���ܥ���		*/
    case Q8TK_TYPE_CHECK_BUTTON:		/* �����å��ܥ���	*/
    case Q8TK_TYPE_RADIO_BUTTON:		/* �饸���ܥ���		*/
	if        (strcmp(name, "clicked") == 0) {
	    widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
	    widget->user_event_0_parm = func_data;
	    return 0;
	} else if (strcmp(name, "toggled") == 0) {
	    widget->user_event_1      = (void (*)(Q8tkWidget*,void*))func;
	    widget->user_event_1_parm = func_data;
	    return 0;
	}
	break;

    case Q8TK_TYPE_NOTEBOOK:			/* �Ρ��ȥ֥å�		*/
	if (strcmp(name, "switch_page") == 0) {
	    widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
	    widget->user_event_0_parm = func_data;
	    return 0;
	}
	break;

    case Q8TK_TYPE_COMBO:			/* ����ܥܥå���	*/
	if        (strcmp(name, "activate") == 0) {
	    widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
	    widget->user_event_0_parm = func_data;
	    return 0;
	} else if (strcmp(name, "changed") == 0) {
	    widget->user_event_1      = (void (*)(Q8tkWidget*,void*))func;
	    widget->user_event_1_parm = func_data;
	    return 0;
	}
	break;

    case Q8TK_TYPE_LISTBOX:			/* �ꥹ�ȥܥå���	*/
	if (strcmp(name, "selection_changed") == 0) {
	    widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
	    widget->user_event_0_parm = func_data;
	    return 0;
	}
	break;

    case Q8TK_TYPE_LIST_ITEM:			/* �ꥹ�ȥ����ƥ�	*/
	if (strcmp(name, "select") == 0) {
	    widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
	    widget->user_event_0_parm = func_data;
	    return 0;
	}
	break;

    case Q8TK_TYPE_ADJUSTMENT:			/* �����㥹�ȥ���	*/
	if (strcmp(name, "value_changed") == 0) {
	    widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
	    widget->user_event_0_parm = func_data;
	    return 0;
	}
	break;

    case Q8TK_TYPE_ENTRY:			/* ����ȥ�		*/
	if        (strcmp(name, "activate") == 0) {
	    widget->user_event_0      = (void (*)(Q8tkWidget*,void*))func;
	    widget->user_event_0_parm = func_data;
	    return 0;
	} else if (strcmp(name, "changed") == 0) {
	    widget->user_event_1      = (void (*)(Q8tkWidget*,void*))func;
	    widget->user_event_1_parm = func_data;
	    return 0;
	}
	break;

    }

    fprintf(stderr,
	    "Undefined signal %s '%s'\n", debug_type(widget->type),name);
    Q8tkAssert(FALSE, NULL);
    return 0;
}



/************************************************************************/
/* ���٥�ȤΥ����å�							*/
/************************************************************************/
/*
 *	DRAGGING
 */
static	void	widget_dragging(Q8tkWidget *focus)
{
    if (focus && focus->event_dragging) {
	(*focus->event_dragging)(focus);
    }
}
/*
 *	DRAG OFF
 */
static	void	widget_drag_off(Q8tkWidget *focus)
{
    if (focus && focus->event_drag_off) {
	(*focus->event_drag_off)(focus);
    }
}

/*
 *	MOUSE BUTTON ON
 */
static	void	widget_button_on(Q8tkWidget *focus)
{
    if (focus == NULL) {
	/* �������åȰʳ��򥯥�å������顢���οƥ�����ɥ��˥��٥������ */
	focus = window_layer[ window_layer_level ];
    }
    if (focus && focus->event_button_on) {
	(*focus->event_button_on)(focus);
    }
}
/*
 *	KEY ON
 */
static	void	widget_key_on(Q8tkWidget *focus, int key)
{
    if (focus && focus->event_key_on) {
	(*focus->event_key_on)(focus, key);
    }
}



/************************************************************************/
/* �ᥤ��								*/
/************************************************************************/
/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
void	q8tk_event_key_on(int code)
{
    Q8tkWidget *w;
    int change = FALSE;
    int back   = FALSE;

    now_mouse_on = FALSE;

    if (code == Q8TK_KEY_SHIFT) now_shift_on = TRUE;

    if (! get_drag_widget()) {			/* �ɥ�å��椸��ʤ�	*/

	w = get_focus_widget();

	switch (code) {
	case Q8TK_KEY_TAB:
	    change = TRUE;
	    if (now_shift_on) back = TRUE;
	    else              back = FALSE;
	    break;

#ifdef	OLD_FOCUS_CHANGE
	/* DO NOTHING */
#else	/* �����ʥե���������ư���� */
	case Q8TK_KEY_UP:
	    if (w->key_up_used == FALSE) {
		change = TRUE;
		back   = TRUE;
	    }
	    break;
	case Q8TK_KEY_DOWN:
	    if (w->key_down_used == FALSE) {
		change = TRUE;
		back   = FALSE;
	    }
	    break;
	case Q8TK_KEY_LEFT:
	    if (w->key_left_used == FALSE) {
		change = TRUE;
		back   = TRUE;
	    }
	    break;
	case Q8TK_KEY_RIGHT:
	    if (w->key_right_used == FALSE) {
		change = TRUE;
		back   = FALSE;
	    }
	    break;
#endif
	}
	    
	/* [TAB][��][��][��][��] �ǡ�����Υ������åȤ˥ե���������ܤ� */
	if (change) {

	    w = widget_focus_list_get_next(w, back);
	    q8tk_widget_set_focus(w);

	    /* �ե����������ư�����������åȤ���
	       ��������ɥ�����ɥ���Ǥ��ġ��������륢�����ΰ�ʤ顢
	       ����Ū�˥������륤�󤵤������Τǡ�������������ͽ�� */
	    if (w && w->type != Q8TK_TYPE_ADJUSTMENT
		  && w->type != Q8TK_TYPE_LISTBOX
		  && w->type != Q8TK_TYPE_DIALOG
		  && w->type != Q8TK_TYPE_FILE_SELECTION) {
		widget_scrollin_register(w);
	    }
	} else {				/*	¾�Υ��� */

	    /* ��������졼���������������ꤵ��Ƥ���н������� */
	    if (window_layer_level >= 0) {
		w = window_layer[ window_layer_level ];
		Q8tkAssert(w->type == Q8TK_TYPE_WINDOW, NULL);

		if (w->stat.window.accel) {
		    w = (w->stat.window.accel)->child;
		    while (w) {
			if (w->stat.accel.key == code) {
			    widget_signal_do(w->stat.accel.widget, w->name);
			    break;
			}
			w = w->next;
		    }
		}
	    }

	    /* ��������졼����������ͭ���ʤ顢ɸ��ν����Ϥ��ʤ��ۤ�������? */
	    widget_key_on(get_focus_widget(), code);
	}
    }
}

void	q8tk_event_key_off(int code)
{
    now_mouse_on = FALSE;

    if (code == Q8TK_KEY_SHIFT) now_shift_on = FALSE;
}

void	q8tk_event_mouse_on(int code)
{
    if (get_construct_flag()) return;	/* ������ɥ��˴�ľ��Υ���å����� */

    if (code == Q8TK_BUTTON_L) {		/* ���ܥ��� ON		*/
	if (! get_drag_widget()) {		/* �ɥ�å��椸��ʤ�	*/
	    void *focus = q8gr_get_focus_screen(mouse.x/8, mouse.y/16);
	    if        (focus == Q8GR_WIDGET_NONE) {
		widget_button_on(NULL);
	    } else if (focus == Q8GR_WIDGET_WINDOW) {
		/* DO NOTHING */ ;
	    } else {
		q8tk_widget_set_focus((Q8tkWidget *)focus);
		widget_button_on((Q8tkWidget *)focus);
	    }
	}
    } else if (code == Q8TK_BUTTON_U) {		/* �ۥ����� UP		*/

	widget_key_on(get_focus_widget(), Q8TK_KEY_PAGE_UP);

    } else if (code == Q8TK_BUTTON_D) {		/* �ۥ����� DOWN	*/

	widget_key_on(get_focus_widget(), Q8TK_KEY_PAGE_DOWN);
    }
}

void	q8tk_event_mouse_off(int code)
{
    now_mouse_on = FALSE;

    if (code == Q8TK_BUTTON_L) {		/* ���ܥ��� OFF		*/
	if (get_drag_widget()) {		/* ���� �ɥ�å���	*/
	    widget_drag_off(get_drag_widget());
	    set_drag_widget(NULL);
	}
    }
}

void	q8tk_event_mouse_moved(int x, int y)
{
    int block_moved;

    mouse.x_old = mouse.x;
    mouse.y_old = mouse.y;
    mouse.x     = x;
    mouse.y     = y;

    if (mouse.x/8  != mouse.x_old/8  ||		/* �ޥ��� 8dot�ʾ� ��ư	*/
	mouse.y/16 != mouse.y_old/16) {
	block_moved = TRUE;
	now_mouse_on = FALSE;

	if (disp_cursor) {
	    set_construct_flag(TRUE);
	}
    } else {
	block_moved = FALSE;
    }

    if (get_drag_widget()) {			/* ���� �ɥ�å��� ����	*/
	if (block_moved) {			/* �ޥ��� 8dot�ʾ� ��ư	*/
	    widget_dragging(get_drag_widget());
	}
    }
}

void	q8tk_event_quit(void)
{
    q8tk_main_quit();
}

/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
static	int	cursor_exist = 0;
static	int	cursor_timer = 0;

#define	Q8GR_CURSOR_BLINK		(30)
#define	Q8GR_CURSOR_CYCLE		(60)

#define	Q8GR_MOUSE_AUTO_REPEAT		(6)


void	q8gr_set_cursor_exist(int exist_flag)
{
    cursor_exist = exist_flag;
}
int	q8gr_get_cursor_exist(void)
{
    return cursor_exist;
}
int	q8gr_get_cursor_blink(void)
{
    return (cursor_timer < Q8GR_CURSOR_BLINK) ? TRUE : FALSE;
}
void	q8gr_set_cursor_blink(void)
{
    cursor_timer = 0;
}

/*--------------------------------------------------------------
 * q8tk_main_loop
 *	����ͤ������ʤ顢�������ȸ�˲��٤Ǥ�ƤӽФ�
 *		  ���ʤ顢��λ
 *--------------------------------------------------------------*/

int	q8tk_main_loop(void)
{
    static int mouse_frame = 0;

    if (get_main_loop_flag()) {

	if (get_delay_widget() == NULL) {

	    if (cursor_exist) {		/* ��������ɽ����ʤ����ǥ����޹��� */
		cursor_timer ++;
		if (cursor_timer > Q8GR_CURSOR_CYCLE) {
		    cursor_timer = 0;
		}
	    } else {
		cursor_timer = 0;
	    }

	    event_update();		/* ���٥�Ȥ��������		  */
					/* (ɬ�פ˱����ơ������ǲ��̹���) */
					/* (��λ���Υե饰�������ǥ��å�) */

	    if (now_mouse_on) {		/* �ޥ����ܥ��󥪡��ȥ�ԡ��Ƚ��� */
		++ mouse_frame;
		if (mouse_frame >= Q8GR_MOUSE_AUTO_REPEAT) {
		    q8tk_event_mouse_on(Q8TK_BUTTON_L);
		    mouse_frame = 0;
		}
	    } else {
		mouse_frame = 0;
	    }

	    if (cursor_exist) {		  /* �������������ڤ��ؤ���	*/
		if (cursor_timer == 0 ||  /* �����ߥ󥰤�����å�	*/
		    cursor_timer == Q8GR_CURSOR_BLINK) {
		    set_construct_flag(TRUE);
		}
	    }

	} else {
      
	    /* ���٥�Ȥ��ٱ䤵���뤿��Ρ������ޡ����� (���ʤ궯��) */
	    /* (���ΤȤ����ܥ���ȥꥹ�ȥ����ƥ�Τ�)              */
	    if (-- delay_timer < 0) {
		Q8tkWidget *widget = get_delay_widget();

		if (widget->type == Q8TK_TYPE_BUTTON) {
		    widget->stat.button.active = Q8TK_BUTTON_OFF;
		    widget_signal_do(widget, "clicked");
		}
		else if (widget->type == Q8TK_TYPE_LIST_ITEM) {
		    widget_signal_do(widget->parent, "selection_changed");
		    widget_signal_do(widget, "select");
		}
		set_construct_flag(TRUE);

		set_delay_widget(NULL, 0);
	    }
	}



	if (get_construct_flag()) {	/* ���̹����ѹ����ϡ����� */

	    widget_construct();
	    set_construct_flag(FALSE);

	    if (q8tk_disp_init_flag) {
		screen_set_dirty_palette();
		q8tk_disp_init_flag = FALSE;
	    }
	    screen_set_dirty_flag(0);

#ifdef	RESET_EVENT			/* ������Ϥ򥭥�󥻥� */
	    event_switch();
#endif
	}

    }

    /* ���̤����������äơ�Q8TK ����λ���Ƥ��ʤ���С������֤�	*/
    /*		���¦�ϡ����衦�������ȸ�ˤ��δؿ�����ٸƤӽФ�����	*/
    /*									*/
    /* ���̤����������äơ�Q8TK ����λ���Ƥ���С������֤�		*/
    /*		���¦�ϡ����ν��������ܤ��뤳��			*/
    /*		���衦�������ȸ�����ܤ��Ƥ⹽��ʤ�			*/
    /*		�ְ�äƺ��ٸƤӽФ�����硢�ʤˤ���������˵����֤�	*/

    if (get_main_loop_flag()) {

	return TRUE;

    } else {

	q8tk_term();
	return FALSE;
    }
}

/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/
void	q8tk_main_quit(void)
{
    set_main_loop_flag(FALSE);
}







/************************************************************************/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/*									*/
/************************************************************************/

/*------------------------------------------------------*/
/* �������åȤ�ɽ������
	�ƥ������åȤ�Ĥ��Ĥ������å�����
	�����Ƥοƥ������åȤ��Ļ����
	�����ĥ������åȤ� WINDOW
	�ξ�硢���� WINDOW �ʲ���Ʒ׻�����ɽ������
	�褦�ˡ��ե饰�򤿤Ƥ롣
	�ºݤκƷ׻���ɽ���ϡ�q8tk_main() �ǹԤʤ���	*/
/*------------------------------------------------------*/
static	void	widget_map(Q8tkWidget *widget)
{
    Q8tkWidget *ancestor, *parent;
    int        size_calc = TRUE;

    if (widget->visible) {
	ancestor = widget;
	parent   = widget->parent;
	while (parent) {
	    if (parent->visible) {
		ancestor = parent;
		parent   = parent->parent;
	    } else {
		size_calc = FALSE;
		break;
	    }
	}
	if (size_calc && ancestor->type == Q8TK_TYPE_WINDOW) {
	    set_construct_flag(TRUE);
	}
    }
}


/*------------------------------------------------------*/
/* �ҥ������åȤΥ������򸵤ˡ���ʬ���ȤΥ�������׻����롣
   �Ƶ�Ū�ˡ����Ƥλҥ��������åȤ�Ʊ���˷׻����롣

   *widget �� ���ֿƤΥ������å�
   *max_sx �� ���Ƥλҥ������åȤΤʤ��Ǻ��祵���� x
   *max_sy �� ���Ƥλҥ������åȤΤʤ��Ǻ��祵���� y
   *sum_sx �� �Ҥ���֥������åȤΥ��������� x
   *sum_sy �� �Ҥ���֥������åȤΥ��������� y          */
/*------------------------------------------------------*/
static	void	widget_resize(Q8tkWidget *widget, int max_sx, int max_sy);
static	void	widget_size(Q8tkWidget *widget, int *max_sx, int *max_sy,
						int *sum_sx, int *sum_sy)
{
    int n_msx, n_msy, n_ssx, n_ssy;

/*  printf("%d \n", widget->type); fflush(stdout);*/


		/* ��ʬ���Ȥ���� (next) ��¸�ߤ���С��Ƶ�Ū�˷׻� */
  
    if (widget->next) {
	widget_size(widget->next, &n_msx, &n_msy, &n_ssx, &n_ssy);
    } else {
	n_msx = n_msy = n_ssx = n_ssy = 0;
    }


    if (widget->visible) {

	int c_msx, c_msy, c_ssx, c_ssy;

		/* �ҥ������åȤΥ������׻�(�Ƶ�) */

	if (widget->child) {
	    widget_size(widget->child, &c_msx, &c_msy, &c_ssx, &c_ssy);
	} else {
	    c_msx = c_msy = c_ssx = c_ssy = 0;
	}

		/* �ҥ������åȤ򸵤ˡ����ȤΥ������׻� */

	switch (widget->type) {

	case Q8TK_TYPE_WINDOW:			/* ������ɥ�		*/
	    if (widget->stat.window.no_frame) {
		widget->sx = c_msx;
		widget->sy = c_msy;
	    } else {
		widget->sx = c_msx +2;
		widget->sy = c_msy +2;
	    }
	    break;

	case Q8TK_TYPE_BUTTON:			/* �ܥ���		*/
	case Q8TK_TYPE_TOGGLE_BUTTON:		/* �ȥ���ܥ���		*/
	    widget->sx = c_msx +2;
	    widget->sy = c_msy +2;
	    break;

	case Q8TK_TYPE_CHECK_BUTTON:		/* �����å��ܥ���	*/
	case Q8TK_TYPE_RADIO_BUTTON:		/* �饸���ܥ���		*/
	    widget->sx = c_msx +3;
	    widget->sy = Q8TKMAX(c_msy, 1);
	    break;

	case Q8TK_TYPE_FRAME:			/* �ե졼��		*/
	    if (widget->name) {
		widget->sx = Q8TKMAX(c_msx,
				     q8gr_strlen(widget->code, widget->name))
									   + 2;
	    } else {
		widget->sx = c_msx + 2;
	    }
	    widget->sy = c_msy +2;
	    break;

	case Q8TK_TYPE_LABEL:			/* ��٥�		*/
	    if (widget->name) {
		widget->sx = q8gr_strlen(widget->code, widget->name);
	    } else {
		widget->sx = 0;
	    }
	    widget->sy = 1;
	    break;

	case Q8TK_TYPE_LOGO:			/* ��			*/
	    widget->sx = Q8GR_LOGO_W;
	    widget->sy = Q8GR_LOGO_H;
	    break;

	case Q8TK_TYPE_NOTEBOOK:		/* �Ρ��ȥ֥å�		*/
	    {
		int	len = 0;
		Q8tkWidget *n = widget->child;
		while (n) {
		    if (n ->name) {
			len += q8gr_strlen(n->code, n->name);
		    }
		    len += 1;
		    n = n->next;
		}
		len += 1;
		widget->sx = Q8TKMAX(c_msx +2, len);
	    }
	    widget->sy = c_msy + 4;
	    break;

	case Q8TK_TYPE_NOTEPAGE:		/* �Ρ��ȥ֥å��Υڡ���	*/
	    widget->sx = c_msx;
	    widget->sy = c_msy;
	    break;

	case Q8TK_TYPE_VBOX:			/* ��ľ�ܥå���		*/
	    widget->sx = c_msx;
	    widget->sy = c_ssy;
	    break;
	case Q8TK_TYPE_HBOX:			/* ��ʿ�ܥå���		*/
	    widget->sx = c_ssx;
	    widget->sy = c_msy;
	    break;

	case Q8TK_TYPE_VSEPARATOR:		/* ��ľ���ڤ���		*/
	    widget->sx = 1;
	    widget->sy = 1;
	    break;
	case Q8TK_TYPE_HSEPARATOR:		/* ��ʿ���ڤ���		*/
	    widget->sx = 1;
	    widget->sy = 1;
	    break;

	case Q8TK_TYPE_COMBO:			/* ����ܥܥå���	*/
	    if (widget->stat.combo.width) {
		widget->sx = widget->stat.combo.width;
	    } else {
		if (widget->stat.combo.length) {
		    widget->sx = widget->stat.combo.length;
		} else {
		    widget->sx = 8;
		}
	    }
	    widget->sx += 3;
	    widget->sy = 1;
	    break;

	case Q8TK_TYPE_LISTBOX:			/* �ꥹ�ȥܥå���	*/
	    widget->sx = Q8TKMAX(c_msx, widget->stat.listbox.width);
	    widget->sy = c_ssy;
	    break;

	case Q8TK_TYPE_LIST_ITEM:		/* �ꥹ�ȥ����ƥ�	*/
	    widget->sx = c_msx;
	    widget->sy = c_msy;
	    break;

	case Q8TK_TYPE_ADJUSTMENT:		/* �����㥹�ȥ���	*/
	    Q8tkAssert(FALSE, NULL);
	    break;

	case Q8TK_TYPE_HSCALE:			/* ��ʿ��������		*/
	case Q8TK_TYPE_VSCALE:			/* ��ľ��������		*/
	    if (widget->stat.scale.adj) {
		int sx, sy;
		adjustment_size(&widget->stat.scale.adj->stat.adj, &sx, &sy);

		if (widget->stat.scale.draw_value) {
		    if (widget->stat.scale.value_pos == Q8TK_POS_LEFT ||
			widget->stat.scale.value_pos == Q8TK_POS_RIGHT) {
			widget->sx = sx + 4;
			widget->sy = Q8TKMAX(sy, 1);
		    } else {		/* Q8TK_POS_UP||Q8TK_POS_BOTTOM*/
			widget->sx = Q8TKMAX(sx, 3);
			widget->sy = sy + 1;
		    }
		} else {
		    widget->sx = sx;
		    widget->sy = sy;
		}
	    } else {
		widget->sx = 0;
		widget->sy = 0;
	    }
	    break;

	case Q8TK_TYPE_SCROLLED_WINDOW:		/* ��������ɥ�����ɥ� */
	    if (widget->child) {

		/* �Ĳ���������С���ɽ�����뤫�ɤ����ϡ�
		   child �Υ�������scrolled �Υ�������scrolled �� policy��
		   ���Ȥ߹�碌�ˤ�ꡢ�ʲ��� 9 �ѥ������ʬ����롣

				   0)              1)              2)
				c_msx < w-2	c_msx = w-2	c_msx > w-2
				(NEVER)				(ALLWAYS)
		0)
		  c_msy < h-2				  
		  (NEVER)					  ---
		1)
		  c_msy = h-2					    |
						  --+
		2)
		  c_msy > h-2	    |		    |		    |
		  (ALLWAYS)			  --+		  --+

		*/

		int w, h, tmp;

		switch (widget->stat.scrolled.hpolicy) {
		case Q8TK_POLICY_NEVER:		w = 0;		break;
		case Q8TK_POLICY_ALWAYS:	w = 2;		break;
		default: /* AUTOMATIC */
		    if      (c_msx <  widget->stat.scrolled.width - 2) w = 0;
		    else if (c_msx == widget->stat.scrolled.width - 2) w = 1;
		    else                                               w = 2;
		}

		switch (widget->stat.scrolled.vpolicy) {
		case Q8TK_POLICY_NEVER:		h = 0;		break;
		case Q8TK_POLICY_ALWAYS:	h = 2;		break;
		default: /* AUTOMATIC */
		    if      (c_msy <  widget->stat.scrolled.height - 2) h = 0;
		    else if (c_msy == widget->stat.scrolled.height - 2) h = 1;
		    else                                                h = 2;
		}

		if        ((w == 0 || w == 1) && (h == 0 || h == 1)) {
		    widget->stat.scrolled.hscrollbar = FALSE;
		    widget->stat.scrolled.vscrollbar = FALSE;
		} else if ((w == 2) && (h == 0)) {
		    widget->stat.scrolled.hscrollbar = TRUE;
		    widget->stat.scrolled.vscrollbar = FALSE;
		} else if ((w == 0) && (h == 2)) {
		    widget->stat.scrolled.hscrollbar = FALSE;
		    widget->stat.scrolled.vscrollbar = TRUE;
		} else {
		    widget->stat.scrolled.hscrollbar = TRUE;
		    widget->stat.scrolled.vscrollbar = TRUE;
		}

		w = (widget->stat.scrolled.vscrollbar)
					? widget->stat.scrolled.width - 3
					: widget->stat.scrolled.width - 2;
		h = (widget->stat.scrolled.hscrollbar)
					? widget->stat.scrolled.height - 3
					: widget->stat.scrolled.height - 2;

		q8tk_adjustment_set_length(widget->stat.scrolled.hadj, w);
		q8tk_adjustment_clamp_page(widget->stat.scrolled.hadj,
					   0, c_msx - w);
		adjustment_size(&widget->stat.scrolled.hadj->stat.adj,
				&tmp, &tmp);

		q8tk_adjustment_set_length(widget->stat.scrolled.vadj, h);
		q8tk_adjustment_clamp_page(widget->stat.scrolled.vadj,
					   0, c_msy - h);
		adjustment_size(&widget->stat.scrolled.vadj->stat.adj,
				&tmp, &tmp);

		if (widget->child->type == Q8TK_TYPE_LISTBOX &&
		    (widget->stat.scrolled.vadj->stat.adj.listbox_changed) &&
		    (widget->stat.scrolled.vadj_value
			!= widget->stat.scrolled.vadj->stat.adj.value)) {
		    list_event_window_scrolled(widget, h);
		}
	    }
	    widget->stat.scrolled.vadj_value
			=  widget->stat.scrolled.vadj->stat.adj.value;
	    widget->stat.scrolled.vadj->stat.adj.listbox_changed = FALSE;
	    widget->sx = widget->stat.scrolled.width;
	    widget->sy = widget->stat.scrolled.height;
	    break;

	case Q8TK_TYPE_ENTRY:			/* ����ȥ�		*/
	    widget->sx = widget->stat.entry.width;
	    widget->sy = 1;
	    break;


	case Q8TK_TYPE_DIALOG:			/* ��������		*/
	    Q8tkAssert(FALSE, NULL);
	    break;
	case Q8TK_TYPE_FILE_SELECTION:		/* �ե����륻�쥯�����	*/
	    Q8tkAssert(FALSE, NULL);
	    break;

	default:
	    Q8tkAssert(FALSE, "Undefined type");
	}

    } else {
	widget->sx = 0;
	widget->sy = 0;
    }


		/* ���������󹹿� */

    *max_sx = Q8TKMAX(widget->sx, n_msx);
    *max_sy = Q8TKMAX(widget->sy, n_msy);
    *sum_sx = widget->sx + n_ssx;
    *sum_sy = widget->sy + n_ssy;


		/* �ҥ������åȤ˥��ѥ졼�����ޤޤ����ϡ�������Ĵ�� */

    widget_resize(widget, widget->sx, widget->sy);


		/* �ꥹ�ȥܥå����ʤɤξ�硢�ҥ������åȤΥ�������Ĵ�� */


    if (widget->type == Q8TK_TYPE_LISTBOX) {
	Q8tkWidget *child = widget->child;
	while (child) {
	    Q8tkAssert(child->type == Q8TK_TYPE_LIST_ITEM, NULL);
	    child->sx = widget->sx;
	    if (child->next) child = child->next;
	    else             break;
	}
    }


/*printf("%s (%02d,%02d) max{ %02d,%02d } sum{ %02d,%02d }\n",debug_type(widget->type),widget->sx,widget->sy,*max_sx,*max_sy,*sum_sx,*sum_sy);fflush(stdout);*/
}


/*
 * ���ѥ졼���ʤɡ��Ƥ��礭���˰�¸���륦�����åȤΥ�������Ʒ׻�����
 */
static	void	widget_resize(Q8tkWidget *widget, int max_sx, int max_sy)
{
    if (widget->type == Q8TK_TYPE_WINDOW   ||
	widget->type == Q8TK_TYPE_NOTEPAGE ||
	widget->type == Q8TK_TYPE_VBOX     ||
	widget->type == Q8TK_TYPE_HBOX) {

	Q8tkWidget *child = widget->child;
    
	if (widget->type == Q8TK_TYPE_WINDOW &&
	    ! widget->stat.window.no_frame) {
	    max_sx -= 2;
	    max_sy -= 2;
	}
	if (child) widget_resize(child, max_sx, max_sy);

	while (child) {
	    switch (child->type) {

	    case Q8TK_TYPE_HSEPARATOR:
		if (widget->type != Q8TK_TYPE_HBOX) {
		    if (child->sx < max_sx) child->sx = max_sx;
		}
		break;

	    case Q8TK_TYPE_VSEPARATOR:
		if (widget->type != Q8TK_TYPE_VBOX) {
		    if (child->sy < max_sy) child->sy = max_sy;
		}
		break;

	    case Q8TK_TYPE_VBOX:
		if (widget->type == Q8TK_TYPE_VBOX) {
		    if (child->sx < max_sx) child->sx = max_sx;
		}
		break;

	    case Q8TK_TYPE_HBOX:
		if (widget->type == Q8TK_TYPE_HBOX) {
		    if (child->sy < max_sy) child->sy = max_sy;
		}
		break;
	    }

	    if (child->next) child = child->next;
	    else             break;
	}
    }
}



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*------------------------------------------------------
   �������åȤ�����������ɥ�����ɥ��˴ޤޤ���硢
   ����������֤ˤ�äƤϡ�ɽ������ʤ����Ȥ����롣

   ͽ�ᡢ���Υ������åȤ�ʲ��δؿ�����Ͽ���Ƥ����С�

	widget_scrollin_register(this_widget);

   ����ɽ���κݤˡ����Υ������åȤ�ɽ�������褦�ˡ���������
   ���֤�Ĵ������褦�ˤ��롣

   ��������ɥ�����ɥ��ˤ��륦�����åȤ�ɽ�����줿���ɤ����ϡ�
   �ºݤ�ɽ������ޤǤ狼��ʤ��Τǡ��ºݤ�ɽ�����ʤ�������
   ���ꤷ����������Ƽ���ǡ���ɽ���Ȥ������Ȥˤʤ롣

   �Ĥޤꡢ�ʲ��Τ褦�ʴ����ˤʤ롣

	widget_scrollin_adjust_reset();   �� �����ν����
	widget_draw();                    �� ����

	if (widget_scrollin_adjust()) {   �� Ĵ�������鿿
	    widget_draw();                �� Ĵ�������Τǡ�������
	}

  �ʤ���������Ԥʤ��Τϡ��ǽ�˸��Ĥ��ä� �Ƥ� SCROLLED WINDOW �Τ�
  ���Ф��Ƥ����ʤΤǡ�SCROLLED WINDOW ������Ҥξ��Ϥɤ��ʤ뤫
  �狼��ʤ���
  ------------------------------------------------------*/

/* Ĵ����ǽ�ʥ������åȤο�������������������פ��� */
#define	MAX_WIDGET_SCROLLIN	(8)
static	struct {
    int		drawn;
    Q8tkWidget	*widget;
} widget_scrollin[ MAX_WIDGET_SCROLLIN ];

/* ������ν���� (���٤����Ԥ�) */
static	void	widget_scrollin_init(void)
{
    int i;
    for (i=0; i<MAX_WIDGET_SCROLLIN; i++) {
	widget_scrollin[i].drawn  = FALSE;
	widget_scrollin[i].widget = NULL;
    }
}
/* �������åȤ���Ͽ */
static	void	widget_scrollin_register(Q8tkWidget *w)
{
    int i;
    for (i=0; i<MAX_WIDGET_SCROLLIN; i++) {
	if (widget_scrollin[i].widget == w) return;
    }
    for (i=0; i<MAX_WIDGET_SCROLLIN; i++) {
	if (widget_scrollin[i].widget == NULL) {
	    widget_scrollin[i].drawn  = FALSE;
	    widget_scrollin[i].widget = w;
	    return;
	}
    }
}
/* ��������������åȤ��������˸ƤӽФ� */
static	void	widget_scrollin_adjust_reset(void)
{
    int	i;
    for (i=0; i<MAX_WIDGET_SCROLLIN; i++) {
	widget_scrollin[i].drawn = FALSE;
    }
}
/* ���ꡧ�ƥ������åȤ�������˸ƤӽФ� */
static	void	widget_scrollin_drawn(Q8tkWidget *w)
{
    int i;
    for (i=0; i<MAX_WIDGET_SCROLLIN; i++) {
	if (widget_scrollin[i].widget == w) {
	    widget_scrollin[i].drawn = TRUE;
	    return;
	}
    }
}
/* Ĵ�������������åȤ������˸ƤӽФ� */
static	int	widget_scrollin_adjust(void)
{
    int i, result = 0;

    for (i=0; i<MAX_WIDGET_SCROLLIN; i++) {

/*printf("%d %d %s\n",i,widget_scrollin[i].drawn,(widget_scrollin[i].widget)?debug_type(widget_scrollin[i].widget->type):"NULL");*/

	if (widget_scrollin[i].widget) {

	    if (widget_scrollin[i].drawn) {

		Q8tkWidget *widget = widget_scrollin[i].widget;
		Q8tkWidget *p = widget->parent;
		/*int	x1 = widget->x;*/
		int	x1 = widget->x + widget->sx -1;
		int	y1 = widget->y + widget->sy -1;

		/*
		 SCROLLED WINDOW ��˾褻�륦�����å����Τ򡢺�������Ȥ�����
		 ���а��� real_y

		 y1 = widget->y + widget->sy -1;
		 p->y + 1 - p->stat.scrolled.child_y0 + [ real_y ] = y1;
		*/

		while (p) {
		    if (p->type == Q8TK_TYPE_SCROLLED_WINDOW) {

			int d, topleft;

			/* d > 0 �ʤ顢��üɽ�� */
			/* d < 0 �ʤ顢��üɽ�� */
			if (widget->type == Q8TK_TYPE_LIST_ITEM &&
			    widget->parent->type == Q8TK_TYPE_LISTBOX) {
			    d = widget->parent->stat.listbox.scrollin_left;
			} else {
			    d = 0;
			}
			/* d == 0 �ϡ��ǥե���� (-1 �Ȥ���) */
			if (d == 0) { d = -1; }

			if (d > 0) { topleft = TRUE;    d -= 1; }
			else       { topleft = FALSE;   d += 1; }

			if (topleft) {	   /* ��ü��ɽ�������褦�˰������� */
						        /* d = 0, 1, 2, 3... */
			    /* �������ʤ���к�ü��ɽ�������Τǡ����Τޤ� */

			} else {	   /* ��ü��ɽ�������褦�˰������� */
						     /* d = 0, -1, -2, -3... */
			    if (p->x +1 <= x1 &&
				x1 < p->x +1 + p->stat.scrolled.child_sx) {
				/* Ok, Expose */
			    } else {
				Q8TK_ADJUSTMENT(p->stat.scrolled.hadj)->value
				    = x1 - (p->x+1 - p->stat.scrolled.child_x0)
					 - p->stat.scrolled.child_sx + 1;
				result = 1;
			    }
			}
			/* �� d ���ͤˤ�ä������̤��Ѥ����������ޤ����٤� */


			/* d > 0 �ʤ顢��üɽ����+1�����١�����1ʸ������ɽ�� */
			/* d < 0 �ʤ顢��üɽ����-1�����١�����1ʸ�����ɽ�� */
			if (widget->type == Q8TK_TYPE_LIST_ITEM &&
			    widget->parent->type == Q8TK_TYPE_LISTBOX) {
			    d = widget->parent->stat.listbox.scrollin_top;
			} else {
			    d = 0;
			}
			/* d == 0 �ϡ��ǥե���� (-1 �Ȥ���) */
			if (d == 0) { d = -1; }

			if (d > 0) { topleft = TRUE;    d -= 1; }
			else       { topleft = FALSE;   d += 1; }

			if (topleft) {	   /* ��ü��ɽ�������褦�˰������� */
						        /* d = 0, 1, 2, 3... */
			    if (p->y +1 == (y1-d)) {
				/* Ok */
			    } else {
				Q8TK_ADJUSTMENT(p->stat.scrolled.vadj)->value =
				 (y1-d) - (p->y+1 - p->stat.scrolled.child_y0);
				result = 1;
			    }

			} else {	   /* ��ü��ɽ�������褦�˰������� */
						     /* d = 0, -1, -2, -3... */
			    if (p->y +1 <= (y1-d) &&
				(y1-d) < p->y +1 + p->stat.scrolled.child_sy) {
				/* Ok, Expose */
			    } else {
				Q8TK_ADJUSTMENT(p->stat.scrolled.vadj)->value =
				 (y1-d) - (p->y+1 - p->stat.scrolled.child_y0)
					 - p->stat.scrolled.child_sy + 1;
				result = 1;
			    }
			}

	    /*
	    printf("  %s %d %d %d %d  %d %d %d %d\n",debug_type(p->type),
		   p->x, p->y, p->sx, p->sy,
		   p->stat.scrolled.child_x0, p->stat.scrolled.child_y0,
		   p->stat.scrolled.child_sx, p->stat.scrolled.child_sy);
	    printf("  %s %d %d %d %d\n",debug_type(widget->type),
		   widget->x, widget->y, widget->sx, widget->sy);
	    printf("  %d\n",y1);
	    if (result) printf("ADJ %d,%d\n",
			       Q8TK_ADJUSTMENT(p->stat.scrolled.vadj)->value,
			       Q8TK_ADJUSTMENT(p->stat.scrolled.hadj)->value);
	    */


			break;
		    }
		    p = p->parent;
		}
		widget_scrollin[i].drawn  = FALSE;
		widget_scrollin[i].widget = NULL;
	    }
	}
    }

    return result;
}


/*------------------------------------------------------
   �������åȤΥե���������ư�Υꥹ�Ⱥ���
   
	TAB�����䡢�������륭���ˤ�롢�ե��������ΰ�ư��¸����뤿��ˡ�
	�оݤȤʤ륦�����åȤΥꥹ�Ȥ��������롣

		widget_focus_list_reset()  �� �ꥹ�Ȥν����
		widget_draw()              �� ���衣�����ǥꥹ�Ȥ���������

	widget_draw() �������Ǥϡ��ե���������ư���оݤȤ������������åȤ�
	����κݤˡ���widget_focus_list_append()�� �ؿ���Ƥ֡�

    TAB�����ʤɤ򲡤��줿�Ȥ��ˡ����˥ե��������ΰܤ륦�����åȤ��������ˤ�
    ��widget_focus_list_get_next()�ٴؿ���Ƥ֡�
  ------------------------------------------------------*/
/* BACKTAB �б�  thanks! floi */

static	Q8List		*widget_focus_list;

/* ������ν���� (���٤����Ԥ�) */
static	void	widget_focus_list_init(void)
{
    widget_focus_list = NULL;
}
/* ��������������åȤ��������˸ƤӽФ� */
static	void	widget_focus_list_reset(void)
{
    q8_list_free(widget_focus_list);
    widget_focus_list = NULL;
}
/* ���ꡧ�ե��������ꥹ�Ȥ���Ͽ���륦�����åȤʤ顢�ƤӽФ� */
static	void	widget_focus_list_append(Q8tkWidget *widget)
{
    if (widget) {
	widget_focus_list = q8_list_append(widget_focus_list, widget);
    }
}
/* �������ե��������ꥹ�Ȥ���Ƭ�������åȤ���� */
static	Q8tkWidget *widget_focus_list_get_top(void)
{
    if (widget_focus_list) {
	return (Q8tkWidget *)(widget_focus_list->data);
    } else {
	return NULL;
    }
}
/* �������ե��������ꥹ�Ȥμ�(��)�Υ������åȤ���� */
static	Q8tkWidget *widget_focus_list_get_next(Q8tkWidget *widget, int back)
{
    Q8List *list;

    if (widget == NULL) {
	return widget_focus_list_get_top();
    }

#if	defined(OLD_FOCUS_CHANGE)
    list = q8_list_find(widget_focus_list, widget);

#else	/* �����ʥե���������ư���� */
    switch (widget->type) {
    case Q8TK_TYPE_RADIO_BUTTON:
	{
	    /* ����� RADIO BUTTON ��õ���ơ���������� */
	    Q8List *l = widget->stat.button.list;
	    while (l) {
		widget = (Q8tkWidget *)(l->data);
		if (widget->stat.button.active == Q8TK_BUTTON_ON) break;
		l = l->next;
	    }
	}
	list = q8_list_find(widget_focus_list, widget);
	break;

    case Q8TK_TYPE_NOTEPAGE:
	/* ���ΤȤ����ե��������Τ���NOTEPAGE�Ͼ��������֤�
	   �ʤ�Τǡ����TAB�դ�����롣�Ĥޤꡢ�����ˤϤ��ʤ��� */
	widget = (widget->parent)->stat.notebook.page;
	list = q8_list_find(widget_focus_list, widget);
	break;

    case Q8TK_TYPE_COMBO:
	if (widget->stat.combo.entry->stat.entry.editable == FALSE) {
	    /* ENTRY ���Խ��ԲĤξ��ϡ�COMBO ���Ȥ������ */
	    list = q8_list_find(widget_focus_list, widget);
	} else {
	    /* ENTRY ���Խ��Ĥξ��� */
	    if (back == FALSE) {
		/* COMBO �μ��ϡ�ENTRY �μ���Ʊ�� */
		list = q8_list_find(widget_focus_list,
				    widget->stat.combo.entry);
	    } else {
		/* COMBO �����ϡ�ENTRY ���Τ�Τˤ��Ƥ��ޤ��� */
		return widget->stat.combo.entry;
	    }
	}
	break;

    default:
	list = q8_list_find(widget_focus_list, widget);
	break;
    }
#endif

    if (list) {
	if (back == FALSE) {
	    if (list->next)  return (Q8tkWidget*)((list->next)->data);
	    else             return (Q8tkWidget*)((q8_list_first(list))->data);
	} else {
	    if (list->prev)  return (Q8tkWidget*)((list->prev)->data);
	    else             return (Q8tkWidget*)((q8_list_last(list))->data);
	}
    }
    return NULL;
}




/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*------------------------------------------------------*/
/* ��ʬ���Ȥ�ɽ�����֤��Ȥˡ����褷��
   �Ƶ�Ū�ˡ����Ƥλҥ��������åȤ����褹�롣		*/
/*------------------------------------------------------*/

static	void	widget_draw(Q8tkWidget *widget,
			    int parent_focus, int parent_sensitive)
{
    int		x = widget->x;
    int		y = widget->y;
    Q8tkWidget	*child = widget->child;
    int		focus, sensitive;
    int		next_focus;
    Q8tkWidget	*sense_widget;

    /* �Ƥ��ե����������꤫�����Ȥ��ե�����������ǡ��ե�������������֤Ȥ���*/
    focus = (parent_focus || (widget == get_focus_widget())) ? TRUE : FALSE;

    /* �Ƥ�����ǽ�ǡ����Ȥ�����ǽ�ʤ顢����ǽ�Ȥ��� */
    sensitive = (parent_sensitive && widget->sensitive) ? TRUE : FALSE;
    sense_widget = (sensitive) ? widget : NULL;

    /* ��֤˥ե����������������硢�����̾�������ʤ� */
    next_focus = FALSE;



    widget_scrollin_drawn(widget);


		/* HBOX��VBOX �λҤξ��˸¤ꡢ*/
		/* ���֤�㴳�ѹ��Ǥ��롣	*/
		/* FRAME ��Ĥ��Ǥ˥襷�Ȥ��褦 */

    if (widget->parent) {
	switch ((widget->parent)->type) {
	case Q8TK_TYPE_FRAME:
	    if        (widget->placement_x == Q8TK_PLACEMENT_X_CENTER) {
		x += ((widget->parent)->sx -2 - widget->sx) / 2;
	    } else if (widget->placement_x == Q8TK_PLACEMENT_X_RIGHT) {
		x += ((widget->parent)->sx -2 - widget->sx);
	    }
	    widget->x = x;
	    break;
	case Q8TK_TYPE_VBOX:
	    if        (widget->placement_x == Q8TK_PLACEMENT_X_CENTER) {
		x += ((widget->parent)->sx - widget->sx) / 2;
	    } else if (widget->placement_x == Q8TK_PLACEMENT_X_RIGHT) {
		x += ((widget->parent)->sx - widget->sx);
	    }
	    widget->x = x;
	    break;
	case Q8TK_TYPE_HBOX:
	    if        (widget->placement_y == Q8TK_PLACEMENT_Y_CENTER) {
		y += ((widget->parent)->sy - widget->sy) / 2;
	    } else if (widget->placement_y == Q8TK_PLACEMENT_Y_BOTTOM) {
		y += ((widget->parent)->sy - widget->sy);
	    }
	    widget->y = y;
	    break;
	}
    }


		/* ��ʬ���Ȥ� type���Ȥ��Ȥʤɤ�񤯡�*/
		/* �Ҥ�����С�x,y ���롣		*/
		/* �Ҥ����(next)�Ρ�x,y ����롣	*/
		/* ľ���λҤ��Ф��ƤΤߺƵ�Ū�˽�����	*/

/*printf("%s (%d,%d) %d %d\n",debug_type(widget->type),widget->sx,widget->sy,widget->x,widget->y);fflush(stdout);*/

    if (widget->visible) {

	switch (widget->type) {

	case Q8TK_TYPE_WINDOW:			/* ������ɥ�		*/
	    if (widget->stat.window.no_frame) {
		/* no frame */
	    } else {
		q8gr_draw_window(x, y, widget->sx, widget->sy,
				 widget->stat.window.shadow_type,
				 (widget->stat.window.type
					== Q8TK_WINDOW_POPUP)
						? Q8GR_WIDGET_WINDOW : NULL);
	    }
	    if (child) {
		child->x = x  + (widget->stat.window.no_frame ? 0 : 1);
		child->y = y  + (widget->stat.window.no_frame ? 0 : 1);
		widget_draw(child, FALSE, sensitive);
	    }
	    break;

	case Q8TK_TYPE_BUTTON:			/* �ܥ���		*/
	case Q8TK_TYPE_TOGGLE_BUTTON:		/* �ȥ���ܥ���		*/
	    widget_focus_list_append(sense_widget);
	    q8gr_draw_button(x, y, widget->sx, widget->sy,
			     widget->stat.button.active, sense_widget);
	    if (child) {
		child->x = x + 1;
		child->y = y + 1;
		widget_draw(child, focus, sensitive);
	    }
	    break;

	case Q8TK_TYPE_CHECK_BUTTON:		/* �����å��ܥ���	*/
	    widget_focus_list_append(sense_widget);
	    q8gr_draw_check_button(x, y, widget->stat.button.active,
				   sense_widget);
	    if (child) {
		child->x = x + 3;
		child->y = y;
		widget_draw(child, focus, sensitive);
	    }
	    break;

	case Q8TK_TYPE_RADIO_BUTTON:		/* �饸���ܥ���		*/
#if	defined(OLD_FOCUS_CHANGE)
	    widget_focus_list_append(sense_widget);
#else	/* �����ʥե���������ư���� */
	    if (widget->stat.button.active == Q8TK_BUTTON_ON) {
		widget_focus_list_append(sense_widget);
	    }
#endif
	    q8gr_draw_radio_button(x, y, widget->stat.button.active,
				   sense_widget);
	    if (child) {
		child->x = x + 3;
		child->y = y;
		widget_draw(child, focus, sensitive);
	    }
	    break;

	case Q8TK_TYPE_FRAME:			/* �ե졼��		*/
	    q8gr_draw_frame(x, y, widget->sx, widget->sy,
			    widget->stat.frame.shadow_type,
			    widget->code, widget->name,
			    sense_widget);
	    if (child) {
		child->x = x + 1;
		child->y = y + 1;
		widget_draw(child, FALSE, sensitive);
	    }
	    break;

	case Q8TK_TYPE_LABEL:			/* ��٥�		*/
	    q8gr_draw_label(x, y,
			    widget->stat.label.foreground,
			    widget->stat.label.background,
			    widget->stat.label.reverse,
			    focus,
			    (widget->name) ? (widget->code) : Q8TK_KANJI_ANK,
			    (widget->name) ? (widget->name) : "",
			    sense_widget);
	    break;

	case Q8TK_TYPE_LOGO:			/* ��			*/
	    q8gr_draw_logo(x, y);
	    break;

	case Q8TK_TYPE_NOTEBOOK:		/* �Ρ��ȥ֥å�		*/
	    q8gr_draw_notebook(x, y, widget->sx, widget->sy,
			       widget, sense_widget);
	    if (child) {
		child->x = x;
		child->y = y;
		widget_draw(child, FALSE, sensitive);
	    }
	    break;
	case Q8TK_TYPE_NOTEPAGE:		/* �Ρ��ȥڡ���		*/
	    {
		int select_flag =
			      (widget == (widget->parent)->stat.notebook.page);

		q8gr_draw_notepage((widget->name) ? widget->code
						  : Q8TK_KANJI_ANK,
				   (widget->name) ? widget->name : "",
				   select_flag,
				   /*(select_flag) ? FALSE :*/ focus,
				   widget->parent,
				   sense_widget);
		if (child && select_flag) {
		    child->x = ((widget->parent)->x) + 1;
		    child->y = ((widget->parent)->y) + 3;
		    widget_focus_list_append(widget);
		    widget_draw(child, FALSE, sensitive);
		}
#if	defined(OLD_FOCUS_CHANGE)
		else {
		    widget_focus_list_append(sense_widget);
		}
#endif
	    }
	    break;

	case Q8TK_TYPE_VBOX:			/* ��ľ�ܥå���		*/
	    if (child) {
		child->x = x;	x += 0;
		child->y = y;	y += child->sy;
		while (child->next) {
		    child = child->next;
		    child->x = x;	x += 0;
		    child->y = y;	y += child->sy;
		}
		child = widget->child;
		widget_draw(child, FALSE, sensitive);
	    }
	    break;
	case Q8TK_TYPE_HBOX:			/* ��ʿ�ܥå���		*/
	    if (child) {
		child->x = x;	x += child->sx;
		child->y = y;	y += 0;
		while (child->next) {
		    child = child->next;
		    child->x = x;	x += child->sx;
		    child->y = y;	y += 0;
		}
		child = widget->child;
		widget_draw(child, FALSE, sensitive);
	    }
	    break;

	case Q8TK_TYPE_VSEPARATOR:		/* ��ľ���ڤ���		*/
	    q8gr_draw_vseparator(x, y, widget->sy);
	    break;
	case Q8TK_TYPE_HSEPARATOR:		/* ��ʿ���ڤ���		*/
	    q8gr_draw_hseparator(x, y, widget->sx);
	    break;

	case Q8TK_TYPE_COMBO:			/* ����ܥܥå���	*/
	    widget->stat.combo.entry->x  = x;
	    widget->stat.combo.entry->y  = y;
	    widget->stat.combo.entry->sx = widget->sx -3;
	    widget_draw(widget->stat.combo.entry, FALSE, sensitive);

#if	defined(OLD_FOCUS_CHANGE)
	    widget_focus_list_append(sense_widget);
#else	/* �����ʥե���������ư���� */
	    if (widget->stat.combo.entry->stat.entry.editable == FALSE) {
		widget_focus_list_append(sense_widget);
	    }
#endif
	    q8gr_draw_combo(x, y, widget->sx -3, focus, sense_widget);
	    break;

	case Q8TK_TYPE_LISTBOX:			/* �ꥹ�ȥܥå���	*/
	    widget_focus_list_append(sense_widget);
	    if (child) {
		child->x = x;	x += 0;
		child->y = y;	y += child->sy;
		while (child->next) {
		    child = child->next;
		    child->x = x;	x += 0;
		    child->y = y;	y += child->sy;
		}
		child = widget->child;
		widget_draw(child, focus, sensitive);
	    }
	    break;

	case Q8TK_TYPE_LIST_ITEM:		/* �ꥹ�ȥ����ƥ�	*/
	    if (child) {
		int rev   = (widget->parent->stat.listbox.selected == widget)
								? TRUE : FALSE;
		int under = (widget->parent->stat.listbox.active == widget)
								? TRUE : FALSE;
		if (rev && under) under = FALSE;

		q8gr_draw_list_item(x, y, widget->sx, focus, rev,
				    under, child->code, child->name,
				    sense_widget);
	    }
	    next_focus = focus; 
	    break;

	case Q8TK_TYPE_ADJUSTMENT:		/* �����㥹�ȥ���	*/
	    Q8tkAssert(FALSE, NULL);
	    break;

	case Q8TK_TYPE_HSCALE:			/* ��ʿ��������		*/
	    if (widget->stat.scale.adj) {
		int a_focus = (focus ||
			       (widget->stat.scale.adj == get_focus_widget()))
								? TRUE : FALSE;
		Q8tkWidget *a_widget = ((sensitive) ? widget->stat.scale.adj
						    : NULL);
		widget_focus_list_append(a_widget);
		widget->stat.scale.adj->x = x;
		widget->stat.scale.adj->y = y;
		q8gr_draw_hscale(x, y,
				 &(widget->stat.scale.adj->stat.adj),
				 a_focus,
				 widget->stat.scale.draw_value,
				 widget->stat.scale.value_pos,
				 a_widget);
	    }
	    break;
	case Q8TK_TYPE_VSCALE:			/* ��ľ��������		*/
	    if (widget->stat.scale.adj) {
		int a_focus = (focus ||
			       (widget->stat.scale.adj == get_focus_widget()))
								? TRUE : FALSE;
		Q8tkWidget *a_widget = ((sensitive) ? widget->stat.scale.adj
						    : NULL);
		widget_focus_list_append(a_widget);
		widget->stat.scale.adj->x = x;
		widget->stat.scale.adj->y = y;
		q8gr_draw_vscale(x, y,
				 &(widget->stat.scale.adj->stat.adj),
				 a_focus,
				 widget->stat.scale.draw_value,
				 widget->stat.scale.value_pos,
				 a_widget);
	    }
	    break;

	case Q8TK_TYPE_SCROLLED_WINDOW:		/* ��������ɥ�����ɥ� */
	    if (child) {
		int a_focus;
		Q8tkWidget *a_widget;

		int sx = widget->sx;
		int sy = widget->sy;

		if (widget->stat.scrolled.vscrollbar) { sx --; }
		if (widget->stat.scrolled.hscrollbar) { sy --; }

		q8gr_draw_scrolled_window(x, y, sx, sy,
					  Q8TK_SHADOW_ETCHED_OUT,
					  sense_widget);
		q8gr_set_screen_mask(x+1, y+1, sx-2, sy-2);
		widget->stat.scrolled.child_x0
				= widget->stat.scrolled.hadj->stat.adj.value;
		widget->stat.scrolled.child_y0
				= widget->stat.scrolled.vadj->stat.adj.value;
		widget->stat.scrolled.child_sx = sx -2;
		widget->stat.scrolled.child_sy = sy -2;

		child->x = x - widget->stat.scrolled.child_x0 +1;
		child->y = y - widget->stat.scrolled.child_y0 +1;

		widget_draw(child, FALSE, sensitive);
		q8gr_reset_screen_mask();

		if (widget->stat.scrolled.vscrollbar) {
		    a_focus = (focus ||
			(widget->stat.scrolled.vadj == get_focus_widget()))
								? TRUE : FALSE;
		    a_widget = ((sensitive) ? widget->stat.scrolled.vadj
					    : NULL);
		    /* �ե��������ꥹ�Ȥˤ�����ʤ��ۤ��������ΤǤϡ� */
		    widget_focus_list_append(a_widget);
		    q8gr_draw_vscale(x + sx, y,
				     &(widget->stat.scrolled.vadj->stat.adj),
				     a_focus,
				     FALSE, 0,
				     a_widget);
		}
		if (widget->stat.scrolled.hscrollbar) {
		    a_focus = (focus ||
			(widget->stat.scrolled.hadj == get_focus_widget()))
								? TRUE : FALSE;
		    a_widget = ((sensitive) ? widget->stat.scrolled.hadj
					    : NULL);
		    /* �ե��������ꥹ�Ȥˤ�����ʤ��ۤ��������ΤǤϡ� */
		    widget_focus_list_append(a_widget);
		    q8gr_draw_hscale(x, y + sy,
				     &(widget->stat.scrolled.hadj->stat.adj),
				     a_focus,
				     FALSE, 0,
				     a_widget);
		}
	    } else {
		q8gr_draw_window(x, y, widget->sx, widget->sy,
				 Q8TK_SHADOW_ETCHED_OUT,
				 NULL);
	    }
	    break;

	case Q8TK_TYPE_ENTRY:			/* ����ȥ�		*/
	    if (widget->stat.entry.editable) {
		widget_focus_list_append(sense_widget);
	    }
	    if (focus &&
		widget->stat.entry.editable &&
		widget->stat.entry.cursor_pos < 0) {
		q8tk_entry_set_position(widget,
				      q8gr_strlen(widget->code, widget->name));
	    }

	    q8gr_draw_entry(x, y, widget->sx, widget->code, widget->name,
			    widget->stat.entry.disp_pos,
			    ((focus && widget->stat.entry.editable)
					? widget->stat.entry.cursor_pos: -1),
			    sense_widget);
	    break;

	case Q8TK_TYPE_DIALOG:			/* ��������		*/
	    Q8tkAssert(FALSE, NULL);
	    break;
	case Q8TK_TYPE_FILE_SELECTION:		/* �ե����륻�쥯�����	*/
	    Q8tkAssert(FALSE, NULL);
	    break;

	default:
	    Q8tkAssert(FALSE, "Undefined type");
	}
    }


	/* ��ʬ���Ȥ���� (next) ��¸�ߤ���С��Ƶ�Ū�˽��� */
  
    if (widget->next) {
	widget = widget->next;
	widget_draw(widget, next_focus, parent_sensitive);
    }
}














/*------------------------------------------------------*/
/* �����꡼����̤������
   q8tk_grab_add() �����ꤵ�줿 WINDOW ��ȥåפȤ��ơ�
   ���Ƥλҥ��������åȤ��礭�������֤�׻�����
   menu_screen[][]�ˡ�ɽ�����Ƥ����ꤹ�롣
   Ʊ���ˡ�TAB �����򲡤��줿���Ρ��ե����������ѹ���
   ������Ƥ�����					*/
/*------------------------------------------------------*/

static	void	widget_construct(void)
{
    int		i, j, tmp;
    Q8tkWidget	*widget;

    q8gr_clear_screen();

    for (i=0; i<MAX_WINDOW_LAYER; i++) {

	widget = window_layer[i];

	if (widget) {
	    Q8tkAssert(widget->type == Q8TK_TYPE_WINDOW, NULL);

	    for (j=0; j<2; j++) {

		q8gr_clear_focus_screen();

		widget_size(widget, &tmp, &tmp, &tmp, &tmp);

		if (widget->stat.window.set_position == FALSE) {
		    widget->x = (Q8GR_SCREEN_X - widget->sx) / 2;
		    widget->y = (Q8GR_SCREEN_Y - widget->sy) / 2;
		} else {
		    widget->x = widget->stat.window.x;
		    widget->y = widget->stat.window.y;
		}

		widget_scrollin_adjust_reset();

		widget_focus_list_reset();
		widget_draw(widget, FALSE, widget->sensitive);

		if (widget_scrollin_adjust()) {
		    /* Redraw! */
		} else {
		    break;
		}

	    }

	} else {
	    break;
	}
    }


    if (get_focus_widget() == NULL) {
	q8tk_widget_set_focus(widget_focus_list_get_top());
    }

#if 0
    if (get_drag_widget()) {
	/* none */
    } else {
	Q8tkWidget *w;
	int exist;

	w = q8tk_tab_top_widget;

	if (w) {

	    do {			/* TAB TOP �� NOTEPAGE �ʳ������� */
		if (w->type != Q8TK_TYPE_NOTEPAGE) break;
		w = w->tab_next;
	    } while (w != q8tk_tab_top_widget);
	    q8tk_tab_top_widget = w;

	    exist = FALSE;		/* event_widget ���ºߤ��뤫�����å� */
	    do {			
		if (w == get_event_widget()) {
		    exist = TRUE;
		    break;
		}
		w = w->tab_next;
	    } while (w != q8tk_tab_top_widget);
	    if (! exist) {		/*    �ºߤ��ʤ���� NULL �ˤ��Ƥ��� */
		set_event_widget(NULL);
	    }
	}
    }
#endif

    if (disp_cursor) {
	q8gr_draw_mouse(mouse.x/8, mouse.y/16);
    }


#if 0
{
 int i, j; const char *c; void *w;
 for (j=0; j<Q8GR_SCREEN_Y; j++) {
  for (i=0; i<Q8GR_SCREEN_X; i++) {
    w = q8gr_get_focus_screen(i, j);
    if      (w == Q8GR_WIDGET_NONE)   c = " ";
    else if (w == Q8GR_WIDGET_WINDOW) c = "*";
    else                              c = debug_type(((Q8tkWidget *)w)->type);
    printf("%c", *c);
  }
  printf("\n");
 }
 printf("\n");
 fflush(0);
}
#endif

#if 0
{
 int i;
 for (i=0; i<MAX_WIDGET_SCROLLIN; i++) {
  Q8tkWidget *w = widget_scrollin[i].widget;
  printf("%d [%s] %p\n",widget_scrollin[i].drawn,(w)?debug_type(w->type):"",w);
 }
 printf("--\n");fflush(0);
}
#endif

#if 0
 for (tmp=0,i=0; i<MAX_WIDGET; i++) { if (widget_table[i]) tmp++; }
 printf("[TOTAL WIDGET] %d/%d : ",tmp,MAX_WIDGET);
 for (tmp=0,i=0; i<MAX_LIST; i++) { if (list_table[i]) tmp++; }
 printf("[TOTAL LIST] %d/%d\n",tmp,MAX_LIST);
#endif
}
