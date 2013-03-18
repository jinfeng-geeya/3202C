#include <sys_config.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_pub.h>


//#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"
#ifdef AD_SANZHOU
#include "sanzhou_ad/adprocess.h"
#endif
#include "win_com.h"
#include "menus_root.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

//include the header from xform 
#include "win_volumebar_vega.h"

/*******************************************************************************
 *	Local vriable & function declare
 *******************************************************************************/
static ID volume_timer = OSAL_INVALID_ID;
UINT32 VOLUME_TIMER_TIME;
#define VOLUME_TIMER_NAME 	"vol"

static void volume_timer_func(UINT unused);

/*******************************************************************************
*	Callback and Keymap Functions
*******************************************************************************/

static VACTION volumebar_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	return act;
}

static PRESULT volumebar_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	SYSTEM_DATA *sys_data;
	PROGRESS_BAR *bar;
	TEXT_FIELD *txt;

	txt = &volumebar_txt2;
	bar = &volumebar_progressbar;
	
	sys_data = sys_data_get();

	switch (event)
	{
		case EVN_PRE_OPEN:
			display_strs_init(0, 0);
			VOLUME_TIMER_TIME = sys_data->osd_set.time_out;
			if (VOLUME_TIMER_TIME == 0 || VOLUME_TIMER_TIME > 10)
				VOLUME_TIMER_TIME = 5;
			VOLUME_TIMER_TIME *= 1000;
			sys_data->volume = (sys_data->volume) / 3;//内部最大为67，现在progressbar 为32
			OSD_SetProgressBarPos(bar, sys_data->volume);
			OSD_SetTextFieldContent(txt,STRING_NUMBER, sys_data->volume);
            		//ad_show(AD_VOLUME);
			break;
		case EVN_POST_OPEN:
			#ifdef AD_SANZHOU
			{
				OSD_RECT vol_pic_rt = {510,400,90,90};
				szxc_ad_show_pic(AD_VOLUME, 0, vol_pic_rt);
			}
			#endif
			volume_timer = api_start_timer(VOLUME_TIMER_NAME, VOLUME_TIMER_TIME, volume_timer_func);
			#ifdef MIS_AD
			MIS_ShowEpgAdv(1);
			#endif
			break;
		case EVN_UNKNOWNKEY_GOT:
			ap_send_msg(CTRL_MSG_SUBTYPE_KEY, param1, FALSE);
			ret = PROC_LEAVE;
			break;
		case EVN_MSG_GOT:
			if (param1 == CTRL_MSG_SUBTYPE_CMD_EXIT)
			{
				ret = PROC_LEAVE;
			}
			break;
		case EVN_PRE_CLOSE:
            		//ad_stop(AD_VOLUME);
		#ifdef MIS_AD
			MIS_HideEpgAdv();
		#endif
			break;
		case EVN_POST_CLOSE:
			#ifdef AD_SANZHOU
			szxc_ad_hide_pic(AD_VOLUME);
			#endif
			api_stop_timer(&volume_timer);
			break;
		default:
			;
	}


	return ret;
}

static VACTION volumebar_bar_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act;
	switch (key)
	{
		case V_KEY_LEFT:
		case V_KEY_V_DOWN:
			act = VACT_DECREASE;
			break;
		case V_KEY_RIGHT:
		case V_KEY_V_UP:
			act = VACT_INCREASE;
			break;
		default:
			act = VACT_PASS;
	}
	return act;
}

static PRESULT volumebar_bar_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	PROGRESS_BAR *bar;
	SYSTEM_DATA *sys_data;
	TEXT_FIELD *txt;

	txt = &volumebar_txt2;
	
	bar = (PROGRESS_BAR*)pObj;

	sys_data = sys_data_get();

	switch (event)
	{
		case EVN_PRE_CHANGE:
			break;
		case EVN_POST_CHANGE:
			#ifdef MIS_AD
			MIS_ShowEpgAdv(1);
		#endif
			sys_data->volume = OSD_GetProgressBarPos(bar);
			sys_data->startup_volumn = (sys_data->volume)*3;
			
			OSD_SetTextFieldContent(txt,STRING_NUMBER, sys_data->volume);
			
			OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
			
			if (sys_data->volume > 0 && GetMuteState())
				SetMuteOnOff(TRUE);
			api_audio_set_volume((sys_data->volume)*3);
			sys_data->volume = (sys_data->volume)*3;
			api_stop_timer(&volume_timer);
			volume_timer = api_start_timer(VOLUME_TIMER_NAME, VOLUME_TIMER_TIME, volume_timer_func);
			break;
		default:
			;
	}

	return ret;
}

static void volume_timer_func(UINT unused)
{
	api_stop_timer(&volume_timer);
	ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT, 1, TRUE);
}

