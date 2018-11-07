/************************************************************************/
/*									*/
/*				QUASI88					*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>	/* setuid, getuid */

#include "quasi88.h"
#include "device.h"

#include "getconf.h"	/* config_init */
#include "suspend.h"	/* stateload_system */
#include "menu.h"	/* menu_about_osd_msg */

#include "snapshot.h"	/* snapshot_cmd_enable */


/***********************************************************************
 * ���ץ����
 ************************************************************************/
static	int	invalid_arg;
static	const	T_CONFIG_TABLE x11_options[] =
{
  /* 300��349: �����ƥ��¸���ץ���� */

  /*  -- GRAPHIC -- */
  { 300, "cmap",         X_INT,  &colormap_type,   0, 2,                    0, 0        },
#ifdef MITSHM
  { 301, "shm",          X_FIX,  &use_SHM,         TRUE,                  0,0, 0        },
  { 301, "noshm",        X_FIX,  &use_SHM,         FALSE,                 0,0, 0        },
#else
  {   0, "shm",          X_INV,                                       0,0,0,0, 0        },
  {   0, "noshm",        X_INV,                                       0,0,0,0, 0        },
#endif
#ifdef USE_XV
  { 302, "xv",           X_FIX,  &use_xv,          TRUE,                  0,0, 0        },
  { 302, "noxv",         X_FIX,  &use_xv,          FALSE,                 0,0, 0        },
#else
  {   0, "xv",           X_INV,                                       0,0,0,0, 0        },
  {   0, "noxv",         X_INV,                                       0,0,0,0, 0        },
#endif
  { 303, "xdnd",         X_FIX,  &use_xdnd,        TRUE,                  0,0, 0        },
  { 303, "noxdnd",       X_FIX,  &use_xdnd,        FALSE,                 0,0, 0        },
  { 304, "xsync",        X_FIX,  &use_xsync,       TRUE,                  0,0, 0        },
  { 304, "noxsync",      X_FIX,  &use_xsync,       FALSE,                 0,0, 0        },

  /*  -- INPUT -- */
  { 311, "use_joy",      X_FIX,  &use_joydevice,   TRUE,                  0,0, 0        },
  { 311, "nouse_joy",    X_FIX,  &use_joydevice,   FALSE,                 0,0, 0        },
  { 312, "keyboard",     X_INT,  &keyboard_type,   0, 2,                    0, 0        },
  { 313, "keyconf",      X_STR,  &file_keyboard,                        0,0,0, 0        },

  /*  -- SYSTEM -- */
  { 321, "sleepparm",    X_INT,  &wait_sleep_min_us, 0, 1000000,            0, 0        },
  { 322, "show_fps",     X_FIX,  &show_fps,        TRUE,                  0,0, 0        },
  { 322, "hide_fps",     X_FIX,  &show_fps,        FALSE,                 0,0, 0        },


  /*  -- ̵�� -- (¾�����ƥ�ΰ����Ĥ����ץ����) */
  {   0, "videodrv",     X_INV,  &invalid_arg,                          0,0,0, 0        },
  {   0, "audiodrv",     X_INV,  &invalid_arg,                          0,0,0, 0        },

  {   0, "sdlbufsize",   X_INV,  &invalid_arg,                          0,0,0, 0        },
  {   0, "sdlbufnum",    X_INV,  &invalid_arg,                          0,0,0, 0        },


  /* ��ü */
  {   0, NULL,           X_INV,                                       0,0,0,0, 0        },
};

static	void	help_msg_x11(void)
{
  fprintf
  (
   stdout,
   "  ** GRAPHIC (X11 depend) **\n"
   "    -cmap <0/1/2>           Colormap type 0(shared)/1(private)/2(static)\n"
#ifdef	MITSHM
   "    -shm/-noshm             Use/Not use MIT SHM extensions [-shm]\n"
#endif
#ifdef	USE_DGA
   "    -dga/-nodga             Use/Not use XFree86-DGA (-fullscreen) [-nodga]\n"
#endif
#ifdef	USE_XV
   "    -xv/-noxv               Use/Not use XV Extension [-noxv]\n"
#endif
   "    -xdnd/-noxdnd           Enable/Disable X-Drag-And-Drop Protcol [-xdnd]\n"
   "    -xsync/-noxsync         Use/Not use XSync as screen refresh [-xsync]\n"
   "  ** INPUT (X11 depend) **\n"
   "    -use_joy/-nouse_joy     Enable/Disabel system joystick [-use_joy]\n"
   "    -keyboard <0|1|2>       Set keyboard type (0:config/1:106key/2:101key) [1]\n"
   "    -keyconf <filename>     Specify keyboard configuration file <filename>\n"
   "  ** SYSTEM (X11 depend) **\n"
   "    -show_fps/-hide_fps     Show/Hide FPS (experimentral)\n"
   );
}



