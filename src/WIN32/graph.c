/***********************************************************************
 * ����ե��å����� (�����ƥ��¸)
 *
 *	�ܺ٤ϡ� graph.h ����
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "quasi88.h"
#include "graph.h"
#include "device.h"



HINSTANCE	g_hInstance;
HWND		g_hWnd;
HMENU		g_hMenu;
int		g_keyrepeat;



static	T_GRAPH_SPEC	graph_spec;		/* ���ܾ���		*/

static	int		graph_exist;		/* ���ǡ����������Ѥ�	*/
static	T_GRAPH_INFO	graph_info;		/* ���λ��Ρ����̾���	*/


/************************************************************************
 *	����ե��å������ν����
 *	����ե��å�������ư��
 *	����ե��å������ν�λ
 ************************************************************************/

const T_GRAPH_SPEC	*graph_init(void)
{
    if (verbose_proc) {
	printf("Initializing Graphic System ... ");
    }

    graph_spec.window_max_width      = 640;
    graph_spec.window_max_height     = 480;
    graph_spec.fullscreen_max_width  = 0;
    graph_spec.fullscreen_max_height = 0;
    graph_spec.forbid_status         = FALSE;
    graph_spec.forbid_half           = FALSE;

    if (verbose_proc)
	printf("OK\n");

    return &graph_spec;
}

/************************************************************************/

static DWORD winStyle;
static int create_window(int width, int height);
static void calc_window_size(int *width, int *height);

static unsigned char *buffer = NULL;
static BITMAPINFO bmpInfo;

const T_GRAPH_INFO	*graph_setup(int width, int height,
				     int fullscreen, double aspect)
{
    int win_width, win_height;

    /* fullscreen, aspect ��̤���� */

    /* ���ե����꡼��Хåե�����ݤ��� */

    if (buffer) {
	free(buffer);
    }

    buffer = malloc(width * height * sizeof(unsigned long));
    if (buffer == FALSE) {
	return NULL;
    }

    memset(&bmpInfo, 0, sizeof(bmpInfo));

    bmpInfo.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmpInfo.bmiHeader.biWidth       =   width;
    bmpInfo.bmiHeader.biHeight      = - height;
    bmpInfo.bmiHeader.biPlanes      = 1;
    bmpInfo.bmiHeader.biBitCount    = 32;
    bmpInfo.bmiHeader.biCompression = BI_RGB;


    /* ������ɥ����������ʤ����ꥵ���� */

    if (graph_exist == FALSE) {		/* ������ɥ���̵��������� */

	if (create_window(width, height) == FALSE) {
	    free(buffer);
	    buffer = NULL;
	    return NULL;
	}

    } else {				/* ������ɥ���ͭ��Хꥵ���� */

	win_width  = width;
	win_height = height;
	calc_window_size(&win_width, &win_height);
	SetWindowPos(g_hWnd,
		     HWND_TOP, 0, 0,		/* ���ߡ�(̵�뤵���)	*/
		     win_width, win_height,	/* ������ɥ��������⤵	*/
		     SWP_NOMOVE | SWP_NOZORDER);
    }

    /* graph_info �˽����򥻥åȤ��� */

    graph_info.fullscreen	= FALSE;
    graph_info.width		= width;
    graph_info.height		= height;
    graph_info.byte_per_pixel	= 4;
    graph_info.byte_per_line	= width * 4;
    graph_info.buffer		= buffer;
    graph_info.nr_color		= 255;
    graph_info.write_only	= FALSE;
    graph_info.broken_mouse	= FALSE;
    graph_info.draw_start	= NULL;
    graph_info.draw_finish	= NULL;
    graph_info.dont_frameskip	= FALSE;

    graph_exist = TRUE;

    return &graph_info;
}



/*
 * ������ɥ�����������
 */
