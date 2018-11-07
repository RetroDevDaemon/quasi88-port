#ifndef DEVICE_H_INCLUDED
#define DEVICE_H_INCLUDED


#include <X11/Xlib.h>
#include <X11/Xutil.h>


/*
 *	src/X11/ �ʲ��ǤΥ����Х��ѿ�
 */
extern	Display	*x11_display;
extern	Window	x11_window;
extern	Atom	x11_atom_kill_type;
extern	Atom	x11_atom_kill_data;

extern	int	x11_width;		/* ������ɥ�(������)�Υ�����	*/
extern	int	x11_height;

extern	int	x11_mouse_rel_move;	/* �ޥ������а�ư�̸��Τ����뤫	*/
extern	int	x11_get_focus;		/* ���ߡ��ե����������꤫�ɤ���	*/

extern	int	x11_scaling;		/* �ޥ�����ɸ�Υ��������̵ͭ	*/
extern	int	x11_scale_x_num;
extern	int	x11_scale_x_den;
extern	int	x11_scale_y_num;
extern	int	x11_scale_y_den;



/*
 *	src/X11/ �ʲ��ǤΥ����Х��ѿ� (���ץ���������ǽ���ѿ�)
 */
extern	int	colormap_type;		/* ���顼�ޥåפΥ�����	0/1/2	*/
extern	int	use_xsync;		/* XSync ����Ѥ��뤫�ɤ���	*/
extern	int	use_xv;			/* Xv ����Ѥ��뤫�ɤ���	*/
#ifdef MITSHM
extern	int	use_SHM;		/* MIT-SHM ����Ѥ��뤫�ɤ���	*/
#endif

extern	int	keyboard_type;		/* �����ܡ��ɤμ���                */
extern	char	*file_keyboard;		/* ��������ե�����̾		   */
extern	int	use_xdnd;		/* XDnD ���б����뤫�ɤ���	   */
extern	int	use_joydevice;		/* ���祤���ƥ��å��ǥХ����򳫤�? */

extern	int	wait_sleep_min_us;	/* �Ĥ� idle���֤����� us�ʲ���
					   ���ϡ� sleep �������Ԥġ�
					   (MAX 1�� = 1,000,000us) */

extern	int	show_fps;		/* test */



void	x11_init(void);
void	x11_exit(void);

void	x11_set_attribute_focus_in(void);
void	x11_set_attribute_focus_out(void);

void	xdnd_initialize(void);
void	xdnd_start(void);


#endif	/* DEVICE_H_INCLUDED */
