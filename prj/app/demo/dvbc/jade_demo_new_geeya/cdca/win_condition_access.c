#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
//#include <api/libosd/osd_lib.h>

#include "win_cas_com.h"

/*******************************************************************************
 *	Objects & callback functions declaration
 *******************************************************************************/
extern CONTAINER g_win_condition_access;
extern CONTAINER con_ca_sub_menu;
extern CONTAINER g_win_operator_message;

extern TEXT_FIELD txt_sm_ca_title; /*in actually it is txt_item0*/
extern TEXT_FIELD txt_ca_item1;
extern TEXT_FIELD txt_ca_item2;
extern TEXT_FIELD txt_ca_item3;
extern TEXT_FIELD txt_ca_item4;
extern TEXT_FIELD txt_ca_item5;
extern TEXT_FIELD txt_ca_item6;

/*******************************************************************************
callback & keymap functions declaration
 *******************************************************************************/

//static VACTION ca_keymap(POBJECT_HEAD pObj, UINT32 key);
static VACTION ca_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT ca_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION ca_item_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT ca_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT ca_message_proc(UINT32 msg_type, UINT32 msg_code);

static PRESULT VolumeProc(VACTION act);
/*******************************************************************************
 *	Objects & callback functions declaration
 *******************************************************************************/
/*rectangle parameters & style define for g_win_condiction_access*/
#define WIN_BG_IDX   WSTL_SEARCH_BG_CON

#define WIN_BG_L     (TV_OFFSET_L+0)
#define WIN_BG_T     (TV_OFFSET_T+90)
#define WIN_BG_W     720
#define WIN_BG_H     400

/**********rectangle parameters & style define for con_ca_sub_menu**********/
#define CON_SUB_MENU_IDX   WSTL_WIN_1

#define CON_SUB_MENU_L    (TV_OFFSET_L+356)
#define CON_SUB_MENU_T    (WIN_BG_T+12)
#define CON_SUB_MENU_W    314
#define CON_SUB_MENU_H    364

/**********rectangle parameters & style define for items********/
#define TXT_TITLE_IDX     WSTL_TXT_4

#define ITEM_L       (TV_OFFSET_L+360)
#define ITEM_T       (CON_SUB_MENU_T+4)
#define ITEM_W       304
#define ITEM_H       30
#define TXT_L_OF     4
#define TXT_T_OF     30

/*********rectangle parameters & style define for preview********/
#define PREVIEW_IDX  WSTL_SUBMENU_PREVIEW

#define PREVIEW_L  (TV_OFFSET_L+64)
#define PREVIEW_T  (TV_OFFSET_T+96)
#define PREVIEW_W  290
#define PREVIEW_H  234

#define PREVIEW_INNER_L  (PREVIEW_L+24)
#define PREVIEW_INNER_T  (PREVIEW_T+25)
#define PREVIEW_INNER_W  (PREVIEW_W-24-27)
#define PREVIEW_INNER_H  (PREVIEW_H-25-29-30)


#define LDEF_WIN(varCon,root,nxtObj,l,t,w,h,nxtInObj,focusID) \
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    1,0,0,0,0,l,t,w,h, WIN_BG_IDX,WIN_BG_IDX,WIN_BG_IDX,WIN_BG_IDX,\
    ca_keymap, ca_callback,  \
    nxtInObj, 1,1)

#define LDEF_CON(root, varCon,nxtObj,l,t,w,h,style,nxtInObj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, l,t,w,h, style,style,style,style,   \
    ca_item_keymap,ca_item_callback,  \
    nxtInObj, 1,0)

#define LDEF_TXT(root,varTxt,nxtObj,ID,IDu,IDd,l,t,w,h,styleNormal,styleHi,styleSel,styleGary,resID,pString)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,0,0,IDu,IDd, l,t,w,h, styleNormal,styleHi,styleSel,styleGary,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,resID,pString)


#define LDEF_SUB_MENU_ITEM(root,varTxt,nxtObj,ID,IDu,IDd,l,t,w,h,styleNormal,styleHi,styleSel,styleGary,strID,pString)	\
    LDEF_TXT(root,varTxt,nxtObj,ID,IDu,IDd,l,t,w,h,styleNormal,styleHi,styleSel,styleGary,strID,pString)


/*******************************************************************************
*	Objects define
*******************************************************************************/
LDEF_WIN(g_win_condition_access,NULL,NULL,WIN_BG_L,WIN_BG_T,WIN_BG_W,WIN_BG_H,&con_ca_sub_menu,1)

LDEF_CON(&g_win_condition_access,con_ca_sub_menu,NULL,CON_SUB_MENU_L,CON_SUB_MENU_T,CON_SUB_MENU_W,CON_SUB_MENU_H,\
         WSTL_WIN_1,&txt_sm_ca_title,1)


         
LDEF_SUB_MENU_ITEM(&con_ca_sub_menu,txt_sm_ca_title,&txt_ca_item1,0,0,0,ITEM_L,(ITEM_T+0*ITEM_H),\
                    ITEM_W,ITEM_H,WSTL_TXT_4,WSTL_TXT_4,WSTL_TXT_4,WSTL_TXT_4,RS_CONDITION_ACCESS,NULL)

LDEF_SUB_MENU_ITEM(&con_ca_sub_menu,txt_ca_item1,&txt_ca_item2,1,6,2,ITEM_L,(ITEM_T+1.5*ITEM_H),ITEM_W,ITEM_H,\
                    WSTL_LCL_ITEM_NORMAL,WSTL_BUTTON_1,WSTL_LCL_ITEM_NORMAL,WSTL_LCL_ITEM_NORMAL,RS_MATURITY_RATING,NULL)

