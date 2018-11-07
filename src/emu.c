/************************************************************************/
/*									*/
/* ���ߥ�⡼��								*/
/*									*/
/************************************************************************/

#include <stdio.h>

#include "quasi88.h"
#include "initval.h"
#include "emu.h"

#include "pc88cpu.h"

#include "screen.h"
#include "keyboard.h"
#include "intr.h"
#include "event.h"
#include "menu.h"
#include "monitor.h"
#include "pause.h"
#include "wait.h"
#include "suspend.h"
#include "status.h"
#include "graph.h"
#include "snddrv.h"




break_t		break_point[2][NR_BP];	/* �֥졼���ݥ����		*/
break_drive_t	break_point_fdc[NR_BP];	/* FDC �֥졼���ݥ����		*/


int	cpu_timing	= DEFAULT_CPU;		/* SUB-CPU ��ư����	*/

int	select_main_cpu = TRUE;			/* -cpu 0 �¹Ԥ���CPU	*/
						/* ���ʤ� MAIN CPU��¹�*/

int	dual_cpu_count	= 0;			/* -cpu 1 Ʊ������STEP��*/
int	CPU_1_COUNT	= 4000;			/* ���Ρ������		*/

int	cpu_slice_us    = 5;			/* -cpu 2 ������ʬ��(us)*/
						/* 10>��SILPHEED��ư����*/

int	trace_counter	= 1;			/* TRACE ���Υ�����	*/



static	int	main_state   = 0;
static	int	sub_state    = 0;
#define	JACKUP	(256)


static	int	emu_mode_execute= GO;
static	int	emu_rest_step;

void	set_emu_exec_mode( int mode )
{
  emu_mode_execute = mode;
}

/***********************************************************************
 * ���ߥ�졼�Ƚ����ν������Ϣ
 ************************************************************************/

void	emu_reset( void )
{
  select_main_cpu = TRUE;
  dual_cpu_count  = 0;

  main_state   = 0;
  sub_state    = 0;
}


void	emu_breakpoint_init( void )
{
  int	i, j;
	/* �֥졼���ݥ���ȤΥ������� (��˥����⡼����) */
  for( j=0; j<2; j++ )
    for( i=0; i<NR_BP; i++ )
      break_point[j][i].type = BP_NONE;

  for( i=0; i<NR_BP; i++ )
    break_point_fdc[i].type = BP_NONE;
}




/***********************************************************************
 * CPU�¹Խ��� (EXEC) ������
 *	-cpu <n> �˱����ơ�ư����Ѥ��롣
 *
 *	STEP  ���ϡ�1step �����¹Ԥ��롣
 *	TRACE ���ϡ�������ʬ��1step �¹Ԥ��롣
 *
 *	�֥졼���ݥ���Ȼ�����ϡ�1step�¹Ԥ��٤� PC ���֥졼���ݥ���Ȥ�
 *	ã�������ɤ������ǧ���롣
 *
 ************************************************************************/

#define	INFINITY	(0)
#define	ONLY_1STEP	(1)

/*------------------------------------------------------------------------*/

/*
 * �֥졼���ݥ���� (������ PC) ��̵ͭ������å�����
 */

static	int	check_break_point_PC( void )
{
  int	i, j;

  for( i=0; i<NR_BP; i++ ) if( break_point[BP_MAIN][i].type == BP_PC ) break;
  for( j=0; j<NR_BP; j++ ) if( break_point[BP_SUB][j].type  == BP_PC ) break;

  if( i==NR_BP && j==NR_BP ) return FALSE;
  else                       return TRUE;
}

/*------------------------------------------------------------------------*/

/*
 * CPU �� 1step �¹Ԥ��ơ�PC���֥졼���ݥ���Ȥ�ã�����������å�����
 *	�֥졼���ݥ����(������PC)̤����ʤ餳�δؿ��ϻȤ鷺��z80_emu()��Ȥ�
 */

static	int	z80_emu_with_breakpoint( z80arch *z80, int unused )
{
  int i, cpu, states;

  states = z80_emu( z80, 1 );		/* 1step �����¹� */

  if( z80==&z80main_cpu ) cpu = BP_MAIN;
  else                    cpu = BP_SUB;

  for( i=0; i<NR_BP; i++ ){
    if( break_point[cpu][i].type == BP_PC     &&
	break_point[cpu][i].addr == z80->PC.W ){

      if( i==BP_NUM_FOR_SYSTEM ){
	break_point[cpu][i].type = BP_NONE;
      }

      printf( "*** Break at %04x *** ( %s[#%d] : PC )\n",
	      z80->PC.W, (cpu==BP_MAIN)?"MAIN":"SUB", i+1 );

      quasi88_debug();
    }
  }

  return states;
}

