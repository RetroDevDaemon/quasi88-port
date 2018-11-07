#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "keyboard.h"
#include "romaji.h"
#include "event.h"

#include "soundbd.h"	/* sound_reg[]			*/
#include "graph.h"	/* set_key_and_mouse()		*/
#include "pc88cpu.h"	/* z80main_cpu			*/
#include "intr.h"	/* state_of_cpu			*/

#include "snddrv.h"	/* xmame_XXX			*/
#include "wait.h"	/* wait_rate			*/
#include "pc88main.h"	/* boot_clock_4mhz		*/

#include "drive.h"

#include "emu.h"
#include "status.h"
#include "pause.h"
#include "menu.h"
#include "screen.h"
#include "snapshot.h"

#include "suspend.h"

/******************************************************************************
 *
 *****************************************************************************/

/*
 *	�������� (���ơ��ȥ���������?)
 */

int		mouse_x;		/* ���ߤ� �ޥ��� x��ɸ		*/
int		mouse_y;		/* ���ߤ� �ޥ��� y��ɸ		*/

static	int	mouse_dx;		/* �ޥ��� x������ư��		*/
static	int	mouse_dy;		/* �ޥ��� y������ư��		*/

static	int	mouse_sx;		/* ���ꥢ��ޥ��� x������ư��	*/
static	int	mouse_sy;		/* ���ꥢ��ޥ��� y������ư��	*/
static	int	mouse_sb;		/* ���ꥢ��ޥ��� �ܥ���	*/


unsigned char	key_scan[ 0x10 ];	/* IN(00h)��(0Eh) �����������	*/
					/* key_scan[0]��[14] ����	*/
					/* I/O�ݡ��� 00H��0EH ��������	*/
					/* key_scan[15] �ϡ����祤	*/
					/* ���ƥ��å��ΰ������˻��� */

static	int	key_func[ KEY88_END ];	/* ���������إ�����,��ǽ����	*/


/*
 *	PC88���� / ��������
 */

static	int	jop1_step;	/* ����I/O�ݡ��ȤΥ꡼�ɥ��ƥå�	*/
static	int	jop1_dx;	/* ����I/O�ݡ��Ȥ��� (�ޥ��� x�����Ѱ�)	*/
static	int	jop1_dy;	/* ����I/O�ݡ��Ȥ��� (�ޥ��� y�����Ѱ�)	*/
static	int	jop1_time;	/* ����I/O�ݡ��ȤΥ��ȥ��ֽ���������	*/

	int	romaji_input_mode = FALSE;	/* ��:���޻�������	*/



/*
  mouse_x, mouse_dx, jop1_dx �δط�
	�ޥ�������ư������
		mouse_x �ˤ������к�ɸ�򥻥åȤ��롣
		�����ɸ�Ȥ��Ѱ� mouse_dx �˥��åȤ��롣
	����I/O�ݡ��Ȥ���ޥ�����ɸ��꡼�ɤ�������
		mouse_dx �򥯥�åԥ󥰤����ͤ� jop1_dx �˥��åȤ��롣
*/



/*
 *	����
 */

int	mouse_mode	= 0;		/* �ޥ��������祤���ƥå�����	*/

int	mouse_sensitivity = 100;	/* �ޥ�������			*/
int	mouse_swap_button = FALSE;	/* �ޥ����ܥ���������ؤ���  	*/


int	mouse_key_mode	= 0;		/* �ޥ������Ϥ򥭡���ȿ��	*/
int	mouse_key_assign[6];		/*     0:�ʤ� 1:�ƥ󥭡� 2:Ǥ�� */
static const int mouse_key_assign_tenkey[6] =
{
  KEY88_KP_8, KEY88_KP_2, KEY88_KP_4, KEY88_KP_6,
  KEY88_x,    KEY88_z,
};


int	joy_key_mode	= 0;		/* ���祤���Ϥ򥭡���ȿ��	*/
int	joy_key_assign[12];		/*     0:�ʤ� 1:�ƥ󥭡� 2:Ǥ�� */
static const int joy_key_assign_tenkey[12] =
{
  KEY88_KP_8, KEY88_KP_2, KEY88_KP_4, KEY88_KP_6,
  KEY88_x,    KEY88_z,    0, 0, 0, 0, 0, 0,
};
int	joy_swap_button   = FALSE;	/* �ܥ����AB�������ؤ���  	*/


int	joy2_key_mode   = 0;		/* ���祤�����Ϥ򥭡���ȿ��	*/
int	joy2_key_assign[12];		/*     0:�ʤ� 1:�ƥ󥭡� 2:Ǥ�� */
int	joy2_swap_button   = FALSE;	/* �ܥ����AB�������ؤ���  	*/



int	cursor_key_mode = 0;		/* �������륭�����̥�����ȿ��	*/
int	cursor_key_assign[4];		/*     0:�ʤ� 1:�ƥ󥭡� 2:Ǥ�� */
static const int cursor_key_assign_tenkey[4] =
{
  KEY88_KP_8, KEY88_KP_2, KEY88_KP_4, KEY88_KP_6,
};		/* Cursor KEY -> 10 KEY , original by funa. (thanks!) */
		/* Cursor Key -> Ǥ�դΥ��� , original by floi. (thanks!) */



int	tenkey_emu      = FALSE;	/* ��:����������ƥ󥭡���	*/
int	numlock_emu     = FALSE;	/* ��:���եȥ�����NumLock��Ԥ�	*/




int	function_f[ 1 + 20 ] =		/* �ե��󥯥���󥭡��ε�ǽ     */
{
  FN_FUNC,    /* [0] �ϥ��ߡ� */
  FN_FUNC,    FN_FUNC,    FN_FUNC,    FN_FUNC,    FN_FUNC,	/* f1 ��f5  */
  FN_FUNC,    FN_FUNC,    FN_FUNC,    FN_FUNC,    FN_FUNC,	/* f6 ��f10 */
  FN_STATUS,  FN_MENU,    FN_FUNC,    FN_FUNC,    FN_FUNC,	/* f11��f15 */
  FN_FUNC,    FN_FUNC,    FN_FUNC,    FN_FUNC,    FN_FUNC,	/* f16��f20 */
};


int	fn_max_speed = 1600;
double	fn_max_clock = CONST_4MHZ_CLOCK*16;
int	fn_max_boost = 16;


int	romaji_type = 0;		/* ���޻��Ѵ��Υ�����		*/





/*
 *	������� ��Ͽ������
 */

char	*file_rec	= NULL;		/* �������ϵ�Ͽ�Υե�����̾ */
char	*file_pb	= NULL;		/* �������Ϻ����Υե�����̾ */

static	OSD_FILE *fp_rec;
static	OSD_FILE *fp_pb;

static struct {				/* �������ϵ�Ͽ��¤��		*/
  Uchar	key[16];			/*	I/O 00H��0FH 		*/
   char	dx_h;				/*	�ޥ��� dx ���		*/
  Uchar	dx_l;				/*	�ޥ��� dx ����		*/
   char	dy_h;				/*	�ޥ��� dy ���		*/
  Uchar	dy_l;				/*	�ޥ��� dy ����		*/
   char	image[2];			/*	���᡼��No -1��,0Ʊ,1��	*/
   char resv[2];
} key_record;				/* 24 bytes			*/





/*---------------------------------------------------------------------------
 *	���������� �� I/O �ݡ��Ȥ��б�
 *---------------------------------------------------------------------------*/

#define	Port0	0x00
#define	Port1	0x01
#define	Port2	0x02
#define	Port3	0x03
#define	Port4	0x04
#define	Port5	0x05
#define	Port6	0x06
#define	Port7	0x07
#define	Port8	0x08
#define	Port9	0x09
#define	PortA	0x0a
#define	PortB	0x0b
#define	PortC	0x0c
#define	PortD	0x0d
#define	PortE	0x0e
#define	PortX	0x0f

#define	Bit0	0x01
#define	Bit1	0x02
#define	Bit2	0x04
#define	Bit3	0x08
#define	Bit4	0x10
#define	Bit5	0x20
#define	Bit6	0x40
#define	Bit7	0x80

#define	PadA	Bit4
#define	PadB	Bit5

#define	PadU	Bit0
#define	PadD	Bit1
#define	PadL	Bit2
#define	PadR	Bit3


enum {
  /* �������������̾��� */

  KEY88_EXT_F6		= KEY88_END + 0,
  KEY88_EXT_F7		= KEY88_END + 1,
  KEY88_EXT_F8		= KEY88_END + 2,
  KEY88_EXT_F9		= KEY88_END + 3,
  KEY88_EXT_F10		= KEY88_END + 4,
  KEY88_EXT_BS		= KEY88_END + 5,
  KEY88_EXT_INS		= KEY88_END + 6,
  KEY88_EXT_DEL		= KEY88_END + 7,
  KEY88_EXT_HENKAN	= KEY88_END + 8,
  KEY88_EXT_KETTEI	= KEY88_END + 9,
  KEY88_EXT_PC		= KEY88_END + 10,
  KEY88_EXT_ZENKAKU	= KEY88_END + 11,
  KEY88_EXT_RETURNL	= KEY88_END + 12,
  KEY88_EXT_RETURNR	= KEY88_END + 13,
  KEY88_EXT_SHIFTL	= KEY88_END + 14,
  KEY88_EXT_SHIFTR	= KEY88_END + 15,

  KEY88_EXT_END		= KEY88_END + 16
};


typedef struct {
    unsigned char	port;
    unsigned char	mask;
} T_KEYPORT;

