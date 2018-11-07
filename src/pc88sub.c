/************************************************************************/
/*									*/
/* PC8801 ���֥����ƥ�(FDD¦)						*/
/*									*/
/************************************************************************/

#include <stdio.h>

#include "quasi88.h"
#include "pc88sub.h"

#include "pc88cpu.h"
#include "fdc.h"
#include "screen.h"	/* state_of_vsync */
#include "intr.h"	/* state_of_vsync */
#include "event.h"
#include "memory.h"
#include "pio.h"

#include "emu.h"
#include "suspend.h"



z80arch	z80sub_cpu;			/* Z80 CPU ( sub system )	*/

int	sub_load_rate = 6;		/*				*/

/************************************************************************/
/* ���ꥢ������							*/
/*			���ꥢ��������������ˡ�ϡ��޾���줵���	*/
/*			�����ˤ�ꡢ���ɡ�				*/
/*				Copyright (c) kenichi kasamatsu		*/
/************************************************************************/
/*----------------------*/
/*    ���ꡦ�ե��å�	*/
/*----------------------*/
byte	sub_fetch( word addr )
{
  if( memory_wait ){
    if( addr < 0x4000 )
      z80sub_cpu.state0 += 1;		/* M1�������륦������ */
  }

#if 0
  if( verbose_io ){
    if( ( 0x2000 <= addr && addr < 0x4000 ) || ( addr & 0x8000 ) ){
      printf("SUB Memory Read BAD %04x\n",addr);
    }
  }
#endif
  return sub_romram[ addr & 0x7fff ];
}


/*----------------------*/
/*    ���ꡦ�꡼��	*/
/*----------------------*/
byte	sub_mem_read( word addr )
{
#if 0
  if( verbose_io ){
    if( ( 0x2000 <= addr && addr < 0x4000 ) || ( addr & 0x8000 ) ){
      printf("SUB Memory Read BAD %04x\n",addr);
    }
  }
#endif
  return sub_romram[ addr & 0x7fff ];
}


/*----------------------*/
/*     ����饤��	*/
/*----------------------*/
void	sub_mem_write( word addr, byte data )
{
  if( (addr & 0xc000) == 0x4000 ){

    sub_romram[ addr & 0x7fff ] = data;

  }else{

    if( verbose_io ) printf("SUB Memory Write BAD %04x\n",addr);
    if( (addr & 0x4000 ) == 0x4000 ){
      sub_romram[ addr & 0x7fff ] = data;
    }

  }
}




/************************************************************************/
/* �ɡ��ϥݡ��ȥ�������							*/
/************************************************************************/

/*----------------------*/
/*    �ݡ��ȡ��饤��	*/
/*----------------------*/

void	sub_io_out( byte port, byte data )
{
  switch( port ){

  case 0xf4:				/* �ɥ饤�֥⡼�ɡ� 2D/2DD/2HD ? */
    return;
  case 0xf7:				/* �ץ�󥿥ݡ��Ƚ���		*/
    return;
  case 0xf8:				/* �ɥ饤�֥⡼���������	*/
    return;

  case 0xfb:				/* FDC �ǡ��� WRITE */
    fdc_write( data );
    CPU_REFRESH_INTERRUPT();
    return;

	/* �Уɣ� */

  case 0xfc:
    logpio("   <==%02x\n",data);
    pio_write_AB( PIO_SIDE_S, PIO_PORT_A, data );
    return;
  case 0xfd:
    logpio("   <--%02x\n",data);
    pio_write_AB( PIO_SIDE_S, PIO_PORT_B, data );
    return;
  case 0xfe:
    pio_write_C_direct( PIO_SIDE_S, data );
    return;
  case 0xff:
    if( data & 0x80 ) pio_set_mode( PIO_SIDE_S, data );
    else              pio_write_C( PIO_SIDE_S, data );
    return;

  }



  if( verbose_io ) printf("SUB OUT data %02X to undecoeded port %02XH\n",data,port);

}

