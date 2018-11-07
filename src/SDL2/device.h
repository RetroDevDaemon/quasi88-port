#ifndef DEVICE_H_INCLUDED
#define DEVICE_H_INCLUDED


#include <SDL.h>


/*
 *	src/SDL/ �ʲ��ǤΥ����Х��ѿ�
 */
extern	int	sdl_mouse_rel_move;	/* �ޥ������а�ư�̸��β�ǽ��	*/



/*
 *	src/SDL/ �ʲ��ǤΥ����Х��ѿ� (���ץ���������ǽ���ѿ�)
 */
extern	int	use_hwsurface;		/* HW SURFACE ��Ȥ����ɤ���	*/
extern	int	use_doublebuf;		/* ���֥�Хåե���Ȥ����ɤ���	*/

extern	int	use_cmdkey;		/* Command�����ǥ�˥塼������     */
extern	int	keyboard_type;		/* �����ܡ��ɤμ���                */
extern	char	*file_keyboard;		/* ��������ե�����̾		   */
extern	int	use_joydevice;		/* ���祤���ƥ��å��ǥХ����򳫤�? */
extern	int	show_fps;		/* test */



int	sdl_init(void);
void	sdl_exit(void);

#endif	/* DEVICE_H_INCLUDED */
