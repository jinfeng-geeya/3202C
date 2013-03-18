#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include <hld/hld_dev.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/decv/vdec_driver.h>
#include <hld/dis/vpo.h>
#include <hld/ge/ge.h>
#include <hld/pan/pan.h>
#include <hld/pan/pan_dev.h>
#include <hld/dmx/dmx.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_pub.h>
#include <api/libtsi/si_monitor.h>
#include <api/libsi/si_eit.h>
#include <api/libsi/lib_epg.h>
//#include <api/libosd/osd_lib.h>
#include <api/libchunk/chunk.h>
//#include <api/libosd/osd_vkey.h>
#include <api/libmp/pe.h>
#include <api/libsi/si_service_type.h>
#ifdef TS_MONITOR_FEATURE
#include <api/libsi/ts_monitor.h>
#include "ts_search/ts_search.h"
#endif
#include <api/libstbinfo/stb_info_data.h>

#ifdef SHOW_LOGO_ON_GE
#include "test_functions/show_logo_on_ge.h" 
#endif

#if ((TTX_ON == 1)||(SUBTITLE_ON == 1)) 
#include <hld/vbi/vbi.h>
#include <api/libttx/ttx_osd.h>
#include <api/libttx/lib_ttx.h>

#include <hld/sdec/sdec.h>
#include <api/libsubt/lib_subt.h>
#endif
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_TF)
#include <api/libcas/tf/cas_tf.h>
#elif(CAS_TYPE==CAS_CDCA)
#include <api/libcas/cdca/CDCAS30.h>
#elif(CAS_TYPE==CAS_GY)
#include <api/libcas/gy/cas_gy.h>
#elif (CAS_TYPE == CAS_DVT)
#include <api/libcas/dvt/cas_dvt.h>
#elif(CAS_TYPE == CAS_IRDETO)
#include <api/libcas/irca/irca_sys.h>
#elif(CAS_TYPE == CAS_CONAX)
#include "conaxca/win_ca_mmi.h"
#endif
#endif

#ifdef AUDIO_SPECIAL_EFFECT
#include "beep_mp2.h"
#endif

#include "osd_config.h"
#include "string.id"
#include "images.id"

#include "key.h"
#include "osd_rsc.h"


#include "win_com_popup.h"
#include "menus_root.h"
#include "win_com.h"

#include "win_mute.h"
#include "win_pause.h"
#include "win_signalstatus.h"

#include "control.h"


/*******************************************************
 * macro define
 ********************************************************/

#define CONTROL_MBF_NAME	"mbcn"
#define CONTROL_MBF_SIZE		30

#define CONTROL_TASK_STACKSIZE	0x2000
#define CONTROL_TASK_QUANTUM		10

#ifdef NVOD_FEATURE
#define WIN_NVOD_WARN       (POBJECT_HEAD)&g_win_nvod_warn
#define WIN_NVOD_EVENT_NAME (POBJECT_HEAD)&g_win_nvod_event_name
#endif
#define WIN_VOLUME (POBJECT_HEAD)&g_win_volume

#define MAIN_MENU_HANDLE		((POBJECT_HEAD)&g_win_mainmenu)
#define CHANNEL_BAR_HANDLE	((POBJECT_HEAD)&g_win_progname)

#define SEARCH_HANDLE           ((POBJECT_HEAD)&g_win_search)

#if ((TTX_ON == 1)||(SUBTITLE_ON == 1)) 
#define EPG_HANDLE		((POBJECT_HEAD)&g_win_epg)
#define CHANNEL_LIST_HANDLE	((POBJECT_HEAD)&g_win_chanlist)
#endif

/*******************************************************
 * struct and variable
 ********************************************************/
OSAL_ID control_mbf_id;
ScreenBackState_t screen_back_state = SCREEN_BACK_VIDEO;
sys_state_t system_state = SYS_STATE_INITIALIZING;

BOOL timer_wakeup_from_control = FALSE;
BOOL timer_message_showed = FALSE;
UINT8 tv_sat_state = 1;
UINT8 play_chan_nim_busy;
msg_backlog_t * msg_back_log_head;


typedef struct
{
	UINT8 mode;
	UINT8 group;
	UINT16 channel;
}
channel_t;

channel_t pre_played_channel =
{
        0xFF,
        0xFF,
        0xFFFF
};
struct channel_info recent_channels[2];


char g_cc_step_name[][23] =
{ "",
"Get key ",
"Key proc ",
"In Api_play_channel ",
"Get Channel ",
"Set Channel Info",
"Check NIM",
"Pre Play callback",
"Close VPO",
"Stop Audio",
"Stop Video",
"Select Vdec",
"Stop DMX",
"Pre Tuner callback",
"Set NIM",
"Post Tuner callback",
"VPO IO CHANCHNG",
"Start DMX",
"Start Video",
"Start Audio"
};
UINT32 g_cc_play_step = 0;
/*******************************************************
 * extern declare
 ********************************************************/
extern UINT8 show_and_playchannel;
extern IR_Key_Map_t ir_key_maps[];
extern BOOL rise_from_standby;
extern TIMER_SET_CONTENT 	nvodTimerContent[];
extern enum signal_flag signal_detect_flag;


/*******************************************************
 * functions declare
 ********************************************************/
extern void SetNVODTimerMode(UINT8 index,UINT8 mode);
static void ap_control_task(void);
extern void SetUsbMountFlag(BOOL flag);
extern void dvn_cas_open_popup();

extern void set_ota_force_flag(BOOL flag);
extern BOOL GetNvodPlayFlag();

#if(defined(MIS_AD) || defined(MIS_AD_NEW))
extern BOOL cas_osdmsg_init(void);
#endif

void reset_group();
void ap_key_commsg_proc(UINT32 msg_type, UINT32 key);
void ap_timer_message_proc(UINT32 msg_type, UINT32 msg_code);
void ap_comand_message_proc(UINT32 msg_type, UINT32 msg_code);
void ap_signal_messag_proc(UINT32 msg_type, UINT32 msg_code);
void ap_OTA_upgrade(UINT32 type);
void ap_nit_update_notify(UINT32 nit);
void ap_manual_upgrade(UINT32 type);
void api_stop_play( BOOL blk_srn );
UINT8 NimDebugKeyProc(UINT32 vkey);
void usb_fs_callback(UINT32 event_type,UINT32 param);
INT32 system_init_process(void);
UINT32 OTASpecialCB(UINT32 param1,UINT32 parma2);
void ap_timer_expire_proc(UINT32 msg_type, UINT32 msg_code);
UINT16 find_valid_channel(TIMER_SET_CONTENT *timer);
#if(TTX_ON==1)
UINT8 TTX_KeyMap(UINT32 msgtype, UINT32 msgcode);
void TTX_OSDOpen(void);
#endif


/*******************************************************
 * external APIs
 ********************************************************/

/////////////////////////////////////////////////////////////
// functions for get keys 
///
void ap_set_system_state(INT32 state)
{
	system_state = state;
}
UINT32 ap_hk_to_vk(UINT32 start, UINT32 key_info, UINT32 *vkey)
{
	UINT16 i, j, n;
	UINT16 key_count;

	*vkey = V_KEY_NULL;

	for (j = 0; j < IRP_NUM; j++)
	{
		for (i = start; i < ir_key_maps[j].irkey_num; i++)
		{
			key_count = (ir_key_maps[j].key_map[i].key_info & 0xFF0000);
			if (((ir_key_maps[j].key_map[i].key_info & 0xFF00FFFF) == (key_info & 0xFF00FFFF))
			        && (key_count == (key_info & 0xFF0000) || (0 == key_count)))
			{
				*vkey = ir_key_maps[j].key_map[i].ui_vkey;
				break;
			}
		}
	}
	
	return (i + 1);
}

UINT32 ap_vk_to_hk(UINT32 start, UINT32 vkey, UINT32 *key_info)
{
	UINT16 i, j, n;
	UINT16 key_count;

	*key_info = INVALID_HK;

	for (j = 0; j < IRP_NUM; j++)
	{
		for (i = start; i <  ir_key_maps[j].irkey_num; i++)
		{
			if (ir_key_maps[j].key_map[i].ui_vkey == vkey)
			{
				*key_info = ir_key_maps[j].key_map[i].key_info;
				break;
			}
		}
	}
	
	return (i + 1);
}

UINT8 ap_get_vk(void)
{
	UINT32 msg_code, vkey;

GET_HK:
	msg_code = ap_get_key_msg();
	if (msg_code == INVALID_HK)
		goto GET_HK;


	if (msg_code != INVALID_HK && msg_code != INVALID_MSG)
	{
		ap_hk_to_vk(0, msg_code, &vkey);

		return (UINT8)vkey;
	}
	else
		return V_KEY_NULL;
}

UINT32 ap_get_hk(void)
{
	OSAL_ER retVal;
	ControlMsg_t msg;
	UINT32 msgSize;
	UINT32 hkey = INVALID_HK;

	retVal = osal_msgqueue_receive(&msg, &msgSize, control_mbf_id, 100);
	if (OSAL_E_OK != retVal)
		return INVALID_MSG;

	if (msg.msgType <= CTRL_MSG_TYPE_KEY)
		return msg.msgCode;
	else
		return INVALID_HK;
}
/////////////////////////////////////////////////////////////
//  osd and menu funcionts
//
void CreateSubtitleRegion()
{
/*
    struct OSDRect region2Rect;
	//g_osd_dev = ( struct osd_device * ) dev_get_by_id ( HLD_DEV_TYPE_OSD, 0 );

	//top subtitle
	region2Rect.uLeft = 0;
	region2Rect.uTop = 10; //0;
	region2Rect.uWidth = 720;
	region2Rect.uHeight = 50;
	OSDDrv_CreateRegion((HANDLE)g_osd_dev, 1, &region2Rect, NULL);

	//bottom subtitle
	region2Rect.uLeft = 0;
	region2Rect.uTop = 510; //0;
	region2Rect.uWidth = 720;
	region2Rect.uHeight = 50;
	OSDDrv_CreateRegion((HANDLE)g_osd_dev, 2, &region2Rect, NULL);
*/
}

void ap_osd_init()
{
#ifdef USE_LIB_GE
	struct ge_device *ge_dev;
	GUI_APP_CFG osd_cfg;
	GUI_RSC_FNC osd_rsc_info;
	ge_anti_flick_t af;
	GUI_RGN_CFG rgn_list[2],deo_lst;
	PGUI_REGION rgnplug;

	MEMSET(&osd_rsc_info, 0, sizeof(osd_rsc_info));
	osd_rsc_info.osd_get_lang_env = OSD_GetLangEnvironment;
	osd_rsc_info.osd_get_obj_info = OSD_GetObjInfo;
	osd_rsc_info.osd_get_rsc_data= OSD_GetRscObjData;
	osd_rsc_info.osd_rel_rsc_data= OSD_ReleaseObjData;
	osd_rsc_info.osd_get_font_lib = OSD_GetDefaultFontLib;
	osd_rsc_info.osd_get_win_style = OSDExt_GetWinStyle;
	osd_rsc_info.osd_get_str_lib_id = OSDExt_GetMsgLibId;
	osd_rsc_info.ap_hk_to_vk = ap_hk_to_vk;
	
	app_rsc_init(&osd_rsc_info);

	ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
    
	if(RET_SUCCESS!=ge_open(ge_dev))
	{
		PRINTF("ge_open failed!!\n");
		ASSERT(0);
	}

	MEMSET(&osd_cfg,0xFF,sizeof(osd_cfg));
	MEMSET(&osd_cfg.layer_color,GUI_INVALIDFORMAT,sizeof(&osd_cfg.layer_color));
	osd_cfg.hdev = (GUI_DEV)ge_dev;

#ifdef USE_16BIT_OSD
	osd_cfg.layer_color[GUI_GMA2_SURF] = GUI_ARGB1555;//GUI_CLUT8;//
	osd_cfg.layer_color[GUI_MEMVSCR_SURF] = GUI_ARGB1555;
#else
    osd_cfg.layer_color[GUI_GMA2_SURF] = GUI_CLUT8;
	osd_cfg.layer_color[GUI_MEMVSCR_SURF] = GUI_CLUT8;
#endif
	osd_cfg.layer_rect[GUI_GMA2_SURF].uStartX = (720 - OSD_MAX_WIDTH)>>1;;
	osd_cfg.layer_rect[GUI_GMA2_SURF].uStartY = (576-OSD_MAX_HEIGHT) >> 1;
	osd_cfg.layer_rect[GUI_GMA2_SURF].uWidth = OSD_MAX_WIDTH;
	osd_cfg.layer_rect[GUI_GMA2_SURF].uHeight = OSD_MAX_HEIGHT;

	osd_cfg.layer_rect[GUI_MEMVSCR_SURF].uStartX = (720 - OSD_MAX_WIDTH)>>1;;
	osd_cfg.layer_rect[GUI_MEMVSCR_SURF].uStartY = (576-OSD_MAX_HEIGHT) >> 1;
	osd_cfg.layer_rect[GUI_MEMVSCR_SURF].uWidth = OSD_MAX_WIDTH;
	osd_cfg.layer_rect[GUI_MEMVSCR_SURF].uHeight = OSD_MAX_HEIGHT;

	osd_cfg.direct_draw = FALSE;
	osd_cfg.animation_support = 0;

	MEMSET(rgn_list,0x0,sizeof(rgn_list));
	rgn_list[0].layer_idx = GUI_GMA2_SURF;
	rgn_list[0].rgn_idx = 0;
	rgn_list[0].rect.uStartX = (720 - OSD_MAX_WIDTH)>>1;
	rgn_list[0].rect.uStartY = (576-OSD_MAX_HEIGHT) >> 1;
	rgn_list[0].rect.uWidth = OSD_MAX_WIDTH;
	rgn_list[0].rect.uHeight = OSD_MAX_HEIGHT;
/*	
	rgn_list[1].layer_idx = GUI_GMA2_SURF;
	rgn_list[1].rgn_idx = 0;
	rgn_list[1].rect.uStartX = 100;
	rgn_list[1].rect.uStartY = 100;
	rgn_list[1].rect.uWidth = 128;
	rgn_list[1].rect.uHeight = 128;
	
	rgn_list[2].layer_idx = GUI_GMA2_SURF;
	rgn_list[2].rgn_idx = 1;
	rgn_list[2].rect.uStartX = 256;
	rgn_list[2].rect.uStartY = 256;
	rgn_list[2].rect.uWidth = 128;
	rgn_list[2].rect.uHeight = 128;
*/	
 
	rgn_list[1].layer_idx = GUI_MEMVSCR_SURF;
	rgn_list[1].rgn_idx = 0;
	rgn_list[1].rect.uStartX = (720 - OSD_MAX_WIDTH)>>1;
	rgn_list[1].rect.uStartY = (576-OSD_MAX_HEIGHT) >> 1;
	rgn_list[1].rect.uWidth = OSD_MAX_WIDTH;
	rgn_list[1].rect.uHeight = OSD_MAX_HEIGHT;//multi region info,rect & parent layer & region id etc.	

	osd_cfg.rgn_cnt = 2;
    
	GUI_INIT_PARAM gui_init;

	MEMSET(&gui_init,0x0,sizeof(gui_init));
	gui_init.ge_version = GE_MODULE_M3202;
	gui_init.gui_lines = GUI_PAL_LINES;
    
	gelib_init((GUI_DEV)ge_dev,&gui_init);
	app_gui_init(rgn_list,(UINT32)&osd_cfg);
	//gelib_setkeycolor(GUI_ARGB1555,0xfc1f);

	OSD_RegionInit2();//use GUI_GMA1_SURF  //init for plugin compatible APIs

	PGUI_VSCR pvsr = OSD_GetTaskVscr(osal_task_get_current_id());
	OSD_SetVscrLayer(pvsr,GUI_MEMVSCR_SURF,0,GUI_GMA2_SURF,0);
	OSD_SetVscrDst(GUI_GMA2_SURF,0);

    	//enable anti-flick
	/*if (sys_ic_get_rev_id() >= IC_REV_2)//m3202
	{
		af.layer = GUI_GMA1_SURF;
		af.valid = 1;
		ge_io_ctrl(ge_dev,GE_IO_ANTI_FLICK_29E,(UINT32)&af);
		af.layer = GUI_GMA2_SURF;
		af.valid = 1;
		ge_io_ctrl(ge_dev,GE_IO_ANTI_FLICK_29E,(UINT32)&af);
	}*/
#else
	struct OSD_RegionInfo osd_region_info;
	struct OSD_RscInfo osd_rsc_info;

	g_osd_dev = (struct osd_device*)dev_get_by_id(HLD_DEV_TYPE_OSD, 0);

	osd_region_info.osddev_handle = (HANDLE)g_osd_dev;
	osd_region_info.tOpenPara.eMode = OSD_256_COLOR;
	osd_region_info.tOpenPara.uGAlphaEnable = IF_GLOBAL_ALPHA;
	osd_region_info.tOpenPara.uGAlpha = 0x0F;
	osd_region_info.tOpenPara.uPalletteSel = 0;
	osd_region_info.osdrect.uLeft = (720-OSD_MAX_WIDTH) >> 1;
	osd_region_info.osdrect.uTop = (576-OSD_MAX_HEIGHT) >> 1;
	osd_region_info.osdrect.uWidth = OSD_MAX_WIDTH;
	osd_region_info.osdrect.uHeight = OSD_MAX_HEIGHT;

	osd_rsc_info.osd_get_lang_env = OSD_GetLangEnvironment;
	osd_rsc_info.osd_get_obj_info = OSD_GetObjInfo;
	osd_rsc_info.osd_get_rsc_data = OSD_GetRscObjData;
	osd_rsc_info.osd_rel_rsc_data = OSD_ReleaseObjData;
	osd_rsc_info.osd_get_font_lib = OSD_GetDefaultFontLib;
	osd_rsc_info.osd_get_win_style = OSDExt_GetWinStyle;
	osd_rsc_info.osd_get_str_lib_id = OSDExt_GetMsgLibId;
	osd_rsc_info.ap_hk_to_vk = ap_hk_to_vk;

	OSD_RegionInit(&osd_region_info, &osd_rsc_info);

	//CreateSubtitleRegion();
#endif

}

