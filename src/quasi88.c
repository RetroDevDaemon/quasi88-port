/************************************************************************/
/* QUASI88 --- PC-8801 emulator						*/
/*	Copyright (c) 1998-2012 Showzoh Fukunaga			*/
/*	All rights reserved.						*/
/*									*/
/*	  ���Υ��եȤϡ�UNIX + X Window System �δĶ���ư��롢	*/
/*	PC-8801 �Υ��ߥ�졼���Ǥ���					*/
/*									*/
/*	  ���Υ��եȤκ����ˤ����ꡢMarat Fayzullin���� fMSX��	*/
/*	Nicola Salmoria�� (MAME/XMAME project) ��� MAME/XMAME��	*/
/*	��ߤ������ PC6001V �Υ������򻲹ͤˤ����Ƥ�餤�ޤ�����	*/
/*									*/
/*	����ա�							*/
/*	  ������ɥɥ饤�Фϡ�MAME/XMAME �Υ�������ή�Ѥ��Ƥ��ޤ���	*/
/*	������ʬ�Υ�����������ϡ�MAME/XMAME �����ढ�뤤�ϥ�������	*/
/*	���ܤ��Ƥ�������Ԥˤ���ޤ���					*/
/*	  FM���������ͥ졼���ϡ�fmgen �Υ�������ή�Ѥ��Ƥ��ޤ���	*/
/*	������ʬ�Υ�����������ϡ� cisc�� �ˤ���ޤ���		*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "quasi88.h"
#include "initval.h"

#include "pc88main.h"
#include "pc88sub.h"
#include "graph.h"
#include "memory.h"
#include "file-op.h"

#include "emu.h"
#include "drive.h"
#include "event.h"
#include "keyboard.h"
#include "monitor.h"
#include "snddrv.h"
#include "wait.h"
#include "status.h"
#include "suspend.h"
#include "snapshot.h"
#include "soundbd.h"
#include "screen.h"
#include "menu.h"
#include "pause.h"
#include "z80.h"
#include "intr.h"


int	verbose_level	= DEFAULT_VERBOSE;	/* ��Ĺ��٥�		*/
int	verbose_proc    = FALSE;		/* �����οʹԾ�����ɽ��	*/
int	verbose_z80	= FALSE;		/* Z80�������顼��ɽ��	*/
int	verbose_io	= FALSE;		/* ̤����I/O��������ɽ��*/
int	verbose_pio	= FALSE;		/* PIO ���������Ѥ�ɽ�� */
int	verbose_fdc	= FALSE;		/* FD���᡼���۾�����	*/
int	verbose_wait	= FALSE;		/* �������Ȼ��ΰ۾����� */
int	verbose_suspend = FALSE;		/* �����ڥ�ɻ��ΰ۾����� */
int	verbose_snd	= FALSE;		/* ������ɤΥ�å�����	*/

static	void	imagefile_all_open(int stateload);
static	void	imagefile_all_close(void);
static	void	status_override(void);

/***********************************************************************
 *
 *			QUASI88 �ᥤ��ؿ�
 *
 ************************************************************************/
void	quasi88(void)
{
    quasi88_start();
    quasi88_main();
    quasi88_stop(TRUE);
}

/* =========================== �ᥤ������ν���� ========================== */

#define	SET_PROC(n)	proc = n; if (verbose_proc) printf("\n"); fflush(NULL);
static	int	proc = 0;

void	quasi88_start(void)
{
    stateload_init();			/* ���ơ��ȥ��ɴ�Ϣ�����	*/
    drive_init();			/* �ǥ���������Υ�������	*/
    /* �� �����ϡ����ơ��ȥ��ɳ��ϤޤǤ˽�������Ƥ�������		*/

    SET_PROC(1);

					/* ���ߥ�졼���ѥ���γ���	*/
    if (memory_allocate() == FALSE) { quasi88_exit(-1); }

    if (resume_flag) {			/* ���ơ��ȥ���		*/
	SET_PROC(2);
	if (stateload() == FALSE) {
	    fprintf(stderr, "stateload: Failed ! (filename = %s)\n",
		    filename_get_state());
	    quasi88_exit(-1);
	}
	if (verbose_proc) printf("Stateload...OK\n"); fflush(NULL);
    }
    SET_PROC(3);

					/* ����ե��å������ƥ�����	*/
    if (screen_init() == FALSE) { quasi88_exit(-1); }
    SET_PROC(4);

					/* �����ƥ।�٥�Ƚ����	*/
    event_init();			/* (screen_init �θ�ǡ�)	*/

    					/* ������ɥɥ饤�н����	*/
    if (xmame_sound_start() == FALSE) {	quasi88_exit(-1); }
    SET_PROC(5);

   					/* ���������ѥ����ޡ������	*/
    if (wait_vsync_init() == FALSE) { quasi88_exit(-1); }
    SET_PROC(6);


    set_signal();			/* INT�����ʥ�ν���������	*/

    imagefile_all_open(resume_flag);	/* ���᡼���ե���������Ƴ���	*/

    					/* ���ߥ��ѥ����缡�����	*/
    pc88main_init((resume_flag) ? INIT_STATELOAD : INIT_POWERON);
    pc88sub_init ((resume_flag) ? INIT_STATELOAD : INIT_POWERON);

    key_record_playback_init();		/* �������ϵ�Ͽ/���� �����	*/

    screen_snapshot_init();		/* ���ʥåץ���åȴ�Ϣ�����   */


    debuglog_init();
    profiler_init();

    emu_breakpoint_init();

    if (verbose_proc) printf("Running QUASI88...\n");
}

