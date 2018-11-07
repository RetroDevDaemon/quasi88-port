#ifndef PIO_H_INCLUDED
#define PIO_H_INCLUDED


#define PIO_SIDE_M	(0)
#define PIO_SIDE_S	(1)

#define PIO_READ	(0)
#define PIO_WRITE	(1)

#define PIO_PORT_A	(0)
#define PIO_PORT_B	(1)

#define PIO_PORT_CH	(0)
#define PIO_PORT_CL	(1)

#define PIO_EMPTY	(0)
#define PIO_EXIST	(1)

typedef	struct{		
  int	type;	      /* PORT �� ��ǽ   READ �� WRITE ��                     */
  int	exist;	      /* PA/PB �ĥǡ�����̵ͭ				     */
  int	cont_f;	      /* PC Ϣ³���������ե饰 (CPU ���ؤ��ե饰)	     */
  byte	data;	      /* PA/PB �����Ϥ���ǡ��� PC �����Ϥ���/���Ϥ����ǡ��� */
} pio_work;		

extern	pio_work	pio_AB[2][2], pio_C[2][2];



void	pio_init( void );

byte	pio_read_AB( int side, int port );
void	pio_write_AB( int side, int port, byte data );
byte	pio_read_C( int side );
void	pio_write_C( int side, byte data );
void	pio_write_C_direct( int side, byte data );
void	pio_set_mode( int side, byte data );


#endif	/* PIO_H_INCLUDED */
