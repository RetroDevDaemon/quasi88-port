/************************************************************************/
/*									*/
/* PC8801 �ᥤ�󥷥��ƥ�(����¦)					*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "quasi88.h"
#include "initval.h"
#include "pc88main.h"

#include "pc88cpu.h"
#include "crtcdmac.h"
#include "screen.h"
#include "intr.h"
#include "keyboard.h"
#include "memory.h"
#include "pio.h"
#include "soundbd.h"
#include "fdc.h"		/* disk_ex_drv */

#include "event.h"
#include "emu.h"
#include "drive.h"
#include "snddrv.h"
#include "suspend.h"
#include "status.h"



static	OSD_FILE *fp_so = NULL;		/* ���ꥢ�������fp		*/
static	OSD_FILE *fp_si = NULL;		/*	   ������fp		*/
static	OSD_FILE *fp_to = NULL;		/* �ơ��׽�����  fp		*/
static	OSD_FILE *fp_ti = NULL;		/*       ������  fp		*/
static	OSD_FILE *fp_prn= NULL;		/* �ץ�󥿽�����fp		*/




int	boot_basic     =DEFAULT_BASIC;	/* ��ư���� BASIC�⡼��		*/
int	boot_dipsw     =DEFAULT_DIPSW;	/* ��ư���Υǥ��å�����		*/
int	boot_from_rom  =DEFAULT_BOOT;	/* ��ư�ǥХ���������		*/
int	boot_clock_4mhz=DEFAULT_CLOCK;	/* ��ư���� CPU����å�		*/

int	monitor_15k    =0x00;		/* 15k ��˥��� 2:Yes 0:No	*/

z80arch	z80main_cpu;			/* Z80 CPU ( main system )	*/

int	high_mode;			/* ��®�⡼�� 1:Yes 0:No	*/


static	byte	dipsw_1;		/* IN[30] �ǥ��åץ����å� 1	*/
static	byte	dipsw_2;		/* IN[31] �ǥ��åץ����å� 2	*/
static	byte	ctrl_boot;		/* IN[40] �ǥ������֡��Ⱦ���	*/
static	byte	cpu_clock;		/* IN[6E] CPU ����å�		*/

int	memory_bank;			/* OUT[5C-5F] IN[5C] ����Х�*/

static	byte	common_out_data;	/* OUT[10] PRT/����		*/
byte	misc_ctrl;			/* I/O[32] �Ƽ�Ctrl		*/
byte	ALU1_ctrl;			/* OUT[34] ALU Ctrl 1		*/
byte	ALU2_ctrl;			/* OUT[35] ALU Ctrl 2		*/
byte	ctrl_signal;			/* OUT[40] ����ȥ��뿮���������¸*/
byte	baudrate_sw = DEFAULT_BAUDRATE;	/* I/O[6F] �ܡ��졼��		*/
word	window_offset;			/* I/O[70] WINDOW ���ե��å�	*/
byte	ext_rom_bank;			/* I/O[71] ��ĥROM BANK		*/
byte	ext_ram_ctrl;			/* I/O[E2] ��ĥRAM����		*/
byte	ext_ram_bank;			/* I/O[E3] ��ĥRAM���쥯��	*/

static	pair	kanji1_addr;		/* OUT[E8-E9] ����ROM(��1) ADDR	*/
static	pair	kanji2_addr;		/* OUT[EC-ED] ����ROM(��2) ADDR	*/

byte	jisho_rom_bank;			/* OUT[F0] ����ROM���쥯��	*/
byte	jisho_rom_ctrl;			/* OUT[F1] ����ROM�Х�	*/


int	calendar_stop = FALSE;		/* ������ߥե饰		*/
static	char	calendar_data[7] =	/* ������߻��� (ǯ�����˻�ʬ��)*/
{ 85, 0, 1, 0, 0, 0, 0, };


int	cmt_speed = 0;			/* �ơ���®��(BPS)�� 0�ϼ�ư	*/
int	cmt_intr  = TRUE;		/* ���ǡ��ơ����ɹ��˳������	*/
int	cmt_wait  = TRUE;		/* ���ǡ��ơ����ɹ��������Ȥ���	*/


int	highspeed_mode = FALSE;		/* ���ǡ���® BASIC �������� 	*/


int	use_siomouse = FALSE;		/* ���ǡ����ꥢ��ޥ�������	*/


/* �ʲ��ϥơ��ץ��᡼���Υե������¸����ʤΤǡ����ơ��ȥ����֤��ʤ� */

static	int	cmt_is_t88;		/* ����T88������CMT		*/
static	int	cmt_block_size;		/* �ǡ���������Υ�����(T88)	*/
static	long	cmt_size;		/* ���᡼���Υ�����		*/
static	int	cmt_EOF = FALSE;	/* ���ǡ��ơ������� EOF   	*/
static	int	com_EOF = FALSE;	/* ���ǡ����ꥢ������ EOF 	*/
static	long	com_size;		/* ���᡼���Υ�����		*/


static byte sio_in_data( void );	/* IN[20] RS232C���� (�ǡ���)	*/
static byte sio_in_status( void );	/* IN[21] RS232C���� (����)	*/
static byte in_ctrl_signal( void );	/* IN[40] ����ȥ��뿮������	*/

static void sio_out_data( byte );	/* OUT[20] RS232C���� (�ǡ���)	*/
static void sio_out_command( byte );	/* OUT[21] RS232C���� (���ޥ��)*/
static void out_ctrl_signal( byte );	/* OUT[40] ����ȥ��뿮�����	*/

static void sio_tape_highspeed_load( void );
static void sio_set_intr_base( void );
static void sio_check_cmt_error( void );

#define	sio_tape_readable()	(fp_ti && !cmt_EOF)	/* �ơ����ɹ��ġ�   */
#define	sio_tape_writable()	(fp_to)			/* �ơ��׽���ġ�   */
#define	sio_serial_readable()	(fp_si && !com_EOF)	/* ���ꥢ���ɹ��ġ� */
#define	sio_serial_writable()	(fp_so)			/* ���ꥢ�����ġ� */


/************************************************************************/
/* ���ꥦ������							*/
/************************************************************************/
/*
 * �ޤ��������б�����Τ���ˤʤꤽ���Ǥ������Ȥꤢ����
 *
 *	��®�⡼�ɤǤ� M1 �������� (�ե��å���� 1���ơ���? )
 *	��®�⡼�ɤǤ� DMA�Υ������� ( 1�Х���9���ơ���? �� VSYNC��? )
 *	��®�⡼�ɤǡ���®RAM�Ǥ� M1 �������� (�ե��å���� 1���ơ���? )
 *	���֥����ƥ�Ǥ� M1 �������� (�ե��å���� 1���ơ���? )
 *
 * �������Ŭ��������Ƥߤ뤳�Ȥˤ��ޤ���
 */

#define	DMA_WAIT	(9)

static	int	mem_wait_highram = FALSE;


/************************************************************************/
/* PCG-8100								*/
/************************************************************************/
static	int	pcg_data;
static	int	pcg_addr;

static	void	pcg_out_data( byte data )
{
  pcg_data = data;
}

static	void	pcg_out_addr_low( byte addr )
{
  pcg_addr = (pcg_addr & 0xff00) | addr;
}

static	void	pcg_out_addr_high( byte addr )
{
  byte src;

  pcg_addr = (pcg_addr & 0x00ff) | ((int)addr << 8);

  if( addr & 0x10 ){	/* exec */

    if( addr & 0x20 ){ src = font_mem[ 0x400 + (pcg_addr&0x3ff) ]; } /* copy */
    else             { src = pcg_data; }			    /* store */

    font_pcg[ 0x400 + (pcg_addr&0x3ff) ] = src;
  }
}



/************************************************************************/
/* ��® BASIC �⡼��							*/
/************************************************************************/
/*
 * ��® BASIC �����ϡ�peach��ˤ���󶡤���ޤ�����
 */

static	word	ret_addr = 0xffff;
static	int	hs_icount = 0;

	int highspeed_flag = FALSE;	/* ���ߡ���®BASIC ������	*/
static	int highspeed_n88rom = FALSE;	/* MAIN-ROM �Х����������	*/
					/* (���λ�����®BASIC ������ǽ)	*/

/* ��® BASIC �⡼�ɤ�����Ȥ��Υ��ɥ쥹 (BIOS��¸����?) */
word highspeed_routine[] = {
    0x6e9a,			/* PSET   */
    0x6eae,			/* LINE   */
    0x6eca,			/* ROLL   */
    0x6ece,			/* CIRCLE */
    0x6eda,			/* PAINT  */
    0x7198,			/* GET@   */
    0x71a6,			/* PUT@   */
    EndofBasicAddr
};


/************************************************************************/
/* ���ꥢ������							*/
/*					special thanks	�޾���줵��	*/
/*							peach ����	*/
/************************************************************************/
/*
   �ᥤ�����ϥХ��ڤ��ؤ��ˤ�äơ��ʲ��Τ褦�˥ޥåԥ󥰤���롣

   0000	+------++------+				+------+ +------+
	|      ||      |				|      | |      |+
	|      ||      |				|      | |      ||
	|      || MAIN |				|N-    | | EXT  ||
	|      || ROM  |				| BASIC| | RAM  ||
	|      ||      |				|  ROM | |      ||
   6000	+      ++      ++------++------++------++------++      + | (x4) ||
	|      ||      ||Ext.0 ||Ext.1 ||Ext.2 ||Ext.3 ||      | |      ||
   8000	+ MAIN ++------++------++------++------++------++------+ +------+|
	| RAM  ||Window|                                          +------+
   8400	+      ++------+
	|      |
	|      |
   C000	+      +	+------++------++------+                 +------+
	|      |	|      ||      ||      |		 |      |+
	|      |	| VRAM || VRAM || VRAM |		 | ���� ||
   F000	+      ++------+|   B  ||   R  ||   G  |		 | ROM  ||
	|      || High ||      ||      ||      |		 | (x32)||
   FFFF	+------++------++------++------++------+		 +------+|
								  +------+
   �Ĥޤꡢ�礭��ʬ����ȡ��ʲ���6�ĤΥ��ꥢ��ʬ�����롣

	0000H��5FFFH	MAIN RAM / MAIN ROM / N-BASIC ROM / ��ĥRAM
	6000H��7FFFH	MAIN RAM / MAIN ROM / ��ĥROM / N-BASIC ROM / ��ĥRAM
	8000H��83FFH	MAIN RAM / ������ɥ�
	8400H��BFFFH	MAIN RAM
	C000H��EFFFH	MAIN RAM / VRAM / ����ROM
	F000H��FFFFH	MAIN RAM / ��®RAM / VRAM / ����ROM

   �Х��ڤ��ؤ���Ԥʤä����ˡ��ơ��Υ��ꥢ���ɤΥХ󥯤˳�����Ƥ�줿�Τ�
   ������å������ºݤΥ��ꥢ�������Ϥ��γ�����ƾ���ˤ��Ԥʤ���


   ��)
   H�⡼�ɤˤ����Ƥϡ� 0xf000 �� 0xffff ���Ϥ�ʲ��Τ褦�˥��ߥ�졼�Ȥ��롣

	��®RAM��    main_ram[ 0xf000 �� 0xffff ]      ��Ȥ�
	�ᥤ��RAM��  main_high_ram[ 0x0000 �� 0x0fff ] ��Ȥ�

   ����ˤ�ꡢ�ƥ�����ɽ�������Ͼ�� main_ram �򻲾Ȥ���Ф褤���Ȥˤʤ롣
*/


static	byte	*read_mem_0000_5fff;	/* �ᥤ����� �꡼�ɥݥ���	*/
static	byte	*read_mem_6000_7fff;
static	byte	*read_mem_8000_83ff;
static	byte	*read_mem_c000_efff;
static	byte	*read_mem_f000_ffff;

static	byte	*write_mem_0000_7fff;	/* �ᥤ����� �饤�ȥݥ���	*/
static	byte	*write_mem_8000_83ff;
static	byte	*write_mem_c000_efff;
static	byte	*write_mem_f000_ffff;

