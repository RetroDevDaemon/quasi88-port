/***********************************************************************
 * ������ɽ��Ͻ��� (�����ƥ��¸)
 *
 *      �ܺ٤ϡ� snddrv.h / mame-quasi88.h ����
 ************************************************************************/

/*----------------------------------------------------------------------*
 * Classic�С������Υ����������ɤ�����ʬ�ϡ�                          *
 * Koichi NISHIDA ��� Classic iP6 PC-6001/mk2/6601 emulator �Υ������� *
 * ���ͤˤ����Ƥ��������ޤ�����                                         *
 *                                                   (c) Koichi NISHIDA *
 *                                       based on MESS/MAME source code *
 *----------------------------------------------------------------------*/


#include <OSUtils.h>
#include <Sound.h>
#include <Timer.h>
#include <QuickTime.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef	USE_SOUND

#include "mame-quasi88.h"

#include "snddrv.h"
static	int use_audiodevice = 1;	/* use audio-devide for audio output */


static Boolean InitializeSound(void);
static void TearDownSound(void);
static void PauseSound(Boolean inPause);
static Boolean isFullSoundStream(int dataNum);
static Boolean isLessSoundStream(int dataNum, long tick);


static Boolean alreadyInit = false;

#define	REFRESH_RATE	60.0
#define	CHANNELS		sNumOfChannles

/*----------------------------------------------------------------------*/
int		attenuation = 0;		/* �ܥ�塼�� -32��0 [db] */

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

  { 354, "volume",       X_INT,  &attenuation,     -32, 0,                  0, OPT_SAVE },

  { 355, "fmvol",        X_INT,  &fmvol,           0, 100,                  0, OPT_SAVE },
  { 356, "psgvol",       X_INT,  &psgvol,          0, 100,                  0, OPT_SAVE },
  { 357, "beepvol",      X_INT,  &beepvol,         0, 100,                  0, OPT_SAVE },
  { 358, "rhythmvol",    X_INT,  &rhythmvol,       0, 200,                  0, OPT_SAVE },
  { 359, "adpcmvol",     X_INT,  &adpcmvol,        0, 200,                  0, OPT_SAVE },
  { 360, "fmgenvol",     X_INT,  &fmgenvol,        0, 100,                  0, OPT_SAVE },
  { 361, "samplevol",    X_INT,  &samplevol,       0, 100,                  0, OPT_SAVE },

  { 362, "samplefreq",   X_INT,  &options.samplerate, 8000, 44100,          0, OPT_SAVE },

  { 363, "samples",      X_FIX,  &options.use_samples, 1,                 0,0, OPT_SAVE },
  { 363, "nosamples",    X_FIX,  &options.use_samples, 0,                 0,0, OPT_SAVE },

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
  "    -volume <n>             Set volume to <n> db, (-32 (soft) - 0(loud)) [0]\n"
  "    -fmvol <level>          Set FM     level to <level> %%, (0 - 100) [100]\n"
  "    -psgvol <level>         Set PSG    level to <level> %%, (0 - 100) [20]\n"
  "    -beepvol <level>        Set BEEP   level to <level> %%, (0 - 100) [60]\n"
  "    -rhythmvol <level>      Set RHYTHM level to <level> %%, (0 - 100) [100]\n"
  "    -adpcmvol <level>       Set ADPCM  level to <level> %%, (0 - 100) [100]\n"
  "    -fmgenvol <level>       Set fmgen  level to <level> %%, (0 - 100) [100]\n"
  "    -samplevol <level>      Set SAMPLE level to <level> %%, (0 - 100) [100]\n"
  "    -samplefreq <rate>      Set the playback sample-frequency/rate [44100]\n"
  "    -[no]samples            Use/don't use samples (if available) [-nosamples]\n"
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
	return NULL;
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
    if (use_audiodevice) return TRUE;
    else                 return FALSE;
}

