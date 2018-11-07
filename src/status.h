#ifndef STATUS_H_INCLUDED
#define STATUS_H_INCLUDED


extern	int	status_imagename;		/* ���᡼��̾ɽ��̵ͭ */


typedef struct {
    byte	*pixmap;	/* ���ơ������Υ��᡼���ѥХåե� */
    int		w;		/* ���᡼��ɽ�������� �� 0��	  */
    int		h;		/*                    ��          */
} T_STATUS_INFO;

extern	T_STATUS_INFO	status_info[3];


#define	STATUS_INFO_TIME	(55 * 3)	/* ɸ���ɽ������ ��3�� */
#define	STATUS_WARN_TIME	(55 * 10)	/* �ٹ��ɽ������ ��10�� */


void	status_init(void);
void	status_setup(int show);
void	status_update(void);

void	status_message_default(int pos, const char *msg);
void	status_message(int pos, int frames, const char *msg);


#endif	/* STATUS_H_INCLUDED */
