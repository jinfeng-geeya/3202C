#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
//#include <api/libosd/osd_lib.h>

#include "win_cas_com.h"

//include the header from xform 
#include "win_ca_operator_info_detail.h"

/*******************************************************************************
*	Local vriable & function declare
*******************************************************************************/
#define OPINFO_SERVICE_ENTITLE			1
#define OPINFO_WALLET_INFO				2
#define OPINFO_IPPV_INFO				3
#define OPINFO_CHILD_PARENT_CARD		4
#define OPINFO_DETITLE					5
#define OPINFO_AREA_CODE				6

extern UINT8 From_op_info_menu;

/*******************************************************************************
*	key mapping and event callback definition
*******************************************************************************/

static void operator_info_enter_detail(ID id)
{
	POBJECT_HEAD temp = NULL;
	UINT8 a=1;
	switch(id)
	{
		case OPINFO_SERVICE_ENTITLE:
			temp = (POBJECT_HEAD)&g_win_service_entitle;
			break;
		case OPINFO_WALLET_INFO:
			temp = (POBJECT_HEAD)&g_win_walletinfo;
			break;
		case OPINFO_IPPV_INFO:
			temp = (POBJECT_HEAD)&g_win_IPPVinfo;
			break;
		case OPINFO_CHILD_PARENT_CARD:
//			From_op_info_menu = 1;
			temp = (POBJECT_HEAD)&g_win_feedcard;
			break;
		case OPINFO_DETITLE:
			temp = (POBJECT_HEAD)&g_win_detitle;
			break;
		case OPINFO_AREA_CODE:
			temp = (POBJECT_HEAD)&g_win_eigenvalue_info;
			break;
		default:
			break;
	}

	if(temp != NULL)
	{
		//OSD_ObjClose((POBJECT_HEAD)&g_win_operator_info_detail, C_CLOSE_CLRBACK_FLG);
		//wincom_close_help(1);
		if (OSD_ObjOpen(temp, 0xFFFFFFFF)!= PROC_LEAVE)
		{
			menu_stack_push(temp);
		}
		else
		{
			OSD_ObjOpen((POBJECT_HEAD)&g_win_operator_info_detail, 0xFFFFFFFF);
		}
	}
}

static VACTION operator_detail_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act;
	
	switch(key)
	{
	case V_KEY_UP:
		act = VACT_CURSOR_UP;
		break;			
	case V_KEY_DOWN:
		act = VACT_CURSOR_DOWN;
		break;			
	case V_KEY_ENTER:
		act = VACT_ENTER;
		break;
	default:
		act = VACT_PASS;
	}

	return act;
}

extern UINT16 current_operator_ID;
static PRESULT operator_detail_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	UINT16 uni_name[30];
	UINT8* new_name;
	UINT16 curitem;
    UINT8 back_saved,crc[10];

	curitem = OSD_GetFocusID((POBJECT_HEAD)&operator_detail_con);

	switch(event)
	{
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1>>16);
			if(unact == VACT_ENTER)
			{
			    #if 0
                if(6 == curitem)
                {
            
                    CDCA_U32  ACArray[CDCA_MAXNUM_ACLIST];
                    CDCA_U16  wOperatorID = 12345;
                    if(CDCASTB_GetACList(current_operator_ID,ACArray) == CDCA_RC_OK)
                	{
                		sprintf(crc,"%08x",ACArray[0]);
                	}
                    else
                        break;
                    win_compopup_init(WIN_POPUP_TYPE_OK);
                    win_compopup_set_title(NULL, NULL, RS_PAY_ATTENTION);                    
        			win_compopup_set_msg(crc, NULL, 0);
        			win_compopup_open_ext(&back_saved);
                    break;
		
			
                }
					#endif
				operator_info_enter_detail(curitem);
			}
			break;
	}

	return ret;
}

static VACTION operator_detail_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch(key)
	{
		case V_KEY_EXIT:
			BackToFullScrPlay();
			break;
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
	
	}

	return act;
}


extern struct help_item_resource win_ca_help[];
static PRESULT operator_detail_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT32 cnt = 0, retval;
	struct help_item_resource win_oid_help[] =
	{
		{1,IM_HELP_ICON_TB,RS_HELP_SELECT},
    		{0,RS_HELP_ENTER,RS_HELP_ENTER_INTO},
    		{0,RS_MENU,RS_HELP_BACK},
    		{0,RS_HELP_EXIT,RS_FULL_PLAY},
	};

	switch(event)
	{
		case EVN_PRE_OPEN:
			wincom_open_title_ext(RS_OPERATOR_INFORMATION, IM_TITLE_ICON_SYSTEM);
			wincom_open_help(win_oid_help, 4);
			break;
		case EVN_POST_OPEN:
			//wincom_open_preview_default();
			break;
		case EVN_PRE_CLOSE:
			/* Make OSD not flickering */
			*((UINT32*)param2)&= ~C_CLOSE_CLRBACK_FLG;	
			break;
		case EVN_MSG_GOT:
			wincom_preview_proc();
			break;
		case EVN_POST_CLOSE:
			break;
	}

	return ret;
}

