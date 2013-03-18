#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

//#include <api/libge/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "win_com.h"
#include "menus_root.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

#include "win_chan_management_obj.h"

static PRESULT chan_mg_message_proc(UINT32 msg_type, UINT32 msg_code);

/*******************************************************************************
 *	Objects & callback functions declaration
 *******************************************************************************/
/*rectangle parameters & style define for g_win_chan_management*/
#define WIN_BG_IDX   WSTL_SEARCH_BG_CON

#define WIN_BG_L     (TV_OFFSET_L+0)
#define WIN_BG_T     (TV_OFFSET_T+90)
#define WIN_BG_W     720
#define WIN_BG_H     400

/**********rectangle parameters & style define for con_sub_chan_menu**********/
#define CON_SUB_MENU_IDX   WSTL_WIN_1

#define CON_SUB_MENU_L    (TV_OFFSET_L+356)
#define CON_SUB_MENU_T    (WIN_BG_T+12)
#define CON_SUB_MENU_W    314
#define CON_SUB_MENU_H    364

/**********rectangle parameters & style define for items********/
#define TXT_TITLE_IDX     WSTL_TXT_4

#define ITEM_L       (TV_OFFSET_L+360)
#define ITEM_T       (CON_SUB_MENU_T+4)
#define ITEM_W       304
#define ITEM_H       30
#define TXT_L_OF     4
#define TXT_T_OF     30

/*********rectangle parameters & style define for preview********/
#define PREVIEW_IDX  WSTL_SUBMENU_PREVIEW

#define PREVIEW_L  (TV_OFFSET_L+64)
#define PREVIEW_T  (TV_OFFSET_T+96)
#define PREVIEW_W  290
#define PREVIEW_H  234

#define PREVIEW_INNER_L  (PREVIEW_L+24)
#define PREVIEW_INNER_T  (PREVIEW_T+25)
#define PREVIEW_INNER_W  (PREVIEW_W-24-27)
#define PREVIEW_INNER_H  (PREVIEW_H-25-29-30)

/*******************************************************************************
*	local variable define
 *******************************************************************************/
struct help_item_resource win_chan_help[] =
{
    {1,IM_HELP_ICON_LR,RS_CHANGE_VOL},
    {0,RS_HELP_ENTER,RS_HELP_ENTER_INTO},
    {0,RS_MENU,RS_HELP_BACK},
    {0,RS_HELP_EXIT,RS_HELP_EXIT},          
};

POBJECT_HEAD windows_chan_list[4] =
{
    (POBJECT_HEAD)&g_win_prog_search,
    (POBJECT_HEAD)&g_win_reserve,
    (POBJECT_HEAD)&g_win_freqset,//g_win_main_freq,
    (POBJECT_HEAD)&g_win_chanedit,
};

#define CHEN_EDIT 4
#define VACT_ITEM_SELECT	    (VACT_PASS + 3)

/*******************************************************************************
 *	helper functions define
 *******************************************************************************/
static VACTION chan_mangagement_keymap(POBJECT_HEAD pObj, UINT32 key)
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
			PreviewVolProc(shift);
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}


static PRESULT chan_mangagement_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	OSD_RECT rc_con;
	OSD_RECT rc_preview;

	switch (event)
	{
		case EVN_PRE_OPEN:
			wincom_open_title_ext(RS_CHN_MANAGEMENT, IM_TITLE_ICON_SYSTEM);
			wincom_open_help(win_chan_help, 4);
			//set_update_flag(TRUE);
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

			wincom_open_preview_ext(rc_con, rc_preview, WSTL_SUBMENU_PREVIEW);
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			//wincom_close_preview();
			break;
		case EVN_MSG_GOT:
			ret = chan_mg_message_proc(param1, param2);
			break;
		case EVN_UNKNOWN_ACTION:
			break;
	}

	return ret;
}

static VACTION chan_mangagement_item_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act;

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


static PRESULT chan_mangagement_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 id = 0;
	VACTION act;
	OSD_RECT rct;
	
	rct.uHeight = 130;
	rct.uWidth = 200;
	rct.uLeft = PREVIEW_L + (PREVIEW_W - rct.uWidth) / 2;
	rct.uTop = PREVIEW_T + PREVIEW_H;

	switch (event)
	{
		case EVN_UNKNOWN_ACTION:
			act = (VACTION)(param1 >> 16);
			switch (act)
			{
			case VACT_ITEM_SELECT:
				id = OSD_GetFocusID(pObj);
				if (NULL != windows_chan_list[id - 1])
				{
					if(id == CHEN_EDIT)
					{
						if (!win_pwd_open(NULL, 0, &rct))
							break;
					}
					if (OSD_ObjOpen(windows_chan_list[id - 1], 0xFFFFFFFF) != PROC_LEAVE)
						menu_stack_push(windows_chan_list[id - 1]);
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


static PRESULT chan_mg_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;

	switch (msg_type)
	{
		case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
			wincom_preview_proc();
			break;
		case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
			if (is_time_inited())
			{
				wincom_draw_title_time();
			}
            break;
		default:
			break;
	}

	return ret;
}
