#include <sys_config.h>

#include <types.h>
#include <basic_types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/osd/osddrv_dev.h>
//#include <api/libosd/osd_lib.h>
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
#include "win_advset_vega.h"

POBJECT_HEAD advset_list[5] = 
{
	(POBJECT_HEAD)&g_win_displayset,
	(POBJECT_HEAD)&g_win_audioset,
	(POBJECT_HEAD)&g_win_factoryset,
	(POBJECT_HEAD)&g_win_freqset,
	(POBJECT_HEAD)&g_win_manual_update,
};
#define FACTORYSET_ID	3
#define FREQSET_ID	4
/*******************************************************************************
*	Callback and Keymap Functions
*******************************************************************************/
static VACTION advset_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch(key)
	{	
		case V_KEY_EXIT:
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
			break;
	}
	return act;
}
static PRESULT advset_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	
	switch (event)
	{
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
	}
	return ret;
}

static VACTION advset_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_UP:
			act  = VACT_CURSOR_UP;
			break;
		case V_KEY_DOWN:
			act  = VACT_CURSOR_DOWN;
			break;
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
		default:
			act = VACT_PASS;
	}

	return act;

}
static PRESULT advset_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	UINT8 id = OSD_GetFocusID(pObj);
	OSD_RECT rct;

	rct.uHeight = 164;
	rct.uWidth = 355;
	rct.uLeft = 302;
	rct.uTop = 178;

	switch (event)
	{
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			if(unact == VACT_ENTER)
			{
				if((id == FACTORYSET_ID) || (id == FREQSET_ID))
				{
					if (!win_pwd_open(NULL, 0, &rct))
							break;
				}
				if(OSD_ObjOpen(advset_list[id-1], 0xFFFFFFFF) != PROC_LEAVE)
					menu_stack_push(advset_list[id-1]);
			}
			break;
	}
	return ret;
}
