/************************************************************************/
/*									*/
/* ��˥塼�⡼��							*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "quasi88.h"
#include "initval.h"
#include "menu.h"

#include "pc88main.h"
#include "pc88sub.h"
#include "graph.h"
#include "intr.h"
#include "keyboard.h"
#include "memory.h"
#include "screen.h"

#include "emu.h"
#include "drive.h"
#include "image.h"
#include "status.h"
#include "monitor.h"
#include "snddrv.h"
#include "wait.h"
#include "file-op.h"
#include "suspend.h"
#include "snapshot.h"
#include "fdc.h"
#include "soundbd.h"
#include "getconf.h"

#include "event.h"
#include "q8tk.h"



int	menu_lang	= MENU_JAPAN;		/* ��˥塼�θ���           */
int	menu_readonly	= FALSE;		/* �ǥ������������������ */
						/* ������֤� ReadOnly ?    */
int	menu_swapdrv	= FALSE;		/* �ɥ饤�֤�ɽ�����       */



/* ������Хå��ؿ��ΰ��� (Q8tkWidget*, void*) ��̤���Ѥξ�硢
 * ��˥󥰤��Ф�ݵƫ�����Τǡ� gcc �����ǵ����Ƥ�餦�� */
#if defined(__GNUC__)
#define	UNUSED_WIDGET	__attribute__((__unused__)) Q8tkWidget *dummy_0
#define	UNUSED_PARM	__attribute__((__unused__)) void *dummy_1
#else
#define	UNUSED_WIDGET	Q8tkWidget *dummy_0
#define	UNUSED_PARM	void *dummy_1
#endif



/*--------------------------------------------------------------*/
/* ��˥塼�Ǥ�ɽ����å����������ơ����Υե���������		*/
/*--------------------------------------------------------------*/
#include "message.h"




/****************************************************************/
/* ���							*/
/****************************************************************/

static	int	menu_last_page = 0;	/* ������Υ�˥塼�����򵭲� */

static	int	menu_boot_clock_async;	/* �ꥻ�åȻ��˥���å�����Ʊ������? */

static	T_RESET_CFG	reset_req;	/* �ꥻ�åȻ����׵᤹���������¸ */

/* ��˥塼��λ���ˡ�������ɤδĶ����Ѥ�äƤʤ�����ǧ�Τ��ᡢ�������¸ */
#define	NR_SD_CFG_LOCAL	(5)
typedef union {
    int		i;
    float	f;
} SD_CFG_LOCAL_VAL;

static struct {

    int		sound_board;
    int		use_fmgen;
    int		sample_freq;
    int		use_samples;

    /* �ʲ��������ƥ��¸������ */
    int		local_cnt;
    struct {
	T_SNDDRV_CONFIG		*info;
	SD_CFG_LOCAL_VAL	val;
    } local[ NR_SD_CFG_LOCAL ];

} sd_cfg_init, sd_cfg_now;
static	void	sd_cfg_save(void);
static	int	sd_cfg_has_changed(void);


static	int	cpu_timing_save;	/* ��˥塼���ϻ��� -cpu �� ���� */

					/* ��ư�ǥХ����������ɬ�� */
static	Q8tkWidget	*widget_reset_boot;
static	Q8tkWidget	*widget_dipsw_b_boot_disk;
static	Q8tkWidget	*widget_dipsw_b_boot_rom;

static	Q8tkWidget	*menu_accel;	/* �ᥤ���˥塼�Υ������ */

/* ��˥塼�����Υꥻ�åȤȡ��ꥻ�åȥ����Υꥻ�åȤ�Ϣư���������Τǡ�
   ���������򤵤줿�顢ȿ�ФΤ����򤵤��褦�ˡ��������åȤ򵭲�         */
static	Q8tkWidget	*widget_reset_basic[2][4];
static	Q8tkWidget	*widget_reset_clock[2][2];



/*===========================================================================
 * �ե��������顼��å������Υ�����������
 *===========================================================================*/
static	void	cb_file_error_dialog_ok(UNUSED_WIDGET, UNUSED_PARM)
{
    dialog_destroy();
}

static	void	start_file_error_dialog(int drv, int result)
{
    char wk[128];
    const t_menulabel *l = (drv<0) ? data_err_file : data_err_drive;

    if (result == ERR_NO) return;
    if (drv < 0) sprintf(wk, GET_LABEL(l, result));
    else         sprintf(wk, GET_LABEL(l, result), drv+1);

    dialog_create();
    {
	dialog_set_title(wk);
	dialog_set_separator();
	dialog_set_button(GET_LABEL(l, ERR_NO),
			  cb_file_error_dialog_ok, NULL);
    }
    dialog_start();
}

/*===========================================================================
 * �ǥ��������� & �ӽ�
 *===========================================================================*/
static void sub_misc_suspend_update(void);
static void sub_misc_snapshot_update(void);
static void sub_misc_waveout_update(void);

/*===========================================================================
 *
 *	�ᥤ��ڡ���	�ꥻ�å�
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
						 /* BASIC�⡼���ڤ��ؤ� */
static	int	get_reset_basic(void)
{
    return reset_req.boot_basic;
}
static	void	cb_reset_basic(UNUSED_WIDGET, void *p)
{
    if (reset_req.boot_basic != (int)p) {
	reset_req.boot_basic = (int)p;

	q8tk_toggle_button_set_state(widget_reset_basic[ 1 ][ (int)p ], TRUE);
    }
}


static	Q8tkWidget	*menu_reset_basic(void)
{
    Q8tkWidget *box;
    Q8List     *list;

    box = PACK_VBOX(NULL);
    {
	list = PACK_RADIO_BUTTONS(box,
				  data_reset_basic, COUNTOF(data_reset_basic),
				  get_reset_basic(), cb_reset_basic);

	/* �ꥹ�Ȥ�귫�äơ����������åȤ���� */
	widget_reset_basic[0][BASIC_V2 ] = list->data;	list = list->next;
	widget_reset_basic[0][BASIC_V1H] = list->data;	list = list->next;
	widget_reset_basic[0][BASIC_V1S] = list->data;	list = list->next;
	widget_reset_basic[0][BASIC_N  ] = list->data;
    }

    return box;
}

/*----------------------------------------------------------------------*/
						       /* CLOCK�ڤ��ؤ� */
static	int	get_reset_clock(void)
{
    return reset_req.boot_clock_4mhz;
}
static	void	cb_reset_clock(UNUSED_WIDGET, void *p)
{
    if (reset_req.boot_clock_4mhz != (int)p) {
	reset_req.boot_clock_4mhz = (int)p;

	q8tk_toggle_button_set_state(widget_reset_clock[ 1 ][ (int)p ], TRUE);
    }
}
static	int	get_reset_clock_async(void)
{
    return menu_boot_clock_async;
}
static	void	cb_reset_clock_async(Q8tkWidget *widget, UNUSED_PARM)
{
    int async = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;
    menu_boot_clock_async = async;
}


static	Q8tkWidget	*menu_reset_clock(void)
{
    Q8tkWidget *box, *box2;
    Q8List     *list;

    box = PACK_VBOX(NULL);
    {
	list = PACK_RADIO_BUTTONS(box,
				  data_reset_clock, COUNTOF(data_reset_clock),
				  get_reset_clock(), cb_reset_clock);

	/* �ꥹ�Ȥ�귫�äơ����������åȤ���� */
	widget_reset_clock[0][CLOCK_4MHZ] = list->data;	list = list->next;
	widget_reset_clock[0][CLOCK_8MHZ] = list->data;

	PACK_LABEL(box, "");			/* ���� */

	box2 = PACK_HBOX(box);
	{
	    PACK_LABEL(box2, "  ");		/* ����ǥ�� */
	    PACK_CHECK_BUTTON(box2,
			      GET_LABEL(data_reset_clock_async, 0),
			      get_reset_clock_async(),
			      cb_reset_clock_async, NULL);
	}
    }

    return box;
}

/*----------------------------------------------------------------------*/
					      /* ������ɥܡ����ڤ��ؤ� */
static	int	get_reset_sound(void)
{
    return reset_req.sound_board;
}
static	void	cb_reset_sound(UNUSED_WIDGET, void *p)
{
    reset_req.sound_board = (int)p;
}


static	Q8tkWidget	*menu_reset_sound(void)
{
    Q8tkWidget *box;

    box = PACK_VBOX(NULL);
    {
	PACK_RADIO_BUTTONS(box,
			   data_reset_sound, COUNTOF(data_reset_sound),
			   get_reset_sound(), cb_reset_sound);
	PACK_LABEL(box, "");			/* ���� */
	PACK_LABEL(box, "");			/* ���� */
    }

    return box;
}

/*----------------------------------------------------------------------*/
								/* ��ư */
static	void	set_reset_dipsw_boot(void)
{
    const t_menulabel *l = data_reset_boot;

    if (widget_reset_boot) {
	q8tk_label_set(widget_reset_boot,
		       (reset_req.boot_from_rom ? GET_LABEL(l, 1)
						: GET_LABEL(l, 0)));
    }
}


static	Q8tkWidget	*menu_reset_boot(void)
{
    Q8tkWidget *vbox;

    vbox = PACK_VBOX(NULL);
    {
	widget_reset_boot = PACK_LABEL(vbox, "");
	set_reset_dipsw_boot();
    }
    return vbox;
}

/*----------------------------------------------------------------------*/
								/* �ܺ� */
static	Q8tkWidget	*reset_detail_widget;
static	int		reset_detail_hide;
static	Q8tkWidget	*reset_detail_button;
static	void	cb_reset_detail(UNUSED_WIDGET, UNUSED_PARM)
{
    reset_detail_hide ^= 1;
  
    if (reset_detail_hide) {
	q8tk_widget_hide(reset_detail_widget);
    } else {
	q8tk_widget_show(reset_detail_widget);
    }
    q8tk_label_set(reset_detail_button->child,
		   GET_LABEL(data_reset_detail, reset_detail_hide));
}


static	Q8tkWidget	*menu_reset_detail(void)
{
    Q8tkWidget *box;

    box = PACK_VBOX(NULL);
    {
	PACK_LABEL(box, "");

	reset_detail_hide = 0;
	reset_detail_button = PACK_BUTTON(box,
					  "",
					  cb_reset_detail, NULL);
	cb_reset_detail(NULL, NULL);

	PACK_LABEL(box, "");
    }

    return box;
}

/*----------------------------------------------------------------------*/
						/* �ǥ��åץ����å����� */
static	void	dipsw_create(void);
static	void	dipsw_start(void);
static	void	dipsw_finish(void);

static	void	cb_reset_dipsw(UNUSED_WIDGET, UNUSED_PARM)
{
    dipsw_start();
}


static	Q8tkWidget	*menu_reset_dipsw(void)
{
    Q8tkWidget *button;

    button = PACK_BUTTON(NULL,
			 GET_LABEL(data_reset, DATA_RESET_DIPSW_BTN),
			 cb_reset_dipsw, NULL);
    q8tk_misc_set_placement(button, Q8TK_PLACEMENT_X_CENTER, 0);

    return button;
}

/*----------------------------------------------------------------------*/
						  /* �С�������ڤ��ؤ� */
static	int	get_reset_version(void)
{
    return reset_req.set_version;
}
static	void	cb_reset_version(Q8tkWidget *widget, UNUSED_PARM)
{
    int i;
    const t_menudata *p = data_reset_version;
    const char       *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(data_reset_version); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    reset_req.set_version = p->val;
	    break;
	}
    }
}


static	Q8tkWidget	*menu_reset_version(void)
{
    Q8tkWidget *box, *combo;
    char wk[4];

    wk[0] = get_reset_version();
    wk[1] = '\0';

    box = PACK_VBOX(NULL);
    {
	combo = PACK_COMBO(box,
			   data_reset_version, COUNTOF(data_reset_version),
			   get_reset_version(), wk, 8,
			   cb_reset_version, NULL,
			   NULL, NULL);
	PACK_LABEL(box, "");			/* ���� */
    }

    return box;
}

/*----------------------------------------------------------------------*/
						  /* ��ĥ�����ڤ��ؤ� */
static	int	get_reset_extram(void)
{
    return reset_req.use_extram;
}
static	void	cb_reset_extram(Q8tkWidget *widget, UNUSED_PARM)
{
    int i;
    const t_menudata *p = data_reset_extram;
    const char       *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(data_reset_extram); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    reset_req.use_extram = p->val;
	    break;
	}
    }
}


static	Q8tkWidget	*menu_reset_extram(void)
{
    Q8tkWidget *box, *combo;
    char wk[16];

    sprintf(wk, "  %5dKB", use_extram * 128);

    box = PACK_VBOX(NULL);
    {
	combo = PACK_COMBO(box,
			   data_reset_extram, COUNTOF(data_reset_extram),
			   get_reset_extram(), wk, 0,
			   cb_reset_extram, NULL,
			   NULL, NULL);
	PACK_LABEL(box, "");			/* ���� */
    }

    return box;
}

/*----------------------------------------------------------------------*/
						     /* ����ROM�ڤ��ؤ� */
static	int	get_reset_jisho(void)
{
    return reset_req.use_jisho_rom;
}
static	void	cb_reset_jisho(UNUSED_WIDGET, void *p)
{
    reset_req.use_jisho_rom = (int)p;
}


static	Q8tkWidget	*menu_reset_jisho(void)
{
    Q8tkWidget *box;

    box = PACK_VBOX(NULL);
    {
	PACK_RADIO_BUTTONS(box,
			   data_reset_jisho, COUNTOF(data_reset_jisho),
			   get_reset_jisho(), cb_reset_jisho);
    }

    return box;
}

/*----------------------------------------------------------------------*/
						   /* ���ߤ�BASIC�⡼�� */
static	Q8tkWidget	*menu_reset_current(void)
{
    static const char *type[] = {
	"PC-8801",
	"PC-8801",
	"PC-8801",
	"PC-8801mkII",
	"PC-8801mkIISR",
	"PC-8801mkIITR/FR/MR",
	"PC-8801mkIITR/FR/MR",
	"PC-8801mkIITR/FR/MR",
	"PC-8801FH/MH",
	"PC-8801FA/MA/FE/MA2/FE2/MC",
    };
    static const char *basic[] = { " N ", "V1S", "V1H", " V2", };
    static const char *clock[] = { "8MHz", "4MHz", };
    const char *t = "";
    const char *b = "";
    const char *c = "";
    int i;
    char wk[80], ext[40];

    i = (ROM_VERSION & 0xff) - '0';
    if (0 <= i && i< COUNTOF(type)) t = type[ i ];

    i = get_reset_basic();
    if (0 <= i && i< COUNTOF(basic)) b = basic[ i ];

    i = get_reset_clock();
    if (0 <= i && i< COUNTOF(clock)) c = clock[ i ];

    ext[0] = 0;
    {
	if (sound_port) {
	    if (ext[0] == 0) strcat(ext, "(");
	    else             strcat(ext, ", ");
	    if (sound_board == SOUND_I) strcat(ext, "OPN");
	    else                        strcat(ext, "OPNA");
	}

	if (use_extram) {
	    if (ext[0] == 0) strcat(ext, "(");
	    else             strcat(ext, ", ");
	    sprintf(wk, "%dKB", use_extram * 128);
	    strcat(ext, wk);
	    strcat(ext, GET_LABEL(data_reset_current,0));/* ExtRAM*/
	}

	if (use_jisho_rom) {
	    if (ext[0] == 0) strcat(ext, "(");
	    else             strcat(ext, ", ");
	    strcat(ext, GET_LABEL(data_reset_current,1));/*DictROM*/
	}
    }
    if (ext[0]) strcat(ext, ")");


    sprintf(wk, " %-30s  %4s  %4s  %30s ",
	    t, b, c, ext);

    return PACK_LABEL(NULL, wk);
}

/*----------------------------------------------------------------------*/
							    /* �ꥻ�å� */
static	void	cb_reset_now(UNUSED_WIDGET, UNUSED_PARM)
{
    /* CLOCK����ȡ�CPU����å���Ʊ�������� */
    if (menu_boot_clock_async == FALSE) {
	cpu_clock_mhz = reset_req.boot_clock_4mhz ? CONST_4MHZ_CLOCK
						  : CONST_8MHZ_CLOCK;
    }

    /* reset_req ������˴�Ť����ꥻ�å� �� �¹� */
    quasi88_reset(&reset_req);

    quasi88_exec();		/* �� q8tk_main_quit() �ƽкѤ� */

#if 0
    printf("boot_dipsw      %04x\n",boot_dipsw   );
    printf("boot_from_rom   %d\n",boot_from_rom  );
    printf("boot_basic      %d\n",boot_basic     );
    printf("boot_clock_4mhz %d\n",boot_clock_4mhz);
    printf("ROM_VERSION     %c\n",ROM_VERSION    );
    printf("baudrate_sw     %d\n",baudrate_sw    );
#endif
}

/*======================================================================*/

static	Q8tkWidget	*menu_reset(void)
{
    Q8tkWidget *hbox, *vbox;
    Q8tkWidget *w, *f;
    const t_menulabel *l = data_reset;

    dipsw_create();		/* �ǥ��åץ����å�������ɥ����� */

    vbox = PACK_VBOX(NULL);
    {
	f = PACK_FRAME(vbox, "", menu_reset_current());
	q8tk_frame_set_shadow_type(f, Q8TK_SHADOW_ETCHED_OUT);

	hbox = PACK_HBOX(vbox);
	{
	    PACK_FRAME(hbox,
		       GET_LABEL(l, DATA_RESET_BASIC), menu_reset_basic());

	    PACK_FRAME(hbox,
		       GET_LABEL(l, DATA_RESET_CLOCK), menu_reset_clock());

	    PACK_FRAME(hbox,
		       GET_LABEL(l, DATA_RESET_SOUND), menu_reset_sound());

	    w = PACK_FRAME(hbox,
			   GET_LABEL(l, DATA_RESET_BOOT), menu_reset_boot());
	    q8tk_frame_set_shadow_type(w, Q8TK_SHADOW_ETCHED_IN);
	}

	PACK_LABEL(vbox, GET_LABEL(l, DATA_RESET_NOTICE));

	hbox = PACK_HBOX(vbox);
	{
	    reset_detail_widget = PACK_HBOX(NULL);

	    q8tk_box_pack_start(hbox, menu_reset_detail());
	    q8tk_box_pack_start(hbox, reset_detail_widget);
	    {
		PACK_VSEP(reset_detail_widget);

		f = PACK_FRAME(reset_detail_widget,
		       GET_LABEL(l, DATA_RESET_DIPSW), menu_reset_dipsw());

		q8tk_misc_set_placement(f, 0, Q8TK_PLACEMENT_Y_CENTER);

		f = PACK_FRAME(reset_detail_widget,
		       GET_LABEL(l, DATA_RESET_VERSION), menu_reset_version());

		q8tk_misc_set_placement(f, 0, Q8TK_PLACEMENT_Y_BOTTOM);

		f = PACK_FRAME(reset_detail_widget,
		       GET_LABEL(l, DATA_RESET_EXTRAM), menu_reset_extram());

		q8tk_misc_set_placement(f, 0, Q8TK_PLACEMENT_Y_BOTTOM);

		f = PACK_FRAME(reset_detail_widget,
		       GET_LABEL(l, DATA_RESET_JISHO), menu_reset_jisho());

		q8tk_misc_set_placement(f, 0, Q8TK_PLACEMENT_Y_BOTTOM);
	    }

	}

	hbox = PACK_HBOX(vbox);
	{
	    w = PACK_LABEL(hbox, GET_LABEL(l, DATA_RESET_INFO));
	    q8tk_misc_set_placement(w, 0, Q8TK_PLACEMENT_Y_CENTER);

	    w = PACK_BUTTON(hbox,
			    GET_LABEL(data_reset, DATA_RESET_NOW),
			    cb_reset_now, NULL);
	}
	q8tk_misc_set_placement(hbox, Q8TK_PLACEMENT_X_RIGHT, 0);
    }

    return vbox;
}



/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 
 *
 *	���֥�����ɥ�	DIPSW
 *
 * = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

static	Q8tkWidget	*dipsw_window;
static	Q8tkWidget	*dipsw[4];
static	Q8tkWidget	*dipsw_accel;

enum {
    DIPSW_WIN,
    DIPSW_FRAME,
    DIPSW_VBOX,
    DIPSW_QUIT
};

/*----------------------------------------------------------------------*/
					    /* �ǥ��åץ����å��ڤ��ؤ� */
static	int	get_dipsw_b(int p)
{
    int shift = data_dipsw_b[p].val;

    return ((p<<1) | ((reset_req.boot_dipsw >> shift) & 1));
}
static	void	cb_dipsw_b(UNUSED_WIDGET, void *p)
{
    int shift = data_dipsw_b[ (int)p >> 1 ].val;
    int on    = (int)p & 1;

    if (on) reset_req.boot_dipsw |=  (1 << shift);
    else    reset_req.boot_dipsw &= ~(1 << shift);
}
static	int	get_dipsw_b2(void)
{
    return (reset_req.boot_from_rom ? TRUE : FALSE);
}
static	void	cb_dipsw_b2(UNUSED_WIDGET, void *p)
{
    if ((int)p) reset_req.boot_from_rom = TRUE;
    else        reset_req.boot_from_rom = FALSE;

    set_reset_dipsw_boot();
}


static	Q8tkWidget	*menu_dipsw_b(void)
{
    int i;
    Q8tkWidget *vbox, *hbox;
    Q8tkWidget *b = NULL;
    const t_dipsw *pd;
    const t_menudata *p;


    vbox = PACK_VBOX(NULL);
    {
	pd = data_dipsw_b;
	for (i=0; i<COUNTOF(data_dipsw_b); i++, pd++) {

	    hbox = PACK_HBOX(vbox);
	    {
		PACK_LABEL(hbox, GET_LABEL(pd, 0));

		PACK_RADIO_BUTTONS(hbox,
				   pd->p, 2,
				   get_dipsw_b(i), cb_dipsw_b);
	    }
	}

	hbox = PACK_HBOX(vbox);
	{
	    pd = data_dipsw_b2;
	    p  = pd->p;

	    PACK_LABEL(hbox, GET_LABEL(pd, 0));

	    for (i=0; i<2; i++, p++) {
		b = PACK_RADIO_BUTTON(hbox,
				      b,
				      GET_LABEL(p, 0), 
				      (get_dipsw_b2() == p->val) ?TRUE :FALSE,
				      cb_dipsw_b2, (void *)(p->val));

		if (i == 0) widget_dipsw_b_boot_disk = b;  /*�����Υܥ����*/
		else        widget_dipsw_b_boot_rom  = b;  /*�Ф��Ƥ���      */
	    }
	}
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
				   /* �ǥ��åץ����å��ڤ��ؤ�(RS-232C) */
static	int	get_dipsw_r(int p)
{
    int shift = data_dipsw_r[p].val;

    return ((p<<1) | ((reset_req.boot_dipsw >> shift) & 1));
}
static	void	cb_dipsw_r(UNUSED_WIDGET, void *p)
{
    int shift = data_dipsw_r[ (int)p >> 1 ].val;
    int on    = (int)p & 1;

    if (on) reset_req.boot_dipsw |=  (1 << shift);
    else    reset_req.boot_dipsw &= ~(1 << shift);
}
static	int	get_dipsw_r_baudrate(void)
{
    return reset_req.baudrate_sw;
}
static	void	cb_dipsw_r_baudrate(Q8tkWidget *widget, UNUSED_PARM)
{
    int i;
    const t_menudata *p = data_dipsw_r_baudrate;
    const char       *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(data_dipsw_r_baudrate); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    reset_req.baudrate_sw = p->val;
	    return;
	}
    }
}


static	Q8tkWidget	*menu_dipsw_r(void)
{
    int i;
    Q8tkWidget *vbox, *hbox;
    const t_dipsw *pd;

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    PACK_LABEL(hbox, GET_LABEL(data_dipsw_r2, 0));

	    PACK_COMBO(hbox,
		       data_dipsw_r_baudrate,
		       COUNTOF(data_dipsw_r_baudrate),
		       get_dipsw_r_baudrate(), NULL, 8,
		       cb_dipsw_r_baudrate, NULL,
		       NULL, NULL);
	}

	pd = data_dipsw_r;
	for (i=0; i<COUNTOF(data_dipsw_r); i++, pd++) {

	    hbox = PACK_HBOX(vbox);
	    {
		PACK_LABEL(hbox, GET_LABEL(data_dipsw_r, i));

		PACK_RADIO_BUTTONS(hbox,
				   pd->p, 2,
				   get_dipsw_r(i), cb_dipsw_r);
	    }
	}
    }

    return vbox;
}

/*----------------------------------------------------------------------*/

static	void	dipsw_create(void)
{
    Q8tkWidget *vbox;
    const t_menulabel *l = data_dipsw;

    vbox = PACK_VBOX(NULL);
    {
	PACK_FRAME(vbox, GET_LABEL(l, DATA_DIPSW_B), menu_dipsw_b());

	PACK_FRAME(vbox, GET_LABEL(l, DATA_DIPSW_R), menu_dipsw_r());
    }

    dipsw_window = vbox;
}

static	void	cb_reset_dipsw_end(UNUSED_WIDGET, UNUSED_PARM)
{
    dipsw_finish();
}

