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
#include "menus_root.h"

#include "copper_common/system_data.h"
#include "copper_common/menu_api.h"
#include "win_com_menu_define.h"
#include "win_com_popup.h"
#include "win_com.h"

#include "win_pwd_set_obj.h"

UINT8 win_pl_password_value[2][4];
UINT8 g_PL_Input_Bit_Number;
UINT8 g_PL_Input_Status;

/*******************************************************************************
 *	Local functions & variables define
 *******************************************************************************/
CONTAINER *par_items[] =
{
        &par_item_con3,
        &par_item_con4,
};

POBJECT_HEAD par_item_name[] =
{
        ( POBJECT_HEAD ) &par_item_txtname3,
        ( POBJECT_HEAD ) &par_item_txtname4,
};

POBJECT_HEAD par_item_set[] =
{
        ( POBJECT_HEAD ) &par_item_txtset3,
        ( POBJECT_HEAD ) &par_item_txtset4,
};

void win_par_switch_confirm(BOOL flag)
{
	UINT8 action;

	if (flag)
		action = C_ATTR_ACTIVE;
	else
		action = C_ATTR_INACTIVE;

	if (!OSD_CheckAttr(&par_item_con4, action))
	{
		OSD_SetAttr(&par_item_con4, action);
		OSD_SetAttr(&par_item_txtname4, action);
		OSD_SetAttr(&par_item_txtset4, action);
		OSD_DrawObject((POBJECT_HEAD) &par_item_con4, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
	}

}

void win_pl_set_password_str(UINT8 index)
{
	UINT8 temp[5];
	if ((g_PL_Input_Status == TRUE) && (g_PL_Input_Bit_Number > 0))
	{
		switch (g_PL_Input_Bit_Number)
		{
			case 1:
				sprintf(temp, "*   ");
				break;
			case 2:
				sprintf(temp, "**  ");
				break;
			case 3:
				sprintf(temp, "*** ");
				break;
			case 4:
				sprintf(temp, "****");
				break;
		}
	}
	else
	{
		sprintf(temp, "    ");
	}
	ComAscStr2Uni(temp, display_strs[index]);
}

/*******************************************************************************
 *	key mapping and event callback definition
 *******************************************************************************/

static VACTION par_item_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_RIGHT:
			act = VACT_INCREASE;
			break;
		case V_KEY_LEFT:
			act = VACT_DECREASE;
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
				act = VACT_NUM_0 + (key - V_KEY_0);
			break;
		default:
			act = VACT_PASS;
			break;
	}

	return act;

}

void win_pl_input_number(UINT8 item, UINT8 number)
{
	// jinfeng: fixed a bug here, which translate the new password "0000" to a NULL('\0') string
	// and error occured in par_con_callback's compare statement.
	// The number 0-9 should be translated to char '0'-'9', so, ascii value is 48-57
	
	number += '0';	// Jinfeng Added
	
	if (g_PL_Input_Status == TRUE)
	{
		switch (g_PL_Input_Bit_Number)
		{
			case 0:
			case 4:
				win_pl_password_value[item][0] = number;
				g_PL_Input_Bit_Number = 1;
				break;
			case 1:
			case 2:
			case 3:
				win_pl_password_value[item][g_PL_Input_Bit_Number] = number;
				g_PL_Input_Bit_Number++;
				break;
		}
	}
	else
	{
		g_PL_Input_Status = TRUE;
		win_pl_password_value[item][0] = number;
		g_PL_Input_Bit_Number = 1;
	}
}

static PRESULT par_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 unact, input, save;
	UINT8 vkey;
	UINT8 id = OSD_GetObjID(pObj);
	SYSTEM_DATA *p_sys_data = sys_data_get();

	switch (event)
	{
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			vkey = param1 &MENU_OPEN_PARAM_MASK;
			if (unact <= VACT_NUM_9)
			{
				win_pl_input_number(id - MENU_PARENTAL_PASSWORD, (UINT8)(vkey - V_KEY_0));
				win_pl_set_password_str(id - MENU_PARENTAL_PASSWORD);
				OSD_TrackObject(pObj, C_UPDATE_ALL);
			}
			ret = PROC_LOOP;
			break;
	}
	return ret;

}

static VACTION par_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_MENU:
		case V_KEY_EXIT:
			sys_data_save(1);
			act = VACT_CLOSE;
			break;
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
		default:
			act = VACT_PASS;
			break;
	}

	return act;
}

static PRESULT par_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT32 i;
	UINT8 unact,save;
	CONTAINER *cont = (CONTAINER*)pObj;
	UINT8 id = OSD_GetObjID(pObj);
	SYSTEM_DATA *p_sys_data= sys_data_get();
	TEXT_FIELD *txt;
	txt = &pwdset_msg;

	UINT8 tmp;
	
	switch (event)
	{
		case EVN_PRE_OPEN:
			g_PL_Input_Bit_Number = 0;
			g_PL_Input_Status = FALSE;
			MEMSET(win_pl_password_value[0], 0, sizeof(win_pl_password_value[0]));
			MEMSET(win_pl_password_value[1], 0, sizeof(win_pl_password_value[1]));
			win_pl_set_password_str(0);
			win_pl_set_password_str(1);
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			sys_data_save(1);
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1>>16);
			if(unact == VACT_ENTER)
			{
				tmp = sizeof(win_pl_password_value[0]);
				tmp = sizeof(win_pl_password_value);
				if ((0 == MEMCMP(win_pl_password_value[0], win_pl_password_value[1], sizeof(win_pl_password_value[0])))\
					&& (0 != MEMCMP(win_pl_password_value[0],"",sizeof(win_pl_password_value))) )
				{
					win_pl_set_password_str(1);
					OSD_SetTextFieldContent(txt, STRING_ID, RS_MSG_CHANGE_PASSWORD_SUCCESS);
					OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
					OSD_SetTextFieldContent(txt, STRING_ID, RS_EMPTY);
					MEMCPY(p_sys_data->menu_password, win_pl_password_value[0], sizeof(p_sys_data->menu_password));
				}
				else
				{
					g_PL_Input_Bit_Number = 0;
					g_PL_Input_Status = FALSE;
					MEMSET(win_pl_password_value[1], 0, sizeof(win_pl_password_value[1]));
					win_pl_set_password_str(1);
					OSD_SetTextFieldContent(txt, STRING_ID, RS_PWDSET_MSG_ERROR);
					OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
					OSD_SetTextFieldContent(txt, STRING_ID, RS_EMPTY);
				}
			}
			break;
	}
	return ret;
}

static VACTION pwdset_con_keymap(POBJECT_HEAD pObj, UINT32 key)
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
		default:
			act = VACT_PASS;
			break;
	}

	return act;
}

