/***********************************************************************
 * ��������Ĵ������ (�����ƥ��¸)
 *
 *	�ܺ٤ϡ� wait.h ����
 ************************************************************************/

/* select, usleep, nanosleep �Τ����줫�Υ����ƥॳ�������Ѥ���Τǡ�
   �ʲ��Τɤ줫��Ĥ�Ĥ��ơ�¾�ϥ����ȥ����Ȥ��� */

#define USE_SELECT
/* #define USE_USLEEP */
/* #define USE_NANOSLEEP */


#include <stdio.h>

#include <sys/types.h>		/* select                        */
#include <sys/time.h>		/* select           gettimeofday */
#include <unistd.h>		/* select usleep                 */
#include <time.h>		/*        nanosleep clock        */

#include "quasi88.h"
#include "initval.h"
#include "wait.h"
#include "suspend.h"
#include "event.h"		/* quasi88_is_exec		*/

#include "intr.h"		/* test */
#include "screen.h"		/* test */
#include "graph.h"		/* test */


/*---------------------------------------------------------------------------*/
static	int	wait_do_sleep;			/* idle���� sleep ����       */
	int	wait_sleep_min_us = 100;	/* �Ĥ� idle���֤����� us�ʲ���
						   ���ϡ� sleep �������Ԥġ�
						   (MAX 1�� = 1,000,000us) */

static	int	wait_counter = 0;		/* Ϣ³������֥����С�������*/
static	int	wait_count_max = 10;		/* ����ʾ�Ϣ³�����С�������
						   ��ö,����Ĵ������������ */

	int	show_fps;			/* test */
static	void	display_fps(void);		/* test */

/* �������Ȥ˻��Ѥ�����֤�����ɽ���ϡ� usñ�̤Ȥ��롣 (ms�������٤��㤤�Τ�) 

   ��������ؿ� (gettimeofday() �ʤ�) �Ǽ������������ us ���Ѵ����ơ�
   long long �����ݻ����뤳�Ȥˤ��褦�� */

#ifdef	HAVE_LONG_LONG
typedef	long long	T_WAIT_TICK;
#else
typedef	long		T_WAIT_TICK;
#endif

static	T_WAIT_TICK	next_time;		/* ���ե졼��λ��� */
static	T_WAIT_TICK	delta_time;		/* 1 �ե졼��λ��� */

static	T_WAIT_TICK	sleep_min_time = 100;	/* sleep ��ǽ�ʺǾ����� */



/* ---- ���ꤵ�줿���� (usecñ��) sleep ���� ---- */

INLINE	void	delay_usec(unsigned int usec)
{
#if	defined(USE_SELECT)		/* select ��Ȥ� */

    struct timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = usec;
    select(0, NULL, NULL, NULL, &tv);

#elif	defined(USE_USLEEP)		/* usleep ��Ȥ� */

    usleep(usec);

#elif	defined(USE_NANOSLEEP)		/* nanosleep ��Ȥ� */

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = usec * 1000;
    nanosleep(&ts, NULL);

#else					/* �ɤ��Ȥ��ʤ� ! */
    wait_do_sleep = FALSE; /* X_X; */
#endif
}



/* ---- ���߻����������� (usecñ��) ---- */
static int tick_error = FALSE;


#ifdef  HAVE_GETTIMEOFDAY		/* gettimeofday() ��Ȥ� */

static struct timeval start_tv;

INLINE	void		set_tick(void)
{
    if (gettimeofday(&start_tv, 0)) {
	if (verbose_wait) printf("Clock Error\n");
	tick_error = TRUE;
	start_tv.tv_sec  = 0;
	start_tv.tv_usec = 0;
    }
}

INLINE	T_WAIT_TICK	get_tick(void)
{
    struct timeval tv;

    if (gettimeofday(&tv, 0)) {
	if (verbose_wait) { if (tick_error == FALSE) printf("Clock Error\n"); }
	tick_error = TRUE;
	tv.tv_sec  = 1;
	tv.tv_usec = 1;
    }

 #if 1
    return ((T_WAIT_TICK) (tv.tv_sec  - start_tv.tv_sec) * 1000000 +
	    (T_WAIT_TICK) (tv.tv_usec - start_tv.tv_usec));
 #else
    tv.tv_sec  -= start_tv.tv_sec;
    tv.tv_usec -= start_tv.tv_usec;
    if (tv.tv_usec < 0) {
	--tv.tv_sec;
	tv.tv_usec += 1000000;
    }
    return ((T_WAIT_TICK) tv.tv_sec * 1000000 + (T_WAIT_TICK) tv.tv_usec);
 #endif
}