static const T_KEYPORT keyport[ KEY88_EXT_END ] =
{
  { 0,0 },			/*	  KEY88_INVALID		= 0,	*/

  { 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },

  { Port9, Bit6 },		/*	  KEY88_SPACE		= 32,	*/
  { Port6, Bit1 },		/*	  KEY88_EXCLAM		= 33,	*/
  { Port6, Bit2 },		/*	  KEY88_QUOTEDBL	= 34,	*/
  { Port6, Bit3 },		/*	  KEY88_NUMBERSIGN	= 35,	*/
  { Port6, Bit4 },		/*	  KEY88_DOLLAR		= 36,	*/
  { Port6, Bit5 },		/*	  KEY88_PERCENT		= 37,	*/
  { Port6, Bit6 },		/*	  KEY88_AMPERSAND	= 38,	*/
  { Port6, Bit7 },		/*	  KEY88_APOSTROPHE	= 39,	*/
  { Port7, Bit0 },		/*	  KEY88_PARENLEFT	= 40,	*/
  { Port7, Bit1 },		/*	  KEY88_PARENRIGHT	= 41,	*/
  { Port7, Bit2 },		/*	  KEY88_ASTERISK	= 42,	*/
  { Port7, Bit3 },		/*	  KEY88_PLUS		= 43,	*/
  { Port7, Bit4 },		/*	  KEY88_COMMA		= 44,	*/
  { Port5, Bit7 },		/*	  KEY88_MINUS		= 45,	*/
  { Port7, Bit5 },		/*	  KEY88_PERIOD		= 46,	*/
  { Port7, Bit6 },		/*	  KEY88_SLASH		= 47,	*/
  { Port6, Bit0 },		/*	  KEY88_0		= 48,	*/
  { Port6, Bit1 },		/*	  KEY88_1		= 49,	*/
  { Port6, Bit2 },		/*	  KEY88_2		= 50,	*/
  { Port6, Bit3 },		/*	  KEY88_3		= 51,	*/
  { Port6, Bit4 },		/*	  KEY88_4		= 52,	*/
  { Port6, Bit5 },		/*	  KEY88_5		= 53,	*/
  { Port6, Bit6 },		/*	  KEY88_6		= 54,	*/
  { Port6, Bit7 },		/*	  KEY88_7		= 55,	*/
  { Port7, Bit0 },		/*	  KEY88_8		= 56,	*/
  { Port7, Bit1 },		/*	  KEY88_9		= 57,	*/
  { Port7, Bit2 },		/*	  KEY88_COLON		= 58,	*/
  { Port7, Bit3 },		/*	  KEY88_SEMICOLON	= 59,	*/
  { Port7, Bit4 },		/*	  KEY88_LESS		= 60,	*/
  { Port5, Bit7 },		/*	  KEY88_EQUAL		= 61,	*/
  { Port7, Bit5 },		/*	  KEY88_GREATER		= 62,	*/
  { Port7, Bit6 },		/*	  KEY88_QUESTION	= 63,	*/
  { Port2, Bit0 },		/*	  KEY88_AT		= 64,	*/
  { Port2, Bit1 },		/*	  KEY88_A		= 65,	*/
  { Port2, Bit2 },		/*	  KEY88_B		= 66,	*/
  { Port2, Bit3 },		/*	  KEY88_C		= 67,	*/
  { Port2, Bit4 },		/*	  KEY88_D		= 68,	*/
  { Port2, Bit5 },		/*	  KEY88_E		= 69,	*/
  { Port2, Bit6 },		/*	  KEY88_F		= 70,	*/
  { Port2, Bit7 },		/*	  KEY88_G		= 71,	*/
  { Port3, Bit0 },		/*	  KEY88_H		= 72,	*/
  { Port3, Bit1 },		/*	  KEY88_I		= 73,	*/
  { Port3, Bit2 },		/*	  KEY88_J		= 74,	*/
  { Port3, Bit3 },		/*	  KEY88_K		= 75,	*/
  { Port3, Bit4 },		/*	  KEY88_L		= 76,	*/
  { Port3, Bit5 },		/*	  KEY88_M		= 77,	*/
  { Port3, Bit6 },		/*	  KEY88_N		= 78,	*/
  { Port3, Bit7 },		/*	  KEY88_O		= 79,	*/
  { Port4, Bit0 },		/*	  KEY88_P		= 80,	*/
  { Port4, Bit1 },		/*	  KEY88_Q		= 81,	*/
  { Port4, Bit2 },		/*	  KEY88_R		= 82,	*/
  { Port4, Bit3 },		/*	  KEY88_S		= 83,	*/
  { Port4, Bit4 },		/*	  KEY88_T		= 84,	*/
  { Port4, Bit5 },		/*	  KEY88_U		= 85,	*/
  { Port4, Bit6 },		/*	  KEY88_V		= 86,	*/
  { Port4, Bit7 },		/*	  KEY88_W		= 87,	*/
  { Port5, Bit0 },		/*	  KEY88_X		= 88,	*/
  { Port5, Bit1 },		/*	  KEY88_Y		= 89,	*/
  { Port5, Bit2 },		/*	  KEY88_Z		= 90,	*/
  { Port5, Bit3 },		/*	  KEY88_BRACKETLEFT	= 91,	*/
  { Port5, Bit4 },		/*	  KEY88_YEN		= 92,	*/
  { Port5, Bit5 },		/*	  KEY88_BRACKETRIGHT	= 93,	*/
  { Port5, Bit6 },		/*	  KEY88_CARET		= 94,	*/
  { Port7, Bit7 },		/*	  KEY88_UNDERSCORE	= 95,	*/
  { Port2, Bit0 },		/*	  KEY88_BACKQUOTE	= 96,	*/
  { Port2, Bit1 },		/*	  KEY88_a		= 97,	*/
  { Port2, Bit2 },		/*	  KEY88_b		= 98,	*/
  { Port2, Bit3 },		/*	  KEY88_c		= 99,	*/
  { Port2, Bit4 },		/*	  KEY88_d		= 100,	*/
  { Port2, Bit5 },		/*	  KEY88_e		= 101,	*/
  { Port2, Bit6 },		/*	  KEY88_f		= 102,	*/
  { Port2, Bit7 },		/*	  KEY88_g		= 103,	*/
  { Port3, Bit0 },		/*	  KEY88_h		= 104,	*/
  { Port3, Bit1 },		/*	  KEY88_i		= 105,	*/
  { Port3, Bit2 },		/*	  KEY88_j		= 106,	*/
  { Port3, Bit3 },		/*	  KEY88_k		= 107,	*/
  { Port3, Bit4 },		/*	  KEY88_l		= 108,	*/
  { Port3, Bit5 },		/*	  KEY88_m		= 109,	*/
  { Port3, Bit6 },		/*	  KEY88_n		= 110,	*/
  { Port3, Bit7 },		/*	  KEY88_o               = 111,	*/
  { Port4, Bit0 },		/*	  KEY88_p               = 112,	*/
  { Port4, Bit1 },		/*	  KEY88_q               = 113,  */
  { Port4, Bit2 },		/*	  KEY88_r               = 114,  */
  { Port4, Bit3 },		/*	  KEY88_s               = 115,  */
  { Port4, Bit4 },		/*	  KEY88_t               = 116,  */
  { Port4, Bit5 },		/*	  KEY88_u               = 117,  */
  { Port4, Bit6 },		/*	  KEY88_v               = 118,  */
  { Port4, Bit7 },		/*	  KEY88_w               = 119,  */
  { Port5, Bit0 },		/*	  KEY88_x		= 120,	*/
  { Port5, Bit1 },		/*	  KEY88_y		= 121,	*/
  { Port5, Bit2 },		/*	  KEY88_z		= 122,	*/
  { Port5, Bit3 },		/*	  KEY88_BRACELEFT	= 123,	*/
  { Port5, Bit4 },		/*	  KEY88_BAR		= 124,	*/
  { Port5, Bit5 },		/*	  KEY88_BRACERIGHT	= 125,	*/
  { Port5, Bit6 },		/*	  KEY88_TILDE		= 126,	*/
  {     0,    0 },
  { Port0, Bit0 },		/*	  KEY88_KP_0		= 128,	*/
  { Port0, Bit1 },		/*	  KEY88_KP_1		= 129,	*/
  { Port0, Bit2 },		/*	  KEY88_KP_2		= 130,	*/
  { Port0, Bit3 },		/*	  KEY88_KP_3		= 131,	*/
  { Port0, Bit4 },		/*	  KEY88_KP_4		= 132,	*/
  { Port0, Bit5 },		/*	  KEY88_KP_5		= 133,	*/
  { Port0, Bit6 },		/*	  KEY88_KP_6		= 134,	*/
  { Port0, Bit7 },		/*	  KEY88_KP_7		= 135,	*/
  { Port1, Bit0 },		/*	  KEY88_KP_8		= 136,	*/
  { Port1, Bit1 },		/*	  KEY88_KP_9		= 137,	*/
  { Port1, Bit2 },		/*	  KEY88_KP_MULTIPLY	= 138,	*/
  { Port1, Bit3 },		/*	  KEY88_KP_ADD		= 139,	*/
  { Port1, Bit4 },		/*	  KEY88_KP_EQUAL	= 140,	*/
  { Port1, Bit5 },		/*	  KEY88_KP_COMMA	= 141,	*/
  { Port1, Bit6 },		/*	  KEY88_KP_PERIOD	= 142,	*/
  { PortA, Bit5 },		/*	  KEY88_KP_SUB		= 143,	*/
  { PortA, Bit6 },		/*	  KEY88_KP_DIVIDE	= 144,	*/

  { Port1, Bit7 },		/*	  KEY88_RETURN		= 145,	*/
  { Port8, Bit0 },		/*	  KEY88_HOME		= 146,	*/
  { Port8, Bit1 },		/*	  KEY88_UP		= 147,	*/
  { Port8, Bit2 },		/*	  KEY88_RIGHT		= 148,	*/
  { Port8, Bit3 },		/*	  KEY88_INS_DEL		= 149,	*/
  { Port8, Bit4 },		/*	  KEY88_GRAPH		= 150,	*/
  { Port8, Bit5 },		/*	  KEY88_KANA		= 151,	*/
  { Port8, Bit6 },		/*	  KEY88_SHIFT		= 152,	*/
  { Port8, Bit7 },		/*	  KEY88_CTRL		= 153,	*/
  { Port9, Bit0 },		/*	  KEY88_STOP		= 154,	*/
  { Port9, Bit6 },		/*	  KEY88_SPACE		= 155,	*/
  { Port9, Bit7 },		/*	  KEY88_ESC		= 156,	*/
  { PortA, Bit0 },		/*	  KEY88_TAB		= 157,	*/
  { PortA, Bit1 },		/*	  KEY88_DOWN		= 158,	*/
  { PortA, Bit2 },		/*	  KEY88_LEFT		= 159,	*/
  { PortA, Bit3 },		/*	  KEY88_HELP		= 160,	*/
  { PortA, Bit4 },		/*	  KEY88_COPY		= 161,	*/
  { PortA, Bit7 },		/*	  KEY88_CAPS		= 162,	*/
  { PortB, Bit0 },		/*	  KEY88_ROLLUP		= 163,	*/
  { PortB, Bit1 },		/*	  KEY88_ROLLDOWN	= 164,	*/

  { Port9, Bit1 },		/*	  KEY88_F1		= 165,	*/
  { Port9, Bit2 },		/*	  KEY88_F2		= 166,	*/
  { Port9, Bit3 },		/*	  KEY88_F3		= 167,	*/
  { Port9, Bit4 },		/*	  KEY88_F4		= 168,	*/
  { Port9, Bit5 },		/*	  KEY88_F5		= 169,	*/

  { 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },

  { Port9, Bit1 },	/*f-1*/	/*	  KEY88_F6		= 180,	*/
  { Port9, Bit2 },	/*f-2*/	/*	  KEY88_F7		= 181,	*/
  { Port9, Bit3 },	/*f-3*/	/*	  KEY88_F8		= 182,	*/
  { Port9, Bit4 },	/*f-4*/	/*	  KEY88_F9		= 183,	*/
  { Port9, Bit5 },	/*f-5*/	/*	  KEY88_F10		= 184,	*/
  { Port8, Bit3 },	/*del*/	/*	  KEY88_BS		= 185,	*/
  { Port8, Bit3 },	/*del*/	/*	  KEY88_INS		= 186,	*/
  { Port8, Bit3 },	/*del*/	/*	  KEY88_DEL		= 187,	*/
  { Port9, Bit6 },	/*spc*/	/*	  KEY88_HENKAN		= 188,	*/
  { Port9, Bit6 },	/*spc*/	/*	  KEY88_KETTEI		= 189,	*/
  {     0,    0 },		/*	  KEY88_PC		= 190,	*/
  {     0,    0 },		/*	  KEY88_ZENKAKU		= 191,	*/
  { Port1, Bit7 },	/*ret*/	/*	  KEY88_RETURNL		= 192,	*/
  { Port1, Bit7 },	/*ret*/	/*	  KEY88_RETURNR		= 193,	*/
  { Port8, Bit6 },	/*sft*/	/*	  KEY88_SHIFTL		= 194,	*/
  { Port8, Bit6 },	/*sft*/	/*	  KEY88_SHIFTR		= 195,	*/

  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },

  {     0,    0 },		/*	  KEY88_MOUSE_UP        = 208,	*/
  {     0,    0 },		/*	  KEY88_MOUSE_DOWN      = 209,	*/
  {     0,    0 },		/*	  KEY88_MOUSE_LEFT      = 210,	*/
  {     0,    0 },		/*	  KEY88_MOUSE_RIGHT     = 211,	*/
  {     0,    0 },		/*	  KEY88_MOUSE_L         = 212,	*/
  {     0,    0 },		/*	  KEY88_MOUSE_M         = 213,	*/
  {     0,    0 },		/*	  KEY88_MOUSE_R         = 214,	*/
  {     0,    0 },		/*	  KEY88_MOUSE_WUP       = 215,	*/
  {     0,    0 },		/*	  KEY88_MOUSE_WDN       = 216,	*/

  { 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },

  { PortX, PadU }, 		/*	  KEY88_PAD1_UP         = 224,	*/
  { PortX, PadD }, 		/*	  KEY88_PAD1_DOWN       = 225,	*/
  { PortX, PadL }, 		/*	  KEY88_PAD1_LEFT       = 226,	*/
  { PortX, PadR }, 		/*	  KEY88_PAD1_RIGHT      = 227,	*/
  { PortX, PadA }, 		/*	  KEY88_PAD1_A          = 228,	*/
  { PortX, PadB }, 		/*	  KEY88_PAD1_B          = 229,	*/
  {     0,    0 },		/*	  KEY88_PAD1_C          = 230,	*/
  {     0,    0 },		/*	  KEY88_PAD1_D          = 231,	*/
  {     0,    0 },		/*	  KEY88_PAD1_E          = 232,	*/
  {     0,    0 },		/*	  KEY88_PAD1_F          = 233,	*/
  {     0,    0 },		/*	  KEY88_PAD1_G          = 234,	*/
  {     0,    0 },		/*	  KEY88_PAD1_H          = 235,	*/

  {     0,    0 }, 		/*	  KEY88_PAD2_UP         = 236,	*/
  {     0,    0 }, 		/*	  KEY88_PAD2_DOWN       = 237,	*/
  {     0,    0 }, 		/*	  KEY88_PAD2_LEFT       = 238,	*/
  {     0,    0 }, 		/*	  KEY88_PAD2_RIGHT      = 239,	*/
  {     0,    0 }, 		/*	  KEY88_PAD2_A          = 240,	*/
  {     0,    0 }, 		/*	  KEY88_PAD2_B          = 241,	*/
  {     0,    0 },		/*	  KEY88_PAD2_C          = 242,	*/
  {     0,    0 },		/*	  KEY88_PAD2_D          = 243,	*/
  {     0,    0 },		/*	  KEY88_PAD2_E          = 244,	*/
  {     0,    0 },		/*	  KEY88_PAD2_F          = 245,	*/
  {     0,    0 },		/*	  KEY88_PAD2_G          = 246,	*/
  {     0,    0 },		/*	  KEY88_PAD2_H          = 247,	*/

  { 0,0 },{ 0,0 },{ 0,0 },{ 0,0 },
  { 0,0 },{ 0,0 },
  { 0,0 },			/*	  KEY88_SYS_STATUS      = 254,	*/
  { 0,0 },			/*	  KEY88_SYS_MENU        = 255,	*/

  { PortC, Bit0 },		/*	  KEY88_EXT_F6		= 256,	*/
  { PortC, Bit1 },		/*	  KEY88_EXT_F7		= 257,	*/
  { PortC, Bit2 },		/*	  KEY88_EXT_F8		= 258,	*/
  { PortC, Bit3 },		/*	  KEY88_EXT_F9		= 259,	*/
  { PortC, Bit4 },		/*	  KEY88_EXT_F10		= 260,	*/
  { PortC, Bit5 },		/*	  KEY88_EXT_BS		= 261,	*/
  { PortC, Bit6 },		/*	  KEY88_EXT_INS		= 262,	*/
  { PortC, Bit7 },		/*	  KEY88_EXT_DEL		= 263,	*/
  { PortD, Bit0 },		/*	  KEY88_EXT_HENKAN	= 264,	*/
  { PortD, Bit1 },		/*	  KEY88_EXT_KETTEI	= 265,	*/
  { PortD, Bit2 },		/*	  KEY88_EXT_PC		= 266,	*/
  { PortD, Bit3 },		/*	  KEY88_EXT_ZENKAKU	= 267,	*/
  { PortE, Bit0 },		/*	  KEY88_EXT_RETURNL	= 268,	*/
  { PortE, Bit1 },		/*	  KEY88_EXT_RETURNR	= 269,	*/
  { PortE, Bit2 },		/*	  KEY88_EXT_SHIFTL	= 270,	*/
  { PortE, Bit3 },		/*	  KEY88_EXT_SHIFTR	= 271,	*/
};


/*---------------------------------------------------------------------------
 *
 *---------------------------------------------------------------------------*/

#define	IS_JOY_STATUS()		key_scan[ PortX ]


#define	KEY88_PRESS(code)	\
	key_scan[ keyport[(code)].port ] &= ~keyport[(code)].mask

#define	KEY88_RELEASE(code)	\
	key_scan[ keyport[(code)].port ] |=  keyport[(code)].mask

#define	KEY88_TOGGLE(code)	\
	key_scan[ keyport[(code)].port ] ^=  keyport[(code)].mask

#define	IS_KEY88_PRESS(code)	\
	(~key_scan[ keyport[(code)].port ] & keyport[(code)].mask)

#define	IS_KEY88_RELEASE(code)	\
	( key_scan[ keyport[(code)].port ] & keyport[(code)].mask)


#define	IS_KEY88_PRINTTABLE(c)	(32 <= (c) && (c) <= 144)
#define	IS_KEY88_FUNCTION(c)	(KEY88_F1 <= (c) && (c) <= KEY88_F20)
#define	IS_KEY88_LATTERTYPE(c)	(KEY88_F6 <= (c) && (c) <= KEY88_SHIFTR)





/*---------------------------------------------------------------------------
 * �����ΥХ���ǥ����ѹ� (���������ɡ���ǽ)
 *---------------------------------------------------------------------------*/
static	void	clr_key_function(void)
{
    int i;
    for (i=0; i<COUNTOF(key_func); i++) { key_func[i] = 0; }

    /* ����2�ĤΥ����ϡ���ǽ���� */
    key_func[ KEY88_SYS_STATUS ] = FN_STATUS;
    key_func[ KEY88_SYS_MENU   ] = FN_MENU;
}


static	void	set_key_function(int keycode, int func_no)
{
    key_func[ keycode ] = func_no;

    /* ����2�ĤΥ����ϡ���ǽ���� */
    key_func[ KEY88_SYS_STATUS ] = FN_STATUS;
    key_func[ KEY88_SYS_MENU   ] = FN_MENU;
}


static	void	swap_key_function(int keycode1, int keycode2)
{
    int              tmp = key_func[ keycode1 ];
    key_func[ keycode1 ] = key_func[ keycode2 ];
    key_func[ keycode2 ] = tmp;
}


/*---------------------------------------------------------------------------
 * �ޥ����κ�ɸ�򡢲��̥������˹�碌����������
 *---------------------------------------------------------------------------*/
