/***********************************************************************
 * ���٥�Ƚ��� (�����ƥ��¸)
 *
 *	�ܺ٤ϡ� event.h ����
 ************************************************************************/

#include <SDL.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "quasi88.h"
#include "getconf.h"
#include "keyboard.h"

#include "drive.h"

#include "emu.h"
#include "device.h"
#include "screen.h"
#include "event.h"

#include "intr.h"			/* test */
#include "screen.h"			/* test */

	int	show_fps;		/* test */
static	int	display_fps_init(void);	/* test */
static	void	display_fps(void);	/* test */

int	use_cmdkey = 1;			/* Command�����ǥ�˥塼������     */

int	keyboard_type = 1;		/* �����ܡ��ɤμ���                */
char	*file_keyboard = NULL;		/* ��������ե�����̾		   */

int	use_joydevice = TRUE;		/* ���祤���ƥ��å��ǥХ����򳫤�? */

	int	use_unicode = FALSE;	/* UNICODE��Ȥ���106�����ܡ��ɤǤ� */
static	int	now_unicode = FALSE;	/* ���Τ˥�����������褦���������� */
					/* ��꡼���θ��Τ����ݤʤΤ���α   */

#define	JOY_MAX   	KEY88_PAD_MAX		/* ���祤���ƥ��å����(2��) */

#define	BUTTON_MAX	KEY88_PAD_BUTTON_MAX	/* �ܥ�����(8��)	     */

#define	AXIS_U		0x01
#define	AXIS_D		0x02
#define	AXIS_L		0x04
#define	AXIS_R		0x08

typedef struct {

    SDL_Joystick *dev;		/* �����ץ󤷤����祤���ƥ��å��ι�¤�� */
    int		  num;		/* QUASI88 �ǤΥ��祤���ƥ��å��ֹ� 0�� */

    int		  axis;		/* �����ܥ��󲡲�����			*/
    int		  nr_button;	/* ͭ���ʥܥ���ο�			*/

} T_JOY_INFO;

static T_JOY_INFO joy_info[ JOY_MAX ];

static	int	joystick_num;		/* �����ץ󤷤����祤���ƥ��å��ο� */


static	const char *debug_sdlkeysym(int code); /* �ǥХå��� */
/*==========================================================================
 * ��������ˤĤ���
 *
 *  ���̥���(ʸ������) �ϡ�
 *	106 �����ܡ��ɤξ�硢PC-8801 ��Ʊ���ʤΤ�����ʤ���
 *	101 �����ܡ��ɤξ�硢�������֤��ۤʤ뤷��������­��ʤ���
 *	�Ȥꤢ�������ʲ��Τ褦�����֤��Ƥߤ롣
 *		` �� \�� = �� ^�� [ ] �Ϥ��Τޤޡ� \ �� @��' �� :����CTRL �� _
 *
 *  �ü쥭��(��ǽ����) �ϡ�
 *	�ۥ���¦�Υ�������Ȼ���ʷ�ϵ��ε�ǽ��PC-8801�Υ����˳�����Ƥ褦��
 *	Pause �� STOP�� PrintScreen �� COPY �ʤɡ��Ŀ�Ū�ʼ�ѤǷ��롣
 *
 *  �ƥ󥭡��ϡ�
 *	PC-8801��106�����ǥ���������㴳�ۤʤ뤬�����ΤޤޤΥ��������Ȥ���
 *	�Ȥʤ�ȡ� = �� , ��̵���� mac �ʤ餢�뤬��
 *
 *  �ǲ��ʤΥ��������֤ϡ�Ŭ���˳�꿶�롣 (���å��Υ����ˤϳ�꿶��ʤ�)
 *
 *	PC-8801        ���� GRPH ����  ���ڡ��� �Ѵ�  PC    ����
 *	101����   Ctrl      Alt        ���ڡ���             Alt          Ctrl
 *	104����   Ctrl Win  Alt        ���ڡ���             Alt  Win App Ctrl
 *	109����   Ctrl Win  Alt ̵�Ѵ� ���ڡ��� �Ѵ� (�Ҥ�) Alt  Win App Ctrl
 *	mac ?     Ctrl      Opt (Cmd)  ���ڡ���      (cmd) (Opt)        (Ctrl)
 *
 * SDL�Υ������ϤˤĤ��Ƥο�¬ (Windows & 106�����ξ��)
 *	���Ķ��ѿ� SDL_VIDEODRIVER �� windib �� directx �Ȥǵ�ư���ۤʤ롣
 *	����SHIFT�פ򲡤��ʤ��� ��1�� �򲡤��Ƥ⡢keysym �� ��1�� �Τޤޡ�
 *	  �Ĥޤꡢ ��!�� �����Ϥ���ʤ��ߤ�����
 *	  ��ʸ�� ��A�� �����Ϥ���ʤ��� keysym �� ��a�� �Ȥʤ롣
 *	����������� 101 ���١����ˤʤäƤ��롣
 *	  ��^�� �򲡤��Ƥ� keysym �� ��=�� �ˤʤ롣
 *	�������Ĥ��Υ����ǡ� keycode ����ʣ���Ƥ���
 *	  windib  ���ȡ��������륭���ȥƥ󥭡��ʤɡ���������
 *	  directx �ʤ顢��ʣ��̵�� ?
 *	�������Ĥ��Υ����ǡ� keysym ����ʣ���Ƥ���
 *	  windib  ���ȡ��� �� ]  (�Ȥ�� �� �ˤʤ�)
 *	  directx �ʤ顢��ʣ��̵�� ?
 *	�������Ĥ��Υ����ǡ���������ܥ뤬̤���
 *	  ̵�Ѵ����Ѵ����������ʤҤ餬�� ��̤���
 *	  windib  ���ȡ��� ��̤���
 *	  directx ���ȡ����������Ⱦ��/���� ��̤���
 *	�������Ĥ��Υ����ǡ�������Υ�������θ��Τ��԰���(?)
 *	  windib  ���� Ⱦ��/���ѡ��������ʤҤ餬�ʡ�PrintScreen
 *	  directx ���� ALT
 *	��������å����񤢤�(?)
 *	  NumLock�ϥ�å��Ǥ��롣
 *	  windib  ���� SHIFT + CapsLock ����å��ġ�
 *	  directx ���� CapsLock���������ʤҤ餬�ʡ�Ⱦ��/���Ѥ���å��ġ�
 *	��NumLock��Υƥ󥭡����Ϥ��񤢤�(?)
 *	  windib  ���� NumLock��� SHIFT + �ƥ󥭡��ǡ�SHIFT�����Ū�˥���
 *	  NumLock���Ƥʤ��������ʤ���
 *	  windib  ���� ��������Ϥʤ���
 *
 *	����˥塼�⡼�ɤǤϡ�UNICODE ��ͭ���ˤ��롣
 *	  ��������С���SHIFT��+��1�� �� ��!�� ��ǧ���Ǥ��뤷����SHIFT��+��2��
 *	  �� ��"�פˤʤ롣��������  directx ���ȡ����ϤǤ��ʤ�ʸ�������뤾��
 *
 *	���Ȥ���ǡ����ܸ�Windows�Ǥ�101�����ܡ��ɤȡ��Ѹ�Windows�Ǥ�
 *	  101�����ܡ��ɤäơ�Ʊ����ư�ʤ������������
 *	  directx �λ��Υ��������ɳ�����Ƥ����餫���Լ����ʤΤ�����
 *===========================================================================*/

/* ���եȥ�����NumLock �򥪥󤷤��ݤΡ������Х���ǥ����ѹ��ơ��֥� */

typedef struct {
    int		type;		/* KEYCODE_INVALID / SYM / SCAN		*/
    int		code;		/* ��������ܥ롢�ʤ�����������󥳡���	*/
    int		new_key88;	/* NumLock ON ���� QUASI88����������	*/
    int		org_key88;	/* NumLock OFF���� QUASI88����������	*/
} T_BINDING;


/* �����Х���ǥ��󥰤�ǥե����(�����)�����ѹ�����ݤΡ��ơ��֥� */

typedef struct {
    int		type;		/* KEYCODE_INVALID / SYM / SCAN		*/
    int		code;		/* ��������ܥ롢�ʤ�����������󥳡���	*/
    int		key88;		/* �ѹ����� QUASI88����������           */
} T_REMAPPING;



/*----------------------------------------------------------------------
 * SDL �� keysym �� QUASI88 �� ���������ɤ��Ѵ�����ơ��֥�
 *
 *	��������ܥ� SDLK_xxx �������줿�顢 
 *	keysym2key88[ SDLK_xxx ] �������줿�Ȥ��롣
 *
 *	keysym2key88[] �ˤϡ� KEY88_xxx �򥻥åȤ��Ƥ�����
 *	����ͤ� keysym2key88_default[] ��Ʊ��
 *----------------------------------------------------------------------*/
static int keysym2key88[ SDLK_LAST ];



/*----------------------------------------------------------------------
 * SDL �� scancode �� QUASI88 �� ���������ɤ��Ѵ�����ơ��֥�
 *
 *	������󥳡��� code �������줿�顢
 *	keycode2key88[ code ] �������줿�Ȥ��롣
 *
 *	keycode2key88[] �ˤϡ� KEY88_xxx �ޤ��� -1 �򥻥åȤ��Ƥ�����
 *	����� keysym2key88[] ��ͥ�褵��롣(������ -1 �ξ���̵��)
 *	����ͤ� ���� -1���Ѵ���ǽ�ʥ�����󥳡��ɤ� 0��255�ޤǤ����¡�
 *----------------------------------------------------------------------*/
static int scancode2key88[ 256 ];

 

/*----------------------------------------------------------------------
 * ���եȥ����� NumLock ������� �����������Ѵ�����
 *
 *	binding[].code (SDL �� keysym �ʤ��� keycode) �������줿�顢
 *	binding[].new_key88 (KEY88_xxx) �������줿���Ȥˤ��롣
 *
 *	���եȥ����� NumLock ������ϡ����ξ���ˤ������äơ�
 *	keysym2key88[] �� keycode2key88[] ��񤭴����롣
 *	�ѹ��Ǥ��륭���θĿ��ϡ�64�Ĥޤ� (�����������Ф�������)
 *----------------------------------------------------------------------*/
static T_BINDING binding[ 64 ];





/*----------------------------------------------------------------------
 * SDLK_xxx �� KEY88_xxx �Ѵ��ơ��֥� (�ǥե����)
 *----------------------------------------------------------------------*/

