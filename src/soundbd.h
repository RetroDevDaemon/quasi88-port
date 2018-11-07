#ifndef SOUND_H_INCLUDED
#define SOUND_H_INCLUDED


extern	int	sound_board;			/* ������ɥܡ���	*/

#define	SD_PORT_44_45	(0x01)			/* �ݡ��� 44H��45H ����	*/
#define	SD_PORT_46_47	(0x02)			/* �ݡ��� 46H��47H ����	*/
#define	SD_PORT_A8_AD	(0x04)			/* �ݡ��� A8H��ADH ����	*/
extern	int	sound_port;			/* ������ɥݡ��Ȥμ���	*/

extern	int	intr_sound_enable;		/*         ����ޥ������� */




extern	int	sound_ENABLE_A;			/* ������ɥ����ޡ����ľ��� */
extern	int	sound_ENABLE_B;
extern	int	sound_LOAD_A;			/* ������ɥ����ޡ���ư���� */
extern	int	sound_LOAD_B;
extern	int	sound_FLAG_A;			/* FLAG �ξ���		*/
extern	int	sound_FLAG_B;

extern	int	sound_TIMER_A;			/* ������ɥ����ޡ�����ֳ� */
extern	int	sound_TIMER_B;

extern	int	sound_prescaler;		/* 1/�ץꥹ�����顼 (2,3,6) */

extern	byte	sound_reg[0x100];
extern	int	sound_reg_select;


extern	int	sound2_MSK_TA;		/* TIMER A �����ߥޥ���	*/
extern	int	sound2_MSK_TB;		/* TIMER B �����ߥޥ���	*/
extern	int	sound2_MSK_EOS;		/* EOS     �����ߥޥ���	*/ 
extern	int	sound2_MSK_BRDY;	/* BRDY    �����ߥޥ���	*/ 
extern	int	sound2_MSK_ZERO;	/* ZERO    �����ߥޥ���	*/ 

extern	int	sound2_EN_TA;		/* TIMER A �����ߵ���		*/
extern	int	sound2_EN_TB;		/* TIMER B �����ߵ���		*/
extern	int	sound2_EN_EOS;		/* EOS     �����ߵ���		*/
extern	int	sound2_EN_BRDY;		/* BDRY    �����ߵ���		*/
extern	int	sound2_EN_ZERO;		/* ZERO    �����ߵ���		*/

extern	int	sound2_FLAG_EOS;	/* FLAG EOS  �ξ���		*/
extern	int	sound2_FLAG_BRDY;	/* FLAG BRDY �ξ���		*/
extern	int	sound2_FLAG_ZERO;	/* FLAG ZERO �ξ���		*/
extern	int	sound2_FLAG_PCMBSY;	/* FLAG PCMBSY �ξ���		*/

extern	byte	sound2_reg[0x100];
extern	int	sound2_reg_select;
extern	byte	*sound2_adpcm;			/* ADPCM�� DRAM (256KB)	*/

extern	int	sound2_repeat;			/* ADPCM ��ԡ��ȥץ쥤	*/
extern	int	sound2_intr_base;		/* ADPCM �����ߥ졼��	*/
extern	int	sound2_notice_EOS;		/* EOS�����å���������	*/



extern	int	use_cmdsing;			/* ���ǡ�CMD SINGͭ��	*/



void	sound_board_init( void );

void	sound_out_reg( byte data );
void	sound_out_data( byte data );
byte	sound_in_status( void );
byte	sound_in_data( int always_sound_II );
void	sound2_out_reg( byte data );
void	sound2_out_data( byte data );
byte	sound2_in_status( void );
byte	sound2_in_data( void );


void	sound_output_after_stateload( void );

#endif	/* SOUND_H_INCLUDED */