/* ======================== �ᥤ������Υᥤ��롼�� ======================= */

void	quasi88_main(void)
{
    for (;;) {

	/* ��λ�α���������ޤǡ������֤��Ƥ�³���� */

	if (quasi88_loop() == QUASI88_LOOP_EXIT) {
	    break;
	}

    }

    /* quasi88_loop() �ϡ� 1�ե졼�� (VSYNC 1����ʬ��1/60��) �����ˡ�
       QUASI88_LOOP_ONE ���֤��Ƥ��롣
       ��������ͤ�Ƚ�Ǥ��ơ��ʤ�餫�ν�����ä��Ƥ�褤��

       �ޤ������������λ���� QUASI88_LOOP_BUSY ���֤��Ƥ��뤳�Ȥ⤢�뤬��
       ���ξ��ϵ��ˤ����ˡ������֤��ƤӽФ���³�Ԥ��뤳�� */

}

/* ========================== �ᥤ������θ����դ� ========================= */

void	quasi88_stop(int normal_exit)
{
    if (normal_exit) {
	if (verbose_proc) printf("Shutting down.....\n");
    }

    /* ���������ξ�硢verbose �ˤ��ܺ�ɽ�����ʤ���С����顼ɽ������ */
#define ERR_DISP(n)	((proc == (n)) && (verbose_proc == 0))

    switch (proc) {
    case 6:			/* ����� ����˽���äƤ��� */
	profiler_exit();
	debuglog_exit();
	screen_snapshot_exit();
	key_record_playback_exit();
	pc88main_term();
	pc88sub_term();
	imagefile_all_close();
	wait_vsync_exit();
	/* FALLTHROUGH */

    case 5:			/* �����ޡ��ν������NG */
	if (ERR_DISP(5)) printf("timer initialize failed!\n");
	xmame_sound_stop();
	/* FALLTHROUGH */

    case 4:			/* ������ɤν������NG */
	if (ERR_DISP(4)) printf("sound system initialize failed!\n");
	event_exit();
	screen_exit();
	/* FALLTHROUGH */

    case 3:			/* ����ե��å��ν������NG */
	if (ERR_DISP(3)) printf("graphic system initialize failed!\n");
	/* FALLTHROUGH */

    case 2:			/* ���ơ��ȥ��ɤ�NG */
	/* FALLTHROUGH */

    case 1:			/* ����ν������NG */
	if (ERR_DISP(2)) printf("memory allocate failed!\n");
	memory_free();
	/* FALLTHROUGH */

    case 0:			/* ��λ���� ���Ǥ˴�λ */
	break;
    }

    proc = 0;	/* ���δؿ���³���ƸƤ�Ǥ�����̵���褦�ˡ����ꥢ���Ƥ��� */
}


/***********************************************************************
 * QUASI88 ���潪λ�����ؿ�
 *	exit() ������˸Ƥܤ���
 ************************************************************************/

#define	MAX_ATEXIT	(32)
static	void (*exit_function[MAX_ATEXIT])(void);

/*
 * �ؿ������ MAX_ATEXIT �ġ���Ͽ�Ǥ��롣��������Ͽ�����ؿ��ϡ�
 * quasi88_exit() ��ƤӽФ������ˡ���Ͽ������ȵս�ǡ��ƤӽФ���롣
 */
void	quasi88_atexit(void (*function)(void))
{
    int i;
    for (i=0; i<MAX_ATEXIT; i++) {
	if (exit_function[i] == NULL) {
	    exit_function[i] = function;
	    return;
	}
    }
    printf("quasi88_atexit: out of array\n");
    quasi88_exit(-1);
}

/*
 * quasi88 ������λ���롣
 * quasi88_atexit() ����Ͽ�����ؿ���ƤӽФ�����ˡ� exit() ����
 */
void	quasi88_exit(int status)
{
    int i;

    quasi88_stop(FALSE);

    for (i=MAX_ATEXIT-1; i>=0; i--) {
	if (exit_function[i]) {
	    (*exit_function[i])();
	    exit_function[i] = NULL;
	}
    }

    exit(status);
}





