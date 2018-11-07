#ifndef	EVENT_H_INCLUDED
#define	EVENT_H_INCLUDED


/***********************************************************************
 * ���٥�Ƚ��� (�����ƥ��¸)
 ************************************************************************/

/******************************************************************************
 * ���٥�Ƚ����ν��������λ
 *
 * void event_init(void)
 *	���祤���ƥ��å������䡢�����ƥ��ͭ�Υ����Х���ǥ��󥰽���������С�
 *	�����Ǥ�äƤ�����
 *	���δؿ��ϡ���ư���� 1������ƤӽФ���롣
 *
 * void event_exit(void)
 *	���٥�Ƚ����θ������Ԥ���
 *	���δؿ��ϡ���λ���� 1������ƤӽФ���롣
 *
 *****************************************************************************/
void	event_init(void);
void	event_exit(void);


/******************************************************************************
 * ���٥�Ƚ����κƽ����
 *
 * void	event_switch(void)
 *	���ߥ塿��˥塼���ݡ�������˥����⡼�ɤ����ػ��˸ƤӽФ���롣
 *	�ƥ⡼�ɤ˱������������ƥ�Υ��٥�Ƚ����ν����򤹤롣
 *	(�������ϥ��٥�Ƚ����䡢��˥塼�С���������Ѥ�����ʤ�)
 *
 *****************************************************************************/
void	event_switch(void);


/******************************************************************************
 * ���٥�Ƚ����μ¹�
 *
 * void event_update(void)
 *	��1/60���� (���ߥ�졼���� VSYNC��) �˸ƤӽФ���롣
 *	�����ǡ����ƤΥ��٥�Ƚ�����Ԥ������ߥ�졼���ؤ����Ϥ򹹿����롣
 *
 *	o ����������������
 *	  �����줿�����˱����ơ��ʲ��δؿ���ƤӽФ���
 *		������: quasi88_key(KEY88_XXX, TRUE);
 *		������: quasi88_key(KEY88_XXX, FALSE);
 *
 *	  ���λ��Ρ������줿������ KEY88_XXX ���б���Ǥ�� (�����ƥ��¸) ������
 *	  �������б��򤷤Ƥ��ʤ� (������ƤƤ��ʤ�) KEY88_XXX �ˤĤ��Ƥϡ�
 *	  QUASI88 �������ԲĤȤʤ롣
 *
 *	  �äˡ�KEY_SPACE �� KEY88_TILDE �������ƤƤ��ʤ��ä���硢��˥塼
 *	  �⡼�ɤ� �����˳������� ASCIIʸ�������Ϥ�����ʤ��ʤ�Τ���ա�
 *
 *	o �ޥ����ܥ��󲡲���������
 *	  �����줿�ܥ���˱����ơ��ʲ��δؿ���ƤӽФ���
 *		������: quasi88_mouse(KEY88_MOUSE_XXX, TRUE);
 *		������: quasi88_mouse(KEY88_MOUSE_XXX, FALSE);
 *
 *	o ���祤���ƥ��å�������������
 *	  �����줿�ܥ���˱����ơ��ʲ��δؿ���ƤӽФ���
 *		������: quasi88_pad(KEY88_PAD_XXX, TRUE);
 *		������: quasi88_pad(KEY88_PAD_XXX, FALSE);
 *
 *	o �ޥ�����ư�� (���к�ɸ / ���к�ɸ)
 *	  ��ư�� x, y (�ʤ�����ư�� dx, xy) �˱����ơ��ʲ��δؿ���ƤӽФ���
 *		��ư��: quasi88_mouse_move( x,  y, TRUE);
 *		��ư��: quasi88_mouse_move(dx, dy, FALSE);
 *
 *	  ���к�ɸ�ξ�硢graph_setup() ������ͤˤƱ������� T_SCREEN_INFO ����
 *	  width * height ���Ф����ͤ򥻥åȤ��뤳�ȡ�(�ͤ��ϰϳ��Ǥ��)
 *
 *	o �ե����������꡿�ʤ���
 *	  ������ɥ����ե�������������(�����ƥ��֤ˤʤä�)���䡢�ե���������
 *	  ���ä�(�󥢥��ƥ��֤ˤʤä�)���˸ƤӽФ���
 *	  �ե��������ʤ����˥ݡ����⡼�ɤ���Ȥ����������ץ���� (-focus) ��
 *	  �����˻ȤäƤ�����ʤΤǡ��������ʤ��Ƥ�ƶ��Ϥʤ���
 *			quasi88_focus_in() / quasi88_focus_out()
 *
 *	o ������λ��
 *	  ������ɥ�������Ū���Ĥ���줿��硢�ʲ��δؿ���ƤӽФ���
 *			quasi88_quit()
 *
 *****************************************************************************/
void	event_update(void);


