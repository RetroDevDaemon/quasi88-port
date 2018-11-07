/************************************************************************/
/*									*/
/* CRTC �� DMAC �ν���							*/
/*									*/
/************************************************************************/

#include "quasi88.h"
#include "crtcdmac.h"
#include "memory.h"

#include "screen.h"
#include "suspend.h"


/*======================================================================
 *
 *======================================================================*/

		/* CRTC¦���鸫�����ȥ�ӥ塼��	*/

#define MONO_SECRET	0x01
#define MONO_BLINK	0x02
#define MONO_REVERSE	0x04
#define MONO_UPPER	0x10
#define MONO_UNDER	0x20
#define MONO_GRAPH	0x80
#define COLOR_SWITCH	0x08
#define COLOR_GRAPH	0x10
#define COLOR_B		0x20
#define COLOR_R		0x40
#define COLOR_G		0x80

		/* ����ɽ���ǻ��Ѥ��륢�ȥ�ӥ塼�� */

#define ATTR_REVERSE	0x01			/* ȿž			*/
#define ATTR_SECRET	0x02			/* ɽ��/��ɽ��		*/
#define ATTR_UPPER	0x04			/* ���åѡ��饤��	*/
#define ATTR_LOWER	0x08			/* ��������饤��	*/
#define ATTR_GRAPH	0x10			/* ����ե��å��⡼��	*/
#define ATTR_B		0x20			/* �� Blue		*/
#define ATTR_R		0x40			/* �� Reg		*/
#define ATTR_G		0x80			/* �� Green		*/

#define MONO_MASK	0x0f
#define COLOR_MASK	0xe0

/*======================================================================*/

int		text_display = TEXT_ENABLE;	/* �ƥ�����ɽ���ե饰	*/

int		blink_cycle;		/* ���Ǥμ���	8/16/24/32	*/
int		blink_counter = 0;	/* �������楫����		*/

int		dma_wait_count = 0;	/* DMA�Ǿ��񤹤륵�������	*/


static	int	crtc_command;
static	int	crtc_param_num;

static	byte	crtc_status;
static	byte	crtc_light_pen[2];
static	byte	crtc_load_cursor_position;


	int	crtc_active;		/* CRTC�ξ��� 0:CRTC��ư 1:CRTC��� */
	int	crtc_intr_mask;		/* CRTC�γ���ޥ��� ==3 ��ɽ��	    */
	int	crtc_cursor[2];		/* ����������� ��ɽ���λ���(-1,-1) */
	byte	crtc_format[5];		/* CRTC �������Υե����ޥå�	    */


	int	crtc_reverse_display;	/* ����ȿžɽ�� / �����̾�ɽ��	*/

	int	crtc_skip_line;		/* ����1�����Ф�ɽ�� / �����̾� */
	int	crtc_cursor_style;	/* �֥�å� / ������饤��	*/
	int	crtc_cursor_blink;	/* �������Ǥ��� �������Ǥ��ʤ�	*/
	int	crtc_attr_non_separate;	/* ����VRAM��ATTR ����ߤ��¤�	*/
	int	crtc_attr_color;	/* ���ĥ��顼 �������		*/
	int	crtc_attr_non_special;	/* ���ĹԤν���� ATTR ���¤�	*/

	int	CRTC_SZ_LINES	   =20;	/* ɽ�������� (20/25)		*/
#define		CRTC_SZ_COLUMNS	   (80)	/* ɽ������Կ� (80����)	*/

	int	crtc_sz_lines      =20;	/* ��� (20��25)		*/
	int	crtc_sz_columns    =80;	/* �Կ� (2��80)			*/
	int	crtc_sz_attrs      =20;	/* °���� (1��20)		*/
	int	crtc_byte_per_line=120;	/* 1�Ԥ�����Υ��� �Х��ȿ�	*/
	int	crtc_font_height   =10;	/* �ե���Ȥι⤵ �ɥåȿ�(8/10)*/



