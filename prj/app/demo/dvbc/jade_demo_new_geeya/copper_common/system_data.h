#ifndef _SYSTEM_DATA_H_
#define _SYSTEM_DATA_H_

#include <sys_config.h>
#include <api/libtsi/db_3l.h>
#include "sys_config.h"
#include <api/libtsi/sec_tdt2.h>
#ifdef AD_TYPE
#include "../ad_interface/ad_interface.h"
#endif
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_DVN)
#include <api/libcas/dvn/dvnca.h>
#endif
#endif

#define MAX_LOCAL_GROUP_NUM 16
#define MAX_GROUP_NUM	(3+MAX_LOCAL_GROUP_NUM)
#define MAX_FAVGROUP_NUM		8
#define FAV_GROP_RENAME
#define FAV_GRP_NAME_LEN    MAX_SERVICE_NAME_LENGTH
#define MAX_LOCAL_GROUP_LEN	16

#define RADIO_CHAN 		PROG_RADIO_MODE
#define TV_CHAN			PROG_TV_MODE


#define PASSWORD_LEN	4

#define ALL_GROUP_TYPE		0
#define LOCAL_GROUP_TYPE	1
#define DATA_GROUP_TYPE	(MAX_LOCAL_GROUP_NUM + 1)//17/
#define NVOD_GROUP_TYPE	(DATA_GROUP_TYPE + 1)//18/
#define FAV_GROUP_TYPE		(NVOD_GROUP_TYPE + 1)

#define GRP_WITH_DATA		(1<<0)
#define GRP_WITH_NVOD		(1<<1)
#define GRP_WITH_FAV		(1<<2)

#define P_INVALID_ID INVALID_POS_NUM

#define DEFAULT_YEAR    2005
#define DEFAULT_MONTH   1
#define DEFAULT_DAY     1

#define MAX_RCU_POSNUM  2


/* Define for VDAC configuration */
#define VDAC_NUM_MAX		6
#define VDAC_TYPE_NUM		6
//Type
#define VDAC_TYPE_CVBS		0
#define VDAC_TYPE_SVIDEO	1
#define VDAC_TYPE_YUV		2
#define VDAC_TYPE_RGB		3
#define VDAC_TYPE_SCVBS	4
#define VDAC_TYPE_SSV		5
#define VDAC_TYPE_MAX		6
//Detail
#define VDAC_CVBS			(VDAC_TYPE_CVBS<<2|0)
#define VDAC_SVIDEO_Y		(VDAC_TYPE_SVIDEO<<2|0)
#define VDAC_SVIDEO_C		(VDAC_TYPE_SVIDEO<<2|1)
#define VDAC_YUV_Y			(VDAC_TYPE_YUV<<2|0)
#define VDAC_YUV_U			(VDAC_TYPE_YUV<<2|1)
#define VDAC_YUV_V			(VDAC_TYPE_YUV<<2|2)
#define VDAC_RGB_R			(VDAC_TYPE_RGB<<2|0)
#define VDAC_RGB_G			(VDAC_TYPE_RGB<<2|1)
#define VDAC_RGB_B			(VDAC_TYPE_RGB<<2|2)
#define VDAC_SCVBS			(VDAC_TYPE_SCVBS<<2|0)
#define VDAC_SSV_Y			(VDAC_TYPE_SSV<<2|0)
#define VDAC_SSV_C			(VDAC_TYPE_SSV<<2|1)
#define VDAC_NULL			0xFF
//Number of sub item
#define TYPE_CVBS_NUM		1
#define TYPE_SVIDEO_NUM	2
#define TYPE_YUV_NUM		3
#define TYPE_RGB_NUM		3
#define TYPE_SCVBS_NUM		1
#define TYPE_SSV_NUM		2

//dac config
#define CVBS  		0x00
#define YC_Y  		0x01
#define YC_C  		0x02
#define CAV_Y  		0x03
#define CAV_Pb  		0x04
#define CAV_Pr  		0x05
#define RGB_R  		0x06
#define RGB_G  		0x07
#define RGB_B  		0x08
#define RGB_SYNC  	0x09
#define SECAM_YC      0x0A
#define SECAM_Y		0x0B
#define SECAM_C        0x0C