/*------------------------------------------------------*/
/* address : 0x0000 �� 0x7fff �� ����������		*/
/*		ext_ram_ctrl, ext_ram_bank, grph_ctrl,	*/
/*		ext_rom_bank, misc_ctrl �ˤ���Ѳ�	*/
/*------------------------------------------------------*/
#if 1
INLINE	void	main_memory_mapping_0000_7fff( void )
{
  highspeed_n88rom = FALSE;	/* �ǥե���� */

  switch( ext_ram_ctrl ){

  case 0x00:					/* ��ĥRAM RW�Բ� */
    if( grph_ctrl&GRPH_CTRL_64RAM ){			/* 64KB RAM mode */
      read_mem_0000_5fff  = &main_ram[ 0x0000 ];
      read_mem_6000_7fff  = &main_ram[ 0x6000 ];
      write_mem_0000_7fff = &main_ram[ 0x0000 ];
    }else{						/* ROM/RAM mode */
      if( grph_ctrl&GRPH_CTRL_N ){				/* N BASIC */
	read_mem_0000_5fff = &main_rom_n[ 0x0000 ];
	read_mem_6000_7fff = &main_rom_n[ 0x6000 ];
      }else{							/*N88 BASIC*/
	read_mem_0000_5fff = &main_rom[ 0x0000 ];
	if( ext_rom_bank&EXT_ROM_NOT ){				/* �̾�ROM */
	  read_mem_6000_7fff = &main_rom[ 0x6000 ];
	  highspeed_n88rom = TRUE;
	}else{							/* ��ĥROM */
	  read_mem_6000_7fff = &main_rom_ext[ misc_ctrl&MISC_CTRL_EBANK ][0];
	}
      }
      write_mem_0000_7fff = &main_ram[ 0x0000 ];
    }
    break;

  case 0x01:					/* ��ĥRAM R�� W�Բ� */
    if( ext_ram_bank < use_extram*4 ){
      read_mem_0000_5fff = &ext_ram[ ext_ram_bank ][ 0x0000 ];
      read_mem_6000_7fff = &ext_ram[ ext_ram_bank ][ 0x6000 ];
    }else{
      read_mem_0000_5fff = dummy_rom;
      read_mem_6000_7fff = dummy_rom;
    }
    write_mem_0000_7fff = &main_ram[ 0x0000 ];
    break;

  case 0x10:					/* ��ĥRAM R�Բ� W��  */
		/* buf fix by peach (thanks!) */
    if( grph_ctrl&GRPH_CTRL_64RAM ){			/* 64KB RAM mode */
      read_mem_0000_5fff  = &main_ram[ 0x0000 ];
      read_mem_6000_7fff  = &main_ram[ 0x6000 ];
    }else{						/* ROM/RAM mode */
      if( grph_ctrl&GRPH_CTRL_N ){				/* N BASIC */
	read_mem_0000_5fff = &main_rom_n[ 0x0000 ];
	read_mem_6000_7fff = &main_rom_n[ 0x6000 ];
      }else{							/*N88 BASIC*/
	read_mem_0000_5fff = &main_rom[ 0x0000 ];
	if( ext_rom_bank&EXT_ROM_NOT ){				/* �̾�ROM */
	  read_mem_6000_7fff = &main_rom[ 0x6000 ];
	  highspeed_n88rom = TRUE;
	}else{							/* ��ĥROM */
	  read_mem_6000_7fff = &main_rom_ext[ misc_ctrl&MISC_CTRL_EBANK ][0];
	}
      }
    }
    if( ext_ram_bank < use_extram*4 ){
      write_mem_0000_7fff = &ext_ram[ ext_ram_bank ][ 0x0000 ];
    }else{
      write_mem_0000_7fff = dummy_ram;
    }
    break;

  case 0x11:					/* ��ĥRAM RW�� */
    if( ext_ram_bank < use_extram*4 ){
      read_mem_0000_5fff  = &ext_ram[ ext_ram_bank ][ 0x0000 ];
      read_mem_6000_7fff  = &ext_ram[ ext_ram_bank ][ 0x6000 ];
      write_mem_0000_7fff = &ext_ram[ ext_ram_bank ][ 0x0000 ];
    }else{
      read_mem_0000_5fff  = dummy_rom;
      read_mem_6000_7fff  = dummy_rom;
      write_mem_0000_7fff = dummy_ram;
    }
    break;
  }
}

#else	/* ���������ä��ꤵ����ۤ��������� */

INLINE	void	main_memory_mapping_0000_7fff( void )
{
  highspeed_n88rom = FALSE;	/* �ǥե���� */

	/* �꡼�ɤϡ����ꤷ���Х󥯤˱��������꤫�� */

				/* buf fix by peach (thanks!) */
  if( grph_ctrl&GRPH_CTRL_64RAM ){			/* 64KB RAM mode */
    read_mem_0000_5fff  = &main_ram[ 0x0000 ];
    read_mem_6000_7fff  = &main_ram[ 0x6000 ];
  }else{						/* ROM/RAM mode */
    if( grph_ctrl&GRPH_CTRL_N ){				/* N BASIC */
      read_mem_0000_5fff = &main_rom_n[ 0x0000 ];
      read_mem_6000_7fff = &main_rom_n[ 0x6000 ];
    }else{							/*N88 BASIC*/
      read_mem_0000_5fff = &main_rom[ 0x0000 ];
      if( ext_rom_bank&EXT_ROM_NOT ){				/* �̾�ROM */
	read_mem_6000_7fff = &main_rom[ 0x6000 ];
	highspeed_n88rom = TRUE;
      }else{							/* ��ĥROM */
	read_mem_6000_7fff = &main_rom_ext[ misc_ctrl&MISC_CTRL_EBANK ][0];
      }
    }
  }

	/* �饤�Ȥϡ���˥ᥤ��RAM �� */

  write_mem_0000_7fff = &main_ram[ 0x0000 ];



	/* ��ĥRAM�ؤΥ����������꤬����С���ĥRAM��꡼�ɡ��饤�Ȥ��� */

  if( ext_ram_ctrl & 0x01 ){				/* ��ĥRAM R�� */
    if( ext_ram_bank < use_extram*4 ){
      read_mem_0000_5fff = &ext_ram[ ext_ram_bank ][ 0x0000 ];
      read_mem_6000_7fff = &ext_ram[ ext_ram_bank ][ 0x6000 ];
    }
  }

  if( ext_ram_ctrl & 0x10 ){				/* ��ĥRAM W�� */
    if( ext_ram_bank < use_extram*4 ){
      write_mem_0000_7fff = &ext_ram[ ext_ram_bank ][ 0x0000 ];
    }
  }
}
#endif


/*------------------------------------------------------*/
/* address : 0x8000 �� 0x83ff �� ����������		*/
/*		grph_ctrl, window_offset �ˤ���Ѳ�	*/
/*------------------------------------------------------*/
INLINE	void	main_memory_mapping_8000_83ff( void )
{
  if( grph_ctrl & ( GRPH_CTRL_64RAM | GRPH_CTRL_N ) ){
    read_mem_8000_83ff  = &main_ram[ 0x8000 ];
    write_mem_8000_83ff = &main_ram[ 0x8000 ];
  }else{
    if( high_mode ){
      if( window_offset <= 0xf000 - 0x400 ){
	read_mem_8000_83ff  = &main_ram[ window_offset ];
	write_mem_8000_83ff = &main_ram[ window_offset ];
      }else if( 0xf000 <= window_offset && window_offset <= 0x10000 - 0x400 ){
	read_mem_8000_83ff  = &main_high_ram[ window_offset - 0xf000 ];
	write_mem_8000_83ff = &main_high_ram[ window_offset - 0xf000 ];
      }else{
	read_mem_8000_83ff  = NULL;
	write_mem_8000_83ff = NULL;
      }
    }else{
      read_mem_8000_83ff  = &main_ram[ window_offset ];
      write_mem_8000_83ff = &main_ram[ window_offset ];
    }
  }
}


/*------------------------------------------------------*/
/* address : 0xc000 �� 0xffff �� ����������		*/
/*		jisho_rom_ctrl, jisho_rom_bank, 	*/
/*		misc_ctrl �ˤ���Ѳ�			*/
/*------------------------------------------------------*/
INLINE	void	main_memory_mapping_c000_ffff( void )
{
  mem_wait_highram = FALSE;

  if( jisho_rom_ctrl ){
    read_mem_c000_efff = &main_ram[ 0xc000 ];
    if( high_mode && (misc_ctrl&MISC_CTRL_TEXT_MAIN) ){
      read_mem_f000_ffff = &main_high_ram[ 0x0000 ];
      mem_wait_highram = TRUE;
    }else{
      read_mem_f000_ffff = &main_ram[ 0xf000 ];
    }
  }else{
    read_mem_c000_efff = &jisho_rom[ jisho_rom_bank ][ 0x0000 ];
    read_mem_f000_ffff = &jisho_rom[ jisho_rom_bank ][ 0x3000 ];
  }

  write_mem_c000_efff = &main_ram[ 0xc000 ];
  if( high_mode && (misc_ctrl&MISC_CTRL_TEXT_MAIN) ){
    write_mem_f000_ffff = &main_high_ram[ 0x0000 ];
    mem_wait_highram = TRUE;
  }else{
    write_mem_f000_ffff = &main_ram[ 0xf000 ];
  }
}


/*------------------------------------------------------*/
/* address : 0xc000 �� 0xffff �� �ᥤ�󢫢�VARM�ڤ��ؤ�	*/
/*		misc_ctrl, ALU2_ctrl,			*/
/*		memory_bank �ˤ���Ѳ�			*/
/*------------------------------------------------------*/
static	int	vram_access_way;	/* vram ������������ˡ	*/
enum VramAccessWay{
  VRAM_ACCESS_BANK,
  VRAM_ACCESS_ALU,
  VRAM_NOT_ACCESS,
  EndofVramAcc
};

INLINE	void	main_memory_vram_mapping( void )
{
  if( misc_ctrl & MISC_CTRL_EVRAM ){		/* ��ĥ���������⡼�� */

    /* ��ɥ饴��ǻ��� (port 35H �����Ϥ���ʤ������) by peach */
    memory_bank = MEMORY_BANK_MAIN;

    if( ALU2_ctrl & ALU2_CTRL_VACCESS ){		/* VRAM��ĥ�������� */
      vram_access_way = VRAM_ACCESS_ALU;
    }else{						/* MAIN RAM�������� */
      vram_access_way = VRAM_NOT_ACCESS;
    }
  }else{					/* ��Ω���������⡼�� */
    if( memory_bank == MEMORY_BANK_MAIN ){		/* MAIN RAM�������� */
      vram_access_way = VRAM_NOT_ACCESS;
    }else{						/* VRAM��������     */
      vram_access_way = VRAM_ACCESS_BANK;
    }
  }
}




/*------------------------------*/
/* �̾�Σ֣ң��ͥ꡼��		*/
/*------------------------------*/
INLINE	byte	vram_read( word addr )
{
  return main_vram[addr][ memory_bank ];
}

/*------------------------------*/
/* �̾�Σ֣ң��ͥ饤��		*/
/*------------------------------*/
INLINE	void	vram_write( word addr, byte data )
{
  screen_set_dirty_flag(addr);

  main_vram[addr][ memory_bank ] = data;
}

/*------------------------------*/
/* ���̣դ�𤷤��֣ң��ͥ꡼��	*/
/*------------------------------*/
typedef	union {
  bit8		c[4];
  bit32		l;
} ALU_memory;

static	ALU_memory	ALU_buf;
static	ALU_memory	ALU_comp;

#ifdef LSB_FIRST
#define	set_ALU_comp()						\
	do{							\
	  ALU_comp.l = 0;					\
	  if( (ALU2_ctrl&0x01)==0 ) ALU_comp.l |= 0x000000ff;	\
	  if( (ALU2_ctrl&0x02)==0 ) ALU_comp.l |= 0x0000ff00;	\
	  if( (ALU2_ctrl&0x04)==0 ) ALU_comp.l |= 0x00ff0000;	\
	}while(0)
#else
#define	set_ALU_comp()						\
	do{							\
	  ALU_comp.l = 0;					\
	  if( (ALU2_ctrl&0x01)==0 ) ALU_comp.l |= 0xff000000;	\
	  if( (ALU2_ctrl&0x02)==0 ) ALU_comp.l |= 0x00ff0000;	\
	  if( (ALU2_ctrl&0x04)==0 ) ALU_comp.l |= 0x0000ff00;	\
	}while(0)
#endif

INLINE	byte	ALU_read( word addr )
{
  ALU_memory	wk;

  ALU_buf.l  = (main_vram4)[addr];
  wk.l       = ALU_comp.l ^ ALU_buf.l;

  return  wk.c[0] & wk.c[1] & wk.c[2];
}

/*------------------------------*/
/* ���̣դ�𤷤��֣ң��ͥ饤��	*/
/*------------------------------*/
INLINE	void	ALU_write( word addr, byte data )
{
  int i, mode;

  screen_set_dirty_flag(addr);

  switch( ALU2_ctrl&ALU2_CTRL_MODE ){

  case 0x00:
    mode = ALU1_ctrl;
    for( i=0;  i<3;  i++, mode>>=1 ){
      switch( mode&0x11 ){
      case 0x00:  main_vram[addr][i] &= ~data;	break;
      case 0x01:  main_vram[addr][i] |=  data;	break;
      case 0x10:  main_vram[addr][i] ^=  data;	break;
      default:					break;
      }
    }
    break;

  case 0x10:
    (main_vram4)[addr] = ALU_buf.l;
    break;

  case 0x20:
    main_vram[addr][0] = ALU_buf.c[1];
    break;

  default:
    main_vram[addr][1] = ALU_buf.c[0];
    break;

  }
}


/*----------------------*/
/*    �ե��å�		*/
/*----------------------*/

byte	main_fetch( word addr )
{

  /* ���ʤ�Ŭ���ʡ����ꥦ�����Ƚ��� */

  if( memory_wait ){

    if( high_mode == FALSE ){		/* ��®�⡼�ɤξ�� */

      z80main_cpu.state0 += 1;			/* M1�������륦������ */

      if( dma_wait_count ){			/* DMA�������Ȥ������    */
	dma_wait_count --;			/* ���������Ĳû����Ƥ��� */
	z80main_cpu.state0 += DMA_WAIT;
      }

    }else{				/* ��®�⡼�ɤξ�� */

      if( addr>=0xf000 && mem_wait_highram ){	/* ��®RAM�Υե��å��� */
	z80main_cpu.state0 += 1;		/* M1�������륦������  */
      }
    }

    /* VRAM�����������Ȥ���8MHz���Υ������Ȥ⤢�뤱��̤���� */
  }


  /* ��® BASIC �⡼�� */		/* peach���� */

  if (highspeed_mode){
    if (!(highspeed_flag) && highspeed_n88rom) {
      int i;
      for (i = 0; highspeed_routine[i] != EndofBasicAddr; i++) {
	if (addr == highspeed_routine[i]) {
	  highspeed_flag = TRUE;
	  ret_addr = main_mem_read(z80main_cpu.SP.W) +
	    	    (main_mem_read(z80main_cpu.SP.W + 1) << 8);
	  hs_icount= z80_state_intchk;

	  z80_state_intchk = HS_BASIC_COUNT*2;
	  /*printf("%x %d -> %d -> ",addr,hs_icount,z80_state_intchk);*/
	  break;
	}
      }
    } else if ((highspeed_flag) &&
	       (ret_addr == addr || z80main_cpu.state0 >= HS_BASIC_COUNT)) {
      ret_addr = 0xffff;
      /*printf("'%d'\n",z80_state_intchk);*/
      z80_state_intchk = hs_icount;
      if (z80main_cpu.state0 > z80_state_intchk) z80main_cpu.state0 = z80_state_intchk;
      highspeed_flag = FALSE;
    }
  }

  /* ����꡼�� */

  if     ( addr < 0x6000 ) return  read_mem_0000_5fff[ addr ];
  else if( addr < 0x8000 ) return  read_mem_6000_7fff[ addr & 0x1fff ];
  else if( addr < 0x8400 ){
    if( read_mem_8000_83ff ) return  read_mem_8000_83ff[ addr & 0x03ff ];
    else{
      addr = (addr & 0x03ff) + window_offset;
      if( addr < 0xf000 ) return  main_ram[ addr ];
      else                return  main_high_ram[ addr & 0x0fff ];
    }
  }
  else if( addr < 0xc000 ) return  main_ram[ addr ];
  else{
    switch( vram_access_way ){
    case VRAM_ACCESS_ALU:  return  ALU_read(  addr & 0x3fff );
    case VRAM_ACCESS_BANK: return  vram_read( addr & 0x3fff );
    default:
      if( addr < 0xf000 )  return  read_mem_c000_efff[ addr & 0x3fff ];
      else                 return  read_mem_f000_ffff[ addr & 0x0fff ];
    }
  }
}

