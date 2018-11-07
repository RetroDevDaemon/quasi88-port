/***********************************************************************
 * ����ե��å����� (�����ƥ��¸)
 *
 *	�ܺ٤ϡ� graph.h ����
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <unistd.h>

#include "quasi88.h"
#include "graph.h"
#include "device.h"

#include "screen.h"


#ifdef MITSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

/************************************************************************/

/* �ʲ��� static ���ѿ������ץ������ѹ��Ǥ���Τǥ����Х�ˤ��Ƥ��� */

	int	colormap_type	= 0;	 /* ���顼�ޥå׻���	 0��2	*/
	int	use_xsync	= TRUE;	 /* XSync ����Ѥ��뤫�ɤ���	*/
	int	use_xv		= FALSE; /* Xv ����Ѥ��뤫�ɤ���	*/
#ifdef MITSHM
	int	use_SHM		= TRUE;	 /* MIT-SHM ����Ѥ��뤫�ɤ���	*/
#endif


/* �ʲ��ϡ� event.c �ʤɤǻ��Ѥ��롢 OSD �ʥ����Х��ѿ� */

	Display	*x11_display;
	Window	x11_window;
	Atom	x11_atom_kill_type;
	Atom	x11_atom_kill_data;

	int	x11_width;		/* ���ߤΥ�����ɥ����� */
	int	x11_height;		/* ���ߤΥ�����ɥ��ι� */

	int	x11_mouse_rel_move;	/* �ޥ������а�ư�̸��Τ����뤫	*/

/************************************************************************/

static	T_GRAPH_SPEC	graph_spec;		/* ���ܾ���		*/

static	int		graph_exist;		/* ���ǡ����������Ѥ�	*/
static	T_GRAPH_INFO	graph_info;		/* ���λ��Ρ����̾���	*/


static	int	x11_enable_fullscreen;		/* ���ǡ������̲�ǽ	*/

static	Screen	*x11_screen;
static	GC	x11_gc;
static	Visual	*x11_visual;

static	int	x11_depth;
static	int	x11_byte_per_pixel;


/* ���ߤ�°�� */
static	int	x11_mouse_show   = TRUE;
static	int	x11_grab         = FALSE;
static	int	x11_keyrepeat_on = TRUE;

/************************************************************************/

/* ������ɥ������������ꥵ�������ϡ�������ɥ��������ѹ��ԲĤ�ؼ����� */
static	void	set_wm_hints(int w, int h, int fullscreen);

#ifdef MITSHM
static	XShmSegmentInfo	SHMInfo;

/* MIT-SHM �μ��Ԥ�ȥ�å� */
static	int	private_handler(Display *display, XErrorEvent *E);
#endif

/************************************************************************
 *	X11 Windod & DGA / XV
 ************************************************************************/


#include "graph-x11dga.c"


#ifdef	USE_XV
#include "graph-xv.c"
#endif


/************************************************************************
 *	X11�ν����
 *	X11�ν�λ
 ************************************************************************/

void	x11_init(void)
{
    x11_enable_fullscreen = FALSE;

    x11_display = XOpenDisplay(NULL);
    if (! x11_display) return;


    /* ���λ����Ǥ� use_xv ��̤����Τ��ᡢ DGA��XV �Ȥ�˽�������Ƥ��� */

#ifdef	USE_DGA
    dga_init();		/* DGA ����� */
#endif
#ifdef	USE_XV
    xv_init();		/* XV ����� */
#endif

    /* ������η�̡������̤���ǽ�ʤ顢 x11_enable_fullscreen �Ͽ��ˤʤ� */
}

static	void	init_verbose(void)
{
    if (verbose_proc) {

	if (! x11_display) { printf("FAILED\n"); return; }
	else               { printf("OK");               }

#if	defined(USE_DGA) || defined(USE_XV)
 #ifdef	USE_DGA
	if (use_xv == FALSE) {
	    dga_verbose();
	}
 #endif
 #ifdef	USE_XV
	if (use_xv) {
	    xv_verbose();
	}
 #endif
#else
	printf(" (fullscreen not supported)\n");
#endif
  }
}

/************************************************************************/

void	x11_exit(void)
{
    if (x11_display) {
	XAutoRepeatOn(x11_display);	/* �����ȥ�ԡ���������Ȥ��᤹ */

#ifdef	USE_DGA
	dga_exit();
#endif
#ifdef	USE_XV
	xv_exit();
#endif

	XSync(x11_display, True);

	/* DGAͭ������XCloseDisplay�ǥ��顼���Ǥ롣�ʤ���? */
	if (use_xv ||
	    x11_enable_fullscreen == FALSE) {	/* �Ȥꤢ����DGA�Ǥʤ������� */
	    XCloseDisplay(x11_display);
	}

	x11_display = NULL;
    }
}


/************************************************************************
 *	����ե��å������ν����
 *	����ե��å�������ư��
 *	����ե��å������ν�λ
 ************************************************************************/

/* �ޥ�����ɽ����¸����뤿�ᡢƩ���ޥ�������������Ѱդ��褦��
   ����ե��å���������˥����������������λ�����˴����롣*/
