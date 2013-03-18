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

/*******************************************************************************
 *	Objects definition
 *******************************************************************************/
extern CONTAINER g_win_volume;
extern BITMAP vol_bmp;
extern PROGRESS_BAR vol_bar;


static VACTION win_volume_bar_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT win_volume_bar_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);


static VACTION win_volume_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT win_volume_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);


#define WIN_SH_IDX				WSTL_VOLUME_01
#define VOLUME_BMP_SH_IDX		WSTL_TXT_3
#define VOLUME_SH_IDX			WSTL_VOLUME_01_BG
#define VOLUME_MID_SH_IDX		0	/*WSTL_VOLUME_01_BG*/
#define VOLUME_FG_SH_IDX		WSTL_VOLUME_01_FG

#define W_L  (TV_OFFSET_L+105)
#define W_T  (TV_OFFSET_T+400)
#define W_W  505
#define W_H  40

#define BMP_W		40
#define BMP_H		40
#define BMP_L_OFF	10
#define BMP_T_OFF	((W_H - BMP_H)/2)

#define R_L_OFF   50
#define R_T_OFF   4
#define R_R_OFF   10
#define R_L (W_L + R_L_OFF)
#define R_T W_T
#define R_W	(W_W - R_L_OFF - R_R_OFF)
#define R_H	W_H
#define RCT_W		R_W
#define RCT_H		(W_H - R_T_OFF*2)

#define LDEF_BMP(root,varBmp,nxtObj,l,t,w,h,icon)		\
	DEF_BITMAP(varBmp,root,nxtObj,C_ATTR_ACTIVE,0, \
	    0,0,0,0,0, l,t,w,h, VOLUME_BMP_SH_IDX,VOLUME_BMP_SH_IDX,VOLUME_BMP_SH_IDX,VOLUME_BMP_SH_IDX,   \
	    NULL,NULL,  \
	    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,icon)


#define LDEF_PROGRESS_BAR(root,varBar,nxtObj,l,t,w,h,style,rl,rt,rw,rh)	\
	DEF_PROGRESSBAR(varBar, root, nxtObj, C_ATTR_ACTIVE, 0, \
		1, 1, 1, 1, 1, l, t, w, h, VOLUME_SH_IDX, VOLUME_SH_IDX, VOLUME_SH_IDX, VOLUME_SH_IDX,\
		win_volume_bar_keymap, win_volume_bar_callback, style, 4, 0, VOLUME_MID_SH_IDX, VOLUME_FG_SH_IDX, \
		rl,rt , rw, rh, 0, 100, 45, 1)

#define LDEF_WIN(varWnd,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varWnd,NULL,NULL,C_ATTR_ACTIVE,0, \
	    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
	    win_volume_keymap,win_volume_callback,  \
	    nxtObj, focusID,1)


LDEF_BMP ( &g_win_volume, vol_bmp, &vol_bar, W_L + BMP_L_OFF, W_T + BMP_T_OFF, BMP_W, W_H, IM_VOLUME )

LDEF_PROGRESS_BAR ( &g_win_volume, vol_bar, NULL, R_L , R_T, R_W, R_H, PBAR_STYLE_RECT_STYLE | PROGRESSBAR_HORI_NORMAL, \
                    0, R_T_OFF, RCT_W, RCT_H )

LDEF_WIN ( g_win_volume, &vol_bmp, W_L, W_T, W_W, W_H, 1 )

/*******************************************************************************
 *	Local vriable & function declare
 *******************************************************************************/

static ID volume_timer = OSAL_INVALID_ID;
UINT32 VOLUME_TIMER_TIME;
#define VOLUME_TIMER_NAME 	"vol"

void volume_timer_func(UINT unused);

/*******************************************************************************
 *	key mapping and event callback definition
 *******************************************************************************/

static VACTION win_volume_bar_keymap(POBJECT_HEAD pObj, UINT32 key)
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

static PRESULT win_volume_bar_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	PROGRESS_BAR *bar;
	SYSTEM_DATA *sys_data;

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
			sys_data->startup_volumn = sys_data->volume;
			if (sys_data->volume > 0 && GetMuteState())
				SetMuteOnOff(TRUE);
			api_audio_set_volume(sys_data->volume);
			api_stop_timer(&volume_timer);
			volume_timer = api_start_timer(VOLUME_TIMER_NAME, VOLUME_TIMER_TIME, volume_timer_func);
			break;
		default:
			;
	}

	return ret;
}

static VACTION win_volume_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	return act;
}

static PRESULT win_volume_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	SYSTEM_DATA *sys_data;
	PROGRESS_BAR *bar;

	bar = &vol_bar;

	sys_data = sys_data_get();

	switch (event)
	{
		case EVN_PRE_OPEN:
			VOLUME_TIMER_TIME = sys_data->osd_set.time_out;
			if (VOLUME_TIMER_TIME == 0 || VOLUME_TIMER_TIME > 10)
				VOLUME_TIMER_TIME = 5;
			VOLUME_TIMER_TIME *= 1000;
			OSD_SetProgressBarPos(bar, sys_data->volume);
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


void volume_timer_func(UINT unused)
{
	api_stop_timer(&volume_timer);
	ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT, 1, TRUE);
}
