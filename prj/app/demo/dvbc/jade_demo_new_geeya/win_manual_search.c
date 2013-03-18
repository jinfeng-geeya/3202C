#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/nim/nim.h>
//#include <api/libosd/osd_lib.h>
#include <hld/decv/decv.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"
#include "menus_root.h"
#include "copper_common/system_data.h"
#include "copper_common/menu_api.h"
//#include "win_com_menu_define.h"
#include "win_com.h"
#include "win_signal.h"

//include the header from xform 
#include "win_manual_search_vega.h"

/*******************************************************************************
 *	Objects definition
 *******************************************************************************/
extern CONTAINER g_win_manual_search;
extern CONTAINER manual_search_con_items;

extern CONTAINER main_search_con_item1;
extern CONTAINER main_search_con_item2;
extern CONTAINER main_search_con_item3;
extern CONTAINER main_search_con_item4;
extern CONTAINER main_search_con_item5;


extern TEXT_FIELD txt_ms_item_name1;
extern MULTISEL txt_ms_item_value1;

extern TEXT_FIELD txt_ms_item_name2;
extern EDIT_FIELD txt_ms_item_value2;

extern TEXT_FIELD txt_ms_item_name3;
extern EDIT_FIELD txt_ms_item_value3;

extern TEXT_FIELD txt_ms_item_name4;
extern MULTISEL txt_ms_item_value4;

extern TEXT_FIELD txt_ms_item_name5;
extern MULTISEL txt_ms_item_value5;

/*******************************************************************************
 *  keymap & callback functions define
 *******************************************************************************/
/*static VACTION win_manual_search_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT win_main_search_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION main_search_item_edit_keymap(POBJECT_HEAD pObj, UINT32 key);

static VACTION main_search_item_sel_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT main_search_item_sel_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);*/


UINT16 ms_model_type_ids[] =
    {
        RS_CONSTELLATION_16,
        RS_CONSTELLATION_32,
        RS_CONSTELLATION_64,
        RS_CONSTELLATION_128,
        RS_CONSTELLATION_256,

    };

typedef enum _SEARCH_MODE
{
	MANUAL_SEARCH = 0,
	AUTO_SEARCH ,
	FULL_BAND_SEARCH
} SEARCH_MODE;

UINT16 NIT_type_ids[] =
{
	RS_COMMON_NO,
	RS_COMMON_YES,
};
#define DVBC_QUICKSCAN_EDIT_LEN  0x06 // add .xx
char win_mainSearch_pat_freq[] = "f32"; //integer width is 5,  fraction width is 2
char win_mainSearch_pat_sym[] = "s5";

//this values is restore the setting of parameters
 UINT32 i_freq = 0x74CC;
 UINT32 i_symbol = 6875;
 UINT8 i_mode = 6; //64QAM	
 UINT8 I_nit_mode = 0; // 0:not nit   1: nit
 UINT8 i_scan_mode = 0; //0:auto,1:Manual,2:full_band
/*******************************************************************************
*	Objects & marco declaration
*******************************************************************************/
/*rectangle parameters & style define for g_win_manual_search*/
/*#define     WIN_BG_IDX   WSTL_SEARCH_BG_CON

#define     WIN_BG_L     (TV_OFFSET_L+0)
#define     WIN_BG_T     (TV_OFFSET_T+90)
#define     WIN_BG_W     720
#define     WIN_BG_H     400*/

/*rectangle parameters & style define for manual_search_con_items*/
/*#define     manual_search_con_items_IDX   WSTL_WIN_2

#define	    W_CON_L      (TV_OFFSET_L+66)
#define	    W_CON_T      (TV_OFFSET_T+102)
#define	    W_CON_W		 604
#define	    W_CON_H      220*/

/*rectangle parameters & style define for main_search_con_item1*/
/*#define     CON_SH_IDX   WSTL_LCL_ITEM_NORMAL
#define     CON_HL_IDX   WSTL_BUTTON_1
#define     CON_SL_IDX   WSTL_LCL_ITEM_NORMAL
#define     CON_GRY_IDX  WSTL_LCL_ITEM_NORMAL

#define     CON_ITEM_L   (W_CON_L+10)
#define     CON_ITEM_T   W_CON_T
#define     CON_ITEM_W   ((W_CON_W-2*15)) //15  is the edge between manual_search_con_items & con_itemxxx
#define     CON_ITEM_H   30*/