/***********************************************************************
 * QUASI88�ᥤ��롼������
 *	QUASI88_LOOP_EXIT ���֤�ޤǡ�̵�¤˸ƤӽФ����ȡ�
 * �����
 *	QUASI88_LOOP_EXIT �� ��λ��
 *	QUASI88_LOOP_ONE  �� 1�ե졼��в�� (�������Ȥ����Τʤ����1/60�ü���)
 *	QUASI88_LOOP_BUSY �� �嵭�ʳ��Ρ��ʤ�餫�Υ����ߥ�
 ************************************************************************/
int	quasi88_event_flags = EVENT_MODE_CHANGED;
static	int mode	= EXEC;		/* ���ߤΥ⡼�� */
static	int next_mode	= EXEC;		/* �⡼�������׵���Ρ����⡼�� */

int	quasi88_loop(void)
{
    static enum {
	INIT,
	MAIN,
	WAIT,
    } step = INIT, step_after_wait = INIT;

    int stat;

    switch (step) {

    /* ======================== ���˥������� ======================== */
    case INIT:
	profiler_lapse( PROF_LAPSE_RESET );

	/* �⡼���ѹ����ϡ�ɬ����������롣�⡼���ѹ��ե饰�򥯥ꥢ */
	quasi88_event_flags &= ~EVENT_MODE_CHANGED;
	mode = next_mode;

	/* �㳰Ū�ʥ⡼���ѹ����ν��� */
	switch (mode) {
#ifndef	USE_MONITOR
	case MONITOR:	/* ���ꤨ�ʤ����ɡ�ǰ�Τ��� */
	    mode = PAUSE;
	    break;
#endif
	case QUIT:	/* QUIT �ʤ顢�ᥤ��롼�׽�λ */
	    return FALSE;
	}

	/* �⡼���̥��˥������� */
	if (mode == EXEC) { xmame_sound_resume(); }
	else              { xmame_sound_suspend();}

	screen_switch();
	event_switch();
	keyboard_switch();

	switch (mode) {
	case EXEC:	emu_init();		break;

	case MENU:	menu_init();		break;
#ifdef	USE_MONITOR
	case MONITOR:	monitor_init();		break;
#endif
	case PAUSE:	pause_init();		break;
	}

	status_override();

	wait_vsync_switch();


	/* ���˥�����������λ�����顢MAIN ������ */
	step = MAIN;

	/* ���ܤ��뤿�ᡢ��ö�ؿ���ȴ���� (FALLTHROUGH�Ǥ⤤������) */
	return QUASI88_LOOP_BUSY;


    /* ======================== �ᥤ����� ======================== */
    case MAIN:
	switch (mode) {

	case EXEC:	profiler_lapse( PROF_LAPSE_RESET );
			emu_main();		break;
#ifdef	USE_MONITOR
	case MONITOR:	monitor_main();		break;
#endif
	case MENU:	menu_main();		break;

	case PAUSE:	pause_main();		break;
	}

	/* �⡼���ѹ���ȯ�����Ƥ����顢(WAIT���) INIT �����ܤ��� */
	/* �����Ǥʤ���С�            (WAIT���) MAIN �����ܤ��� */
	if (quasi88_event_flags & EVENT_MODE_CHANGED) {
	    step_after_wait = INIT;
	} else {
	    step_after_wait = MAIN;
	}

	/* ���西���ߥ󥰤ʤ�Ф��������衣���θ� WAIT ��  */
	/* �����Ǥʤ���С�                WAIT ���������� */
	if (quasi88_event_flags & EVENT_FRAME_UPDATE) {
	    quasi88_event_flags &= ~EVENT_FRAME_UPDATE;
	    screen_update();
	    step = WAIT;
	} else {
	    step = step_after_wait;
	}

	/* ��˥������ܻ��佪λ���ϡ� WAIT ������¨���� INIT �� */
	if (quasi88_event_flags & (EVENT_DEBUG | EVENT_QUIT)) {
	    step = INIT;
	}

	/* ���ܤ��뤿�ᡢ��ö�ؿ���ȴ���� (WAIT����FALLTHROUGH�Ǥ⤤������) */
	return QUASI88_LOOP_BUSY;


    /* ======================== �������Ƚ��� ======================== */
    case WAIT:
	stat = WAIT_JUST;

	switch (mode) {
	case EXEC:
	    profiler_lapse( PROF_LAPSE_IDLE );
	    if (! no_wait) { stat = wait_vsync_update(); }
	    break;

	case MENU:
	case PAUSE:
	    /* Esound �ξ�硢 MENU/PAUSE �Ǥ� stream ��ή���Ƥ����ʤ���
	       ʣ����ư���ˡ� MENU/PAUSE ���Ƥʤ��ۤ��������Ǥʤ��ʤ롣
	       �������Τޤޤ��ȡ����ߤβ���ή��äѤʤ��ˤʤ�Τǡ�
	       ̵����ή���褦�ˤ��ʤ��ȡ� */
	    xmame_sound_update();		/* ������ɽ��� */
	    xmame_update_video_and_audio();	/* ������ɽ��� ����2 */
	    stat = wait_vsync_update();
	    break;
	}

	if (stat == WAIT_YET) { return QUASI88_LOOP_BUSY; }


	/* �������Ȼ��֤򸵤ˡ��ե졼�ॹ���åפ�̵ͭ����� */
	if (mode == EXEC) {
	    frameskip_check((stat == WAIT_JUST) ? TRUE : FALSE);
	}

	/* �������Ƚ�������λ�����顢�� (INIT �� MAIN) ������ */
	step = step_after_wait;
	return QUASI88_LOOP_ONE;
    }

    /* �����ˤ���ʤ� ! */
    return QUASI88_LOOP_EXIT;
}



