#ifndef _SND_H_
#define _SND_H_

#include <hld/snd/snd_dev.h>
#include <osal/osal.h>
#define DEBUG_SND 1
#ifdef DEBUG_SND
#define SND_PRINTF   PRINTF
#else
#define SND_PRINTF(...)	do{}while(0)
#endif

typedef struct{
	void (*spec_call_back)(INT32 *);
	UINT32 collumn_num;
}spec_param;

typedef struct
{
    UINT32 column_num;
    UINT8 *ptr_table;
}spec_step_table;

typedef struct
{
    UINT32 drop_threshold;
    UINT32 wait_threshold;
    UINT32 delay_video_sthreshold;
}snd_sync_param;

/* add for CI+ protect spdif output */
struct snd_spdif_scms
{
	UINT8 copyright:1;
	UINT8 reserved:7;
	UINT8 l_bit:1;
	UINT8 category_code:7;
	UINT16 reserved16;
};

#define STC_DELAY  0x400
#define MUTE_BY_GPIO	0
#define MUTE_BY_SCART	1

#define SND_STATE_DETACH   0
#define SND_STATE_ATTACH   1
#define SND_STATE_IDLE        2
#define SND_STATE_PLAY       4

#define SND_SUB_STATE_BUSY           1
#define SND_SUB_STATE_NO_DATA    2
#define SND_SUB_STATE_NO_BUFF     4

/* define the device IO control code for sound */
#define  SND_IO  0x0000000F

#define IS_SND_RUNNING  	       	(SND_IO + 1)
#define IS_SND_MUTE					(SND_IO + 2)
#define SND_CC_MUTE				(SND_IO + 3)
#define SND_CC_MUTE_RESUME		(SND_IO + 4)
#define SND_SET_FADE_SPEED		(SND_IO + 5)
#define IS_PCM_EMPTY				(SND_IO + 6)
#define SND_PAUSE_MUTE				(SND_IO + 7)
#define SND_SPO_ONOFF				(SND_IO + 8)
#define SND_REQ_REM_DATA			(SND_IO + 9)
#define SND_SPECTRUM_START		(SND_IO + 10)
#define SND_SPECTRUM_STOP			(SND_IO + 11)
#define SND_SPECTRUM_CLEAR		(SND_IO + 12)
#define SND_BYPASS_VCR				(SND_IO + 13)
#define FORCE_SPDIF_TYPE			(SND_IO + 14)//strongly recommend call this command in channel change task.
#define SND_DAC_MUTE            		(SND_IO + 15)
#define SND_CHK_SPDIF_TYPE			(SND_IO + 16)
#define SND_CHK_DAC_PREC			(SND_IO + 17)
#define SND_CHK_PCM_BUF_DEPTH	(SND_IO + 18)
#define SND_POST_PROCESS_0		(SND_IO + 19)
#define SND_SPECIAL_MUTE_REG		(SND_IO + 20)
#define STEREO_FUN_ON				(SND_IO + 21)
#define SND_REQ_REM_PCM_DATA		(SND_IO + 22)
#define SND_SPECTRUM_STEP_TABLE     (SND_IO + 23)
#define SND_SPECTRUM_VOL_INDEPEND   (SND_IO + 24)
#define SND_SPECTRUM_CAL_COUNTER    (SND_IO + 25)
#define SND_SET_SYNC_DELAY          (SND_IO + 26)
#define SND_REQ_REM_PCM_DURA		(SND_IO + 27)
#define SND_SET_SYNC_LEVEL          (SND_IO + 28)
#define SND_GET_SPDIF_TYPE          (SND_IO + 29)   // enum ASndOutSpdifType *
#define SND_SET_BS_OUTPUT_SRC       (SND_IO + 30)
#define SND_SET_MUTE_TH             (SND_IO + 31)
#define SND_GET_MUTE_TH             (SND_IO + 32)
#define SND_SET_SPDIF_SCMS          (SND_IO + 33)   // struct snd_spdif_scms *
#define SND_GET_SAMPLES_REMAIN      (SND_IO + 34)
#define SND_SECOND_DECA_ENABLE      (SND_IO + 35)
#define SND_SET_DESC_VOLUME_OFFSET  (SND_IO + 36)
#define SND_GET_TONE_STATUS         (SND_IO + 37)
#define SND_DO_DDP_CERTIFICATION    (SND_IO + 38)
#define SND_AUTO_RESUME             (SND_IO + 39)
#define SND_SET_SYNC_PARAM          (SND_IO + 40)   // snd_sync_param *
#define SND_RESET_DMA_BUF           (SND_IO + 41)   // TRUE or FALSE. TRUE -- reset sound dma buffer to drop some frames

