#ifndef _COM_API_H_
#define _COM_API_H_


#include <api/libsi/si_tdt.h>
#include <math.h>
#if ((SUBTITLE_ON == 1)||(TTX_ON == 1)) 
#include <hld/vbi/vbi.h>
#include <api/libttx/ttx_osd.h>
#include <api/libttx/lib_ttx.h>
#include <hld/sdec/sdec.h>
#include <api/libsubt/lib_subt.h>
#endif

enum OSD_MODE
{
	OSD_WINDOW = 0,
	OSD_SUBTITLE,
	OSD_TELETEXT,
#if(defined(MULTI_CAS) && (CAS_TYPE== CAS_ABEL) )
	OSD_NO_SHOW,
#endif	
	OSD_INVALID_MODE = 0XFF,
};

typedef enum
{
	//key
	CTRL_MSG_SUBTYPE_KEY = 0,
	CTRL_MSG_SUBTYPE_KEY_UI,
	CTRL_MSG_TYPE_KEY = 2,// 2//don't use this type, 
	//cmd
	CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT,
	CTRL_MSG_SUBTYPE_CMD_EXIT_ROOT,
	CTRL_MSG_SUBTYPE_CMD_EXIT_TOP,
	CTRL_MSG_SUBTYPE_CMD_EXIT_ALL,
	CTRL_MSG_SUBTYPE_CMD_EXIT,
	CTRL_MSG_SUBTYPE_CMD_SLEEP,
	CTRL_MSG_SUBTYPE_CMD_PLAY_PROGRAM,
	CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW,
	CTRL_MSG_SUBTYPE_CMD_FACTORY_RESET,
	CTRL_MSG_SUBTYPE_CMD_TIMER_WAKEUP,
	CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE,
	CTRL_MSG_SUBTYPE_CMD_TVSAT_SWITCH,
	CTRL_MSG_SUBTYPE_CMD_SCAN,
	CTRL_MSG_SUBTYPE_CMD_TP_TURN,
	CTRL_MSG_SUBTYPE_CMD_UPGRADE,
	CTRL_MSG_SUBTYPE_CMD_UPGRADE_SLAVE,
	CTRL_MSG_SUBTYPE_CMD_REBOOT,
	CTRL_MSG_SUBTYPE_CMD_EPG_PF_UPDATED,
	CTRL_MSG_SUBTYPE_CMD_EPG_SCH_UPDATED,
	CTRL_MSG_SUBTYPE_CMD_EPG_DETAIL_UPDATED,
	CTRL_MSG_SUBTYPE_CMD_EPG_FULL,
	CTRL_MSG_SUBTYPE_CMD_PIDCHANGE,
	CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE,
	CTRL_MSG_SUBTYPE_CMD_AGE_LIMIT_UPDATED,
	CTRL_MSG_SUBTYPE_CMD_PLAYMUSIC,
	CTRL_MSG_SUBTYPE_CMD_PLAYIMAGE,
	CTRL_MSG_SUBTYPE_CMD_USBUNMOUT,
	CTRL_MSG_SUBTYPE_CMD_TP_SERVICE_CHANGED,
	CTRL_MSG_SUBTYPE_CMD_TP_SERVICE_SEARCH_OVER,
	CTRL_MSG_TYPE_CMD = 100,

	//status
	CTRL_MSG_SUBTYPE_STATUS_SIGNAL,
	CTRL_MSG_SUBTYPE_STATUS_BITERROR,
	CTRL_MSG_SUBTYPE_STATUS_TIMEOUT,
	CTRL_MSG_SUBTYPE_STATUS_RF_SET,
	CTRL_MSG_SUBTYPE_STATUS_PVR,
	CTRL_MSG_SUBTYPE_STATUS_MCAS,
	CTRL_MSG_SUBTYPE_STATUS_CI,
	CTRL_MSG_SUBTYPE_STATUS_SCROLL_STRING,

	CTRL_MSG_SUBTYPE_STATUS_NVOD_REFEVENT,
	CTRL_MSG_SUBTYPE_STATUS_NVOD_SCHEDULE,
	CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMEOUT,
	CTRL_MSG_SUBTYPE_STATUS_NVOD_NOINFO,
	CTRL_MSG_SUBTYPE_STATUS_NVOD_SCANOVER,
	CTRL_MSG_SUBTYPE_STATUS_NVOD_DIRECT_UPDATE,
	CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMER_WAKEUP,
	CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMER_EXPIRE,
	CTRL_MSG_SUBTYPE_STATUS_NVOD_TIME_UPDATE,

	CTRL_MSG_SUBTYPE_STATUS_PMTPID_CHANGE,
	CTRL_MSG_SUBTYPE_STATUS_SDT_CHANGE,
	CTRL_MSG_SUBTYPE_STATUS_PAT_CHANGE,

	CTRL_MSG_SUBTYPE_STATUS_OTA_FORCE,
	CTRL_MSG_SUBTYPE_STATUS_OTA_MANUAL,
	CTRL_MSG_SUBTYPE_STATUS_UPGRADE_MANUAL,
	CTRL_MSG_SUBTYPE_STATUS_USB,

	CTRL_MSG_SUBTYPE_STATUS_NIT_UPDATE,

//for usb upg	
	CTRL_MSG_SUBTYPE_STATUS_USB_UPG_MOUNT,
	CTRL_MSG_SUBTYPE_STATUS_USB_UPG_UNMOUNT,
//end
	CTRL_MSG_SUBTYPE_STATUS_USBMOUNT,
	CTRL_MSG_SUBTYPE_STATUS_USBUNMOUNT,
	CTRL_MSG_SUBTYPE_STATUS_USBOVER,
	CTRL_MSG_SUBTYPE_STATUS_USBREFLASH,
	CTRL_MSG_SUBTYPE_STATUS_IMAGEOVER,
	CTRL_MSG_SUBTYPE_STATUS_IMGTOOVER,
	CTRL_MSG_SUBTYPE_STATUS_MP3OVER,
	CTRL_MSG_SUBTYPE_STATUS_JPEG_DEC_PROCESS,
    	CTRL_MSG_SUBTYPE_STATUS_JPEG_DEC_ERR,
    	CTRL_MSG_SUBTYPE_STATUS_MPLAYER_DISPLAY,
    	CTRL_MSG_SUBTYPE_STATUS_SPECTRUM_UPDATE,

	CTRL_MSG_SUBTYPE_CMD_CAS_SMC_INFO_UPDATED,
	CTRL_MSG_SUBTYPE_CMD_CAS_EMM_INFO_UPDATED,
	CTRL_MSG_SUBTYPE_CMD_CAS_ECM_INFO_UPDATED,
	CTRL_MSG_SUBTYPE_CMD_CAS_PRODUCT_UPDATED,
	CTRL_MSG_SUBTYPE_CMD_CAS_NORMAIL_UPDATED,
} ControlMsgType_t;


