#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_as.h>
#include <hld/dis/vpo.h>
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

#include "control.h"

#include "win_timer_set.h"
/*******************************************************************************
 *	WINDOW's objects declaration
 *******************************************************************************/
extern CONTAINER g_win_timerset;

extern TEXT_FIELD timer_cur_datetime;

extern CONTAINER timer_con_mode; // timer mode: off, once, daily, weekly, monthly, yearly
extern CONTAINER timer_con_servicetype; // timer service: channel, REC, messages
extern CONTAINER timer_con_serviceinfo; //channel // message
extern CONTAINER timer_con_wakeupdate;
extern CONTAINER timer_con_wakeuptime;
extern CONTAINER timer_con_duration;

extern TEXT_FIELD timer_txt_mode; // timer mode: off, once, daily, weekly, monthly, yearly
extern TEXT_FIELD timer_txt_servicetype; // timer service: channel, REC, messages
extern TEXT_FIELD timer_txt_serviceinfo; //channel / message
extern TEXT_FIELD timer_txt_wakeupdate;
extern TEXT_FIELD timer_txt_wakeuptime;
extern TEXT_FIELD timer_txt_duration;

extern MULTISEL timer_msel_mode;
extern MULTISEL timer_msel_servicetype;
extern MULTISEL timer_msel_serviceinfo;
extern EDIT_FIELD timer_edf_wakeupdate;
extern EDIT_FIELD timer_edf_wakeuptime;
extern EDIT_FIELD timer_edf_duration;

extern TEXT_FIELD timer_ok;
extern TEXT_FIELD timer_cancel;

static VACTION timerset_msel_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT timerset_msel_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION timerset_mselinfo_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT timerset_mselinfo_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION timerset_edf_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT timerset_edf_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION timerset_item_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT timerset_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION timerset_btn_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT timerset_btn_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION timerset_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT timerset_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

#define WIN_SH_IDX	 WSTL_POP_WIN_1

#define CON_SH_IDX   WSTL_TXT_POPWIN
#define CON_HL_IDX   WSTL_BUTTON_1
#define CON_SL_IDX   WSTL_BUTTON_1
#define CON_GRY_IDX  WSTL_TXT_POPWIN

#define TXT_SH_IDX   WSTL_TXT_POPWIN
#define TXT_HL_IDX   WSTL_TXT_5
#define TXT_SL_IDX   WSTL_TXT_5
#define TXT_GRY_IDX  WSTL_TXT_10

#define SEL_SH_IDX   WSTL_TXT_POPWIN
#define SEL_HL_IDX   WSTL_BUTTON_5//WSTL_BUTTON_3
#define SEL_SL_IDX   WSTL_BUTTON_3
#define SEL_GRY_IDX  WSTL_TXT_10

#define EDF_SH_IDX   WSTL_TXT_POPWIN
#define EDF_HL_IDX   WSTL_BUTTON_5//WSTL_BUTTON_3
#define EDF_SL_IDX   WSTL_BUTTON_3
#define EDF_GRY_IDX  WSTL_TXT_10


#define BTN_SH_IDX	WSTL_BUTTON_POP_SH
#define BTN_HL_IDX	WSTL_BUTTON_POP_HL


#define W_L     140
#define W_T     100
#define W_W     440
#define W_H     300

#define DATETIME_L (W_L + 10)
#define DATETIME_T (W_T + 12)
#define DATETIME_W (W_W - 40)
#define DATETIME_H 26

#define CON_L	(W_L + 4)
#define CON_T	(DATETIME_T + DATETIME_H + 10)
#define CON_W	(W_W - 8)
#define CON_H	28
#define CON_GAP 2

#define TXT_L_OF  	4
#define TXT_W  		180
#define TXT_H		28
#define TXT_T_OF	((CON_H - TXT_H)/2)

#define SEL_L_OF  (TXT_L_OF + TXT_W)
#define SEL_W  		220
#define SEL_H		28
#define SEL_T_OF	((CON_H - SEL_H)/2)

#define BTN1_L (W_L + 120)
#define BTN2_L (BTN1_L + 120)
#define BTN_T (W_T + 250)
#define BTN_W  80
#define BTN_H 30

#define LDEF_CON(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    timerset_item_keymap,timerset_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,varTxt,nxtObj,l,t,w,h,resID)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,resID,NULL)

#define LDEF_MSEL(root, varNum, nxtObj, ID, l, t, w, h,style,cur,cnt,ptabl)	\
    DEF_MULTISEL(varNum,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    timerset_msel_keymap,timerset_msel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_TXT_DATETIME(root,varTxt,nxtObj,l,t,w,h,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT | C_ALIGN_VCENTER, 0,0,0,str)


#define LDEF_TXT_BTN(root,varTxt,nxtObj,l,t,w,h,ID,IDl,IDr,IDu,IDd,resID)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, BTN_SH_IDX,BTN_HL_IDX,BTN_SH_IDX,BTN_SH_IDX,   \
    timerset_btn_keymap,timerset_btn_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,resID,NULL)


#define LDEF_MSEL_INFO(root, varNum, nxtObj, ID, l, t, w, h,style,cur,cnt,ptabl)	\
    DEF_MULTISEL(varNum,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    timerset_mselinfo_keymap,timerset_mselinfo_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)


#define LDEF_EDIT(root, varNum, nxtObj, ID, l, t, w, h,style,cursormode,pat,pre,sub,str)	\
    DEF_EDITFIELD(varNum,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, EDF_SH_IDX,EDF_HL_IDX,EDF_SL_IDX,EDF_GRY_IDX,   \
    timerset_edf_keymap,timerset_edf_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,pat,10,cursormode,pre,sub,str)


