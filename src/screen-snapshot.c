/************************************************************************/
/*									*/
/* ���̤�ɽ��		���̥��ʥåץ���å�				*/
/*									*/
/************************************************************************/

#include <string.h>

#include "quasi88.h"
#include "screen.h"
#include "screen-func.h"
#include "crtcdmac.h"
#include "memory.h"
extern	char			screen_snapshot[];


#define SCREEN_WIDTH		640
#define SCREEN_HEIGHT		400
#define SCREEN_SX		640
#define SCREEN_SY		400
#define SCREEN_TOP		screen_snapshot
#define SCREEN_START		screen_snapshot

#define COLOR_PIXEL(x)		(x)
#define MIXED_PIXEL(a,b)
#define BLACK			(16)






#define TYPE		char

/*----------------------------------------------------------------------
 *			�� 200�饤��			ɸ��
 *----------------------------------------------------------------------*/
#define NORMAL

#define COLOR						/* ���顼640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_C80x25_normal
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_C80x20_normal
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_C40x25_normal
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_C40x20_normal
#include					"screen-vram-full.h"
#undef	COLOR

#define MONO						/* ���	 640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_M80x25_normal
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_M80x20_normal
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_M40x25_normal
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_M40x20_normal
#include					"screen-vram-full.h"
#undef	MONO

#define UNDISP						/* ��ɽ��640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_U80x25_normal
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_U80x20_normal
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_U40x25_normal
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_U40x20_normal
#include					"screen-vram-full.h"
#undef	UNDISP

#undef	NORMAL
/*----------------------------------------------------------------------
 *			�� 200�饤��			�饤�󥹥��å�
 *----------------------------------------------------------------------*/
#define SKIPLINE

#define COLOR						/* ���顼640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_C80x25_skipln
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_C80x20_skipln
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_C40x25_skipln
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_C40x20_skipln
#include					"screen-vram-full.h"
#undef	COLOR

#define MONO						/* ���	 640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_M80x25_skipln
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_M80x20_skipln
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_M40x25_skipln
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_M40x20_skipln
#include					"screen-vram-full.h"
#undef	MONO

#define UNDISP						/* ��ɽ��640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_U80x25_skipln
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_U80x20_skipln
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_U40x25_skipln
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_U40x20_skipln
#include					"screen-vram-full.h"
#undef	UNDISP

#undef	SKIPLINE
/*----------------------------------------------------------------------
 *			�� 200�饤��			���󥿡��졼��
 *----------------------------------------------------------------------*/
#define INTERLACE

#define COLOR						/* ���顼640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_C80x25_itlace
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_C80x20_itlace
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_C40x25_itlace
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_C40x20_itlace
#include					"screen-vram-full.h"
#undef	COLOR

#define MONO						/* ���	 640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_M80x25_itlace
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_M80x20_itlace
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_M40x25_itlace
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_M40x20_itlace
#include					"screen-vram-full.h"
#undef	MONO

#define UNDISP						/* ��ɽ��640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_U80x25_itlace
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_U80x20_itlace
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_U40x25_itlace
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_U40x20_itlace
#include					"screen-vram-full.h"
#undef	UNDISP

#undef	INTERLACE
/*----------------------------------------------------------------------
 *			�� 400�饤��			ɸ��
 *----------------------------------------------------------------------*/
#define HIRESO						/* ���	 640x400 */

#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_H80x25_normal
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_H80x20_normal
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_ALL			snapshot_H40x25_normal
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_ALL			snapshot_H40x20_normal
#include					"screen-vram-full.h"

#undef	HIRESO

/*===========================================================================
 * ���̾õ�
 *===========================================================================*/

#define		SCREEN_BUF_INIT			snapshot_clear
#include					"screen-vram-clear.h"


#undef	TYPE		/* char */












/* ========================================================================= */
/* ���ܥ����� - ɸ�� */

