/***********************************************************************
 * ������ɽ��Ͻ��� (�����ƥ��¸)
 *
 *      �ܺ٤ϡ� snddrv.h / mame-quasi88.h ����
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef	USE_SOUND

#include "mame-quasi88.h"

#define  SNDDRV_WORK_DEFINE
#include "audio.h"



/*---------------------------------------------------------------*/
static int use_audiodevice = 1;		/* use audio-devide for audio output */
/* static int attenuation = 0;		 * �ܥ�塼�� -32��0 [db] ����̤���ݡ��� */

static int sound_enabled;			/* �����ǥ����ǥХ����򳫤����顢�� */

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
int		xmame_config_init(void)
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

static	const	T_CONFIG_TABLE xmame_options[] =
{
  /* 350��399: ������ɰ�¸���ץ���� */

  { 351, "sound",        X_FIX,  &use_sound,       TRUE,                  0,0, OPT_SAVE },
  { 351, "snd",          X_FIX,  &use_sound,       TRUE,                  0,0, 0        },
  { 351, "nosound",      X_FIX,  &use_sound,       FALSE,                 0,0, OPT_SAVE },
  { 351, "nosnd",        X_FIX,  &use_sound,       FALSE,                 0,0, 0        },

  { 352, "audio",        X_FIX,  &use_audiodevice, TRUE,                  0,0, OPT_SAVE },
  { 352, "ao",           X_FIX,  &use_audiodevice, TRUE,                  0,0, 0        },
  { 352, "noaudio",      X_FIX,  &use_audiodevice, FALSE,                 0,0, OPT_SAVE },
  { 352, "noao",         X_FIX,  &use_audiodevice, FALSE,                 0,0, 0        },

  { 353, "fmgen",        X_FIX,  &use_fmgen,       TRUE,                  0,0, OPT_SAVE },
  { 353, "nofmgen",      X_FIX,  &use_fmgen,       FALSE,                 0,0, OPT_SAVE },

/*{ 354, "volume",       X_INT,  &attenuation,     -32, 0,                  0, OPT_SAVE },*/
/*{ 354, "v",            X_INT,  &attenuation,     -32, 0,                  0, 0        },*/

  { 355, "fmvol",        X_INT,  &fmvol,           0, 100,                  0, OPT_SAVE },
  { 355, "fv",           X_INT,  &fmvol,           0, 100,                  0, 0        },
  { 356, "psgvol",       X_INT,  &psgvol,          0, 100,                  0, OPT_SAVE },
  { 356, "pv",           X_INT,  &psgvol,          0, 100,                  0, 0        },
  { 357, "beepvol",      X_INT,  &beepvol,         0, 100,                  0, OPT_SAVE },
  { 357, "bv",           X_INT,  &beepvol,         0, 100,                  0, 0        },
  { 358, "rhythmvol",    X_INT,  &rhythmvol,       0, 200,                  0, OPT_SAVE },
  { 358, "rv",           X_INT,  &rhythmvol,       0, 200,                  0, 0        },
  { 359, "adpcmvol",     X_INT,  &adpcmvol,        0, 200,                  0, OPT_SAVE },
  { 359, "av",           X_INT,  &adpcmvol,        0, 200,                  0, 0        },
  { 360, "fmgenvol",     X_INT,  &fmgenvol,        0, 100,                  0, OPT_SAVE },
  { 360, "fmv",          X_INT,  &fmgenvol,        0, 100,                  0, 0        },
  { 361, "samplevol",    X_INT,  &samplevol,       0, 100,                  0, OPT_SAVE },
  { 361, "sv",           X_INT,  &samplevol,       0, 100,                  0, 0        },

  { 362, "samplefreq",   X_INT,  &options.samplerate, 8000, 48000,          0, OPT_SAVE },
  { 362, "sf",           X_INT,  &options.samplerate, 8000, 48000,          0, 0        },

  { 363, "samples",      X_FIX,  &options.use_samples, 1,                 0,0, OPT_SAVE },
  { 363, "sam",          X_FIX,  &options.use_samples, 1,                 0,0, 0        },
  { 363, "nosamples",    X_FIX,  &options.use_samples, 0,                 0,0, OPT_SAVE },
  { 363, "nosam",        X_FIX,  &options.use_samples, 0,                 0,0, 0        },

  { 364, "sdlbufsize",   X_INT,  &sdl_buffersize,  32, 65536,               0, OPT_SAVE },
  { 365, "close",        X_FIX,  &close_device,    TRUE,                  0,0, OPT_SAVE },
  { 365, "noclose",      X_FIX,  &close_device,    FALSE,                 0,0, OPT_SAVE },

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
void	xmame_config_show_option(void)
{
  fprintf(stdout,
  "\n"
  "==========================================\n"
  "== SOUND OPTIONS ( dependent on XMAME ) ==\n"
  "==                     [ XMAME 0.71.1 ] ==\n"
  "==========================================\n"
  "    -[no]sound / -[no]snd   Enable/disable sound (if available) [-sound]\n"
  "    -[no]audio / -[no]ao    Enable/disable audio-device [-audio]\n"
  "    -[no]fmgen              Use/don't use cisc's fmgen library\n"
  "                                               (if compiled in)  [-nofmgen]\n"
/*"    -volume / -v <i>        Set volume to <int> db, (-32(soft) - 0(loud))\n"*/
  "    -fmvol / -fv <i>        Set FM     level to <i> %%, (0 - 100) [100]\n"
  "    -psgvol / -pv <i>       Set PSG    level to <i> %%, (0 - 100) [20]\n"
  "    -beepvol / -bv <i>      Set BEEP   level to <i> %%, (0 - 100) [60]\n"
  "    -rhythmvol / -rv <i>    Set RHYTHM level to <i> %%, (0 - 100) [100]\n"
  "    -adpcmvol / -av <i>     Set ADPCM  level to <i> %%, (0 - 100) [100]\n"
  "    -fmgenvol / -fmv <i>    Set fmgen  level to <i> %%, (0 - 100) [100]\n"
  "    -samplevol / -sv <i>    Set SAMPLE level to <i> %%, (0 - 100) [100]\n"
  "    -samplefreq / -sf <i>   Set the playback sample-frequency/rate [44100]\n"
  "    -[no]samples / -[no]sam Use/don't use samples (if available) [-nosamples]\n"
  "    -sdlbufsize <i>         buffer size of sound stream (power of 2) [2048]\n"
  "    -sdlbufnum <i>          Number of frames of sound to buffer [4]\n"
  "    -[no]close              Close/no close sound device in MENU mode [-noclose]\n"
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
int		xmame_config_check_option(char *opt1, char *opt2, int priority)
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
int		xmame_config_save_option(void (*real_write)
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
			" Buffer size of sound (512 - 16384, power of 2) ",
			&sdl_buffersize,  32, 65536,
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
int		xmame_has_audiodevice(void)
{
	if (use_sound) {
		if (use_audiodevice) return TRUE;
	}
	return FALSE;
}

int		xmame_has_mastervolume(void)
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

static struct sysdep_dsp_struct *sound_dsp = NULL;
static float sound_bufsize = 3.0;
static int sound_samples_per_frame = 0;
static int type;


/*
 * xmame-0.71.1/src/unix/sysdep/sysdep_dsp.c 
 */
static struct sysdep_dsp_struct *sysdep_dsp_create(
   int      *samplerate,    /* sample_rate (==22050) */
   int      *type,          /* */
   float    bufsize)        /* 3.0 / 55.4 */
{
   struct sysdep_dsp_struct *dsp = NULL;
   struct sysdep_dsp_create_params params;
   
   /* fill the params struct */
   params.bufsize = bufsize;
   params.device = "SDL";
   params.samplerate = *samplerate;
   params.type = *type;
   params.flags = 0;	/* SYSDEP_DSP_EMULATE_TYPE | SYSDEP_DSP_O_NONBLOCK */
   
   /* create the instance */
   if (!(dsp = sdl_dsp_create(&params)))
   {
      return NULL;
   }
   
   /* calculate buf_size if not done by the plugin */
   if(!dsp->hw_info.bufsize)
      dsp->hw_info.bufsize = (int)(bufsize * dsp->hw_info.samplerate);

   return dsp;
}
/*
 * xmame-0.71.1/src/unix/sysdep/sysdep_dsp.c 
 */
static void sysdep_dsp_destroy(struct sysdep_dsp_struct *dsp)
{
   if(dsp->convert_buf)
      free(dsp->convert_buf);
   dsp->destroy(dsp);
}


/*
 * xmame-0.71.1/src/unix/sound.c
 */
int osd_start_audio_stream(int stereo)
{
   type = SYSDEP_DSP_16BIT | (stereo? SYSDEP_DSP_STEREO:SYSDEP_DSP_MONO);
   
   sound_dsp = NULL;

   sound_enabled = 1;

   /* set initial value */
   Machine->sample_rate = options.samplerate;
   sound_samples_per_frame = (int)(Machine->sample_rate /
      Machine->drv->frames_per_second);	      /* mean 22050Hz/55.4Hz */

   if (use_audiodevice == 0) {
      sound_enabled = 0;
      return sound_samples_per_frame;
   }

   /* create dsp */
   if(sound_enabled)
   {
      if(!(sound_dsp = sysdep_dsp_create(
         &options.samplerate,
         &type,
         sound_bufsize * (1 / Machine->drv->frames_per_second))))
      {
         osd_stop_audio_stream();
         sound_enabled = 0;
      }
   }
   
   /* create sound_stream */
   if(sound_enabled)
   {
      /* sysdep_dsp_open may have changed the samplerate */
      Machine->sample_rate = options.samplerate;
      
      /* calculate samples_per_frame */
      sound_samples_per_frame = (int)(Machine->sample_rate /
         Machine->drv->frames_per_second);	      /* mean 22050Hz/55.4Hz */
   }

   return sound_samples_per_frame;
}
/*
 * xmame-0.71.1/src/unix/sound.c
 */
int osd_update_audio_stream(INT16 *buffer)
{
   /* sound enabled ? */
   if (sound_enabled)
      sound_dsp->write(sound_dsp, (unsigned char *)buffer,
         sound_samples_per_frame);
   
   return sound_samples_per_frame;
}
/*
 * xmame-0.71.1/src/unix/sound.c
 */
void osd_stop_audio_stream(void)
{
   if(sound_dsp)
      sysdep_dsp_destroy(sound_dsp);

   sound_dsp = NULL;
}
/*
 * xmame-0.71.1/src/unix/sound.c
 */
void osd_sound_enable (int enable_it)
{
   if (enable_it)
   {
      sound_enabled = 1;
      if (!sound_dsp)
      {
	 if (!(sound_dsp = sysdep_dsp_create(
	    &options.samplerate,
	    &type,
	    sound_bufsize * (1 / Machine->drv->frames_per_second))))
	    sound_enabled = 0;
      }
   }
   else
   {
      if (sound_dsp)
      {
	 sysdep_dsp_destroy(sound_dsp);
	 sound_dsp = NULL;
      }
      sound_enabled = 0;
   }
}


/*
 * xmame-0.71.1/src/unix/video.c
 */
void	osd_update_video_and_audio(void)
{
   /* nothing */
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
void osd_set_mastervolume(int attenuation)
{
}

int osd_get_mastervolume(void)
{
    return VOL_MIN;
}

#endif	/* USE_SOUND */
