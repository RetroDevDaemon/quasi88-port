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

static unsigned char *buffer = NULL;

const T_GRAPH_INFO	*graph_setup(int width, int height,
				     int fullscreen, double aspect)
{
    if (graph_exist == FALSE) {

	/* �ޤ�������ɥ���̵���ʤ�С�������ɥ����������� */

	if (buffer == NULL) {
	    buffer = malloc(width * height * sizeof(unsigned short));
	    if (buffer == FALSE) {
		return NULL;
	    }
	}
    } else {

	/* ���Ǥ˥�����ɥ���ͭ��ʤ�С��ꥵ�������� */
    }

    /* graph_info �˽����򥻥åȤ��� */

    graph_info.fullscreen	= FALSE;
    graph_info.width		= width;
    graph_info.height		= height;
    graph_info.byte_per_pixel	= 2;
    graph_info.byte_per_line	= width * 2;
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
	pixel[i] = ((((unsigned long) color[i].red   >> 3) << 10) |
	    	    (((unsigned long) color[i].green >> 3) <<  5) |
	    	    (((unsigned long) color[i].blue  >> 3)));
    }
}

/************************************************************************/

void	graph_remove_color(int nr_pixel, unsigned long pixel[])
{
}

/************************************************************************
 *	����ե��å��ι���
 ************************************************************************/

void	graph_update(int nr_rect, T_GRAPH_RECT rect[])
{
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
}