/*rectangle parameters & style define for txt_item_nameXXX*/
/*#define     TXT_NAME_L   (CON_ITEM_L+10)
#define     TXT_NAME_W   (CON_ITEM_W*4/7)*/

/*rectangle parameters & style define for txt_item_valueXXX*/
/*#define     TXT_VALUE_L   (TXT_NAME_L+TXT_NAME_W+50)
#define     TXT_VALUE_W   (CON_ITEM_W*2/7-2*20+30)*/


/*#define LDEF_WIN(varCon,root,nxtObj,l,t,w,h,nxtInObj,focusID) \
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    1,0,0,0,0,l,t,w,h, WIN_BG_IDX,WIN_BG_IDX,WIN_BG_IDX,WIN_BG_IDX,\
    win_manual_search_keymap, win_main_search_callback,  \
    nxtInObj, 1,0)

#define LDEF_CON(varCon,root,nxtObj,nxtInObj,l,t,w,h)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, l,t,w,h, manual_search_con_items_IDX,manual_search_con_items_IDX,manual_search_con_items_IDX,manual_search_con_items_IDX,   \
    con_keymap,con_callback,  \
    nxtInObj, 1,0)

#define LDEF_CON_ITEM(varCon,root,nxtObj,nxtInObj,id,idUp,idDown,l,t,w,h,styleNormal,styleHi,styleSel,styleGary)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    id,id,id,idUp,idDown,l,t,w,h,styleNormal,styleHi,styleSel,styleGary,  \
    NULL,NULL,  \
    nxtInObj, id,1)
    
#define LDEF_TXTNAME(root,varTxt,nxtObj,id,l,t,w,h,resID,pstring)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    id,id,id,id,id, l,t,w,h,WSTL_TXT_4,WSTL_TXT_3,WSTL_TXT_4,WSTL_TXT_4,\
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,resID,pstring)

#define LDEF_BMP(root,varBmp,nxtObj,id,l,t,w,h,icon,IDX)		\
    DEF_BITMAP(varBmp,root,nxtObj,C_ATTR_ACTIVE,0, \
    id,id,id,id,id, l,t,w,h, IDX,IDX,IDX,IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER,0,0,icon)

#define LDEF_TXTVALUE(root,varTxt,nxtObj,id,l,t,w,h,resID,pstring)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    id,id,id,id,id, l,t,w,h,WSTL_TXT_11,WSTL_BUTTON_3,WSTL_TXT_11,WSTL_TXT_11,\
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,resID,pstring)


#define LDEF_EDIT(root, varNum, nxtObj, ID,l, t, w, h,style,cursormode,pat,pre,sub,str)	\
    DEF_EDITFIELD(varNum,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, WSTL_TXT_11,WSTL_BUTTON_3,WSTL_TXT_11,WSTL_TXT_11,   \
    main_search_item_edit_keymap,NULL, \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,pat,10,cursormode,pre,sub,str)

#define LDEF_MSEL(root, varNum, nxtObj, ID, l, t, w, h,style,cur,cnt,ptabl)	\
    DEF_MULTISEL(varNum,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, WSTL_TXT_11,WSTL_BUTTON_5,WSTL_TXT_11,WSTL_TXT_11,   \
    main_search_item_sel_keymap,main_search_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)
    
    
#define LEDF_ITEM_SEL(root,var,nxtObj,nxtTxtName,nxtTxtValue,id,idUp,idDown,l,t,w,h,styleNormal,styleHi,styleSel,styleGary,resNameID,pNameString,style,cur,cnt,ptbl)\
    LDEF_CON_ITEM(var,root,nxtObj,&nxtTxtName,id,idUp,idDown,l,t,w,h,styleNormal,styleHi,styleSel,styleGary)\
    LDEF_TXTNAME(&var,nxtTxtName,&nxtTxtValue,0,TXT_NAME_L,t,TXT_NAME_W,h,resNameID,pNameString)\
    LDEF_MSEL(&var,nxtTxtValue,NULL,id,TXT_VALUE_L,t,TXT_VALUE_W,h,style,cur,cnt,ptbl)


#define LEDF_ITEM_EDIT(root,var,nxtObj,nxtTxtName,nxtTxtValue,id,idUp,idDown,l,t,w,h,styleNormal,styleHi,styleSel,styleGary,resNameID,pNameString,pat,pSubFix,pstring)\
    LDEF_CON_ITEM(var,root,nxtObj,&nxtTxtName,id,idUp,idDown,l,t,w,h,styleNormal,styleHi,styleSel,styleGary)\
    LDEF_TXTNAME(&var,nxtTxtName,&nxtTxtValue,0,TXT_NAME_L,t,TXT_NAME_W,h,resNameID,pNameString)\
    LDEF_EDIT(&var,nxtTxtValue,NULL,id,TXT_VALUE_L,t,TXT_VALUE_W,h,NORMAL_EDIT_MODE,CURSOR_NORMAL,pat,NULL,pSubFix,pstring)*/