static	void	dipsw_start(void)
{
    Q8tkWidget *w, *f, *x, *b;
    const t_menulabel *l = data_reset;

    {						/* �ᥤ��Ȥʤ륦����ɥ� */
	w = q8tk_window_new(Q8TK_WINDOW_DIALOG);
	dipsw_accel = q8tk_accel_group_new();
	q8tk_accel_group_attach(dipsw_accel, w);
    }
    {						/* �ˡ��ե졼���褻�� */
	f = q8tk_frame_new(GET_LABEL(l, DATA_RESET_DIPSW_SET));
	q8tk_frame_set_shadow_type(f, Q8TK_SHADOW_OUT);
	q8tk_container_add(w, f);
	q8tk_widget_show(f);
    }
    {						/* ����˥ܥå�����褻�� */
	x = q8tk_vbox_new();
	q8tk_container_add(f, x);
	q8tk_widget_show(x);
							/* �ܥå����ˤ�     */
	{						/* DIPSW��˥塼 �� */
	    q8tk_box_pack_start(x, dipsw_window);
	}
	{						/* ��λ�ܥ�������� */
	    b = PACK_BUTTON(x,
			    GET_LABEL(l, DATA_RESET_DIPSW_QUIT),
			    cb_reset_dipsw_end, NULL);

	    q8tk_accel_group_add(dipsw_accel, Q8TK_KEY_ESC, b, "clicked");
	}
    }

    q8tk_widget_show(w);
    q8tk_grab_add(w);

    q8tk_widget_set_focus(b);


    dipsw[ DIPSW_WIN   ] = w;	/* �����������Ĥ����Ȥ��������� */
    dipsw[ DIPSW_FRAME ] = f;	/* �������åȤ�Ф��Ƥ����ޤ�     */
    dipsw[ DIPSW_VBOX  ] = x;
    dipsw[ DIPSW_QUIT  ] = b;
}

/* �ǥ��åץ����å����ꥦ����ɥ��ξõ� */

static	void	dipsw_finish(void)
{
    q8tk_widget_destroy(dipsw[ DIPSW_QUIT ]);
    q8tk_widget_destroy(dipsw[ DIPSW_VBOX ]);
    q8tk_widget_destroy(dipsw[ DIPSW_FRAME ]);

    q8tk_grab_remove(dipsw[ DIPSW_WIN ]);
    q8tk_widget_destroy(dipsw[ DIPSW_WIN ]);
    q8tk_widget_destroy(dipsw_accel);
}





/*===========================================================================
 *
 *	�ᥤ��ڡ���	CPU����
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
						     /* CPU�����ڤ��ؤ� */
static	int	get_cpu_cpu(void)
{
    return cpu_timing;
}
static	void	cb_cpu_cpu(UNUSED_WIDGET, void *p)
{
    cpu_timing = (int)p;
}


static	Q8tkWidget	*menu_cpu_cpu(void)
{
    Q8tkWidget *vbox;

    vbox = PACK_VBOX(NULL);
    {
	PACK_RADIO_BUTTONS(vbox,
			   data_cpu_cpu, COUNTOF(data_cpu_cpu),
			   get_cpu_cpu(), cb_cpu_cpu);
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
								/* ���� */
static	Q8tkWidget	*help_widget[5];
static	Q8tkWidget	*help_string[40];
static	int		help_string_cnt;	
static	Q8tkWidget	*help_accel;

enum {
    HELP_WIN,
    HELP_VBOX,
    HELP_SWIN,
    HELP_BOARD,
    HELP_EXIT
};

static	void	help_finish(void);
static	void	cb_cpu_help_end(UNUSED_WIDGET, UNUSED_PARM)
{
    help_finish();
}

static	void	cb_cpu_help(UNUSED_WIDGET, UNUSED_PARM)
{
    Q8tkWidget *w, *swin, *x, *b, *z;

    {						/* �ᥤ��Ȥʤ륦����ɥ� */
	w = q8tk_window_new(Q8TK_WINDOW_DIALOG);
	help_accel = q8tk_accel_group_new();
	q8tk_accel_group_attach(help_accel, w);
    }
    {						/* ����˥ܥå�����褻�� */
	x = q8tk_vbox_new();
	q8tk_container_add(w, x);
	q8tk_widget_show(x);
							/* �ܥå����ˤ�     */
	{						/* SCRL������ɥ��� */
	    swin  = q8tk_scrolled_window_new(NULL, NULL);
	    q8tk_widget_show(swin);
	    q8tk_scrolled_window_set_policy(swin, Q8TK_POLICY_NEVER,
						  Q8TK_POLICY_AUTOMATIC);
	    q8tk_misc_set_size(swin, 71, 20);
	    q8tk_box_pack_start(x, swin);
	}
	{						/* ��λ�ܥ�������� */
	    b = PACK_BUTTON(x,
			    " O K ",
			    cb_cpu_help_end, NULL);
	    q8tk_misc_set_placement(b, Q8TK_PLACEMENT_X_CENTER,
				       Q8TK_PLACEMENT_Y_CENTER);

	    q8tk_accel_group_add(help_accel, Q8TK_KEY_ESC, b, "clicked");
	}
    }

    {							/* SCRL������ɥ��� */
	int i;
	const char **s = (menu_lang == MENU_JAPAN) ? help_jp : help_en;
	z = q8tk_vbox_new();				/* VBOX���ä�     */
	q8tk_container_add(swin, z);
	q8tk_widget_show(z);

	for (i=0; i<COUNTOF(help_string); i++) {	/* ������٥������ */
	    if (s[i] == NULL) break;
	    help_string[i] = q8tk_label_new(s[i]);
	    q8tk_widget_show(help_string[i]);
	    q8tk_box_pack_start(z, help_string[i]);
	}
	help_string_cnt = i;
    }

    q8tk_widget_show(w);
    q8tk_grab_add(w);

    q8tk_widget_set_focus(b);


    help_widget[ HELP_WIN   ] = w;	/* �����������Ĥ����Ȥ��������� */
    help_widget[ HELP_VBOX  ] = x;	/* �������åȤ�Ф��Ƥ����ޤ�     */
    help_widget[ HELP_SWIN  ] = swin;
    help_widget[ HELP_BOARD ] = z;
    help_widget[ HELP_EXIT  ] = b;
}

/* ����������ɥ��ξõ� */

static	void	help_finish(void)
{
    int i;
    for (i=0; i<help_string_cnt; i++)
	q8tk_widget_destroy(help_string[ i ]);

    q8tk_widget_destroy(help_widget[ HELP_EXIT  ]);
    q8tk_widget_destroy(help_widget[ HELP_BOARD ]);
    q8tk_widget_destroy(help_widget[ HELP_SWIN  ]);
    q8tk_widget_destroy(help_widget[ HELP_VBOX  ]);

    q8tk_grab_remove(help_widget[ HELP_WIN ]);
    q8tk_widget_destroy(help_widget[ HELP_WIN ]);
    q8tk_widget_destroy(help_accel);
}



static	Q8tkWidget	*menu_cpu_help(void)
{
    Q8tkWidget *button;
    const t_menulabel *l = data_cpu;

    button = PACK_BUTTON(NULL,
			 GET_LABEL(l, DATA_CPU_HELP),
			 cb_cpu_help, NULL);
    q8tk_misc_set_placement(button, Q8TK_PLACEMENT_X_CENTER,
				    Q8TK_PLACEMENT_Y_CENTER);
    return button;
}

/*----------------------------------------------------------------------*/
						 /* CPU����å��ڤ��ؤ� */
static	double	get_cpu_clock(void)
{
    return cpu_clock_mhz;
}
static	void	cb_cpu_clock(Q8tkWidget *widget, void *mode)
{
    int i;
    const t_menudata *p = data_cpu_clock_combo;
    const char       *combo_str = q8tk_combo_get_text(widget);
    char buf[16], *conv_end;
    double val = 0;
    int fit = FALSE;

    /* COMBO BOX ���� ENTRY �˰��פ����Τ�õ�� */
    for (i=0; i<COUNTOF(data_cpu_clock_combo); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    val = (double)p->val / 1000000.0;
	    fit = TRUE;					/* ���פ����ͤ�Ŭ�� */
	    break;
	}
    }

    if (fit == FALSE) {			/* COMBO BOX �˳������ʤ���� */
	strncpy(buf, combo_str, 15);
	buf[15] = '\0';

	val = strtod(buf, &conv_end);

	if (((int)mode == 0) &&				/* ���� + ENTER ��   */
	    (strlen(buf) == 0 || val == 0.0)) {		/*   0  + ENTER ���� */
							/* �ǥե�����ͤ�Ŭ��*/
	    val = boot_clock_4mhz ? CONST_4MHZ_CLOCK
				  : CONST_8MHZ_CLOCK;
	    fit = TRUE;

	} else if (*conv_end != '\0') {			/* �����Ѵ����Ԥʤ� */
	    fit = FALSE;				/* �����ͤϻȤ��ʤ� */

	} else {					/* �����Ѵ������ʤ� */
	    fit = TRUE;					/* �����ͤ�Ŭ�Ѥ��� */
	}
    }

    if (fit) {				/* Ŭ�Ѥ����ͤ�ͭ���ϰϤʤ顢���å� */
	if (0.1 <= val && val < 1000.0) {
	    cpu_clock_mhz = val;
	    interval_work_init_all();
	}
    }

    if ((int)mode == 0) {		/* COMBO �ʤ��� ENTER���ϡ��ͤ��ɽ��*/
	sprintf(buf, "%8.4f", get_cpu_clock());
	q8tk_combo_set_text(widget, buf);
    }
}


static	Q8tkWidget	*menu_cpu_clock(void)
{
    Q8tkWidget *vbox, *hbox;
    const t_menulabel *p = data_cpu_clock;
    char buf[16];

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    PACK_LABEL(hbox, GET_LABEL(p, DATA_CPU_CLOCK_CLOCK));

	    sprintf(buf, "%8.4f", get_cpu_clock());
	    PACK_COMBO(hbox,
		       data_cpu_clock_combo, COUNTOF(data_cpu_clock_combo),
		       (int) get_cpu_clock(), buf, 9,
		       cb_cpu_clock, (void *)0,
		       cb_cpu_clock, (void *)1);

	    PACK_LABEL(hbox, GET_LABEL(p, DATA_CPU_CLOCK_MHZ));

	    PACK_LABEL(hbox, GET_LABEL(p, DATA_CPU_CLOCK_INFO));
	}
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
							/* ���������ѹ� */
static	int	get_cpu_nowait(void)
{
    return no_wait;
}
static	void	cb_cpu_nowait(Q8tkWidget *widget, UNUSED_PARM)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;
    no_wait = key;
}

static	int	get_cpu_wait(void)
{
    return wait_rate;
}
static	void	cb_cpu_wait(Q8tkWidget *widget, void *mode)
{
    int i;
    const t_menudata *p = data_cpu_wait_combo;
    const char       *combo_str = q8tk_combo_get_text(widget);
    char buf[16], *conv_end;
    int val = 0;
    int fit = FALSE;

    /* COMBO BOX ���� ENTRY �˰��פ����Τ�õ�� */
    for (i=0; i<COUNTOF(data_cpu_wait_combo); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    val = p->val;
	    fit = TRUE;					/* ���פ����ͤ�Ŭ�� */
	    break;
	}
    }

    if (fit == FALSE) {			/* COMBO BOX �˳������ʤ���� */
	strncpy(buf, combo_str, 15);
	buf[15] = '\0';

	val = strtoul(buf, &conv_end, 10);

	if (((int)mode == 0) &&				/* ���� + ENTER ��   */
	    (strlen(buf) == 0 || val == 0)) {		/*   0  + ENTER ���� */
							/* �ǥե�����ͤ�Ŭ��*/
	    val = 100;
	    fit = TRUE;

	} else if (*conv_end != '\0') {			/* �����Ѵ����Ԥʤ� */
	    fit = FALSE;				/* �����ͤϻȤ��ʤ� */

	} else {					/* �����Ѵ������ʤ� */
	    fit = TRUE;					/* �����ͤ�Ŭ�Ѥ��� */
	}
    }

    if (fit) {				/* Ŭ�Ѥ����ͤ�ͭ���ϰϤʤ顢���å� */
	if (5 <= val && val <= 5000) {
	    wait_rate = val;
	}
    }

    if ((int)mode == 0) {		/* COMBO �ʤ��� ENTER���ϡ��ͤ��ɽ��*/
	sprintf(buf, "%4d", get_cpu_wait());
	q8tk_combo_set_text(widget, buf);
    }
}


static	Q8tkWidget	*menu_cpu_wait(void)
{
    Q8tkWidget *vbox, *hbox, *button;
    const t_menulabel *p = data_cpu_wait;
    char buf[16];

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    PACK_LABEL(hbox, GET_LABEL(p, DATA_CPU_WAIT_RATE));

	    sprintf(buf, "%4d", get_cpu_wait());
	    PACK_COMBO(hbox,
		       data_cpu_wait_combo, COUNTOF(data_cpu_wait_combo),
		       get_cpu_wait(), buf, 5,
		       cb_cpu_wait, (void *)0,
		       cb_cpu_wait, (void *)1);

	    PACK_LABEL(hbox, GET_LABEL(p, DATA_CPU_WAIT_PERCENT));

	    PACK_LABEL(hbox, GET_LABEL(p, DATA_CPU_WAIT_INFO));
	}

	button = PACK_CHECK_BUTTON(vbox,
				   GET_LABEL(p, DATA_CPU_WAIT_NOWAIT),
				   get_cpu_nowait(),
				   cb_cpu_nowait, NULL);
	q8tk_misc_set_placement(button, Q8TK_PLACEMENT_X_RIGHT,
					Q8TK_PLACEMENT_Y_CENTER);
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
							    /* �֡����� */
static	int	get_cpu_boost(void)
{
    return boost;
}
static	void	cb_cpu_boost(Q8tkWidget *widget, void *mode)
{
    int i;
    const t_menudata *p = data_cpu_boost_combo;
    const char       *combo_str = q8tk_combo_get_text(widget);
    char buf[16], *conv_end;
    int val = 0;
    int fit = FALSE;

    /* COMBO BOX ���� ENTRY �˰��פ����Τ�õ�� */
    for (i=0; i<COUNTOF(data_cpu_boost_combo); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    val = p->val;
	    fit = TRUE;					/* ���פ����ͤ�Ŭ�� */
	    break;
	}
    }

    if (fit == FALSE) {			/* COMBO BOX �˳������ʤ���� */
	strncpy(buf, combo_str, 15);
	buf[15] = '\0';

	val = strtoul(buf, &conv_end, 10);

	if (((int)mode == 0) &&				/* ���� + ENTER ��   */
	    (strlen(buf) == 0 || val == 0)) {		/*   0  + ENTER ���� */
							/* �ǥե�����ͤ�Ŭ��*/
	    val = 1;
	    fit = TRUE;

	} else if (*conv_end != '\0') {			/* �����Ѵ����Ԥʤ� */
	    fit = FALSE;				/* �����ͤϻȤ��ʤ� */

	} else {					/* �����Ѵ������ʤ� */
	    fit = TRUE;					/* �����ͤ�Ŭ�Ѥ��� */
	}
    }

    if (fit) {				/* Ŭ�Ѥ����ͤ�ͭ���ϰϤʤ顢���å� */
	if (1 <= val && val <= 100) {
	    if (boost != val) {
		boost_change(val);
	    }
	}
    }

    if ((int)mode == 0) {		/* COMBO �ʤ��� ENTER���ϡ��ͤ��ɽ��*/
	sprintf(buf, "%4d", get_cpu_boost());
	q8tk_combo_set_text(widget, buf);
    }
}

static	Q8tkWidget	*menu_cpu_boost(void)
{
    Q8tkWidget *hbox;
    char buf[8];
    const t_menulabel *p = data_cpu_boost;

    hbox = PACK_HBOX(NULL);
    {
	PACK_LABEL(hbox, GET_LABEL(p, DATA_CPU_BOOST_MAGNIFY));

	sprintf(buf, "%4d", get_cpu_boost());
	PACK_COMBO(hbox,
		   data_cpu_boost_combo, COUNTOF(data_cpu_boost_combo),
		   get_cpu_boost(), buf, 5,
		   cb_cpu_boost, (void*)0,
		   cb_cpu_boost, (void*)1);

	PACK_LABEL(hbox, GET_LABEL(p, DATA_CPU_BOOST_UNIT));

	PACK_LABEL(hbox, GET_LABEL(p, DATA_CPU_BOOST_INFO));
    }

    return hbox;
}

/*----------------------------------------------------------------------*/
						      /* �Ƽ�������ѹ� */
static	int	get_cpu_misc(int type)
{
    switch (type) {
    case DATA_CPU_MISC_FDCWAIT:
	return (fdc_wait == 0) ? FALSE : TRUE;

    case DATA_CPU_MISC_HSBASIC:
	return highspeed_mode;

    case DATA_CPU_MISC_MEMWAIT:
	return memory_wait;

    case DATA_CPU_MISC_CMDSING:
	return use_cmdsing;
    }
    return FALSE;
}
static	void	cb_cpu_misc(Q8tkWidget *widget, void *p)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    switch ((int)p) {
    case DATA_CPU_MISC_FDCWAIT:
	fdc_wait = (key) ? 1 : 0;
	return;

    case DATA_CPU_MISC_HSBASIC:
	highspeed_mode = (key) ? TRUE : FALSE;
	return;

    case DATA_CPU_MISC_MEMWAIT:
	memory_wait = (key) ? TRUE : FALSE;
	return;

    case DATA_CPU_MISC_CMDSING:
	use_cmdsing = (key) ? TRUE : FALSE;
#ifdef	USE_SOUND
	xmame_dev_beep_cmd_sing((byte) use_cmdsing);
#endif
	return;
    }
}


static	Q8tkWidget	*menu_cpu_misc(void)
{
    int i;
    Q8tkWidget *vbox, *l;
    const t_menudata *p = data_cpu_misc;

    vbox = PACK_VBOX(NULL);
    {
	for (i=0; i<COUNTOF(data_cpu_misc); i++, p++) {
	    if (p->val >= 0) {
		PACK_CHECK_BUTTON(vbox,
				  GET_LABEL(p, 0),
				  get_cpu_misc(p->val),
				  cb_cpu_misc, (void *)(p->val));
	    } else {
		l = PACK_LABEL(vbox, GET_LABEL(p, 0));
		q8tk_misc_set_placement(l, Q8TK_PLACEMENT_X_RIGHT, 0);
	    }
	}
    }

    return vbox;
}

/*======================================================================*/

static	Q8tkWidget	*menu_cpu(void)
{
    Q8tkWidget *vbox, *hbox, *vbox2;
    Q8tkWidget *f;
    const t_menulabel *l = data_cpu;

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    PACK_FRAME(hbox, GET_LABEL(l, DATA_CPU_CPU), menu_cpu_cpu());

	    f = PACK_FRAME(hbox, "              ", menu_cpu_help());
	    q8tk_frame_set_shadow_type(f, Q8TK_SHADOW_NONE);
	}

	hbox = PACK_HBOX(vbox);
	{
	    vbox2 = PACK_VBOX(hbox);
	    {
		PACK_FRAME(vbox2,
			   GET_LABEL(l, DATA_CPU_CLOCK), menu_cpu_clock());

		PACK_FRAME(vbox2,
			   GET_LABEL(l, DATA_CPU_WAIT), menu_cpu_wait());

		PACK_FRAME(vbox2,
			   GET_LABEL(l, DATA_CPU_BOOST), menu_cpu_boost());
	    }

	    f = PACK_FRAME(hbox, "", menu_cpu_misc());
	    q8tk_frame_set_shadow_type(f, Q8TK_SHADOW_NONE);
	}
    }

    return vbox;
}








/*===========================================================================
 *
 *	�ᥤ��ڡ���	����
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
						  /* �ե졼��졼���ѹ� */
static	int	get_graph_frate(void)
{
    return quasi88_cfg_now_frameskip_rate();
}
static	void	cb_graph_frate(Q8tkWidget *widget, void *label)
{
    int i;
    const t_menudata *p = data_graph_frate;
    const char       *combo_str = q8tk_combo_get_text(widget);
    char  str[32];

    for (i=0; i<COUNTOF(data_graph_frate); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    sprintf(str, " fps (-frameskip %d)", p->val);
	    q8tk_label_set((Q8tkWidget*)label, str);

	    quasi88_cfg_set_frameskip_rate(p->val);
	    return;
	}
    }
}
						/* thanks floi ! */
static	int	get_graph_autoskip(void)
{
    return use_auto_skip;
}
static	void	cb_graph_autoskip(Q8tkWidget *widget, UNUSED_PARM)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;
    use_auto_skip = key;
}


static	Q8tkWidget	*menu_graph_frate(void)
{
    Q8tkWidget *vbox, *hbox, *combo, *label;
    char wk[32];

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    label = q8tk_label_new(" fps");
	    {
		sprintf(wk, "%6.3f", 60.0f / get_graph_frate());
		combo = PACK_COMBO(hbox,
				   data_graph_frate, COUNTOF(data_graph_frate),
				   get_graph_frate(), wk, 6,
				   cb_graph_frate, label,
				   NULL, NULL);
	    }
	    {
		q8tk_box_pack_start(hbox, label);
		q8tk_widget_show(label);
		cb_graph_frate(combo, (void*)label);
	    }
	}

	PACK_LABEL(vbox, "");			/* ���� */
							/* thanks floi ! */
	PACK_CHECK_BUTTON(vbox,
			  GET_LABEL(data_graph_autoskip, 0),
			  get_graph_autoskip(),
			  cb_graph_autoskip, NULL);
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
						  /* ���̥������ڤ��ؤ� */
static	int	get_graph_resize(void)
{
    return quasi88_cfg_now_size();
}
static	void	cb_graph_resize(UNUSED_WIDGET, void *p)
{
    int new_size = (int)p;

    quasi88_cfg_set_size(new_size);
}
static	int	get_graph_fullscreen(void)
{
    return use_fullscreen;
}
static	void	cb_graph_fullscreen(Q8tkWidget *widget, UNUSED_PARM)
{
    int on = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    if (quasi88_cfg_can_fullscreen()) {
	quasi88_cfg_set_fullscreen(on);
	
	/* Q8TK ��������̵ͭ���� (���������ػ��˸Ƥ�ɬ�פ���) */
	q8tk_set_cursor(now_swcursor);
    }
}


