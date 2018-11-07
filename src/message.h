/************************************************************************/
/*                                                                      */
/* ����ϥ�å�������ʸ�����������Ƥ���ե�����Ǥ���			*/
/* �����Ǥϡ����ܸ��ʸ���� �� char �����ѿ��˥��åȤ��Ƥ��뤿�ᡢ	*/
/* 8bitʸ����ƥ��ΰ����륳��ѥ���Ǥʤ��ȡ������ư��ޤ���	*/
/*                                                                      */
/************************************************************************/

/*----------------------------------------------------------------------*/
/* ���Υե�����ϡ�menu.c �ե�������ˤ� include ����Ƥ��ޤ���		*/
/*----------------------------------------------------------------------*/
/*									*/
/* QUASI88 �� �����ȥ�ɽ������ӥ�˥塼����ɽ���ˤ�ɽ�������ʸ�����	*/
/* ���Ƥ��Υե�������������Ƥ��ޤ���					*/
/*									*/
/*----------------------------------------------------------------------*/

/*
#ifndef	NR_DRIVE
#define	NR_DRIVE	2
#endif
*/

/* 0xb4, 0xc1, 0xbb, 0xfa �ʤ顢 EUC-Japan */
/* 0x8a, 0xbf, 0x8e, 0x9a �ʤ顢 Shift-JIS */
/* ����ʳ��ʤ顢                �����ġ�  */

/* ���Υե����뤬 EUC-japan �� Shift-JIS ��������å����롦���� */

const char *menu_kanji_code = "����";
const char menu_kanji_code_euc[]  = { 0xb4, 0xc1, 0xbb, 0xfa, 0x00 };
const char menu_kanji_code_sjis[] = { 0x8a, 0xbf, 0x8e, 0x9a, 0x00 };
const char menu_kanji_code_utf8[] = { 0xe6, 0xbc, 0xa2, 0xe5, 0xad, 0x97, 0x00 };


typedef struct {
    char	*str[2];	/* [0]��ANKʸ����  [1]�����ܸ�ʸ����	*/
    int		val;		/* int�� ����				*/
} t_menudata;

typedef struct {
    char	*str[2];	/* [0]��ANKʸ����  [1]�����ܸ�ʸ����	*/
} t_menulabel;




/***************************************************************
 * QUASI88 �� ��˥塼�ˤƻ��Ѥ���ʸ����
 ****************************************************************/

/*--------------------------------------------------------------
 *	��˥塼�� '*' ���֤ˤ�ɽ����������
 *--------------------------------------------------------------*/

static const char *data_about_en[] =
{
#ifdef	USE_SOUND
    "MAME Sound Driver ... Available",
    "   " Q_MAME_COPYRIGHT,
    "@MAMEVER",
#ifdef	USE_FMGEN
    "",
    "FM Sound Generator ... Available",
    "   " Q_FMGEN_COPYRIGHT,
    "@FMGENVER",
#endif
#else
    "SOUND OUTPUT ... Not available",
#endif  
    "",
#ifdef	USE_MONITOR
    "Monitor mode ... Supported",
    "",
#endif

    NULL,	/* ��ü */
};


static const char *data_about_jp[] =
{
#ifdef	USE_SOUND
    "MAME ������ɥɥ饤�� ���Ȥ߹��ޤ�Ƥ��ޤ�",
    "   " Q_MAME_COPYRIGHT,
    "@MAMEVER",
#ifdef	USE_FMGEN
    "",
    "FM Sound Generator ���Ȥ߹��ޤ�Ƥ��ޤ�",
    "   " Q_FMGEN_COPYRIGHT,
    "@FMGENVER",
#endif
#else
    "������ɽ��� ���Ȥ߹��ޤ�Ƥ��ޤ���",
#endif
    "",
#ifdef	USE_MONITOR
    "��˥����⡼�ɤ����ѤǤ��ޤ�",
    "",
#endif

    NULL,	/* ��ü */
};





/*--------------------------------------------------------------
 *	�ᥤ���˥塼����
 *--------------------------------------------------------------*/
enum {
  DATA_TOP_RESET,
  DATA_TOP_CPU,
  DATA_TOP_GRAPH,
  DATA_TOP_VOLUME,
  DATA_TOP_DISK,
  DATA_TOP_KEY,
  DATA_TOP_MOUSE,
  DATA_TOP_TAPE,
  DATA_TOP_MISC,
  DATA_TOP_ABOUT
};
static const t_menudata data_top[] =
{
  { { " RESET ",        " �ꥻ�å� ",       }, DATA_TOP_RESET  },
  { { " CPU ",          " CPU  ",           }, DATA_TOP_CPU    },
  { { " SCREEN ",       " ���� ",           }, DATA_TOP_GRAPH  },
  { { " VOLUME ",       " ���� ",           }, DATA_TOP_VOLUME },
  { { " DISK ",         " �ǥ����� ",       }, DATA_TOP_DISK   },
  { { " KEY ",          " ���� ",           }, DATA_TOP_KEY    },
  { { " MOUSE ",        " �ޥ��� ",         }, DATA_TOP_MOUSE  },
  { { " TAPE ",         " �ơ��� ",         }, DATA_TOP_TAPE   },
  { { " MISC ",         "  ¾  ",           }, DATA_TOP_MISC   },
  { { " ABOUT ",        "��",               }, DATA_TOP_ABOUT  },
};


enum {
    DATA_TOP_STATUS_PAD,
    DATA_TOP_STATUS_CHK,
    DATA_TOP_STATUS_KEY
};
static const t_menulabel data_top_status[] =
{
  { { "              ",  "              " } },
  { { "Status",          "���ơ�����"     } },
  { { "        (F11) ",  "         (F11)" } },
};


enum {
    DATA_TOP_MONITOR_PAD,
    DATA_TOP_MONITOR_BTN
};
static const t_menulabel data_top_monitor[] =
{
  { { "            ",  "            ", } },
  { { " MONITOR  ",    " ��˥��� ",   } },
};


enum {
  DATA_TOP_SAVECFG,
  DATA_TOP_QUIT,
  DATA_TOP_EXIT
};
static const t_menudata data_top_button[] =
{
  { { " Save Cfg.",    " ������¸ ",      }, DATA_TOP_SAVECFG },
  { { " QUIT(F12) ",   " ��λ(F12) ",     }, DATA_TOP_QUIT    },
  { { " EXIT(ESC) ",   " ���(ESC) ",     }, DATA_TOP_EXIT    },
};


enum {
  DATA_TOP_SAVECFG_TITLE,
  DATA_TOP_SAVECFG_INFO,
  DATA_TOP_SAVECFG_AUTO,
  DATA_TOP_SAVECFG_OK,
  DATA_TOP_SAVECFG_CANCEL
};
static const t_menulabel data_top_savecfg[] =
{
  { { "Save settings in following file. ", "���ߤ�����򡢰ʲ��δĶ�����ե��������¸���ޤ�" } },
  { { "(Some settings are not saved)    ", "�ʰ���������ϡ���¸����ޤ����                " } },
  { { "Save when QUASI88 exit. ",          "��λ���ˡ���ư����¸����"                         } },
  { { "   OK   ",                          " ��¸ "                                           } },
  { { " CANCEL ",                          " ��� "                                           } },
};


enum {
  DATA_TOP_QUIT_TITLE,
  DATA_TOP_QUIT_OK,
  DATA_TOP_QUIT_CANCEL
};
static const t_menulabel data_top_quit[] =
{
  { { " *** QUIT NOW, REALLY ? *** ", "�����˽�λ���ơ����������" } },
  { { "   OK   (F12) ",               " ��λ (F12) "                 } },
  { { " CANCEL (ESC) ",               " ��� (ESC) "                 } },
};


static const t_menudata data_quickres_basic[] =
{
  { { "V2 ", "V2 ", }, BASIC_V2,  },
  { { "V1H", "V1H", }, BASIC_V1H, },
  { { "V1S", "V1S", }, BASIC_V1S, },
  { { "N",   "N",   }, BASIC_N,   },	/* ��ɽ���� */
};
static const t_menudata data_quickres_clock[] =
{
  { { "4MHz", "4MHz", }, CLOCK_4MHZ, },
  { { "8MHz", "8MHz", }, CLOCK_8MHZ, },
};
static const t_menulabel data_quickres_reset[] =
{
  { { "RST", "RST" } },
};






/*--------------------------------------------------------------
 *	�ֲ��̡� ����
 *--------------------------------------------------------------*/
enum {
  DATA_GRAPH_FRATE,
  DATA_GRAPH_RESIZE,
  DATA_GRAPH_PCG,
  DATA_GRAPH_FONT
};
static const t_menulabel data_graph[] =
{
  { { " <<< FRAME RATE >>> ", " �ե졼��졼�� " } },
  { { " <<< RESIZE >>> ",     " ���̥����� "     } },
  { { " <<< PCG-8100 >>>",    " PCG-8100 "       } },
  { { " <<< FONT >>> ",       " �ե���� "       } },
};

static const t_menudata data_graph_frate[] =
{
  { { "60", "60", },   1, },
  { { "30", "30", },   2, },
  { { "20", "20", },   3, },
  { { "15", "15", },   4, },
  { { "12", "12", },   5, },
  { { "10", "10", },   6, },
  { { "6",  "6",  },  10, },
  { { "5",  "5",  },  12, },
  { { "4",  "4",  },  15, },
  { { "3",  "3",  },  20, },
  { { "2",  "2",  },  30, },
  { { "1",  "1",  },  60, },
};

static const t_menulabel data_graph_autoskip[] =
{
  { { "Auto frame skip (-autoskip) ", "�����ȥ����åפ�ͭ���ˤ��� (-autoskip) ", } },
};

static const t_menudata data_graph_resize[] =
{
  { { " HALF SIZE (-half) ",     " Ⱦʬ������ (-half) ", }, SCREEN_SIZE_HALF,   },
  { { " FULL SIZE (-full) ",     " ɸ�ॵ���� (-full) ", }, SCREEN_SIZE_FULL,   },
#ifdef	SUPPORT_DOUBLE
  { { " DOUBLE SIZE (-double) ", " �ܥ����� (-double) ", }, SCREEN_SIZE_DOUBLE, },
#endif
};

static const t_menulabel data_graph_fullscreen[] =
{
  { { "Full Screen (-fullscreen)", "�ե륹���꡼�� (-fullscreen) ", } },
};

enum {
  DATA_GRAPH_MISC_15K,
  DATA_GRAPH_MISC_DIGITAL,
  DATA_GRAPH_MISC_NOINTERP
};
static const t_menudata data_graph_misc[] =
{
  { { "Monitor Freq. 15k       (-15k)",         "��˥����ȿ���15k������      (-15k)"        }, DATA_GRAPH_MISC_15K        },
  { { "Digital Monitor         (-digital)",     "�ǥ������˥�������         (-digital)"    }, DATA_GRAPH_MISC_DIGITAL    },
  { { "No reduce interpolation (-nointerp)",    "Ⱦʬ���������˽̾���֤��ʤ� (-nointerp)"   }, DATA_GRAPH_MISC_NOINTERP   },
};

static const t_menudata data_graph_misc2[] =
{
  { { "Fill-Line Display       (-noskipline)",  "�饤��η�֤�����         (-noskipline)" }, SCREEN_INTERLACE_NO   },
  { { "Skip-Line Display       (-skipline)",    "1�饤�󤪤���ɽ������        (-skipline)"   }, SCREEN_INTERLACE_SKIP },
  { { "Interlace Display       (-interlace)",   "���󥿡��졼��ɽ������       (-interlace)"  }, SCREEN_INTERLACE_YES  },
};

static const t_menudata data_graph_pcg[] =
{
  { { " Noexist ", " �ʤ� "  }, FALSE },
  { { " Exist ",   " ����  " }, TRUE  },
};

#if 0
static const t_menudata data_graph_font[] =
{
  { { " Standard Font ",  " ɸ��ե���� " }, 0 },
  { { " 2nd Font ",       " �裲�ե���� " }, 1 },
  { { " 3rd Font ",       " �裳�ե���� " }, 2 },
};
#else
static const t_menudata data_graph_font1[2] =
{
  { { " Built-in Font ",  " �� ¢ �ե���� " }, 0 },
  { { " Standard Font ",  " ɸ �� �ե���� " }, 0 },
};
static const t_menudata data_graph_font2[2] =
{
  { { " Hiragana Font ",  " ʿ��̾�ե���� " }, 1 },
  { { " 2nd Font ",       " �� �� �ե���� " }, 1 },
};
static const t_menudata data_graph_font3[2] =
{
  { { " Transparent Font ", " Ʃ �� �ե���� " }, 2 },
  { { " 3rd Font ",         " �� �� �ե���� " }, 2 },
};
#endif



/*--------------------------------------------------------------
 *	��CPU�� ����
 *--------------------------------------------------------------*/
enum {
  DATA_CPU_CPU,
  DATA_CPU_CLOCK,
  DATA_CPU_WAIT,
  DATA_CPU_BOOST,
  DATA_CPU_HELP
};
static const t_menulabel data_cpu[] =
{
  { { " <<< SUB-CPU MODE >>> ", " SUB-CPU��ư <�ѹ����ϥꥻ�åȤ�侩> ", } },
  { { " << CLOCK >> ",          " CPU ����å� (-clock) ",                } },
  { { " << WAIT >> ",           " ®�� (-speed, -nowait) ",               } },
  { { " << BOOST >> ",          " �֡����� (-boost) ",                    } },
  { { " HELP ",                 " ���� ",                                 } },
};



static const t_menudata data_cpu_cpu[] =
{
  { { "   0  Run SUB-CPU only during the disk access. (-cpu 0)  ", "   0  �ǥ����������桢����CPU�Τ߶�ư������ (-cpu 0)  ", }, 0 },
  { { "   1  Run both CPUs during the disk access.    (-cpu 1)  ", "   1  �ǥ����������桢ξCPU���ư������     (-cpu 1)  ", }, 1 },
  { { "   2  Always run both CPUs.                    (-cpu 2)  ", "   2  �����ξCPU���ư������               (-cpu 2)  ", }, 2 },
};


enum {
  DATA_CPU_CLOCK_CLOCK,
  DATA_CPU_CLOCK_MHZ,
  DATA_CPU_CLOCK_INFO
};
static const t_menulabel data_cpu_clock[] =
{
  { { " CLOCK     ",          " ���ȿ� "              }, },
  { { "[MHz] ",               "[MHz] "                }, },
  { { "(Range = 0.1-999.9) ", "���ϰϡ�0.1��999.9�� " }, },
};
static const t_menudata data_cpu_clock_combo[] =
{
  { { " ( 1MHz) ",  " ( 1MHz) ", }, (int)(CONST_4MHZ_CLOCK * 1000000.0/4) },
  { { " ( 2MHz) ",  " ( 2MHz) ", }, (int)(CONST_4MHZ_CLOCK * 1000000.0/2) },
  { { "== 4MHz==",  "== 4MHz==", }, (int)(CONST_4MHZ_CLOCK * 1000000.0)   },
  { { " ( 8MHz) ",  " ( 8MHz) ", }, (int)(CONST_8MHZ_CLOCK * 1000000.0)   },
  { { " (16MHz) ",  " (16MHz) ", }, (int)(CONST_8MHZ_CLOCK * 1000000.0*2) },
  { { " (32MHz) ",  " (32MHz) ", }, (int)(CONST_8MHZ_CLOCK * 1000000.0*4) },
  { { " (64MHz) ",  " (64MHz) ", }, (int)(CONST_8MHZ_CLOCK * 1000000.0*8) },
};