static void mouse_movement_adjust(int *x, int *y)
{
    (*x) -= SCREEN_DX;
    (*y) -= SCREEN_DY;

    switch (quasi88_cfg_now_size()) {
    case SCREEN_SIZE_HALF:	(*x) *= 2;	(*y) *= 2;	break;
    case SCREEN_SIZE_FULL:					break;
#ifdef	SUPPORT_DOUBLE
    case SCREEN_SIZE_DOUBLE:	(*x) /= 2;	(*y) /= 2;	break;
#endif
    }
}


/****************************************************************************
 * �����ΥХ���ǥ����ѹ�
 *	���ץ����䡢��˥塼�Ǥ�����˴�Ť��������Х���ɤ��ѹ�����/�᤹��
 *****************************************************************************/
void	keyboard_switch(void)
{
    int swap;
    const int *p;

    if (quasi88_is_exec()) {	/* ���ߥ���ϡ������Х���ǥ����ѹ� */

	clr_key_function();

	if (numlock_emu) {
	    event_numlock_on();		/* Num lock ��ͭ���� (�����ƥ��¸) */
	}

	if (tenkey_emu) {		/* ����������ƥ󥭡������� */
	    set_key_function( KEY88_1, KEY88_KP_1 );
	    set_key_function( KEY88_2, KEY88_KP_2 );
	    set_key_function( KEY88_3, KEY88_KP_3 );
	    set_key_function( KEY88_4, KEY88_KP_4 );
	    set_key_function( KEY88_5, KEY88_KP_5 );
	    set_key_function( KEY88_6, KEY88_KP_6 );
	    set_key_function( KEY88_7, KEY88_KP_7 );
	    set_key_function( KEY88_8, KEY88_KP_8 );
	    set_key_function( KEY88_9, KEY88_KP_9 );
	    set_key_function( KEY88_0, KEY88_KP_0 );
	}

	if (cursor_key_mode) {		/* ������������ϳ�������ѹ� */
	    if (cursor_key_mode == 1) p = cursor_key_assign_tenkey;
	    else                      p = cursor_key_assign;

	    set_key_function( KEY88_UP,    *p );	p ++;
	    set_key_function( KEY88_DOWN,  *p );	p ++;
	    set_key_function( KEY88_LEFT,  *p );	p ++;
	    set_key_function( KEY88_RIGHT, *p );
	}

	/*
	  �����ƥफ��Υޥ������ϡ�����ӥ��祤���ƥ��å����Ϥ���
	  QUASI88 �Υݡ��ȤˤɤΤ褦��ȿ�Ǥ���뤫�ΰ�����

			 |  �ޥ�����ư    �ޥ����ܥ��� | ���祤���ƥ��å�
	  ---------------+--------------+--------------+-----------------
	  MOUSE_NONE	 | �ʤ�         | �ʤ�         | �ʤ�
	  ---------------+--------------+--------------+-----------------
	  MOUSE_JOYMOUSE | ������ȿ��   | ������ȿ��   | �ʤ�
	  ---------------+--------------+--------------+-----------------
	  MOUSE_MOUSE	 | ��ư�̤�ȿ�� | ������ȿ��   | �ʤ�
	  ---------------+--------------+--------------+-----------------
	  MOUSE_JOYSTICK | �ʤ�         | �ʤ�         | ������ȿ��

	  �֤ʤ��פ���ϡ�QUASI88�ˤ�ȿ�Ǥ���ʤ��Τǡ���ͳ�˥���������Ʋ�ǽ��
	*/
	swap = FALSE;

	switch (mouse_mode) {		/* �ޥ��������ϳ�������ѹ� */
	case MOUSE_NONE:
	case MOUSE_JOYSTICK:
	    if (mouse_key_mode) {
		if (mouse_key_mode == 1) p = mouse_key_assign_tenkey;
		else                     p = mouse_key_assign;

		set_key_function( KEY88_MOUSE_UP,    *p );	p ++;
		set_key_function( KEY88_MOUSE_DOWN,  *p );	p ++;
		set_key_function( KEY88_MOUSE_LEFT,  *p );	p ++;
		set_key_function( KEY88_MOUSE_RIGHT, *p );	p ++;
		set_key_function( KEY88_MOUSE_L,     *p );	p ++;
		set_key_function( KEY88_MOUSE_R,     *p );
		if (mouse_key_mode == 1) {
		    swap = mouse_swap_button;
		}
	    }
	    break;

	case MOUSE_JOYMOUSE:
	    set_key_function( KEY88_MOUSE_UP,    KEY88_PAD1_UP    );
	    set_key_function( KEY88_MOUSE_DOWN,  KEY88_PAD1_DOWN  );
	    set_key_function( KEY88_MOUSE_LEFT,  KEY88_PAD1_LEFT  );
	    set_key_function( KEY88_MOUSE_RIGHT, KEY88_PAD1_RIGHT );
	    set_key_function( KEY88_MOUSE_L,     KEY88_PAD1_A     );
	    set_key_function( KEY88_MOUSE_R,     KEY88_PAD1_B     );
	    swap = mouse_swap_button;
	    break;

	case MOUSE_MOUSE:
	    set_key_function( KEY88_MOUSE_L,     KEY88_PAD1_A     );
	    set_key_function( KEY88_MOUSE_R,     KEY88_PAD1_B     );
	    swap = mouse_swap_button;
	    break;
	}
	if (swap) {
	    swap_key_function(KEY88_MOUSE_L, KEY88_MOUSE_R);
	}


	swap = FALSE;

	switch (mouse_mode) {		/* ���祤���ƥ��å����ϳ�������ѹ� */
	case MOUSE_NONE:
	case MOUSE_MOUSE:
	case MOUSE_JOYMOUSE:
	    if (joy_key_mode) {
		if (joy_key_mode == 1) p = joy_key_assign_tenkey;
		else                   p = joy_key_assign;

		set_key_function( KEY88_PAD1_UP,    *p );	p ++;
		set_key_function( KEY88_PAD1_DOWN,  *p );	p ++;
		set_key_function( KEY88_PAD1_LEFT,  *p );	p ++;
		set_key_function( KEY88_PAD1_RIGHT, *p );	p ++;
		set_key_function( KEY88_PAD1_A,     *p );	p ++;
		set_key_function( KEY88_PAD1_B,     *p );	p ++;
		set_key_function( KEY88_PAD1_C,     *p );	p ++;
		set_key_function( KEY88_PAD1_D,     *p );	p ++;
		set_key_function( KEY88_PAD1_E,     *p );	p ++;
		set_key_function( KEY88_PAD1_F,     *p );	p ++;
		set_key_function( KEY88_PAD1_G,     *p );	p ++;
		set_key_function( KEY88_PAD1_H,     *p );	p ++;
		if (joy_key_mode == 1) {
		    swap = joy_swap_button;
		}
	    }
	    break;

	case MOUSE_JOYSTICK:
	    /* KEY88_PAD1_C �� KEY88_PAD1_H �ϳ�����Ƥʤ� */
	    swap = joy_swap_button;
	    break;
	}
	if (swap) {
	    swap_key_function(KEY88_PAD1_A, KEY88_PAD1_B);
	}


	if (joy2_key_mode) {	      /* ���祤���ƥ��å������ϳ�������ѹ� */
	    if (joy2_key_mode == 1) p = joy_key_assign_tenkey;
	    else                    p = joy2_key_assign;

	    set_key_function( KEY88_PAD2_UP,    *p );	p ++;
	    set_key_function( KEY88_PAD2_DOWN,  *p );	p ++;
	    set_key_function( KEY88_PAD2_LEFT,  *p );	p ++;
	    set_key_function( KEY88_PAD2_RIGHT, *p );	p ++;
	    set_key_function( KEY88_PAD2_A,     *p );	p ++;
	    set_key_function( KEY88_PAD2_B,     *p );	p ++;
	    set_key_function( KEY88_PAD2_C,     *p );	p ++;
	    set_key_function( KEY88_PAD2_D,     *p );	p ++;
	    set_key_function( KEY88_PAD2_E,     *p );	p ++;
	    set_key_function( KEY88_PAD2_F,     *p );	p ++;
	    set_key_function( KEY88_PAD2_G,     *p );	p ++;
	    set_key_function( KEY88_PAD2_H,     *p );	p ++;
	    if (joy2_key_mode == 1) {
		swap = joy2_swap_button;
	    }
	}
	if (swap) {
	    swap_key_function(KEY88_PAD2_A, KEY88_PAD2_B);
	}


					/* �ե��󥯥���󥭡��ε�ǽ������� */
	set_key_function( KEY88_F1,  function_f[  1 ] );
	set_key_function( KEY88_F2,  function_f[  2 ] );
	set_key_function( KEY88_F3,  function_f[  3 ] );
	set_key_function( KEY88_F4,  function_f[  4 ] );
	set_key_function( KEY88_F5,  function_f[  5 ] );
	set_key_function( KEY88_F6,  function_f[  6 ] );
	set_key_function( KEY88_F7,  function_f[  7 ] );
	set_key_function( KEY88_F8,  function_f[  8 ] );
	set_key_function( KEY88_F9,  function_f[  9 ] );
	set_key_function( KEY88_F10, function_f[ 10 ] );
	set_key_function( KEY88_F11, function_f[ 11 ] );
	set_key_function( KEY88_F12, function_f[ 12 ] );
	set_key_function( KEY88_F13, function_f[ 13 ] );
	set_key_function( KEY88_F14, function_f[ 14 ] );
	set_key_function( KEY88_F15, function_f[ 15 ] );
	set_key_function( KEY88_F16, function_f[ 16 ] );
	set_key_function( KEY88_F17, function_f[ 17 ] );
	set_key_function( KEY88_F18, function_f[ 18 ] );
	set_key_function( KEY88_F19, function_f[ 19 ] );
	set_key_function( KEY88_F20, function_f[ 20 ] );


    } else {			/* ��˥塼��ʤɤϡ������Х���ǥ����᤹ */

	romaji_clear();			/* ���޻��Ѵ���������� */

	event_numlock_off();		/* Num lock ��̵���� (�����ƥ��¸) */
    }


				/* �ޥ������Ϥ������ơ��ޥ������֤������ */
    event_get_mouse_pos(&mouse_x, &mouse_y);
    mouse_movement_adjust(&mouse_x, &mouse_y);

    mouse_dx = 0;
    mouse_dy = 0;

    mouse_sx = 0;
    mouse_sy = 0;
    mouse_sb = 0;
}



/****************************************************************************
 * �������ϤΥ��(�ݡ���)�����
 *****************************************************************************/
void	keyboard_reset(void)
{
    size_t i;
    for (i=0; i<sizeof(key_scan); i++) key_scan[i] = 0xff;

    romaji_init();
}



/****************************************************************************
 * �������Ϥ���(�ݡ���)��ȿ��		���ߥ�⡼�ɤΤ�
 *****************************************************************************/
static	void	record_playback(void);

void	keyboard_update(void)
{
    int status;

	/* ���޻����ϥ⡼�ɻ��ϡ����޻��Ѵ���Υ��ʤ� key_scan[] ��ȿ�� */

    if (romaji_input_mode) romaji_output();


	/* �ޥ������Ϥ� key_scan[] ��ȿ�� */

    switch (mouse_mode) {

    case MOUSE_NONE:
    case MOUSE_JOYSTICK:
	if (mouse_key_mode == 0) {	/* �ޥ��������������̵���ʤ� */
	    mouse_dx = 0;		/* �ޥ�����ư�̤�����         */
	    mouse_dy = 0;
	    break;
	}
	/* FALLTHROUGH */		/* �ޥ��������������ͭ�꤫�� */
					/* ���祤���ƥ��å��⡼�ɤʤ� */
    case MOUSE_JOYMOUSE:		/* �ޥ�����ư�̤�ݡ��Ȥ�ȿ�� */
	if (mouse_dx == 0) {
	    if      (mouse_dy == 0) status = 0;		/* ---- */
	    else if (mouse_dy >  0) status = (PadD);	/* ��   */
	    else                    status = (PadU);	/* ��   */
	} else if (mouse_dx > 0) {
	    int a = mouse_dy * 100 / mouse_dx;
	    if      (a >  241) status = (PadD);		/* ��   */
	    else if (a >   41) status = (PadD | PadR);	/* ���� */
	    else if (a >  -41) status = (       PadR);	/*   �� */
	    else if (a > -241) status = (PadU | PadR);	/* ���� */
	    else               status = (PadU);		/* ��   */
	} else {
	    int a = -mouse_dy * 100 / mouse_dx;
	    if      (a >  241) status = (PadD);		/* ��   */
	    else if (a >   41) status = (PadD | PadL);	/* ���� */
	    else if (a >  -41) status = (       PadL);	/*   �� */
	    else if (a > -241) status = (PadU | PadL);	/* ���� */
	    else               status = (PadU);		/* ��   */
	}

	quasi88_mouse( KEY88_MOUSE_UP,    (status & PadU) );
	quasi88_mouse( KEY88_MOUSE_DOWN,  (status & PadD) );
	quasi88_mouse( KEY88_MOUSE_LEFT,  (status & PadL) );
	quasi88_mouse( KEY88_MOUSE_RIGHT, (status & PadR) );

	mouse_dx = 0;		/* �ݡ���ȿ�Ǹ�ϰ�ư�̥��ꥢ */
	mouse_dy = 0;
	break;

    case MOUSE_MOUSE:		/* �ޥ���������ʤ顢         */
	break;			/* �ޥ�����ư�̤��ݻ����Ƥ��� */
    }


	/* �������(scan_key[], mouse_dx, mouse_dy) ��Ͽ������ */

    record_playback();


	/* key_scan[0x0f] (���祤���ƥ��å�) �򥵥�����������ϥݡ��Ȥ�ȿ�� */

    switch (mouse_mode) {
    case MOUSE_NONE:
	sound_reg[ 0x0e ] = 0xff;
	sound_reg[ 0x0f ] = 0xff;
	break;

    case MOUSE_MOUSE:
	sound_reg[ 0x0f ] = (IS_JOY_STATUS() >> 4) | 0xfc;
	break;

    case MOUSE_JOYMOUSE:
    case MOUSE_JOYSTICK:
	sound_reg[ 0x0e ] = (IS_JOY_STATUS()     ) | 0xf0;
	sound_reg[ 0x0f ] = (IS_JOY_STATUS() >> 4) | 0xfc;
	/*printf("%02x\n",sound_reg[ 0x0e ]&0xff);*/
	break;
    }

}