/*---------------------------------------------------------------------------*/

static	int	passed_step;		/* �¹Ԥ��� step�� */
static	int	target_step;		/* ���� step����ã����ޤǼ¹Ԥ��� */

static	int	infinity, only_1step;
static	int	(*z80_exec)( z80arch *, int );


void	emu_init(void)
{
/*xmame_sound_update();*/
  xmame_update_video_and_audio();
  event_update();
/*keyboard_update();*/



/*screen_set_dirty_all();*/
/*screen_set_dirty_palette();*/

  /* ���ơ����������ꥢ */
  status_message_default(0, NULL);
  status_message_default(1, NULL);
  status_message_default(2, NULL);



	/* �֥졼���ݥ���������̵ͭ�ǡ��ƤӽФ��ؿ����Ѥ��� */
  if( check_break_point_PC() ) z80_exec = z80_emu_with_breakpoint;
  else                         z80_exec = z80_emu;


	/* GO/TRACE/STEP/CHANGE �˱����ƽ����η����֤��������� */

  passed_step = 0;

  switch( emu_mode_execute ){
  default:
  case GO:
    target_step = 0;			/* ̵�¤˼¹� */
    infinity    = INFINITY;
    only_1step  = ONLY_1STEP;
    break;

  case TRACE:
    target_step = trace_counter;	/* ���ꥹ�ƥå׿��¹� */
    infinity    = ONLY_1STEP;
    only_1step  = ONLY_1STEP;
    break;

  case STEP:
    target_step = 1;			/* 1���ƥå׼¹� */
    infinity    = ONLY_1STEP;
    only_1step  = ONLY_1STEP;
    break;

  case TRACE_CHANGE:
    target_step = 0;			/* ̵�¤˼¹� */
    infinity    = ONLY_1STEP;
    only_1step  = ONLY_1STEP;
    break;
  }


  /* �¹Ԥ���Ĥꥹ�ƥå׿���
	TRACE / STEP �λ��ϡ����ꤵ�줿���ƥå׿���
	GO / TRACE_CHANGE �ʤ� ̵�¤ʤΤǡ� 0��
		�ʤ�������ǥ�˥塼�����ܤ�����硢����Ū�� 1 �����åȤ���롣
		����ˤ��̵�¤˽���������Ǥ⡢�롼�פ�ȴ����褦�ˤʤ롣 */
  emu_rest_step = target_step;
}