enum {
  DATA_CPU_WAIT_NOWAIT,
  DATA_CPU_WAIT_RATE,
  DATA_CPU_WAIT_PERCENT,
  DATA_CPU_WAIT_INFO
};
static const t_menulabel data_cpu_wait[] =
{
  { { "No Wait             ", "�������Ȥʤ��ˤ���  "  } },
  { { " Rate of Speed ",      " ®����     "          } },
  { { "[%]   ",               "[��]  "                } },
  { { "(Range =   5-5000)  ", "���ϰϡ�  5��5000��  " } },
};
static const t_menudata data_cpu_wait_combo[] =
{
  { { "  25",  "  25" },   25 },
  { { "  50",  "  50" },   50 },
  { { " 100",  " 100" },  100 },
  { { " 200",  " 200" },  200 },
  { { " 400",  " 400" },  400 },
  { { " 800",  " 800" },  800 },
  { { "1600",  "1600" }, 1600 },
};


enum {
  DATA_CPU_BOOST_MAGNIFY,
  DATA_CPU_BOOST_UNIT,
  DATA_CPU_BOOST_INFO
};
static const t_menulabel data_cpu_boost[] =
{
  { { " Power         ",      " ��Ψ       "          } },
  { { "      ",               "��    "                } },
  { { "(Range =   1-100)   ", "���ϰϡ�  1��100��   " } },
};
static const t_menudata data_cpu_boost_combo[] =
{
  { { "   1", "   1", },   1, },
  { { "   2", "   2", },   2, },
  { { "   4", "   4", },   4, },
  { { "   8", "   8", },   8, },
  { { "  16", "  16", },  16, },
};



enum {
  DATA_CPU_MISC_FDCWAIT,
  DATA_CPU_MISC_FDCWAIT_X,
  DATA_CPU_MISC_BLANK,
  DATA_CPU_MISC_HSBASIC,
  DATA_CPU_MISC_HSBASIC_X,
  DATA_CPU_MISC_BLANK2,
  DATA_CPU_MISC_MEMWAIT,
  DATA_CPU_MISC_MEMWAIT_X,
  DATA_CPU_MISC_CMDSING,
};
static const t_menudata data_cpu_misc[] =
{
  { { "FDC Wait ON",        "FDC�������Ȥ��� ",       }, DATA_CPU_MISC_FDCWAIT },
  { { "(-fdc_wait)",        "(-fdc_wait)",            }, -1                    },
  { { "",                   "",                       }, -1                    },
  { { "HighSpeed BASIC ON", "��®BASIC���� ͭ�� ",    }, DATA_CPU_MISC_HSBASIC },
  { { "(-hsbasic)",         "(-hsbasic)",             }, -1                    },
  { { "",                   "",                       }, -1                    },
  { { "Memory Wait(dummy)", "�����ꥦ������",       }, DATA_CPU_MISC_MEMWAIT },
  { { "(-mem_wait)",        "(-mem_wait)",            }, -1                    },

#if 0
  { { "",                   "",                       }, -1                    },
  { { "CMD SING",           "CMD SING",               }, DATA_CPU_MISC_CMDSING },
#endif
};



/*--------------------------------------------------------------
 *	�֥ꥻ�åȡ� ����
 *--------------------------------------------------------------*/
enum {
  DATA_RESET_CURRENT,
  DATA_RESET_BASIC,
  DATA_RESET_CLOCK,
  DATA_RESET_VERSION,
  DATA_RESET_DIPSW,
  DATA_RESET_DIPSW_BTN,
  DATA_RESET_SOUND,
  DATA_RESET_EXTRAM,
  DATA_RESET_JISHO,
  DATA_RESET_NOTICE,
  DATA_RESET_DIPSW_SET,
  DATA_RESET_DIPSW_QUIT,
  DATA_RESET_BOOT,
  DATA_RESET_NOW,
  DATA_RESET_INFO
};
static const t_menulabel data_reset[] =
{
  { { " Current Mode : ",         " ���ߤΥ⡼�� ��"               } },
  { { " BASIC MODE ",             " BASIC �⡼�� "                 } },
  { { " CPU CLOCK ",              " CPU ����å� "                 } },
  { { " ROM VERSION ",            " ROM �С������ "               } },
  { { " DIP-Switch ",             " �ǥ��åץ����å� "             } },
  { { " Setting ",                " ���� "                         } },
  { { " Sound Board",             " ������ɥܡ��� "               } },
  { { " ExtRAM",                  " ��ĥRAM "                      } },
  { { " Dict.ROM",                " ����ROM "                      } },
  { { "(*) When checked, Real CPU clock depend on the 'CPU'-TAB setting. ",
      "(��) �����å������äƤ����硢�ºݤΥ���å��ϡ�CPU�٥�������ΤޤޤȤʤ�ޤ�" } },
  { { " <<< DIP-SW Setting >>> ", " <<< �ǥ��åץ����å����� >>> " } },
  { { " EXIT ",                   " ��� "                         } },
  { { " BOOT ",                   " ��ư "                         } },
  { { " RESET now ! ",            " ��������ǥꥻ�åȤ��� "       } },
  { { " (Without a reset, the setting is not applied.) ",
      " �ʥꥻ�åȤ򤷤ʤ��ȡ������ȿ�Ǥ���ޤ����"              } },
};

static const t_menudata data_reset_basic[] =
{
  { { " N88 V2  ", " N88 V2  ", }, BASIC_V2,  },
  { { " N88 V1H ", " N88 V1H ", }, BASIC_V1H, },
  { { " N88 V1S ", " N88 V1S ", }, BASIC_V1S, },
  { { " N       ", " N       ", }, BASIC_N,   },
};

static const t_menudata data_reset_clock[] =
{
  { { " 4MHz ", " 4MHz ", }, CLOCK_4MHZ, },
  { { " 8MHz ", " 8MHz ", }, CLOCK_8MHZ, },
};

static const t_menulabel data_reset_clock_async[] =
{
  { { "Async (*) ",   "��Ϣư (��) ", }, },
};

static const t_menudata data_reset_version[] =
{
  { { "Default",  " ������" }, 0   },
  { { "  1.0",    "  1.0"   }, '0' },
  { { "  1.1",    "  1.1"   }, '1' },
  { { "  1.2",    "  1.2"   }, '2' },
  { { "  1.3",    "  1.3"   }, '3' },
  { { "  1.4",    "  1.4"   }, '4' },
  { { "  1.5",    "  1.5"   }, '5' },
  { { "  1.6",    "  1.6"   }, '6' },
  { { "  1.7",    "  1.7"   }, '7' },
  { { "  1.8",    "  1.8"   }, '8' },
  { { "  1.9*",   "  1.9*"  }, '9' },
};

static const t_menulabel data_reset_boot[] =
{
  { { " Boot from DISK  ", "  �ǥ����� " } },
  { { " Boot from ROM   ", "  �ңϣ�   " } },
};

static const t_menudata data_reset_sound[] =
{
  { { " Sound board    (OPN)  ", " ������ɥܡ���   (OPN)  ", }, SOUND_I  },
  { { " Sound board II (OPNA) ", " ������ɥܡ���II (OPNA) ", }, SOUND_II },
};

static const t_menudata data_reset_extram[] =
{
  { { " Nothing ",   " �ʤ�    "  },  0 },
  { { "    128KB",   "    128KB"  },  1 },
  { { "    256KB",   "    256KB"  },  2 },
  { { "    384KB",   "    384KB"  },  3 },
  { { "    512KB",   "    512KB"  },  4 },
  { { "      1MB",   "      1MB"  },  8 },
  { { " 1M+128KB",   " 1M+128KB"  },  9 },
  { { " 1M+256KB",   " 1M+256KB"  }, 10 },
  { { "      2MB",   "      2MB"  }, 16 },
};

static const t_menudata data_reset_jisho[] =
{
  { { " no-jisho ",  " �ʤ� ", }, 0 },
  { { " has jisho ", " ���� ", }, 1 },
};

static const t_menulabel data_reset_current[] =
{
  { { " ExtRAM", "��ĥRAM" } },
  { { "DictROM", "����ROM" } },
};

static const t_menulabel data_reset_detail[] =
{
  { { " Misc. << ", " ����¾ << " } },
  { { " Misc. >> ", " ����¾ >> " } },
};



/*--------------------------------------------------------------
 *	�ֲ��̡� ����
 *--------------------------------------------------------------*/
enum {
  DATA_VOLUME_TOTAL,
  DATA_VOLUME_LEVEL,
  DATA_VOLUME_DEPEND,
  DATA_VOLUME_AUDIO,
  DATA_VOLUME_AUDIO_SET,
  DATA_VOLUME_AUDIO_QUIT,
  DATA_VOLUME_AUDIO_INFO,
};
static const t_menulabel data_volume[] =
{
  { { " Volume ",             " ���� "                       } },
  { { " Level ",              " ��٥� "                     } },
  { { " depend on FM-level ", " �ʲ��ϣƣͲ��̤˰�¸���ޤ� " } },

  { { " Setting ",                                               " �ܺ����� "                                        } },
  { { " <<< Sound-device Setting >>> ",                          " <<< ������ɥǥХ����ܺ����� >>> "                } },
  { { " EXIT ",                                                  " ��� "                                            } },
  { { " The settings are applied in the return from the menu. ", " ����ϡ���˥塼�⡼�ɤ������ݤ�ȿ�Ǥ���ޤ���" } },
};



static const t_menulabel data_volume_no[] =
{
  { { " SoundBoard (OPN)                        [ Sound Output is not available. ] "," ������ɥܡ��� (OPN)                [ ������ɽ��Ϥ��Ȥ߹��ޤ�Ƥ��ޤ��� ] " } },
  { { " SoundBoard II (OPNA)                    [ Sound Output is not available. ] "," ������ɥܡ���II (OPNA)             [ ������ɽ��Ϥ��Ȥ߹��ޤ�Ƥ��ޤ��� ] " } },
  { { " SoundBoard (OPN)                   [ Sound Output is OFF. ] ",               " ������ɥܡ��� (OPN)       [ ������ɽ��Ϥϥ��վ��֤Ǥ� ] " } },
  { { " SoundBoard II (OPNA)               [ Sound Output is OFF. ] ",               " ������ɥܡ���II (OPNA)    [ ������ɽ��Ϥϥ��վ��֤Ǥ� ] " } },
};



static const t_menulabel data_volume_type[] =
{
  { { " SoundBoard (OPN)            [ MAME built-in FM Generator ] ",                " ������ɥܡ��� (OPN)      [ FM���������ͥ졼����MAME��¢ ] " } },
  { { " SoundBoard II (OPNA)        [ MAME built-in FM-Generator ] ",                " ������ɥܡ���II (OPNA)   [ FM���������ͥ졼����MAME��¢ ] " } },
  { { " SoundBoard (OPN)                    [ fmgen FM-Generator ] ",                " ������ɥܡ��� (OPN)         [ FM���������ͥ졼����fmgen ] " } },
  { { " SoundBoard II (OPNA)                [ fmgen FM-Generator ] ",                " ������ɥܡ���II (OPNA)      [ FM���������ͥ졼����fmgen ] " } },
};



typedef struct {
  char	*str[2];
  int	val;
  int	min;
  int	max;
  int	step;
  int	page;
} t_volume;



enum {
  VOL_TOTAL,
  VOL_FM,
  VOL_PSG,
  VOL_BEEP,
  VOL_RHYTHM,
  VOL_ADPCM,
  VOL_FMGEN,
  VOL_SAMPLE
};

static const t_volume data_volume_total[] =
{
  { { " VOLUME [db]    :",  " ���� [���]    ��" }, VOL_TOTAL, VOL_MIN,      VOL_MAX,      1, 4},
};

static const t_volume data_volume_level[] =
{
  { { " FM sound   [%] :",  " �ƣͲ���   [��]��" }, VOL_FM,    FMVOL_MIN,    FMVOL_MAX,    1,10},
  { { " PSG sound  [%] :",  " �Уӣǲ��� [��]��" }, VOL_PSG,   PSGVOL_MIN,   PSGVOL_MAX,   1,10},
  { { " BEEP sound [%] :",  " �£ţţв� [��]��" }, VOL_BEEP,  BEEPVOL_MIN,  BEEPVOL_MAX,  1,10},
};

static const t_volume data_volume_rhythm[] =
{
  { { " RHYTHM     [%] :",  " �ꥺ�಻�� [��]��" }, VOL_RHYTHM,RHYTHMVOL_MIN,RHYTHMVOL_MAX,1,10},
  { { " ADPCM      [%] :",  " ADPCM ���� [��]��" }, VOL_ADPCM, ADPCMVOL_MIN, ADPCMVOL_MAX, 1,10},
};

static const t_volume data_volume_fmgen[] =
{
  { { " FM & PSG   [%] :",  " FM/PSG���� [��]��" }, VOL_FMGEN, FMGENVOL_MIN, FMGENVOL_MAX, 1,10},
  { { " BEEP sound [%] :",  " �£ţţв� [��]��" }, VOL_BEEP,  BEEPVOL_MIN,  BEEPVOL_MAX,  1,10},
};

static const t_volume data_volume_sample[] =
{
  { { " SAMPLE snd [%] :",  " ����ץ벻 [��]��" }, VOL_SAMPLE,SAMPLEVOL_MIN,SAMPLEVOL_MAX,1,10},
};


enum {
  DATA_VOLUME_AUDIO_FMGEN,
  DATA_VOLUME_AUDIO_FREQ,
  DATA_VOLUME_AUDIO_SAMPLE,
};
static const t_menulabel data_volume_audio[] =
{
  { { " FM Generator     ",                            " FM���������ͥ졼��       ",                    } },
  { { " Sample-Frequency ([Hz], Range = 8000-48000) ", " ����ץ�󥰼��ȿ� ([Hz],�ϰϡ�8000��48000) ", } },
  { { " Sample Data      ",                            " ����ץ벻�λ���̵ͭ     ",                    } },
};

static const t_menudata data_volume_audio_fmgen[] =
{
  { { " MAME built-in ", " MAME ��¢  ", }, FALSE  },
  { { " fmgen",          " fmgen",       }, TRUE   },
};
static const t_menudata data_volume_audio_freq_combo[] =
{
  { { "48000", "48000" }, 48000 },
  { { "44100", "44100" }, 44100 },
  { { "22050", "22050" }, 22050 },
  { { "11025", "11025" }, 11025 },
};
static const t_menudata data_volume_audio_sample[] =
{
  { { " Not Use       ", " ���Ѥ��ʤ� ", }, FALSE  },
  { { " Use",            " ���Ѥ��� ",   }, TRUE   },
};


static const t_menulabel data_volume_audiodevice_stop[] =
{
  { { " (The sound device is stopping.)", " �ʥ�����ɥǥХ����ϡ������Ǥ���", } },
};







/*--------------------------------------------------------------
 *	��DIP-SW��
 *--------------------------------------------------------------*/
enum {
  DATA_DIPSW_B,
  DATA_DIPSW_R
};
static const t_menulabel data_dipsw[] =
{
  { { " Boot up ", " ������� "    } },
  { { " RC232C ",  " RS232C ���� " } },
};