/*======================================================================
 * QUASI88 �Υ⡼������
 *	�⡼�ɤȤ� QUASI88 �ξ��֤Τ��Ȥǡ� EXEC (�¹�)��PAUSE (������)��
 *	MENU (��˥塼����)�� MONITOR (���÷��ǥХå�)�� QUIT(��λ) �����롣
 *======================================================================*/
/* QUASI88�Υ⡼�ɤ����ꤹ�� */
static	void	set_mode(int newmode)
{
    if (mode != newmode) {

	if (mode == MENU) {		/* ��˥塼����¾�⡼�ɤ����ؤ� */
	    q8tk_event_quit();		/* Q8TK �ν�λ��ɬ��            */
	}

	next_mode = newmode;
	quasi88_event_flags |= EVENT_MODE_CHANGED;
	CPU_BREAKOFF();
    }
}

/* QUASI88�Υ⡼�ɤ��ڤ��ؤ��� */
void	quasi88_exec(void)
{
    set_mode(EXEC);
    set_emu_exec_mode(GO);
}

void	quasi88_exec_step(void)
{
    set_mode(EXEC);
    set_emu_exec_mode(STEP);
}

void	quasi88_exec_trace(void)
{
    set_mode(EXEC);
    set_emu_exec_mode(TRACE);
}

void	quasi88_exec_trace_change(void)
{
    set_mode(EXEC);
    set_emu_exec_mode(TRACE_CHANGE);
}

void	quasi88_menu(void)
{
    set_mode(MENU);
}

void	quasi88_pause(void)
{
    set_mode(PAUSE);
}

void	quasi88_monitor(void)
{
#ifdef	USE_MONITOR
    set_mode(MONITOR);
#else
    set_mode(PAUSE);
#endif
}

void	quasi88_debug(void)
{
#ifdef	USE_MONITOR
    set_mode(MONITOR);
    quasi88_event_flags |= EVENT_DEBUG;
#else
    set_mode(PAUSE);
#endif
}

void	quasi88_quit(void)
{
    set_mode(QUIT);
    quasi88_event_flags |= EVENT_QUIT;
}

/* QUASI88�Υ⡼�ɤ�������� */
int	quasi88_is_exec(void)
{
  return (mode == EXEC) ? TRUE : FALSE;
}
int	quasi88_is_menu(void)
{
  return (mode == MENU) ? TRUE : FALSE;
}
int	quasi88_is_pause(void)
{
  return (mode == PAUSE) ? TRUE : FALSE;
}
int	quasi88_is_monitor(void)
{
  return (mode == MONITOR) ? TRUE : FALSE;
}





/***********************************************************************
 *	Ŭ�ڤʰ��֤˰�ư����
 ************************************************************************/
void	wait_vsync_switch(void)
{
    long dt;

    /* dt < 1000000us (1sec) �Ǥʤ��ȥ��� */
    if (quasi88_is_exec()) {
	dt = (long)((1000000.0 / (CONST_VSYNC_FREQ * wait_rate/100)));
	wait_vsync_setup(dt, wait_by_sleep);
    } else {
	dt = (long)(1000000.0 / CONST_VSYNC_FREQ);
	wait_vsync_setup(dt, TRUE);
    }
}


static	void	status_override(void)
{
    static int first_fime = TRUE;

    if (first_fime) {

	/* EMU�⡼�ɤǵ�ư�������Τߡ����ơ�������ɽ�����Ѥ��� */
	if (mode == EXEC) {

	    status_message(0, STATUS_INFO_TIME, Q_TITLE " " Q_VERSION);

	    if (resume_flag == 0) {
		if (status_imagename == FALSE) {
		    status_message_default(1, "<F12> key to MENU");
		}
	    } else {
		status_message(1, STATUS_INFO_TIME, "State-Load Successful");
	    }
	}
	first_fime = FALSE;
    }
}



/***********************************************************************
 *	�ǥХå���
 ************************************************************************/
#include "debug.c"



/***********************************************************************
 *	��¿�ʴؿ�
 ************************************************************************/
#include "utility.c"



/***********************************************************************
 *			�ե�����̾���桿����
 ************************************************************************/
#include "fname.c"