/******************************************************************************
 * ����¾�λ�¿�ʴؿ�
 *
 * int  event_numlock_on(void)
 * void event_numlock_off(void)
 *	���եȥ����� NumLock ��ͭ����̵���ˤ���ݤˡ��ƤӽФ���롣
 *	�ɤΥ����򲡤����顢�ɤΥƥ󥭡��򲡤������Ȥˤʤ�Τ��ϡ������ƥ��¸
 *	�Ȥ���Τǡ������ƥ�Υ�������˹�碌�ơ��������롣
 *	�ѹ��Ǥ��ʤ��ʤ�С� event_numlock_on() ������ͤ� �� �ˤ��롣
 *
 * void	event_get_mouse_pos(int *x, int *y)
 *	���ߤΥޥ��������к�ɸ�򥻥åȤ��롣
 *	���к�ɸ�γ�ǰ���ʤ����ϡ�Ǥ�դ��ͤ򥻥åȤ��Ƥ�褤��
 *	���åȤ����ɸ�ͤϡ�graph_setup() ������ͤˤƱ������� T_SCREEN_INFO ��
 *	�� width * height ���Ф����ͤȤ��롣(�ͤ��ϰϳ��Ǥ��)
 *	���δؿ��ϡ��⡼�ɤ��ڤ��ؤ����˸ƤӽФ���롣
 *
 * int	event_get_joystick_num(void)
 *	���Ѳ�ǽ�ʥ��祤���ƥ��å��ο� (0��2) ���֤���
 *
 *****************************************************************************/
int	event_numlock_on(void);
void	event_numlock_off(void);

void	event_get_mouse_pos(int *x, int *y);

int	event_get_joystick_num(void);




/******************************************************************************
 * �ʲ��ϡ��嵭�Υ����ƥ��¸�ʴؿ���ꡢ�ƤӽФ����ؿ�
 *		���Τ�������꤯�������ʤ��Ƥϡ�
 *****************************************************************************/

/*----------------------------------------------------------------------
 * ���٥�Ƚ������н�
 *----------------------------------------------------------------------*/
void	quasi88_key  (int code, int on_flag);
void	quasi88_mouse(int code, int on_flag);
void	quasi88_pad  (int code, int on_flag);
void	quasi88_mouse_move(int x, int y, int abs_flag);

#define	quasi88_key_pressed(code)	quasi88_key  (code, TRUE )
#define	quasi88_key_released(code)	quasi88_key  (code, FALSE)
#define	quasi88_mouse_pressed(code)	quasi88_mouse(code, TRUE )
#define	quasi88_mouse_released(code)	quasi88_mouse(code, FALSE)
#define	quasi88_pad_pressed(code)	quasi88_pad  (code, TRUE )
#define	quasi88_pad_released(code)	quasi88_pad  (code, FALSE)
#define	quasi88_mouse_moved_abs(x, y)	quasi88_mouse_move(x, y, TRUE)
#define	quasi88_mouse_moved_rel(x, y)	quasi88_mouse_move(x, y, FALSE)



void	quasi88_expose(void);
void	quasi88_focus_in(void);
void	quasi88_focus_out(void);


/* �ʲ��ϡ������¸��档�ƤӽФ��˾�郎����Ǥ���� */

typedef struct {
    int		boot_basic;
    int		boot_dipsw;
    int		boot_from_rom;
    int		boot_clock_4mhz;
    int		set_version;
    byte	baudrate_sw;
    int		use_extram;
    int		use_jisho_rom;
    int		sound_board;
} T_RESET_CFG;
void	quasi88_get_reset_cfg(T_RESET_CFG *cfg);
void	quasi88_reset(const T_RESET_CFG *cfg);
int	quasi88_stateload(int serial);
int	quasi88_statesave(int serial);
int	quasi88_screen_snapshot(void);
int	quasi88_waveout(int start);
int	quasi88_drag_and_drop(const char *filename);


int	quasi88_cfg_now_wait_rate(void);
void	quasi88_cfg_set_wait_rate(int rate);
int	quasi88_cfg_now_no_wait(void);
void	quasi88_cfg_set_no_wait(int enable);


int	quasi88_disk_insert_all(const char *filename, int ro);
int	quasi88_disk_insert(int drv, const char *filename, int image, int ro);
int	quasi88_disk_insert_A_to_B(int src_drv, int dst_drv, int dst_img);
void	quasi88_disk_eject_all(void);
void	quasi88_disk_eject(int drv);

void	quasi88_disk_image_select(int drv, int img);
void	quasi88_disk_image_empty(int drv);
void	quasi88_disk_image_next(int drv);
void	quasi88_disk_image_prev(int drv);

int	quasi88_load_tape_insert( const char *filename );
int	quasi88_load_tape_rewind( void );
void	quasi88_load_tape_eject( void );
int	quasi88_save_tape_insert( const char *filename );
void	quasi88_save_tape_eject( void );

int	quasi88_serial_in_connect( const char *filename );
void	quasi88_serial_in_remove( void );
int	quasi88_serial_out_connect( const char *filename );
void	quasi88_serial_out_remove( void );
int	quasi88_printer_connect( const char *filename );
void	quasi88_printer_remove( void );



#endif	/* EVENT_H_INCLUDED */
