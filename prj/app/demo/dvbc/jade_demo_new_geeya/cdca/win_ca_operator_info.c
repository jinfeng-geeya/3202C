#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include <api/libcas/gy/gyca_def.h>
#include <api/libcas/gy/gyca_interface.h>

#include "win_cas_com.h"
#include "win_ca_operator_info.h"

/*******************************************************************************
*	Objects definition
*******************************************************************************

extern CONTAINER g_win_operator_info;
extern CONTAINER win_operator_info_con;

extern OBJLIST	 operator_olist;

extern CONTAINER operator_item0;
extern CONTAINER operator_item1;
extern CONTAINER operator_item2;
extern CONTAINER operator_item3;
extern CONTAINER operator_item4;
extern CONTAINER operator_item5;
extern CONTAINER operator_item6;
extern CONTAINER operator_item7;
extern CONTAINER operator_item8;


extern TEXT_FIELD operator_table_title_id;
extern TEXT_FIELD operator_table_title_ppname;

extern TEXT_FIELD operator_idx0;
extern TEXT_FIELD operator_idx1;
extern TEXT_FIELD operator_idx2;
extern TEXT_FIELD operator_idx3;
extern TEXT_FIELD operator_idx4;
extern TEXT_FIELD operator_idx5;
extern TEXT_FIELD operator_idx6;
extern TEXT_FIELD operator_idx7;
extern TEXT_FIELD operator_idx8;

extern TEXT_FIELD operator_name0;
extern TEXT_FIELD operator_name1;
extern TEXT_FIELD operator_name2;
extern TEXT_FIELD operator_name3;
extern TEXT_FIELD operator_name4;
extern TEXT_FIELD operator_name5;
extern TEXT_FIELD operator_name6;
extern TEXT_FIELD operator_name7;
extern TEXT_FIELD operator_name8;

//extern SCROLL_BAR operator_scb;

//static VACTION operator_list_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
//static PRESULT operator_list_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION operator_list_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT operator_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION operator_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT operator_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

#define WIN_SH_IDX	  WSTL_COMMON_BACK_2
#define WIN_HL_IDX	  WSTL_COMMON_BACK_2
#define WIN_SL_IDX	  WSTL_COMMON_BACK_2
#define WIN_GRY_IDX  WSTL_COMMON_BACK_2

#define CON_IDX	        WSTL_WIN_2

#define LST_SH_IDX		WSTL_NOSHOW_IDX//WSTL_BUTTON_01
#define LST_HL_IDX		WSTL_NOSHOW_IDX//WSTL_BUTTON_01
#define LST_SL_IDX		WSTL_NOSHOW_IDX//WSTL_BUTTON_01
#define LST_GRY_IDX		WSTL_NOSHOW_IDX//WSTL_BUTTON_01

#define CON_SH_IDX   WSTL_BUTTON_2
#define CON_HL_IDX   WSTL_BUTTON_1
#define CON_SL_IDX   WSTL_BUTTON_2
#define CON_GRY_IDX  WSTL_BUTTON_2

#define TXTI_SH_IDX   WSTL_TXT_4
#define TXTI_HL_IDX   WSTL_TXT_5//WSTL_TXT_5
#define TXTI_SL_IDX   WSTL_TXT_4
#define TXTI_GRY_IDX  WSTL_TXT_10

#define TXTN_SH_IDX   WSTL_TXT_4
#define TXTN_HL_IDX   WSTL_TXT_5
#define TXTN_SL_IDX   WSTL_TXT_4
#define TXTN_GRY_IDX  WSTL_TXT_10

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_1
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_1

#define LIST_BAR_MID_RECT_IDX  	    WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX  	WSTL_SCROLLBAR_2

#define W_L		0
#define W_T		90
#define W_W		720
#define W_H		400

#define W_CON_L		66
#define W_CON_T	102
#define W_CON_W	604
#define W_CON_H     	264

#define SCB_L (W_CON_L + 2)
#define SCB_T (W_CON_T )
#define SCB_W 12
#define SCB_H (W_CON_H - 4)


#define LST_L	(SCB_L + SCB_W)
#define LST_T	(W_CON_T + 10)
#define LST_W	(W_CON_W - SCB_W - 8)
#define LST_H	300
#define LST_GAP 4

#define ITEM_L	(LST_L + 0)
#define ITEM_T	(LST_T + 2)
#define ITEM_W	(LST_W - 0)
#define ITEM_H	28
#define ITEM_GAP	2


#define ITEM_IDX_L  2
#define ITEM_IDX_W  200
#define ITEM_NAME_L  (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_NAME_W  360//180

#define LDEF_CON(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    NULL, NULL, /*operator_list_item_con_keymap,operator_list_item_con_callback,  \
    conobj, 1,1)
    
#define LDEF_TXTIDX(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXTI_SH_IDX,TXTI_HL_IDX,TXTI_SL_IDX,TXTI_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,resID,str)

#define LDEF_TXTNAME(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 10,0,resID,str)

#define LDEF_LIST_ITEM(root,varCon,varTxtidx,varTxtName,ID,l,t,w,h,idxstr,namestr)	\
	LDEF_CON(&root,varCon,NULL,ID,ID,ID,ID,ID,l,t,w,h,&varTxtidx,1)	\
	LDEF_TXTIDX(&varCon,varTxtidx,&varTxtName ,0,0,0,0,0,l + ITEM_IDX_L, t,ITEM_IDX_W,h,0,idxstr)	\
	LDEF_TXTNAME(&varCon,varTxtName,NULL,1,1,1,1,1,l + ITEM_NAME_L, t,ITEM_NAME_W,h,0,namestr)

#define LDEF_LISTBAR(root,varScb,page,l,t,w,h)	\
	DEF_SCROLLBAR(varScb, &root, NULL, C_ATTR_ACTIVE, 0, \
		0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
		NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
		0, 10, w, h - 20, 100, 1)

#define LDEF_OL(root,varOl,nxtObj,l,t,w,h,style,dep,count,flds,sb,mark,selary)	\
  DEF_OBJECTLIST(varOl,&root,nxtObj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_HL_IDX,LST_SL_IDX,LST_GRY_IDX,   \
    operator_list_keymap,operator_list_callback,    \
    flds,sb,mark,style,dep,count,selary)

#define LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    operator_keymap,operator_callback,  \
    nxtObj, focusID,0)

LDEF_TXTIDX(&g_win_operator_info,operator_table_title_id,&operator_table_title_ppname,0,0,0,0,0,\
    ITEM_L+ITEM_IDX_L, ITEM_T,ITEM_IDX_W,ITEM_H,RS_OPERATOR_ID,NULL)

LDEF_TXTIDX(&g_win_operator_info,operator_table_title_ppname,&operator_olist,0,0,0,0,0,\
    ITEM_L+ITEM_NAME_L, ITEM_T,ITEM_NAME_W,ITEM_H,RS_OPERATOR_NAME,NULL)

LDEF_LIST_ITEM(operator_olist,operator_item0,operator_idx0,operator_name0,  1,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*1,ITEM_W, ITEM_H, display_strs[0], display_strs[9])

LDEF_LIST_ITEM(operator_olist,operator_item1,operator_idx1,operator_name1,2,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*2,ITEM_W, ITEM_H, display_strs[1], display_strs[10])

LDEF_LIST_ITEM(operator_olist,operator_item2,operator_idx2,operator_name2,3,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*3,ITEM_W, ITEM_H, display_strs[2], display_strs[11])	

LDEF_LIST_ITEM(operator_olist,operator_item3,operator_idx3,operator_name3,4,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*4,ITEM_W, ITEM_H, display_strs[3], display_strs[12])

LDEF_LIST_ITEM(operator_olist,operator_item4,operator_idx4,operator_name4,5,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*5,ITEM_W, ITEM_H, display_strs[4], display_strs[13])

LDEF_LIST_ITEM(operator_olist,operator_item5,operator_idx5,operator_name5,6,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*6,ITEM_W, ITEM_H, display_strs[5], display_strs[14])

LDEF_LIST_ITEM(operator_olist,operator_item6,operator_idx6,operator_name6,7,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*7,ITEM_W, ITEM_H, display_strs[6], display_strs[15])

LDEF_LIST_ITEM(operator_olist,operator_item7,operator_idx7,operator_name7,8,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*8,ITEM_W, ITEM_H, display_strs[7], display_strs[16])

LDEF_LIST_ITEM(operator_olist,operator_item8,operator_idx8,operator_name8,9,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*9,ITEM_W, ITEM_H, display_strs[8], display_strs[17])

//LDEF_LISTBAR(operator_olist,operator_scb,9,SCB_L,SCB_T, SCB_W, SCB_H)

POBJECT_HEAD operator_items[] =
{
	(POBJECT_HEAD)&operator_item0,
	(POBJECT_HEAD)&operator_item1,
	(POBJECT_HEAD)&operator_item2,
	(POBJECT_HEAD)&operator_item3,
	(POBJECT_HEAD)&operator_item4,
	(POBJECT_HEAD)&operator_item5,
	(POBJECT_HEAD)&operator_item6,
	(POBJECT_HEAD)&operator_item7,
	(POBJECT_HEAD)&operator_item8,
};

#define LIST_STYLE (LIST_VER | LIST_NO_SLECT     | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)

LDEF_OL(g_win_operator_info,operator_olist,NULL, LST_L,LST_T,LST_W,LST_H,LIST_STYLE, 4, 4,operator_items,NULL,NULL,NULL)

DEF_CONTAINER (win_operator_info_con, NULL, &operator_table_title_id, C_ATTR_ACTIVE, 0, \
                0, 0, 0, 0, 0, W_CON_L, W_CON_T, W_CON_W, W_CON_H, CON_IDX, CON_IDX, CON_IDX, CON_IDX,   \
                NULL, NULL, NULL, 1, 0 )
                
LDEF_WIN(g_win_operator_info,&win_operator_info_con, W_L,W_T,W_W, W_H, 1)
*/
#define W_L		0
#define W_T		90
#define W_W		720
#define W_H		400

