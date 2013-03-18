/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2005 Copyright (C)
 *
 *  File: win_game_com.c
 *
 *  Description: Common game panel for Copper UI.
 *
 *  History:
 *  Date            Author  Version  Comment
 *  ====            ======  =======  =======
 *  1.  2006.6.2    Liu Lan 0.1.000  Initial
 *
 ****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/pan/pan_dev.h>

//#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "copper_common/dev_handle.h"
#include "copper_common/com_api.h"
#include "copper_common/system_data.h"

#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_com.h"

#include "copper_common/menu_api.h"
#include "win_com_menu_define.h"
#include "win_game_com.h"



#define WIN_GAME_LEFT       (TV_OFFSET_L+0)
#define WIN_GAME_TOP        (TV_OFFSET_T+90)
#define WIN_GAME_WIDTH      720
#define WIN_GAME_HEIGHT     400

#define CON_SUB_MENU_IDX    WSTL_WIN_1
#define CON_SUB_MENU_L      (TV_OFFSET_L+416)
#define CON_SUB_MENU_T      (TV_OFFSET_T+110)
#define CON_SUB_MENU_W      240
#define CON_SUB_MENU_H      144

#define TXT_GAME_LEFT       418
#define TXT_GAME_TOP        (WIN_GAME_TOP + 24)
#define TXT_GAME_WIDTH      236
#define TXT_GAME_HEIGHT     24

#define TXT_GAME_BTN_TOP    (TXT_GAME_TOP + 40)
#define TXT_GAME_BTN_HEIGHT 28
#define TXT_GAME_BTN_GAP    32

#define TXT_GAME_LBL_TOP    (TXT_GAME_BTN_TOP + 120)
#define TXT_GAME_LBL_HEIGHT 24
#define TXT_GAME_LBL_GAP    56

#define TXT_GAME_VAL_TOP    (TXT_GAME_LBL_TOP + 28)
#define TXT_GAME_VAL_HEIGHT 24
#define TXT_GAME_VAL_GAP    56

#define WSTL_TEXT_02		WSTL_TXT_4
#define WSTL_GAME_BUTTON_01	WSTL_LCL_ITEM_NORMAL
#define WSTL_GAME_BUTTON_03	WSTL_BUTTON_1
#define WSTL_GAME_TEXT_01	WSTL_TXT_12
#define WSTL_GAME_BG_02		WSTL_SEARCH_BG_CON


PRESULT(*game_key_proc)(UINT32, UINT8, UINT8);
PRESULT(*game_setup)(POBJECT_HEAD, VEVENT, UINT32, UINT32);
void(*game_start)(void);
void(*game_pre_open)(void);
void(*game_1st_draw)(void);
void(*game_event)(UINT32, UINT32);


/*******************************************************************************
 *  Local functions & variables define
 *******************************************************************************/

static unsigned char level;
static unsigned int score1;
static unsigned int score2;
static UINT16 str_value[64];

static PRESULT game_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION game_btn_keymap(POBJECT_HEAD obj, UINT32 key);
static PRESULT game_start_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION game_setup_keymap(POBJECT_HEAD obj, UINT32 key);
static PRESULT game_setup_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT game_cancel_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT game_state_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION game_con_keymap(POBJECT_HEAD obj, UINT32 key);
static PRESULT game_con_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION game_con_submenu_keymap(POBJECT_HEAD obj, UINT32 key);
static PRESULT game_con_submenu_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);

extern CONTAINER game_con;
extern CONTAINER game_con_sub_menu;
extern TEXT_FIELD txt_playarea;
extern TEXT_FIELD txt_title;
extern TEXT_FIELD txt_start;
extern TEXT_FIELD txt_setup;
extern TEXT_FIELD txt_cancel;
extern TEXT_FIELD txt_level;
extern TEXT_FIELD txt_score1;
extern TEXT_FIELD txt_score2;
extern TEXT_FIELD txt_value1;
extern TEXT_FIELD txt_value2;
extern TEXT_FIELD txt_value3;


DEF_TEXTFIELD ( txt_playarea, &game_con, NULL, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                4, 0, 0, 0, 0, \
                WIN_GAME_LEFT, WIN_GAME_TOP, 1, 1, \
                WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
                NULL, game_callback, \
                C_ALIGN_CENTER, 0, 0, \
                0, NULL );

