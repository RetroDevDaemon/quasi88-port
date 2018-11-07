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

#include "sysdep_mixer.h"	/* sysdep_mixer_init()	*/

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
/*		xmame_config_init() [src/unix/config.c] */
int		xmame_config_init(void)
{
	/* create the rc object */
	if (!(rc = rc_create()))
		return OSD_NOT_OK;

	if(sysdep_dsp_init(rc, NULL))
		return OSD_NOT_OK;

	if(sysdep_mixer_init(rc, NULL))
		return OSD_NOT_OK;

	if(rc_register(rc, sound_opts))
		return OSD_NOT_OK;

	return OSD_OK;
}

/*		xmame_config_exit() [src/unix/config.c] */
void	xmame_config_exit(void)
{
	if(rc)
	{
		sysdep_mixer_exit();
		sysdep_dsp_exit();
		rc_destroy(rc);
		rc = NULL;
	}
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
const T_CONFIG_TABLE *xmame_config_get_opt_tbl(void)
{
	return NULL;
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
			"==                     [ XMAME  0.106 ] ==\n"
			"==========================================\n"
			);

	rc_print_help(rc, stdout);
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
	return rc_quasi88(rc, opt1, opt2, priority);
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
#include "rc_priv.h"
int		xmame_config_save_option(void (*real_write)
								   (const char *opt_name, const char *opt_arg))
{
	return rc_quasi88_save(rc->option, real_write);
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
/* T_SNDDRV_CONFIG *xmame_config_get_sndopt_tbl(void)     [src/unix/sound.c] */


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
		if (osd_has_audio_device()) return TRUE;
	}
	return FALSE;
}

int		xmame_has_mastervolume(void)
{
    if (use_sound) {
		if (osd_has_sound_mixer()) return TRUE;
	}
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
/* int  osd_start_audio_stream(int stereo)                [src/unix/sound.c] */
/* int  osd_update_audio_stream(INT16 *buffer)            [src/unix/sound.c] */
/* void osd_stop_audio_stream(void)                       [src/unix/sound.c] */
/* void osd_sound_enable(int enable)                      [src/unix/sound.c] */

/*		osd_update_video_and_audio(mame_display *display) [src/unix/video.c] */
void	osd_update_video_and_audio(void)
{
	if (sysdep_sound_stream)
	{
		sysdep_sound_stream_update(sysdep_sound_stream);
	}
/*	if (GAME_REFRESH_RATE_CHANGED)									*/
/*		sound_update_refresh_rate(display->game_refresh_rate);		*/
/*	}																*/
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
/* void osd_set_mastervolume(int attenuation)             [src/unix/sound.c] */
/* int  osd_get_mastervolume(void)                        [src/unix/sound.c] */

#endif	/* USE_SOUND */