void ap_clear_all_menus(void)
{
	UINT32 i, n;
	POBJECT_HEAD rootmenu, menu;
	PRESULT proc_ret;
	UINT32 osd_msg_type, hwkey;
	OSD_RECT rect;
	SYSTEM_DATA *sys_data;
	enum OSD_MODE osd_mode = 0xFFF;


	osd_msg_type = MSG_TYPE_MSG << 16;
	ap_vk_to_hk(0, V_KEY_EXIT, &hwkey);


	timer_wakeup_from_control = TRUE;

	if (hde_get_mode() == VIEW_MODE_PREVIEW)
		hde_set_mode(VIEW_MODE_FULL);
	else if (hde_get_mode() == VIEW_MODE_MULTI)
	{
		menu = menu_stack_get_top();
		OSD_ObjProc(menu, (MSG_TYPE_KEY << 16), hwkey, 0);
		menu_stack_pop();
	}

	osd_mode = api_osd_mode_change(OSD_WINDOW);

	/* Hide OSD */
	//OSD_ShowOnOff(OSDDRV_OFF);

	rootmenu = menu_stack_get(0); /* root menu*/
	menu = menu_stack_get_top(); /* current menu*/


	/*If there menu is on the screen,send EXIT key to it to exit*/
	i = 0;
	n = 0;
	while (menu != NULL && (i < 100))
	{
		i++;
		n++;
		proc_ret = OSD_ObjProc(menu, (MSG_TYPE_KEY << 16), hwkey, 0);
		/* If a window exit or try to exit for 3 times with no response,
		force it to exit. (For example,when no channel,main menu will not exit in normal case.)
		 */
		if (proc_ret == PROC_LEAVE || n >= 3)
		{
			n = 0;
			menu_stack_pop();
		}
		menu = menu_stack_get_top();
	}
	timer_wakeup_from_control = FALSE;

	if (i >= 100)
	{
		PRINTF("==============For timer msg,exit menu timeout!!!!============\n");
		/* Force all menu exit.*/
		menu_stack_pop_all();
	}
#ifdef USE_LIB_GE
    OSD_ClearScrn(NULL);
#else
	/* Clear OSD */
	OSD_GetRectOnScreen(&rect);
	rect.uLeft = rect.uTop = 0;
	OSDDrv_RegionFill((HANDLE)g_osd_dev, 0, &rect, OSD_TRANSPARENT_COLOR);

#endif
	timer_wakeup_from_control = FALSE;
}


//////////////////////////////////////////////////////////////
//    control message functions
//
void ap_clear_all_message(void)
{
	ER retVal;
	ControlMsg_t msg;
	UINT32 msgSize;

	do
	{
		retVal = osal_msgqueue_receive((VP) &msg, &msgSize, control_mbf_id, 0);
	}
	while (retVal == E_OK);
}

BOOL ap_send_msg(ControlMsgType_t msg_type, UINT32 msg_code, BOOL if_clear_buffer)
{
	ER ret_val;
	ControlMsg_t control_msg;

	control_msg.msgType = msg_type;
	control_msg.msgCode = msg_code;

	ret_val = osal_msgqueue_send(control_mbf_id, &control_msg, sizeof(ControlMsg_t), 0);
	if (OSAL_E_OK != ret_val)
	{
		if (if_clear_buffer)
		{
			ap_clear_all_message();
			ret_val = osal_msgqueue_send(control_mbf_id, &control_msg, sizeof(ControlMsg_t), 0);
            if(OSAL_E_OK != ret_val)
                return FALSE;
            else 
                return TRUE;
		}
		else
			return FALSE;
	}
	else
		return TRUE;
}

BOOL ap_send_msg_expand(ControlMsgType_t msg_type,INT32 msg_sbtype, UINT32 msg_code, BOOL if_clear_buffer)
{
	ER ret_val;
	ControlMsg_t control_msg;

	control_msg.msgType = msg_type;
	control_msg.msgSbuType = msg_sbtype;
	control_msg.msgCode = msg_code;

	ret_val = osal_msgqueue_send(control_mbf_id, &control_msg, sizeof(ControlMsg_t), 0);
	if (OSAL_E_OK != ret_val)
	{
		if (if_clear_buffer)
		{
			ap_clear_all_message();
			ret_val = osal_msgqueue_send(control_mbf_id, &control_msg, sizeof(ControlMsg_t), 0);
		}
		else
			return FALSE;
	}
	else
		return TRUE;
}

///////////////////////////////////////////////////////////////////
//  some  callbacks 
//
void time_refresh_callback(void)
{
	static UINT32 count = 0;
	date_time dt;
	count++;

	if (system_state != SYS_STATE_UPGRAGE_HOST /* && sys_data_get_time_state()*/)
		ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE, 0, FALSE);

	if (count % 3 == 0)
	{
		api_timers_proc();
	}
}

void ap_epg_call_back(UINT32 tp_id, UINT16 service_id, UINT8 event_type)
{
	if(event_type == EPG_FULL_EVENT)
	{
		ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EPG_FULL,CTRL_MSG_SUBTYPE_CMD_EPG_FULL,FALSE);
		return;
	}

	if (epg_check_active_service(tp_id, service_id))
	{
		if (event_type == SCHEDULE_EVENT)
			ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EPG_SCH_UPDATED, CTRL_MSG_SUBTYPE_CMD_EPG_SCH_UPDATED, FALSE);
		else
			ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EPG_PF_UPDATED, CTRL_MSG_SUBTYPE_CMD_EPG_PF_UPDATED, FALSE);
	}
}

#ifdef PSI_MONITOR_SUPPORT
void ap_pid_change(BOOL need_chgch)
{
	if (need_chgch)
		ap_send_msg(CTRL_MSG_SUBTYPE_CMD_PIDCHANGE, need_chgch, FALSE);
}

#endif

UINT32 OTASpecialCB(UINT32 param1,UINT32 parma2)
{
    ap_send_msg(param1, 0, FALSE);
}
//////////////////////////////////////////////////////////////////
//get keys when in some windows  such as  popup window
//need to get and pass commonds from control buffer because control task
//may stop when in these windows
////////////////////////////////////////////////////////////////////
UINT32 ap_get_key_msg(void)
{
	OSAL_ER retVal;
	ControlMsg_t msg;
	UINT32 i, msgSize;
	OSD_RECT rect;
	SYSTEM_DATA *sys_data;
	UINT32 idx = 0;
	UINT32 hkey = INVALID_HK;
	UINT32 vkey;
	TIMER_SET_CONTENT *timer;

	sys_data = sys_data_get();

	retVal = osal_msgqueue_receive(&msg, &msgSize, control_mbf_id, 100);
	if (OSAL_E_OK != retVal)
		return INVALID_MSG;

#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_TF)
	else if ((msg.msgType == CTRL_MSG_SUBTYPE_STATUS_MCAS)
			&& ((msg.msgCode>>24) == TFCA_MSG_BUYMSG))
	{
		ap_cas_message_setid(msg.msgCode&0xff);
	}
#elif (CAS_TYPE == CAS_CDCA)
	else if ((msg.msgType == CTRL_MSG_SUBTYPE_STATUS_MCAS)
			&& ((msg.msgCode>>24) == CAS_MSG_BUYMSG))
	{
		ap_cas_message_setid(msg.msgCode&0xff);
	}
#elif (CAS_TYPE == CAS_DVT || (CAS_TYPE==CAS_GY))
	if ((msg.msgType == CTRL_MSG_SUBTYPE_STATUS_MCAS)
		&& ((msg.msgCode>>24 == CAS_MSG_SHOWPROMPTMSG)
			||(msg.msgCode>>24 == CAS_MSG_HIDEPROMPTMSG)))
	{
		ap_cas_message_proc(msg.msgType, msg.msgCode);
	}
#elif (CAS_TYPE == CAS_DVN)
	if((msg.msgType == CTRL_MSG_SUBTYPE_STATUS_MCAS)
     &&(msg.msgCode>>24 == CAS_DISP_OSD)
     &&(1 == win_compopup_get_status()))
    {
		//DVN CA: set the flag of overlapping the OLD Osd displaying content
		win_compopup_set_status(2);
    }
#elif (CAS_TYPE == CAS_ABEL)
	else if ((msg.msgType == CTRL_MSG_SUBTYPE_STATUS_MCAS))
	{
		if(ap_cas_message_proc(msg.msgType, msg.msgCode)== PROC_LEAVE)
		{
			ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
		}
	}	
#endif
#endif

	if (msg.msgType <= CTRL_MSG_TYPE_KEY)
	{
		ap_hk_to_vk(0, msg.msgCode, &vkey);

		if (vkey == V_KEY_POWER)
		{
			if (system_state != SYS_STATE_UPGRAGE_HOST)
				power_switch(0);
			else
				hkey = msg.msgCode;
		}
		else
			hkey = msg.msgCode;

		return hkey;
	}

	/* Timer wakeup */
	if ( msg.msgType == CTRL_MSG_SUBTYPE_CMD_TIMER_WAKEUP
	        || msg.msgType == CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE
	        || msg.msgType == CTRL_MSG_SUBTYPE_CMD_SLEEP
	        || msg.msgType == CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMER_WAKEUP
	        || timer_wakeup_from_control == TRUE )
	{
        if (system_state == SYS_STATE_UPGRAGE_HOST)
		 /* System in upgrade mode */
		{
			/* Discard the timer message */
		}
		else if (system_state == SYS_STATE_NORMAL
			|| system_state == SYS_STATE_NVOD)
		 /*Normal mode */
		{


			if (timer_message_showed)
			{
				ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
				ap_send_msg(msg.msgType, msg.msgCode, TRUE);
				timer_message_showed = FALSE;
				return hkey;
			}

			if (msg.msgType == CTRL_MSG_SUBTYPE_CMD_TIMER_WAKEUP)
			{
				timer = &sys_data->timer_set.TimerContent[msg.msgCode];
				if (timer->timer_service == TIMER_SERVICE_MESSAGE)
				{
					timer->wakeup_state = TIMER_STATE_READY;
					if (as_service_query_stat() == ERR_BUSY)
						return hkey;
				}else if(timer->timer_service == TIMER_SERVICE_CHANNEL)
				{
					ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
					ap_send_msg(msg.msgType, msg.msgCode, TRUE);
					return hkey;
				}
			}

			if (msg.msgType == CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMER_WAKEUP)
			{
				ap_vk_to_hk(0, V_KEY_EXIT, &hkey);
				ap_send_msg(msg.msgType, msg.msgCode, TRUE);
				return hkey;
			}

			/* Hide OSD */
			//OSD_ShowOnOff(OSDDRV_OFF);

			/* 1. Send EXIT key to exit current popup mode */
			ap_vk_to_hk(0, V_KEY_EXIT, &hkey);

			/* 2. Re-post the timer message */
			if (timer_wakeup_from_control != TRUE)
				ap_send_msg(msg.msgType, msg.msgCode, TRUE);
		}
		else
		{
			/* Discard the timer message */
		}
	}

	return hkey;
}


void ap_osd_exit(void)
{
	g_osd_dev = (struct osd_device*)dev_get_by_id(HLD_DEV_TYPE_OSD, 0);
	OSDDrv_Close((HANDLE)g_osd_dev);

	g_osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
	if(g_osd_dev!=NULL)
		OSDDrv_Close((HANDLE)g_osd_dev);	
}


////////////////////////////////////////////////////
//control task
////////////////////////////////////////////////////
BOOL ap_task_init(void)
{
	ID control_task_id; //= OSAL_INVALID_ID;
	OSAL_T_CMBF t_cmbf;
	OSAL_T_CTSK t_ctsk;

	//t_cmbf.mbfatr = TA_TFIFO;
	t_cmbf.bufsz = CONTROL_MBF_SIZE * sizeof(ControlMsg_t);
	t_cmbf.maxmsz = sizeof(ControlMsg_t);
	t_cmbf.name[0] = 'c';
	t_cmbf.name[1] = 't';
	t_cmbf.name[2] = 'l';

	control_mbf_id = OSAL_INVALID_ID;
	control_task_id = OSAL_INVALID_ID;
	control_mbf_id = osal_msgqueue_create(&t_cmbf);
	if (OSAL_INVALID_ID == control_mbf_id)
	{
		PRINTF("cre_mbf control_mbf_id failed\n");
		return FALSE;
	}

	t_ctsk.stksz = CONTROL_TASK_STACKSIZE * 4;
	t_ctsk.quantum = CONTROL_TASK_QUANTUM;
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.name[0] = 'C';
	t_ctsk.name[1] = 'T';
	t_ctsk.name[2] = 'L';
	t_ctsk.task = (FP)ap_control_task;
	control_task_id = osal_task_create(&t_ctsk);
	if (OSAL_INVALID_ID == control_task_id)
	{
		//delete control_mbf_id
		PRINTF("cre_tsk control_task_id failed\n");
		return FALSE;
	}

	return TRUE;
}

