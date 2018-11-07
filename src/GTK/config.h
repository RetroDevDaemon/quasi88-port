#ifndef	CONFIG_H_INCLUDED
#define	CONFIG_H_INCLUDED


/*----------------------------------------------------------------------*/
/* GTK �С�������ͭ�����						*/
/*----------------------------------------------------------------------*/

#include <gtk/gtk.h>


/* GTK�� QUASI88 �Τ���μ����� */

#ifndef	QUASI88_GTK
#define	QUASI88_GTK
#endif



/* ����ǥ�����ͥ�������å� */

#if	(G_BYTE_ORDER == G_LITTLE_ENDIAN)
#undef	LSB_FIRST
#define	LSB_FIRST
#else
#undef	LSB_FIRST
#endif



/* ��˥塼�Υ����ȥ롿�С������ɽ���ˤ��ɲä�ɽ��������� (Ǥ�դ�ʸ����) */

#define	Q_COMMENT	"GTK port"



/* ���̤� bpp �����������Ǥ�ɤ줫��Ĥ�������ʤ��ƤϤʤ�ʤ� */

#define	SUPPORT_8BPP
#define	SUPPORT_16BPP
#undef	SUPPORT_32BPP






#endif	/* CONFIG_H_INCLUDED */