/****************************************************************************
 * ���ꥢ��ޥ�������Υ��ꥢ�������ͼ���
 *
 *	���ȥåץӥå� 1���ǡ���Ĺ 7bit���ѥ�ƥ��ʤ����ܡ��졼�� 1200bps��
 *			+--+--+--+--+--+--+--+--+
 *	1�Х�����	|��|��|��|��|Y7|Y6|X7|X6|
 *			+--+--+--+--+--+--+--+--+
 *			+--+--+--+--+--+--+--+--+
 *	2�Х�����	|��|��|X5|X4|X3|X2|X1|X0|
 *			+--+--+--+--+--+--+--+--+
 *			+--+--+--+--+--+--+--+--+
 *	3�Х�����	|��|��|Y5|Y4|Y3|Y2|Y1|Y0|
 *			+--+--+--+--+--+--+--+--+
 *
 *	�� ��            �� ���ܥ��󡢱��ܥ��� (��������1)
 *	X7X6X5X4X3X2X1X0 �� X������ư�� (����դ�)
 *	Y7Y6Y5Y4Y3Y2Y1Y0 �� X������ư�� (����դ�)
 *
 *****************************************************************************/
static int serial_mouse_x;
static int serial_mouse_y;
static int serial_mouse_step;

void	init_serial_mouse_data(void)
{
    serial_mouse_x = 0;
    serial_mouse_y = 0;
    serial_mouse_step = 0;
}
int	get_serial_mouse_data(void)
{
    int result;
    switch (serial_mouse_step) {
    case 0:
	if      (mouse_sx >  127) serial_mouse_x =  127;
	else if (mouse_sx < -127) serial_mouse_x = -127;
	else                      serial_mouse_x = mouse_sx;
	if      (mouse_sy >  127) serial_mouse_y =  127;
	else if (mouse_sy < -127) serial_mouse_y = -127;
	else                      serial_mouse_y = mouse_sy;
	mouse_sx = 0;
	mouse_sy = 0;

	result = 
	    0x40 | 
	    mouse_sb |
	    ((serial_mouse_y & 0xc0) >> 4) |
	    ((serial_mouse_x & 0xc0) >> 6);
	/*printf("%02x\n", result);*/
	break;

    case 1:
	result = (serial_mouse_x & 0x3f);
	/*printf("   %02x\n", result);*/
	break;

    default:
	result = (serial_mouse_y & 0x3f);
	/*printf("      %02x\n", result);*/
	break;
    }
    if (++serial_mouse_step >= 3) serial_mouse_step = 0;
    return result;
}



/****************************************************************************
 * �������� ��Ͽ������
 *****************************************************************************/
void	key_record_playback_init(void)
{
  int i;

  for( i=0; i<16; i++ ) key_record.key[i]     = 0xff;
  key_record.dx_h = 0;
  key_record.dx_l = 0;
  key_record.dy_h = 0;
  key_record.dy_l = 0;
  key_record.image[0] = -1;
  key_record.image[1] = -1;


  fp_pb  = NULL;
  fp_rec = NULL;

  if( file_pb && file_pb[0] ){			/* �����ѥե�����򥪡��ץ� */

    fp_pb = osd_fopen( FTYPE_KEY_PB, file_pb, "rb" );

    if( fp_pb ){
      if( verbose_proc )
	printf( "Key-Input Playback file <%s> ... OK\n", file_pb );
    }else{
      printf( "Can't open <%s>\nKey-Input PlayBack is invalid\n", file_pb );
    }
  }

  if( file_rec && file_rec[0] ){		/* ��Ͽ�ѥե�����򥪡��ץ� */

    fp_rec = osd_fopen( FTYPE_KEY_REC, file_rec, "wb" );

    if( fp_rec ){
      if( verbose_proc )
	printf( "Key-Input Record file <%s> ... OK\n", file_rec );
    }else{
      printf( "Can't open <%s>\nKey-Input Record is invalid\n", file_rec );
    }
  }
}

void	key_record_playback_exit(void)
{
  if( fp_pb ){
    osd_fclose( fp_pb );
    fp_pb = NULL;
    if( file_pb ) file_pb[0] = '\0';
  }
  if( fp_rec ){
    osd_fclose( fp_rec );
    fp_rec = NULL;
    if( file_rec ) file_rec[0] = '\0';
  }
}



/*----------------------------------------------------------------------
 * �������� ��Ͽ����������
 *----------------------------------------------------------------------*/
static	void	record_playback(void)
{
  int i, img;

  if (quasi88_is_exec() == FALSE) return;

  if( fp_rec ){
    for( i=0; i<0x10; i++ )
      key_record.key[i] = key_scan[i];

    key_record.dx_h = (mouse_dx>>8) & 0xff;
    key_record.dx_l =  mouse_dx     & 0xff;
    key_record.dy_h = (mouse_dy>>8) & 0xff;
    key_record.dy_l =  mouse_dy     & 0xff;

    for( i=0; i<2; i++ ){
      if( disk_image_exist( i ) &&
	  drive_check_empty( i ) == FALSE )
	img = disk_image_selected(i) + 1;
      else
	img = -1;
      if( key_record.image[i] != img ) key_record.image[i] = img;
      else                             key_record.image[i] = 0;
    }

    if( osd_fwrite( &key_record, sizeof(char), sizeof(key_record), fp_rec )
							== sizeof(key_record)){
      ;
    }else{
      printf( "Can't write Record file <%s>\n", file_rec );
      osd_fclose( fp_rec );
      fp_rec = NULL;
    }
  }


  if( fp_pb ){

    if( osd_fread( &key_record, sizeof(char), sizeof(key_record), fp_pb )
							== sizeof(key_record)){
      for( i=0; i<0x10; i++ )
	key_scan[i] = key_record.key[i];

      mouse_dx  = (int)key_record.dx_h << 8;
      mouse_dx |=      key_record.dx_l;
      mouse_dy  = (int)key_record.dy_h << 8;
      mouse_dy |=      key_record.dy_l;

      for( i=0; i<2; i++ ){
	if( key_record.image[i]==-1 ){
	  drive_set_empty( i );
	}else if( disk_image_exist( i ) &&
		  key_record.image[i] > 0 &&
		  key_record.image[i] <= disk_image_num( i ) ){
	  drive_unset_empty( i );
	  disk_change_image( i, key_record.image[i]-1 );
	}
      }

    }else{
      printf(" (( %s : Playback file EOF ))\n", file_pb );
      status_message( 1, STATUS_INFO_TIME, "Playback  [EOF]" );
      osd_fclose( fp_pb );
      fp_pb = NULL;
    }
  }

  /* ���ꥢ��ޥ����ϡ�ȿ�ǥ����ߥ󥰤� VSYNC �Ȥϰۤʤ�Τǡ�Ŭ�Ѥ��ʤ� */
}



/****************************************************************************
 * ���� I/O �ݡ��ȤΥ�������
 *****************************************************************************/
/* ���� I/O �ݡ��Ȥ������Ϣ����� ���ڤ��ؤ�ä����ν��� */
void	keyboard_jop1_reset(void)
{
#if 0	/* - - - - - - - - - - - - - - - - - �ޥ�����Ϣ�Υ������������  */

#if 0						/* �ޥ�����ɸ�����������ۤ�*/
  event_get_mouse_pos(&mouse_x, &mouse_y);	/* ���������⤷��ʤ����ɡ�  */
  mouse_movement_adjust(&mouse_x, &mouse_y);	/* �����ޤǤ��ʤ��Ƥ⤤�ä�  */
#endif
  mouse_dx = 0;
  mouse_dy = 0;

#endif	/* - - - - - - - - �Ǥ⡢�ºݤˤϤ���򤷤ʤ��Ƥ�³��ϤǤʤ��Ȼפ���*/
	/*			�³������ä��Τǡ�ver 0.6.2 �ʹߤǤϺ��     */

    jop1_step = 0;
    jop1_dx = 0;
    jop1_dy = 0;
}



/****************************************************************************
 * ���� I/O �ݡ��� ���ȥ��� ON/OFF
 *****************************************************************************/

/* ���ȥ��ֽ����� 720state ����˴�λ�����롣   (��1.25�ϥޡ�����)	*/
/*  (8MHz�ξ��� 1440state �ʤ�����ɤޤ����ä�)			*/
#define	JOP1_STROBE_LIMIT		((int)(720 * 1.25))

void	keyboard_jop1_strobe(void)
{
  if( mouse_mode==MOUSE_MOUSE       &&		/* �ޥ��� ͭ�� */
      (sound_reg[ 0x07 ] & 0x80)==0 ){		/* ����I/O ��������� */

    {
      int now = state_of_cpu + z80main_cpu.state0;

      /*int interval = now - jop1_time;
	if( interval < 0 ) interval += state_of_vsync;
	printf("JOP %d (%d)\n",jop1_step,interval);*/

      if( jop1_step == 2 ){
	int interval = now - jop1_time;
	if( interval < 0 ) interval += state_of_vsync;
	if( interval > JOP1_STROBE_LIMIT ) keyboard_jop1_reset();
      }

      jop1_time = now;
    }

    switch( jop1_step ){

    case 0:		/* �ǽ�Υ��ȥ���(ON)�ǡ��ޥ�����ư�̤��ͤ���ꤷ  */
			/* 2���ܰʹߤΥ��ȥ��֤ǡ����γ��ꤷ���ͤ�ž������ */
		{
		  int dx = mouse_dx;			/* x ���� �Ѱ� */
		  int dy = mouse_dy;			/* y ���� �Ѱ� */

#if 1			/* �Ѱ̤��127���ϰ���˥���åԥ󥰤��� */
		  int f = 0;

			/* x��y�Τ��� �Ѱ̤� ��127��Ķ���Ƥ�������õ����     */
		  	/* �Ȥ��Ķ���Ƥ��顢�Ѱ̤��礭���ۤ���Ķ�����Ȥ��롣*/
		  if( dx < -127 || 127 < dx ) f |= 0x01;
		  if( dy < -127 || 127 < dy ) f |= 0x02;
		  if( f==0x03 ){
		    if( ABS(dx) > ABS(dy) ) f = 0x01;
		    else                    f = 0x02;
		  }
		  if( f==0x01 ){		/* x�Ѱ̤� ��127��Ķ������� */
		    				/* x�Ѱ̤�max�ͤˤ���y������ */
		    dy = 127 * SGN(dx) * dy / dx;
		    dx = 127 * SGN(dx);
		  }
		  else if( f==0x02 ){		/* y�Ѱ̤� ��127��Ķ������� */
						/* y�Ѱ̤�max�ͤˤ���x������ */
		    dx = 127 * SGN(dy) * dx / dy;
		    dy = 127 * SGN(dy);
		  }
#endif
		  mouse_dx -= dx;
		  mouse_dy -= dy;

		  jop1_dx = dx;
		  jop1_dy = dy;
		  /*printf("%d,%d\n",jop1_dx,jop1_dy);*/
		}
		sound_reg[ 0x0e ] = ((-jop1_dx)>>4) & 0x0f;	break;
    case 1:	sound_reg[ 0x0e ] =  (-jop1_dx)     & 0x0f;	break;
    case 2:	sound_reg[ 0x0e ] = ((-jop1_dy)>>4) & 0x0f;	break;
    case 3:	sound_reg[ 0x0e ] =  (-jop1_dy)     & 0x0f;	break;
    }

  }else{
    		sound_reg[ 0x0e ] = 0xff;
  }

  jop1_step = (jop1_step + 1) & 0x03;
}



/***********************************************************************
 * ���եȥ�����NumLock
 * ���ʡʥ��޻��˥�����å�
 ************************************************************************/
void	quasi88_cfg_key_numlock(int on)
{
    if (((numlock_emu)          && (on == FALSE)) ||
	((numlock_emu == FALSE) && (on))) {

	if (numlock_emu) event_numlock_off();
	numlock_emu ^= 1;
	keyboard_switch();
    }
}
void	quasi88_cfg_key_kana(int on)
{
    if (((IS_KEY88_PRESS(KEY88_KANA))          && (on == FALSE)) ||
	((IS_KEY88_PRESS(KEY88_KANA) == FALSE) && (on))) {

	KEY88_TOGGLE(KEY88_KANA);
	romaji_input_mode = FALSE;
    }
}
void	quasi88_cfg_key_romaji(int on)
{
    if (((IS_KEY88_PRESS(KEY88_KANA))          && (on == FALSE)) ||
	((IS_KEY88_PRESS(KEY88_KANA) == FALSE) && (on))) {

	KEY88_TOGGLE(KEY88_KANA);
	if (IS_KEY88_PRESS(KEY88_KANA)) {
	    romaji_input_mode = TRUE;
	    romaji_clear();
	} else {
	    romaji_input_mode = FALSE;
	}
    }
}



/****************************************************************************
 * �ڵ����¸����ꡢ�����������˸ƤӽФ�����
 *
 *	code �ϡ����������ɤǡ� KEY88_SPACE <= code <= KEY88_SHIFTR
 *	on   �ϡ����������ʤ鿿�����������ʤ鵶
 *****************************************************************************/
static	void	do_lattertype(int code, int on);
static	int	do_func(int func, int on);

void	quasi88_key(int code, int on)
{
    if (quasi88_is_exec()) {		/*===================================*/

	if (key_func[ code ]) {			/* �ü����������ƺѤξ�� */
	    code = do_func(key_func[ code ], on);   /*	�ü쵡ǽ������¹�  */
	    if (code == 0) return;		    /*	���ͤ� ������������ */
	}

	if (romaji_input_mode && on) {		/* ���޻����ϥ⡼�ɤξ�� */
	    if (romaji_input(code) == 0) {	/*	�Ѵ�������¹�      */
		return;
	    }
	}

	if (IS_KEY88_LATTERTYPE(code)) {	/* ����������ܡ��ɤν���   */
	    do_lattertype(code, on);
	}
						/* ����������IO�ݡ��Ȥ�ȿ�� */
	if (on) KEY88_PRESS(code);
	else    KEY88_RELEASE(code);

	/*
	  if(code == KEY88_RETURNL){
	  if (on) printf("+%d\n",code);
	  else    printf("-%d\n",code);
	  }
	*/

    } else
    if (quasi88_is_menu()) {		/*===================================*/
	/*printf("%d\n",code);*/
						/* ��˥塼�Ѥ��ɤ��ؤ��� */
	switch (code) {
	case KEY88_KP_0:	code = KEY88_0;		break;
	case KEY88_KP_1:	code = KEY88_1;		break;
	case KEY88_KP_2:	code = KEY88_2;		break;
	case KEY88_KP_3:	code = KEY88_3;		break;
	case KEY88_KP_4:	code = KEY88_4;		break;
	case KEY88_KP_5:	code = KEY88_5;		break;
	case KEY88_KP_6:	code = KEY88_6;		break;
	case KEY88_KP_7:	code = KEY88_7;		break;
	case KEY88_KP_8:	code = KEY88_8;		break;
	case KEY88_KP_9:	code = KEY88_9;		break;
	case KEY88_KP_MULTIPLY:	code = KEY88_ASTERISK;	break;
	case KEY88_KP_ADD:	code = KEY88_PLUS;	break;
	case KEY88_KP_EQUAL:	code = KEY88_EQUAL;	break;
	case KEY88_KP_COMMA:	code = KEY88_COMMA;	break;
	case KEY88_KP_PERIOD:	code = KEY88_PERIOD;	break;
	case KEY88_KP_SUB:	code = KEY88_MINUS;	break;
	case KEY88_KP_DIVIDE:	code = KEY88_SLASH;	break;

	case KEY88_INS_DEL:	code = KEY88_BS;	break;
      /*case KEY88_DEL:		code = KEY88_BS;	break;	DEL�Τޤ� */
	case KEY88_KETTEI:	code = KEY88_SPACE;	break;
	case KEY88_HENKAN:	code = KEY88_SPACE;	break;
	case KEY88_RETURNL:	code = KEY88_RETURN;	break;
	case KEY88_RETURNR:	code = KEY88_RETURN;	break;
	case KEY88_SHIFTL:	code = KEY88_SHIFT;	break;
	case KEY88_SHIFTR:	code = KEY88_SHIFT;	break;
	}
	if (on) q8tk_event_key_on(code);
	else    q8tk_event_key_off(code);

    } else
    if (quasi88_is_pause()) {		/*===================================*/

	if (on) {
	    if (key_func[ code ]) {
		if (key_func[ code ] == FN_MENU) {

		    pause_event_key_on_menu();

		} else if (key_func[ code ] == FN_PAUSE) {

		    pause_event_key_on_esc();

		}

	    } else if (code == KEY88_ESC) {

		pause_event_key_on_esc();

	    }

	}
    }
}





