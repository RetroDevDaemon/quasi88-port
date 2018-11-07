#ifndef DEVICE_H_INCLUDED
#define DEVICE_H_INCLUDED


#include <gtk/gtk.h>


/*
 *	src/GTK/ �ʲ��ǤΥ����Х��ѿ�
 */
extern	int	gtksys_get_focus;	/* ���ߡ��ե����������꤫�ɤ���	*/



/*
 *	src/GTK/ �ʲ��ǤΥ����Х��ѿ� (���ץ���������ǽ���ѿ�)
 */
extern	int	use_gdk_image;		/* ���ǡ�GdkImage����� */



void	gtksys_set_signal_frame(GtkWidget *main_window);
void	gtksys_set_signal_view(GtkWidget *drawing_area);

void	gtksys_set_attribute_focus_in(void);
void	gktsys_set_attribute_focus_out(void);

void	create_menubar(GtkWidget *target_window,
		       GtkWidget **created_menubar);
void	menubar_setup(int active);



#endif	/* DEVICE_H_INCLUDED */