static	Q8tkWidget	*menu_graph_resize(void)
{
    Q8tkWidget *vbox;
    int i = COUNTOF(data_graph_resize);
    int j = quasi88_cfg_max_size() - quasi88_cfg_min_size() + 1;

    vbox = PACK_VBOX(NULL);
    {
	PACK_RADIO_BUTTONS(PACK_VBOX(vbox),
			   &data_graph_resize[quasi88_cfg_min_size()],
			   MIN(i, j),
			   get_graph_resize(), cb_graph_resize);

	if (quasi88_cfg_can_fullscreen()) {

	    PACK_LABEL(vbox, "");		/* ���� */

	    PACK_CHECK_BUTTON(vbox,
			      GET_LABEL(data_graph_fullscreen, 0),
			      get_graph_fullscreen(),
			      cb_graph_fullscreen, NULL);
	}
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
						      /* �Ƽ�������ѹ� */
static	int	get_graph_misc(int type)
{
    switch (type) {
    case DATA_GRAPH_MISC_15K:
	return (monitor_15k == 0x02) ? TRUE: FALSE;

    case DATA_GRAPH_MISC_DIGITAL:
	return (monitor_analog == FALSE) ? TRUE : FALSE;

    case DATA_GRAPH_MISC_NOINTERP:
	return (quasi88_cfg_now_interp() == FALSE) ? TRUE : FALSE;
    }
    return FALSE;
}
static	void	cb_graph_misc(Q8tkWidget *widget, void *p)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    switch ((int)p) {
    case DATA_GRAPH_MISC_15K:
	monitor_15k = (key) ? 0x02 : 0x00;
	return;

    case DATA_GRAPH_MISC_DIGITAL:
	monitor_analog = (key) ? FALSE : TRUE;
	return;

    case DATA_GRAPH_MISC_NOINTERP:
	if (quasi88_cfg_can_interp()) {
	    quasi88_cfg_set_interp((key) ? FALSE : TRUE);
	}
	return;
    }
}
static	int	get_graph_misc2(void)
{
    return quasi88_cfg_now_interlace();
}
static	void	cb_graph_misc2(UNUSED_WIDGET, void *p)
{
    quasi88_cfg_set_interlace((int)p);
}


static	Q8tkWidget	*menu_graph_misc(void)
{
    Q8tkWidget *vbox;
    const t_menudata *p = data_graph_misc;
    int i       = COUNTOF(data_graph_misc);

    if (quasi88_cfg_can_interp() == FALSE) {
	i --;
    }

    vbox = PACK_VBOX(NULL);
    {
	PACK_CHECK_BUTTONS(vbox,
			   p, i,
			   get_graph_misc, cb_graph_misc);

	PACK_LABEL(vbox, "");

	PACK_RADIO_BUTTONS(vbox,
			   data_graph_misc2, COUNTOF(data_graph_misc2),
			   get_graph_misc2(), cb_graph_misc2);
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
						 /* PCG̵ͭ �� �ե���� */
static	Q8tkWidget	*graph_font_widget;
static	int	get_graph_pcg(void)
{
    return use_pcg;
}
static	void	cb_graph_pcg(Q8tkWidget *widget, void *p)
{
    if (widget) {
	use_pcg = (int)p;
	memory_set_font();
    }

    if (use_pcg) { q8tk_widget_set_sensitive(graph_font_widget, FALSE); }
    else         { q8tk_widget_set_sensitive(graph_font_widget, TRUE);  }
}

static	int	get_graph_font(void)
{
    return font_type;
}
static	void	cb_graph_font(UNUSED_WIDGET, void *p)
{
    font_type = (int)p;
    memory_set_font();
}

static	Q8tkWidget	*menu_graph_pcg(void)
{
    Q8tkWidget *vbox, *b;
    const t_menulabel *l = data_graph;
    t_menudata data_graph_font[3];


    /* �ե�������򥦥����å����� (PCG̵ͭ�ˤ�ꡢinsensitive �ˤʤ�) */
    {
	data_graph_font[0] = data_graph_font1[ (font_loaded & 1) ? 1 : 0 ];
	data_graph_font[1] = data_graph_font2[ (font_loaded & 2) ? 1 : 0 ];
	data_graph_font[2] = data_graph_font3[ (font_loaded & 4) ? 1 : 0 ];

	b = PACK_VBOX(NULL);
	{
	    PACK_RADIO_BUTTONS(b,
			       data_graph_font, COUNTOF(data_graph_font),
			       get_graph_font(), cb_graph_font);
	}
	graph_font_widget = PACK_FRAME(NULL,
				       GET_LABEL(l, DATA_GRAPH_FONT), b);
    }

    /* PCG̵ͭ�������åȤȡ��ե�������򥦥����åȤ��¤٤� */
    vbox = PACK_VBOX(NULL);
    {
	{
	    b = PACK_HBOX(NULL);
	    {
		PACK_RADIO_BUTTONS(b,
				   data_graph_pcg, COUNTOF(data_graph_pcg),
				   get_graph_pcg(), cb_graph_pcg);
	    }
	    PACK_FRAME(vbox, GET_LABEL(l, DATA_GRAPH_PCG), b);
	}

	q8tk_box_pack_start(vbox, graph_font_widget);
    }

    return vbox;
}

/*======================================================================*/

static	Q8tkWidget	*menu_graph(void)
{
    Q8tkWidget *vbox, *hbox;
    Q8tkWidget *w;
    const t_menulabel *l = data_graph;

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    PACK_FRAME(hbox,
		       GET_LABEL(l, DATA_GRAPH_FRATE), menu_graph_frate());

	    PACK_FRAME(hbox,
		       GET_LABEL(l, DATA_GRAPH_RESIZE), menu_graph_resize());
	}

	hbox = PACK_HBOX(vbox);
	{
	    w = PACK_FRAME(hbox, "", menu_graph_misc());
	    q8tk_frame_set_shadow_type(w, Q8TK_SHADOW_NONE);

	    w = PACK_FRAME(hbox, "", menu_graph_pcg());
	    q8tk_frame_set_shadow_type(w, Q8TK_SHADOW_NONE);
	}
    }

    return vbox;
}








/*===========================================================================
 *
 *	�ᥤ��ڡ���	����
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
						      /* �ܥ�塼���ѹ� */
#ifdef	USE_SOUND
static	int	get_volume(int type)
{
    switch (type) {
    case VOL_TOTAL:  return  xmame_cfg_get_mastervolume();
    case VOL_FM:     return  xmame_cfg_get_mixer_volume(XMAME_MIXER_FM);
    case VOL_PSG:    return  xmame_cfg_get_mixer_volume(XMAME_MIXER_PSG);
    case VOL_BEEP:   return  xmame_cfg_get_mixer_volume(XMAME_MIXER_BEEP);
    case VOL_RHYTHM: return  xmame_cfg_get_mixer_volume(XMAME_MIXER_RHYTHM);
    case VOL_ADPCM:  return  xmame_cfg_get_mixer_volume(XMAME_MIXER_ADPCM);
    case VOL_FMGEN:  return  xmame_cfg_get_mixer_volume(XMAME_MIXER_FMGEN);
    case VOL_SAMPLE: return  xmame_cfg_get_mixer_volume(XMAME_MIXER_SAMPLE);
    }
    return 0;
}
static	void	cb_volume(Q8tkWidget *widget, void *p)
{
    int vol = Q8TK_ADJUSTMENT(widget)->value;

    switch ((int)p) {
    case VOL_TOTAL:  xmame_cfg_set_mastervolume(vol);			 break;
    case VOL_FM:     xmame_cfg_set_mixer_volume(XMAME_MIXER_FM, vol);	 break;
    case VOL_PSG:    xmame_cfg_set_mixer_volume(XMAME_MIXER_PSG, vol);	 break;
    case VOL_BEEP:   xmame_cfg_set_mixer_volume(XMAME_MIXER_BEEP, vol);  break;
    case VOL_RHYTHM: xmame_cfg_set_mixer_volume(XMAME_MIXER_RHYTHM, vol);break;
    case VOL_ADPCM:  xmame_cfg_set_mixer_volume(XMAME_MIXER_ADPCM, vol); break;
    case VOL_FMGEN:  xmame_cfg_set_mixer_volume(XMAME_MIXER_FMGEN, vol); break;
    case VOL_SAMPLE: xmame_cfg_set_mixer_volume(XMAME_MIXER_SAMPLE, vol);break;
    }
}


static	Q8tkWidget	*menu_volume_unit(const t_volume *p, int count)
{
    int i;
    Q8tkWidget *vbox, *hbox;

    vbox = PACK_VBOX(NULL);
    {
	for (i=0; i<count; i++, p++) {

	    hbox = PACK_HBOX(vbox);
	    {
		PACK_LABEL(hbox, GET_LABEL(p, 0));

		PACK_HSCALE(hbox,
			    p,
			    get_volume(p->val),
			    cb_volume, (void*)(p->val));
	    }
	}
    }

    return vbox;
}


static	Q8tkWidget	*menu_volume_total(void)
{
    return menu_volume_unit(data_volume_total, COUNTOF(data_volume_total));
}
static	Q8tkWidget	*menu_volume_level(void)
{
    return menu_volume_unit(data_volume_level, COUNTOF(data_volume_level));
}
static	Q8tkWidget	*menu_volume_rhythm(void)
{
    return menu_volume_unit(data_volume_rhythm, COUNTOF(data_volume_rhythm));
}
static	Q8tkWidget	*menu_volume_fmgen(void)
{
    return menu_volume_unit(data_volume_fmgen, COUNTOF(data_volume_fmgen));
}
static	Q8tkWidget	*menu_volume_sample(void)
{
    return menu_volume_unit(data_volume_sample, COUNTOF(data_volume_sample));
}
#endif
/*----------------------------------------------------------------------*/
					    /* ������ɤʤ�����å����� */

static	Q8tkWidget	*menu_volume_no_available(void)
{
    int type;
    Q8tkWidget *l;

#ifdef	USE_SOUND
    type = 2;
#else
    type = 0;
#endif

    if (sound_board == SOUND_II) {
	type |= 1;
    }

    l = q8tk_label_new(GET_LABEL(data_volume_no, type));

    q8tk_widget_show(l);

    return l;
}

/*----------------------------------------------------------------------*/
					    /* ������ɥɥ饤�м���ɽ�� */
#ifdef	USE_SOUND
static	Q8tkWidget	*menu_volume_type(void)
{
    int type;
    Q8tkWidget *l;

#ifdef	USE_FMGEN
    if (xmame_cfg_get_use_fmgen()) {
	type = 2;
    } else
#endif
    {
	type = 0;
    }

    if (sound_board == SOUND_II) {
	type |= 1;
    }

    l = q8tk_label_new(GET_LABEL(data_volume_type, type));

    q8tk_widget_show(l);

    return l;
}
#endif

/*----------------------------------------------------------------------*/
							/* ������ɾܺ� */
#ifdef	USE_SOUND
static	void	audio_create(void);
static	void	audio_start(void);
static	void	audio_finish(void);

static	void	cb_volume_audio(UNUSED_WIDGET, UNUSED_PARM)
{
    audio_start();
}

static	Q8tkWidget	*menu_volume_audio(void)
{
    Q8tkWidget *hbox;

    hbox = PACK_HBOX(NULL);
    {
	PACK_LABEL(hbox, " ");			/* ���� */
	PACK_BUTTON(hbox,
		    GET_LABEL(data_volume, DATA_VOLUME_AUDIO),
		    cb_volume_audio, NULL);
	PACK_LABEL(hbox, " ");			/* ���� */
    }
    return hbox;
}
#endif

/*----------------------------------------------------------------------*/

static	Q8tkWidget	*menu_volume(void)
{
    Q8tkWidget *vbox, *hbox, *vbox2;
    Q8tkWidget *w;
    const t_menulabel *l = data_volume;

    if (xmame_has_sound() == FALSE) {

	w = PACK_FRAME(NULL, "", menu_volume_no_available());
	q8tk_frame_set_shadow_type(w, Q8TK_SHADOW_ETCHED_OUT);

	return w;
    }


#ifdef	USE_SOUND
    audio_create();			/* ������ɾܺ٥�����ɥ����� */

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(NULL);
	{
	    w = PACK_FRAME(hbox, "", menu_volume_type());
	    q8tk_frame_set_shadow_type(w, Q8TK_SHADOW_ETCHED_OUT);

	    PACK_LABEL(hbox, "  ");

	    PACK_BUTTON(hbox, GET_LABEL(data_volume, DATA_VOLUME_AUDIO),
			cb_volume_audio, NULL);
	}
	q8tk_box_pack_start(vbox, hbox);

	if (xmame_has_mastervolume()) {
	    PACK_FRAME(vbox,
		       GET_LABEL(l, DATA_VOLUME_TOTAL), menu_volume_total());
	}

	vbox2 = PACK_VBOX(NULL);
	{
#ifdef	USE_FMGEN
	    if (xmame_cfg_get_use_fmgen()) {
		w = menu_volume_fmgen();
	    }
	    else
#endif
	    {
		w = menu_volume_level();
	    }
	    q8tk_box_pack_start(vbox2, w);
	    
	    if (xmame_cfg_get_use_samples()) {
		q8tk_box_pack_start(vbox2, menu_volume_sample());
	    }
	}
	PACK_FRAME(vbox, GET_LABEL(l, DATA_VOLUME_LEVEL), vbox2);

#ifdef	USE_FMGEN
	if (xmame_cfg_get_use_fmgen()) {
	    ;
	}
	else
#endif
	if (sound_board == SOUND_II) {
	    PACK_FRAME(vbox,
		       GET_LABEL(l, DATA_VOLUME_DEPEND), menu_volume_rhythm());
	}

	if (xmame_has_audiodevice() == FALSE) {
	    PACK_LABEL(vbox, "");
	    PACK_LABEL(vbox, GET_LABEL(data_volume_audiodevice_stop, 0));
	}
    }

    return vbox;
#endif
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 
 *
 *	���֥�����ɥ�	AUDIO
 *
 * = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
#ifdef	USE_SOUND

static	Q8tkWidget	*audio_window;
static	Q8tkWidget	*audio[6];
static	Q8tkWidget	*audio_accel;

enum {
    AUDIO_WIN,
    AUDIO_FRAME,
    AUDIO_VBOX,
    AUDIO_HBOX,
    AUDIO_BUTTON,
    AUDIO_LABEL
};

/*----------------------------------------------------------------------*/
						      /* FM���������ͥ졼�� */
#ifdef	USE_FMGEN
static	int	get_volume_audio_fmgen(void)
{
    return sd_cfg_now.use_fmgen;
}
static	void	cb_volume_audio_fmgen(UNUSED_WIDGET, void *p)
{
    sd_cfg_now.use_fmgen = xmame_cfg_set_use_fmgen((int)p);
}


static	Q8tkWidget	*volume_audio_fmgen(void)
{
    Q8tkWidget *box;
    const t_menulabel *l = data_volume_audio;

    box = PACK_HBOX(NULL);
    {
	PACK_LABEL(box, GET_LABEL(l, DATA_VOLUME_AUDIO_FMGEN));

	PACK_RADIO_BUTTONS(box,
		    data_volume_audio_fmgen, COUNTOF(data_volume_audio_fmgen),
		    get_volume_audio_fmgen(), cb_volume_audio_fmgen);
    }

    return box;
}
#endif

/*----------------------------------------------------------------------*/
						      /* ����ץ���ȿ� */
static	int	get_volume_audio_freq(void)
{
    return sd_cfg_now.sample_freq;
}
static	void	cb_volume_audio_freq(Q8tkWidget *widget, void *mode)
{
    int i;
    const t_menudata *p = data_volume_audio_freq_combo;
    const char       *combo_str = q8tk_combo_get_text(widget);
    char buf[16], *conv_end;
    int val = 0;
    int fit = FALSE;

    /* COMBO BOX ���� ENTRY �˰��פ����Τ�õ�� */
    for (i=0; i<COUNTOF(data_volume_audio_freq_combo); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    val = p->val;
	    fit = TRUE;					/* ���פ����ͤ�Ŭ�� */
	    break;
	}
    }

    if (fit == FALSE) {			/* COMBO BOX �˳������ʤ���� */
	strncpy(buf, combo_str, 15);
	buf[15] = '\0';

	val = strtoul(buf, &conv_end, 10);

	if (((int)mode == 0) &&				/* ���� + ENTER ��   */
	    (strlen(buf) == 0 || val == 0)) {		/*   0  + ENTER ���� */
							/* �ǥե�����ͤ�Ŭ��*/
	    val = 44100;
	    fit = TRUE;

	} else if (*conv_end != '\0') {			/* �����Ѵ����Ԥʤ� */
	    fit = FALSE;				/* �����ͤϻȤ��ʤ� */

	} else {					/* �����Ѵ������ʤ� */
	    fit = TRUE;					/* �����ͤ�Ŭ�Ѥ��� */
	}
    }

    if (fit) {				/* Ŭ�Ѥ����ͤ�ͭ���ϰϤʤ顢���å� */
	if (8000 <= val && val <= 48000) {
	    sd_cfg_now.sample_freq = xmame_cfg_set_sample_freq(val);
	}
    }

    if ((int)mode == 0) {		/* COMBO �ʤ��� ENTER���ϡ��ͤ��ɽ��*/
	sprintf(buf, "%5d", get_volume_audio_freq());
	q8tk_combo_set_text(widget, buf);
    }
}


static	Q8tkWidget	*volume_audio_freq(void)
{
    Q8tkWidget *box;
    char buf[16];
    const t_menulabel *l = data_volume_audio;

    box = PACK_HBOX(NULL);
    {
	PACK_LABEL(box, GET_LABEL(l, DATA_VOLUME_AUDIO_FREQ));

	sprintf(buf, "%5d", get_volume_audio_freq());
	PACK_COMBO(box,
		   data_volume_audio_freq_combo,
		   COUNTOF(data_volume_audio_freq_combo),
		   get_volume_audio_freq(), buf, 6,
		   cb_volume_audio_freq, (void*)0,
		   cb_volume_audio_freq, (void*)1);
    }

    return box;
}

/*----------------------------------------------------------------------*/
						      /* ����ץ벻 */
static	int	get_volume_audio_sample(void)
{
    return sd_cfg_now.use_samples;
}
static	void	cb_volume_audio_sample(UNUSED_WIDGET, void *p)
{
    sd_cfg_now.use_samples = xmame_cfg_set_use_samples((int)p);
}


static	Q8tkWidget	*volume_audio_sample(void)
{
    Q8tkWidget *box;
    const t_menulabel *l = data_volume_audio;

    box = PACK_HBOX(NULL);
    {
	PACK_LABEL(box, GET_LABEL(l, DATA_VOLUME_AUDIO_SAMPLE));

	PACK_RADIO_BUTTONS(box,
		  data_volume_audio_sample, COUNTOF(data_volume_audio_sample),
		  get_volume_audio_sample(), cb_volume_audio_sample);
    }

    return box;
}

/*----------------------------------------------------------------------*/

static	void	cb_audio_config(Q8tkWidget *widget, void *modes)
{
    int index = ((int)modes) / 2;
    int mode  = ((int)modes) & 1;	/* 0:ENTER 1:INPUT */

    T_SNDDRV_CONFIG *p       = sd_cfg_now.local[index].info;
    SD_CFG_LOCAL_VAL def_val = sd_cfg_now.local[index].val;

    const char       *entry_str = q8tk_entry_get_text(widget);
    char buf[16], *conv_end;
    SD_CFG_LOCAL_VAL val = { 0 };
    int fit = FALSE;
    int zero = FALSE;

    strncpy(buf, entry_str, 15);
    buf[15] = '\0';

    switch (p->type) {
    case SNDDRV_INT:
	val.i = (int)strtoul(buf, &conv_end, 10);
	if (val.i == 0) zero = TRUE;
	break;

    case SNDDRV_FLOAT:
	val.f = (float)strtod(buf, &conv_end);
	if (val.f == 0) zero = TRUE;
	break;
    }

    if (((int)mode == 0) &&				/* ���� + ENTER ��   */
	(strlen(buf) == 0 || zero)) {			/*   0  + ENTER ���� */
							/* ľ����ͭ���ͤ�Ŭ��*/
	val = def_val;
	fit = TRUE;

    } else if (*conv_end != '\0') {			/* �����Ѵ����Ԥʤ� */
	fit = FALSE;					/* �����ͤϻȤ��ʤ� */

    } else {						/* �����Ѵ������ʤ� */
	fit = TRUE;					/* �����ͤ�Ŭ�Ѥ��� */
    }

    if (fit) {				/* Ŭ�Ѥ����ͤ�ͭ���ϰϤʤ顢���å� */
	switch (p->type) {
	case SNDDRV_INT:
	    if ((int)(p->low) <=val.i && val.i <= (int)(p->high)) {
		sd_cfg_now.local[index].val.i = 
		    *((int *)(p->work)) = val.i;
	    }
	    break;

	case SNDDRV_FLOAT:
	    if ((float)(p->low) <=val.f && val.f <= (float)(p->high)) {
		sd_cfg_now.local[index].val.f = 
		    *((float *)(p->work)) = val.f;
	    }
	    break;
	}
    }

    if ((int)mode == 0) {		/* COMBO �ʤ��� ENTER���ϡ��ͤ��ɽ��*/
	switch (p->type) {
	case SNDDRV_INT:
	    sprintf(buf, "%7d", sd_cfg_now.local[index].val.i);
	    break;

	case SNDDRV_FLOAT:
	    sprintf(buf, "%7.3f", sd_cfg_now.local[index].val.f);
	    break;
	}

	q8tk_entry_set_text(widget, buf);
    }

    /*if(p->type==SNDDRV_INT)  printf("%d\n", *((int *)(p->work)));*/
    /*if(p->type==SNDDRV_FLOAT)printf("%f\n", *((float *)(p->work)));fflush(stdout);*/
}

static	int	audio_config_widget(Q8tkWidget *box)
{
    Q8tkWidget *hbox;
    char buf[32];
    int i;
    T_SNDDRV_CONFIG *p;

    for (i=0; i<sd_cfg_now.local_cnt; i++) {

	p = sd_cfg_now.local[i].info;

	switch (p->type) {
	case SNDDRV_INT:
	    sprintf(buf, "%7d", sd_cfg_now.local[i].val.i);
	    break;

	case SNDDRV_FLOAT:
	    sprintf(buf, "%7.3f", sd_cfg_now.local[i].val.f);
	    break;
	}

	hbox = PACK_HBOX(NULL);
	{
	    PACK_LABEL(hbox, p->title);

	    PACK_ENTRY(hbox,
		       8, 9, buf,
		       cb_audio_config, (void *)(i*2),
		       cb_audio_config, (void *)(i*2 + 1));
	}
	q8tk_box_pack_start(box, hbox);
	PACK_LABEL(box, "");
    }

    return i;
}


/*----------------------------------------------------------------------*/

static	void	audio_create(void)
{
    int i;
    Q8tkWidget *vbox;
    const t_menulabel *l = data_volume;

    vbox = PACK_VBOX(NULL);
    {
	PACK_HSEP(vbox);

#ifdef	USE_FMGEN
	q8tk_box_pack_start(vbox, volume_audio_fmgen());
	PACK_HSEP(vbox);
#else
	PACK_LABEL(vbox, "");
	PACK_LABEL(vbox, "");
#endif
	q8tk_box_pack_start(vbox, volume_audio_freq());
	PACK_LABEL(vbox, "");
	q8tk_box_pack_start(vbox, volume_audio_sample());
	PACK_HSEP(vbox);

	i = audio_config_widget(vbox);

	for ( ; i<5; i++) {
	    PACK_LABEL(vbox, "");
	    PACK_LABEL(vbox, "");
	}
	PACK_HSEP(vbox);
    }

    audio_window = vbox;
}

static	void	cb_volume_audio_end(UNUSED_WIDGET, UNUSED_PARM)
{
    audio_finish();
}

static	void	audio_start(void)
{
    Q8tkWidget *w, *f, *v, *h, *b, *l;
    const t_menulabel *p = data_volume;

    {						/* �ᥤ��Ȥʤ륦����ɥ� */
	w = q8tk_window_new(Q8TK_WINDOW_DIALOG);
	audio_accel = q8tk_accel_group_new();
	q8tk_accel_group_attach(audio_accel, w);
    }
    {						/* �ˡ��ե졼���褻�� */
	f = q8tk_frame_new(GET_LABEL(p, DATA_VOLUME_AUDIO_SET));
	q8tk_frame_set_shadow_type(f, Q8TK_SHADOW_OUT);
	q8tk_container_add(w, f);
	q8tk_widget_show(f);
    }
    {						/* ����˥ܥå�����褻�� */
	v = q8tk_vbox_new();
	q8tk_container_add(f, v);
	q8tk_widget_show(v);
							/* �ܥå����ˤ�     */
	{						/* AUDIO��˥塼 �� */
	    q8tk_box_pack_start(v, audio_window);
	}
	{						/* ����˥ܥå���   */
	    h = q8tk_hbox_new();
	    q8tk_box_pack_start(v, h);
	    q8tk_widget_show(h);
							/* �ܥå����ˤ�     */
	    {						/* ��λ�ܥ�������� */
		b = PACK_BUTTON(h,
				GET_LABEL(p, DATA_VOLUME_AUDIO_QUIT),
				cb_volume_audio_end, NULL);

		q8tk_accel_group_add(audio_accel, Q8TK_KEY_ESC, b, "clicked");


							/* ��٥������ */
		l = PACK_LABEL(h, GET_LABEL(p, DATA_VOLUME_AUDIO_INFO));
		q8tk_misc_set_placement(l, 0, Q8TK_PLACEMENT_Y_CENTER);
	    }
	}
    }

    q8tk_widget_show(w);
    q8tk_grab_add(w);

    q8tk_widget_set_focus(b);


    audio[ AUDIO_WIN    ] = w;	/* �����������Ĥ����Ȥ��������� */
    audio[ AUDIO_FRAME  ] = f;	/* �������åȤ�Ф��Ƥ����ޤ�     */
    audio[ AUDIO_VBOX   ] = v;
    audio[ AUDIO_HBOX   ] = h;
    audio[ AUDIO_BUTTON ] = b;
    audio[ AUDIO_LABEL  ] = l;
}

/* ������ɾܺ٥�����ɥ��ξõ� */

static	void	audio_finish(void)
{
    q8tk_widget_destroy(audio[ AUDIO_LABEL  ]);
    q8tk_widget_destroy(audio[ AUDIO_BUTTON ]);
    q8tk_widget_destroy(audio[ AUDIO_HBOX   ]);
    q8tk_widget_destroy(audio[ AUDIO_VBOX   ]);
    q8tk_widget_destroy(audio[ AUDIO_FRAME  ]);

    q8tk_grab_remove(audio[ AUDIO_WIN ]);
    q8tk_widget_destroy(audio[ AUDIO_WIN ]);
    q8tk_widget_destroy(audio_accel);
}

#endif



/*===========================================================================
 *
 *	�ᥤ��ڡ���	�ǥ�����
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/

typedef struct{
    Q8tkWidget	*list;			/* ���᡼�������Υꥹ��	*/
/*  Q8tkWidget	*button[2];		 * �ܥ����		*/
    Q8tkWidget	*label[2];		/* ���Υ�٥� (2��)	*/
    int		func[2];		/* �ܥ���ε�ǽ IMG_xxx	*/
    Q8tkWidget	*stat_label;		/* ���� - Busy/Ready	*/
    Q8tkWidget	*attr_label;		/* ���� - RO/RW°��	*/
    Q8tkWidget	*num_label;		/* ���� - ���᡼����	*/
} T_DISK_INFO;

static	T_DISK_INFO	disk_info[2];	/* 2�ɥ饤��ʬ�Υ��	*/

static	char		disk_filename[ QUASI88_MAX_FILENAME ];

static	int		disk_drv;	/* ����ɥ饤�֤��ֹ� */
static	int		disk_img;	/* ���륤�᡼�����ֹ� */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

static	void	set_disk_widget(void);


/* BOOT from DISK �ǡ�DISK �� CLOSE �������䡢
   BOOT from ROM  �ǡ�DISK �� OPEN �������ϡ� DIP-SW ��������ѹ� */
static	void	disk_update_dipsw_b_boot(void)
{
    if (disk_image_exist(0) || disk_image_exist(1)) {
	q8tk_toggle_button_set_state(widget_dipsw_b_boot_disk, TRUE);
    } else {
	q8tk_toggle_button_set_state(widget_dipsw_b_boot_rom,  TRUE);
    }
    set_reset_dipsw_boot();

    /* �ꥻ�åȤ��ʤ��ǥ�˥塼�⡼�ɤ�ȴ��������꤬��¸����ʤ��Τǡ����� */
    boot_from_rom = reset_req.boot_from_rom;		/* thanks floi ! */
}


/*----------------------------------------------------------------------*/
/* °���ѹ��γƼ����							*/

enum {
    ATTR_RENAME,	/* drive[drv] �Υ��᡼�� img ���͡���		*/
    ATTR_PROTECT,	/* drive[drv] �Υ��᡼�� img ��ץ�ƥ���	*/
    ATTR_FORMAT,	/* drive[drv] �Υ��᡼�� img �򥢥�ե����ޥå�	*/
    ATTR_UNFORMAT,	/* drive[drv] �Υ��᡼�� img ��ե����ޥå�	*/
    ATTR_APPEND,	/* drive[drv] �˺Ǹ�˥��᡼�����ɲ�		*/
    ATTR_CREATE		/* �����˥ǥ��������᡼���ե���������		*/
};

