#ifndef PAUSE_H_INCLUDED
#define PAUSE_H_INCLUDED

/************************************************************************/
/* �����ߥ⡼��							*/
/************************************************************************/

void	pause_init(void);
void	pause_main(void);



/*----------------------------------------------------------------------
 * ���٥�Ƚ������н�
 *----------------------------------------------------------------------*/
void	pause_event_focus_out_when_exec(void);
void	pause_event_focus_in_when_pause(void);
void	pause_event_key_on_esc(void);
void	pause_event_key_on_menu(void);





#endif	/* PAUSE_H_INCLUDED */
