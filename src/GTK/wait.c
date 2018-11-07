/***********************************************************************
 * ��������Ĵ������ (�����ƥ��¸)
 *
 *	�ܺ٤ϡ� wait.h ����
 ************************************************************************/
#include <gtk/gtk.h>

#include "quasi88.h"
#include "initval.h"
#include "wait.h"



/*---------------------------------------------------------------------------*/
static	GTimer *timer_id = NULL;

static	int	wait_do_sleep;			/* idle���� sleep ����       */

static	int	wait_counter = 0;		/* Ϣ³������֥����С�������*/
static	int	wait_count_max = 10;		/* ����ʾ�Ϣ³�����С�������
						   ��ö,����Ĵ������������ */

/* �������Ȥ˻��Ѥ�����֤�����ɽ���ϡ� usñ�̤Ȥ��롣 (ms�������٤��㤤�Τ�) 

   GTK �λ�������ؿ� g_timer_elapsed() �ϡ���ư���������֤���
   ��������� us ���Ѵ����ƻ��Ѥ��뤳�Ȥˤ��롣
   long ������ 71ʬ�Ƿ夢�դ줷�Ƥ��ޤ��Τǡ�����ɽ���� long long ���ˤ��褦 */

#ifdef G_HAVE_GINT64
typedef	gint64		T_WAIT_TICK;
#else
typedef	gint32		T_WAIT_TICK;
#endif

static	T_WAIT_TICK	next_time;		/* ���ե졼��λ��� */
static	T_WAIT_TICK	delta_time;		/* 1 �ե졼��λ��� */



/* ---- ���߻����������� (usecñ��) ---- */

#define	GET_TICK() ((T_WAIT_TICK)(g_timer_elapsed(timer_id, NULL) * 1000000.0))





/****************************************************************************
 * ��������Ĵ�������ν��������λ
 *****************************************************************************/
int	wait_vsync_init(void)
{
    if (timer_id == NULL) {
	timer_id = g_timer_new();
    }

    return TRUE;
}

void	wait_vsync_exit(void)
{
    if (timer_id) {
	g_timer_destroy(timer_id);
	timer_id = NULL;
    }
}



/****************************************************************************
 * ��������Ĵ������������
 *****************************************************************************/
void	wait_vsync_setup(long vsync_cycle_us, int do_sleep)
{
    g_timer_start(timer_id);
    /* stop���ʤ��Ƥ⡢start����ȡ������ޡ��ϥꥻ�å� & ���������褦�� */

    wait_counter = 0;


    delta_time = (T_WAIT_TICK) vsync_cycle_us;		/* 1�ե졼����� */
    next_time  = GET_TICK() + delta_time;		/* ���ե졼����� */

    wait_do_sleep = do_sleep;				/* Sleep ̵ͭ */
}



/****************************************************************************
 * ��������Ĵ�������μ¹�
 *****************************************************************************/
int	wait_vsync_update(void)
{
    int on_time = FALSE;
    T_WAIT_TICK diff_ms;


    diff_ms = (next_time - GET_TICK()) / 1000;

    if (diff_ms > 0) {			/* �٤�Ƥʤ�(���֤�;�äƤ���)�ʤ� */
					/* diff_ms �ߥ��á��������Ȥ���     */

	if (wait_do_sleep) {		/* ���֤����ޤ� sleep ������ */

#if 0	    /* ��ˡ (1) */
	    g_usleep(diff_ms * 1000);		/* ���꡼�� */
#else	    /* ��ˡ (2) */
	    while (GET_TICK() <= next_time)	/* �ӥ����������� */
		;
#endif

	} else {			/* ���֤����ޤ�Tick��ƻ뤹���� */

	    while (GET_TICK() <= next_time)
		;				/* �ӥ����������� */
	}

	on_time = TRUE;
    }


    /* ���ե졼�����򻻽� */
    next_time += delta_time;


    if (on_time) {			/* ������˽����Ǥ��� */
	wait_counter = 0;
    } else {				/* ������˽����Ǥ��Ƥ��ʤ� */
	wait_counter ++;
	if (wait_counter >= wait_count_max) {	/* �٤줬�Ҥɤ����� */
	    wait_vsync_setup(delta_time,	/* �������Ȥ�����   */
			     wait_do_sleep);
	}
    }

#if 0
    {
	static int x = 0, y = 0;
	if (++x == 55) {
	    y++;
	    x = 0;
	    printf("wait %d\n", y);
	    fflush(stdout);
	}
    }
#endif

    if (on_time) return WAIT_JUST;
    else         return WAIT_OVER;
}
