/***********************************************************************
 * ����ե��å����� (X11 Window & DGA 1.0)
 *
 *
 ************************************************************************/

#ifdef	USE_DGA
#include <unistd.h>
#include <sys/types.h>
#if 1	/* obsolete? */
#include <X11/extensions/xf86dga.h>
#else
#include <X11/extensions/Xxf86dga.h>
#endif
#include <X11/extensions/xf86vmode.h>
#endif

/************************************************************************/

#ifdef	USE_DGA
static	char			*dga_addr;
static	int			dga_width;
static	int			dga_bank;
static	int			dga_ram;
static	XF86VidModeModeInfo	**dga_mode = NULL;
static	int			dga_mode_count;

static	int			dga_mode_selected = 0;
#endif

/************************************************************************
 *	DGA�ν����
 *	DGA�ν�λ
 ************************************************************************/

#ifdef	USE_DGA
enum {
    DGA_ERR_NONE = 0,
    DGA_ERR_AVAILABLE,
    DGA_ERR_ROOT_RIGHTS,
    DGA_ERR_LOCAL_DISPLAY,
    DGA_ERR_QUERY_VERSION,
    DGA_ERR_QUERY_EXTENSION,
    DGA_ERR_QUERY_DIRECT_VIDEO,
    DGA_ERR_QUERY_DIRECT_PRESENT,
    DGA_ERR_GET_VIDEO,
    DGA_ERR_BAD_VALUE,
    DGA_ERR_MANY_BANKS,
    DGA_ERR_XVM_QUERY_VERSION,
    DGA_ERR_XVM_QUERY_EXTENSION,
    DGA_ERR_XVM_GET_ALL_MODE_LINES
};
static	int	DGA_error = DGA_ERR_AVAILABLE;

static	void	dga_init(void)
{
    int i, j;
    char *s;

    if (geteuid()) {
	DGA_error = DGA_ERR_ROOT_RIGHTS;
    }
    else
    if (! (s = getenv("DISPLAY")) || (s[0] != ':')) {
	DGA_error = DGA_ERR_LOCAL_DISPLAY;
    }
    else
    if (! XF86DGAQueryVersion(x11_display, &i, &j)) {
	DGA_error = DGA_ERR_QUERY_VERSION;
    }
    else
    if (! XF86DGAQueryExtension(x11_display, &i, &j)) {
	DGA_error = DGA_ERR_QUERY_EXTENSION;
    }
    else
    if (! XF86DGAQueryDirectVideo(x11_display,
				  DefaultScreen(x11_display), &i)) {
	DGA_error = DGA_ERR_QUERY_DIRECT_VIDEO;
    }
    else
    if (! (i & XF86DGADirectPresent)) {
	DGA_error = DGA_ERR_QUERY_DIRECT_PRESENT;
    }
    else
    if (! XF86DGAGetVideo(x11_display, DefaultScreen(x11_display),
			  &dga_addr, &dga_width, &dga_bank, &dga_ram)) {
	DGA_error = DGA_ERR_GET_VIDEO;
    }
    else
    if (dga_addr==NULL || dga_width==0 || dga_bank==0 || dga_ram==0) {
	DGA_error = DGA_ERR_BAD_VALUE;
    }
    else
    if (dga_ram * 1024 != dga_bank) {
	DGA_error = DGA_ERR_MANY_BANKS;
    }
    else
    if (! XF86VidModeQueryVersion(x11_display, &i, &j)) {
	DGA_error = DGA_ERR_XVM_QUERY_VERSION;
    }
    else
    if (! XF86VidModeQueryExtension(x11_display, &i, &j)) {
	DGA_error = DGA_ERR_XVM_QUERY_EXTENSION;
    }
    else
    if (! XF86VidModeGetAllModeLines(x11_display,
				     DefaultScreen(x11_display),
				     &dga_mode_count, &dga_mode)) {
	DGA_error = DGA_ERR_XVM_GET_ALL_MODE_LINES;
    }
    else
    {
	DGA_error = DGA_ERR_NONE;
	x11_enable_fullscreen = 1;
    }
}


static	void	dga_verbose(void)
{
    printf("\n");
    printf("  DGA : ");

    if      (DGA_error == DGA_ERR_NONE)
	printf("OK");
    else if (DGA_error == DGA_ERR_ROOT_RIGHTS)
	printf("FAILED (Must be suid root)");
    else if (DGA_error == DGA_ERR_LOCAL_DISPLAY)
	printf("FAILED (Only works on a local display)");
    else if (DGA_error == DGA_ERR_QUERY_VERSION)
	printf("FAILED (XF86DGAQueryVersion)");
    else if (DGA_error == DGA_ERR_QUERY_EXTENSION)
	printf("FAILED (XF86DGAQueryExtension)");
    else if (DGA_error == DGA_ERR_QUERY_DIRECT_VIDEO)
	printf("FAILED (XF86DGAQueryDirectVideo)");
    else if (DGA_error == DGA_ERR_QUERY_DIRECT_PRESENT)
	printf("FAILED (Xserver not support DirectVideo)");
    else if (DGA_error == DGA_ERR_GET_VIDEO)
	printf("FAILED (XF86DGAGetVideo)");
    else if (DGA_error == DGA_ERR_BAD_VALUE)
	printf("FAILED (XF86DGAGetVideo Bad Value)");
    else if (DGA_error == DGA_ERR_MANY_BANKS)
	printf("FAILED (banked graphics modes not supported)");
    else if (DGA_error == DGA_ERR_XVM_QUERY_VERSION)
	printf("FAILED (XF86VidModeQueryVersion)");
    else if (DGA_error == DGA_ERR_XVM_QUERY_EXTENSION)
	printf("FAILED (XF86VidModeQueryExtension)");
    else if (DGA_error == DGA_ERR_XVM_GET_ALL_MODE_LINES)
	printf("FAILED (XF86VidModeGetAllModeLines)");
    else
	printf("FAILED (Not Support)");

    if (DGA_error == DGA_ERR_NONE) printf(", fullscreen available\n");
    else                           printf(", fullscreen not available\n");
}


