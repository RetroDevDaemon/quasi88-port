/************************************************************************/
/*									*/
/* PIO �� ����								*/
/*									*/
/************************************************************************/

#include <stdio.h>

#include "quasi88.h"
#include "pio.h"

#include "pc88cpu.h"

#include "emu.h"
#include "suspend.h"


/*
  PIO���������ȡ�����CPU�ζ�ư
	  PC88 �ϡ�����¦�� �ᥤ��CPU���ǥ������ɥ饤��¦�� ����CPU �ȡ�
	2�Ĥ�CPU����äƤ��롣�ǥ��������������򤹤���ϡ��ᥤ��CPU ��
	����CPU �� PIO��ͳ�ǥ��ޥ�ɤ��������������������� ����CPU ��
	�ºݤ˽�����Ԥäơ���̤�ᥤ��CPU ����������褦�ˤʤäƤ��롣
	�Ĥޤꡢ����CPU�ϥǥ�����������ʳ��λ��֤����ơ��ᥤ��CPU 
	����Υ��ޥ�ɤμ����Ԥ��Ȥ������Ȥˤʤ롣
	  �ʾ�Τ��Ȥ�ꡢ����CPU�������ߥ�졼�Ȥ���Τ�̵�̤ʤΤǡ�
	ɬ�פʻ��Τߥ��ߥ�졼�Ȥ��뤳�Ȥˤ��롣

	  �̾�ᥤ�󡦥���CPU�Ȥ��̿����ˤϡ�PIO �� C�ݡ��Ȥ�꡼�ɤ��롣
	�����ǡ��ᥤ��CPU �� C�ݡ��Ȥ�꡼�ɤ������ʹߤϡ�����CPU �Τߤ�
	��ư����������CPU �� C�ݡ��Ȥ�꡼�ɤ������ʹߤϡ��ᥤ��CPU �Τߤ�
	��ư������褦�ˤ��롣
		(-cpu 0 ���ץ�������ꤷ�����Ͼ嵭��ư��򤹤�)

	  ����������ˤ� C�ݡ��Ȥ�𤵤��ˡ��̿���Ԥ����ץꥱ��������
	¸�ߤ��롣(2�Ĥ�CPU�ν���®�٤�Ʊ���˰�¸���Ƥ���)�������к��Ȥ��ơ�
	A/B/C �ݡ��ȤΤ����줫�˥ᥤ��CPU����������������������������
	(ɸ��Ǥ� 4000���ƥå�)������CPU��Ʊ���˶�ư�����롣
		(-cpu 1 ���ץ�������ꤷ�����Ͼ嵭��ư��򤹤�)

	  ����Ǥ�ޤ�ư���ʤ����ץꥱ������󤬤���Τǡ������к��Ȥ��ơ�
	�ᥤ��CPU �ȥ���CPU ������Ʊ���˶�ư�����롣
		(-cpu 2 ���ץ�������ꤷ�����Ͼ嵭��ư��򤹤�)
*/

/*---------------------------------------------------------------------------*/
/* ����									     */
/*					A  ######## --\/-- ######## A	     */
/*	���饤�Ȥϼ�ʬ�Υ����	B  ######## --/\-- ######## B	     */
/*	  �Ф��ƹԤʤ���		CH ####     --\/-- ####	    CH	     */
/*	���꡼�ɤ����Υ��		CL     #### --/\--     #### CL	     */
/*	  ����Ԥʤ���							     */
/*	  ��ʬ�Υ����READ����ʤ顢					     */
/*	  ��ʬ�Υ�����ɤࡣ						     */
/*	���ݡ���C�Υ꡼�ɥ饤�Ȼ����㳰������Ԥʤ���			     */
/*---------------------------------------------------------------------------*/


pio_work	pio_AB[2][2], pio_C[2][2];


static	z80arch	*z80[2] = { &z80main_cpu, &z80sub_cpu };