/*----------------------------------------------------------------------
 * ����������ܡ��ɤΥ�������/�������ν���
 *		���եȥ����䡢Ʊ�쵡ǽ�����Υݡ��Ȥ�Ʊ����������
 *----------------------------------------------------------------------*/
static	void	do_lattertype(int code, int on)
{
				  /* KEY88_XXX �� KEY88_EXT_XXX ���Ѵ� */
    int code2 = code - KEY88_F6 + KEY88_END;

    switch (code) {
    case KEY88_F6:
    case KEY88_F7:
    case KEY88_F8:
    case KEY88_F9:
    case KEY88_F10:
    case KEY88_INS:		/* KEY88_SHIFTR, KEY88_SHIFTL �� ? */
	if (on) { KEY88_PRESS  (KEY88_SHIFT);  KEY88_PRESS  (code2); }
	else    { KEY88_RELEASE(KEY88_SHIFT);  KEY88_RELEASE(code2); }
	break;

    case KEY88_DEL:
    case KEY88_BS:
    case KEY88_HENKAN:
    case KEY88_KETTEI:
    case KEY88_ZENKAKU:
    case KEY88_PC:
    case KEY88_RETURNR:		/* KEY88_RETURNL �� ? */
    case KEY88_RETURNL:		/* KEY88_RETURNR �� ? */
    case KEY88_SHIFTR:		/* KEY88_SHIFTL  �� ? */
    case KEY88_SHIFTL:		/* KEY88_SHIFTR  �� ? */
	if (on) { KEY88_PRESS  (code2); }
	else    { KEY88_RELEASE(code2); }
	break;

    default:
	return;
    }
}

/*----------------------------------------------------------------------
 * �ե��󥯥���󥭡��˳�����Ƥ���ǽ�ν���
 *		����ͤϡ������ʥ��������� (0�ʤ饭�������ʤ�����)
 *----------------------------------------------------------------------*/
static void change_framerate(int sign);
static void change_volume(int sign);
static void change_wait(int sign);
static void change_max_speed(int new_speed);
static void change_max_clock(double new_clock);
static void change_max_boost(int new_boost);

static	int	do_func(int func, int on)
{
    switch (func) {
    case FN_FUNC:				/* ��ǽ�ʤ� */
	return 0;

    case FN_FRATE_UP:				/* �ե졼�� */
	if (on) change_framerate(+1);
	return 0;
    case FN_FRATE_DOWN:				/* �ե졼�� */
	if (on) change_framerate(-1);
	return 0;

    case FN_VOLUME_UP:				/* ���� */
	if (on) change_volume(-1);
	return 0;
    case FN_VOLUME_DOWN:			/* ���� */
	if (on) change_volume(+1);
	return 0;

    case FN_PAUSE:				/* ������ */
	if (on) quasi88_pause();
	return 0;

    case FN_RESIZE:				/* �ꥵ���� */
	if (on) {
	    quasi88_cfg_set_size_large();
	}
	return 0;

    case FN_NOWAIT:				/* �������� */
	if (on) change_wait(0);
	return 0;
    case FN_SPEED_UP:
	if (on) change_wait(+1);
	return 0;
    case FN_SPEED_DOWN:
	if (on) change_wait(-1);
	return 0;

    case FN_FULLSCREEN:				/* �������ڤ��ؤ� */
	if (on) {
	    if (quasi88_cfg_can_fullscreen()) {
		int now  = quasi88_cfg_now_fullscreen();
		int next = (now) ? FALSE : TRUE;
		quasi88_cfg_set_fullscreen(next);
	    }
	}
	return 0;

    case FN_IMAGE_NEXT1:			/* DRIVE1: ���᡼���ѹ� */
	if (on) quasi88_disk_image_empty(DRIVE_1);
	else    quasi88_disk_image_next(DRIVE_1);
	return 0;
    case FN_IMAGE_PREV1:
	if (on) quasi88_disk_image_empty(DRIVE_1);
	else    quasi88_disk_image_prev(DRIVE_1);
	return 0;
    case FN_IMAGE_NEXT2:			/* DRIVE2: ���᡼���ѹ� */
	if (on) quasi88_disk_image_empty(DRIVE_2);
	else    quasi88_disk_image_next(DRIVE_2);
	return 0;
    case FN_IMAGE_PREV2:
	if (on) quasi88_disk_image_empty(DRIVE_2);
	else    quasi88_disk_image_prev(DRIVE_2);
	return 0;

    case FN_NUMLOCK:				/* NUM lock */
	if (on) {
	    if (numlock_emu) event_numlock_off();
	    numlock_emu ^= 1;
	    keyboard_switch();
	}
	return 0;

    case FN_RESET:				/* �ꥻ�å� */
	if (on) quasi88_reset(NULL);
	return 0;

    case FN_KANA:				/* ���� */
	if (on) {
	    KEY88_TOGGLE(KEY88_KANA);
	    romaji_input_mode = FALSE;
	}
	return 0;

    case FN_ROMAJI:				/* ����(���޻�) */
	if (on) {
	    KEY88_TOGGLE(KEY88_KANA);
	    if (IS_KEY88_PRESS(KEY88_KANA)) {
		romaji_input_mode = TRUE;
		romaji_clear();
	    } else {
		romaji_input_mode = FALSE;
	    }
	}
	return 0;

    case FN_CAPS:				/* CAPS */
	if (on) {
	    KEY88_TOGGLE(KEY88_CAPS);
	}
	return 0;

    case FN_SNAPSHOT:				/* �����꡼�󥹥ʥåץ���å�*/
	if (on) quasi88_screen_snapshot();
	return 0;

    case FN_MAX_SPEED:
	if (on) change_max_speed(fn_max_speed);
	return 0;
    case FN_MAX_CLOCK:
	if (on) change_max_clock(fn_max_clock);
	return 0;
    case FN_MAX_BOOST:
	if (on) change_max_boost(fn_max_boost);
	return 0;

    case FN_STATUS:				/* FDD���ơ�����ɽ�� */
	if (on) {
	    if (quasi88_cfg_can_showstatus()) {
		int now  = quasi88_cfg_now_showstatus();
		int next = (now) ? FALSE : TRUE;
		quasi88_cfg_set_showstatus(next);
	    }
	}
	return 0;
    case FN_MENU:				/* ��˥塼�⡼�� */
	if (on) quasi88_menu();
	return 0;

    }
    return func;
}



/*
 * �ե졼�ॹ���å׿��ѹ� : -frameskip ���ͤ��ѹ����롣
 */
static void change_framerate(int sign)
{
    int	i, rate;
				/*     framerate up ��                  */
    static const int list[] = { 1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30, 60, };
				/*                  �� framerate down   */

    rate = quasi88_cfg_now_frameskip_rate();

    if (sign > 0) {		/* framerate up */

	for (i=COUNTOF(list)-1; i>=0; i--) {
	    if (list[i] < rate) { rate = list[i]; break; }
	}
	if (i < 0) rate = list[COUNTOF(list)-1];

    } else {			/* framerate down*/

	for (i=0; i<COUNTOF(list); i++) {
	    if (rate < list[i]) { rate = list[i]; break; }
	}
	if (i == COUNTOF(list)) rate = list[0];
    }

    quasi88_cfg_set_frameskip_rate(rate);
}



/*
 * �ܥ�塼���ѹ� : -vol ���ͤ��ѹ����롣
 */
static void change_volume(int sign)
{
#ifdef USE_SOUND
    int diff, vol;
    char str[32];

    if (xmame_has_sound()) {
	if (xmame_has_mastervolume()) {
	    diff = (sign > 0) ? +1 : ((sign < 0) ? -1 : 0);
	    if (diff){
		vol = xmame_cfg_get_mastervolume() + diff;
		if (vol >   0) vol = 0;
		if (vol < -32) vol = -32;
		xmame_cfg_set_mastervolume(vol);
	    }
    
	    sprintf(str, "VOLUME  %3d[db]", xmame_cfg_get_mastervolume());
	    status_message(1, STATUS_INFO_TIME, str);
	    /* �ѹ�������ϡ����Ф餯���̤˲��̤�ɽ�������� */
	}
    }
#endif
}



/*
 * �����������ѹ� : -nowait, -speed ���ͤ��ѹ����롣
 */
static void change_wait(int sign)
{
    int w;

    if (sign == 0) {		/* ��������̵ͭ���ѹ� */

	w = quasi88_cfg_now_no_wait();
	w ^= 1;
	quasi88_cfg_set_no_wait(w);

    } else {			/* ����������Ψ������ */

	w = quasi88_cfg_now_wait_rate();

	if (sign < 0) {
	    w -= 10;
	    if (w < 10) w = 10;
	} else {
	    w += 10;
	    if (w > 200) w = 200;
	}

	quasi88_cfg_set_wait_rate(w);
    }
}


/*
 * ®���ѹ� (speed/clock/boost)
 */
static void change_max_speed(int new_speed)
{
    char str[32];

    if (! (5 <= new_speed || new_speed <= 5000)) {
	new_speed = 1600;
    }

    if (wait_rate < new_speed) wait_rate = new_speed;
    else                       wait_rate = 100;

    wait_vsync_switch();

    no_wait = 0;

    sprintf(str, "WAIT  %4d[%%]", wait_rate);
    status_message(1, STATUS_INFO_TIME, str);
}
static void change_max_clock(double new_clock)
{
    double def_clock = (boot_clock_4mhz ? CONST_4MHZ_CLOCK : CONST_8MHZ_CLOCK);
    char str[32];

    if (! (0.1 <= new_clock && new_clock < 1000.0)) {
	new_clock = CONST_4MHZ_CLOCK * 16;
    }

    if (cpu_clock_mhz < new_clock) cpu_clock_mhz = new_clock;
    else                           cpu_clock_mhz = def_clock;
    interval_work_init_all();

    sprintf(str, "CLOCK %8.4f[MHz]", cpu_clock_mhz);
    status_message(1, STATUS_INFO_TIME, str);
}
static void change_max_boost(int new_boost)
{
    char str[32];

    if (! (1 <= new_boost || new_boost <= 100)) {
	new_boost = 16;
    }

    if (boost < new_boost) boost_change(new_boost);
    else                   boost_change(1);
  
    sprintf(str, "BOOST [x%2d]", boost);
    status_message(1, STATUS_INFO_TIME, str);
}













/******************************************************************************
 * �ڵ����¸����ꡢ�ޥ����Υܥ��󲡲����˸ƤӽФ�����
 *
 *	code �ϡ����������ɤǡ� KEY88_MOUSE_L <= code <= KEY88_MOUSE_DOWN
 *	on   �ϡ����������ʤ鿿�����������ʤ鵶
 *****************************************************************************/
void	quasi88_mouse(int code, int on)
{
    if (quasi88_is_exec()) {		/*===================================*/
	/*
	  if (on) printf("+%d\n",code);
	  else    printf("-%d\n",code);
	*/

	if (key_func[ code ]) {			/* ���������ɤ��ɤ��ؤ�      */
	    code = do_func(key_func[ code ], on);   /* �ü쵡ǽ������С�    */
	    if (code == 0) return;		    /* ���������Τ����    */
	}					    /* ���޻����Ϥ䡢      */
						    /* ����������Ͻ������ʤ�*/

	if (on) KEY88_PRESS(code);		/* I/O�ݡ��Ȥ�ȿ��           */
	else    KEY88_RELEASE(code);

	/* ���ꥢ��ޥ����ѤΥ�����å� */
	if (code == KEY88_MOUSE_L) {
	    if (on) mouse_sb |=  0x20;
	    else    mouse_sb &= ~0x20;
	}
	if (code == KEY88_MOUSE_R) {
	    if (on) mouse_sb |=  0x10;
	    else    mouse_sb &= ~0x10;
	}

    }
    else if (quasi88_is_menu()) {	/*===================================*/

	if (on) q8tk_event_mouse_on(code);
	else    q8tk_event_mouse_off(code);
    }

    if (on) screen_attr_mouse_click();	/* �ޥ�����ư����֡Ĥ����ǥ���� */
}




/******************************************************************************
 * �ڵ����¸����ꡢ���祤���ƥ��å����ϻ��˸ƤӽФ�����
 *
 *	code �ϡ����������ɤǡ� KEY88_PAD1_UP <= code <= KEY88_PAD1_H
 *	on   �ϡ����������ʤ鿿�����������ʤ鵶
 *****************************************************************************/
void	quasi88_pad(int code, int on)
{
    if (quasi88_is_exec()) {		/*===================================*/

	if (key_func[ code ]) {			/* ���������ɤ��ɤ��ؤ�      */
	    code = do_func(key_func[ code ], on);   /* �ü쵡ǽ������С�    */
	    if (code == 0) return;		    /* ���������Τ����    */
	}					    /* ���޻����Ϥ䡢      */
						    /* ����������Ͻ������ʤ�*/

	if (on) KEY88_PRESS(code);		/* I/O�ݡ��Ȥ�ȿ��           */
	else    KEY88_RELEASE(code);

    }
}






/****************************************************************************
 * �ڵ����¸����ꡢ�ޥ�����ư���˸ƤӽФ���롣��
 *
 *	abs_coord �������ʤ顢x,y �ϥޥ����ΰ�ư��κ�ɸ�򼨤���
 *		��ɸ�ϡ�graph_setup() ������ͤˤƱ������� ���̥�����
 *		width �� height ���Ф����ͤ򥻥åȤ��뤳�ȡ�
 *		(�ʤ����ϰϳ����ͤǤ�ĤȤ���)
 *
 *	abs_coord �� ���ʤ顢x,y �ϥޥ����ΰ�ư�̤򼨤���
 *****************************************************************************/
