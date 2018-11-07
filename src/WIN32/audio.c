/***********************************************************************
 * ������ɽ��Ͻ��� (�����ƥ��¸)
 *
 *      �ܺ٤ϡ� snddrv.h / mame-quasi88.h ����
 ************************************************************************/

#include "quasi88.h"
#include "device.h"

#include "snddrv.h"

#ifdef	USE_SOUND

#include "mame-quasi88.h"

	int g_pcm_bufsize = 100;	/* PCM �Хåե������� [ms] */
static	int use_audiodevice = 1;	/* use audio-devide for audio output */

static int device_opened = FALSE;	/* �ǥХ���������Ѥߤʤ鿿 */

/*===========================================================================*/
/*              QUASI88 ����ƤӽФ���롢MAME �ν����ؿ�                    */
/*===========================================================================*/

/******************************************************************************
 * ������ɷϥ��ץ��������ν��������λ�ؿ�
 *
 * int xmame_config_init(void)
 *      config_init() ��ꡢ���ץ����β��Ϥ򳫻Ϥ������˸ƤӽФ���롣
 *      MAME��¸����ν�����ʤɤ�ɬ�פʾ��ϡ������ǹԤ���
 *
 * void xmame_config_exit(void)
 *      config_exit() ��ꡢ�����κǸ�˸ƤӽФ���롣
 *      xmame_config_init() �ν����θ����դ���ɬ�פʤ顢�����ǹԤ���
 *
 *****************************************************************************/
int	xmame_config_init(void)
{
    return 0;		/*OSD_OK;*/
}
void	xmame_config_exit(void)
{
}


/******************************************************************************
 * ������ɷϥ��ץ����Υ��ץ����ơ��֥����
 *
 * const T_CONFIG_TABLE *xmame_config_get_opt_tbl(void)
 *      config_init() ��ꡢ xmame_config_init() �θ�˸ƤӽФ���롣
 *
 *      ������ɷϥ��ץ����β��Ͻ����ˤ����ơ� QUASI88 �Υ��ץ����ơ��֥�
 *      T_CONFIG_TABLE ����Ѥ����硢���Υݥ��󥿤��֤���
 *      �ȼ������ǲ��Ϥ�����ϡ� NULL ���֤���
 *****************************************************************************/
static	int	invalid_arg;			/* ̵���ʥ��ץ�����ѤΥ��ߡ��ѿ� */
static	const	T_CONFIG_TABLE xmame_options[] =
{
  /* 350��399: ������ɰ�¸���ץ���� */

  { 351, "sound",        X_FIX,  &use_sound,       TRUE,                  0,0, OPT_SAVE },
  { 351, "nosound",      X_FIX,  &use_sound,       FALSE,                 0,0, OPT_SAVE },

  { 352, "audio",        X_FIX,  &use_audiodevice, TRUE,                  0,0, OPT_SAVE },
  { 352, "noaudio",      X_FIX,  &use_audiodevice, FALSE,                 0,0, OPT_SAVE },

  { 353, "fmgen",        X_FIX,  &use_fmgen,       TRUE,                  0,0, OPT_SAVE },
  { 353, "nofmgen",      X_FIX,  &use_fmgen,       FALSE,                 0,0, OPT_SAVE },

  { 354, "volume",       X_INV,  &invalid_arg,                          0,0,0, 0        },

  { 355, "fmvol",        X_INT,  &fmvol,           0, 100,                  0, OPT_SAVE },
  { 356, "psgvol",       X_INT,  &psgvol,          0, 100,                  0, OPT_SAVE },
  { 357, "beepvol",      X_INT,  &beepvol,         0, 100,                  0, OPT_SAVE },
  { 358, "rhythmvol",    X_INT,  &rhythmvol,       0, 200,                  0, OPT_SAVE },
  { 359, "adpcmvol",     X_INT,  &adpcmvol,        0, 200,                  0, OPT_SAVE },
  { 360, "fmgenvol",     X_INT,  &fmgenvol,        0, 100,                  0, OPT_SAVE },
  { 361, "samplevol",    X_INT,  &samplevol,       0, 100,                  0, OPT_SAVE },

  { 362, "samplefreq",   X_INT,  &options.samplerate, 8000, 48000,          0, OPT_SAVE },

  { 363, "samples",      X_FIX,  &options.use_samples, 1,                 0,0, OPT_SAVE },
  { 363, "nosamples",    X_FIX,  &options.use_samples, 0,                 0,0, OPT_SAVE },

  { 364, "pcmbufsize",   X_INT,  &g_pcm_bufsize,   10, 1000,                0, OPT_SAVE },

  /* ��ü */
  {   0, NULL,           X_INV,                                       0,0,0,0, 0        },
};