static	void	sub_disk_attr_file_ctrl(int drv, int img, int cmd, char *c)
{
    int ro = FALSE;
    int result = -1;
    OSD_FILE *fp;


    if (cmd != ATTR_CREATE) {		/* �ɥ饤�֤Υե�������ѹ������� */

	fp = drive[ drv ].fp;			/* ���Υե�����ݥ��󥿤����*/
	if (drive[ drv ].read_only) {
	    ro = TRUE;
	}

    } else {				/* �̤Υե�����򹹿������� */

	fp = osd_fopen(FTYPE_DISK, c, "r+b");		/* "r+b" �ǥ����ץ� */
	if (fp == NULL) {
	    fp = osd_fopen(FTYPE_DISK, c, "rb");	/* "rb" �ǥ����ץ� */
	    if (fp) ro = TRUE;
	}

	if (fp) {					/* �����ץ�Ǥ����� */
	    if      (fp == drive[ 0 ].fp) drv = 0;	/* ���Ǥ˥ɥ饤�֤� */
	    else if (fp == drive[ 1 ].fp) drv = 1;	/* �����Ƥʤ�����   */
	    else                          drv = -1;	/* �����å�����     */
	}
	else {						/* �����ץ�Ǥ��ʤ� */
	    fp = osd_fopen(FTYPE_DISK, c, "ab");	/* ���ϡ������˺��� */
	    drv = -1;
	}

    }


    if (fp == NULL) {			/* �����ץ��� */
	start_file_error_dialog(drv, ERR_CANT_OPEN);
	return;
    }
    else if (ro) {			/* �꡼�ɥ���꡼�ʤΤǽ����Բ� */
	if (drv < 0) osd_fclose(fp);
	if (cmd != ATTR_CREATE) start_file_error_dialog(drv, ERR_READ_ONLY);
	else                    start_file_error_dialog( -1, ERR_READ_ONLY);
	return;
    }
    else if (drv>=0 &&			/* ���줿���᡼�����ޤޤ��Τ��Բ� */
	     drive[ drv ].detect_broken_image) {
	start_file_error_dialog(drv, ERR_MAYBE_BROKEN);
	return;
    }


#if 0
    if (cmd == ATTR_CREATE || cmd == ATTR_APPEND) {
	/* ���ν����˻��֤�������褦�ʾ�硢��å�������������� */
	/* ���ν���������ʤ˻��֤������뤳�ȤϤʤ����� */
    }
#endif

		/* �������ե�������Ф��ơ����� */

    switch (cmd) {
    case ATTR_RENAME:	result = d88_write_name(fp, drv, img, c);	break;
    case ATTR_PROTECT:	result = d88_write_protect(fp, drv, img, c);	break;
    case ATTR_FORMAT:	result = d88_write_format(fp, drv, img);	break;
    case ATTR_UNFORMAT:	result = d88_write_unformat(fp, drv, img);	break;
    case ATTR_APPEND:
    case ATTR_CREATE:	result = d88_append_blank(fp, drv);		break;
    }

		/* ���η�� */

    switch (result) {
    case D88_SUCCESS:	result = ERR_NO;			break;
    case D88_NO_IMAGE:	result = ERR_MAYBE_BROKEN;		break;
    case D88_BAD_IMAGE:	result = ERR_MAYBE_BROKEN;		break;
    case D88_ERR_READ:	result = ERR_MAYBE_BROKEN;		break;
    case D88_ERR_SEEK:	result = ERR_SEEK;			break;
    case D88_ERR_WRITE:	result = ERR_WRITE;			break;
    default:		result = ERR_UNEXPECTED;		break;
    }

		/* ��λ�������ʤ������顼���ϥ�å�������Ф� */

    if (drv < 0) {		/* ���������ץ󤷤��ե�����򹹿�������� */
	osd_fclose(fp);			/* �ե�������Ĥ��ƽ����	  */

    } else {			/* �ɥ饤�֤Υե�����򹹿��������	  */
	if (result == ERR_NO) {		/* ��˥塼���̤򹹿����ͤ�	  */
	    set_disk_widget();
	    if (cmd != ATTR_CREATE) disk_update_dipsw_b_boot();
	}
    }

    if (result != ERR_NO) {
	start_file_error_dialog(drv, result);
    }

    return;
}

/*----------------------------------------------------------------------*/
/* �֥�͡���ץ�������						*/

static	void	cb_disk_attr_rename_activate(UNUSED_WIDGET, void *p)
{
    char wk[16 + 1];

    if ((int)p) {		/* dialog_destroy() �����˥���ȥ�򥲥å� */
	strncpy(wk, dialog_get_entry(), 16);
	wk[16] = '\0';
    }

    dialog_destroy();

    if ((int)p) {
	sub_disk_attr_file_ctrl(disk_drv, disk_img, ATTR_RENAME, wk);
    }
}
static	void	sub_disk_attr_rename(const char *image_name)
{
    int save_code;
    const t_menulabel *l = data_disk_attr_rename;


    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_RENAME_TITLE1 +disk_drv));

	save_code = q8tk_set_kanjicode(Q8TK_KANJI_SJIS);
	dialog_set_title(image_name);
	q8tk_set_kanjicode(save_code);

	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_RENAME_TITLE2));

	dialog_set_separator();

	save_code = q8tk_set_kanjicode(Q8TK_KANJI_SJIS);
	dialog_set_entry(drive[disk_drv].image[disk_img].name,
			 16,
			 cb_disk_attr_rename_activate, (void*)TRUE);
	q8tk_set_kanjicode(save_code);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_RENAME_OK),
			  cb_disk_attr_rename_activate, (void*)TRUE);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_RENAME_CANCEL),
			  cb_disk_attr_rename_activate, (void*)FALSE);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}

/*----------------------------------------------------------------------*/
/* �֥ץ�ƥ��ȡץ�������						*/

static	void	cb_disk_attr_protect_clicked(UNUSED_WIDGET, void *p)
{
    char c;

    dialog_destroy();

    if ((int)p) {
	if ((int)p == 1) c = DISK_PROTECT_TRUE;
	else             c = DISK_PROTECT_FALSE;

	sub_disk_attr_file_ctrl(disk_drv, disk_img, ATTR_PROTECT, &c);
    }
}
static	void	sub_disk_attr_protect(const char *image_name)
{
    int save_code;
    const t_menulabel *l = data_disk_attr_protect;

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_PROTECT_TITLE1+disk_drv));

	save_code = q8tk_set_kanjicode(Q8TK_KANJI_SJIS);
	dialog_set_title(image_name);
	q8tk_set_kanjicode(save_code);

	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_PROTECT_TITLE2));

	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_PROTECT_SET),
			  cb_disk_attr_protect_clicked, (void*)1);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_PROTECT_UNSET),
			  cb_disk_attr_protect_clicked, (void*)2);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_PROTECT_CANCEL),
			  cb_disk_attr_protect_clicked, (void*)0);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}

/*----------------------------------------------------------------------*/
/* �֥ե����ޥåȡץ�������						*/

static	void	cb_disk_attr_format_clicked(UNUSED_WIDGET, void *p)
{
    dialog_destroy();

    if ((int)p) {
	if ((int)p == 1)
	    sub_disk_attr_file_ctrl(disk_drv, disk_img, ATTR_FORMAT,   NULL);
	else 
	    sub_disk_attr_file_ctrl(disk_drv, disk_img, ATTR_UNFORMAT, NULL);
    }
}
static	void	sub_disk_attr_format(const char *image_name)
{
    int save_code;
    const t_menulabel *l = data_disk_attr_format;

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_FORMAT_TITLE1 +disk_drv));

	save_code = q8tk_set_kanjicode(Q8TK_KANJI_SJIS);
	dialog_set_title(image_name);
	q8tk_set_kanjicode(save_code);

	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_FORMAT_TITLE2));

	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_FORMAT_WARNING));

	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_FORMAT_DO),
			  cb_disk_attr_format_clicked, (void*)1);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_FORMAT_NOT),
			  cb_disk_attr_format_clicked, (void*)2);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_FORMAT_CANCEL),
			  cb_disk_attr_format_clicked, (void*)0);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}

/*----------------------------------------------------------------------*/
/* �֥֥�󥯥ǥ������ץ�������					*/

static	void	cb_disk_attr_blank_clicked(UNUSED_WIDGET, void *p)
{
    dialog_destroy();

    if ((int)p) {
	sub_disk_attr_file_ctrl(disk_drv, disk_img, ATTR_APPEND, NULL);
    }
}
static	void	sub_disk_attr_blank(void)
{
    const t_menulabel *l = data_disk_attr_blank;

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_BLANK_TITLE1 + disk_drv));

	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_BLANK_TITLE2));

	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_BLANK_OK),
			  cb_disk_attr_blank_clicked, (void*)TRUE);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_BLANK_CANCEL),
			  cb_disk_attr_blank_clicked, (void*)FALSE);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}

/*----------------------------------------------------------------------*/
/* ��°���ѹ��� �ܥ��󲡲����ν���  -  �ܺ�����Υ��������򳫤�	*/

static char disk_attr_image_name[20];
static	void	cb_disk_attr_clicked(UNUSED_WIDGET, void *p)
{
    char *name = disk_attr_image_name;

    dialog_destroy();

    switch ((int)p) {
    case DATA_DISK_ATTR_RENAME:	 sub_disk_attr_rename(name);	break;
    case DATA_DISK_ATTR_PROTECT: sub_disk_attr_protect(name);	break;
    case DATA_DISK_ATTR_FORMAT:	 sub_disk_attr_format(name);	break;
    case DATA_DISK_ATTR_BLANK:	 sub_disk_attr_blank();		break;
    }
}


static void sub_disk_attr(void)
{
    int save_code;
    const t_menulabel *l = data_disk_attr;

    sprintf(disk_attr_image_name,		/* ���᡼��̾�򥻥å� */
	    "\"%-16s\"", drive[disk_drv].image[disk_img].name);

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_TITLE1 + disk_drv));

	save_code = q8tk_set_kanjicode(Q8TK_KANJI_SJIS);
	dialog_set_title(disk_attr_image_name);
	q8tk_set_kanjicode(save_code);

	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_TITLE2));

	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_RENAME),
			  cb_disk_attr_clicked, (void*)DATA_DISK_ATTR_RENAME);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_PROTECT),
			  cb_disk_attr_clicked, (void*)DATA_DISK_ATTR_PROTECT);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_FORMAT),
			  cb_disk_attr_clicked, (void*)DATA_DISK_ATTR_FORMAT);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_BLANK),
			  cb_disk_attr_clicked, (void*)DATA_DISK_ATTR_BLANK);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_CANCEL),
			  cb_disk_attr_clicked, (void*)DATA_DISK_ATTR_CANCEL);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}




/*----------------------------------------------------------------------*/
/* �֥��᡼���ե�����򳫤��� �ܥ��󲡲����ν���			*/

static	int	disk_open_ro;
static	int	disk_open_cmd;
static void sub_disk_open_ok(void);

static void sub_disk_open(int cmd)
{
    const char *initial;
    int num;
    const t_menulabel *l = (disk_drv == 0) ? data_disk_open_drv1
					   : data_disk_open_drv2;

    disk_open_cmd = cmd;
    num = (cmd == IMG_OPEN) ? DATA_DISK_OPEN_OPEN : DATA_DISK_OPEN_BOTH;

    /* �ǥ�����������Ф��Υե�����򡢤ʤ���Хǥ������ѥǥ��쥯�ȥ����� */
    initial = filename_get_disk_or_dir(disk_drv);

    START_FILE_SELECTION(GET_LABEL(l, num),
			 (menu_readonly) ? 1 : 0,     /* ReadOnly �����򤬲� */
			 initial,

			 sub_disk_open_ok,
			 disk_filename,
			 QUASI88_MAX_FILENAME,
			 &disk_open_ro);
}

static void sub_disk_open_ok(void)
{
    if (disk_open_cmd == IMG_OPEN) {

	if (quasi88_disk_insert(disk_drv, disk_filename, 0, disk_open_ro)
								    == FALSE) {
	    start_file_error_dialog(disk_drv, ERR_CANT_OPEN);
	}
	else {
	    if (disk_same_file()) {	/* ȿ��¦��Ʊ���ե�������ä���� */
		int dst = disk_drv;
		int src = disk_drv ^ 1;
		int img;

		if (drive[ src ].empty) {	    /* ȿ��¦�ɥ饤�� ���ʤ� */
		    img = 0;			    /*        �ǽ�Υ��᡼�� */
		} else {
		    if (disk_image_num(src) == 1) { /* ���᡼����1�Ĥξ��� */
			img = -1;		    /*        �ɥ饤�� ����  */

		    } else {			    /* ���᡼����ʣ�������  */
						    /*        ��(��)���᡼�� */
			img = disk_image_selected(src)
						+ ((dst == DRIVE_1) ? -1 : +1);
			if ((img < 0) || 
			    (disk_image_num(dst) - 1 < img)) img = -1;
		    }
		}
		if (img < 0) drive_set_empty(dst);
		else         disk_change_image(dst, img);
	    }
	}

    } else {	/*   IMG_BOTH */

	if (quasi88_disk_insert_all(disk_filename, disk_open_ro) == FALSE) {

	    disk_drv = 0;
	    start_file_error_dialog(disk_drv, ERR_CANT_OPEN);

	}

    }

    if (filename_synchronize) {
	sub_misc_suspend_update();
	sub_misc_snapshot_update();
	sub_misc_waveout_update();
    }
    set_disk_widget();
    disk_update_dipsw_b_boot();
}

/*----------------------------------------------------------------------*/
/* �֥��᡼���ե�������Ĥ���� �ܥ��󲡲����ν���			*/

static void sub_disk_close(void)
{
    quasi88_disk_eject(disk_drv);

    if (filename_synchronize) {
	sub_misc_suspend_update();
	sub_misc_snapshot_update();
	sub_misc_waveout_update();
    }
    set_disk_widget();
    disk_update_dipsw_b_boot();
}

/*----------------------------------------------------------------------*/
/* ��ȿ�Хɥ饤�֤�Ʊ���ե�����򳫤��� �ܥ��󲡲����ν���		*/

static void sub_disk_copy(void)
{
    int dst = disk_drv;
    int src = disk_drv ^ 1;
    int img;

    if (! disk_image_exist(src)) return;

    if (drive[ src ].empty) {			/* ȿ��¦�ɥ饤�� ���ʤ� */
	img = 0;				/*        �ǽ�Υ��᡼�� */
    } else {
	if (disk_image_num(src) == 1) {		/* ���᡼����1�Ĥξ��� */
	    img = -1;				/*        �ɥ饤�� ����  */

	} else {				/* ���᡼����ʣ�������  */
						/*        ��(��)���᡼�� */
	    img = disk_image_selected(src) + ((dst == DRIVE_1) ? -1 : +1);
	    if ((img < 0) || 
		(disk_image_num(dst) -1 < img)) img = -1;
	}
    }

    if (quasi88_disk_insert_A_to_B(src, dst, img) == FALSE) {
	start_file_error_dialog(disk_drv, ERR_CANT_OPEN);
    }

    if (filename_synchronize) {
	sub_misc_suspend_update();
	sub_misc_snapshot_update();
	sub_misc_waveout_update();
    }
    set_disk_widget();
    disk_update_dipsw_b_boot();
}





/*----------------------------------------------------------------------*/
/* ���᡼���Υꥹ�ȥ����ƥ�������Ρ�������Хå��ؿ�			*/

static	void	cb_disk_image(UNUSED_WIDGET, void *p)
{
    int drv = ((int)p) & 0xff;
    int img = ((int)p) >> 8;

    if (img < 0) {			/* img == -1 �� <<�ʤ�>> */
	drive_set_empty(drv);
    } else {				/* img >= 0 �ʤ� ���᡼���ֹ� */
	drive_unset_empty(drv);
	disk_change_image(drv, img);
    }
}

/*----------------------------------------------------------------------*/
/* �ɥ饤�����¸�ߤ���ܥ���Ρ�������Хå��ؿ�			*/

static	void	cb_disk_button(UNUSED_WIDGET, void *p)
{
    int drv    = ((int)p) & 0xff;
    int button = ((int)p) >> 8;

    disk_drv = drv;
    disk_img = disk_image_selected(drv);

    switch (disk_info[drv].func[button]) {
    case IMG_OPEN:
    case IMG_BOTH:
	sub_disk_open(disk_info[drv].func[button]);
	break;
    case IMG_CLOSE:
	sub_disk_close();
	break;
    case IMG_COPY:
	sub_disk_copy();
	break;
    case IMG_ATTR:
	if (! drive_check_empty(drv)) {	     /* ���᡼��<<�ʤ�>>�������̵�� */
	    sub_disk_attr();
	}
	break;
    }
}

/*----------------------------------------------------------------------*/
/* �ե�����򳫤���ˡ�disk_info[] �˾���򥻥å�			*/
/*		(���᡼���Υꥹ���������ܥ��󡦾���Υ�٥�򥻥å�)	*/

static	void	set_disk_widget(void)
{
    int i, drv, save_code;
    Q8tkWidget *item;
    T_DISK_INFO *w;
    const t_menulabel *inf = data_disk_info;
    const t_menulabel *l   = data_disk_image;
    const t_menulabel *btn;
    char wk[40], wk2[20];
    const char *s;


    for (drv=0; drv<2; drv++) {
	w = &disk_info[drv];

	if (menu_swapdrv) {
	    btn = (drv == 0) ? data_disk_button_drv1swap
			     : data_disk_button_drv2swap;
	} else {
	    btn = (drv == 0) ? data_disk_button_drv1
			     : data_disk_button_drv2;
	}

		/* ���᡼��̾�� LIST ITEM ���� */

	q8tk_listbox_clear_items(w->list, 0, -1);

	item = q8tk_list_item_new_with_label(GET_LABEL(l,
						       DATA_DISK_IMAGE_EMPTY));
	q8tk_widget_show(item);
	q8tk_container_add(w->list, item);		/* <<�ʤ�>> ITEM */
	q8tk_signal_connect(item, "select",
			    cb_disk_image, (void *)((-1 << 8) + drv));

	if (disk_image_exist(drv)) {		/* ---- �ǥ����������� ---- */
	    save_code = q8tk_set_kanjicode(Q8TK_KANJI_SJIS);
	    {
		for (i=0; i<disk_image_num(drv); i++) {
		    sprintf(wk, "%3d  %-16s  %s ", /*���᡼��No ���᡼��̾ RW*/
			    i+1,
			    drive[drv].image[i].name,
			    (drive[drv].image[i].protect) ? "RO" : "RW");

		    item = q8tk_list_item_new_with_label(wk);
		    q8tk_widget_show(item);
		    q8tk_container_add(w->list, item);
		    q8tk_signal_connect(item, "select",
					cb_disk_image, (void *)((i<<8) + drv));
		}
	    }
	    q8tk_set_kanjicode(save_code);

				/* <<�ʤ�>> or ����image �� ITEM �򥻥쥯�� */
	    if (drive_check_empty(drv)) i = 0;
	    else                        i = disk_image_selected(drv) + 1;
	    q8tk_listbox_select_item(w->list, i);

	} else {				/* ---- �ɥ饤�ֶ��ä� ---- */
	    q8tk_listbox_select_item(w->list, 0);	    /* <<�ʤ�>> ITEM */
	}

		/* �ܥ���ε�ǽ ���Ĥ���ס�°���ѹ��� / �ֳ����סֳ����� */

	if (disk_image_exist(drv)) {
	    w->func[0] = IMG_CLOSE;
	    w->func[1] = IMG_ATTR;
	} else {
	    w->func[0] = (disk_image_exist(drv^1)) ? IMG_COPY : IMG_BOTH;
	    w->func[1] = IMG_OPEN;
	}
	q8tk_label_set(w->label[0], GET_LABEL(btn, w->func[0]));
	q8tk_label_set(w->label[1], GET_LABEL(btn, w->func[1]));

		/* ���� - Busy/Ready */

	if (get_drive_ready(drv)) s = GET_LABEL(inf,DATA_DISK_INFO_STAT_READY);
	else                      s = GET_LABEL(inf,DATA_DISK_INFO_STAT_BUSY);
	q8tk_label_set(w->stat_label, s);
	q8tk_label_set_reverse(w->stat_label,	/* BUSY�ʤ�ȿžɽ�� */
			       (get_drive_ready(drv)) ? FALSE : TRUE);

		/* ���� - RO/RW°�� */

	if (disk_image_exist(drv)) {
	    if (drive[drv].read_only) s =GET_LABEL(inf,DATA_DISK_INFO_ATTR_RO);
	    else                      s =GET_LABEL(inf,DATA_DISK_INFO_ATTR_RW);
	} else {
	    s = "";
	}
	q8tk_label_set(w->attr_label, s);
	q8tk_label_set_color(w->attr_label,	/* ReadOnly�ʤ��ֿ�ɽ�� */
			     (drive[drv].read_only) ? Q8GR_PALETTE_RED : -1);

		/* ���� - ���᡼���� */

	if (disk_image_exist(drv)) {
	    if (drive[drv].detect_broken_image) {	/* ��»���� */
		s = GET_LABEL(inf, DATA_DISK_INFO_NR_BROKEN);
	    } else
	    if (drive[drv].over_image ||		/* ���᡼��¿�᤮ */
		disk_image_num(drv) > 99) {
		s = GET_LABEL(inf, DATA_DISK_INFO_NR_OVER);
	    } else {
		s = "";
	    }
	    sprintf(wk, "%2d%s",
		    (disk_image_num(drv)>99) ? 99 : disk_image_num(drv), s);
	    sprintf(wk2, "%9.9s", wk);			/* 9ʸ�����ͤ���Ѵ� */
	} else {
	    wk2[0] = '\0';
	}
	q8tk_label_set(w->num_label,  wk2);
    }
}


/*----------------------------------------------------------------------*/
/* �֥֥�󥯺����� �ܥ��󲡲����ν���					*/

static	void	sub_disk_blank_ok(void);
static	void	cb_disk_blank_warn_clicked(Q8tkWidget *, void *);


static	void	cb_disk_blank(UNUSED_WIDGET, UNUSED_PARM)
{
    const char *initial;
    const t_menulabel *l = data_disk_blank;

    /* �ǥ�����������Ф��Υե�����򡢤ʤ���Хǥ������ѥǥ��쥯�ȥ����� */
    initial = filename_get_disk_or_dir(DRIVE_1);

    START_FILE_SELECTION(GET_LABEL(l, DATA_DISK_BLANK_FSEL),
			 -1,	/* ReadOnly ��������Բ� */
			 initial,

			 sub_disk_blank_ok,
			 disk_filename,
			 QUASI88_MAX_FILENAME,
			 NULL);
}

static	void	sub_disk_blank_ok(void)
{
    const t_menulabel *l = data_disk_blank;

    switch (osd_file_stat(disk_filename)) {

    case FILE_STAT_NOEXIST:
	/* �ե�����򿷵��˺��������֥�󥯤���� */
	sub_disk_attr_file_ctrl(0, 0, ATTR_CREATE, disk_filename);
	break;

    case FILE_STAT_DIR:
	/* �ǥ��쥯�ȥ�ʤΤǡ��֥�󥯤��ɲäǤ��ʤ� */
	start_file_error_dialog(-1, ERR_CANT_OPEN);
	break;

    default:
	/* ���Ǥ˥ե����뤬¸�ߤ��ޤ����֥�󥯤��ɲä��ޤ����� */
	dialog_create();
	{
	    dialog_set_title(GET_LABEL(l, DATA_DISK_BLANK_WARN_0));

	    dialog_set_title(GET_LABEL(l, DATA_DISK_BLANK_WARN_1));

	    dialog_set_separator();

	    dialog_set_button(GET_LABEL(l, DATA_DISK_BLANK_WARN_APPEND),
			      cb_disk_blank_warn_clicked, (void*)TRUE);

	    dialog_set_button(GET_LABEL(l, DATA_DISK_BLANK_WARN_CANCEL),
			      cb_disk_blank_warn_clicked, (void*)FALSE);

	    dialog_accel_key(Q8TK_KEY_ESC);
	}
	dialog_start();
	break;
    }
}

static	void	cb_disk_blank_warn_clicked(UNUSED_WIDGET, void *p)
{
    dialog_destroy();

    if ((int)p) {
	/* �ե�����ˡ��֥�󥯤��ɵ� */
	sub_disk_attr_file_ctrl(0, 0, ATTR_CREATE, disk_filename);
    }
}

/*----------------------------------------------------------------------*/
/* �֥ե�����̾��ǧ�� �ܥ��󲡲����ν���				*/

static	void	cb_disk_fname_dialog_ok(UNUSED_WIDGET, UNUSED_PARM)
{
    dialog_destroy();
}

static	void	cb_disk_fname(UNUSED_WIDGET, UNUSED_PARM)
{
    const t_menulabel *l = data_disk_fname;
    char filename[66 +5 +1];		/* 5 == strlen("[1:] "), 1 �� '\0' */
    int save_code;
    int i, width, len;
    const char *ptr[2];
    const char *none = "(No Image File)";

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_DISK_FNAME_TITLE));
	dialog_set_title(GET_LABEL(l, DATA_DISK_FNAME_LINE));

	{
	    save_code = q8tk_set_kanjicode(osd_kanji_code());

	    width = 0;
	    for (i=0; i<2; i++) {
		if ((ptr[i] = filename_get_disk(i)) == NULL) { ptr[i] = none; }
		len = sprintf(filename, "%.66s", ptr[i]);	/* == max 66 */
		width = MAX(width, len);
	    }

	    for (i=0; i<2; i++) {
		sprintf(filename, "[%d:] %-*.*s", i+1, width, width, ptr[i]);
		dialog_set_title(filename);
	    }

	    q8tk_set_kanjicode(save_code);
	}

	if (disk_image_exist(0) && disk_same_file()) {
	    dialog_set_title(GET_LABEL(l, DATA_DISK_FNAME_SAME));
	}

	if ((disk_image_exist(0) && drive[0].read_only) ||
	    (disk_image_exist(1) && drive[1].read_only)) {
	    dialog_set_title(GET_LABEL(l, DATA_DISK_FNAME_SEP));
	    dialog_set_title(GET_LABEL(l, DATA_DISK_FNAME_RO));

	    if (fdc_ignore_readonly == FALSE) {
		dialog_set_title(GET_LABEL(l, DATA_DISK_FNAME_RO_1));
		dialog_set_title(GET_LABEL(l, DATA_DISK_FNAME_RO_2));
	    } else {
		dialog_set_title(GET_LABEL(l, DATA_DISK_FNAME_RO_X));
		dialog_set_title(GET_LABEL(l, DATA_DISK_FNAME_RO_Y));
	    }
	}


	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_DISK_FNAME_OK),
			  cb_disk_fname_dialog_ok, NULL);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}


/*----------------------------------------------------------------------*/
					/* �ɥ饤��ɽ������ �������촹�� */

static	void	cb_disk_dispswap_clicked(UNUSED_WIDGET, UNUSED_PARM)
{
    dialog_destroy();
}

