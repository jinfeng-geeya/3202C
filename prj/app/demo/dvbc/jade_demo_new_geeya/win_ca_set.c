#include <sys_config.h>

#include <types.h>
#include <basic_types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/osd/osddrv_dev.h>
#include <api/libge/osd_lib.h>
#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"

#include "control.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"

//include the header from xform 
#include "win_ca_set.h"

POBJECT_HEAD ca_set_windows_list[] =
{
	(POBJECT_HEAD)&g_win_pin_pwd,
    (POBJECT_HEAD)&g_win_maturity_rating,
    (POBJECT_HEAD)&g_win_binding
};

#define VACT_ITEM_SELECT	    (VACT_PASS + 4)


/*******************************************************************************
*	Callback and Keymap Functions
*******************************************************************************/
static VACTION Containerca_set1_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	UINT32 hKey;

	switch (key)
	{
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		case V_KEY_EXIT:
			BackToFullScrPlay();
			break;
		default:
			act = VACT_PASS;
	}

	return act;

}
static VACTION Containerca_set2_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	//UINT32 hKey;

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
static PRESULT Containerca_set1_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	

	switch (event)
	{
		case EVN_PRE_OPEN:
			OSD_ClearObject((POBJECT_HEAD) &g_win_sys_management, C_UPDATE_ALL);
			hde_set_mode(0);
			//wincom_close_preview();
		//	wincom_open_title_ext(RS_SYSTEM_SETUP, IM_TITLE_ICON_SYSTEM);
		//	wincom_open_help(win_sys_help, 4);
			//set_update_flag(TRUE);
			break;
		case EVN_POST_OPEN:
			
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			//wincom_close_preview();
			break;
		case EVN_MSG_GOT:
			//ret = sys_mg_message_proc(param1, param2);
			break;
		case EVN_UNKNOWN_ACTION:
			break;
	}

	return ret;
}
static PRESULT Containerca_set2_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 id = 0;
	VACTION act;

	switch (event)
	{
		case EVN_UNKNOWN_ACTION:
			act = (VACTION)(param1 >> 16);
			switch (act)
			{
			case VACT_ITEM_SELECT:
				id = OSD_GetFocusID(pObj);
				if (NULL != ca_set_windows_list[id - 1])
				{
					if (OSD_ObjOpen(ca_set_windows_list[id - 1], 0xFFFFFFFF) != PROC_LEAVE)
						menu_stack_push(ca_set_windows_list[id - 1]);
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