int		xmame_has_mastervolume(void)
{
	return TRUE;
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

static int sound_samples_per_frame = 0;


// C string <> Pascal string conversion
static void CopyCStringToPascal(const char *src, unsigned char *dst)
{
	int num = 0;
	int i;
	const char *srcBak = src;

	while (*(src++) != '\0') {
		num++;
	}
	src = srcBak;
	dst[0] = num;
	for (i=0; i<num; i++) {
		dst[i+1]=*(src++);
	}
}
static void CopyPascalStringToC(const unsigned char *src, char *dst)
{
	int i;
	for (i=0; i < src[0]; i++) {
		*(dst++) = src[i+1];
	}
	*(dst) = '\0';
}



// constants
enum
{
	kSoundStreamFramesAt44kHz	= 12288*1,
	kTotalSoundBuffers			= 2,
	kQueuedSoundBuffers			= 2
};

// our buffered data
static SInt16	sSoundStream[kSoundStreamFramesAt44kHz * 2];
static UInt32	sSoundStreamFrames;
static UInt32	sSoundIn;
static UInt32	sSoundOut;

// the sound channel
static SndChannelPtr sSoundChannel;

// sound timing
ComponentInstance 	gSoundClock;

// sound buffer parameters
static UInt32			sSoundBufferFrames;
static UInt32			sSoundBufferBytes;
static Boolean			sSoundIsPaused;

// sound buffers
static SndCallBackUPP	sSoundCallback;
static ExtSoundHeader	*sSoundBuffer[kTotalSoundBuffers];
static SndCommand		sSoundBufferCmd[kTotalSoundBuffers];
static SndCommand		sSoundCallbackCmd[kTotalSoundBuffers];

static int				sNumOfChannles = 2;

// function prototypes
static pascal void SoundCallback(SndChannelPtr inChannel, SndCommand *inCommand);
static void FillSoundBuffer(SInt16 *inBuffer);
static Boolean InitializeSoundChannel(void);
static Boolean InitializeSoundBuffers(void);
static void FreeSoundBuffers(void);

//#pragma segment SegMain

static void memcpyW(register void *dst, register void *src, register unsigned long num)
{
	register int i;
	if (num&1) {
		for (i = 0; i < num; i++)
			*(((short *)dst)++) = *(((short *)src)++);
	} else {
		for (i = 0; i < num>>1; i++)
			*(((long *)dst)++) = *(((long *)src)++);
	}
}

// examine whether the sSoundStreamFrames is full or not
static Boolean isFullSoundStream(int dataNum)
{
	UInt32		soundIn = sSoundIn;
	Boolean		updateOk = false;

	// check	
	if (soundIn < sSoundOut) {
		if ((soundIn + dataNum) <= sSoundOut) updateOk = true;
	} else if ((soundIn + dataNum) > sSoundStreamFrames) {
		if ((soundIn + dataNum - sSoundStreamFrames) <= sSoundOut) updateOk = true;
	} else updateOk = true;
	if (!updateOk) return true;
	else return false;
}

// examine whether the sSoundStreamFrames will be empty soon or not
static Boolean isLessSoundStream(int dataNum, long tick)
{
	Boolean		updateOk = false;

	// check	
	if (sSoundOut < sSoundIn) {
		return sSoundIn - sSoundOut <= dataNum*tick;
	} else if (sSoundIn < sSoundOut) {
		return sSoundIn - (sSoundOut - sSoundStreamFrames) <= dataNum*tick;
	} else return true;
}

//#pragma segment SegInit

// start the audio system going
int osd_start_audio_stream(int stereo)
{
	CHANNELS = (stereo) ? 2 : 1;

	if (Machine->sample_rate < 8000) {
		Machine->sample_rate = 8000;
	} else if (Machine->sample_rate > 44100) {
		Machine->sample_rate = 44100;
	}

#ifdef	XMAME_SNDDRV_071

#if 0
	sound_samples_per_frame = (int)(Machine->sample_rate /
         Machine->drv->frames_per_second);
#else
	sound_samples_per_frame = (int)(Machine->sample_rate / REFRESH_RATE);
#endif

#else	/* ver 0.106 */

#if 0
	sound_samples_per_frame = Machine->sample_rate / Machine->refresh_rate;
#else
	sound_samples_per_frame = (int)(Machine->sample_rate / REFRESH_RATE);
#endif

#endif


	if (use_audiodevice == FALSE) { return sound_samples_per_frame; }

	if (alreadyInit) return sound_samples_per_frame;
	alreadyInit = true;

	InitializeSound();

	////////
	sSoundIsPaused = false;
	
	// reset the sound buffer
	sSoundIn = sSoundOut = 0;
	sSoundStreamFrames = kSoundStreamFramesAt44kHz * CHANNELS * Machine->sample_rate / 44100;

	// pick the appropriate sound buffer size
	sSoundBufferFrames = 512 * Machine->sample_rate / 44100;
	sSoundBufferBytes = sSoundBufferFrames << CHANNELS;
	
	// allocate the sound channel and initialize the buffering
	if (!InitializeSoundBuffers())
	{
		return 0;
	}
	
	// set the initial volume (-0 dB)
	osd_set_mastervolume(attenuation);
	////////

	osd_sound_enable(1);

	return sound_samples_per_frame;
}

//#pragma segment SegMain

// pushes data into the audio stream
int osd_update_audio_stream(INT16 *buffer)
{
	register UInt32	soundIn = sSoundIn;
	register UInt32	framesToCopy;
	int dataNum;
	INT16 *bufferX;

	static long prevTick = 0;
	register int times;

	if (use_audiodevice == FALSE) { return sound_samples_per_frame; }

#if 1
	{
		register long tick = TickCount() - prevTick;
		if (tick > 4) tick = 4;
		prevTick = TickCount();

		if (isFullSoundStream(sound_samples_per_frame * CHANNELS)) return sound_samples_per_frame;
		times = isLessSoundStream(sound_samples_per_frame * CHANNELS, tick+1) ? (tick+1) : 1;
	}
#else
	times = 1;
#endif

	for (; times; times--) {

		bufferX = buffer;
		dataNum = sound_samples_per_frame * CHANNELS;

		// copy up to the end of the stream buffer
		while (dataNum > 0)
		{
			// determine how many frames we can copy
			framesToCopy = sSoundStreamFrames - soundIn;
			if (framesToCopy > dataNum)
				framesToCopy = dataNum;

			// copy and count the samples
			memcpyW(&sSoundStream[soundIn], bufferX, framesToCopy);
			dataNum -= framesToCopy;
			bufferX += framesToCopy;

			// adjust the output pointer
			soundIn += framesToCopy;
			if (soundIn >= sSoundStreamFrames)
				soundIn -= sSoundStreamFrames;
		}
	}
	sSoundIn = soundIn;

	/* return the samples to play this next frame */
	return sound_samples_per_frame;
}

void osd_stop_audio_stream(void)
{
	if (use_audiodevice == FALSE) { return; }

	if (!alreadyInit) return;
	alreadyInit = false;

	osd_sound_enable(0);
	TearDownSound();
}

void osd_update_video_and_audio(void)
{
	/* nothing */
}

void osd_sound_enable(int enable_it)
{
	if (use_audiodevice == FALSE) { return; }

	PauseSound(!enable_it);
}

//#pragma segment SegInit

//	sets the main volume ( -Attenuation dB )
static int now_attenuation = 0;
void osd_set_mastervolume(int attenuation)
{
	float multiplier = 1.0;
	UInt32 volume;

	if (use_audiodevice == FALSE) { return; }

	now_attenuation = attenuation;

	// compute a multiplier from the original volume
	while (attenuation++ < 0)
		multiplier *= 1.0 / 1.122018454;	// = (10 ^ (1/20)) = 1dB

	// apply it
	volume = (UInt32)(multiplier * 0x100);
	if (volume > 0x100) volume = 0x100;

	{
		SndCommand mySndCmd;
		//OSErr myErr;

		mySndCmd.cmd = volumeCmd;
		mySndCmd.param1 = 0;		// unused with volumeCmd
		mySndCmd.param2 = (volume << 16) | volume;
		SndDoCommand(sSoundChannel, &mySndCmd, false);
	}
}

int osd_get_mastervolume (void)
{
	if (use_audiodevice == FALSE) { return -32; }

	return now_attenuation;
}

// allocates a sound channel and starts it running
// first this must be called ?
static Boolean InitializeSound(void)
{
	SndCommand 		command;
	OSErr			err;
	
	// attempt to initialize the sound buffers
	if (!InitializeSoundChannel()) return false;

	// nuke any existing clock references
	gSoundClock = NULL;

	// first turn on the clock
	command.cmd = clockComponentCmd;
	command.param1 = true;
	err = SndDoImmediate(sSoundChannel, &command);
	if (err != noErr)
		return false;
		
	// then get a component instance
	command.cmd = getClockComponentCmd;
	command.param2 = (SInt32)&gSoundClock;
	err = SndDoImmediate(sSoundChannel, &command);
	if (err != noErr)
	{
		sSoundChannel = NULL;
		return false;
	}
	return true;
}

// tearDownSound
static void TearDownSound(void)
{
	// free the channel
	if (sSoundChannel != NULL)
		SndDisposeChannel(sSoundChannel, true);
	sSoundChannel = NULL;

	// attempt to free the sound buffers
	FreeSoundBuffers();
}

// pauses/resumes sound output
static void PauseSound(Boolean inPause)
{
	sSoundIsPaused = inPause;
}

//#pragma segment SegMain

// common code to fill a sound buffer.
// called from call back
static void FillSoundBuffer(register SInt16 *inBuffer)
{
	register SInt32		framesAvailable;
	register UInt32		framesToCopy;

	if (sSoundIsPaused) {
		memset(inBuffer, 0, sSoundBufferFrames << CHANNELS);
		return;
	}
	
#if 1
	{
		static UInt16 skipCounter = 0;

		if (sSoundIn == sSoundOut) {
			if (skipCounter > 20) {
				memset(inBuffer, 0, sSoundBufferFrames << CHANNELS);
				return;
			}
			skipCounter ++;
		} else {
			skipCounter = 0;
		}
	}
#endif

	framesAvailable = sSoundIn - sSoundOut;

	// account for the circular buffer
	if (framesAvailable < 0)
		framesAvailable += sSoundStreamFrames;

	// if we have more than enough data, clip to the maximum per buffer
	if (framesAvailable >= sSoundBufferFrames * CHANNELS)
		framesAvailable = sSoundBufferFrames * CHANNELS;

	// copy up to the end of the stream buffer
	while (framesAvailable > 0)
	{
		// determine how many frames we can copy
		framesToCopy = sSoundStreamFrames - sSoundOut;
		if (framesToCopy > framesAvailable)
			framesToCopy = framesAvailable;

		// copy and count the samples
		memcpyW(inBuffer, &sSoundStream[sSoundOut], framesToCopy);
		
		framesAvailable -= framesToCopy;
		inBuffer += framesToCopy;

		// adjust the output pointer
		sSoundOut += framesToCopy;

		// wrap the output pointer
		if (sSoundOut >= sSoundStreamFrames)
			sSoundOut -= sSoundStreamFrames;
	}
}

//#pragma segment SegInit

// initialize the sound channel and any other one-shot data
static Boolean InitializeSoundChannel(void)
{
	OSErr err;
	long initOptions;

	// allocate a UPP callback
	sSoundCallback = NewSndCallBackUPP(SoundCallback);
	
	// now allocate the channel
	sSoundChannel = NULL;

    if (CHANNELS == 1) {
		initOptions = initMono   + initNoInterp + initNoDrop;
	} else {
		initOptions = initStereo + initNoInterp + initNoDrop;
	}
	err = SndNewChannel(&sSoundChannel, sampledSynth, initOptions, sSoundCallback);
	return (err == noErr);
}

// initialize the sound buffers and create the sound channel
static Boolean InitializeSoundBuffers(void)
{
	extended80 extFreq;
	int i;
	OSErr err;
	Handle itlHandle;               // The 'itl4' resource handle
	long offset, length;            // Offset-length of parts table
	NumFormatStringRec myFormatRec; // Canonical format record
	Str15 pRate;
	char cRate[16];
	
	sprintf(cRate, "%5d", Machine->sample_rate); 
	CopyCStringToPascal(cRate, pRate);
	GetIntlResourceTable (smCurrentScript, smNumberPartsTable, &itlHandle, &offset, &length);
	StringToFormatRec ("\p#####", (NumberParts *)(*itlHandle + offset), &myFormatRec);
	StringToExtended (pRate, &myFormatRec, (NumberParts *)(*itlHandle + offset), &extFreq);

	for (i = 0; i < kTotalSoundBuffers; i++)
	{
		// allocate and clear the sound buffer
		sSoundBuffer[i] = malloc(sizeof(ExtSoundHeader) + sSoundBufferBytes);
		// R. Nabet 0001212 : added error handling
		if (sSoundBuffer[i] == NULL)
		{
			while (--i >= 0)
				free(sSoundBuffer[i]);
			return false;
		}
		memset(sSoundBuffer[i], 0, sizeof(ExtSoundHeader) + sSoundBufferBytes);
		
		// intialize the buffer structures
		sSoundBuffer[i]->numChannels 	= CHANNELS;
		sSoundBuffer[i]->sampleRate 	= (UInt32)Machine->sample_rate << 16;
		sSoundBuffer[i]->encode 		= extSH;
		sSoundBuffer[i]->numFrames 		= sSoundBufferFrames;
		sSoundBuffer[i]->AIFFSampleRate = extFreq;
		sSoundBuffer[i]->sampleSize 	= 16;
		
		// initialize the sound commands
		sSoundBufferCmd[i].cmd 			= bufferCmd;
		sSoundBufferCmd[i].param2 		= (SInt32)sSoundBuffer[i];
		sSoundCallbackCmd[i].cmd 		= callBackCmd;
		sSoundCallbackCmd[i].param1 	= (i + kQueuedSoundBuffers) % kTotalSoundBuffers;
	}
	
	// start stuff playing
	for (i = 0; i < kQueuedSoundBuffers; i++)
	{
		err = SndDoCommand(sSoundChannel, &sSoundBufferCmd[i], true);
		if (err != noErr)
			return false;
		err = SndDoCommand(sSoundChannel, &sSoundCallbackCmd[i], true);
		if (err != noErr)
			return false;
	}
	return true;
}

// free the sound buffers and the sound channel
static void FreeSoundBuffers(void)
{
	UInt32 i;
	
	// free the buffers
	for (i = 0; i < kTotalSoundBuffers; i++)
	{
		if (sSoundBuffer[i] != NULL)
			free(sSoundBuffer[i]);
		sSoundBuffer[i] = NULL;
	}
}

//#pragma segment SegMain

// Callback command to stream data
static pascal void SoundCallback(SndChannelPtr inChannel, SndCommand *inCommand)
{
	UInt32		bufferIndex = inCommand->param1;

	// fill the sound buffer
	FillSoundBuffer((SInt16 *)sSoundBuffer[bufferIndex]->sampleArea);

	// queue up the commands

	SndDoCommand(sSoundChannel, &sSoundBufferCmd[bufferIndex], true);
	SndDoCommand(sSoundChannel, &sSoundCallbackCmd[bufferIndex], true);
}

#endif		/* USE_SOUND */