/******************************************************************************

			���������������� crtc_byte_per_line  ����������������
			������   crtc_sz_columns  ������ ��  crtc_sz_attrs ��
			+-------------------------------+-------------------+
		      ��|				|��		    |
		      ��|	+--+ ��			|��		    |
		      ��|	|  | crtc_font_height	|��		    |
			|	+--+ ��			|		    |
	   CRTC_SZ_LINES|				|crtc_sz_lines	    |
			|				|		    |
		      ��|				|��		    |
		      ��|				|��		    |
		      ��|				|��		    |
			+-------------------------------+-------------------+
			������   CRTC_SZ_COLUMNS  ������ 

	crtc_sz_columns		���	2��80
	crtc_sz_attrs		°����	1��20
	crtc_byte_per_line	1�Ԥ�����Υ�����	columns + attrs*2
	crtc_sz_lines		�Կ�	20��25
	crtc_font_height	�ե���Ȥι⤵�ɥå���	8/10
	CRTC_SZ_COLUMNS		ɽ��������	80
	CRTC_SZ_LINES		ɽ������Կ�	20/25

******************************************************************************/









/* ���ͤޤǤˡġġ� 						*/
/*	SORCERIAN          �� 1�����Ф�����			*/
/*	Marchen Veil       �� ���ȥ�ӥ塼�Ȥʤ��⡼��		*/
/*	Xanadu II (E disk) ��             ��			*/
/*	Wizardry V         �� �Υ�ȥ�󥹥ڥ���������⡼��	*/


enum{
  CRTC_RESET		= 0,
  CRTC_STOP_DISPLAY	= 0,
  CRTC_START_DISPLAY,
  CRTC_SET_INTERRUPT_MASK,
  CRTC_READ_LIGHT_PEN,
  CRTC_LOAD_CURSOR_POSITION,
  CRTC_RESET_INTERRUPT,
  CRTC_RESET_COUNTERS,
  CRTC_READ_STATUS,
  EndofCRTC
};
#define CRTC_STATUS_VE	(0x10)		/* ����ɽ��ͭ��		*/
#define CRTC_STATUS_U	(0x08)		/* DMA����������	*/
#define CRTC_STATUS_N	(0x04)		/* �ü�����ʸ�����ȯ�� */
#define CRTC_STATUS_E	(0x02)		/* ɽ����λ���ȯ��	*/
#define CRTC_STATUS_LP	(0x01)		/* �饤�ȥڥ����� 	*/


/****************************************************************/
/* CRTC��Ʊ����������� (OUT 40H,A ... bit3)			*/
/*	�ä˥��ߥ�졼�Ȥ�ɬ�פʤ������������Ȼפ���		*/
/****************************************************************/
#ifdef	SUPPORT_CRTC_SEND_SYNC_SIGNAL
void	crtc_send_sync_signal( int flag )
{
}
#endif




/****************************************************************/
/*    CRTC ���ߥ�졼�����					*/
/****************************************************************/

/*-------- ����� --------*/

void	crtc_init( void )
{
  crtc_out_command( CRTC_RESET << 5 );
  crtc_out_parameter( 0xce );
  crtc_out_parameter( 0x98 );
  crtc_out_parameter( 0x6f );
  crtc_out_parameter( 0x58 );
  crtc_out_parameter( 0x53 );

  crtc_out_command( CRTC_LOAD_CURSOR_POSITION << 5 );
  crtc_out_parameter( 0 );
  crtc_out_parameter( 0 );
}

/*-------- ���ޥ�����ϻ� --------*/