const T_CONFIG_TABLE *xmame_config_get_opt_tbl(void)
{
    return xmame_options;
}


/******************************************************************************
 * ������ɷϥ��ץ����Υإ�ץ�å�������ɽ��
 *
 * void xmame_config_show_option(void)
 *      config_init() ��ꡢ���ץ���� -help �ν����κݤ˸ƤӽФ���롣
 *      ɸ����Ϥ˥إ�ץ�å�������ɽ�����롣
 *****************************************************************************/
#ifdef	XMAME_SNDDRV_071
#define	XMAME_VER "0.71.1"
#else	/* ver 0.106 */
#define	XMAME_VER " 0.106"
#endif

void	xmame_config_show_option(void)
{
  fprintf(stdout,
  "\n"
  "==========================================\n"
  "== SOUND OPTIONS ( dependent on XMAME ) ==\n"
  "==                     [ XMAME " XMAME_VER " ] ==\n"
  "==========================================\n"
  "    -[no]sound              Enable/disable sound (if available) [-sound]\n"
  "    -[no]fmgen              Use/don't use cisc's fmgen library\n"
  "                                               (if compiled in)  [-nofmgen]\n"
  "    -volume <n>             Set volume to <n> db, (-32 (soft) - 0(loud))\n"
  "                                                             <NOT SUPPORT>\n"
  "    -fmvol <level>          Set FM     level to <level> %%, (0 - 100) [100]\n"
  "    -psgvol <level>         Set PSG    level to <level> %%, (0 - 100) [20]\n"
  "    -beepvol <level>        Set BEEP   level to <level> %%, (0 - 100) [60]\n"
  "    -rhythmvol <level>      Set RHYTHM level to <level> %%, (0 - 100) [100]\n"
  "    -adpcmvol <level>       Set ADPCM  level to <level> %%, (0 - 100) [100]\n"
  "    -fmgenvol <level>       Set fmgen  level to <level> %%, (0 - 100) [100]\n"
  "    -samplevol <level>      Set SAMPLE level to <level> %%, (0 - 100) [100]\n"
  "    -samplefreq <rate>      Set the playback sample-frequency/rate [44100]\n"
  "    -[no]samples            Use/don't use samples (if available) [-nosamples]\n"
  "    -pcmbufsize <n>         Set sound-buffer-size to <n> ms (10 - 1000) [100]\n"
  );
}


/******************************************************************************
 * ������ɷϥ��ץ����β��Ͻ���
 *
 * int xmame_config_check_option(char *opt1, char *opt2, int priority)
 *      config_init() ��ꡢ����������ե�����β��Ϥ�Ԥʤ��ݤˡ�
 *      ����Υ��ץ����Τ�����ˤ���פ��ʤ���硢���δؿ����ƤӽФ���롣
 *
 *              opt1     �� �ǽ�ΰ���(ʸ����)
 *              opt2     �� ���Τΰ���(ʸ���� �ʤ��� NULL)
 *              priority �� ͥ���� (�ͤ��礭���ۤ�ͥ���٤��⤤)
 *
 *      �����  1  �� ��������������1�� (opt1 �Τ߽����� opt2 ��̤����)
 *              2  �� ��������������2�� (opt1 �� opt2 �����)
 *              0  �� opt1 ��̤�Τΰ����Τ��ᡢ opt1 opt2 �Ȥ��̤����
 *              -1 �� ��������̿Ū�ʰ۾郎ȯ��
 *
 *      ������ΰ۾� (�����λ����ͤ��ϰϳ��ʤ�) �䡢ͥ���٤ˤ������������å�
 *      ���줿�褦�ʾ��ϡ������������Ǥ�������Ʊ�ͤˡ� 1 �� 2 ���֤���
 *
 *      �� ���δؿ��ϡ��ȼ������ǥ��ץ�������Ϥ��뤿��δؿ��ʤΤǡ�
 *         ���ץ����ơ��֥� T_CONFIG_TABLE ����Ѥ�����ϡ����ߡ��Ǥ褤��
 *****************************************************************************/