/*----------------------------------------------------------------------*/
/* PIO �����								*/
/*	PA / PCL ���� �� PB / PCH ����					*/
/*----------------------------------------------------------------------*/
void	pio_init( void )
{
  int	side;

  for( side=0; side<2; side++ ){
    pio_AB[ side ][ PIO_PORT_A ].type    = PIO_READ;
    pio_AB[ side ][ PIO_PORT_A ].exist   = PIO_EMPTY;
/*  pio_AB[ side ][ PIO_PORT_A ].cont_f  = not used ! */
    pio_AB[ side ][ PIO_PORT_A ].data    = 0x00;

    pio_AB[ side ][ PIO_PORT_B ].type    = PIO_WRITE;
    pio_AB[ side ][ PIO_PORT_B ].exist   = PIO_EMPTY;
/*  pio_AB[ side ][ PIO_PORT_B ].cont_f  = not used ! */
    pio_AB[ side ][ PIO_PORT_B ].data    = 0x00;

    pio_C[ side ][ PIO_PORT_CH ].type    = PIO_WRITE;
    pio_C[ side ][ PIO_PORT_CH ].cont_f  = 1;
    pio_C[ side ][ PIO_PORT_CH ].data    = 0x00;

    pio_C[ side ][ PIO_PORT_CL ].type    = PIO_READ;
    pio_C[ side ][ PIO_PORT_CL ].cont_f  = 1;
    pio_C[ side ][ PIO_PORT_CL ].data    = 0x00;
  }

}


/* verbose ������Υ�å�����ɽ���ޥ���					*/

#define	pio_mesAB( s )							\
	if( verbose_pio )						\
	  printf( s " : side = %s : port = %s\n",			\
		 (side==PIO_SIDE_M)?"M":"S", (port==PIO_PORT_A)?"A":"B" )

#define	pio_mesC( s )							\
	if( verbose_pio )						\
	  printf( s " : side = %s\n",					\
		 (side==PIO_SIDE_M)?"M":"S" )

/*----------------------------------------------------------------------*/
/* PIO A or B ����꡼��						*/
/*	�꡼�ɤκݤΥǡ����ϡ�����¦���դΥݡ��Ȥ����ɤ߽Ф���	*/
/*		���Υݡ��Ȥ����꤬  READ �ʤ饨�顼ɽ��		*/
/*		��ʬ�Υݡ��Ȥ����꤬ WRITE �ʤ饨�顼ɽ��		*/
/*		Ϣ³�꡼�ɤκݤϡ������󥿤򥫥���ȥ����󤹤롣	*/
/*			�����󥿤� 0 �ʤ�����ơ�Ϣ³�꡼�ɤ��롣	*/
/*			�����󥿤� 1 �ʾ�ʤ顢CPU �����ؤ��롣		*/
/*----------------------------------------------------------------------*/
byte	pio_read_AB( int side, int port )
{
		/* �ݡ���°���԰��� */

  if( pio_AB[ side^1 ][ port^1 ].type == PIO_READ  ){	/* ���ݡ��Ȥ� READ */
    pio_mesAB( "PIO AB READ PORT Mismatch" );
  }
  if( pio_AB[ side   ][ port   ].type == PIO_WRITE ){	/* ��ʬ�ݡ��Ȥ� WRITE*/
    pio_mesAB( "PIO Read from WRITE-PORT" );
    return (pio_AB[ side ][ port ].data);
  }
		/* �ɤߤ��� */

  if( pio_AB[ side^1 ][ port^1 ].exist == PIO_EXIST ){	/* -- �ǽ���ɤߤ��� */

    pio_AB[ side^1 ][ port^1 ].exist   = PIO_EMPTY;

  }else{						/* -- Ϣ³���ɤߤ��� */

    switch( cpu_timing ){
    case 1:						/*     1:����CPU��ư */
      if( side==PIO_SIDE_M ){
	dual_cpu_count = CPU_1_COUNT;
	CPU_BREAKOFF();
      } /*No Break*/
    case 0:						/*     0:���Τޤ��ɤ�*/
    case 2:						/*     2:���Τޤ��ɤ�*/
      pio_mesAB( "PIO Read continuously" );
      break;
    }

  }
  return (pio_AB[ side^1 ][ port^1 ].data);
}


