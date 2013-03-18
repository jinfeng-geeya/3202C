#if 0
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include "win_cas_com.h"

//include the header from xform 
#include "win_operator_message.h"

#define OPERATOR_POPUP_ITEMS 5

#define OPINFO_SERVICE_ENTITLE			1
#define OPINFO_WALLET_INFO			2
#define OPINFO_IPPV_INFO				3
#define OPINFO_CHILD_PARENT_CARD		4
#define OPINFO_DETITLE					5


static void win_new_operator_set_display(void);
static UINT8 exit_ind = 0;
UINT16 operator_num1 = 0;
UINT16 CAS_operator_ID[CDCA_MAXNUM_OPERATOR];
UINT16 current_operator_ID;
SCDCAOperatorInfo CAS_operator_info[CDCA_MAXNUM_OPERATOR];
#define VACT_OPERATOR_POPUP (VACT_PASS + 1)
/*******************************************************************************
*	Callback and Keymap Functions
*******************************************************************************/
#if 0

UINT16 operator_popup[OPERATOR_POPUP_ITEMS]=
{
	RS_SERVICE_ENTITLES,
	RS_SLOT_INFO,
	RS_IPPV_PROGRAM,
	RS_CHILD_PARENT_CARD,
	RS_DETITLE
};
UINT8 win_ca_operator_get_num()
{
	return OPERATOR_POPUP_ITEMS;
}

VACTION comlist_menu_ca_operator_keymap ( POBJECT_HEAD pObj, UINT32 vkey )
{
	PRESULT ret = VACT_PASS;
	switch ( vkey )
	{
		case V_KEY_MENU:
        case V_KEY_RECALL:
		case V_KEY_EXIT:
			ret = VACT_CLOSE;
			exit_ind = 1;
			break;
	}

	return ret;
}
PRESULT comlist_menu_ca_operator_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 )
{
	UINT16 i, wTop;
	OBJLIST* ol;

	PRESULT cbRet = PROC_PASS;

	ol = ( OBJLIST* ) pObj;

	if ( event == EVN_PRE_DRAW )
	{
		wTop = OSD_GetObjListTop ( ol );
		for ( i = 0;i < ol->wDep && ( i + wTop ) < ol->wCount;i++ )
		{
			win_comlist_set_str ( i + wTop, NULL, NULL, operator_popup[i]);
		}
	}
	else if ( event == EVN_POST_CHANGE )
		cbRet = PROC_LEAVE;

	return cbRet;
}

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
			temp = (POBJECT_HEAD)&g_win_feedcard;
			break;
		case OPINFO_DETITLE:
			temp = (POBJECT_HEAD)&g_win_detitle;
			break;
		default:
			break;
	}

	if(temp != NULL)
		if (OSD_ObjOpen(temp, 0xFFFFFFFF) != PROC_LEAVE)
			menu_stack_push(temp);
}

void Operator_enter_key(POBJECT_HEAD pObj, UINT8 id) 
{
	UINT8 sel = 0;
	OSD_RECT rect;
	COM_POP_LIST_PARAM_T param;
/*
	rect.uLeft = ITEM_L+ITEM_NAME_L;
	rect.uWidth = ITEM_NAME_W;
	rect.uHeight = 150;
	rect.uTop = ITEM_T + (ITEM_H + ITEM_GAP)*(1+id);
	param.selecttype = POP_LIST_SINGLESELECT;
	param.cur = 0;
*/
	current_operator_ID = CAS_operator_ID[id];
	sel = win_com_open_sub_list(POP_LIST_MENU_CA_OPERATOR_INFO, &rect, &param);
	if (1 == exit_ind) //Because if rcv "exit/Menu" key, it would return still return a valid sel value
	{
		exit_ind = 0;
		return;
	}
	else
		operator_info_enter_detail(sel+1);
}

#endif




static VACTION op_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch(key)
	{
		case V_KEY_EXIT:
			act = VACT_CLOSE;
			//BackToFullScrPlay();
			break;
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
	
	}

	return act;

}
static VACTION op_list_keymap(POBJECT_HEAD pObj, UINT32 key)
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
	case V_KEY_P_UP:
		act = VACT_CURSOR_PGUP;
		break;			
	case V_KEY_P_DOWN:
		act = VACT_CURSOR_PGDN;
		break;	
		break;		
	case V_KEY_ENTER:
	case V_KEY_RIGHT:
		act = VACT_OPERATOR_POPUP;
		break;
	default:
		act = VACT_PASS;
	}

	return act;

}

extern BOOL pwd_for_operator_info;
extern CONTAINER g_win_operator_info_detail;

static PRESULT op_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT32 cnt = 0, retval;
	

	switch(event)
	{
	case EVN_PRE_OPEN:
		OSD_ClearObject((POBJECT_HEAD) &g_win_sys_management, C_UPDATE_ALL);
		hde_set_mode(0);
		operator_num1 = 0;
		MEMSET(CAS_operator_ID,0, sizeof(CAS_operator_ID));
		MEMSET(CAS_operator_info, 0, sizeof(CAS_operator_info));
        //retval = operator_new_got_info();
		if (0 == retval)
		{
			ret = PROC_LEAVE;
			break;
		}
	
		win_init_pstring(18);
		OSD_SetObjListCount(&op_ol,operator_num1);
		break;
	case EVN_POST_OPEN:
		break;
	case EVN_PRE_CLOSE:
		/* Make OSD not flickering */
		*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;	
		break;
	case EVN_POST_CLOSE:
		break;
	}

	return ret;
}