#ifdef TS_MONITOR_FEATURE
static void on_ts_service_changed(DB_TP_ID tp_id)
{
	ap_send_msg(CTRL_MSG_SUBTYPE_CMD_TP_SERVICE_CHANGED, tp_id, FALSE);
}
#endif

extern BOOL cur_mute_state;
static void ap_control_task(void)
{
	ER retVal;

	ControlMsg_t msg;
	UINT32 msgSize;
	PRESULT proc_ret = VACT_PASS;
	UINT32 vkey;

	struct VDec_StatusInfo curStatus;
	UINT8 video_Src, tv_out;
	SYSTEM_DATA *sys_data;

	UINT32 prog_num;
	UINT32 cur_channel=0xffffffff;
	INT32 ret,iprob;
	POBJECT_HEAD rootmenu, menu;
	INT32 i, n;
	UINT32 vcr_detect = 0, vcr_on = 0;
	static UINT32 itmp=0;
	TIMER_SET_CONTENT * timer_cont = NULL;


	
	/* system initializaton */
	ret = system_init_process();
	
	sys_data = sys_data_get();
	
/* for fast run
#ifndef GEEYA_AD
	GYSTBAD_Start();
	while(GYSTBAD_End())
	{
		osal_task_sleep(1000);  
		gyad_check_data_timeout();
	}
	gyad_check_data_timeout();
#endif
*/

	#ifdef AD_TYPE
	extern void menu_open_callback(POBJECT_HEAD pobj);
	extern void menu_close_callback(POBJECT_HEAD pobj);
	AD_Init();
	OSD_RegOpenNotifyFunc(menu_open_callback);
	OSD_RegCloseNotifyFunc(menu_close_callback);
	#endif

    #ifdef ALI_DEMO_AD  
    ali_ad_init();
    #endif
#ifdef AD_SANZHOU
	//ad_init();
	ad_update();
	szxc_ad_show_corner();
#endif
#ifdef MULTI_CAS
//	cas_init();
#endif

//show 32bit or 16bit pic when showing logo
#ifdef SHOW_LOGO_ON_GE
    capture_frame_demo();
//    osal_task_sleep(2000); //for watching the result, no special use
    osd_layer_close();
	OSDDrv_DeleteRegion(g_osd_dev, 0);   
	OSDDrv_Close(g_osd_dev);   	
    ap_osd_init();
#endif
	//开机应用
#ifdef MIS_AD
	MIS_Main();
	Mis_Set_SameChan_AdvShowOnce(TRUE);
#endif

	ota_callback_register(OTASpecialCB, 0);
	if (sys_data->force_ota == 1)
	{
		//libc_printf("***find force ota flag\n");
		sys_data->force_ota = 0;
		sys_data_save(0);
	}

#ifdef TS_MONITOR_FEATURE    
	ts_monitor_register(on_ts_service_changed);
#endif
       extern void sdt_monitor_on(UINT32 index);
       sdt_monitor_on(0);

	if(-1==ret)
	{
		timer_cont = &sys_data->timer_set.TimerContent[sys_data->wakeup_timer];
		cur_channel = timer_cont->wakeup_channel;
		libc_printf("sys_data->wakeup_timer=%d\n",sys_data->wakeup_timer);
#ifdef NVOD_FEATURE
		if(sys_data->wakeup_timer>=MAX_EPG_TIMER_NUM)
		{
			iprob=nvod_rise_from_timer();
			prog_num = get_prog_num(VIEW_ALL | PROG_NVOD_MODE, 0);
			cur_channel=get_prog_pos(cur_channel);
			if(cur_channel< prog_num )
			{
				if(API_PLAY_NORMAL==api_play_channel ( cur_channel, TRUE, TRUE, FALSE))
				{
					if(!iprob)
					{
						nvod_draw_remain_time();
						NvodUpdateTimeList();
					}
					rise_from_standby = TRUE;
					SetNvodProgID(cur_channel);
					menu = CHANNEL_BAR_HANDLE;/*also open bar menu */
					goto SEND_MENU_MESSAGE;
				}
			}

			ReSetNvodPlayFlag();
			SetNvodFromTimerFlag(FALSE);
			SetNvodModle(FALSE);
			cur_channel = 0xffffffff;/*go to else of the NORMAL_STRAT */	
		}	
#endif
	}
NORMAL_STRAT:
	/*set correct mode from boot*/
	if( cur_channel!=0xffffffff &&timer_cont != NULL)
	{
		sys_data_set_cur_chan_mode(timer_cont->wakeup_chan_mode);
		sys_data_change_group(timer_cont->wakeup_group_idx);
		prog_num = get_prog_num(VIEW_ALL | sys_data->cur_chan_mode, 0);
		cur_channel=get_prog_pos(cur_channel);
		if(cur_channel< prog_num )
			sys_data_set_cur_group_channel(cur_channel);
		else
			sys_data_set_cur_group_channel(0);
	}
	else
	{
		//sys_data_set_cur_chan_mode(TV_CHAN);
		UINT8 cur_group_idx=sys_data_get_cur_group_index();
		if(cur_group_idx>=MAX_GROUP_NUM+MAX_FAVGROUP_NUM||cur_group_idx<MAX_GROUP_NUM)
		    sys_data_change_group(0); 
		else//fav group
		    sys_data_change_group(cur_group_idx);
	}

	prog_num = get_prog_num(VIEW_ALL | sys_data->cur_chan_mode, 0);
	
	if (prog_num == 0)
	{
#ifdef NOCHANNEL_AUTOSEARCH      
        system_state = SYS_STATE_NORMAL;
        nochannel_startScan(0,1);
#endif
		menu = MAIN_MENU_HANDLE;
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_CONAX)
		set_mmi_showed(2);
#endif
#endif
	}
	 /* Main menu */
	//if not played last free service, open channel bar
	else if(FALSE==api_get_fastplay_flag())
	{
		menu = CHANNEL_BAR_HANDLE; /* Channel bar menu */
		show_and_playchannel = 1;
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_CONAX)
		set_mmi_showed(10);
#endif
#endif
	}
	else
	{
		menu = NULL;
		//if played last free service, show channel num on the front panel
		if(TRUE==api_get_fastplay_flag())
		{
			key_pan_display_channel(sys_data_get_cur_group_cur_mode_channel());
          #ifdef TS_MONITOR_FEATURE
          {
            P_NODE p_node;
            UINT16 cur_channel = sys_data_get_cur_group_cur_mode_channel();
            if(SUCCESS == get_prog_at(cur_channel, &p_node))
            {             
               ts_monitor_start(p_node.prog_id);
     //          api_play_channel(cur_channel, TRUE, TRUE, FALSE);
            }
          }
            
           #endif

            #if ((SUBTITLE_ON == 1)||(TTX_ON == 1))
            {
                P_NODE p_node;
                UINT16 cur_channel = sys_data_get_cur_group_cur_mode_channel();
                if(SUCCESS == get_prog_at(cur_channel, &p_node))
                {               
                   api_play_channel(cur_channel, TRUE, TRUE, FALSE);
                   osal_task_sleep(500);
                   api_osd_mode_change(OSD_SUBTITLE);
                }
            }
            #endif
                   
		}
	}
	system_state = SYS_STATE_NORMAL;

SEND_MENU_MESSAGE:
	if (menu != NULL)
		ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)menu, TRUE);

#ifdef MULTI_CAS
#if ((CAS_TYPE != CAS_CONAX) && (CAS_TYPE != CAS_ABEL))
#if(defined(MIS_AD) || defined(MIS_AD_NEW))
	cas_osdmsg_init();
#else
	cas_osdmsg_task_init();
#endif
#endif
#if (CAS_TYPE == CAS_TF)
	TFCASTB_RefreshInterface();
#elif (CAS_TYPE == CAS_CDCA)
	CDCASTB_RefreshInterface();
#endif
#endif
	while (1)
	{
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_ABEL)
		if(!api_abel_cas_check_card_inited())
		{
			api_abel_cas_get_card_initialisation_info();	
		}
#endif	
#endif
		retVal = osal_msgqueue_receive(&msg, &msgSize, control_mbf_id, 200);
			
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_DVN)
		dvn_cas_open_popup();//used to check if need to show or clear some ca popups
#endif
#endif

#if (CAS_TYPE == CAS_ABEL)
		ap_abel_osd_msg_popup_control_proc();
#endif

		if (1)//(retVal != OSAL_E_OK)
		{

			if ( sys_data->avset.tv_mode == TV_MODE_AUTO
			        && hde_get_mode() != VIEW_MODE_MULTI
			        && hde_get_mode() != VIEW_MODE_PREVIEW
			        && screen_back_state != SCREEN_BACK_MENU
			        && screen_back_state != SCREEN_BACK_RADIO )
			{
				vdec_io_control(g_decv_dev, VDEC_IO_GET_STATUS, (UINT32) &curStatus);
				if ( curStatus.uCurStatus == VDEC_DECODING
				        && api_video_get_srcmode ( &video_Src ) )
				{
					tv_out = api_video_get_tvout();
					if (tv_out != video_Src)
					{
						api_video_set_tvout(video_Src);
					}
				}
			}
			//continue;
		}
		
		if(retVal != OSAL_E_OK)
		{
			ap_cas_periodic_check();
		    continue;
		}

		menu = menu_stack_get_top(); /* The top menu*/
		///////////for ali ad////////////////////
#ifdef ALI_DEMO_AD
#ifdef ALI_AD_DEMO_ON
        static UINT8 float_img_flag=0;
        if(NULL == menu && 0 == ad_get_cur_scene())
        {
            if(float_img_flag>3)
            {                
                ad_show(AD_FLOAT_IMG);
                float_img_flag = 0;
            }
            else
            if(msg.msgType > CTRL_MSG_TYPE_KEY)
            {
                float_img_flag++;
            }
            else
            {
                float_img_flag = 0;
            }
        }
        else
        {
            float_img_flag = 0;
        }
#endif
#endif
        //////////////////////////////////////////
		//show mute when mis_video over or exit mis
		#ifdef MIS_AD
		if(Mis_Get_Mute_Flag()&&!cur_mute_state){
			ShowMuteOSDOnOff(TRUE);	
		}
		#endif
		if (msg.msgType <= CTRL_MSG_TYPE_KEY)
		{
			ap_key_commsg_proc(msg.msgType, msg.msgCode);
		}
		else if (msg.msgType <= CTRL_MSG_TYPE_CMD)
		{
			ap_comand_message_proc(msg.msgType, msg.msgCode);
		}
		else if((msg.msgType == CTRL_MSG_SUBTYPE_STATUS_OTA_FORCE)
			||(msg.msgType == CTRL_MSG_SUBTYPE_STATUS_OTA_MANUAL))
		{
			ap_OTA_upgrade(msg.msgType);
		}
		// jinfeng added
		else if(msg.msgType == CTRL_MSG_SUBTYPE_STATUS_NIT_UPDATE)
		{
			ap_nit_update_notify(msg.msgCode);
		}
		else if((msg.msgType == CTRL_MSG_SUBTYPE_STATUS_UPGRADE_MANUAL))
		{
			ap_manual_upgrade(msg.msgType);
		}
		else
		{
			if (msg.msgType == CTRL_MSG_SUBTYPE_STATUS_SIGNAL)
			{
				ap_signal_messag_proc(msg.msgType, msg.msgCode);
#ifdef MULTI_CAS
#if ((CAS_TYPE == CAS_CONAX) || (CAS_TYPE == CAS_ABEL))
				ap_mcas_mail_detitle_proc();
#else
				ap_cas_periodic_check();
#endif

			}
			else if ( msg.msgType == CTRL_MSG_SUBTYPE_STATUS_MCAS )
#if (CAS_TYPE == CAS_IRDETO)
				/*irdeto need one more msgSbuType to dispatch message to aviod data missing*/
				ap_cas_message_proc ( msg.msgSbuType, msg.msgCode );
#else
				ap_cas_message_proc ( msg.msgType, msg.msgCode );
#endif
#else
			}
#endif
			else
			{
				ap_comand_message_proc(msg.msgType, msg.msgCode);
			}
			continue;
		}

#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_CONAX)
		if(get_mmi_msg_cnt()>0 && get_mmi_showed()==0 && menu_stack_get_top()!=(POBJECT_HEAD)&g_win_audio && menu_stack_get_top()!=(POBJECT_HEAD)&g_win_light_chanlist && menu_stack_get_top()!=(POBJECT_HEAD)&g_win_chanlist)
		{
		set_mmi_showed(10);
		ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
		MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS: exit win; code:0\n");
		}
#endif
#endif	
		if (menu_stack_get_top() == NULL)
			sys_data_save(TRUE);

	
	}
}