int	xmame_config_check_option(char *opt1, char *opt2, int priority)
{
    return 0;
}


/******************************************************************************
 * ������ɷϥ��ץ�������¸���뤿��δؿ�
 *
 * int  xmame_config_save_option(void (*real_write)
 *                                 (const char *opt_name, const char *opt_arg))
 *
 *      ����ե��������¸�κݤˡ��ƤӽФ���롣
 *              ��opt_name �˥��ץ����� opt_arg �˥��ץ���������
 *                ���åȤ���real_write ��ƤӽФ���
 *              �Ȥ���ư�����¸�����������ץ������Ф��Ʒ����֤��Ԥʤ���
 *
 *              (��) "-sound" ������ե��������¸���������
 *                      (real_write)("sound", NULL) ��ƤӽФ���
 *              (��) "-fmvol 80" ������ե��������¸���������
 *                      (real_write)("fmvol", "80") ��ƤӽФ���
 *
 *      �����  ��� 0 ���֤�
 *
 *      �� ���δؿ��ϡ��ȼ������ǥ��ץ�������Ϥ��뤿��δؿ��ʤΤǡ�
 *         ���ץ����ơ��֥� T_CONFIG_TABLE ����Ѥ�����ϡ����ߡ��Ǥ褤��
 *****************************************************************************/
int	xmame_config_save_option(void (*real_write)
				   (const char *opt_name, const char *opt_arg))
{
    return 0;
}


/******************************************************************************
 * ������ɷϥ��ץ������˥塼�����ѹ����뤿��Υơ��֥�����ؿ�
 *
 * T_SNDDRV_CONFIG *xmame_config_get_sndopt_tbl(void)
 *
 *      ��˥塼�⡼�ɤγ��ϻ��˸ƤӽФ���롣
 *              �ѹ���ǽ�ʥ�����ɷϥ��ץ����ξ����T_SNDDRV_CONFIG ����
 *              ����Ȥ����Ѱդ���������Ƭ�ݥ��󥿤��֤���
 *              ����Ϻ���5�Ĥޤǡ�����������ˤϽ�ü�ǡ����򥻥åȤ��Ƥ�����
 *
 *              �ä��ѹ����������Ǥ����Τ�̵������ NULL ���֤���
 *****************************************************************************/
T_SNDDRV_CONFIG *xmame_config_get_sndopt_tbl(void)
{
    static T_SNDDRV_CONFIG config[] =
    {
	{
	    SNDDRV_INT,
#if 0
	    " Buffer size of PCM data (10 - 1000[ms])    ",
#else
	    " PCM �Хåե��Υ�����  (10 - 1000[ms])      ",
#endif
	    &g_pcm_bufsize,  10, 1000,
	},
	{
	    SNDDRV_NULL, 0, 0, 0, 0,
	},
    };

    if (use_audiodevice) {
	return config;
    } else {
	return NULL;
    }
}


/******************************************************************************
 * ������ɵ�ǽ�ξ�����������ؿ�
 *
 * int xmame_has_audiodevice(void)
 *      ������ɥ��ǥХ������Ϥβ��ݤ��֤���
 *      ���ʤ�ǥХ������ϲġ����ʤ��Բġ�
 *
 * int xmame_has_mastervolume(void)
 *      ������ɥǥХ����β��̤��ѹ���ǽ���ɤ������֤���
 *      ���ʤ��ѹ���ǽ�����ʤ��Բġ�
 *
 *****************************************************************************/
