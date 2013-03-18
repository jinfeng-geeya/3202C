#ifndef _LIB_PUB_H_
#define _LIB_PUB_H_

#include <basic_types.h>
#include <mediatypes.h>
#include <sys_config.h>
#include <osal/osal.h>
#include <api/libpub/lib_as.h>
#include <api/libpub/lib_mv.h>
#include <hld/nim/nim_dev.h>
#include <api/libpub/lib_frontend.h>




/*******************************************
libpub cmd bit pattern
/********************************************/
#define LIBPUB_CMD_MAIN_TYPE_MASK	0xFF00
#define LIBPUB_CMD_SUB_TYPE_MASK		0x00FF

#define LIBPUB_CMD_NULL			0x0000
#define LIBPUB_CMD_CC				0x0100
#define LIBPUB_CMD_MV				0x0200
#define LIBPUB_CMD_AS				0x0300


/****!! NOTE: 0x01xx is reserved for channel change!!***/
#define LIBPUB_CMD_CC_PLAY		0x0101
#define LIBPUB_CMD_CC_RESET		0x0102
#define LIBPUB_CMD_CC_STOP		0x0103
#define LIBPUB_CMD_CC_PAUSE_VIDEO	0x0104
#define LIBPUB_CMD_CC_RESUME_VIDEO	0x0105
#define LIBPUB_CMD_CC_SWITCH_APID       0x0106
#define LIBPUB_CMD_CC_SWITCH_ACH        0x0107
#define LIBPUB_CMD_CC_SHOW_LOGO         0x0108
#define LIBPUB_CMD_CC_SET_FRONTEND      0x0109
//ci patch cmd
#define LIBPUB_CMD_CC_START_PATCH      0x010a


/****!! NOTE: 0x02xx is reserved for multiview!!*****/
#define LIBPUB_CMD_MV_ENTER                 0x0201
#define LIBPUB_CMD_MV_DRAWONEBOX    0x0202
#define LIBPUB_CMD_MV_EXIT                  0x0203


/****!! NOTE: 0x03xx is reserved for channel scan!!*****/
#define LIBPUB_CMD_AS_START		0x0301
#define LIBPUB_CMD_AS_STOP			0x0302


/****!! NOTE: 0x07xx is reserved for media player!!*****/
#define LIBPUB_CMD_MPPE				0x0700
#define LIBPUB_FLG_MPPE_BIT			0x00100000


/****!! NOTE: 0x09xx is reserved for pvr!!*****/
#define LIBPUB_CMD_NV				0x0900


/****!!channel change cmd bit!!****/
#define CC_CMD_NULL					0x00000000
#define CC_CMD_CLOSE_VPO			0x00000001
#define CC_CMD_STOP_AUDIO			0x00000002
#define CC_CMD_STOP_VIDEO			0x00000004
#define CC_CMD_STOP_DMX				0x00000008
#define CC_CMD_FILL_FB				0x00000010
#define CC_CMD_START_AUDIO			0x00000020
#define CC_CMD_START_VIDEO			0x00000040
#define CC_CMD_START_DMX			0x00000080
#define CC_CMD_DMX_CHG_APID			0x00000100
#define CC_CMD_AUDIO_SWITCH_CHAN	0x00000200
#define CC_CMD_SET_FRONTEND			0x00000400
#define CC_CMD_PAUSE_VIDEO			0x00000800
#define CC_CMD_RESUME_VIDEO			0x00001000
//#define CC_CMD_SET_ANTENNA		0x00002000
#define CC_CMD_SET_XPOND			0x00004000
#define CC_CMD_START_SI_MONITOR		0x00008000
#define CC_CMD_INFO_VPO_CHANCHNG	0x00100000
#define CC_CMD_STOP_MCAS_SERVICE	0x00200000
#define CC_CMD_DMX_DISABLE			0x00400000


/*******************************************
libpub flag bit patterns
/********************************************/
/* change channel related flag bit pattern */
#define LIBPUB_FLG_CC_PLAY_BIT			0x00000001
#define LIBPUB_FLG_CC_STOP_BIT			0x00000002
#define LIBPUB_FLG_CC_PAUSE_BIT		0x00000004
#define LIBPUB_FLG_CC_SWITCH_APID_BIT	0x00000008
#define LIBPUB_FLG_CC_SWITCH_ACH_BIT	0x00000010
#define LIBPUB_FLG_CC_LOGO_BIT			0x00000020
//set front end
#define LIBPUB_FLG_CC_SETFT_BIT		0x00000040
//ci patch
#define LIBPUB_FLG_CC_PATCH_BIT		0x00000080


