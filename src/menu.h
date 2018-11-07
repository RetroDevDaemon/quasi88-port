#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

extern	int	menu_lang;			/* ��˥塼�θ���           */
extern	int	menu_readonly;			/* �ǥ������������������ */
						/* ������֤� ReadOnly ?    */
extern	int	menu_swapdrv;			/* �ɥ饤�֤�ɽ�����       */


extern	int	file_coding;			/* �ե�����̾�δ���������   */
extern	int	filename_synchronize;		/* �ե�����̾��ƱĴ������   */


	/* ��˥塼�⡼�� */

void	menu_init(void);
void	menu_main(void);



/***********************************************************************
 * ��˥塼���̤�ɽ�����롢�����ƥ��ͭ�Υ�å��������������ؿ�
 *
 * int	menu_about_osd_msg(int        req_japanese,
 *			   int        *result_code,
 *			   const char *message[])
 *
 *	��˥塼���̤ν�������˸ƤӽФ���롣
 *
 *	req_japanese �� ���ʤ顢���ܸ�Υ�å�����������
 *			���ʤ顢�Ѹ�(ASCII)��å������������׵᤹�롣
 *	
 *	result_code  �� ���ܸ�Υ�å������ξ�硢���������ɤ򥻥åȤ��롣
 *			EUC �ʤ� 1��SJIS �ʤ� 2������ʤ��ʤ顢-1��
 *			�Ѹ�(ASCII)��å������ʤ顢����Ǥ褤��
 *
 *	message      �� ��å�����ʸ����ؤΥݥ��󥿤򥻥åȤ��롣
 *
 *	�����       �� ��å������������硢�����֤����ʤ��ʤ顢�����֤���
 *			�����֤���硢 code, message ������Ǥ褤��
 ************************************************************************/
int	menu_about_osd_msg(int        req_japanese,
			   int        *result_code,
			   const char *message[]);


void	menu_sound_restart(int output);
/*----------------------------------------------------------------------
 * ���٥�Ƚ������н�
 *----------------------------------------------------------------------*/
void	q8tk_event_key_on(int code);
void	q8tk_event_key_off(int code);
void	q8tk_event_mouse_on(int code);
void	q8tk_event_mouse_off(int code);
void	q8tk_event_mouse_moved(int x, int y);
void	q8tk_event_quit(void);




#endif	/* MENU_H_INCLUDED */
