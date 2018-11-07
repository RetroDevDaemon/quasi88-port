/************************************************************************/
/*									*/
/*				QUASI88					*/
/*									*/
/************************************************************************/

/*
 * ImmDisableIME() �ϡ� WINVER >= 0x040A �����API�餷����(Win98/2K�ʹ�)
 * VC6 �Ǥϡ��ʤˤ⤷�ʤ��� WINVER == 0x0400 �ȤʤäƤ��ޤ��Τǡ�
 * windows.h �򥤥󥯥롼�ɤ������ˡ� WINVER ����Ū�˻��ꤷ�Ƥ��ޤ���
 */
#ifdef	_MSC_VER
#ifndef WINVER
#define WINVER 0x040A
#endif
#if WINVER < 0x040A
#undef WINVER
#define WINVER 0x040A
#endif
#endif


#include <stdio.h>
#include <stdlib.h>

#include "quasi88.h"
#include "device.h"

#include "getconf.h"	/* config_init */
#include "keyboard.h"	/* romaji_type */
#include "suspend.h"	/* stateload_system */
#include "menu.h"	/* menu_about_osd_msg */

FILE	*debugfp;
/***********************************************************************
 * �ᥤ�����
 ************************************************************************/
static	void	finish(void);

int WINAPI WinMain(HINSTANCE hInst,
		   HINSTANCE hPrevInst,
		   LPSTR pCmdLine,
		   int showCmd)
{
    g_hInstance = hInst;


#ifndef	NDEBUG
    /* ���󥽡�����������ɸ����Ϥ������Ƥ� */
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
#endif
/*
    debugfp = fopen("debug.txt", "w");
*/
    if (debugfp == NULL) { debugfp = stdout; }



#if 1
    /* IME ��̵���ˤ��Ƥ��� (imm.h, imm32.lib)		*/
    /* ������ɥ��������˸ƤӽФ�ɬ�פ�����餷��	*/
#if (WINVER >= 0x040A)
    ImmDisableIME(0);
#endif
#endif

    /* �����ν���ͤ���� (����������Ϥʤ����ʡ�) */
    romaji_type = 1;			/* ���޻��Ѵ��ε�§�� MS-IME���� */


    if (config_init(0, NULL,		/* �Ķ������ & �������� */
		    NULL,
		    NULL)) {

	quasi88_atexit(finish);		/* quasi88() �¹���˶�����λ�����ݤ�
					   ������Хå��ؿ�����Ͽ���� */
	quasi88();			/* PC-8801 ���ߥ�졼����� */

	config_exit();			/* ������������� */
    }

    return 0;
}



/*
 * ������λ���Υ�����Хå��ؿ� (quasi88_exit()�ƽл��ˡ����������)
 */
static	void	finish(void)
{
    config_exit();			/* ������������� */
}



/***********************************************************************
 * ���ơ��ȥ��ɡ����ơ��ȥ�����
 ************************************************************************/

/*	¾�ξ��󤹤٤Ƥ����� or �����֤��줿��˸ƤӽФ���롣
 *	ɬ�פ˱����ơ������ƥ��ͭ�ξ�����ղä��Ƥ⤤�����ȡ�
 */

int	stateload_system(void)
{
    return TRUE;
}
int	statesave_system(void)
{
    return TRUE;
}



/***********************************************************************
 * ��˥塼���̤�ɽ�����롢�����ƥ��ͭ��å�����
 ************************************************************************/

int	menu_about_osd_msg(int        req_japanese,
			   int        *result_code,
			   const char *message[])
{
    static const char *about_en =
    {
	"Fullscreen mode not supported.\n"
	"Joystick not supported.\n"
    };

    static const char *about_jp =
    {
	"�ե륹���꡼��ɽ���ϥ��ݡ��Ȥ���Ƥ��ޤ���\n"
	"���祤���ƥ��å��ϥ��ݡ��Ȥ���Ƥ��ޤ���\n"
	"�ޥ������������ɽ������ϥ��ݡ��Ȥ���Ƥ��ޤ���\n"
	"��������ե�������ɤ߹��ߤϥ��ݡ��Ȥ���Ƥ��ޤ���\n"
    };


    *result_code = -1;				/* ʸ�������ɻ���ʤ� */

    if (req_japanese == FALSE) {
	*message = about_en;
    } else {
	*message = about_jp;
    }

    return TRUE;
}