static const int keysym2key88_default[ SDLK_LAST ] =
{
  0,				/*	SDLK_UNKNOWN		= 0,	*/
  0, 0, 0, 0, 0, 0, 0,
  KEY88_INS_DEL,		/*	SDLK_BACKSPACE		= 8,	*/
  KEY88_TAB,			/*	SDLK_TAB		= 9,	*/
  0, 0,
  KEY88_HOME,			/*	SDLK_CLEAR		= 12,	*/
  KEY88_RETURNL,		/*	SDLK_RETURN		= 13,	*/
  0, 0, 0, 0, 0,
  KEY88_STOP,			/*	SDLK_PAUSE		= 19,	*/
  0, 0, 0, 0, 0, 0, 0,
  KEY88_ESC,			/*	SDLK_ESCAPE		= 27,	*/
  0, 0, 0, 0,

  KEY88_SPACE,			/*	SDLK_SPACE		= 32,	*/
  KEY88_EXCLAM,			/*	SDLK_EXCLAIM		= 33,	*/
  KEY88_QUOTEDBL,		/*	SDLK_QUOTEDBL		= 34,	*/
  KEY88_NUMBERSIGN,		/*	SDLK_HASH		= 35,	*/
  KEY88_DOLLAR,			/*	SDLK_DOLLAR		= 36,	*/
  KEY88_PERCENT,		/*					*/
  KEY88_AMPERSAND,		/*	SDLK_AMPERSAND		= 38,	*/
  KEY88_APOSTROPHE,		/*	SDLK_QUOTE		= 39,	*/
  KEY88_PARENLEFT,		/*	SDLK_LEFTPAREN		= 40,	*/
  KEY88_PARENRIGHT,		/*	SDLK_RIGHTPAREN		= 41,	*/
  KEY88_ASTERISK,		/*	SDLK_ASTERISK		= 42,	*/
  KEY88_PLUS,			/*	SDLK_PLUS		= 43,	*/
  KEY88_COMMA,			/*	SDLK_COMMA		= 44,	*/
  KEY88_MINUS,			/*	SDLK_MINUS		= 45,	*/
  KEY88_PERIOD,			/*	SDLK_PERIOD		= 46,	*/
  KEY88_SLASH,			/*	SDLK_SLASH		= 47,	*/
  KEY88_0,			/*	SDLK_0			= 48,	*/
  KEY88_1,			/*	SDLK_1			= 49,	*/
  KEY88_2,			/*	SDLK_2			= 50,	*/
  KEY88_3,			/*	SDLK_3			= 51,	*/
  KEY88_4,			/*	SDLK_4			= 52,	*/
  KEY88_5,			/*	SDLK_5			= 53,	*/
  KEY88_6,			/*	SDLK_6			= 54,	*/
  KEY88_7,			/*	SDLK_7			= 55,	*/
  KEY88_8,			/*	SDLK_8			= 56,	*/
  KEY88_9,			/*	SDLK_9			= 57,	*/
  KEY88_COLON,			/*	SDLK_COLON		= 58,	*/
  KEY88_SEMICOLON,		/*	SDLK_SEMICOLON		= 59,	*/
  KEY88_LESS,			/*	SDLK_LESS		= 60,	*/
  KEY88_EQUAL,			/*	SDLK_EQUALS		= 61,	*/
  KEY88_GREATER,		/*	SDLK_GREATER		= 62,	*/
  KEY88_QUESTION,		/*	SDLK_QUESTION		= 63,	*/
  KEY88_AT,			/*	SDLK_AT			= 64,	*/
  KEY88_A,			/*					*/
  KEY88_B,			/*					*/
  KEY88_C,			/*					*/
  KEY88_D,			/*					*/
  KEY88_E,			/*					*/
  KEY88_F,			/*					*/
  KEY88_G,			/*					*/
  KEY88_H,			/*					*/
  KEY88_I,			/*					*/
  KEY88_J,			/*					*/
  KEY88_K,			/*					*/
  KEY88_L,			/*					*/
  KEY88_M,			/*					*/
  KEY88_N,			/*					*/
  KEY88_O,			/*					*/
  KEY88_P,			/*					*/
  KEY88_Q,			/*					*/
  KEY88_R,			/*					*/
  KEY88_S,			/*					*/
  KEY88_T,			/*					*/
  KEY88_U,			/*					*/
  KEY88_V,			/*					*/
  KEY88_W,			/*					*/
  KEY88_X,			/*					*/
  KEY88_Y,			/*					*/
  KEY88_Z,			/*					*/
  KEY88_BRACKETLEFT,		/*	SDLK_LEFTBRACKET	= 91,	*/
  KEY88_YEN,			/*	SDLK_BACKSLASH		= 92,	*/
  KEY88_BRACKETRIGHT,		/*	SDLK_RIGHTBRACKET	= 93,	*/
  KEY88_CARET,			/*	SDLK_CARET		= 94,	*/
  KEY88_UNDERSCORE,		/*	SDLK_UNDERSCORE		= 95,	*/
  KEY88_BACKQUOTE,		/*	SDLK_BACKQUOTE		= 96,	*/
  KEY88_a,			/*	SDLK_a			= 97,	*/
  KEY88_b,			/*	SDLK_b			= 98,	*/
  KEY88_c,			/*	SDLK_c			= 99,	*/
  KEY88_d,			/*	SDLK_d			= 100,	*/
  KEY88_e,			/*	SDLK_e			= 101,	*/
  KEY88_f,			/*	SDLK_f			= 102,	*/
  KEY88_g,			/*	SDLK_g			= 103,	*/
  KEY88_h,			/*	SDLK_h			= 104,	*/
  KEY88_i,			/*	SDLK_i			= 105,	*/
  KEY88_j,			/*	SDLK_j			= 106,	*/
  KEY88_k,			/*	SDLK_k			= 107,	*/
  KEY88_l,			/*	SDLK_l			= 108,	*/
  KEY88_m,			/*	SDLK_m			= 109,	*/
  KEY88_n,			/*	SDLK_n			= 110,	*/
  KEY88_o,			/*	SDLK_o			= 111,	*/
  KEY88_p,			/*	SDLK_p			= 112,	*/
  KEY88_q,			/*	SDLK_q			= 113,	*/
  KEY88_r,			/*	SDLK_r			= 114,	*/
  KEY88_s,			/*	SDLK_s			= 115,	*/
  KEY88_t,			/*	SDLK_t			= 116,	*/
  KEY88_u,			/*	SDLK_u			= 117,	*/
  KEY88_v,			/*	SDLK_v			= 118,	*/
  KEY88_w,			/*	SDLK_w			= 119,	*/
  KEY88_x,			/*	SDLK_x			= 120,	*/
  KEY88_y,			/*	SDLK_y			= 121,	*/
  KEY88_z,			/*	SDLK_z			= 122,	*/
  KEY88_BRACELEFT,		/*					*/
  KEY88_BAR,			/*					*/
  KEY88_BRACERIGHT,		/*					*/
  KEY88_TILDE,			/*					*/
  KEY88_DEL,			/*	SDLK_DELETE		= 127,	*/

  0, 0, 0, 0, 0, 0, 0, 0,	/*	SDLK_WORLD_0		= 160,	*/
  0, 0, 0, 0, 0, 0, 0, 0,	/*		:			*/
  0, 0, 0, 0, 0, 0, 0, 0,	/*		:			*/
  0, 0, 0, 0, 0, 0, 0, 0,	/*		:			*/
				/*		:			*/
  0, 0, 0, 0, 0, 0, 0, 0,	/*		:			*/
  0, 0, 0, 0, 0, 0, 0, 0,	/*		:			*/
  0, 0, 0, 0, 0, 0, 0, 0,	/*		:			*/
  0, 0, 0, 0, 0, 0, 0, 0,	/*		:			*/
  0, 0, 0, 0, 0, 0, 0, 0,	/*		:			*/
  0, 0, 0, 0, 0, 0, 0, 0,	/*		:			*/
  0, 0, 0, 0, 0, 0, 0, 0,	/*		:			*/
  0, 0, 0, 0, 0, 0, 0, 0,	/*		:			*/
  0, 0, 0, 0, 0, 0, 0, 0,	/*		:			*/
  0, 0, 0, 0, 0, 0, 0, 0,	/*		:			*/
  0, 0, 0, 0, 0, 0, 0, 0,	/*		:			*/
  0, 0, 0, 0, 0, 0, 0, 0,	/*	SDLK_WORLD_95		= 255,	*/

  KEY88_KP_0,			/*	SDLK_KP0		= 256,	*/
  KEY88_KP_1,			/*	SDLK_KP1		= 257,	*/
  KEY88_KP_2,			/*	SDLK_KP2		= 258,	*/
  KEY88_KP_3,			/*	SDLK_KP3		= 259,	*/
  KEY88_KP_4,			/*	SDLK_KP4		= 260,	*/
  KEY88_KP_5,			/*	SDLK_KP5		= 261,	*/
  KEY88_KP_6,			/*	SDLK_KP6		= 262,	*/
  KEY88_KP_7,			/*	SDLK_KP7		= 263,	*/
  KEY88_KP_8,			/*	SDLK_KP8		= 264,	*/
  KEY88_KP_9,			/*	SDLK_KP9		= 265,	*/
  KEY88_KP_PERIOD,		/*	SDLK_KP_PERIOD		= 266,	*/
  KEY88_KP_DIVIDE,		/*	SDLK_KP_DIVIDE		= 267,	*/
  KEY88_KP_MULTIPLY,		/*	SDLK_KP_MULTIPLY	= 268,	*/
  KEY88_KP_SUB,			/*	SDLK_KP_MINUS		= 269,	*/
  KEY88_KP_ADD,			/*	SDLK_KP_PLUS		= 270,	*/
  KEY88_RETURNR,		/*	SDLK_KP_ENTER		= 271,	*/
  KEY88_KP_EQUAL,		/*	SDLK_KP_EQUALS		= 272,	*/
  KEY88_UP,			/*	SDLK_UP			= 273,	*/
  KEY88_DOWN,			/*	SDLK_DOWN		= 274,	*/
  KEY88_RIGHT,			/*	SDLK_RIGHT		= 275,	*/
  KEY88_LEFT,			/*	SDLK_LEFT		= 276,	*/
  KEY88_INS,			/*	SDLK_INSERT		= 277,	*/
  KEY88_HOME,			/*	SDLK_HOME		= 278,	*/
  KEY88_HELP,			/*	SDLK_END		= 279,	*/
  KEY88_ROLLDOWN,		/*	SDLK_PAGEUP		= 280,	*/
  KEY88_ROLLUP,			/*	SDLK_PAGEDOWN		= 281,	*/
  KEY88_F1,			/*	SDLK_F1			= 282,	*/
  KEY88_F2,			/*	SDLK_F2			= 283,	*/
  KEY88_F3,			/*	SDLK_F3			= 284,	*/
  KEY88_F4,			/*	SDLK_F4			= 285,	*/
  KEY88_F5,			/*	SDLK_F5			= 286,	*/
  KEY88_F6,			/*	SDLK_F6			= 287,	*/
  KEY88_F7,			/*	SDLK_F7			= 288,	*/
  KEY88_F8,			/*	SDLK_F8			= 289,	*/
  KEY88_F9,			/*	SDLK_F9			= 290,	*/
  KEY88_F10,			/*	SDLK_F10		= 291,	*/
  KEY88_F11,			/*	SDLK_F11		= 292,	*/
  KEY88_F12,			/*	SDLK_F12		= 293,	*/
  KEY88_F13,			/*	SDLK_F13		= 294,	*/
  KEY88_F14,			/*	SDLK_F14		= 295,	*/
  KEY88_F15,			/*	SDLK_F15		= 296,	*/
  0, 0, 0,
  0,				/*	SDLK_NUMLOCK		= 300,	*/
  KEY88_CAPS,			/*	SDLK_CAPSLOCK		= 301,	*/
  KEY88_KANA,			/*	SDLK_SCROLLOCK		= 302,	*/
  KEY88_SHIFTR,			/*	SDLK_RSHIFT		= 303,	*/
  KEY88_SHIFTL,			/*	SDLK_LSHIFT		= 304,	*/
  KEY88_CTRL,			/*	SDLK_RCTRL		= 305,	*/
  KEY88_CTRL,			/*	SDLK_LCTRL		= 306,	*/
  KEY88_GRAPH,			/*	SDLK_RALT		= 307,	*/
  KEY88_GRAPH,			/*	SDLK_LALT		= 308,	*/
  KEY88_GRAPH,			/*	SDLK_RMETA		= 309,	*/
  KEY88_GRAPH,			/*	SDLK_LMETA		= 310,	*/
  0,				/*	SDLK_LSUPER		= 311,	*/
  0,				/*	SDLK_RSUPER		= 312,	*/
  0,				/*	SDLK_MODE		= 313,	*/
  0,				/*	SDLK_COMPOSE		= 314,	*/
  KEY88_HELP,			/*	SDLK_HELP		= 315,	*/
  KEY88_COPY,			/*	SDLK_PRINT		= 316,	*/
  0,				/*	SDLK_SYSREQ		= 317,	*/
  KEY88_STOP,			/*	SDLK_BREAK		= 318,	*/
  0,				/*	SDLK_MENU		= 319,	*/
  0,				/*	SDLK_POWER		= 320,	*/
  0,				/*	SDLK_EURO		= 321,	*/
  0,				/*	SDLK_UNDO		= 322,	*/
};