typedef struct{
  char	*str[2];
  int	val;
  const t_menudata *p;
} t_dipsw;




static const t_menudata data_dipsw_b_term[] =
{
  { { "TERMINAL   ", "�����ߥʥ� " }, (0<<1) | 0 },
  { { "BASIC      ", "�£��ӣɣ� " }, (0<<1) | 1 },
};
static const t_menudata data_dipsw_b_ch80[] =
{
  { { "80ch / line", "������     " }, (1<<1) | 0 },
  { { "40ch / line", "������     " }, (1<<1) | 1 },
};
static const t_menudata data_dipsw_b_ln25[] =
{
  { { "25line/scrn", "������     " }, (2<<1) | 0 },
  { { "20line/scrn", "������     " }, (2<<1) | 1 },
};
static const t_menudata data_dipsw_b_boot[] =
{
  { { "DISK       ", "�ǥ�����   " }, FALSE },
  { { "ROM        ", "�ңϣ�     " }, TRUE  },
};

static const t_dipsw data_dipsw_b[] =
{
  { { "BOOT MODE           :", "Ω���夲�⡼��     ��" },  1, data_dipsw_b_term },
  { { "Chars per Line      :", "���Ԥ������ʸ���� ��" },  2, data_dipsw_b_ch80 },
  { { "Lines per screen    :", "�����̤�����ιԿ� ��" },  3, data_dipsw_b_ln25 },
};
static const t_dipsw data_dipsw_b2[] =
{
  { { "Boot Up from        :", "�����ƥ��Ω���夲 ��" }, -1, data_dipsw_b_boot },
};



static const t_menudata data_dipsw_r_baudrate[] =
{
  { {    "75",    "75" }, 0 },
  { {   "150",   "150" }, 1 },
  { {   "300",   "300" }, 2 },
  { {   "600",   "600" }, 3 },
  { {  "1200",  "1200" }, 4 },
  { {  "2400",  "2400" }, 5 },
  { {  "4800",  "4800" }, 6 },
  { {  "9600",  "9600" }, 7 },
  { { "19200", "19200" }, 8 },
};


static const t_menudata data_dipsw_r_hdpx[] =
{
  { { "HALF       ", "Ⱦ���     " }, (0<<1) | 0 },
  { { "FULL       ", "�����     " }, (0<<1) | 1 },
};
static const t_menudata data_dipsw_r_xprm[] =
{
  { { "Enable     ", "ͭ  ��     " }, (1<<1) | 0 },
  { { "Disable    ", "̵  ��     " }, (1<<1) | 1 },
};
static const t_menudata data_dipsw_r_st2b[] =
{
  { { "2 bit      ", "�� bit     " }, (2<<1) | 0 },
  { { "1 bit      ", "�� bit     " }, (2<<1) | 1 },
};
static const t_menudata data_dipsw_r_dt8b[] =
{
  { { "8 bit      ", "�� bit     " }, (3<<1) | 0 },
  { { "7 bit      ", "�� bit     " }, (3<<1) | 1 },
};
static const t_menudata data_dipsw_r_sprm[] =
{
  { { "Enable     ", "ͭ  ��     " }, (4<<1) | 0 },
  { { "Disable    ", "̵  ��     " }, (4<<1) | 1 },
};
static const t_menudata data_dipsw_r_pdel[] =
{
  { { "Enable     ", "ͭ  ��     " }, (5<<1) | 0 },
  { { "Disable    ", "̵  ��     " }, (5<<1) | 1 },
};
static const t_menudata data_dipsw_r_enpty[] =
{
  { { "Yes        ", "ͭ  ��     " }, (6<<1) | 0 },
  { { "No         ", "̵  ��     " }, (6<<1) | 1 },
};
static const t_menudata data_dipsw_r_evpty[] =
{
  { { "Even       ", "��  ��     " }, (7<<1) | 0 },
  { { "Odd        ", "��  ��     " }, (7<<1) | 1 },
};

static const t_menulabel data_dipsw_r2[] =
{
  { { "Baud Rate (BPS)     :", "�̿�®�١Υܡ���   ��" } },
};
static const t_dipsw data_dipsw_r[] =
{
  { { "Duplex              :", "��  ��  ��  ��     ��" }, 5 +8, data_dipsw_r_hdpx  },
  { { "X parameter         :", "�إѥ�᡼��       ��" }, 4 +8, data_dipsw_r_xprm  },
  { { "Stop Bit            :", "���ȥåץӥå�Ĺ   ��" }, 3 +8, data_dipsw_r_st2b  },
  { { "Data Bit            :", "�ǡ����ӥå�Ĺ     ��" }, 2 +8, data_dipsw_r_dt8b  },
  { { "S parameter         :", "�ӥѥ�᡼��       ��" }, 4,    data_dipsw_r_sprm  },
  { { "DEL code            :", "�ģţ̥�����       ��" }, 5,    data_dipsw_r_pdel  },
  { { "Patiry Check        :", "�ѥ�ƥ������å�   ��" }, 0 +8, data_dipsw_r_enpty },
  { { "Patiry              :", "��  ��  �ƥ�       ��" }, 1 +8, data_dipsw_r_evpty },
};


/*--------------------------------------------------------------
 *	�֥ǥ������� ����
 *--------------------------------------------------------------*/
static const t_menulabel data_disk_image_drive[] =
{
  { { " <<< DRIVE [1:] >>> ", " <<< DRIVE [1:] >>> " } },
  { { " <<< DRIVE [2:] >>> ", " <<< DRIVE [2:] >>> " } },
};
static const t_menulabel data_disk_info_drive[] =
{
  { { "   DRIVE [1:]   ", "   DRIVE [1:]   " } },
  { { "   DRIVE [2:]   ", "   DRIVE [2:]   " } },
};

enum {
  DATA_DISK_IMAGE_EMPTY,
  DATA_DISK_IMAGE_BLANK
};
static const t_menulabel data_disk_image[] =
{
  { { "< EMPTY >                 ", "< �ʤ� >                  " } },
  { { "  Create Blank  ",           " �֥�󥯤κ��� "           } },
};

enum {
  DATA_DISK_INFO_STAT,		/* "STATUS     READY" ��16ʸ�� */
  DATA_DISK_INFO_STAT_READY,
  DATA_DISK_INFO_STAT_BUSY,
  DATA_DISK_INFO_ATTR,		/* "ATTR  READ/WRITE" ��16ʸ�� */
  DATA_DISK_INFO_ATTR_RW,
  DATA_DISK_INFO_ATTR_RO,
  DATA_DISK_INFO_NR,		/* "IMAGE  xxxxxxxxx" ��16ʸ���� x �� 9ʸ�� */
  DATA_DISK_INFO_NR_BROKEN,
  DATA_DISK_INFO_NR_OVER
};
static const t_menulabel data_disk_info[] =
{
  { { "STATUS     ",         "����       "      } },
  { {            "READY",               "READY" } },
  { {            "BUSY ",               "BUSY " } },
  { { "ATTR  ",              "°��  "           } },
  { {       "  Writable",          "    �����" } },
  { {       " Read Only",          "  �ɹ�����" } },
  { { "IMAGE  ",             "���   "          } },
  { {          "+BROKEN",              " +��»" } },
  { {            " OVER",              " �ʾ� " } },
};



enum {
  IMG_OPEN,
  IMG_CLOSE,
  IMG_BOTH,
  IMG_COPY,
  IMG_ATTR
};
static const t_menulabel data_disk_button_drv1[] =
{
  { { " DRIVE [1:]           OPEN ", " DRIVE [1:]           ���� " } },
  { { " DRIVE [1:]          CLOSE ", " DRIVE [1:]         �Ĥ��� " } },
  { { " DRIVE [1:][2:] BOTH  OPEN ", " DRIVE [1:][2:] ξ���˳��� " } },
  { { " DRIVE [1:] <= [2:]   OPEN ", " DRIVE [1:] �� [2:]   ���� " } },
  { { " CHANGE ATTRIBUTE of IMAGE ", " ���᡼���� °�����ѹ����� " } },
};
static const t_menulabel data_disk_button_drv2[] =
{
  { { " DRIVE [2:]           OPEN ", " DRIVE [2:]           ���� " } },
  { { " DRIVE [2:]          CLOSE ", " DRIVE [2:]         �Ĥ��� " } },
  { { " DRIVE [1:][2:] BOTH  OPEN ", " DRIVE [1:][2:] ξ���˳��� " } },
  { { " DRIVE [1:] => [2:]   OPEN ", " DRIVE [1:] �� [2:]   ���� " } },
  { { " CHANGE ATTRIBUTE of IMAGE ", " ���᡼���� °�����ѹ����� " } },
};
static const t_menulabel data_disk_button_drv1swap[] =
{
  { { " OPEN           DRIVE [1:] ", " ����           DRIVE [1:] " } },
  { { " CLOSE          DRIVE [1:] ", " �Ĥ���         DRIVE [1:] " } },
  { { " OPEN  BOTH DRIVE [1:][2:] ", " ξ���˳��� DRIVE [2:][1:] " } },
  { { " OPEN   DRIVE [1:] => [2:] ", " ����   DRIVE [2:] �� [1:] " } },
  { { " CHANGE ATTRIBUTE of IMAGE ", " ���᡼���� °�����ѹ����� " } },
};
static const t_menulabel data_disk_button_drv2swap[] =
{
  { { " OPEN           DRIVE [2:] ", " ����           DRIVE [2:] " } },
  { { " CLOSE          DRIVE [2:] ", " �Ĥ���         DRIVE [2:] " } },
  { { " OPEN  BOTH DRIVE [1:][2:] ", " ξ���˳��� DRIVE [2:][1:] " } },
  { { " OPEN   DRIVE [1:] <= [2:] ", " ����   DRIVE [2:] �� [1:] " } },
  { { " CHANGE ATTRIBUTE of IMAGE ", " ���᡼���� °�����ѹ����� " } },
};



enum {
  DATA_DISK_OPEN_OPEN,
  DATA_DISK_OPEN_BOTH
};
static const t_menulabel data_disk_open_drv1[] =
{
  { { " OPEN FILE in DRIVE [1:] ",        " DRIVE [1:] �˥��᡼���ե�����򥻥åȤ��ޤ� "         } },
  { { " OPEN FILE in DRIVE [1:] & [2:] ", " DRIVE [1:] �� [2:] �˥��᡼���ե�����򥻥åȤ��ޤ� " } },
};
static const t_menulabel data_disk_open_drv2[] =
{
  { { " OPEN FILE in DRIVE [2:] ",        " DRIVE [2:] �˥��᡼���ե�����򥻥åȤ��ޤ� "         } },
  { { " OPEN FILE in DRIVE [1:] & [2:] ", " DRIVE [1:] �� [2:] �˥��᡼���ե�����򥻥åȤ��ޤ� " } },
};



enum {
  DATA_DISK_ATTR_TITLE1,
  DATA_DISK_ATTR_TITLE1_,
  DATA_DISK_ATTR_TITLE2,
  DATA_DISK_ATTR_RENAME,
  DATA_DISK_ATTR_PROTECT,
  DATA_DISK_ATTR_FORMAT,
  DATA_DISK_ATTR_BLANK,
  DATA_DISK_ATTR_CANCEL
};
static const t_menulabel data_disk_attr[] =
{
  { { " Change Attribute of the image at drive 1: ", " �ɥ饤�� 1: �Υ��᡼�� "    } },
  { { " Change Attribute of the image at drive 2: ", " �ɥ饤�� 2: �Υ��᡼�� "    } },
  { { " ",                                           " �� °���ѹ��ʤɤ�Ԥ��ޤ� " } },
  { { "RENAME",                                      "̾���ѹ�"                    } },
  { { "PROTECT",                                     "°���ѹ�"                    } },
  { { "(UN)FORMAT",                                  "(����)�ե����ޥå�"          } },
  { { "APPEND BLANK",                                "�֥�󥯤��ɲ�"              } },
  { { "CANCEL",                                      " ��� "                      } },
};

enum {
  DATA_DISK_ATTR_RENAME_TITLE1,
  DATA_DISK_ATTR_RENAME_TITLE1_,
  DATA_DISK_ATTR_RENAME_TITLE2,
  DATA_DISK_ATTR_RENAME_OK,
  DATA_DISK_ATTR_RENAME_CANCEL
};
static const t_menulabel data_disk_attr_rename[] =
{
  { { " Rename the image at drive 1: ", " �ɥ饤�� 1: �Υ��᡼�� " } },
  { { " Rename the image at drive 2: ", " �ɥ饤�� 2: �Υ��᡼�� " } },
  { { " ",                              " �� ̾�����ѹ����ޤ� "    } },
  { { "  OK  ",                         " �ѹ� "                   } },
  { { "CANCEL",                         " ��� "                   } },
};

enum {
  DATA_DISK_ATTR_PROTECT_TITLE1,
  DATA_DISK_ATTR_PROTECT_TITLE1_,
  DATA_DISK_ATTR_PROTECT_TITLE2,
  DATA_DISK_ATTR_PROTECT_SET,
  DATA_DISK_ATTR_PROTECT_UNSET,
  DATA_DISK_ATTR_PROTECT_CANCEL
};
static const t_menulabel data_disk_attr_protect[] =
{
  { { " (Un)Peotect the image at drive 1: ", " �ɥ饤�� 1: �Υ��᡼�� "        } },
  { { " (Un)Peotect the image at drive 2: ", " �ɥ饤�� 2: �Υ��᡼�� "        } },
  { { " ",                                   " �� �ץ�ƥ��Ⱦ��֤��ѹ����ޤ� " } },
  { { " SET PROTECT ",                       " �ץ�ƥ��Ⱦ��֤ˤ��� "          } },
  { { " UNSET PROTECT ",                     " �ץ�ƥ��Ȥ������� "          } },
  { { " CANCEL ",                            " ��� "                          } },
};

enum {
  DATA_DISK_ATTR_FORMAT_TITLE1,
  DATA_DISK_ATTR_FORMAT_TITLE1_,
  DATA_DISK_ATTR_FORMAT_TITLE2,
  DATA_DISK_ATTR_FORMAT_WARNING,
  DATA_DISK_ATTR_FORMAT_DO,
  DATA_DISK_ATTR_FORMAT_NOT,
  DATA_DISK_ATTR_FORMAT_CANCEL
};
static const t_menulabel data_disk_attr_format[] =
{
  { { " (Un)Format the image at drive 1: ",       " �ɥ饤�� 1: �Υ��᡼�� "                  } },
  { { " (Un)Format the image at drive 2: ",       " �ɥ饤�� 2: �Υ��᡼�� "                  } },
  { { " ",                                        " �� �ʥ���˥ե����ޥåȤ��ޤ� "           } },
  { { "[WARNING : data in the image will lost!]", "[���:���᡼����Υǡ����Ͼõ��ޤ���]" } },
  { { " FORMAT ",                                 " �ե����ޥåȤ��� "                        } },
  { { " UNFORMAT ",                               " ����ե����ޥåȤ��� "                    } },
  { { " CANCEL ",                                 " ��� "                                    } },
};