#define LDEF_MENU_ITEM_SEL(root,varCon,nxtObj,varTxt,varNum,ID,IDu,IDd,\
						l,t,w,h,resID,style,cur,cnt,ptbl) \
    LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,&varTxt,ID)   \
    LDEF_TXT(&varCon,varTxt,&varNum,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,resID)    \
    LDEF_MSEL(&varCon,varNum,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)

#define LDEF_MENU_ITEM_SELINFO(root,varCon,nxtObj,varTxt,varNum,ID,IDu,IDd,\
						l,t,w,h,resID,style,cur,cnt,ptbl) \
    LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,&varTxt,ID)   \
    LDEF_TXT(&varCon,varTxt,&varNum,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,resID)    \
    LDEF_MSEL_INFO(&varCon,varNum,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)

#define LDEF_MENU_ITEM_EDF(root,varCon,nxtObj,varTxt,varNum,ID,IDu,IDd,\
						l,t,w,h,resID,style,cursormode,pat,pre,sub,str) \
    LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,CON_L,t,CON_W,CON_H,&varTxt,ID)   \
    LDEF_TXT(&varCon,varTxt,&varNum,    l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,resID)    \
    LDEF_EDIT(&varCon,varNum,NULL   ,ID,l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cursormode,pat,pre,sub,str)

#define LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    timerset_keymap,timerset_callback,  \
    nxtObj, focusID,0)

UINT16	TimerModeStr_id[]  =
    {
        RS_COMMON_OFF,
        RS_SYSTEM_TIMERSET_ONCE,
        RS_SYSTEM_TIMERSET_DAILY,
    };

UINT16	TimerServiceStr_id[]  =
    {
        RS_SYSTEM_TIMERSET_CHANNEL,
        RS_SYSTEM_TIMERSET_MESSAGE,
    };

UINT16	TimerServiceTxtStr_id[]  =
    {
        RS_SYSTEM_TIMERSET_WAKEUP_CHANNEL,
        RS_SYSTEM_TIMERSET_WAKEUP_MESSAGE,
    };

UINT16	WakeupMessageStr_id[]  =
    {
        RS_SYSTEM_TIMERSET_MSG_BIRTHDAY,
        RS_SYSTEM_TIMERSET_MSG_ANNIVERSARY,
        RS_SYSTEM_TIMERSET_MSG_GENERAL,
    };

//extern UINT16	TimerModeStr_id[];
//extern UINT16	TimerServiceStr_id[];
//extern UINT16	WakeupMessageStr_id[];
extern char 	date_pat[];
extern char 	time_pat[];

LDEF_TXT_DATETIME ( &g_win_timerset, timer_cur_datetime, &timer_con_mode,	\
                    DATETIME_L, DATETIME_T, DATETIME_W, DATETIME_H, display_strs[4] )

LDEF_MENU_ITEM_SEL ( g_win_timerset, timer_con_mode, &timer_con_servicetype, timer_txt_mode, timer_msel_mode, 1, 7, 2, \
                     CON_L, CON_T + ( CON_H + CON_GAP ) *0, CON_W, CON_H, RS_SYSTEM_TIMERSET_TIMER_MODE, STRING_ID, 0, 3, TimerModeStr_id )

LDEF_MENU_ITEM_SEL ( g_win_timerset, timer_con_servicetype, &timer_con_serviceinfo, timer_txt_servicetype, timer_msel_servicetype, 2, 1, 3, \
                     CON_L, CON_T + ( CON_H + CON_GAP ) *1, CON_W, CON_H, RS_SYSTEM_TIMERSET_TIMER_SERVICE, STRING_ID, 0, TIMER_SERVICE_MESSAGE + 1, TimerServiceStr_id )

LDEF_MENU_ITEM_SELINFO ( g_win_timerset, timer_con_serviceinfo, &timer_con_wakeupdate, timer_txt_serviceinfo, timer_msel_serviceinfo, 3, 2, 4, \
                         CON_L, CON_T + ( CON_H + CON_GAP ) *2, CON_W, CON_H, RS_SYSTEM_TIMERSET_WAKEUP_MESSAGE, STRING_PROC, 0, 3, NULL )

LDEF_MENU_ITEM_EDF ( g_win_timerset, timer_con_wakeupdate, &timer_con_wakeuptime, timer_txt_wakeupdate, timer_edf_wakeupdate, 4, 3, 5, \
                     CON_L, CON_T + ( CON_H + CON_GAP ) *3, CON_W, CON_H, RS_SYSTEM_TIMERSET_WAKEUPDATE, \
                     NORMAL_EDIT_MODE, CURSOR_NORMAL, date_pat, NULL, NULL, display_strs[0] )

LDEF_MENU_ITEM_EDF ( g_win_timerset, timer_con_wakeuptime, &timer_con_duration, timer_txt_wakeuptime, timer_edf_wakeuptime, 5, 4, 6, \
                     CON_L, CON_T + ( CON_H + CON_GAP ) *4, CON_W, CON_H, RS_SYSTEM_TIMERSET_ONTIME, \
                     NORMAL_EDIT_MODE, CURSOR_NORMAL, time_pat, NULL, NULL, display_strs[1] )

LDEF_MENU_ITEM_EDF ( g_win_timerset, timer_con_duration, &timer_ok, timer_txt_duration, timer_edf_duration, 6, 5, 7, \
                     CON_L, CON_T + ( CON_H + CON_GAP ) *5, CON_W, CON_H, RS_SYSTEM_TIMERSET_DURATION, \
                     NORMAL_EDIT_MODE, CURSOR_NORMAL, time_pat, NULL, NULL, display_strs[2] )