/*----------------------------------------------------------------------
 * keysym2key88[]   �ν���ͤϡ�keysym2key88_default[] ��Ʊ����
 * scancode2key88[] �ν���ͤϡ����� -1 (̤����) �Ǥ��뤬��
 * �����ܡ��ɤμ���˱����ơ�keysym2key88[] �� scancode2key88[] �ΰ�����
 * �ѹ����뤳�Ȥˤ��롣�ʲ��ϡ������ѹ��ξ���
 *----------------------------------------------------------------------*/

static const T_REMAPPING remapping_x11_106[] =
{
    {	KEYCODE_SYM,  SDLK_LSUPER,	KEY88_KANA,	    },
    {	KEYCODE_SYM,  SDLK_RALT,	KEY88_ZENKAKU,	    },
/*  {	KEYCODE_SYM,  SDLK_RCTRL,	KEY88_UNDERSCORE,   },*/
    {	KEYCODE_SYM,  SDLK_MENU,	KEY88_SYS_MENU,     },
    {	KEYCODE_SCAN,     49,		KEY88_ZENKAKU,	    },   /* Ⱦ������ */
    {	KEYCODE_SCAN,    133,		KEY88_YEN,	    },   /* \ |      */
    {	KEYCODE_SCAN,    123,		KEY88_UNDERSCORE,   },   /* \ _ ��   */
    {	KEYCODE_SCAN,    131,		KEY88_KETTEI,	    },
    {	KEYCODE_SCAN,    129,		KEY88_HENKAN,	    },
    {	KEYCODE_SCAN,    120,		KEY88_KANA,	    },   /* �����Ҥ� */
    {	KEYCODE_INVALID, 0,		0,		    },
};

static const T_REMAPPING remapping_x11_101[] =
{
    {	KEYCODE_SYM,  SDLK_BACKQUOTE,	KEY88_YEN,	    },
    {	KEYCODE_SYM,  SDLK_EQUALS,	KEY88_CARET,	    },
    {	KEYCODE_SYM,  SDLK_BACKSLASH,	KEY88_AT,	    },
    {	KEYCODE_SYM,  SDLK_QUOTE,	KEY88_COLON,	    },
    {	KEYCODE_SYM,  SDLK_LSUPER,	KEY88_KANA,	    },
    {	KEYCODE_SYM,  SDLK_RALT,	KEY88_ZENKAKU,	    },
    {	KEYCODE_SYM,  SDLK_RCTRL,	KEY88_UNDERSCORE,   },
    {	KEYCODE_SYM,  SDLK_MENU,	KEY88_SYS_MENU,     },
    {	KEYCODE_INVALID, 0,		0,		    },
};

static const T_REMAPPING remapping_windib_106[] =
{
    {	KEYCODE_SYM,  SDLK_BACKQUOTE,	0,		    },   /* Ⱦ������ */
    {	KEYCODE_SYM,  SDLK_EQUALS,	KEY88_CARET,	    },   /* ^        */
    {	KEYCODE_SYM,  SDLK_LEFTBRACKET,	KEY88_AT,	    },   /* @        */
    {	KEYCODE_SYM,  SDLK_RIGHTBRACKET,KEY88_BRACKETLEFT,  },   /* [        */
    {	KEYCODE_SYM,  SDLK_QUOTE,	KEY88_COLON,	    },   /* :        */
    {	KEYCODE_SYM,  SDLK_LSUPER,	KEY88_KANA,	    },   /* ��Window */
    {	KEYCODE_SYM,  SDLK_RALT,	KEY88_ZENKAKU,	    },   /* ��Alt    */
/*  {	KEYCODE_SYM,  SDLK_RCTRL,	KEY88_UNDERSCORE,   },*/ /* ��Ctrl   */
    {	KEYCODE_SYM,  SDLK_MENU,	KEY88_SYS_MENU,     },   /* Menu     */
    {	KEYCODE_SCAN,    125,		KEY88_YEN,	    },   /* \ |      */
    {	KEYCODE_SCAN,     43,		KEY88_BRACKETRIGHT, },   /* ] }      */
    {	KEYCODE_SCAN,    115,		KEY88_UNDERSCORE,   },   /* \ _ ��   */
    {	KEYCODE_SCAN,    123,		KEY88_KETTEI,	    },   /* ̵�Ѵ�   */
    {	KEYCODE_SCAN,    121,		KEY88_HENKAN,	    },   /* �Ѵ�     */
/*  {	KEYCODE_SCAN,    112,		0,		    },*/ /* �����Ҥ� */
    {	KEYCODE_INVALID, 0,		0,		    },
};

static const T_REMAPPING remapping_windib_101[] =
{
    {	KEYCODE_SYM,  SDLK_BACKQUOTE,	KEY88_YEN,	    },   /* `        */
    {	KEYCODE_SYM,  SDLK_EQUALS,	KEY88_CARET,	    },   /* =        */
    {	KEYCODE_SYM,  SDLK_BACKSLASH,	KEY88_AT,	    },   /* \        */
    {	KEYCODE_SYM,  SDLK_QUOTE,	KEY88_COLON,	    },   /* '        */
    {	KEYCODE_SYM,  SDLK_LSUPER,	KEY88_KANA,	    },   /* ��Window */
    {	KEYCODE_SYM,  SDLK_RALT,	KEY88_ZENKAKU,	    },   /* ��Alt    */
    {	KEYCODE_SYM,  SDLK_RCTRL,	KEY88_UNDERSCORE,   },   /* ��Ctrl   */
    {	KEYCODE_SYM,  SDLK_MENU,	KEY88_SYS_MENU,     },   /* Menu     */
    {	KEYCODE_INVALID, 0,		0,		    },
};

static const T_REMAPPING remapping_directx_106[] =
{
    {	KEYCODE_SYM,  SDLK_BACKSLASH,	KEY88_UNDERSCORE,   },   /* \ _ ��   */
    {	KEYCODE_SYM,  SDLK_LMETA,	KEY88_KANA,	    },   /* ��Window */
    {	KEYCODE_SYM,  SDLK_RALT,	KEY88_ZENKAKU,	    },   /* ��Alt    */
/*  {	KEYCODE_SYM,  SDLK_RCTRL,	KEY88_UNDERSCORE,   },*/ /* ��Ctrl   */
    {	KEYCODE_SYM,  SDLK_MENU,	KEY88_SYS_MENU,     },   /* Menu     */
/*  {	KEYCODE_SCAN,    148,		0,		    },*/ /* Ⱦ������ */
    {	KEYCODE_SCAN,    144,		KEY88_CARET,	    },   /* ^        */
    {	KEYCODE_SCAN,    125,		KEY88_YEN,	    },   /* \        */
    {	KEYCODE_SCAN,    145,		KEY88_AT,	    },   /* @        */
    {	KEYCODE_SCAN,    146,		KEY88_COLON,	    },   /* :        */
    {	KEYCODE_SCAN,    123,		KEY88_KETTEI,	    },   /* ̵�Ѵ�   */
    {	KEYCODE_SCAN,    121,		KEY88_HENKAN,	    },   /* �Ѵ�     */
    {	KEYCODE_SCAN,    112,		KEY88_KANA,	    },   /* �����Ҥ� */
    {	KEYCODE_INVALID, 0,		0,		    },
};