enum {
  DATA_DISK_ATTR_BLANK_TITLE1,
  DATA_DISK_ATTR_BLANK_TITLE1_,
  DATA_DISK_ATTR_BLANK_TITLE2,
  DATA_DISK_ATTR_BLANK_OK,
  DATA_DISK_ATTR_BLANK_CANCEL,
  DATA_DISK_ATTR_BLANK_END
};
static const t_menulabel data_disk_attr_blank[] =
{
  { { " Append Blank image at drive 1: ", " �ɥ饤�� 1: �Υե������ "     } },
  { { " Append Blank image at drive 2: ", " �ɥ饤�� 2: �Υե������ "     } },
  { { " ",                                " �֥�󥯥��᡼�����ɲä��ޤ� " } },
  { { " APPEND ",                         " �֥�󥯥��᡼�����ɲ� "       } },
  { { " CANCEL ",                         " ��� "                         } },
};



enum {
  DATA_DISK_BLANK_FSEL,
  DATA_DISK_BLANK_WARN_0,
  DATA_DISK_BLANK_WARN_1,
  DATA_DISK_BLANK_WARN_APPEND,
  DATA_DISK_BLANK_WARN_CANCEL
};
static const t_menulabel data_disk_blank[] =
{
  { { " Create a new file as blank image file.", " �֥�󥯥��᡼���ե�����򿷵��������ޤ� " } },
  { { " This File Already Exist. ",              " ���ꤷ���ե�����Ϥ��Ǥ�¸�ߤ��ޤ��� "     } },
  { { " Append a blank image ? ",                " �֥�󥯥��᡼�����ɲä��ޤ����� "         } },
  { { " APPEND ",                                " �ɲä��� "                                 } },
  { { " CANCEL ",                                " ��� "                                     } },
};


enum {
  DATA_DISK_FNAME,
  DATA_DISK_FNAME_TITLE,
  DATA_DISK_FNAME_LINE,
  DATA_DISK_FNAME_SAME,
  DATA_DISK_FNAME_SEP,
  DATA_DISK_FNAME_RO,
  DATA_DISK_FNAME_RO_1,
  DATA_DISK_FNAME_RO_2,
  DATA_DISK_FNAME_RO_X,
  DATA_DISK_FNAME_RO_Y,
  DATA_DISK_FNAME_OK
};
static const t_menulabel data_disk_fname[] =
{
  { { " Show Filename  ",                                 " �ե�����̾��ǧ "                                 } },
  { { " Disk Image Filename ",                            " �ǥ��������᡼���ե�����̾��ǧ "                 } },
  { { "------------------------------------------------", "------------------------------------------------" } },
  { { " Same file Drive 1: as Drive 2 ",                  " �ɥ饤�� 1: �� 2: ��Ʊ���ե�����Ǥ� "           } },
  { { " ",                                                " "                                                } },
  { { "    * The disk image file(s) is read-only.      ", "���ɹ����ѤΥǥ��������᡼���ե����뤬����ޤ���" } },
  { { "      All images in this file are regarded      ", "  ���Υե�����˴ޤޤ�뤹�٤ƤΥ��᡼���ϡ�    " } },
  { { "      as WRITE-PROTECTED.                       ", "  �饤�ȥץ�ƥ��Ⱦ��֤�Ʊ�ͤ˰����ޤ���      " } },
  { { "      Writing to the image is ignored, but      ", "  ���Υե�����ؤν񤭹��ߤ�����̵�뤵��ޤ�����" } },
  { { "      not error depending on situation.         ", "  ���顼�Ȥ�ǧ������ʤ���礬����ޤ���        " } },
  { { "  OK  ",                                           " ��ǧ "                                           } },
};



enum {
  DATA_DISK_DISPSWAP,
  DATA_DISK_DISPSWAP_INFO_1,
  DATA_DISK_DISPSWAP_INFO_2,
  DATA_DISK_DISPSWAP_OK
};
static const t_menulabel data_disk_dispswap[] =
{
  { { "Swap Drv-Disp",                                   "ɽ�� ��������"                                     } },
  { { "Swap Drive-Display placement",                    "DRIVE [1:] �� [2:] ��ɽ�����֤򡢺������촹���ޤ�" } },
  { { "This setting effects next time. ",                "��������ϼ���Υ�˥塼�⡼�ɤ��ͭ���Ȥʤ�ޤ� " } },
  { { "  OK  ",                                          " ��ǧ "                                            } },
};



enum {
  DATA_DISK_DISPSTATUS,
  DATA_DISK_DISPSTATUS_INFO,
  DATA_DISK_DISPSTATUS_OK
};
static const t_menulabel data_disk_dispstatus[] =
{
  { { "Show in status",                                  "ɽ�� ���ơ�����"                                   } },
  { { "Display image name in status area. ",             "���ơ��������ˡ����᡼��̾��ɽ�����ޤ�"            } },
  { { "  OK  ",                                          " ��ǧ "                                            } },
};



/*--------------------------------------------------------------
 *	�֤���¾�� ����
 *--------------------------------------------------------------*/

enum {
  DATA_MISC_SUSPEND,
  DATA_MISC_SNAPSHOT,
  DATA_MISC_WAVEOUT,
};
static const t_menulabel data_misc[] =
{
  { { "State Save     ",    "���ơ��ȥ����� " } },
  { { "Screen Shot    ",    "������¸       " } },
  { { "Sound Record   ",    "���������¸   " } },
};





enum {
  DATA_MISC_SUSPEND_CHANGE,
  DATA_MISC_SUSPEND_SAVE,
  DATA_MISC_SUSPEND_LOAD,
  DATA_MISC_SUSPEND_NUMBER,
  DATA_MISC_SUSPEND_FSEL
};
static const t_menulabel data_misc_suspend[] =
{
  { { " Change ",                           " �ե������ѹ� "                       } },
  { { " SAVE ",                             " ������ "                             } },
  { { " LOAD ",                             " ���� "                             } },
  { { "                         number : ", "                 Ϣ�֡� "             } },
  { { " Input (Select) a state filename. ", " ���ơ��ȥե�����̾�����Ϥ��Ʋ����� " } },
};


static const t_menudata data_misc_suspend_num[] =
{
  { { "(none)",  "(�ʤ�)" }, 0   },
  { { "0",       "0"      }, '0' },
  { { "1",       "1"      }, '1' },
  { { "2",       "2"      }, '2' },
  { { "3",       "3"      }, '3' },
  { { "4",       "4"      }, '4' },
  { { "5",       "5"      }, '5' },
  { { "6",       "6"      }, '6' },
  { { "7",       "7"      }, '7' },
  { { "8",       "8"      }, '8' },
  { { "9",       "9"      }, '9' },
};




enum {
  DATA_MISC_SUSPEND_OK,
  DATA_MISC_RESUME_OK,
  DATA_MISC_SUSPEND_LINE,
  DATA_MISC_SUSPEND_INFO,
  DATA_MISC_SUSPEND_AGREE,
  DATA_MISC_SUSPEND_ERR,
  DATA_MISC_RESUME_ERR,
  DATA_MISC_SUSPEND_REALLY,
  DATA_MISC_SUSPEND_OVERWRITE,
  DATA_MISC_SUSPEND_CANCEL,
  DATA_MISC_RESUME_CANTOPEN
};
static const t_menulabel data_misc_suspend_err[] =
{
  { { "State save Finished.",                           "���֤���¸���ޤ�����"                               } },
  { { "State load Finished.",                           "���֤��������ޤ�����"                               } },
  { { "----------------------------------------------", "----------------------------------------------"     } },
  { { "      ( Following image files are set )       ", " ( �ʲ��Υ��᡼���ե����뤬���ꤵ��Ƥ��ޤ� ) "     } },
  { { " OK ",                                           "��ǧ"                                               } },
  { { "Error / State save failed.",                     "���顼�����֤���¸����ޤ���Ǥ�����"               } },
  { { "Error / State load failed. Reset done",          "���顼�����֤������˼��Ԥ��ޤ������ꥻ�åȤ��ޤ���" } },
  { { "State-file already exist, Over write ?",         "�ե�����Ϥ��Ǥ�¸�ߤ��ޤ�����񤭤��ޤ�����"       } },
  { { " Over Write ",                                   "���"                                             } },
  { { " Cancel ",                                       "���"                                               } },
  { { "State-file not exist or broken.",                "���ơ��ȥե����뤬̵����������Ƥ��ޤ���"           } },
};



enum {
  DATA_MISC_SNAPSHOT_FORMAT,
  DATA_MISC_SNAPSHOT_CHANGE,
  DATA_MISC_SNAPSHOT_PADDING,
  DATA_MISC_SNAPSHOT_BUTTON,
  DATA_MISC_SNAPSHOT_FSEL,
  DATA_MISC_SNAPSHOT_CMD
};
static const t_menulabel data_misc_snapshot[] =
{
  { { " Format   ",                                        " ��������   "                                   } },
  { { " Change ",                                          " �١���̾�ѹ� "                                 } },
  { { "                    ",                              "            "                                   } },
  { { " SAVE ",                                            " ��¸ "                                         } },
  { { " Input (Select) a screen-snapshot base-filename. ", " ��¸����ե����� (�١���̾) �����Ϥ��Ʋ����� " } },
  { { "Exec following Command",                            "���Υ��ޥ�ɤ�¹Ԥ���"                         } },
};

static const t_menudata data_misc_snapshot_format[] =
{
  { { " BMP ", " BMP " }, 0 },
  { { " PPM ", " PPM " }, 1 },
  { { " RAW ", " RAW " }, 2 },
};



enum {
  DATA_MISC_WAVEOUT_CHANGE,
  DATA_MISC_WAVEOUT_START,
  DATA_MISC_WAVEOUT_STOP,
  DATA_MISC_WAVEOUT_PADDING,
  DATA_MISC_WAVEOUT_FSEL
};
static const t_menulabel data_misc_waveout[] =
{
  { { " Change ",                                       " �١���̾�ѹ� "                                 } },
  { { " START ",                                        " ���� "                                         } },
  { { " STOP ",                                         " ��� "                                         } },
  { { "                                            ",   "                                       "        } },
  { { " Input (Select) a sound-record base-filename. ", " ���Ϥ���ե����� (�١���̾) �����Ϥ��Ʋ����� " } },
};



static const t_menulabel data_misc_sync[] =
{
  { { "synchronize filename with disk-image filename", "�ƥե�����̾��ǥ��������᡼���Υե�����̾�˹�碌��", } },
};





/*--------------------------------------------------------------
 *	�֥����� ����
 *--------------------------------------------------------------*/
enum {
  DATA_KEY_FKEY,
  DATA_KEY_CURSOR,
  DATA_KEY_CURSOR_SPACING,
  DATA_KEY_SKEY,
  DATA_KEY_SKEY2
};
static const t_menulabel data_key[] =
{
  { { " Function key Config ",             " �ե��󥯥���󥭡����� ",         } },
  { { " Curosr Key Config ",               " �������륭������ ",               } }, 
  { { "                                 ", "                                ", } },
  { { " Software ",                        " ���եȥ����� ",                   } },
  { { "   Keyboard  ",                     "  �����ܡ���  ",                   } },
};



enum {
  DATA_KEY_CFG_TENKEY,
  DATA_KEY_CFG_NUMLOCK
};
static const t_menudata data_key_cfg[] =
{
  { { "Set numeric key to TEN-key (-tenkey) ",  "����������ƥ󥭡��˳�����Ƥ�   (-tenkey)  ", }, DATA_KEY_CFG_TENKEY,  },
  { { "software NUM-Lock ON       (-numlock)",  "���եȥ����� NUM Lock �򥪥󤹤� (-numlock) ", }, DATA_KEY_CFG_NUMLOCK, },
};



static const t_menudata data_key_fkey[] =
{
  { { "   f6  key ",  "   f6  ���� ", },  6 },
  { { "   f7  key ",  "   f7  ���� ", },  7 },
  { { "   f8  key ",  "   f8  ���� ", },  8 },
  { { "   f9  key ",  "   f9  ���� ", },  9 },
  { { "   f10 key ",  "   f10 ���� ", }, 10 },
};
static const t_menudata data_key_fkey_fn[] =
{
  { { "----------- : function or another key",  "----------- : �ե��󥯥����ޤ���Ǥ�ե���", },  FN_FUNC,        },
  { { "FRATE-UP    : Frame Rate  Up",           "FRATE-UP    : �ե졼��졼�� �夲�� ",       },  FN_FRATE_UP,    },
  { { "FRATE-DOWN  : Frame Rate  Down",         "FRATE-DOWN  : �ե졼��졼�� ������ ",       },  FN_FRATE_DOWN,  },
  { { "VOLUME-UP   : Volume  Up",               "VOLUME-UP   : ���� �夲��",                  },  FN_VOLUME_UP,   },
  { { "VOLUME-DOWN : Volume  Down",             "VOLUME-DOWN : ���� ������",                  },  FN_VOLUME_DOWN, },
  { { "PAUSE       : Pause",                    "PAUSE       : ������",                     },  FN_PAUSE,       },
  { { "RESIZE      : Resize",                   "RESIZE      : ���̥������ѹ�",               },  FN_RESIZE,      },
  { { "NOWAIT      : No-Wait",                  "NOWAIT      : �������Ȥʤ�",                 },  FN_NOWAIT,      },
  { { "SPEED-UP    : Speed Up",                 "SPEED-UP    : ®�� �夲�� ",                 },  FN_SPEED_UP,    },
  { { "SPEED-DOWN  : Speed Down",               "SPEED-DOWN  : ®�� ������ ",                 },  FN_SPEED_DOWN,  },
  { { "FULLSCREEN  : Full Screen Mode",         "FULLSCREEN  : �ե륹���꡼������",           },  FN_FULLSCREEN,  },
  { { "SNAPSHOT    : Save Screen Snapshot",     "SNAPSHOT    : �����꡼�󥹥ʥåץ���å�",   },  FN_SNAPSHOT,    },
  { { "IMAGE-NEXT1 : Drive 1:  Next Image",     "IMAGE-NEXT1 : Drive 1:  �����᡼��",         },  FN_IMAGE_NEXT1, },
  { { "IMAGE-PREV1 : Drive 1:  Prev Image",     "IMAGE-PREV1 : Drive 1:  �����᡼��",         },  FN_IMAGE_PREV1, },
  { { "IMAGE-NEXT2 : Drive 2:  Next Image",     "IMAGE-NEXT2 : Drive 2:  �����᡼��",         },  FN_IMAGE_NEXT2, },
  { { "IMAGE-PREV2 : Drive 2:  Prev Image",     "IMAGE-PREV2 : Drive 2:  �����᡼��",         },  FN_IMAGE_PREV2, },
  { { "NUMLOCK     : Software NUM Lock",        "NUMLOCK     : ���եȥ����� NUM Lock",        },  FN_NUMLOCK,     },
  { { "RESET       : Reset switch",             "RESET       : �ꥻ�å� �����å�",            },  FN_RESET,       },
  { { "KANA        : KANA key",                 "KANA        : ���� ����",                    },  FN_KANA,        },
  { { "ROMAJI      : KANA(ROMAJI) Key",         "ROMAJI      : ����(���޻�����) ����",      },  FN_ROMAJI,      },
  { { "CAPS        : CAPS Key",                 "CAPS        : CAPS ����",                    },  FN_CAPS,        },
  { { "MAX-SPEED   : Max Speed",                "MAX-SPEED   : ®�ٺ���������",               },  FN_MAX_SPEED,   },
  { { "MAX-CLOCK   : Max CPU-Clock",            "MAX-CLOCK   : CPU����å�����������",        },  FN_MAX_CLOCK,   },
  { { "MAX-BOOST   : Max Boost",                "MAX-BOOST   : �֡����Ⱥ���������",           },  FN_MAX_BOOST,   },
  { { "STATUS      : Display status",           "STATUS      : ���ơ�����ɽ���Υ��󡿥���",   },  FN_STATUS,      },
  { { "MENU        : Go Menu-Mode",             "MENU        : ��˥塼",                     },  FN_MENU,        },
};



