#ifndef SUSPEND_H_INCLUDED
#define SUSPEND_H_INCLUDED

#include <stdio.h>
#include "file-op.h"


#if 0	/* �� file-op.h */
extern char file_state[QUASI88_MAX_FILENAME];	/* ���ơ��ȥե�����̾   */
#endif

extern	int	resume_flag;			/* ��ư���Υ쥸�塼��	*/
extern	int	resume_force;			/* �����쥸�塼��	*/
extern	int	resume_file;			/* �ե�����̾���ꤢ��	*/



/* ���ơ��ȥե�����Υإå����� */

#define	STATE_ID	"QUASI88"		/* ���̻� */
#define	STATE_VER	"0.6.0"			/* �ե�����С������ */
#define	STATE_REV	"1"			/* �ѹ��С������ */




typedef	struct{
  enum {
    TYPE_INT,
    TYPE_LONG,
    TYPE_SHORT,
    TYPE_CHAR,
    TYPE_BYTE,
    TYPE_WORD,
    TYPE_PAIR,
    TYPE_DOUBLE,
    TYPE_256,
    TYPE_STR,
    TYPE_END
  }	type;
  void	*work;
} T_SUSPEND_W;


int	stateload_emu( void );
int	stateload_memory( void );
int	stateload_pc88main( void );
int	stateload_crtcdmac( void );
int	stateload_sound( void );
int	stateload_pio( void );
int	stateload_screen( void );
int	stateload_intr( void );
int	stateload_keyboard( void );
int	stateload_pc88sub( void );
int	stateload_fdc( void );
int	stateload_system( void );

int	statesave_emu( void );
int	statesave_memory( void );
int	statesave_pc88main( void );
int	statesave_crtcdmac( void );
int	statesave_sound( void );
int	statesave_pio( void );
int	statesave_screen( void );
int	statesave_intr( void );
int	statesave_keyboard( void );
int	statesave_pc88sub( void );
int	statesave_fdc( void );
int	statesave_system( void );





void		filename_init_state(int synchronize);
const char	*filename_get_state(void);
int		filename_get_state_serial(void);
void		filename_set_state(const char *filename);
void		filename_set_state_serial(int serial);


void	stateload_init( void );
int	statesave( void );
int	stateload( void );
int	statesave_check_file_exist(void);
int	stateload_check_file_exist(void);

int	statefile_revision( void );

#define	STATE_OK	(0)		/* ����/���������ｪλ */
#define	STATE_ERR	(-1)		/* ����/�����ְ۾ｪλ */
#define	STATE_ERR_ID	(-2)		/* ���ɻ� ID���Ĥ��餺 */
#define	STATE_ERR_SIZE	(-3)		/* ���ɻ� ������������ */
int	statesave_block( const char id[4], void *top, int size );
int	stateload_block( const char id[4], void *top, int size );

int	statesave_table( const char id[4], T_SUSPEND_W *tbl );
int	stateload_table( const char id[4], T_SUSPEND_W *tbl );


#endif	/* SUSPEND_H_INCLUDED */