static const T_REMAPPING remapping_directx_101[] =
{
    {	KEYCODE_SYM,  SDLK_BACKQUOTE,	KEY88_YEN,	    },   /* `        */
    {	KEYCODE_SYM,  SDLK_EQUALS,	KEY88_CARET,	    },   /* =        */
    {	KEYCODE_SYM,  SDLK_BACKSLASH,	KEY88_AT,	    },   /* \        */
    {	KEYCODE_SYM,  SDLK_QUOTE,	KEY88_COLON,	    },   /* '        */
    {	KEYCODE_SYM,  SDLK_LMETA,	KEY88_KANA,	    },   /* ��Window */
    {	KEYCODE_SYM,  SDLK_RALT,	KEY88_ZENKAKU,	    },   /* ��Alt    */
    {	KEYCODE_SYM,  SDLK_RCTRL,	KEY88_UNDERSCORE,   },   /* ��Ctrl   */
    {	KEYCODE_SYM,  SDLK_MENU,	KEY88_SYS_MENU,     },   /* Menu     */
    {	KEYCODE_SCAN,    148,		KEY88_YEN,	    },
    {	KEYCODE_SCAN,    144,		KEY88_CARET,	    },
    {	KEYCODE_SCAN,    145,		KEY88_AT,	    },
    {	KEYCODE_SCAN,    146,		KEY88_COLON,	    },
    {	KEYCODE_SCAN,    125,		KEY88_YEN,	    },
    {	KEYCODE_INVALID, 0,		0,		    },
};

static const T_REMAPPING remapping_toolbox_106[] =
{
    {	KEYCODE_SYM,  SDLK_LMETA,	KEY88_SYS_MENU,	    },
    {	KEYCODE_SYM,  SDLK_RMETA,	KEY88_SYS_MENU,	    },
    {	KEYCODE_SCAN,    95,		KEY88_KP_COMMA,	    },
/*  {	KEYCODE_SCAN,    102,		0,		    },*/ /* �ѿ�     */
/*  {	KEYCODE_SCAN,    104,		0,		    },*/ /* ����     */
    {	KEYCODE_INVALID, 0,		0,		    },
};

static const T_REMAPPING remapping_toolbox_101[] =
{
    {	KEYCODE_SYM,  SDLK_LMETA,	KEY88_SYS_MENU,	    },
    {	KEYCODE_SYM,  SDLK_RMETA,	KEY88_SYS_MENU,	    },
    {	KEYCODE_SYM,  SDLK_BACKQUOTE,	KEY88_YEN,	    },
    {	KEYCODE_SYM,  SDLK_EQUALS,	KEY88_CARET,	    },
    {	KEYCODE_SYM,  SDLK_BACKSLASH,	KEY88_AT,	    },
    {	KEYCODE_SYM,  SDLK_QUOTE,	KEY88_COLON,	    },
    {	KEYCODE_INVALID, 0,		0,		    },
};

static const T_REMAPPING remapping_dummy[] =
{
    {	KEYCODE_INVALID, 0,		0,		    },
};



/*----------------------------------------------------------------------
 * ���եȥ����� NumLock ������� �����������Ѵ����� (�ǥե����)
 *----------------------------------------------------------------------*/

static const T_BINDING binding_106[] =
{
    {	KEYCODE_SYM,	SDLK_5,		KEY88_HOME,		0,	},
    {	KEYCODE_SYM,	SDLK_6,		KEY88_HELP,		0,	},
    {	KEYCODE_SYM,	SDLK_7,		KEY88_KP_7,		0,	},
    {	KEYCODE_SYM,	SDLK_8,		KEY88_KP_8,		0,	},
    {	KEYCODE_SYM,	SDLK_9,		KEY88_KP_9,		0,	},
    {	KEYCODE_SYM,	SDLK_0,		KEY88_KP_MULTIPLY,	0,	},
    {	KEYCODE_SYM,	SDLK_MINUS,	KEY88_KP_SUB,		0,	},
    {	KEYCODE_SYM,	SDLK_CARET,	KEY88_KP_DIVIDE,	0,	},
    {	KEYCODE_SYM,	SDLK_u,		KEY88_KP_4,		0,	},
    {	KEYCODE_SYM,	SDLK_i,		KEY88_KP_5,		0,	},
    {	KEYCODE_SYM,	SDLK_o,		KEY88_KP_6,		0,	},
    {	KEYCODE_SYM,	SDLK_p,		KEY88_KP_ADD,		0,	},
    {	KEYCODE_SYM,	SDLK_j,		KEY88_KP_1,		0,	},
    {	KEYCODE_SYM,	SDLK_k,		KEY88_KP_2,		0,	},
    {	KEYCODE_SYM,	SDLK_l,		KEY88_KP_3,		0,	},
    {	KEYCODE_SYM,	SDLK_SEMICOLON,	KEY88_KP_EQUAL,		0,	},
    {	KEYCODE_SYM,	SDLK_m,		KEY88_KP_0,		0,	},
    {	KEYCODE_SYM,	SDLK_COMMA,	KEY88_KP_COMMA,		0,	},
    {	KEYCODE_SYM,	SDLK_PERIOD,	KEY88_KP_PERIOD,	0,	},
    {	KEYCODE_SYM,	SDLK_SLASH,	KEY88_RETURNR,		0,	},
    {	KEYCODE_INVALID,0,		0,			0,	},
};

static const T_BINDING binding_101[] =
{
    {	KEYCODE_SYM,	SDLK_5,		KEY88_HOME,		0,	},
    {	KEYCODE_SYM,	SDLK_6,		KEY88_HELP,		0,	},
    {	KEYCODE_SYM,	SDLK_7,		KEY88_KP_7,		0,	},
    {	KEYCODE_SYM,	SDLK_8,		KEY88_KP_8,		0,	},
    {	KEYCODE_SYM,	SDLK_9,		KEY88_KP_9,		0,	},
    {	KEYCODE_SYM,	SDLK_0,		KEY88_KP_MULTIPLY,	0,	},
    {	KEYCODE_SYM,	SDLK_MINUS,	KEY88_KP_SUB,		0,	},
    {	KEYCODE_SYM,	SDLK_EQUALS,	KEY88_KP_DIVIDE,	0,	},
    {	KEYCODE_SYM,	SDLK_u,		KEY88_KP_4,		0,	},
    {	KEYCODE_SYM,	SDLK_i,		KEY88_KP_5,		0,	},
    {	KEYCODE_SYM,	SDLK_o,		KEY88_KP_6,		0,	},
    {	KEYCODE_SYM,	SDLK_p,		KEY88_KP_ADD,		0,	},
    {	KEYCODE_SYM,	SDLK_j,		KEY88_KP_1,		0,	},
    {	KEYCODE_SYM,	SDLK_k,		KEY88_KP_2,		0,	},
    {	KEYCODE_SYM,	SDLK_l,		KEY88_KP_3,		0,	},
    {	KEYCODE_SYM,	SDLK_SEMICOLON,	KEY88_KP_EQUAL,		0,	},
    {	KEYCODE_SYM,	SDLK_m,		KEY88_KP_0,		0,	},
    {	KEYCODE_SYM,	SDLK_COMMA,	KEY88_KP_COMMA,		0,	},
    {	KEYCODE_SYM,	SDLK_PERIOD,	KEY88_KP_PERIOD,	0,	},
    {	KEYCODE_SYM,	SDLK_SLASH,	KEY88_RETURNR,		0,	},
    {	KEYCODE_INVALID,0,		0,			0,	},
};

static const T_BINDING binding_directx[] =
{
    {	KEYCODE_SYM,	SDLK_5,		KEY88_HOME,		0,	},
    {	KEYCODE_SYM,	SDLK_6,		KEY88_HELP,		0,	},
    {	KEYCODE_SYM,	SDLK_7,		KEY88_KP_7,		0,	},
    {	KEYCODE_SYM,	SDLK_8,		KEY88_KP_8,		0,	},
    {	KEYCODE_SYM,	SDLK_9,		KEY88_KP_9,		0,	},
    {	KEYCODE_SYM,	SDLK_0,		KEY88_KP_MULTIPLY,	0,	},
    {	KEYCODE_SYM,	SDLK_MINUS,	KEY88_KP_SUB,		0,	},
    {	KEYCODE_SYM,	SDLK_EQUALS,	KEY88_KP_DIVIDE,	0,	},
    {	KEYCODE_SCAN,	144,		KEY88_KP_DIVIDE,	0,	},
    {	KEYCODE_SYM,	SDLK_u,		KEY88_KP_4,		0,	},
    {	KEYCODE_SYM,	SDLK_i,		KEY88_KP_5,		0,	},
    {	KEYCODE_SYM,	SDLK_o,		KEY88_KP_6,		0,	},
    {	KEYCODE_SYM,	SDLK_p,		KEY88_KP_ADD,		0,	},
    {	KEYCODE_SYM,	SDLK_j,		KEY88_KP_1,		0,	},
    {	KEYCODE_SYM,	SDLK_k,		KEY88_KP_2,		0,	},
    {	KEYCODE_SYM,	SDLK_l,		KEY88_KP_3,		0,	},
    {	KEYCODE_SYM,	SDLK_SEMICOLON,	KEY88_KP_EQUAL,		0,	},
    {	KEYCODE_SYM,	SDLK_m,		KEY88_KP_0,		0,	},
    {	KEYCODE_SYM,	SDLK_COMMA,	KEY88_KP_COMMA,		0,	},
    {	KEYCODE_SYM,	SDLK_PERIOD,	KEY88_KP_PERIOD,	0,	},
    {	KEYCODE_SYM,	SDLK_SLASH,	KEY88_RETURNR,		0,	},
    {	KEYCODE_INVALID,0,		0,			0,	},
};



/******************************************************************************
 * ���٥�ȥϥ�ɥ��
 *
 *	1/60��˸ƤӽФ���롣
 *****************************************************************************/
static	int	joystick_init(void);
static	void	joystick_exit(void);
static	int	analyze_keyconf_file(void);

static	char	video_driver[32];

/*
 * ����� ��ư����1������ƤФ��
 */