void	crtc_out_command( byte data )
{
  crtc_command = data >> 5;
  crtc_param_num = 0;

  switch( crtc_command ){

  case CRTC_RESET:					/* �ꥻ�å� */
    crtc_status &= ~( CRTC_STATUS_VE | CRTC_STATUS_N | CRTC_STATUS_E );
    crtc_active = FALSE;
    set_text_display();
    screen_set_dirty_all();
    break;

  case CRTC_START_DISPLAY:				/* ɽ������ */
    crtc_reverse_display = data & 0x01;
    crtc_status |= CRTC_STATUS_VE;
    crtc_status &= ~( CRTC_STATUS_U );
    crtc_active = TRUE;
    set_text_display();
    screen_set_dirty_palette();
    break;

  case CRTC_SET_INTERRUPT_MASK:
    crtc_intr_mask = data & 0x03;
    set_text_display();
    screen_set_dirty_all();
    break;

  case CRTC_READ_LIGHT_PEN:
    crtc_status &= ~( CRTC_STATUS_LP );
    break;

  case CRTC_LOAD_CURSOR_POSITION:			/* ������������ */
    crtc_load_cursor_position = data & 0x01;
    crtc_cursor[ 0 ] = -1;
    crtc_cursor[ 1 ] = -1;
    break;

  case CRTC_RESET_INTERRUPT:
  case CRTC_RESET_COUNTERS:
    crtc_status &= ~( CRTC_STATUS_N | CRTC_STATUS_E );
    break;

  }
}

/*-------- �ѥ�᡼�����ϻ� --------*/

void	crtc_out_parameter( byte data )
{
  switch( crtc_command ){
  case CRTC_RESET:
    if( crtc_param_num < 5 ){
      crtc_format[ crtc_param_num++ ] = data;
    }

    crtc_skip_line         = crtc_format[2] & 0x80;		/* bool  */

    crtc_attr_non_separate = crtc_format[4] & 0x80;		/* bool */
    crtc_attr_color        = crtc_format[4] & 0x40;		/* bool */
    crtc_attr_non_special  = crtc_format[4] & 0x20;		/* bool */

    crtc_cursor_style      =(crtc_format[2] & 0x40) ?ATTR_REVERSE :ATTR_LOWER;
    crtc_cursor_blink      = crtc_format[2] & 0x20;		/* bool */
    blink_cycle            =(crtc_format[1]>>6) * 8 +8;		/* 8,16,24,48*/

    crtc_sz_lines          =(crtc_format[1] & 0x3f) +1;		/* 1��25 */
    if     ( crtc_sz_lines <= 20 ) crtc_sz_lines = 20;
    else if( crtc_sz_lines >= 25 ) crtc_sz_lines = 25;
    else                           crtc_sz_lines = 24;

    crtc_sz_columns        =(crtc_format[0] & 0x7f) +2;		/* 2��80 */
    if( crtc_sz_columns > 80 ) crtc_sz_columns = 80;

    crtc_sz_attrs          =(crtc_format[4] & 0x1f) +1;		/* 1��20 */
    if     ( crtc_attr_non_special ) crtc_sz_attrs = 0;
    else if( crtc_sz_attrs > 20 )    crtc_sz_attrs = 20;

    crtc_byte_per_line  = crtc_sz_columns + crtc_sz_attrs * 2;	/*column+attr*/

    crtc_font_height    = (crtc_sz_lines>20) ?  8 : 10;
    CRTC_SZ_LINES	= (crtc_sz_lines>20) ? 25 : 20;

    frameskip_blink_reset();
    break;

  case CRTC_LOAD_CURSOR_POSITION:
    if( crtc_param_num < 2 ){
      if( crtc_load_cursor_position ){
	crtc_cursor[ crtc_param_num++ ] = data;
      }else{
	crtc_cursor[ crtc_param_num++ ] = -1;
      }
    }
    break;

  }
}

/*-------- ���ơ��������ϻ� --------*/

byte	crtc_in_status( void )
{
  return crtc_status;
}

/*-------- �ѥ�᡼�����ϻ� --------*/

byte	crtc_in_parameter( void )
{
  byte data = 0xff;

  switch( crtc_command ){
  case CRTC_READ_LIGHT_PEN:
    if( crtc_param_num < 2 ){
      data = crtc_light_pen[ crtc_param_num++ ];
    }
    return data;
  }

  return 0xff;
}





/****************************************************************/
/*    DMAC ���ߥ�졼�����					*/
/****************************************************************/

static	int	dmac_flipflop;
	pair	dmac_address[4];
	pair	dmac_counter[4];
	int	dmac_mode;