static int create_window(int width, int height)
{
    WNDCLASSEX wc;
    int win_width, win_height;

    /* ������ɥ����饹�ξ�������� */
    wc.cbSize = sizeof(wc);			/* ��¤�Υ����� */
    wc.style = 0;				/* ������ɥ��������� */
    wc.lpfnWndProc = WndProc;			/* ������ɥ��ץ������� */
    wc.cbClsExtra = 0;				/* ��ĥ���� */
    wc.cbWndExtra = 0;				/* ��ĥ���� */
    wc.hInstance = g_hInstance;			/* ���󥹥��󥹥ϥ�ɥ� */
    wc.hIcon = NULL;				/* �������� */
/*
    wc.hIcon = (HICON)LoadImage(NULL, MAKEINTRESOURCE(IDI_APPLICATION),
				IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
*/
    wc.hIconSm = wc.hIcon;			/* �������������� */
    wc.hCursor = NULL;				/* �ޥ����������� */
/*
    wc.hCursor = (HCURSOR)LoadImage(NULL, MAKEINTRESOURCE(IDC_ARROW),
				    IMAGE_CURSOR, 0, 0,
				    LR_DEFAULTSIZE | LR_SHARED);
*/
						/* ������ɥ��ط� */
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
/*  wc.lpszMenuName = NULL;*/			/* ��˥塼̾ */
    wc.lpszMenuName = "QUASI88";		/* ��˥塼̾ �� quasi88.rc */
    wc.lpszClassName = "Win32App";		/* ������ɥ����饹̾ Ŭ�� */

    /* ������ɥ����饹����Ͽ���� */
    if (RegisterClassEx(&wc) == 0) { return FALSE; }

    /* ������ɥ���������Ϥ��� */
    winStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

    /* ������ɥ��������η׻� */
    win_width  = width;
    win_height = height;
    calc_window_size(&win_width, &win_height);

    /* ������ɥ���������� */
    g_hWnd = CreateWindowEx(WS_EX_ACCEPTFILES,	/* ��ĥ������ɥ��������� */
			    wc.lpszClassName,	/* ������ɥ����饹̾	  */
			    "QUASI88 win32",	/* �����ȥ�С�ʸ����	  */
			    winStyle,		/* ������ɥ���������	  */
			    CW_USEDEFAULT,	/* ������ɥ���x��ɸ	  */
			    CW_USEDEFAULT,	/* ������ɥ���y��ɸ	  */
			    win_width,		/* ������ɥ�����	  */
			    win_height,		/* ������ɥ��ι⤵	  */
			    NULL,		/* �ƥ�����ɥ��Υϥ�ɥ� */
			    NULL,		/* ��˥塼�ϥ�ɥ�	  */
			    g_hInstance,	/* ���󥹥��󥹥ϥ�ɥ�	  */
			    NULL);		/* �ղþ���		  */

    if (g_hWnd == NULL) { return FALSE; }

    /* ������ɥ���ɽ������ */
    ShowWindow(g_hWnd, SW_SHOW);
    UpdateWindow(g_hWnd);

    /* ��˥塼�ϥ�ɥ� */
    g_hMenu = GetMenu(g_hWnd);
    
    /* Drug & Drop �ε��� */
#if 0
    /* ������ɥ��κ������ˡ� WS_EX_ACCEPTFILES ��Ĥ��Ƥ���Τǡ���������� */
    DragAcceptFiles(g_hWnd, TRUE);
#endif


#if 0
    if (verbose_proc) {	/* �ǥ����ץ쥤���� */
	HDC hdc;
	hdc = GetDC(g_hWnd);
	fprintf(debugfp, "Info: H-pixel %d\n", GetDeviceCaps(hdc, HORZRES));
	fprintf(debugfp, "Info: V-pixel %d\n", GetDeviceCaps(hdc, VERTRES));
	fprintf(debugfp, "Info: Depth   %d\n", GetDeviceCaps(hdc, BITSPIXEL));
	ReleaseDC(g_hWnd, hdc);
    }
#endif
    return TRUE;
}



/*
 * �����Υ�����ɥ���������׻�����
 */
static void calc_window_size(int *width, int *height)
{
    RECT rect;

    rect.left = 0;	rect.right  = *width;
    rect.top  = 0;	rect.bottom = *height;

    AdjustWindowRectEx(&rect,			/* ���饤����ȶ��       */
		       winStyle,		/* ������ɥ���������     */
		       TRUE,			/* ��˥塼�ե饰         */
		       0);			/* ��ĥ������ɥ��������� */

    *width  = rect.right - rect.left;		/* �����Υ�����ɥ�����   */
    *height = rect.bottom - rect.top;		/* �����Υ�����ɥ��ι⤵ */
}



/************************************************************************/

void	graph_exit(void)
{
    if (buffer) {
	free(buffer);
    }
}

/************************************************************************
 *	���γ���
 *	���β���
 ************************************************************************/