/*******************************************************************************
*	Objects define
*******************************************************************************/


/*LDEF_WIN(g_win_manual_search,NULL,NULL,WIN_BG_L,WIN_BG_T,WIN_BG_W,WIN_BG_H,&manual_search_con_items,1)

LDEF_CON(manual_search_con_items,&g_win_manual_search,NULL,&main_search_con_item1,W_CON_L,W_CON_T,W_CON_W,W_CON_H)

LEDF_ITEM_EDIT(&manual_search_con_items,main_search_con_item1,&main_search_con_item2,txt_ms_item_name1,txt_ms_item_value1,1,5,2,CON_ITEM_L,CON_ITEM_T+(0.2*CON_ITEM_H),\
        CON_ITEM_W,CON_ITEM_H,CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,RS_FREQUENCY,NULL,win_mainSearch_pat_freq,display_strs[30],display_strs[20])

LEDF_ITEM_EDIT(&manual_search_con_items,main_search_con_item2,&main_search_con_item3,txt_ms_item_name2,txt_ms_item_value2,2,1,3,CON_ITEM_L,CON_ITEM_T+(1.5*CON_ITEM_H),\
        CON_ITEM_W,CON_ITEM_H,CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,RS_SEARCH_COD_SYMBOL,NULL,win_mainSearch_pat_sym,display_strs[31],display_strs[21])

LEDF_ITEM_SEL(&manual_search_con_items,main_search_con_item3,&main_search_con_item4,txt_ms_item_name3,txt_ms_item_value3,3,2,4,CON_ITEM_L,CON_ITEM_T+(2.8*CON_ITEM_H),\
        CON_ITEM_W,CON_ITEM_H,CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,RS_SEARCH_COD_COMSEL,NULL,STRING_ID, 0, 5, ms_model_type_ids)

LEDF_ITEM_SEL(&manual_search_con_items,main_search_con_item4,&txt_ms_item_name5,txt_ms_item_name4,txt_ms_item_value4,4,3,5,CON_ITEM_L,CON_ITEM_T+(4.1*CON_ITEM_H),\
        CON_ITEM_W,CON_ITEM_H,CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,RS_INSTALLATION_NIT_SCAN,NULL,STRING_ID, 0, 2, NIT_type_ids)

DEF_TEXTFIELD(txt_ms_item_name5,&manual_search_con_items,NULL,C_ATTR_ACTIVE,0, \
    5,5,5,4,1,TXT_NAME_L,CON_ITEM_T+(5.4*CON_ITEM_H),CON_ITEM_W,CON_ITEM_H,WSTL_TXT_11,WSTL_BUTTON_3,WSTL_TXT_11,WSTL_TXT_11,\
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,RS_SEARCH_START,NULL)*/


/*	local variable define
*******************************************************************************/
struct help_item_resource win_main_search_help[] = 
{
    {1,IM_HELP_ICON_LR,RS_CHANGTYPE},
    {1,IM_HELP_ICON_TB,RS_SELECT_EDIT_ITEM},
    {0,RS_MENU,RS_HELP_BACK},
    {0,RS_HELP_EXIT,RS_HELP_EXIT},          
};


POBJECT_HEAD search_set_items[] =
{
    ( POBJECT_HEAD ) &main_search_con_item1,
    ( POBJECT_HEAD ) &main_search_con_item2,
    ( POBJECT_HEAD ) &main_search_con_item3,
    ( POBJECT_HEAD ) &main_search_con_item4,
    ( POBJECT_HEAD ) &main_search_con_item5,
};


static void dvbc_quickscan_set_def_val(void);
static void win_qs_update_param(BOOL save);
static PRESULT manual_search_message_proc(UINT32 msg_type, UINT32 msg_code);
static UINT8 qs_check_value();
static INT32 check_freq_value(UINT16 *str);
static INT32 check_symbol_value(UINT16 *str);
static void win_ms_set_search_param();
inline UINT32 get_freq_from_manual();
inline UINT32 get_symbol_from_manual();
inline UINT8 get_qam_from_manual();
static void search_set_item_state(UINT16 State,BOOL Flag);
static void search_set_change_state(BOOL);
#define START_ID	6
#define VACT_SET_DECREASE	(VACT_PASS + 1)
#define VACT_SET_INCREASE	(VACT_PASS + 2)