/*----------------------------------------------------------------------*/
/* PIO A or B �˥饤��							*/
/*	�饤�Ȥϡ���ʬ��¦����ʬ�Υݡ��Ȥ��Ф��ƹԤʤ���		*/
/*		���Υݡ��Ȥ����꤬ WRITE �ʤ饨�顼ɽ��		*/
/*		��ʬ�Υݡ��Ȥ����꤬  READ �ʤ饨�顼ɽ��		*/
/*		Ϣ³�饤�Ȥκݤϡ������󥿤򥫥���ȥ����󤹤롣	*/
/*			�����󥿤� 0 �ʤ�����ơ�Ϣ³�饤�Ȥ��롣	*/
/*			�����󥿤� 1 �ʾ�ʤ顢CPU �����ؤ��롣		*/
/*----------------------------------------------------------------------*/
void	pio_write_AB( int side, int port, byte data )
{
		/* �ݡ���°���԰��� */

  if( pio_AB[ side^1 ][ port^1 ].type == PIO_WRITE ){	/* ���Υݡ��� WRITE*/
    pio_mesAB( "PIO AB Write PORT Mismatch" );
  }
  if( pio_AB[ side   ][ port   ].type == PIO_READ ){	/* ��ʬ�Υݡ��� READ */
    pio_mesAB( "PIO Write to READ-PORT" );
  }
		/* �񤭹��� */

  if( pio_AB[ side ][ port ].exist == PIO_EMPTY ){	/* -- �ǽ�ν񤭹��� */

    pio_AB[ side ][ port ].exist   = PIO_EXIST;
    pio_AB[ side ][ port ].data    = data;

  }else{						/* -- Ϣ³�ν񤭹��� */

    switch( cpu_timing ){
    case 1:						/*     1:����CPU��ư */
      if( side==PIO_SIDE_M ){
	dual_cpu_count = CPU_1_COUNT;
	CPU_BREAKOFF();
      } /*No Break*/
    case 0:						/*     0:���Τޤ޽�*/
    case 2:						/*     2:���Τޤ޽�*/
      pio_mesAB( "PIO Write continuously" );
      pio_AB[ side ][ port ].data    = data;
      break;
    }

  }
  return;
}








/*----------------------------------------------------------------------*/
/* PIO C ����꡼��							*/
/*	�꡼�ɤκݤΥǡ����ϡ�����¦���դΥݡ��Ȥ����ɤ߽Ф���	*/
/*		���Υݡ��Ȥ����꤬  READ �ʤ饨�顼ɽ��		*/
/*		��ʬ�Υݡ��Ȥ����꤬ WRITE �ʤ饨�顼ɽ��		*/
/*		�꡼�ɤκݤˡ�CPU�����ؤ�Ƚ��������			*/
/*----------------------------------------------------------------------*/
byte	pio_read_C( int side )
{
  byte	data;

		/* �ݡ���°���԰��� */
  if( pio_C[ side^1 ][ PIO_PORT_CH ].type == PIO_READ  &&
      pio_C[ side^1 ][ PIO_PORT_CL ].type == PIO_READ  ){
    pio_mesC( "PIO C READ PORT Mismatch" );
  }
  if( pio_C[ side   ][ PIO_PORT_CH ].type == PIO_WRITE &&
      pio_C[ side   ][ PIO_PORT_CL ].type == PIO_WRITE ){
    pio_mesC( "PIO C Read from WRITE-PORT" );
  }
		/* �꡼�� */

  if( pio_C[ side ][ PIO_PORT_CH ].type == PIO_READ ){
    data  = pio_C[ side^1 ][ PIO_PORT_CL ].data << 4;
  }else{
    data  = pio_C[ side   ][ PIO_PORT_CH ].data << 4;
  }

  if( pio_C[ side ][ PIO_PORT_CL ].type == PIO_READ ){
    data |= pio_C[ side^1 ][ PIO_PORT_CH ].data;
  }else{
    data |= pio_C[ side   ][ PIO_PORT_CL ].data;
  }

  pio_C[ side ][ PIO_PORT_CL ].cont_f ^= 1;
  if( pio_C[ side ][ PIO_PORT_CL ].cont_f == 0 ){	/* -- Ϣ³���ɤߤ��� */

    switch( cpu_timing ){
    case 0:						/*     0:CPU�����ؤ�*/
      select_main_cpu ^= 1;
      CPU_BREAKOFF();        /* PC-=2 */
      break;
    case 1:						/*     1:����CPU��ư */
      if( side==PIO_SIDE_M ){
	dual_cpu_count = CPU_1_COUNT;
	CPU_BREAKOFF();
      }
      break;
    case 2:						/*     2:�ʤˤ⤷�ʤ�*/
      break;
    }

  }

  return data;
}