/*----------------------*/
/*    �ݡ��ȡ��꡼��	*/
/*----------------------*/

byte	sub_io_in( byte port )
{
  switch( port ){

  case 0xf8:				/* FDC �� TC �����	*/
    CPU_REFRESH_INTERRUPT();
    fdc_TC();
    return 0xff;

  case 0xfa:				/* FDC ���ơ����� ���� */
    CPU_REFRESH_INTERRUPT();
    return	fdc_status();
  case 0xfb:				/* FDC �ǡ��� READ */
    CPU_REFRESH_INTERRUPT();
    return	fdc_read();

	/* �Уɣ� */

  case 0xfc:
    {
      byte data = pio_read_AB( PIO_SIDE_S, PIO_PORT_A );
      logpio("   ==>%02x\n",data);
      return data;
    }
  case 0xfd:
    {
      byte data = pio_read_AB( PIO_SIDE_S, PIO_PORT_B );
      logpio("   -->%02x\n",data);
      return data;
    }
  case 0xfe:
    return pio_read_C( PIO_SIDE_S );

  }

  if( verbose_io ) printf("SUB IN        from undecoeded port %02XH\n",port);

  return 0xff;
}






/************************************************************************/
/* Peripheral ���ߥ�졼�����						*/
/************************************************************************/

/************************************************************************/
/* �ޥ����֥�����ߥ��ߥ�졼��					*/
/************************************************************************/

/*------------------------------*/
/* �����(Z80�ꥻ�åȻ��˸Ƥ�)	*/
/*------------------------------*/
void	sub_INT_init( void )
{
  FDC_flag = FALSE;
}

/*----------------------------------------------------------------------*/
/* �����ߤ��������롣��Ʊ���ˡ����γ����ߤޤǤΡ��Ǿ� state ��׻�	*/
/*	�����ͤϡ�Z80����������λ�Υե饰(TRUE/FALSE)			*/
/*----------------------------------------------------------------------*/
void	sub_INT_update( void )
{
  static int sub_total_state = 0;	/* ����CPU����������̿���      */
  int icount;

  icount = fdc_ctrl( z80sub_cpu.state0 );

  if( FDC_flag ){ z80sub_cpu.INT_active = TRUE;  }
  else          { z80sub_cpu.INT_active = FALSE; }


	/* ���������������ɽ���ν����ϡ��ᥤ��CPU������	*/
	/* �ԤʤäƤ��뤿�ᡢ-cpu 0 ������ˡ�����CPU�����椬	*/
	/* �ܤä��ޤ���äƤ��ʤ��ʤ�ȡ���˥塼���̤ؤΰܹ�	*/
	/* �ʤɤ��Ǥ��ʤ��ʤ롣					*/
	/* �����ǡ�����CPU�Ǥ������ֽ�����Ԥʤ����Ӥˡ�	*/
	/* �����������Ȳ���ɽ����Ԥʤ����Ȥˤ��롣		*/
	/* �����������٤ϥǥ��������������ʤɤΥ���CPU�ν�����	*/
	/* ���������ˡ�������ɤ��ߤޤ�ʤɤ��������ФƤ�����	*/
	/* �ʤΤǡ����Ρ֥���CPU�Ǥ⥭���������׽�����Ԥʤ�	*/
	/* ���٤��ѹ��Ǥ���褦�ˤ��Ƥ�������			*/

	/* D.C.���ͥ������Ȥ��� */

  if( sub_load_rate && cpu_timing < 2 ){
    sub_total_state += z80sub_cpu.state0;
    if( sub_total_state/sub_load_rate >= state_of_vsync ){

      CPU_BREAKOFF();
      quasi88_event_flags |= (EVENT_FRAME_UPDATE | EVENT_AUDIO_UPDATE);
      sub_total_state = 0;
    }
  }

  z80sub_cpu.icount = (icount<0) ? 999999 : icount;


	/* ��˥塼�ؤ����ܤʤɤϡ������ǳ�ǧ */

  if (quasi88_event_flags & EVENT_MODE_CHANGED) {
    CPU_BREAKOFF();
  }
}


