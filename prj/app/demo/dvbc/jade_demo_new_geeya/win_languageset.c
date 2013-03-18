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
#include "osd_rsc.h"

#include "copper_common/system_data.h"
#include "copper_common/menu_api.h"
//#include "win_com_menu_define.h"
#include "win_com.h"

//include the header from xform 
#include "win_languageset_vega.h"

#define VACT_SET_DECREASE	(VACT_PASS + 1)
#define VACT_SET_INCREASE	(VACT_PASS + 2)

static void sysLanguageDataLoad();
static void sysLanguageChange(VACTION act);
/*******************************************************************************
*	Callback and Keymap Functions
*******************************************************************************/
static VACTION languageset_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_EXIT:
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
	}

	return act;

}

static PRESULT languageset_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;

	switch (event)
	{
		case EVN_PRE_OPEN:
			sysLanguageDataLoad();
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;//可消除闪屏现象---#JingJin
			break;
	}
	return ret;
}

static VACTION languageset_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_LEFT:
			act = VACT_SET_DECREASE;
			break;
		case V_KEY_RIGHT:
			act = VACT_SET_INCREASE;
			break;
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
		default:
			act = VACT_PASS;
	}

	return act;

}

// modified by jinfeng
static PRESULT languageset_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	TEXT_FIELD *txtTime = &languageset_state;

	switch (event)
	{
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			if(unact == VACT_SET_DECREASE || unact == VACT_SET_INCREASE)
			{
				//sysLanguageChange(unact); 
				if (RS_SYSTEM_LANGUAGE_CHINESE == txtTime->wStringID)
					txtTime->wStringID = RS_SYSTEM_LANGUAGE_ENGLISH;
				else
					txtTime->wStringID = RS_SYSTEM_LANGUAGE_CHINESE;
				OSD_TrackObject((POBJECT_HEAD) &g_win_languageset, C_UPDATE_ALL);
			}
			
			if(unact == VACT_ENTER)
			{
				sysLanguageChange(unact);
				sys_data_save(1);
				ret = PROC_LEAVE;
			}
			break;
	}
	return ret;
}

static void sysLanguageDataLoad()
{
	SYSTEM_DATA *pSysData = NULL;
	pSysData = sys_data_get();
	win_osd_set_language_string(pSysData->lang.OSD_lang);
}

/*set language string on osd*/
static void win_osd_set_language_string(UINT8 input)
{
	UINT16 lang_string_id = 0;

	TEXT_FIELD *txtTime = &languageset_state;
	lang_string_id = (input == 0) ? RS_SYSTEM_LANGUAGE_ENGLISH : RS_SYSTEM_LANGUAGE_CHINESE;
	OSD_SetTextFieldContent(txtTime, STRING_ID, lang_string_id);
}

/* here we not use multitsel,so all the items value change is handed in this function*/
// modified by jinfeng
static void sysLanguageChange(VACTION act)
{
	SYSTEM_DATA *pSysData = NULL;
	pSysData = sys_data_get();
	TEXT_FIELD *txtTime = &languageset_state;

	/* this code just support two language, if need more to support,can change to switch*/
	if (RS_SYSTEM_LANGUAGE_CHINESE == txtTime->wStringID)
		pSysData->lang.OSD_lang = 1;
	else
		pSysData->lang.OSD_lang = 0;

	sys_data_select_language(pSysData->lang.OSD_lang);
	/*we redraw all the objects to show the effects that language changed*/
	OSD_TrackObject((POBJECT_HEAD) &g_win_languageset, C_UPDATE_ALL);
}

