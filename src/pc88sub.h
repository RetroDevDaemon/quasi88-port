#ifndef PC88SUB_H_INCLUDED
#define PC88SUB_H_INCLUDED


	/**** �ѿ� ****/

extern	int	sub_load_rate;


	/**** �ؿ� ****/


void	pc88sub_init( int init );
void	pc88sub_term( void );
void	pc88sub_bus_setup( void );


byte	sub_mem_read( word addr );
void	sub_mem_write( word addr, byte data );
byte	sub_io_in( byte port );
void	sub_io_out( byte port, byte data );


void	subcpu_keyscan_draw( void );


#endif	/* PC88SUB_H_INCLUDED */