#else					/* clock() ��Ȥ� */

/* #define CLOCK_SLICE	CLK_TCK */		/* ���줸�����ܡ� */
#define	CLOCK_SLICE	CLOCKS_PER_SEC		/* ���ä������� */

INLINE	void		set_tick(void)
{
}

INLINE	T_WAIT_TICK	get_tick(void)
{
    clock_t t = clock();
    if (t == (clock_t)-1) {
	if (verbose_wait) { if (tick_error == FALSE) printf("Clock Error\n"); }
	tick_error = TRUE;
	t = CLOCK_SLICE;
    }

    return ((T_WAIT_TICK) (t / CLOCK_SLICE) * 1000000 +
	    (T_WAIT_TICK)((t % CLOCK_SLICE) * 1000000.0 / CLOCK_SLICE));
}

#endif





/****************************************************************************
 * ��������Ĵ�������ν��������λ
 *****************************************************************************/
int	wait_vsync_init(void)
{
    if (verbose_proc) {
#ifdef  HAVE_GETTIMEOFDAY
	printf("Timer start (gettimeofday(2) - based)\n");
#else
	printf("Timer start (clock(3) - based)\n");
#endif
    }

    return TRUE;
}

void	wait_vsync_exit(void)
{
}



/****************************************************************************
 * ��������Ĵ������������
 *****************************************************************************/
void	wait_vsync_setup(long vsync_cycle_us, int do_sleep)
{
    set_tick();

    sleep_min_time = wait_sleep_min_us;
    wait_counter = 0;


    delta_time = (T_WAIT_TICK) vsync_cycle_us;		/* 1�ե졼����� */
    next_time  = get_tick() + delta_time;		/* ���ե졼����� */

    wait_do_sleep = do_sleep;				/* Sleep ̵ͭ */
}



/****************************************************************************
 * ��������Ĵ�������μ¹�
 *****************************************************************************/