void	dmac_init( void )
{
  dmac_flipflop = 0;
  dmac_address[0].W = 0;
  dmac_address[1].W = 0;
  dmac_address[2].W = 0xf3c8;
  dmac_address[3].W = 0;
  dmac_counter[0].W = 0;
  dmac_counter[1].W = 0;
  dmac_counter[2].W = 0;
  dmac_counter[3].W = 0;
}


void	dmac_out_mode( byte data )
{
  dmac_flipflop = 0;
  dmac_mode = data;

  set_text_display();
  screen_set_dirty_all();
}
byte	dmac_in_status( void )
{
  return 0x1f;
}


void	dmac_out_address( byte addr, byte data )
{
  if( dmac_flipflop==0 ) dmac_address[ addr ].B.l=data;
  else                   dmac_address[ addr ].B.h=data;

  dmac_flipflop ^= 0x1;
  screen_set_dirty_all();	/* �����ϡ�addr==2�λ��Τߡġġ� */
}
void	dmac_out_counter( byte addr, byte data )
{
  if( dmac_flipflop==0 ) dmac_counter[ addr ].B.l=data;
  else                   dmac_counter[ addr ].B.h=data;

  dmac_flipflop ^= 0x1;
}


byte	dmac_in_address( byte addr )
{
  byte data;

  if( dmac_flipflop==0 ) data = dmac_address[ addr ].B.l;
  else                   data = dmac_address[ addr ].B.h;

  dmac_flipflop ^= 0x1;
  return data;
}
byte	dmac_in_counter( byte addr )
{
  byte data;

  if( dmac_flipflop==0 ) data = dmac_counter[ addr ].B.l;
  else                   data = dmac_counter[ addr ].B.h;

  dmac_flipflop ^= 0x1;
  return data;
}


/***********************************************************************
 * CRTC,DMAC���������ӡ�I/O 31H / 53H ���ϻ��˸Ƥ�
 ************************************************************************/
void	set_text_display(void)
{
    if( (dmac_mode & 0x4) && (crtc_active) && crtc_intr_mask==3){
	if( !(grph_pile & GRPH_PILE_TEXT) ){
	    text_display = TEXT_ENABLE;
	}else{
	    if( grph_ctrl & GRPH_CTRL_COLOR )
		text_display = TEXT_DISABLE;
	    else
		text_display = TEXT_ATTR_ONLY;
	}
    }else{
	text_display = TEXT_DISABLE;
    }
}


/***********************************************************************
 * ����ɽ���Τ���δؿ�
 ************************************************************************/


/*======================================================================
 * �ƥ�����VRAM�Υ��ȥ�ӥ塼�Ȥ����ѥ�������ꤹ��
 *
 *	�Хåե���2�Ĥ��ꡢ��ߤ����ؤ��ƻ��Ѥ��롣
 *	���̽񤭴����κݤϡ�����2�ĤΥХåե�����Ӥ����Ѳ���
 *	���ä���ʬ�����򹹿����롣
 *
 *	����ϡ�16bit�ǡ����8bit��ʸ�������ɡ����̤�°����
 *		��������ե��å��⡼�ɡ���������饤��
 *		���åѡ��饤�󡢥�������åȡ���С���
 *		+---------------------+--+--+--+--+--+--+--+--+
 *		|    ASCII 8bit       |��|��|��|GR|LO|UP|SC|RV|
 *		+---------------------+--+--+--+--+--+--+--+--+
 *	BLINK°���ϡ���������̵�롢�������ϥ�������åȡ�
 *
 *	����ˡ���������å�°���ξ��� ʸ�������ɤ� 0 ���ִ����롣
 *	(ʸ��������==0��̵���Ƕ���Ȥ��Ƥ���Τ�)
 *		+---------------------+--+--+--+--+--+--+--+--+
 *	     ��	|    ASCII == 0       |��|��|��|��|LO|UP|��|RV|
 *		+---------------------+--+--+--+--+--+--+--+--+
 *	        ����ե��å��⡼�ɤȥ�������å�°����ä��Ƥ�OK������
 *		������������åѡ��饤�󡢥�С�����ͭ���ʤΤǻĤ���
 *
 *======================================================================*/

