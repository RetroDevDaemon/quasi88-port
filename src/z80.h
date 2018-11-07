#ifndef Z80_H_INCLUDED
#define Z80_H_INCLUDED



/* --- Z80 CPU �Υ��ߥ�졼�ȹ�¤�� --- */

typedef struct{

  pair	AF, BC, DE, HL;			/* ���ѥ쥸���� */
  pair	IX, IY, PC, SP;			/* ���ѥ쥸���� */
  pair	AF1,BC1,DE1,HL1;		/*  ΢ �쥸����	*/
  byte	I, R;				/* �ü�쥸���� */
  byte	R_saved;			/* R reg - bit7 ��¸�� */
  Uchar	IFF,IFF2;			/* IFF1 ��IFF2	*/
  Uchar	IM;				/* ����⡼��	*/
  Uchar	HALT;				/* HALT �ե饰	*/

  int	INT_active;
  int	icount;				/* ���γ��ȯ���ޤǤΥ��ơ��ȿ�	*/
  int	state0;				/* �����������ơ��ȿ�	*/
					/* (z80->intr_update)()�ƽл��˽����*/

  int	skip_intr_chk;

  Uchar	log;				/* ���ʤ�ǥХå��ѤΥ���Ͽ	*/
  Uchar	break_if_halt;			/* HALT���˽����롼�פ��鶯��æ��*/

  byte	(*fetch)(word);
  byte	(*mem_read)(word);		/* ����꡼�ɴؿ�	*/
  void	(*mem_write)(word,byte);	/* ����饤�ȴؿ�	*/
  byte	(*io_read)(byte);		/* I/O ���ϴؿ�		*/
  void	(*io_write)(byte,byte);		/* I/O ���ϴؿ�		*/

  void	(*intr_update)(void);		/* ������󹹿��ؿ�	*/
  int	(*intr_ack)(void);		/* ��������ؿ�		*/

  pair  PC_prev;			/* ľ���� PC (��˥���)	*/

} z80arch;


/* IFF ����� */
#define INT_DISABLE	(0)
#define INT_ENABLE	(1)



/*------------------------------------------------------------------------
 * ���߽������ CPU ��ư�����Ū�˻ߤ�뤿��Υޥ���
 *------------------------------------------------------------------------*/

extern	int z80_state_goal;	/* ����state��ʬ�������򷫤��֤�(0��̵��) */
extern	int z80_state_intchk;	/* ����state���¹Ը塢���Ƚ�ꤹ��	  */


/* PIO����ˤ��CPU���ػ��䡢��˥塼���ܻ��ʤɤˡ�CPU��������Ū����� */
   
#define	CPU_BREAKOFF()	do{ z80_state_intchk = 0; z80_state_goal = 1; }while(0)

/* ���ȯ������ѹ����ˡ�CPU��������Ū�� �������������ʬ�� */

#define	CPU_REFRESH_INTERRUPT()		do{ z80_state_intchk = 0; }while(0)




extern	void	z80_reset( z80arch *z80 );
extern	int	z80_emu( z80arch *z80, int state_of_exec );
extern	void	z80_debug( z80arch *z80, char *mes );
extern	int	z80_line_disasm( z80arch *z80, word addr );

extern	void	z80_logging( z80arch *z80 );



#endif		/* Z80_H_INCLUDED */