/*
return value:
0  - Boot at normal state(need to play channel or show menu when no channels)
-1 - Boot at time wakeup */
INT32 system_init_process(void)
{
	SYSTEM_DATA *sys_data;
	UINT8 cur_mode;
	UINT32 i, prog_num;
	date_time dt;
	UINT32 wakeup_time, sys_time;
	INT32 ret;
	INT32 timer_idx=-1;
	struct YCbCrColor tColor;
	UINT32 chunk_id, db_addr, db_len, default_db_addr, default_db_len;
	UINT8 TV_Out;

	tColor.uY = 0x10;
	tColor.uCb = 0x80;
	tColor.uCr = 0x80;

	dm_set_onoff(0); /* Disable moniter */

	//key_pan_set_key_table(ir_key_maps);


	//user db chunk
	chunk_id = 0x04FB0100;
	api_get_chuck_addlen(chunk_id, &db_addr, &db_len);
	db_len = db_len - (64 *1024); //last sector for temp info
	//default db chunk
	chunk_id = STB_CHUNK_DEFAULT_DB_ID;
	api_get_chuck_addlen(chunk_id, &default_db_addr, &default_db_len);

	if(init_db(db_addr,db_len,db_addr+db_len,0x10000) != SUCCESS)
	{
		osal_task_sleep(50);
		if(init_db(db_addr,db_len,db_addr+db_len,0x10000) != SUCCESS)
		{
			osal_task_sleep(50);        
			set_default_value(DEFAULT_ALL_PROG,default_db_addr + 16);
		}
	}
	if(get_sat_num(VIEW_ALL) == 0) 
	{
		osal_task_sleep(50);
		if(get_sat_num(VIEW_ALL) == 0) 
		{
			osal_task_sleep(50);    	
			set_default_value(DEFAULT_ALL_PROG,default_db_addr + 16);
		}
	}
	recreate_prog_view(VIEW_ALL | TV_CHAN, 0);
	/*set bmp and font resource as seperate bin file*/
	{
		RSC_font_init();
		RSC_bmp_init();
	}
	ap_osd_init();
	TV_Out = api_video_get_tvout();
	api_osd_set_tv_system(TV_Out == TV_MODE_NTSC358 || TV_Out == TV_MODE_PAL_M || TV_Out == TV_MODE_NTSC443);

	/* Load temp data */
	sys_data_load();
	sys_data = sys_data_get();

	/* If support low power*/
//#ifdef REAL_STANDBY
//#if (REAL_STANDBY==SYS_FUNC_OFF)
	ret = sys_reboot_get_timer(&wakeup_time, &sys_time);
	if ( ( SUCCESS  == ret )
	        && ( ( wakeup_time >> 22 ) & 0xF ) //*month is ok
	 		&& ((wakeup_time >> 17) &0x1F) )//day is ok
	{
		libc_printf("coming here in standy bootup!\n");
		dt.year = ((sys_data->sys_dt.year / 100) *100) + (wakeup_time >> 26);
		dt.month = (wakeup_time >> 22) &0xF;
		dt.day = (wakeup_time >> 17) &0x1F;
		dt.hour = (wakeup_time >> 12) &0x1F;
		dt.min = (wakeup_time >> 6) &0x3F;
		dt.sec = wakeup_time &0x3F;
		
		timer_idx = find_timer_with_st((date_time *)&dt);
		libc_printf("timer_idx = %d!\n",timer_idx);
		if(-1!= timer_idx)
		{	
			sys_data->wakeup_timer = timer_idx;
			ret = -1;
		}
		else
		{
			ret = 0;
		}
	}
	else
	{
		ret = 0;
	}
//#endif
//#endif

	/* Panel & screen initital display */
	if (!sys_data->bstandmode)
		key_pan_display("strt", 4);

	if (sys_data->bMuteSate)
		SetMuteOnOff(FALSE);
	cur_mode = sys_data_get_cur_chan_mode();

	//unnecessary stop vdec
	//vdec_stop(g_decv_dev, 0, 0);

	if (!sys_data->bstandmode)
	{
		#ifndef AD_SANZHOU
		if (cur_mode == RADIO_CHAN)
			api_show_radio_logo();
		#endif
	}
	else
		key_pan_display("----", 4);

	prog_num = get_prog_num(VIEW_ALL | cur_mode, 0);

	/* Set all timer in not runing status */
	for (i = 0; i < MAX_TIMER_NUM; i++)
		sys_data->timer_set.TimerContent[i].wakeup_state = TIMER_STATE_READY;

	/* Clear all message */
	//speed up init process
	//osal_task_sleep(2000);
	pan_buff_clear();
	ap_clear_all_message();
	//unnecessary stop vdec
	//vdec_stop(g_decv_dev, 0, 0);
	dm_set_onoff(1);

	start_tdt();
#ifdef EPG_MULTI_TP
    //demo set 0x100000,  for customer project, could set larger memery for epg if memery enough
	epg_init(SIE_EIT_WHOLE_TP, NULL, 0x100000, ap_epg_call_back);
#else
	epg_init(SIE_EIT_WHOLE_TP, NULL, 0x100000, ap_epg_call_back);
#endif
	sim_open_monitor(0);

#ifdef PSI_MONITOR_SUPPORT
	si_monitor_register((on_pid_change_t)ap_pid_change);
#endif
	init_clock(time_refresh_callback);

#ifdef MULTI_CAS
#if(CAS_TYPE == CAS_IRDETO)
	init_msg_backlog();
#endif
#endif

#ifdef IRD_OTA_SUPPORT
	//ird_ota_task_init();
	//ird_ota_send_msg(IRD_OTA_INIT_PARAM,0);
	//SDBBP();
	ird_ota_init_params();
	//ird_ota_start();	
#endif

	return ret;
}


///////////////////////////////////////////////////////
//   some functions to handle messages 
//
//Return 1:open the nim debug window,else do not.
UINT8 NimDebugKeyProc(UINT32 vkey)
{
    static UINT8 nimreg_pwd_match_num = 0;
    PRESULT proc_ret;

    switch(nimreg_pwd_match_num)
    {
        case 0:
            if(V_KEY_RED == vkey)
                nimreg_pwd_match_num++;
            break;
        case 1:
            if(V_KEY_YELLOW== vkey)
                nimreg_pwd_match_num++;
            else
                nimreg_pwd_match_num = 0;
            break;
        case 2:
            if(V_KEY_YELLOW== vkey)
                nimreg_pwd_match_num++;
            else
                nimreg_pwd_match_num = 0;
            break;
        case 3:
#ifdef NIM_REG_ENABLE
            if((V_KEY_RED == vkey)&&
                (menu_stack_get_top()!=(POBJECT_HEAD)&g_win_nimreg))
            {
				proc_ret = OSD_ObjOpen((POBJECT_HEAD)&g_win_nimreg, MENU_OPEN_TYPE_KEY + vkey);
				if (proc_ret != PROC_LEAVE)
				{
					menu_stack_push((POBJECT_HEAD)&g_win_nimreg);    
				}
                nimreg_pwd_match_num = 0;
                return 1;
            }
#endif          
#ifdef REG_ENABLE
			else
			{
			    if((V_KEY_YELLOW == vkey)&&
                (menu_stack_get_top()!=(POBJECT_HEAD)&g_win_reg))
	            {
					proc_ret = OSD_ObjOpen((POBJECT_HEAD)&g_win_reg, MENU_OPEN_TYPE_KEY + vkey);
					if (proc_ret != PROC_LEAVE)
					{
						menu_stack_push((POBJECT_HEAD)&g_win_reg);    
					}
	                nimreg_pwd_match_num = 0;
	                return 1;
	            }		
			}
#endif
            nimreg_pwd_match_num = 0;
            break;
        default:
            break;
            
    } 

    return 0;
}
#ifdef MIS_AD
int MIS_key_proc(UINT32 vkey)
{
	int ret;
	P_NODE p_node;
	T_NODE t_node;
	UINT16 cur_channel = sys_data_get_cur_group_cur_mode_channel();
	
	//黄色键代替F1
	if(vkey == V_KEY_YELLOW){
		MIS_SetYellowKey_Press(TRUE);
	}else if(vkey != V_KEY_ENTER){
		MIS_HideAdvNotify(0);
	}
	
	switch(vkey){
		case V_KEY_RED:
			MIS_SysForbit();
			break;
		case V_KEY_ENTER:
			//当一个频道同时有泡泡和屏显时，先进泡泡应用
			if(MIS_GetShowAdvType()!=0xff)
			{
				MIS_SetEnterMis_Auto(TRUE);
				Mis_Set_EnterAutoShow(FALSE);
				ret = MIS_EnterMisNotify();
				if(ret != 0){
					MIS_SetEnterMisNotify(FALSE);
					MIS_SetIsMisControl(FALSE);
					MIS_HideAdvNotify(0);
				}
			}
			break;
	}
	return 0;
}
#endif
void ap_key_commsg_proc(UINT32 msg_type, UINT32 msg_code)
{
	POBJECT_HEAD topmenu, newMenu;
	PRESULT proc_ret;
	BOOL bFind, bFlag, bShowOSD = FALSE;
	UINT32 key, vkey, prevkey = 0xFFFFFFFF;
	OSD_RECT *pmenuRect = NULL, rect, crossRect;
	SYSTEM_DATA *sys_data;
	UINT32 osd_msg_type = 0, osd_msg_code = 0;
	UINT32 start = 0, next_start;
	UINT8 cur_mode;
    UINT8 menu_nums;

	topmenu = menu_stack_get_top();
	cur_mode = sys_data_get_cur_chan_mode();


	if (msg_type <= CTRL_MSG_TYPE_KEY)
	{
		key = msg_code;

		if (msg_type == CTRL_MSG_SUBTYPE_KEY)
		{
			#if 1//def CHCHG_TICK_PRINTF
			//key pressed
			if((key>>28)&0x1)
			{
				g_cc_play_tick[g_cc_play_step].step = KEY_PROC;
				g_cc_play_tick[g_cc_play_step].start_tick = osal_get_tick();
				g_cc_play_tick[g_cc_play_step++].end_tick= osal_get_tick();
				//libc_printf("####CHCHG %s():line%d  proc key_msg 0x%x, tick=%d\n", __FUNCTION__,__LINE__, key,osal_get_tick());
			}
			#endif
			osd_msg_type = (MSG_TYPE_KEY << 16);

HK_TO_VKEY:
			next_start = ap_hk_to_vk ( start, key, &vkey );
			#ifdef MIS_AD
			if (vkey == V_KEY_ENTER && (topmenu == ( POBJECT_HEAD ) &g_win_progname ||topmenu == ( POBJECT_HEAD )&g_win_volume))
			{
				MIS_EnterEpgAdv();
				return;
			}
		#endif

#if 1
            if(V_KEY_NULL != vkey)
            {
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_DVT)
			ap_cas_caslib_print(vkey);
#elif (CAS_TYPE == CAS_MG)
			if(vkey==V_KEY_RED)
			{
                ippv_window_open();
			}
#endif
#endif
                if(NimDebugKeyProc(vkey))
                    return;
            }
#endif           
			if (start == 0)
				prevkey = vkey;
			start = next_start;
#ifdef AD_SANZHOU
			//test code. need removed.
/*			if (vkey == V_KEY_GREEN)
			{
				szxc_ad_show_corner();
			}*/
#endif			
			if (vkey == V_KEY_NULL)
			{
				if (prevkey == vkey)
				 /* First time */
					return ;
				else
				{
					/* If try mapping the key for sever times ,then
					try to use the first mapping vkey for unkown key processing.
					( Only valid for no menu on screen processing )
					 */
					vkey = prevkey;
					proc_ret = PROC_PASS;
					goto UNKOWN_KEY_PROC;
				}


			}
     #ifdef MULTI_CAS			
     #if (CAS_TYPE == CAS_ABEL)			
			else
			{			
				on_event_user_pressed_key(vkey);
			}
     #endif	
     #endif
		}
		else
		//CTRL_MSG_SUBTYPE_KEY_UI
		{
			vkey = key;
			next_start = ap_vk_to_hk(0, vkey, &key);
		}

		osd_msg_code = key;
		#ifdef MIS_AD
		//mis key notify
		if(topmenu == NULL){
			MIS_key_proc(vkey);
			if(MIS_GetIsMisControl()){
				return;
			}
		}
		#endif
	}
	else if ( msg_type == CTRL_MSG_SUBTYPE_CMD_EXIT_ROOT
	          || msg_type == CTRL_MSG_SUBTYPE_CMD_EXIT_TOP )
	{
		if (msg_type == CTRL_MSG_SUBTYPE_CMD_EXIT_ROOT)
		{
			if (msg_code == (UINT32)topmenu && topmenu != NULL)
			{
				proc_ret = OSD_ObjClose(topmenu, C_DRAW_SIGN_EVN_FLG | C_CLOSE_CLRBACK_FLG);
				proc_ret = PROC_PASS;
				goto POP_PRE_MENU;
			}
		}
		else
		//CTRL_MSG_SUBTYPE_CMD_EXIT_TOP
		{
			if (topmenu == NULL)
				return ;
			proc_ret = OSD_ObjClose(topmenu, C_DRAW_SIGN_EVN_FLG | C_CLOSE_CLRBACK_FLG);
			proc_ret = PROC_PASS;
			goto POP_PRE_MENU;

		}
	}
	else
	{
		osd_msg_type = (MSG_TYPE_MSG << 16) | (msg_type &0xFFFF);
		osd_msg_code = msg_code;
	}

	proc_ret = PROC_PASS;


	if (topmenu != NULL)
	{
#if ((SUBTITLE_ON == 1)||(TTX_ON ==1))
		menu_nums = GetCurWinNum();
        if((menu_nums>1)||(topmenu == MAIN_MENU_HANDLE)||(topmenu == EPG_HANDLE))
        {
#if (SUBTITLE_ON == 1)
			subt_show_onoff(FALSE);
#endif	
#if (TTX_ON ==1)
			TTXEng_ShowOnOff(FALSE);
#endif
        }
#endif
    
MENU_KEY_PROC:
		proc_ret = OSD_ObjProc ( topmenu, osd_msg_type, osd_msg_code, 0 );

CEHCK_PROC_RETURN:
		if ( proc_ret == PROC_LEAVE )
		{
/* there is a underlying rule here. make sure @do not@ modify the menu stack manually in some 
 * standard process(such as each window's EVN_PRE_CLOSE,EVN_POST_CLOSE).
 * some function blew will cause problem:
 * 1:call OSD_ObjOpen(newMenu, MENU_OPEN_TYPE_STACK);
 * 2:call menu_stack_push()
 * 2:call BackToFullScrPlay()
 */
POP_PRE_MENU:
			menu_stack_pop();
			newMenu = menu_stack_get_top();
			if (newMenu != NULL)
			 /* Open the top menu */
			{
				proc_ret = OSD_ObjOpen(newMenu, MENU_OPEN_TYPE_STACK);
				if (proc_ret == PROC_LEAVE)
					goto POP_PRE_MENU;

				proc_ret = PROC_LOOP;
			}
			else
			 /* Restore previous menu */
			{
				ShowMuteOnOff();
				ShowPauseOnOff();
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_DVN)		
				ap_cas_message_show();
#elif (CAS_TYPE == CAS_IRDETO)
				popfrom_backlog();
#elif (CAS_TYPE == CAS_CONAX)
				if(get_mmi_msg_cnt()>0&&get_mmi_showed()==2)
				{
					set_mmi_showed(10);
					ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
					MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS: exit win; code:0\n");
				}
#endif
#endif
				proc_ret = PROC_LOOP;
			}
		}
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_CONAX)
		else
		{
			if(get_mmi_showed()==0&&get_mmi_msg_cnt()>0)
			{
				ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
				MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS: no menu and no msg show; code:%d\n",0);
			}
		}
