/************************************************************************/
/*									*/
/*				QUASI88					*/
/*									*/
/************************************************************************/

/*----------------------------------------------------------------------*
 * Classic�С������Υ����������ɤ�����ʬ�ϡ�                          *
 * Koichi NISHIDA ��� Classic iP6 PC-6001/mk2/6601 emulator �Υ������� *
 * ���ͤˤ����Ƥ��������ޤ�����                                         *
 *                                                   (c) Koichi NISHIDA *
 *----------------------------------------------------------------------*/

#include "quasi88.h"
#include "device.h"

#include "getconf.h"	/* config_init */
#include "keyboard.h"	/* romaji_type */
#include "suspend.h"	/* stateload_system */
#include "menu.h"	/* menu_about_osd_msg */

#include "intr.h"
#include "screen.h"


/***********************************************************************
 * ���ץ����
 ************************************************************************/
static	int	invalid_arg;
static	const	T_CONFIG_TABLE classic_options[] =
{
  /* 300��349: �����ƥ��¸���ץ���� */

  /*  -- GRAPHIC -- */
  { 300, "8bpp",         X_FIX,  &mac_8bpp,        TRUE,                  0,0, 0        },
  { 300, "15bpp",        X_FIX,  &mac_8bpp,        FALSE,                 0,0, 0        },
  { 300, "16bpp",        X_FIX,  &mac_8bpp,        FALSE,                 0,0, 0        },

  /*  -- ̵�� -- (¾�����ƥ�ΰ����Ĥ����ץ����) */
  {   0, "cmap",         X_INV,  &invalid_arg,                          0,0,0, 0        },
  {   0, "keyboard",     X_INV,  &invalid_arg,                          0,0,0, 0        },
  {   0, "keyconf",      X_INV,  &invalid_arg,                          0,0,0, 0        },
  {   0, "sleepparm",    X_INT,  &invalid_arg,                          0,0,0, 0        },
  {   0, "videodrv",     X_INV,  &invalid_arg,                          0,0,0, 0        },
  {   0, "audiodrv",     X_INV,  &invalid_arg,                          0,0,0, 0        },

  /* ��ü */
  {   0, NULL,           X_INV,                                       0,0,0,0, 0        },
};



/***********************************************************************
 * �ᥤ�����
 ************************************************************************/
static	void	finish(void);

int	main(void)
{
    /* �����Ϣ������ */

    /* ��ĥ���륹���å��Υ���������� ��       �ɤ�������ݤ��٤��ʤΡ�	*/
    SetApplLimit(GetApplLimit() - 65536*2);	/* �����å����������ĥ	*/
    MaxApplZone();				/* �ҡ����ΰ���ĥ	*/
    MoreMasters();				/* �ʤ�Τ��ޤ��ʤ���	*/



    /* �����ν���ͤ���� (����������Ϥʤ����ʡ�) */
    romaji_type = 1;			/* ���޻��Ѵ��ε�§�� MS-IME���� */


    if (config_init(0, NULL,		/* �Ķ������ & �������� */
		    classic_options,
		    NULL)) {

	mac_init();			/* CLASSIC��Ϣ�ν���� */

	{   /* CLASSIC�Ķ��˺�Ŭ(��) �������ͤ˽񤭴����Ƥ��� */
	    vsync_freq_hz  = 60.0;  /* ���ַв�η�¬��1/60��ñ�̸���ʤΤ� */
	    file_coding = 1;        /* SJIS���� */
	}

	quasi88_atexit(finish);		/* quasi88() �¹���˶�����λ�����ݤ�
					   ������Хå��ؿ�����Ͽ���� */
	quasi88();			/* PC-8801 ���ߥ�졼����� */

	mac_exit();			/* CLASSIC��Ϣ����� */

	config_exit();			/* ������������� */
    }

    return 0;
}



/*
 * ������λ���Υ�����Хå��ؿ� (quasi88_exit()�ƽл��ˡ����������)
 */
static	void	finish(void)
{
    mac_exit();				/* CLASSIC��Ϣ����� */
    config_exit();			/* ������������� */
}








/***********************************************************************
 * ���ơ��ȥ��ɡ����ơ��ȥ�����
 ************************************************************************/

/*	¾�ξ��󤹤٤Ƥ����� or �����֤��줿��˸ƤӽФ���롣
 *	ɬ�פ˱����ơ������ƥ��ͭ�ξ�����ղä��Ƥ⤤�����ȡ�
 */

int	stateload_system( void )
{
  return TRUE;
}
int	statesave_system( void )
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
	"Mouse and joystick are not supported.\n"
	"\n"
	"Many many menu items are not available.\n"
    };

    static const char *about_jp =
    {
	"®�٤˴ؤ���������ѹ��Ǥ��ޤ���\n"
	"�ޥ��������祤���ƥ��å��ϻ��ѤǤ��ޤ���\n"
	"�ޥ������������ɽ������ϥ��ݡ��Ȥ���Ƥ��ޤ���\n"
	"���եȥ����� NumLock �ϥ��ݡ��Ȥ���Ƥ��ޤ���\n"
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