static	int	get_disk_dispswap(void)
{
    return menu_swapdrv;
}
static	void	cb_disk_dispswap(Q8tkWidget *widget, UNUSED_PARM)
{
    const t_menulabel *l = data_disk_dispswap;
    int parm = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    menu_swapdrv = parm;

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_DISK_DISPSWAP_INFO_1));
	dialog_set_title(GET_LABEL(l, DATA_DISK_DISPSWAP_INFO_2));

	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_DISK_DISPSWAP_OK),
			  cb_disk_dispswap_clicked, NULL);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}

/*----------------------------------------------------------------------*/
					/* ���ơ������˥��᡼��̾ɽ�� */

static	void	cb_disk_dispstatus_clicked(UNUSED_WIDGET, UNUSED_PARM)
{
    dialog_destroy();
}

static	int	get_disk_dispstatus(void)
{
    return status_imagename;
}
static	void	cb_disk_dispstatus(Q8tkWidget *widget, UNUSED_PARM)
{
    const t_menulabel *l = data_disk_dispstatus;
    int parm = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    status_imagename = parm;

    if (status_imagename) {
	/* ����ˤ������Τߡ�������ɽ�� */
	dialog_create();
	{
	    dialog_set_title(GET_LABEL(l, DATA_DISK_DISPSTATUS_INFO));

	    dialog_set_separator();

	    dialog_set_button(GET_LABEL(l, DATA_DISK_DISPSTATUS_OK),
			      cb_disk_dispstatus_clicked, NULL);

	    dialog_accel_key(Q8TK_KEY_ESC);
	}
	dialog_start();
    }
}

/*======================================================================*/

static	Q8tkWidget	*menu_disk(void)
{
    Q8tkWidget *hbox, *vbox, *swin, *lab, *btn;
    Q8tkWidget *f, *vx, *hx;
    T_DISK_INFO *w;
    int i,j,k;
    const t_menulabel *l;


    hbox = PACK_HBOX(NULL);
    {
	for (k=0; k<COUNTOF(disk_info); k++) {

	    if (menu_swapdrv) { i = k ^ 1; }
	    else              { i = k;     }

	    w = &disk_info[i];
	    {
		vbox = PACK_VBOX(hbox);
		{
		    lab = PACK_LABEL(vbox, GET_LABEL(data_disk_image_drive,i));

		    if (menu_swapdrv)
			q8tk_misc_set_placement(lab, Q8TK_PLACEMENT_X_RIGHT,0);

		    {
			swin  = q8tk_scrolled_window_new(NULL, NULL);
			q8tk_widget_show(swin);
			q8tk_scrolled_window_set_policy(swin,
							Q8TK_POLICY_NEVER,
							Q8TK_POLICY_AUTOMATIC);
			q8tk_misc_set_size(swin, 29, 11);

			w->list = q8tk_listbox_new();
			q8tk_widget_show(w->list);
			q8tk_container_add(swin, w->list);

			q8tk_box_pack_start(vbox, swin);
		    }

		    for (j=0; j<2; j++) {
#if 0
						/* ����٥�Υ������åȳ��� */
			w->label[j] = q8tk_label_new("");
			q8tk_widget_show(w->label[j]);
			w->button[j] = q8tk_button_new();
			q8tk_widget_show(w->button[j]);
			q8tk_container_add(w->button[j], w->label[j]);
			q8tk_signal_connect(w->button[j], "clicked",
					    cb_disk_button,
					    (void *)((j << 8) + i));

			q8tk_box_pack_start(vbox, w->button[j]);
#else
						/* ����٥�Υ������åȳ��� */
			w->label[j] = q8tk_label_new("");
			q8tk_widget_show(w->label[j]);
			btn = q8tk_button_new();
			q8tk_widget_show(btn);
			q8tk_container_add(btn, w->label[j]);
			q8tk_signal_connect(btn, "clicked",
					    cb_disk_button,
					    (void *)((j << 8) + i));

			q8tk_box_pack_start(vbox, btn);
#endif
		    }
		}
	    }

	    PACK_VSEP(hbox);
	}

	{
	    vbox = PACK_VBOX(hbox);
	    {
		l = data_disk_info;
		for (i=0; i<COUNTOF(disk_info); i++) {
		    w = &disk_info[i];

		    vx = PACK_VBOX(NULL);
		    {
			hx = PACK_HBOX(vx);
			{
			    PACK_LABEL(hx, GET_LABEL(l, DATA_DISK_INFO_STAT));
						/* ����٥�Υ������åȳ��� */
			    w->stat_label = PACK_LABEL(hx, "");
			}

			hx = PACK_HBOX(vx);
			{
			    PACK_LABEL(hx, GET_LABEL(l, DATA_DISK_INFO_ATTR));
						/* ����٥�Υ������åȳ��� */
			    w->attr_label = PACK_LABEL(hx, "");
			}

			hx = PACK_HBOX(vx);
			{
			    PACK_LABEL(hx, GET_LABEL(l, DATA_DISK_INFO_NR));
						/* ����٥�Υ������åȳ��� */
			    w->num_label = PACK_LABEL(hx, "");
			    q8tk_misc_set_placement(w->num_label,
						    Q8TK_PLACEMENT_X_RIGHT, 0);
			}
		    }

		    f = PACK_FRAME(vbox,
				   GET_LABEL(data_disk_info_drive, i), vx);
		    q8tk_frame_set_shadow_type(f, Q8TK_SHADOW_IN);
		}

		hx = PACK_HBOX(vbox);
		{
		    PACK_BUTTON(hx,
				GET_LABEL(data_disk_fname, DATA_DISK_FNAME),
				cb_disk_fname, NULL);
		}

		PACK_CHECK_BUTTON(vbox,
				  GET_LABEL(data_disk_dispswap,
					    DATA_DISK_DISPSWAP),
				  get_disk_dispswap(),
				  cb_disk_dispswap, NULL);

		PACK_CHECK_BUTTON(vbox,
				  GET_LABEL(data_disk_dispstatus, 0),
				  get_disk_dispstatus(),
				  cb_disk_dispstatus, NULL);

#if 0
		for (i=0; i<1; i++)	    /* ����Ĵ���Τ�����ߡ��򲿸Ĥ� */
		    PACK_LABEL(vbox, "");
#endif

		PACK_BUTTON(vbox,
			    GET_LABEL(data_disk_image, DATA_DISK_IMAGE_BLANK),
			    cb_disk_blank, NULL);
	    }
	}
    }


    set_disk_widget();

    return hbox;
}



/*===========================================================================
 *
 *	�ᥤ��ڡ���	��������
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
static Q8tkWidget *fkey_widget[20+1][2];

				    /* �ե��󥯥���󥭡�������Ƥ��ѹ� */
static	int	get_key_fkey(int fn_key)
{
    return (function_f[ fn_key ] < 0x20) ? function_f[ fn_key ] : FN_FUNC;
}
static	void	cb_key_fkey(Q8tkWidget *widget, void *fn_key)
{
    int i;
    const t_menudata *p = data_key_fkey_fn;
    const char       *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(data_key_fkey_fn); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    function_f[ (int)fn_key ] = p->val;
	    q8tk_combo_set_text(fkey_widget[(int)fn_key][1],
				keymap_assign[0].str);
	    return;
	}
    }
}

static	int	get_key_fkey2(int fn_key)
{
    return (function_f[ fn_key ] >= 0x20) ? function_f[ fn_key ]
					  : KEY88_INVALID;
}
static	void	cb_key_fkey2(Q8tkWidget *widget, void *fn_key)
{
    int i;
    const t_keymap *q = keymap_assign;
    const char     *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(keymap_assign); i++, q++) {
	if (strcmp(q->str, combo_str) == 0) {
	    function_f[ (int)fn_key ] = q->code;
	    q8tk_combo_set_text(fkey_widget[(int)fn_key][0],
				data_key_fkey_fn[0].str[menu_lang]);
	    return;
	}
    }
}


static	Q8tkWidget	*menu_key_fkey(void)
{
    int i;
    Q8tkWidget *vbox, *hbox;
    const t_menudata *p = data_key_fkey;

    vbox = PACK_VBOX(NULL);
    {
	for (i=0; i<COUNTOF(data_key_fkey); i++, p++) {

	    hbox = PACK_HBOX(vbox);
	    {
		PACK_LABEL(hbox, GET_LABEL(p, 0));

		fkey_widget[p->val][0] =
		    PACK_COMBO(hbox,
			       data_key_fkey_fn, COUNTOF(data_key_fkey_fn),
			       get_key_fkey(p->val), NULL, 42,
			       cb_key_fkey, (void*)(p->val),
			       NULL, NULL);

		fkey_widget[p->val][1] =
		    MAKE_KEY_COMBO(hbox,
				   &data_key_fkey2[i],
				   get_key_fkey2,
				   cb_key_fkey2);
	    }
	}
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
						      /* ����������ѹ� */
static	int	get_key_cfg(int type)
{
    switch (type) {
    case DATA_KEY_CFG_TENKEY:	return	tenkey_emu;
    case DATA_KEY_CFG_NUMLOCK:	return	numlock_emu;
    }
    return FALSE;
}
static	void	cb_key_cfg(Q8tkWidget *widget, void *type)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    switch ((int)type) {
    case DATA_KEY_CFG_TENKEY:	tenkey_emu  = (key) ? TRUE : FALSE;	break;
    case DATA_KEY_CFG_NUMLOCK:	numlock_emu = (key) ? TRUE : FALSE;	break;
    }
}


static	Q8tkWidget	*menu_key_cfg(void)
{
    Q8tkWidget *vbox;

    vbox = PACK_VBOX(NULL);
    {
	PACK_CHECK_BUTTONS(vbox,
			   data_key_cfg, COUNTOF(data_key_cfg),
			   get_key_cfg, cb_key_cfg);
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
					      /* ���եȥ����������ܡ��� */
static	void	keymap_start(void);
static	void	keymap_finish(void);

static	void	cb_key_softkeyboard(UNUSED_WIDGET, UNUSED_PARM)
{
    keymap_start();
}

static	Q8tkWidget	*menu_key_softkeyboard(void)
{
    Q8tkWidget *button;
    const t_menulabel *l = data_skey_set;

    button = PACK_BUTTON(NULL,
			 GET_LABEL(l, DATA_SKEY_BUTTON_SETUP),
			 cb_key_softkeyboard, NULL);

    return button;
}



/*----------------------------------------------------------------------*/
static	void	menu_key_cursor_setting(void);
					    /* �������륭���������ޥ��� */
				     /* original idea by floi, thanks ! */
static	int		key_cursor_widget_init_done;
static	Q8tkWidget	*key_cursor_widget_sel;
static	Q8tkWidget	*key_cursor_widget_sel_none;
static	Q8tkWidget	*key_cursor_widget_sel_key;

static	int	get_key_cursor_key_mode(void)
{
    return cursor_key_mode;
}
static	void	cb_key_cursor_key_mode(UNUSED_WIDGET, void *p)
{
    cursor_key_mode = (int)p;

    menu_key_cursor_setting();
}
static	int	get_key_cursor_key(int type)
{
    return cursor_key_assign[ type ];
}
static	void	cb_key_cursor_key(Q8tkWidget *widget, void *type)
{
    int i;
    const t_keymap *q = keymap_assign;
    const char     *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(keymap_assign); i++, q++) {
	if (strcmp(q->str, combo_str) == 0) {
	    cursor_key_assign[ (int)type ] = q->code;
	    return;
	}
    }
}


static	Q8tkWidget	*menu_key_cursor(void)
{
    int i;
    Q8tkWidget *hbox, *vbox;

    key_cursor_widget_init_done = FALSE;

    hbox = PACK_HBOX(NULL);
    {
	{			/* ����������ƥ⡼�ɤ����� */
	    vbox = PACK_VBOX(hbox);
	    PACK_RADIO_BUTTONS(vbox,
			       data_key_cursor_mode,
			       COUNTOF(data_key_cursor_mode),
			       get_key_cursor_key_mode(),
			       cb_key_cursor_key_mode);

	    for (i=0; i<2; i++) PACK_LABEL(vbox, "");	/* ����Ĵ���Υ��ߡ� */
	    
	    key_cursor_widget_sel = vbox;
	}

	PACK_VSEP(hbox);	/* ���ڤ��� */

	{			/* ����������� �ʤ��α����� */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, GET_LABEL(data_key, DATA_KEY_CURSOR_SPACING));

	    key_cursor_widget_sel_none = vbox;
	}

	{			/* ����������� Ǥ�դα����� */
	    key_cursor_widget_sel_key =
		PACK_KEY_ASSIGN(hbox,
				data_key_cursor, COUNTOF(data_key_cursor),
				get_key_cursor_key, cb_key_cursor_key);
	}
    }

    key_cursor_widget_init_done = TRUE;
    menu_key_cursor_setting();

    return hbox;
}

static	void	menu_key_cursor_setting(void)
{
    if (key_cursor_widget_init_done == FALSE) return;

    {
	q8tk_widget_show(key_cursor_widget_sel);
	q8tk_widget_hide(key_cursor_widget_sel_none);
	q8tk_widget_hide(key_cursor_widget_sel_key);

	switch (cursor_key_mode) {
	case 0:
	case 1:   q8tk_widget_show(key_cursor_widget_sel_none);		break;
	case 2:   q8tk_widget_show(key_cursor_widget_sel_key);		break;
	}
    }
}

/*----------------------------------------------------------------------*/

static	Q8tkWidget	*menu_key(void)
{
    Q8tkWidget *vbox, *hbox, *vbox2, *w;
    const t_menulabel *l = data_key;

    vbox = PACK_VBOX(NULL);
    {
	PACK_FRAME(vbox, GET_LABEL(l, DATA_KEY_FKEY), menu_key_fkey());

	hbox = PACK_HBOX(vbox);
	{
	    PACK_FRAME(hbox, GET_LABEL(l, DATA_KEY_CURSOR), menu_key_cursor());

	    vbox2 = PACK_VBOX(NULL);
	    {
		PACK_LABEL(vbox2, GET_LABEL(l, DATA_KEY_SKEY2));

		w = menu_key_softkeyboard();
		q8tk_box_pack_start(vbox2, w);
		q8tk_misc_set_placement(w, Q8TK_PLACEMENT_X_CENTER,
					Q8TK_PLACEMENT_Y_CENTER);

		PACK_LABEL(vbox2, "");	/* ����Ĵ���Υ��ߡ� */
	    }
	    PACK_FRAME(hbox, GET_LABEL(l, DATA_KEY_SKEY), vbox2);
	}

	w = PACK_FRAME(vbox, "", menu_key_cfg());
	q8tk_frame_set_shadow_type(w, Q8TK_SHADOW_NONE);
    }

    return vbox;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 
 *
 *	���֥�����ɥ�	���եȥ����������ܡ���
 *
 * = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

static	Q8tkWidget	*keymap[129];
static	int		keymap_num;
static	Q8tkWidget	*keymap_accel;

enum {			/* keymap[] �ϰʲ��Υ������åȤ���¸�˻Ȥ� */
    KEYMAP_WIN,

    KEYMAP_VBOX,
    KEYMAP_SCRL,
    KEYMAP_SEP,
    KEYMAP_HBOX,

    KEYMAP_BTN_1,
    KEYMAP_BTN_2,

    KEYMAP_LINES,
    KEYMAP_LINE_1,
    KEYMAP_LINE_2,
    KEYMAP_LINE_3,
    KEYMAP_LINE_4,
    KEYMAP_LINE_5,
    KEYMAP_LINE_6,

    KEYMAP_KEY
};

/*----------------------------------------------------------------------*/

static	int	get_key_softkey(int code)
{
    return softkey_is_pressed(code);
}
static	void	cb_key_softkey(Q8tkWidget *button, void *code)
{
    if (Q8TK_TOGGLE_BUTTON(button)->active) softkey_press  ((int)code);
    else                                    softkey_release((int)code);
}

static	void	cb_key_softkey_release(UNUSED_WIDGET, UNUSED_PARM)
{
    softkey_release_all();	/* ���ƤΥ�����Υ�������֤ˤ���         */
    keymap_finish();		/* ���եȥ��������������������åȤ���� */
}

static	void	cb_key_softkey_end(UNUSED_WIDGET, UNUSED_PARM)
{
    softkey_bug();		/* ʣ������Ʊ���������Υϡ��ɥХ���Ƹ� */
    keymap_finish();		/* ���եȥ��������������������åȤ���� */
}


/* ���եȥ��ե��������ܡ��� ������ɥ�������ɽ�� */

static	void	keymap_start(void)
{
    Q8tkWidget *w, *v, *s, *l, *h, *b1, *b2, *vx, *hx, *n;
    int i,j;
    int model = (ROM_VERSION < '8') ? 0 : 1;

    for (i=0; i<COUNTOF(keymap); i++) keymap[i] = NULL;

    {						/* �ᥤ��Ȥʤ륦����ɥ� */
	w = q8tk_window_new(Q8TK_WINDOW_DIALOG);
	keymap_accel = q8tk_accel_group_new();
	q8tk_accel_group_attach(keymap_accel, w);
    }

    {						/* �ˡ��ܥå�����褻�� */
	v = q8tk_vbox_new();
	q8tk_container_add(w, v);
	q8tk_widget_show(v);
    }

    {							/* �ܥå����ˤ�     */
	{						/* ���������� WIN */
	    s = q8tk_scrolled_window_new(NULL, NULL);
	    q8tk_box_pack_start(v, s);
	    q8tk_misc_set_size(s, 80, 21);
	    q8tk_scrolled_window_set_policy(s, Q8TK_POLICY_AUTOMATIC,
					       Q8TK_POLICY_NEVER);
	    q8tk_widget_show(s);
	}
	{						/* ���ɤ��Τ���ζ���*/
	    l = q8tk_label_new("");
	    q8tk_box_pack_start(v, l);
	    q8tk_widget_show(l);
	}
	{						/* �ܥ��������� HBOX */
	    h = q8tk_hbox_new();
	    q8tk_box_pack_start(v, h);
	    q8tk_misc_set_placement(h, Q8TK_PLACEMENT_X_CENTER, 0);
	    q8tk_widget_show(h);

	    {							/* HBOX�ˤ� */
		const t_menulabel *l = data_skey_set;
		{						/* �ܥ��� 1 */
		    b1 = q8tk_button_new_with_label(
					GET_LABEL(l,DATA_SKEY_BUTTON_OFF));
		    q8tk_signal_connect(b1, "clicked",
					cb_key_softkey_release, NULL);
		    q8tk_box_pack_start(h, b1);
		    q8tk_widget_show(b1);
		}
		{						/* �ܥ��� 2 */
		    b2 = q8tk_button_new_with_label(
					GET_LABEL(l,DATA_SKEY_BUTTON_QUIT));
		    q8tk_signal_connect(b2, "clicked",
					cb_key_softkey_end, NULL);
		    q8tk_box_pack_start(h, b2);
		    q8tk_widget_show(b2);
		    q8tk_accel_group_add(keymap_accel,
					 Q8TK_KEY_ESC, b2, "clicked");
		}
	    }
	}
    }

    /* ���������� WIN �ˡ������ȥåפ�ʸ���Τ����줿���ܥ�����¤٤� */

    vx = q8tk_vbox_new();		/* ����6��ʬ���Ǽ���� VBOX ������ */
    q8tk_container_add(s, vx);
    q8tk_widget_show(vx);

    keymap[ KEYMAP_WIN   ] = w;
    keymap[ KEYMAP_VBOX  ] = v;
    keymap[ KEYMAP_SCRL  ] = s;
    keymap[ KEYMAP_SEP   ] = l;
    keymap[ KEYMAP_HBOX  ] = h;
    keymap[ KEYMAP_BTN_1 ] = b1;
    keymap[ KEYMAP_BTN_2 ] = b2;
    keymap[ KEYMAP_LINES ] = vx;

    keymap_num = KEYMAP_KEY;


    for (j=0; j<6; j++) {		/* ����6��ʬ�����֤� */

	const t_keymap *p = keymap_line[ model ][ j ];

	hx = q8tk_hbox_new();		/* ����ʣ���Ĥ��Ǽ���뤿���HBOX�� */
	q8tk_box_pack_start(vx, hx);
	q8tk_widget_show(hx);
	keymap[ KEYMAP_LINE_1 + j ] = hx;

	for (i=0; p[ i ].str; i++) {	/* ������1�ĤŤ����֤��Ƥ���*/

	    if (keymap_num >= COUNTOF(keymap))	/* �ȥ�å� */
		{ fprintf(stderr, "%s %d\n", __FILE__, __LINE__); break; }
      
	    if (p[i].code)			/* �����ȥå�ʸ�� (�ܥ���) */
	    {
		n = q8tk_toggle_button_new_with_label(p[i].str);
		if (get_key_softkey(p[i].code)) {
		    q8tk_toggle_button_set_state(n, TRUE);
		}
		q8tk_signal_connect(n, "toggled",
				    cb_key_softkey, (void *)p[i].code);
	    }
	    else				/* �ѥǥ����Ѷ��� (��٥�) */
	    {
		n = q8tk_label_new(p[i].str);
	    }
	    q8tk_box_pack_start(hx, n);
	    q8tk_widget_show(n);

	    keymap[ keymap_num ++ ] = n;
	}
    }


    q8tk_widget_show(w);
    q8tk_grab_add(w);

    q8tk_widget_set_focus(b2);
}


/* �����ޥåץ��������ν�λ������ */

static	void	keymap_finish(void)
{
    int i;
    for (i=keymap_num-1; i; i--) {
	if (keymap[i]) {
	    q8tk_widget_destroy(keymap[i]);
	}
    }

    q8tk_grab_remove(keymap[ KEYMAP_WIN ]);
    q8tk_widget_destroy(keymap[ KEYMAP_WIN ]);
    q8tk_widget_destroy(keymap_accel);
}








/*===========================================================================
 *
 *	�ᥤ��ڡ���	�ޥ���
 *
 *===========================================================================*/

static	void	menu_mouse_mouse_setting(void);
static	void	menu_mouse_joy_setting(void);
static	void	menu_mouse_joy2_setting(void);
/*----------------------------------------------------------------------*/
						/* �ޥ����⡼���ڤ��ؤ� */
static	int	get_mouse_mode(void)
{
    return mouse_mode;
}
static	void	cb_mouse_mode(Q8tkWidget *widget, UNUSED_PARM)
{
    int i;
    const t_menudata *p = data_mouse_mode;
    const char       *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(data_mouse_mode); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    mouse_mode = p->val;
	    menu_mouse_mouse_setting();
	    menu_mouse_joy_setting();
	    menu_mouse_joy2_setting();
	    return;
	}
    }
}


static	Q8tkWidget	*menu_mouse_mode(void)
{
    Q8tkWidget *hbox;


    hbox  = PACK_HBOX(NULL);
    {
	PACK_LABEL(hbox, " ");		/* ����ǥ�� */

	PACK_COMBO(hbox,
		   data_mouse_mode, COUNTOF(data_mouse_mode),
		   get_mouse_mode(), NULL, 0,
		   cb_mouse_mode, NULL,
		   NULL, NULL);
    }

    return hbox;
}

/*----------------------------------------------------------------------*/
						      /* ���ꥢ��ޥ��� */
static	int	get_mouse_serial(void)
{
    return use_siomouse;
}
static	void	cb_mouse_serial(Q8tkWidget *widget, UNUSED_PARM)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;
    use_siomouse = key;

    sio_mouse_init(use_siomouse);
}


static	Q8tkWidget	*menu_mouse_serial(void)
{
    Q8tkWidget *hbox;


    hbox  = PACK_HBOX(NULL);
    {
/*	PACK_LABEL(hbox, " ");*/

	PACK_CHECK_BUTTON(hbox,
			  GET_LABEL(data_mouse_serial, 0),
			  get_mouse_serial(),
			  cb_mouse_serial, NULL);
    }

    return hbox;
}

/*----------------------------------------------------------------------*/
						  /* �ޥ������������ѹ� */
static	int		mouse_mouse_widget_init_done;
static	Q8tkWidget	*mouse_mouse_widget_sel;
static	Q8tkWidget	*mouse_mouse_widget_sel_none;
static	Q8tkWidget	*mouse_mouse_widget_sel_tenkey;
static	Q8tkWidget	*mouse_mouse_widget_sel_key;
static	Q8tkWidget	*mouse_mouse_widget_con;
static	Q8tkWidget	*mouse_mouse_widget_con_con;

static	int	get_mouse_mouse_key_mode(void)
{
    return mouse_key_mode;
}
static	void	cb_mouse_mouse_key_mode(UNUSED_WIDGET, void *p)
{
    mouse_key_mode = (int)p;

    menu_mouse_mouse_setting();
}

static	Q8tkWidget	*mouse_swap_widget[2];
static	int	get_mouse_swap(void)
{
    return mouse_swap_button;
}
static	void	cb_mouse_swap(Q8tkWidget *widget, void *p)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    if (mouse_swap_button != key) {
	mouse_swap_button = key;
	if ((int)p >= 0) {
	    q8tk_toggle_button_set_state(mouse_swap_widget[(int)p], -1);
	}
    }
}

static	int	get_mouse_sensitivity(void)
{
    return mouse_sensitivity;
}
static	void	cb_mouse_sensitivity(Q8tkWidget *widget, UNUSED_PARM)
{
    int val = Q8TK_ADJUSTMENT(widget)->value;

    mouse_sensitivity = val;
}

static	int	get_mouse_mouse_key(int type)
{
    return mouse_key_assign[ type ];
}
static	void	cb_mouse_mouse_key(Q8tkWidget *widget, void *type)
{
    int i;
    const t_keymap *q = keymap_assign;
    const char     *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(keymap_assign); i++, q++) {
	if (strcmp(q->str, combo_str) == 0) {
	    mouse_key_assign[ (int)type ] = q->code;
	    return;
	}
    }
}