LDEF_TXT_BTN ( &g_win_timerset, timer_ok, &timer_cancel, \
               BTN1_L, BTN_T, BTN_W, BTN_H, 7, 8, 8, 6, 1, RS_COMMON_SAVE )
LDEF_TXT_BTN ( &g_win_timerset, timer_cancel, NULL, \
               BTN2_L, BTN_T, BTN_W, BTN_H, 8, 7, 7, 6, 1, RS_COMMON_CANCEL )

LDEF_WIN ( g_win_timerset, &timer_cur_datetime, W_L, W_T, W_W, W_H, 1 )

typedef enum
{
    TIMER_MODE_ID = 1,
    TIMER_SERVICE_ID,
    TIMER_SERVICEINFO_ID,
    TIMER_DATE_ID,
    TIMER_TIME_ID,
    TIMER_DURATION_ID,
    TIMER_BTN_OK_ID,
    TIMER_BTN_CANCEL_ID,
}
TIMER_SET_ID;

#define TIMER_MODE			(timer_msel_mode.nSel)
#define TIMER_SERVICE_TYPE (timer_msel_servicetype.nSel)
#define TIMER_SERVICE_INFO (timer_msel_serviceinfo.nSel)
#define TIMER_SERVICE_INFO_CNT (timer_msel_serviceinfo.nCount)

/*******************************************************************************
 *	Local vriable & function declare
 *******************************************************************************/
#define TIMER_ADD 		0
#define TIMER_MODIFY 	1

char date_pat[] = "d00";
char time_pat[] = "t0";
UINT8 timer_set_ok_cancel_flag; /* 0 - cancel, 1 - ok*/
UINT8 timer_set_modify_add_flag;
UINT8 timer_set_check_starttime_flag;
TIMER_SET_CONTENT timer_set;

extern UINT16 WakeupMessageStr_id[];
extern UINT16 TimerServiceTxtStr_id[];

void win_timer_load(TIMER_SET_CONTENT *settimer);
void win_timerset_timermode_change(BOOL update);
void win_timerset_timerservice_change(BOOL update);
void win_timerset_timerserviceinfo_change(BOOL update, UINT16 sel);
void win_timerset_set_currenttime(BOOL update);

INT32 win_timerset_check_and_save(void);

INT32 IsValidTimer(TIMER_SET_CONTENT *timer);
/*******************************************************************************
 *	key mapping and event callback definition
 *******************************************************************************/

// mode, service
static VACTION timerset_msel_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act;

	switch (key)
	{
		case V_KEY_LEFT:
			act = VACT_DECREASE;
			break;
		case V_KEY_RIGHT:
			act = VACT_INCREASE;
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}

static PRESULT timerset_msel_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 bID;
	UINT32 sel;

	bID = OSD_GetObjID(pObj);

	switch (event)
	{
		case EVN_PRE_CHANGE:
			sel = *((UINT32*)param1);
		case EVN_POST_CHANGE:
			sel = param1;
			if (bID == TIMER_MODE_ID)
				win_timerset_timermode_change(TRUE);
			else if (bID == TIMER_SERVICE_ID)
				win_timerset_timerservice_change(TRUE);
			else if (bID == TIMER_SERVICEINFO_ID)
				win_timerset_timerserviceinfo_change(TRUE, (UINT16)sel);
			break;
	}

	return ret;
}

// service information
static VACTION timerset_mselinfo_keymap(POBJECT_HEAD pObj, UINT32 key)
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
		default:
			break;
	}

	if (TIMER_SERVICE_TYPE == TIMER_SERVICE_CHANNEL)
	{
		if (key == V_KEY_ENTER)
			act = VACT_ENTER;
	}

	return act;
}

static PRESULT timerset_mselinfo_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT32 sel;
	VACTION unact;
	P_NODE p_node;
	UINT16 *Uni_str;
	TIMER_SET_CONTENT *settimer;
	COM_POP_LIST_PARAM_T param;
	OSD_RECT rect;


	settimer = &timer_set;


	switch (event)
	{
		case EVN_PRE_CHANGE:
			sel = *((UINT32*)param1);
			break;
		case EVN_POST_CHANGE:
			sel = param1;
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			if (unact == VACT_ENTER)
			// select channel
			{
				OSD_SetRect2(&rect, &pObj->frame);
				rect.uLeft -= 40;
				rect.uWidth += 40;
				rect.uHeight = 180;

				param.selecttype = POP_LIST_SINGLESELECT;
				param.cur = TIMER_SERVICE_INFO;
				sel = win_com_open_sub_list(POP_LIST_TYPE_CHANLIST, &rect, &param);
				if (sel < (UINT32)TIMER_SERVICE_INFO_CNT)
				{
					get_prog_at(sel, &p_node);
					settimer->wakeup_channel = p_node.prog_id;
					//MEMCPY(settimer->event_name, (UINT8 *)&p_node.service_name[0], 40);
					TIMER_SERVICE_INFO = sel;
				}

				OSD_TrackObject((POBJECT_HEAD) &timer_con_serviceinfo, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
			}
			break;
		case EVN_REQUEST_STRING:
			sel = param1;
			Uni_str = (UINT16*)param2;
			get_prog_at(sel, &p_node);
			if (p_node.ca_mode)
				ComAscStr2Uni("$", Uni_str);
			ComUniStrCopyChar((UINT8*) &Uni_str[p_node.ca_mode], p_node.service_name);
			MEMCPY(settimer->event_name, (UINT8 *)Uni_str, 40);
			break;

	}

	return ret;
}

//date, on time, duration
static VACTION timerset_edf_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act;

	switch (key)
	{
		case V_KEY_LEFT:
		case V_KEY_RIGHT:
			act = (key == V_KEY_LEFT) ? VACT_EDIT_LEFT : VACT_EDIT_RIGHT;
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
			act = key - V_KEY_0 + VACT_NUM_0;
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}