DEF_TEXTFIELD ( txt_title, &game_con, &txt_start, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                0, 0, 0, 0, 0, \
                TXT_GAME_LEFT, TXT_GAME_TOP, TXT_GAME_WIDTH, TXT_GAME_HEIGHT, \
                WSTL_TEXT_02, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
                NULL, NULL, \
                C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
                0, NULL );

DEF_TEXTFIELD ( txt_start, &game_con, &txt_setup, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                1, 1, 1, 3, 2, \
                TXT_GAME_LEFT, TXT_GAME_BTN_TOP + TXT_GAME_BTN_GAP * 0, TXT_GAME_WIDTH, TXT_GAME_BTN_HEIGHT, \
                WSTL_GAME_BUTTON_01, WSTL_GAME_BUTTON_03, WSTL_NOSHOW_IDX, WSTL_GAME_BUTTON_01, \
                game_btn_keymap, game_start_callback, \
                C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
                RS_COMMON_START, NULL );
DEF_TEXTFIELD ( txt_setup, &game_con, &txt_cancel, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                2, 2, 2, 1, 3, \
                TXT_GAME_LEFT, TXT_GAME_BTN_TOP + TXT_GAME_BTN_GAP * 1, TXT_GAME_WIDTH, TXT_GAME_BTN_HEIGHT, \
                WSTL_GAME_BUTTON_01, WSTL_GAME_BUTTON_03, WSTL_NOSHOW_IDX, WSTL_GAME_BUTTON_01, \
                game_setup_keymap, game_setup_callback, \
                C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
                RS_GAME_SETUP, NULL );
DEF_TEXTFIELD ( txt_cancel, &game_con, &txt_level, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                3, 3, 3, 2, 1, \
                TXT_GAME_LEFT, TXT_GAME_BTN_TOP + TXT_GAME_BTN_GAP * 2, TXT_GAME_WIDTH, TXT_GAME_BTN_HEIGHT, \
                WSTL_GAME_BUTTON_01, WSTL_GAME_BUTTON_03, WSTL_NOSHOW_IDX, WSTL_GAME_BUTTON_01, \
                game_btn_keymap, game_cancel_callback, \
                C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
                RS_COMMON_CANCEL, NULL );

DEF_TEXTFIELD ( txt_level, &game_con, &txt_score1, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                0, 0, 0, 0, 0, \
                TXT_GAME_LEFT, TXT_GAME_LBL_TOP + TXT_GAME_LBL_GAP * 0, TXT_GAME_WIDTH, TXT_GAME_LBL_HEIGHT, \
                WSTL_GAME_TEXT_01, WSTL_GAME_TEXT_01, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
                NULL, NULL, \
                C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
                0, NULL );
DEF_TEXTFIELD ( txt_score1, &game_con, &txt_score2, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                0, 0, 0, 0, 0, \
                TXT_GAME_LEFT, TXT_GAME_LBL_TOP + TXT_GAME_LBL_GAP * 1, TXT_GAME_WIDTH, TXT_GAME_LBL_HEIGHT, \
                WSTL_GAME_TEXT_01, WSTL_GAME_TEXT_01, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
                NULL, NULL, \
                C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
                0, NULL );
DEF_TEXTFIELD ( txt_score2, &game_con, &txt_value1, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                0, 0, 0, 0, 0, \
                TXT_GAME_LEFT, TXT_GAME_LBL_TOP + TXT_GAME_LBL_GAP * 2, TXT_GAME_WIDTH, TXT_GAME_LBL_HEIGHT, \
                WSTL_GAME_TEXT_01, WSTL_GAME_TEXT_01, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
                NULL, NULL, \
                C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
                0, NULL );

DEF_TEXTFIELD ( txt_value1, &game_con, &txt_value2, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                31, 0, 0, 0, 0, \
                TXT_GAME_LEFT, TXT_GAME_VAL_TOP + TXT_GAME_VAL_GAP * 0, TXT_GAME_WIDTH, TXT_GAME_VAL_HEIGHT, \
                WSTL_GAME_TEXT_01, WSTL_GAME_TEXT_01, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
                NULL, game_state_callback, \
                C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
                0, NULL );
DEF_TEXTFIELD ( txt_value2, &game_con, &txt_value3, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                32, 0, 0, 0, 0, \
                TXT_GAME_LEFT, TXT_GAME_VAL_TOP + TXT_GAME_VAL_GAP * 1, TXT_GAME_WIDTH, TXT_GAME_VAL_HEIGHT, \
                WSTL_GAME_TEXT_01, WSTL_GAME_TEXT_01, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
                NULL, game_state_callback, \
                C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
                0, NULL );