#endif
#endif
	}
	else       /*  topmenu==NULL */
	{	
		if (msg_type <= CTRL_MSG_TYPE_KEY)
		{
#ifdef NVOD_FEATURE
			if(GetNvodPlayFlag())
				bFind = menus_find_root_nvodmode(CTRL_MSG_SUBTYPE_KEY, osd_msg_code, &bFlag, &newMenu);
			else
#endif
			bFind = menus_find_root(CTRL_MSG_SUBTYPE_KEY, osd_msg_code, &bFlag, &newMenu);

			if (!bFind)
				goto HK_TO_VKEY;
			if (bFind)
			 /* Enter a new menu*/
			{
#if (CAS_TYPE == CAS_ABEL)
			//libc_printf("[B]: Before Enter a new menu (ABEL_DISP_OFF)\n");
			on_event_system_is_entering_new_mmi_menu(newMenu);
#endif
		
#if ((TTX_ON == 1)||(SUBTITLE_ON == 1)) 
                if((newMenu == MAIN_MENU_HANDLE )||(newMenu == CHANNEL_LIST_HANDLE )||(newMenu == EPG_HANDLE))
                    api_osd_mode_change(OSD_WINDOW); 
#else
                api_osd_mode_change(OSD_WINDOW); 
#endif
				/* No signal OSD */
				//if(newMenu == )
				{
					/* Clear no signal show */
					if (GetSignalStatausShow())
						ShowSignalStatusOSDOnOff(0);
				}

				/* Time show OSD*/

#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_CONAX)
				if(get_mmi_msg_cnt()>0&&(get_mmi_showed()!=5||newMenu!=(POBJECT_HEAD)&g_win_progname))
				{
					win_mmipopup_close();
				}
#endif					
#endif
#ifdef AD_SANZHOU
				if((newMenu!=(POBJECT_HEAD)&g_win_volume)\
					&&(newMenu!=(POBJECT_HEAD)&g_win_audio))
				{
					szxc_ad_hide_txt();
				}
#endif
				/* Open the new menu*/
				//打开全屏界面，关闭字幕显示
				#ifdef MIS_AD
				if(vkey == V_KEY_MENU || vkey == V_KEY_EPG){
					MIS_Set_EnterMainMenu_Full(TRUE);
					MIS_SysHideAdv(1);	
				}
				#endif
#ifdef ALI_AD_DEMO_ON
                //////////////ad////////////////////
                if(ad_get_scene_status(AD_FLOAT_IMG))
                {
                    ad_stop(AD_FLOAT_IMG);
                }
#endif
                ////////////////////////////////////
                
				proc_ret = OSD_ObjOpen(newMenu, MENU_OPEN_TYPE_KEY + vkey);
				if (proc_ret != PROC_LEAVE)
				{
					menu_stack_push(newMenu);
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_CONAX)
					if(menu_stack_get_top()==(POBJECT_HEAD)&g_win_mainmenu ||
                     menu_stack_get_top()==(POBJECT_HEAD)&g_win_epg)
					{
						/*
						if(get_mmi_showed()==5)
						{
//							win_mmipopup_close();
							add_mmi_msg_clr();
						}
						*/

						if(get_mmi_msg_cnt()>0)
						{
							clean_mmi_cur_msg();
							//win_mmi_close();
						}
						set_mmi_showed(2);
					}
					else if(menu_stack_get_top()==(POBJECT_HEAD)&g_win_audio || menu_stack_get_top()==(POBJECT_HEAD)&g_win_light_chanlist||  menu_stack_get_top()==(POBJECT_HEAD)&g_win_chanlist)
						set_mmi_showed(0);
					else if( menu_stack_get_top() != (POBJECT_HEAD)&g_win_progname)
					{
						set_mmi_showed(10);
					}
#endif
#endif
					if (bFlag)
					{
						topmenu = newMenu;
						goto MENU_KEY_PROC;
					}

					proc_ret = PROC_LOOP;
				}
				else
				{
#ifdef MULTI_CAS				
#if (CAS_TYPE == CAS_CONAX)
					if(get_mmi_msg_cnt()>0)
					{
						ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS, 0, FALSE);
						MMI_PRINTF("CTRL_MSG_SUBTYPE_STATUS_MCAS: open root menu fail; code: 0\n");
						set_mmi_showed(10);
					}
#endif
#endif
					proc_ret = PROC_PASS;
				}
			}
			else
				proc_ret = PROC_PASS;
		}
	}


	UNKOWN_KEY_PROC:

	topmenu = menu_stack_get_top();
	if (topmenu != NULL)
		pmenuRect = &topmenu->frame;
	else
		bShowOSD = TRUE;
#if(TTX_ON==1)
	static UINT8 bFirstShow=FALSE;
	static UINT16 pre_channel = 0xffff;
  	UINT16 cur_channel = sys_data_get_cur_group_cur_mode_channel();
    if(cur_channel != pre_channel)
    {
        bFirstShow = TRUE;
	    pre_channel = cur_channel; 
    }
#endif

	sys_data = sys_data_get();


	if (msg_type <= CTRL_MSG_TYPE_KEY && proc_ret == PROC_PASS)
	{
		/* Menu not processed key processing */
		if (vkey == V_KEY_POWER&&topmenu!=(POBJECT_HEAD) &g_win_search)
		{
			power_switch(0);
		}
		else if ((vkey == V_KEY_MUTE) || ((vkey == V_KEY_PAUSE) && (cur_mode == TV_CHAN)))
		{
			if (topmenu == (POBJECT_HEAD) &g_win_search)
				return ;

			if (screen_back_state == SCREEN_BACK_MENU)
				bShowOSD = FALSE;
			else
			{
				if (vkey == V_KEY_MUTE)
					GetMuteRect(&rect);
				else
					GetPauseRect(&rect);
				if (topmenu != NULL)
				{
					if (menu_stack_get(0) != topmenu)
					 /* If there more menus in the stack*/
						bShowOSD = FALSE;
					else
					{
						OSD_GetRectsCross(pmenuRect, &rect, &crossRect);
						if (crossRect.uWidth == 0 || crossRect.uHeight == 0)
							bShowOSD = TRUE;
						else
							bShowOSD = FALSE;
					}
				}
			}

#if ((SUBTITLE_ON == 1)||(TTX_ON == 1)) 
#else
			if (bShowOSD)
            {
				api_osd_mode_change(OSD_WINDOW);               
            }
#endif 
			if (vkey == V_KEY_MUTE)
            {   
				SetMuteOnOff(bShowOSD);               
            }
			else
            {
				SetPauseOnOff(bShowOSD);
            }
            
#if ((SUBTITLE_ON == 1)||(TTX_ON == 1)) 
//make sure to show the Mute&Pause bmp correctly when play program with no osd window
			if (bShowOSD)
            {
				ShowMuteOnOff();
	            ShowPauseOnOff();
	        	api_osd_mode_change(OSD_SUBTITLE);               
            }              
#endif             

		}
		/*else if (vkey == V_KEY_RGBCVBS)
		{
			unsigned int sw;

			sw = sys_data->avset.scart_out;
			sys_data->avset.scart_out = (sw == SCART_RGB) ? SCART_CVBS : SCART_RGB;

			sw = (sys_data->avset.scart_out == SCART_RGB) ? 1 : 0;
		}*/

#if (SUBTITLE_ON==1)
		else if (vkey == V_KEY_SUBTITLE)
		{
			//			win_func_testing_proc();
		}
//for testing        
		else if (vkey == V_KEY_BLUE)
		{
		    UINT8 menu_num =0xFF ;  //only used when playing program with no window
            menu_num = GetCurWinNum();
            if(menu_num==0)
            {
				//win_func_testing_proc();	            
            }	
		}
#endif
		else if( vkey == V_KEY_TEXT)
		{
#if (TTX_ON == 1)				
			struct t_ttx_lang *lang_list;
			UINT8 lang_num;

			if(topmenu == NULL)
			{
//				TTXEng_GetInitLang(&lang_list, &lang_num);
				SYSTEM_DATA* sys_data;
                UINT8 sel_lang=0;
                sys_data = sys_data_get();

				TTXEng_GetInitLang(&lang_list, &lang_num);
				sel_lang = api_ttxsub_getlang(lang_list, NULL,sys_data->lang.ttx_lang,lang_num);

/*			
#ifndef MUTI_TTX_SUB                            
		        if( (p_node.teletext_pid==0 || p_node.teletext_pid==0x1FFF)
#else
		        if(p_node.teletext_count==0 )
#endif
*/		    
				if(lang_num == 0)
				{
					/* If no ttx data*/
//					api_osd_layer2_onoff(OSDDRV_OFF);
					ShowSignalStatusOSDOnOff(0);//win_nosig_close();
					win_com_popup_open(WIN_POPUP_TYPE_SMSG,NULL, RS_DISPLAY_NO_DATA);
					osal_task_sleep(1000);
					win_compopup_close();
				}
				else
				{
					sys_state_t back_state;
			        if(bFirstShow)
			        {
//						api_osd_layer2_onoff(OSDDRV_OFF);
						ShowSignalStatusOSDOnOff(0);
						win_com_popup_open(WIN_POPUP_TYPE_SMSG,NULL,RS_PLEASE_WAIT);
						osal_task_sleep(6000);
						win_compopup_close();
						bFirstShow = 0;
			        }
					back_state = system_state;//for pip state
					system_state = SYS_STATE_TEXT;
					TTX_OSDOpen();
					system_state = back_state;
				}
			}
#endif
		}

	}

	/* If no OSD showed, then switch to subtitle mode. */
	if ( topmenu == NULL
	        && GetMuteState() != TRUE
	        && GetPauseState() != TRUE
	        && GetSignalStatausShow() != TRUE )
	{
		//api_osd_mode_change ( OSD_SUBTITLE );	
	}

}






void ap_timer_expire_proc(UINT32 msg_type, UINT32 msg_code)
{
	UINT32 i;
	SYSTEM_DATA *sys_data;
	TIMER_SET_CONTENT *timer;
	enum OSD_MODE osd_mode = 0xFFF;
	BOOL enter_standby = FALSE;
	OSD_RECT rect;

	if ( msg_type != CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE
	        && msg_type != CTRL_MSG_SUBTYPE_CMD_SLEEP
	        && msg_type != CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMER_EXPIRE)
		return ;

	sys_data = sys_data_get();

	if (system_state == SYS_STATE_UPGRAGE_HOST)
		return ;

	if ( msg_type == CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE || msg_type == CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMER_EXPIRE)
	{
#ifdef NVOD_FEATURE		
        if(msg_type == CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMER_EXPIRE)//nvod
        {
			nvod_back_from_timer();
		    show_and_playchannel = 1;
	        ap_send_msg ( CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, ( UINT32 ) CHANNEL_BAR_HANDLE, TRUE );
	        return;
        }
#endif
		for (i = 0; i < MAX_TIMER_NUM; i++)
		{
			timer = &sys_data->timer_set.TimerContent[i];

			if (timer->wakeup_state == TIMER_STATE_RUNING)
				break;
		}

		if (i == MAX_TIMER_NUM)
		 /* Currently no other timers running*/
			enter_standby = TRUE;

	}
	else if (msg_type == CTRL_MSG_SUBTYPE_CMD_SLEEP)
	{
		enter_standby = TRUE;
	}

	ap_clear_all_menus();

	if (msg_type == CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE)
	{
		/* Display time expoire information */
		/* Open OSD */
		OSD_ShowOnOff(OSDDRV_ON);

		win_compopup_init(WIN_POPUP_TYPE_SMSG);
		win_compopup_set_title(NULL,NULL,RS_INFO_INFO);
		win_compopup_set_msg(NULL, NULL, RS_MSG_EVT_FINISHED);
		win_compopup_open();
		osal_task_sleep(1000);
		win_compopup_close();
	}

	if (enter_standby)
	{
		//osal_timer_activate(GAME_CYCLIC_ID, FALSE);
		//api_stop_timer(&GAME_CYCLIC_ID);
		//power_off();
		power_switch(0);
	}
	else
	{
		UINT8 av_flag;

		av_flag = sys_data_get_cur_chan_mode();

		if (sys_data_get_sate_group_num(av_flag) == 0)
			ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)MAIN_MENU_HANDLE, TRUE);
		else
		{
			show_and_playchannel = 1;
			ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)CHANNEL_BAR_HANDLE, TRUE);
		}
	}


}





void ap_timer_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	UINT32 i, n;
	UINT32 osd_msg_type, hwkey;
	OSD_RECT rect;
	SYSTEM_DATA *sys_data;
	TIMER_SET_CONTENT *timer;
	UINT32 prog_id;
	UINT16 cur_channel;
	UINT8 cur_mode, back_save;
	P_NODE p_node;
	enum OSD_MODE osd_mode = 0xFFF;
	POBJECT_HEAD topmenu;
	win_popup_choice_t pop_choice;
	UINT16 book_msg[100];
	UINT16 str_len;
	UINT8 *s,str[5];
	UINT8 pre_group_idx,pre_mode;
	enum API_PLAY_TYPE play_ret;
	INT32 iprob = 0;

	sys_data = sys_data_get();

    if(menu_stack_get_top()==(POBJECT_HEAD)&g_win_search)//Do not response to timer when searching. 
    {
        if(msg_type == CTRL_MSG_SUBTYPE_CMD_TIMER_WAKEUP ||
         msg_type == CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMER_WAKEUP)
        {
            timer = &sys_data->timer_set.TimerContent[msg_code];
            timer->wakeup_time+=60;
            timer->wakeup_state =TIMER_STATE_READY;
            //timer->timer_mode = TIMER_MODE_OFF;
        }
        return;
    }
    
	osd_msg_type = MSG_TYPE_MSG << 16;
	ap_vk_to_hk(0, V_KEY_EXIT, &hwkey);

	if (msg_type == CTRL_MSG_SUBTYPE_CMD_TIMER_WAKEUP ||
         msg_type == CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMER_WAKEUP)
	{
		timer = &sys_data->timer_set.TimerContent[msg_code];
#if 0
		if ( timer->timer_service == TIMER_SERVICE_MESSAGE
		        || timer->wakeup_duration_time == 0 )
			timer->wakeup_state = TIMER_STATE_READY;
#else
		timer->timer_mode = TIMER_MODE_OFF;
#endif

		if (timer->timer_service == TIMER_SERVICE_MESSAGE)
		{
			if (as_service_query_stat() == ERR_BUSY)
				return ;

			if (hde_get_mode() != VIEW_MODE_MULTI)
				osd_mode = api_osd_mode_change(OSD_WINDOW);

			if (hde_get_mode() != VIEW_MODE_MULTI)
			{
				if (menu_stack_get_top() != NULL)
					ap_send_msg(CTRL_MSG_SUBTYPE_CMD_UPDATE_VIEW, 0, FALSE);
			}
			return ;
		}

		//sys_data_check_channel_groups();
		pre_group_idx = sys_data_get_cur_group_index();
		pre_mode = sys_data_get_cur_chan_mode();
		
		if(msg_type == CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMER_WAKEUP)
			sys_data_change_group(NVOD_GROUP_TYPE);
		else
			sys_data_change_group(ALL_GROUP_TYPE);
		
//		cur_channel = find_valid_channel(timer);
		prog_id = timer->wakeup_channel;
		if (get_prog_by_id(prog_id, &p_node) != DB_SUCCES)
		{
			//timer->timer_mode = TIMER_MODE_OFF;
			win_compopup_init(WIN_POPUP_TYPE_OK);
			win_compopup_set_title(NULL,NULL,RS_INFO_INFO);
			win_compopup_set_btnstr(0,RS_KEY_OK);
			win_compopup_set_msg(NULL, NULL, RS_MSG_TIMER_CH_NOEXIST);
			win_compopup_open_ext(&back_save);
			/* no program */
			sys_data_change_group(pre_group_idx);
			sys_data_set_cur_chan_mode(pre_mode);
			goto EXIT_BAD_PROGID;
		}
	
		cur_channel = get_prog_pos(prog_id);

		if (cur_channel == P_INVALID_ID)
		 /*If current group has not timer channel */
        {
			sys_data_set_cur_group_index(0);
			sys_data_change_group(0); /*  Switch to all group */
			cur_channel = get_prog_pos(prog_id);

			if (cur_channel != P_INVALID_ID)
			 /* find channel */
				sys_data_set_cur_group_index(0);
		}

		if(cur_channel == P_INVALID_ID)
		{
			cur_mode = sys_data_get_cur_chan_mode();
			if(cur_mode==RADIO_CHAN)
				sys_data_set_cur_chan_mode(TV_CHAN);
			else
				sys_data_set_cur_chan_mode(RADIO_CHAN);

			sys_data_change_group(0); /*  Switch to all sate group */
			cur_channel =  get_prog_pos(prog_id);  
		}
	
		if (cur_channel == P_INVALID_ID)
			cur_channel = 0;

		MEMSET(book_msg,0,sizeof(book_msg));
		str_len = 0;

		//频道
		s = OSD_GetUnicodeString(RS_RESERVE_CHANNEL);
		ComUniStrCopyChar((UINT8 *)&book_msg[str_len], s);
		str_len = ComUniStrLen(book_msg);
		if(msg_type == CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMER_WAKEUP)
			s = OSD_GetUnicodeString(RS_NVOD_PROG);
		else
			s = p_node.service_name;
		ComUniStrCopyChar((UINT8*)&book_msg[str_len], s);	
		str_len = ComUniStrLen(book_msg);

		sprintf(str,"\n");
		ComAscStr2Uni(str,&book_msg[str_len]);
		str_len = ComUniStrLen(book_msg);

		//节目
		s = OSD_GetUnicodeString(RS_RESERVE_PROG);
		ComUniStrCopyChar((UINT8 *)&book_msg[str_len], s);
		str_len = ComUniStrLen(book_msg);
		ComUniStrCopyChar((UINT8*)&book_msg[str_len], timer->event_name);	
		str_len = ComUniStrLen(book_msg);

		sprintf(str,"\n");
		ComAscStr2Uni(str,&book_msg[str_len]);
		str_len = ComUniStrLen(book_msg);

		//一分钟后预约节目开始!
		s = OSD_GetUnicodeString(RS_MSG_EVT_NOTIFY);
		ComUniStrCopyChar((UINT8 *)&book_msg[str_len], s);
		str_len = ComUniStrLen(book_msg);

		win_compopup_init(WIN_POPUP_TYPE_OKNO);
		win_compopup_set_title(NULL,NULL,RS_INFO_INFO);
		win_compopup_set_btnstr(0,RS_KEY_WATCH);
		win_compopup_set_btnstr(1,RS_KEY_CANCEL);
		win_compopup_set_msg(NULL, (UINT8*)book_msg, 0);
		pop_choice = win_compopup_open_timer(60*1000,&back_save);

		if(pop_choice == WIN_POP_CHOICE_YES)
		{
#ifdef NVOD_FEATURE
			if(msg_type == CTRL_MSG_SUBTYPE_CMD_TIMER_WAKEUP
				&&GetNvodPlayFlag())
			{
				nvod_close();
				nvod_stop_play();
				reset_all_flags();
				sys_data_change_group(0);
				system_state = SYS_STATE_NORMAL;
			} 
#endif			
#if 0
#ifdef SHOW_PICS
#if(SHOW_PICS == SHOW_SINGLE_PIC)
			if(menu_stack_get_top()==(POBJECT_HEAD)&g_win_show_single_pic_ad)//exit ad menu when timer wakeup. 
				win_show_single_pic_exit();
#endif
#endif
#endif
			sys_data_set_cur_group_channel(cur_channel);
			goto EXIT_NORMAL;
        }
        else
        {
			sys_data_change_group(pre_group_idx);
			sys_data_set_cur_chan_mode(pre_mode);
			topmenu = menu_stack_get_top();
			if(topmenu == (POBJECT_HEAD)&g_win_reserve)
			{
				//if(msg_code<MAX_EPG_TIMER_NUM)
				{
					OSD_TrackObject((POBJECT_HEAD) &g_win_reserve, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL  );
					//win_epg_sch_event_update(day_offset,TRUE);
				}
			}
#ifdef NVOD_FEATURE
			else if(topmenu == (POBJECT_HEAD)&g_win_nvod
				||topmenu == (POBJECT_HEAD)&g_win_epg)
#else
			else if(topmenu == (POBJECT_HEAD)&g_win_epg)
#endif
			{
				OSD_ObjOpen(topmenu, C_UPDATE_ALL);
        }
#ifdef NVOD_FEATURE
			else if(topmenu == (POBJECT_HEAD)&g_win_nvod)
			{
				OSD_TrackObject((POBJECT_HEAD) &g_win_nvod, C_UPDATE_ALL);

				if(msg_code>=MAX_EPG_TIMER_NUM)
		{
				//	OSD_TrackObject((POBJECT_HEAD) &nvod_sch_ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG|C_DRAW_TYPE_HIGHLIGHT);
				}
			}			
#endif			
			goto EXIT_DIRECTLY;
		}
		
	}