/*----------------------*/
/*    ���ꡦ�꡼��	*/
/*----------------------*/
byte	main_mem_read( word addr )
{
  if     ( addr < 0x6000 ) return  read_mem_0000_5fff[ addr ];
  else if( addr < 0x8000 ) return  read_mem_6000_7fff[ addr & 0x1fff ];
  else if( addr < 0x8400 ){
    if( read_mem_8000_83ff ) return  read_mem_8000_83ff[ addr & 0x03ff ];
    else{
      addr = (addr & 0x03ff) + window_offset;
      if( addr < 0xf000 ) return  main_ram[ addr ];
      else                return  main_high_ram[ addr & 0x0fff ];
    }
  }
  else if( addr < 0xc000 ) return  main_ram[ addr ];
  else{
    switch( vram_access_way ){
    case VRAM_ACCESS_ALU:  return  ALU_read(  addr & 0x3fff );
    case VRAM_ACCESS_BANK: return  vram_read( addr & 0x3fff );
    default:
      if( addr < 0xf000 )  return  read_mem_c000_efff[ addr & 0x3fff ];
      else                 return  read_mem_f000_ffff[ addr & 0x0fff ];
    }
  }
}

/*----------------------*/
/*     ���ꡦ�饤��	*/
/*----------------------*/
void	main_mem_write( word addr, byte data )
{
  if     ( addr < 0x8000 ) write_mem_0000_7fff[ addr ]          = data;
  else if( addr < 0x8400 ){
    if( write_mem_8000_83ff ) write_mem_8000_83ff[ addr & 0x03ff ] = data;
    else{
      addr = (addr & 0x03ff) + window_offset;
      if( addr < 0xf000 ) main_ram[ addr ]               = data;
      else                main_high_ram[ addr & 0x0fff ] = data;
    }
  }
  else if( addr < 0xc000 ) main_ram[ addr ]                     = data;
  else{
    switch( vram_access_way ){
    case VRAM_ACCESS_ALU:  ALU_write( addr & 0x3fff, data );	break;
    case VRAM_ACCESS_BANK: vram_write( addr & 0x3fff, data );	break;
    default:
      if( addr < 0xf000 )  write_mem_c000_efff[ addr & 0x3fff ] = data;
      else                 write_mem_f000_ffff[ addr & 0x0fff ] = data;
    }
  }
}





/************************************************************************/
/* �ɡ��ϥݡ��ȥ�������							*/
/************************************************************************/

/*----------------------*/
/*    �ݡ��ȡ��饤��	*/
/*----------------------*/

void	main_io_out( byte port, byte data )
{
  byte chg;
  PC88_PALETTE_T new_pal;

  switch( port ){

	/* ��®�ơ��ץ��� / PCG */
  case 0x00:
    /*if( use_pcg )*/
      pcg_out_data( data );

    if( boot_basic != BASIC_N )
      sio_tape_highspeed_load();
    return;

  case 0x01:
    /*if( use_pcg )*/
      pcg_out_addr_low( data );
    return;

  case 0x02:
    /*if( use_pcg )*/
      pcg_out_addr_high( data );
    return;

  case 0x0c:
  case 0x0d:
  case 0x0e:
  case 0x0f:
    /* PCG �Υ�����ɽ��ϤΥݡ��Ȥ餷�� */
    return;


	/* �ץ�󥿽��ϡ�����������å� ���ϥǡ��� */
  case 0x10:
    common_out_data = data;
    return;


	/* RS-232C��CMT ���ϥǡ��� */
  case 0x20:
    sio_out_data( data );
    return;

	/* RS-232C��CMT ���楳�ޥ�� */
  case 0x21:
    sio_out_command( data );
    return;


	/* �����ƥॳ��ȥ������ */
  case 0x30:
    if( (sys_ctrl^data) & (SYS_CTRL_80) ){	/* SYS_CTRL_MONO ��̵�� */
      screen_set_dirty_all();			/* (�ƥ����ȤΥ��顼��  */
    }						/*  CRTC����ˤƷ���)   */

    if( sio_tape_readable() ){
      if( (sys_ctrl & 0x08) && !(data & 0x08) ) sio_check_cmt_error();
    }

    /* �����åȥ⡼������졼�� */
    if ((~sys_ctrl &  data) & 0x08) { xmame_dev_sample_motoron();  }
    if (( sys_ctrl & ~data) & 0x08) { xmame_dev_sample_motoroff(); }

    sys_ctrl = data;
    sio_set_intr_base();
/*
printf("CMT %02x, %s: Motor %s: CDS %d\n",data,
       ((data&0x3)==0)?"  600":( ((data&0x30)==0x10)?" 1200":"RS232"),
       ((data&8)==0)?"Off":"On ",(data>>2)&1);
*/
    return;

	/* ����ե��å�����ȥ������ */
  case 0x31:
    chg = grph_ctrl ^ data;

		/* GRPH_CTRL_25 ��̵�� (�ƥ�����25�Ԥ� CRTC����ˤƷ���) */
    if( chg & (GRPH_CTRL_200|GRPH_CTRL_VDISP|GRPH_CTRL_COLOR) ){
      screen_set_dirty_all();

      if( chg & GRPH_CTRL_COLOR ){
	screen_set_dirty_palette();
      }
    }

    /* M88 �ǤϤ����ʤäƤ롩 (peach) */
    /*if (chg & (GRPH_CTRL_64RAM|GRPH_CTRL_N))*/
    /*grph_ctrl = data & (GRPH_CTRL_64RAM|GRPH_CTRL_N);*/
    /*else grph_ctrl = data;*/

    grph_ctrl = data;
    set_text_display();
    main_memory_mapping_0000_7fff();
    main_memory_mapping_8000_83ff();
    return;

	/* �Ƽ����������� */
  case 0x32:
    chg = misc_ctrl ^ data;
    if( chg & MISC_CTRL_ANALOG ){
      screen_set_dirty_palette();
    }
    if( sound_port & SD_PORT_44_45 ){
      intr_sound_enable = (data & INTERRUPT_MASK_SOUND) ^ INTERRUPT_MASK_SOUND;
      if( highspeed_flag == FALSE ) CPU_REFRESH_INTERRUPT();
      /*if( intr_sound_enable == FALSE ) SOUND_flag = FALSE;*/
    }
    misc_ctrl = data;
    main_memory_mapping_0000_7fff();
    main_memory_mapping_c000_ffff();
    main_memory_vram_mapping();
    return;


	/* ��ĥVRAM���� */
  case 0x34:
    ALU1_ctrl = data;
    return;
  case 0x35:
    ALU2_ctrl = data;
    set_ALU_comp();

    /* ����ॾ�󣳤��ɥ饴��,STAR TRADER�ʤɤǻ��� */
    if (data & ALU2_CTRL_VACCESS) memory_bank = MEMORY_BANK_MAIN;
					/* bug fix by peach (thanks!) */

    main_memory_vram_mapping();
    return;


	/* ����ȥ��뿮����� */
  case 0x40:
    out_ctrl_signal( data );
    return;


	/* ������ɽ��� */
  case 0x44:
    if( sound_port & SD_PORT_44_45 ) sound_out_reg( data );
    return;
  case 0x45:
    if( sound_port & SD_PORT_44_45 ) sound_out_data( data );
    return;
  case 0x46:
    if( sound_port & SD_PORT_46_47 ) sound2_out_reg( data );
    return;
  case 0x47:
    if( sound_port & SD_PORT_46_47 ) sound2_out_data( data );
    return;


    	/* CRTC���� */
  case 0x50:
    crtc_out_parameter( data );
/*printf("CRTC PARM %02x\n",data);*/
    return;
  case 0x51:
    crtc_out_command( data );
/*printf("CRTC CMD %02x\n",data);*/
    return;

	/* �طʿ��ʥǥ������*/
  case 0x52:
    if( data&0x1 ) new_pal.blue  = 7;
    else           new_pal.blue  = 0;
    if( data&0x2 ) new_pal.red   = 7;
    else           new_pal.red   = 0;
    if( data&0x4 ) new_pal.green = 7;
    else           new_pal.green = 0;

    if( new_pal.blue  != vram_bg_palette.blue  ||
	new_pal.red   != vram_bg_palette.red   ||
        new_pal.green != vram_bg_palette.green ){
      vram_bg_palette.blue  = new_pal.blue;
      vram_bg_palette.red   = new_pal.red;
      vram_bg_palette.green = new_pal.green;
      screen_set_dirty_palette();
    }
    return;

	/* ���̽Ť͹�碌 */
  case 0x53:
    grph_pile = data;
    set_text_display();
    screen_set_dirty_all();
    return;

	/* �ѥ�å����� */
  case 0x54:
    if( (data & 0x80) &&
	(misc_ctrl & MISC_CTRL_ANALOG) ){	/* ���ʥ��⡼�� */
      if( (data & 0x40) == 0 ){
	new_pal.blue  = (data     ) & 0x07;
	new_pal.red   = (data >> 3) & 0x07;
	new_pal.green = vram_bg_palette.green;
      }else{
	new_pal.blue  = vram_bg_palette.blue;
	new_pal.red   = vram_bg_palette.red;
	new_pal.green = (data     ) & 0x07;
      }
      if( new_pal.blue  != vram_bg_palette.blue  ||
	  new_pal.red   != vram_bg_palette.red   ||
          new_pal.green != vram_bg_palette.green ){
	vram_bg_palette.blue  = new_pal.blue;
	vram_bg_palette.red   = new_pal.red;
	vram_bg_palette.green = new_pal.green;
	screen_set_dirty_palette();
      }
      return;
    }	/* else no return; (.. continued) */
    /* FALLTHROUGH */
  case 0x55:
  case 0x56:
  case 0x57:
  case 0x58:
  case 0x59:
  case 0x5a:
  case 0x5b:
/*printf("PAL %02xH %02x\n",port,data );*/
    if( ! (misc_ctrl&MISC_CTRL_ANALOG) ){	/* �ǥ�����⡼�� */

      if( data&0x1 ) new_pal.blue  = 7;
      else           new_pal.blue  = 0;
      if( data&0x2 ) new_pal.red   = 7;
      else           new_pal.red   = 0;
      if( data&0x4 ) new_pal.green = 7;
      else           new_pal.green = 0;

    }else{					/* ���ʥ��⡼�� */
      if( (data & 0x40) == 0 ){
	new_pal.blue  = (data      ) & 0x07;
	new_pal.red   = (data >> 3 ) & 0x07;
	new_pal.green = vram_palette[ port-0x54 ].green;
      }else{
	new_pal.green = (data      ) & 0x07;
	new_pal.red   = vram_palette[ port-0x54 ].red;
	new_pal.blue  = vram_palette[ port-0x54 ].blue;
      }
    }

    if( new_pal.blue  != vram_palette[ port-0x54 ].blue  ||
	new_pal.red   != vram_palette[ port-0x54 ].red   ||
	new_pal.green != vram_palette[ port-0x54 ].green ){
      vram_palette[ port-0x54 ].blue  = new_pal.blue;
      vram_palette[ port-0x54 ].red   = new_pal.red;
      vram_palette[ port-0x54 ].green = new_pal.green;
      screen_set_dirty_palette();
    }
    return;

    
	/* ����Х����ؤ� */
  case 0x5c:
    memory_bank = MEMORY_BANK_GRAM0;
    main_memory_vram_mapping();
    return;
  case 0x5d:
    memory_bank = MEMORY_BANK_GRAM1;
    main_memory_vram_mapping();
    return;
  case 0x5e:
    memory_bank = MEMORY_BANK_GRAM2;
    main_memory_vram_mapping();
    return;
  case 0x5f:
    memory_bank = MEMORY_BANK_MAIN;
    main_memory_vram_mapping();
    return;

	/* DMAC���� */

  case 0x60:
  case 0x62:
  case 0x64:
  case 0x66:
    dmac_out_address( (port-0x60)/2, data );
/*printf("DMAC %x ADDR %02x\n",(port-0x60)/2,data );*/
    return;
  case 0x61:
  case 0x63:
  case 0x65:
  case 0x67:
    dmac_out_counter( (port-0x61)/2, data );
/*printf("DMAC %x CNTR %02x\n",(port-0x61)/2,data );*/
    return;
  case 0x68:
    dmac_out_mode( data );
/*printf("DMAC MODE %02x\n",data );*/
    return;


	/* �ܡ��졼�� */
  case 0x6f:
    if( ROM_VERSION >= '8' ) baudrate_sw = data;	/* FH/MH �ʹߤ��б� */
    return;


	/* Window ���ե��åȥ��ɥ쥹������ */
  case 0x70:
    window_offset = (word)data << 8;
    main_memory_mapping_8000_83ff();
    return;

	/* ��ĥ ROM �Х� */
  case 0x71:
    ext_rom_bank = data;
    main_memory_mapping_0000_7fff();
    return;

	/* Window ���ե��åȥ��ɥ쥹 ���󥯥���� */

  case 0x78:
    window_offset += 0x100;
    main_memory_mapping_8000_83ff();
    return;



	/* ������ɽ���(���ץ����) */
  case 0xa8:
    if( sound_port & SD_PORT_A8_AD ){
      sound_out_reg( data );
    }
    return;
  case 0xa9:
    if( sound_port & SD_PORT_A8_AD ){
      sound_out_data( data );
    }
    return;
  case 0xaa:
    if( sound_port & SD_PORT_A8_AD ){
      intr_sound_enable = (data & INTERRUPT_MASK_SOUND) ^ INTERRUPT_MASK_SOUND;
      if( highspeed_flag == FALSE ) CPU_REFRESH_INTERRUPT();
      /*if( intr_sound_enable == FALSE ) SOUND_flag = FALSE;*/
    }
    return;
  case 0xac:
    if( sound_port & SD_PORT_A8_AD ){
      sound2_out_reg( data );
    }
    return;
  case 0xad:
    if( sound_port & SD_PORT_A8_AD ){
      sound2_out_data( data );
    }
    return;


	/* ��ĥ RAM ���� */
  case 0xe2:
    if( use_extram ){
      ext_ram_ctrl = data & 0x11;
      main_memory_mapping_0000_7fff();
    }
    return;
  case 0xe3:
/*printf("OUT E3 <=  %02X\n",data);*/
    if( use_extram ){
      if( linear_ext_ram ){		/* �����ͤ��̤�˥Х󥯤��꿶�� */
	ext_ram_bank = data;
      }else{				/* �µ��äݤ�(?)�Х󥯤��꿶�� */
	ext_ram_bank = 0xff;
	if (use_extram <= 4) {				/* 128KB*4�ʲ� */
	    if ((data & 0x0f) < use_extram * 4) {
		ext_ram_bank = data & 0x0f;
	    }
	} else if (use_extram == 8) {			/* 1MB */
	    /* ���� 00-07h, 10-17h, 20-27h, 30-37h �Ȥ��� */
	    if ((data & 0xc8) == 0x00) {
		ext_ram_bank = ((data & 0x30) >> 1) | (data & 0x07);
	    }
	} else if (use_extram <= 10) {			/* 1MB + 128KB*2�ʲ� */
	    /* ���� 08-0Fh, 18-1Fh, 28-2Fh, 38-3Fh �Ȥ��� */
	    if ((data & 0xc8) == 0x08) {
		ext_ram_bank = ((data & 0x30) >> 1) | (data & 0x07);
	    } else if ((data & 0x0f) < (use_extram - 8) * 4) {
		ext_ram_bank = (data & 0x0f) + 0x20;
	    }
	} else if (use_extram == 16) {			/* 2MB */
	    /* ���� 08-0Fh, 18-1Fh, 28-2Fh, 38-3Fh �Ȥ��� */
	    /* ���� 48-4Fh, 58-5Fh, 68-6Fh, 78-7Fh �Ȥ��� */
	    if ((data & 0x88) == 0x08) {
		ext_ram_bank = ((data & 0x70) >> 1) | (data & 0x07);
	    }
	}
      }
      main_memory_mapping_0000_7fff();
    }
    return;


	/* �����ߥ�٥������ */
  case 0xe4:
    intr_priority = data & 0x08;
    if( intr_priority ) intr_level = 7;
    else                intr_level = data & 0x07;
    if( highspeed_flag == FALSE ){
      CPU_REFRESH_INTERRUPT();

      /* 'ASHE�к��ġ� */	/* thanks! peach */
      z80main_cpu.skip_intr_chk = TRUE;
    }
    return;

	/* �����ߥޥ��� */
  case 0xe6:
    intr_sio_enable   = data & INTERRUPT_MASK_SIO;
    intr_vsync_enable = data & INTERRUPT_MASK_VSYNC;
    intr_rtc_enable   = data & INTERRUPT_MASK_RTC;

    if( intr_sio_enable   == FALSE ){RS232C_flag = FALSE; sio_data_clear(); }
    if( intr_vsync_enable == FALSE ) VSYNC_flag  = FALSE;
    if( intr_rtc_enable   == FALSE ) RTC_flag    = FALSE;

    if( highspeed_flag == FALSE ) CPU_REFRESH_INTERRUPT();
    return;


	/* �����ңϣ� ���ɥ쥹���� */
  case 0xe8:
    kanji1_addr.B.l = data;
    return;
  case 0xe9:
    kanji1_addr.B.h = data;
    return;

  case 0xea:
  case 0xeb:
    return;

  case 0xec:
    kanji2_addr.B.l = data;
    return;
  case 0xed:
    kanji2_addr.B.h = data;
    return;


	/* ����ROM������ */

  case 0xf0:
    if( use_jisho_rom ){
      jisho_rom_bank = data & JISHO_BANK;
      main_memory_mapping_c000_ffff();
    }
    return;
  case 0xf1:
    if( use_jisho_rom ){
      jisho_rom_ctrl = data & JISHO_NOT_SELECT; 
      main_memory_mapping_c000_ffff();
    }
    return;

	/* �Уɣ� */

  case 0xfc:
    logpio(" %02x-->\n",data);
    pio_write_AB( PIO_SIDE_M, PIO_PORT_A, data );
    return;
  case 0xfd:
    logpio(" %02x==>\n",data);
    pio_write_AB( PIO_SIDE_M, PIO_PORT_B, data );
    return;
  case 0xfe:
    pio_write_C_direct( PIO_SIDE_M, data );
    return;
  case 0xff:
    if( data & 0x80 ) pio_set_mode( PIO_SIDE_M, data );
    else              pio_write_C( PIO_SIDE_M, data );
    return;




	/* ����¾�Υݡ��� */

  case 0x90:  case 0x91:  case 0x92:  case 0x93:	/* CD-ROM */
  case 0x94:  case 0x95:  case 0x96:  case 0x97:
  case 0x98:  case 0x99:  case 0x9a:  case 0x9b:
  case 0x9c:  case 0x9d:  case 0x9e:  case 0x9f:

  case 0xa0:  case 0xa1:  case 0xa2:  case 0xa3:	/* MUSIC & NETWORK */

			  case 0xc2:  case 0xc3:	/* MUSIC */
  case 0xc4:  case 0xc5:  case 0xc6:  case 0xc7:
  case 0xc8:  case 0xc9:  case 0xca:  case 0xcb:
  case 0xcc:  case 0xcd:  case 0xce:  case 0xcf:

  case 0xd0:  case 0xd1:  case 0xd2:  case 0xd3:	/* MUSIC & GP-IB*/
  case 0xd4:  case 0xd5:  case 0xd6:  case 0xd7:
  case 0xd8:						/* GP-IB */

  case 0xdc:  case 0xdd:  case 0xde:  case 0xdf:	/* MODEM */

  case 0xb4:  case 0xb5:				/* VIDEO ART */


  case 0xc1:				/* ??? Access in N88-BASIC ver 1.8 */
  case 0xf3:  case 0xf4:  case 0xf8:
  
  case 0xe7:				/* ??? Access in N-BASIC ver 1.8 */

    return;
  }


  if( verbose_io )printf("OUT data %02X to undecoeded port %02XH\n",data,port);

}