static const t_menudata data_key_fkey2[] =
{
  { { "   ",  "   ", },  6 },
  { { "   ",  "   ", },  7 },
  { { "   ",  "   ", },  8 },
  { { "   ",  "   ", },  9 },
  { { "   ",  "   ", }, 10 },
};



enum {
  DATA_SKEY_BUTTON_SETUP,
  DATA_SKEY_BUTTON_OFF,
  DATA_SKEY_BUTTON_QUIT
};
static const t_menulabel data_skey_set[] =
{
  { { "Setting",                " ���� ",               } },
  { { "All key release & QUIT", " ���ƥ��դˤ������ ", } },
  { { " QUIT ",                 " ��� ",               } },
};



static const t_menudata data_key_cursor_mode[] =
{
  { { " Default(CursorKey)",  " ɸ��(�������륭��)",    },  0, },
  { { " Assign to 2,4,6,8",   " 2,4,6,8 ��������",    },  1, },
  { { " Assign arbitrarily ", " Ǥ�դΥ����������� ", },  2, },
};
static const t_menudata data_key_cursor[] =
{
  { { "             ",          "             ",     },   0, },
  { { "                \036",   "               ��", },  -1, },
  { { " ",                      " ",                 },   2, },
  { { "\035           \034 ",   "��        �� ",     },   3, },
  { { "                \037",   "               ��", },  -1, },
  { { "             ",          "             ",     },   1, },
};



/*--------------------------------------------------------------
 *	�֥ޥ����� ����
 *--------------------------------------------------------------*/
enum {
  DATA_MOUSE_MODE,
  DATA_MOUSE_SERIAL,

  DATA_MOUSE_SYSTEM,

  DATA_MOUSE_DEVICE_MOUSE,
  DATA_MOUSE_DEVICE_JOY,
  DATA_MOUSE_DEVICE_JOY2,
  DATA_MOUSE_DEVICE_ABOUT,

  DATA_MOUSE_DEVICE_NUM,

  DATA_MOUSE_CONNECTING,
  DATA_MOUSE_SWAP_MOUSE,
  DATA_MOUSE_SWAP_JOY,
  DATA_MOUSE_SWAP_JOY2
};
static const t_menulabel data_mouse[] =
{
  { { " Mouse / Joystick setting ", " �ޥ��������祤���ƥ��å���³ "  } },
  { { " Serial-mouse ",             " ���ꥢ��ޥ��� "                } },

  { { " [ System Setup  (Some settings are disabled in some systems.)]               ",
      " �ڥ����ƥ�����  (����ΰ����ϡ������ƥ�ˤ�äƤ�̵���Ǥ�)��                 ", } },

  { { " Mouse ",                         " �ޥ������� ",                         } },
  { { " Joystick ",                      " ���祤���ƥ��å����� ",               } },
  { { " Joystick(2) ",                   " ���祤���ƥ��å�(2)���� "             } },
  { { " About ",                         " �� "                                  } },

  { { " %d Joystick(s) is found.",       "  %d �ĤΥ��祤���ƥ��å������ѤǤ��ޤ���" } },

  { { "  Connecting mouse-port   ",         "  �ޥ����ݡ��Ȥ���³��    "         } },
  { { "Swap mouse buttons",                 "�����ܥ���������ؤ���"             } },
  { { "Swap joystick buttons (-joy_swap)",  "���¥ܥ���������ؤ��� (-joyswap)"  } },
  { { "Swap joystick buttons",              "���¥ܥ���������ؤ���"             } },
};



static const t_menudata data_mouse_mode[] =
{
  { { "Not Connect               (-nomouse) ", "�ʤˤ���³���ʤ�                     (-nomouse) " }, MOUSE_NONE     },
  { { "Connect Mouse             (-mouse)   ", "�ޥ�������³                         (-mouse)   " }, MOUSE_MOUSE    },
  { { "Connect Mouse as joystick (-joymouse)", "�ޥ����򥸥祤���ƥ��å��⡼�ɤ���³ (-joymouse)" }, MOUSE_JOYMOUSE },
  { { "Connect joystick          (-joystick)", "���祤���ƥ��å�����³               (-joystick)" }, MOUSE_JOYSTICK },
};



static const t_menulabel data_mouse_serial[] =
{
  { { "Connect (-serialmouse)", "��³ (-serialmouse)" } },
};



static const t_menudata data_mouse_mouse_key_mode[] =
{
  { { " Not Assigned ",          " ����������Ƥʤ�"       },  0 },
  { { " Assign to 2,4,6,8,x,z ", " 2,4,6,8,x,z�������� " },  1 },
  { { " Assign arbitrarily    ", " Ǥ�դΥ�����������"   },  2 },
};
static const t_menudata data_mouse_mouse[] =
{
  { { "             ",          "             ",     },   0, },
  { { "                \036",   "               ��", },  -1, },
  { { " ",                      " ",                 },   2, },
  { { "\035           \034 ",   "��        �� ",     },   3, },
  { { "                \037",   "               ��", },  -1, },
  { { "             ",          "             ",     },   1, },
  { { "",                       "",                  },  -1, },
  { { " L ",                    " �� "               },   4, },
  { { "",                       "",                  },  -1, },
  { { " R ",                    " �� "               },   5, },
};



static const t_menudata data_mouse_joy_key_mode[] =
{
  { { " Not Assigned ",          " ����������Ƥʤ�"       },  0 },
  { { " Assign to 2,4,6,8,x,z ", " 2,4,6,8,x,z�������� " },  1 },
  { { " Assign arbitrarily    ", " Ǥ�դΥ�����������"   },  2 },
};
static const t_menudata data_mouse_joy[] =
{
  { { "             ",          "             ",     },   0, },
  { { "                \036",   "               ��", },  -1, },
  { { " ",                      " ",                 },   2, },
  { { "\035           \034 ",   "��        �� ",     },   3, },
  { { "                \037",   "               ��", },  -1, },
  { { "             ",          "             ",     },   1, },
  { { " A ",                    " �� "               },   4, },
  { { " B ",                    " �� "               },   5, },
  { { " C ",                    " �� "               },   6, },
  { { " D ",                    " �� "               },   7, },
  { { " E ",                    " �� "               },   8, },
  { { " F ",                    " �� "               },   9, },
  { { " G ",                    " �� "               },  10, },
  { { " H ",                    " �� "               },  11, },
};
static const t_menudata data_mouse_joy2_key_mode[] =
{
  { { " Not Assigned ",          " ����������Ƥʤ�"       },  0 },
  { { " Assign to 2,4,6,8,x,z ", " 2,4,6,8,x,z�������� " },  1 },
  { { " Assign arbitrarily    ", " Ǥ�դΥ�����������"   },  2 },
};
static const t_menudata data_mouse_joy2[] =
{
  { { "             ",          "             ",     },   0, },
  { { "                \036",   "               ��", },  -1, },
  { { " ",                      " ",                 },   2, },
  { { "\035           \034 ",   "��        �� ",     },   3, },
  { { "                \037",   "               ��", },  -1, },
  { { "             ",          "             ",     },   1, },
  { { " A ",                    " �� "               },   4, },
  { { " B ",                    " �� "               },   5, },
  { { " C ",                    " �� "               },   6, },
  { { " D ",                    " �� "               },   7, },
  { { " E ",                    " �� "               },   8, },
  { { " F ",                    " �� "               },   9, },
  { { " G ",                    " �� "               },  10, },
  { { " H ",                    " �� "               },  11, },
};



static const t_volume data_mouse_sensitivity[] =
{
  { { " Sensitivity [%] :",  " �ޥ������� [��]��" }, -1, 10, 200,  1, 10},
};



static const t_menulabel data_mouse_misc_msg[] =
{
  { { " Mouse Cursor : ", " �ޥ������������ " } },
};
static const t_menudata data_mouse_misc[] =
{
  { { "Always show the mouse cursor            (-show_mouse) ", "���ɽ������             (-show_mouse) " }, SHOW_MOUSE },
  { { "Always Hide the mouse cursor            (-hide_mouse) ", "��˱���                 (-hide_mouse) " }, HIDE_MOUSE },
  { { "Auto-hide the mouse cutsor              (-auto_mouse) ", "��ưŪ�˱���             (-auto_mouse) " }, AUTO_MOUSE },
  { { "Confine the mouse cursor on the screend (-grab_mouse) ", "���̤��Ĥ������ʱ����� (-grab_mouse) " }, -1         },
  { { "Confine the mouse cursor when mouse clicked           ", "����å����Ĥ������                   " }, -2         },
};



static const t_menudata data_mouse_debug_hide[] =
{
  { { " SHOW ", " ɽ�� ", }, SHOW_MOUSE, },
  { { " HIDE ", " ���� ", }, HIDE_MOUSE, },
  { { " AUTO ", " ��ư ", }, AUTO_MOUSE, },
};
static const t_menudata data_mouse_debug_grab[] =
{
  { { " UNGRAB ", " Υ�� ", }, UNGRAB_MOUSE, },
  { { " GRAB   ", " �Ϥ� ", }, GRAB_MOUSE,   },
  { { " AUTO   ", " ��ư ", }, AUTO_MOUSE,   },
};


/*--------------------------------------------------------------
 *	�֥ơ��ס� ����
 *--------------------------------------------------------------*/
enum {
  DATA_TAPE_IMAGE,
  DATA_TAPE_INTR
};
static const t_menulabel data_tape[] =
{
  { { " Tape image ",       " �ơ��ץ��᡼�� "         } },
  { { " Tape Load Timing ", " �ơ��ץ��ɤν�����ˡ " } },
};



enum {
  DATA_TAPE_FOR,
  DATA_TAPE_CHANGE,
  DATA_TAPE_EJECT,
  DATA_TAPE_FSEL,
  DATA_TAPE_REWIND,
  DATA_TAPE_WARN_0,
  DATA_TAPE_WARN_1,
  DATA_TAPE_WARN_APPEND,
  DATA_TAPE_WARN_CANCEL
};
static const t_menulabel data_tape_load[] =
{
  { { " for Load :",                                           " �����ѡ�"                                      } },
  { { " Change File ",                                         " �ե������ѹ� "                                   } },
  { { " Eject  ",                                              " ��Ф� "                                         } },
  { { " Input (Select) a tape-load-image filename. (CMT/T88)", " �����ѥơ��ץ��᡼��(CMT/T88)�����Ϥ��Ʋ�����" } },
  { { " Rewind ",                                              " ���ᤷ "                                         } },
};
static const t_menulabel data_tape_save[] =
{
  { { " for Save :",                                           " �������ѡ�"                                      } },
  { { " Change File ",                                         " �ե������ѹ� "                                   } },
  { { " Eject  ",                                              " ��Ф� "                                         } },
  { { " Input (Select) a tape-save-image filename. (CMT)",     " �������ѥơ��ץ��᡼��(CMT)�����Ϥ��Ʋ�����"     } },
  { { NULL,                                                    NULL,                                              } },
  { { " This File Already Exist. ",                            " ���ꤷ���ե�����Ϥ��Ǥ�¸�ߤ��ޤ��� "           } },
  { { " Append a tape image ? ",                               " �ơ��ץ��᡼�����ɵ����Ƥ����ޤ����� "           } },
  { { " OK ",                                                  " �ɵ����� "                                       } },
  { { " CANCEL ",                                              " ��� "                                           } },
};


static const t_menudata data_tape_intr[] =
{
  { { " Use Interrupt     (Choose in N88-BASIC mode) ",                 " �����ߤ�Ȥ�     (N88-BASIC �Ǥϡ�ɬ������������򤷤Ƥ�������) "   }, TRUE  },
  { { " Not Use Interrupt (Choose in N-BASIC mode for LOAD speed-up) ", " �����ߤ�Ȥ�ʤ� (N-BASIC �ϡ�������Ǥ�ġ����ɤ�®���ʤ�ޤ�) " }, FALSE },
};


/*--------------------------------------------------------------
 * �ե��������顼��������
 *--------------------------------------------------------------*/

enum {
  ERR_NO,
  ERR_CANT_OPEN,
  ERR_READ_ONLY,
  ERR_MAYBE_BROKEN,
  ERR_SEEK,
  ERR_WRITE,
  ERR_OVERFLOW,
  ERR_UNEXPECTED
};
static const t_menulabel data_err_drive[] =
{
  { { " OK ",                                                     " ��ǧ "                                                       } },
  { { "File in DRIVE %d: / can't open the file, or bad format.",  "�ɥ饤�� %d:���ե����뤬�����ʤ������ե�����������㤤�ޤ���" } },
  { { "File in DRIVE %d: / can't write the file.",                "�ɥ饤�� %d:�����Υե�����ˤϽ񤭹��ߤ��Ǥ��ޤ���"         } },
  { { "File in DRIVE %d: / maybe broken.",                        "�ɥ饤�� %d:���ե����뤬(¿ʬ)����Ƥ��ޤ���"                 } },
  { { "File in DRIVE %d: / SEEK Error.",                          "�ɥ饤�� %d:�����������顼��ȯ�����ޤ�����"                   } },
  { { "File in DRIVE %d: / WRITE Error.",                         "�ɥ饤�� %d:���񤭹��ߥ��顼��ȯ�����ޤ�����"                 } },
  { { "File in DRIVE %d: / strings too long.",                    "�ɥ饤�� %d:������ʸ����Ĺ�᤮�ޤ���"                       } },
  { { "File in DRIVE %d: / UNEXPECTED Error.",                    "�ɥ饤�� %d:��ͽ�����̥��顼��ȯ�����ޤ�����"                 } },   
};
static const t_menulabel data_err_file[] =
{
  { { " OK ",                           " ��ǧ "                                         } },
  { { "Error / can't open the file.",   "���顼���ե����뤬�����ޤ���"                 } },
  { { "Error / can't write the file.",  "���顼�����Υե�����ˤϽ񤭹��ߤ��Ǥ��ޤ���" } },
  { { "Error / maybe broken.",          "���顼���ե����뤬(¿ʬ)����Ƥ��ޤ���"         } },
  { { "Error / SEEK Error.",            "���顼�����������顼��ȯ�����ޤ�����"           } },
  { { "Error / WRITE Error.",           "���顼���񤭹��ߥ��顼��ȯ�����ޤ�����"         } },
  { { "Error / strings too long.",      "���顼������ʸ����Ĺ�᤮�ޤ���"               } },
  { { "Error / UNEXPECTED Error.",      "���顼��ͽ�����̥��顼��ȯ�����ޤ�����"         } },
};


/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/

