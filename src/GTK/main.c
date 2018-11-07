/************************************************************************/
/*									*/
/*				QUASI88					*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "quasi88.h"
#include "device.h"

#include "getconf.h"	/* config_init */
#include "suspend.h"	/* stateload_system */
#include "menu.h"	/* menu_about_osd_msg */


/***********************************************************************
 * ���ץ����
 ************************************************************************/
static	const	T_CONFIG_TABLE gtksys_options[] =
{
  /* 300��349: �����ƥ��¸���ץ���� */

  /*  -- GRAPHIC -- */
  { 300, "gdkimage",     X_FIX,  &use_gdk_image,   TRUE,                  0,0, 0        },
  { 300, "nogdkimage",   X_FIX,  &use_gdk_image,   FALSE,                 0,0, 0        },


  /* ��ü */
  {   0, NULL,           X_INV,                                       0,0,0,0, 0        },
};

static	void	help_msg_gtksys(void)
{
  fprintf
  (
   stdout,
   "  ** GRAPHIC (GTK depend) **\n"
   "    -gdkimage/-nogdkimage   use GdkImage/use GdkRGB [-gdkimage]\n"
   );
}



/***********************************************************************
 * �ᥤ�����
 ************************************************************************/
static	void	gtksys_init(void);
static	void	gtksys_exit(void);
static	void	finish(void);

int	main(int argc, char **argv)
{
    gtk_set_locale();
#ifdef __CYGWIN__
    _Xsetlocale(LC_ALL, "ja_JP.EUC");	/* ??? */
#endif	// __CYGWIN__

    gtk_init(&argc, &argv);
    /* gtk_rc_parse("./gtkrc"); */

    if (config_init(argc, argv,		/* �Ķ������ & �������� */
		    gtksys_options,
		    help_msg_gtksys)) {

	quasi88_atexit(finish);		/* quasi88() �¹���˶�����λ�����ݤ�
					   ������Хå��ؿ�����Ͽ���� */

	gtksys_init();

	gtk_main();

	config_exit();			/* ������������� */
    }

    return 0;
}



/*
 * ������λ���Υ�����Хå��ؿ� (quasi88_exit()�ƽл��ˡ����������)
 */
static	void	finish(void)
{
    gtksys_exit();

    config_exit();			/* ������������� */
}



/*---------------------------------------------------------------------------*/
static	gint		idle_id = 0;
static	int		start_flg = FALSE;
static	gboolean	idle_callback(gpointer dummy);

static	void	gtksys_init(void)
{
    idle_id = g_idle_add(idle_callback, NULL);

    start_flg = TRUE;

    quasi88_start();
}

static	void	gtksys_exit(void)
{
    quasi88_stop(TRUE);

    if (idle_id) {
	g_source_remove(idle_id);
	idle_id = 0;
    }
    if (start_flg) {
	gtk_main_quit();
	start_flg = FALSE;
    }
}

static	gboolean idle_callback(gpointer dummy)
{
    switch (quasi88_loop()) {
    case QUASI88_LOOP_EXIT:		/* ��λ */
	gtksys_exit();
	/* exit(0); */				/* Motif�ʤ� �Ǹ�� exit() ? */
	return FALSE;

    case QUASI88_LOOP_ONE:		/* 1�ե졼��в� */
#if 0
    	{
	    static int i = 0, j=0;
	    if (++j > 55) {
		printf( "%d\n", i );
		i++;
		j=0;
	    }
	}
#endif
	return TRUE;

    case QUASI88_LOOP_BUSY:		/* ̵�� */
    default:
	return TRUE;
    }
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