#define W_CON_L		66
#define W_CON_T	102
#define W_CON_W	604
#define W_CON_H     	264

#define SCB_L (W_CON_L + 2)
#define SCB_T (W_CON_T )
#define SCB_W 12
#define SCB_H (W_CON_H - 4)


#define LST_L	(SCB_L + SCB_W)
#define LST_T	(W_CON_T + 10)
#define LST_W	(W_CON_W - SCB_W - 8)
#define LST_H	300
#define LST_GAP 4

#define ITEM_L	(LST_L + 0)
#define ITEM_T	(LST_T + 2)
#define ITEM_W	(LST_W - 0)
#define ITEM_H	28
#define ITEM_GAP	2

#define ITEM_IDX_L  2
#define ITEM_IDX_W  200
#define ITEM_NAME_L  (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_NAME_W  360//180
/*******************************************************************************
*	Local vriable & function declare
*******************************************************************************/
#define OPERATOR_POPUP_ITEMS 5

#define OPINFO_SERVICE_ENTITLE			1
#define OPINFO_WALLET_INFO			2
#define OPINFO_IPPV_INFO				3
#define OPINFO_CHILD_PARENT_CARD		4
#define OPINFO_DETITLE					5

static void win_operator_set_display(void);
static UINT8 exit_ind = 0;
UINT16 CAS_operator_ID[4];
UINT8 operator_num = 0;
UINT16 current_operator_ID;
T_CA_OperatorInfo CAS_operator_info[4];
UINT16 operator_popup[OPERATOR_POPUP_ITEMS]=
{
	RS_SERVICE_ENTITLES,
	RS_SLOT_INFO,
	RS_IPPV_PROGRAM,
	RS_CHILD_PARENT_CARD,
	RS_DETITLE
};
/*******************************************************************************
*	key mapping and event callback definition
*******************************************************************************/

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
/*
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
*/
void Operator_enter_key(POBJECT_HEAD pObj, UINT8 id) 
{
	UINT8 sel = 0;
	OSD_RECT rect;
	COM_POP_LIST_PARAM_T param;

	rect.uLeft = ITEM_L+ITEM_NAME_L;
	rect.uWidth = ITEM_NAME_W;
	rect.uHeight = 150;
	rect.uTop = ITEM_T + (ITEM_H + ITEM_GAP)*(1+id);
	param.selecttype = POP_LIST_SINGLESELECT;
	param.cur = 0;

	current_operator_ID = CAS_operator_ID[id];
	sel = win_com_open_sub_list(POP_LIST_MENU_CA_OPERATOR_INFO, &rect, &param);
	if (1 == exit_ind) //Because if rcv "exit/Menu" key, it would return still return a valid sel value
	{
		exit_ind = 0;
		return;
	}
	else
    {;}//operator_info_enter_detail(sel+1);
}