int	text_attr_flipflop = 0;
Ushort	text_attr_buf[2][2048];		/* ���ȥ�ӥ塼�Ⱦ���	*/
			/* �� 80ʸ��x25��=2000��­���Τ�����	*/
			/* ;ʬ�˻Ȥ��Τǡ�¿��˳��ݤ��롣	*/
				   

void	crtc_make_text_attr( void )
{
  int		global_attr  = (ATTR_G|ATTR_R|ATTR_B);
  int		global_blink = FALSE;
  int		i, j, tmp;
  int		column, attr, attr_rest;
  word		char_start_addr, attr_start_addr;
  word		c_addr, a_addr;
  Ushort	*text_attr = &text_attr_buf[ text_attr_flipflop ][0];


	/* CRTC �� DMAC ��ߤޤäƤ����� */
	/*  (ʸ���⥢�ȥ�ӥ塼�Ȥ�̵��)   */

  if( text_display==TEXT_DISABLE ){		/* ASCII=0���򿧡������ʤ� */
    for( i=0; i<CRTC_SZ_LINES; i++ ){		/* �ǽ�������롣	   */
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
	*text_attr ++ =  (ATTR_G|ATTR_R|ATTR_B);
      }
    }
    return;			/* ������ȿž�䥫�������ʤ������������  */
  }



	/* �Υ󡦥ȥ�󥹥ڥ����ȷ��ξ�� */
	/* (1ʸ���֤��ˡ�VRAM��ATTR ������) */

			/* �ġġ� ���ܺ����� 				*/
			/*	CRTC������ѥ����󤫤餷�ơ�����˹Ԥ�	*/
			/*	�Ǹ��°����������⤢�ꤨ����������?	*/

  if( crtc_attr_non_separate ){

    char_start_addr = text_dma_addr.W;
    attr_start_addr = text_dma_addr.W + 1;

    for( i=0; i<crtc_sz_lines; i++ ){

      c_addr	= char_start_addr;
      a_addr	= attr_start_addr;

      char_start_addr += crtc_byte_per_line;
      attr_start_addr += crtc_byte_per_line;

      for( j=0; j<CRTC_SZ_COLUMNS; j+=2 ){		/* °�������������ɤ�*/
	attr = main_ram[ a_addr ];			/* �Ѵ�����°�����*/
	a_addr += 2;					/* ���������롣    */
	global_attr =( global_attr & COLOR_MASK ) |
		     ((attr &  MONO_GRAPH) >> 3 ) |
		     ((attr & (MONO_UNDER|MONO_UPPER|MONO_REVERSE))>>2) |
		     ((attr &  MONO_SECRET) << 1 );

					/* BLINK��OFF����SECRET����    */
	if( (attr & MONO_BLINK) && ((blink_counter&0x03)==0) ){
	  global_attr |= ATTR_SECRET;
	}

	*text_attr ++ = ((Ushort)main_ram[ c_addr ++ ] << 8 ) | global_attr;
	*text_attr ++ = ((Ushort)main_ram[ c_addr ++ ] << 8 ) | global_attr;

      }

      if( crtc_skip_line ){
	if( ++i < crtc_sz_lines ){
	  for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
	    *text_attr ++ =  global_attr | ATTR_SECRET;
	  }
	}
      }

    }
    for( ; i<CRTC_SZ_LINES; i++ ){		/* �Ĥ�ιԤϡ�SECRET */
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){	/*  (24�������к�)    */
	*text_attr ++ =  global_attr | ATTR_SECRET;
      }
    }

  }else{

	/* �ȥ�󥹥ڥ����ȷ��ξ�� */
	/* (�ԤκǸ�ˡ�ATTR������)   */

    char_start_addr = text_dma_addr.W;
    attr_start_addr = text_dma_addr.W + crtc_sz_columns;

    for( i=0; i<crtc_sz_lines; i++ ){			/* ��ñ�̤�°������ */

      c_addr	= char_start_addr;
      a_addr	= attr_start_addr;

      char_start_addr += crtc_byte_per_line;
      attr_start_addr += crtc_byte_per_line;


      attr_rest = 0;						/*°������� */
      for( j=0; j<=CRTC_SZ_COLUMNS; j++ ) text_attr[j] = 0;	/* [0]��[80] */


      for( j=0; j<crtc_sz_attrs; j++ ){			/* °����������ܤ� */
	column = main_ram[ a_addr++ ];			/* ����˳�Ǽ       */
	attr   = main_ram[ a_addr++ ];

	if( j!=0 && column==0    ) column = 0x80;		/* �ü����?*/
	if( j==0 && column==0x80 ){column = 0;
/*				   global_attr = (ATTR_G|ATTR_R|ATTR_B);
				   global_blink= FALSE;  }*/}

	if( column==0x80  &&  !attr_rest ){			/* 8bit�ܤ� */
	  attr_rest = attr | 0x100;				/* ���ѺѤ� */
	}							/* �ե饰   */
	else if( column <= CRTC_SZ_COLUMNS  &&  !text_attr[ column ] ){
	  text_attr[ column ] = attr | 0x100;
	}
      }


      if( !text_attr[0] && attr_rest ){			/* �����-1�ޤ�°����*/
	for( j=CRTC_SZ_COLUMNS; j; j-- ){		/* ͭ�����Ȥ�������*/
	  if( text_attr[j] ){				/* ������(�����ʹ� */
	    tmp          = text_attr[j];		/* °����ͭ�����Ȥ���*/
	    text_attr[j] = attr_rest;			/* �դ����¤��ؤ���) */
	    attr_rest    = tmp;
	  }
	}
	text_attr[0] = attr_rest;
      }


      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){		/* °�������������ɤ�*/
							/* �Ѵ�����°�����*/
	if( ( attr = *text_attr ) ){			/* ���������롣    */
	  if( crtc_attr_color ){
	    if( attr & COLOR_SWITCH ){
	      global_attr =( global_attr & MONO_MASK ) |
			   ( attr & (COLOR_G|COLOR_R|COLOR_B|COLOR_GRAPH));
	    }else{
	      global_attr =( global_attr & (COLOR_MASK|ATTR_GRAPH) ) |
			   ((attr & (MONO_UNDER|MONO_UPPER|MONO_REVERSE))>>2) |
			   ((attr &  MONO_SECRET) << 1 );
	      global_blink= (attr & MONO_BLINK);
	    }
	  }else{
	    global_attr =( global_attr & COLOR_MASK ) |
			 ((attr &  MONO_GRAPH) >> 3 ) |
			 ((attr & (MONO_UNDER|MONO_UPPER|MONO_REVERSE))>>2) |
			 ((attr &  MONO_SECRET) << 1 );
	    global_blink= (attr & MONO_BLINK);
	  }
					/* BLINK��OFF����SECRET����    */
	  if( global_blink && ((blink_counter&0x03)==0) ){
	    global_attr =  global_attr | ATTR_SECRET;
	  }
	}

	*text_attr ++ = ((Ushort)main_ram[ c_addr ++ ] << 8 ) | global_attr;

      }

      if( crtc_skip_line ){				/* 1�����Ф��������*/
	if( ++i < crtc_sz_lines ){			/* ���ιԤ�SECRET�� */
	  for( j=0; j<CRTC_SZ_COLUMNS; j++ ){		/* ���롣         */
	    *text_attr ++ =  global_attr | ATTR_SECRET;
	  }
	}
      }

    }

    for( ; i<CRTC_SZ_LINES; i++ ){		/* �Ĥ�ιԤϡ�SECRET */
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){	/*  (24�������к�)    */
	*text_attr ++ =  global_attr | ATTR_SECRET;
      }
    }

  }



	/* CRTC �� DMAC ��ư���Ƥ��뤱�ɡ� �ƥ����Ȥ���ɽ�� */
	/* ��VRAM����ξ�� (���ȥ�ӥ塼�Ȥο�������ͭ��)  */

  if( text_display==TEXT_ATTR_ONLY ){

    text_attr = &text_attr_buf[ text_attr_flipflop ][0];

    for( i=0; i<CRTC_SZ_LINES; i++ ){
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
	*text_attr ++ &=  (ATTR_G|ATTR_R|ATTR_B);
      }
    }
    return;			/* ������ȿž�䥫����������ס������Ǥ����  */
  }




		/* ����ȿž���� */

  if( crtc_reverse_display && (grph_ctrl & GRPH_CTRL_COLOR)){
    text_attr = &text_attr_buf[ text_attr_flipflop ][0];
    for( i=0; i<CRTC_SZ_LINES; i++ ){
      for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
	*text_attr ++ ^= ATTR_REVERSE;
      }
    }
  }

		/* ��������ɽ������ */

  if( 0 <= crtc_cursor[0] && crtc_cursor[0] < crtc_sz_columns &&
      0 <= crtc_cursor[1] && crtc_cursor[1] < crtc_sz_lines   ){
    if( !crtc_cursor_blink || (blink_counter&0x01) ){
      text_attr_buf[ text_attr_flipflop ][ crtc_cursor[1]*80 + crtc_cursor[0] ]
							^= crtc_cursor_style;
    }
  }


	/* ��������å�°������ (ʸ�������� 0x00 ���ִ�) */

  text_attr = &text_attr_buf[ text_attr_flipflop ][0];
  for( i=0; i<CRTC_SZ_LINES; i++ ){
    for( j=0; j<CRTC_SZ_COLUMNS; j++ ){
      if( *text_attr & ATTR_SECRET ){		/* SECRET °���ϡ�������00�� */
	*text_attr &= (COLOR_MASK|ATTR_UPPER|ATTR_LOWER|ATTR_REVERSE);
      }
      text_attr ++;
    }
  }

}