static	Q8tkWidget	*menu_mouse_mouse(void)
{
    int i;
    Q8tkWidget *hbox, *vbox;

    mouse_mouse_widget_init_done = FALSE;

    hbox = PACK_HBOX(NULL);
    {
	{			/* ����������ƥ⡼�ɤ����� */
	    vbox = PACK_VBOX(hbox);
	    PACK_RADIO_BUTTONS(vbox,
			       data_mouse_mouse_key_mode,
			       COUNTOF(data_mouse_mouse_key_mode),
			       get_mouse_mouse_key_mode(),
			       cb_mouse_mouse_key_mode);

	    for (i=0; i<5; i++) PACK_LABEL(vbox, "");	/* ����Ĵ���Υ��ߡ� */

	    mouse_mouse_widget_sel = vbox;
	}

	{			/* ������������Բ� (��³��) */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, "");			/* ����Ĵ���Υ��ߡ� */

	    PACK_LABEL(vbox, GET_LABEL(data_mouse, DATA_MOUSE_CONNECTING));

	    for (i=0; i<6; i++) PACK_LABEL(vbox, "");	/* ����Ĵ���Υ��ߡ� */

	    mouse_mouse_widget_con = vbox;
	}

	PACK_VSEP(hbox);	/* ���ڤ��� */

	{			/* ����������� �ʤ��α����� */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, "");

	    mouse_mouse_widget_sel_none = vbox;
	}

	{			/* ����������� �ƥ󥭡��α����� */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, "");

	    mouse_swap_widget[0] =
		PACK_CHECK_BUTTON(vbox,
				  GET_LABEL(data_mouse, DATA_MOUSE_SWAP_MOUSE),
				  get_mouse_swap(),
				  cb_mouse_swap, (void*)1);

	    mouse_mouse_widget_sel_tenkey = vbox;
	}

	{			/* ����������� Ǥ�դα����� */

	    mouse_mouse_widget_sel_key = 
		PACK_KEY_ASSIGN(hbox,
				data_mouse_mouse, COUNTOF(data_mouse_mouse),
				get_mouse_mouse_key, cb_mouse_mouse_key);
	}

	{			/* ������������Բ� ������ */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, "");

	    mouse_swap_widget[1] =
		PACK_CHECK_BUTTON(vbox,
				  GET_LABEL(data_mouse, DATA_MOUSE_SWAP_MOUSE),
				  get_mouse_swap(),
				  cb_mouse_swap, (void*)0);

	    for (i=0; i<4; i++) PACK_LABEL(vbox, "");	/* ����Ĵ���Υ��ߡ� */

	    {				/* �ޥ������� */
		const t_volume *p = data_mouse_sensitivity;
		Q8tkWidget *hbox2, *scale;

		hbox2 = PACK_HBOX(vbox);
		{
		    PACK_LABEL(hbox2, GET_LABEL(p, 0));

		    scale = PACK_HSCALE(hbox2,
					 p,
					 get_mouse_sensitivity(),
					 cb_mouse_sensitivity, NULL);

		    q8tk_adjustment_set_length(scale->stat.scale.adj, 20);
		}
	    }

	    mouse_mouse_widget_con_con = vbox;
	}

    }

    mouse_mouse_widget_init_done = TRUE;
    menu_mouse_mouse_setting();

    return hbox;
}

static	void	menu_mouse_mouse_setting(void)
{
    if (mouse_mouse_widget_init_done == FALSE) return;

    if (mouse_mode == MOUSE_NONE ||		/* �ޥ����ϥݡ��Ȥ�̤��³ */
	mouse_mode == MOUSE_JOYSTICK) {

	q8tk_widget_show(mouse_mouse_widget_sel);
	q8tk_widget_hide(mouse_mouse_widget_sel_none);
	q8tk_widget_hide(mouse_mouse_widget_sel_tenkey);
	q8tk_widget_hide(mouse_mouse_widget_sel_key);

	switch (mouse_key_mode) {
	case 0:   q8tk_widget_show(mouse_mouse_widget_sel_none);	break;
	case 1:   q8tk_widget_show(mouse_mouse_widget_sel_tenkey);	break;
	case 2:   q8tk_widget_show(mouse_mouse_widget_sel_key);		break;
	}

	q8tk_widget_hide(mouse_mouse_widget_con);
	q8tk_widget_hide(mouse_mouse_widget_con_con);

    } else {					/* �ޥ����ϥݡ��Ȥ���³�� */
	q8tk_widget_hide(mouse_mouse_widget_sel);
	q8tk_widget_hide(mouse_mouse_widget_sel_none);
	q8tk_widget_hide(mouse_mouse_widget_sel_tenkey);
	q8tk_widget_hide(mouse_mouse_widget_sel_key);

	q8tk_widget_show(mouse_mouse_widget_con);
	q8tk_widget_hide(mouse_mouse_widget_con_con);

	if (mouse_mode == MOUSE_JOYMOUSE) {
	    q8tk_widget_show(mouse_mouse_widget_sel_tenkey);
	} else {
	    q8tk_widget_show(mouse_mouse_widget_con_con);
	}
    }
}

/*----------------------------------------------------------------------*/
					/* ���祤���ƥ��å����������ѹ� */
static	int		mouse_joy_widget_init_done;
static	Q8tkWidget	*mouse_joy_widget_sel;
static	Q8tkWidget	*mouse_joy_widget_sel_none;
static	Q8tkWidget	*mouse_joy_widget_sel_tenkey;
static	Q8tkWidget	*mouse_joy_widget_sel_key;
static	Q8tkWidget	*mouse_joy_widget_con;

static	int	get_mouse_joy_key_mode(void)
{
    return joy_key_mode;
}
static	void	cb_mouse_joy_key_mode(UNUSED_WIDGET, void *p)
{
    joy_key_mode = (int)p;

    menu_mouse_joy_setting();
}

static	int	get_joy_swap(void)
{
    return joy_swap_button;
}
static	void	cb_joy_swap(Q8tkWidget *widget, UNUSED_PARM)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    joy_swap_button = key;
}

static	int	get_mouse_joy_key(int type)
{
    return joy_key_assign[ type ];
}
static	void	cb_mouse_joy_key(Q8tkWidget *widget, void *type)
{
    int i;
    const t_keymap *q = keymap_assign;
    const char     *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(keymap_assign); i++, q++) {
	if (strcmp(q->str, combo_str) == 0) {
	    joy_key_assign[ (int)type ] = q->code;
	    return;
	}
    }
}


static	Q8tkWidget	*menu_mouse_joy(void)
{
    int i;
    Q8tkWidget *hbox, *vbox;

    mouse_joy_widget_init_done = FALSE;

    hbox = PACK_HBOX(NULL);
    {
	{			/* ����������ƥ⡼�ɤ����� */
	    vbox = PACK_VBOX(hbox);
	    PACK_RADIO_BUTTONS(vbox,
			       data_mouse_joy_key_mode,
			       COUNTOF(data_mouse_joy_key_mode),
			       get_mouse_joy_key_mode(),
			       cb_mouse_joy_key_mode);

	    for (i=0; i<5; i++) PACK_LABEL(vbox, "");	/* ����Ĵ���Υ��ߡ� */
	    
	    mouse_joy_widget_sel = vbox;
	}

	{			/* ������������Բ� (��³��) */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, "");			/* ����Ĵ���Υ��ߡ� */

	    PACK_LABEL(vbox, GET_LABEL(data_mouse, DATA_MOUSE_CONNECTING));

	    for (i=0; i<6; i++) PACK_LABEL(vbox, "");	/* ����Ĵ���Υ��ߡ� */

	    mouse_joy_widget_con = vbox;
	}

	PACK_VSEP(hbox);	/* ���ڤ��� */

	{			/* ����������� �ʤ��α����� */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, "");

	    mouse_joy_widget_sel_none = vbox;
	}

	{			/* ����������� �ƥ󥭡��α����� */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, "");

	    PACK_CHECK_BUTTON(vbox,
			      GET_LABEL(data_mouse, DATA_MOUSE_SWAP_JOY),
			      get_joy_swap(),
			      cb_joy_swap, NULL);

	    mouse_joy_widget_sel_tenkey = vbox;
	}

	{			/* ����������� Ǥ�դα����� */

	    mouse_joy_widget_sel_key = 
		PACK_KEY_ASSIGN(hbox,
				data_mouse_joy, COUNTOF(data_mouse_joy),
				get_mouse_joy_key, cb_mouse_joy_key);
	}
    }

    mouse_joy_widget_init_done = TRUE;
    menu_mouse_joy_setting();

    return hbox;
}

static	void	menu_mouse_joy_setting(void)
{
    if (mouse_joy_widget_init_done == FALSE) return;

    if (mouse_mode == MOUSE_NONE ||	/* ���祤���ƥ��å��ϥݡ��Ȥ�̤��³ */
	mouse_mode == MOUSE_MOUSE ||
	mouse_mode == MOUSE_JOYMOUSE) {

	q8tk_widget_show(mouse_joy_widget_sel);
	q8tk_widget_hide(mouse_joy_widget_sel_none);
	q8tk_widget_hide(mouse_joy_widget_sel_tenkey);
	q8tk_widget_hide(mouse_joy_widget_sel_key);

	switch (joy_key_mode) {
	case 0:   q8tk_widget_show(mouse_joy_widget_sel_none);		break;
	case 1:   q8tk_widget_show(mouse_joy_widget_sel_tenkey);	break;
	case 2:   q8tk_widget_show(mouse_joy_widget_sel_key);		break;
	}

	q8tk_widget_hide(mouse_joy_widget_con);

    } else {				/* ���祤���ƥ��å��ϥݡ��Ȥ���³�� */
	q8tk_widget_hide(mouse_joy_widget_sel);
	q8tk_widget_hide(mouse_joy_widget_sel_none);
	q8tk_widget_show(mouse_joy_widget_sel_tenkey);
	q8tk_widget_hide(mouse_joy_widget_sel_key);

	q8tk_widget_show(mouse_joy_widget_con);
    }
}

/*----------------------------------------------------------------------*/
				      /* ���祤���ƥ��å������������ѹ� */
static	int		mouse_joy2_widget_init_done;
static	Q8tkWidget	*mouse_joy2_widget_sel;
static	Q8tkWidget	*mouse_joy2_widget_sel_none;
static	Q8tkWidget	*mouse_joy2_widget_sel_tenkey;
static	Q8tkWidget	*mouse_joy2_widget_sel_key;

static	int	get_mouse_joy2_key_mode(void)
{
    return joy2_key_mode;
}
static	void	cb_mouse_joy2_key_mode(UNUSED_WIDGET, void *p)
{
    joy2_key_mode = (int)p;

    menu_mouse_joy2_setting();
}

static	int	get_joy2_swap(void)
{
    return joy2_swap_button;
}
static	void	cb_joy2_swap(Q8tkWidget *widget, UNUSED_PARM)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    joy2_swap_button = key;
}

static	int	get_mouse_joy2_key(int type)
{
    return joy2_key_assign[ type ];
}
static	void	cb_mouse_joy2_key(Q8tkWidget *widget, void *type)
{
    int i;
    const t_keymap *q = keymap_assign;
    const char     *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(keymap_assign); i++, q++) {
	if (strcmp(q->str, combo_str) == 0) {
	    joy2_key_assign[ (int)type ] = q->code;
	    return;
	}
    }
}


static	Q8tkWidget	*menu_mouse_joy2(void)
{
    int i;
    Q8tkWidget *hbox, *vbox;

    mouse_joy2_widget_init_done = FALSE;

    hbox = PACK_HBOX(NULL);
    {
	{			/* ����������ƥ⡼�ɤ����� */
	    vbox = PACK_VBOX(hbox);
	    PACK_RADIO_BUTTONS(vbox,
			       data_mouse_joy2_key_mode,
			       COUNTOF(data_mouse_joy2_key_mode),
			       get_mouse_joy2_key_mode(),
			       cb_mouse_joy2_key_mode);

	    for (i=0; i<5; i++) PACK_LABEL(vbox, "");	/* ����Ĵ���Υ��ߡ� */
	    
	    mouse_joy2_widget_sel = vbox;
	}

	PACK_VSEP(hbox);	/* ���ڤ��� */

	{			/* ����������� �ʤ��α����� */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, "");

	    mouse_joy2_widget_sel_none = vbox;
	}

	{			/* ����������� �ƥ󥭡��α����� */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, "");

	    PACK_CHECK_BUTTON(vbox,
			      GET_LABEL(data_mouse, DATA_MOUSE_SWAP_JOY2),
			      get_joy2_swap(),
			      cb_joy2_swap, NULL);

	    mouse_joy2_widget_sel_tenkey = vbox;
	}

	{			/* ����������� Ǥ�դα����� */
	    mouse_joy2_widget_sel_key = 
		PACK_KEY_ASSIGN(hbox,
				data_mouse_joy2, COUNTOF(data_mouse_joy2),
				get_mouse_joy2_key, cb_mouse_joy2_key);
	}
    }

    mouse_joy2_widget_init_done = TRUE;
    menu_mouse_joy2_setting();

    return hbox;
}

static	void	menu_mouse_joy2_setting(void)
{
    if (mouse_joy2_widget_init_done == FALSE) return;

    {
	q8tk_widget_show(mouse_joy2_widget_sel);
	q8tk_widget_hide(mouse_joy2_widget_sel_none);
	q8tk_widget_hide(mouse_joy2_widget_sel_tenkey);
	q8tk_widget_hide(mouse_joy2_widget_sel_key);

	switch (joy2_key_mode) {
	case 0:   q8tk_widget_show(mouse_joy2_widget_sel_none);		break;
	case 1:   q8tk_widget_show(mouse_joy2_widget_sel_tenkey);	break;
	case 2:   q8tk_widget_show(mouse_joy2_widget_sel_key);		break;
	}
    }
}

/*----------------------------------------------------------------------*/
				      /* �ޥ��������祤�����ѹ��ˤĤ��� */
static	Q8tkWidget	*menu_mouse_about(void)
{
    Q8tkWidget *vbox;
    char wk[128];
    int joy_num = event_get_joystick_num();

    vbox = PACK_VBOX(NULL);
    {
	sprintf(wk, GET_LABEL(data_mouse, DATA_MOUSE_DEVICE_NUM), joy_num);

	/* �Ρ��ȥ֥å��Υ����������줹�뤿��ˡ��������ܰ��դΥ���������� */
	PACK_LABEL(vbox, "                                                                          ");
	PACK_LABEL(vbox, wk);
    }
    return vbox;
}

/*----------------------------------------------------------------------*/
static	int	menu_mouse_last_page = 0;	/* ������Υ����򵭲� */
static	void	cb_mouse_notebook_changed(Q8tkWidget *widget, UNUSED_PARM)
{
    menu_mouse_last_page = q8tk_notebook_current_page(widget);
}

static	Q8tkWidget	*menu_mouse_device(void)
{
    Q8tkWidget *notebook, *w;

    notebook = q8tk_notebook_new();
    {
	w = menu_mouse_mouse();
	q8tk_notebook_append(notebook, w,
			     GET_LABEL(data_mouse, DATA_MOUSE_DEVICE_MOUSE));

	w = menu_mouse_joy();
	q8tk_notebook_append(notebook, w,
			     GET_LABEL(data_mouse, DATA_MOUSE_DEVICE_JOY));

	w = menu_mouse_joy2();
	q8tk_notebook_append(notebook, w,
			     GET_LABEL(data_mouse, DATA_MOUSE_DEVICE_JOY2));

	w = menu_mouse_about();
	q8tk_notebook_append(notebook, w,
			     GET_LABEL(data_mouse, DATA_MOUSE_DEVICE_ABOUT));
    }

    q8tk_notebook_set_page(notebook, menu_mouse_last_page);

    q8tk_signal_connect(notebook, "switch_page",
			cb_mouse_notebook_changed, NULL);
    q8tk_widget_show(notebook);

    return notebook;
}

/*----------------------------------------------------------------------*/
						      /* �Ƽ�������ѹ� */

/* �ǥХå��ѡ����ޥ���������ȹ礻�򸡾� */
static int  get_mouse_debug_hide(void) { return hide_mouse; }
static int  get_mouse_debug_grab(void) { return grab_mouse; }
static void cb_mouse_debug_hide(UNUSED_WIDGET, void *p) { hide_mouse = (int)p;}
static void cb_mouse_debug_grab(UNUSED_WIDGET, void *p) { grab_mouse = (int)p;}

static	Q8tkWidget	*menu_mouse_debug(void)
{
    Q8tkWidget *hbox, *hbox2;

    hbox = PACK_HBOX(NULL);
    {
	hbox2 = PACK_HBOX(hbox);
	{
	    PACK_RADIO_BUTTONS(hbox2,
			data_mouse_debug_hide, COUNTOF(data_mouse_debug_hide),
			get_mouse_debug_hide(), cb_mouse_debug_hide);
	}
	PACK_VSEP(hbox);	/* ���ڤ��� */
	PACK_VSEP(hbox);	/* ���ڤ��� */
	hbox2 = PACK_HBOX(hbox);
	{
	    PACK_RADIO_BUTTONS(hbox2,
			data_mouse_debug_grab, COUNTOF(data_mouse_debug_grab),
			get_mouse_debug_grab(), cb_mouse_debug_grab);
	}
    }
    return hbox;
}



static	int	get_mouse_misc(void)
{
    if      (grab_mouse == AUTO_MOUSE) return -2;
    else if (grab_mouse == GRAB_MOUSE) return -1;
    else                               return hide_mouse;
}
static	void	cb_mouse_misc(Q8tkWidget *widget, UNUSED_PARM)
{
    int i;
    const t_menudata *p = data_mouse_misc;
    const char       *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(data_mouse_misc); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    if (p->val == -2) {
		grab_mouse = AUTO_MOUSE;
		hide_mouse = SHOW_MOUSE;
	    } else if (p->val == -1) {
		grab_mouse = GRAB_MOUSE;
		/* hide_mouse = HIDE_MOUSE; */ /* ������ݻ����� */
	    } else {
		grab_mouse = UNGRAB_MOUSE;
		hide_mouse = p->val;
	    }
	    return;
	}
    }
}


static	Q8tkWidget	*menu_mouse_misc(void)
{
    Q8tkWidget *vbox, *hbox;


    vbox = PACK_VBOX(NULL);
    {
	if (screen_attr_mouse_debug() == FALSE) {	/* �̾�� */
	    hbox  = PACK_HBOX(vbox);
	    {
		PACK_LABEL(hbox, GET_LABEL(data_mouse_misc_msg, 0));

		PACK_COMBO(hbox,
			   data_mouse_misc, COUNTOF(data_mouse_misc),
			   get_mouse_misc(), NULL, 0,
			   cb_mouse_misc, NULL,
			   NULL, NULL);
	    }
	} else {					/* �ǥХå��� */
	    q8tk_box_pack_start(vbox, menu_mouse_debug());
	}
    }

    return vbox;
}

/*----------------------------------------------------------------------*/

static	Q8tkWidget	*menu_mouse(void)
{
    Q8tkWidget *vbox, *hbox, *vbox2, *w;
    const t_menulabel *l = data_mouse;


    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    PACK_FRAME(hbox,
		       GET_LABEL(l, DATA_MOUSE_MODE),   menu_mouse_mode());
	    PACK_FRAME(hbox,
		       GET_LABEL(l, DATA_MOUSE_SERIAL), menu_mouse_serial());
	}

	PACK_LABEL(vbox, "");			/* ���� */
	PACK_LABEL(vbox, GET_LABEL(l, DATA_MOUSE_SYSTEM));

	vbox2 = PACK_VBOX(vbox);
	{
	    w = menu_mouse_device();
	    q8tk_box_pack_start(vbox2, w);
	    q8tk_misc_set_placement(w, Q8TK_PLACEMENT_X_RIGHT, 0);

	    w = menu_mouse_misc();
	    q8tk_box_pack_start(vbox2, w);
	    q8tk_misc_set_placement(w, Q8TK_PLACEMENT_X_RIGHT, 0);
	}
    }

    return vbox;
}



/*===========================================================================
 *
 *	�ᥤ��ڡ���	�ơ���
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
				      /* ���ɥ��᡼���������֥��᡼�� */
int			tape_mode;
static	char		tape_filename[ QUASI88_MAX_FILENAME ];

static	Q8tkWidget	*tape_name[2];
static	Q8tkWidget	*tape_rate[2];

static	Q8tkWidget	*tape_button_eject[2];
static	Q8tkWidget	*tape_button_rew;

/*----------------------------------------------------------------------*/
static	void	set_tape_name(int c)
{
    const char *p = filename_get_tape(c);

    q8tk_entry_set_text(tape_name[ c ], (p ? p : "(No File)"));
}
static	void	set_tape_rate(int c)
{
    char buf[16];
    long cur, end;

    if (c == CLOAD) {

	if (sio_tape_pos(&cur, &end)) {
	    if (end == 0) {
		sprintf(buf, "   END ");
	    } else {
		sprintf(buf, "   %3ld%%", cur * 100 / end);
	    }
	} else {
	    sprintf(buf, "   ---%%");
	}

	q8tk_label_set(tape_rate[ c ], buf);
    }
}


/*----------------------------------------------------------------------*/
/* ��EJECT�ץܥ��󲡲����ν���						*/

static	void	cb_tape_eject_do(UNUSED_WIDGET, void *c)
{
    if ((int)c == CLOAD) {
	quasi88_load_tape_eject();
    } else {
	quasi88_save_tape_eject();
    }

    set_tape_name((int)c);
    set_tape_rate((int)c);

    q8tk_widget_set_sensitive(tape_button_eject[(int)c], FALSE);
    if ((int)c == CLOAD) {
	q8tk_widget_set_sensitive(tape_button_rew, FALSE);
    }
    q8tk_widget_set_focus(NULL);
}

/*----------------------------------------------------------------------*/
/* ��REW�ץܥ��󲡲����ν���						*/

static	void	cb_tape_rew_do(UNUSED_WIDGET, void *c)
{
    if ((int)c == CLOAD) {
					/* ���᡼���򴬤��᤹ */
	if (quasi88_load_tape_rewind()) {			/* ���� */
	    ;
	} else {						/* ���� */
	    set_tape_name((int)c);
	}
	set_tape_rate((int)c);
    }
}

/*----------------------------------------------------------------------*/
/* ��OPEN�ץܥ��󲡲����ν���						*/

static	void	sub_tape_open(void);
static	void	sub_tape_open_do(void);
static	void	cb_tape_open_warn_clicked(Q8tkWidget *, void *);

static	void	cb_tape_open(UNUSED_WIDGET, void *c)
{
    const char *initial;
    const t_menulabel *l = ((int)c == CLOAD) ? data_tape_load : data_tape_save;

				/* ��������������ե����륻�쥯������ */
    tape_mode = (int)c;		/* LOAD�� �� SAVE�Ѥ� ��Ф��Ƥ���      */

    initial = filename_get_tape_or_dir(tape_mode);

    START_FILE_SELECTION(GET_LABEL(l, DATA_TAPE_FSEL),
			 -1,	/* ReadOnly ��������Բ� */
			 initial,

			 sub_tape_open,
			 tape_filename,
			 QUASI88_MAX_FILENAME,
			 NULL);
}

static	void	sub_tape_open(void)
{
    const t_menulabel *l = data_tape_save;

    switch (osd_file_stat(tape_filename)) {

    case FILE_STAT_NOEXIST:
	if (tape_mode == CLOAD) {		/* �ե�����̵���Τǥ��顼   */
	    start_file_error_dialog(-1, ERR_CANT_OPEN);
	} else {				/* �ե�����̵���Τǿ������� */
	    sub_tape_open_do();
	}
	break;

    case FILE_STAT_DIR:
	/* �ǥ��쥯�ȥ�ʤΤǡ������������ */
	start_file_error_dialog(-1, ERR_CANT_OPEN);
	break;

    default:
	if (tape_mode == CSAVE) {
	    /* ���Ǥ˥ե����뤬¸�ߤ��ޤ������᡼�����ɵ����ޤ����� */
	    dialog_create();
	    {
		dialog_set_title(GET_LABEL(l, DATA_TAPE_WARN_0));

		dialog_set_title(GET_LABEL(l, DATA_TAPE_WARN_1));

		dialog_set_separator();

		dialog_set_button(GET_LABEL(l, DATA_TAPE_WARN_APPEND),
				  cb_tape_open_warn_clicked, (void*)TRUE);

		dialog_set_button(GET_LABEL(l, DATA_TAPE_WARN_CANCEL),
				  cb_tape_open_warn_clicked, (void*)FALSE);

		dialog_accel_key(Q8TK_KEY_ESC);
	    }
	    dialog_start();
	} else {
	    sub_tape_open_do();
	}
	break;
    }
}

static	void	sub_tape_open_do(void)
{
    int result, c = tape_mode;

    if (c == CLOAD) {			/* �ơ��פ򳫤� */
	result = quasi88_load_tape_insert(tape_filename);
    } else {
	result = quasi88_save_tape_insert(tape_filename);
    }

    set_tape_name(c);
    set_tape_rate(c);


    if (result == FALSE) {
	start_file_error_dialog(-1, ERR_CANT_OPEN);
    }

    q8tk_widget_set_sensitive(tape_button_eject[(int)c], (result ?TRUE :FALSE));
    if ((int)c == CLOAD) {
	q8tk_widget_set_sensitive(tape_button_rew, (result ?TRUE :FALSE));
    }
}

static	void	cb_tape_open_warn_clicked(UNUSED_WIDGET, void *p)
{
    dialog_destroy();

    if ((int)p) {
	sub_tape_open_do();
    }
}




/*----------------------------------------------------------------------*/