// 0 for error,  1 for success
INT32 operator_new_got_info()
{
	UINT8 back_saved;
	UINT16 ret,i, j;
	
	ret = CDCASTB_GetOperatorIds(CAS_operator_ID);   //得到ID
	if ((CDCA_RC_POINTER_INVALID== ret) ||(CDCA_RC_POINTER_INVALID == ret))  //没卡的情况
	{
		win_popup_msg(NULL, NULL, RS_CAERR_NOSMC);
		
		MEMSET(CAS_operator_ID,0, sizeof(CAS_operator_ID));
		MEMSET(CAS_operator_info, 0, sizeof(CAS_operator_info));
		return 0;

	}
	else if (CDCA_RC_OK == ret)
	{
		for (i=0; i<CDCA_MAXNUM_OPERATOR; i++)
		{
			if (0 == CAS_operator_ID[i])
				break;
		}
		operator_num1 = i;
		for (j=0; j<operator_num1; j++)
		{
			ret = CDCASTB_GetOperatorInfo(CAS_operator_ID[j], &CAS_operator_info[j]);
			if ((CDCA_RC_CARD_INVALID== ret) ||(CDCA_RC_POINTER_INVALID == ret))
			{
				win_popup_msg(NULL, NULL, RS_CAERR_NOSMC);
				return 0;
			}
			else if (CDCA_RC_DATA_NOT_FIND == ret)
			{
				win_popup_msg(NULL, NULL, RS_CAERR_DATA_NOT_FOUND);//没有找到
				return 0;	
			}
			else if (CDCA_RC_OK == ret)
			{
				//OPERATOR_PRINT("@@@@operator %d: name: %s", CAS_operator_ID[j], CAS_operator_info[j].m_szTVSPriInfo);
			}
			else
			{
				win_popup_msg(NULL, NULL, RS_CAERR_UNKNOWN);  //未知错误
				return 0;
			}
		}
		
	}
	else
	{
		win_popup_msg(NULL, NULL, RS_CAERR_UNKNOWN);
		return 0;
	}

	//check if there is no readed retitle
	j=0;
	for (i=0; i<operator_num1; i++)
	{
		ret = CDCASTB_GetDetitleReaded(CAS_operator_ID[i]);
		if (FALSE == ret)
		{
			j = 1;
			break;
		}
	}

	if (0==j) //all operator detitle have been read
	{
		ap_cas_callback(CAS_MSG_DETITLE_RECEIVED, CDCA_Detitle_All_Readed);
	}
	return 1;
}

static PRESULT op_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	UINT16 uni_name[30];
	UINT8* new_name;
	OBJLIST* ol;
	UINT16 curitem;
    

	ol = &op_ol;
	curitem = OSD_GetObjListCurPoint(ol);

	switch(event)
	{
		case EVN_PRE_DRAW:
			win_new_operator_set_display();
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1>>16);
			if(unact == VACT_OPERATOR_POPUP)
			{
				current_operator_ID = CAS_operator_ID[curitem];
				//OSD_ObjOpen((POBJECT_HEAD)&g_win_operator_info_detail,0);
               // menu_stack_push((POBJECT_HEAD)&g_win_operator_info_detail);
			}
			break;
	}

	return ret;
}
static void  win_new_operator_set_display(void)
{
	CONTAINER* con;
	TEXT_FIELD* text;
	OBJLIST* ol;	
	
	UINT16 operator_name[CDCA_MAXLEN_TVSPRIINFO+1];
	UINT16 top,cnt,page,index;
	INT32 j = -1;
	UINT32 infoid;
	
	ol = &op_ol;

	page = OSD_GetObjListPage(ol); 
	//here total 4 provider are supported
	if (operator_num1 != 0)
	{
		MEMSET(operator_name, 0, sizeof(operator_name));
		for(index=0;index<page;index++)
	    	{
	    	
	    	   con = (CONTAINER*)op_ol_ListField[index];
			
		      /* IDX */
		       text = (TEXT_FIELD*)OSD_GetContainerNextObj(con);
		       if (index < operator_num1)
		        {
			      sprintf(text, "%02d", index + 1);
			      OSD_SetTextFieldContent(text, STRING_ANSI, (UINT32)text);
		        }
		       else
			      OSD_SetTextFieldContent(text, STRING_ANSI, (UINT32)"");
			   
			   
			
		        /* num */
		              text = (TEXT_FIELD*)OSD_GetObjpNext(text);// text = (TEXT_FIELD*)OSD_GetContainerNextObj(); 

				if (index < operator_num1)
				{
					
				OSD_SetTextFieldContent(text, STRING_NUMBER, (UINT32)CAS_operator_ID[index]);
				}
				else
					OSD_SetTextFieldContent(text, STRING_ANSI, (UINT32)"");

		 
		        /* Name**/
		        text = (TEXT_FIELD*)OSD_GetObjpNext(text);

				if (index < operator_num1)
				{
				convert_gb2312_to_unicode( CAS_operator_info[index].m_szTVSPriInfo, CDCA_MAXLEN_TVSPRIINFO,
										   operator_name, CDCA_MAXLEN_TVSPRIINFO);
					OSD_SetTextFieldContent(text, STRING_UNICODE, (UINT32)&operator_name);
				}
				else
					OSD_SetTextFieldContent(text, STRING_ANSI, (UINT32)"");

	        
		}
	}
	
}
#endif