#define DAC_DEFAULT  	0x0A

typedef enum
{
	CHAN_LOCK_NO = 0,
	CHAN_LOCK_10,
	CHAN_LOCK_14,
	CHAN_LOCK_18,
	CHAN_LOCK_TOTAL
} CHANNEL_LOCK_TYPE;

typedef enum
{
	TV_MODE_AUTO = 0,
	TV_MODE_PAL,
	TV_MODE_PAL_M,//  PAL3.58
	TV_MODE_PAL_N,
	TV_MODE_NTSC358,//NTSC3.58
	TV_MODE_NTSC443,
	TV_MODE_SECAM,
} TV_MODE_TYPE;


typedef enum
{
	TV_ASPECT_RATIO_43 = 0,
	TV_ASPECT_RATIO_169,
	TV_ASPECT_RATIO_AUTO
} TV_ASPECT_RATIO_TYPE;

typedef enum
{
    DISPLAY_MODE_NORMAL = 0,
    DISPLAY_MODE_LETTERBOX,
    DISPLAY_MODE_PANSCAN,
} DISPLAY_MODE_TYPE;


typedef enum
{
	SCART_CVBS = 0,
	SCART_RGB
} SCART_OUT_TYPE;

typedef enum
{
	RF_MODE_NTSC_M = 0,
	RF_MODE_PAL_BG,
	RF_MODE_PAL_I,
	RF_MODE_PAL_DK
} RF_MODE_TYPE;


typedef struct Local_Time_tag
{
	UINT8 buseGMT;
	UINT8 GMToffset_count; /*0-47  0(-11:30)<-23(00:00)->47(+12:00) */
	UINT8 SummerTime;
	UINT8 TimeDisplay;
} Local_Time;

typedef struct Parental_Control_tag
{
	UINT8 bchannel_lock;
	UINT8 channel_lock_level;
	UINT8 bmenu_lock;
} Parental_Control;


typedef struct AV_Set_tag
{
	UINT8 tv_mode: 4;
	UINT8 tv_ratio: 4;
	UINT8 display_mode: 4;
	UINT8 scart_out: 4;
	UINT8 rf_mode;
	UINT8 rf_channel;
	UINT8 vdac_out[VDAC_NUM_MAX];
} AV_Set;



typedef struct
{
	UINT8 pallete;
	UINT8 time_display: 2;
	UINT8 subtitle_display: 2; // 0--> off ,1--> standard subtitle , 2---> picture subtitle
	UINT8 prog_position: 4;
	UINT8 time_out;
	UINT8 osd_trans;
#if ((SUBTITLE_ON == 1)||(TTX_ON == 1))   
        UINT8 subtitle_lang;
	UINT8 second_osd_mode;
#endif
} Osd_Set;

typedef struct
{
	UINT16 tv_channel;
	UINT16 radio_channel;
} group_channel_t;



typedef struct
{
	UINT16 sat_id;
	UINT16 tp_id;
	UINT16 prog_number;
} prog_id_t;

typedef enum
{
    CHAN_SWITCH_FREE = 0,
    CHAN_SWITCH_SCRAMBLED,
    CHAN_SWITCH_ALL,
} chan_sw_t;

typedef enum
{
	CHAN_CHG_VIDEO_BLACK = 0,
	CHAN_CHG_VIDEO_FREEZE
} chan_chg_video_t;

typedef struct  //liuyi added 2005-3-10
{
	UINT8 OSD_lang;
	UINT8 Audio_lang_1;
	UINT8 Audio_lang_2;
	UINT8 ttx_lang;
	UINT8 sub_lang;
	UINT8 epg_lang;
} Language;


#define MAX_EPG_TIMER_NUM    32
#define MAX_NVOD_TIMER_NUM 4
#define AUTO_START_TIMER_NUM 2
#define MAX_TIMER_NUM    (MAX_EPG_TIMER_NUM+MAX_NVOD_TIMER_NUM)
#define MAX_TIMER_MESSAGE_LEN   20