/***********************************************************************
 * �Ƽ�ư��ѥ�᡼�����ѹ�
 *	�����δؿ��ϡ����硼�ȥ��åȥ��������䡢�����¸���Υ��٥��
 *	�����ʤɤ���ƤӽФ���뤳�Ȥ� *���* ���ꤷ�Ƥ��롣
 *
 *	��˥塼���̤�ɽ����˸ƤӽФ��ȡ���˥塼ɽ���ȿ����㤤��������
 *	�Τǡ���˥塼��ϸƤӽФ��ʤ��褦�ˡ����ߥ�¹���˸ƤӽФ��Τ�
 *	���ְ����������󡢤��ޤ�����
 *
 *	if( mode == EXEC ){
 *	    quasi88_disk_insert_and_reset( file, FALSE );
 *	}
 *
 ************************************************************************/

/***********************************************************************
 * QUASI88 ��ư��Υꥻ�åȽ����ؿ�
 ************************************************************************/
void	quasi88_get_reset_cfg(T_RESET_CFG *cfg)
{
    cfg->boot_basic	= boot_basic;
    cfg->boot_dipsw	= boot_dipsw;
    cfg->boot_from_rom	= boot_from_rom;
    cfg->boot_clock_4mhz= boot_clock_4mhz;
    cfg->set_version	= set_version;
    cfg->baudrate_sw	= baudrate_sw;
    cfg->use_extram	= use_extram;
    cfg->use_jisho_rom	= use_jisho_rom;
    cfg->sound_board	= sound_board;
}

void	quasi88_reset(const T_RESET_CFG *cfg)
{
    int sb_changed = FALSE;
    int empty[2];

    if (verbose_proc) printf("Reset QUASI88...start\n");

    pc88main_term();
    pc88sub_term();

    if (cfg) {
	if (sound_board	!= cfg->sound_board) {
	    sb_changed = TRUE;
	}

	boot_basic	= cfg->boot_basic;
	boot_dipsw	= cfg->boot_dipsw;
	boot_from_rom	= cfg->boot_from_rom;
	boot_clock_4mhz	= cfg->boot_clock_4mhz;
	set_version	= cfg->set_version;
	baudrate_sw	= cfg->baudrate_sw;
	use_extram	= cfg->use_extram;
	use_jisho_rom	= cfg->use_jisho_rom;
	sound_board	= cfg->sound_board;
    }

    /* ����κƳ��ݤ�ɬ�פʤ顢�������� */
    if (memory_allocate_additional() == FALSE) {
	quasi88_exit(-1);	/* ���ԡ� */
    }

    /* ������ɽ��ϤΥꥻ�å� */
    if (sb_changed == FALSE) {
	xmame_sound_reset();
    } else {
	menu_sound_restart(FALSE);	/* ������ɥɥ饤�Фκƽ���� */
    }

    /* ����ν���� */
    pc88main_init(INIT_RESET);
    pc88sub_init(INIT_RESET);

    /* FDC�ν���� */
    empty[0] = drive_check_empty(0);
    empty[1] = drive_check_empty(1);
    drive_reset();
    if (empty[0]) drive_set_empty(0);
    if (empty[1]) drive_set_empty(1);

    /*if (xmame_has_sound()) xmame_sound_reset();*/

    emu_reset();

    if (verbose_proc) printf("Reset QUASI88...done\n");
}



/***********************************************************************
 * QUASI88 ��ư��Υ��ơ��ȥ��ɽ����ؿ�
 *	TODO �����ǡ��ե�����̾���ꡩ
 ************************************************************************/
int	quasi88_stateload(int serial)
{
    int now_board, success;

    if (serial >= 0) {			/* Ϣ�ֻ��ꤢ�� (>=0) �ʤ� */
	filename_set_state_serial(serial);	/* Ϣ�֤����ꤹ�� */
    }

    if (verbose_proc) printf("Stateload...start (%s)\n",filename_get_state());

    if (stateload_check_file_exist() == FALSE) {	/* �ե�����ʤ� */
	if (quasi88_is_exec()) {
	    status_message(1, STATUS_INFO_TIME, "State-Load file not found !");
	} /* ��˥塼�Ǥϥ�������ɽ������Τǡ����ơ�����ɽ����̵���ˤ��� */

	if (verbose_proc) printf("State-file not found\n");
	return FALSE;
    }


    pc88main_term();			/* ǰ�Τ��ᡢ�����λ���֤� */
    pc88sub_term();
    imagefile_all_close();		/* ���᡼���ե�����������Ĥ��� */

    /*xmame_sound_reset();*/		/* ǰ�Τ��ᡢ������ɥꥻ�å� */
    /*quasi88_reset();*/		/* ǰ�Τ��ᡢ������ꥻ�å� */


    now_board = sound_board;

    success = stateload();		/* ���ơ��ȥ��ɼ¹� */

    if (now_board != sound_board) { 	/* ������ɥܡ��ɤ��Ѥ�ä��� */
	menu_sound_restart(FALSE);	/* ������ɥɥ饤�Фκƽ���� */
    }

    if (verbose_proc) {
	if (success) printf("Stateload...done\n");
	else         printf("Stateload...Failed, Reset start\n");
    }


    if (success) {			/* ���ơ��ȥ������������顦���� */

	imagefile_all_open(TRUE);		/* ���᡼���ե���������Ƴ���*/

	pc88main_init(INIT_STATELOAD);
	pc88sub_init(INIT_STATELOAD);

    } else {				/* ���ơ��ȥ��ɼ��Ԥ����顦���� */

	quasi88_reset(NULL);			/* �Ȥꤢ�����ꥻ�å� */
    }


    if (quasi88_is_exec()) {
	if (success) {
	    status_message(1, STATUS_INFO_TIME, "State-Load Successful");
	} else {
	    status_message(1, STATUS_INFO_TIME, "State-Load Failed !  Reset done ...");
	}

	/* quasi88_loop ���������֤� INIT �ˤ��뤿�ᡢ�⡼���ѹ������Ȥ��� */
	quasi88_event_flags |= EVENT_MODE_CHANGED;
    }
    /* ��˥塼�Ǥϥ�������ɽ������Τǡ����ơ�����ɽ����̵���ˤ��� */

    return success;
}