/*******************************************************************************
 *	helper functions define
 *******************************************************************************/

static VACTION win_manual_search_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		case V_KEY_EXIT:
			dm_set_onoff(1);
			if(get_last_searched_prog()!=-1)
			{
				api_play_channel(get_last_searched_prog(),TRUE,FALSE,FALSE);
				set_last_searched_prog(-1);
			}
			else
			{
				restore_scene();
			}

		//	win_signal_close();
            #if ((SUBTITLE_ON == 1)||(TTX_ON == 1)) 
            osal_task_sleep(500);
            #endif
			BackToFullScrPlay();
			system_state = SYS_STATE_NORMAL;
			break;
		default:
			break;
	}

	return act;
}

static PRESULT win_main_search_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	SYSTEM_DATA *pSysData = NULL;
	UINT8 Temp=0;
	pSysData = sys_data_get();
	UINT8 Mul_Sel;
	switch (event)
	{
		case EVN_PRE_OPEN:
				soc_printf("time 1=%d\n",osal_get_tick());
				Mul_Sel=OSD_GetMultiselSel(&txt_ms_item_value1);
				if(Mul_Sel == 0)
				{
					OSD_SetAttr(&main_search_con_item2, C_ATTR_INACTIVE);
					OSD_SetAttr(&main_search_con_item3, C_ATTR_INACTIVE);
					OSD_SetAttr(&main_search_con_item4, C_ATTR_INACTIVE);
					OSD_SetAttr(&main_search_con_item5, C_ATTR_INACTIVE);
					OSD_TrackObject((POBJECT_HEAD)& manual_search_con_items, C_UPDATE_ALL);
				}
				else if(Mul_Sel == 2 )
				{
					OSD_SetAttr(&main_search_con_item2, C_ATTR_INACTIVE);
					OSD_SetAttr(&main_search_con_item3, C_ATTR_ACTIVE);
					OSD_SetAttr(&main_search_con_item4, C_ATTR_ACTIVE);
					OSD_SetAttr(&main_search_con_item5, C_ATTR_ACTIVE);
					OSD_TrackObject((POBJECT_HEAD)& manual_search_con_items, C_UPDATE_ALL);
				}
				else
				{
					OSD_SetAttr(&main_search_con_item2, C_ATTR_ACTIVE);
					OSD_SetAttr(&main_search_con_item3, C_ATTR_ACTIVE);
					OSD_SetAttr(&main_search_con_item4, C_ATTR_ACTIVE);
					OSD_SetAttr(&main_search_con_item5, C_ATTR_ACTIVE);
					OSD_TrackObject((POBJECT_HEAD)& manual_search_con_items, C_UPDATE_ALL);
				}
		//	wincom_open_title_ext(RS_MANUAL_SEARCH, IM_TITLE_ICON_SYSTEM);
		//	wincom_open_help(win_main_search_help, 4);
		soc_printf("time 2=%d\n",osal_get_tick());
			dvbc_quickscan_set_def_val();
			record_scene();
		//	OSD_SetContainerFocus(&manual_search_con_items,1);
			Temp=OSD_GetContainerFocus(&manual_search_con_items);
			dm_set_onoff(0);
			soc_printf("time 5=%d\n",osal_get_tick());
			win_qs_update_param(1);
			soc_printf("time 6=%d\n",osal_get_tick());
			
			soc_printf("time 3=%d\n",osal_get_tick());
		//	search_set_change_state(FALSE);
			system_state = SYS_STATE_SEARCH_PROG;
			break;
		case EVN_POST_OPEN:
			soc_printf("time 4=%d\n",osal_get_tick());
		//	win_signal_open(pObj);
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;			
			break;
		case EVN_POST_CLOSE:
			dm_set_onoff(1);

			restore_scene();
		//	win_signal_close();
			system_state = SYS_STATE_NORMAL;
			break;
		case EVN_MSG_GOT:
			ret = manual_search_message_proc(param1, param2);
			break;

	}

	return ret;
}


static VACTION con_keymap(POBJECT_HEAD pObj, UINT32 key)
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
			break;
	}
	return act;
}


