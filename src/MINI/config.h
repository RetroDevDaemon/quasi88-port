#ifndef	CONFIG_H_INCLUDED
#define	CONFIG_H_INCLUDED


/*----------------------------------------------------------------------*/
/* �����ƥ��ͭ�����							*/
/*----------------------------------------------------------------------*/

/* �����ƥ��ͭ�Υ��󥯥롼�ɥե����� */
/* #include <XXXXX.h> */


/* ���Υ����ƥ����Ѥ� QUASI88 �Τ���μ����� (ɬ�פʤ�) */

#ifndef	QUASI88_MINI
#define	QUASI88_MINI
#endif



/* ����ǥ�����ͥ��������������ʤ���С��ӥå�����ǥ�����ˤʤ� */

#define	LSB_FIRST



/* ��˥塼�Υ����ȥ롿�С������ɽ���ˤ��ɲä�ɽ��������� (Ǥ�դ�ʸ����) */

#define	Q_COMMENT	"MINI port"



/* ���̤� bpp �����������Ǥ�ɤ줫��Ĥ�������ʤ��ƤϤʤ�ʤ� */

#define	SUPPORT_8BPP
#define	SUPPORT_16BPP
#define	SUPPORT_32BPP



/* ����饤�󥭡���ɤ����������饤������ԲĤʤ顢���ΤޤޤǤ褤 */
#define	INLINE		static



#if	0

/* ������ɽ��Ϥ��б������� (MAME�١���) */
#define	USE_SOUND

/* ����ˡ� fmgen �ˤ��б������� */
#define	USE_FMGEN

/* ����ѥ���ˤ�äƤϡ�PI(��) �� M_PI(��)�������ɬ�פ��⤷��ʤ� */
#ifndef PI
#define PI 3.14159265358979323846
#endif
#ifndef	M_PI
#define	M_PI	PI
#endif

#endif

#endif	/* CONFIG_H_INCLUDED */
