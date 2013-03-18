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
#include "win_com.h"
#include "menus_root.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

//include the header from xform 
#include "win_games_vega.h"


/*******************************************************************************
 *	Objects & callback functions declaration
 *******************************************************************************/
extern CONTAINER g_win_games;
extern CONTAINER con_game_sub_menu;

extern TEXT_FIELD txt_sm_game_title; /*in actually it is txt_item0*/
extern TEXT_FIELD txt_game_item1;
extern TEXT_FIELD txt_game_item2;
extern TEXT_FIELD txt_game_item3;


/*******************************************************************************
callback & keymap functions declaration
 *******************************************************************************/

/*//static VACTION game_keymap(POBJECT_HEAD pObj, UINT32 key);
static VACTION game_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT game_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION game_item_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT game_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);*/
static PRESULT game_message_proc(UINT32 msg_type, UINT32 msg_code);

static PRESULT VolumeProc(VACTION act);
/*******************************************************************************
 *	Objects & callback functions declaration
 *******************************************************************************/
/*rectangle parameters & style define for g_win_games*/
/*#define WIN_BG_IDX   WSTL_SEARCH_BG_CON

#define WIN_BG_L     (TV_OFFSET_L+0)
#define WIN_BG_T     (TV_OFFSET_T+90)
#define WIN_BG_W     720
#define WIN_BG_H     400*/

/**********rectangle parameters & style define for con_game_sub_menu**********/
/*#define CON_SUB_MENU_IDX   WSTL_WIN_1

#define CON_SUB_MENU_L    (TV_OFFSET_L+356)
#define CON_SUB_MENU_T    (WIN_BG_T+12)
#define CON_SUB_MENU_W    314
#define CON_SUB_MENU_H    364*/

/**********rectangle parameters & style define for items********/
/*#define TXT_TITLE_IDX     WSTL_TXT_4

#define ITEM_L       (TV_OFFSET_L+360)
#define ITEM_T       (CON_SUB_MENU_T+4)
#define ITEM_W       304
#define ITEM_H       30
//#define     TXT_L_OF     4
//#define     TXT_T_OF     30*/

/*********rectangle parameters & style define for preview********/
/*#define PREVIEW_IDX  WSTL_SUBMENU_PREVIEW*/

#define PREVIEW_L  (TV_OFFSET_L+64)
#define PREVIEW_T  (TV_OFFSET_T+96)
#define PREVIEW_W  290
#define PREVIEW_H  234

#define PREVIEW_INNER_L  (PREVIEW_L+24)
#define PREVIEW_INNER_T  (PREVIEW_T+25)
#define PREVIEW_INNER_W  (PREVIEW_W-24-27)
#define PREVIEW_INNER_H  (PREVIEW_H-25-29-30)


/*#define LDEF_WIN(varCon,root,nxtObj,l,t,w,h,nxtInObj,focusID) \
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    1,0,0,0,0,l,t,w,h, WIN_BG_IDX,WIN_BG_IDX,WIN_BG_IDX,WIN_BG_IDX,\
    game_keymap, game_callback,  \
    nxtInObj, 1,1)

#define LDEF_CON(root, varCon,nxtObj,l,t,w,h,style,nxtInObj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    1,0,0,0,0, l,t,w,h, style,style,style,style,   \
    game_item_keymap,game_item_callback,  \
    nxtInObj, 1,0)

#define LDEF_TXT(root,varTxt,nxtObj,ID,IDu,IDd,l,t,w,h,styleNormal,styleHi,styleSel,styleGary,resID,pString)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,0,0,IDu,IDd, l,t,w,h, styleNormal,styleHi,styleSel,styleGary,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,resID,pString)


#define LDEF_SUB_MENU_ITEM(root,varTxt,nxtObj,ID,IDu,IDd,l,t,w,h,styleNormal,styleHi,styleSel,styleGary,strID,pString)	\
    LDEF_TXT(root,varTxt,nxtObj,ID,IDu,IDd,l,t,w,h,styleNormal,styleHi,styleSel,styleGary,strID,pString)*/


/*******************************************************************************
 *	Objects define
 *******************************************************************************/