static	void	dga_exit(void)
{
    if (dga_mode) XFree(dga_mode);
}
#endif


/************************************************************************
 *	����ե��å������ν����
 *	����ե��å�������ư��
 *	����ե��å������ν�λ
 ************************************************************************/

static	const T_GRAPH_SPEC	*x11_graph_init(void)
{
    int	win_w, win_h;
    int	ful_w, ful_h;

    int i, count;
    XPixmapFormatValues *pixmap;


    /* �����٤ȡ��ԥ����뤢����ΥХ��ȿ�������å� */

    pixmap = XListPixmapFormats(x11_display, &count);
    if (pixmap == NULL) {
	if (verbose_proc) printf("  X11 error (Out of memory ?)\n");
	return NULL;
    }
    for (i = 0; i < count; i++) {
	if (pixmap[i].depth == DefaultDepthOfScreen(x11_screen)) {
	    x11_depth = pixmap[i].depth;
	    if      (x11_depth <=  8 && pixmap[i].bits_per_pixel ==  8) {
		x11_byte_per_pixel = 1;
	    }
	    else if (x11_depth <= 16 && pixmap[i].bits_per_pixel == 16) {
		x11_byte_per_pixel = 2;
	    }
	    else if (x11_depth <= 32 && pixmap[i].bits_per_pixel == 32) {
		x11_byte_per_pixel = 4;
	    }
	    else {		/* �嵭�ʳ��Υե����ޥåȤ����ݤʤΤ� NG */
		x11_byte_per_pixel = 0;
	    }
	    break;
	}
    }
    XFree(pixmap);


    {				/* ���б��� depth �ʤ��Ƥ� */
	const char *s = NULL;
	switch (x11_byte_per_pixel) {
	case 0:	s = "this bpp is not supported";	break;
#ifndef	SUPPORT_8BPP
	case 1:	s = "8bpp is not supported";		break;
#endif
#ifndef	SUPPORT_16BPP
	case 2:	s = "16bpp is not supported";		break;
#endif
#ifndef	SUPPORT_32BPP
	case 4:	s = "32bpp is not supported";		break;
#endif
	}
	if (s) {
	    if (verbose_proc) printf("  %s\n",s);    
	    return NULL;
	}
    }

    if (x11_depth < 4) {
	if (verbose_proc) printf("  < 4bpp is not supported\n");
	return NULL;
    }


    /* ���Ѳ�ǽ�ʥ�����ɥ��Υ�������Ĵ�٤Ƥ��� */

    win_w = 10000;	/* ������ɥ�������ʤ���Ŭ�����礭���ͤ򥻥å� */
    win_h = 10000;

#ifdef	USE_DGA		/* �����̥⡼�ɤϡ������������κǤ��礭�ʤΤ򥻥å� */
    if (x11_enable_fullscreen) {
	int i;
	ful_w = 0;
	ful_h = 0;
	for (i = 0; i < dga_mode_count; i++) {
	    if (ful_w < dga_mode[i]->hdisplay) {
		ful_w = dga_mode[i]->hdisplay;
		ful_h = dga_mode[i]->vdisplay;
	    }
	    if (verbose_proc)
		printf("  VidMode %3d: %dx%d %d\n",
		       i, dga_mode[i]->hdisplay, dga_mode[i]->vdisplay,
		       dga_mode[i]->privsize);
	}
    }
    else
#endif
    {
	ful_w = 0;
	ful_h = 0;
    }

    graph_spec.window_max_width      = win_w;
    graph_spec.window_max_height     = win_h;
    graph_spec.fullscreen_max_width  = ful_w;
    graph_spec.fullscreen_max_height = ful_h;
    graph_spec.forbid_status         = FALSE;
    graph_spec.forbid_half           = FALSE;

    if (verbose_proc)
	printf("  INFO: %dbpp(%dbyte), Maxsize=win(%d,%d),full(%d,%d)\n",
	       x11_depth, x11_byte_per_pixel,
	       win_w, win_h, ful_w, ful_h);


    return &graph_spec;
}

/************************************************************************/

/* ������ɥ������������ꥵ���������˴��� ����ӡ�
   �����̥⡼�ɳ��ϻ������������ػ�����λ�� �Ρ� �ºݤν����򤹤�ؿ� */

static	int	create_window(int width, int height,
			      void **ret_buffer, int *ret_nr_color);