void	emu_main(void)
{
  int	wk;

  profiler_lapse( PROF_LAPSE_CPU );

  switch( emu_mode_execute ){

  /*------------------------------------------------------------------------*/
  case GO:				/* �Ҥ�����¹Ԥ���           */
  case TRACE:				/* ���ꤷ�����ƥåס��¹Ԥ��� */
  case STEP:				/* 1���ƥåפ������¹Ԥ���    */

    for(;;){

      switch( cpu_timing ){

      case 0:		/* select_main_cpu �ǻ��ꤵ�줿�ۤ���CPU��̵�¼¹� */
	if( select_main_cpu ) (z80_exec)( &z80main_cpu, infinity );
	else                  (z80_exec)( &z80sub_cpu,  infinity );
	break;

      case 1:		/* dual_cpu_count==0 �ʤ�ᥤ��CPU��̵�¼¹ԡ�*/
			/*               !=0 �ʤ�ᥤ�󥵥֤��߼¹� */
	if( dual_cpu_count==0 ) (z80_exec)( &z80main_cpu, infinity   );
	else{
	  (z80_exec)( &z80main_cpu, only_1step );
	  (z80_exec)( &z80sub_cpu,  only_1step );
	  dual_cpu_count --;
	}
	break;

      case 2:		/* �ᥤ��CPU������CPU���ߤ� 5us ���ļ¹� */
	if( main_state < 1*JACKUP  &&  sub_state < 1*JACKUP ){
	  main_state += (int) ((cpu_clock_mhz * cpu_slice_us) * JACKUP);
	  sub_state  += (int) ((3.9936        * cpu_slice_us) * JACKUP);
	}
	if( main_state >= 1*JACKUP ){
	  wk = (infinity==INFINITY) ? main_state/JACKUP : ONLY_1STEP;
	  main_state -= (z80_exec( &z80main_cpu, wk ) ) * JACKUP;
	}
	if( sub_state >= 1*JACKUP ){
	  wk = (infinity==INFINITY) ? sub_state/JACKUP : ONLY_1STEP;
	  sub_state  -= (z80_exec( &z80sub_cpu, wk ) ) * JACKUP;
	}
	break;
      }

      /* TRACE/STEP�¹Ի������ꥹ�ƥå׼¹Դ�λ�����顢��˥��������ܤ��� */
      if( emu_rest_step ){
	passed_step ++;
	if( -- emu_rest_step <= 0 ) {
	  quasi88_debug();
	}
      }

      /* ������ɽ��ϥ����ߥ󥰤Ǥ���С����� */
      if (quasi88_event_flags & EVENT_AUDIO_UPDATE) {
	quasi88_event_flags &= ~EVENT_AUDIO_UPDATE;

	profiler_lapse( PROF_LAPSE_SND );

	xmame_sound_update();			/* ������ɽ��� */

	profiler_lapse( PROF_LAPSE_AUDIO );

	xmame_update_video_and_audio();		/* ������ɽ��� ����2 */

	profiler_lapse( PROF_LAPSE_INPUT );

	event_update();				/* ���٥�Ƚ���		*/
	keyboard_update();

	profiler_lapse( PROF_LAPSE_CPU2 );
      }

      /* �ӥǥ����ϥ����ߥ󥰤Ǥ���С�CPU�����ϰ�ö��ߡ���̤�ȴ���� */
      if (quasi88_event_flags & EVENT_FRAME_UPDATE) {
	return;
      }

      /* ��˥������ܻ��佪λ���ϡ� CPU�����ϰ�ö��ߡ���̤�ȴ���� */
      if (quasi88_event_flags & (EVENT_DEBUG | EVENT_QUIT)) {
	return;
      }

      /* �⡼�����ؤ�ȯ�����Ƥ⡢��̤ˤ�ȴ���ʤ����ӥǥ����Ϥޤ��Ԥ� */
      /* (ȴ����ȡ� ���ߥ� �� ���� �� �������� ��ή�줬�����Τǡ�) */
    }
    break;

  /*------------------------------------------------------------------------*/

	/* ���ä��Υ֥졼�������Ϥ��ޤ�ư���ʤ����⡦���� (̤����) */

  case TRACE_CHANGE:			/* CPU���ڤ��ؤ��ޤǽ����򤹤� */
    if( cpu_timing >= 1 ){
      printf( "command 'trace change' can use when -cpu 0\n");
      quasi88_monitor();
      break;
    }

    wk = select_main_cpu;
    while( wk==select_main_cpu ){
      if( select_main_cpu ) (z80_exec)( &z80main_cpu, infinity );
      else                  (z80_exec)( &z80sub_cpu,  infinity );
      if( emu_rest_step ){
	passed_step ++;
	if( -- emu_rest_step <= 0 ) {
	  quasi88_debug();
	}
      }

      if (quasi88_event_flags & EVENT_AUDIO_UPDATE) {
	quasi88_event_flags &= ~EVENT_AUDIO_UPDATE;
	profiler_lapse( PROF_LAPSE_SND );
	xmame_sound_update();			/* ������ɽ��� */
	profiler_lapse( PROF_LAPSE_AUDIO );
	xmame_update_video_and_audio();		/* ������ɽ��� ����2 */
	profiler_lapse( PROF_LAPSE_INPUT );
	event_update();				/* ���٥�Ƚ���		*/
	keyboard_update();
      }

      if (quasi88_event_flags & EVENT_FRAME_UPDATE) {
	return;
      }
      if (quasi88_event_flags & (EVENT_DEBUG | EVENT_QUIT)) {
	return;
      }
    }
    quasi88_debug();
    break;
  }
  return;
}
















/***********************************************************************
 * ���ơ��ȥ��ɡ����ơ��ȥ�����
 ************************************************************************/

#define	SID	"EMU "

static	T_SUSPEND_W	suspend_emu_work[] =
{
  { TYPE_INT,	&cpu_timing,		},
  { TYPE_INT,	&select_main_cpu,	},
  { TYPE_INT,	&dual_cpu_count,	},
  { TYPE_INT,	&CPU_1_COUNT,		},
  { TYPE_INT,	&cpu_slice_us,		},
  { TYPE_INT,	&main_state,		},
  { TYPE_INT,	&sub_state,		},

  { TYPE_END,	0			},
};


int	statesave_emu( void )
{
  if( statesave_table( SID, suspend_emu_work ) == STATE_OK ) return TRUE;
  else                                                       return FALSE;
}

int	stateload_emu( void )
{
  if( stateload_table( SID, suspend_emu_work ) == STATE_OK ) return TRUE;
  else                                                       return FALSE;
}
