#ifndef	__HDMI_API_H__
#define __HDMI_API_H__

#include <mediatypes.h>


/*******************************************************************
*					The API interface of HDMI					     *
*******************************************************************/

//******************* HDMI Resolution definition ************************
enum HDMI_API_RES
{
	HDMI_RES_INVALID = 0,
	HDMI_RES_480I,
	HDMI_RES_480P,
	HDMI_RES_576I ,
	HDMI_RES_576P,
	HDMI_RES_720P_50,
	HDMI_RES_720P_60,
	HDMI_RES_1080I_25,
	HDMI_RES_1080I_30,	
	HDMI_RES_1080P_24,
	HDMI_RES_1080P_25,
	HDMI_RES_1080P_30,
	HDMI_RES_1080P_50,
	HDMI_RES_1080P_60,	
};
//******************The control state of HDMI task***********************
enum HDMI_CTRL_STATE
{
	HDMI_CTRL_STATE_IDLE = 0,		//hotplug has been detached
	HDMI_CTRL_STATE_READY,		//hotplug has been attached
	HDMI_CTRL_STATE_PLAY,					//hdmi has transmitted data
	HDMI_CTRL_STATE_AV_CHANGED		//hdmi A/V has been changed
};


//*************** The Structure of audio information in EDID *******************
enum EDID_AUD_FMT_CODE
{
	EDID_AUDIO_LPCM 				= 0x0001,
	EDID_AUDIO_AC3					= 0x0002,
	EDID_AUDIO_MPEG1				= 0x0004,
	EDID_AUDIO_MP3					= 0x0008,
	EDID_AUDIO_MPEG2				= 0x0010,
	EDID_AUDIO_AAC					= 0x0020,
	EDID_AUDIO_DTS					= 0x0040,
	EDID_AUDIO_ATRAC				= 0x0080,
	EDID_AUDIO_ONEBITAUDIO 	= 0x0100,
	EDID_AUDIO_DD_PLUS			= 0x0200,
	EDID_AUDIO_DTS_HD				= 0x0400,
	EDID_AUDIO_MAT_MLP			= 0x0800,
	EDID_AUDIO_DST					= 0x1000,
	EDID_AUDIO_WMAPRO				= 0x2000,
};

enum HDMI_AV_CHG_STE
{
	HDMI_CB_NOTHING = 0x00,
	HDMI_CB_CLK_RDY2CHG = 0x01,
	HDMI_CB_AV_INFO_CHG = 0x02,
	HDMI_CB_CLK_CHG_DONE = 0x04,
	HDMI_CB_CLR_RDY2CHG = 0x08,
};
#if 0
//*********************The Structure of Video Interface ***********************
struct de2Hdmi_video_infor
{
	enum TVSystem			tv_mode;
	UINT16					width;
	UINT16					height;
	enum PicFmt				format;
	BOOL					scan_mode;
	enum TVMode			output_aspect_ratio;
	enum HDMI_AV_CHG_STE	av_chg_ste;
};
#else
//*********************The Structure of Video Interface ***********************
struct de2Hdmi_video_infor
{
	enum TVSystem			tv_mode;
	UINT16					width;
	UINT16					height;
	enum PicFmt				format;
	BOOL					scan_mode;
	BOOL					afd_present;
	enum TVMode			output_aspect_ratio;
	UINT8					active_format_aspect_ratio;
	enum HDMI_AV_CHG_STE	av_chg_ste;
};
#endif
//*********************The Structure of Audio Interface ***********************
enum I2S_FMT_TYPE{
	I2S_FMT_I2S	=0,
	I2S_FMT_LEFT,
	I2S_FMT_RIGHT
};

enum AUDIO_CODING_TYPE{
	AUD_TYPE_PCM 					= 0x01,//0X0,
	AUD_TYPE_AC3 					= 0x02,//0X200,
	AUD_TYPE_MPEG1_L12 			= 0x03,//0X101,
	AUD_TYPE_MPEG1_L3 			= 0x04,//0X102,
	AUD_TYPE_MEPG2	  			= 0x05,//0X110,
	AUD_TYPE_AAC					= 0x06,//0X130,	
	AUD_TYPE_DTS					= 0x07,
	AUD_TYPE_ATRAC				= 0x08,
	AUD_TYPE_ONEBITAUDIO			= 0x09,
	AUD_TYPE_DD_PLUS				= 0x0A,
	AUD_TYPE_DTS_HD				= 0x0B,
	AUD_TYPE_MAT_MLP				= 0x0C,
	AUD_TYPE_DST					= 0x0D,	
	AUD_TYPE_WMAPRO				= 0x0E,
};