/*----------------------------------------------------------------------*/
/* PIO C �˥饤��							*/
/*	�饤�Ȥϡ���ʬ��¦����ʬ�Υݡ��Ȥ��Ф��ƹԤʤ���		*/
/*		���Υݡ��Ȥ����꤬ WRITE �ʤ饨�顼ɽ��		*/
/*		��ʬ�Υݡ��Ȥ����꤬  READ �ʤ饨�顼ɽ��		*/
/*		�饤�Ȥκݤˡ�CPU�����ؤ�Ƚ��������			*/
/*----------------------------------------------------------------------*/
void	pio_write_C( int side, byte data )
{
  int port;

  if( data & 0x08 ) port = PIO_PORT_CH;
  else              port = PIO_PORT_CL;
  data &= 0x07;

		/* �ݡ���°���԰��� */

  if( pio_C[ side^1 ][ port^1 ].type == PIO_WRITE ){	/* ���Υݡ��� WRITE*/
    pio_mesC( "PIO C Write PORT Mismatch" );
  }
  if( pio_C[ side   ][ port   ].type == PIO_READ ){	/* ��ʬ�Υݡ��� READ */
    pio_mesC( "PIO C Write to READ-PORT" );
  }
		/* �饤�� */

  if( data & 0x01 ) pio_C[ side ][ port ].data |=  ( 1 << (data>>1) );
  else              pio_C[ side ][ port ].data &= ~( 1 << (data>>1) );

  switch( cpu_timing ){
  case 0:						/*     0:���Τޤ޽�*/
  case 2:						/*     2:���Τޤ޽�*/
    break;
  case 1:						/*     1:����CPU��ư */
    if( side==PIO_SIDE_M ){
      dual_cpu_count = CPU_1_COUNT;
      CPU_BREAKOFF();
    }
    break;
  }
  return;
}


/*--------------------------------------------------------------*/
/* ľ�� Port C �˽񤭹���					*/
/*--------------------------------------------------------------*/
void	pio_write_C_direct( int side, byte data )
{
		/* �ݡ���°���԰��� */
  if( pio_C[ side^1 ][ PIO_PORT_CH ].type == PIO_WRITE &&
      pio_C[ side^1 ][ PIO_PORT_CL ].type == PIO_WRITE ){
    pio_mesC( "PIO C WRITE PORT Mismatch" );
  }
  if( pio_C[ side   ][ PIO_PORT_CH ].type == PIO_READ  &&
      pio_C[ side   ][ PIO_PORT_CL ].type == PIO_READ  ){
    pio_mesC( "PIO C Write to READ-PORT" );
  }
		/* �饤�� */

  pio_C[ side ][ PIO_PORT_CH ].data = data >> 4;
  pio_C[ side ][ PIO_PORT_CL ].data = data & 0x0f;

  switch( cpu_timing ){
  case 0:						/*     0:���Τޤ޽�*/
  case 2:						/*     2:���Τޤ޽�*/
    break;
  case 1:						/*     1:����CPU��ư */
    if( side==PIO_SIDE_M ){
      dual_cpu_count = CPU_1_COUNT;
      CPU_BREAKOFF();
    }
    break;
  }
  return;
}