/*----------------------*/
/*    �ݡ��ȡ��꡼��	*/
/*----------------------*/
byte	main_io_in( byte port )
{
  switch( port ){

	/* �����ܡ��� */
  case 0x00:
  case 0x01:
  case 0x02:
  case 0x03:
  case 0x04:
  case 0x05:
  case 0x06:
  case 0x07:
  case 0x08:
  case 0x09:
  case 0x0a:
  case 0x0b:
  case 0x0c:
  case 0x0d:
  case 0x0e:
  case 0x0f:
    disk_ex_drv = 0;		/* �������Ϥǥꥻ�å� */
#ifdef	USE_KEYBOARD_BUG				/* peach���� */
    {
      int i;
      byte mkey, mkey_old;

      mkey = key_scan[port];
      do {
	mkey_old = mkey;
	for (i = 0; i < 0x10; i++) {
	  if (i != port && key_scan[i] != 0xff) {
	    /* [SHIFT],[CTRL],[GRAPH],[����]�ˤ�Ŭ�Ѥ��ʤ� */
	    if ((i == 0x08 && (mkey | key_scan[i] | 0xf0) != 0xff) ||
		(i != 0x08 && (mkey | key_scan[i])        != 0xff))
	      mkey &= key_scan[i];
	  }
	}
      } while (mkey_old != mkey);
      return(mkey);
    }
#else
    return key_scan[ port ];
#endif


	/* RS-232C��CMT ���ϥǡ��� */
  case 0x20:
    return sio_in_data();


	/* RS-232C/CMT ���� */
  case 0x21:
    return sio_in_status();


	/* �ǥ��åץ����å����� */

  case 0x30:
    return dipsw_1 | 0xc0;
  case 0x31:
    return dipsw_2;

	/* �Ƽ��������� */
  case 0x32:
    return misc_ctrl;

	/* ����ȥ��뿮������ */
  case 0x40:
    return in_ctrl_signal() | 0xc0 | 0x04;
 /* return in_ctrl_signal() | 0xc0;*/


	/* ����������� */
	
  case 0x44:
    if( sound_port & SD_PORT_44_45 ) return sound_in_status( );
    else                             return 0xff;
  case 0x45:
    if( sound_port & SD_PORT_44_45 ) return sound_in_data( FALSE );
    else                             return 0xff;
  case 0x46:
    if( sound_port & SD_PORT_46_47 ) return sound2_in_status( );
    else                             return 0xff;
  case 0x47:
    if( sound_port & SD_PORT_46_47 ) return sound2_in_data( );
    else                             return 0xff;


    	/* CRTC���� */
  case 0x50:
/*printf("READ CRTC parm\n");*/
    return crtc_in_parameter( );
  case 0x51:
/*printf("READ CRTC stat\n");*/
    return crtc_in_status( );


	/* ����Х� */
  case 0x5c:
    return (1<<memory_bank) | 0xf8;


	/* DMAC���� */

  case 0x60:
  case 0x62:
  case 0x64:
  case 0x66:
/*printf("READ DMAC addr\n");*/
    return dmac_in_address( (port-0x60)/2 );
  case 0x61:
  case 0x63:
  case 0x65:
  case 0x67:
/*printf("READ DMAC cntr\n");*/
    return dmac_in_counter( (port-0x61)/2 );
  case 0x68:
/*printf("READ DMAC stat\n");*/
    return dmac_in_status( );


	/* CPU ����å� */
  case 0x6e:
    if( ROM_VERSION >= '8' ) return cpu_clock | 0x10;	/* FH/MH �ʹߤ��б� */
    else		     return 0xff;


	/* �ܡ��졼�� */
  case 0x6f:
    if( ROM_VERSION >= '8' ) return baudrate_sw | 0xf0;	/* FH/MH �ʹߤ��б� */
    else		     return 0xff;


	/* Window ���ե��åȥ��ɥ쥹������ */
  case 0x70:
    return window_offset >> 8;


	/* ��ĥ ROM �Х� */
  case 0x71:
    return ext_rom_bank;


	/* ��ĥ RAM ���� */
  case 0xe2:
    if( use_extram ) return ~ext_ram_ctrl | 0xee;
    return 0xff;
  case 0xe3:
    if( linear_ext_ram ){		/* �����ͤ��̤�˥Х󥯤��꿶�ä� */
      if( use_extram &&
	(ext_ram_bank < use_extram*4) ) return ext_ram_bank;
      return 0xff;
    }else{				/* �µ��äݤ�(?)�Х󥯤��꿶�ä� */
	byte ret = 0xff;
	if (use_extram && (ext_ram_bank != 0xff)) {
	    if (use_extram <= 4) {			/* 128KB*4�ʲ� */
		ret = (ext_ram_bank | 0xf0);
	    } else if (use_extram == 8) {		/* 1MB */
		/* ���� 00-07h, 10-17h, 20-27h, 30-37h �Ȥ��� */
		if (ext_ram_bank < 8) {
		    ret = (ext_ram_bank | 0xf0);
		} else {
		    ret = ((ext_ram_bank & 0x18) << 1) | (ext_ram_bank & 0x07);
		}
	    } else if (use_extram <= 10) {		/* 1MB + 128KB*2�ʲ� */
		/* ���� 08-0Fh, 18-1Fh, 28-2Fh, 38-3Fh �Ȥ��� */
		if (ext_ram_bank < 0x20) {
		    ret = ((ext_ram_bank & 0x18) << 1) | 0x08 |
							 (ext_ram_bank & 0x07);
		} else {
		    ret = ((ext_ram_bank - 0x20) | 0xf0);
		}
	    } else if (use_extram == 16) {		/* 2MB */
		/* ���� 08-0Fh, 18-1Fh, 28-2Fh, 38-3Fh �Ȥ��� */
		/* ���� 48-4Fh, 58-5Fh, 68-6Fh, 78-7Fh �Ȥ��� */
		ret = ((ext_ram_bank & 0x38) << 1) | 0x08 |
							(ext_ram_bank & 0x07);
	    }
	}
/*printf("IN E3 -----> %02X\n",ret);*/
	return ret;
    }


	/* �����������(���ץ����) */
  case 0xa8:
    if( sound_port & SD_PORT_A8_AD ) return sound_in_status( );
    else                             return 0xff;
  case 0xa9:
    if( sound_port & SD_PORT_A8_AD ) return sound_in_data( TRUE );
    else                             return 0xff;
  case 0xaa:
    if( sound_port & SD_PORT_A8_AD ) return intr_sound_enable | 0x7f;
    else                             return 0xff;
  case 0xac:
    if( sound_port & SD_PORT_A8_AD ) return sound2_in_status( );
    else                             return 0xff;
  case 0xad:
    if( sound_port & SD_PORT_A8_AD ) return sound2_in_data( );
    else                             return 0xff;



	/* �����ңϣ� �ե�������� */
  case 0xe8:
    return kanji_rom[0][kanji1_addr.W][1];
  case 0xe9:
    return kanji_rom[0][kanji1_addr.W][0];

  case 0xec:
    return kanji_rom[1][kanji2_addr.W][1];
  case 0xed:
    return kanji_rom[1][kanji2_addr.W][0];



	/* �Уɣ� */

  case 0xfc:
    {
      byte data = pio_read_AB( PIO_SIDE_M, PIO_PORT_A );
      logpio(" %02x<--\n",data);
/*      {
	static byte debug_pio_halt[4] = { 0,0,0,0 };
	debug_pio_halt[0] = debug_pio_halt[1];
	debug_pio_halt[1] = debug_pio_halt[2];
	debug_pio_halt[2] = debug_pio_halt[3];
	debug_pio_halt[3] = data;
	if(debug_pio_halt[0]==0x20&&
	   debug_pio_halt[1]==0x3d&&
	   debug_pio_halt[2]==0x02&&
	   debug_pio_halt[3]==0x00) emu_mode=MONITOR;
      }*/
      return data;
    }
  case 0xfd:
    {
      byte data = pio_read_AB( PIO_SIDE_M, PIO_PORT_B );
      logpio(" %02x<==\n",data);
      return data;
    }
  case 0xfe:
    return pio_read_C( PIO_SIDE_M );




	/* ����¾�Υݡ��� */

  case 0x90:  case 0x91:  case 0x92:  case 0x93:	/* CD-ROM */
  case 0x94:  case 0x95:  case 0x96:  case 0x97:
  case 0x98:  case 0x99:  case 0x9a:  case 0x9b:
  case 0x9c:  case 0x9d:  case 0x9e:  case 0x9f:

  case 0xa0:  case 0xa1:  case 0xa2:  case 0xa3:	/* MUSIC & NETWORK */

			  case 0xc2:  case 0xc3:	/* MUSIC */
  case 0xc4:  case 0xc5:  case 0xc6:  case 0xc7:
  case 0xc8:  case 0xc9:  case 0xca:  case 0xcb:
  case 0xcc:  case 0xcd:  case 0xce:  case 0xcf:

  case 0xd0:  case 0xd1:  case 0xd2:  case 0xd3:	/* MUSIC & GP-IB*/
  case 0xd4:  case 0xd5:  case 0xd6:  case 0xd7:
  case 0xd8:						/* GP-IB */

  case 0xdc:  case 0xdd:  case 0xde:  case 0xdf:	/* MODEM */

  case 0xb4:  case 0xb5:				/* VIDEO ART */


  case 0xc1:				/* ??? Access in N88-BASIC ver 1.8 */
  case 0xf3:  case 0xf4:  case 0xf8:
  
    return 0xff;
#if 0
  case 0xf4:				/* ??? */
    return 0xff;
  case 0xf8:				/* ??? */
    return 0xff;
#endif
  }


  if( verbose_io )printf("IN        from undecoeded port %02XH\n",port);

  return 0xff;
}