/*Channel Position*/
#define CH_FL	0X0
#define CH_FC	0X1
#define CH_FR	0X2
#define CH_FLC	0X3
#define CH_FRC	0X4
#define CH_RL	0X5
#define CH_RC	0X6
#define CH_RR	0X7
#define CH_RLC	0X8
#define CH_RRC	0X9
#define CH_LFE	0Xa

struct snd2Hdmi_audio_infor
{
	/* INFO by user setting */
	UINT32 user_def_ch_num;		/* 2 or 8 							*/
	UINT32 pcm_out;					/* 1: PCM 0: BIT STREAM 2: FORCE DD 				*/

	/* AUDIO stream status */
	enum AUDIO_CODING_TYPE coding_type;
	UINT32 max_bit_rate;				/* maximum bit rate 					*/
	UINT32 ch_count;					/* 2, 6(5.1), 8							*/
	UINT32 sample_rate;				/* 48000, 44100, 32000 etc				*/
	UINT32 level_shift;				/* level shift after down-mixing			*/

	/* S/PDIF config dynamic setting */
	UINT32 spdif_edge_clk;			/*0: rising edge latch data, 1: falling edge latch data	*/

	/* I2S config dynamic setting */
	UINT32 ch_status;			/*	31:30	Reserved
									29:28	Clock Accuracy
									27:24	Sample rate
									23:20	Channel Number
									19:16	Source Number
									15:8		L & Category
									7:6		Mode
									5:3		Emphasis
									2		Copyright
									1		Audio content flag
									0		Professional flag				*/
	UINT8   ch_position[8];		/* ch_position[i] 
								bit0~bit3: speaker(CH_FL, CH_FC etc)
								bit4~bit6: channel position(0~7, the position in I2S dma buffer)
								bit7: speaker enable(1:enable, 0:disable)		*/
											  
	/* I2S config fixed setting */
	UINT32 bclk_lrck;					/* 32, 64										*/
	UINT32 word_length;				/* 16bits, 24bits								*/
	UINT32 i2s_edge_clk;				/* 0:rising edge latch data, 1:falling edge latch data	*/
	enum I2S_FMT_TYPE i2s_format;	/* I2S, Left Justify and Right Justify				*/
	UINT32 lrck_hi_left;				/* 1: lrck high for left channel, 0: reverse			*/
	enum HDMI_AV_CHG_STE	av_chg_ste;
};

//*********************End - The Structure of Audio Interface ***********************

INT32 set_audio_info_to_hdmi(UINT32 param);
INT32 set_video_info_to_hdmi(UINT32 param);
UINT16 api_get_physical_address(void);
INT32 api_set_logical_address(UINT8 logical_address);
UINT8 api_get_logical_address(void);
INT32 api_hdmi_cec_transmit(UINT8* message, UINT8 message_length);
INT32 api_get_edid_video_format(enum PicFmt *format);
INT32 api_get_edid_video_resolution(enum HDMI_API_RES *res);
INT32 api_get_edid_all_video_resolution(UINT32 *native_res_index, enum HDMI_API_RES *video_res);
INT32 api_get_edid_audio_out(enum EDID_AUD_FMT_CODE *aud_fmt);
INT32 api_get_edid_all_audio_out(enum EDID_AUD_FMT_CODE *aud_fmt);
void api_hdmi_switch(BOOL bOnOff);
BOOL api_get_hdmi_hdcp_onoff(void);
void api_set_hdmi_hdcp_onoff(BOOL bOnOff);
void api_set_hdmi_res(enum HDMI_API_RES res);
UINT32 api_get_hdmi_res(void);
UINT32 api_get_hdmi_state(void);
char *get_hdmi_sw_version(void);
INT32 api_hdmi_register(void);
UINT8 *get_hdmi_cm_mfc_code(void);
UINT8 *get_hdmi_cm_key_id(void);
UINT8 *get_hdmi_cm_status(void);
void hdmi_log_edid_start(void);
void hdmi_log_edid_stop(void);
void hdmi_log_debug_start(void);						
void hdmi_log_debug_stop(void);
INT16 api_get_edid_audio_delay(void);
#endif