static const char *help_jp[] =
{
  "  �ǥ��������᡼������Ѥ��륢�ץꥱ�������򥨥ߥ�졼�Ȥ����硢",
  "��SUB-CPU��ư�٤ȡ�FDC�������ȡ٤����꤬Ŭ�ڤǤʤ��ȡ������ư��",
  "���ʤ����Ȥ�����ޤ���",
  "",
  "��SUB-CPU��ư�٤ȡ�FDC�������ȡ٤�������ȹ礻�ϡ��ʲ��ΤȤ���Ǥ���",
  "",
  "           ���� �� SUB-CPU��ư  �� FDC�������ȡ�        ",
  "          �ݡݡݡܡݡݡݡݡݡݡݡܡݡݡݡݡݡݡܡݡݡݡ�",
  "            (1) �� 0  (-cpu 0)  ��    �ʤ�    ��  ��®  ",
  "            (2) �� 1  (-cpu 1)  ��    �ʤ�    ��   ��   ",
  "            (3) �� 1  (-cpu 1)  ��    ����    ��        ",
  "            (4) �� 2  (-cpu 2)  ��    �ʤ�    ��   ��   ",
  "            (5) �� 2  (-cpu 2)  ��    ����    ��  ����  ",
  "",
  "  ����(1) �ġ� �Ǥ��®�ǡ����ץꥱ������������ʬ��ư��ޤ���",
  "               �ǥե���Ȥ�����Ϥ���ˤʤ�ޤ���",
  "  ����(2) �ġ� ����®�Ǥ��������Υ��ץꥱ�������Ϥ�������Ǥʤ�",
  "               ��ư��ޤ���",
  "  ����(3) �ġ� �����®�Ǥ����ޤ�ˤ�������Ǥʤ���ư��ʤ����ץ�",
  "               ��������󤬤���ޤ�",
  "  ����(4)(5)�� �Ǥ���®�Ǥ�����������Ǥʤ���ư��ʤ����ץꥱ��",
  "               �����ϤۤȤ��̵���Ȼפ��ޤ���¿ʬ��",
  "",
  "  ����(1)�ǥ��ץꥱ�������ư��ʤ���硢����(2)��(3) �� ���Ѥ�",
  "�ƤߤƤ��������� �ޤ���ư��뤱��ɤ�ǥ������Υ�����������®�٤�",
  "�㲼���롢������ɤ����ڤ�롢�ʤɤξ���������Ѥ���Ȳ�������",
  "��ǽ��������ޤ���",
  0,
};

static const char *help_en[] =
{
  " I'm waiting for translator... ",
  0,
};

/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/

/* �����ܡ������� (������) �Υ������å������ѥǡ��� */

typedef struct{
  char	*str;		/* �����ȥåפ�ʸ�� or �ѥǥ����Ѷ���		*/
  int	code;		/* ����������       or  0			*/
} t_keymap;

static const t_keymap keymap_old0[] =
{
  { "STOP",       KEY88_STOP,    },
  { "COPY",       KEY88_COPY,    },
  { " ",          0,             },
  { "   f1   ",   KEY88_F1,      },
  { "   f2   ",   KEY88_F2,      },
  { "   f3   ",   KEY88_F3,      },
  { "   f4   ",   KEY88_F4,      },
  { "   f5   ",   KEY88_F5,      },
  { "   ",        0,             },
  { "R-UP",       KEY88_ROLLUP,  },
  { "R-DN",       KEY88_ROLLDOWN,},
  { "   ",        0,             },
  { " \036 ",     KEY88_UP,      },
  { " \037 ",     KEY88_DOWN,    },
  { " \035 ",     KEY88_LEFT,    },
  { " \034 ",     KEY88_RIGHT,   },
  { 0,0 },
};
static const t_keymap keymap_old1[] =
{
  { " ESC ", KEY88_ESC,       },
  { " 1 ",   KEY88_1,         },
  { " 2 ",   KEY88_2,         },
  { " 3 ",   KEY88_3,         },
  { " 4 ",   KEY88_4,         },
  { " 5 ",   KEY88_5,         },
  { " 6 ",   KEY88_6,         },
  { " 7 ",   KEY88_7,         },
  { " 8 ",   KEY88_8,         },
  { " 9 ",   KEY88_9,         },
  { " 0 ",   KEY88_0,         },
  { " - ",   KEY88_MINUS,     },
  { " ^ ",   KEY88_CARET,     },
  { " \\ ",  KEY88_YEN,       },
  { " BS ",  KEY88_INS_DEL,   },
  { "   ",   0,               },
  { "CLR",   KEY88_HOME,      },
  { "HLP",   KEY88_HELP,      },
  { " - ",   KEY88_KP_SUB,    },
  { " / ",   KEY88_KP_DIVIDE, },
  { 0,0 },
};
static const t_keymap keymap_old2[] =
{
  { "  TAB  ", KEY88_TAB,         },
  { " Q ",     KEY88_q,           },
  { " W ",     KEY88_w,           },
  { " E ",     KEY88_e,           },
  { " R ",     KEY88_r,           },
  { " T ",     KEY88_t,           },
  { " Y ",     KEY88_y,           },
  { " U ",     KEY88_u,           },
  { " I ",     KEY88_i,           },
  { " O ",     KEY88_o,           },
  { " P ",     KEY88_p,           },
  { " @ ",     KEY88_AT,          },
  { " [ ",     KEY88_BRACKETLEFT, },
  { "RETURN ", KEY88_RETURN,      },
  { "   ",     0,                 },
  { " 7 ",     KEY88_KP_7,        },
  { " 8 ",     KEY88_KP_8,        },
  { " 9 ",     KEY88_KP_9,        },
  { " * ",     KEY88_KP_MULTIPLY, },
  { 0,0 },
};
static const t_keymap keymap_old3[] =
{
  { "CTRL",      KEY88_CTRL,         },
  { "CAPS",      KEY88_CAPS,         },
  { " A ",       KEY88_a,            },
  { " S ",       KEY88_s,            },
  { " D ",       KEY88_d,            },
  { " F ",       KEY88_f,            },
  { " G ",       KEY88_g,            },
  { " H ",       KEY88_h,            },
  { " J ",       KEY88_j,            },
  { " K ",       KEY88_k,            },
  { " L ",       KEY88_l,            },
  { " ; ",       KEY88_SEMICOLON,    },
  { " : ",       KEY88_COLON,        },
  { " ] ",       KEY88_BRACKETRIGHT, },
  { "         ", 0,                  },
  { " 4 ",       KEY88_KP_4,         },
  { " 5 ",       KEY88_KP_5,         },
  { " 6 ",       KEY88_KP_6,         },
  { " + ",       KEY88_KP_ADD,       },
  { 0,0 },
};
static const t_keymap keymap_old4[] =
{
  { "    SHIFT   ", KEY88_SHIFT,      },
  { " Z ",          KEY88_z,          },
  { " X ",          KEY88_x,          },
  { " C ",          KEY88_c,          },
  { " V ",          KEY88_v,          },
  { " B ",          KEY88_b,          },
  { " N ",          KEY88_n,          },
  { " M ",          KEY88_m,          },
  { " , ",          KEY88_COMMA,      },
  { " . ",          KEY88_PERIOD,     },
  { " / ",          KEY88_SLASH,      },
  { " _ ",          KEY88_UNDERSCORE, },
  { " SHIFT ",      KEY88_SHIFT,      },
  { "   ",          0,                },
  { " 1 ",          KEY88_KP_1,       },
  { " 2 ",          KEY88_KP_2,       },
  { " 3 ",          KEY88_KP_3,       },
  { " = ",          KEY88_KP_EQUAL,   },
  { 0,0 },
};
static const t_keymap keymap_old5[] =
{
  { "       ",                                     0,               },
  { "KANA",                                        KEY88_KANA,      },
  { "GRPH",                                        KEY88_GRAPH,     },
  { "                                           ", KEY88_SPACE,     },
  { "                 ",                           0,               },
  { " 0 ",                                         KEY88_KP_0,      },
  { " , ",                                         KEY88_KP_COMMA,  },
  { " . ",                                         KEY88_KP_PERIOD, },
  { "RET",                                         KEY88_RETURN,    },
  { 0,0 },
};

static const t_keymap keymap_new0[] =
{
  { "STOP",   KEY88_STOP,     },
  { "COPY",   KEY88_COPY,     },
  { "  ",     0,              },
  { " f1 ",   KEY88_F1,       },
  { " f2 ",   KEY88_F2,       },
  { " f3 ",   KEY88_F3,       },
  { " f4 ",   KEY88_F4,       },
  { " f5 ",   KEY88_F5,       },
  { "   ",    0,              },
  { " f6 ",   KEY88_F6,       },
  { " f7 ",   KEY88_F7,       },
  { " f8 ",   KEY88_F8,       },
  { " f9 ",   KEY88_F9,       },
  { " f10 ",  KEY88_F10,      },
  { "   ",    0,              },
  { "ROLUP",  KEY88_ROLLUP,   },
  { "ROLDN",  KEY88_ROLLDOWN, },
  { 0,0 },
};
static const t_keymap keymap_new1[] =
{
  { " ESC ", KEY88_ESC,      },
  { " 1 ",   KEY88_1,        },
  { " 2 ",   KEY88_2,        },
  { " 3 ",   KEY88_3,        },
  { " 4 ",   KEY88_4,        },
  { " 5 ",   KEY88_5,        },
  { " 6 ",   KEY88_6,        },
  { " 7 ",   KEY88_7,        },
  { " 8 ",   KEY88_8,        },
  { " 9 ",   KEY88_9,        },
  { " 0 ",   KEY88_0,        },
  { " - ",   KEY88_MINUS,    },
  { " ^ ",   KEY88_CARET,    },
  { " \\ ",  KEY88_YEN,      },
  { " BS ",  KEY88_BS,       },
  { "   ",   0,              },
  { " DEL ", KEY88_DEL,      },
  { " INS ", KEY88_INS,      },
  { "  ",    0,              },
  { "CLR",  KEY88_HOME,      },
  { "HLP",  KEY88_HELP,      },
  { " - ",  KEY88_KP_SUB,    },
  { " / ",  KEY88_KP_DIVIDE, },
  { 0,0 },
};
static const t_keymap keymap_new2[] =
{
  { "  TAB  ",             KEY88_TAB,         },
  { " Q ",                 KEY88_q,           },
  { " W ",                 KEY88_w,           },
  { " E ",                 KEY88_e,           },
  { " R ",                 KEY88_r,           },
  { " T ",                 KEY88_t,           },
  { " Y ",                 KEY88_y,           },
  { " U ",                 KEY88_u,           },
  { " I ",                 KEY88_i,           },
  { " O ",                 KEY88_o,           },
  { " P ",                 KEY88_p,           },
  { " @ ",                 KEY88_AT,          },
  { " [ ",                 KEY88_BRACKETLEFT, },
  { "RETURN ",             KEY88_RETURNL,     },
  { "                   ", 0,                 },
  { " 7 ",                 KEY88_KP_7,        },
  { " 8 ",                 KEY88_KP_8,        },
  { " 9 ",                 KEY88_KP_9,        },
  { " * ",                 KEY88_KP_MULTIPLY, },
  { 0,0 },
};
static const t_keymap keymap_new3[] =
{
  { "CTRL",          KEY88_CTRL,         },
  { "CAPS",          KEY88_CAPS,         },
  { " A ",           KEY88_a,            },
  { " S ",           KEY88_s,            },
  { " D ",           KEY88_d,            },
  { " F ",           KEY88_f,            },
  { " G ",           KEY88_g,            },
  { " H ",           KEY88_h,            },
  { " J ",           KEY88_j,            },
  { " K ",           KEY88_k,            },
  { " L ",           KEY88_l,            },
  { " ; ",           KEY88_SEMICOLON,    },
  { " : ",           KEY88_COLON,        },
  { " ] ",           KEY88_BRACKETRIGHT, },
  { "             ", 0,                  },
  { " \036 ",        KEY88_UP,           },
  { "       ",       0,                  },
  { " 4 ",           KEY88_KP_4,         },
  { " 5 ",           KEY88_KP_5,         },
  { " 6 ",           KEY88_KP_6,         },
  { " + ",           KEY88_KP_ADD,       },
  { 0,0 },
};
static const t_keymap keymap_new4[] =
{
  { "    SHIFT   ", KEY88_SHIFTL,     },
  { " Z ",          KEY88_z,          },
  { " X ",          KEY88_x,          },
  { " C ",          KEY88_c,          },
  { " V ",          KEY88_v,          },
  { " B ",          KEY88_b,          },
  { " N ",          KEY88_n,          },
  { " M ",          KEY88_m,          },
  { " , ",          KEY88_COMMA,      },
  { " . ",          KEY88_PERIOD,     },
  { " / ",          KEY88_SLASH,      },
  { " _ ",          KEY88_UNDERSCORE, },
  { " SHIFT ",      KEY88_SHIFTR,     },
  { "  ",           0,                },
  { " \035 ",       KEY88_LEFT,       },
  { " \037 ",       KEY88_DOWN,       },
  { " \034 ",       KEY88_RIGHT,      },
  { "  ",           0,                },
  { " 1 ",          KEY88_KP_1,       },
  { " 2 ",          KEY88_KP_2,       },
  { " 3 ",          KEY88_KP_3,       },
  { " = ",          KEY88_KP_EQUAL,   },
  { 0,0 },
};
static const t_keymap keymap_new5[] =
{
  { "       ",                           0,              },
  { "KANA",                              KEY88_KANA,     },
  { "GRPH",                              KEY88_GRAPH,    },
  { " KETTEI ",                          KEY88_KETTEI,   },
  { "           ",                       KEY88_SPACE,    },
  { "  HENKAN  ",                        KEY88_HENKAN,   },
  { "PC ",                               KEY88_PC,       },
  { "ZEN",                               KEY88_ZENKAKU,  },
  { "                                 ", 0               },
  { " 0 ",                               KEY88_KP_0,     },
  { " , ",                               KEY88_KP_COMMA, },
  { " . ",                               KEY88_KP_PERIOD,},
  { "RET",                               KEY88_RETURNR,  },
  { 0,0 },
};

static const t_keymap * keymap_line[2][6] =
{
  {
    keymap_old0,
    keymap_old1,
    keymap_old2,
    keymap_old3,
    keymap_old4,
    keymap_old5,
  },
  {
    keymap_new0,
    keymap_new1,
    keymap_new2,
    keymap_new3,
    keymap_new4,
    keymap_new5,
  },
};


/* ���������ѹ�����ܥܥå����� �Υ������å������ѥǡ��� */