/*===========================================================================*/
/* ���ꥢ��ݡ���							     */
/*===========================================================================*/

static	int	sio_instruction;		/* USART �Υ��ޥ�ɾ��� */
static	byte	sio_mode;			/* USART ������⡼��   */
static	byte	sio_command;			/* USART �Υ��ޥ��	*/
static	int	sio_data_exist;			/* �ɹ�̤�� SIO�ǡ���ͭ */
static	byte	sio_data;			/* SIO�ǡ���            */

static	int	com_X_flow = FALSE;		/* ���ǡ�X�ե�������	*/

static	int	cmt_dummy_read_cnt = 0;		/* ���̤���ѻ��Υ��ߡ�	*/
static	int	cmt_skip;			/* ̵���ǡ��������ɤ����Ф� */
static	int	cmt_skip_data;			/* �ɤ����Ф�ľ��Υǡ���   */

static	long	cmt_read_chars = 0;		/* �ɤ߹�����¥Х��ȿ�	*/
static	long	cmt_stateload_chars = 0;	/* ���ơ��ȥ��ɻ��ɹ���*/
static	int	cmt_stateload_skip = 0;		/* ���ơ��ȥ��ɻ� skip */

static	int	sio_getc( int is_cmt, int *tick );

void	sio_data_clear(void)
{
    sio_data_exist = FALSE;
}


/*-------- �����ѥơ��ץ��᡼���ե������ "rb" �ǳ��� --------*/

int	sio_open_tapeload( const char *filename )
{
  sio_close_tapeload();

  if( (fp_ti = osd_fopen( FTYPE_TAPE_LOAD, filename, "rb" )) ){

    sio_set_intr_base();
    return sio_tape_rewind();

  }else{
    if (quasi88_is_menu() == FALSE)
      printf("\n[[[ %s : Tape load image can't open ]]]\n\n", filename );
  }
  cmt_stateload_chars = 0;
  return FALSE;
}
void	sio_close_tapeload( void )
{
  if( fp_ti ){ osd_fclose( fp_ti ); fp_ti = NULL; }
  sio_set_intr_base();

  cmt_read_chars = 0;
}

/*-------- �������ѥơ��ץ��᡼���ե������ "ab" �ǳ��� --------*/

int	sio_open_tapesave( const char *filename )
{
  sio_close_tapesave();

  if( (fp_to = osd_fopen( FTYPE_TAPE_SAVE, filename, "ab" )) ){

    return TRUE;

  }else{
    if (quasi88_is_menu() == FALSE)
      printf("\n[[[ %s : Tape save image can't open ]]]\n\n", filename );
  }
  return FALSE;
}
void	sio_close_tapesave( void )
{
  if( fp_to ){ osd_fclose( fp_to ); fp_to = NULL; }
}

/*-------- ���ꥢ�������ѤΥե������ "rb" �ǳ��� --------*/

int	sio_open_serialin( const char *filename )
{
  sio_close_serialin();

  if( (fp_si = osd_fopen( FTYPE_COM_LOAD, filename, "rb" )) ){

    sio_set_intr_base();
    com_EOF = FALSE;

    if( osd_fseek( fp_si, 0, SEEK_END ) ) goto ERR;
    if( (com_size = osd_ftell( fp_si )) < 0 ) goto ERR;
    if( osd_fseek( fp_si, 0, SEEK_SET ) ) goto ERR;

    return TRUE;
  }
 ERR:
  if( fp_si ){
      printf("\n[[[ Serial input image access error ]]]\n\n" );
  }else{
    if (quasi88_is_menu() == FALSE)
      printf("\n[[[ %s : Serial input file can't open ]]]\n\n", filename );
  }
  sio_close_serialin();

  return FALSE;
}
void	sio_close_serialin( void )
{
  if( fp_si ){ osd_fclose( fp_si ); fp_si = NULL; }
  sio_set_intr_base();
  /* com_X_flow = FALSE; */
}

/*-------- ���ꥢ������ѤΥե������ "ab" �ǳ��� --------*/

int	sio_open_serialout( const char *filename )
{
  sio_close_serialout();

  if( (fp_so = osd_fopen( FTYPE_COM_SAVE, filename, "ab" )) ){

    return TRUE;

  }else{
    if (quasi88_is_menu() == FALSE)
      printf("\n[[[ %s : Serial output file can't open ]]]\n\n", filename );
  }
  return FALSE;
}
void	sio_close_serialout( void )
{
  if( fp_so ){ osd_fclose( fp_so ); fp_so = NULL; }
}

/*-------- ���ꥢ��ޥ���������/��λ���� --------*/

void	sio_mouse_init(int initial)
{
  if (initial) {
    init_serial_mouse_data();
  }
  sio_set_intr_base();
}

/*-------- �����Ƥ���ơ��ץ��᡼���򴬤��᤹ --------*/

#define T88_HEADER_STR		"PC-8801 Tape Image(T88)"
int	sio_tape_rewind( void )
{
  int size;
  char buf[ sizeof(T88_HEADER_STR) ];

  cmt_read_chars = 0;

  if( fp_ti ){
    if( osd_fseek( fp_ti, 0, SEEK_END ) ) goto ERR;
    if( (cmt_size = osd_ftell( fp_ti )) < 0 ) goto ERR;

    if( osd_fseek( fp_ti, 0, SEEK_SET ) ) goto ERR;

    size = osd_fread( buf, sizeof(char), sizeof(buf), fp_ti );
    if( size == sizeof(buf) &&
	memcmp( buf, T88_HEADER_STR, sizeof(buf) ) == 0 ){	/* T88 */
      cmt_is_t88     = TRUE;
      cmt_block_size = 0;
      cmt_EOF        = FALSE;
      cmt_skip       = 0;
    }else{							/* CMT */
      cmt_is_t88     = FALSE;
      cmt_EOF        = FALSE;
      cmt_skip       = 0;
      if( osd_fseek( fp_ti, 0, SEEK_SET ) ) goto ERR;
    }

    while( cmt_stateload_chars -- ){	/* ���ơ��ȥ��ɻ��ϡ��ơ��������� */
      if( sio_getc( TRUE, NULL ) == EOF ){
	break;
      }
    }
    cmt_skip = cmt_stateload_skip;
    cmt_stateload_chars = 0;

/*printf("%d\n",osd_ftell(fp_ti));*/
    return TRUE;
  }

 ERR:
  if( fp_ti ){
    printf("\n[[[ Tape image access error ]]]\n\n" );
  }
  sio_close_tapeload();

  cmt_stateload_chars = 0;
  return FALSE;
}

/*-------- �����Ƥ���ơ��פθ��߰��֤��֤� (��%�ɤ�����γ�ǧ��) --------*/

int	sio_tape_pos( long *cur, long *end )
{
  long v;

  if( fp_ti ){
    if( cmt_EOF ){		/* ��ü�ʤ顢����=0/��ü=0 �ˤ��������֤� */
      *cur = 0;
      *end = 0;
      return TRUE;
    }else{			/* ����ʤ顢���֤Ƚ�ü�򥻥åȤ������֤� */
      v = osd_ftell( fp_ti );
      if( v >= 0 ){
	*cur = v;
	*end = cmt_size;
	return TRUE;
      }
    }
  }
  *cur = 0;			/* �������ϡ�����=0/��ü=0 �ˤ��������֤� */
  *end = 0;
  return FALSE;
}

int	sio_com_pos( long *cur, long *end )
{
  long v;

  if( fp_si ){
    if( com_EOF ){		/* ��ü�ʤ顢����=0/��ü=0 �ˤ��������֤� */
      *cur = 0;
      *end = 0;
      return TRUE;
    }else{			/* ����ʤ顢���֤Ƚ�ü�򥻥åȤ������֤� */
      v = osd_ftell( fp_si );
      if( v >= 0 ){
	*cur = v;
	*end = com_size;
	return TRUE;
      }
    }
  }
  *cur = 0;			/* �������ϡ�����=0/��ü=0 �ˤ��������֤� */
  *end = 0;
  return FALSE;
}





/*
 * �����Ƥ���sio���᡼������1ʸ���ɤ߹��� 
 */
static	int	sio_getc( int is_cmt, int *tick )
{
  int i, c, id, size, time;

  if( tick ) *tick = 0;

  if( is_cmt==FALSE ){			/* ���ꥢ������ */

    if (use_siomouse) {
      c = get_serial_mouse_data();
      return c;
    }

    if( fp_si==NULL ) return EOF;
    if( com_EOF )     return EOF;

    c = osd_fgetc( fp_si );
    if( c==EOF ){
      printf(" (( %s : Serial input file EOF ))\n", file_sin );
      status_message( 1, STATUS_WARN_TIME, "Serial input  [EOF]" );
      com_EOF = TRUE;
    }
    return c;

  }else{				/* �ơ������� */

    if( fp_ti==NULL ) return EOF;
    if( cmt_EOF )     return EOF;

    if( cmt_is_t88 == FALSE ){			/* CMT�����ξ�� */

      c = osd_fgetc( fp_ti );

    }else{					/* T88�����ξ�� */

      while( cmt_block_size == 0 ){

	if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	id = c;
	if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	id += c << 8;

	if( id==0x0000 ){				/* ��λ���� */
	  c = EOF; break;
	}
	else {
	  if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	  size = c;
	  if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	  size += c << 8;

	  if( id == 0x0101 ){				/* �ǡ������� */

	    if( size < 12 ){ c = EOF; break; }

	    for( i=0; i<12; i++ ){	/* ���������̵�� */
	      if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	    }
	    if( c==EOF ) break;
	    cmt_block_size = size - 12;

	  }else{					

	    if( id == 0x0100 ||				/* �֥�󥯥��� */
		id == 0x0102 ||				/* ���ڡ������� */
		id == 0x0103 ){				/* �ޡ�������   */

	      if( size != 8 ){ c = EOF; break; }

	      for( i=0; i<4; i++ ){	/* ���ϻ��֤�̵�� */
		if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	      }
	      if( c==EOF ) break;
					/* Ĺ�����֤ϼ��� */
	      if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	      time = c;
	      if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	      time += c << 8;
	      if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	      time += c << 16;
	      if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	      time += c << 24;

	      if( tick ) *tick += time;

	    }else{					/* ¾�Υ���(̵��) */

	      for( i=0; i<size; i++ ){
		if( (c=osd_fgetc(fp_ti))==EOF ){ break; }
	      }
	      if( c==EOF ) break;

	    }
	  }
	}
      }

      cmt_block_size --;
      c = osd_fgetc( fp_ti );
    }

    if( c==EOF ){
      cmt_EOF = TRUE;
      status_message( 1, STATUS_WARN_TIME, "Tape Read  [EOF]");
    }else{
      cmt_read_chars ++;
    }

    return c;
  }
}