static	int	resize_window(int width, int height, void *old_buffer,
			      void **ret_buffer, int *ret_nr_color);
static	void	destroy_window(void *old_buffer);

#ifdef	USE_DGA
static	int	create_DGA(int *width, int *height, double aspect,
			   int *ret_nr_color);
static	int	resize_DGA(int *width, int *height, double aspect,
			   int *ret_nr_color);
static	void	destroy_DGA(void);
#else
#define		create_DGA(w, h, aspect, ret_nr_color)	(FALSE)
#define		resize_DGA(w, h, screen, ret_nr_color)	(FALSE)
#define		destroy_DGA()
#endif


static	const T_GRAPH_INFO	*x11_graph_setup(int width, int height,
						 int fullscreen, double aspect)
{

    /*
        ���׵��|           Window            |           ������            |
          ��    |-----------------------------|-----------------------------|
      ���ߤϡ�  |  Ʊ��������  |  �㤦������  |  Ʊ��������  |  �㤦������  |
    ------------+-----------------------------+-----------------------------+
      ̤�����  |          Window����         |          ����������         |
    ------------+--------------+--------------+-----------------------------+
     ������ɥ� |              |Window�ꥵ����|  Window�˴� �� ����������   |
    ------------+--------------+--------------+--------------+--------------+
       ������   |  �������˴� �� Window����   |              |�����̥ꥵ����|
    ------------+-----------------------------+--------------+--------------+
    */

    int nr_color = 0;
    void *buf = NULL;
    int success;

    /* �������ԲĤʤ顢�������׵��̵�� */
    if ((x11_enable_fullscreen == FALSE) && (fullscreen)) {
	fullscreen = FALSE;
    }


    /* ������ɥ������������ؤξ�硢ͽ�ḽ�ߤξ��֤��˴����Ƥ��� */
    if (graph_exist) {
	if (verbose_proc) printf("Re-Initializing Graphic System (X11)\n");

	if ((graph_info.fullscreen == FALSE) && (fullscreen)) {
	    /* ������ɥ� �� ������ �ڤ��ؤ� */
	    destroy_window(graph_info.buffer);
	    graph_exist = FALSE;
	}
	else if ((graph_info.fullscreen) && (fullscreen == FALSE)) {
	    /* ������ �� ������ɥ� �ڤ��ؤ� */
	    destroy_DGA();
	    graph_exist = FALSE;
	}
    }


    /* �� �� ������ �ξ�� */
    if (fullscreen) {
	if (graph_exist == 0) {
	    success = create_DGA(&width, &height, aspect, &nr_color);
	} else {
	    success = resize_DGA(&width, &height, aspect, &nr_color);
	}

	if (success) {		/* ���� */
	    goto SUCCESS;
	} else {		/* ���Ԥ����饦����ɥ��Ǥ��ľ�� */
	    fullscreen = FALSE;
	    graph_exist = 0;
	}
    }


    /* �� �� ������ɥ� �ξ�� */
    {
	if (graph_exist == 0) {
	    success = create_window(width, height, &buf, &nr_color);
	} else {
	    success = resize_window(width, height,
				    graph_info.buffer, &buf, &nr_color);
	}

	if (success) {		/* ���� */
	    goto SUCCESS;
	}
    }

    /* ���Ȥ��Ȥ����� */
    graph_exist = FALSE;
    return NULL;


 SUCCESS:
    graph_exist = TRUE;

#ifdef	USE_DGA
    if (fullscreen) {
	graph_info.fullscreen		= TRUE;
	graph_info.width		= width;
	graph_info.height		= height;
	graph_info.byte_per_pixel	= x11_byte_per_pixel;
	graph_info.byte_per_line	= dga_width * x11_byte_per_pixel;
	graph_info.buffer		= dga_addr;
	graph_info.nr_color		= nr_color;
	graph_info.write_only		= TRUE;
      /*graph_info.write_only		= FALSE;*/	/*TEST*/
	graph_info.broken_mouse		= TRUE;
	graph_info.draw_start		= NULL;
	graph_info.draw_finish		= NULL;
	graph_info.dont_frameskip	= FALSE;
    } else
#endif
    {
	graph_info.fullscreen		= FALSE;
	graph_info.width		= width;
	graph_info.height		= height;
	graph_info.byte_per_pixel	= x11_byte_per_pixel;
	graph_info.byte_per_line	= width * x11_byte_per_pixel;
	graph_info.buffer		= buf;
	graph_info.nr_color		= nr_color;
	graph_info.write_only		= FALSE;
	graph_info.broken_mouse		= FALSE;
	graph_info.draw_start		= NULL;
	graph_info.draw_finish		= NULL;
	graph_info.dont_frameskip	= FALSE;

	/* ������ɥ��Υ����ȥ�С������� */
	graph_set_window_title(NULL);

	XMapRaised(x11_display, x11_window);
    }

    x11_width  = width;
    x11_height = height;

#if 0	/* debug */
printf("@ fullscreen      %d\n",    graph_info.fullscreen    );
printf("@ width           %d\n",    graph_info.width         );
printf("@ height          %d\n",    graph_info.height        );
printf("@ byte_per_pixel  %d\n",    graph_info.byte_per_pixel);
printf("@ byte_per_line   %d\n",    graph_info.byte_per_line );
printf("@ buffer          %p\n",    graph_info.buffer        );
printf("@ nr_color        %d\n",    graph_info.nr_color      );
printf("@ write_only      %d\n",    graph_info.write_only    );
printf("@ broken_mouse    %d\n",    graph_info.broken_mouse  );
printf("@ dont_frameskip  %d\n",    graph_info.dont_frameskip);
#endif

    return &graph_info;
}