void	quasi88_mouse_move(int x, int y, int abs_coord)
{
    if (abs_coord) {
	mouse_movement_adjust(&x, &y);
    } else {
	/* �������ˤ�������ϡ� */
    }


    if (quasi88_is_exec()) {		/*===================================*/
	if (abs_coord) {
	    mouse_dx += x - mouse_x;
	    mouse_dy += y - mouse_y;
	    mouse_x = x;
	    mouse_y = y;

	} else {

	    /* �ޥ�����®��Ĵ�� */
	    x = x * mouse_sensitivity / 100;
	    y = y * mouse_sensitivity / 100;

	    mouse_dx += x;
	    mouse_dy += y;
	}
	mouse_sx = mouse_dx;
	mouse_sy = mouse_dy;

    }
    else if (quasi88_is_menu()) {	/*===================================*/

	if (abs_coord) {
	    mouse_x = x;
	    mouse_y = y;

	} else {
	    mouse_x += x;
	    if (mouse_x <   0) mouse_x = 0;
	    if (mouse_x > 640) mouse_x = 640;

	    mouse_y += y;
	    if (mouse_y <   0) mouse_y = 0;
	    if (mouse_y > 400) mouse_y = 400;
	}

	q8tk_event_mouse_moved(mouse_x, mouse_y);
    }

    screen_attr_mouse_move();	/* �ޥ�����ư�Ǳ����Ĥ����ǲ�� */
}



/****************************************************************************
 * ��˥塼�⡼�ɤΥ��եȥ����������ܡ��ɽ��� (����Ȥ��� scan_key �����)
 *****************************************************************************/

/* ���եȥ����������ܡ��ɤ������줿���֤ʤ鿿���֤� */
int	softkey_is_pressed(int code)
{
    if (IS_KEY88_LATTERTYPE(code)) {
	code = code - KEY88_F6 + KEY88_END;
    }
    return ((key_scan[ keyport[(code)].port ] & keyport[(code)].mask) == 0);
}

/* ���եȥ����������ܡ��ɤ򲡤� */
void	softkey_press(int code)
{
    if (IS_KEY88_LATTERTYPE(code)) {
	do_lattertype(code, TRUE);
    }
    KEY88_PRESS(code);
}

/* ���եȥ����������ܡ��ɤ�Υ�� */
void	softkey_release(int code)
{
    if (IS_KEY88_LATTERTYPE(code)) {
	do_lattertype(code, FALSE);
    }
    KEY88_RELEASE(code);
}

/* ���եȥ����������ܡ��ɤ�����Υ�� */
void	softkey_release_all(void)
{
    size_t i;
    for (i=0; i<sizeof(key_scan); i++) key_scan[i] = 0xff;
}

/* ���եȥ����������ܡ��ɤΡ������ܡ��ɥХ���Ƹ� */
void	softkey_bug(void)
{
    int  my_port, your_port;
    byte my_val,  your_val,  save_val;

    save_val = key_scan[8] & 0xf0;	/* port 8 �� ��� 4bit ���оݳ� */
    key_scan[8] |= 0xf0;

    /* port 0��11(����������ܡ��ɤ��ϰ�) �Τߡ��������� */
    for (my_port=0; my_port<12; my_port++) {
	for (your_port=0; your_port<12; your_port++) {

	    if (my_port == your_port) continue;

	    my_val   = key_scan[ my_port ];
	    your_val = key_scan[ your_port ];

	    if ((my_val | your_val) != 0xff) {
		key_scan[ my_port ]   =
		    key_scan[ your_port ] = my_val & your_val;
	    }

	}
    }

    key_scan[8] &= ~0xf0;
    key_scan[8] |= save_val;
}







/***********************************************************************
 * ���ơ��ȥ��ɡ����ơ��ȥ�����
 ************************************************************************/
/* ver 0.6.2 ������ ver 0.6.3 �ʹߤǥե��󥯥���󥭡��ε�ǽ���Ѥ�ä��Τǡ�
   ���ơ��ȥ��ɡ����ơ��ȥ����֤κݤ��Ѵ����롣*/

enum {			/* ver 0.6.2�����Ρ��ե��󥯥���󥭡��ε�ǽ	*/
  OLD_FN_FUNC,
  OLD_FN_FRATE_UP,
  OLD_FN_FRATE_DOWN,
  OLD_FN_VOLUME_UP,
  OLD_FN_VOLUME_DOWN,
  OLD_FN_PAUSE,
  OLD_FN_RESIZE,
  OLD_FN_NOWAIT,
  OLD_FN_SPEED_UP,
  OLD_FN_SPEED_DOWN,
  OLD_FN_MOUSE_HIDE,
  OLD_FN_FULLSCREEN,
  OLD_FN_IMAGE_NEXT1,
  OLD_FN_IMAGE_PREV1,
  OLD_FN_IMAGE_NEXT2,
  OLD_FN_IMAGE_PREV2,
  OLD_FN_NUMLOCK,
  OLD_FN_RESET,
  OLD_FN_KANA,
  OLD_FN_ROMAJI,
  OLD_FN_CAPS,
  OLD_FN_KETTEI,
  OLD_FN_HENKAN,
  OLD_FN_ZENKAKU,
  OLD_FN_PC,
  OLD_FN_SNAPSHOT,
  OLD_FN_STOP,
  OLD_FN_COPY,
  OLD_FN_STATUS,
  OLD_FN_MENU,
  OLD_FN_end
};
static struct{		/* ver 0.6.3�ʹߤȤΡ���ǽ���б�ɽ */
	int	old;		int	now;
} func_f_convert[] =
{
  {	OLD_FN_FUNC,		FN_FUNC,	},
  {	OLD_FN_FRATE_UP,	FN_FRATE_UP,	},
  {	OLD_FN_FRATE_DOWN,	FN_FRATE_DOWN,	},
  {	OLD_FN_VOLUME_UP,	FN_VOLUME_UP,	},
  {	OLD_FN_VOLUME_DOWN,	FN_VOLUME_DOWN, },
  {	OLD_FN_PAUSE,		FN_PAUSE,	},
  {	OLD_FN_RESIZE,		FN_RESIZE,	},
  {	OLD_FN_NOWAIT,		FN_NOWAIT,	},
  {	OLD_FN_SPEED_UP,	FN_SPEED_UP,	},
  {	OLD_FN_SPEED_DOWN,	FN_SPEED_DOWN,	},
  {	OLD_FN_MOUSE_HIDE,	FN_FUNC,	},
  {	OLD_FN_FULLSCREEN,	FN_FULLSCREEN,	},
  {	OLD_FN_IMAGE_NEXT1,	FN_IMAGE_NEXT1, },
  {	OLD_FN_IMAGE_PREV1,	FN_IMAGE_PREV1, },
  {	OLD_FN_IMAGE_NEXT2,	FN_IMAGE_NEXT2, },
  {	OLD_FN_IMAGE_PREV2,	FN_IMAGE_PREV2, },
  {	OLD_FN_NUMLOCK,		FN_NUMLOCK,	},
  {	OLD_FN_RESET,		FN_RESET,	},
  {	OLD_FN_KANA,		FN_KANA,	},
  {	OLD_FN_ROMAJI,		FN_ROMAJI,	},
  {	OLD_FN_CAPS,		FN_CAPS,	},
  {	OLD_FN_KETTEI,		KEY88_KETTEI,	},
  {	OLD_FN_HENKAN,		KEY88_HENKAN,	},
  {	OLD_FN_ZENKAKU,		KEY88_ZENKAKU,	},
  {	OLD_FN_PC,		KEY88_PC,	},
  {	OLD_FN_SNAPSHOT,	FN_SNAPSHOT,	},
  {	OLD_FN_STOP,		KEY88_STOP,	},
  {	OLD_FN_COPY,		KEY88_COPY,	},
  {	OLD_FN_STATUS,		FN_STATUS,	},
  {	OLD_FN_MENU,		FN_MENU,	},
  {	OLD_FN_FUNC,		FN_MAX_SPEED,	},
  {	OLD_FN_FUNC,		FN_MAX_CLOCK,	},
  {	OLD_FN_FUNC,		FN_MAX_BOOST,	},
};
static	int	old_func_f[ 1 + 20 ];
static	void	function_new2old( void )
{
  int i, j;
  for( i=1; i<=20; i++ ){
    old_func_f[i] = OLD_FN_FUNC;
    for( j=0; j<COUNTOF(func_f_convert); j++ ){
      if( function_f[i] == func_f_convert[j].now ){
	old_func_f[i] = func_f_convert[j].old;
	break;
      }
    }
  }
}
static	void	function_old2new( void )
{
  int i, j;
  for( i=1; i<=20; i++ ){
    function_f[i] = FN_FUNC;
    for( j=0; j<COUNTOF(func_f_convert); j++ ){
      if( old_func_f[i] == func_f_convert[j].old ){
	function_f[i] = func_f_convert[j].now;
	break;
      }
    }
  }
}



#define	SID	"KYBD"
#define	SID2	"KYB2"
#define	SID3	"KYB3"
#define	SID4	"KYB4"

static	T_SUSPEND_W	suspend_keyboard_work[] =
{
  { TYPE_INT,	&jop1_step		},
  { TYPE_INT,	&jop1_dx		},
  { TYPE_INT,	&jop1_dy		},

  { TYPE_INT,	&romaji_input_mode	},

  { TYPE_INT,	&mouse_mode		},
  { TYPE_INT,	&mouse_key_mode		},
  { TYPE_INT,	&mouse_key_assign[ 0]	},
  { TYPE_INT,	&mouse_key_assign[ 1]	},
  { TYPE_INT,	&mouse_key_assign[ 2]	},
  { TYPE_INT,	&mouse_key_assign[ 3]	},
  { TYPE_INT,	&mouse_key_assign[ 4]	},
  { TYPE_INT,	&mouse_key_assign[ 5]	},

  { TYPE_INT,	&joy_key_mode		},
  { TYPE_INT,	&joy_key_assign[ 0]	},
  { TYPE_INT,	&joy_key_assign[ 1]	},
  { TYPE_INT,	&joy_key_assign[ 2]	},
  { TYPE_INT,	&joy_key_assign[ 3]	},
  { TYPE_INT,	&joy_key_assign[ 4]	},
  { TYPE_INT,	&joy_key_assign[ 5]	},
  { TYPE_INT,	&joy_key_assign[ 6]	},
  { TYPE_INT,	&joy_key_assign[ 7]	},
  { TYPE_INT,	&joy_key_assign[ 8]	},
  { TYPE_INT,	&joy_key_assign[ 9]	},
  { TYPE_INT,	&joy_key_assign[10]	},
  { TYPE_INT,	&joy_key_assign[11]	},
  { TYPE_INT,	&joy_swap_button	},

  { TYPE_INT,	&cursor_key_mode	},
  { TYPE_INT,	&cursor_key_assign[0]	},
  { TYPE_INT,	&cursor_key_assign[1]	},
  { TYPE_INT,	&cursor_key_assign[2]	},
  { TYPE_INT,	&cursor_key_assign[3]	},

  { TYPE_INT,	&tenkey_emu		},
  { TYPE_INT,	&numlock_emu		},

  { TYPE_INT,	&old_func_f[ 1]		},
  { TYPE_INT,	&old_func_f[ 2]		},
  { TYPE_INT,	&old_func_f[ 3]		},
  { TYPE_INT,	&old_func_f[ 4]		},
  { TYPE_INT,	&old_func_f[ 5]		},
  { TYPE_INT,	&old_func_f[ 6]		},
  { TYPE_INT,	&old_func_f[ 7]		},
  { TYPE_INT,	&old_func_f[ 8]		},
  { TYPE_INT,	&old_func_f[ 9]		},
  { TYPE_INT,	&old_func_f[10]		},
  { TYPE_INT,	&old_func_f[11]		},
  { TYPE_INT,	&old_func_f[12]		},
  { TYPE_INT,	&old_func_f[13]		},
  { TYPE_INT,	&old_func_f[14]		},
  { TYPE_INT,	&old_func_f[15]		},
  { TYPE_INT,	&old_func_f[16]		},
  { TYPE_INT,	&old_func_f[17]		},
  { TYPE_INT,	&old_func_f[18]		},
  { TYPE_INT,	&old_func_f[19]		},
  { TYPE_INT,	&old_func_f[20]		},

  { TYPE_INT,	&romaji_type		},

  { TYPE_END,	0			},
};

static	T_SUSPEND_W	suspend_keyboard_work2[] =
{
  { TYPE_INT,	&jop1_time		},
  { TYPE_END,	0			},
};

static	T_SUSPEND_W	suspend_keyboard_work3[] =
{
  { TYPE_INT,	&function_f[ 1]		},
  { TYPE_INT,	&function_f[ 2]		},
  { TYPE_INT,	&function_f[ 3]		},
  { TYPE_INT,	&function_f[ 4]		},
  { TYPE_INT,	&function_f[ 5]		},
  { TYPE_INT,	&function_f[ 6]		},
  { TYPE_INT,	&function_f[ 7]		},
  { TYPE_INT,	&function_f[ 8]		},
  { TYPE_INT,	&function_f[ 9]		},
  { TYPE_INT,	&function_f[10]		},
  { TYPE_INT,	&function_f[11]		},
  { TYPE_INT,	&function_f[12]		},
  { TYPE_INT,	&function_f[13]		},
  { TYPE_INT,	&function_f[14]		},
  { TYPE_INT,	&function_f[15]		},
  { TYPE_INT,	&function_f[16]		},
  { TYPE_INT,	&function_f[17]		},
  { TYPE_INT,	&function_f[18]		},
  { TYPE_INT,	&function_f[19]		},
  { TYPE_INT,	&function_f[20]		},
  { TYPE_END,	0			},
};

static	T_SUSPEND_W	suspend_keyboard_work4[] =
{
  { TYPE_INT,	&mouse_sensitivity	},
  { TYPE_INT,	&mouse_swap_button	},

  { TYPE_INT,	&joy2_key_mode		},
  { TYPE_INT,	&joy2_key_assign[ 0]	},
  { TYPE_INT,	&joy2_key_assign[ 1]	},
  { TYPE_INT,	&joy2_key_assign[ 2]	},
  { TYPE_INT,	&joy2_key_assign[ 3]	},
  { TYPE_INT,	&joy2_key_assign[ 4]	},
  { TYPE_INT,	&joy2_key_assign[ 5]	},
  { TYPE_INT,	&joy2_key_assign[ 6]	},
  { TYPE_INT,	&joy2_key_assign[ 7]	},
  { TYPE_INT,	&joy2_key_assign[ 8]	},
  { TYPE_INT,	&joy2_key_assign[ 9]	},
  { TYPE_INT,	&joy2_key_assign[10]	},
  { TYPE_INT,	&joy2_key_assign[11]	},
  { TYPE_INT,	&joy2_swap_button	},

  { TYPE_INT,	&serial_mouse_x		},
  { TYPE_INT,	&serial_mouse_y		},
  { TYPE_INT,	&serial_mouse_step	},

  { TYPE_END,	0			},
};


