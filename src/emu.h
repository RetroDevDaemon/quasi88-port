#ifndef EMU_H_INCLUDED
#define EMU_H_INCLUDED


extern	int	cpu_timing;			/* SUB-CPU ��ư����	*/
extern	int	select_main_cpu;		/* -cpu 0 �¹Ԥ���CPU	*/
extern	int	dual_cpu_count;			/* -cpu 1 Ʊ������STEP��*/
extern	int	CPU_1_COUNT;			/* ���Ρ������		*/
extern	int	cpu_slice_us;			/* -cpu 2 ������ʬ��(us)*/

extern	int	trace_counter;			/* TRACE ���Υ�����	*/


typedef struct{					/* �֥졼���ݥ�������� */
  short	type;
  word	addr;
} break_t;

typedef struct{					/* FDC �֥졼���ݥ�������� */
  short type;
  short drive;
  short track;
  short sector;
} break_drive_t;

enum BPcpu { BP_MAIN, BP_SUB,                                    EndofBPcpu  };
enum BPtype{ BP_NONE, BP_PC,  BP_READ, BP_WRITE, BP_IN, BP_OUT,  BP_DIAG, 
								 EndofBPtype };

#define	NR_BP			(10)		/* �֥졼���ݥ���Ȥο�   */
#define	BP_NUM_FOR_SYSTEM	(9)		/* �����ƥब�Ȥ�BP���ֹ� */
extern	break_t	break_point[2][NR_BP];
extern  break_drive_t break_point_fdc[NR_BP];









	/**** �ؿ� ****/

void	emu_breakpoint_init( void );
void	emu_reset( void );
void	set_emu_exec_mode( int mode );

void	emu_init(void);
void	emu_main(void);


#endif	/* EMU_H_INCLUDED */
