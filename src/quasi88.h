#ifndef QUASI88_H_INCLUDED
#define QUASI88_H_INCLUDED


/*----------------------------------------------------------------------*/
/* �����ƥࡦ�Ķ���¸�����						*/
/*----------------------------------------------------------------------*/
#include "config.h"


/*----------------------------------------------------------------------*/
/* �ե����륷���ƥ��¸�����						*/
/*----------------------------------------------------------------------*/
#include "filename.h"


/* QUASI88 ���������ݻ���ǽ�ʥѥ����ߤΥե�����̾�Х��ȿ� (NUL�ޤ�) */
#define	QUASI88_MAX_FILENAME	(1024)


/*----------------------------------------------------------------------*/
/* �С���������							*/
/*----------------------------------------------------------------------*/
#include "version.h"



/*----------------------------------------------------------------------*/
/* �������								*/
/*----------------------------------------------------------------------*/

typedef	unsigned char	Uchar;
typedef	unsigned short	Ushort;
typedef	unsigned int	Uint;
typedef	unsigned long	Ulong;

typedef unsigned char  byte;
typedef unsigned short word;
typedef signed   char  offset;


typedef unsigned char  bit8;
typedef unsigned short bit16;
typedef unsigned int   bit32;


#define	COUNTOF(arr)	(int)(sizeof(arr)/sizeof((arr)[0]))
#define	OFFSETOF(s, m)	((size_t)(&((s *)0)->m))
#undef  MAX
#define	MAX(a,b)	(((a)>(b))?(a):(b))
#undef  MIN
#define	MIN(a,b)	(((a)<(b))?(a):(b))
#undef  ABS
#define	ABS(x)		(((x) >= 0)? (x) : -(x))
#define	SGN(x)		(((x) > 0) ? 1 : (((x) < 0) ? -1 : 0)) 
#define	BETWEEN(l,x,h)	((l) <= (x) && (x) <= (h))


#ifdef LSB_FIRST			/* ��ȥ륨��ǥ����� */

typedef union
{
  struct { byte l,h; }	B;
  word			W;
} pair;

#else					/* �ӥå�����ǥ����� */

typedef union
{
  struct { byte h,l; }	B;
  word			W;
} pair;

#endif



#ifndef TRUE
#define	TRUE	(1)
#endif
#ifndef FALSE
#define	FALSE	(0)
#endif



#ifndef	INLINE
#define	INLINE	static
#endif



/*----------------------------------------------------------------------*/
/* �ѿ� (verbose_*)���ؿ�						*/
/*----------------------------------------------------------------------*/
extern	int	verbose_level;		/* ��Ĺ��٥�			*/
extern	int	verbose_proc;		/* �����οʹԾ�����ɽ��		*/
extern	int	verbose_z80;		/* Z80�������顼��ɽ��		*/
extern	int	verbose_io;		/* ̤���� I/O�������������	*/
extern	int	verbose_pio;		/* PIO ���������Ѥ�ɽ��		*/
extern	int	verbose_fdc;		/* FD���᡼���۾�����		*/
extern	int	verbose_wait;		/* ���������Ԥ����ΰ۾�����	*/
extern	int	verbose_suspend;	/* �����ڥ�ɻ��ΰ۾�����	*/
extern	int	verbose_snd;		/* ������ɤΥ�å�����		*/


enum {
    EVENT_NONE		= 0x0000,
    EVENT_FRAME_UPDATE	= 0x0001,
    EVENT_AUDIO_UPDATE	= 0x0002,
    EVENT_MODE_CHANGED	= 0x0004,
    EVENT_DEBUG		= 0x0008,
    EVENT_QUIT		= 0x0010
};
extern	int	quasi88_event_flags;
extern	int	quasi88_debug_pause;	/* 1�ʤ�pause, 0�ʤ�monitor */

enum EmuMode
{
    EXEC,

    GO,
    TRACE,
    STEP,
    TRACE_CHANGE,

    MONITOR,
    MENU,
    PAUSE,

    QUIT
};


#define	INIT_POWERON	(0)
#define	INIT_RESET	(1)
#define	INIT_STATELOAD	(2)

void	quasi88(void);