typedef enum
{
    TIMER_MODE_OFF = 0,
    TIMER_MODE_ONCE,        
    TIMER_MODE_DAILY,       
    TIMER_MODE_WEEKLY,        
    TIMER_MODE_MONTHLY,        
    TIMER_MODE_YEARLY,        
} TIMER_MODE;

typedef enum
{
    TIMER_SERVICE_CHANNEL = 0,
    TIMER_SERVICE_MESSAGE,
} TIMER_SERVICE;

typedef enum
{
    TIMER_MSG_BIRTHDAY = 0,
    TIMER_MSG_ANNIVERSARY,
    TIMER_MSG_GENERAL,
} TIMER_SERVICE_SMG;

typedef enum
{
    TIMER_STATE_READY = 0,
    TIMER_STATE_RUNING,
} TIMER_STATE;

typedef struct{
	UINT8 timer_mode; //TIMER_MODE
	UINT8 timer_service;
	UINT16 wakeup_year;
	UINT8 wakeup_month;
	UINT8 wakeup_day;
	//	UINT8       wakeup_time_hour;
	//	UINT8       wakeup_time_min;
	UINT32 wakeup_time;
	//	UINT8       wakeup_duration_hour;
	//	UINT8       wakeup_duration_min;
	UINT16 wakeup_duration_time;
	UINT8 wakeup_chan_mode; //RADIO_CHAN  or TV_CHAN
	UINT8	wakeup_group_idx;  
	//prog_id_t   wakeup_channel;//for TIMER_SERVICE_CHANNEL
	UINT32 wakeup_channel; //prog_id
	UINT8 wakeup_message; //TIMER_SERVICE_SMG
	UINT8 wakeup_state;
	UINT16 nvod_event_servceid;
	UINT16 nvod_event_id;
	UINT8	event_name[40];
} TIMER_SET_CONTENT; //050310-cmchen623

typedef struct{
	UINT8 timer_num;
	TIMER_SET_CONTENT TimerContent[MAX_TIMER_NUM];
} TIMER_SET_T;

typedef enum
{
    STANDBY_MODE_LOWPOWER = 0,
    STANDBY_MODE_UPGRADE,        
} STANDBY_MODE_T;

typedef struct
{
        UINT8   brightness;    /*range [0-9] refer to 10%,20%....90%,100%*/
        UINT8   contrast;      /*as above*/
        UINT8   saturation;    /*as above*/
}st_display_effect;

