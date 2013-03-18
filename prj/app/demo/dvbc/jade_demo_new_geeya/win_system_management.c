#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

//#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "win_com.h"
#include "menus_root.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"
#include "win_system_management_obj.h"

static PRESULT sys_mg_message_proc(UINT32 msg_type, UINT32 msg_code);

#define PREVIEW_L  (TV_OFFSET_L+64)
#define PREVIEW_T  (TV_OFFSET_T+96)
#define PREVIEW_W  290
#define PREVIEW_H  234

#define PREVIEW_INNER_L  (PREVIEW_L+24)
#define PREVIEW_INNER_T  (PREVIEW_T+25)
#define PREVIEW_INNER_W  (PREVIEW_W-24-25)
#define PREVIEW_INNER_H  (PREVIEW_H-25-29-30)

#define CHILDLOCK_ID 3
/*******************************************************************************
 *	local variable define
 *******************************************************************************/
struct help_item_resource win_sys_help[] =
{
    {1,IM_HELP_ICON_LR,RS_CHANGE_VOL},
    {0,RS_HELP_ENTER,RS_HELP_ENTER_INTO},
    {0,RS_MENU,RS_HELP_BACK},
    {0,RS_HELP_EXIT,RS_HELP_EXIT},          
};
#if 0
POBJECT_HEAD windows_list[] =
{
    (POBJECT_HEAD)&g_win_chan_management,
    (POBJECT_HEAD)&g_win_sys_items,
    (POBJECT_HEAD)&g_win_sys_information,
#ifdef IRD_OTA_SUPPORT
    (POBJECT_HEAD)&g_win_ird_ota_upgrade
#endif
};
#endif

POBJECT_HEAD windows_list[] =
{
	(POBJECT_HEAD)&g_win_manual_search,
    (POBJECT_HEAD)&g_win_chan_manage,//&g_win_chan_management,---#JingJin
    (POBJECT_HEAD)&g_win_childlock,//Child limit
    (POBJECT_HEAD)&g_win_advset,//¸ß¼¶ÉèÖÃ
    (POBJECT_HEAD)&g_win_sys_information,
    (POBJECT_HEAD)&g_win_signal0,    //g_win_signal_test,
    (POBJECT_HEAD)&g_win_languageset,
    (POBJECT_HEAD)&g_ca_message,//NULL,//CA Information  ----yuanlin
    (POBJECT_HEAD)&g_win_reserve
#ifdef IRD_OTA_SUPPORT
    (POBJECT_HEAD)&g_win_ird_ota_upgrade
#endif
};

#define VACT_ITEM_SELECT	    (VACT_PASS + 4)

/*******************************************************************************
 *	helper functions define
 *******************************************************************************/
static VACTION sys_mangagement_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	INT8 shift = 1;

	switch (key)
	{
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		case V_KEY_EXIT:
			BackToFullScrPlay();
			break;
		case V_KEY_LEFT:
			shift =  - 1;
		case V_KEY_RIGHT:
		//	PreviewVolProc(shift);
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}

extern UINT8 show_and_playchannel;

static PRESULT sys_mangagement_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	OSD_RECT rc_con;
	OSD_RECT rc_preview;

	switch (event)
	{
		case EVN_PRE_OPEN:
		//	wincom_open_title_ext(RS_SYSTEM_SETUP, IM_TITLE_ICON_SYSTEM);
		//	wincom_open_help(win_sys_help, 4);
		//	set_update_flag(TRUE);
			break;
		case EVN_POST_OPEN:
			rc_con.uLeft = PREVIEW_L;
			rc_con.uTop = PREVIEW_T;
			rc_con.uWidth = PREVIEW_W;
			rc_con.uHeight = PREVIEW_H;

			rc_preview.uLeft = PREVIEW_INNER_L;
			rc_preview.uTop = PREVIEW_INNER_T;
			rc_preview.uWidth = PREVIEW_INNER_W;
			rc_preview.uHeight = PREVIEW_INNER_H;

			//wincom_open_preview_ext(rc_con, rc_preview, WSTL_SUBMENU_PREVIEW);
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			//wincom_close_preview();
			break;
		case EVN_MSG_GOT:
			ret = sys_mg_message_proc(param1, param2);
			break;
		case EVN_UNKNOWN_ACTION:
			break;
	}

	return ret;
}

static VACTION sys_mangagement_item_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;


	switch (key)
	{
		case V_KEY_UP:
			act = VACT_CURSOR_UP;
			break;
		case V_KEY_DOWN:
			act = VACT_CURSOR_DOWN;
			break;
		case V_KEY_ENTER:
			act = VACT_ITEM_SELECT;
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}


static PRESULT sys_mangagement_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 id = 0;
	VACTION act;
	OSD_RECT rct;
	rct.uHeight = 164;
	rct.uWidth = 355;
	rct.uLeft = 302;
	rct.uTop = 178;
	
	switch (event)
	{
		case EVN_UNKNOWN_ACTION:
			act = (VACTION)(param1 >> 16);
			switch (act)
			{
			case VACT_ITEM_SELECT:
				id = OSD_GetFocusID(pObj);
				if (NULL != windows_list[id - 1])
				{
					if(id == CHILDLOCK_ID)
					{
						if (!win_pwd_open(NULL, 0, &rct))
							break;
					}
					if (OSD_ObjOpen(windows_list[id - 1], 0xFFFFFFFF) != PROC_LEAVE)
						menu_stack_push(windows_list[id - 1]);
				}
				break;
			default:
				break;
			}
			ret = PROC_LOOP;
			break;
		default:
			break;
	}
	return ret;
}


static PRESULT sys_mg_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;

	switch (msg_type)
	{
		case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
			//wincom_preview_proc();
			break;
		case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
			if (is_time_inited())
			{
			//	wincom_draw_title_time();
			}
            break;
		default:
			break;
	}

	return ret;
}