/***********************************************************************
 * QUASI88 ��ư��Υ��ơ��ȥ����ֽ����ؿ�
 *	TODO �����ǡ��ե�����̾���ꡩ
 ************************************************************************/
int	quasi88_statesave(int serial)
{
    int success;

    if (serial >= 0) {			/* Ϣ�ֻ��ꤢ�� (>=0) �ʤ� */
	filename_set_state_serial(serial);	/* Ϣ�֤����ꤹ�� */
    }

    if (verbose_proc) printf("Statesave...start (%s)\n",filename_get_state());

    success = statesave();		/* ���ơ��ȥ����ּ¹� */

    if (verbose_proc) {
	if (success) printf("Statesave...done\n");
	else         printf("Statesave...Failed, Reset done\n");
    }


    if (quasi88_is_exec()) {
	if (success) {
	    status_message(1, STATUS_INFO_TIME, "State-Save Successful");
	} else {
	    status_message(1, STATUS_INFO_TIME, "State-Save Failed !");
	}
    }	/* ��˥塼�Ǥϥ�������ɽ������Τǡ����ơ�����ɽ����̵���ˤ��� */

    return success;
}



/***********************************************************************
 * ���̥��ʥåץ���å���¸
 *	TODO �����ǡ��ե�����̾���ꡩ
 ************************************************************************/
int	quasi88_screen_snapshot(void)
{
    int success;

    success = screen_snapshot_save();


    if (success) {
	status_message(1, STATUS_INFO_TIME, "Screen Capture Saved");
    } else {
	status_message(1, STATUS_INFO_TIME, "Screen Capture Failed !");
    }

    return success;
}



/***********************************************************************
 * ������ɥǡ����Υե��������
 *	TODO �����ǡ��ե�����̾���ꡩ
 ************************************************************************/
int	quasi88_waveout(int start)
{
    int success;

    if (start) {
	success = waveout_save_start();

	if (success) {
	    status_message(1, STATUS_INFO_TIME, "Sound Record Start ...");
	} else {
	    status_message(1, STATUS_INFO_TIME, "Sound Record Failed !");
	}

    } else {

	success = TRUE;

	waveout_save_stop();
	status_message(1, STATUS_INFO_TIME, "Sound Record Stopped");
    }

    return success;
}



/***********************************************************************
 * �ɥ�å�����ɥɥ�å�
 *	TODO ����ͤ�⤦�칩��
 ************************************************************************/
int	quasi88_drag_and_drop(const char *filename)
{
    if (quasi88_is_exec() ||
	quasi88_is_pause()) {

	if (quasi88_disk_insert_all(filename, FALSE)) {

	    status_message(1, STATUS_INFO_TIME, "Disk Image Set and Reset");
	    quasi88_reset(NULL);

	    if (quasi88_is_pause()) {
		quasi88_exec();
	    }

	} else {

	    status_message(1, STATUS_WARN_TIME, "D&D Failed !  Disk Unloaded ...");
	}

	return TRUE;
    }

    return FALSE;
}



/***********************************************************************
 * �������Ȥ���Ψ����
 * �������Ȥ�̵ͭ����
 ************************************************************************/