/*----------------------------------------------*/
/* �����å� (������Ļ� 1���ƥå���˸ƤФ��)	*/
/*----------------------------------------------*/
int	sub_INT_chk( void )
{
  z80sub_cpu.INT_active = FALSE;

  if( FDC_flag ) return 0;
  else           return -1;
}










/************************************************************************/
/* PC88 ���֥����ƥ� �����						*/
/************************************************************************/
void	pc88sub_init( int init )
{

	/* Z80 ���ߥ�졼���������� */

  if( init == INIT_POWERON  ||  init == INIT_RESET ){

    z80_reset( &z80sub_cpu );
  }

  pc88sub_bus_setup();

  z80sub_cpu.intr_update = sub_INT_update;
  z80sub_cpu.intr_ack    = sub_INT_chk;

  z80sub_cpu.break_if_halt = TRUE;
  z80sub_cpu.PC_prev   = z80sub_cpu.PC;		/* dummy for monitor */

#ifdef	DEBUGLOG
  z80sub_cpu.log	= TRUE;
#else
  z80sub_cpu.log	= FALSE;
#endif


  if( init == INIT_POWERON  ||  init == INIT_RESET ){

    sub_INT_init();

    /* fdc_init(); �� drive_init()    �ǽ����Ѥ� */
    /* pio_init(); �� pc88main_init() �ǽ����Ѥ� */
  }
}


/************************************************************************/
/* PC88 ���֥����ƥ� ��λ						*/
/************************************************************************/
void	pc88sub_term( void )
{
}















/************************************************************************/
/* �֥졼���ݥ���ȴ�Ϣ							*/
/************************************************************************/
INLINE	void	check_break_point( int type, word addr, char *str )
{
  int	i;

  if (quasi88_is_monitor()) return; /* ��˥����⡼�ɻ��ϥ��롼 */
  for( i=0; i<NR_BP; i++ ){
    if( break_point[BP_SUB][i].type == type &&
        break_point[BP_SUB][i].addr == addr ){
      printf( "*** Break at %04x *** ( SUB[#%d] : %s %04x )\n",
	      z80sub_cpu.PC.W, i+1, str, addr );
      quasi88_debug();
      break;
    }
  }
}

byte	sub_fetch_with_BP( word addr )
{
  check_break_point( BP_READ, addr, "FETCH from" );
  return sub_fetch( addr );
}

byte	sub_mem_read_with_BP( word addr )
{
  check_break_point( BP_READ, addr, "READ from" );
  return sub_mem_read( addr );
}

void	sub_mem_write_with_BP( word addr, byte data )
{
  check_break_point( BP_WRITE, addr, "WRITE to" );
  sub_mem_write( addr, data );
}

byte	sub_io_in_with_BP( byte port )
{
  check_break_point( BP_IN, port, "IN from" );
  return sub_io_in( port );
}

void	sub_io_out_with_BP( byte port, byte data )
{
  check_break_point( BP_OUT, port, "OUT to" );
  sub_io_out( port, data );
}