void	event_init(void)
{
    const T_REMAPPING *map;
    const T_BINDING   *bin;
    int i;

    /* ���祤���ƥ��å������ */

    if (use_joydevice) {
	if (verbose_proc) printf("Initializing Joystick System ... ");
	i = joystick_init();
	if (verbose_proc) {
	    if (i) printf("OK (found %d joystick(s))\n", i);
	    else   printf("FAILED\n");
	}
    }


    /* �����ޥåԥ󥰽���� */

    if (SDL_VideoDriverName(video_driver, sizeof(video_driver)) == NULL) {
	memset(video_driver, 0, sizeof(video_driver));
    }

    memset(keysym2key88, 0, sizeof(keysym2key88));
    for (i=0; i<COUNTOF(scancode2key88); i++) {
	scancode2key88[ i ] = -1;
    }
    memset(binding, 0, sizeof(binding));



    switch (keyboard_type) {

    case 0:					/* �ǥե���ȥ����ܡ��� */
	if (analyze_keyconf_file()) {
	    ;
	} else {
	    memcpy(keysym2key88,
		   keysym2key88_default, sizeof(keysym2key88_default));
	    memcpy(binding,
		   binding_106, sizeof(binding_106));
	}
	break;


    case 1:					/* 106���ܸ쥭���ܡ��� */
    case 2:					/* 101�Ѹ쥭���ܡ��� ? */
	memcpy(keysym2key88,
	       keysym2key88_default, sizeof(keysym2key88_default));

#if	defined(QUASI88_FUNIX)

	if (keyboard_type == 1) {
	    map = remapping_x11_106;
	    bin = binding_106;
	} else {
	    map = remapping_x11_101;
	    bin = binding_101;
	}

#elif	defined(QUASI88_FWIN)

	if (strcmp(video_driver, "directx") == 0) {
	    if (keyboard_type == 1) map = remapping_directx_106;
	    else                    map = remapping_directx_101;
	    bin = binding_directx;
	} else {
	    if (keyboard_type == 1) map = remapping_windib_106;
	    else                    map = remapping_windib_101;
	    bin = binding_101;
	}

#elif	defined(QUASI88_FMAC)

	if (keyboard_type == 1) {
	    map = remapping_toolbox_106;
	    bin = binding_106;
	} else {
	    map = remapping_toolbox_101;
	    bin = binding_101;
	}

	if (use_cmdkey == FALSE) {
	    map += 2;
	}

#else
	map = remapping_dummy;
	bin = binding_106;
#endif

	for ( ; map->type; map ++) {

	    if        (map->type == KEYCODE_SYM) {

		keysym2key88[ map->code ] = map->key88;

	    } else if (map->type == KEYCODE_SCAN) {

		scancode2key88[ map->code ] = map->key88;

	    }
	}

	for (i=0; i<COUNTOF(binding); i++) {
	    if (bin->type == KEYCODE_INVALID) break;

	    binding[ i ].type      = bin->type;
	    binding[ i ].code      = bin->code;
	    binding[ i ].org_key88 = bin->org_key88;
	    binding[ i ].new_key88 = bin->new_key88;
	    bin ++;
	}
	break;
    }



    /* ���եȥ�����NumLock ���Υ��������ؤ��ν��� */

    for (i=0; i<COUNTOF(binding); i++) {

	if        (binding[i].type == KEYCODE_SYM) {

	    binding[i].org_key88 = keysym2key88[ binding[i].code ];

	} else if (binding[i].type == KEYCODE_SCAN) {

	    binding[i].org_key88 = scancode2key88[ binding[i].code ];

	} else {
	    break;
	}
    }


    /* test */
    if (show_fps) {
	if (display_fps_init() == FALSE) {
	    show_fps = FALSE;
	}
    }
}



/*
 * �� 1/60 ��˸ƤФ��
 */
void	event_update(void)
{
    SDL_Event E;
    SDLKey keysym;
    int    key88, x, y;


    SDL_PumpEvents();		/* ���٥�Ȥ��߾夲�� */

    while (SDL_PeepEvents(&E, 1, SDL_GETEVENT,
			  SDL_EVENTMASK(SDL_KEYDOWN)        | 
			  SDL_EVENTMASK(SDL_KEYUP)          |
			  SDL_EVENTMASK(SDL_MOUSEMOTION)    |
			  SDL_EVENTMASK(SDL_MOUSEBUTTONDOWN)|
			  SDL_EVENTMASK(SDL_MOUSEBUTTONUP)  |
			  SDL_EVENTMASK(SDL_JOYAXISMOTION)  |
			  SDL_EVENTMASK(SDL_JOYBUTTONDOWN)  |
			  SDL_EVENTMASK(SDL_JOYBUTTONUP)    |
			  SDL_EVENTMASK(SDL_VIDEOEXPOSE)    |
			  SDL_EVENTMASK(SDL_ACTIVEEVENT)    |
			  SDL_EVENTMASK(SDL_USEREVENT)      |
			  SDL_EVENTMASK(SDL_QUIT))) {

	switch (E.type) {

	case SDL_KEYDOWN:	/*------------------------------------------*/
	case SDL_KEYUP:

	    keysym  = E.key.keysym.sym;

	    if (now_unicode == FALSE) {	/* ����ASCII�� */

		/* scancode2key88[] ������Ѥʤ顢���Υ��������ɤ�ͥ�褹�� */
		if (E.key.keysym.scancode < COUNTOF(scancode2key88) &&
		    scancode2key88[ E.key.keysym.scancode ] >= 0) {

		    key88 = scancode2key88[ E.key.keysym.scancode ];

		} else {
		    key88 = keysym2key88[ keysym ];
		}

	    } else {			/* ����UNICODE�� (��˥塼�ʤ�) */

		if (E.key.keysym.unicode <= 0xff &&
		    isprint(E.key.keysym.unicode)) {
		    keysym = E.key.keysym.unicode;
		}
		if (SDLK_SPACE <= keysym && keysym < SDLK_DELETE) {
		    /* ASCII�����ɤ��ϰ���Ǥϡ�SDLK_xx �� KEY88_xx ������ */
		    key88 = keysym;
		} else {
		    key88 = keysym2key88[ keysym ];
		}
	    }

	    /*if (E.type==SDL_KEYDOWN)
		printf("scan=%3d : %04x=%-16s -> %d\n", E.key.keysym.scancode,
		       keysym, debug_sdlkeysym(keysym), key88);*/
	    /*printf("%d %d %d\n",key88,keysym,E.key.keysym.scancode);*/
	    quasi88_key(key88, (E.type == SDL_KEYDOWN));

	    break;

	case SDL_MOUSEMOTION:	/*------------------------------------------*/
	    if (sdl_mouse_rel_move) {	/* �ޥ�����������ɥ���ü���Ϥ��Ƥ� */
					/* ����Ū��ư���򸡽ФǤ�����     */
		x = E.motion.xrel;
		y = E.motion.yrel;

		quasi88_mouse_moved_rel(x, y);

	    } else {

		x = E.motion.x;
		y = E.motion.y;

		quasi88_mouse_moved_abs(x, y);
	    }
	    break;

	case SDL_MOUSEBUTTONDOWN:/*------------------------------------------*/
	case SDL_MOUSEBUTTONUP:
	    /* �ޥ�����ư���٥�Ȥ�Ʊ���˽�������ɬ�פ�����ʤ顢
	       quasi88_mouse_moved_abs/rel �ؿ��򤳤��ǸƤӽФ��Ƥ��� */

	    switch (E.button.button) {
	    case SDL_BUTTON_LEFT:	key88 = KEY88_MOUSE_L;		break;
	    case SDL_BUTTON_MIDDLE:	key88 = KEY88_MOUSE_M;		break;
	    case SDL_BUTTON_RIGHT:	key88 = KEY88_MOUSE_R;		break;
	    case SDL_BUTTON_WHEELUP:	key88 = KEY88_MOUSE_WUP;	break;
	    case SDL_BUTTON_WHEELDOWN:	key88 = KEY88_MOUSE_WDN;	break;
	    default:			key88 = 0;			break;
	    }
	    if (key88) {
		quasi88_mouse(key88, (E.type == SDL_MOUSEBUTTONDOWN));
	    }
	    break;

	case SDL_JOYAXISMOTION:	/*------------------------------------------*/
	    /*printf("%d %d %d\n",E.jaxis.which,E.jaxis.axis,E.jaxis.value);*/

	    if (E.jbutton.which < JOY_MAX &&
		joy_info[E.jbutton.which].dev != NULL) {

		int now, chg;
		T_JOY_INFO *joy = &joy_info[E.jbutton.which];
		int offset = (joy->num) * KEY88_PAD_OFFSET;

		if (E.jaxis.axis == 0) {	/* �������� */

		    now = joy->axis & ~(AXIS_L|AXIS_R);

		    if      (E.jaxis.value < -0x4000) now |= AXIS_L;
		    else if (E.jaxis.value >  0x4000) now |= AXIS_R;

		    chg = joy->axis ^ now;
		    if (chg & AXIS_L) {
			quasi88_pad(KEY88_PAD1_LEFT + offset,  (now & AXIS_L));
		    }
		    if (chg & AXIS_R) {
			quasi88_pad(KEY88_PAD1_RIGHT + offset, (now & AXIS_R));
		    }

		} else {			/* �岼���� */

		    now = joy->axis & ~(AXIS_U|AXIS_D);

		    if      (E.jaxis.value < -0x4000) now |= AXIS_U;
		    else if (E.jaxis.value >  0x4000) now |= AXIS_D;

		    chg = joy->axis ^ now;
		    if (chg & AXIS_U) {
			quasi88_pad(KEY88_PAD1_UP + offset,   (now & AXIS_U));
		    }
		    if (chg & AXIS_D) {
			quasi88_pad(KEY88_PAD1_DOWN + offset, (now & AXIS_D));
		    }
		}
		joy->axis = now;
	    }
	    break;

	case SDL_JOYBUTTONDOWN:	/*------------------------------------------*/
	case SDL_JOYBUTTONUP:
	    /*printf("%d %d\n",E.jbutton.which,E.jbutton.button);*/

	    if (E.jbutton.which < JOY_MAX &&
		joy_info[E.jbutton.which].dev != NULL) {

		T_JOY_INFO *joy = &joy_info[E.jbutton.which];
		int offset = (joy->num) * KEY88_PAD_OFFSET;

		if (E.jbutton.button < KEY88_PAD_BUTTON_MAX) {
		    key88 = KEY88_PAD1_A + E.jbutton.button + offset;
		    quasi88_pad(key88, (E.type == SDL_JOYBUTTONDOWN));
		}
	    }
	    break;

	case SDL_QUIT:		/*------------------------------------------*/
	    if (verbose_proc) printf("Window Closed !\n");
	    quasi88_quit();
	    break;

	case SDL_ACTIVEEVENT:	/*------------------------------------------*/
	    /* -focus ���ץ�����ǽ���������ʤ顢 
	       quasi88_focus_in / quasi88_focus_out ��Ŭ���ƤӽФ�ɬ�פ����� */

	    if (E.active.state & SDL_APPINPUTFOCUS) {
		if (E.active.gain) {
		    quasi88_focus_in();
		} else {
		    quasi88_focus_out();
		}
	    }
	    break;

	case SDL_VIDEOEXPOSE:	/*------------------------------------------*/
	    quasi88_expose();
	    break;

	case SDL_USEREVENT:	/*------------------------------------------*/
	    if (E.user.code == 1) {
		display_fps();		/* test */
	    }
	    break;
	}
    }
}