int	xmame_has_audiodevice(void)
{
    if (use_sound) {
	if (device_opened) return TRUE;
    }
    return FALSE;
}

int	xmame_has_mastervolume(void)
{
    return FALSE;
}


/*===========================================================================*/
/*              MAME �ν����ؿ�����ƤӽФ���롢�����ƥ��¸�����ؿ�        */
/*===========================================================================*/

/******************************************************************************
 * ������ɥǥХ�������
 *      �����δؿ��ϡ������Х��ѿ� use_sound �����ξ��ϡ��ƤӽФ���ʤ���
 *
 * int osd_start_audio_stream(int stereo)
 *      ������ɥǥХ������������롣
 *          stereo �����ʤ饹�ƥ쥪���ϡ����ʤ��Υ����Ϥǽ�������롣
 *          (��Υ����Ϥϡ��Ť��С������� MAME/XMAME �� YM2203 ���������
 *           ���Τߡ����Ѥ��Ƥ��롣����ʳ��Ϥ��٤ƥ��ƥ쥪���Ϥ���Ѥ��롣)
 *      ���δؿ��ϡ����ߥ�졼�����γ��ϻ��˸ƤӽФ���롣
 *          �������ϡ�1�ե졼�ढ����Υ���ץ�����֤���
 *          ���Ի��ϡ�0 ���֤���
 *              ���������� 0 ���֤��� QUASI88 ��������λ���Ƥ��ޤ��Τǡ�
 *              ������ɥǥХ����ν�����˼��Ԥ������Ǥ⡢������ɽ��Ϥʤ���
 *              ������ʤ᤿���Ȥ�����硢�Ȥˤ���Ŭ�����ͤ��֤�ɬ�פ����롣
 *
 * int osd_update_audio_stream(INT16 *buffer)
 *      ������ɥǥХ����˽��Ϥ��롣
 *      ���δؿ��ϡ�1�ե졼�������˸ƤӽФ���롣buffer �ˤ�1�ե졼��ʬ
 *      (Machine->sample_rate / Machine->drv->frames_per_second) �Υ������
 *      �ǡ�������Ǽ����Ƥ��롣�ǡ����� 16bit����դ��ǡ����ƥ쥪�ξ���
 *      ���ȱ��Υ���ץ뤬��ߤ��¤�Ǥ��롣
 *
 *      �ºݤˤ��δؿ����ƤӽФ��줿�����ߥ󥰤ǥǥХ����˽��Ϥ��뤫�����뤤��
 *      �����ǥХåե���󥰤������ӽ��Ϥ��뤫�ϡ���������Ǥ��롣
 *
 *      ���ͤϡ� osd_start_audio_stream() ��Ʊ��
 *
 * void osd_stop_audio_stream(void)
 *      ������ɥǥХ�����λ���롣
 *      ���δؿ��ϡ����ߥ�졼�����ν�λ���˸ƤӽФ���롣
 *      �ʹߡ� osd_update_audio_stream() �ʤɤϸƤӽФ���ʤ������ߥ�졼�����
 *      ��Ƴ�������ϡ� osd_start_audio_stream() ����ƤӽФ���롣
 *
 * void osd_update_video_and_audio(void)
 *      ������ɥǥХ����ν��Ϥ��Σ�
 *      �����ߥ�Ū�ˤϡ� osd_update_audio_stream() ��ľ��˸ƤӽФ���롣
 *      XMAME 0.106 �˹�碌���������Ƥ��뤬�� osd_update_audio_stream() ��
 *      ������ȼ����Ǥ��줤��С�������δؿ��ϥ��ߡ��Ǥ褤��
 *
 * void osd_sound_enable(int enable)
 *      ������ɥǥХ����ؤν��Ϥ��ꡦ�ʤ������ꤹ�롣
 *          enable �����ʤ���Ϥ��ꡢ���ʤ���Ϥʤ���
 *      ���δؿ��ϡ������Х��ѿ� close_device �����ξ��Τߡ��ƤӽФ���롣
 *          ��˥塼�⡼�ɤ����ä��ݤˡ������򵶤Ȥ��ƸƤӽФ���
 *          ��˥塼�⡼�ɤ���Ф�ݤˡ������򿿤Ȥ��ƸƤӽФ���
 *      ���δؿ��ϡ����������ξ��ˡ�������ɥǥХ�������� (close) ����
 *      ���ξ��˳��� (open) ����褦�ʼ�������Ԥ��Ƥ��뤬��������ɥǥХ���
 *      �������ݤ����ޤޤˤ���褦�ʼ����Ǥ���С����ߡ��δؿ��Ǥ褤��
 *      �ʤ���������ɥǥХ����ؤν��Ϥʤ��ξ��� osd_update_audio_stream() 
 *      �ʤɤδؿ��ϸƤӽФ���롣
 *
 *****************************************************************************/