LDEF_SUB_MENU_ITEM(&con_ca_sub_menu,txt_ca_item2,&txt_ca_item3,2,1,3,ITEM_L,(ITEM_T+3*ITEM_H),ITEM_W,ITEM_H,\
                    WSTL_LCL_ITEM_NORMAL,WSTL_BUTTON_1,WSTL_LCL_ITEM_NORMAL,WSTL_LCL_ITEM_NORMAL,RS_MODIFY_PIN,NULL)

LDEF_SUB_MENU_ITEM(&con_ca_sub_menu,txt_ca_item3,&txt_ca_item4,3,2,4,ITEM_L,(ITEM_T+4.5*ITEM_H),ITEM_W,ITEM_H,\
                    WSTL_LCL_ITEM_NORMAL,WSTL_BUTTON_1,WSTL_LCL_ITEM_NORMAL,WSTL_LCL_ITEM_NORMAL,RS_WORKTIME_SETTING,NULL)

LDEF_SUB_MENU_ITEM(&con_ca_sub_menu,txt_ca_item4,&txt_ca_item5,4,3,5,ITEM_L,(ITEM_T+6*ITEM_H),ITEM_W,ITEM_H,\
                    WSTL_LCL_ITEM_NORMAL,WSTL_BUTTON_1,WSTL_LCL_ITEM_NORMAL,WSTL_LCL_ITEM_NORMAL,RS_PAIRING_SETTING,NULL)

LDEF_SUB_MENU_ITEM(&con_ca_sub_menu,txt_ca_item5,&txt_ca_item6,5,4,6,ITEM_L,(ITEM_T+7.5*ITEM_H),ITEM_W,ITEM_H,\
                    WSTL_LCL_ITEM_NORMAL,WSTL_BUTTON_1,WSTL_LCL_ITEM_NORMAL,WSTL_LCL_ITEM_NORMAL,RS_OPERATOR_INFORMATION,NULL)

LDEF_SUB_MENU_ITEM(&con_ca_sub_menu,txt_ca_item6,NULL,6,5,1,ITEM_L,(ITEM_T+9*ITEM_H),ITEM_W,ITEM_H,\
                    WSTL_LCL_ITEM_NORMAL,WSTL_BUTTON_1,WSTL_LCL_ITEM_NORMAL,WSTL_LCL_ITEM_NORMAL,RS_SMC_UPGRADE_INFO,NULL)


/*******************************************************************************
 *	local variable define
 *******************************************************************************/

POBJECT_HEAD windows_ca_list[6] =
{
    (POBJECT_HEAD)&g_win_maturity_rating,
    (POBJECT_HEAD)&g_win_pin_pwd,
    (POBJECT_HEAD)&g_win_work_time,
    (POBJECT_HEAD)&g_win_STBIDlst,
    (POBJECT_HEAD)&g_win_operator_info,
    //(POBJECT_HEAD)&g_win_operator_message,
    (POBJECT_HEAD)&g_win_smc_upgrade_info,
};

#define VACT_VOLUME_DECREASE	(VACT_PASS + 1)
#define VACT_VOLUME_INCREASE	(VACT_PASS + 2)
#define VACT_ITEM_SELECT	    (VACT_PASS + 3)
/*******************************************************************************
 *	helper functions define
 *******************************************************************************/
static VACTION ca_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	INT8 shift = 1;

	switch (key)
	{
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		case V_KEY_EXIT:
			BackToFullScrPlay();
			break;
		case V_KEY_LEFT:
			shift =  - 1;
		case V_KEY_RIGHT:
			PreviewVolProc(shift);
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}

extern struct help_item_resource win_ca_help[];
static PRESULT ca_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	OSD_RECT rc_con;
	OSD_RECT rc_preview;
	struct help_item_resource win_cas_help[] =
	{
	    {1,IM_HELP_ICON_TB,RS_HELP_SELECT},
	    {0,RS_HELP_ENTER,RS_HELP_ENTER},
	    {0,RS_MENU,RS_HELP_BACK},
	    {0,RS_HELP_EXIT,RS_HELP_EXIT},          
	};

	switch (event)
	{
		case EVN_PRE_OPEN:
			wincom_open_title_ext(RS_CONDITION_ACCESS, IM_TITLE_ICON_SYSTEM);
			wincom_open_help(win_cas_help, 4);
			break;
		case EVN_POST_OPEN:
			rc_con.uLeft = PREVIEW_L;
			rc_con.uTop = PREVIEW_T;
			rc_con.uWidth = PREVIEW_W;
			rc_con.uHeight = PREVIEW_H;

			rc_preview.uLeft = PREVIEW_INNER_L;
			rc_preview.uTop = PREVIEW_INNER_T;
			rc_preview.uWidth = PREVIEW_INNER_W;
			rc_preview.uHeight = PREVIEW_INNER_H;

			wincom_open_preview_ext(rc_con, rc_preview, WSTL_SUBMENU_PREVIEW);
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			//wincom_close_preview();
			break;
		case EVN_MSG_GOT:
			ret = ca_message_proc(param1, param2);
			break;
		case EVN_UNKNOWN_ACTION:
			break;
	}

	return ret;
}

static VACTION ca_item_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act;

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


static PRESULT ca_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
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
					if (NULL != windows_ca_list[id - 1])
					{
						if (OSD_ObjOpen(windows_ca_list[id - 1], 0xFFFFFFFF) != PROC_LEAVE)
							menu_stack_push(windows_ca_list[id - 1]);
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


static PRESULT ca_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;

	switch (msg_type)
	{
		case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
			wincom_preview_proc();
			break;
		default:
			break;
	}

	return ret;
}