/*
 * ����� ��λ����1������ƤФ��
 */
void	event_exit(void)
{
    joystick_exit();
}



/***********************************************************************
 * ���ߤΥޥ�����ɸ�����ؿ�
 *
 *	���ߤΥޥ��������к�ɸ�� *x, *y �˥��å�
 ************************************************************************/

void	event_get_mouse_pos(int *x, int *y)
{
    int win_x, win_y;

    SDL_PumpEvents();
    SDL_GetMouseState(&win_x, &win_y);

    *x = win_x;
    *y = win_y;
}




/******************************************************************************
 * ���եȥ����� NumLock ͭ����̵��
 *
 *****************************************************************************/

static	void	numlock_setup(int enable)
{
    int i;

    for (i=0; i<COUNTOF(binding); i++) {

	if        (binding[i].type == KEYCODE_SYM) {

	    if (enable) {
		keysym2key88[ binding[i].code ] = binding[i].new_key88;
	    } else {
		keysym2key88[ binding[i].code ] = binding[i].org_key88;
	    }

	} else if (binding[i].type == KEYCODE_SCAN) {

	    if (enable) {
		scancode2key88[ binding[i].code ] = binding[i].new_key88;
	    } else {
		scancode2key88[ binding[i].code ] = binding[i].org_key88;
	    }

	} else {
	    break;
	}
    }
}

int	event_numlock_on (void){ numlock_setup(TRUE);  return TRUE; }
void	event_numlock_off(void){ numlock_setup(FALSE); }



/******************************************************************************
 * ���ߥ�졼�ȡ���˥塼���ݡ�������˥����⡼�� �� ���ϻ��ν���
 *
 *****************************************************************************/

void	event_switch(void)
{
    /* ��¸�Υ��٥�Ȥ򤹤٤��˴� */
    /* �ʤ�Ƥ��Ȥϡ����ʤ� ? */

    if (quasi88_is_exec()) {

	if (use_unicode) {

	    /* ���������� UNICODE ���Ѵ���ǽ�Ȥ���
	       (�������Ť��餷���Τǻ�����Τ�) */
	    SDL_EnableUNICODE( 1 );
	    now_unicode = TRUE;

	} else {

	    /* ���������� ASCII �����ɤ��Ѵ���ǽ�Ȥ��� */
	    SDL_EnableUNICODE( 0 );
	    now_unicode = FALSE;
	}

    } else {

	SDL_EnableUNICODE( 1 );
	now_unicode = TRUE;

    }
}



/******************************************************************************
 * ���祤���ƥ��å�
 *****************************************************************************/

static	int	joystick_init(void)
{
    SDL_Joystick *dev;
    int i, max, nr_button;

    /* �������� */
    joystick_num = 0;

    memset(joy_info, 0, sizeof(joy_info));
    for (i=0; i<JOY_MAX; i++) {
	joy_info[i].dev = NULL;
    }

    /* ���祤���ƥ��å����֥����ƥ����� */
    if (! SDL_WasInit(SDL_INIT_JOYSTICK)) {
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK)) {
	    return 0;
	}
    }

    /* ���祤���ƥ��å��ο���Ĵ�٤ơ��ǥХ��������ץ� */
    max = SDL_NumJoysticks();
    max = MIN(max, JOY_MAX);		/* ����ο�������ͭ�� */

    for (i=0; i<max; i++) {
	dev = SDL_JoystickOpen(i);	/* i���ܤΥ��祤���ƥ��å��򥪡��ץ� */

	if (dev) {
	    /* �ܥ���ο���Ĵ�٤� */
	    nr_button = SDL_JoystickNumButtons(dev);
	    nr_button = MIN(nr_button, BUTTON_MAX);

	    joy_info[i].dev = dev;
	    joy_info[i].num = joystick_num ++;
	    joy_info[i].nr_button = nr_button;
	}
    }

    if (joystick_num > 0) {			/* 1�İʾ奪���ץ�Ǥ�����  */
	SDL_JoystickEventState(SDL_ENABLE);	/* ���٥�Ƚ�����ͭ���ˤ��� */
    }

    return joystick_num;			/* ���祤���ƥ��å��ο����֤� */
}



static	void	joystick_exit(void)
{
    int i;

    if (joystick_num > 0) {

	for (i=0; i<JOY_MAX; i++) {
	    if (joy_info[i].dev) {
		SDL_JoystickClose(joy_info[i].dev);
		joy_info[i].dev = NULL;
	    }
	}

	joystick_num = 0;
    }
}



int	event_get_joystick_num(void)
{
    return joystick_num;
}






/****************************************************************************
 * ��������ե�������ɤ߹���ǡ����ꤹ�롣
 *	����ե����뤬̵����е�������н������ƿ����֤�
 *****************************************************************************/

/* SDL �� keysym ��ʸ����� int �ͤ��Ѵ�����ơ��֥� */