/***********************************************************************
 * ���ꤵ�줿ʸ��������(°����ʸ��)��ꡢ�ե���Ȼ����ǡ�������������
 *
 *	int attr	�� ʸ�������ɡ� text_attr_buf[]���ͤǤ��롣
 *	T_GRYPH *gryph	�� gryph->b[0]��[7] �� �ե���ȤΥӥåȥޥåפ�
 *			   ��Ǽ����롣(20�Ի��ϡ�b[0]��[9]�˳�Ǽ)
 *	int *color	�� �ե���Ȥο�����Ǽ����롣�ͤϡ� 8��15
 *
 *	�����ǡ����ϡ�char 8��10�ĤʤΤ�������©�ʹ�®���Τ���� long ��
 *	�����������Ƥ��롣���Τ��ᡢ T_GRYPH �Ȥ���̯�ʷ���ȤäƤ��롣
 *	(����ס���������͡�)
 *
 *	��)	���ȥ�ӥ塼�Ⱦ���ɬ�פʤΤǡ�
 *		ͽ�� make_text_attr_table( ) ��Ƥ�Ǥ�������
 ************************************************************************/

void	get_font_gryph( int attr, T_GRYPH *gryph, int *color )
{
  int	chara;
  bit32	*src;
  bit32	*dst = (bit32 *)gryph;

  *color = ((attr & COLOR_MASK) >> 5) | 8;


  if( ( attr & ~(COLOR_MASK|ATTR_REVERSE) )==0 ){

    if( ( attr & ATTR_REVERSE ) == 0 ){		/* ����ե���Ȼ� */

      *dst++ = 0;
      *dst++ = 0;
      *dst   = 0;

    }else{					/* �٥��ե���Ȼ� */

      *dst++ = 0xffffffff;
      *dst++ = 0xffffffff;
      *dst   = 0xffffffff;
    }

  }else{					/* �̾�ե���Ȼ� */

    chara = attr >> 8;

    if( attr & ATTR_GRAPH )
      src = (bit32 *)&font_rom[ (chara | 0x100)*8 ];
    else
      src = (bit32 *)&font_rom[ (chara        )*8 ];

					/* �ե���Ȥ�ޤ���������˥��ԡ� */
    *dst++ = *src++;
    *dst++ = *src;
    *dst   = 0;

					/* °���ˤ����������ե���Ȥ�ù�*/
    if( attr & ATTR_UPPER ) gryph->b[ 0 ] |= 0xff;
    if( attr & ATTR_LOWER ) gryph->b[ crtc_font_height-1 ] |= 0xff;
    if( attr & ATTR_REVERSE ){
      dst -= 2;
      *dst++ ^= 0xffffffff;
      *dst++ ^= 0xffffffff;
      *dst   ^= 0xffffffff;
    }
  }
}