/* multiview related flag bit pattern */
//recieve mv_enter cmd
#define LIBPUB_FLG_MV_ENTER	0x00000800
#define LIBPUB_FLG_MV_BUSY 	0x00001000
#define LIBPUB_FLG_MV_CANCEL	0x00002000
#define LIBPUB_FLG_MV_ACTIVE	0x00004000
#define LIBPUB_FLG_MV_EXIT		0x00008000


/*monitor flag bit*/
#define LIBPUB_MONITOR_MUTEX            0x00010000
#define LIBPUB_MONITOR_CHECK_VDEC   0x00020000
#define LIBPUB_MONITOR_CHECK_NIM    0x00040000

/*monitor callback to send device status msg. msg-message type;param-device handle*/
typedef void (*libpub_monitor_callback)(UINT32 msg, UINT32 param);

/*monitor device msg*/
#define LIB_MONITOR_NIM_UNLOCK      1
#define LIB_MONITOR_VDEC_HOLD       2



#define LIBPUB_CMD_DATA_SIZE				80
struct libpub_cmd
{
    UINT16 cmd_type;
    UINT8 sync;
    UINT8 reserve;
    UINT32 cmd_bit;
    UINT32 para1;	
    UINT32 para2;
    UINT8 data1[LIBPUB_CMD_DATA_SIZE];
    UINT8 data2[LIBPUB_CMD_DATA_SIZE];
};

struct pub_module_config
{
	BOOL dm_enable;			
	UINT32 monitor_interval;//unit ms
	libpub_monitor_callback function;
};

enum
{
	GET_KEY=1,
	KEY_PROC,
	API_PLAY_CHANNEL,
	API_GET_CHANNEL,
	API_SET_CHAN_INFO,
	CC_CHECK_NIM ,//= 1,
	CC_PRE_PLAY,
	CC_CLOSE_VPO,
	CC_STOP_AUDIO,
	CC_STOP_VIDEO,
	CC_SELECT_DECV,
	CC_STOP_DMX,
	CC_PRE_TUNER,
	CC_SET_XPOND,
	CC_POST_TUNER,
	CC_VPO_CHANCHNG,
	CC_START_DMX,
	CC_START_VIDEO,
	CC_START_AUDIO,
	CC_STEP_NUM
};

struct
{
	UINT32 start_tick;
	UINT32 end_tick;
	UINT32 step;
} g_cc_play_tick[CC_STEP_NUM];


#ifdef __cplusplus
extern "C" {
#endif 

typedef void(*libpub_cmd_handler)(struct libpub_cmd *);
	
BOOL libpub_cmd_handler_register(UINT16 cmd_main_type, libpub_cmd_handler handler);
BOOL libpub_overlay_cmd_register(UINT16 repeat_cmd);
RET_CODE libpub_init(struct pub_module_config *config);
INT32 libpub_send_cmd(struct libpub_cmd* cmd,UINT32 tmout);
INT32 libpub_wait_cmd_finish(UINT32 cmd_bit,UINT32 tmout);
void libpub_declare_cmd_finish(UINT32 cmd_bit);


/*****************************************/

void chchg_init(void);

INT32 chchg_play_channel(struct ft_frontend *ft, struct cc_param *param, BOOL blk_scrn);
INT32 chchg_stop_channel(struct cc_es_info *es, struct cc_device_list *dev_list, BOOL blk_scrn);
INT32 chchg_pause_video(struct vdec_device *vdec);
INT32 chchg_resume_video(struct vdec_device *vdec);
INT32 chchg_switch_audio_pid(struct cc_es_info *es, struct cc_device_list *dev_list);
INT32 chchg_switch_audio_chan(struct snd_device *snd, UINT8 audio_chan);
INT32 chchg_show_logo(struct cc_logo_info *plogo, struct cc_device_list *dev_list);





/*************************************************************/ 

 /*BIT 0*/
 #define DM_OFF	0		
 #define DM_ON	1		
 /*BIT 1~2*/
 #define DM_NIMCHECK_OFF 	4	
 #define DM_NIMCHECK_ON	6	

 #define dm_set_onoff(a) do{PRINTF("DM (%d):________ <%s>\n", a, __FUNCTION__);MonitorSwitch(a);}while(0)


void MonitorInit(UINT32 interval, libpub_monitor_callback func);
void MonitorProc();
void MonitorRst();
void MonitorSwitch(UINT32 onoff);
void monitor_switch_dev_monitor(UINT32 device_flag, UINT8 onoff);
 
 #ifdef __cplusplus
 }
 #endif 

#define CC_CMD_RESET_CRNT_CH		0xFFFFFFFD


#endif