#define VACT_OPERATOR_POPUP (VACT_PASS + 1)
static VACTION operator_list_keymap(POBJECT_HEAD pObj, UINT32 key)
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
static PRESULT operator_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	UINT16 uni_name[30];
	UINT8* new_name;
	OBJLIST* ol;
	UINT16 curitem;
    

	ol = &operator_olist;
	curitem = OSD_GetObjListCurPoint(ol);

	switch(event)
	{
		case EVN_PRE_DRAW:
			win_operator_set_display();
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1>>16);
			if(unact == VACT_OPERATOR_POPUP)
			{
				current_operator_ID = CAS_operator_ID[curitem];
				OSD_ObjOpen((POBJECT_HEAD)&g_win_operator_message,0);//yuanlin 2013.02.18
				menu_stack_push((POBJECT_HEAD)&g_win_operator_message);//yuanlin 2013.02.18
				//OSD_ObjOpen((POBJECT_HEAD)&g_win_operator_info_detail,0);
              // menu_stack_push((POBJECT_HEAD)&g_win_operator_info_detail);
			}
			break;
	}

	return ret;
}

static VACTION operator_keymap(POBJECT_HEAD pObj, UINT32 key)
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

// 0 for error,  1 for success
INT32 operator_got_info()
{
	UINT32 ret,i;
	//T_CA_OperatorInfo operinfo;
	
	operator_num = 1;
#if 1
	MEMSET(&CAS_operator_info, 0, sizeof(T_CA_OperatorInfo));
	ret = GYCA_GetOperatorInfo(&CAS_operator_info);
	if (GYCA_OK != ret)
	{
		cas_popup_msg(NULL, NULL, RS_CAERR_NOINFO);
		operator_num = 0;
		return 0;
	}
	operator_num = 1;
	//MEMCPY(CAS_operator_info .m_szTVSName, operinfo.m_szSPName, STRLEN(operinfo.m_szSPName));
	
#else //debug only
	operator_num = RAND(DVTCA_MAXNUMBER_TVSID);
	for (i=0; i<operator_num; i++)
	{
		CAS_operator_info[i].m_wTVSID = i*3;
		sprintf(CAS_operator_info[i].m_szTVSName, "扬智电子开发数码视讯%d", i );
	}
	
#endif
	
	return 1;
}


