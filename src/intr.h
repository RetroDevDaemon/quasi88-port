#ifndef INTR_H_INCLUDED
#define INTR_H_INCLUDED


extern	int	intr_level;			/* OUT[E4] �����ߥ�٥� */
extern	int	intr_priority;			/* OUT[E4] ������ͥ���� */
extern	int	intr_sio_enable;		/* OUT[E6] ����ޥ��� SIO */ 
extern	int	intr_vsync_enable;		/* OUT[E6] ����ޥ���VSYNC*/ 
extern	int	intr_rtc_enable;		/* OUT[E6] ����ޥ��� RTC */ 



extern	double	cpu_clock_mhz;		/* �ᥤ�� CPU�Υ���å�     [MHz] */
extern	double	sound_clock_mhz;	/* ������ɥ��åפΥ���å� [MHz] */
extern	double	vsync_freq_hz;		/* VSYNC �����ߤμ���	    [Hz]  */


extern	int	state_of_cpu;			/*�ᥤ��CPU����������̿��� */
extern	int	state_of_vsync;			/* VSYNC�����Υ��ơ��ȿ�   */

extern	int	wait_rate;			/* ��������Ĵ�� ��Ψ    [%]  */
extern	int	wait_by_sleep;			/* ��������Ĵ���� sleep ���� */

extern	int	no_wait;			/* �������Ȥʤ�		*/

extern	int	boost;				/* �֡�����		*/
extern	int	boost_cnt;			/* 			*/




extern	int	ctrl_vrtc;			/* 1:��ľ������  0: ɽ���� */

extern	int	VSYNC_flag;			/* �Ƽ�����߿���ե饰 */
extern	int	RTC_flag;
extern	int	SOUND_flag;
extern	int	RS232C_flag;





void	interval_work_init_all( void );

void	interval_work_set_RS232C( int bps, int framesize );

void	interval_work_set_TIMER_A( void );
void	interval_work_set_TIMER_B( void );

void	interval_work_set_BDRY( void );
void	interval_work_set_EOS( int length );

void	change_sound_flags( int port );
void	change_sound_prescaler( int new_prescaler );

void	boost_change( int new_val );


void	main_INT_init( void );
void	main_INT_update( void );
int	main_INT_chk( void );

int	quasi88_info_vsync_count(void);

#endif	/* INTR_H_INCLUDED */