static	int	create_sound_device(int stereo);
static	void	destroy_sound_device(void);
static	void	write_sound_device(unsigned char *data, int count);

static int samples_per_frame = 0;


int	osd_start_audio_stream(int stereo)
{
    if (use_audiodevice) {
	if (!(device_opened = create_sound_device(stereo))) {
	    /* �ǥХ����������ʤ��Ƥ⡢���ˤ���³�� */
	}
    } else {
	device_opened = FALSE;
    }

    /* calculate samples_per_frame */
#ifdef	XMAME_SNDDRV_071
    samples_per_frame =(int)(Machine->sample_rate / Machine->drv->frames_per_second);
#else	/* ver 0.106 */
    samples_per_frame =(int)(Machine->sample_rate / Machine->refresh_rate);
#endif

    return samples_per_frame;
}

int	osd_update_audio_stream(INT16 *buffer)
{
    if (device_opened) {
	write_sound_device((unsigned char *)buffer, samples_per_frame);
    }

    return samples_per_frame;
}

void	osd_stop_audio_stream(void)
{
    if (device_opened) {
	destroy_sound_device();
	device_opened = FALSE;
    }
}

void	osd_update_video_and_audio(void)
{
    /* DO NOTHING */
}

void	osd_sound_enable(int enable)
{
    /* DO NOTHING */
}


/******************************************************************************
 * ��������
 *
 * void osd_set_mastervolume(int attenuation)
 *      ������ɥǥХ����β��̤����ꤹ�롣 attenuation �� ���̤ǡ� -32��0 
 *      (ñ�̤� db)�� �����ѹ��ΤǤ��ʤ��ǥХ����Ǥ���С����ߡ��Ǥ褤��
 *
 * int osd_get_mastervolume(void)
 *      ���ߤΥ�����ɥǥХ����β��̤�������롣 ���ͤ� -32��0 (ñ�̤� db)��
 *      �����ѹ��ΤǤ��ʤ��ǥХ����Ǥ���С����ߡ��Ǥ褤��
 *
 *****************************************************************************/
void	osd_set_mastervolume(int attenuation)
{
    /* waveOutSetVolume */
}

int	osd_get_mastervolume(void)
{
    /* waveOutGetVolume */

    return VOL_MIN;
}


/*===========================================================================*/
/*									     */
/*===========================================================================*/
/* #define	USE_WAVE_OUT_PROC */

#ifdef	USE_WAVE_OUT_PROC
static void CALLBACK waveOutProc(HWAVEOUT hwo , UINT msg,         
				 DWORD dwInstance,  
				 DWORD dwParam1, DWORD dwParam2);
#endif

static HWAVEOUT hWaveOut;		/* �ǥХ��������ѥϥ�ɥ� */

#define	BUFFER_NUM	(2)		/* ���֥�Хåե��ǽ�ʬ�餷�� */
static WAVEHDR whdr[BUFFER_NUM];

