/************************************************************************/
/*	�ǥХå���							*/
/************************************************************************/


/*----------------------------------------------------------------------
 *	�ǥХå��� printf
 *----------------------------------------------------------------------*/
#ifdef	DEBUGPRINTF
#include <stdarg.h>
void	debugprintf(const char *format, ...)
{
    va_list  list;

    va_start(list, format);
    vfprintf(stdout, format, list);
    va_end(list);

    fflush(stdout);
}
#endif

/*----------------------------------------------------------------------
 *	�ǥХå��� ��
 *----------------------------------------------------------------------*/
int	pio_debug = 0;			/* ���ʤ顢�ե��������		*/
int	fdc_debug = 0;			/* ���ʤ顢�ե��������		*/
int	main_debug = 0;			/* ���ʤ顢�ե��������		*/
int	sub_debug = 0;			/* ���ʤ顢�ե��������		*/

#ifdef	DEBUGLOG
#include <stdarg.h>
static	FILE	*LOG = NULL;

void	debuglog_init(void)
{
    LOG = fopen("quasi88.log","w");

    if (verbose_proc) {
	printf("+ Support debug logging.\n");
    }
}

void	debuglog_sync(void)
{
    fflush(LOG);
}

void	debuglog_exit(void)
{
    if (LOG) fclose(LOG);
}

void	logpio(const char *format, ...)
{
    va_list  list;
    if (pio_debug) {
	va_start(list, format);
	vfprintf(LOG, format, list);
	va_end(list);
    }
}

void	logfdc(const char *format, ...)
{
    va_list  list;
    if (fdc_debug) {
	va_start(list, format);
	vfprintf(LOG, format, list);
	va_end(list);
    }
}

static	int	z80_debug_wk;
void	logz80_target(int debug_flag)
{
    z80_debug_wk = debug_flag;
}
void	logz80( const char *format, ... )
{
    va_list  list;
    if (z80_debug_wk) {
	va_start(list, format);
	vfprintf(LOG, format, list);
	va_end(list);
    }
}
#endif

/*----------------------------------------------------------------------
 *	�������� ��ַ�¬
 *----------------------------------------------------------------------*/
int	debug_profiler;			/*
					  bit0: ��֥�åפ�ե��������
					  bit1: ��λ���˶�֥�å�ʿ��ɽ��
					  bit2: 1��������������ɽ��
					*/

#ifdef	PROFILER
#ifdef  HAVE_GETTIMEOFDAY
#include <sys/time.h>		/* gettimeofday */

/* ������ˡ
   �� ��ư���ˡ� -profiler ���ץ�����Ĥ���ȡ����Υ������������

   profiler_init();			�ǽ��1������ƤӽФ�
   profiler_exit();			�Ǹ��1������ƤӽФ�


   profiler_lapse(PROF_LAPSE_RESET);	�ǽ��RESET������������Τ�
	��
   profiler_lapse(PROF_LAPSE_CPU);	RESETľ��Ϥʤˤ⤷�ʤ�
	��
   profiler_lapse(PROF_LAPSE_SND);	CPU�������ޤǤλ��֤��¬
	��
   profiler_lapse(PROF_LAPSE_AUDIO);	SND�������ޤǤλ��֤��¬
	��
   profiler_lapse(PROF_LAPSE_INPUT);	AUDIO�������ޤǤλ��֤��¬
	��
   profiler_lapse(PROF_LAPSE_CPU2);	INPUT�������ޤǤλ��֤��¬
	��
   profiler_lapse(PROF_LAPSE_BLIT);	CPU2�������ޤǤλ��֤��¬
	��
   profiler_lapse(PROF_LAPSE_VIDEO);	BLIT�������ޤǤλ��֤��¬
	��
   profiler_lapse(PROF_LAPSE_IDLE);	VIDEO�������ޤǤλ��֤��¬
	��
   profiler_lapse(PROF_LAPSE_RESET);	IDLE�������ޤǤλ��֤��¬
					�����RESET�������ޤǤλ��֤��¬
	��
   profiler_lapse(PROF_LAPSE_CPU);	RESETľ��Ϥʤˤ⤷�ʤ�
	��
	��
   profiler_lapse(PROF_LAPSE_RESET);	
	��
   profiler_lapse(PROF_LAPSE_RESET);	RESET��Ϣ³������������������Τ�
	��
*/

