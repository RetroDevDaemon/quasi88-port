#ifndef MONITOR_H_INCLUDED
#define MONITOR_H_INCLUDED



extern	int	debug_mode;			/* �ǥХå���ǽ(��˥���)  */
extern	char	alt_char;			/* ����ʸ�� */

void	set_signal( void );



#ifdef	USE_MONITOR
void	monitor_init(void);
void	monitor_main(void);
#endif

void print_hankaku(FILE *fp, Uchar *str, char npc);

#endif	/* MONITOR_H_INCLUDED */
