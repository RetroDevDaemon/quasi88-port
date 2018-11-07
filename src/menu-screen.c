/************************************************************************/
/*									*/
/* ��˥塼�⡼�ɤˤ�������̤�ɽ��					*/
/*									*/
/* QUASI88 �� ��˥塼�⡼�ɤǤβ���ɽ���ˤϡ��ʲ��δؿ����Ѥ��롣	*/
/* �ʤ��������δؿ��ϡ�������� Q8TK ����ƤӽФ���롣		*/
/*									*/
/*									*/
/* �ڴؿ���								*/
/*									*/
/* voie	menu_draw( void )						*/
/*	����˥塼���̤�ɽ������� T_Q8GR_SCREEN menu_screen[][] ��	*/
/*	  �����ơ�screen_buf �˥��᡼������������ѥ�åȤ����ꤷ�ơ�	*/
/*	  ɽ�� (menu_draw_screen()) ���롣				*/
/*									*/
/* void menu_draw_screen( void )					*/
/*	����˥塼���̤�ɽ�� (screen_buf ��ɽ��)���롣			*/
/*	 �ֲ���Ϫ�Фˤ�������פκݤˤ�ƤФ�롣			*/
/*									*/
/*======================================================================*/
/*									*/
/*	��˥塼���̤Υ�� menu_screen[25][80] �ι�¤�ΤǤ��롣	*/
/*	��˥塼���̤ϡ�80ʸ����25�Ԥǹ�������Ƥ��ꡢ����1ʸ����ˡ�	*/
/*	T_Q8GR_SCREEN ���Υ����1�ġ�������Ƥ��Ƥ��롣		*/
/*									*/
/*	�ºݤ�ɽ�������ʸ���ϡ�addr (����ROM���ɥ쥹) �Ǽ�����Ƥ��뤬	*/
/*	font_type �ˤ�äơ����� addr �ΰ�̣���ۤʤ롣���ʤ����	*/
/*									*/
/*	font_type   �ե����						*/
/*	---------   ---------------------------------------------------	*/
/*	FONT_ANK    font_mem[addr] ����� 8 �Х���			*/
/*	FONT_QUART  kanji_rom[0][addr][0] ����� 8 �Х��� (addr<0x0800)	*/
/*	FONT_HALF   kanji_rom[0][addr][0] ����� 16 �Х���(addr<0xc000)	*/
/*	FONT_KNJ1L  kanji_rom[0][addr][0] ���� 1�Х��Ȥ�����16 �Х���	*/
/*	FONT_KNJ1R  kanji_rom[0][addr][1] ���� 1�Х��Ȥ�����16 �Х���	*/
/*	FONT_KNJ2L  kanji_rom[1][addr][0] ���� 1�Х��Ȥ�����16 �Х���	*/
/*	FONT_KNJ2R  kanji_rom[1][addr][1] ���� 1�Х��Ȥ�����16 �Х���	*/
/*	FONT_KNJXL  kanji_dummy_rom[0][0] ���� 1�Х��Ȥ�����16 �Х���	*/
/*	FONT_KNJXR  kanji_dummy_rom[0][1] ���� 1�Х��Ȥ�����16 �Х���	*/
/*									*/
/*	����ɽ������ե���ȤΥӥåȥѥ�����Ȥʤ롣			*/
/*	�ޤ���QUASI88 �� ���̤� 640x400 �ɥå� �ʤΤǡ�80ʸ����25�Ԥ�	*/
/*	��硢1 ʸ�������� 8x16 �ɥåȤȤʤ롣				*/
/*	�ʤ���FONT_ANK �� FONT_QUART �ξ��ϡ��ӥåȥѥ�����ǡ�����	*/
/*	8x8 �ȡ�Ⱦʬ�����ʤ��Τǡ��������˳��礷��ɽ������ɬ�פ����롣	*/
/*									*/
/************************************************************************/

#include <string.h>

#include "quasi88.h"
#include "screen.h"
#include "menu-screen.h"

#include "q8tk.h"
#include "graph.h"


/*----------------------------------------------------------------------*/
/* ��˥塼���̤Υ��եȥ�������������λ���				*/
/*----------------------------------------------------------------------*/
#if 0		/* ����������� */
byte menu_cursor_on[]  = { 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff,
			   0xd8, 0xa8, 0x0c, 0x0c, 0x06, 0x06, 0x03, 0x03 };
#else		/* �֥�å��������� */
byte menu_cursor_on[]  = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
#endif

byte menu_cursor_off[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

int	menu_cursor_x;
int	menu_cursor_y;


/*----------------------------------------------------------------------*/
/* ��˥塼ɽ���ѥѥ�åȤ����ꤷ�������ƥ��ž��			*/
/*----------------------------------------------------------------------*/
void	screen_get_menu_palette(PC88_PALETTE_T pal[16])
{
    int i;

    static const struct {
	int index;
	int col;
    } menupal[ 16 ] = {
	{ Q8GR_PALETTE_FOREGROUND, MENU_COLOR_FOREGROUND,	},
	{ Q8GR_PALETTE_BACKGROUND, MENU_COLOR_BACKGROUND,	},
	{ Q8GR_PALETTE_LIGHT,      MENU_COLOR_LIGHT,		},
	{ Q8GR_PALETTE_SHADOW,     MENU_COLOR_SHADOW,		},
	{ Q8GR_PALETTE_FONT_FG,    MENU_COLOR_FONT_FG,		},
	{ Q8GR_PALETTE_FONT_BG,    MENU_COLOR_FONT_BG,		},
	{ Q8GR_PALETTE_LOGO_FG,    MENU_COLOR_LOGO_FG,		},
	{ Q8GR_PALETTE_LOGO_BG,    MENU_COLOR_LOGO_BG,		},
	{ Q8GR_PALETTE_BLACK,      0x000000,			},
	{ Q8GR_PALETTE_SCALE_SLD,  MENU_COLOR_SCALE_SLD,	},
	{ Q8GR_PALETTE_SCALE_BAR,  MENU_COLOR_SCALE_BAR,	},
	{ Q8GR_PALETTE_SCALE_ACT,  MENU_COLOR_SCALE_ACT,	},
	{ Q8GR_PALETTE_RED,        0xff0000,			},
	{ Q8GR_PALETTE_GREEN,      0x00ff00,			},
	{ Q8GR_PALETTE_BLUE,       0x0000ff,			},
	{ Q8GR_PALETTE_WHITE,      0xffffff,			},
    };

    for (i=0; i<COUNTOF(menupal); i++) {
	pal[ menupal[i].index ].red   = (menupal[i].col >> 16) & 0xff;
	pal[ menupal[i].index ].green = (menupal[i].col >>  8) & 0xff;
	pal[ menupal[i].index ].blue  = (menupal[i].col >>  0) & 0xff;
    }
}