INLINE	Q8tkWidget	*menu_tape_image_unit(const t_menulabel *l, int c)
{
    int save_code;
    Q8tkWidget *vbox, *hbox, *w, *e;

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    w = PACK_LABEL(hbox, GET_LABEL(l, DATA_TAPE_FOR));
	    q8tk_misc_set_placement(w, Q8TK_PLACEMENT_X_CENTER,
				       Q8TK_PLACEMENT_Y_CENTER);

	    {
		save_code = q8tk_set_kanjicode(osd_kanji_code());

		e = PACK_ENTRY(hbox,
			       QUASI88_MAX_FILENAME, 65, NULL,
			       NULL, NULL, NULL, NULL);
		q8tk_entry_set_editable(e, FALSE);

		tape_name[ c ] = e;
		set_tape_name(c);

		q8tk_set_kanjicode(save_code);
	    }
	}

	hbox = PACK_HBOX(vbox);
	{
	    PACK_BUTTON(hbox,
			GET_LABEL(l, DATA_TAPE_CHANGE),
			cb_tape_open, (void*)c);

	    PACK_VSEP(hbox);

	    tape_button_eject[c] = PACK_BUTTON(hbox,
					       GET_LABEL(l, DATA_TAPE_EJECT),
					       cb_tape_eject_do, (void*)c);

	    if (c == CLOAD) {
		tape_button_rew = PACK_BUTTON(hbox,
					      GET_LABEL(l, DATA_TAPE_REWIND),
					      cb_tape_rew_do, (void*)c);
	    }
	    if (c == CLOAD) {
		w = PACK_LABEL(hbox, "");
		q8tk_misc_set_placement(w, Q8TK_PLACEMENT_X_CENTER,
					   Q8TK_PLACEMENT_Y_CENTER);
		tape_rate[ c ] = w;
		set_tape_rate(c);
	    }

	    if (c == CLOAD) {
		if (tape_readable() == FALSE) {
		    q8tk_widget_set_sensitive(tape_button_eject[c], FALSE);
		    q8tk_widget_set_sensitive(tape_button_rew, FALSE);
		}
	    } else {
		if (tape_writable() == FALSE) {
		    q8tk_widget_set_sensitive(tape_button_eject[c], FALSE);
		}
	    }
	}
    }

    return vbox;
}

static	Q8tkWidget	*menu_tape_image(void)
{
    Q8tkWidget *vbox;

    vbox = PACK_VBOX(NULL);
    {
	q8tk_box_pack_start(vbox, menu_tape_image_unit(data_tape_load, CLOAD));

	PACK_HSEP(vbox);

	q8tk_box_pack_start(vbox, menu_tape_image_unit(data_tape_save, CSAVE));
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
					    /* �ơ��׽����⡼���ڤ��ؤ� */
static	int	get_tape_intr(void)
{
    return cmt_intr;
}
static	void	cb_tape_intr(UNUSED_WIDGET, void *p)
{
    cmt_intr = (int)p;
}


static	Q8tkWidget	*menu_tape_intr(void)
{
    Q8tkWidget *vbox;

    vbox = PACK_VBOX(NULL);
    {
	PACK_RADIO_BUTTONS(vbox,
			   data_tape_intr, COUNTOF(data_tape_intr),
			   get_tape_intr(), cb_tape_intr);
    }

    return vbox;
}

/*======================================================================*/

static	Q8tkWidget	*menu_tape(void)
{
    Q8tkWidget *vbox;
    const t_menulabel *l = data_tape;

    vbox = PACK_VBOX(NULL);
    {
	PACK_FRAME(vbox, GET_LABEL(l, DATA_TAPE_IMAGE), menu_tape_image());

	PACK_FRAME(vbox, GET_LABEL(l, DATA_TAPE_INTR), menu_tape_intr());
    }

    return vbox;
}



/*===========================================================================
 *
 *	�ᥤ��ڡ���	����¾
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
							  /* �����ڥ�� */

static	Q8tkWidget	*misc_suspend_entry;
static	Q8tkWidget	*misc_suspend_combo;

static	char		state_filename[ QUASI88_MAX_FILENAME ];


/*	�����ڥ�ɻ��Υ�å���������������ä�			  */
static	void	cb_misc_suspend_dialog_ok(UNUSED_WIDGET, void *result)
{
    dialog_destroy();

    if ((int)result == DATA_MISC_RESUME_OK ||
	(int)result == DATA_MISC_RESUME_ERR) {
	quasi88_exec();		/* �� q8tk_main_quit() �ƽкѤ� */
    }
}

/*	�����ڥ�ɼ¹Ը�Υ�å�������������				  */
static	void	sub_misc_suspend_dialog(int result)
{
    const t_menulabel *l = data_misc_suspend_err;
    char filename[60 +11 +1];	/* 11 == strlen("[DRIVE 1:] "), 1 �� '\0' */
    int save_code;
    int i, width, len;
    const char *ptr[4];
    const char *none = "(No Image File)";
    const char *dev[] = { "[DRIVE 1:]", "[DRIVE 2:]",
			  "[TapeLOAD]", "[TapeSAVE]", };

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, result));	/* ���ɽ�� */

	if (result == DATA_MISC_SUSPEND_OK ||	/* �������ϥ��᡼��̾ɽ�� */
	    result == DATA_MISC_RESUME_OK) {

	    dialog_set_title(GET_LABEL(l, DATA_MISC_SUSPEND_LINE));
	    dialog_set_title(GET_LABEL(l, DATA_MISC_SUSPEND_INFO));

	    save_code = q8tk_set_kanjicode(osd_kanji_code());

	    width = 0;
	    for (i=0; i<4; i++) {
		if (i<2) {
		    if ((ptr[i] = filename_get_disk(i)) == NULL) ptr[i] = none;
		} else {
		    if ((ptr[i] = filename_get_tape(i-2))==NULL) ptr[i] = none;
		}
		len = sprintf(filename, "%.60s", ptr[i]);	/* == max 60 */
		width = MAX(width, len);
	    }

	    for (i=0; i<4; i++) {
		sprintf(filename, "%s %-*.*s", dev[i], width, width, ptr[i]);
		dialog_set_title(filename);
	    }

	    q8tk_set_kanjicode(save_code);
	}

	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_MISC_SUSPEND_AGREE),
			  cb_misc_suspend_dialog_ok, (void*)result);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}

/*	�쥸�塼��¹����Υ�å�������������				  */
static	void	sub_misc_suspend_not_access(void)
{
    const t_menulabel *l = data_misc_suspend_err;

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_MISC_RESUME_CANTOPEN));

	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_MISC_SUSPEND_AGREE),
			  cb_misc_suspend_dialog_ok,
			  (void*)DATA_MISC_SUSPEND_AGREE);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}

/*	�����ڥ�ɼ¹����Υ�å�������������				  */
static	void	cb_misc_suspend_overwrite(UNUSED_WIDGET, UNUSED_PARM);
static	void	sub_misc_suspend_really(void)
{
    const t_menulabel *l = data_misc_suspend_err;

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_MISC_SUSPEND_REALLY));

	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_MISC_SUSPEND_OVERWRITE),
			  cb_misc_suspend_overwrite, NULL);
	dialog_set_button(GET_LABEL(l, DATA_MISC_SUSPEND_CANCEL),
			  cb_misc_suspend_dialog_ok,
			  (void*)DATA_MISC_SUSPEND_CANCEL);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}

static	void	cb_misc_suspend_overwrite(UNUSED_WIDGET, UNUSED_PARM)
{
    dialog_destroy();
    {
	if (quasi88_statesave(-1)) {
	    sub_misc_suspend_dialog(DATA_MISC_SUSPEND_OK);	/* ���� */
	} else {
	    sub_misc_suspend_dialog(DATA_MISC_SUSPEND_ERR);	/* ���� */
	}
    }
}

/*----------------------------------------------------------------------*/
/*	�����ڥ�ɽ��� (�֥����֡ץ���å���)				*/
static	void	cb_misc_suspend_save(UNUSED_WIDGET, UNUSED_PARM)
{
#if 0	/* ����������񤭳�ǧ���Ƥ���ΤϤ������� */
    if (statesave_check_file_exist()) {			/* �ե����뤢�� */
	sub_misc_suspend_really();
    } else
#endif
    {
	if (quasi88_statesave(-1)) {
	    sub_misc_suspend_dialog(DATA_MISC_SUSPEND_OK);	/* ���� */
	} else {
	    sub_misc_suspend_dialog(DATA_MISC_SUSPEND_ERR);	/* ���� */
	}
    }
}

/*----------------------------------------------------------------------*/
/*	�����ڥ�ɽ��� (�֥��ɡץ���å���)				*/
static	void	cb_misc_suspend_load(UNUSED_WIDGET, UNUSED_PARM)
{
    if (stateload_check_file_exist() == FALSE) {	/* �ե�����ʤ� */
	sub_misc_suspend_not_access();
    } else {
	if (quasi88_stateload(-1)) {
	    sub_misc_suspend_dialog(DATA_MISC_RESUME_OK);	/* ���� */
	} else {
	    sub_misc_suspend_dialog(DATA_MISC_RESUME_ERR);	/* ���� */
	}
    }
}

/*----------------------------------------------------------------------*/
/*	�ե�����̾���ѹ�������ȥ꡼ changed (����)���˸ƤФ�롣       */
/*		(�ե����륻�쥯�����Ǥ��ѹ����Ϥ���ϸƤФ�ʤ�)      */

static void sub_misc_suspend_combo_update(void)
{
    int i;
    char buf[4];
				/* ���ơ��ȥե������Ϣ�֤˱����ƥ�����ѹ� */
    i = filename_get_state_serial();
    if ('0' <= i && i <= '9') { buf[0] = i;   }
    else                      { buf[0] = ' '; }
    buf[1] = '\0';
    if (*(q8tk_combo_get_text(misc_suspend_combo)) != buf[0]) {
	q8tk_combo_set_text(misc_suspend_combo, buf);
    }
}

static void cb_misc_suspend_entry_change(Q8tkWidget *widget, UNUSED_PARM)
{
    filename_set_state(q8tk_entry_get_text(widget));

    sub_misc_suspend_combo_update();
}

/*----------------------------------------------------------------------*/
/*	�ե���������������ե����륻�쥯���������			*/

static void sub_misc_suspend_update(void);
static void sub_misc_suspend_change(void);

static	void	cb_misc_suspend_fsel(UNUSED_WIDGET, UNUSED_PARM)
{
    const t_menulabel *l = data_misc_suspend;


    START_FILE_SELECTION(GET_LABEL(l, DATA_MISC_SUSPEND_FSEL),
			 -1,	/* ReadOnly ��������Բ� */
			 q8tk_entry_get_text(misc_suspend_entry),

			 sub_misc_suspend_change,
			 state_filename,
			 QUASI88_MAX_FILENAME,
			 NULL);
}

static void sub_misc_suspend_change(void)
{
    filename_set_state(state_filename);
    q8tk_entry_set_text(misc_suspend_entry, state_filename);

    sub_misc_suspend_combo_update();
}

static void sub_misc_suspend_update(void)
{
    q8tk_entry_set_text(misc_suspend_entry, filename_get_state());

    sub_misc_suspend_combo_update();
}


static void cb_misc_suspend_num(Q8tkWidget *widget, UNUSED_PARM)
{
    int i;
    const t_menudata *p = data_misc_suspend_num;
    const char       *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(data_misc_suspend_num); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {

	    filename_set_state_serial(p->val);

	    q8tk_entry_set_text(misc_suspend_entry, filename_get_state());
	    return;
	}
    }
}

/*----------------------------------------------------------------------*/

static	Q8tkWidget	*menu_misc_suspend(void)
{
    Q8tkWidget *vbox, *hbox;
    Q8tkWidget *w, *e;
    const t_menulabel *l = data_misc_suspend;

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    {
		PACK_LABEL(hbox, GET_LABEL(data_misc, DATA_MISC_SUSPEND));
	    }
	    {
		int save_code = q8tk_set_kanjicode(osd_kanji_code());

		e = PACK_ENTRY(hbox,
			       QUASI88_MAX_FILENAME, 74 - 11,
			       filename_get_state(),
			       NULL, NULL,
			       cb_misc_suspend_entry_change, NULL);
/*		q8tk_entry_set_position(e, 0);*/
		misc_suspend_entry = e;

		q8tk_set_kanjicode(save_code);
	    }
	}

	hbox = PACK_HBOX(vbox);
	{
	    PACK_LABEL(hbox, "    ");

	    PACK_BUTTON(hbox,
			GET_LABEL(l, DATA_MISC_SUSPEND_SAVE),
			cb_misc_suspend_save, NULL);

	    PACK_LABEL(hbox, " ");
	    PACK_VSEP(hbox);
	    PACK_LABEL(hbox, " ");

	    PACK_BUTTON(hbox,
			GET_LABEL(l, DATA_MISC_SUSPEND_LOAD),
			cb_misc_suspend_load, NULL);

	    w = PACK_LABEL(hbox, GET_LABEL(l, DATA_MISC_SUSPEND_NUMBER));
	    q8tk_misc_set_placement(w, Q8TK_PLACEMENT_X_CENTER,
				       Q8TK_PLACEMENT_Y_CENTER);

	    w = PACK_COMBO(hbox,
			   data_misc_suspend_num,
			   COUNTOF(data_misc_suspend_num),
			   filename_get_state_serial(), " ", 0,
			   cb_misc_suspend_num, NULL,
			   NULL, NULL);
	    q8tk_misc_set_placement(w, Q8TK_PLACEMENT_X_CENTER,
				       Q8TK_PLACEMENT_Y_CENTER);
	    misc_suspend_combo = w;

	    PACK_LABEL(hbox, "  ");

	    PACK_BUTTON(hbox,
			GET_LABEL(l, DATA_MISC_SUSPEND_CHANGE),
			cb_misc_suspend_fsel, NULL);

	}
    }

    return vbox;
}



/*----------------------------------------------------------------------*/
					 /* �����꡼�� ���ʥåץ���å� */

static	Q8tkWidget	*misc_snapshot_entry;

static	char		snap_filename[ QUASI88_MAX_FILENAME ];


/*----------------------------------------------------------------------*/
/*	���ʥåץ���å� ������ (�ּ¹ԡץ���å���)			*/
static	void	cb_misc_snapshot_do(void)
{
    /* ǰ�Τ��ᡢ���ʥåץ���åȤΥե�����̾������� */
    filename_set_snap_base(q8tk_entry_get_text(misc_snapshot_entry));

    quasi88_screen_snapshot();

    /* ���ʥåץ���åȤΥե�����̾�ϡ��¹Ի����Ѥ�뤳�Ȥ�����ΤǺ����� */
    q8tk_entry_set_text(misc_snapshot_entry, filename_get_snap_base());
}

/*----------------------------------------------------------------------*/
/*	�����ե����ޥå��ڤ��ؤ�					*/
static	int	get_misc_snapshot_format(void)
{
    return snapshot_format;
}
static	void	cb_misc_snapshot_format(UNUSED_WIDGET, void *p)
{
    snapshot_format = (int)p;
}



/*----------------------------------------------------------------------*/
/*	�ե�����̾���ѹ�������ȥ꡼ changed (����)���˸ƤФ�롣	*/
/*		(�ե����륻�쥯�����Ǥ��ѹ����Ϥ���ϸƤФ�ʤ�)	*/

static void cb_misc_snapshot_entry_change(Q8tkWidget *widget, UNUSED_PARM)
{
    filename_set_snap_base(q8tk_entry_get_text(widget));
}

/*----------------------------------------------------------------------*/
/*	�ե���������������ե����륻�쥯���������			*/

static void sub_misc_snapshot_update(void);
static void sub_misc_snapshot_change(void);

static	void	cb_misc_snapshot_fsel(UNUSED_WIDGET, UNUSED_PARM)
{
    const t_menulabel *l = data_misc_snapshot;


    START_FILE_SELECTION(GET_LABEL(l, DATA_MISC_SNAPSHOT_FSEL),
			 -1,	/* ReadOnly ��������Բ� */
			 q8tk_entry_get_text(misc_snapshot_entry),

			 sub_misc_snapshot_change,
			 snap_filename,
			 QUASI88_MAX_FILENAME,
			 NULL);
}

static void sub_misc_snapshot_change(void)
{
    filename_set_snap_base(snap_filename);
    q8tk_entry_set_text(misc_snapshot_entry, snap_filename);
}

static void sub_misc_snapshot_update(void)
{
    q8tk_entry_set_text(misc_snapshot_entry, filename_get_snap_base());
}


/*----------------------------------------------------------------------*/
#ifdef	USE_SSS_CMD
/*	���ޥ�ɼ¹Ծ����ѹ� */
static	int	get_misc_snapshot_c_do(void)
{
    return snapshot_cmd_do;
}
static	void	cb_misc_snapshot_c_do(Q8tkWidget *widget, UNUSED_PARM)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;
    snapshot_cmd_do = key;
}

/*	���ޥ���ѹ�������ȥ꡼ changed (����)���˸ƤФ�롣  */
static void cb_misc_snapshot_c_entry_change(Q8tkWidget *widget, UNUSED_PARM)
{
    strncpy(snapshot_cmd, q8tk_entry_get_text(widget),
	    SNAPSHOT_CMD_SIZE - 1);
    snapshot_cmd[ SNAPSHOT_CMD_SIZE - 1 ] = '\0';
}
#endif

/*----------------------------------------------------------------------*/
static	Q8tkWidget	*menu_misc_snapshot(void)
{
    Q8tkWidget *hbox, *vbox, *hbox2, *vbox2, *vbox3, *hbox3;
    Q8tkWidget *e;
    const t_menulabel *l = data_misc_snapshot;

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    {
		PACK_LABEL(hbox, GET_LABEL(data_misc, DATA_MISC_SNAPSHOT));
	    }
	    {
		int save_code = q8tk_set_kanjicode(osd_kanji_code());

		e = PACK_ENTRY(hbox,
			       QUASI88_MAX_FILENAME, 74 - 11,
			       filename_get_snap_base(),
			       NULL, NULL,
			       cb_misc_snapshot_entry_change, NULL);
/*		q8tk_entry_set_position(e, 0);*/
		misc_snapshot_entry = e; 

		q8tk_set_kanjicode(save_code);
	    }
	}

	hbox = PACK_HBOX(vbox);
	{
	    PACK_LABEL(hbox, "    ");

	    PACK_BUTTON(hbox,
			GET_LABEL(l, DATA_MISC_SNAPSHOT_BUTTON),
			cb_misc_snapshot_do, NULL);

	    PACK_LABEL(hbox, " ");
	    PACK_VSEP(hbox);


	    vbox2 = PACK_VBOX(hbox);
	    {
		hbox2 = PACK_HBOX(vbox2);
		{
		    vbox3 = PACK_VBOX(hbox2);

		    PACK_LABEL(vbox3, "");

		    hbox3 = PACK_HBOX(vbox3);
		    {
			PACK_LABEL(hbox3,
				   GET_LABEL(l, DATA_MISC_SNAPSHOT_FORMAT));

			PACK_RADIO_BUTTONS(PACK_HBOX(hbox3),
					   data_misc_snapshot_format,
					   COUNTOF(data_misc_snapshot_format),
					   get_misc_snapshot_format(),
					   cb_misc_snapshot_format);

			PACK_LABEL(hbox3,
				   GET_LABEL(l, DATA_MISC_SNAPSHOT_PADDING));
		    }

		    PACK_BUTTON(hbox2,
				GET_LABEL(l, DATA_MISC_SNAPSHOT_CHANGE),
				cb_misc_snapshot_fsel, NULL);
		}

#ifdef	USE_SSS_CMD
		if (snapshot_cmd_enable) {
		    hbox2 = PACK_HBOX(vbox2);
		    {
			PACK_CHECK_BUTTON(hbox2,
					  GET_LABEL(l, DATA_MISC_SNAPSHOT_CMD),
					  get_misc_snapshot_c_do(),
					  cb_misc_snapshot_c_do, NULL);

			PACK_LABEL(hbox2, " ");

			PACK_ENTRY(hbox2,
				   SNAPSHOT_CMD_SIZE, 38, snapshot_cmd,
				   NULL, NULL,
				   cb_misc_snapshot_c_entry_change, NULL);
		    }
		}
#endif	/* USE_SSS_CMD */
	    }
	}
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
						  /* ������ɽ��ϥ����� */

static	Q8tkWidget	*misc_waveout_entry;
static	Q8tkWidget	*misc_waveout_start;
static	Q8tkWidget	*misc_waveout_stop;
static	Q8tkWidget	*misc_waveout_change;

static	char		wave_filename[ QUASI88_MAX_FILENAME ];


static void sub_misc_waveout_sensitive(void)
{
    if (xmame_wavout_opened() == FALSE) {
	q8tk_widget_set_sensitive(misc_waveout_start,  TRUE);
	q8tk_widget_set_sensitive(misc_waveout_stop,   FALSE);
	q8tk_widget_set_sensitive(misc_waveout_change, TRUE);
	q8tk_widget_set_sensitive(misc_waveout_entry,  TRUE);
    } else {
	q8tk_widget_set_sensitive(misc_waveout_start,  FALSE);
	q8tk_widget_set_sensitive(misc_waveout_stop,   TRUE);
	q8tk_widget_set_sensitive(misc_waveout_change, FALSE);
	q8tk_widget_set_sensitive(misc_waveout_entry,  FALSE);
    }
    q8tk_widget_set_focus(NULL);
}
/*----------------------------------------------------------------------*/
/*	������ɽ��� ��¸���� (�ֳ��ϡץ���å���)			*/
static	void	cb_misc_waveout_start(void)
{
    /* ǰ�Τ��ᡢ������ɽ��ϤΥե�����̾������� */
    filename_set_wav_base(q8tk_entry_get_text(misc_waveout_entry));

    quasi88_waveout(TRUE);

    /* ���ʥåץ���åȤΥե�����̾�ϡ��¹Ի����Ѥ�뤳�Ȥ�����ΤǺ����� */
    q8tk_entry_set_text(misc_waveout_entry, filename_get_wav_base());


    sub_misc_waveout_sensitive();
}

/*----------------------------------------------------------------------*/
/*	������ɽ��� ��¸��λ (����ߡץ���å���)			*/
static	void	cb_misc_waveout_stop(void)
{
    quasi88_waveout(FALSE);

    sub_misc_waveout_sensitive();
}

/*----------------------------------------------------------------------*/
/*	�ե�����̾���ѹ�������ȥ꡼ changed (����)���˸ƤФ�롣	*/
/*		(�ե����륻�쥯�����Ǥ��ѹ����Ϥ���ϸƤФ�ʤ�)	*/

static void cb_misc_waveout_entry_change(Q8tkWidget *widget, UNUSED_PARM)
{
    filename_set_wav_base(q8tk_entry_get_text(widget));
}

/*----------------------------------------------------------------------*/
/*	�ե���������������ե����륻�쥯���������			*/

static void sub_misc_waveout_update(void);
static void sub_misc_waveout_change(void);

static	void	cb_misc_waveout_fsel(UNUSED_WIDGET, UNUSED_PARM)
{
    const t_menulabel *l = data_misc_waveout;


    START_FILE_SELECTION(GET_LABEL(l, DATA_MISC_WAVEOUT_FSEL),
			 -1,	/* ReadOnly ��������Բ� */
			 q8tk_entry_get_text(misc_waveout_entry),

			 sub_misc_waveout_change,
			 wave_filename,
			 QUASI88_MAX_FILENAME,
			 NULL);
}

static void sub_misc_waveout_change(void)
{
    filename_set_wav_base(wave_filename);
    q8tk_entry_set_text(misc_waveout_entry, wave_filename);
}

static void sub_misc_waveout_update(void)
{
    q8tk_entry_set_text(misc_waveout_entry, filename_get_wav_base());
}


/*----------------------------------------------------------------------*/
static	Q8tkWidget	*menu_misc_waveout(void)
{
    Q8tkWidget *hbox, *vbox;
    Q8tkWidget *e;
    const t_menulabel *l = data_misc_waveout;

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    {
		PACK_LABEL(hbox, GET_LABEL(data_misc, DATA_MISC_WAVEOUT));
	    }
	    {
		int save_code = q8tk_set_kanjicode(osd_kanji_code());

		e = PACK_ENTRY(hbox,
			       QUASI88_MAX_FILENAME, 63,
			       filename_get_wav_base(),
			       NULL, NULL,
			       cb_misc_waveout_entry_change, NULL);
/*		q8tk_entry_set_position(e, 0);*/
		misc_waveout_entry = e; 

		q8tk_set_kanjicode(save_code);
	    }
	}

	hbox = PACK_HBOX(vbox);
	{
	    PACK_LABEL(hbox, "    ");

	    misc_waveout_start =
		PACK_BUTTON(hbox,
			    GET_LABEL(l, DATA_MISC_WAVEOUT_START),
			    cb_misc_waveout_start, NULL);

	    PACK_LABEL(hbox, " ");
	    PACK_VSEP(hbox);
	    PACK_LABEL(hbox, " ");

	    misc_waveout_stop =
		PACK_BUTTON(hbox,
			    GET_LABEL(l, DATA_MISC_WAVEOUT_STOP),
			    cb_misc_waveout_stop, NULL);

	    PACK_LABEL(hbox, GET_LABEL(l, DATA_MISC_WAVEOUT_PADDING));

	    misc_waveout_change =
		PACK_BUTTON(hbox,
			    GET_LABEL(l, DATA_MISC_WAVEOUT_CHANGE),
			    cb_misc_waveout_fsel, NULL);
	}
    }

    sub_misc_waveout_sensitive();

    return vbox;
}

/*----------------------------------------------------------------------*/
						    /* �ե�����̾��碌 */
static	int	get_misc_sync(void)
{
    return filename_synchronize;
}
static	void	cb_misc_sync(Q8tkWidget *widget, UNUSED_PARM)
{
    filename_synchronize = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;
}



/*======================================================================*/

static	Q8tkWidget	*menu_misc(void)
{
    Q8tkWidget *vbox;
    Q8tkWidget *w;

    vbox = PACK_VBOX(NULL);
    {
	PACK_HSEP(vbox);

	q8tk_box_pack_start(vbox, menu_misc_suspend());

	PACK_HSEP(vbox);

	q8tk_box_pack_start(vbox, menu_misc_snapshot());

	PACK_HSEP(vbox);

	w = menu_misc_waveout();

	if (xmame_has_sound()) {
	    q8tk_box_pack_start(vbox, w);
	    PACK_HSEP(vbox);
	}

	PACK_CHECK_BUTTON(vbox,
			  GET_LABEL(data_misc_sync, 0),
			  get_misc_sync(),
			  cb_misc_sync, NULL);
    }

    return vbox;
}










/*===========================================================================
 *
 *	�ᥤ��ڡ���	�С���������
 *
 *===========================================================================*/