void	graph_add_color(const PC88_PALETTE_T color[],
			int nr_color, unsigned long pixel[])
{
    int i;
    for (i=0; i<nr_color; i++) {

	pixel[i] = ((((unsigned long) color[i].red)   << 16) |
	    	    (((unsigned long) color[i].green) <<  8) |
	    	    (((unsigned long) color[i].blue)));

	/* RGB()�ޥ���ϡ������ȿž���Ƥ���ΤǻȤ��ʤ� */
    }
}

/************************************************************************/

void	graph_remove_color(int nr_pixel, unsigned long pixel[])
{
    /* ���˴ؤ��Ƥϲ���������ʤ��Τǡ������Ǥ�ʤˤ⤷�ʤ� */
}

/************************************************************************
 *	����ե��å��ι���
 ************************************************************************/

static	int	graph_update_counter = 0;

int	graph_update_WM_PAINT(void)
{
    int drawn;
    HDC hdc;
    PAINTSTRUCT ps;

    hdc = BeginPaint(g_hWnd, &ps);

    /* graph_update() �ˤ�ꡢ WM_PAINT ���٥�Ȥ�ȯ��������硢���褹�롣
       OS �������ȯ�������� WM_PAINT ���٥�Ȥξ��ϡ��ʤˤ⤷�ʤ���
       (quasi88_expose() �ν����ˤ�ꡢ graph_update() ���ƤӽФ���뤿��) */

    if (graph_update_counter > 0) {
#if 1	/* �ɤ���� API �Ǥ�褵����®�٤ϡ� */
	StretchDIBits(hdc,
		      0, 0, graph_info.width, graph_info.height,
		      0, 0, graph_info.width, graph_info.height,
		      buffer, &bmpInfo, DIB_RGB_COLORS, SRCCOPY);
#else	/* ���ä��ϡ�ž����ι⤵��������Ǥ��ʤ� */
	SetDIBitsToDevice(hdc,
			  0, 0, graph_info.width, graph_info.height,
			  0, 0, 0, graph_info.height,
			  buffer, &bmpInfo, DIB_RGB_COLORS);
#endif
	graph_update_counter = 0;
	drawn = TRUE;
    } else {
	drawn = FALSE;
    }

/*
    fprintf(debugfp,
	    "%s %d:(%3d,%3d)-(%3d,%3d)\n",
	    (drawn) ? "update" : "EXPOSE", graph_update_counter, 
	    ps.rcPaint.left,  ps.rcPaint.top,
	    ps.rcPaint.right, ps.rcPaint.bottom);
*/
    EndPaint(g_hWnd, &ps);

    return drawn;
}

void	graph_update(int nr_rect, T_GRAPH_RECT rect[])
{
    graph_update_counter = 1;

    InvalidateRect(g_hWnd, NULL, FALSE);
    UpdateWindow(g_hWnd);

    /* �����ǡ�ľ�ܥ�����ɥ�����򤷤褦�Ȥ����Τ������ʤ󤫤��ޤ������ʤ���
       WndProc() �������Ǥ�����������ɥ�����ϤǤ��ʤ��Τ��ʡ�

       �Ȥꤢ������ InvalidateRect() �򤹤�ȡ� WM_PAINT ���٥�Ȥ�ȯ������
       �Τǡ����θ�� WndProc() �� WM_PAINT �����ˤ����褵���뤳�Ȥˤ��褦��

       ���ʤߤˡ� InvalidateRect() ��ľ��ˡ� UpdateWindow() ��ƤӽФ��ȡ�
       ���δؿ��������� WndProc() ���ƤӽФ��졢 WM_PAINT �ν������Ԥ���
       �餷�����Ĥޤ� UpdateWindow() ������ä�����������Ͻ���äƤ��롣

       ���褳���� nr_rect �󡢽����򷫤��֤��褦�ˤ��٤��ʤΤ��������ݤʤΤ�
       �����̤�1��������褵���Ƥ��롣(®���ޥ���ʤ鵤�ˤʤ�ʤ� ^^;) */
}


/************************************************************************
 *	�����ȥ������
 *	°��������
 ************************************************************************/

void	graph_set_window_title(const char *title)
{
}

/************************************************************************/

void	graph_set_attribute(int mouse_show, int grab, int keyrepeat_on)
{
    g_keyrepeat = keyrepeat_on;

    if (mouse_show) ShowCursor(TRUE);
    else            ShowCursor(FALSE);
}