/************************************************************************/
/*									*/
/************************************************************************/
void	pc88sub_bus_setup( void )
{
#ifdef	USE_MONITOR

  int	i, buf[4];
  for( i=0; i<4; i++ ) buf[i]=0;
  for( i=0; i<NR_BP; i++ ){
    switch( break_point[BP_SUB][i].type ){
    case BP_READ:	buf[0]++;	break;
    case BP_WRITE:	buf[1]++;	break;
    case BP_IN:		buf[2]++;	break;
    case BP_OUT:	buf[3]++;	break;
    }
  }
   
  if( memory_wait ){
    if( buf[0] ) z80sub_cpu.fetch   = sub_fetch_with_BP;
    else         z80sub_cpu.fetch   = sub_fetch;
  }else{
    if( buf[0] ) z80sub_cpu.fetch   = sub_mem_read_with_BP;
    else         z80sub_cpu.fetch   = sub_mem_read;
  }

  if( buf[0] ) z80sub_cpu.mem_read  = sub_mem_read_with_BP;
  else         z80sub_cpu.mem_read  = sub_mem_read;

  if( buf[1] ) z80sub_cpu.mem_write = sub_mem_write_with_BP;
  else         z80sub_cpu.mem_write = sub_mem_write;

  if( buf[2] ) z80sub_cpu.io_read   = sub_io_in_with_BP;
  else         z80sub_cpu.io_read   = sub_io_in;

  if( buf[3] ) z80sub_cpu.io_write  = sub_io_out_with_BP;
  else         z80sub_cpu.io_write  = sub_io_out;

#else

  if( memory_wait ){
    z80sub_cpu.fetch   = sub_fetch;
  }else{
    z80sub_cpu.fetch   = sub_mem_read;
  }
  z80sub_cpu.mem_read  = sub_mem_read;
  z80sub_cpu.mem_write = sub_mem_write;
  z80sub_cpu.io_read   = sub_io_in;
  z80sub_cpu.io_write  = sub_io_out;

#endif
}




/***********************************************************************
 * ���ơ��ȥ��ɡ����ơ��ȥ�����
 ************************************************************************/

#define	SID	"SUB "

static	T_SUSPEND_W	suspend_pc88sub_work[]=
{
  { TYPE_PAIR,	&z80sub_cpu.AF,		},
  { TYPE_PAIR,	&z80sub_cpu.BC,		},
  { TYPE_PAIR,	&z80sub_cpu.DE,		},
  { TYPE_PAIR,	&z80sub_cpu.HL,		},
  { TYPE_PAIR,	&z80sub_cpu.IX,		},
  { TYPE_PAIR,	&z80sub_cpu.IY,		},
  { TYPE_PAIR,	&z80sub_cpu.PC,		},
  { TYPE_PAIR,	&z80sub_cpu.SP,		},
  { TYPE_PAIR,	&z80sub_cpu.AF1,	},
  { TYPE_PAIR,	&z80sub_cpu.BC1,	},
  { TYPE_PAIR,	&z80sub_cpu.DE1,	},
  { TYPE_PAIR,	&z80sub_cpu.HL1,	},
  { TYPE_BYTE,	&z80sub_cpu.I,		},
  { TYPE_BYTE,	&z80sub_cpu.R,		},
  { TYPE_BYTE,	&z80sub_cpu.R_saved,	},
  { TYPE_CHAR,	&z80sub_cpu.IFF,	},
  { TYPE_CHAR,	&z80sub_cpu.IFF2,	},
  { TYPE_CHAR,	&z80sub_cpu.IM,		},
  { TYPE_CHAR,	&z80sub_cpu.HALT,	},
  { TYPE_INT,	&z80sub_cpu.INT_active,	},
  { TYPE_INT,	&z80sub_cpu.icount,	},
  { TYPE_INT,	&z80sub_cpu.state0,	},
  { TYPE_INT,	&z80sub_cpu.skip_intr_chk,	},
  { TYPE_CHAR,	&z80sub_cpu.log,		},
  { TYPE_CHAR,	&z80sub_cpu.break_if_halt,	},

  { TYPE_INT,	&sub_load_rate,		},

  { TYPE_END,	0			},
};


int	statesave_pc88sub( void )
{
  if( statesave_table( SID, suspend_pc88sub_work ) == STATE_OK ) return TRUE;
  else                                                           return FALSE;
}

int	stateload_pc88sub( void )
{
  if( stateload_table( SID, suspend_pc88sub_work ) == STATE_OK ) return TRUE;
  else                                                           return FALSE;
}