static	void	create_invisible_mouse(void);
static	void	destroy_invisible_mouse(void);


const T_GRAPH_SPEC	*graph_init(void)
{
    const T_GRAPH_SPEC *spec = NULL;

#ifndef	USE_XV
    use_xv = FALSE;
    x11_scaling = FALSE;
#endif

    if (verbose_proc) {
	printf("Initializing Graphic System (X11) ... ");
	init_verbose();
    }

    if (! x11_display) {
	return NULL;
    }


    x11_screen = DefaultScreenOfDisplay(x11_display);
    x11_gc     = DefaultGCOfScreen(x11_screen);
    x11_visual = DefaultVisualOfScreen(x11_screen);


    if (use_xv == FALSE) {
	spec = x11_graph_init();
    }
#ifdef	USE_XV
    if (use_xv) {
	spec = xv_graph_init();
    }
#endif


    if (spec) {

	/* �ޥ�����ɽ���Τ���Ρ�Ʃ���ޥ���������������� */
	create_invisible_mouse();

	/* Drag & Drop ����� */
	xdnd_initialize();
    }

    return spec;
}

/************************************************************************/

const T_GRAPH_INFO	*graph_setup(int width, int height,
				     int fullscreen, double aspect)
{
    if (use_xv == FALSE) {
	return x11_graph_setup(width, height, fullscreen, aspect);
    }
#ifdef	USE_XV
    if (use_xv) {
	return xv_graph_setup(width, height, fullscreen, aspect);
    }
#endif

    return NULL;
}

/************************************************************************/

void	graph_exit(void)
{
    if (use_xv == FALSE) {
	x11_graph_exit();
    }
#ifdef	USE_XV
    if (use_xv) {
	xv_graph_exit();
    }
#endif

    /* Ʃ���ޥ�������������˴� */
    destroy_invisible_mouse();
}

/*======================================================================*/

static	Cursor x11_cursor_id;
static	Pixmap x11_cursor_pix;

static	void	create_invisible_mouse(void)
{
    char data[1] = { 0x00 };
    XColor color;

    x11_cursor_pix = XCreateBitmapFromData(x11_display,
					   DefaultRootWindow(x11_display),
					   data, 8, 1);
    color.pixel    = BlackPixelOfScreen(x11_screen);
    x11_cursor_id  = XCreatePixmapCursor(x11_display,
					 x11_cursor_pix, x11_cursor_pix,
					 &color, &color, 0, 0);
}

static	void	destroy_invisible_mouse(void)
{
    if (x11_mouse_show == FALSE) {
	XUndefineCursor(x11_display, DefaultRootWindow(x11_display));
    }
    XFreePixmap(x11_display, x11_cursor_pix);
}

/*======================================================================*/

#ifdef MITSHM
/* MIT-SHM �μ��Ԥ�ȥ�å� */
static	int	private_handler(Display *display, XErrorEvent *E)
{
    char str[256];

    if (E->error_code == BadAccess ||
	E->error_code == BadAlloc) {
	use_SHM = FALSE;
	return 0;
    }

    XGetErrorText(display, E->error_code, str, 256);
    fprintf(stderr, "X Error (%s)\n", str);
    fprintf(stderr, " Error Code   %d\n", E->error_code);
    fprintf(stderr, " Request Code %d\n", E->request_code);
    fprintf(stderr, " Minor code   %d\n", E->minor_code);

    exit(-1);

    return 1;
}
#endif

/*======================================================================*/

/* ������ɥ��ޥ͡�����˥������ѹ��ԲĤ�ؼ����� */
static	void	set_wm_hints(int w, int h, int fullscreen)
{
    XSizeHints Hints;
    XWMHints WMHints;

    if (fullscreen) {
#if 1
	/* ���Τ��ޤ��ʤ����Τ�󤬡�����򤹤�ȥ�����ɥ��Υ����ȥ���Ȥ�
	   ̵���ʤ�Τǡ������̥������Υ�����ɥ������̤˥ե��åȤ��롣
	   xmame �Υ��������饳�ԥ� */
	#define MWM_HINTS_DECORATIONS   2
	typedef struct {
	    long flags;
	    long functions;
	    long decorations;
	    long input_mode;
	} MotifWmHints;

	Atom mwmatom;
	MotifWmHints mwmhints;
	mwmhints.flags = MWM_HINTS_DECORATIONS;
	mwmhints.decorations = 0;
	mwmatom = XInternAtom(x11_display,"_MOTIF_WM_HINTS",0);

	XChangeProperty(x11_display, x11_window,
			mwmatom, mwmatom, 32,
			PropModeReplace, (unsigned char *)&mwmhints, 4);
#endif

	Hints.x      = 0;
	Hints.y      = 0;
	Hints.flags  = PMinSize|PMaxSize|USPosition|USSize;
	Hints.win_gravity = NorthWestGravity;
    } else {
	Hints.flags      = PSize|PMinSize|PMaxSize;
    }
    Hints.min_width  = Hints.max_width  = Hints.base_width  = w;
    Hints.min_height = Hints.max_height = Hints.base_height = h;
    WMHints.input = True;
    WMHints.flags = InputHint;

    XSetWMHints(x11_display, x11_window, &WMHints);
    XSetWMNormalHints(x11_display, x11_window, &Hints);
}