static PRESULT timerset_edf_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;

	switch (event)
	{
		case EVN_PRE_CHANGE:
			break;
		case EVN_POST_CHANGE:
			break;
	}


	return ret;
}

static VACTION timerset_item_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	return act;
}

static PRESULT timerset_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;

	return ret;
}

// ok, cancel btn
static VACTION timerset_btn_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	if (key == V_KEY_ENTER)
		act = VACT_ENTER;

	return act;

}

static PRESULT timerset_btn_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 bID;
	UINT8 back_save;

	bID = OSD_GetObjID(pObj);

	switch (event)
	{
		case EVN_UNKNOWN_ACTION:
			if (bID == TIMER_BTN_OK_ID)
			{
				if (win_timerset_check_and_save() == 0)
				{
					timer_set_ok_cancel_flag = 1;
					ret = PROC_LEAVE;
				}
				else
				{
					win_compopup_init(WIN_POPUP_TYPE_OK);
					win_compopup_set_msg(NULL, NULL, RS_MSG_INVALID_INPUT_CONTINUE);
					win_compopup_open_ext(&back_save);
				}
			}
			else
				ret = PROC_LEAVE;

			break;
	}

	return ret;
}


static VACTION timerset_keymap(POBJECT_HEAD pObj, UINT32 key)
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
		case V_KEY_LEFT:
			act = VACT_CURSOR_LEFT;
			break;
		case V_KEY_RIGHT:
			act = VACT_CURSOR_RIGHT;
			break;
		case V_KEY_EXIT:
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}

static PRESULT timerset_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 msg_flag;

	switch (event)
	{
		case EVN_PRE_OPEN:
			win_timer_load(&timer_set);
#ifdef SUPPORT_MSG
			msg_flag = 1;
#else
			msg_flag = 0;
#endif
			if (!msg_flag)
				remove_menu_item((CONTAINER*)pObj, (POBJECT_HEAD) &timer_con_servicetype, (CON_H + CON_GAP));
			break;
		case EVN_POST_OPEN:
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			break;
	}

	return ret;
}

void set_container_active(CONTAINER *con, UINT8 action)
{
	POBJECT_HEAD pObj;

	OSD_SetAttr(con, action);
	pObj = OSD_GetContainerNextObj(con);
	while (pObj)
	{
		OSD_SetAttr(pObj, action);
		pObj = OSD_GetObjpNext(pObj);
	}
}

static UINT8 timerSec = 0;

void win_timer_load(TIMER_SET_CONTENT *settimer)
{
	MULTISEL *msel;
	EDIT_FIELD *edf;
	TEXT_FIELD *txt;
	P_NODE p_node;
	UINT16 ch_idx, ch_cnt;
	UINT32 value;
	UINT8 action;

	win_timerset_set_currenttime(FALSE);

	/* TIMER_MODE*/
	msel = &timer_msel_mode;
	if (settimer->timer_mode >= OSD_GetMultiselCount(msel))
		settimer->timer_mode = TIMER_MODE_OFF;
	OSD_SetMultiselSel(msel, settimer->timer_mode);

	/* TIMER_SERVICE */
	msel = &timer_msel_servicetype;
	if (settimer->timer_service >= OSD_GetMultiselCount(msel))
		settimer->timer_service = 0;
	OSD_SetMultiselSel(msel, settimer->timer_service);

	/* TIMER service information: message or channel */
	txt = &timer_txt_serviceinfo;
	OSD_SetTextFieldContent(txt, STRING_ID, (UINT32)TimerServiceTxtStr_id[settimer->timer_service]);


	win_timerset_timerservice_change(FALSE);

	edf = &timer_edf_wakeupdate;
	value = settimer->wakeup_year *10000+settimer->wakeup_month *100+settimer->wakeup_day;
	OSD_SetEditFieldContent(edf, STRING_NUMBER, value);


	edf = &timer_edf_wakeuptime;
	value = (settimer->wakeup_time / 3600) *10000+((settimer->wakeup_time / 60) % 60)*100 + settimer->wakeup_time%60;
	timerSec = settimer->wakeup_time % 60;
	OSD_SetEditFieldContent(edf, STRING_NUMBER, value);


	edf = &timer_edf_duration;
	value = settimer->wakeup_duration_time / 3600 * 10000 + (settimer->wakeup_duration_time/60%60)*100 + settimer->wakeup_duration_time%60;
	OSD_SetEditFieldContent(edf, STRING_NUMBER, value);

	win_timerset_timermode_change(FALSE);


}

void win_timerset_timermode_change(BOOL update)
{
	UINT8 action;


	if (TIMER_MODE == TIMER_MODE_OFF)
		action = C_ATTR_INACTIVE;
	else
		action = C_ATTR_ACTIVE;

	if (OSD_CheckAttr(&timer_con_servicetype, action))
		return ;

	set_container_active(&timer_con_servicetype, action);
	set_container_active(&timer_con_serviceinfo, action);
	set_container_active(&timer_con_wakeupdate, action);
	set_container_active(&timer_con_wakeuptime, action);

	if (action == C_ATTR_ACTIVE && TIMER_SERVICE_TYPE == TIMER_SERVICE_MESSAGE)
		action = C_ATTR_INACTIVE;
	set_container_active(&timer_con_duration, action);
	if (update)
	{
		OSD_DrawObject((POBJECT_HEAD) &timer_con_servicetype, C_UPDATE_ALL);
		OSD_DrawObject((POBJECT_HEAD) &timer_con_serviceinfo, C_UPDATE_ALL);
		OSD_DrawObject((POBJECT_HEAD) &timer_con_wakeupdate, C_UPDATE_ALL);
		OSD_DrawObject((POBJECT_HEAD) &timer_con_wakeuptime, C_UPDATE_ALL);
		OSD_DrawObject((POBJECT_HEAD) &timer_con_duration, C_UPDATE_ALL);
	}
}