typedef struct
{
	/* First 4 byts */
	UINT32 bLNB_power: 1;
	UINT32 menu_lock: 1;
	UINT32 channel_lock: 1;
	UINT32 factory_reset: 1;
	UINT32 chan_sw: 2; /* CHAN_SWITCH_FREE / CHAN_SWITCH_SCRAMBLED / CHAN_SWITCH_ALL*/
	UINT32 bstandmode: 1;
	UINT32 bMuteSate: 1;

	UINT32 vcrloopmode: 1; /* VCRLOOP_VCR_MASTER / VCRLOOP_STB_MASTER */
	UINT32 poweroffmode: 1; /* STANDBY_MODE_LOWPOWER / STANDBY_MODE_UPGRADE */
	UINT32 rcupos: 2;
	UINT32 wakeup_timer: 8; /*used to store the index of the wakeup timer*/ 

	UINT32 channel_lock_level: 8;
	UINT32 volume: 8;

	/*Second 4 bytes */
	UINT32 chchgvideo_type: 1; /* CHAN_CHG_VIDEO_BLACK / CHAN_CHG_VIDEO_FREEZE */
	UINT32 install_beep: 1;
	UINT32 motor_lmt: 2;
	UINT32 reserved28b: 26; /* reserved */

	UINT8 antenna_connect_type; /* For two tuner only.  see TWOTUNER_ANTENNA_CONNECT_TYPE_T */
	UINT8 tuner_lnb_type[2]; /* LNB_FIXED / LNB_MOTOR : for two tuner*/
	//antenna_t 	tuner_lnb_antenna[2];			/* For two tuner: valid for LNB_MOTOR only */

	UINT8 menu_password[4];
	UINT32 chan_password;

	Language lang;
	AV_Set avset;
	Osd_Set osd_set;

	Local_Time local_time;
	TIMER_SET_T timer_set;
	date_time sys_dt;

#ifdef DATA_BROADCAST
#if(DATA_BROADCAST==DATA_BROADCAST_ENREACH)
    UINT8 enreach_scr_size;//0:720*576  1:640*526
#endif
#endif

#ifdef FAV_GROP_RENAME
	char favgrp_names[MAX_FAVGROUP_NUM][FAV_GRP_NAME_LEN + 1];
#endif

	/* Start up channel definition */
	UINT8 startup_switch;
	UINT8 startup_mode; /* RADIO_CHAN , TV_CHAN, 2 both enable,others not enabled*/
	UINT8 startup_group_index[2]; /* TV & Radio*/
	prog_id_t statup_chsid[2]; /* TV & Radio*/

	/* Current channel group information  */
	UINT8 cur_chan_mode; // 0 - radio, 1 - TV
	UINT8 cur_chan_group_index; // cur_chan_group_index in cur_chan_indexs and channel_groups
	UINT16 cur_sat_idx;
	
	/* group info
     * 0: all group    1-16: local groups    17: data 18: nvod  19----:fav
     */
	group_channel_t cur_chan_group[MAX_GROUP_NUM+MAX_FAVGROUP_NUM];
	UINT8 local_group_cnt;
	UINT16 local_group_id[MAX_LOCAL_GROUP_NUM];
	UINT16  local_group_name[MAX_LOCAL_GROUP_NUM][MAX_LOCAL_GROUP_LEN];
	UINT32 main_frequency;
	UINT32 main_symbol;
	UINT8 main_qam;
	UINT8 feed_power; //0: disable  1:enable
	UINT8 force_ota; //0: disable  1:enable manual ota
	st_display_effect display_effect;

	/* jinfeng added: NIT table version, if changed, research programs */
	UINT8 nit_version;

#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_DVN)
    dvn_email dvn_emails;
	UINT8 rating_control;
#elif (CAS_TYPE == CAS_CDCA)
	date_time smc_update_dt;
	UINT8 bUpd_status;
	UINT8 upd_smc_no[17]; //Upgrade smc NO.
#elif (CAS_TYPE == CAS_TF)
		date_time smc_update_dt;
		UINT8 bUpd_status;
		UINT8 upd_smc_no[17]; //Upgrade smc NO
#elif(CAS_TYPE==CAS_DVT)
	UINT32 pdsd;
#endif
#endif
	/*add for fast play last free service after power on*/
	UINT8 startup_service_valid;
	UINT8 startup_qam;
	UINT16 startup_pid_list[3];
	UINT32 startup_fre;
	UINT32 startup_sym;
	UINT8 startup_audio_chan;
	UINT8 startup_volumn;
    UINT8 startup_lock;
#ifdef AD_TYPE
	ad_sys_data ad_data;
#endif
#ifdef AD_SANZHOU
	UINT32 ad_frequency;
	UINT32 ad_symbol;
	UINT8 ad_qam;
#endif
#ifdef MIS_AD
	UINT32 mis_frequency;
	UINT32 mis_symbol;
	UINT8 mis_qam;
	UINT8 mis_para[16];
	UINT8 area_code;
#endif	
} SYSTEM_DATA;

extern SYSTEM_DATA system_config;


/*******************************************************************************
 * system data: load / save / get / init APIs
 ********************************************************************************/
void sys_data_load(void);
void sys_data_save(INT32 block);
SYSTEM_DATA *sys_data_get(void);
void sys_data_factroy_init(void);

/*******************************************************************************
 * system data: Channel group operation APIs
 ********************************************************************************/