static const T_SYMBOL_TABLE sdlkeysym_list[] =
{
    {	"SDLK_BACKSPACE",	SDLK_BACKSPACE		}, /*	= 8,	*/
    {	"SDLK_TAB",		SDLK_TAB		}, /*	= 9,	*/
    {	"SDLK_CLEAR",		SDLK_CLEAR		}, /*	= 12,	*/
    {	"SDLK_RETURN",		SDLK_RETURN		}, /*	= 13,	*/
    {	"SDLK_PAUSE",		SDLK_PAUSE		}, /*	= 19,	*/
    {	"SDLK_ESCAPE",		SDLK_ESCAPE		}, /*	= 27,	*/
    {	"SDLK_SPACE",		SDLK_SPACE		}, /*	= 32,	*/
    {	"SDLK_EXCLAIM",		SDLK_EXCLAIM		}, /*	= 33,	*/
    {	"SDLK_QUOTEDBL",	SDLK_QUOTEDBL		}, /*	= 34,	*/
    {	"SDLK_HASH",		SDLK_HASH		}, /*	= 35,	*/
    {	"SDLK_DOLLAR",		SDLK_DOLLAR		}, /*	= 36,	*/
    {	"SDLK_AMPERSAND",	SDLK_AMPERSAND		}, /*	= 38,	*/
    {	"SDLK_QUOTE",		SDLK_QUOTE		}, /*	= 39,	*/
    {	"SDLK_LEFTPAREN",	SDLK_LEFTPAREN		}, /*	= 40,	*/
    {	"SDLK_RIGHTPAREN",	SDLK_RIGHTPAREN		}, /*	= 41,	*/
    {	"SDLK_ASTERISK",	SDLK_ASTERISK		}, /*	= 42,	*/
    {	"SDLK_PLUS",		SDLK_PLUS		}, /*	= 43,	*/
    {	"SDLK_COMMA",		SDLK_COMMA		}, /*	= 44,	*/
    {	"SDLK_MINUS",		SDLK_MINUS		}, /*	= 45,	*/
    {	"SDLK_PERIOD",		SDLK_PERIOD		}, /*	= 46,	*/
    {	"SDLK_SLASH",		SDLK_SLASH		}, /*	= 47,	*/
    {	"SDLK_0",		SDLK_0			}, /*	= 48,	*/
    {	"SDLK_1",		SDLK_1			}, /*	= 49,	*/
    {	"SDLK_2",		SDLK_2			}, /*	= 50,	*/
    {	"SDLK_3",		SDLK_3			}, /*	= 51,	*/
    {	"SDLK_4",		SDLK_4			}, /*	= 52,	*/
    {	"SDLK_5",		SDLK_5			}, /*	= 53,	*/
    {	"SDLK_6",		SDLK_6			}, /*	= 54,	*/
    {	"SDLK_7",		SDLK_7			}, /*	= 55,	*/
    {	"SDLK_8",		SDLK_8			}, /*	= 56,	*/
    {	"SDLK_9",		SDLK_9			}, /*	= 57,	*/
    {	"SDLK_COLON",		SDLK_COLON		}, /*	= 58,	*/
    {	"SDLK_SEMICOLON",	SDLK_SEMICOLON		}, /*	= 59,	*/
    {	"SDLK_LESS",		SDLK_LESS		}, /*	= 60,	*/
    {	"SDLK_EQUALS",		SDLK_EQUALS		}, /*	= 61,	*/
    {	"SDLK_GREATER",		SDLK_GREATER		}, /*	= 62,	*/
    {	"SDLK_QUESTION",	SDLK_QUESTION		}, /*	= 63,	*/
    {	"SDLK_AT",		SDLK_AT			}, /*	= 64,	*/
    {	"SDLK_LEFTBRACKET",	SDLK_LEFTBRACKET	}, /*	= 91,	*/
    {	"SDLK_BACKSLASH",	SDLK_BACKSLASH		}, /*	= 92,	*/
    {	"SDLK_RIGHTBRACKET",	SDLK_RIGHTBRACKET	}, /*	= 93,	*/
    {	"SDLK_CARET",		SDLK_CARET		}, /*	= 94,	*/
    {	"SDLK_UNDERSCORE",	SDLK_UNDERSCORE		}, /*	= 95,	*/
    {	"SDLK_BACKQUOTE",	SDLK_BACKQUOTE		}, /*	= 96,	*/
    {	"SDLK_a",		SDLK_a			}, /*	= 97,	*/
    {	"SDLK_b",		SDLK_b			}, /*	= 98,	*/
    {	"SDLK_c",		SDLK_c			}, /*	= 99,	*/
    {	"SDLK_d",		SDLK_d			}, /*	= 100,	*/
    {	"SDLK_e",		SDLK_e			}, /*	= 101,	*/
    {	"SDLK_f",		SDLK_f			}, /*	= 102,	*/
    {	"SDLK_g",		SDLK_g			}, /*	= 103,	*/
    {	"SDLK_h",		SDLK_h			}, /*	= 104,	*/
    {	"SDLK_i",		SDLK_i			}, /*	= 105,	*/
    {	"SDLK_j",		SDLK_j			}, /*	= 106,	*/
    {	"SDLK_k",		SDLK_k			}, /*	= 107,	*/
    {	"SDLK_l",		SDLK_l			}, /*	= 108,	*/
    {	"SDLK_m",		SDLK_m			}, /*	= 109,	*/
    {	"SDLK_n",		SDLK_n			}, /*	= 110,	*/
    {	"SDLK_o",		SDLK_o			}, /*	= 111,	*/
    {	"SDLK_p",		SDLK_p			}, /*	= 112,	*/
    {	"SDLK_q",		SDLK_q			}, /*	= 113,	*/
    {	"SDLK_r",		SDLK_r			}, /*	= 114,	*/
    {	"SDLK_s",		SDLK_s			}, /*	= 115,	*/
    {	"SDLK_t",		SDLK_t			}, /*	= 116,	*/
    {	"SDLK_u",		SDLK_u			}, /*	= 117,	*/
    {	"SDLK_v",		SDLK_v			}, /*	= 118,	*/
    {	"SDLK_w",		SDLK_w			}, /*	= 119,	*/
    {	"SDLK_x",		SDLK_x			}, /*	= 120,	*/
    {	"SDLK_y",		SDLK_y			}, /*	= 121,	*/
    {	"SDLK_z",		SDLK_z			}, /*	= 122,	*/
    {	"SDLK_DELETE",		SDLK_DELETE		}, /*	= 127,	*/
    {	"SDLK_WORLD_0",		SDLK_WORLD_0		}, /*	= 160,	*/
    {	"SDLK_WORLD_1",		SDLK_WORLD_1		}, /*	= 161,	*/
    {	"SDLK_WORLD_2",		SDLK_WORLD_2		}, /*	= 162,	*/
    {	"SDLK_WORLD_3",		SDLK_WORLD_3		}, /*	= 163,	*/
    {	"SDLK_WORLD_4",		SDLK_WORLD_4		}, /*	= 164,	*/
    {	"SDLK_WORLD_5",		SDLK_WORLD_5		}, /*	= 165,	*/
    {	"SDLK_WORLD_6",		SDLK_WORLD_6		}, /*	= 166,	*/
    {	"SDLK_WORLD_7",		SDLK_WORLD_7		}, /*	= 167,	*/
    {	"SDLK_WORLD_8",		SDLK_WORLD_8		}, /*	= 168,	*/
    {	"SDLK_WORLD_9",		SDLK_WORLD_9		}, /*	= 169,	*/
    {	"SDLK_WORLD_10",	SDLK_WORLD_10		}, /*	= 170,	*/
    {	"SDLK_WORLD_11",	SDLK_WORLD_11		}, /*	= 171,	*/
    {	"SDLK_WORLD_12",	SDLK_WORLD_12		}, /*	= 172,	*/
    {	"SDLK_WORLD_13",	SDLK_WORLD_13		}, /*	= 173,	*/
    {	"SDLK_WORLD_14",	SDLK_WORLD_14		}, /*	= 174,	*/
    {	"SDLK_WORLD_15",	SDLK_WORLD_15		}, /*	= 175,	*/
    {	"SDLK_WORLD_16",	SDLK_WORLD_16		}, /*	= 176,	*/
    {	"SDLK_WORLD_17",	SDLK_WORLD_17		}, /*	= 177,	*/
    {	"SDLK_WORLD_18",	SDLK_WORLD_18		}, /*	= 178,	*/
    {	"SDLK_WORLD_19",	SDLK_WORLD_19		}, /*	= 179,	*/
    {	"SDLK_WORLD_20",	SDLK_WORLD_20		}, /*	= 180,	*/
    {	"SDLK_WORLD_21",	SDLK_WORLD_21		}, /*	= 181,	*/
    {	"SDLK_WORLD_22",	SDLK_WORLD_22		}, /*	= 182,	*/
    {	"SDLK_WORLD_23",	SDLK_WORLD_23		}, /*	= 183,	*/
    {	"SDLK_WORLD_24",	SDLK_WORLD_24		}, /*	= 184,	*/
    {	"SDLK_WORLD_25",	SDLK_WORLD_25		}, /*	= 185,	*/
    {	"SDLK_WORLD_26",	SDLK_WORLD_26		}, /*	= 186,	*/
    {	"SDLK_WORLD_27",	SDLK_WORLD_27		}, /*	= 187,	*/
    {	"SDLK_WORLD_28",	SDLK_WORLD_28		}, /*	= 188,	*/
    {	"SDLK_WORLD_29",	SDLK_WORLD_29		}, /*	= 189,	*/
    {	"SDLK_WORLD_30",	SDLK_WORLD_30		}, /*	= 190,	*/
    {	"SDLK_WORLD_31",	SDLK_WORLD_31		}, /*	= 191,	*/
    {	"SDLK_WORLD_32",	SDLK_WORLD_32		}, /*	= 192,	*/
    {	"SDLK_WORLD_33",	SDLK_WORLD_33		}, /*	= 193,	*/
    {	"SDLK_WORLD_34",	SDLK_WORLD_34		}, /*	= 194,	*/
    {	"SDLK_WORLD_35",	SDLK_WORLD_35		}, /*	= 195,	*/
    {	"SDLK_WORLD_36",	SDLK_WORLD_36		}, /*	= 196,	*/
    {	"SDLK_WORLD_37",	SDLK_WORLD_37		}, /*	= 197,	*/
    {	"SDLK_WORLD_38",	SDLK_WORLD_38		}, /*	= 198,	*/
    {	"SDLK_WORLD_39",	SDLK_WORLD_39		}, /*	= 199,	*/
    {	"SDLK_WORLD_40",	SDLK_WORLD_40		}, /*	= 200,	*/
    {	"SDLK_WORLD_41",	SDLK_WORLD_41		}, /*	= 201,	*/
    {	"SDLK_WORLD_42",	SDLK_WORLD_42		}, /*	= 202,	*/
    {	"SDLK_WORLD_43",	SDLK_WORLD_43		}, /*	= 203,	*/
    {	"SDLK_WORLD_44",	SDLK_WORLD_44		}, /*	= 204,	*/
    {	"SDLK_WORLD_45",	SDLK_WORLD_45		}, /*	= 205,	*/
    {	"SDLK_WORLD_46",	SDLK_WORLD_46		}, /*	= 206,	*/
    {	"SDLK_WORLD_47",	SDLK_WORLD_47		}, /*	= 207,	*/
    {	"SDLK_WORLD_48",	SDLK_WORLD_48		}, /*	= 208,	*/
    {	"SDLK_WORLD_49",	SDLK_WORLD_49		}, /*	= 209,	*/
    {	"SDLK_WORLD_50",	SDLK_WORLD_50		}, /*	= 210,	*/
    {	"SDLK_WORLD_51",	SDLK_WORLD_51		}, /*	= 211,	*/
    {	"SDLK_WORLD_52",	SDLK_WORLD_52		}, /*	= 212,	*/
    {	"SDLK_WORLD_53",	SDLK_WORLD_53		}, /*	= 213,	*/
    {	"SDLK_WORLD_54",	SDLK_WORLD_54		}, /*	= 214,	*/
    {	"SDLK_WORLD_55",	SDLK_WORLD_55		}, /*	= 215,	*/
    {	"SDLK_WORLD_56",	SDLK_WORLD_56		}, /*	= 216,	*/
    {	"SDLK_WORLD_57",	SDLK_WORLD_57		}, /*	= 217,	*/
    {	"SDLK_WORLD_58",	SDLK_WORLD_58		}, /*	= 218,	*/
    {	"SDLK_WORLD_59",	SDLK_WORLD_59		}, /*	= 219,	*/
    {	"SDLK_WORLD_60",	SDLK_WORLD_60		}, /*	= 220,	*/
    {	"SDLK_WORLD_61",	SDLK_WORLD_61		}, /*	= 221,	*/
    {	"SDLK_WORLD_62",	SDLK_WORLD_62		}, /*	= 222,	*/
    {	"SDLK_WORLD_63",	SDLK_WORLD_63		}, /*	= 223,	*/
    {	"SDLK_WORLD_64",	SDLK_WORLD_64		}, /*	= 224,	*/
    {	"SDLK_WORLD_65",	SDLK_WORLD_65		}, /*	= 225,	*/
    {	"SDLK_WORLD_66",	SDLK_WORLD_66		}, /*	= 226,	*/
    {	"SDLK_WORLD_67",	SDLK_WORLD_67		}, /*	= 227,	*/
    {	"SDLK_WORLD_68",	SDLK_WORLD_68		}, /*	= 228,	*/
    {	"SDLK_WORLD_69",	SDLK_WORLD_69		}, /*	= 229,	*/
    {	"SDLK_WORLD_70",	SDLK_WORLD_70		}, /*	= 230,	*/
    {	"SDLK_WORLD_71",	SDLK_WORLD_71		}, /*	= 231,	*/
    {	"SDLK_WORLD_72",	SDLK_WORLD_72		}, /*	= 232,	*/
    {	"SDLK_WORLD_73",	SDLK_WORLD_73		}, /*	= 233,	*/
    {	"SDLK_WORLD_74",	SDLK_WORLD_74		}, /*	= 234,	*/
    {	"SDLK_WORLD_75",	SDLK_WORLD_75		}, /*	= 235,	*/
    {	"SDLK_WORLD_76",	SDLK_WORLD_76		}, /*	= 236,	*/
    {	"SDLK_WORLD_77",	SDLK_WORLD_77		}, /*	= 237,	*/
    {	"SDLK_WORLD_78",	SDLK_WORLD_78		}, /*	= 238,	*/
    {	"SDLK_WORLD_79",	SDLK_WORLD_79		}, /*	= 239,	*/
    {	"SDLK_WORLD_80",	SDLK_WORLD_80		}, /*	= 240,	*/
    {	"SDLK_WORLD_81",	SDLK_WORLD_81		}, /*	= 241,	*/
    {	"SDLK_WORLD_82",	SDLK_WORLD_82		}, /*	= 242,	*/
    {	"SDLK_WORLD_83",	SDLK_WORLD_83		}, /*	= 243,	*/
    {	"SDLK_WORLD_84",	SDLK_WORLD_84		}, /*	= 244,	*/
    {	"SDLK_WORLD_85",	SDLK_WORLD_85		}, /*	= 245,	*/
    {	"SDLK_WORLD_86",	SDLK_WORLD_86		}, /*	= 246,	*/
    {	"SDLK_WORLD_87",	SDLK_WORLD_87		}, /*	= 247,	*/
    {	"SDLK_WORLD_88",	SDLK_WORLD_88		}, /*	= 248,	*/
    {	"SDLK_WORLD_89",	SDLK_WORLD_89		}, /*	= 249,	*/
    {	"SDLK_WORLD_90",	SDLK_WORLD_90		}, /*	= 250,	*/
    {	"SDLK_WORLD_91",	SDLK_WORLD_91		}, /*	= 251,	*/
    {	"SDLK_WORLD_92",	SDLK_WORLD_92		}, /*	= 252,	*/
    {	"SDLK_WORLD_93",	SDLK_WORLD_93		}, /*	= 253,	*/
    {	"SDLK_WORLD_94",	SDLK_WORLD_94		}, /*	= 254,	*/
    {	"SDLK_WORLD_95",	SDLK_WORLD_95		}, /*	= 255,	*/
    {	"SDLK_KP0",		SDLK_KP0		}, /*	= 256,	*/
    {	"SDLK_KP1",		SDLK_KP1		}, /*	= 257,	*/
    {	"SDLK_KP2",		SDLK_KP2		}, /*	= 258,	*/
    {	"SDLK_KP3",		SDLK_KP3		}, /*	= 259,	*/
    {	"SDLK_KP4",		SDLK_KP4		}, /*	= 260,	*/
    {	"SDLK_KP5",		SDLK_KP5		}, /*	= 261,	*/
    {	"SDLK_KP6",		SDLK_KP6		}, /*	= 262,	*/
    {	"SDLK_KP7",		SDLK_KP7		}, /*	= 263,	*/
    {	"SDLK_KP8",		SDLK_KP8		}, /*	= 264,	*/
    {	"SDLK_KP9",		SDLK_KP9		}, /*	= 265,	*/
    {	"SDLK_KP_PERIOD",	SDLK_KP_PERIOD		}, /*	= 266,	*/
    {	"SDLK_KP_DIVIDE",	SDLK_KP_DIVIDE		}, /*	= 267,	*/
    {	"SDLK_KP_MULTIPLY",	SDLK_KP_MULTIPLY	}, /*	= 268,	*/
    {	"SDLK_KP_MINUS",	SDLK_KP_MINUS		}, /*	= 269,	*/
    {	"SDLK_KP_PLUS",		SDLK_KP_PLUS		}, /*	= 270,	*/
    {	"SDLK_KP_ENTER",	SDLK_KP_ENTER		}, /*	= 271,	*/
    {	"SDLK_KP_EQUALS",	SDLK_KP_EQUALS		}, /*	= 272,	*/
    {	"SDLK_UP",		SDLK_UP			}, /*	= 273,	*/
    {	"SDLK_DOWN",		SDLK_DOWN		}, /*	= 274,	*/
    {	"SDLK_RIGHT",		SDLK_RIGHT		}, /*	= 275,	*/
    {	"SDLK_LEFT",		SDLK_LEFT		}, /*	= 276,	*/
    {	"SDLK_INSERT",		SDLK_INSERT		}, /*	= 277,	*/
    {	"SDLK_HOME",		SDLK_HOME		}, /*	= 278,	*/
    {	"SDLK_END",		SDLK_END		}, /*	= 279,	*/
    {	"SDLK_PAGEUP",		SDLK_PAGEUP		}, /*	= 280,	*/
    {	"SDLK_PAGEDOWN",	SDLK_PAGEDOWN		}, /*	= 281,	*/
    {	"SDLK_F1",		SDLK_F1			}, /*	= 282,	*/
    {	"SDLK_F2",		SDLK_F2			}, /*	= 283,	*/
    {	"SDLK_F3",		SDLK_F3			}, /*	= 284,	*/
    {	"SDLK_F4",		SDLK_F4			}, /*	= 285,	*/
    {	"SDLK_F5",		SDLK_F5			}, /*	= 286,	*/
    {	"SDLK_F6",		SDLK_F6			}, /*	= 287,	*/
    {	"SDLK_F7",		SDLK_F7			}, /*	= 288,	*/
    {	"SDLK_F8",		SDLK_F8			}, /*	= 289,	*/
    {	"SDLK_F9",		SDLK_F9			}, /*	= 290,	*/
    {	"SDLK_F10",		SDLK_F10		}, /*	= 291,	*/
    {	"SDLK_F11",		SDLK_F11		}, /*	= 292,	*/
    {	"SDLK_F12",		SDLK_F12		}, /*	= 293,	*/
    {	"SDLK_F13",		SDLK_F13		}, /*	= 294,	*/
    {	"SDLK_F14",		SDLK_F14		}, /*	= 295,	*/
    {	"SDLK_F15",		SDLK_F15		}, /*	= 296,	*/
    {	"SDLK_NUMLOCK",		SDLK_NUMLOCK		}, /*	= 300,	*/
    {	"SDLK_CAPSLOCK",	SDLK_CAPSLOCK		}, /*	= 301,	*/
    {	"SDLK_SCROLLOCK",	SDLK_SCROLLOCK		}, /*	= 302,	*/
    {	"SDLK_RSHIFT",		SDLK_RSHIFT		}, /*	= 303,	*/
    {	"SDLK_LSHIFT",		SDLK_LSHIFT		}, /*	= 304,	*/
    {	"SDLK_RCTRL",		SDLK_RCTRL		}, /*	= 305,	*/
    {	"SDLK_LCTRL",		SDLK_LCTRL		}, /*	= 306,	*/
    {	"SDLK_RALT",		SDLK_RALT		}, /*	= 307,	*/
    {	"SDLK_LALT",		SDLK_LALT		}, /*	= 308,	*/
    {	"SDLK_RMETA",		SDLK_RMETA		}, /*	= 309,	*/
    {	"SDLK_LMETA",		SDLK_LMETA		}, /*	= 310,	*/
    {	"SDLK_LSUPER",		SDLK_LSUPER		}, /*	= 311,	*/
    {	"SDLK_RSUPER",		SDLK_RSUPER		}, /*	= 312,	*/
    {	"SDLK_MODE",		SDLK_MODE		}, /*	= 313,	*/
    {	"SDLK_COMPOSE",		SDLK_COMPOSE		}, /*	= 314,	*/
    {	"SDLK_HELP",		SDLK_HELP		}, /*	= 315,	*/
    {	"SDLK_PRINT",		SDLK_PRINT		}, /*	= 316,	*/
    {	"SDLK_SYSREQ",		SDLK_SYSREQ		}, /*	= 317,	*/
    {	"SDLK_BREAK",		SDLK_BREAK		}, /*	= 318,	*/
    {	"SDLK_MENU",		SDLK_MENU		}, /*	= 319,	*/
    {	"SDLK_POWER",		SDLK_POWER		}, /*	= 320,	*/
    {	"SDLK_EURO",		SDLK_EURO		}, /*	= 321,	*/
    {	"SDLK_UNDO",		SDLK_UNDO		}, /*	= 322,	*/
};
/* �ǥХå��� */
static	const char *debug_sdlkeysym(int code)
{
    int i;
    for (i=0; i<COUNTOF(sdlkeysym_list); i++) {
	if (code == sdlkeysym_list[i].val)
	    return sdlkeysym_list[i].name;
    }
    return "invalid";
}