int	wait_vsync_update(void)
{
    int	on_time = FALSE;
    T_WAIT_TICK	diff_us;

    /* �¹���ϡ�sleep ���뤫�ɤ����ϡ����ץ����ˤ�뤱�ɡ� */
    /* ��˥塼�Ǥ�ɬ�� sleep �����뤳�Ȥˤ��롣              */
    /* (�줤FreeBSD �ǡ��ʤ����ϥ󥰥��åפ��뤳�Ȥ�����ġ�) */
    /* (PAUSE�������ʤ��Τ����ġĤʤ���?                  ) */
    int need_sleep = (quasi88_is_exec() ? wait_do_sleep : TRUE);

    diff_us = next_time - get_tick();

    if (tick_error == FALSE) {

	if (diff_us > 0) {	/* �ޤ����֤�;�äƤ���ʤ� */
			 	/* diff_us �̥ߥ��å������� */
	    if (need_sleep) {		/* ���֤����ޤ� sleep ������ */

		/* FreeBSD ���ξ�硢�ʲ��Τ��������ܤ���ˡ�ʳ��ϡ�
		   10msñ�̤ǥ��꡼�פ��뤳�Ȥ�Ƚ����
		   �Ĥޤꡢ55.4hz�ξ�硢18ms�ǤϤʤ���20ms�Υ��꡼��
		   �ˤʤ롣*/

#if 1
		/* ver 0.6.3 �ޤǤ���ˡ
		   �������٤ޤǥ������Ȥ򤷤Ƥ��뤿��㴳�٤줬ȯ������Ϥ���
		   ������ɥΥ������װ����� */
		if (diff_us < sleep_min_time) {	/* �Ĥ�Ϥ��ʤ�ӥ�����������*/
		    while (tick_error == FALSE) {
			if (next_time <= get_tick())
			    break;
		    }
		} else {			/* �Ĥ�¿����Хǥ��쥤      */
		    delay_usec(diff_us);
		}
#elif 0
		/* 1ms �ʾ���֤�������� 100us �� sleep ��
		   1ms ̤���ξ��ϥӥ����������Ȥ򤷤Ƥߤ���XMAME����
		   �ӥ����������Ȥ�������٤ǤϤʤ���100us���˽����褦��
		   �Ȥ����� 100us �� sleep �� CPU��٤� 100% �᤯�ˤʤ�
		   ���Ȥ�Ƚ��������Ϥޤ�������*/
		while (tick_error == FALSE) {
		    diff_us = next_time - get_tick();
		    if (diff_us >= 1000) {
			delay_usec(100);
		    } else if (diff_us <= 100) {
			break;
		    }
		}
#elif 0
		/* SDL �� 1ms ñ�̤� sleep �ǡ�CPU��٤�ʤ���������ɤ�
		   �Υ������Ǥʤ��Τǡ����Τޤ޿������Ƥߤ褦��*/
		if (diff_us >= 1000) {
		    delay_usec((diff_us/1000) * 1000);
		}
#elif 0
		/* �������� */
		while (tick_error == FALSE) {
		    diff_us = next_time - get_tick();
		    if (diff_us >= 1100) {
			delay_usec(diff_us - 100);
		    } else if (diff_us <= 100) {
			break;
		    }
		}
#endif
	    } else {			/* ���֤����ޤ�Tick ��ƻ뤹���� */

		while (tick_error == FALSE) {
		    if (next_time <= get_tick())
			break;
		}
	    }

	    on_time = TRUE;
	}
    }


    /* ���ե졼�����򻻽� */
    next_time += delta_time;


    if (on_time) {			/* ������˽����Ǥ��� */
	wait_counter = 0;
    } else {				/* ������˽����Ǥ��Ƥ��ʤ� */
	wait_counter ++;
	if (wait_counter >= wait_count_max) {	/* �٤줬�Ҥɤ����� */
	    wait_vsync_setup((long) delta_time,	/* �������Ȥ�����   */
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
    if (show_fps) {
	display_fps();
    }

    if (on_time) return WAIT_JUST;
    else         return WAIT_OVER;
}



/****************************************************************************
 *
 *****************************************************************************/
/* Ŭ���� fps �׻� */
static	void	display_fps(void)
{
#ifdef  HAVE_GETTIMEOFDAY
    static struct timeval tv0;
    static int frame_count;
    static int prev_drawn_count;
    static int prev_vsync_count;

    /* ���� tv0 �Ͻ����̤�ʤΤǡ����߻���򥻥å� */
    if (tv0.tv_sec == 0 &&
	tv0.tv_usec == 0) {
	gettimeofday(&tv0, 0);
    }

    if (quasi88_is_exec()) {

	/* ���δؿ��ϡ����������� �� �ե졼����˸ƤӽФ���롣
	   60�ե졼���(��1��)�ƤӽФ��줿�顢FPS��׻�����ɽ������ */
	if (++ frame_count >= 60) {

	    char buf[32];
	    struct timeval tv1;
	    double dt, fps, hz;
	    int now_drawn_count = quasi88_info_draw_count();
	    int now_vsync_count = quasi88_info_vsync_count();

	    gettimeofday(&tv1, 0);
	    dt  = (double)(tv1.tv_sec  - tv0.tv_sec);
	    dt += (double)(tv1.tv_usec - tv0.tv_usec) / 1000000.0;

	    hz  = (double)(now_vsync_count - prev_vsync_count) / dt;
	    fps = (double)(now_drawn_count - prev_drawn_count) / dt;
	    sprintf(buf, "FPS: %4.1f (VSYNC %4.1f)", fps, hz);
	    graph_set_window_title(buf);


	    tv0 = tv1;
	    frame_count = 0;
	    prev_drawn_count = now_drawn_count;
	    prev_vsync_count = now_vsync_count;
	}

    } else {
	tv0.tv_sec  = 0;
	tv0.tv_usec = 0;
	frame_count = 0;
	prev_drawn_count = quasi88_info_draw_count();
	prev_vsync_count = quasi88_info_vsync_count();
    }
#endif
}