int	quasi88_cfg_now_wait_rate(void)
{
    return wait_rate;
}
void	quasi88_cfg_set_wait_rate(int rate)
{
    int time = STATUS_INFO_TIME;
    char str[32];
    long dt;

    if (rate < 5)    rate = 5;
    if (rate > 5000) rate = 5000;

    if (wait_rate != rate) {
	wait_rate = rate;

	if (quasi88_is_exec()) {

	    sprintf(str, "WAIT  %4d[%%]", wait_rate);

	    status_message(1, time, str);
	    /* �� ���������ѹ������Τǡ�ɽ�����֤ϥ��������ܤˤʤ� */

	    dt = (long)((1000000.0 / (CONST_VSYNC_FREQ * wait_rate / 100)));
	    wait_vsync_setup(dt, wait_by_sleep);
	}
    }
}
int	quasi88_cfg_now_no_wait(void)
{
    return no_wait;
}
void	quasi88_cfg_set_no_wait(int enable)
{
    int time = STATUS_INFO_TIME;
    char str[32];
    long dt;

    if (no_wait != enable) {
	no_wait = enable;

	if (quasi88_is_exec()) {

	    if (no_wait) { sprintf(str, "WAIT  OFF");    time *= 10; }
	    else           sprintf(str, "WAIT  ON");

	    status_message(1, time, str);
	    /* �� �������Ȥʤ��ʤΤǡ�ɽ�����֤ϼºݤΤȤ������� */

	    dt = (long)((1000000.0 / (CONST_VSYNC_FREQ * wait_rate / 100)));
	    wait_vsync_setup(dt, wait_by_sleep);
	}
    }
}



/***********************************************************************
 * �ǥ��������᡼���ե���������
 *	��ξ�ɥ饤�֤�����
 *	������ɥ饤�֤�����
 *	��ȿ�Хɥ饤�֤Υ��᡼���ե����������
 *	��ξ�ɥ饤�ּ��Ф�
 *	������ɥ饤�ּ��Ф�
 ************************************************************************/
int	quasi88_disk_insert_all(const char *filename, int ro)
{
    int success;

    quasi88_disk_eject_all();

    success = quasi88_disk_insert(DRIVE_1, filename, 0, ro);

    if (success) {

	if (disk_image_num(DRIVE_1) > 1) {
	    quasi88_disk_insert_A_to_B(DRIVE_1, DRIVE_2, 1);
	}
    }

    if (quasi88_is_exec()) {
	status_message_default(1, NULL);
    }
    return success;
}
int	quasi88_disk_insert(int drv, const char *filename, int image, int ro)
{
    int success = FALSE;

    quasi88_disk_eject(drv);

    if (strlen(filename) < QUASI88_MAX_FILENAME) {

	if (disk_insert(drv, filename, image, ro) == 0) success = TRUE;
	else                                            success = FALSE;

	if (success) {

	    if (drv == DRIVE_1) boot_from_rom = FALSE;

	    strcpy(file_disk[ drv ], filename);
	    readonly_disk[ drv ] = ro;

	    if (filename_synchronize) {
		filename_init_state(TRUE);
		filename_init_snap(TRUE);
		filename_init_wav(TRUE);
	    }
	}
    }

    if (quasi88_is_exec()) {
	status_message_default(1, NULL);
    }
    return success;
}
int	quasi88_disk_insert_A_to_B(int src, int dst, int img)
{
    int success;

    quasi88_disk_eject(dst);

    if (disk_insert_A_to_B(src, dst, img) == 0) success = TRUE;
    else                                        success = FALSE;

    if (success) {
	strcpy(file_disk[ dst ], file_disk[ src ]);
	readonly_disk[ dst ] = readonly_disk[ src ];

	if (filename_synchronize) {
	    filename_init_state(TRUE);
	    filename_init_snap(TRUE);
	    filename_init_wav(TRUE);
	}
    }

    if (quasi88_is_exec()) {
	status_message_default(1, NULL);
    }
    return success;
}
void	quasi88_disk_eject_all(void)
{
    int drv;

    for (drv = 0; drv<2; drv++) {
	quasi88_disk_eject(drv);
    }

    boot_from_rom = TRUE;

    if (quasi88_is_exec()) {
	status_message_default(1, NULL);
    }
}
void	quasi88_disk_eject(int drv)
{
    if (disk_image_exist(drv)) {
	disk_eject(drv);
	memset(file_disk[ drv ], 0, QUASI88_MAX_FILENAME);

	if (filename_synchronize) {
	    filename_init_state(TRUE);
	    filename_init_snap(TRUE);
	    filename_init_wav(TRUE);
	}
    }

    if (quasi88_is_exec()) {
	status_message_default(1, NULL);
    }
}

/***********************************************************************
 * �ǥ��������᡼���ե���������
 *	���ɥ饤�֤���Ū�˶��ξ��֤ˤ���
 *	���ɥ饤�֤Υ��᡼�����ѹ�����
 *	���ɥ饤�֤Υ��᡼�������Υ��᡼�����ѹ�����
 *	���ɥ饤�֤Υ��᡼���򼡤Υ��᡼�����ѹ�����
 ************************************************************************/
enum { TYPE_SELECT, TYPE_EMPTY, TYPE_NEXT, TYPE_PREV };