/************************************************************************/

static	void	x11_graph_exit(void)
{
    if (graph_exist) {

	if (graph_info.fullscreen) {
	    destroy_DGA();
	} else {
	    destroy_window(graph_info.buffer);
	}

	graph_exist = FALSE;
    }
}


/*======================================================================*/

/* ������ɥ������������ꥵ���������˴��� ���뤤�ϡ�
   �����̥⡼�ɳ��ϻ������������ػ�����λ�� �ˡ�
   ���줾�� ���顼�ޥåפ���ݡ��ƽ���������� �Ȥ���������ɬ�� */
static	int	create_colormap(int fullscreen);
static	int	reuse_colormap(void);
static	void	destroy_colormap(void);

/* ������ɥ������������ꥵ���������˴����ˤϡ����᡼���γ��ݡ�������ɬ�� */
static	void	*create_image(int width, int height);
static	void	destroy_image(void *buf);


static	int	create_window(int width, int height,
			      void **ret_buffer, int *ret_nr_color)
{
    if (verbose_proc) printf("  Opening window ... ");
    x11_window = XCreateSimpleWindow(x11_display,
				     RootWindowOfScreen(x11_screen),
				     0, 0,
				     width, height,
				     0,
				     WhitePixelOfScreen(x11_screen),
				     BlackPixelOfScreen(x11_screen));
    if (verbose_proc)
	printf("%s (%dx%d)\n", (x11_window ? "OK" : "FAILED"), width, height);

    if (! x11_window) {
	return FALSE; 
    }

    /* ������ɥ��ޥ͡����㡼������(�������ѹ��Բ�)��ؼ����� */
    set_wm_hints(width, height, FALSE);

    /* ���顼����ݤ��� */
    (*ret_nr_color) = create_colormap(FALSE);

    /* ���٥�Ȥ����� */
    XSelectInput(x11_display, x11_window,
		 FocusChangeMask | ExposureMask |
		 KeyPressMask | KeyReleaseMask |
		 ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

    /* �ꥵ������ (���ơ����� ON/OFF���ػ�) �ˡ����̤��ä��ʤ��褦�ˤ���(?) */
    {
	XSetWindowAttributes attributes;
	attributes.bit_gravity = NorthWestGravity;
	XChangeWindowAttributes(x11_display, x11_window,
				CWBitGravity, &attributes);
    }

    /* ������λ�����Ǥ������ơ����ȥ������ */
    x11_atom_kill_type = XInternAtom(x11_display, "WM_PROTOCOLS", False);
    x11_atom_kill_data = XInternAtom(x11_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(x11_display, x11_window, &x11_atom_kill_data, 1);

    /* �����꡼��Хåե� �� image ����� */
    (*ret_buffer) = create_image(width, height);

    /* Drag & Drop �����դ����� */
    xdnd_start();

    return (((*ret_nr_color) >= 16) && (*ret_buffer)) ? TRUE : FALSE;
}


static	int	resize_window(int width, int height, void *old_buffer,
			      void **ret_buffer, int *ret_nr_color)
{
    Window child;
    int x, y;

    if (verbose_proc) printf("  Resizing window ... ");

    /* ������ɥ��ޥ͡����㡼�ؿ����ʥ�������ؼ����� */
    set_wm_hints(width, height, FALSE);

    /* �ꥵ�������ƥ�����ɥ������̳��˽ФƤ��ޤä��饤��ʤΤǡ����ξ���
       ������ɥ��������˰�ư�����褦�Ȼפ������Ķ��ˤ�äƤ� XGetGeometry()
       ��ȤäƤ�����Ⱥ�ɸ�������Ǥ��ʤ����� XMoveWindow() ��ȤäƤ⡢
       ������ɥ����ȤȤ����θ�����˰�ư�����礬���롣������ɥ��ޥ͡����㡼
       ���ؤ�äƤ��뤫����Ȼפ��Τ������ɤ�����Τ���������Ǥ��礦 ? */
#if 1
    /* �Ȥꤢ�����롼�ȥ�����ɥ���������а��֤���ơ��������夫���β��̳�
       ���ä����ư�����롣���ۥ�����ɥ��ޥ͡����㡼�Ǥ�����פ��� */

    XTranslateCoordinates(x11_display, x11_window,
			  DefaultRootWindow(x11_display), 0, 0,
			  &x, &y, &child);
    if (x < 0 || y < 0) {
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	XMoveResizeWindow(x11_display, x11_window, x, y, width, height);
    } else
#endif
    {
	XResizeWindow(x11_display, x11_window, width, height);
    }

    if (verbose_proc)
	printf("%s (%dx%d)\n", (x11_window ? "OK" : "FAILED"), width, height);

    /* image ���˴����� */
    if (old_buffer) {
	destroy_image(old_buffer);
    }

    /* ���顼�ޥå׾��֤κ����� */
    (*ret_nr_color) = reuse_colormap();

    /* �����꡼��Хåե� �� image ����� */
    (*ret_buffer) = create_image(width, height);

    return (((*ret_nr_color) >= 16) && (*ret_buffer)) ? TRUE : FALSE;
}


static	void	destroy_window(void *old_buffer)
{
    if (verbose_proc) printf("  Closing Window\n");

    /* ���顼�ޥå��˴� */
    destroy_colormap();

    /* ���᡼���˴� */
    if (old_buffer) {
	destroy_image(old_buffer);
    }

    /* ������ɥ��˴� */
    XDestroyWindow(x11_display, x11_window);

    if (x11_grab) {
	XUngrabPointer(x11_display, CurrentTime);
	x11_grab = FALSE;
    }

    XSync(x11_display, True);		/* �����٥���˴� */
}



#ifdef	USE_DGA

#define	FABS(a)		(((a) >= 0.0) ? (a) : -(a))

static	int	search_mode(int w, int h, double aspect)
{
    int i;
    int fit = -1;
    int fit_w = 0, fit_h = 0;
    float fit_a = 0.0;

    for (i = 0; i < dga_mode_count; i++) {
	/* ���̥������˼��ޤäƤ��뤳�� */
	if (w <= dga_mode[i]->hdisplay &&
	    h <= dga_mode[i]->vdisplay) {

	    int tmp_w = dga_mode[i]->hdisplay;
	    int tmp_h = dga_mode[i]->vdisplay;
	    double tmp_a = FABS(((float)tmp_w / tmp_h) - aspect);

	    /* �ǽ�˸��Ĥ��ä���Τ�ޤ��ϥ��祤�� */
	    if (fit == -1) {
		fit = i;
		fit_w = tmp_w;
		fit_h = tmp_h;
		fit_a = tmp_a;

	    } else {
	    /* ������ϡ�����Τ���٤ơ����ե��åȤ���Х��祤�� */

		/* ��Ĺ��˥������ʤ����������ڥ���̤����ξ�� */
		if (aspect >= 1.0 || aspect < 0.01) {

		    /* �Ĥκ��ξ��ʤ��ۤ����ޤ��ϥ����ڥ�����ζᤤ�ۤ� */
		    if (((tmp_h - h) < (fit_h - h)) ||
			((tmp_h == fit_h) && (tmp_a < fit_a))) {
			fit = i;
			fit_w = tmp_w;
			fit_h = tmp_h;
			fit_a = tmp_a;
		    }

		} else {	/* ��Ĺ��˥����ξ�� (�Ȥä����Ȥʤ�����) */

		    /* ���κ��ξ��ʤ��ۤ����ޤ��ϥ����ڥ�����ζᤤ�ۤ� */
		    if (((tmp_w - w) < (fit_w - w)) ||
			((tmp_w == fit_w) && (tmp_a < fit_a))) {
			fit = i;
			fit_w = tmp_w;
			fit_h = tmp_h;
			fit_a = tmp_a;
		    }
		}
	    }
	}
    }
    /* ��������Τ������ʤ����ϡ� -1 ���֤� */
    return fit;
}


static	int	create_DGA(int *width, int *height, double aspect,
			   int *ret_nr_color)
{
    int fit = search_mode(*width, *height, aspect);

    if (fit < 0 || dga_mode_count <= fit) {
	return FALSE;
    }

    if (verbose_proc) printf("  Starting DGA <%dx%d> ... ", *width, *height);


    /* �ºݤβ��̥������򥻥åȤ��� */
    *width  = dga_mode[ fit ]->hdisplay;
    *height = dga_mode[ fit ]->vdisplay;

    /* �⡼�ɥ饤����ڤ��ؤ� */
    XF86VidModeSwitchToMode(x11_display, DefaultScreen(x11_display),
			    dga_mode[fit]);

    /* DGA��ͭ���ˤ��� */
    XF86DGADirectVideo(x11_display, DefaultScreen(x11_display), 
		       XF86DGADirectGraphics |
		       XF86DGADirectMouse |
		       XF86DGADirectKeyb);

    XF86DGASetViewPort(x11_display, DefaultScreen(x11_display), 0, 0);

    if (verbose_proc) printf("OK (%dx%d)\n", *width, *height);

    /* �����ܡ��ɡ��ޥ����򥰥�֤��� */
    x11_window = DefaultRootWindow(x11_display);

    XGrabKeyboard(x11_display, x11_window, True, GrabModeAsync,
		  GrabModeAsync,  CurrentTime);

    XGrabPointer(x11_display, x11_window, True,
		 PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
		 GrabModeAsync, GrabModeAsync, x11_window, None, CurrentTime);

    /* ���顼����ݤ��� */
    (*ret_nr_color) = create_colormap(TRUE);

    /* ���٥�Ȥ����ꤹ����ܤ���Τǡ����ꤷ�ʤ� */

    x11_get_focus = TRUE;
    x11_set_attribute_focus_in();

    return TRUE;
}


static	int	resize_DGA(int *width, int *height, double aspect,
			   int *ret_nr_color)
{
    int fit = search_mode(*width, *height, aspect);

    if (fit < 0 || dga_mode_count <= fit) {
	return FALSE;
    }

    if (verbose_proc) printf("  Switching DGA <%dx%d> ... ", *width, *height);


    /* �ºݤβ��̥������򥻥åȤ��� */
    *width  = dga_mode[ fit ]->hdisplay;
    *height = dga_mode[ fit ]->vdisplay;

    /* DGAͭ���Τޤޤ��ȼ��Ԥ���? */
    XF86DGADirectVideo(x11_display, DefaultScreen(x11_display), 0);

    XF86VidModeSwitchToMode(x11_display, DefaultScreen(x11_display),
			    dga_mode[fit]);

    /* �⡼�����ؤκݤ�DGA��̵���ˤ��Ƥ��ޤä��Τǡ�����ͭ���ˤ��� */
    XF86DGADirectVideo(x11_display, DefaultScreen(x11_display),
		       XF86DGADirectGraphics |
		       XF86DGADirectMouse |
		       XF86DGADirectKeyb);

    if (verbose_proc) printf("OK (%dx%d)\n", *width, *height);

    /* ���顼�ޥå׾��֤κ����� */
    (*ret_nr_color) = reuse_colormap();

    return TRUE;
}


static	void	destroy_DGA(void)
{
    if (verbose_proc) printf("  Stopping DGA\n");

    /* ���顼�ޥå��˴� */
    destroy_colormap();

    /* DGA ��� */
    XF86DGADirectVideo(x11_display, DefaultScreen(x11_display), 0);

    /* �����٤򸵤��᤹ */
    XF86VidModeSwitchToMode(x11_display, DefaultScreen(x11_display),
			    dga_mode[0]);
    /* XF86VidModeSwitchMode(x11_display, DefaultScreen(x11_display), -1);
       XF86VidModeSwitchMode(x11_display, DefaultScreen(x11_display), +1); */

    dga_mode_selected = 0;	/* �����������DGA�⡼�ɾ��󥯥ꥢ */

    /* �����ܡ��ɡ��ޥ����Υ���֤��� */
    XUngrabPointer(x11_display, CurrentTime);
    XUngrabKeyboard(x11_display, CurrentTime);

    x11_get_focus = FALSE;
    x11_set_attribute_focus_out();

    XSync(x11_display, True);		/* �����٥���˴� */
}

#endif


/*======================================================================*/

static	Colormap	x11_colormap;		/* ������Υ��顼�ޥå�ID */
static	int		x11_cmap_type;		/* ���ߤΥ��顼�ޥå׽��� */

/* ������ο��ο����ϤǴ�������Τǡ����Τ�����ѿ����Ѱ� */
static	unsigned long	color_cell[256];	/* �ԥ������ͤ�����	*/
static	int		nr_color_cell_used;	/* ����λ��ѺѤ߰���	*/
static	int		sz_color_cell;		/* ����κ����		*/


static	int	create_colormap(int fullscreen)
{
    int i, j, max;
    unsigned long plane[1];	/* dummy */
    unsigned long pixel[256];

    if (verbose_proc) printf("  Colormap: ");

    x11_colormap = DefaultColormapOfScreen(x11_screen);

    sz_color_cell = 0;

    switch (colormap_type) {
    case 0:				/* ��ͭ���顼�������� */
	if (fullscreen == FALSE) {
	    if (verbose_proc) printf("shared ... ");
	    for (i = 0; i < 3; i++) {
		if      (i == 0) max = 144;	/* �ǽ�ϡ�144������     */
		else if (i == 1) max = 24;	/* ����ʤ� 24����       */
		else             max = 16;	/* ����ˤ� 16�����Ȼ */

		if (XAllocColorCells(x11_display, 
				     DefaultColormapOfScreen(x11_screen),
				     False, plane, 0, pixel, max)) {
		    /* ok */
		    nr_color_cell_used = 0;
		    sz_color_cell      = max;
		    for (j = 0; j < sz_color_cell; j++) {
			color_cell[j] = pixel[j];
		    }
		    break;
		}
	    }
	    if (sz_color_cell > 0) {
		if (verbose_proc) printf("OK (%d colors)\n", sz_color_cell);
		x11_cmap_type = 0;
		return sz_color_cell;
	    }
	    if (verbose_proc) printf("FAILED, ");
	}
	/* FALLTHROUGH */

    case 1:				/* �ץ饤�١��ȥ��顼�ޥåפ���� */
	if (x11_visual->class == PseudoColor) {
	    if (verbose_proc) printf("private ... ");

	    x11_colormap = XCreateColormap(x11_display, x11_window,
					   x11_visual, AllocAll);

	    if (fullscreen == FALSE) {
		XSetWindowColormap(x11_display, x11_window, x11_colormap);
	    }

	    /* ������ bpp �˰�¸�����ͤΤϤ����������� */
	    nr_color_cell_used = 0;
	    sz_color_cell      = 144;
	    for (j = 0; j < sz_color_cell; j++) {
		color_cell[j] = j;
	    }

	    if (verbose_proc) printf("OK\n");
	    x11_cmap_type = 1;
	    return sz_color_cell;
	}
	/* FALLTHROUGH */

    case 2:				/* ����ɬ�׻���ưŪ�˳��� */
	if (verbose_proc) printf("no color allocated\n");
#if 0
	if (x11_visual->class == PseudoColor ||
	    x11_visual->class == DirectColor) {
	    if        (x11_depth <= 4) {
		nr_color_cell_used = 0;
		sz_color_cell      = 16;
	    } else if (x11_depth <= 8) {
		nr_color_cell_used = 0;
		sz_color_cell      = 24;
	    } else {
		nr_color_cell_used = 0;
		sz_color_cell      = 144;
	    }
	} else
#endif
	{
	    nr_color_cell_used = 0;
	    sz_color_cell      = 144;
	}
	x11_cmap_type = 2;
	return sz_color_cell;
    }

    return 0;
}


static	int	reuse_colormap(void)
{
    switch (x11_cmap_type) {
    case 0:
	/* ����³����Ʊ����ͭ���顼�������� */
	break;

    case 1:
	/* ����³����Ʊ�����顼�ޥåפ���� */
	break;

    case 2:
	XFreeColors(x11_display, x11_colormap,
		    color_cell, nr_color_cell_used, 0);
	break;
    }

    nr_color_cell_used = 0;

    return sz_color_cell;
}


static	void	destroy_colormap(void)
{
    switch (x11_cmap_type) {
    case 0:
	XFreeColors(x11_display, DefaultColormapOfScreen(x11_screen),
		    color_cell, sz_color_cell, 0);
	break;

    case 1:
	XFreeColormap(x11_display, x11_colormap);
#if 0		/* DGA�ǥ��顼�ޥåפ򥻥åȤ�����硢ɬ���ǥե���Ȥ��᤹ ! */
	XSetWindowColormap(x11_display, x11_window,
			   DefaultColormapOfScreen(x11_screen));
#endif
	break;

    case 2:
	XFreeColors(x11_display, x11_colormap,
		    color_cell, nr_color_cell_used, 0);
	break;
    }

    sz_color_cell = 0;
}


/*======================================================================*/

static	XImage   *image;


static	void	*create_image(int width, int height)
{
    void *buf = NULL;

#ifdef MITSHM
    if (use_SHM) {			/* MIS-SHM ����������Ƥ뤫��Ƚ�� */
	int tmp;
	if (! XQueryExtension(x11_display, "MIT-SHM", &tmp, &tmp, &tmp)) {
	    if (verbose_proc) printf("  X-Server not support MIT-SHM\n");
	    use_SHM = FALSE;
	}
    }

    if (use_SHM) {

	if (verbose_proc) printf("  Using shared memory (MIT-SHM):\n"
				 "    CreateImage ... ");
	image = XShmCreateImage(x11_display, x11_visual, x11_depth,
				ZPixmap, NULL, &SHMInfo,
				width, height);

	if (image) {

	    if (verbose_proc) printf("GetInfo ... ");
	    SHMInfo.shmid = shmget(IPC_PRIVATE,
				   image->bytes_per_line * image->height,
				   IPC_CREAT | 0777);
	    if (SHMInfo.shmid < 0) {
		use_SHM = FALSE;
	    }

	    XSetErrorHandler(private_handler);	/* ���顼�ϥ�ɥ�򲣼�� */
						/* (XShmAttach()�۾︡��) */
	    if (use_SHM) {

		if (verbose_proc) printf("Allocate ... ");
		image->data =
		    SHMInfo.shmaddr = (char *)shmat(SHMInfo.shmid, 0, 0);
		if (image->data == NULL) {
		    use_SHM = FALSE;
		}

		if (use_SHM) {
		    if (verbose_proc) printf("Attach ... ");
		    SHMInfo.readOnly = False;

		    if (! XShmAttach(x11_display, &SHMInfo)) {
			use_SHM = FALSE;
		    }

		    XSync(x11_display, False);
		    /* sleep(2); */
		}
	    }

	    if (SHMInfo.shmid >= 0) shmctl(SHMInfo.shmid, IPC_RMID, 0);


	    if (use_SHM) {				/* ���٤����� */
		buf = image->data;
		if (verbose_proc) printf("OK\n");
	    } else {					/* �ɤä��Ǽ��� */
		if (verbose_proc) printf("FAILED(can't use shared memory)\n");
		if (SHMInfo.shmaddr) shmdt(SHMInfo.shmaddr);
		XDestroyImage(image);
		image = NULL;
	    }

	    XSetErrorHandler(None);		/* ���顼�ϥ�ɥ���᤹ */

	} else {
	    if (verbose_proc) printf("FAILED(can't use shared memory)\n");
	    use_SHM = FALSE;
	}
    }

    if (use_SHM == FALSE)
#endif
    {
	/* �����꡼��Хåե������ */

	if (verbose_proc) printf("  Screen buffer: Memory allocate ... ");
	buf = malloc(width * height * x11_byte_per_pixel);
	if (verbose_proc) { if (buf == NULL) printf("FAILED\n"); }

	if (buf) {
	    /* �����꡼��Хåե��򥤥᡼���˳������ */

	    if (verbose_proc) printf("CreateImage ... ");
	    image = XCreateImage(x11_display, x11_visual, x11_depth,
				 ZPixmap, 0, buf,
				 width, height, 8, 0);
	    if (verbose_proc) printf("%s\n", (image ? "OK" : "FAILED"));
	    if (image == NULL) {
		free(buf);
		buf = NULL;
	    }
	}
    }

    return buf;
}


static	void	destroy_image(void *buf)
{

#ifdef MITSHM
    if (use_SHM) {
	XShmDetach(x11_display, &SHMInfo);
	if (SHMInfo.shmaddr) shmdt(SHMInfo.shmaddr);
	/*if (SHMInfo.shmid >= 0) shmctl(SHMInfo.shmid,IPC_RMID,0);*/
    }
#endif

    if (image) {
	XDestroyImage(image);
	image = NULL;
    }

#if 0		/* buf �Ϥ⤦���פʤΤǡ������� free ���褦�Ȥ�������
		   image �Τۤ��Ǥޤ�������餷����free ����ȥ����Ƥ��ޤ���
		   �Ȥ������� XDestroyImage ���Ƥ�Τˡġġġ�
		   XSync �򤷤Ƥ������ͻҡ����㤢������ free ����� ?? */
#ifdef MITSHM
    if (use_SHM == FALSE)
#endif
    {
	free(buf);
    }
#endif
}


/************************************************************************
 *	���γ���
 *	���β���
 ************************************************************************/

static	void	x11_graph_add_color(const PC88_PALETTE_T color[],
				    int nr_color, unsigned long pixel[])
{
    

    int i;
    XColor xcolor[256];

    /* debug */
    if (nr_color_cell_used + nr_color > sz_color_cell) {
	/* �ɲä��٤�����¿���� */
	printf("color add err? %d %d\n", nr_color, nr_color_cell_used);
	return;
    }


    for (i = 0; i < nr_color; i++) {
	xcolor[i].red   = (unsigned short)color[i].red   << 8;
	xcolor[i].green = (unsigned short)color[i].green << 8;
	xcolor[i].blue  = (unsigned short)color[i].blue  << 8;
	xcolor[i].flags = DoRed | DoGreen | DoBlue;
    }


    switch (x11_cmap_type) {
    case 0:
    case 1:
	for (i = 0; i < nr_color; i++) {
	    pixel[i] = 
		xcolor[i].pixel = color_cell[ nr_color_cell_used ];
	    nr_color_cell_used ++;
	}

	XStoreColors(x11_display, x11_colormap, xcolor, nr_color);
#ifdef	USE_DGA
	if (graph_info.fullscreen) {
	    XF86DGAInstallColormap(x11_display, DefaultScreen(x11_display),
				   x11_colormap);
	}
#endif
	break;

    case 2:
	for (i = 0; i < nr_color; i++) {
	    if (XAllocColor(x11_display, x11_colormap, &xcolor[i])) {
		/* ���� */;	/* DO NOTHING */
	    } else {
		/* ���Ԥ����顢��������ݡ�����ϼ��Ԥ��ʤ������� */
		xcolor[i].red = xcolor[i].green = xcolor[i].blue = 0;
		XAllocColor(x11_display, x11_colormap, &xcolor[i]);
	    }
	    pixel[i] = 
		color_cell[ nr_color_cell_used ] = xcolor[i].pixel;
	    nr_color_cell_used ++;
	}
    }
}

/************************************************************************/

static	void	x11_graph_remove_color(int nr_pixel, unsigned long pixel[])
{

    /* debug */
    if (nr_pixel > nr_color_cell_used) {
	/* ������٤�����¿���� */
	printf("color remove err? %d %d\n", nr_pixel, nr_color_cell_used);
    } else {
	if (memcmp(&color_cell[ nr_color_cell_used - nr_pixel ], pixel,
		   sizeof(unsigned long) * nr_pixel) != 0) {
	    /* ������٤�������ľ�����ɲä������Ȱ㤦 */
	    printf("color remove unmatch???\n");
	}
    }


    switch (x11_cmap_type) {
    case 0:
    case 1:
	nr_color_cell_used -= nr_pixel;
	break;
    case 2:
	nr_color_cell_used -= nr_pixel;
	XFreeColors(x11_display, x11_colormap, pixel, nr_pixel, 0);
	break;
    }
}


/************************************************************************
 *	����ե��å��ι���
 ************************************************************************/

static	void	x11_graph_update(int nr_rect, T_GRAPH_RECT rect[])
{
    int i;

    if (graph_info.fullscreen == FALSE) {

	for (i = 0; i < nr_rect; i++) {
#ifdef MITSHM
	    if (use_SHM) {
		XShmPutImage(x11_display, x11_window, x11_gc, image,
			     rect[i].x, rect[i].y, 
			     rect[i].x, rect[i].y, 
			     rect[i].width, rect[i].height, False);
	    } else
#endif
	    {
		XPutImage(x11_display, x11_window, x11_gc, image,
			  rect[i].x, rect[i].y, 
			  rect[i].x, rect[i].y, 
			  rect[i].width, rect[i].height);
	    }
	}

	if (use_xsync) {
	    XSync(x11_display, False);
	} else {
	    XFlush(x11_display);
	}
    }
}