/* 
 * �ơ��פ��ɤ߹�������˥⡼��OFF���줿�顢�ɤ߹��ߥ��顼ȯ���Ȥ���
 * 1�Х����ɤ����Ф����ǡ��������椫�ɤ����� T88 �Ǥʤ��ȥ����å��Ǥ��ʤ���
 * ����ʥ����å���ɬ�פʤΤ�����
 */
static	void	sio_check_cmt_error( void )
{
  int c;
  if( sio_tape_readable() ){
    if( cmt_is_t88     &&	/* T88 ���ġ��ǡ�������������λ��Τ� */
	cmt_skip == 0  &&
	cmt_block_size ){
      cmt_block_size --;
      c = osd_fgetc( fp_ti );

      if( verbose_proc )
	printf( "Tape read: lost 1 byte\n" );

      if( c==EOF ){
	cmt_EOF = TRUE;
	status_message(1, STATUS_WARN_TIME, "Tape Read  [EOF]");
      }else{
	cmt_read_chars ++;
      }
    }
  }
}






/*
 * �����Ƥ���sio���᡼����1ʸ���񤭹��� 
 */
static	int	sio_putc( int is_cmt, int c )
{
  OSD_FILE *fp;

  if( is_cmt==FALSE ){ fp = fp_so; }	/* ���ꥢ����� */
  else               { fp = fp_to; }	/* �ơ��׽��� */
  
  if( fp ){
    osd_fputc( c, fp );
    osd_fflush( fp );
  }
  return c;
}


/*
 * ��®�ơ��ץ��� �ġ� �ܺ�����������ʵ�ǽ���ä��Τ��� 
 */
static	void	sio_tape_highspeed_load( void )
{
  int c, sum, addr, size;

  if( sio_tape_readable()==FALSE ) return;

			  /* �ޥ����إå���õ�� */

  do{						/* 0x3a ���ФƤ���ޤǥ꡼�� */
    if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
  } while( c != 0x3a );
						/* ž���襢�ɥ쥹 H */
  if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
  sum = c;
  addr = c * 256;
						/* ž���襢�ɥ쥹 L */
  if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
  sum += c;
  addr += c;
						/* �إå������� */
  if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
  sum += c;
  if( (sum&0xff) != 0 ){ return; }


		/* ���Ȥϥǡ������η����֤� */

  while( TRUE ){

    do{						/* 0x3a ���ФƤ���ޤǥ꡼�� */
      if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
    } while( c != 0x3a );

						/* �ǡ����� */
    if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
    sum  = c;
    size = c;
    if( c==0 ){						/* �ǡ�����==0�ǽ�ü */
      return;
    }

    for( ; size; size -- ){			/* �ǡ�����ʬ��ž�� */

      if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
      sum += c;
      main_mem_write( addr, c );
      addr ++;
    }
						/* �ǡ��������� */
    if( (c = sio_getc(TRUE,0)) == EOF ){ return; }
    sum += c;
    if( (sum&0xff) != 0 ){ return; }
  }
}



/*
 * RS232C�����߼����Υ��å� 
 */
				/* �������ѿ��ϡ����꤫�黻�Ф��� */
static int sio_bps;		/* BPS */
static int sio_framesize;	/* StartBit + BitĹ + StopBit ��Ŭ���˷׻� */

static void sio_set_intr_base( void )
{
  static const int table[] = {
    75, 150, 300, 600, 1200, 2400, 4800, 9600, 19200,
  };

  /* ���᡼���ե����륻�åȺѤ� ���ġ�
     ���� Enable ���ġ�
     RS232C (I/O 30h:bit5=ON) ����CMT���ĥ⡼��ON (I/O 30h:bit5=OFF,bit3=ON)
     �λ��ˡ�����Ū�˳����ߤ�ȯ�������롣���μ�����׻����롣 */

  if( (fp_si || fp_ti || use_siomouse) &&
      (sio_command & 0x04) &&
      ( (sys_ctrl & 0x20) || (sys_ctrl & 0x08) ) ){

    if( sys_ctrl & 0x20 ){		/* RS232C �����  */

      sio_bps = table[ baudrate_sw ];		/* BPS �� �ܡ��졼������ */
      sio_framesize = 10;			/* �ե졼��Ĺ��10bit���� */

    }else{				/* CMT ����� */

      if( cmt_speed == 0 ){			/* �̾�ϡ�*/
	if( sys_ctrl & 0x10 ) sio_bps = 1200;	/* I/O 30h:bit4=1 �� 1200bps */
	else                  sio_bps =  600;	/*             =0 ��  600bps */
      }else{
	sio_bps = cmt_speed;			/* ����������Ϥ����� */
      }
      sio_framesize = 11;			/* �ե졼��Ĺ��11bit���� */
    }

  }else{				/* ���ꥢ���ԲĻ� */
    sio_bps = 0;
    sio_framesize = 0;
  }

  interval_work_set_RS232C( sio_bps, sio_framesize );  
}


/*
 * T88 �ե����ޥå� �� TICK���֤� ���ꥢ������߲���˴�������
 *
 * �֥�󥯡����ڡ������ޡ��������ϡ�1/4800sñ�̤����л��֤����Ҥ��Ƥ��롣
 * �ʤΤǡ����֤� n �ξ�硢n/4800�å������Ȥ򤤤��Ȥ��������ˤʤ�Ϥ���
 * �Ĥޤꡢ ( n / 4800 ) * CPU_CLOCK / rs232c_intr_base ��ʬ��
 * RS232C�����ߤ�;ʬ���ԤƤФ��������μ��ϡ��֤��������
 *	     ( n / 4800 ) * ( bps / framesize )
 * �ˤʤ롣�ޤ�������ˤ��褫�ʤ�Ŭ���ʥ������ȤǤϤ��뤬��
*/
static	int	tick_2_intr_skip( int tick )
{
  if( sio_framesize == 0 ) return 0;

  return (tick * sio_bps / 4800 / sio_framesize) + 1;
}



/*
 *
 */
static	void	sio_init( void )
{
  sio_instruction = 0;
  sio_command     = 0;
  sio_mode        = 0;
  sio_data_exist  = FALSE;
  sio_data        = 0;		/* ����ͤ�0���Ȥ��륲����εߺѤΤ���� ;_; */

  com_X_flow      = FALSE;

  cmt_dummy_read_cnt = 0;
}
/*
 *
 */
static	void	sio_out_command( byte data )
{
  if( sio_instruction==0 ){			/* �����ꥻ�å�ľ��ϡ� */
    sio_mode        = data;				/* �⡼�ɼ��դ� */
    sio_instruction = 1;

  }else{					/* ����ʳ��ϥ��ޥ�ɼ��դ� */

    if( data & 0x40 ){					/* �����ꥻ�å� */
      sio_mode        = 0;
      sio_instruction = 0;
      sio_command     = 0x40;
      sio_data        = 0;
    }else{						/* ����¾       */
      sio_command     = data;
    }

    if( (sio_command & 0x04) == 0 ){			/* �ꥻ�å�or�����ػ�*/
      sio_data_exist = FALSE;				/* �ʤ顢�������  */
      RS232C_flag   = FALSE;				/* �򥯥ꥢ����      */
    }

    sio_set_intr_base();
  }
/*printf("SIO %02x -> mode:%02x cmd:%02x\n",data,sio_mode,sio_command);*/
}
/*
 *
 */
static	void	sio_out_data( byte data )
{
  int is_cmt;

  if( (sio_command & 0x01) ){		/* �������͡��֥� */
    if( sys_ctrl & 0x20 ){			/* ���ꥢ����Ϥξ�� */
      is_cmt = FALSE;
      if     ( data==0x11 ){				/* ^Q ���� */
	com_X_flow = FALSE;
      }else if( data==0x13 ){				/* ^S ���� */
	com_X_flow = TRUE;
      }
    }else{					/* �ơ��׽��Ϥξ�� */
      is_cmt = TRUE;
    }
    sio_putc( is_cmt, data );
  }
}
/*
 *
 */
static	byte	sio_in_data( void )
{
/*printf("->%02x ",sio_data);fflush(stdout);*/
  sio_data_exist = FALSE;
  RS232C_flag = FALSE;
  return sio_data;
}
/*
 *
 */
static	byte	sio_in_status( void )
{
  int c;
  byte	status = 0x80 | 0x04;		/* �����Хåե�����ץƥ� */
                /* DSR| TxE */

  if( sio_command & 0x04 ){		/* ���ߡ��������͡��֥�ξ�� */

    if( (sys_ctrl & 0x20)==0 && 		/* �ơ��פǡ�SIO�����ߤ�  */
	sio_tape_readable() &&			/* �Ȥ�ʤ���硢�������ɤ� */
	cmt_intr == FALSE ){

      cmt_dummy_read_cnt ++;			/* IN 21 �� 2��¹Ԥ����٤� */
      if( cmt_dummy_read_cnt >= 2 ){
	cmt_dummy_read_cnt = 0;

	c = sio_getc( TRUE, 0 );		/* �ơ��פ���1ʸ���ɤ� */
/*printf("[%03x]",c&0xfff);fflush(stdout);*/
	if( c != EOF ){
	  sio_data = (byte)c;
	  sio_data_exist = TRUE;
	}
      }
    }

    if( sio_data_exist ){			/* �ǡ���������� */
      status |= 0x02;					/* ������ǥ� */
             /* RxRDY */
    }
  }

  if( sio_command & 0x01 ){		/* ���ߡ��������͡��֥�ξ�� */
    if(( (sys_ctrl & 0x20) /*&& sio_serial_writable()*/ ) ||
       (!(sys_ctrl & 0x20)   && sio_tape_writable()     ) ){
      status |= 0x01;				/* ������ǥ� */
             /* TxRDY */
    }
  }

  return	status;
}
/*
 *
 */
static	void	sio_term( void )
{
}



/*
 * RS-232C ���������߽���
 */
int	sio_intr( void )
{
  int c = EOF;
  int tick;

  if( (sio_command & 0x04) &&		/* ���ߡ��������͡��֥��   */
      ! sio_data_exist ){		/* �ɹ�̤�Υǡ������ʤ���� */

    if( sys_ctrl & 0x20 ){			/* ���ꥢ������ */

      if( com_X_flow ) return FALSE;
      c = sio_getc( FALSE, 0 );

    }else{					/* �ơ�������(������ѻ��Τ�)*/
      if( cmt_intr ){

	if( cmt_skip==0 ){
	  if( cmt_wait ){
	    c = sio_getc( TRUE, &tick );
	    if( tick ){
	      cmt_skip = tick_2_intr_skip( tick );
	      if( cmt_skip!=0  ){
		cmt_skip_data = c;
		c = EOF;
	      }
	    }
	  }else{
	    c = sio_getc( TRUE, 0 );
	  }
	}else{						/* T88�ξ��ϡ�    */
	  cmt_skip --;					/* ̵���ǡ�����ʬ�� */
	  if( cmt_skip==0 ){				/* �֡������٤����� */
	    c = cmt_skip_data;
	  }
	}
      }
    }

    if( c!=EOF ){
      sio_data = (byte)c;
      sio_data_exist = TRUE;
/*printf("<%02x> ",sio_data);fflush(stdout);*/
      return TRUE;				/* RxRDY������ȯ�� */
    }
  }
  return FALSE;
}



/*
 *	���֥����å��ؿ�
 */
int	tape_exist( void )
{
  return (fp_ti || fp_to);
}

int	tape_readable( void )
{
  return (fp_ti) ? TRUE : FALSE;
}

int	tape_writable( void )
{
  return (fp_to) ? TRUE : FALSE;
}

int	tape_reading( void )
{
  return( fp_ti &&
	  (sio_command & 4) &&
	  ((sys_ctrl & 0x28) == 0x08) );
}

int	tape_writing( void )
{
  return( fp_to &&
	  (sio_command & 1) &&
	  ((sys_ctrl & 0x28) == 0x08) );
}



/*===========================================================================*/
/* �ѥ���ݡ���							     */
/*===========================================================================*/

int	printer_open( const char *filename )
{
  printer_close();

  if( (fp_prn = osd_fopen( FTYPE_PRN, filename, "ab") ) ){

    return TRUE;

  }else{
    if (quasi88_is_menu() == FALSE)
      printf("\n[[[ %s : Printer output file can't open ]]]\n\n", filename );
  }
  return FALSE;
}
void	printer_close( void )
{
  if( fp_prn ){ osd_fclose( fp_prn ); fp_prn = NULL; }
}



void	printer_init( void )
{
}
void	printer_stlobe( void )
{
  if( fp_prn ){
    osd_fputc( common_out_data, fp_prn );
    osd_fflush( fp_prn );
  }
}
void	printer_term( void )
{
}


/*===========================================================================*/
/* ����������å�							     */
/*===========================================================================*/

static	Uchar	shift_reg[7];
static	Uchar	calendar_cdo;
static	int	calendar_diff;