static const char *prof_label[PROF_LAPSE_END] = {
    "----total----",	/* PROF_LAPSE_RESET	*/
    "CPU",		/* PROF_LAPSE_CPU	*/
    "INPUT",		/* PROF_LAPSE_INPUT	*/
    "SND",		/* PROF_LAPSE_SND	*/
    "AUDIO",		/* PROF_LAPSE_AUDIO	*/
    "CPU2",		/* PROF_LAPSE_CPU2	*/
    "BLIT",		/* PROF_LAPSE_BLIT	*/
    "VIDEO",		/* PROF_LAPSE_VIDEO	*/
    "IDLE",		/* PROF_LAPSE_IDLE	*/
};
static FILE		*prof_lap_fp;
static struct timeval	prof_lap_reset_t0;	/* ���� RESET �ƤӽФ����� */
static struct timeval	prof_lap_t0;		/* ����ƤӽФ����� */
static int              prof_lap_type;		/* ����ƤӽФ����� */
static struct {					/* �߷׾��� */
    struct timeval all;				/* �߷׻��� */
    int            count;			/* �߷ײ�� */
}	prof_lap[PROF_LAPSE_END];

void	profiler_init(void)
{
    prof_lap_fp = fopen("quasi88.lap", "w");

    if (verbose_proc) {
	printf("+ Support profiler logging.\n");
    }

    if (prof_lap_fp) {
	fprintf(prof_lap_fp, "%-16s%8ld[us]\n",
		"(vsync)",
		(long)(1000000.0/(CONST_VSYNC_FREQ * wait_rate / 100)));
    }
}