static const t_keymap keymap_assign[] =
{
  { "(none)",    KEY88_INVALID      },
  { "0 (10)",    KEY88_KP_0         },
  { "1 (10)",    KEY88_KP_1         },
  { "2 (10)",    KEY88_KP_2         },
  { "3 (10)",    KEY88_KP_3         },
  { "4 (10)",    KEY88_KP_4         },
  { "5 (10)",    KEY88_KP_5         },
  { "6 (10)",    KEY88_KP_6         },
  { "7 (10)",    KEY88_KP_7         },
  { "8 (10)",    KEY88_KP_8         },
  { "9 (10)",    KEY88_KP_9         },
  { "* (10)",    KEY88_KP_MULTIPLY  },
  { "+ (10)",    KEY88_KP_ADD       },
  { "= (10)",    KEY88_KP_EQUAL     },
  { ", (10)",    KEY88_KP_COMMA     },
  { ". (10)",    KEY88_KP_PERIOD    },
  { "- (10)",    KEY88_KP_SUB       },
  { "/ (10)",    KEY88_KP_DIVIDE    },
  { "A     ",    KEY88_a            },
  { "B     ",    KEY88_b            },
  { "C     ",    KEY88_c            },
  { "D     ",    KEY88_d            },
  { "E     ",    KEY88_e            },
  { "F     ",    KEY88_f            },
  { "G     ",    KEY88_g            },
  { "H     ",    KEY88_h            },
  { "I     ",    KEY88_i            },
  { "J     ",    KEY88_j            },
  { "K     ",    KEY88_k            },
  { "L     ",    KEY88_l            },
  { "M     ",    KEY88_m            },
  { "N     ",    KEY88_n            },
  { "O     ",    KEY88_o            },
  { "P     ",    KEY88_p            },
  { "Q     ",    KEY88_q            },
  { "R     ",    KEY88_r            },
  { "S     ",    KEY88_s            },
  { "T     ",    KEY88_t            },
  { "U     ",    KEY88_u            },
  { "V     ",    KEY88_v            },
  { "W     ",    KEY88_w            },
  { "X     ",    KEY88_x            },
  { "Y     ",    KEY88_y            },
  { "Z     ",    KEY88_z            },
  { "0     ",    KEY88_0            },
  { "1 (!) ",    KEY88_1            },
  { "2 (\") ",   KEY88_2            },
  { "3 (#) ",    KEY88_3            },
  { "4 ($) ",    KEY88_4            },
  { "5 (%) ",    KEY88_5            },
  { "6 (&) ",    KEY88_6            },
  { "7 (') ",    KEY88_7            },
  { "8 (() ",    KEY88_8            },
  { "9 ()) ",    KEY88_9            },
  { ", (<) ",    KEY88_COMMA        },
  { "- (=) ",    KEY88_MINUS        },
  { ". (>) ",    KEY88_PERIOD       },
  { "/ (?) ",    KEY88_SLASH        },
  { ": (*) ",    KEY88_COLON        },
  { "; (+) ",    KEY88_SEMICOLON    },
  { "@ (~) ",    KEY88_AT           },
  { "[ ({) ",    KEY88_BRACKETLEFT  },
  { "\\ (|) ",   KEY88_YEN          },
  { "] (}) ",    KEY88_BRACKETRIGHT },
  { "^     ",    KEY88_CARET        },
  { "  (_) ",    KEY88_UNDERSCORE   },
  { "space ",    KEY88_SPACE        },
  { "RETURN",    KEY88_RETURN       },
  { "SHIFT ",    KEY88_SHIFT        },
  { "CTRL  ",    KEY88_CTRL         },
  { "CAPS  ",    KEY88_CAPS         },
  { "kana  ",    KEY88_KANA         },
  { "GRPH  ",    KEY88_GRAPH        },
  { "HM-CLR",    KEY88_HOME         },
  { "HELP  ",    KEY88_HELP         },
  { "DELINS",    KEY88_INS_DEL      },
  { "STOP  ",    KEY88_STOP         },
  { "COPY  ",    KEY88_COPY         },
  { "ESC   ",    KEY88_ESC          },
  { "TAB   ",    KEY88_TAB          },
  { "\036     ", KEY88_UP           },
  { "\037     ", KEY88_DOWN         },
  { "\035     ", KEY88_LEFT         },
  { "\034     ", KEY88_RIGHT        },
  { "ROLLUP",    KEY88_ROLLUP       },
  { "ROLLDN",    KEY88_ROLLDOWN     },
  { "f1    ",    KEY88_F1           },
  { "f2    ",    KEY88_F2           },
  { "f3    ",    KEY88_F3           },
  { "f4    ",    KEY88_F4           },
  { "f5    ",    KEY88_F5           },
#if 1
  { "f6    ",    KEY88_F6           },
  { "f7    ",    KEY88_F7           },
  { "f8    ",    KEY88_F8           },
  { "f9    ",    KEY88_F9           },
  { "f10   ",    KEY88_F10          },
  { "BS    ",    KEY88_BS           },
  { "INS   ",    KEY88_INS          },
  { "DEL   ",    KEY88_DEL          },
  { "henkan",    KEY88_HENKAN       },
  { "kettei",    KEY88_KETTEI       },
  { "PC    ",    KEY88_PC           },
  { "zenkak",    KEY88_ZENKAKU      },
  { "RET  L",    KEY88_RETURNL      },
  { "RET  R",    KEY88_RETURNR      },
  { "SHIFTL",    KEY88_SHIFTL       },
  { "SHIFTR",    KEY88_SHIFTR       },
#endif
};




/************************************************************************/
/*									*/
/* ���� QUASI88 ��˥塼�� Tool Kit ��API�ϡ��ɤ�����������η����֤���	*/
/* ¿���ʤ�Τǡ������褦�ʽ�����ޤȤ᤿�ؿ����äƤߤ���		*/
/*									*/
/************************************************************************/

/* t_menulabel �� index ���ܤ�ʸ������������ޥ��� -------------------*/

#define		GET_LABEL(l, index)	(l[index].str[menu_lang])


/* �ե졼����������� --------------------------------------------------
				box	!= NULL �ʤ顢����� PACK ���롣
				label	�ե졼��Υ�٥�
				widget	!= NULL �ʤ顢�����褻�롣
*/
static	Q8tkWidget *PACK_FRAME(Q8tkWidget *box,
			       const char *label, Q8tkWidget *widget)
{
    Q8tkWidget *frame = q8tk_frame_new(label);

    if (widget)
	q8tk_container_add(frame, widget);

    q8tk_widget_show(frame);
    if (box)
	q8tk_box_pack_start(box, frame);

    return frame;
}


/* HBOX���������� ------------------------------------------------------
				box	!= NULL �ʤ顢����� PACK ���롣
*/
static	Q8tkWidget *PACK_HBOX(Q8tkWidget *box)
{
    Q8tkWidget *hbox = q8tk_hbox_new();

    q8tk_widget_show(hbox);
    if (box)
	q8tk_box_pack_start(box, hbox);

    return hbox;
}


/* VBOX���������� ------------------------------------------------------
				box	!= NULL �ʤ顢����� PACK ���롣
*/
static	Q8tkWidget *PACK_VBOX(Q8tkWidget *box)
{
    Q8tkWidget *vbox = q8tk_vbox_new();

    q8tk_widget_show(vbox);
    if (box)
	q8tk_box_pack_start(box, vbox);

    return vbox;
}


/* LABEL ���������� ----------------------------------------------------
				box	!= NULL �ʤ顢����� PACK ���롣
				label	��٥�
*/
static	Q8tkWidget *PACK_LABEL(Q8tkWidget *box, const char *label)
{
    Q8tkWidget *labelwidget = q8tk_label_new(label);

    q8tk_widget_show(labelwidget);
    if (box)
	q8tk_box_pack_start(box, labelwidget);

    return labelwidget;
}


/* VSEPATATOR ���������� -----------------------------------------------
				box	!= NULL �ʤ顢����� PACK ���롣
*/
static	Q8tkWidget *PACK_VSEP(Q8tkWidget *box)
{
    Q8tkWidget *vsep = q8tk_vseparator_new();

    q8tk_widget_show(vsep);
    if (box)
	q8tk_box_pack_start(box, vsep);

    return vsep;
}


/* HSEPATATOR ���������� -----------------------------------------------
				box	!= NULL �ʤ顢����� PACK ���롣
*/
static	Q8tkWidget *PACK_HSEP(Q8tkWidget *box)
{
    Q8tkWidget *hsep = q8tk_hseparator_new();

    q8tk_widget_show(hsep);
    if (box)
	q8tk_box_pack_start(box, hsep);

    return hsep;
}


/* �ܥ������������ ----------------------------------------------------
				box	!= NULL �ʤ顢����� PACK ���롣
				label	��٥�
				callback "clicked" ���Υ�����Хå��ؿ�
				parm	���Υѥ�᡼��
*/
static	Q8tkWidget *PACK_BUTTON(Q8tkWidget *box,
				const char *label,
				Q8tkSignalFunc callback, void *parm)
{
    Q8tkWidget *button = q8tk_button_new_with_label(label);

    q8tk_signal_connect(button, "clicked", callback, parm);
    q8tk_widget_show(button);
    if (box)
	q8tk_box_pack_start(box, button);

    return button;
}



/* �����å��ܥ������������ --------------------------------------------
				box	!= NULL �ʤ顢����� PACK ���롣
				label	��٥�
				on	���ʤ顢�����å����֤Ȥ���
				callback "clicked" ���Υ�����Хå��ؿ�
				parm	���Υѥ�᡼��
*/
static	Q8tkWidget *PACK_CHECK_BUTTON(Q8tkWidget *box,
				      const char *label, int on,
				      Q8tkSignalFunc callback, void *parm)
{
    Q8tkWidget *button = q8tk_check_button_new_with_label(label);

    if (on)
	q8tk_toggle_button_set_state(button, TRUE);

    q8tk_signal_connect(button, "toggled", callback, parm);
    q8tk_widget_show(button);
    if (box)
	q8tk_box_pack_start(box, button);

    return button;
}


/* �饸���ܥ������������ --------------------------------------------
				box	!= NULL �ʤ顢����� PACK ���롣
				button	���롼�פ��������ܥ���
				label	��٥�
				on	���ʤ顢�����å����֤Ȥ���
				callback "clicked" ���Υ�����Хå��ؿ�
				parm	���Υѥ�᡼��
*/
static	Q8tkWidget *PACK_RADIO_BUTTON(Q8tkWidget *box,
				      Q8tkWidget *button,
				      const char *label, int on,
				      Q8tkSignalFunc callback, void *parm)
{
    Q8tkWidget *b = q8tk_radio_button_new_with_label(button, label);

    q8tk_widget_show(b);
    q8tk_signal_connect(b, "clicked", callback, parm);

    if (on)
	q8tk_toggle_button_set_state(b, TRUE);

    if (box)
	q8tk_box_pack_start(box, b);

    return b;
}


/* ����ܥܥå������������� --------------------------------------------
				box	!= NULL �ʤ顢����� PACK ���롣
				p	t_menudata ���������Ƭ�ݥ��󥿡�
				count	��������ο���
					p[0] �� p[count-1] �ޤǤΥǡ�����
					ʸ����򥳥�ܥܥå��������롣
				initval	p[].val == initval �ξ�硢����
					ʸ�������ʸ����Ȥ��롣
				initstr	�嵭������̵�����ν��ʸ����
				width	ɽ����������0�Ǽ�ư
				act_callback "activate"���Υ�����Хå��ؿ�
				act_parm     ���Υѥ�᡼��
				chg_callback "changed"���Υ�����Хå��ؿ�
				chg_parm     ���Υѥ�᡼��
*/
static	Q8tkWidget *PACK_COMBO(Q8tkWidget *box,
			       const t_menudata *p, int count,
			       int initval, const char *initstr, int width,
			       Q8tkSignalFunc act_callback, void *act_parm,
			       Q8tkSignalFunc chg_callback, void *chg_parm)
{
    int i;
    Q8tkWidget *combo = q8tk_combo_new();

    for (i=0; i<count; i++, p++) {
	q8tk_combo_append_popdown_strings(combo, p->str[menu_lang], NULL);

	if (initval == p->val) initstr = p->str[menu_lang];
    }

    q8tk_combo_set_text(combo, initstr ? initstr : " ");
    q8tk_signal_connect(combo, "activate", act_callback, act_parm);
    if (chg_callback) {
	q8tk_combo_set_editable(combo, TRUE);
	q8tk_signal_connect(combo, "changed",  chg_callback, chg_parm);
    }
    q8tk_widget_show(combo);

    if (width)
	q8tk_misc_set_size(combo, width, 0);

    if (box)
	q8tk_box_pack_start(box, combo);

    return combo;
}


/* ����ȥ꡼���������� ------------------------------------------------
				box	!= NULL �ʤ顢����� PACK ���롣
				length	����ʸ����Ĺ��0��̵��
				width	ɽ��ʸ����Ĺ��0�Ǽ�ư
				text	���ʸ����
				act_callback "activate"���Υ�����Хå��ؿ�
				act_parm     ���Υѥ�᡼��
				chg_callback "changed"���Υ�����Хå��ؿ�
				chg_parm     ���Υѥ�᡼��
*/
static	Q8tkWidget *PACK_ENTRY(Q8tkWidget *box,
			       int length, int width, const char *text,
			       Q8tkSignalFunc act_callback, void *act_parm,
			       Q8tkSignalFunc chg_callback, void *chg_parm)
{
    Q8tkWidget *e;

    e = q8tk_entry_new_with_max_length(length);

    if (width)
	q8tk_misc_set_size(e, width, 1);

    if (text)
	q8tk_entry_set_text(e, text);

    if (act_callback)
	q8tk_signal_connect(e, "activate", act_callback, act_parm);
    if (chg_callback)
	q8tk_signal_connect(e, "changed",  chg_callback, chg_parm);

    q8tk_misc_set_placement(e, 0, Q8TK_PLACEMENT_Y_CENTER);
    q8tk_widget_show(e);

    if (box)
	q8tk_box_pack_start(box, e);

    return e;
}


/*======================================================================*/

/* �����å��ܥ����ʣ���������� ----------------------------------------
				box	����� PACK ���롣(NULL�϶ػ�)
				p	t_menudata ���������Ƭ�ݥ��󥿡�
				count	��������ο���
					p[0] �� p[count-1] �ޤǤΥǡ�����
					ʸ����ǥ����å��ܥ�����������롣
				f_initval �ؿ� (*f_initval)(p[].val) ����
					  �ʤ顢�����å����֤Ȥ���
				callback "toggled"���Υ�����Хå��ؿ�
					 �ѥ�᡼���� (void*)(p[].val)
*/
static	void	PACK_CHECK_BUTTONS(Q8tkWidget *box,
				   const t_menudata *p, int count,
				   int (*f_initval)(int),
				   Q8tkSignalFunc callback)
{
    int i;
    Q8tkWidget *button;

    for (i=0; i<count; i++, p++) {

	button = q8tk_check_button_new_with_label(p->str[menu_lang]);

	if ((*f_initval)(p->val))
	    q8tk_toggle_button_set_state(button, TRUE);

	q8tk_signal_connect(button, "toggled", callback, (void *)(p->val));

	q8tk_widget_show(button);
	q8tk_box_pack_start(box, button);
    }
}


/* �饸���ܥ����ʣ���������� ------------------------------------------
				box	����� PACK ���롣(NULL�϶ػ�)
				p	t_menudata ���������Ƭ�ݥ��󥿡�
				count	��������ο���
					p[0] �� p[count-1] �ޤǤΥǡ�����
					ʸ����ǥ饸���ܥ�����������롣
				initval p[].val == initval �ʤ�С�
					���Υܥ����ON���֤Ȥ���
				callback "clicked"���Υ�����Хå��ؿ�
					 �ѥ�᡼���� (void*)(p[].val)
*/
static	Q8List	*PACK_RADIO_BUTTONS(Q8tkWidget *box,
				    const t_menudata *p, int count,
				    int initval, Q8tkSignalFunc callback)
{
    int i;
    Q8tkWidget *button = NULL;

    for (i=0; i<count; i++, p++) {

	button = q8tk_radio_button_new_with_label(button, p->str[menu_lang]);

	q8tk_widget_show(button);
	q8tk_box_pack_start(box, button);
	q8tk_signal_connect(button, "clicked", callback, (void *)(p->val));

	if (initval == p->val) {
	    q8tk_toggle_button_set_state(button, TRUE);
	}
    }
    return q8tk_radio_button_get_list(button);
}


