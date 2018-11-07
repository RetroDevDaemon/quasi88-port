/***********************************************************************
 * ��������Ĵ������ (�����ƥ��¸)
 *
 *	�ܺ٤ϡ� wait.h ����
 ************************************************************************/
#include <windows.h>
#include <mmsystem.h>

#include "quasi88.h"
#include "device.h"
#include "initval.h"
#include "wait.h"



#if	1			/* �Ǿ�������ʬ��ǽ�ϡ����ꤷ�ʤ� */
#define	START_APP_PRECISION()
#define	STOP_APP_PRECISION()
#define	BEGIN_PRECISION()
#define	END_PRECISION()
#elif	0			/* �Ǿ�������ʬ��ǽ�ϡ����ץ���̤��� 1ms */
#define	START_APP_PRECISION()	timeBeginPeriod(1)
#define	STOP_APP_PRECISION()	timeEndPeriod(1)
#define	BEGIN_PRECISION()
#define	END_PRECISION()
#elif	0			/* �Ǿ�������ʬ��ǽ�ϡ����ַ�¬�λ��Τ� 1ms */
#define	START_APP_PRECISION()
#define	STOP_APP_PRECISION()
#define	BEGIN_PRECISION()	timeBeginPeriod(1)
#define	END_PRECISION()		timeEndPeriod(1)
#endif


/*---------------------------------------------------------------------------*/
static	int	wait_do_sleep;			/* idle���� sleep ����       */

static	int	wait_counter = 0;		/* Ϣ³������֥����С�������*/
static	int	wait_count_max = 10;		/* ����ʾ�Ϣ³�����С�������
						   ��ö,����Ĵ������������ */

/* �������Ȥ˻��Ѥ�����֤�����ɽ���ϡ� usñ�̤Ȥ��롣 (ms�������٤��㤤�Τ�) 

   WIN32 �λ�������ؿ� timeGetTime() �� ms ñ�̤ǡ� DWORD�����֤���
   ����� 1000�ܤ��� (us���Ѵ�����) ���Ѥ���ȡ�71ʬ�Ƿ夢�դ줷�Ƥ��ޤ��Τǡ�
   ����ɽ���� __int64 ���ˤ��褦��

   �ʤ��� timeGetTime() �� 49���ܤ���ä�(wrap)���ޤ��Τǡ�����ɽ���⤳�νִ֤�
   �������ʤ�Τˤʤ� (�������Ȼ��֤��Ѥˤʤ�) �������ˤ��ʤ����Ȥˤ��롣 */

typedef	signed __int64	T_WAIT_TICK;

static	T_WAIT_TICK	next_time;		/* ���ե졼��λ��� */
static	T_WAIT_TICK	delta_time;		/* 1 �ե졼��λ��� */



/* ---- ���߻����������� (usecñ��) ---- */

#define	GET_TICK()	((T_WAIT_TICK)timeGetTime() * 1000)





/****************************************************************************
 * ��������Ĵ�������ν��������λ
 *****************************************************************************/
int	wait_vsync_init(void)
{
    START_APP_PRECISION();
    return TRUE;
}

void	wait_vsync_exit(void)
{
    STOP_APP_PRECISION();
}



/****************************************************************************
 * ��������Ĵ������������
 *****************************************************************************/
void	wait_vsync_setup(long vsync_cycle_us, int do_sleep)
{
    wait_counter = 0;


    delta_time = (T_WAIT_TICK) vsync_cycle_us;		/* 1�ե졼����� */

    BEGIN_PRECISION();		/* �����٤�1ms�ˤ���(timeGetTime, Sleep�ʤ�) */

    next_time  = GET_TICK() + delta_time;		/* ���ե졼����� */

    END_PRECISION();		/* �����٤��᤹ */

    wait_do_sleep = do_sleep;				/* Sleep ̵ͭ */
}



/****************************************************************************
 * ��������Ĵ�������μ¹�
 *****************************************************************************/
int	wait_vsync_update(void)
{
    int on_time = FALSE;
    T_WAIT_TICK diff_ms;


    BEGIN_PRECISION();		/* �����٤�1ms�ˤ���(timeGetTime, Sleep�ʤ�) */

    diff_ms = (next_time - GET_TICK()) / 1000;

    if (diff_ms > 0) {			/* �٤�Ƥʤ�(���֤�;�äƤ���)�ʤ� */
					/* diff_ms �ߥ��á��������Ȥ���     */

	if (wait_do_sleep) {		/* ���֤����ޤ� sleep ������ */

	    Sleep((DWORD)diff_ms);		/* diff_ms �ߥ��á��ǥ��쥤 */

	} else {			/* ���֤����ޤ�Tick��ƻ뤹���� */

	    while (GET_TICK() <= next_time)
		;				/* �ӥ����������� */
	}

	on_time = TRUE;
    }

    END_PRECISION();		/* �����٤��᤹ */


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
	    fprintf(debugfp, "wait %d\n", y);
	}
    }
#endif

    if (on_time) return WAIT_JUST;
    else         return WAIT_OVER;
}
