#include <sys_config.h>

#include <types.h>
#include <basic_types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include "win_cas_com.h"

//include the header from xform 
#include "win_ca_message.h"

extern CONTAINER g_ca_message;
extern TEXT_FIELD win_ca_message_title;
extern CONTAINER ca_message_item1;
extern TEXT_FIELD txt_ca_message_item1;
extern CONTAINER ca_message_item2;
extern CONTAINER g_ca_message;
extern CONTAINER ca_message_item3;
extern TEXT_FIELD txt_ca_message_item3;
extern TEXT_FIELD txt_ca_message_item2;
extern BITMAP bmp_ca_message_title;
extern CONTAINER ca_message;
extern BITMAP bmp_ca_message_title1;

#define VACT_ITEM_SELECT	    (VACT_PASS + 4)
/*******************************************************************************
*	Callback and Keymap Functions
*******************************************************************************/
POBJECT_HEAD ca_message_windows_list[] =
{
	(POBJECT_HEAD)&g_win_smc_info,
    (POBJECT_HEAD)&g_win_operator_info,
    (POBJECT_HEAD)&g_win_ca_set
};
static VACTION ca_message_keymap(POBJECT_HEAD pObj, UINT32 key)
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
static VACTION ca_message_item_keymap(POBJECT_HEAD pObj, UINT32 key)
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
static PRESULT ca_message_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	

	switch (event)
	{
		case EVN_PRE_OPEN:
			//OSD_ClearObject((POBJECT_HEAD) &g_win_sys_management, C_UPDATE_ALL);
			//hde_set_mode(0);
			
			break;
		case EVN_POST_OPEN:
			
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			
			break;
		case EVN_MSG_GOT:
			
			break;
		case EVN_UNKNOWN_ACTION:
			break;
	}

	return ret;
}
static PRESULT ca_message_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
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
				if (NULL !=  ca_message_windows_list[id - 1])
				{
					if (OSD_ObjOpen( ca_message_windows_list[id - 1], 0xFFFFFFFF) != PROC_LEAVE)
						menu_stack_push( ca_message_windows_list[id - 1]);
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