/* Get & Set current channel mode */
UINT8 sys_data_get_cur_chan_mode(void);
void sys_data_set_cur_chan_mode(UINT8 chan_mode);

/* Sync group information with database  */
void sys_data_check_channel_groups(void);

/* Get channel group count */
UINT8 sys_data_get_group_num(void);
UINT8 sys_data_get_sate_group_num(UINT8 av_flag);
UINT8 sys_data_get_fav_group_num(UINT8 av_flag);

/* Get & Set current group index */
UINT8 sys_data_get_cur_group_index(void);
void sys_data_set_cur_group_index(UINT8 group_idx);
UINT8 sys_data_get_cur_intgroup_index(void);
void sys_data_set_cur_intgroup_index(UINT8 intgroup_idx);

UINT8 sys_data_cur_group_inc_dec(BOOL inc, UINT32 flag);

UINT8 sys_data_group_inc_dec(UINT8 grp_idx, BOOL inc, UINT32 flag);


/* Get & set group's play channel index)  */
UINT16 sys_data_get_cur_group_cur_mode_channel(void);
void sys_data_set_group_channel(UINT8 group_idx, UINT16 channel);
void sys_data_get_group_channel(UINT8 group_idx, UINT16 *channel, UINT8 av_flag);
void sys_data_set_cur_group_channel(UINT16 channel);
void sys_data_get_cur_group_channel(UINT16 *channel, UINT8 av_flag);
BOOL sys_data_get_sate_group_channel(UINT16 sate_pos, UINT16 *channel, UINT8 av_flag);
BOOL sys_data_get_fav_group_channel(UINT8 fav_group, UINT16 *channel, UINT8 av_flag);




/* Get & set specified group infor(channel index)  */
BOOL sys_data_get_cur_mode_group_infor(UINT8 group_idx, UINT8 *group_type, UINT8 *group_pos, UINT16 *channel);

/* Change group index */
INT32 sys_data_change_group(UINT8 group_idx);
BOOL sys_data_get_curprog_info(P_NODE *cur_prog_node);
/*******************************************************************************
 * system data: Extend APIs
 ********************************************************************************/

/* Get & Set Menu/Chan password & lock*/
BOOL sys_data_get_menu_lock(void);
BOOL sys_data_get_channel_lock(void);
UINT8 *sys_data_get_menu_password(void);
UINT32 sys_data_get_chan_password(void);

void sys_data_set_menu_lock(BOOL lock);
void sys_data_set_channel_lock(BOOL lock);
void sys_data_set_menu_password(UINT8 *password);
void sys_data_set_chan_password(UINT32 password);


/* Get & set UTC offset */
UINT8 sys_data_get_utcoffset(void);
UINT8 sys_data_set_utcoffset(UINT8 offset);
void sys_data_gmtoffset_2_hmoffset(INT32 *hoffset, INT32 *moffset);


/* Set OSD language */
void sys_data_select_language(UINT8 langid);
/* Set Audio language */
void sys_data_select_audio_language(UINT8 langid1, UINT8 langid2);


/*Get & set factory reset */
BOOL sys_data_get_factory_reset();
void sys_data_set_factory_reset(BOOL bset);



void sys_data_set_palette(UINT8 flag);
void sys_data_set_display_mode(AV_Set *av_set);
void sys_data_set_rf(UINT8 rf_mode, UINT8 rf_channel);
UINT16 sys_data_get_cur_satidx(void);
void sys_data_set_cur_satidx(UINT16 sat_idx);
void do_set_display_effect(UINT8 type,UINT8 value);
void sys_data_load_display_effect(BOOL b_scal);
#ifdef AD_TYPE
void sys_data_get_advdata(ad_sys_data *pData);
void sys_data_set_advdata(ad_sys_data *pData);
#endif
typedef enum
{
	ENUM_DISP_BRIGHTNESS= 0,
	ENUM_DISP_CONTRAST,
	ENUM_DISP_SATURATION,
}enum_disp_effect;

#endif //_SYSTEM_DATA_H_
