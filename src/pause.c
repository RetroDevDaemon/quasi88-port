/************************************************************************/
/*									*/
/* �ݡ��� (������) �⡼��						*/
/*									*/
/************************************************************************/

/*	�ѿ� pause_by_focus_out �ˤ��������Ѥ��			*/
/*	��pause_by_focus_out == 0 �λ�					*/
/*		ESC���������Ȳ����	��������� PAUSE��ɽ��		*/
/*	��pause_by_focus_out != 0 �λ�					*/
/*		X �Υޥ����������������Ȳ��				*/

#include <stdio.h>

#include "quasi88.h"
#include "pause.h"

#include "emu.h"
#include "initval.h"
#include "status.h"
#include "screen.h"
#include "wait.h"
#include "event.h"


int	need_focus = FALSE;			/* �ե���������������ߤ��� */


static	int	pause_by_focus_out = FALSE;

/*
 * ���ߥ������ˡ��ե���������̵���ʤä� (-focus������ϡ��ݡ�������)
 */
void	pause_event_focus_out_when_exec(void)
{
    if (need_focus) {				/* -focus ������� */
	pause_by_focus_out = TRUE;
	quasi88_pause();			/* ������ PAUSE ���� */
    }
}

/*
 * �ݡ�����ˡ��ե�������������
 */
void	pause_event_focus_in_when_pause(void)
{
    if (pause_by_focus_out) {
	quasi88_exec();
    }
}

/*
 * �ݡ�����ˡ��ݡ�����λ�Υ���(ESC����)�������Τ���
 */
void	pause_event_key_on_esc(void)
{
    quasi88_exec();
}

/*
 * �ݡ�����ˡ���˥塼���ϤΥ����������Τ���
 */
void	pause_event_key_on_menu(void)
{
    quasi88_menu();
}









void	pause_init(void)
{
    status_message_default(0, " PAUSE ");
    status_message_default(1, "<ESC> key to return");
    status_message_default(2, NULL);
}


void	pause_main(void)
{
    /* ��λ�ʤɤ��Τ��뤿��ˡ����٥�Ƚ��������»� */
    event_update();


    /* �����ߤ�ȴ�����顢����ƽ���� */
    if (quasi88_event_flags & EVENT_MODE_CHANGED) {

	pause_by_focus_out = FALSE;

    } else {

	quasi88_event_flags |= EVENT_FRAME_UPDATE;
    }
}