static int byte_per_sample = 4;		/* 1����ץ뤢����ΥХ��ȿ�	*/
					/* (�Хåե��������׻����η���)	*/
					/* 4 = Stereo, 16bit		*/
					/* 2 = Mono,   16bit		*/

/*
  PCM�ǡ����Υ��塼�����ϡ� XMAME 0.106 �򻲹ͤˤ��ޤ�����

  ���ͥ����� xmame-0.106/src/unix/sysdep/dsp-drivers/sdl.c
  Copyright 2001 Jack Burton aka Stefano Ceccherini
*/
  
static struct {
    unsigned char	*data;
    int			dataSize;
    int			amountRemain;
    int			amountWrite;
    int			amountRead;
    int			tmp;
    unsigned long	soundlen;
    int			sound_n_pos;
    int			sound_w_pos;
    int			sound_r_pos;
} sample; 



static	int	create_sound_device(int stereo)
{
    int i, err, bufsize;
    WAVEFORMATEX f;

    if (stereo) byte_per_sample = 4;
    else        byte_per_sample = 2;

    /* PCM�η��������� */
    memset(&f, 0, sizeof(f));

    f.wFormatTag	= WAVE_FORMAT_PCM;		/* PCM����	*/
    f.nChannels		= (stereo) ? 2 : 1;		/* stereo/mono	*/
    f.wBitsPerSample	= 16;				/* �̻Ҳ� bit��	*/
    f.nSamplesPerSec	= Machine->sample_rate;		/* ɸ�ܲ����ȿ�	*/
    f.nBlockAlign	= f.nChannels * f.wBitsPerSample / 8;
    f.nAvgBytesPerSec	= f.nSamplesPerSec * f.nBlockAlign;
    f.cbSize		= 0;

    /* �ǥХ����򳫤� */
    memset(&hWaveOut, 0, sizeof(hWaveOut));

    if (waveOutOpen(&hWaveOut,		/* �����ˡ��ϥ�ɥ뤬�֤����	     */
		    WAVE_MAPPER,	/* �ǥХ����ϡ��桼������Τ�Τ����*/
		    &f,			/* PCM�η���			     */
#ifdef	USE_WAVE_OUT_PROC
		    (DWORD)waveOutProc,	/* ������Хå��ؿ�		     */
		    0,			/* ������Хå��ؿ��ΰ����ǡ���	     */
		    CALLBACK_FUNCTION	/* ������Хå��ؿ������	     */
#else
		    (DWORD)g_hWnd,	/* ������ɥ��ϥ�ɥ�		     */
		    0,			/* ̤����			     */
		    CALLBACK_WINDOW	/* ������ɥ��ϥ�ɥ�����	     */
#endif
				) != MMSYSERR_NOERROR) {

	fprintf(stderr, "failed opening audio device\n");
	return FALSE;

	/* �������ϡ� MM_WOM_OPEN / WOM_OPEN ��ȯ������ */
    }

    /* WAV�Хåե������� */
    memset(&whdr, 0, sizeof(whdr));

    err = FALSE;
    bufsize = Machine->sample_rate * byte_per_sample * g_pcm_bufsize / 1000;
    /* �Хåե��������λ�������� (2�Τ٤���Ȥ�������ͤȤ�) ��̵���Ρ� */
    for (i=0; i<BUFFER_NUM; i++) {
	whdr[i].lpData		= (LPSTR)calloc(1, bufsize);
	whdr[i].dwBufferLength	= bufsize;
	whdr[i].dwFlags		= WHDR_BEGINLOOP | WHDR_ENDLOOP;
	whdr[i].dwLoops		= 1;
	whdr[i].dwUser		= FALSE;    /* FALSE = Started, Need to Write*/

	if (whdr[i].lpData == NULL) { err = TRUE; }
    }

    /* WAV�Хåե��˥ǡ��������뤿��Ρ���֥Хåե����������� */
    /* ��֥Хåե��ϡ�WAV�Хåե������礭���ʤ��Ȥ����ʤ�   */
    if (err == FALSE) {
	memset(&sample, 0, sizeof(sample));

	sample.dataSize = bufsize * 4;	/* �Хåե���4�ܤ���ݡ������ͤǤ���?*/
	if (!(sample.data = calloc(1, sample.dataSize))) {
	    err = TRUE;
	}
    }

    /* �����ޤǤ˥��顼���ФƤ��顢��λ */
    if (err){
	for (i=0; i<BUFFER_NUM; i++) {
	    if (whdr[i].lpData) { free(whdr[i].lpData); }
	}
	waveOutClose(hWaveOut);
	return FALSE;
    }


    if (verbose_proc)
	printf("  waveOutOpen=16bit, %s, %dHz : buf-size=%d\n",
	       (stereo) ? "stereo" : "mono",
	       f.nSamplesPerSec, bufsize / byte_per_sample);

    /* WAV�Хåե� (����̵��) ����� */
    for (i=0; i<BUFFER_NUM; i++) {
	if (waveOutPrepareHeader(hWaveOut, &whdr[i], sizeof(WAVEHDR))
							== MMSYSERR_NOERROR) {

	    waveOutWrite(hWaveOut, &whdr[i], sizeof(WAVEHDR));
	    /* BUFFER_NUM �󡢽�˺�������뤬�����Τ��κ�����λ����
	       MM_WOM_DONE / WOM_DONE ��ȯ������ */
	}
    }

    return TRUE;
}