/* ��������ե�����Ρ����̥���������å����륳����Хå��ؿ� */

static const char *identify_callback(const char *parm1,
				     const char *parm2,
				     const char *parm3)
{
    if (my_strcmp(parm1, "[SDL]") == 0) {
	if (parm2 == NULL ||
	    my_strcmp(parm2, video_driver) == 0) {
	    return NULL;				/* ͭ�� */
	}
    }

    return "";						/* ̵�� */
}

/* ��������ե�����Ρ������������륳����Хå��ؿ� */

static const char *setting_callback(int type,
				    int code,
				    int key88,
				    int numlock_key88)
{
    static int binding_cnt = 0;

    if (type == KEYCODE_SCAN) {
	if (code >= COUNTOF(scancode2key88)) {
	    return "scancode too large";	/* ̵�� */
	}
	scancode2key88[ code ] = key88;
    } else {
	keysym2key88[ code ]   = key88;
    }

    if (numlock_key88 >= 0) {
	if (binding_cnt >= COUNTOF(binding)) {
	    return "too many NumLock-code";	/* ̵�� */
	}
	binding[ binding_cnt ].type      = type;
	binding[ binding_cnt ].code      = code;
	binding[ binding_cnt ].new_key88 = numlock_key88;
	binding_cnt ++;
    }

    return NULL;				/* ͭ�� */
}

/* ��������ե�����ν����ؿ� */

static	int	analyze_keyconf_file(void)
{
    return
	config_read_keyconf_file(file_keyboard,		  /* ��������ե�����*/
				 identify_callback,	  /* ���̥����� �ؿ� */
				 sdlkeysym_list,	  /* �Ѵ��ơ��֥�    */
				 COUNTOF(sdlkeysym_list), /* �ơ��֥륵����  */
				 TRUE,			  /* �羮ʸ��̵��    */
				 setting_callback);	  /* ����� �ؿ�     */
}



/******************************************************************************
 * FPS
 *****************************************************************************/

/* test */

#define	FPS_INTRVAL		(1000)		/* 1000ms���ɽ������ */
static	Uint32	display_fps_callback(Uint32 interval, void *dummy);

static	int	display_fps_init(void)
{
    if (show_fps == FALSE) return TRUE;

    if (! SDL_WasInit(SDL_INIT_TIMER)) {
	if (SDL_InitSubSystem(SDL_INIT_TIMER)) {
	    return FALSE;
	}
    }

    SDL_AddTimer(FPS_INTRVAL, display_fps_callback, NULL);
    return TRUE;
}

static	Uint32	display_fps_callback(Uint32 interval, void *dummy)
{
#if 0

    /* ������Хå��ؿ����������饦����ɥ������ȥ���ѹ�����Τϴ��� ?
       �֥�����Хå��ؿ���ǤϤɤ�ʴؿ���ƤӽФ��٤��Ǥʤ��פȤʤäƤ��� */

    display_fps();

#else

    /* SDL_PushEvent �����ϸƤӽФ��Ƥ�����ȤʤäƤ���Τǡ�
       �桼�����٥�Ȥǽ������Ƥߤ褦 */

    SDL_Event user_event;

    user_event.type = SDL_USEREVENT;
    user_event.user.code  = 1;
    user_event.user.data1 = NULL;
    user_event.user.data2 = NULL;
    SDL_PushEvent(&user_event);		/* ���顼��̵�� */
#endif

    return FPS_INTRVAL;
}

static	void	display_fps(void)
{
    static int prev_drawn_count;
    static int prev_vsync_count;
    int now_drawn_count;
    int now_vsync_count;

    if (show_fps == FALSE) return;

    now_drawn_count = quasi88_info_draw_count();
    now_vsync_count = quasi88_info_vsync_count();

    if (quasi88_is_exec()) {
	char buf[32];

	sprintf(buf, "FPS: %3d (VSYNC %3d)",
		now_drawn_count - prev_drawn_count,
		now_vsync_count - prev_vsync_count);

	SDL_WM_SetCaption(buf, buf);
    }

    prev_drawn_count = now_drawn_count;
    prev_vsync_count = now_vsync_count;
}