static PRESULT con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{

	PRESULT ret = PROC_PASS;
	VACTION act;
	POBJECT_HEAD submenu;
	UINT8 id = OSD_GetFocusID(pObj);


	switch (event)
	{
		case EVN_ITEM_POST_CHANGE:
			if (1 == qs_check_value())
				win_qs_update_param(1);
			break;
		case EVN_UNKNOWN_ACTION:
			act = (VACTION)(param1 >> 16);
			if (act == VACT_ENTER)
			{
				if (1 == qs_check_value())
				{
					win_qs_update_param(1);
						switch(i_scan_mode)
						{
							case 0:
								win_set_search_param(0, 0, 0, 0, 1);	
								break;
							case 1:
								win_set_search_param(1, i_freq, i_symbol, i_mode, I_nit_mode);
								break;
							case 2:	
								win_set_search_param(2,0,i_symbol,i_mode,0);
								break;
						}
		
						//SetSearchParamMode(1);
						submenu = (POBJECT_HEAD) &g_win_search;
						if (OSD_ObjOpen(submenu, 0xFFFFFFFF) != PROC_LEAVE)
							menu_stack_push(submenu);
				}
				ret = PROC_LOOP;
			}
			break;
		default:
			break;
	}
	return ret;
}


static VACTION main_search_item_edit_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION Action = VACT_PASS;

	switch (key)
	{
		case V_KEY_LEFT:
			Action = VACT_EDIT_LEFT;
			break;
		case V_KEY_RIGHT:
			Action = VACT_EDIT_RIGHT;
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
			Action = key - V_KEY_0 + VACT_NUM_0;
			break;
		default:
			break;
	}

	return Action;
}


static VACTION main_search_item_sel_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	switch (key)
	{
		case V_KEY_LEFT:
			act = VACT_DECREASE;
			break;
		case V_KEY_RIGHT:
			act = VACT_INCREASE;
			break;
		case V_KEY_ENTER:
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}

static PRESULT main_search_item_sel_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 bID;
	VACTION unact;
	UINT8 Mul_Sel;
	bID = OSD_GetObjID(pObj);

	switch (event)
	{
		case EVN_PRE_CHANGE:

			break;
		case EVN_POST_CHANGE:
			if ((bID == 1))
			{
				Mul_Sel=OSD_GetMultiselSel(&txt_ms_item_value1);
				if(Mul_Sel == 0)
				{
					OSD_SetAttr(&main_search_con_item2, C_ATTR_INACTIVE);
					OSD_SetAttr(&main_search_con_item3, C_ATTR_INACTIVE);
					OSD_SetAttr(&main_search_con_item4, C_ATTR_INACTIVE);
					OSD_SetAttr(&main_search_con_item5, C_ATTR_INACTIVE);
					OSD_TrackObject((POBJECT_HEAD)& manual_search_con_items, C_UPDATE_ALL);
				}
				else if(Mul_Sel == 2 )
				{
					OSD_SetAttr(&main_search_con_item2, C_ATTR_INACTIVE);
					OSD_SetAttr(&main_search_con_item3, C_ATTR_ACTIVE);
					OSD_SetAttr(&main_search_con_item4, C_ATTR_ACTIVE);
					OSD_SetAttr(&main_search_con_item5, C_ATTR_ACTIVE);
					OSD_TrackObject((POBJECT_HEAD)& manual_search_con_items, C_UPDATE_ALL);
				}
				else
				{
					OSD_SetAttr(&main_search_con_item2, C_ATTR_ACTIVE);
					OSD_SetAttr(&main_search_con_item3, C_ATTR_ACTIVE);
					OSD_SetAttr(&main_search_con_item4, C_ATTR_ACTIVE);
					OSD_SetAttr(&main_search_con_item5, C_ATTR_ACTIVE);
					OSD_TrackObject((POBJECT_HEAD)& manual_search_con_items, C_UPDATE_ALL);
				}
			}
			win_qs_update_param(1);
		//	search_set_change_state(TRUE);
			break;
		default:
			break;
	}

	return ret;
}