/***********************************************************************
 * �ᥤ�����
 ************************************************************************/
static	void	finish(void);

int	main(int argc, char *argv[])
{
    int x = 1;

	/* root���¤�ɬ�פʽ��� (X11��Ϣ) �򿿤���˹Ԥ� */

    x11_init();	/* �����ǥ��顼���ФƤ⥪�ץ������Ϥ���Τ���˿ʤ� */


	/* �ǡ����줬�����С� root ���¤������ */

    if (setuid( getuid() ) != 0) {
	fprintf(stderr, "%s : setuid error\n", argv[0]);
	x11_exit();
	return -1;
    }

	/* root �� exec() �Ǥ���Τϴ��ʤΤǡ����Ȥ� */

    if (getuid() == 0) snapshot_cmd_enable = FALSE;


	/* ����ǥ�����ͥ������å� */

#ifdef LSB_FIRST
    if (*(char *)&x != 1) {
	fprintf(stderr,
		"%s CAN'T EXCUTE !\n"
		"This machine is Big-Endian.\n"
		"Compile again comment-out 'LSB_FIRST = 1' in Makefile.\n",
		argv[0]);
	x11_exit();
	return -1;
    }
#else
    if (*(char *)&x == 1) {
	fprintf(stderr,
		"%s CAN'T EXCUTE !\n"
		"This machine is Little-Endian.\n"
		"Compile again comment-in 'LSB_FIRST = 1' in Makefile.\n",
		argv[0]);
	x11_exit();
	return -1;
    }
#endif


    if (config_init(argc, argv,		/* �Ķ������ & �������� */
		    x11_options,
		    help_msg_x11)) {

	quasi88_atexit(finish);		/* quasi88() �¹���˶�����λ�����ݤ�
					   ������Хå��ؿ�����Ͽ���� */
	quasi88();			/* PC-8801 ���ߥ�졼����� */

	config_exit();			/* ������������� */
    }

    x11_exit();				/* X11��Ϣ����� */

    return 0;
}



/*
 * ������λ���Υ�����Хå��ؿ� (quasi88_exit()�ƽл��ˡ����������)
 */
static	void	finish(void)
{
    config_exit();			/* ������������� */
    x11_exit();				/* X11��Ϣ����� */
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
#ifdef	MITSHM
	"MIT-SHM ... Supported\n"
#endif

#ifdef	USE_DGA
	"XFree86-DGA ... Supported\n"
#endif

#ifdef	USE_XV
	"XVideo ... Supported\n"
#endif

#if	defined (JOY_SDL)
	"JOYSTICK (SDL) ... Supported\n"
#elif	defined (JOY_LINUX_USB)
	"JOYSTICK (Linux USB-joystick) ... Supported\n"
#elif	defined (JOY_BSD_USB)
	"JOYSTICK (BSD USB-joystick) ... Supported\n"
#else
	"JOYSTICK ... Not supported\n"
#endif  
    };

    static const char *about_jp =
    {
#ifdef	MITSHM
	"MIT-SHM �����ݡ��Ȥ���Ƥ��ޤ�\n"
#endif

#ifdef	USE_DGA
	"XFree86-DGA �����ݡ��Ȥ���Ƥ��ޤ�\n"
#endif

#ifdef	USE_XV
	"XVideo �����ݡ��Ȥ���Ƥ��ޤ�\n"
#endif

#if	defined (JOY_SDL)
	"���祤���ƥ��å� (SDL) �����ݡ��Ȥ���Ƥ��ޤ�\n"
#elif	defined (JOY_LINUX_USB)
	"���祤���ƥ��å� (Linux USB-joystick) �����ݡ��Ȥ���Ƥ��ޤ�\n"
#elif	defined (JOY_BSD_USB)
	"���祤���ƥ��å� (BSD USB-joystick) �����ݡ��Ȥ���Ƥ��ޤ�\n"
#else
	"���祤���ƥ��å� �ϥ��ݡ��Ȥ���Ƥ��ޤ���\n"
#endif  
    };


    *result_code = -1;				/* ʸ�������ɻ���ʤ� */

    if (req_japanese == FALSE) {
	*message = about_en;
    } else {
	*message = about_jp;
    }

    return TRUE;
}