DEF_TEXTFIELD ( txt_value3, &game_con, &txt_playarea, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                33, 0, 0, 0, 0, \
                TXT_GAME_LEFT, TXT_GAME_VAL_TOP + TXT_GAME_VAL_GAP * 2, TXT_GAME_WIDTH, TXT_GAME_VAL_HEIGHT, \
                WSTL_GAME_TEXT_01, WSTL_GAME_TEXT_01, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
                NULL, game_state_callback, \
                C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, \
                0, NULL );

DEF_CONTAINER ( game_con, NULL, NULL, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                0, 0, 0, 0, 0, \
                WIN_GAME_LEFT, WIN_GAME_TOP, WIN_GAME_WIDTH, WIN_GAME_HEIGHT, \
                WSTL_GAME_BG_02, WSTL_GAME_BG_02, WSTL_NOSHOW_IDX, WSTL_NOSHOW_IDX, \
                game_con_keymap, game_con_callback, \
                &game_con_sub_menu, 1, 0 );

DEF_CONTAINER ( game_con_sub_menu, &game_con, &txt_title, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                0, 0, 0, 0, 0, \
                CON_SUB_MENU_L, CON_SUB_MENU_T, CON_SUB_MENU_W, CON_SUB_MENU_H, \
                CON_SUB_MENU_IDX, CON_SUB_MENU_IDX, CON_SUB_MENU_IDX, CON_SUB_MENU_IDX, \
                game_con_submenu_keymap, game_con_submenu_callback, \
                NULL, 1, 0 );

/*******************************************************************************
 *  Game function
 *******************************************************************************/
void setup_game_panel(UINT16 tit_id, UINT16 lev_id, UINT16 sco1_id, UINT16 sco2_id)
{
	OSD_SetTextFieldContent(&txt_title, STRING_ID, tit_id);
	OSD_SetTextFieldContent(&txt_level, STRING_ID, lev_id);
	OSD_SetTextFieldContent(&txt_score1, STRING_ID, sco1_id);
	OSD_SetTextFieldContent(&txt_score2, STRING_ID, sco2_id);
}

void update_status(unsigned int value1, unsigned int value2, unsigned int value3)
{
	VSCR *lpVscr;

	level = value1;
	score1 = value2;
	score2 = value3;
	OSD_DrawObject((POBJECT_HEAD) &txt_value1, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
	OSD_DrawObject((POBJECT_HEAD) &txt_value2, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
	OSD_DrawObject((POBJECT_HEAD) &txt_value3, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
	lpVscr = OSD_GetTaskVscr(osal_task_get_current_id());
	OSD_UpdateVscr(lpVscr);
#ifndef USE_LIB_GE
	lpVscr->lpbScr = NULL;
#endif
}
void update_status_box(unsigned int value1)
{
	VSCR *lpVscr;
	level = value1;
	OSD_DrawObject((POBJECT_HEAD) &txt_value1, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
	lpVscr = OSD_GetTaskVscr(osal_task_get_current_id());
	OSD_UpdateVscr(lpVscr);
#ifndef USE_LIB_GE
	lpVscr->lpbScr = NULL;
#endif
}

static UINT8 game_id=1;
UINT8 get_game_id()
{
	return game_id;
}
void set_game_id(UINT8 id)
{
	game_id=id;
}

void drawSquare(UINT16 x, UINT16 y, UINT16 w, UINT16 h, UINT8 bShIdx)
{
	OSD_RECT r;
	VSCR*	lpVscr ;
#if 0//def USE_LIB_GE
	lpVscr= OSD_GetTaskVscr(osal_task_get_current_id());
	OSD_SetRect(&r, x, y, w, h);
	OSD_DrawStyleRect(&r, bShIdx, lpVscr);
	
	OSD_UpdateVscr(lpVscr);
#else
	OSD_SetRect(&r, x, y, w, h);
	OSD_DrawStyleRect(&r, bShIdx, NULL);
#endif
}

void drawBitmap(UINT16 x, UINT16 y, UINT16 w, UINT16 h, UINT16 rsc_id)
{
	VSCR*	lpVscr ;
/*#ifdef USE_LIB_GE
	lpVscr= OSD_GetTaskVscr(osal_task_get_current_id());
	OSD_DrawPicture(x, y, rsc_id, LIB_ICON, 0, lpVscr);
	OSD_UpdateVscr(lpVscr);
#else*/
	OSD_DrawPicture(x, y, rsc_id, LIB_ICON, 0, NULL);
//#endif

}

static PRESULT games_com_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;

	switch (msg_type)
	{
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

static PRESULT game_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	UINT32 vkey;
	UINT8 key_repeat_cnt;
	UINT8 key_status;
	PRESULT ret = PROC_PASS;

	switch (event)
	{
		case EVN_UNKNOWNKEY_GOT:
			ret = PROC_LOOP;
			key_repeat_cnt = (UINT8)(param1 >> 16)&0xFF;
			key_status = (UINT8)(param1 >> 28)&0x01;
			ap_hk_to_vk(0, param1, &vkey);

			ret = game_key_proc(vkey, key_repeat_cnt, key_status);
			break;
	}

	return ret;
}


static VACTION game_btn_keymap(POBJECT_HEAD obj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
	}

	return act;
}


static PRESULT game_start_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 unact;

	switch (event)
	{
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			if (unact == VACT_ENTER)
			{
				OSD_SetAttr((POBJECT_HEAD)obj, C_ATTR_INACTIVE);
				OSD_ChangeFocus((POBJECT_HEAD) &game_con, 4,  \
					C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);

				game_start();
			}
			break;
	}

	return ret;
}


static VACTION game_setup_keymap(POBJECT_HEAD obj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_LEFT:
			act = VACT_DECREASE;
			break;
		case V_KEY_ENTER:
		case V_KEY_RIGHT:
			act = VACT_INCREASE;
			break;
	}

	return act;
}


static PRESULT game_setup_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	return game_setup(obj, event, param1, param2);
}