/************************************************************************
 *	���γ���
 *	���β���
 ************************************************************************/

void	graph_add_color(const PC88_PALETTE_T color[],
			int nr_color, unsigned long pixel[])
{
    if (use_xv == FALSE) {
	x11_graph_add_color(color, nr_color, pixel);
    }
#ifdef	USE_XV
    if (use_xv) {
	xv_graph_add_color(color, nr_color, pixel);
    }
#endif
}

/************************************************************************/

void	graph_remove_color(int nr_pixel, unsigned long pixel[])
{
    if (use_xv == FALSE) {
	x11_graph_remove_color(nr_pixel, pixel);
    }
#ifdef	USE_XV
    if (use_xv) {
	xv_graph_remove_color(nr_pixel, pixel);
    }
#endif
}


/************************************************************************
 *	����ե��å��ι���
 ************************************************************************/

void	graph_update(int nr_rect, T_GRAPH_RECT rect[])
{
    if (use_xv == FALSE) {
	x11_graph_update(nr_rect, rect);
    }
#ifdef	USE_XV
    if (use_xv) {
	xv_graph_update(nr_rect, rect);
    }
#endif
}


/************************************************************************
 *	�����ȥ������
 *	°��������
 ************************************************************************/

void	graph_set_window_title(const char *title)
{
    static char saved_title[128];

    if (title) {
	saved_title[0] = '\0';
	strncat(saved_title, title, sizeof(saved_title)-1);
    }

    if (graph_exist) {
	XStoreName(x11_display, x11_window, saved_title);
    }
}

/************************************************************************/

void	graph_set_attribute(int mouse_show, int grab, int keyrepeat_on)
{
    x11_mouse_show   = mouse_show;
    x11_grab         = grab;
    x11_keyrepeat_on = keyrepeat_on;

    if (x11_get_focus) {
	x11_set_attribute_focus_in();
    }
}


/***********************************************************************
 *
 *	X11 �ȼ��ؿ�
 *
 ************************************************************************/

/* �ե��������򼺤ä������ޥ���ɽ�������󥰥�֡�������ԡ��Ȥ���ˤ��� */
void	x11_set_attribute_focus_out(void)
{
    XUndefineCursor(x11_display, x11_window);
    XUngrabPointer(x11_display, CurrentTime);
    XAutoRepeatOn(x11_display);
}

/* �ե��������������������ޥ���������֡�������ԡ��Ȥ������̤���᤹ */
void	x11_set_attribute_focus_in(void)
{
    int dga = (use_xv == FALSE && graph_info.fullscreen) ? TRUE : FALSE;

    if (x11_mouse_show) XUndefineCursor(x11_display, x11_window);
    else                XDefineCursor(x11_display, x11_window, x11_cursor_id);

    if (x11_grab || dga)		/* ����ֻؼ����ꡢ�ޤ��� */
					/* DGA ���ѻ��ϥ���֤��� */
			  XGrabPointer(x11_display, x11_window, True,
				       PointerMotionMask | ButtonPressMask |
				       ButtonReleaseMask,
				       GrabModeAsync, GrabModeAsync,
				       x11_window, None, CurrentTime);
    else		  XUngrabPointer(x11_display, CurrentTime);

    if (x11_keyrepeat_on) XAutoRepeatOn(x11_display);
    else                  XAutoRepeatOff(x11_display);


    /* �ޥ�����ư�ˤ�륤�٥��ȯ�����ν�����ˡ������ */

    if      (dga)                     x11_mouse_rel_move = 1;
    else if (x11_grab &&
	     x11_mouse_show == FALSE) x11_mouse_rel_move = -1;
    else                              x11_mouse_rel_move = 0;

    /* �ޥ�����ư���Υ��٥�ȤˤĤ��� (event.c)

       DGA �ξ�硢�ޥ�����ư�̤����Τ����   �� 1: ���а�ư
       X11 �ξ�硢�ޥ������а��֤����Τ���� �� 0: ���а�ư

       ������ɥ�����ֻ��ϡ��ޥ�����������ɥ���ü�ˤ��ɤ��夯��
       ����ʾ�ư���ʤ��ʤ롣
       �����ǡ��ޥ������˥�����ɥ�������˥����פ����뤳�Ȥǡ�
       ̵�¤˥ޥ�����ư�������Ȥ��Ǥ��뤫�Τ褦�ˤ��롣
       ���λ��Υޥ����ΰ�ư�����������׻��ˤ�������̤Ȥ��ư�����
       �ޤ����ޥ�������ɽ���ˤ��Ƥ����ʤ��ȡ�̵�ͤ��ͻҤ�������Τ�
       ���ε�ǽ�ϥޥ����ʤ����ΤߤȤ���       �� -1: ��������а�ư
    */
}