static void disk_image_sub(int drv, int type, int img)
{
    int d;
    char str[48];

    if (disk_image_exist(drv)) {
	switch (type) {

	case TYPE_EMPTY:
	    drive_set_empty(drv);
	    sprintf(str, "DRIVE %d:  <<<< Eject >>>>         ", drv + 1);
	    break;

	case TYPE_NEXT:
	case TYPE_PREV:
	    if (type == TYPE_NEXT) d = +1;
	    else                   d = -1;

	    img = disk_image_selected(drv) + d;
	    /* FALLTHROUGH */

	default:
	    if (img < 0) img = disk_image_num(drv)-1;
	    if (img >= disk_image_num(drv)) img = 0;

	    drive_unset_empty(drv);
	    disk_change_image(drv, img);

	    sprintf(str, "DRIVE %d:  %-16s   %s  ",
		    drv + 1,
		    drive[drv].image[ disk_image_selected(drv) ].name,
		    (drive[drv].image[ disk_image_selected(drv) ].protect)
							? "(p)" : "   ");
	    break;
	}
    } else {
	sprintf(str, "DRIVE %d:   --  No Disk  --        ", drv + 1);
    }

    if (quasi88_is_exec()) {
	status_message_default(1, NULL);
    }
    status_message(1, STATUS_INFO_TIME, str);
}
void	quasi88_disk_image_select(int drv, int img)
{
    disk_image_sub(drv, TYPE_SELECT, img);
}
void	quasi88_disk_image_empty(int drv)
{
    disk_image_sub(drv, TYPE_EMPTY, 0);
}
void	quasi88_disk_image_next(int drv)
{
    disk_image_sub(drv, TYPE_NEXT, 0);
}
void	quasi88_disk_image_prev(int drv)
{
    disk_image_sub(drv, TYPE_PREV, 0);
}






/*======================================================================
 * �ơ��ץ��᡼���ե���������
 *		�������ѥơ��ץ��᡼���ե����륻�å�
 *		�������ѥơ��ץ��᡼���ե����봬���ᤷ
 *		�������ѥơ��ץ��᡼���ե������곰��
 *		���������ѥơ��ץ��᡼���ե����륻�å�
 *		���������ѥơ��ץ��᡼���ե������곰��
 *======================================================================*/
int	quasi88_load_tape_insert(const char *filename)
{
    quasi88_load_tape_eject();

    if (strlen(filename) < QUASI88_MAX_FILENAME &&
	sio_open_tapeload(filename)) {

	strcpy(file_tape[ CLOAD ], filename);
	return TRUE;

    }
    return FALSE;
}
int	quasi88_load_tape_rewind(void)
{
    if (sio_tape_rewind()) {

	return TRUE;

    }
    quasi88_load_tape_eject();
    return FALSE;
}
void	quasi88_load_tape_eject(void)
{
    sio_close_tapeload();
    memset(file_tape[ CLOAD ], 0, QUASI88_MAX_FILENAME);
}

int	quasi88_save_tape_insert(const char *filename)
{
    quasi88_save_tape_eject();

    if (strlen(filename) < QUASI88_MAX_FILENAME &&
	sio_open_tapesave(filename)) {

	strcpy(file_tape[ CSAVE ], filename);
	return TRUE;

    }
    return FALSE;
}
void	quasi88_save_tape_eject(void)
{
    sio_close_tapesave();
    memset(file_tape[ CSAVE ], 0, QUASI88_MAX_FILENAME);
}

/*======================================================================
 * ���ꥢ�롦�ѥ��륤�᡼���ե���������
 *		�����ꥢ�������ѥե����륻�å�
 *		�����ꥢ�������ѥե������곰��
 *		�����ꥢ������ѥե����륻�å�
 *		�����ꥢ������ѥե������곰��
 *		���ץ�󥿽����ѥե����륻�å�
 *		���ץ�������ѥե����륻�å�
 *======================================================================*/
int	quasi88_serial_in_connect( const char *filename )
{
  quasi88_serial_in_remove();

  if( strlen( filename ) < QUASI88_MAX_FILENAME &&
      sio_open_serialin( filename ) ){

    strcpy( file_sin, filename );
    return TRUE;

  }
  return FALSE;
}
void	quasi88_serial_in_remove( void )
{
  sio_close_serialin();
  memset( file_sin, 0, QUASI88_MAX_FILENAME );
}
int	quasi88_serial_out_connect( const char *filename )
{
  quasi88_serial_out_remove();

  if( strlen( filename ) < QUASI88_MAX_FILENAME &&
      sio_open_serialout( filename ) ){

    strcpy( file_sout, filename );
    return TRUE;

  }
  return FALSE;
}
void	quasi88_serial_out_remove( void )
{
  sio_close_serialout();
  memset( file_sout, 0, QUASI88_MAX_FILENAME );
}
int	quasi88_printer_connect( const char *filename )
{
  quasi88_printer_remove();

  if( strlen( filename ) < QUASI88_MAX_FILENAME &&
      printer_open( filename ) ){

    strcpy( file_prn, filename );
    return TRUE;

  }
  return FALSE;
}
void	quasi88_printer_remove( void )
{
  printer_close();
  memset( file_prn, 0, QUASI88_MAX_FILENAME );
}
