/***********************************************************************
 * ��������Ĵ������ (�����ƥ��¸)
 *
 *      �ܺ٤ϡ� wait.h ����
 ************************************************************************/

#include "quasi88.h"

#include "device.h"
#include <OSUtils.h>

#include "wait.h"



/*---------------------------------------------------------------------------*/
static	int	wait_counter = 0;		/* Ϣ³������֥����С�������*/
static	int	wait_count_max = 10;		/* ����ʾ�Ϣ³�����С�������
						   ��ö,����Ĵ������������ */

/* �������Ȥ˻��Ѥ�����֤�����ɽ���ϡ� usñ�̤Ȥ��롣 (ms�������٤��㤤�Τ�) 

   ToolBox �λ�������ؿ� TickCount() �� 1/60s ñ�̤��ͤ��֤���
   ��������� us ���Ѵ����ƻ��Ѥ��뤳�Ȥˤ��롣
   ����ɽ���� long ���ˤ���ȡ�71ʬ�Ƿ夢�դ�(wrap)�򵯤����Ƥ��ޤ������νִ֤�
   �������ʤ�Τˤʤ� (�������Ȼ��֤��Ѥˤʤ�) ��
   �Ǥ���� 64bit��(long long)�ˤ��������ɡ��ɤ����������Ƥʤ� ? */

#if	0
typedef	long long	T_WAIT_TICK;
#else
typedef	long		T_WAIT_TICK;
#endif

static	T_WAIT_TICK	next_time;		/* ���ե졼��λ��� */
static	T_WAIT_TICK	delta_time;		/* 1 �ե졼��λ��� */



/* ---- ���߻����������� (usecñ��) ---- */

#define	GET_TICK()	( (T_WAIT_TICK)TickCount() * 1000000/60 )





/****************************************************************************
 * ��������Ĵ�������ν��������λ
 *****************************************************************************/
int	wait_vsync_init(void)
{
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


    delta_time = (T_WAIT_TICK)(1000000.0 / 60.0);	/* 1�ե졼����� */
    next_time  = GET_TICK() + delta_time;		/* ���ե졼����� */


    /* ���� vsync_cycle_us, do_sleep ��̵�뤹�� */
}



/****************************************************************************
 * ��������Ĵ�������μ¹�
 *****************************************************************************/
int	wait_vsync_update(void)
{
    int on_time = FALSE;
    T_WAIT_TICK diff_us;


    diff_us = next_time - GET_TICK();

    if (diff_us > 0) {			/* �٤�Ƥʤ�(���֤�;�äƤ���)�ʤ� */

#if 0					/* �ӥ����������Ȥ���Ȥ����롩  */
	while (GET_TICK() <= next_time)
	    ;

#else					/* Delay ���Ƥߤ롦���� */
	UInt32 unused;
	diff_us = diff_us * 60 / 1000000;
	if (diff_us) {
	    Delay(diff_us,&unused);
	}
#endif

	on_time = TRUE;
    }


    /* ���ե졼�����򻻽� */
    next_time += delta_time;


    if (on_time) {			/* ������˽����Ǥ��� */
	wait_counter = 0;
    } else {				/* ������˽����Ǥ��Ƥ��ʤ� */
	wait_counter ++;
	if (wait_counter >= wait_count_max) {	/* �٤줬�Ҥɤ����� */
	    wait_vsync_setup(0,0);		/* �������Ȥ�����   */
	}
    }

    if (on_time) return WAIT_JUST;
    else         return WAIT_OVER;
}