extern struct help_item_resource win_ca_help[];
static PRESULT operator_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT32 cnt = 0, retval;
		/*struct help_item_resource win_oi_help[] =
	{
		{1,IM_HELP_ICON_TB,RS_HELP_SELECT},
    		{0,RS_HELP_ENTER,RS_HELP_ENTER_INTO},
    		{0,RS_MENU,RS_HELP_BACK},
    		{0,RS_HELP_EXIT,RS_FULL_PLAY},
	};*/

	switch(event)
	{
	case EVN_PRE_OPEN:
		//wincom_open_title(pObj,RS_OPERATOR_INFORMATION,0);
		operator_num = 0;
		MEMSET(CAS_operator_ID,0, sizeof(CAS_operator_ID));
		MEMSET(CAS_operator_info, 0, sizeof(CAS_operator_info));
        	retval = operator_got_info();
		if (0 == retval)  
		{
			ret = PROC_LEAVE;
			break;
		}
		//wincom_open_title_ext(RS_OPERATOR_INFORMATION, IM_TITLE_ICON_SYSTEM);
		//wincom_open_help(win_oi_help, 4);
		win_init_pstring(27);
		OSD_SetObjListCount(&operator_olist,operator_num);
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


static void  win_operator_set_display(void)
{
	CONTAINER* con;
	TEXT_FIELD* text;
	OBJLIST* ol;	
	UINT8 str[6];
	UINT16 operator_name[30+1];
	UINT16 top,cnt,page,index;
	INT32 j = -1;
	UINT32 infoid;
	
	ol = &operator_olist;

	page = OSD_GetObjListPage(ol); 
	//here total 4 provider are supported
	if (operator_num != 0)
	{
		MEMSET(operator_name, 0, sizeof(operator_name));
		for(index=0;index<page;index++)
	    	{
			con = (CONTAINER*)operator_olist_ListField[index];
            /*num*/
			
           text = (TEXT_FIELD*)OSD_GetContainerNextObj(con);   //yuanlin
		       if (index < operator_num)
		        {
			      sprintf(str, "%02d", index + 1);
			      OSD_SetTextFieldContent(text, STRING_ANSI, (UINT32)str);
		        }
		       else
			      OSD_SetTextFieldContent(text, STRING_ANSI, (UINT32)"");

			
		        /* ID */
		      text = (TEXT_FIELD*)OSD_GetObjpNext(text);

				if (index < operator_num)
				{
					
				OSD_SetTextFieldContent(text, STRING_NUMBER, (UINT32)CAS_operator_ID[index]);
				}
				else
					OSD_SetTextFieldContent(text, STRING_ANSI, (UINT32)"");

		 
		        /* Name**/
		        text = (TEXT_FIELD*)OSD_GetObjpNext(text);

				if (index < operator_num)
				{
					convert_gb2312_to_unicode(CAS_operator_info[index].m_szSPName, 30,operator_name, 30);
					OSD_SetTextFieldContent(text, STRING_UNICODE, (UINT32)&operator_name);
				}
				else
					OSD_SetTextFieldContent(text, STRING_ANSI, (UINT32)"");

	        
		}
	}
	
}