int	statesave_keyboard( void )
{
  function_new2old();

  if( statesave_table( SID, suspend_keyboard_work ) != STATE_OK ) return FALSE;

  if( statesave_table( SID2,suspend_keyboard_work2) != STATE_OK ) return FALSE;

  if( statesave_table( SID3,suspend_keyboard_work3) != STATE_OK ) return FALSE;

  if( statesave_table( SID4,suspend_keyboard_work4) != STATE_OK ) return FALSE;

  return TRUE;
}

int	stateload_keyboard( void )
{
  if( stateload_table( SID, suspend_keyboard_work ) != STATE_OK ) return FALSE;

  if( stateload_table( SID2,suspend_keyboard_work2) != STATE_OK ){

    /* ��С������ʤ顢�ߤΤ��� */

    printf( "stateload : Statefile is old. (ver 0.6.0 or 1?)\n" );

    goto NOT_HAVE_SID2;
  }

  if( stateload_table( SID3,suspend_keyboard_work3) != STATE_OK ){

    /* ��С������ʤ顢�ߤΤ��� */

    printf( "stateload : Statefile is old. (ver 0.6.0, 1 or 2?)\n" );

    goto NOT_HAVE_SID3;
  }

  if( stateload_table( SID4,suspend_keyboard_work4) != STATE_OK ){

    /* ��С������ʤ顢�ߤΤ��� */

    printf( "stateload : Statefile is old. (ver 0.6.0, 1, 2 or 3?)\n" );
  }

  return TRUE;



 NOT_HAVE_SID2:
  /* ���δؿ��θƤӽФ������ˡ� stateload_pc88main �� stateload_intr ��
     �ƤӽФ���Ƥ��ʤ���С��ʲ��ν�����ϰ�̣���ʤ� */

  jop1_time = state_of_cpu + z80main_cpu.state0;


 NOT_HAVE_SID3:
  /* function_f[] �򺹤��ؤ��� */
  function_old2new();


  return TRUE;
}










/****************************************************************************
 *
 *	�桼�ƥ���ƥ�
 *
 *****************************************************************************/

/* QUASI88 ���������ɤ�ʸ����� int �ͤ��Ѵ�����ơ��֥� */

static const T_SYMBOL_TABLE key88sym_list[] =
{
    { "KEY88_INVALID"		,	KEY88_INVALID		},
    { "KEY88_SPACE" 		,	KEY88_SPACE         	},
    { "KEY88_EXCLAM"		,	KEY88_EXCLAM        	},
    { "KEY88_QUOTEDBL"		,	KEY88_QUOTEDBL      	},
    { "KEY88_NUMBERSIGN"	,	KEY88_NUMBERSIGN    	},
    { "KEY88_DOLLAR"   		,	KEY88_DOLLAR        	},
    { "KEY88_PERCENT"		,	KEY88_PERCENT       	},
    { "KEY88_AMPERSAND"		,	KEY88_AMPERSAND     	},
    { "KEY88_APOSTROPHE"	,	KEY88_APOSTROPHE    	},
    { "KEY88_PARENLEFT"		,	KEY88_PARENLEFT     	},
    { "KEY88_PARENRIGHT"	,	KEY88_PARENRIGHT    	},
    { "KEY88_ASTERISK" 		,	KEY88_ASTERISK      	},
    { "KEY88_PLUS"   		,	KEY88_PLUS          	},
    { "KEY88_COMMA"  		,	KEY88_COMMA         	},
    { "KEY88_MINUS"		,	KEY88_MINUS         	},
    { "KEY88_PERIOD"		,	KEY88_PERIOD        	},
    { "KEY88_SLASH"		,	KEY88_SLASH         	},
    { "KEY88_0"  		,	KEY88_0             	},
    { "KEY88_1"			,	KEY88_1             	},
    { "KEY88_2"			,	KEY88_2             	},
    { "KEY88_3"			,	KEY88_3             	},
    { "KEY88_4"			,	KEY88_4             	},
    { "KEY88_5"			,	KEY88_5             	},
    { "KEY88_6"			,	KEY88_6             	},
    { "KEY88_7"			,	KEY88_7             	},
    { "KEY88_8"			,	KEY88_8             	},
    { "KEY88_9"			,	KEY88_9             	},
    { "KEY88_COLON"		,	KEY88_COLON         	},
    { "KEY88_SEMICOLON"		,	KEY88_SEMICOLON     	},
    { "KEY88_LESS"    		,	KEY88_LESS          	},
    { "KEY88_EQUAL"		,	KEY88_EQUAL         	},
    { "KEY88_GREATER"		,	KEY88_GREATER       	},
    { "KEY88_QUESTION"		,	KEY88_QUESTION      	},
    { "KEY88_AT" 	   	,	KEY88_AT            	},
    { "KEY88_A"			,	KEY88_A             	},
    { "KEY88_B"			,	KEY88_B             	},
    { "KEY88_C"			,	KEY88_C             	},
    { "KEY88_D"			,	KEY88_D             	},
    { "KEY88_E"			,	KEY88_E             	},
    { "KEY88_F"			,	KEY88_F             	},
    { "KEY88_G"			,	KEY88_G             	},
    { "KEY88_H"			,	KEY88_H             	},
    { "KEY88_I"			,	KEY88_I             	},
    { "KEY88_J"			,	KEY88_J             	},
    { "KEY88_K"			,	KEY88_K             	},
    { "KEY88_L"			,	KEY88_L             	},
    { "KEY88_M"			,	KEY88_M             	},
    { "KEY88_N"			,	KEY88_N             	},
    { "KEY88_O"			,	KEY88_O             	},
    { "KEY88_P"			,	KEY88_P             	},
    { "KEY88_Q"			,	KEY88_Q             	},
    { "KEY88_R"			,	KEY88_R             	},
    { "KEY88_S"			,	KEY88_S             	},
    { "KEY88_T"			,	KEY88_T             	},
    { "KEY88_U"			,	KEY88_U             	},
    { "KEY88_V"			,	KEY88_V             	},
    { "KEY88_W"			,	KEY88_W             	},
    { "KEY88_X"			,	KEY88_X             	},
    { "KEY88_Y"			,	KEY88_Y             	},
    { "KEY88_Z"			,	KEY88_Z             	},
    { "KEY88_BRACKETLEFT"	,	KEY88_BRACKETLEFT   	},
    { "KEY88_YEN"	 	,	KEY88_YEN	     	},
    { "KEY88_BRACKETRIGHT"	,	KEY88_BRACKETRIGHT  	},
    { "KEY88_CARET"      	,	KEY88_CARET         	},
    { "KEY88_UNDERSCORE"	,	KEY88_UNDERSCORE    	},
    { "KEY88_BACKQUOTE"		,	KEY88_BACKQUOTE     	},
    { "KEY88_a"  	    	,	KEY88_a             	},
    { "KEY88_b"  	    	,	KEY88_b             	},
    { "KEY88_c"			,	KEY88_c             	},
    { "KEY88_d"			,	KEY88_d             	},
    { "KEY88_e"			,	KEY88_e             	},
    { "KEY88_f"			,	KEY88_f             	},
    { "KEY88_g"			,	KEY88_g             	},
    { "KEY88_h"			,	KEY88_h             	},
    { "KEY88_i"			,	KEY88_i             	},
    { "KEY88_j"			,	KEY88_j             	},
    { "KEY88_k"			,	KEY88_k             	},
    { "KEY88_l"			,	KEY88_l             	},
    { "KEY88_m"			,	KEY88_m             	},
    { "KEY88_n"			,	KEY88_n             	},
    { "KEY88_o"			,	KEY88_o             	},
    { "KEY88_p"			,	KEY88_p             	},
    { "KEY88_q"			,	KEY88_q             	},
    { "KEY88_r"			,	KEY88_r             	},
    { "KEY88_s"			,	KEY88_s             	},
    { "KEY88_t"			,	KEY88_t             	},
    { "KEY88_u"			,	KEY88_u             	},
    { "KEY88_v"			,	KEY88_v             	},
    { "KEY88_w"			,	KEY88_w             	},
    { "KEY88_x"			,	KEY88_x             	},
    { "KEY88_y"			,	KEY88_y             	},
    { "KEY88_z"			,	KEY88_z             	},
    { "KEY88_BRACELEFT"		,	KEY88_BRACELEFT     	},
    { "KEY88_BAR"	    	,	KEY88_BAR           	},
    { "KEY88_BRACERIGHT"	,	KEY88_BRACERIGHT    	},
    { "KEY88_TILDE"    		,	KEY88_TILDE         	},
    { "KEY88_KP_0"		,	KEY88_KP_0          	},
    { "KEY88_KP_1"		,	KEY88_KP_1          	},
    { "KEY88_KP_2"		,	KEY88_KP_2          	},
    { "KEY88_KP_3"		,	KEY88_KP_3          	},
    { "KEY88_KP_4"		,	KEY88_KP_4          	},
    { "KEY88_KP_5"		,	KEY88_KP_5          	},
    { "KEY88_KP_6"		,	KEY88_KP_6          	},
    { "KEY88_KP_7"		,	KEY88_KP_7          	},
    { "KEY88_KP_8"		,	KEY88_KP_8          	},
    { "KEY88_KP_9"		,	KEY88_KP_9          	},
    { "KEY88_KP_MULTIPLY"	,	KEY88_KP_MULTIPLY   	},
    { "KEY88_KP_ADD"    	,	KEY88_KP_ADD        	},
    { "KEY88_KP_EQUAL"		,	KEY88_KP_EQUAL     	},
    { "KEY88_KP_COMMA"		,	KEY88_KP_COMMA      	},
    { "KEY88_KP_PERIOD"		,	KEY88_KP_PERIOD     	},
    { "KEY88_KP_SUB"  		,	KEY88_KP_SUB        	},
    { "KEY88_KP_DIVIDE"		,	KEY88_KP_DIVIDE     	},
    { "KEY88_RETURN"  		,	KEY88_RETURN        	},
    { "KEY88_HOME" 		,	KEY88_HOME          	},
    { "KEY88_UP" 		,	KEY88_UP            	},
    { "KEY88_RIGHT"		,	KEY88_RIGHT         	},
    { "KEY88_INS_DEL"		,	KEY88_INS_DEL       	},
    { "KEY88_GRAPH" 		,	KEY88_GRAPH         	},
    { "KEY88_KANA"		,	KEY88_KANA          	},
    { "KEY88_SHIFT"		,	KEY88_SHIFT         	},
    { "KEY88_CTRL"		,	KEY88_CTRL          	},
    { "KEY88_STOP"		,	KEY88_STOP          	},
    { "KEY88_ESC"		,	KEY88_ESC           	},
    { "KEY88_TAB"		,	KEY88_TAB           	},
    { "KEY88_DOWN"		,	KEY88_DOWN          	},
    { "KEY88_LEFT"		,	KEY88_LEFT          	},
    { "KEY88_HELP"		,	KEY88_HELP          	},
    { "KEY88_COPY"		,	KEY88_COPY          	},
    { "KEY88_CAPS"		,	KEY88_CAPS          	},
    { "KEY88_ROLLUP"		,	KEY88_ROLLUP        	},
    { "KEY88_ROLLDOWN"		,	KEY88_ROLLDOWN      	},
    { "KEY88_F1" 	   	,	KEY88_F1            	},
    { "KEY88_F2"		,	KEY88_F2            	},
    { "KEY88_F3"		,	KEY88_F3            	},
    { "KEY88_F4"		,	KEY88_F4            	},
    { "KEY88_F5"		,	KEY88_F5            	},
    { "KEY88_F11"		,	KEY88_F11           	},
    { "KEY88_F12"		,	KEY88_F12           	},
    { "KEY88_F13"		,	KEY88_F13           	},
    { "KEY88_F14"		,	KEY88_F14           	},
    { "KEY88_F15"		,	KEY88_F15           	},
    { "KEY88_F16"		,	KEY88_F16           	},
    { "KEY88_F17"		,	KEY88_F17           	},
    { "KEY88_F18"		,	KEY88_F18           	},
    { "KEY88_F19"		,	KEY88_F19           	},
    { "KEY88_F20"		,	KEY88_F20           	},
    { "KEY88_F6"		,	KEY88_F6            	},
    { "KEY88_F7"		,	KEY88_F7            	},
    { "KEY88_F8"		,	KEY88_F8            	},
    { "KEY88_F9"		,	KEY88_F9            	},
    { "KEY88_F10"		,	KEY88_F10           	},
    { "KEY88_BS"		,	KEY88_BS            	},
    { "KEY88_INS"		,	KEY88_INS           	},
    { "KEY88_DEL"		,	KEY88_DEL           	},
    { "KEY88_HENKAN"		,	KEY88_HENKAN        	},
    { "KEY88_KETTEI"		,	KEY88_KETTEI        	},
    { "KEY88_PC" 	 	,	KEY88_PC            	},
    { "KEY88_ZENKAKU"		,	KEY88_ZENKAKU       	},
    { "KEY88_RETURNL"		,	KEY88_RETURNL       	},
    { "KEY88_RETURNR"		,	KEY88_RETURNR       	},
    { "KEY88_SHIFTL"		,	KEY88_SHIFTL        	},
    { "KEY88_SHIFTR"		,	KEY88_SHIFTR        	},

    { "KEY88_SYS_MENU"		,	KEY88_SYS_MENU        	},
    { "KEY88_SYS_STATUS"	,	KEY88_SYS_STATUS       	},
};


/***********************************************************************
 * Ϳ����줿ʸ�����QUASI88 ���������ɤ��Ѵ�����
 *	��������ե�����β��Ϥʤɤ˻Ȥ���
 *
 *   ��)
 *	"KEY88_SPACE" -> KEY88_SPACE	������Τޤޤ�ʸ����ϡ�ľ���Ѵ�
 *	"key88_SPACE" -> KEY88_SPACE	��ʸ�����ߤǤ�褤
 *	"KEY88_Z"     -> KEY88_Z	����⡢������Τޤޤ���
 *	"KEY88_z"     -> KEY88_z	����⡢������Τޤޤ���
 *	"key88_z"     -> KEY88_Z	��ʸ�����ߤξ�����ʸ���ˤʤ��
 *	"0x20"        -> KEY88_SPACE	0x20��0xf7 ��ľ�˥��������ɤ��Ѵ�
 *	"0x01"        -> KEY88_INVALID	�嵭���ϰϳ��ʤ�̵��(0)���֤�
 *	"32"          -> KEY88_SPACE	10�ʿ���8�ʿ��Ǥ�Ʊ��
 *	"KP1"         -> KEY88_KP_1	KP �� 1ʸ�� �ǡ��ƥ󥭡��Ȥ���
 *	"KP+"         -> KEY88_KP_ADD	����Ǥ�褤
 *	"Kp9"         -> KEY88_KP_9	��ʸ�����ߤǤ�褤
 *	"Err"         -> -1		�ɤ�ˤ���פ��ʤ��ä��顢����֤�
 ************************************************************************/