static	Q8tkWidget	*menu_about(void)
{
    int i;
    Q8tkWidget *vx, *hx, *vbox, *swin, *hbox, *w;

    vx = PACK_VBOX(NULL);
    {
	hx = PACK_HBOX(vx);				/* ��Ⱦʬ�˥�ɽ�� */
	{
	    PACK_LABEL(hx, " ");	/* ����ǥ�� */

	    if (strcmp(Q_TITLE, "QUASI88") == 0) {
		w = q8tk_logo_new();
		q8tk_widget_show(w);
		q8tk_box_pack_start(hx, w);
	    } else {
		PACK_LABEL(hx, Q_TITLE);
	    }

	    vbox = PACK_VBOX(hx);
	    {
		i = Q8GR_LOGO_H;

		PACK_LABEL(vbox, "  " Q_COPYRIGHT);
		i--;

		for ( ; i>1; i--) PACK_LABEL(vbox, "");

		PACK_LABEL(vbox, "  ver. " Q_VERSION  "  <" Q_COMMENT ">");
	    }
	}
							/* ��Ⱦʬ�Ͼ���ɽ�� */
	swin = q8tk_scrolled_window_new(NULL, NULL);
	{
	    hbox = PACK_HBOX(NULL);
	    {
		vbox = PACK_VBOX(hbox);
		{
		    {		/* ������ɤ˴ؤ������ɽ�� */
			const char *(*s) = (menu_lang == 0) ? data_about_en
							    : data_about_jp;

			for (i=0; s[i]; i++) {
			    if (strcmp(s[i], "@MAMEVER") == 0) {
				PACK_LABEL(vbox, xmame_version_mame());
			    } else if (strcmp(s[i], "@FMGENVER") == 0) {
				PACK_LABEL(vbox, xmame_version_fmgen());
			    } else {
				PACK_LABEL(vbox, s[i]);
			    }
			}
		    }

		    {		/* �����ƥ��¸���˴ؤ������ɽ�� */
			int new_code, save_code = 0;
			const char *msg;
			char buf[256];
			int i;

			if (menu_about_osd_msg(menu_lang, &new_code, &msg)) {

			    if (menu_lang == MENU_JAPAN && new_code >= 0) {
				save_code = q8tk_set_kanjicode(new_code);
			    }

			    i = 0;
			    for (;;) {
				if (i == 255 || *msg == '\n' || *msg == '\0') {
				    buf[i] = '\0';
				    PACK_LABEL(vbox, buf);
				    i = 0;
				    if (*msg == '\n') msg++;
				    if (*msg == '\0') break;
				} else {
				    buf[i] = *msg++;
				    i++;
				}
			    }
			
			    if (menu_lang == MENU_JAPAN && new_code >= 0) {
				q8tk_set_kanjicode(save_code);
			    }
			}
		    }
		}
	    }
	    q8tk_container_add(swin, hbox);
	}

	q8tk_scrolled_window_set_policy(swin, Q8TK_POLICY_AUTOMATIC,
					      Q8TK_POLICY_AUTOMATIC);
	q8tk_misc_set_size(swin, 78, 18-Q8GR_LOGO_H);
	q8tk_widget_show(swin);
	q8tk_box_pack_start(vx, swin);
    }

    return vx;
}










/*===========================================================================
 *
 *	�ᥤ�󥦥���ɥ�
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
				     /* NOTEBOOK ��ĥ���դ��롢�ƥڡ��� */
static	struct{
    int		data_num;
    Q8tkWidget	*(*menu_func)(void);
} menu_page[] =
{
    { DATA_TOP_RESET,	menu_reset,	},
    { DATA_TOP_CPU,	menu_cpu,	},
    { DATA_TOP_GRAPH,	menu_graph,	},
#ifdef	USE_SOUND
    { DATA_TOP_VOLUME,	menu_volume,	},
#endif
    { DATA_TOP_DISK,	menu_disk,	},
    { DATA_TOP_KEY,	menu_key,	},
    { DATA_TOP_MOUSE,	menu_mouse,	},
    { DATA_TOP_TAPE,	menu_tape,	},
    { DATA_TOP_MISC,	menu_misc,	},
    { DATA_TOP_ABOUT,	menu_about,	},
};

/*----------------------------------------------------------------------*/
/* NOTEBOOK �γƥڡ����򡢥ե��󥯥���󥭡�����������褦�ˡ�
   ��������졼�����������ꤹ�롣���Τ��ᡢ���ߡ��������å����� */

#define	cb_note_fake(fn,n)						\
static	void	cb_note_fake_##fn(UNUSED_WIDGET, Q8tkWidget *notebook)	\
{									\
    q8tk_notebook_set_page(notebook, n);				\
}
cb_note_fake(f1,0)
cb_note_fake(f2,1)
cb_note_fake(f3,2)
cb_note_fake(f4,3)
cb_note_fake(f5,4)
cb_note_fake(f6,5)
cb_note_fake(f7,6)
cb_note_fake(f8,7)
cb_note_fake(f9,8)
cb_note_fake(f10,9)

     /* �ʲ��Υ�������졼�����������ϡ� floi�� �󶡡� Thanks ! */
static	void	cb_note_fake_prev(UNUSED_WIDGET, Q8tkWidget *notebook)
{
    int n = q8tk_notebook_current_page(notebook) - 1;
    if (n < 0) n = COUNTOF(menu_page) - 1;
    q8tk_notebook_set_page(notebook, n);
}

static	void	cb_note_fake_next(UNUSED_WIDGET, Q8tkWidget *notebook)
{
    int n = q8tk_notebook_current_page(notebook) + 1;
    if (COUNTOF(menu_page) <= n) n = 0;
    q8tk_notebook_set_page(notebook, n);
}

static	struct {
    int		key;
    void	(*cb_func)(Q8tkWidget *, Q8tkWidget *);
} menu_fkey[] =
{
    { Q8TK_KEY_F1,	cb_note_fake_f1,  },
    { Q8TK_KEY_F2,	cb_note_fake_f2,  },
    { Q8TK_KEY_F3,	cb_note_fake_f3,  },
    { Q8TK_KEY_F4,	cb_note_fake_f4,  },
    { Q8TK_KEY_F5,	cb_note_fake_f5,  },
    { Q8TK_KEY_F6,	cb_note_fake_f6,  },
    { Q8TK_KEY_F7,	cb_note_fake_f7,  },
    { Q8TK_KEY_F8,	cb_note_fake_f8,  },
    { Q8TK_KEY_F9,	cb_note_fake_f9,  },
    { Q8TK_KEY_F10,	cb_note_fake_f10, },

    { Q8TK_KEY_HOME,	cb_note_fake_prev, },
    { Q8TK_KEY_END,	cb_note_fake_next, },
};

/*----------------------------------------------------------------------*/
				/* �ʰץꥻ�åȥܥ��� �� ��˥����ܥ��� */
static	Q8tkWidget	*monitor_widget;
static	Q8tkWidget	*quickres_widget;

static	int		top_misc_stat	= 1;
static	Q8tkWidget	*top_misc_button;

static	Q8tkWidget *menu_top_misc_quickres(void);
static	Q8tkWidget *menu_top_misc_monitor(void);

static	void	cb_top_misc_stat(UNUSED_WIDGET, UNUSED_PARM)
{
    top_misc_stat ^= 1;
    if (top_misc_stat) {
	q8tk_widget_hide(quickres_widget);
	q8tk_label_set(top_misc_button->child,  ">>");
	q8tk_widget_show(monitor_widget);
    } else {
	q8tk_widget_show(quickres_widget);
	q8tk_label_set(top_misc_button->child,  "<<");
	q8tk_widget_hide(monitor_widget);
    }
}
static	Q8tkWidget	*menu_top_button_misc(void)
{
    Q8tkWidget *box;

    box = PACK_HBOX(NULL);

    quickres_widget = menu_top_misc_quickres();
    monitor_widget  = menu_top_misc_monitor();

    top_misc_button = PACK_BUTTON(NULL,
				  "<<",
				  cb_top_misc_stat, NULL);

    q8tk_box_pack_start(box, quickres_widget);
    q8tk_box_pack_start(box, top_misc_button);
    q8tk_box_pack_start(box, monitor_widget);
    PACK_LABEL(box, "     ");

    top_misc_stat ^= 1;
    cb_top_misc_stat(0, 0);

    return box;
}

/*----------------------------------------------------------------------*/
						/* �ʰץꥻ�åȥܥ���   */
static	int	get_quickres_basic(void)
{
    return reset_req.boot_basic;
}
static	void	cb_quickres_basic(UNUSED_WIDGET, void *p)
{
    if (reset_req.boot_basic != (int)p) {
	reset_req.boot_basic = (int)p;

	q8tk_toggle_button_set_state(widget_reset_basic[ 0 ][ (int)p ], TRUE);
    }
}
static	int	get_quickres_clock(void)
{
    return reset_req.boot_clock_4mhz;
}
static	void	cb_quickres_clock(UNUSED_WIDGET, void *p)
{
    if (reset_req.boot_clock_4mhz != (int)p) {
	reset_req.boot_clock_4mhz = (int)p;

	q8tk_toggle_button_set_state(widget_reset_clock[ 0 ][ (int)p ], TRUE);
    }
}

static	Q8tkWidget *menu_top_misc_quickres(void)
{
    Q8tkWidget *box;
    Q8List     *list;

    box = PACK_HBOX(NULL);
    {
	list = PACK_RADIO_BUTTONS(PACK_VBOX(box),
			data_quickres_basic, COUNTOF(data_quickres_basic),
			get_quickres_basic(), cb_quickres_basic);

	/* �ꥹ�Ȥ�귫�äơ����������åȤ���� */
	widget_reset_basic[1][BASIC_V2 ] = list->data;	list = list->next;
	widget_reset_basic[1][BASIC_V1H] = list->data;	list = list->next;
	widget_reset_basic[1][BASIC_V1S] = list->data;	list = list->next;
	widget_reset_basic[1][BASIC_N  ] = list->data;


	list = PACK_RADIO_BUTTONS(PACK_VBOX(box),
			data_quickres_clock, COUNTOF(data_quickres_clock),
			get_quickres_clock(), cb_quickres_clock);

	/* �ꥹ�Ȥ�귫�äơ����������åȤ���� */
	widget_reset_clock[1][CLOCK_4MHZ] = list->data;	list = list->next;
	widget_reset_clock[1][CLOCK_8MHZ] = list->data;


	PACK_BUTTON(box,
		    GET_LABEL(data_quickres_reset, 0),
		    cb_reset_now, NULL);

	PACK_VSEP(box);
    }
    q8tk_widget_hide(box);

    return box;
}

/*----------------------------------------------------------------------*/
						/* ��˥����ܥ���	*/
static	void	cb_top_monitor(UNUSED_WIDGET, UNUSED_PARM)
{
    quasi88_monitor();		/* �� q8tk_main_quit() �ƽкѤ� */
}

static	Q8tkWidget *menu_top_misc_monitor(void)
{
    Q8tkWidget *box;

    box = PACK_HBOX(NULL);
    {
	PACK_LABEL(box, "  ");

	if (debug_mode) {
	    PACK_BUTTON(box,
			GET_LABEL(data_top_monitor, DATA_TOP_MONITOR_BTN),
			cb_top_monitor, NULL);
	} else {
	    PACK_LABEL(box,
		       GET_LABEL(data_top_monitor, DATA_TOP_MONITOR_PAD));
	}

	PACK_LABEL(box, "     ");
    }
    q8tk_widget_hide(box);

    return box;
}
/*----------------------------------------------------------------------*/
							/* ���ơ�����	*/
static	int	get_top_status(void) { return quasi88_cfg_now_showstatus(); }
static	void	cb_top_status(Q8tkWidget *widget, UNUSED_PARM)
{
    int on = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    if (quasi88_cfg_can_showstatus()) {
	quasi88_cfg_set_showstatus(on);
    }
}

static	void	menu_top_status(Q8tkWidget *base_hbox)
{
    Q8tkWidget *vbox;

    vbox = PACK_VBOX(base_hbox);
    {
	PACK_LABEL(vbox, GET_LABEL(data_top_status, DATA_TOP_STATUS_PAD));

	if (quasi88_cfg_can_showstatus()) {

	    PACK_CHECK_BUTTON(vbox,
			      GET_LABEL(data_top_status, DATA_TOP_STATUS_CHK),
			      get_top_status(),
			      cb_top_status, NULL);

	    PACK_LABEL(vbox, GET_LABEL(data_top_status, DATA_TOP_STATUS_KEY));
	}
    }
}
/*----------------------------------------------------------------------*/
					/* �ᥤ�󥦥���ɥ������Υܥ��� */
static	void	sub_top_savecfg(void);
static	void	sub_top_quit(void);

static	void	cb_top_button(UNUSED_WIDGET, void *p)
{
    switch ((int)p) {
    case DATA_TOP_SAVECFG:
	sub_top_savecfg();
	break;
    case DATA_TOP_EXIT:
	quasi88_exec();		/* �� q8tk_main_quit() �ƽкѤ� */
	break;
    case DATA_TOP_QUIT:
	sub_top_quit();
	return;
    }
}

static	Q8tkWidget	*menu_top_button(void)
{
    int i;
    Q8tkWidget *hbox, *w;
    const t_menudata *p = data_top_button;

    hbox = PACK_HBOX(NULL);
    {
	w = menu_top_button_misc();
	q8tk_box_pack_start(hbox, w);

	menu_top_status(hbox);

	for (i=0; i<COUNTOF(data_top_button); i++, p++) {

	    w = PACK_BUTTON(hbox, GET_LABEL(p, 0),
			    cb_top_button, (void *)(p->val));

	    if (p->val == DATA_TOP_QUIT) {
		q8tk_accel_group_add(menu_accel, Q8TK_KEY_F12, w, "clicked");
	    }
	    if (p->val == DATA_TOP_EXIT) {
		q8tk_accel_group_add(menu_accel, Q8TK_KEY_ESC, w, "clicked");
	    }
	}
    }
    q8tk_misc_set_placement(hbox, Q8TK_PLACEMENT_X_RIGHT, 0);

    return hbox;
}


/*----------------------------------------------------------------------*/
				/* ������¸�ܥ��󲡲����Ρ���ǧ�������� */

static	char	*top_savecfg_filename;

static	void	cb_top_savecfg_clicked(UNUSED_WIDGET, void *p)
{
    dialog_destroy();

    if ((int)p) {
	config_save(top_savecfg_filename);
    }

    free(top_savecfg_filename);
    top_savecfg_filename = NULL;
}
static	int	get_top_savecfg_auto(void)
{
    return save_config;
}
static	void	cb_top_savecfg_auto(Q8tkWidget *widget, UNUSED_PARM)
{
    int parm = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;
    save_config = parm;
}

static	void	sub_top_savecfg(void)
{
    const t_menulabel *l = data_top_savecfg;

    top_savecfg_filename = filename_alloc_global_cfgname();

    if (top_savecfg_filename) {
	dialog_create();
	{
	    dialog_set_title(GET_LABEL(l, DATA_TOP_SAVECFG_TITLE));
	    dialog_set_title(GET_LABEL(l, DATA_TOP_SAVECFG_INFO));
	    dialog_set_title("");
	    dialog_set_title(top_savecfg_filename);
	    dialog_set_title("");
	    dialog_set_check_button(GET_LABEL(l, DATA_TOP_SAVECFG_AUTO),
				    get_top_savecfg_auto(),
				    cb_top_savecfg_auto, NULL);

	    dialog_set_separator();

	    dialog_set_button(GET_LABEL(l, DATA_TOP_SAVECFG_OK),
			      cb_top_savecfg_clicked, (void*)TRUE);

	    dialog_accel_key(Q8TK_KEY_F12);

	    dialog_set_button(GET_LABEL(l, DATA_TOP_SAVECFG_CANCEL),
			      cb_top_savecfg_clicked, (void*)FALSE);

	    dialog_accel_key(Q8TK_KEY_ESC);
	}
	dialog_start();
    }
}


/*----------------------------------------------------------------------*/
				  /* QUIT�ܥ��󲡲����Ρ���ǧ�������� */

static	void	cb_top_quit_clicked(UNUSED_WIDGET, void *p)
{
    dialog_destroy();

    if ((int)p) {
	quasi88_quit();		/* �� q8tk_main_quit() �ƽкѤ� */
    }
}
static	void	sub_top_quit(void)
{
    const t_menulabel *l = data_top_quit;

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_TOP_QUIT_TITLE));

	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_TOP_QUIT_OK),
			  cb_top_quit_clicked, (void*)TRUE);

	dialog_accel_key(Q8TK_KEY_F12);

	dialog_set_button(GET_LABEL(l, DATA_TOP_QUIT_CANCEL),
			  cb_top_quit_clicked, (void*)FALSE);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}


/*----------------------------------------------------------------------*/
				  /* ��˥塼�ΥΡ��ȥڡ������Ѥ�ä��� */

static	void	cb_top_notebook_changed(Q8tkWidget *widget, UNUSED_PARM)
{
    menu_last_page = q8tk_notebook_current_page(widget);
}

/*======================================================================*/

static Q8tkWidget *menu_top(void)
{
    int i;
    const t_menudata *l = data_top;
    Q8tkWidget *note_fake[ COUNTOF(menu_fkey) ];
    Q8tkWidget *win, *vbox, *notebook, *w;

    win = q8tk_window_new(Q8TK_WINDOW_TOPLEVEL);
    menu_accel = q8tk_accel_group_new();
    q8tk_accel_group_attach(menu_accel, win);
    q8tk_widget_show(win);

    vbox = PACK_VBOX(NULL);
    {
	{
	    /* �ƥ�˥塼��Ρ��ȥڡ����˾褻�Ƥ��� */

	    notebook = q8tk_notebook_new();
	    {
		for (i=0; i<COUNTOF(menu_page); i++) {

		    w = (*menu_page[i].menu_func)();
		    q8tk_notebook_append(notebook, w,
					 GET_LABEL(l, menu_page[i].data_num));

		    if (i<COUNTOF(menu_fkey)) {
			note_fake[i] = q8tk_button_new();
			q8tk_signal_connect(note_fake[i], "clicked",
					    menu_fkey[i].cb_func, notebook);
			q8tk_accel_group_add(menu_accel, menu_fkey[i].key,
					     note_fake[i], "clicked");
		    }
		}

		for ( ; i < COUNTOF(menu_fkey); i++) {
		    note_fake[i] = q8tk_button_new();
		    q8tk_signal_connect(note_fake[i], "clicked",
					menu_fkey[i].cb_func, notebook);
		    q8tk_accel_group_add(menu_accel, menu_fkey[i].key,
					 note_fake[i], "clicked");
		}
	    }
	    q8tk_signal_connect(notebook, "switch_page",
				cb_top_notebook_changed, NULL);
	    q8tk_widget_show(notebook);
	    q8tk_box_pack_start(vbox, notebook);

	    /* �¸����ȥåפΥΡ��ȥ֥å��ϡ��ե���������Ŭ�����ꥢ
	             (������ʬ�Υ�������饤��ɽ�����ʤ��ʤ롣�����������) */
	    q8tk_notebook_hook_focus_lost(notebook, TRUE);
	}
	{
	    /* ���Ĥ��ϡ��ܥ��� */

	    w = menu_top_button();
	    q8tk_box_pack_start(vbox, w);
	}
    }
    q8tk_container_add(win, vbox);


	/* �Ρ��ȥ֥å����֤��ޤ� */
    return notebook;
}





/****************************************************************/
/* ��˥塼�⡼�� �ᥤ�����					*/
/****************************************************************/

void	menu_init(void)
{
    int i;

    for (i=0; i<0x10; i++) {			/* ������������������ */
	if      (i == 0x08) key_scan[i] |= 0xdf;	/* ���ʤϻĤ� */
	else if (i == 0x0a) key_scan[i] |= 0x7f;	/* CAPS��Ĥ� */
	else                key_scan[i]  = 0xff;
    }

    /* ���ߤΡ��ꥻ�åȾ������� */
    quasi88_get_reset_cfg(&reset_req);

    /* ���ߤΡ�������ɤ��������¸ */
    sd_cfg_save();

    cpu_timing_save = cpu_timing;

    widget_reset_boot    = NULL;


    status_message_default(0, " MENU ");
    status_message_default(1, "<ESC> key to return");
    status_message_default(2, NULL);



    /* �������顢Q8TK ��Ϣ�ν���� */
    {
	Q8tkWidget *notebook;

	/* Q8TK ����� */
	q8tk_init();

	/* Q8TK ʸ�������ɥ��å� */
	if     (strcmp(menu_kanji_code, menu_kanji_code_euc) == 0)
	{
	    q8tk_set_kanjicode(Q8TK_KANJI_EUC);
	}
	else if (strcmp(menu_kanji_code, menu_kanji_code_sjis) == 0)
	{
	    q8tk_set_kanjicode(Q8TK_KANJI_SJIS);
	}
	else if (strcmp(menu_kanji_code, menu_kanji_code_utf8) == 0)
	{
	    q8tk_set_kanjicode(Q8TK_KANJI_UTF8);
	}
	else
	{
	    q8tk_set_kanjicode(Q8TK_KANJI_ANK);
	}

	/* Q8TK ��������̵ͭ���� (��������˸Ƥ�ɬ�פ���) */
	q8tk_set_cursor(now_swcursor);

	/* Q8TK ��˥塼���� */
	notebook = menu_top();
	q8tk_notebook_set_page(notebook, menu_last_page);
    }
}



void	menu_main(void)
{
    /* Q8TK �ᥤ����� */
    q8tk_main_loop();


    /* ��˥塼��ȴ�����顢��˥塼���ѹ��������Ƥ˱����ơ��ƽ���� */
    if (quasi88_event_flags & EVENT_MODE_CHANGED) {

	if (quasi88_event_flags & EVENT_QUIT) {

	    /* QUASI88��λ���ϡ��ʤˤ⤷�ʤ�      */
	    /* (�ƽ�������Ƥ⤹���˽�λ�ʤΤǡ�) */

	} else {

#ifdef	USE_SOUND
	    if (sd_cfg_has_changed()) {	/* ������ɴ�Ϣ��������ѹ�������� */
		menu_sound_restart(TRUE);     /* ������ɥɥ饤�Фκƽ���� */
	    }
#endif

	    if (cpu_timing_save != cpu_timing) {
		emu_reset();
	    }

	    pc88main_bus_setup();
	    pc88sub_bus_setup();
	}

    } else {

	quasi88_event_flags |= EVENT_FRAME_UPDATE;
    }
}


/*---------------------------------------------------------------------------*/
/*
 * ���ߤΥ�����ɤ������ͤ򵭲����� (��˥塼���ϻ��˸ƤӽФ�)
 */
static	void	sd_cfg_save(void)
{
    int i;
    T_SNDDRV_CONFIG *p;


    memset(&sd_cfg_init, 0, sizeof(sd_cfg_init));
    memset(&sd_cfg_now,  0, sizeof(sd_cfg_now));

    sd_cfg_init.sound_board = sound_board;

#ifdef	USE_SOUND
    sd_cfg_init.sample_freq = xmame_cfg_get_sample_freq();
    sd_cfg_init.use_samples = xmame_cfg_get_use_samples();

#ifdef	USE_FMGEN
    sd_cfg_init.use_fmgen = xmame_cfg_get_use_fmgen();
#endif

    p = xmame_config_get_sndopt_tbl();

    if (p == NULL) {

	i = 0;

    } else {

	for (i=0; i<NR_SD_CFG_LOCAL; i++, p++) {
	    if (p->type == SNDDRV_NULL) break;

	    sd_cfg_init.local[i].info = p;

	    switch (p->type) {
	    case SNDDRV_INT:
		sd_cfg_init.local[i].val.i = *((int *)(p->work));
		break;

	    case SNDDRV_FLOAT:
		sd_cfg_init.local[i].val.f = *((float *)(p->work));
		break;
	    }
	}
    }

    sd_cfg_init.local_cnt = i;

#endif

    sd_cfg_now = sd_cfg_init;
}

/*
 * ������ɤ������ͤ������������ͤȰ㤦�ȡ������֤� (��˥塼��λ���˥����å�)
 */
static	int	sd_cfg_has_changed(void)
{
#ifdef	USE_SOUND
    int i;
    T_SNDDRV_CONFIG *p;

    /* ������ɥܡ��ɤ��ѹ������������å����������㤦������ */
    if (sd_cfg_init.sound_board != sound_board) {
	return TRUE;
    }


#ifdef	USE_FMGEN
    if (sd_cfg_init.use_fmgen != sd_cfg_now.use_fmgen) {
	return TRUE;
    }
#endif

    if (sd_cfg_init.sample_freq != sd_cfg_now.sample_freq) {
	return TRUE;
    }

    if (sd_cfg_init.use_samples != sd_cfg_now.use_samples) {
	return TRUE;
    }

    for (i = 0; i<sd_cfg_init.local_cnt; i++) {

	p = sd_cfg_init.local[i].info;

	switch (p->type) {
	case SNDDRV_INT:
	    if (sd_cfg_init.local[i].val.i != sd_cfg_now.local[i].val.i) {
		return TRUE;
	    }
	    break;

	case SNDDRV_FLOAT:
	    if (sd_cfg_init.local[i].val.f != sd_cfg_now.local[i].val.f) {
		return TRUE;
	    }
	    break;
	}
    }
#endif

    return FALSE;
}

void	menu_sound_restart(int output)
{
    xmame_sound_resume();		/* ���Ǥ���������ɤ�������� */
    xmame_sound_stop();			/* ������ɤ���ߤ����롣     */
    xmame_sound_start();		/* �����ơ�������ɺƽ����   */


    /* ������ɥɥ饤�Ф�ƽ��������ȡ�WAV���Ϥ���³�Ǥ��ʤ���礬���� */
    if (xmame_wavout_damaged()) {
	quasi88_waveout(FALSE);
	XPRINTF("*** Waveout Stop ***\n");
    }


    /* �����ꥹ�����Ȼ��ϡ��ݡ��Ȥκƽ�����ϡ��ƤӽФ����ˤƼ»ܤ��롣
       �����Ǥʤ����ϡ������Ǻƽ���� */
    if (output) {
	sound_output_after_stateload();
    }


    /* ��˥塼�⡼�ɤǤ��δؿ����ƤФ줿���������ơ�����ꥻ�å� */
    sd_cfg_save();


    XPRINTF("*** Sound Setting Is Applied ***\n\n");
}