/*LDEF_WIN(g_win_games, NULL, NULL, WIN_BG_L, WIN_BG_T, WIN_BG_W, WIN_BG_H, &con_game_sub_menu, 1)

LDEF_CON(&g_win_games, con_game_sub_menu, NULL, CON_SUB_MENU_L, CON_SUB_MENU_T, CON_SUB_MENU_W, CON_SUB_MENU_H,  \
	WSTL_WIN_1, &txt_sm_game_title, 1)



LDEF_SUB_MENU_ITEM(&con_game_sub_menu, txt_sm_game_title, &txt_game_item1, 0, 0, 0, ITEM_L, (ITEM_T + 0 * ITEM_H),  \
	ITEM_W, ITEM_H, WSTL_TXT_4, WSTL_TXT_4, WSTL_TXT_4, WSTL_TXT_4, RS_GAMES_WORLD, NULL)

LDEF_SUB_MENU_ITEM(&con_game_sub_menu, txt_game_item1, &txt_game_item2, 1, 3, 2, ITEM_L, (ITEM_T + 2 * ITEM_H), ITEM_W, ITEM_H,  \
	WSTL_LCL_ITEM_NORMAL, WSTL_BUTTON_1, WSTL_LCL_ITEM_NORMAL, WSTL_LCL_ITEM_NORMAL, RS_GAME_TETRIS, NULL)

LDEF_SUB_MENU_ITEM(&con_game_sub_menu, txt_game_item2, &txt_game_item3, 2, 1, 3, ITEM_L, (ITEM_T + 4 * ITEM_H), ITEM_W, ITEM_H,  \
	WSTL_LCL_ITEM_NORMAL, WSTL_BUTTON_1, WSTL_LCL_ITEM_NORMAL, WSTL_LCL_ITEM_NORMAL, RS_GAME_OTHELLO, NULL)

LDEF_SUB_MENU_ITEM(&con_game_sub_menu, txt_game_item3, NULL, 3, 2, 1, ITEM_L, (ITEM_T + 6 * ITEM_H), ITEM_W, ITEM_H,  \
	WSTL_LCL_ITEM_NORMAL, WSTL_BUTTON_1, WSTL_LCL_ITEM_NORMAL, WSTL_LCL_ITEM_NORMAL, RS_GAME_BOX, NULL)*/




/*******************************************************************************
 *	local variable define
 *******************************************************************************/
struct help_item_resource win_game_help[] =
{
    {1,IM_HELP_ICON_LR,RS_CHANGE_VOL},
    {0,RS_HELP_ENTER,RS_HELP_ENTER_INTO},
    {0,RS_MENU,RS_HELP_BACK},
    {0,RS_HELP_EXIT,RS_HELP_EXIT},          
};
/*
struct help_item_resource win_game_help_tetris_started[] =
{
    {0,RS_HELP_ENTER,RS_START_PAUSE},
    {0,RS_MENU,RS_HELP_BACK},
    {0,RS_HELP_EXIT,RS_HELP_EXIT},          
};*/

struct help_item_resource win_game_help_othello_started[] =
{
    {0,RS_MENU,RS_HELP_BACK},
    {0,RS_HELP_EXIT,RS_HELP_EXIT},          
};
struct help_item_resource win_game_help_box_started[] =
{
    {0,RS_MENU,RS_HELP_BACK},
    {0,RS_HELP_EXIT,RS_HELP_EXIT},          
};
POBJECT_HEAD windows_game_list[] =
{
    (POBJECT_HEAD)&game_con,
    (POBJECT_HEAD)&game_con,
    //(POBJECT_HEAD)&game_con,
};

#define VACT_VOLUME_DECREASE	(VACT_PASS + 1)
#define VACT_VOLUME_INCREASE	(VACT_PASS + 2)
#define VACT_ITEM_SELECT	    (VACT_PASS + 3)

//#define GAME_TETRIS_ID  1
//#define GAME_SNAKE_ID  2
#define GAME_OTHELLO_ID  1
#define GAME_BOX_ID	2	

/*******************************************************************************
 *	helper functions define
 *******************************************************************************/
static VACTION game_keymap(POBJECT_HEAD pObj, UINT32 key)
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


static PRESULT game_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	OSD_RECT rc_con;
	OSD_RECT rc_preview;

	switch (event)
	{
		case EVN_PRE_OPEN:
			wincom_open_title_ext(RS_TV_GAME, IM_TITLE_ICON_SYSTEM);
			wincom_open_help(win_game_help, 4);
			//set_update_flag(TRUE);
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

		//	wincom_open_preview_ext(rc_con, rc_preview, WSTL_SUBMENU_PREVIEW);
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			//wincom_close_preview();
			break;
		case EVN_MSG_GOT:
			ret = game_message_proc(param1, param2);
			break;
		case EVN_UNKNOWN_ACTION:
			break;
	}

	return ret;
}

static VACTION game_item_keymap(POBJECT_HEAD pObj, UINT32 key)
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


static PRESULT game_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
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
				switch (id)
				{
				/*case GAME_TETRIS_ID:
					game_tetris_init();
					break;*/
				case GAME_OTHELLO_ID:
					game_othello_init();
					break;
				case GAME_BOX_ID:
					game_Box_init();
					break;
				}
				set_game_id(id);
				//wincom_close_preview();
				if (NULL != windows_game_list[id - 1])
				{
					if (OSD_ObjOpen(windows_game_list[id - 1], 0xFFFFFFFF) != PROC_LEAVE)
						menu_stack_push(windows_game_list[id - 1]);
					/*if (GAME_TETRIS_ID == id)
					{
						wincom_open_help(win_game_help_tetris_started, 3);
					}
					else*/ 
					if(GAME_OTHELLO_ID==id)
					{
						wincom_open_help(win_game_help_othello_started, 2);
					}
					else
					{
						wincom_open_help(win_game_help_box_started, 2);
					}
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


static PRESULT game_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;

	switch (msg_type)
	{
		case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
			wincom_preview_proc();
			break;
		case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
			if (is_time_inited())
			{
				wincom_draw_title_time();
			}
            break;
		default:
			break;
	}

	return ret;
}