#ifdef NVOD_FEATURE
	else if(msg_type == CTRL_MSG_SUBTYPE_STATUS_NVOD_TIME_UPDATE)
	{
		nvod_draw_remain_time();
	}
	else if ( msg_type == CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE
	          || msg_type == CTRL_MSG_SUBTYPE_CMD_SLEEP 
			  || msg_type == CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMER_EXPIRE)
	{
		ap_timer_expire_proc(msg_type, msg_code);
		/*fall through into EXIT_DIRECTLY*/
	}
#else
	else if ( msg_type == CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE
	          || msg_type == CTRL_MSG_SUBTYPE_CMD_SLEEP )
	{
		ap_timer_expire_proc(msg_type, msg_code);
		/*fall through into EXIT_DIRECTLY*/
	}
#endif
/*do nothing*/
EXIT_DIRECTLY:	
	return;

/*in most case,we enter into the main menu*/
EXIT_BAD_PROGID:
	if(system_state != SYS_STATE_DATACAST)
	{
	ap_clear_all_message();
	ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)MAIN_MENU_HANDLE, TRUE);
	}
	return;

/*normal exit path*/
EXIT_NORMAL:
#ifdef NVOD_FEATURE
	if(msg_type == CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMER_WAKEUP)
		set_nt_event_para(timer->nvod_event_id,timer->nvod_event_servceid);
#endif
	if(system_state == SYS_STATE_DATACAST)
	{
		//if(msg_type == CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMER_WAKEUP)
			//enr_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_NVOD, (UINT32)prog_id, TRUE);
		//else		
			//enr_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)prog_id, TRUE);
	}
	else
	{
		ForceToScrnDonothing();
	/* Show OSD */
	OSD_ShowOnOff(OSDDRV_ON);
		//api_play_channel ( cur_channel, TRUE, TRUE, FALSE );
#ifdef NVOD_FEATURE
		if(msg_type == CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMER_WAKEUP)
		{
			iprob = nvod_rise_from_timer();
			play_ret = api_play_channel ( cur_channel, TRUE, TRUE, FALSE );
			if(play_ret==API_PLAY_NORMAL)
			{
				/* always start timer show for nvod if got right envent info*/
				if(!iprob)
				{
					nvod_draw_remain_time();
					NvodUpdateTimeList();
				}
				SetNvodProgID(prog_id);
			}
			else
				SetNvodProgID(0xFFFFFFFF);
		}
		else
#endif
	{
		show_and_playchannel = 1;
		ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)CHANNEL_BAR_HANDLE, TRUE);	
	}
	}
	return;	
	
}

#ifdef TS_MONITOR_FEATURE
static UINT16	view_create_flag_bk;
static UINT32	view_create_parm_bk;
#endif
void ap_comand_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	POBJECT_HEAD menu;
	PRESULT ret;
	UINT32 osd_msg_type = 0, osd_msg_code = 0;
	UINT16 cur_channel;
    P_NODE p_node;


	if (msg_type == CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT)
	{
		menu = (POBJECT_HEAD)msg_code;
		if (menu != NULL && menu_stack_get_top() == NULL)
		{
			api_osd_mode_change(OSD_WINDOW);
			if (OSD_ObjOpen(menu, MENU_OPEN_TYPE_OTHER) != PROC_LEAVE)
				menu_stack_push(menu);
		}
	}
	else if ( msg_type == CTRL_MSG_SUBTYPE_CMD_TIMER_WAKEUP
	          || msg_type == CTRL_MSG_SUBTYPE_CMD_TIMER_EXPIRE
	          || msg_type == CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMER_WAKEUP
	          || msg_type == CTRL_MSG_SUBTYPE_STATUS_NVOD_TIMER_EXPIRE
			  || msg_type == CTRL_MSG_SUBTYPE_STATUS_NVOD_TIME_UPDATE)
		ap_timer_message_proc(msg_type, msg_code);
	else if ( msg_type == CTRL_MSG_SUBTYPE_CMD_UPGRADE_SLAVE
	          || msg_type == CTRL_MSG_SUBTYPE_CMD_REBOOT )
	{
#ifdef MULTI_CAS
#if(CAS_TYPE == CAS_ABEL)
		upgrade_serial_proc();
#endif
#endif
		/* Reboot to upgrade */
		power_off_process();
		power_on_process();
	}
	else if (msg_type == CTRL_MSG_SUBTYPE_CMD_PIDCHANGE)
	{
		if ( screen_back_state == SCREEN_BACK_VIDEO
		        || screen_back_state == SCREEN_BACK_RADIO )
		{
			menu = menu_stack_get_top();

			osd_msg_type = (MSG_TYPE_MSG << 16) | (msg_type &0xFFFF);
			osd_msg_code = msg_code;
			if (menu != NULL)
				ret = OSD_ObjProc(menu, osd_msg_type, osd_msg_code, 0);
			else
				ret = PROC_PASS;

			if (ret == PROC_PASS)
			{
				cur_channel = sys_data_get_cur_group_cur_mode_channel();
				api_play_channel(cur_channel, TRUE, FALSE, FALSE);
			}
		}

	}
    else if(msg_type == CTRL_MSG_SUBTYPE_CMD_EPG_FULL)
	{
		epg_off();
		epg_reset();
		cur_channel = sys_data_get_cur_group_cur_mode_channel();
		get_prog_at(cur_channel, &p_node);
		epg_on(p_node.sat_id,p_node.tp_id,p_node.prog_number);
	}
#ifdef TS_MONITOR_FEATURE
    	else if(msg_type == CTRL_MSG_SUBTYPE_CMD_TP_SERVICE_CHANGED)
	{
		INT prog_num, i;
		DB_TP_ID tp_id = (DB_TP_ID)msg_code;
	
		get_cur_view_feature(&view_create_flag_bk, &view_create_parm_bk);
		if(recreate_prog_view(VIEW_SINGLE_TP|PROG_ALL_MODE, tp_id) == DB_SUCCES)
		{
			//delete old data...
			prog_num = get_prog_num(VIEW_SINGLE_TP|PROG_ALL_MODE, tp_id);
			for(i=prog_num-1; i>=0; i--)
			{
				del_prog_at(i);
			}			

			//search new...
			reset_all_flags();
			nvod_release();
			nvod_init(system_config.main_frequency, system_config.main_symbol, system_config.main_qam);
			ts_service_search(tp_id);
		}		
	}
	else if(msg_type == CTRL_MSG_SUBTYPE_CMD_TP_SERVICE_SEARCH_OVER)
	{
		recreate_prog_view(view_create_flag_bk, view_create_parm_bk);
		ts_service_search_end();		
		update_data();
	}
#endif
	else
	{
		#ifdef AD_SANZHOU
		if(msg_type == CTRL_MSG_SUBTYPE_STATUS_SCROLL_STRING)
		{
			win_ad_txt_scrolling();
		}
		#endif
		ap_key_commsg_proc(msg_type, msg_code);
	}

}


void ap_signal_messag_proc(UINT32 msg_type, UINT32 msg_code)
{
	POBJECT_HEAD menu;
	PRESULT proc_ret;
	static UINT32 unlock_count = 0, radio_chanbar_disappear_cnt = 0, unlock = 0;
	BOOL b, bFlag;

	INT8 level, quality, lock;
	signal_lock_status lock_flag;
	signal_scramble_status scramble_flag;
	signal_lnbshort_status lnbshort_flag;
	signal_parentlock_status parrentlock_flag;
	OSD_RECT signal_rect, cross_rect;
	OSD_RECT *pmenu_rect = NULL;

	struct YCbCrColor tColor;
	tColor.uY = 0x10;
	tColor.uCb = 0x80;
	tColor.uCr = 0x80;

	level = (UINT8)(msg_code >> 16);
	quality = (UINT8)(msg_code >> 8);
	lock = (UINT8)(msg_code >> 0);

	if (signal_detect_flag != SIGNAL_CHECK_NORMAL)
		return ;

	menu = menu_stack_get_top();

	if (menu != NULL)
	{
		proc_ret = OSD_ObjProc(menu, (MSG_TYPE_MSG << 16) | msg_type, msg_code, 0);
		if (proc_ret != PROC_PASS)
			return ;
	}


	if (lock)
		unlock_count = 0;
	else
		unlock_count++;

	b = FALSE;

	if ( unlock_count == 2
	        && sys_data_get_cur_chan_mode() == TV_CHAN
	        && screen_back_state == SCREEN_BACK_VIDEO
	        && GetPauseState() == UNPAUSE_STATE
	        && hde_get_mode() != VIEW_MODE_MULTI )
		b = TRUE;


	if (b)
		vdec_io_control(g_decv_dev, VDEC_IO_FILL_FRM, (UINT32)(&tColor));


	/*If currently is in radio mode,and no other menu showed, and not in nvod mode,show channel bar*/
#ifdef NVOD_FEATURE
	if (!GetNvodPlayFlag() && menu == NULL && sys_data_get_cur_chan_mode() == RADIO_CHAN)
#else
	if(menu == NULL && sys_data_get_cur_chan_mode() == RADIO_CHAN)
#endif
	{
		radio_chanbar_disappear_cnt++;
		if (radio_chanbar_disappear_cnt >= 5)
		{
			ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)CHANNEL_BAR_HANDLE, TRUE);
		}
	}
	else
		radio_chanbar_disappear_cnt = 0;
/*    
#if ((SUBTITLE_ON==1)||(TTX_ON==1))
	if((menu == NULL) || (menu == CHANNEL_BAR_HANDLE))
	{
//		bFlag = GetSignalStatus(&lock_flag, &scramble_flag,&lnbshort_flag,&parrentlock_flag); 
        bFlag = GetSignalStatus(&lock_flag, &scramble_flag, &parrentlock_flag);
		
		if(bFlag == TRUE)//when signal abnormal,reset subtitle mode
		{
			api_osd_mode_change(OSD_WINDOW);
		}
		else	if(menu == NULL
		&& GetPauseState() != TRUE
		&& GetSignalStatausShow() != TRUE)
		{
//			api_osd_layer2_onoff(OSDDRV_ON);        
			api_osd_mode_change(OSD_SUBTITLE);
		}	
	}
#endif
*/ 
#if ((defined MULTI_CAS) && (CAS_TYPE != CAS_CONAX) && (CAS_TYPE != CAS_ABEL))
	if (( menu == NULL || menu == CHANNEL_BAR_HANDLE ))
	{
		if (unlock_count >= 2 )
			ap_set_signal_status(TRUE);//TFCAS_SIGNAL_STATUS = TRUE;
		else
			ap_set_signal_status(FALSE);//TFCAS_SIGNAL_STATUS = FALSE;
	}
	
#else	
	b = TRUE;
	if (b && (menu == NULL || menu == CHANNEL_BAR_HANDLE))
	{
		bFlag = GetSignalStatus(&lock_flag, &scramble_flag, &parrentlock_flag);
		b = GetSignalStatausShow(); /* Signal osd displayed ?*/
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_CONAX)                
            if(((b == FALSE && bFlag) || (bFlag != b))&&(ap_get_cas_show_flag()==FALSE))
#else     
             if ((bFlag) || (bFlag != b))
#endif   
#endif
		{
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_CONAX)
			if(get_mmi_showed()!=0)
			{
				if((get_mmi_msg_cnt()>0))
				{
					win_mmipopup_close();
					clean_mmi_cur_msg();
				}
			}
#elif(CAS_TYPE == CAS_ABEL)
			on_event_system_show_no_signal_icon();			