static PRESULT game_cancel_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 unact;

	switch (event)
	{
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			if (unact == VACT_ENTER)
			{
				ret = PROC_LEAVE;
			}
			break;
	}

	return ret;
}


static PRESULT game_state_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	unsigned char ascstr[6];
	PRESULT ret = PROC_PASS;

	switch (event)
	{
		case EVN_PRE_DRAW:
			switch (obj->bID)
			{
			case 31:
				sprintf(ascstr, "%d", level);
				break;
			case 32:
				sprintf(ascstr, "%d", score1);
				break;
			case 33:
				sprintf(ascstr, "%d", score2);
				break;
			}
			ComAscStr2Uni(ascstr, str_value);
			if((get_game_id()==3)&&obj->bID!=31)
			{
				sprintf(ascstr,"%s"," ");
				ComAscStr2Uni(ascstr, str_value); 
			}
			OSD_SetTextFieldStrPoint((PTEXT_FIELD)obj, str_value);
	}

	return ret;
}


static VACTION game_con_keymap(POBJECT_HEAD obj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		case V_KEY_EXIT:
			restore_scene();
			BackToFullScrPlay();
			break;

		default:
			act = OSD_ContainerKeyMap(obj, key);
			break;
	}

	return act;
}


extern CONTAINER g_win_mainmenu;
static PRESULT game_con_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;

	switch (event)
	{
		case EVN_PRE_OPEN:
#ifdef USE_LIB_GE
			OSD_SetVscrDst(GUI_MEMVSCR_SURF,0);
			PGUI_VSCR pVscr=OSD_GetTaskVscr(osal_task_get_current_id());
			OSD_SetRect(&(pVscr->frm),0,0,0,0);
#endif
			record_scene();
			OSD_SetAttr((POBJECT_HEAD) &txt_start, C_ATTR_ACTIVE);
			OSD_SetContainerFocus(&game_con, 1);
			game_pre_open();
			break;
		case EVN_POST_OPEN:
#ifdef USE_LIB_GE
			OSD_SetVscrDst(GUI_GMA2_SURF,0);
#endif
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			restore_scene();
			break;
		case EVN_POST_DRAW:
			game_1st_draw();
			break;

		case EVN_MSG_GOT:
			ret = games_com_message_proc(param1, param2);
			game_event(param1, param2);
			break;
	}
	return ret;
}

static VACTION game_con_submenu_keymap(POBJECT_HEAD obj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (act)
	{
		case V_KEY_MENU:
		case V_KEY_EXIT:
			break;
		default:
			break;
			//act = OSD_ContainerKeyMap ( obj, key );
	}
	return act;
}

static PRESULT game_con_submenu_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	return ret;
}