/*----------------------------------------------------------------------*/
/* PIO ����								*/
/*	PA / PB / PCH / PCL ������������ꡣ				*/
/*	�⡼�ɤ����� (�⡼�ɤ� 0 �˸��ꡣ�ܺ�����)			*/
/*----------------------------------------------------------------------*/
void	pio_set_mode( int side, byte data )
{
  if( data & 0x60 ){
    if( verbose_pio )
      printf("PIO mode A & CH not 0 : side = %s : mode = %d\n",
	     (side!=PIO_SIDE_M)?"M":"S", (data>>5)&0x3 );
  }
	/* PIO A */

  if( data & 0x10 ){
    pio_AB[ side ][ PIO_PORT_A ].type  = PIO_READ;
  }else{
    pio_AB[ side ][ PIO_PORT_A ].type   = PIO_WRITE;
  }
  pio_AB[ side ][ PIO_PORT_A ].data    = 0;
  pio_AB[ side ][ PIO_PORT_A ].exist   = PIO_EMPTY;

	/* PIO C-H */

  if( data & 0x08 ){
    pio_C[ side ][ PIO_PORT_CH ].type  = PIO_READ;
  }else{
    pio_C[ side ][ PIO_PORT_CH ].type  = PIO_WRITE;
  }
  pio_C[ side ][ PIO_PORT_CH ].data    = 0;
  pio_C[ side ][ PIO_PORT_CH ].cont_f  = 1;

  if( data & 0x04 ){
    if( verbose_pio )
      printf("PIO mode B & CL not 0 : side = %s : mode = %d\n",
	     (side!=PIO_SIDE_M)?"M":"S", (data>>2)&0x1 );
  }
	/* PIO B */

  if( data & 0x02 ){
    pio_AB[ side ][ PIO_PORT_B ].type  = PIO_READ;
  }else{
    pio_AB[ side ][ PIO_PORT_B ].type  = PIO_WRITE;
  }
  pio_AB[ side ][ PIO_PORT_B ].data    = 0;
  pio_AB[ side ][ PIO_PORT_B ].exist   = PIO_EMPTY;

	/* PIO C-L */

  if( data & 0x01 ){
    pio_C[ side ][ PIO_PORT_CL ].type  = PIO_READ;
  }else{
    pio_C[ side ][ PIO_PORT_CL ].type  = PIO_WRITE;
  }
  pio_C[ side ][ PIO_PORT_CL ].data    = 0;
  pio_C[ side ][ PIO_PORT_CL ].cont_f  = 1;

}






/***********************************************************************
 * ���ơ��ȥ��ɡ����ơ��ȥ�����
 ************************************************************************/

#define	SID	"PIO "

static	T_SUSPEND_W	suspend_pio_work[]=
{
  { TYPE_INT,	&pio_AB[0][0].type,	},
  { TYPE_INT,	&pio_AB[0][0].exist,	},
  { TYPE_INT,	&pio_AB[0][0].cont_f,	},
  { TYPE_BYTE,	&pio_AB[0][0].data,	},

  { TYPE_INT,	&pio_AB[0][1].type,	},
  { TYPE_INT,	&pio_AB[0][1].exist,	},
  { TYPE_INT,	&pio_AB[0][1].cont_f,	},
  { TYPE_BYTE,	&pio_AB[0][1].data,	},

  { TYPE_INT,	&pio_AB[1][0].type,	},
  { TYPE_INT,	&pio_AB[1][0].exist,	},
  { TYPE_INT,	&pio_AB[1][0].cont_f,	},
  { TYPE_BYTE,	&pio_AB[1][0].data,	},

  { TYPE_INT,	&pio_AB[1][1].type,	},
  { TYPE_INT,	&pio_AB[1][1].exist,	},
  { TYPE_INT,	&pio_AB[1][1].cont_f,	},
  { TYPE_BYTE,	&pio_AB[1][1].data,	},

  { TYPE_INT,	&pio_C[0][0].type,	},
  { TYPE_INT,	&pio_C[0][0].exist,	},
  { TYPE_INT,	&pio_C[0][0].cont_f,	},
  { TYPE_BYTE,	&pio_C[0][0].data,	},

  { TYPE_INT,	&pio_C[0][1].type,	},
  { TYPE_INT,	&pio_C[0][1].exist,	},
  { TYPE_INT,	&pio_C[0][1].cont_f,	},
  { TYPE_BYTE,	&pio_C[0][1].data,	},

  { TYPE_INT,	&pio_C[1][0].type,	},
  { TYPE_INT,	&pio_C[1][0].exist,	},
  { TYPE_INT,	&pio_C[1][0].cont_f,	},
  { TYPE_BYTE,	&pio_C[1][0].data,	},

  { TYPE_INT,	&pio_C[1][1].type,	},
  { TYPE_INT,	&pio_C[1][1].exist,	},
  { TYPE_INT,	&pio_C[1][1].cont_f,	},
  { TYPE_BYTE,	&pio_C[1][1].data,	},

  { TYPE_END,	0			},
};


int	statesave_pio( void )
{
  if( statesave_table( SID, suspend_pio_work ) == STATE_OK ) return TRUE;
  else                                                       return FALSE;
}

int	stateload_pio( void )
{
  if( stateload_table( SID, suspend_pio_work ) == STATE_OK ) return TRUE;
  else                                                       return FALSE;
}