static	void	destroy_sound_device(void)
{
    int i;

    for (i=0; i<BUFFER_NUM; i++) {
	whdr[i].dwUser = TRUE;		    /* TRUE = Stopped, Never Write */
    }
    waveOutReset(hWaveOut);
    /* BUFFER_NUM �� MM_WOM_DONE / WOM_DONE ��ȯ������ */

    for (i=0; i<BUFFER_NUM; i++) {
	waveOutUnprepareHeader(hWaveOut, &whdr[i], sizeof(WAVEHDR));
    }
    waveOutClose(hWaveOut);
    /* MM_WOM_CLOSE / WOM_CLOSE ��ȯ������ */

    for (i=0; i<BUFFER_NUM; i++) {
	free(whdr[i].lpData);
    }

    if (sample.data) {
	free(sample.data);
    }
    memset(&sample, 0, sizeof(sample));
    sample.data = NULL;
}

/*
 * ��֥Хåե��ˡ�PCM�ǡ����򥻥åȤ��롣
 * (������ɥǥХ����˽��Ϥ���櫓�ǤϤʤ�)
 * ���δؿ��ϡ�QUASI88��ꡢ1�ե졼����˸ƤӽФ����(�Ϥ�)
 *
 * ���� sdl_dsp_write()
 */
static	void	write_sound_device(unsigned char *data, int count)
{
    /* sound_n_pos = normal position
       sound_r_pos = read position
       and so on.			*/
    unsigned char *src;
    int bytes_written = 0;
    /* Lock */

    sample.amountRemain = sample.dataSize - sample.sound_n_pos;
    sample.amountWrite = count * byte_per_sample;

    if(sample.amountRemain <= 0) {
	/* Unlock */
	return;
    }

    if(sample.amountRemain < sample.amountWrite) sample.amountWrite = sample.amountRemain;
    sample.sound_n_pos += sample.amountWrite;
		
    src = (unsigned char *)data;
    sample.tmp = sample.dataSize - sample.sound_w_pos;
		
    if(sample.tmp < sample.amountWrite){
	memcpy(sample.data + sample.sound_w_pos, src, sample.tmp);
	bytes_written += sample.tmp;
	sample.amountWrite -= sample.tmp;
	src += sample.tmp;
	memcpy(sample.data, src, sample.amountWrite);			
	bytes_written += sample.amountWrite;
	sample.sound_w_pos = sample.amountWrite;
    }
    else{
	memcpy( sample.data + sample.sound_w_pos, src, sample.amountWrite);
	bytes_written += sample.amountWrite;
	sample.sound_w_pos += sample.amountWrite;
    }
    /* Unlock */
		
    return;
}

