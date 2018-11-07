#ifndef INITVAL_H_INCLUDED
#define INITVAL_H_INCLUDED


/*----------------------------------------------------------------------*/
/* ���									*/
/*----------------------------------------------------------------------*/
#define	CONST_4MHZ_CLOCK	(3.9936)
#define	CONST_8MHZ_CLOCK	(CONST_4MHZ_CLOCK*2.0)
#define	CONST_VSYNC_FREQ	(55.4)


/*----------------------------------------------------------------------*/
/* ��ư���ν��������							*/
/*----------------------------------------------------------------------*/

enum BasicMode { BASIC_AUTO=-1, BASIC_N,    BASIC_V1S,  BASIC_V1H,  BASIC_V2 };
enum CpuClock  {                CLOCK_8MHZ, CLOCK_4MHZ  };
enum BootDevice{ BOOT_AUTO =-1, BOOT_DISK,  BOOT_ROM    };
enum SoundBoard{                SOUND_I,    SOUND_II    };
enum JishoRom  {                NOT_JISHO,  EXIST_JISHO };
enum MenuLang  {                MENU_ENGLISH, MENU_JAPAN };
enum BaudRate  { BAUDRATE_75,   BAUDRATE_150,   BAUDRATE_300,  BAUDRATE_600,
		 BAUDRATE_1200, BAUDRATE_2400,  BAUDRATE_4800, BAUDRATE_9600,
		 BAUDRATE_19200 };


/* DEFAULT_BASIC    BASIC �⡼��       -1:��ư / 0:N / 1:V1S / 2:V1H / 3:V2  */
/* DEFAULT_CLOCK    CPU����å�        -1:��ư / 0:8MHz / 1:4MHz             */
/* DEFAULT_BOOT     ��ư�ǥХ���       -1:��ư / 0:DISK / 1:ROM              */
/* DEFAULT_EXTRAM   ��ĥRAM             0:�ʤ� / 1��4:����(�����ɿ�)         */
/* DEFAULT_JISHO    ����ROM             0:�ʤ� / 1:����                      */
/* DEFAULT_SOUND    ������ɥܡ���      0:��� / 1:������ɥܡ���II���      */
/* DEFAULT_DIPSW    �ǥ��åץ����å�   16bit�ͤ����                         */
/* DEFAULT_BAUDRATE �ܡ��졼��         BADU_RATE_75 �� BADU_RATE_19200����� */

/* DEFAULT_VERBOSE         -verbose   �ν���� */
/* DEFAULT_FRAMESKIP       -frameskip �ν���� */
/* DEFAULT_CPU             -cpu       �ν���� */


#define	DEFAULT_BASIC		( BASIC_AUTO )
#define	DEFAULT_CLOCK		( CLOCK_4MHZ )
#define	DEFAULT_BOOT		( BOOT_AUTO  )
#define	DEFAULT_SOUND		( SOUND_I    )
#define	DEFAULT_JISHO		( NOT_JISHO  )
#define	DEFAULT_EXTRAM		( 0 )
#define	DEFAULT_DIPSW		( 0x391a )
#define	DEFAULT_BAUDRATE	( BAUDRATE_1200 )

#define	DEFAULT_VERBOSE		(0x00)
#define	DEFAULT_FRAMESKIP	(1)
#define	DEFAULT_CPU		(0)


/*----------------------------------------------------------------------*/
/* DEFAULT_CPU_CLOCK_MHZ	�ᥤ�� CPU�Υ���å�     double�� [MHz] */
/* DEFAULT_SOUND_CLOCK_MHZ	������ɥ��åפΥ���å� double�� [MHz] */
/* DEFAULT_VSYNC_FREQ_HZ	VSYNC �����ߤμ���     int��    [Hz]  */
/* DEFAULT_WAIT_FREQ		��������Ĵ���Ѽ��ȿ�     int��    [Hz]  */

#define	DEFAULT_CPU_CLOCK_MHZ		CONST_4MHZ_CLOCK
#define	DEFAULT_SOUND_CLOCK_MHZ		CONST_4MHZ_CLOCK
#define	DEFAULT_VSYNC_FREQ_HZ		CONST_VSYNC_FREQ
#define	DEFAULT_WAIT_FREQ_HZ		CONST_VSYNC_FREQ


/*----------------------------------------------------------------------*/
/* �ɥ饤�֤ο� ����� �ե�������Υ��᡼���κ����			*/
/*	�ɥ饤�֤ο� �� 2�ʳ����ͤ�̤�б���������Ѥ��ʤ��褦�ˡ���	*/
/*----------------------------------------------------------------------*/
enum {
  DRIVE_1,
  DRIVE_2,
  NR_DRIVE
};
#define	MAX_NR_IMAGE	(32)

enum {
  CLOAD,
  CSAVE,
  NR_TAPE
};

#endif		/* INITVAL_H_INCLUDED */