/* HSCALE���������� ----------------------------------------------------
				box	!= NULL �ʤ顢����� PACK ���롣
				p	t_volume ���������Ƭ�ݥ��󥿡�
					���ξ�����Ȥ�HSCALE��������
				initval �����
				callback "value_changed"���Υ�����Хå��ؿ�
				parm     ���Υѥ�᡼��
*/
static	Q8tkWidget *PACK_HSCALE(Q8tkWidget *box,
				const t_volume *p,
				int initval,
				Q8tkSignalFunc callback, void *parm)
{
    Q8tkWidget *adj, *scale;

    adj = q8tk_adjustment_new(initval,
			      p->min, p->max, p->step, p->page);

    q8tk_signal_connect(adj, "value_changed", callback, parm);

    scale = q8tk_hscale_new(adj);
    q8tk_adjustment_set_arrow(scale->stat.scale.adj, TRUE);
    /*q8tk_adjustment_set_length(scale->stat.scale.adj, 11);*/
    q8tk_scale_set_draw_value(scale, TRUE);
    q8tk_scale_set_value_pos(scale, Q8TK_POS_LEFT);

    q8tk_widget_show(scale);

    if (box)
	q8tk_box_pack_start(box, scale);

    return scale;
}



/* �������������ѹ��ѥ������åȤ��������� ------------------------------ */
static	Q8tkWidget *MAKE_KEY_COMBO(Q8tkWidget *box,
				   const t_menudata *p,
				   int (*f_initval)(int),
				   Q8tkSignalFunc callback)
{
    {
	Q8tkWidget *label = q8tk_label_new(GET_LABEL(p, 0));
	q8tk_box_pack_start(box, label);
	q8tk_widget_show(label);
    }
    {
	int i;
	const t_keymap *k = keymap_assign;
	const char     *initstr = " ";
	int             initval = (*f_initval)(p->val);

	Q8tkWidget *combo = q8tk_combo_new();

	for (i=0; i<COUNTOF(keymap_assign); i++, k++) {
	    q8tk_combo_append_popdown_strings(combo, k->str, NULL);

	    if (initval == k->code) initstr = k->str;
	}

	q8tk_combo_set_text(combo, initstr);
	q8tk_misc_set_size(combo, 6, 0);
	q8tk_signal_connect(combo, "activate", callback, (void*)(p->val));

	q8tk_box_pack_start(box, combo);
	q8tk_widget_show(combo);

	return combo;
    }
}

static	Q8tkWidget *PACK_KEY_ASSIGN(Q8tkWidget *box,
				    const t_menudata *p, int count,
				    int (*f_initval)(int),
				    Q8tkSignalFunc callback)
{
    int i;
    Q8tkWidget *vbox, *hbox, *allbox;

    vbox = q8tk_vbox_new();
    {
	{							/* combo */
	    hbox = q8tk_hbox_new();
	    {
		MAKE_KEY_COMBO(hbox, p, f_initval, callback);
		p++;
	    }
	    q8tk_widget_show(hbox);
	    q8tk_box_pack_start(vbox, hbox);
	}

	PACK_LABEL(vbox, GET_LABEL(p, 0));			/* �� */
	p++;

	{						/* combo �� �� combo */
	    hbox = q8tk_hbox_new();
	    {
		MAKE_KEY_COMBO(hbox, p, f_initval, callback);
		p++;
		MAKE_KEY_COMBO(hbox, p, f_initval, callback);
		p++;
	    }
	    q8tk_widget_show(hbox);
	    q8tk_box_pack_start(vbox, hbox);
	}

	PACK_LABEL(vbox, GET_LABEL(p, 0));			/* �� */
	p++;

	{							/* combo */
	    hbox = q8tk_hbox_new();
	    {
		MAKE_KEY_COMBO(hbox, p, f_initval, callback);
		p++;
	    }
	    q8tk_widget_show(hbox);
	    q8tk_box_pack_start(vbox, hbox);
	}
    }
    q8tk_widget_show(vbox);


    if (count < 6) {		/* �������������ǽ�������� */

	allbox = vbox;

    } else {			/* ¾�ˤ�������륭������ */

	allbox = q8tk_hbox_new();
	q8tk_box_pack_start(allbox, vbox);

	{
	    vbox = q8tk_vbox_new();
	    for (i=6; i<count; i++) {
		if (p->val < 0) {
		    PACK_LABEL(vbox, GET_LABEL(p, 0));
		    p++;
		} else {
		    hbox = q8tk_hbox_new();
		    {
			MAKE_KEY_COMBO(hbox, p, f_initval, callback);
			p++;
		    }
		    q8tk_widget_show(hbox);
		    q8tk_box_pack_start(vbox, hbox);
		}
	    }
	    q8tk_widget_show(vbox);
	    q8tk_box_pack_start(allbox, vbox);
	}

	q8tk_widget_show(allbox);
    }


    if (box)
	q8tk_box_pack_start(box, allbox);
    
    return allbox;
}


/*======================================================================*/

/* �ե����륻�쥯�������������� --------------------------------------
	�����塢������ɥ��򥰥�֤���
	CANCEL���ϡ��ʤˤ���������˥���֤�Υ����
	OK ���ϡ� (*ok_button)()��ƤӽФ������λ����ե�����̾��
	get_filename �ˡ��꡼�ɥ���꡼°���� get_ro �˥��åȤ���Ƥ��롣
	�ʤ����ƤӽФ��������ǤϤ��Ǥ˥���֤�Υ����
				label	��٥�
				select_ro >=0�ʤ顢ReadOnly�����(1�ǥ����å�)
				filename  ����ե�����(�ǥ��쥯�ȥ�)̾
				ok_button OK���˸ƤӽФ��ؿ�
				get_filename    OK���˥ե�����̾�򤳤�
						�Хåե��˥��å�
				sz_get_filename ���ΥХåե��Υ�����
				get_ro	select_ro �� >=0 �λ����꡼�ɥ���꡼
					������󤬤����˥��å�
*/
static struct{
    void	(*ok_button)(void);	/* OK�������θƤӽФ��ؿ�   */
    char	*get_filename;		/* ���򤷤��ե�����̾��Ǽ�� */
    int		sz_get_filename;	/* ���ΥХåե�������       */
    int		*get_ro;		/* RO ���ɤ����Υե饰      */
    Q8tkWidget	*accel;
} FSEL;
static void cb_fsel_ok(UNUSED_WIDGET, Q8tkWidget *f);
static void cb_fsel_cancel(UNUSED_WIDGET, Q8tkWidget *f);

static void START_FILE_SELECTION(const char *label,	/* �����ȥ�       */
				 int select_ro,		/* RO�������     */
				 const char *filename,	/* ����ե�����̾ */

				 void (*ok_button)(void),
				 char *get_filename,
				 int  sz_get_filename,
				 int  *get_ro)
{
    Q8tkWidget *f;

    f = q8tk_file_selection_new(label, select_ro);
    q8tk_widget_show(f);
    q8tk_grab_add(f);

    if (filename)
	q8tk_file_selection_set_filename(f, filename);

    q8tk_signal_connect(Q8TK_FILE_SELECTION(f)->ok_button,
			"clicked", cb_fsel_ok, f);
    q8tk_signal_connect(Q8TK_FILE_SELECTION(f)->cancel_button,
			"clicked", cb_fsel_cancel, f);
    q8tk_widget_set_focus(Q8TK_FILE_SELECTION(f)->cancel_button);

    FSEL.ok_button       = ok_button;
    FSEL.get_filename    = get_filename;
    FSEL.sz_get_filename = sz_get_filename;
    FSEL.get_ro          = (select_ro >= 0) ? get_ro : NULL;

    FSEL.accel = q8tk_accel_group_new();

    q8tk_accel_group_attach(FSEL.accel, f);
    q8tk_accel_group_add(FSEL.accel, Q8TK_KEY_ESC,
			 Q8TK_FILE_SELECTION(f)->cancel_button, "clicked");
}


static void cb_fsel_cancel(UNUSED_WIDGET, Q8tkWidget *f)
{
    q8tk_grab_remove(f);
    q8tk_widget_destroy(f);
    q8tk_widget_destroy(FSEL.accel);
}

static void cb_fsel_ok(UNUSED_WIDGET, Q8tkWidget *f)
{
    *FSEL.get_filename = '\0';
    strncat(FSEL.get_filename, q8tk_file_selection_get_filename(f), 
	    FSEL.sz_get_filename - 1);

    if (FSEL.get_ro)
	*FSEL.get_ro = q8tk_file_selection_get_readonly(f);

    q8tk_grab_remove(f);
    q8tk_widget_destroy(f);
    q8tk_widget_destroy(FSEL.accel);

    if (FSEL.ok_button)
	(*FSEL.ok_button)();
}


/*======================================================================*/

/* ������������������� ----------------------------------------------

	���������ϡ��ʲ��ι����Ȥ���
	+-----------------------------------+
	|               ���Ф� 1            |	���Ф���٥�   (1�İʾ�)
	|                 ��                |
	|               ���Ф� 2            |
	|                 ��                |
	|          [�����å��ܥ���]         |	�����å��ܥ��� (1�İʾ�)
	| --------------------------------- |	���ѥ졼��     (1�İʾ�)
	| [����ȥ�] [�ܥ���] �ġ� [�ܥ���] |	����ȥ�       (����1��)
	+-----------------------------------+	�ܥ���         (1�İʾ�)

	��٥롢���ѥ졼�����ܥ��󡢥���ȥ��������碌�ƺ���ǡ�
	DIA_MAX �Ĥޤǡ�
	�Ǹ���ɲä����������å� (�ܥ��󤫥���ȥ�) �˥ե������������롣
*/

#define	DIA_MAX		(12)

static	Q8tkWidget	*dialog[ DIA_MAX ];
static	Q8tkWidget	*dialog_main;
static	int		dialog_num;
static	Q8tkWidget	*dialog_entry;
static	Q8tkWidget	*dialog_accel;


/* ���������������� */

static	void	dialog_create(void)
{
    int i;
    Q8tkWidget *d = q8tk_dialog_new();
    Q8tkWidget *a = q8tk_accel_group_new();

    q8tk_misc_set_placement(Q8TK_DIALOG(d)->action_area,
			    Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_CENTER);

    q8tk_accel_group_attach(a, d);


    for (i=0; i<DIA_MAX; i++) dialog[ i ] = NULL;
    dialog_num   = 0;
    dialog_entry = NULL;

    dialog_accel = a;
    dialog_main  = d;
}

/* ���������˥�٥�ʸ��Ф��ˤ��ɲá�ʣ���ġ��ɲäǤ��� */

static	void	dialog_set_title(const char *label)
{
    Q8tkWidget *l = q8tk_label_new(label);

    if (dialog_num>=DIA_MAX) {fprintf(stderr, "%s %d\n", __FILE__, __LINE__);}

    q8tk_box_pack_start(Q8TK_DIALOG(dialog_main)->vbox, l);
    q8tk_widget_show(l);
    q8tk_misc_set_placement(l, Q8TK_PLACEMENT_X_CENTER, Q8TK_PLACEMENT_Y_TOP);

    dialog[ dialog_num ++ ] = l;
}

/* ���������˥����å��ܥ�����ɲ� (�����ĥܥ���̾,����,������Хå��ؿ�) */

static	void	dialog_set_check_button(const char *label, int on,
					Q8tkSignalFunc callback, void *parm)
{
    Q8tkWidget *b = q8tk_check_button_new_with_label(label);

    if (dialog_num>=DIA_MAX) {fprintf(stderr, "%s %d\n", __FILE__, __LINE__);}

    if (on)
	q8tk_toggle_button_set_state(b, TRUE);

    q8tk_box_pack_start(Q8TK_DIALOG(dialog_main)->vbox, b);
    q8tk_widget_show(b);
    q8tk_signal_connect(b, "toggled", callback, parm);

    dialog[ dialog_num ++ ] = b;
}

/* ���������˥��ѥ졼�����ɲá� */

static	void	dialog_set_separator(void)
{
    Q8tkWidget *s = q8tk_hseparator_new();

    if (dialog_num>=DIA_MAX) {fprintf(stderr, "%s %d\n", __FILE__, __LINE__);}

    q8tk_box_pack_start(Q8TK_DIALOG(dialog_main)->vbox, s);
    q8tk_widget_show(s);

    dialog[ dialog_num ++ ] = s;
}

/* ���������˥ܥ�����ɲ� (�����ĥܥ����̾��,������Хå��ؿ�) */

static	void	dialog_set_button(const char *label,
				  Q8tkSignalFunc callback, void *parm)
{
    Q8tkWidget *b = q8tk_button_new_with_label(label);

    if (dialog_num>=DIA_MAX) {fprintf(stderr, "%s %d\n", __FILE__, __LINE__);}

    q8tk_box_pack_start(Q8TK_DIALOG(dialog_main)->action_area, b);
    q8tk_widget_show(b);
    q8tk_signal_connect(b, "clicked", callback, parm);

    dialog[ dialog_num ++ ] = b;
}

/* ���������˥���ȥ���ɲ� (�����Ľ��ʸ����,����ʸ����,������Хå��ؿ�) */

static	void	dialog_set_entry(const char *text, int max_length,
				 Q8tkSignalFunc callback, void *parm)
{
    Q8tkWidget *e = q8tk_entry_new_with_max_length(max_length);

    q8tk_box_pack_start(Q8TK_DIALOG(dialog_main)->action_area, e);
    q8tk_widget_show(e);
    q8tk_signal_connect(e, "activate", callback, parm);
    q8tk_misc_set_size(e, max_length+1, 0);
    q8tk_misc_set_placement(e, 0, Q8TK_PLACEMENT_Y_CENTER);
    q8tk_entry_set_text(e, text);

    dialog_entry = e;

    dialog[ dialog_num ++ ] = e;
}

/* ����������Ρ�����ȥ��ʸ�����Ȥ�Ф� */

static	const	char	*dialog_get_entry(void)
{
    return q8tk_entry_get_text(dialog_entry);
}

/* ľ�����ɲä������������Υܥ���ˡ����硼�ȥ��åȥ��������� */

static	void	dialog_accel_key(int key)
{
    Q8tkWidget *w = dialog[ dialog_num-1 ];
    q8tk_accel_group_add(dialog_accel, key, w, "clicked");
}

/* ��������ɽ������ (����֤���롣�ե��������ϺǸ���ɲä����ܥ����) */

static	void	dialog_start(void)
{
    q8tk_widget_show(dialog_main);
    q8tk_grab_add(dialog_main);

    if (dialog[ dialog_num -1 ]) {
	q8tk_widget_set_focus(dialog[ dialog_num -1 ]);
    }
}

/* ����������õ� (������ɥ���õ������֤�������) */

static	void	dialog_destroy(void)
{
    int i;
    for (i=0; i<DIA_MAX; i++) {
	if (dialog[i]) {
	    q8tk_widget_destroy(dialog[i]);
	    dialog[i] = NULL;
	}
    }

    q8tk_grab_remove(dialog_main);
    q8tk_widget_destroy(dialog_main);
    q8tk_widget_destroy(dialog_accel);

    dialog_num   = 0;
    dialog_main  = NULL;
    dialog_entry = NULL;
    dialog_accel = NULL;
}