#define SND_I2S_OUT (SND_IO + 42)
#define SND_HDMI_OUT (SND_IO + 43)
#define SND_SPDIF_OUT (SND_IO + 44)

/* Add by Joy for implementing an independent A/V sync module.module
 * Date: 2011.10.10.
 */
#define SND_SET_FRAME_SHOW_PTS_CALLBACK (SND_IO + 45)
#define SND_GET_RAW_PTS (SND_IO +46)

#define SND_ADV_IO		(SND_IO + 0x200) //snd advanced io command
#define SND_BASS_TYPE				(SND_ADV_IO + 1)
#define SND_REG_HDMI_CB			(SND_ADV_IO + 2)
/*****************************************************/

#define SPDO_SRC_FLR				0x00
#define SPDO_SRC_SLR				0x01
#define SPDO_SRC_CSW				0x02
#define SPDO_SRC_DMLR				0x03
#define SPDO_SRC_EXLR				0x04
#define SPDO_SRC_BUF				0x07
#define SPDO_SRC_LFEC				0x01
// SPDIF raw data coding type

//bass_type
#define BASS_DISABLE				0x00
#define BASS_CON0					0x01
#define BASS_CON1					0x02
#define BASS_CON2_NSUB				0x03
#define BASS_CON2_WSUB			0x04
#define BASS_ALT_CON2				0x05
#define BASS_CON3_NSUB				0x06
#define BASS_CON3_WSUB			0x07
#define BASS_SIMP_NSUB				0x08
#define BASS_SIMP_WSUB				0x09

enum SndToneStatus
{
	SND_STREAM_STATUS = 0,	// None.
	SND_TONE_STATUS,			// Left channel.
};

enum SndChannel
{
	SND_CH_NONE = 0,	// None.
	SND_CH_L,			// Left channel.
	SND_CH_R,			// Right channel.
	SND_CH_LS,			// Left surround channel.
	SND_CH_RS,			// Right surround channel.
	SND_CH_C,			// Center channel.
	SND_CH_LFE,		// Low frequency effect channel.
	SND_CH_DML,		// downmix L channel.
	SND_CH_DMR		// downmix R channel.
};

enum SndDownMixChannel
{
	SND_DOWNMIXCHANNEL_DM,
	SND_DOWNMIXCHANNEL_LR
};

enum SndDownMixMode
{
	SND_DOWNMIXMODE_51 = 1,
	SND_DOWNMIXMODE_LORO,
	SND_DOWNMIXMODE_LTRT,
	SND_DOWNMIXMODE_VIR
};

enum SndDescOutputChannel
{
	SND_FORWARD_CH = 1,
    SND_SURROUND_CH,
    SND_LFE_C_CH,
    SND_DOWNMIX_CH
};

enum SndEqualizer
{
	SND_EQ_NONE = 0,		// Disable equalizer mode.
	SND_EQ_CLASSIC,			// Classsic mode.
	SND_EQ_ROCK,			// Rock mode.
	SND_EQ_JAZZ,			// Jazz mode.
	SND_EQ_POP,				// Pop mode
	SND_EQ_BASS,			// Bass mode.
	SND_EQ_USER				// User define mode.
};