int	keyboard_str2key88(const char *str)
{
  static const T_SYMBOL_TABLE tenkey_list[] =
  {				/* �ƥ󥭡��˸¤ꡢ�㳰Ū��ɽ������ǽ */
    { "KP0"			,	KEY88_KP_0              },
    { "KP1"			,	KEY88_KP_1              },
    { "KP2"			,	KEY88_KP_2              },
    { "KP3"			,	KEY88_KP_3              },
    { "KP4"			,	KEY88_KP_4              },
    { "KP5"			,	KEY88_KP_5              },
    { "KP6"			,	KEY88_KP_6              },
    { "KP7"			,	KEY88_KP_7              },
    { "KP8"			,	KEY88_KP_8              },
    { "KP9"			,	KEY88_KP_9              },
    { "KP*"			,	KEY88_KP_MULTIPLY       },
    { "KP+"			,	KEY88_KP_ADD            },
    { "KP="			,	KEY88_KP_EQUAL          },
    { "KP,"			,	KEY88_KP_COMMA          },
    { "KP."			,	KEY88_KP_PERIOD         },
    { "KP-"			,	KEY88_KP_SUB            },
    { "KP/"			,	KEY88_KP_DIVIDE         },
  };

    int len, i;
    char *conv_end;
    unsigned long l;

    if (str == NULL) return -1;
    len = strlen(str);
    if (len == 0) return -1;


					/* 0��9 �ǻϤޤ�С��������Ѵ� */
    if ('0'<=str[0] && str[0]<='9') {
	l = strtoul(str, &conv_end, 0);		/* 10��,16��,8�ʿ�����ǽ */
	if (*conv_end == '\0') {
	    if (32 <= l && l <= 247) {
		return l;
	    } else {
		return KEY88_INVALID;
	    }
	}
	return -1;
    }
					/* 3ʸ���ʤ顢�ƥ󥭡����� */
    if (len == 3) {
	for (i=0; i<COUNTOF(tenkey_list); i++) {
	    if (strcmp(tenkey_list[i].name, str) == 0) {
		return tenkey_list[i].val;
	    }
	}
	for (i=0; i<COUNTOF(tenkey_list); i++) {
	    if (my_strcmp(tenkey_list[i].name, str) == 0) {
		return tenkey_list[i].val;
	    }
	}
    }
					/* ���ʸ����˹��פ���Τ�õ�� */
    for (i=0; i<COUNTOF(key88sym_list); i++) {
	if (strcmp(key88sym_list[i].name, str) == 0) {
	    return key88sym_list[i].val;
	}
    }
    for (i=0; i<COUNTOF(key88sym_list); i++) {
	if (my_strcmp(key88sym_list[i].name, str) == 0) {
	    return key88sym_list[i].val;
	}
    }

    return -1;
}


const char	*keyboard_key882str(int key88)
{
    int i;
    for (i=0; i<COUNTOF(key88sym_list); i++) {
	if (key88sym_list[i].val == KEY88_INVALID) continue;
	if (key88sym_list[i].val == key88) {
	    return key88sym_list[i].name;
	}
    }
    return NULL;
}





/****************************************************************************
 * ��������ե�������ɤ߹���ǡ������Ԥ���
 *
 *	����Ū�ˤϡ���������ե�����򣱹��ɤि�Ӥˡ�������Хå��ؿ���Ƥ֡�
 *	���ιԤ����Ƥ������ȿ�Ǥ��뤫�ɤ����ϡ�������Хå��ؿ����衣
 *
 * --------------------------------------------------------------------------
 * ��������ե�����ν񼰤ϡ�����ʴ���
 * (��ԤˤĤ� ����3�ĤΥȡ�������¤٤��롣 # ��������ޤǤϥ����ȤȤ���)
 *
 *	[SDL]			dga
 *	SDLK_ESCAPE		KEY88_ESC		KEY88_KP_4
 *	<49>			KEY88_ZENKAKU
 *
 * 1���ܤΤ褦�ˡ�[��] �ǻϤޤ�Ԥϡ��ּ��̥����ԡפȤ��롣
 * 1���ܤΥȡ������ [��] �η����ǡ�2���ܡ�3���ܤΥȡ������Ǥ�� (̵���Ƥ�褤)
 * �ּ��̥����ԡפ��ɤि�Ӥˡ��ʲ��Υ�����Хå��ؿ����ƤӽФ���롣
 *
 *	identify_callback(const char *param1, const char *param2,
 *			  const char *param3)
 *
 *	�����ξ�硢�����ϰʲ������åȤ����
 *		param1 �� "[SDL]"
 *		param2 �� "dga"
 *		param3 �� NULL (3���ܤΥȡ����󤬤ʤ����Ȥ��̣����)
 *
 * ���Υ�����Хå��ؿ��ϡ����Ρּ��̥����ԡפ�ͭ���Ǥ���С� NULL ���֤��Τǡ�
 * ���μ��ιԤ��顢���Ρּ��̥����ԡ� ([��] �ǻϤޤ��) �ޤǤ򡢽������롣
 * �֤��ͤ� NULL �Ǥʤ����ϡ����Ρּ��̥����ԡפޤǥ����åפ��롣
 * �ʤ����֤��ͤ���ʸ���� "" �Ǥʤ���С���˥󥰤Ȥ��Ƥ����ɽ�����롣
 *
 * 2���ܤϡ������Ρ�����ԡפȤ��롣
 * 1���ܡ�2���ܤΥȡ������ɬ�ܤǡ�3���ܤΥȡ������Ǥ�� (̵���Ƥ�褤)
 * ������ԡפΤ��Ӥˡ��ʲ��Υ�����Хå��ؿ����ƤӽФ���롣
 *
 *	setting_callback(int type, int code, int key88, int numlock_key88);
 *
 *	�����ξ�硢�����ϰʲ������åȤ���롣
 *		type          �� 1
 *		code          �� "SDLK_ESCAPE" �� int ���Ѵ�������� (��� ��1)
 *		key88         �� KEY88_ESC �� enum��
 *		numlock_key88 �� KEY88_KP_4 �� enum��
 *
 * 3���ܤ⡢�����Ρ�����ԡפ������񼰤��㴳�ۤʤäƤ��롣
 * ���ξ��⡢Ʊ�ͤ˥�����Хå��ؿ����ƤӽФ���롣
 *
 *	callback(int type, int code, int key88, int numlock_key88);
 *
 *	�����ξ�硢�����ϰʲ������åȤ���롣
 *		type          �� 2
 *		code          �� 49
 *		key88         �� KEY88_ZENKAKU �� enum��
 *		numlock_key88 �� -1 (3���ܤΥȡ����󤬤ʤ����Ȥ��̣����)
 *
 * �Ĥޤꡢ2���ܤη����ξ��ϡ�1���ܤΥȡ������ int ���Ѵ������ͤ� code ��
 * ���åȤ���뤬��3���ܤη����ξ��ϡ� <��> ����ο��ͤ����åȤ���롣
 *
 * ���Υ�����Хå��ؿ��ϡ����꤬ͭ���Ǥ���� NULL ���֤����֤��ͤ� NULL ��
 * �ʤ�����ʸ���� "" �Ǥ�ʤ����ϥ�˥󥰤Ȥ��Ƥ����ɽ�����롣
 *
 *
 * ���ơ���1 �Τ褦�ʡ� SDLK_ESCAPE �� int ���Ѵ�������ˡ�ϡ��ʲ�������ˤ�롣
 *
 *	T_SYMBOL_TABLE	table_symbol2int[]; (����Υ������ϡ� table_size)
 *
 * �����������Ƭ��������å����ơ� table_symbol2int[].name ��1���ܤΥȡ�����
 * �����פ������ˡ�table_symbol2int[].val ���ͤ��֤���
 *
 * table_ignore_case �����ξ�硢��ʸ����ʸ����̵�뤷�ƥ����å�����Τǡ�
 * �����ξ��� "SDLK_ESCAPE" "sdlk_escape" �ɤ���Ǥ���פȤߤʤ���
 *
 *
 *
 *
 *****************************************************************************/
#include <file-op.h>

static	int	symbol2int(const char *str,
			   const T_SYMBOL_TABLE table_symbol2int[],
			   int                  table_size,
			   int                  table_ignore_case);

/* ��������ե�����1�Ԥ�����κ���ʸ���� */
#define	MAX_KEYFILE_LINE	(256)

int	config_read_keyconf_file(
			const char *keyconf_filename,
			const char *(*identify_callback)(const char *parm1,
							 const char *parm2,
							 const char *parm3),
			const T_SYMBOL_TABLE table_symbol2int[],
			int                  table_size,
			int                  table_ignore_case,
			const char *(*setting_callback)(int type,
							int code,
							int key88,
							int numlock_key88))
{
    const char *filename;
    OSD_FILE *fp = NULL;
    int  working   = FALSE;
    int  effective = FALSE;

    int    line_cnt = 0;
    char   line[ MAX_KEYFILE_LINE ];
    char buffer[ MAX_KEYFILE_LINE ];

    char *parm1, *parm2, *parm3, *parm4;

    int  type, code, key88, numlock_key88;
    const char *err_mes;


	/* ��������ե�����򳫤� */

    if (keyconf_filename == NULL) {		   /* �ե�����̤̾����ʤ�� */
	filename = filename_alloc_keyboard_cfgname();  /* �ǥե����̾����� */
    } else {
	filename = keyconf_filename;
    }

    if (filename) {
	fp = osd_fopen(FTYPE_CFG, filename, "r");
	if (verbose_proc) {
	    if (fp) {
		printf("\"%s\" read and initialize\n", filename);
	    } else {
		printf("can't open keyboard configuration file \"%s\"\n",
		       filename);
		printf("\n");
	    }
	}
	if (keyconf_filename == NULL) {			/* �ǥե���Ȥʤ�� */
	    free((void*)filename);			/* ����������Ȥ� */
	}
    }

    if (fp == NULL) return FALSE;		/* �����ʤ��ä��鵶���֤� */



	/* ��������ե������1�ԤŤĲ��� */

    while (osd_fgets(line, MAX_KEYFILE_LINE, fp)) {

	line_cnt ++;
	parm1 = parm2 = parm3 = parm4 = NULL;

	/* �Ԥ����Ƥ�ȡ������ʬ�򡣳ƥȡ�����ϡ�parm1 �� parm4 �˥��å� */

	{ char *str = line;
	char *b; {             b = &buffer[0];      str = my_strtok(b, str); }
	if (str) { parm1 = b;  b += strlen(b) + 1;  str = my_strtok(b, str); }
	if (str) { parm2 = b;  b += strlen(b) + 1;  str = my_strtok(b, str); }
	if (str) { parm3 = b;  b += strlen(b) + 1;  str = my_strtok(b, str); }
	if (str) { parm4 = b;  }
	}


	/* �ȡ����󤬤ʤ���м��ιԤ� */
	if (parm1 == NULL) continue;

	/* �ȡ����󤬻͸İʾ夢��С����ιԤϥ��顼�ʤΤǼ��ιԤ� */
	if (parm4 != NULL) {
	    if (working) {
		fprintf(stderr,
			"warning: too many argument in line %d\n", line_cnt);
	    }
	    continue;
	}

	/* �ȡ����󤬰�ġ����Ĥʤ���� */
	if (parm1[0] == '[') {			/* �ּ��̥����ԡפ���� */

	    /* ������Хå��ؿ���ƤӽФ��ơ�ͭ���ʼ��̥�������Ƚ�� */
	    err_mes = (identify_callback)(parm1, parm2, parm3);

	    if (err_mes == NULL) {		/* ͭ���ʼ��̥������ä� */
		working   = TRUE;
		effective = TRUE;

		if (verbose_proc)
		    printf("(read start in line %d)\n", line_cnt);

	    } else {				/* ̵���ʼ��̥������ä� */

		if (working) {				/* ������ʤ齪λ */
		    if (verbose_proc)
			printf("(read stop  in line %d)\n", line_cnt - 1);
		}					/* �Ǥʤ����̵�� */

		if (err_mes[0] != '\0') {
		    fprintf(stderr,
			    "warning: %s in %d (ignored)\n",
			    err_mes, line_cnt);
		}

		working = FALSE;
	    }

	} else {				/* ������ԡפ���� */

	    if (working) {

		/* ������ԡפǡ��ȡ������Ĥ����ϡ����顼�����ιԤ� */
		if (parm2 == NULL) {
		    fprintf(stderr,
			    "warning: error in line %d (ignored)\n", line_cnt);
		} else {

		    code = symbol2int(parm1, table_symbol2int,
				      table_size, table_ignore_case);
		    key88 = keyboard_str2key88(parm2);

		    if (parm3) { numlock_key88 = keyboard_str2key88(parm3); }
		    else       { numlock_key88 = -1; }

		    if (code < 0 ||
			key88 < 0 ||
			(parm3 && numlock_key88 < 0)) {
			fprintf(stderr,
			    "warning: error in line %d (ignored)\n", line_cnt);
		    } else {

			if (parm1[0] == '<') { type = KEYCODE_SCAN; }
			else                 { type = KEYCODE_SYM;  }

			err_mes = (setting_callback)(type, code,
						     key88, numlock_key88);

			if (err_mes == NULL) {	/* ͭ����������ä� */
			    /* OK */ ;
			} else {		/* ̵����������ä� */
			    /* NG */
			    if (err_mes[0] != '\0') {
				fprintf(stderr,
					"warning: %s in %d (ignored)\n",
					err_mes, line_cnt);
			    }
			}
		    }
		}
	    }
	}
    }
    osd_fclose(fp);


    if (working) {
	if(verbose_proc) printf("(read end   in line %d)\n", line_cnt-1);
    }

    if (effective == FALSE) {
	fprintf(stderr, "warning: not configured (use initial config)\n");
    }

    if (verbose_proc) {
	printf("\n");
    }

    return (effective) ? TRUE : FALSE;
}


static	int	symbol2int(const char *str,
			   const T_SYMBOL_TABLE table_symbol2int[],
			   int                  table_size,
			   int                  table_ignore_case)
{
    int i;
    char *conv_end;
    unsigned long l;

    if ( str == NULL) return -1;
    if (*str == '\0') return -1;

    if (str[0] == '<') {		/* <����> �ξ�� */
	l = strtoul(&str[1], &conv_end, 0);
	if (*conv_end == '>') {
	    return l;
	}
	return -1;
    }
					/* ���ʸ����˹��פ���Τ�õ�� */
    for (i=0; i<table_size; i++) {
	if (strcmp(table_symbol2int[i].name, str) == 0) {
	    return table_symbol2int[i].val;
	}
    }
    if (table_ignore_case) {
	for (i=0; i<table_size; i++) {
	    if (my_strcmp(table_symbol2int[i].name, str) == 0) {
		return table_symbol2int[i].val;
	    }
	}
    }

    return -1;
}