/***********************************************************************
 * ���ơ��ȥ��ɡ����ơ��ȥ�����
 ************************************************************************/

#define	SID	"CRTC"

static	T_SUSPEND_W	suspend_crtcdmac_work[]=
{
  { TYPE_INT,	&text_display,		},
  { TYPE_INT,	&blink_cycle,		},
  { TYPE_INT,	&blink_counter,		},

  { TYPE_INT,	&dma_wait_count,	},

  { TYPE_INT,	&crtc_command,		},
  { TYPE_INT,	&crtc_param_num,	},
  { TYPE_BYTE,	&crtc_status,		},
  { TYPE_BYTE,	&crtc_light_pen[0],	},
  { TYPE_BYTE,	&crtc_light_pen[1],	},
  { TYPE_BYTE,	&crtc_load_cursor_position,	},
  { TYPE_INT,	&crtc_active,		},
  { TYPE_INT,	&crtc_intr_mask,	},
  { TYPE_INT,	&crtc_cursor[0],	},
  { TYPE_INT,	&crtc_cursor[1],	},
  { TYPE_BYTE,	&crtc_format[0],	},
  { TYPE_BYTE,	&crtc_format[1],	},
  { TYPE_BYTE,	&crtc_format[2],	},
  { TYPE_BYTE,	&crtc_format[3],	},
  { TYPE_BYTE,	&crtc_format[4],	},
  { TYPE_INT,	&crtc_reverse_display,	},
  { TYPE_INT,	&crtc_skip_line,	},
  { TYPE_INT,	&crtc_cursor_style,	},
  { TYPE_INT,	&crtc_cursor_blink,	},
  { TYPE_INT,	&crtc_attr_non_separate,},
  { TYPE_INT,	&crtc_attr_color,	},
  { TYPE_INT,	&crtc_attr_non_special,	},
  { TYPE_INT,	&CRTC_SZ_LINES,		},
  { TYPE_INT,	&crtc_sz_lines,		},
  { TYPE_INT,	&crtc_sz_columns,	},
  { TYPE_INT,	&crtc_sz_attrs,		},
  { TYPE_INT,	&crtc_byte_per_line,	},
  { TYPE_INT,	&crtc_font_height,	},

  { TYPE_INT,	&dmac_flipflop,		},
  { TYPE_PAIR,	&dmac_address[0],	},
  { TYPE_PAIR,	&dmac_address[1],	},
  { TYPE_PAIR,	&dmac_address[2],	},
  { TYPE_PAIR,	&dmac_address[3],	},
  { TYPE_PAIR,	&dmac_counter[0],	},
  { TYPE_PAIR,	&dmac_counter[1],	},
  { TYPE_PAIR,	&dmac_counter[2],	},
  { TYPE_PAIR,	&dmac_counter[3],	},
  { TYPE_INT,	&dmac_mode,		},

  { TYPE_END,	0			},
};


int	statesave_crtcdmac( void )
{
  if( statesave_table( SID, suspend_crtcdmac_work ) == STATE_OK ) return TRUE;
  else                                                            return FALSE;
}

int	stateload_crtcdmac( void )
{
  if( stateload_table( SID, suspend_crtcdmac_work ) == STATE_OK ) return TRUE;
  else                                                            return FALSE;
}