static	void	get_calendar_work( void )
{
  struct tm t;

  if( calendar_stop==FALSE ){
    time_t now_time;
    struct tm *tp;

    now_time  = time( NULL );
    now_time += (time_t)calendar_diff;
    tp = localtime( &now_time );
    t = *tp;
  }else{
    t.tm_year = calendar_data[0] + 1900;
    t.tm_mon  = calendar_data[1];
    t.tm_mday = calendar_data[2];
    t.tm_wday = calendar_data[3];
    t.tm_hour = calendar_data[4];
    t.tm_min  = calendar_data[5];
    t.tm_sec  = calendar_data[6];
  }

  shift_reg[0] = ( t.tm_sec %10 <<4 );
  shift_reg[1] = ( t.tm_min %10 <<4 ) | ( t.tm_sec /10 );
  shift_reg[2] = ( t.tm_hour%10 <<4 ) | ( t.tm_min /10 );
  shift_reg[3] = ( t.tm_mday%10 <<4 ) | ( t.tm_hour/10 );
  shift_reg[4] = ( t.tm_wday    <<4 ) | ( t.tm_mday/10 );
  shift_reg[5] = ( (t.tm_year%100)%10 <<4 ) | ( t.tm_mon+1 );
  shift_reg[6] = ( (t.tm_year%100)/10 );
}
static	void	set_calendar_work( int x )
{
#define BCD2INT(b)	((((b)>>4)&0x0f)*10 + ((b)&0x0f))
  time_t now_time;
  time_t chg_time;
  struct tm *tp;
  struct tm t;
  int i;

  static const char *week[]=
  { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "???" };

  if(x==0){
    if( verbose_io )
      printf("Set Clock %02d/%02x(%s) %02x:%02x:%02x\n",
	     (shift_reg[4]>>4)&0x0f, shift_reg[3], week[ shift_reg[4]&0x07 ],
	     shift_reg[2], shift_reg[1], shift_reg[0]);

    now_time  = time( NULL );
    now_time += (time_t)calendar_diff;
    tp = localtime( &now_time );
    t.tm_year = tp->tm_year;

  }else{
    if( verbose_io )
      printf("Set Clock %02x/%02d/%02x(%s) %02x:%02x:%02x\n",
	     shift_reg[5],
	     (shift_reg[4]>>4)&0x0f, shift_reg[3], week[ shift_reg[4]&0x07 ],
	     shift_reg[2], shift_reg[1], shift_reg[0]);

    i = BCD2INT( shift_reg[5] );
    if( i >= 38 ) t.tm_year = 1900 + i -1900;
    else          t.tm_year = 2000 + i -1900;
  }

  t.tm_mon  = ((shift_reg[4]>>4)&0x0f) -1;
  t.tm_mday = BCD2INT( shift_reg[3] );
  t.tm_wday = shift_reg[4]&0x07;
  t.tm_hour = BCD2INT( shift_reg[2] );
  t.tm_min  = BCD2INT( shift_reg[1] );
  t.tm_sec  = BCD2INT( shift_reg[0] );
  t.tm_yday = 0;
  t.tm_isdst= 0;

  now_time = time( NULL );
  chg_time = mktime( &t );

  if( now_time != -1 && chg_time != -1  )
    calendar_diff = (int)difftime( chg_time, now_time );

#undef BCD2INT
}


void	calendar_init( void )
{
  int	i;
  for(i=0;i<7;i++) shift_reg[i] = 0;
  calendar_cdo = 0;

  calendar_diff = 0;
}

void	calendar_shift_clock( void )
{
  byte	x = ( common_out_data>>3 ) & 0x01;

  calendar_cdo = shift_reg[0] & 0x01;
  shift_reg[0] = ( shift_reg[0]>>1 ) | ( shift_reg[1]<<7 );
  shift_reg[1] = ( shift_reg[1]>>1 ) | ( shift_reg[2]<<7 );
  shift_reg[2] = ( shift_reg[2]>>1 ) | ( shift_reg[3]<<7 );
  shift_reg[3] = ( shift_reg[3]>>1 ) | ( shift_reg[4]<<7 );
  shift_reg[4] = ( shift_reg[4]>>1 ) | ( shift_reg[5]<<7 );
  shift_reg[5] = ( shift_reg[5]>>1 ) | ( shift_reg[6]<<7 );
  shift_reg[6] = ( shift_reg[6]>>1 ) | ( x<<3 );
}

void	calendar_stlobe( void )
{
  switch( common_out_data & 0x7 ){
  case 0:	/*calendar_init();*/	break;		/* ����� */
  case 1:	calendar_shift_clock();	break;		/* ���ե� */
  case 2:	calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		set_calendar_work(0);	break;
  case 3:	get_calendar_work();			/* ������� */
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();
		calendar_shift_clock();	break;
  case 4:	break;
  case 5:	break;
  case 6:	break;
  case 7:
    switch( shift_reg[6] & 0xf ){

    case 0:	/*calendar_init();*/	break;		/* ����� */
    case 1:	calendar_shift_clock();	break;		/* ���ե� */
    case 2:	set_calendar_work(1);	break;		/* �������� */
    case 3:	get_calendar_work();	break;		/* ������� */
    case 4:	break;
    case 5:	break;
    case 6:	break;
    case 7:	break;
    case 8:	break;
    case 9:	break;
    case 10:	break;
    case 11:	break;
    case 12:	break;
    case 13:	break;
    case 14:	break;
    case 15:	/*test_mode();*/	break;
    }
    break;
  }
}


/*===========================================================================*/
/* ����ȥ��뿮��������						     */
/*===========================================================================*/

void	out_ctrl_signal( byte data )
{
  byte	trg_on  = ~ctrl_signal &  data;
  byte	trg_off =  ctrl_signal & ~data;

  if( trg_on  & 0x01 ) printer_stlobe();
  if( trg_off & 0x02 ) calendar_stlobe();
  if( trg_off & 0x04 ) calendar_shift_clock();

  if( data & 0x08 ) set_crtc_sync_bit();
  else		    clr_crtc_sync_bit();

  if( (trg_on & (0x80|0x20)) || (trg_off & (0x80|0x20)) ){
    xmame_dev_beep_out_data( data );
  }

  if( trg_on  & 0x40 ) keyboard_jop1_strobe();
  if( trg_off & 0x40 ) keyboard_jop1_strobe();

  ctrl_signal = data;
}

byte	in_ctrl_signal( void )
{
  return ((ctrl_vrtc    << 5 ) |
	  (calendar_cdo << 4 ) |
	   ctrl_boot           |
	   monitor_15k         );
}








/************************************************************************/
/* ����ν���� (�Ÿ��������Τ�)					*/
/************************************************************************/
#if 0
void	power_on_ram_init( void )
{
  int   addr, i;
  Uchar data;

		/* �ᥤ�� RAM ���ü�ʥѥ���������� */

  for( addr = 0; addr < 0x10000; addr += 0x100 ){
    if( (addr&0x0d00)==0x0100 || (addr&0x0f00)==0x0500 ||
        (addr&0x0f00)==0x0a00 || (addr&0x0d00)==0x0c00 )  data = 0xff;
    else                                                  data = 0x00;

    if( addr&0x4000 ) data ^= 0xff;
    if( addr&0x8000 ) data ^= 0xff;
    if((addr&0xf000)==0xb000 ) data ^= 0xff;
#if 0
    if((addr&0xf000)==0xe000 ) data ^= 0xff; /* �Ȥꤢ����ȿž */
					     /* changed by peach */
#endif

    for(i=0;i<4;i++){
      memset( &main_ram[ addr + i*64     ], data,      16 );
      memset( &main_ram[ addr + i*64 +16 ], data^0xff, 16 );
      memset( &main_ram[ addr + i*64 +32 ], data,      16 );
      memset( &main_ram[ addr + i*64 +48 ], data^0xff, 16 );
      data ^= 0xff;
    }
  }
  if( high_mode ){
    for( i=0xf000; i<0x10000; i++ ) main_ram[i] ^= 0xff;
  }


		/* ��® RAM(��΢) ���ü�ʥѥ���������� */

  memcpy( main_high_ram, &main_ram[0xf000], 0x1000 );
  for( addr=0xf000; addr<0x10000; addr++ ) main_ram[addr] ^= 0xff;
}


#else	/* FH �ǤϤ��Τ褦�ʵ������뤱�ɡġġ� */
void	power_on_ram_init( void )
{
  int   addr, i;
  Uchar data;

		/* �ᥤ�� RAM ���ü�ʥѥ���������� */

  for( addr = 0; addr < 0x4000; addr += 0x100 ){

    if( ((addr & 0x0d00) == 0x0100) ||		/* x100, x300 */
	((addr & 0x0d00) == 0x0c00) ){		/* xc00, xe00 */
      data = 0xff;
    }else if( ((addr & 0x0f00) == 0x0500) &&
	      ((addr & 0x2000) == 0x0000) ){	/* 0500, 1500 */
      data = 0xff;
    }else if( ((addr & 0x0f00) == 0x0a00) &&
	      ((addr & 0x3000) != 0x0000) ){	/* 1a00, 2a00, 3a00 */
      data = 0xff;
    }else{
      data = 0x00;
    }

    for(i=0;i<4;i++){
      memset( &main_ram[ addr + i*64     ], data,      16 );
      memset( &main_ram[ addr + i*64 +16 ], data^0xff, 16 );
      memset( &main_ram[ addr + i*64 +32 ], data,      16 );
      memset( &main_ram[ addr + i*64 +48 ], data^0xff, 16 );
      data ^= 0xff;
    }
  }

  for( addr = 0x4000; addr < 0x8000; addr += 0x100 ){
    for( i=0; i<0x100; i++ ){
      main_ram[ addr + i ] = main_ram[ addr + i - 0x4000 ] ^ 0xff;
    }
  }

  for( addr = 0x8000; addr < 0x10000; addr += 0x100 ){
    memcpy( &main_ram[ addr ], &main_ram[ 0x7f00 - (addr - 0x8000) ], 0x100 );
  }

  memcpy( &main_high_ram[0], &main_ram[0xf000], 0x1000 );

/*
  memset( &main_ram[0x9000], 0x00, 0x100 );
  main_ram[0x90ff] = 0xff;
  memset( &main_ram[0xbf00], 0x00, 0x100 );
  main_ram[0xbfff] = 0xff;
*/
  for( addr = 0xff00; addr < 0xffff; addr ++ ){
    main_ram[ addr ] = 0xff;
  }
  main_ram[ 0xffff ] = 0x00;

  /* ���ͤޤǤˡ��ʲ��Υ��եȤ�RAM�ν���ͤ�ȤäƤ���褦����
     ��������        : 0xffff �� 0x00 �Ǥ��뤳��
     ŷ�Ȥ����θ��2 : 0xfff8��0xffff �� ���Х��Ȥ�OR !=0x00 �Ǥ��뤳��
     ŷ�Ȥ����θ��  : �������
  */
}
#endif










/************************************************************************/
/* PC88 �ᥤ�󥷥��ƥ� �����						*/
/************************************************************************/

static	void	bootup_work_init(void)
{
	/* V1�⡼�ɤΥС������ξ������ʲ������ѹ����� */

    if (set_version) ROM_VERSION = set_version;
    else             ROM_VERSION = rom_version;

	/* ��ư�ǥХ���(ROM/DISK)̤��λ� */

    if (boot_from_rom == BOOT_AUTO) {
	if (disk_image_exist(0))	/* �ǥ�������������DISK */
	    boot_from_rom = FALSE;
	else				/* ����ʳ��ϡ�    ROM  */
	    boot_from_rom = TRUE;
    }

	/* ��ư���� BASIC�⡼��̤��λ�	  */

    if (boot_basic == BASIC_AUTO) {			
	if (ROM_VERSION >= '4')			/* SR �ʹߤϡ�V2	  */
	    boot_basic = BASIC_V2;
	else					/* ��������ϡ�V1S	  */
	    boot_basic = BASIC_V1S;
    }

	/* �������(I/II)�Υݡ��Ȥ�����	 */

    if (sound_board == SOUND_II) {

	if      (ROM_VERSION >= '8')		/* FH/MH �ʹߤϡ�44��47H */
	    sound_port = SD_PORT_44_45 | SD_PORT_46_47;
	else if (ROM_VERSION >= '4')		/* SR �ʹߤϡ�44��45,A8��ADH */
	    sound_port = SD_PORT_44_45 | SD_PORT_A8_AD;
	else					/* ��������ϡ�  A8��ADH */
	    sound_port = SD_PORT_A8_AD;

    } else {

	if (ROM_VERSION >= '4')			/* SR�ʹߤϡ�44��45H	 */
	    sound_port = SD_PORT_44_45;
	else					/* ��������ϡ�������	 */
	  /*sound_port = SD_PORT_A8_AD;*/
	    sound_port = 0;			/*	�б����ʤ��ʤ� 0 */
    }
}