/*
 * ��֥Хåե��� PCM �ǡ����򡢥ǥХ����˽��Ϥ��롣
 *
 *
 * ���� sdl_fill_sound()
 */
#ifdef	USE_WAVE_OUT_PROC
/* waveOutProc �������ǡ� WaveOut��API��ƤӽФ��ΤϤ���餷����
   �Ȥ������Ȥϡ����δؿ��ϻȤ��ʤ�?  WndProc ���������ƤӽФ�? */
static void CALLBACK waveOutProc(HWAVEOUT hwo,
				 UINT msg,
				 DWORD dwInstance,
				 DWORD dwParam1, DWORD dwParam2)
{
    /* hwo      == hWaveOut                          */
    /* dwParam1 == whdr[n]  ���٥��ȯ�����ΥХåե� */

    switch (msg) {
    case WOM_OPEN:
	wave_event_open(hwo);
	break;

    case WOM_DONE:
	wave_event_done(hwo, (LPWAVEHDR)dwParam1);
	break;

    case WOM_CLOSE:
	wave_event_close(hwo);
	break;
    }
}
#endif

/*
 * MM_WOM_OPEN / WOM_OPEN ȯ�����˸ƤӽФ��ؿ�
 */
void	wave_event_open(HWAVEOUT hwo)
{
    /* printf("S:open\n"); */
}

/*
 * MM_WOM_DONE / WOM_DONW ȯ�����˸ƤӽФ��ؿ�
 */
void	wave_event_done(HWAVEOUT hwo, LPWAVEHDR lpwhdr)
{
    if (lpwhdr->dwUser) {		    /* TRUE = Stopped, Never Write */
	/* printf("S:break\n"); */
	/* DO NOTHING */
    } else {				    /* FALSE = Started, Need to Write*/

	int result;
	unsigned char *dst = (unsigned char *) (lpwhdr->lpData);
	int len = lpwhdr->dwBufferLength;

	/* int i; for (i=0; i<BUFFER_NUM; i++) if (lpwhdr == &whdr[i]) break;*/
	/* printf("S:done %d\n",i); */

	sample.amountRead = len;
	if(sample.sound_n_pos <= 0) {
	    /* ��֥Хåե������ʤ顢���褦���ʤ��Τ�̵����񤭹��� */
	    memset(lpwhdr->lpData, 0, len);
/*
	    fprintf(debugfp, "sound empty\n");
*/
	} else {
	    if(sample.sound_n_pos<sample.amountRead) sample.amountRead = sample.sound_n_pos;
	    result = (int)sample.amountRead;
	    sample.sound_n_pos -= sample.amountRead;

	    sample.tmp = sample.dataSize - sample.sound_r_pos;
	    if(sample.tmp<sample.amountRead){
		memcpy( dst, sample.data + sample.sound_r_pos, sample.tmp);
		sample.amountRead -= sample.tmp;
		dst += sample.tmp;
		memcpy( dst, sample.data, sample.amountRead);	
		sample.sound_r_pos = sample.amountRead;
	    }
	    else{
		memcpy( dst, sample.data + sample.sound_r_pos, sample.amountRead);
		sample.sound_r_pos += sample.amountRead;
	    }
	}

	if (waveOutPrepareHeader(hWaveOut, lpwhdr, sizeof(WAVEHDR))
							== MMSYSERR_NOERROR) {
	    waveOutWrite(hWaveOut, lpwhdr, sizeof(WAVEHDR));
	    /* �� ������� MM_WOM_DONE / WOM_DONE ��ȯ������ */
	}
    }
}

/*
 * MM_WOM_CLOSE / WOM_CLOSE ȯ�����˸ƤӽФ��ؿ�
 */
void	wave_event_close(HWAVEOUT hwo)
{
    /* printf("S:close\n"); */
}

#endif	/* USE_SOUND */
