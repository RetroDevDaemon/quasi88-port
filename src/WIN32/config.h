#ifndef	CONFIG_H_INCLUDED
#define	CONFIG_H_INCLUDED


/*----------------------------------------------------------------------*/
/* WIN32 ��ͭ�����							*/
/*----------------------------------------------------------------------*/

#include <windows.h>


/* WIN32�� QUASI88 �Τ���μ����� */

#ifndef	QUASI88_WIN32
#define	QUASI88_WIN32
#endif



/* ����ǥ�����ͥ� */

#define	LSB_FIRST



/* ��˥塼�Υ����ȥ롿�С������ɽ���ˤ��ɲä�ɽ��������� (Ǥ�դ�ʸ����) */

#define	Q_COMMENT	"WIN32 port"



/* WIN32�Ǥ� 32bpp(bit per pixel) ����Ȥ��� */

#undef	SUPPORT_8BPP
#undef	SUPPORT_16BPP
#define	SUPPORT_32BPP



/* VC �Υ���饤�󥭡���� */
#define	INLINE		__inline


/* ������ɥɥ饤���Ѥˡ�PI(��)��M_PI(��)����� */
#ifndef PI
#define PI 3.14159265358979323846
#endif
#ifndef	M_PI
#define	M_PI	PI
#endif

#endif	/* CONFIG_H_INCLUDED */
