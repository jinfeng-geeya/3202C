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
#include <api/libcas/gy/gyca_def.h>
#include <api/libcas/gy/gyca_interface.h>
#include "cdca/win_cas_com.h"
//include the header from xform 
#include "win_ca_binding.h"

void smc_binding_info_display(void);
/*******************************************************************************
*	Callback and Keymap Functions
*******************************************************************************/
static VACTION win_binding_keymap(POBJECT_HEAD pObj, UINT32 key)
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
		default:
			act = VACT_PASS;
	}

	return act;

}
static PRESULT win_binding_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	INT32 pin_ret;
	switch(event)
	{
		case EVN_UNKNOWN_ACTION:
			unact = ( VACTION ) ( param1 >> 16 ) ;
			if(unact == VACT_ENTER)
			{
				pin_ret = GYCA_SetPaired(pin_pwd_password_str[PIN_ARRAY_0]);

				if (GYCA_OK == pin_ret)
				{
					win_popup_msg(NULL, NULL, RS_GY_PAIRE_SUCCESS);
					pin_pwd_init(1UL<<PIN_ARRAY_0);
					pin_pwd_set_status(0, FALSE);
					pin_pwd_set_str(PIN_ARRAY_0);
					OSD_TrackObject ( pObj, C_UPDATE_ALL );
					ret = PROC_LEAVE; 
					return 1;
				}
				else
				{
					win_popup_msg(NULL, NULL, RS_GY_PAIRE_FAILED);
					ret = PROC_LOOP;
				}
			}
			break;
	}
	return ret;
}
static VACTION binding_item_keymap(POBJECT_HEAD pObj, UINT32 key)
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
		default:
			act = VACT_PASS;
	}

	return act;

}
static PRESULT binding_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	switch(event)
	{
		case EVN_PRE_DRAW:
			smc_binding_info_display();
			break;
	}
	return ret;
}
static VACTION binding_pwd_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	UINT32 hKey;

	switch (key)
	{
		case V_KEY_LEFT:
			act = VACT_EDIT_LEFT;
			break;
		case V_KEY_RIGHT:
			act = VACT_EDIT_RIGHT;
			break;
		case V_KEY_0:
		case V_KEY_1:
		case V_KEY_2:
		case V_KEY_3:
		case V_KEY_4:
		case V_KEY_5:
		case V_KEY_6:
		case V_KEY_7:
		case V_KEY_8:
		case V_KEY_9:
			act = VACT_NUM_0 + ( key - V_KEY_0 );
			break;
		default:
			act = VACT_PASS;
			break;
	}

	return act;

}

static PRESULT binding_pwd_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8	unact;
	UINT8	id = OSD_GetObjID ( pObj );
	switch (event)
	{
		case EVN_UNKNOWN_ACTION:
			unact = ( VACTION ) ( param1 >> 16 ) ;
			if ( unact <= VACT_NUM_9 )
			{
				pin_pwd_input(PIN_ARRAY_0, (UINT8)(unact - VACT_NUM_0));
				pin_pwd_set_str(PIN_ARRAY_0);
				OSD_TrackObject (pObj, C_UPDATE_ALL );
				ret = PROC_LOOP;
				break;
			}
			else if (unact == VACT_EDIT_LEFT)
			{
				pin_pwd_drawback(PIN_ARRAY_0);
				OSD_TrackObject (pObj, C_UPDATE_ALL );
				ret = PROC_LOOP;
			} 
			break;
	}
	return ret;
}
void smc_binding_info_display(void)
{
	UINT32 strid;
	UINT8 tmp[50];
	UINT16  id;
	INT32 ret;
	TEXT_FIELD *bind_statu;
	bind_statu = &binding_result;
	/*机卡配对信息*/
	ret = GYCA_GetPaired();
	if (GYCA_OK== ret)
	{
		strid = RS_CAERR_PAIRED_CURRENT;
	}
	else if (GYCA_CARD_NOPAIR == ret)
	{
		strid = RS_CAERR_NOT_PAIRED;
	}
	else if (GYCA_CARD_MISPAIR == ret)
	{
		strid = RS_CAERR_PAIRED_OTHERS;
	}
	else
	{
		strid = RS_CAERR_NOSMC;
        	MEMSET(tmp,0,50);
	}
	OSD_SetTextFieldContent(bind_statu, STRING_ID, strid);
}