typedef enum
{
    SYS_STATE_INITIALIZING = 0,
    SYS_STATE_UPGRAGE_HOST,
    SYS_STATE_UPGRAGE_SLAVE,
    SYS_STATE_USB_MP,
    SYS_STATE_POWER_OFF,
    SYS_STATE_TEXT,
    SYS_STATE_NORMAL,
    SYS_STATE_NVOD,
    SYS_STATE_DATACAST,
    SYS_STATE_SEARCH_PROG,
    SYS_STATE_ADV,
} sys_state_t;

typedef enum
{
	USB_STATUS_PLUGIN = 0,
	USB_STATUS_PULLOUT,
	USB_STATUS_MOUNTFAIL,
	USB_STATUS_UNMOUNTFAIL,
	USB_STATUS_ATTACH,
	USB_STATUS_DETACH,
	USB_STATUS_OVER,
	USB_STATUS_CHECKDISK,
	USB_STATUS_PVRCHECK,
	USB_STATUS_MPCHECK,
	USB_STATUS_PVRCLEAR,
	USB_STATUS_MPCLEAR,
	USB_STATUS_FSMOUNT,
	USB_STATUS_FSUNMOUNT,
}usb_state_t;

typedef enum
{
    SCREEN_BACK_VIDEO = 0,
    SCREEN_BACK_MENU,
    SCREEN_BACK_RADIO,
	//    SCREEN_BACK_CHLST,
	SCREEN_BACK_TEXT
} ScreenBackState_t;


