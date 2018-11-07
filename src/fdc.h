#ifndef FDC_H_INCLUDED
#define FDC_H_INCLUDED


extern int fdc_debug_mode;	/* FDC �ǥХå��⡼�ɤΥե饰		*/
extern int disk_exchange;	/* �ǥ��������������ؤ��ե饰		*/
extern int disk_ex_drv;		/* �ǥ��������������ؤ��ɥ饤��		*/

extern	int	FDC_flag;			/* FDC �����߿���	*/
extern	int	fdc_wait;			/* FDC �� ��������	*/

extern	int	fdc_ignore_readonly;	/* �ɹ����ѻ����饤�Ȥ�̵�뤹��	*/



int	fdc_ctrl( int interval );

void	fdc_write( byte data );
byte	fdc_read( void );
byte	fdc_status( void );
void	fdc_TC( void );


void pc88fdc_break_point(void);

#endif	/* FDC_H_INCLUDED */