static void dvbc_quickscan_set_def_val(void)
{
	INT8 rule[20];
	UINT32 freq, symbol, constellation;
	P_NODE p_node;
	T_NODE t_node;
	UINT32 prog_num;
	UINT16 cur_channel;

	/* FREQ */
	wincom_i_to_mbs_with_dot(display_strs[20],
	                         i_freq, DVBC_QUICKSCAN_EDIT_LEN - 1/*without dot*/, 0x02/*xxx.xx*/);
	ComAscStr2Uni("MHz", display_strs[30]);
	/* SYMBOL */
	wincom_i_to_mbs(display_strs[21],
	                i_symbol, DVBC_QUICKSCAN_EDIT_LEN - 1);
	ComAscStr2Uni("KBaud", display_strs[31]);

	OSD_SetMultiselSel(&txt_ms_item_value4, i_mode - QAM16);
	OSD_SetMultiselSel(&txt_ms_item_value1, i_scan_mode);
}



// 0 not save, 1 save the parameter
static void win_qs_update_param(BOOL save)
{
	UINT8 dot_pos;
	UINT32 freq, symbol, constellation, nit_mode;
	UINT8 search_mode;
	//freq
	wincom_mbs_to_i_with_dot(display_strs[20], &freq, &dot_pos);
	// SYMBOL
	symbol = wincom_mbs_to_i(display_strs[21]);
	// scan mode
	constellation = QAM16 + OSD_GetMultiselSel(&txt_ms_item_value4);

	nit_mode = OSD_GetMultiselSel(&txt_ms_item_value5);
	search_mode = OSD_GetMultiselSel(&txt_ms_item_value1);
	//change channel
	#if 0 //we don't need set Ft device
    union ft_xpond xpond;
    struct nim_device *nim = dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    MEMSET(&xpond, 0, sizeof(xpond));
    xpond.c_info.type = FRONTEND_TYPE_C;
    xpond.c_info.frq = freq;
    xpond.c_info.sym = symbol;
    xpond.c_info.modulation = constellation;
	soc_printf("time 7=%d\n",osal_get_tick());
	frontend_set_nim(nim, NULL, &xpond, 1);
	soc_printf("time 8=%d\n",osal_get_tick());
	#endif
	if (1 == save)
	{
		i_freq = freq;
		i_symbol = symbol;
		i_mode = constellation;
		I_nit_mode = nit_mode;
		i_scan_mode = search_mode;
	}
}


static PRESULT manual_search_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;

	switch (msg_type)
	{
		case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
		//	win_signal_update();
			break;
		case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
			if (is_time_inited())
			{
		//		wincom_draw_title_time();
			}
            break;

		default:
			break;
	}

	return ret;

}



// 1 valid 0 invalid
static UINT8 qs_check_value()
{
	UINT32 ret1, ret2;
	UINT8 save;

	ret1 = check_freq_value(display_strs[20]);
	ret2 = check_symbol_value(display_strs[21]);
	if (ret1 != 0 || ret2 != 0)
	{
		win_popup_msg(NULL, NULL, RS_MSG_INVALID_INPUT_CONTINUE);
		return 0;
	}
	else
		return 1;
}



static INT32 check_freq_value(UINT16 *str)
{
	INT32 ret = 0;
	UINT32 val;
	UINT8 dot_pos;

	/* FREQ */
	wincom_mbs_to_i_with_dot(str, &val, &dot_pos);
	if (val < 4825 || val > 86200)
		ret =  - 1;

	return ret;
}

static INT32 check_symbol_value(UINT16 *str)
{
	INT32 ret = 0;
	UINT32 val;

	/* SYMBOL */
	val = wincom_mbs_to_i(str);
	if (val < 1000 || val > 7000)
		ret =  - 1;

	return ret;
}


inline UINT32 get_freq_from_manual()
{
	return i_freq;
}

inline UINT32 get_symbol_from_manual()
{
	return i_symbol;
}

inline UINT8 get_qam_from_manual()
{
	return i_mode;
}

static void search_set_change_state(BOOL Flag)
{
	if(i_scan_mode == 0)
	search_set_item_state(C_ATTR_INACTIVE,TRUE);
	else if(i_scan_mode = 1)
	{
			search_set_item_state(C_ATTR_ACTIVE,TRUE);	
	}
	else
	search_set_item_state(C_ATTR_INACTIVE,FALSE);
}

static void search_set_item_state(UINT16 State,BOOL Flag)
{
	UINT8 i;
	if(Flag)
		for(i= 1;i<5;i++)
		OSD_SetAttr(search_set_items[i], State);
	else
		OSD_SetAttr(search_set_items[2],State);//FULL_BAND SET 
		if(Flag)
		for(i = 0;i<5;i++)
		OSD_TrackObject(search_set_items[i], C_UPDATE_ALL);
}