void	quasi88_start(void);
void	quasi88_main(void);
void	quasi88_stop(int normal_exit);
enum {
    QUASI88_LOOP_EXIT,
    QUASI88_LOOP_ONE,
    QUASI88_LOOP_BUSY
};
int	quasi88_loop(void);

void	quasi88_atexit(void (*function)(void));
void	quasi88_exit(int status);

void	quasi88_exec(void);
void	quasi88_exec_step(void);
void	quasi88_exec_trace(void);
void	quasi88_exec_trace_change(void);
void	quasi88_menu(void);
void	quasi88_pause(void);
void	quasi88_monitor(void);
void	quasi88_debug(void);
void	quasi88_quit(void);
int	quasi88_is_exec(void);
int	quasi88_is_menu(void);
int	quasi88_is_pause(void);
int	quasi88_is_monitor(void);



/*----------------------------------------------------------------------*/
/* ����¾	(���Τϡ� quasi88.c  �ˤ�������Ƥ���)			*/
/*----------------------------------------------------------------------*/
void	wait_vsync_switch(void);

void	sjis2euc( char *euc_p, const char *sjis_p );
void	euc2sjis( char *sjis_p, const char *euc_p );
int	euclen( const char *euc_p );
int	my_strcmp( const char *s, const char *d );
void	my_strncpy( char *s, const char *ct, unsigned long n );
void	my_strncat( char *s, const char *ct, unsigned long n );
char	*my_strtok( char *dst, char *src );



/*----------------------------------------------------------------------*/
/*									*/
/*----------------------------------------------------------------------*/
const char	*filename_get_disk(int drv);
const char	*filename_get_tape(int mode);
const char	*filename_get_prn(void);
const char	*filename_get_sin(void);
const char	*filename_get_sout(void);
const char	*filename_get_disk_or_dir(int drv);
const char	*filename_get_tape_or_dir(int mode);
const char	*filename_get_disk_name(int drv);
const char	*filename_get_tape_name(int mode);

char	*filename_alloc_diskname(const char *filename);
char	*filename_alloc_romname(const char *filename);
char	*filename_alloc_global_cfgname(void);
char	*filename_alloc_local_cfgname(const char *imagename);
char	*filename_alloc_keyboard_cfgname(void);



/*----------------------------------------------------------------------*/
/*	�ǥХå���							*/
/*----------------------------------------------------------------------*/
#ifdef	DEBUGPRINTF
void	debugprintf(const char *format, ...);
#define	XPRINTF	debugprintf
#else
#define	XPRINTF	if(1){}else printf
#endif

extern	int	pio_debug;
extern	int	fdc_debug;
extern	int	main_debug;
extern	int	sub_debug;

#ifdef	DEBUGLOG
void	debuglog_init(void);
void	debuglog_sync(void);
void	debuglog_exit(void);

void	logpio(const char *format, ...);
void	logfdc(const char *format, ...);
void	logz80(const char *format, ...);
void	logz80_target(int debug_flag);
#else

#define	debuglog_init()
#define	debuglog_sync()
#define	debuglog_exit()
#define	logpio	if(1){}else printf
#define	logfdc	if(1){}else printf
#define	logz80	if(1){}else printf
#define	logz80_target(x)
#endif


extern	int	debug_profiler;

#ifdef	PROFILER
enum {
    PROF_LAPSE_RESET,
    PROF_LAPSE_CPU,
    PROF_LAPSE_INPUT,
    PROF_LAPSE_SND,
    PROF_LAPSE_AUDIO,
    PROF_LAPSE_CPU2,
    PROF_LAPSE_BLIT,
    PROF_LAPSE_VIDEO,
    PROF_LAPSE_IDLE,
    PROF_LAPSE_END
};
void	profiler_init(void);
void	profiler_lapse(int type);
void	profiler_exit(void);
void	profiler_current_time(void);
void	profiler_watch_start(void);
void	profiler_watch_stop(void);
void	profiler_video_output(int timing, int skip, int drawn);
#else
#define	profiler_init()
#define	profiler_lapse(i)
#define	profiler_exit()
#define	profiler_current_time()
#define	profiler_watch_start()
#define	profiler_watch_stop()
#define	profiler_video_output(t,s,d)
#endif

#endif		/* QUASI88_H_INCLUDED */