enum SndReverb
{
	SND_REVERB_OFF = 0,
	SND_REVERB_CHURCH,
	SND_REVERB_CINEMA,
	SND_REVERB_CONCERT,
	SND_REVERB_HALL,
	SND_REVERB_LIVE,
	SND_REVERB_ROOM,
	SND_REVERB_STADIUM,
	SND_REVERB_STANDARD
};

enum SndSpeakerSize
{
	SND_SPEAKER_SIZE_OFF= 0,
	SND_SPEAKER_SIZE_ON,
	SND_SPEAKER_SIZE_SMALL,
	SND_SPEAKER_SIZE_BIG
};

enum SndDRC
{
	SND_DRC_OFF = 0,
	SND_DRC_CUSTOM_A,
	SND_DRC_CUSTOM_D,
	SND_DRC_LINE_OUT,
	SND_DRC_RF_REMOD
};

enum ASndOutMode
{
	SND_OUT_GEN = 0,
	SND_OUT_DIG,
	SND_OUT_DIGGEN
};

#define SND_SUPPORT_AD      0x01
#define SND_SUPPORT_DDPLUS  0x02

struct snd_feature_config
{
	struct snd_output_cfg output_config;
	UINT8 support_spdif_mute;
	UINT8 swap_lr_channel;
	UINT8 conti_clk_while_ch_chg;
    UINT8 support_desc;
};

struct snd_callback
{
    OSAL_T_HSR_PROC_FUNC_PTR	phdmi_snd_cb;
    void (*spec_call_back)(INT32 *);
};

RET_CODE snd_open(struct snd_device * dev);
RET_CODE snd_close(struct snd_device * dev);
RET_CODE snd_set_mute(struct snd_device * dev, enum SndSubBlock sub_blk, UINT8 enable);
RET_CODE snd_set_volume(struct snd_device * dev, enum SndSubBlock sub_blk, UINT8 volume);
UINT8 snd_get_volume(struct snd_device * dev);
RET_CODE snd_io_control(struct snd_device * dev, UINT32 cmd, UINT32 param);
RET_CODE snd_request_pcm_buff(struct snd_device * dev, UINT32 size);
RET_CODE snd_s3601_request_pcm_sync_buff(struct snd_device * dev, UINT32 size);
void snd_s3601_send_pcm_to_buff(struct snd_device *dev, struct pcm_output *pcm, UINT32 *frame_header);
RET_CODE snd_data_enough(struct snd_device * dev);
RET_CODE snd_config(struct snd_device * dev, UINT32 sample_rate, UINT16 sample_num, UINT8 precision);
RET_CODE snd_set_spdif_type(struct snd_device * dev, enum ASndOutSpdifType type);
void snd_write_pcm_data(struct snd_device*dev,struct pcm_output*pcm,UINT32*frame_header);
void snd_write_pcm_data2(struct snd_device * dev, UINT32 * frame_header, UINT32 * left, UINT32 * right, UINT32 number,UINT32 ch_num);
void snd_start(struct snd_device*dev);
void snd_stop(struct snd_device*dev);	 

RET_CODE get_stc(UINT32 * stc_msb32, UINT8 stc_num);
void set_stc(UINT32 stc_msb32, UINT8 stc_num);
void get_stc_divisor(UINT16 * stc_divisor, UINT8 stc_num);
void set_stc_divisor(UINT16 stc_divisor, UINT8 stc_num);
void stc_invalid(void);
void stc_valid(void);
RET_CODE snd_request_desc_pcm_buff(struct snd_device * dev, UINT32 size);
void snd_write_desc_pcm_data(struct snd_device*dev,struct pcm_output*pcm,UINT32*frame_header);

#ifdef DVBT_BEE_TONE
void snd_gen_tone_voice(struct snd_device * dev, struct pcm_output*pcm, UINT8 init); //tone voice
void snd_stop_tone_voice(struct snd_device * dev);  //tone voice
#endif
void snd_output_config(struct snd_device * dev, struct snd_output_cfg * cfg_param);
#endif /*_SND_H_*/