#endif
#endif
			 /*if status is abnormal,we should refresh the OSD promption*/
			api_osd_mode_change(OSD_WINDOW);
			ShowSignalStatusOnOff();
		}
	}
	else
	{
		if ( screen_back_state != SCREEN_BACK_VIDEO
		        && screen_back_state != SCREEN_BACK_VIDEO )
			return ;

		if (GetSignalStatausShow())
		{
			if (menu != NULL)
			{
				pmenu_rect = &menu->frame;
				GetSignalStatusRect(&signal_rect);
				OSD_GetRectsCross(pmenu_rect, &signal_rect, &cross_rect);
				if (cross_rect.uWidth == 0 || cross_rect.uHeight == 0)
					ShowSignalStatusOSDOnOff(0);
			}
			else
				ShowSignalStatusOSDOnOff(0);
		}
	}
#endif

}


/*****************************************
 *Common API to popup a msg box,1second
 *****************************************/
void win_popup_msg(char *str, char *unistr, UINT16 strID)
{
	UINT8 save;
	win_compopup_init(WIN_POPUP_TYPE_SMSG);
	win_compopup_set_msg(str, unistr, strID);
	win_compopup_open_ext(&save);
	osal_task_sleep(1500);
	win_compopup_smsg_restoreback();
}
extern UINT16 manual_upgrade_get_param_pid(void);
extern void manual_upgrade_get_param(UINT32 *,UINT16 *,UINT16 *,UINT32 *);
void ap_manual_upgrade(UINT32 type)
{
	static BOOL ota_rejected=FALSE;
	UINT8 back_saved;
	SYSTEM_DATA *sys_data = sys_data_get();
	win_popup_choice_t sel;
	UINT32 i;
	UINT32 freq;
	UINT32 sym;
	UINT32 qam;
	UINT16 pid;
	UINT8 version_ctrl;
	union UPG_INFO *upg_info = NULL;

	if (CTRL_MSG_SUBTYPE_STATUS_UPGRADE_MANUAL == type)
	{
		if(ota_rejected == TRUE)
			return;

		win_compopup_init(WIN_POPUP_TYPE_OKNO);
		win_compopup_set_msg(NULL, NULL, RS_OTA_MANUAL);
		win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
		sel = win_compopup_open_ext(&back_saved);
		if (sel != WIN_POP_CHOICE_YES)
		{
		//	ota_rejected = TRUE;
			return ;
		}
	}
//	osal_interrupt_disable();

	win_compopup_init(WIN_POPUP_TYPE_SMSG);
	win_compopup_set_msg(NULL, NULL, RS_OTA_FORCE);
	win_compopup_open_ext(&back_saved);

	upg_info =(union UPG_INFO *)stb_info_data_get_upginfo();
	manual_upgrade_get_param(&freq,&sym,&qam,NULL);
	pid = manual_upgrade_get_param_pid();
	//version_ctrl = ota_get_sw_version_ctrl();
	upg_info->c_info.ota_frequency = freq/100;
	upg_info->c_info.ota_symbol = sym;
	upg_info->c_info.ota_qam = (UINT8)qam;
	upg_info->c_info.ota_pid = pid;
	//upg_info->c_info.sw_version_ctrl = 2;//version_ctrl;//xc.liu add fixed manual_upgrade

	upg_info->c_info.upg_trigger_type = STB_INFO_UPG_TRIGGER_APPOTA;
	upg_info->c_info.upg_control_flag = OTA_CTRL_ALL;
	for (i=0; i<VDAC_NUM_MAX; i++)
	{
		upg_info->c_info.vdac_out[i] = sys_data->avset.vdac_out[i];
	}
	upg_info->c_info.osd_lang = sys_data->lang.OSD_lang;

	stb_info_data_save();
	
	osal_task_sleep(3000);
	win_compopup_smsg_restoreback();

	pan_close(g_pan_dev);
	sys_watchdog_reboot();
}

void ap_OTA_upgrade(UINT32 type)
{
	static BOOL ota_rejected=FALSE;
	UINT8 back_saved;
	SYSTEM_DATA *sys_data = sys_data_get();
	win_popup_choice_t sel;
	UINT32 i;
	UINT32 freq;
	UINT32 sym;
	UINT32 qam;
	UINT16 pid;
	UINT8 version_ctrl;
	union UPG_INFO *upg_info = NULL;

	if (CTRL_MSG_SUBTYPE_STATUS_OTA_MANUAL == type)
	{
		if(ota_rejected == TRUE)
			return;

		win_compopup_init(WIN_POPUP_TYPE_OKNO);
		win_compopup_set_msg(NULL, NULL, RS_OTA_MANUAL);
		win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
		sel = win_compopup_open_ext(&back_saved);
		if (sel != WIN_POP_CHOICE_YES)
		{
			ota_rejected = TRUE;
			return ;
		}
	}
//	osal_interrupt_disable();

	win_compopup_init(WIN_POPUP_TYPE_SMSG);
	win_compopup_set_msg(NULL, NULL, RS_OTA_FORCE);
	win_compopup_open_ext(&back_saved);

	upg_info =(union UPG_INFO *) stb_info_data_get_upginfo();

	ota_get_tp_info(&freq,&sym,&qam);
	pid = ota_get_download_pid();
	version_ctrl = ota_get_sw_version_ctrl();
	upg_info->c_info.ota_frequency = freq/100;
	upg_info->c_info.ota_symbol = sym;
	upg_info->c_info.ota_qam = (UINT8)qam;
	upg_info->c_info.ota_pid = pid;
	upg_info->c_info.sw_version_ctrl = version_ctrl;

	upg_info->c_info.upg_trigger_type = STB_INFO_UPG_TRIGGER_APPOTA;
	upg_info->c_info.upg_control_flag = OTA_CTRL_ALL;
	for (i=0; i<VDAC_NUM_MAX; i++)
	{
		upg_info->c_info.vdac_out[i] = sys_data->avset.vdac_out[i];
	}
	upg_info->c_info.osd_lang = sys_data->lang.OSD_lang;

	stb_info_data_save();
	
	osal_task_sleep(3000);
	win_compopup_smsg_restoreback();

	pan_close(g_pan_dev);
	sys_watchdog_reboot();
}

UINT8 cur_channel_is_scrambled()
{
	UINT8 ret = 0, temp = 0;
	UINT8 scramble_type = 0xff;
	UINT16 i;
	struct dmx_device *dmx_dev = (struct dmx_device*)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

	osal_task_sleep(30);
	temp = dmx_io_control(dmx_dev, IS_AV_SCRAMBLED, (UINT32)(&scramble_type));
	//libc_printf("$$$get channel scramble type=0x%x, ret=%d $$$\n",scramble_type,ret);
	if (temp == RET_SUCCESS)
	{
		ret = 1;
		//libc_printf("\n 1  :channel_is_scrambled\n");
	}
	else
	{
		ret = 0;
		//libc_printf("\n 0  :channel_is not_scrambled\n");
	}

	return ret;
}



UINT32 api_resume_play(void)
{
	return 0;
}

void reset_group()
{
	UINT8 cur_group;
	UINT16 cur_channel, max_channel;
	UINT8 group_type, group_pos;
	S_NODE s_node;
	UINT8 cur_mode;

	cur_mode = sys_data_get_cur_chan_mode();
	if (sys_data_get_group_num() == 0)
	{
		recreate_prog_view(VIEW_ALL | cur_mode, 0);
		return ;
	}

	cur_group = sys_data_get_cur_group_index();
	sys_data_set_cur_group_index(cur_group);
	sys_data_get_cur_mode_group_infor(cur_group, &group_type, &group_pos, &cur_channel);

	if (group_type == ALL_GROUP_TYPE)
	{
		PRINTF("========Change to ALL_SATE_GROUP_TYPE\n");
		recreate_prog_view(VIEW_ALL | cur_mode, 0);
	}
	else if (group_type == DATA_GROUP_TYPE)
	{
		get_sat_at(group_pos, VIEW_ALL, &s_node);
		recreate_prog_view(VIEW_SINGLE_SAT | cur_mode, s_node.sat_id);
		PRINTF("========Change to DATA_GROUP_TYPE(%d,%s)\n", group_pos, s_node.sat_name);
	}
	else
	{
		PRINTF("========Change to FAV_GROUP_TYPE(%d)\n", group_pos);
		recreate_prog_view(VIEW_FAV_GROUP | cur_mode, group_pos);
	}
}


BOOL api_set_frontend(UINT32 tp_id,struct ft_frontend *ft)
{
    T_NODE t_node;
    UINT8 nim_cnt;
    
    if(SUCCESS!=get_tp_by_id (tp_id, &t_node ))
        return FALSE;
    nim_cnt = dev_get_device_cnt(HLD_DEV_TYPE_NIM, t_node.ft_type);
    
	if ( nim_cnt > 0 )
	{
		cur_tuner_idx = nim_cnt - 1;
		play_chan_nim_busy = 0;
	}
	else
		play_chan_nim_busy = 1;
	SetChannelNimBusy ( play_chan_nim_busy );

    /*****************************************/
    //set front end 
    ft->nim = g_nim_dev;
    ft->xpond.c_info.type = t_node.ft_type;
    ft->xpond.c_info.tp_id = t_node.tp_id;
    ft->xpond.c_info.frq = t_node.frq;
    ft->xpond.c_info.sym = t_node.sym;
    ft->xpond.c_info.modulation = t_node.FEC_inner;

    return TRUE;
}

void api_set_channel_info(P_NODE *p_node,struct cc_param *param)
{
    SYSTEM_DATA *sys_data = NULL;
    sys_data = sys_data_get();
    
    param->es.prog_id = p_node->prog_id;
    param->es.tp_id = p_node->tp_id;
    param->es.service_id = p_node->prog_number;
    param->es.service_type = p_node->service_type;
    param->es.pcr_pid = p_node->pcr_pid;
    param->es.v_type = MPEG2_VIDEO_STREAM;
    param->es.v_pid = p_node->video_pid;
    param->es.a_type = AUDIO_MPEG2;
    param->es.a_pid = p_node->audio_pid[p_node->cur_audio];
    if(param->es.a_pid&0x2000)
        param->es.a_type = AUDIO_AC3;
    param->es.spdif_enable = 0;
#if(AC3DEC==0)
    param->es.spdif_enable = 1;
#endif

    param->es.a_ch = p_node->audio_channel;
    param->es.a_volumn = sys_data->volume;
    param->es.close_vpo_layer = VPO_LAYER_M;
    param->dev_list.vpo = g_vpo_dev;
    param->dev_list.vdec = g_decv_dev;
    param->dev_list.deca = g_deca_dev;
    param->dev_list.snd_dev = g_snd_dev;
    param->dev_list.dmx = g_dmx_dev;
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_ABEL)
   extern void on_event_cc_post_callback();
    param->callback = on_event_cc_post_callback;
#endif
#endif	
}

void api_stop_play( BOOL blk_srn )
{
    P_NODE p_node;
    UINT16 chan_idx;
    struct cc_param param;
    
    MEMSET(&param, 0, sizeof(param));

    chan_idx = sys_data_get_cur_group_cur_mode_channel();
    if(SUCCESS!=get_prog_at(chan_idx, &p_node))
        return;
    api_set_channel_info(&p_node, &param);
   
    chchg_stop_channel(&param.es, &param.dev_list, blk_srn);
}

//////////for ali ad///////////////////////
static UINT16 ad_cur_chan = 0xffff;
UINT16 ap_get_current_prog(void)
{
    return ad_cur_chan;
}
//////////////////////////////////////
enum API_PLAY_TYPE api_play_channel(UINT32 channel, BOOL b_force, BOOL b_check_password, BOOL bID)
{
#ifdef MIS_AD
	UINT16 mis_app_pid;
#endif
	POBJECT_HEAD menu;
	OSD_RECT rect, crossR;
	BOOL b;
	P_NODE p_node, pre_p_node;
	UINT8 cur_mode, cur_group;
	static UINT8 recall_sate = 0;
	INT8 bPreChannel;

	SYSTEM_DATA *sys_data;
	UINT16 chan_idx;
	UINT32 chan_id;
	UINT32 key_exit[] ={V_KEY_UP, V_KEY_DOWN,V_KEY_MENU,V_KEY_EXIT};
	struct ft_frontend ft;
	struct cc_param param;
	enum API_PLAY_TYPE ret;

	g_cc_play_tick[g_cc_play_step].step = API_PLAY_CHANNEL;
	g_cc_play_tick[g_cc_play_step].start_tick = osal_get_tick();

#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_DVT || (CAS_TYPE==CAS_GY))
	cas_disp_blackscreen_task_del();		//add by Jarod for remove black screen task
#endif	
#endif
	key_set_signal_check_flag(SIGNAL_CHECK_PAUSE);

	if (system_state == SYS_STATE_POWER_OFF)
	{
		ret = API_PLAY_UNEXPECTED_ABORT;
		goto PLAY_EXIT;
	}
	
/*********Step 1: process signalstatus and pause icon*******************/
	menu = menu_stack_get_top();
	if (menu == NULL)
		b = TRUE;
	else
	{
		if (screen_back_state == SCREEN_BACK_MENU)
			b = FALSE;
		else if (menu_stack_get(0) != menu)
		 /* If there are more menus on the stack*/
			b = FALSE;
		else
		{
			GetSignalStatusRect(&rect);
			OSD_GetRectsCross(&menu->frame, &rect, &crossR);
			b = (crossR.uWidth == 0 || crossR.uHeight == 0) ? TRUE : FALSE;
		}
	}
	if (b && GetSignalStatausShow())
		ShowSignalStatusOSDOnOff(0);

	if (GetPauseState())
	{
		if (menu == NULL)
			b = TRUE;
		else
		{
			GetPauseRect(&rect);
			OSD_GetRectsCross(&menu->frame, &rect, &crossR);
			b = (crossR.uWidth == 0 || crossR.uHeight == 0) ? TRUE : FALSE;
		}

		SetPauseOnOff(b);
	}

	g_cc_play_tick[g_cc_play_step++].end_tick = osal_get_tick();
/*********step end*********************************************/
		
/*********Step 2: get channel*******************/
	g_cc_play_tick[g_cc_play_step].step = API_GET_CHANNEL;
	g_cc_play_tick[g_cc_play_step].start_tick = osal_get_tick();

	if (!bID)
	{
		/* If not program, return*/
		if (channel == INVALID_POS_NUM)
		{
			ret = API_PLAY_NOCHANNEL;
			goto PLAY_EXIT;
		}
		if (get_prog_at((UINT16)channel, &p_node) != SUCCESS)
		{
			ret = API_PLAY_NOCHANNEL;
			goto PLAY_EXIT;
		}
		chan_idx = channel;
		chan_id = p_node.prog_id;
	}
	else
	{
		if (get_prog_by_id(channel, &p_node) != SUCCESS)
		{
			ret = API_PLAY_NOCHANNEL;
			goto PLAY_EXIT;
		}
		chan_idx = get_prog_pos(channel);
		if (chan_idx == INVALID_POS_NUM)
			chan_idx = 0;
		chan_id = channel;
	}
	g_cc_play_tick[g_cc_play_step++].end_tick = osal_get_tick();

/*********step end*********************************************/