void	pc88main_init( int init )
{
  int i;

  bootup_work_init();

	/* CPU �������� */

  if( init == INIT_POWERON  ||  init == INIT_RESET ){

    z80_reset( &z80main_cpu );
  }

  pc88main_bus_setup();

  z80main_cpu.intr_update = main_INT_update;
  z80main_cpu.intr_ack    = main_INT_chk;

  z80main_cpu.break_if_halt = FALSE;		/* for debug */
  z80main_cpu.PC_prev   = z80main_cpu.PC;	/* dummy for monitor */

#ifdef	DEBUGLOG
  z80main_cpu.log	= TRUE;
#else
  z80main_cpu.log	= FALSE;
#endif


	/* RAM���Ÿ��������ѥ�����ǽ���� */

  if( init == INIT_POWERON ){
    power_on_ram_init();
  }


	/* �ե���Ƚ���� */

  if( init == INIT_POWERON  ||  init == INIT_RESET ){

    memory_reset_font();
  }else{
    memory_set_font();
  }


	/* �����ܡ��ɽ���� */

  if( init == INIT_POWERON  ||  init == INIT_STATELOAD ){
    keyboard_reset();
  }



  printer_init();			/* PRINTER ���������ʤ�	*/

  if( init == INIT_POWERON  ||  init == INIT_RESET ){

    sio_init();

    main_INT_init();

    crtc_init();
    dmac_init();
    calendar_init();
    keyboard_jop1_reset();
    sound_board_init();
    pio_init();


    dipsw_1 = (boot_dipsw   ) & SW_1_MASK;
    dipsw_2 = (boot_dipsw>>8) & SW_2_MASK;

    switch( boot_basic ){
    case BASIC_N:
      dipsw_1 &= ~SW_N88;
      dipsw_2 |=  SW_V1;
      dipsw_2 &= ~SW_H;
      high_mode = FALSE;
      break;
    case BASIC_V1S:
      dipsw_1 |=  SW_N88;
      dipsw_2 |=  SW_V1;
      dipsw_2 &= ~SW_H;
      high_mode = FALSE;
      break;
    case BASIC_V1H:
      dipsw_1 |=  SW_N88;
      dipsw_2 |=  SW_V1;
      dipsw_2 |=  SW_H;
      high_mode = TRUE;
      break;
    case BASIC_V2:
      dipsw_1 |=  SW_N88;
      dipsw_2 &= ~SW_V1;
      dipsw_2 |=  SW_H;
      high_mode = TRUE;
      break;
    }

    ctrl_boot		= (boot_from_rom) ? SW_ROMBOOT : 0;
    memory_bank		= MEMORY_BANK_MAIN;
    cpu_clock		= (boot_clock_4mhz) ? SW_4MHZ : 0;

    sys_ctrl		= 0x31;
    grph_ctrl		= 0x31;
    misc_ctrl		= 0x90;
    ALU1_ctrl		= 0x77;
    ALU2_ctrl		= 0x00;
    ctrl_signal		= 0x0f;
    grph_pile		= 0x00;
  /*baudrate_sw		= 0;*/
    window_offset	= 0x0000;
    ext_rom_bank	= 0xff;

    ext_ram_ctrl	= 0;
    ext_ram_bank	= 0;

    jisho_rom_ctrl	= JISHO_NOT_SELECT;
    jisho_rom_bank	= 0;

    vram_bg_palette.blue  = 0;
    vram_bg_palette.red   = 0;
    vram_bg_palette.green = 0;
    for( i=0; i<8; i++ ){
      vram_palette[ i ].blue  = (i&1) ? 7 : 0;
      vram_palette[ i ].red   = (i&2) ? 7 : 0;
      vram_palette[ i ].green = (i&4) ? 7 : 0;
    }

    intr_level		= 7;
    intr_priority	= 0;
    intr_sio_enable	= 0x00;
    intr_vsync_enable	= 0x00;
    intr_rtc_enable	= 0x00;
  }

  main_memory_mapping_0000_7fff();
  main_memory_mapping_8000_83ff();
  main_memory_mapping_c000_ffff();
  main_memory_vram_mapping();


  /* CRTC/DMAC��Ϣ�ˤ������ */
  set_text_display();
  frameskip_blink_reset();

  /* ���ꥢ��ޥ�������� */
  if (use_siomouse) {
    sio_mouse_init(TRUE);
  }

  /* ������ɤˤĤ��ơ����� */
  if( init == INIT_STATELOAD ){
    sound_output_after_stateload();
  }

}


/************************************************************************/
/* PC88 �ᥤ�󥷥��ƥ� ��λ						*/
/************************************************************************/
void	pc88main_term( void )
{
  printer_term();
  sio_term();
}










/************************************************************************/
/* �֥졼���ݥ���ȴ�Ϣ							*/
/************************************************************************/
INLINE	void	check_break_point( int type, word addr, byte data, char *str )
{
  int	i;

  if (quasi88_is_monitor())  return; /* ��˥����⡼�ɻ��ϥ��롼 */
  for( i=0; i<NR_BP; i++ ){
    if( break_point[BP_MAIN][i].type == type &&
        break_point[BP_MAIN][i].addr == addr ){
      printf( "*** Break at %04x *** "
	      "( MAIN - #%d [ %s %04XH , data = %02XH ]\n",
	      z80main_cpu.PC.W, i+1, str, addr, data );
      quasi88_debug();
      break;
    }
  }
}

byte	main_fetch_with_BP( word addr )
{
  byte	data = main_fetch( addr );
  check_break_point( BP_READ, addr, data, "FETCH from" );
  return data;
}

byte	main_mem_read_with_BP( word addr )
{
  byte	data = main_mem_read( addr );
  check_break_point( BP_READ, addr, data, "READ from" );
  return data;
}

void	main_mem_write_with_BP( word addr, byte data )
{
  main_mem_write( addr, data );
  check_break_point( BP_WRITE, addr, data, "WRITE to" );
}

byte	main_io_in_with_BP( byte port )
{
  byte	data =  main_io_in( port );
  check_break_point( BP_IN, port, data, "IN from" );
  return data;
}

void	main_io_out_with_BP( byte port, byte data )
{
  main_io_out( port, data );
  check_break_point( BP_OUT, port, data, "OUT to" );
}



/************************************************************************/
/*									*/
/************************************************************************/
void	pc88main_bus_setup( void )
{
#ifdef	USE_MONITOR

  int	i, buf[4];
  for( i=0; i<4; i++ ) buf[i]=0;
  for( i=0; i<NR_BP; i++ ){
    switch( break_point[BP_MAIN][i].type ){
    case BP_READ:	buf[0]++;	break;
    case BP_WRITE:	buf[1]++;	break;
    case BP_IN:		buf[2]++;	break;
    case BP_OUT:	buf[3]++;	break;
    }
  }
   
  if( memory_wait || highspeed_mode ){
    if( buf[0] ) z80main_cpu.fetch   = main_fetch_with_BP;
    else         z80main_cpu.fetch   = main_fetch;
  }else{
    if( buf[0] ) z80main_cpu.fetch   = main_mem_read_with_BP;
    else         z80main_cpu.fetch   = main_mem_read;
  }

  if( buf[0] ) z80main_cpu.mem_read  = main_mem_read_with_BP;
  else         z80main_cpu.mem_read  = main_mem_read;

  if( buf[1] ) z80main_cpu.mem_write = main_mem_write_with_BP;
  else         z80main_cpu.mem_write = main_mem_write;

  if( buf[2] ) z80main_cpu.io_read   = main_io_in_with_BP;
  else         z80main_cpu.io_read   = main_io_in;

  if( buf[3] ) z80main_cpu.io_write  = main_io_out_with_BP;
  else         z80main_cpu.io_write  = main_io_out;

#else

  if( memory_wait || highspeed_mode ){
    z80main_cpu.fetch   = main_fetch;
  }else{
    z80main_cpu.fetch   = main_mem_read;
  }
  z80main_cpu.mem_read  = main_mem_read;
  z80main_cpu.mem_write = main_mem_write;
  z80main_cpu.io_read   = main_io_in;
  z80main_cpu.io_write  = main_io_out;

#endif
}




/***********************************************************************
 * ���ơ��ȥ��ɡ����ơ��ȥ�����
 ************************************************************************/

#define	SID	"MAIN"
#define	SID2	"MAI2"

static	T_SUSPEND_W	suspend_pc88main_work[]=
{
  { TYPE_STR,	&file_tape[0][0],	},
  { TYPE_STR,	&file_tape[1][0],	},

  { TYPE_INT,	&boot_basic,		},
  { TYPE_INT,	&boot_dipsw,		},
  { TYPE_INT,	&boot_from_rom,		},
  { TYPE_INT,	&boot_clock_4mhz,	},
  { TYPE_INT,	&monitor_15k,		},

  { TYPE_PAIR,	&z80main_cpu.AF,	},
  { TYPE_PAIR,	&z80main_cpu.BC,	},
  { TYPE_PAIR,	&z80main_cpu.DE,	},
  { TYPE_PAIR,	&z80main_cpu.HL,	},
  { TYPE_PAIR,	&z80main_cpu.IX,	},
  { TYPE_PAIR,	&z80main_cpu.IY,	},
  { TYPE_PAIR,	&z80main_cpu.PC,	},
  { TYPE_PAIR,	&z80main_cpu.SP,	},
  { TYPE_PAIR,	&z80main_cpu.AF1,	},
  { TYPE_PAIR,	&z80main_cpu.BC1,	},
  { TYPE_PAIR,	&z80main_cpu.DE1,	},
  { TYPE_PAIR,	&z80main_cpu.HL1,	},
  { TYPE_BYTE,	&z80main_cpu.I,		},
  { TYPE_BYTE,	&z80main_cpu.R,		},
  { TYPE_BYTE,	&z80main_cpu.R_saved,	},
  { TYPE_CHAR,	&z80main_cpu.IFF,	},
  { TYPE_CHAR,	&z80main_cpu.IFF2,	},
  { TYPE_CHAR,	&z80main_cpu.IM,	},
  { TYPE_CHAR,	&z80main_cpu.HALT,	},
  { TYPE_INT,	&z80main_cpu.INT_active,},
  { TYPE_INT,	&z80main_cpu.icount,	},
  { TYPE_INT,	&z80main_cpu.state0,	},
  { TYPE_INT,	&z80main_cpu.skip_intr_chk,	},
  { TYPE_CHAR,	&z80main_cpu.log,		},
  { TYPE_CHAR,	&z80main_cpu.break_if_halt,	},

  { TYPE_INT,	&high_mode,		},

  { TYPE_BYTE,	&dipsw_1,		},
  { TYPE_BYTE,	&dipsw_2,		},
  { TYPE_BYTE,	&ctrl_boot,		},
  { TYPE_INT,	&memory_bank,		},
  { TYPE_BYTE,	&cpu_clock,		},

  { TYPE_BYTE,	&common_out_data,	},
  { TYPE_BYTE,	&misc_ctrl,		},
  { TYPE_BYTE,	&ALU1_ctrl,		},
  { TYPE_BYTE,	&ALU2_ctrl,		},
  { TYPE_BYTE,	&ctrl_signal,		},
  { TYPE_BYTE,	&baudrate_sw,		},
  { TYPE_WORD,	&window_offset,		},
  { TYPE_BYTE,	&ext_rom_bank,		},
  { TYPE_BYTE,	&ext_ram_ctrl,		},
  { TYPE_BYTE,	&ext_ram_bank,		},

  { TYPE_PAIR,	&kanji1_addr,		},
  { TYPE_PAIR,	&kanji2_addr,		},

  { TYPE_BYTE,	&jisho_rom_bank,	},
  { TYPE_BYTE,	&jisho_rom_ctrl,	},

  { TYPE_INT,	&calendar_stop,		},
  { TYPE_CHAR,	&calendar_data[0],	},
  { TYPE_CHAR,	&calendar_data[1],	},
  { TYPE_CHAR,	&calendar_data[2],	},
  { TYPE_CHAR,	&calendar_data[3],	},
  { TYPE_CHAR,	&calendar_data[4],	},
  { TYPE_CHAR,	&calendar_data[5],	},
  { TYPE_CHAR,	&calendar_data[6],	},

  { TYPE_INT,	&cmt_speed,		},
  { TYPE_INT,	&cmt_intr,		},
  { TYPE_INT,	&cmt_wait,		},

  { TYPE_INT,	&highspeed_mode,	},
  { TYPE_INT,	&memory_wait,		},

  { TYPE_INT,	&ALU_buf.l,	},	/*  TYPE_CHAR, ALU_buf.c[0]-[3] ?  */
  { TYPE_INT,	&ALU_comp.l,	},	/*  TYPE_CHAR, ALU_comp.c[0]-[3] ? */

  { TYPE_INT,	&pcg_data,		},
  { TYPE_INT,	&pcg_addr,		},

  { TYPE_INT,	&sio_instruction,	},
  { TYPE_BYTE,	&sio_mode,		},
  { TYPE_BYTE,	&sio_command,		},
  { TYPE_INT,	&sio_data_exist,	},
  { TYPE_BYTE,	&sio_data,		},

  { TYPE_INT,	&com_X_flow,		},

  { TYPE_INT,	&cmt_dummy_read_cnt,	},
  { TYPE_INT,	&cmt_skip,		},
  { TYPE_INT,	&cmt_skip_data,		},

  { TYPE_LONG,	&cmt_read_chars,	},

  { TYPE_CHAR,	&shift_reg[0],		},
  { TYPE_CHAR,	&shift_reg[1],		},
  { TYPE_CHAR,	&shift_reg[2],		},
  { TYPE_CHAR,	&shift_reg[3],		},
  { TYPE_CHAR,	&shift_reg[4],		},
  { TYPE_CHAR,	&shift_reg[5],		},
  { TYPE_CHAR,	&shift_reg[6],		},
  { TYPE_CHAR,	&calendar_cdo,		},
  { TYPE_INT,	&calendar_diff,		},

  { TYPE_END,	0			},
};

static	T_SUSPEND_W	suspend_pc88main_work2[]=
{
  { TYPE_INT,	&use_siomouse,		},
  { TYPE_END,	0			},
};


int	statesave_pc88main( void )
{
/*if( fp_ti ) printf("%d\n",osd_ftell(fp_ti));*/

  if( statesave_table( SID, suspend_pc88main_work ) != STATE_OK ) return FALSE;

  if( statesave_table(SID2, suspend_pc88main_work2) != STATE_OK ) return FALSE;

  return TRUE;
}

int	stateload_pc88main( void )
{
  if( stateload_table( SID, suspend_pc88main_work ) != STATE_OK ) return FALSE;

  if( stateload_table(SID2, suspend_pc88main_work2) != STATE_OK ) {

    /* ��С������ʤ顢�ߤΤ��� */

    printf( "stateload : Statefile is old. (ver 0.6.0, 1, 2 or 3?)\n" );

    use_siomouse = FALSE;
  }


  cmt_stateload_chars = cmt_read_chars;
  cmt_read_chars = 0;

  cmt_stateload_skip = cmt_skip;

  return TRUE;
}