void	profiler_lapse(int type)
{
    struct timeval t1, dt;

    if (debug_profiler & 3) {

	gettimeofday(&t1, 0);

	if (prof_lap_type == PROF_LAPSE_RESET) {
	    if (type      == PROF_LAPSE_RESET) {
		/* RESET ��Ϣ³�ǸƤӽФ��줿��� (�ʤ������) �ϡ������ */
		prof_lap_reset_t0 = t1;

	    } else {   /* != PROF_LAPSE_RESET */
		/* RESET �μ��� RESET �ʳ����ƤӽФ��줿�顢�ʤˤ⤷�ʤ� */
		/* DO NOTHING */
	    }
	} else {
	    {
		/* ľ���� profiler_lapse �ƤӽФ�����ηв���� */
		dt.tv_sec  = t1.tv_sec  - prof_lap_t0.tv_sec;
		dt.tv_usec = t1.tv_usec - prof_lap_t0.tv_usec;
		if (dt.tv_usec < 0) {
		    dt.tv_sec --;
		    dt.tv_usec += 1000000;
		}

		/* �в�����߷פˡ��û����Ƥ��� */
		prof_lap[ prof_lap_type ].all.tv_sec  += dt.tv_sec;
		prof_lap[ prof_lap_type ].all.tv_usec += dt.tv_usec;
		if (prof_lap[ prof_lap_type ].all.tv_usec >= 1000000) {
		    prof_lap[ prof_lap_type ].all.tv_sec ++;
		    prof_lap[ prof_lap_type ].all.tv_usec -= 1000000;
		}
		prof_lap[ prof_lap_type ].count ++;

		/* dt ɽ�� */
		if ((debug_profiler & 1) && prof_lap_fp)
		    fprintf(prof_lap_fp, "%-13s%6ld\n",
			    prof_label[ prof_lap_type ], dt.tv_usec);
	    }

	    if (type == PROF_LAPSE_RESET) {
		/* ����� PROF_LAPSE_RESET �ƤӽФ�����ηв���� */
		dt.tv_sec  = t1.tv_sec  - prof_lap_reset_t0.tv_sec;
		dt.tv_usec = t1.tv_usec - prof_lap_reset_t0.tv_usec;
		if (dt.tv_usec < 0) {
		    dt.tv_sec --;
		    dt.tv_usec += 1000000;
		}
		prof_lap_reset_t0 = t1;

		/* �в�����߷פˡ��û����Ƥ��� */
		prof_lap[ type ].all.tv_sec  += dt.tv_sec;
		prof_lap[ type ].all.tv_usec += dt.tv_usec;
		if (prof_lap[ type ].all.tv_usec >= 1000000) {
		    prof_lap[ type ].all.tv_sec ++;
		    prof_lap[ type ].all.tv_usec -= 1000000;
		}
		prof_lap[ type ].count ++;

		/* dt ɽ�� */
		if ((debug_profiler & 1) && prof_lap_fp)
		    fprintf(prof_lap_fp, "%-13s%6ld\n",
			    prof_label[ type ], dt.tv_usec);
	    }
	}
	prof_lap_t0 = t1;
	prof_lap_type = type;
    }
}
void	profiler_exit(void)
{
    int i;
    double d;

    if (debug_profiler & 2) {

	printf("\n*** profiler ***\n");

	for (i = 0; i < PROF_LAPSE_END; i++) {

	    d = prof_lap[i].all.tv_sec + (prof_lap[i].all.tv_usec / 1000000.0);

	    printf("%-16s%5d[times], %4d.%06ld[sec] (ave = %f[sec])\n",
		   prof_label[i], prof_lap[i].count,
		   (int) prof_lap[i].all.tv_sec, prof_lap[i].all.tv_usec,
		   d / prof_lap[i].count);
	}
	printf("\n");
    }

    if (prof_lap_fp) {
	fclose(prof_lap_fp);
	prof_lap_fp = NULL;
    }
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* ����¾�λ�¿�ʡ����ַ�¬�ǥХå��ؿ� */

/* 1970/01/01 ������ÿ��� usec ��ɽ�� */
void	profiler_current_time(void)
{
    struct timeval tv;
    if (gettimeofday(&tv, 0) == 0) {
	printf("%d.%06ld\n", (int) tv.tv_sec, tv.tv_usec);
    }
}


/* profiler_watch_start �� profiler_watch_stop �ޤǤλ��֤� usec ��ɽ��  */
static struct timeval watch_t0;
void	profiler_watch_start(void)
{
    gettimeofday(&watch_t0, 0);
}

void	profiler_watch_stop(void)
{
    static struct timeval watch_t1, dt;

    gettimeofday(&watch_t1, 0);

    dt.tv_sec  = watch_t1.tv_sec  - watch_t0.tv_sec;
    dt.tv_usec = watch_t1.tv_usec - watch_t0.tv_usec;
    if (dt.tv_usec < 0) {
	dt.tv_sec --;
	dt.tv_usec += 1000000; 
    }

    printf("%d.%06ld\n", (int) dt.tv_sec, dt.tv_usec);
}


#else
void	profiler_init(void) {}
void	profiler_lapse(int type) {}
void	profiler_exit(void) {}
void	profiler_current_time(void) {}
void	profiler_watch_start(void) {}
void	profiler_watch_stop(void) {}
#endif

void	profiler_video_output(int timing, int skip, int drawn)
{
    static int n;

    if (debug_profiler & 4) {
	if (timing) {
	    if (skip == FALSE) {
		if (drawn) printf("@"); /* ���������η�̡�������ɬ�פ��ä� */
		else       printf("o"); /* ���������η�̡����������פ��ä� */
	    } else         printf("-"); /* ���֤��ʤ��Τǡ������åפ���     */
	} else             printf(" "); /* ����ϡ������������ʤ��ä�       */

	if (++n > 56){
	    n=0;
	    printf("\n");
	    fflush(stdout);
	}
    }
}

#endif