void win_timerset_timerservice_change(BOOL update)
{
	UINT8 action;
	TEXT_FIELD *txt;
	MULTISEL *msel;
	UINT16 ch_idx, ch_cnt;
	TIMER_SET_CONTENT *settimer;
	P_NODE p_node;


	settimer = &timer_set;

	if (TIMER_MODE == TIMER_MODE_OFF)
		action = C_ATTR_INACTIVE;
	else
	{
		if (TIMER_SERVICE_TYPE == TIMER_SERVICE_MESSAGE)
			action = C_ATTR_INACTIVE;
		else
			action = C_ATTR_ACTIVE;
	}

	/* TIMER service information: message or channel */
	txt = &timer_txt_serviceinfo;
	msel = &timer_msel_serviceinfo;

	if (TIMER_SERVICE_TYPE == TIMER_SERVICE_MESSAGE)
	{
		/* TIMER_SERVICE_SMG */
		OSD_SetMultiselSelType(msel, STRING_ID);
		OSD_SetMultiselCount(msel, 3);
		OSD_SetMultiselSelTable(msel, (void*)WakeupMessageStr_id);
		if (settimer->wakeup_message >= 3)
			settimer->wakeup_message = 0;
		OSD_SetMultiselSel(msel, settimer->wakeup_message);
	}
	else
	{
		//modified by Robin

		if (get_prog_by_id(settimer->wakeup_channel, &p_node) == DB_SUCCES)
			ch_idx = get_prog_pos(settimer->wakeup_channel);
		else
			ch_idx = sys_data_get_cur_group_cur_mode_channel();
		ch_cnt = get_prog_num(VIEW_ALL | settimer->wakeup_chan_mode, 0);
		OSD_SetMultiselSelType(msel, STRING_PROC);
		OSD_SetMultiselCount(msel, ch_cnt);
		OSD_SetMultiselSel(msel, ch_idx);
	}
	OSD_SetTextFieldContent(txt, STRING_ID, (UINT32)TimerServiceTxtStr_id[TIMER_SERVICE_TYPE]);
	if (update)
		OSD_DrawObject((POBJECT_HEAD) &timer_con_serviceinfo, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);

	set_container_active(&timer_con_duration, action);
	if (update)
		OSD_DrawObject((POBJECT_HEAD) &timer_con_duration, C_UPDATE_ALL);
}