 	/////////////ad///////////////
	ad_cur_chan = channel;
  
    

/*********Step 3: record recent channel*******************/
	sys_data_set_cur_group_channel(chan_idx);
	cur_mode = sys_data_get_cur_chan_mode();
	cur_group = sys_data_get_cur_group_index();
	if (recall_sate == 1)
	{
		if ( ! ( pre_played_channel.mode == cur_mode
		         && pre_played_channel.group == cur_group
		         && pre_played_channel.channel ==chan_idx ) )
		{
			MEMCPY(&recent_channels[0], &recent_channels[1], sizeof(struct channel_info));
		}
		/* If switch TV/Radio Mode, save power off data immediately.*/
		if (pre_played_channel.mode != cur_mode)
		{
			sys_data_save(TRUE);
		}
	}
	recent_channels[recall_sate].mode = cur_mode;
	recent_channels[recall_sate].internal_group_idx = sys_data_get_cur_intgroup_index();
	MEMCPY(&recent_channels[recall_sate].p_node, &p_node, sizeof(p_node));
	if (recall_sate == 0)
		MEMCPY(&recent_channels[1], &recent_channels[0], sizeof(struct channel_info));
	recall_sate = 1;
	
	if (cur_mode == TV_CHAN)
		screen_back_state = SCREEN_BACK_VIDEO;
	else
	{
		if (screen_back_state != SCREEN_BACK_RADIO)
		{
			api_show_radio_logo();
			screen_back_state = SCREEN_BACK_RADIO;
		}
	}
/*********step end*********************************************/

#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_ABEL)
	on_event_system_do_channel_change();
#endif
#endif

/*********Step 4: check if need password*******************/
	bPreChannel = ((pre_played_channel.mode==cur_mode)&&(pre_played_channel.group==cur_group)
		&&(pre_played_channel.channel==chan_idx)) ? 1 : 0;
	/*check if this channel locked, need password*/
	if ( sys_data_get_channel_lock()
	        && ( p_node.lock_flag || p_node.provider_lock )
	//&& b_check_password
	 && (!bPreChannel || (bPreChannel && GetChannelParrentLock())))
	{
		api_stop_play(screen_back_state != SCREEN_BACK_RADIO);
		SetChannelParrentLock(SIGNAL_STATUS_PARENT_LOCK);
		if (b_check_password)
		{
            win_progname_redraw(TRUE);
			if (!win_pwd_open(key_exit, sizeof(key_exit) / sizeof(UINT32), NULL))
			{
				pre_played_channel.mode = cur_mode;
				pre_played_channel.group = cur_group;
				pre_played_channel.channel =  chan_idx;
				ret = API_PLAY_PASSWORD_ERROR;
				goto PLAY_EXIT;
			}
		}
		else 
		{
			ret = API_PLAY_ABORT_BY_PWD_CHECK;
			goto PLAY_EXIT;
		}
	}
	/* Clear parrent lock status. */
	SetChannelParrentLock(SIGNAL_STATUS_PARENT_UNLOCK);
	pre_played_channel.mode = cur_mode;
	pre_played_channel.group = cur_group;
	pre_played_channel.channel =  chan_idx;
/*********step end*********************************************/


/*********Step 5: play the channel*******************/

	g_cc_play_tick[g_cc_play_step].step = API_SET_CHAN_INFO;
	g_cc_play_tick[g_cc_play_step].start_tick = osal_get_tick();
	MEMSET(&ft, 0, sizeof(ft));
	MEMSET(&param, 0, sizeof(param));
	api_set_channel_info(&p_node, &param);
	g_cc_play_tick[g_cc_play_step++].end_tick = osal_get_tick();

#ifdef AD_TYPE
	AD_ChgChnPre();
#endif
#ifdef MIS_AD
	MIS_SysStop();
#endif
	if(FALSE==api_set_frontend(p_node.tp_id, &ft))
	{
		ret = API_PLAY_FRONTEND_FALIER;
		goto PLAY_EXIT;
	}


#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_ABEL)
	on_event_cc_pre_callback();
#endif
#endif

#if (defined(MULTI_CAS) && ((CAS_TYPE==CAS_DVT)|| (CAS_TYPE==CAS_GY)))
	chchg_play_channel(&ft, &param, TRUE);
#else	
	chchg_play_channel(&ft, &param, FALSE);
#endif
/*********step end*********************************************/
		/* Display cur channel number, except in mode nvod*/
#ifdef NVOD_FEATURE
	if(!GetNvodModeFlag())
#endif
	key_pan_display_channel(chan_idx);


/*********Step 6: *******************/
	/*save current service infor to system data for fast play*/
	if((p_node.service_type==SERVICE_TYPE_DTV) ||(p_node.service_type==SERVICE_TYPE_DRADIO))
	{
		system_config.startup_pid_list[0] = param.es.v_pid;
		system_config.startup_pid_list[1] = param.es.a_pid;
		system_config.startup_pid_list[2] = param.es.pcr_pid;
		system_config.startup_audio_chan = param.es.a_ch;
		system_config.startup_volumn = param.es.a_volumn;
		system_config.startup_fre = ft.xpond.c_info.frq;
		system_config.startup_sym = ft.xpond.c_info.sym;
		system_config.startup_qam = ft.xpond.c_info.modulation;
		if(p_node.ca_mode==1)
			system_config.startup_service_valid = 0;
		else
			system_config.startup_service_valid = 1;

        if(p_node.lock_flag==1)
            system_config.startup_lock=1;
        else
            system_config.startup_lock=0;
	}
	else
	{
		system_config.startup_service_valid = 0;
	}
/*********step end*********************************************/

		
	sys_data = sys_data_get();
	/* Enable TDT parssing. */
	if (sys_data->local_time.buseGMT)
	{
		enable_time_parse();
	}
	ret = API_PLAY_NORMAL;

PLAY_EXIT:
	key_set_signal_check_flag(SIGNAL_CHECK_RESET);
#ifdef AD_TYPE
	AD_ChgChnPost();
#endif
	#ifdef MIS_AD
	osal_task_sleep(65);
	if(MIS_GetAppPid(&mis_app_pid) != 0){
		mis_app_pid = 0x1fff;
	}
	MIS_SysStart(/*0x1b62*/mis_app_pid, FALSE);
	#endif
	return ret;
}




/* return value: 
 * 		0XFFFF -  invalid, and no program
 * 		>0     -  valid program id		  
 */
UINT16 find_valid_channel(TIMER_SET_CONTENT *timer)
{
	P_NODE p_node;
	UINT16 channel = 0;
	UINT8 cur_mode,save;
	UINT32 prog_id;

	prog_id = timer->wakeup_channel;
	if (get_prog_by_id(prog_id, &p_node) != DB_SUCCES)
	{
		timer->timer_mode = TIMER_MODE_OFF;
		win_compopup_init(WIN_POPUP_TYPE_OK);
		win_compopup_set_frame(100, 100, 400, 80);
		win_compopup_set_msg(NULL, NULL, RS_MSG_TIMER_CH_NOEXIST);
		win_compopup_open_ext(&save);
		channel = 0XFFFF;
		return channel;
	}
	
	channel = get_prog_pos(prog_id);

	if (channel == P_INVALID_ID)
	 /*If current group has not timer channel */
	{
		sys_data_set_cur_group_index(0);
		sys_data_change_group(0); /*  Switch to all group */
		channel = get_prog_pos(prog_id);

		if (channel != P_INVALID_ID)
		 /* find channel */
			sys_data_set_cur_group_index(0);
	}
	if (channel == P_INVALID_ID)
	{
		cur_mode = sys_data_get_cur_chan_mode();
		if (cur_mode == RADIO_CHAN)
			sys_data_set_cur_chan_mode(TV_CHAN);
		else
			sys_data_set_cur_chan_mode(RADIO_CHAN);

		sys_data_change_group(0); /*  Switch to all sate group */
		channel = get_prog_pos(prog_id);
	}
	if (channel == P_INVALID_ID)
		channel = 0;
	sys_data_set_cur_group_channel(channel);

	return channel;

}


#if(TTX_ON==1)
UINT8 TTX_KeyMap(UINT32 msgtype, UINT32 msgcode)
{
	TTX_KEYTYPE TTX_Key = PROC_PASS;
	
	switch(msgcode)
    {
	case V_KEY_0:
		TTX_Key = TTX_KEY_NUM0;
		break;
	case V_KEY_1:
		TTX_Key = TTX_KEY_NUM1;
		break;
	case V_KEY_2:
		TTX_Key = TTX_KEY_NUM2;
		break;
	case V_KEY_3:
		TTX_Key = TTX_KEY_NUM3;
		break;
	case V_KEY_4:
		TTX_Key = TTX_KEY_NUM4;
		break;
	case V_KEY_5:
		TTX_Key = TTX_KEY_NUM5;
		break;
	case V_KEY_6:
		TTX_Key = TTX_KEY_NUM6;
		break;
	case V_KEY_7:
		TTX_Key = TTX_KEY_NUM7;
		break;
	case V_KEY_8:
		TTX_Key = TTX_KEY_NUM8;
		break;
	case V_KEY_9:
		TTX_Key = TTX_KEY_NUM9;
		break;
	case V_KEY_UP:
		TTX_Key = TTX_KEY_UP;
		break;
	case V_KEY_DOWN:
		TTX_Key = TTX_KEY_DOWN;
		break;
    case V_KEY_LEFT:
        TTX_Key = TTX_KEY_LEFT;
        break;
    case V_KEY_RIGHT:
        TTX_Key = TTX_KEY_RIGHT;
        break;

    case V_KEY_RED:
        TTX_Key = TTX_KEY_RED;
        break;
    case V_KEY_GREEN:         
        TTX_Key = TTX_KEY_GREEN;
        break;
    case V_KEY_YELLOW:
        TTX_Key = TTX_KEY_YELLOW;
        break;
    case V_KEY_BLUE:       
        TTX_Key = TTX_KEY_CYAN;
        break;

    case V_KEY_ENTER:
		TTX_Key = TTX_KEY_ALPHA;
        break;
	case V_KEY_TEXT:
    case V_KEY_EXIT:
		TTX_Key = PROC_LEAVE;
		break;
	default:
		TTX_Key = PROC_PASS;
			break;
	}        		
    return TTX_Key;
}

void TTX_OSDOpen(void)
{
	UINT8 TTX_Key = PROC_LOOP;
    struct t_ttx_lang *lang_list;
	struct t_subt_lang	*sublang_list;
	UINT8 lang_num,sel_lang = 0;
	UINT32 key,vkey; 
	SYSTEM_DATA* sys_data;
	sys_data = sys_data_get();

	TTXEng_GetInitLang(&lang_list, &lang_num);
	sel_lang = api_ttxsub_getlang(lang_list, NULL,sys_data->lang.ttx_lang,lang_num);
	if(sel_lang == 0xFF)
	{
//		api_osd_layer2_onoff(OSDDRV_OFF);
		return;//NO Teletext!!
	}

	if(OSD_GetDrawMode() != GE_DRAW_MODE)
		OSDDrv_ShowOnOff((HANDLE)g_osd_dev,OSDDRV_OFF);
	api_osd_mode_change(OSD_TELETEXT);

	TTXEng_SetInitLang(sel_lang);
	while(TTX_Key != PROC_LEAVE)
	{
		key = 0;
		key = ap_get_key_msg();
		ap_hk_to_vk(0, key,&vkey);
		
		if(vkey == V_KEY_NULL)
			continue;

		TTX_Key = TTX_KeyMap( CTRL_MSG_SUBTYPE_KEY, vkey);
		if(TTX_Key == PROC_LEAVE)
			break;
		TTXEng_SendKey(TTX_Key);
	}	

	api_osd_mode_change(OSD_WINDOW);

	if(OSD_GetDrawMode() != GE_DRAW_MODE)
		OSDDrv_ShowOnOff((HANDLE)g_osd_dev,OSDDRV_ON);
	ap_clear_all_message();
	ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32)CHANNEL_BAR_HANDLE,TRUE);
}

#endif


#ifdef MULTI_CAS
#if(CAS_TYPE == CAS_IRDETO)

void  init_msg_backlog()										
{																
	msg_back_log_head = (msg_backlog_t *)MALLOC(sizeof(msg_backlog_t));
	if(msg_back_log_head)										
		msg_back_log_head->next = msg_back_log_head->prev = msg_back_log_head;																				
}


void add_msg_backlog(msg_backlog_t *msg)
{
	msg->prev = (msg_back_log_head->prev)?msg_back_log_head->prev : msg_back_log_head;
	msg->next = msg_back_log_head;
	
	msg->prev->next = msg;
	msg_back_log_head->prev = msg;
}


msg_backlog_t *remove_head_from_backlog()
{
	msg_backlog_t * ret = msg_back_log_head->next;

	msg_back_log_head->next = msg_back_log_head->next->next;
	msg_back_log_head->next->prev = msg_back_log_head;
	
	ret->next = NULL;
	ret->prev = NULL;
	return ret;
}



void popfrom_backlog()  
{								
		CA_MAIL_T * pmail = NULL;
		msg_backlog_t * msg =NULL;
		UINT32 nr=0,idx=0;
		BOOL sr=FALSE;																
		if(msg_back_log_head->next != msg_back_log_head )
		{											
			msg = remove_head_from_backlog();
			if(msg->msg_data.msgSbuType==MCAS_DISP_MAIL_MSG)	
				nr = CA_GetAllMail_info((CA_MAIL_T *)&pmail);
			else
				nr = CA_GetAllAnn_info((CA_MAIL_T *)&pmail);
			
			for(idx=0; idx<nr; idx++)
			{
				if(pmail[idx].bIndex == msg->msg_data.msgCode-NR_BACKMSG_BASE)
					break;
			}				
			
			if(idx==nr)
				goto DISCARD;		
			sr=ap_send_msg_expand(CTRL_MSG_SUBTYPE_STATUS_MCAS,msg->msg_data.msgSbuType, msg->msg_data.msgCode, FALSE);
			if(sr)																	
				FREE(sr);															
			else																	
				libc_printf("resend falied in %s line:d%",__FILE__,__LINE__);		
		}
		return;	

DISCARD:
	libc_printf("the mail index:d% was discarded!!!\n",msg->msg_data.msgCode);
	return;																	
}  
#endif
#endif

UINT8 gy_switch_debug_get(void)
{
	SYSTEM_DATA *sys_data;
	sys_data = sys_data_get();

	return (sys_data->osd_set.osd_trans == 0 ? 0 : 1);
}

void ap_nit_update_notify(UINT32 nit)
{
	static DWORD startTimeValue = 0;
	
	// Because of the next there statement, the notify window cannot receive
	// any messages after opened and if the user leaves from house. So we 
	// get time and do some judgements below.
	POBJECT_HEAD window = menu_stack_get_top();
	// if there already exist an infomation window...
	if(window == &g_win_information){
		DWORD currentTimeValue = osal_get_time();
		if(currentTimeValue - startTimeValue >= 60){
			ap_send_msg(CTRL_MSG_SUBTYPE_KEY, 0x3100F807, FALSE);
			/*// :(     :)
			system_config.nit_version = nit;
			// clear the information window
			OSD_ClearObject(&g_win_information, C_UPDATE_ALL);
			// open search window
			POBJECT_HEAD window = (POBJECT_HEAD)&g_win_search;
			if(PROC_LEAVE != OSD_ObjOpen(window, 0xffffffff))
				menu_stack_push(window);
			*/
		}
		return;
	}

	// the 1st time 
	startTimeValue = osal_get_time();
	window = (POBJECT_HEAD)&g_win_information;
	if(OSD_ObjOpen(window, nit) != PROC_LEAVE)
		menu_stack_push(window);
}