int  (*snapshot_list_normal[4][4][2])(void) =
{
    {
	{ snapshot_C80x25_normal, snapshot_C80x25_normal },
	{ snapshot_C80x20_normal, snapshot_C80x20_normal },
	{ snapshot_C40x25_normal, snapshot_C40x25_normal },
	{ snapshot_C40x20_normal, snapshot_C40x20_normal },
    },
    {
	{ snapshot_M80x25_normal, snapshot_M80x25_normal },
	{ snapshot_M80x20_normal, snapshot_M80x20_normal },
	{ snapshot_M40x25_normal, snapshot_M40x25_normal },
	{ snapshot_M40x20_normal, snapshot_M40x20_normal },
    },
    {
	{ snapshot_U80x25_normal, snapshot_U80x25_normal },
	{ snapshot_U80x20_normal, snapshot_U80x20_normal },
	{ snapshot_U40x25_normal, snapshot_U40x25_normal },
	{ snapshot_U40x20_normal, snapshot_U40x20_normal },
    },
    {
	{ snapshot_H80x25_normal, snapshot_H80x25_normal },
	{ snapshot_H80x20_normal, snapshot_H80x20_normal },
	{ snapshot_H40x25_normal, snapshot_H40x25_normal },
	{ snapshot_H40x20_normal, snapshot_H40x20_normal },
    },
};

/* ���ܥ����� - �����åץ饤�� */

int  (*snapshot_list_skipln[4][4][2])(void) =
{
    {
	{ snapshot_C80x25_skipln, snapshot_C80x25_skipln },
	{ snapshot_C80x20_skipln, snapshot_C80x20_skipln },
	{ snapshot_C40x25_skipln, snapshot_C40x25_skipln },
	{ snapshot_C40x20_skipln, snapshot_C40x20_skipln },
    },
    {
	{ snapshot_M80x25_skipln, snapshot_M80x25_skipln },
	{ snapshot_M80x20_skipln, snapshot_M80x20_skipln },
	{ snapshot_M40x25_skipln, snapshot_M40x25_skipln },
	{ snapshot_M40x20_skipln, snapshot_M40x20_skipln },
    },
    {
	{ snapshot_U80x25_skipln, snapshot_U80x25_skipln },
	{ snapshot_U80x20_skipln, snapshot_U80x20_skipln },
	{ snapshot_U40x25_skipln, snapshot_U40x25_skipln },
	{ snapshot_U40x20_skipln, snapshot_U40x20_skipln },
    },
    {
	{ snapshot_H80x25_normal, snapshot_H80x25_normal },
	{ snapshot_H80x20_normal, snapshot_H80x20_normal },
	{ snapshot_H40x25_normal, snapshot_H40x25_normal },
	{ snapshot_H40x20_normal, snapshot_H40x20_normal },
    },
};

/* ���ܥ����� - ���󥿡��졼�� */

int  (*snapshot_list_itlace[4][4][2])(void) =
{
    {
	{ snapshot_C80x25_itlace, snapshot_C80x25_itlace },
	{ snapshot_C80x20_itlace, snapshot_C80x20_itlace },
	{ snapshot_C40x25_itlace, snapshot_C40x25_itlace },
	{ snapshot_C40x20_itlace, snapshot_C40x20_itlace },
    },
    {
	{ snapshot_M80x25_itlace, snapshot_M80x25_itlace },
	{ snapshot_M80x20_itlace, snapshot_M80x20_itlace },
	{ snapshot_M40x25_itlace, snapshot_M40x25_itlace },
	{ snapshot_M40x20_itlace, snapshot_M40x20_itlace },
    },
    {
	{ snapshot_U80x25_itlace, snapshot_U80x25_itlace },
	{ snapshot_U80x20_itlace, snapshot_U80x20_itlace },
	{ snapshot_U40x25_itlace, snapshot_U40x25_itlace },
	{ snapshot_U40x20_itlace, snapshot_U40x20_itlace },
    },
    {
	{ snapshot_H80x25_normal, snapshot_H80x25_normal },
	{ snapshot_H80x20_normal, snapshot_H80x20_normal },
	{ snapshot_H40x25_normal, snapshot_H40x25_normal },
	{ snapshot_H40x20_normal, snapshot_H40x20_normal },
    },
};

/* ========================================================================= */