extern sys_state_t system_state;
extern ScreenBackState_t screen_back_state;


typedef struct
{
	ControlMsgType_t msgType;
	INT32				msgSbuType;
	UINT32 msgCode;
} ControlMsg_t,  *pControlMsg_t;

typedef struct
{
	UINT8 panel_type; /* PANEL_TYPE_NORMAL / PANEL_TYPE_M51 */
	UINT8 rcu_type; /* SMT_006 / SMT_008 : sys_def.h */
	UINT8 rcu_sw_pos; /* 0 / 1 /2 */
} rcu_fp_type_t;

enum API_PLAY_TYPE
{
	API_PLAY_NOCHANNEL		= 0,
	API_PLAY_NORMAL,
	API_PLAY_PASSWORD_ERROR,
	API_PLAY_ABORT_BY_PWD_CHECK,
	API_PLAY_OUT_OF_RANGE,/*??*/
 	API_PLAY_MOVING_DISH,/*for what*/
	API_PLAY_FRONTEND_FALIER,
	API_PLAY_UNEXPECTED_ABORT,
	
};

typedef struct hw_sw_info
{
	UINT32 oui;
	UINT32 hw_model;
	UINT32 hw_version;
	UINT32 sw_model;
	UINT32 sw_version;
} HW_SW_INFO;

/*------------------------------------------------------------

GPIO related functions : LNB and Scart control
------------------------------------------------------------*/


void api_audio_set_volume(UINT8 volume);
void api_audio_set_mute(UINT8 flag);
#ifdef DVBT_BEE_TONE
void api_audio_gen_tonevoice(UINT8 level, UINT8 init);
void api_audio_stop_tonevoice(void);
#endif

#ifdef AUDIO_SPECIAL_EFFECT
void api_audio_beep_start(UINT8 *data, UINT32 data_len);
void api_audio_beep_set_interval(UINT32 interval);
void api_audio_beep_stop(void);
#endif

int api_video_get_srcmode(UINT8 *video_Src);
UINT8 api_video_get_tvout(void);
void api_video_set_tvout(UINT8 tv_mode);
void api_video_set_pause(UINT8 flag);

void api_Svideo_OnOff(unsigned int sw);
void api_YUV_OnOff(unsigned int sw);

ID api_start_timer(char *name, UINT32 interval, OSAL_T_TIMER_FUNC_PTR handler);
ID api_start_cycletimer(char* name, UINT32 interval, OSAL_T_TIMER_FUNC_PTR handler);
void api_stop_timer(ID *pTimerID);


enum OSD_MODE api_osd_mode_change(enum OSD_MODE mode);


enum API_PLAY_TYPE api_play_channel(UINT32 channel, BOOL b_force, BOOL b_check_password, BOOL bID);

BOOL api_tv_radio_switch();

void api_show_bootup_logo(void);
void api_show_radio_logo(void);
void api_show_menu_logo(void);

int api_parse_defaultset_subblock(void);


//about time
UINT8 api_check_is_leap_year(UINT16 yy);
INT32 api_check_valid_date(UINT16 year, UINT8 month, UINT8 day);
BOOL api_correct_yy_mm_dd(UINT16 *yy, UINT8 *mm, UINT8 *dd, UINT8 type);

/* Only compare up to minute */
INT32 api_compare_time(date_time *dt1, date_time *dt2);
INT32 api_compare_day(date_time *dt1, date_time *dt2);
INT32 api_compare_day_time(date_time *dt1, date_time *dt2);

/*Compare up to second */
INT32 api_compare_time_ext(date_time *dt1, date_time *dt2);
INT32 api_compare_day_time_ext(date_time *dt1, date_time *dt2);


UINT8 api_get_first_timer(void);
void api_timers_proc(void);

UINT16 Hex2Dec(UINT16 x);

#ifdef MUTILI_DATACAST
#define	generic_dbcast_message_send(mbf_id, msg_type, msg_code, if_clear_buffer) _generic__dbcast_message_send(mbf_id, msg_type, msg_code, if_clear_buffer)
#else
#define generic_dbcast_message_send(mbf_id, msg_type, msg_code, if_clear_buffer) do{}while(0)
#endif


#endif //_COM_API_H_