void win_timerset_timerserviceinfo_change(BOOL update, UINT16 sel)
{
	MULTISEL *msel;
	TIMER_SET_CONTENT *settimer;
	P_NODE p_node;
	UINT16 ch_idx, ch_cnt;


	settimer = &timer_set;
	msel = &timer_msel_serviceinfo;

	if (TIMER_SERVICE_TYPE == TIMER_SERVICE_MESSAGE)
	{
		OSD_SetMultiselSelType(msel, STRING_ID);
		OSD_SetMultiselCount(msel, 3);
		OSD_SetMultiselSelTable(msel, (void*)WakeupMessageStr_id);
		if (sel >= 3)
			sel = 0;
		settimer->wakeup_message = sel;
		OSD_SetMultiselSel(msel, settimer->wakeup_message);
	}
	else
	{
		if (get_prog_at(sel, &p_node) == DB_SUCCES)
			ch_idx = sel;
		else
		{
			ch_idx = sel = 0;
			get_prog_at(sel, &p_node);
		}
		settimer->wakeup_channel = p_node.prog_id;

		ch_cnt = get_prog_num(VIEW_ALL | settimer->wakeup_chan_mode, 0);
		OSD_SetMultiselSelType(msel, STRING_PROC);
		OSD_SetMultiselCount(msel, ch_cnt);
		OSD_SetMultiselSel(msel, ch_idx);
	}
	if (update)
		OSD_DrawObject((POBJECT_HEAD) &timer_con_serviceinfo, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
}

void win_timerset_set_currenttime(BOOL update)
{
	TEXT_FIELD *txt;
	char str[30];
	date_time dt;

	get_local_time(&dt);
	txt = &timer_cur_datetime;
	sprintf(str, "%d:%02d %04d-%02d-%02d", dt.hour, dt.min, dt.year, dt.month, dt.day);
	OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
	if (update)
		OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
}

/*
return value:
-1 :   wakeup_year  <= 2000 || wakeup_year>2100
-2 :   wakeup_month<0 || wakeup_month>12
-3 :   wakeup_day not valied in the wakeup_year & wakeup_month
-4 :   on time hour > 23
-5 :   on timer minute > 59
-6 :   duration minute > 59
-7 :   add timer the wakeup date & time < current time
-8:    add timer conflict with other timers
 */
INT32 win_timerset_check_and_save(void)
{
	EDIT_FIELD *edf;
	UINT16 ch_idx, ch_cnt;
	P_NODE p_node;
	date_time dt, wkdt;
	UINT32 val, duration;
	INT32 ret;


	TIMER_SET_CONTENT *timer;

	timer = &timer_set;

	timer->timer_mode = TIMER_MODE;
	timer->timer_service = TIMER_SERVICE_TYPE;
	if (timer->timer_service == TIMER_SERVICE_MESSAGE)
		timer->wakeup_message = TIMER_SERVICE_INFO;
	else
	{
		get_prog_at(TIMER_SERVICE_INFO, &p_node);
		timer->wakeup_channel = p_node.prog_id;
	}

	edf = &timer_edf_wakeupdate;
	OSD_GetEditFieldIntValue(edf, &val);
	wkdt.year = val / 10000;
	val -= wkdt.year * 10000;
	wkdt.month = val / 100;
	val -= wkdt.month * 100;
	wkdt.day = val;

	edf = &timer_edf_wakeuptime;
	OSD_GetEditFieldIntValue(edf, &val);
	wkdt.hour = val / 10000;
	val -= wkdt.hour * 10000;
	wkdt.min = val/100;
	wkdt.sec = timerSec;

	ret = api_check_valid_date(wkdt.year, wkdt.month, wkdt.day);
	if (ret != 0)
		return ret;

	if (wkdt.hour > 23)
		return  - 4;
	if (wkdt.min > 59)
		return  - 5;

	if (timer_set_check_starttime_flag)
	{
		if (timer->timer_mode != TIMER_MODE_OFF)
		{
			get_local_time(&dt);
			if (api_compare_day_time_ext(&wkdt, &dt) < 0)
				return  - 7;
		}
	}
	else
	{
		if (timer->timer_mode != TIMER_MODE_OFF)
		{
			/*
			if(timer->wakeup_state == TIMER_STATE_RUNING)
			{
			}
			 */
		}
	}


	if (timer->timer_mode != TIMER_MODE_OFF)
	{

		timer->wakeup_year = wkdt.year;
		timer->wakeup_month = wkdt.month;
		timer->wakeup_day = wkdt.day;
		timer->wakeup_time = wkdt.hour * 3600+wkdt.min * 60+wkdt.sec;
	}


	edf = &timer_edf_duration;
	OSD_GetEditFieldIntValue(edf, &val);
	duration = val/10000*3600 + (val/100%100)*60 + val%100;

	if (timer->timer_service == TIMER_SERVICE_MESSAGE)
		timer->wakeup_duration_time = 0;
	else
	{
		if (val/100%100 > 59)
			return  - 6;
		timer->wakeup_duration_time = duration;
	}

	ret = IsValidTimer(timer);
	if (ret != 0)
		return  - 8;

	return 0;
}

BOOL win_timerset_open(TIMER_SET_CONTENT *settimer, TIMER_SET_CONTENT *rettimer, BOOL check_starttime)
{
	POBJECT_HEAD pObj = (POBJECT_HEAD) &g_win_timerset;
	PRESULT bResult = PROC_LOOP;
	UINT32 hkey;
	UINT8 cur_mode, mode;
	UINT8 group_idx;
	date_time dt, dt1;

	sys_data_check_channel_groups();
	cur_mode = sys_data_get_cur_chan_mode();
	group_idx = sys_data_get_cur_group_index();


	timer_set_ok_cancel_flag = 0;

	MEMSET(&timer_set, 0, sizeof(TIMER_SET_CONTENT));

	if (settimer != NULL)
		MEMCPY(&timer_set, settimer, sizeof(TIMER_SET_CONTENT));

	if ( timer_set.timer_mode != TIMER_MODE_OFF
	        && timer_set.timer_service != TIMER_SERVICE_MESSAGE )
	{
		if (timer_set.wakeup_chan_mode > 1)
			timer_set.wakeup_chan_mode = 1;

		mode = timer_set.wakeup_chan_mode;
	}
	else
	{
		mode = cur_mode;
		timer_set.wakeup_chan_mode = cur_mode;
	}
	if (timer_set.timer_mode == TIMER_MODE_OFF)
	{
		timer_set_modify_add_flag = TIMER_ADD;
		timer_set_check_starttime_flag = 1;
	}
	else
	{
		timer_set_modify_add_flag = TIMER_MODIFY;
		timer_set_check_starttime_flag = 1;
	}

	get_local_time(&dt1);

	dt1.min = dt1.min / 10 * 10;
	dt1.sec = 0;
	convert_time_by_offset(&dt, &dt1, 0, 10);

	if (timer_set_modify_add_flag == TIMER_ADD)
	{
		timer_set.wakeup_year = dt.year;
		timer_set.wakeup_month = dt.month;
		timer_set.wakeup_day = dt.day;
		timer_set.wakeup_time = dt.hour * 360+dt.min * 60+dt.sec;
		timer_set.wakeup_state = TIMER_STATE_READY;
	}
	if (timer_set.timer_service == TIMER_SERVICE_MESSAGE)
		timer_set.wakeup_duration_time = 0;


	sys_data_set_cur_chan_mode(mode);
	sys_data_change_group(0);

	OSD_SetContainerFocus(&g_win_timerset, TIMER_MODE_ID);
	OSD_ObjOpen(pObj, MENU_OPEN_TYPE_OTHER);

	while (bResult != PROC_LEAVE)
	{

		hkey = ap_get_key_msg();
		if (hkey == INVALID_HK || hkey == INVALID_MSG)
		{
			win_timerset_set_currenttime(TRUE);
			continue;
		}

		bResult = OSD_ObjProc(pObj, (MSG_TYPE_KEY << 16), hkey, 0);
	}

	if (timer_set_ok_cancel_flag)
		MEMCPY(rettimer, &timer_set, sizeof(TIMER_SET_CONTENT));

	sys_data_set_cur_chan_mode(cur_mode);
	sys_data_change_group(group_idx);

	return timer_set_ok_cancel_flag;

}

/*UINT8 find_available_timer()
{
	UINT8 i;
	SYSTEM_DATA *sys_data;

	sys_data = sys_data_get();
	for (i = 0; i < MAX_TIMER_NUM; i++)
		if (sys_data->timer_set.TimerContent[i].timer_mode == TIMER_MODE_OFF)
			return i + 1;
	return 0;
}*/

/*************************************
function: IsValidTimer
parameter:
return:
0	- invalid
1   	- valid
 **************************************/
/*INT32 IsValidTimer(TIMER_SET_CONTENT *timer)
{
	INT32 ret, i;
	SYSTEM_DATA *sys_data;
	TIMER_SET_CONTENT *timer1,  *timer2;
	date_time dt, dt2;

	if (timer->timer_mode == TIMER_MODE_OFF)
		return 1;

	MEMSET(&dt, 0, sizeof(date_time));
	MEMSET(&dt2, 0, sizeof(date_time));

	dt.year = timer->wakeup_year;
	dt.month = timer->wakeup_month;
	dt.day = timer->wakeup_day;
	dt.hour = timer->wakeup_time / 3600;
	dt.min = timer->wakeup_time / 60 % 60;
	dt.sec = timer->wakeup_time % 60;

	sys_data = sys_data_get();

	for (i = 0; i < MAX_TIMER_NUM; i++)
	{
		if ( i == sys_data->timer_set.timer_num - 1 ) continue;

		if ( sys_data->timer_set.TimerContent[i].timer_mode == TIMER_MODE_OFF ) continue;

		switch (timer->timer_mode)
		{
			case TIMER_MODE_WEEKLY:
				switch (sys_data->timer_set.TimerContent[i].timer_mode)
				{
				case TIMER_MODE_MONTHLY:
				case TIMER_MODE_YEARLY:
					break;
				default:
					goto CHECKTIMER;
				}
				break;
			case TIMER_MODE_MONTHLY:
			case TIMER_MODE_YEARLY:
				switch (sys_data->timer_set.TimerContent[i].timer_mode)
				{
				case TIMER_MODE_WEEKLY:
					break;
				default:
					goto CHECKTIMER;
				}
				break;
			default:
CHECKTIMER:
				timer2 = &sys_data->timer_set.TimerContent[i];

				dt2.year = timer2->wakeup_year;
				dt2.month = timer2->wakeup_month;
				dt2.day = timer2->wakeup_day;
				dt2.hour = timer2->wakeup_time / 3600;
				dt2.min = timer2->wakeup_time / 60 % 60;
				dt2.sec = timer2->wakeup_time % 60;

				ret = api_compare_day_time_ext(&dt, &dt2);
				if (ret == 0)
					return 0;
				ret = api_compare_timer(timer, &sys_data->timer_set.TimerContent[i]);

				if ( ret <= 0 ) return 0;

				// timer1: [a,b]; timer2: [c,d]; b<c
				if (ret == 1)
				{
					timer1 = timer;
					timer2 = &sys_data->timer_set.TimerContent[i];
				}
				else
				{
					timer1 = &sys_data->timer_set.TimerContent[i];
					timer2 = timer;
				}

				if (api_check_timer(timer1, timer2) == 0)
					return 0;

				break;
		}


	}

	return 1;
}*/




/*******************************************************************************
*	WINDOW's objects declaration
*******************************************************************************/
extern void show_pop_up_info(UINT16 ID);

/*******************************************************************************
*	Local vriable & function declare
*******************************************************************************/
UINT8 find_available_timer()
{
    UINT8 i;
    SYSTEM_DATA* sys_data;

    sys_data = sys_data_get();
	resort_timer(TIMER_TYPE_EPG,&sys_data->timer_set.TimerContent[0]);
    for(i = 0; i < MAX_TIMER_NUM; i++)
        if(sys_data->timer_set.TimerContent[i].timer_mode == TIMER_MODE_OFF)
            return i+1;
    return 0;
}

/*************************************
function: find_suited_timer
parameter: 
return:
	0	- not find a suited timer
	1-MAX_TIMER_NUM   	- suited timer idx
**************************************/
INT32 find_suited_timer(UINT32 prog_id, date_time* start_dt, date_time* end_dt)
{
	SYSTEM_DATA* sys_data;
	TIMER_SET_CONTENT *timer;
	INT32 duration;
	INT32 i;

	sys_data = sys_data_get();
	for(i=0; i<MAX_TIMER_NUM; i++)
	{
		timer = &sys_data->timer_set.TimerContent[i];
		if(timer->timer_mode == TIMER_MODE_OFF)
			continue;

		if(timer->wakeup_channel!=prog_id)
			continue;
		
		if((timer->wakeup_year!=start_dt->year)
			||(timer->wakeup_month!=start_dt->month)
			||(timer->wakeup_day!=start_dt->day)
			||(timer->wakeup_time!=(UINT32)(start_dt->hour*3600+start_dt->min*60+start_dt->sec)))
			continue;

		api_get_time_len(start_dt,end_dt,&duration);
		if(timer->wakeup_duration_time==duration)
			return (i+1);
	}

	return 0;
}

/*************************************
function: IsValidTimer
parameter: 
return:
	0	- valid
	1-MAX_TIMER_NUM   	- conflict timer idx
**************************************/
INT32 IsValidTimer(TIMER_SET_CONTENT *timer)
{
	INT32 ret,i;
	SYSTEM_DATA *sys_data;
	TIMER_SET_CONTENT *timer1,*timer2;
	date_time time1,time2;
	INT32 time_span =0;

	if(timer->timer_mode == TIMER_MODE_OFF)
		return 0;

	sys_data = sys_data_get();
	
	for(i=0; i<MAX_TIMER_NUM; i++)
	{
		//if(i == sys_data->timer_set.timer_num-1) continue;

		if(sys_data->timer_set.TimerContent[i].timer_mode == TIMER_MODE_OFF) continue;

		switch(timer->timer_mode)
		{
			case TIMER_MODE_WEEKLY:
				switch(sys_data->timer_set.TimerContent[i].timer_mode)
				{
					case TIMER_MODE_MONTHLY:
					case TIMER_MODE_YEARLY:
						break;
					default:
						goto CHECKTIMER;
				}
				break;
			case TIMER_MODE_MONTHLY:
			case TIMER_MODE_YEARLY:
				switch(sys_data->timer_set.TimerContent[i].timer_mode)
				{
					case TIMER_MODE_WEEKLY:
						break;
					default:
						goto CHECKTIMER;
				}
				break;
			default:
CHECKTIMER:
#if 0	// check duration	
				ret = api_compare_timer(timer,&sys_data->timer_set.TimerContent[i]);

				if(ret <=0) return (i+1);

				// timer1: [a,b]; timer2: [c,d]; b<c
				if(ret == 1) 
				{
					timer1 = timer;
					timer2 = &sys_data->timer_set.TimerContent[i];
				}
				else
				{
					timer1 = &sys_data->timer_set.TimerContent[i];
					timer2 = timer;
				}
				
				if(api_check_timer(timer1,timer2)==0)
					return (i+1);
#else 	// only check start time
				timer1 = timer;
				timer2 = &sys_data->timer_set.TimerContent[i];
				
				MEMSET(&time1, 0, sizeof(date_time));
				MEMSET(&time2, 0, sizeof(date_time));

				time1.year  = timer1->wakeup_year;
				time1.month = timer1->wakeup_month;
				time1.day = timer1->wakeup_day;
				time1.hour = timer1->wakeup_time/3600;
				time1.min = timer1->wakeup_time/60%60;
				time1.sec = timer1->wakeup_time%60;

				time2.year  = timer2->wakeup_year;
				time2.month = timer2->wakeup_month;
				time2.day = timer2->wakeup_day;
				time2.hour = timer2->wakeup_time/3600;
				time2.min = timer2->wakeup_time/60%60;
				time2.sec = timer2->wakeup_time%60;

				/* two start time can not in one min */
				api_get_time_len(&time1,&time2,&time_span);
				if(time_span<0)
					time_span = -time_span;
				if(time_span <= 60)	return (i+1);
#endif
				break;
		}
		
		
	}
	
	return 0;
}


/*we insert the timer ahead of other timer if it has early wakeup time*/
#ifdef NVOD_FEATURE
INT32 insert_timer(TYPE_TIMER type,TIMER_SET_CONTENT  *t_insert, UINT32 pos)
{
	SYSTEM_DATA * sys_data = sys_data_get();
	TIMER_SET_CONTENT * tmp = NULL;
	UINT32 highpos =0;
	INT32 ret=0;
	
	date_time insert,other;
	
	insert.year = t_insert->wakeup_year;
	insert.month = t_insert->wakeup_month;
	insert.day = t_insert->wakeup_day;

	
	
	if(type == TIMER_TYPE_EPG)
	{
		tmp= &sys_data->timer_set.TimerContent[0];
		highpos = MAX_EPG_TIMER_NUM;
	}
	else if(type ==TIMER_TYPE_NVOD)
	{
		tmp= &sys_data->timer_set.TimerContent[32];
		highpos = MAX_NVOD_TIMER_NUM;
	}

	UINT32 idx = 0;
	
	if(pos>=highpos)
	{
		show_pop_up_info(RS_BOOK_FULL);
		return !SUCCESS;
	}

	for(idx =0; idx<pos;idx++)
	{
		other.year = tmp[idx].wakeup_year;
		other.month = tmp[idx].wakeup_month;
		other.day = tmp[idx].wakeup_day;
		
		ret = api_compare_day(&other,&insert);

		if(ret > 0)
			break;
		else if(ret ==0)
		{
			if(t_insert->wakeup_time < tmp[idx].wakeup_time)
				break;	
		}
	}
	
	if(idx==pos)
	{
		MEMCPY(&tmp[pos],t_insert,sizeof(TIMER_SET_CONTENT));
		return SUCCESS;
	}
	else
	{
		MEMMOVE(&tmp[idx+1],&tmp[idx],((UINT32)&tmp[pos]-(UINT32)&tmp[idx]));
		MEMCPY(&tmp[idx],t_insert,sizeof(TIMER_SET_CONTENT));
		return SUCCESS;
	}
}
#endif
void resort_timer(TYPE_TIMER type,TIMER_SET_CONTENT* base)
{
	UINT32 i =0;
	UINT32 nr_high =0;
	UINT32 jump_out=0;
	
	if(type==TIMER_TYPE_EPG)
		nr_high = MAX_EPG_TIMER_NUM;
	else
		nr_high = MAX_NVOD_TIMER_NUM;
	
	for(i=0;i<nr_high;)
	{
		if(base[i].timer_mode == TIMER_MODE_OFF)
		{
			if(i+jump_out>=nr_high-1)
				break;
			MEMMOVE(&base[i],&base[i+1],((UINT32)&base[nr_high]-(UINT32)&base[i+1]));
			jump_out++;
			MEMSET(&base[nr_high-jump_out],0,sizeof(TIMER_SET_CONTENT));
		}
		else
			i++;	
	}
}


INT32 find_timer_with_st(date_time * start_dt)
{
	SYSTEM_DATA* sys_data;
	TIMER_SET_CONTENT *timer;
	INT32 i;

	sys_data = sys_data_get();
	for(i=0; i<MAX_TIMER_NUM; i++)
	{
		timer = &sys_data->timer_set.TimerContent[i];
		if(timer->timer_mode == TIMER_MODE_OFF)
			continue;

		
		if((timer->wakeup_year!=start_dt->year)
			||(timer->wakeup_month!=start_dt->month)
			||(timer->wakeup_day!=start_dt->day)
			||(timer->wakeup_time!=(UINT32)(start_dt->hour*3600+start_dt->min*60+start_dt->sec)))
			continue;

		return i;
	}

	return -1;;

}


