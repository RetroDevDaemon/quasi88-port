/***********************************************************************
 * ��������Ĵ������ (�����ƥ��¸)
 *
 *	�ܺ٤ϡ� wait.h ����
 ************************************************************************/

#include <stdio.h>
#include <SDL.h>

#include "quasi88.h"
#include "initval.h"
#include "wait.h"



/*---------------------------------------------------------------------------*/
static	int	wait_do_sleep;			/* idle���� sleep ����       */

static	int	wait_counter = 0;		/* Ϣ³������֥����С�������*/
static	int	wait_count_max = 10;		/* ����ʾ�Ϣ³�����С�������
						   ��ö,����Ĵ������������ */

/* �������Ȥ˻��Ѥ�����֤�����ɽ���ϡ� usñ�̤Ȥ��롣 (ms�������٤��㤤�Τ�) 

   SDL �λ�������ؿ� SDL_GetTicks() �� ms ñ�̤ǡ� unsigned long �����֤���
   ����� 1000�ܤ��� (us���Ѵ�����) ���Ѥ���ȡ�71ʬ�Ƿ夢�դ줷�Ƥ��ޤ��Τǡ�
   ����ɽ���� long long ���ˤ��褦��

   �ʤ��� SDL_GetTicks �� 49���ܤ���ä�(wrap)���ޤ��Τǡ�����ɽ���⤳�νִ֤�
   �������ʤ�Τˤʤ� (�������Ȼ��֤��Ѥˤʤ�) �������ˤ��ʤ����Ȥˤ��롣 */

#ifdef SDL_HAS_64BIT_TYPE
typedef	Sint64		T_WAIT_TICK;
#else
typedef	long		T_WAIT_TICK;
#endif

static	T_WAIT_TICK	next_time;		/* ���ե졼��λ��� */
static	T_WAIT_TICK	delta_time;		/* 1 �ե졼��λ��� */



/* ---- ���߻����������� (usecñ��) ---- */

#define	GET_TICK()	((T_WAIT_TICK)SDL_GetTicks() * 1000)





/****************************************************************************
 * ��������Ĵ�������ν��������λ
 *****************************************************************************/
int	wait_vsync_init(void)
{
    if (! SDL_WasInit(SDL_INIT_TIMER)) {
	if (SDL_InitSubSystem(SDL_INIT_TIMER) != 0) {
	    if (verbose_wait) printf("Error Wait (SDL)\n");
	    return FALSE;
	}
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
    wait_counter = 0;


    delta_time = (T_WAIT_TICK) vsync_cycle_us;		/* 1�ե졼����� */
    next_time  = GET_TICK() + delta_time;		/* ���ե졼����� */

    wait_do_sleep = do_sleep;				/* Sleep ̵ͭ */
}



/****************************************************************************
 * �������Ƚ���
 *****************************************************************************/
int	wait_vsync_update(void)
{
    int slept   = FALSE;
    int on_time = FALSE;
    T_WAIT_TICK diff_ms;


    diff_ms = (next_time - GET_TICK()) / 1000;

    if (diff_ms > 0) {			/* �٤�Ƥʤ�(���֤�;�äƤ���)�ʤ� */
					/* diff_ms �ߥ��á��������Ȥ���     */

	if (wait_do_sleep) {		/* ���֤����ޤ� sleep ������ */

#if 1	    /* ��ˡ (1) */
	    SDL_Delay((Uint32) diff_ms);	/* diff_ms �ߥ��á��ǥ��쥤 */
	    slept = TRUE;

#else	    /* ��ˡ (2) */
	    if (diff_ms < 10) {			/* 10ms̤���ʤ�ӥ�����������*/
		while (GET_TICK() <= next_time)
		    ;
	    } else {				/* 10ms�ʾ�ʤ�ǥ��쥤      */
		SDL_Delay((Uint32) diff_ms);
		slept = TRUE;
	    }
#endif

	} else {			/* ���֤����ޤ�Tick��ƻ뤹���� */

	    while (GET_TICK() <= next_time)
		;				/* �ӥ����������� */
	}

	on_time = TRUE;
    }

    if (slept == FALSE) {	/* ���٤� SDL_Delay ���ʤ��ä���� */
	SDL_Delay(1);			/* for AUDIO thread ?? */
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

    if (on_time) return WAIT_JUST;
    else         return WAIT_OVER;
}
