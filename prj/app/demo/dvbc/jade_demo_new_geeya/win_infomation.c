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
#include "win_infomation.h"

//==============================
// global variable for nit_version_check
//==============================

UINT8 nit_cur_version;


/*******************************************************************************
*	Callback and Keymap Functions
*******************************************************************************/
static VACTION g_win_infomation_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	UINT32 hKey;

	switch (key)
	{
		case V_KEY_EXIT:
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
		case V_KEY_UP:
			act = VACT_CURSOR_UP;
			break;
		default:
			act = VACT_PASS;
	}

	return act;

}
static PRESULT g_win_infomation_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	static UINT8 version = 0;			// as param2 always change, we save it in corrent condition!
	static BOOL FlagWindowOpen= TRUE;	// by default, we think user leave home, we select auto update
	
	PRESULT ret = PROC_PASS;
	VACTION unact;

	switch (event)
	{
		// Two conditions: 1. new version  2. back from search window
		case EVN_PRE_OPEN:

			// if user say: don't show it again, we ...
			if(FlagWindowOpen == FALSE){
				ret = PROC_LEAVE;
				break;
			}
			// In 1st condition, if there has existed an information window, break;
			// In 2nd condition, we return to previous window.
			if(&g_win_information== menu_stack_get_top()){
				ret = PROC_LEAVE;	// quick !I don't want to see you again!
				break;
			}
			// In 1st condition, we save the version passed by param2, which 
			// will  be stored to system data when 'Enter' has been pressed.
			//version = (UINT8)param2;
			break;

		// Whatever user selected, if the window closed, it wouldn't show again!
		case EVN_PRE_CLOSE:
			
			FlagWindowOpen = FALSE;
			
			break;
			
		case EVN_UNKNOWN_ACTION:
			
			unact = (VACTION)(param1>>16);
			
			if(VACT_ENTER == unact){
				// store the current version into system data
				//system_config.nit_version = version;
				system_config.nit_version = nit_cur_version;
				sys_data_save(TRUE);

				win_set_search_param(0, 0, 0, 0, 1);
				// clear the information window
				OSD_ClearObject(&g_win_information, C_UPDATE_ALL);
				// open search window
				POBJECT_HEAD window = (POBJECT_HEAD)&g_win_search;
				if(PROC_LEAVE != OSD_ObjOpen(window, 0xffffffff))
					menu_stack_push(window);
			}
			
			ret = PROC_LOOP;
			break;

		//case EVN_KEY_GOT:
		case EVN_MSG_GOT:
			system_config.nit_version++;
			break;
	}
	return ret;
}
