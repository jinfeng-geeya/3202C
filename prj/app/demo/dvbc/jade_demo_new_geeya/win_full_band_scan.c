#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/nim/nim.h>

#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_as.h>

#include <hld/decv/decv.h>

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

#include "win_com_menu_define.h"

UINT16 autoscan_edit_str[10][30];

//include the header from xform 
#include "win_full_band_scan_vega.h"

/*******************************************************************************
 *	Objects definition
 *******************************************************************************/
extern CONTAINER g_win_fullband_scan;

extern CONTAINER autoscan_con;

extern CONTAINER autosrch_item1; /* 开始 频率*/
extern CONTAINER autosrch_item2; /* 结束 频率*/
extern CONTAINER autosrch_item3; /* 符号率*/
extern CONTAINER autosrch_item4; /* 调制方式 */
extern CONTAINER autosrch_item5; /* 开始搜索 */



extern TEXT_FIELD autosrch_txt1;
extern TEXT_FIELD autosrch_txt2;
extern TEXT_FIELD autosrch_txt3;
extern TEXT_FIELD autosrch_txt4;
extern TEXT_FIELD autosrch_txt5;


extern EDIT_FIELD autosrch_edt1;
extern EDIT_FIELD autosrch_edt2;
extern EDIT_FIELD autosrch_edt3;

extern MULTISEL autosrch_sel4;

/****************************************************************************************/
/*static VACTION autosrch_item_sel_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT autosrch_item_sel_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION autosrch_item_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT autosrch_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION autosrch_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT autosrch_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION autosrch_item_edit_keymap(POBJECT_HEAD pObj, UINT32 key);*/
static void auto_scan_save_val();
static UINT8 as_check_value();
inline UINT32 get_symbol_from_full_ban();
inline UINT32 get_qam_from_full_ban();

static void win_as_set_search_param(void);
void reset_search_param();

/****************************************************************************************/
/*#define WIN_IDX	  WSTL_COMMON_BACK_2
#define CON_IDX	        WSTL_WIN_2

#define CON_SH_IDX   WSTL_BUTTON_2
#define CON_HL_IDX   WSTL_BUTTON_1
#define CON_SL_IDX   WSTL_BUTTON_2
#define CON_GRY_IDX  WSTL_BUTTON_2

#define TXT_SH_IDX   WSTL_TXT_4
#define TXT_HL_IDX   WSTL_TXT_5
#define TXT_SL_IDX   WSTL_TXT_4
#define TXT_GRY_IDX  WSTL_TXT_4

#define EDIT_SH_IDX   WSTL_TXT_4
#define EDIT_HL_IDX   WSTL_BUTTON_3
#define EDIT_SL_IDX   WSTL_TXT_4
#define EDIT_GRY_IDX  WSTL_TXT_4

#define SEL_SH_IDX   WSTL_TXT_4
#define SEL_HL_IDX   WSTL_BUTTON_5
#define SEL_SL_IDX   WSTL_TXT_4
#define SEL_GRY_IDX  WSTL_TXT_4

#define CON_L 84
#define CON_T 142
#define CON_W 560
#define CON_H 30
#define CON_GAP 10

#define TXT_L_OF 0
#define TXT_T_OF ((CON_H-TXT_H)/2)
#define TXT_W 250
#define TXT_H 30

#define SEL_L_OF   (CON_W-SEL_W)
#define SEL_T_OF   TXT_T_OF
#define SEL_W      180
#define SEL_H      TXT_H

#define WIN_L     0
#define WIN_T     90
#define WIN_W     720
#define WIN_H     400

#define W_CON_L     	66
#define W_CON_T     	102
#define W_CON_W		    604
#define W_CON_H     	264*/

/****************************************************************************************/
/*#define LDEF_CON(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,hl,conobj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, CON_SH_IDX,hl,CON_SL_IDX,CON_GRY_IDX,   \
    autosrch_item_keymap,autosrch_item_callback,  \
    conobj, ID,1)

#define LDEF_TXT(root,varTxt,nxtObj,l,t,w,h,resID)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXT_SH_IDX,TXT_HL_IDX,TXT_SL_IDX,TXT_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 20,0,resID,NULL)

#define LDEF_MSEL(root, varNum, nxtObj, ID, l, t, w, h,style,cur,cnt,ptabl)	\
    DEF_MULTISEL(varNum,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, SEL_SH_IDX,SEL_HL_IDX,SEL_SL_IDX,SEL_GRY_IDX,   \
    autosrch_item_sel_keymap,autosrch_item_sel_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,ptabl,cur,cnt)

#define LDEF_EDIT(root, varNum, nxtObj, ID, IDl, IDr, IDu, IDd, l, t, w, h,style,cursormode,pat,pre,sub,str)	\
    DEF_EDITFIELD(varNum,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, EDIT_SH_IDX,EDIT_HL_IDX,EDIT_SL_IDX,EDIT_GRY_IDX,   \
    autosrch_item_edit_keymap,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,pat,10,cursormode,pre,sub,str)*/

/****************************************************************************************/

/*#define LDEF_MENU_ITEM_EDT(root,varCon,nxtObj,varTxt,varTxtset,ID,IDu,IDd,l,t,w,h,nameID,setstr,pat)	\
    LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,CON_HL_IDX,&varTxt,ID)	\
    LDEF_TXT(&varCon,varTxt,&varTxtset,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,nameID)	\
    LDEF_EDIT(&varCon,varTxtset,NULL   ,ID,ID,ID,ID,ID,l + SEL_L_OF, t + SEL_T_OF,SEL_W,SEL_H,NORMAL_EDIT_MODE, CURSOR_NORMAL, pat,NULL,NULL, setstr)


#define LDEF_MENU_ITEM_SEL(root,varCon,nxtObj,varTxt,varNum,ID,IDu,IDd,l,t,w,h,resID,style,cur,cnt,ptbl) \
    LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,CON_HL_IDX,&varTxt,ID)   \
    LDEF_TXT(&varCon,varTxt,&varNum,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,resID)    \
    LDEF_MSEL(&varCon,varNum,NULL   ,ID, l + SEL_L_OF ,t + SEL_T_OF,SEL_W,SEL_H,style,cur,cnt,ptbl)

#define LDEF_MENU_ITEM_TXT(root,varCon,nxtObj,varTxt,ID,IDu,IDd,l,t,w,h,resID) \
    LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,CON_HL_IDX,&varTxt,ID)   \
    LDEF_TXT(&varCon,varTxt,NULL,l + TXT_L_OF,t + TXT_T_OF,TXT_W,TXT_H,resID)

#define LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_IDX,WIN_IDX,WIN_IDX,WIN_IDX,   \
    autosrch_keymap,autosrch_callback,  \
    nxtObj, focusID,0)


#define WIN g_win_fullband_scan*/

#define DVBC_AUTOSCAN_SFREQ  0
#define DVBC_AUTOSCAN_EFREQ    1
#define DVBC_AUTOSCAN_SYMBOL  2
#define DVBC_AUTOSCAN_EDIT_LEN				0x06 // add .xx


#define FREQ_START_ID		1
#define FREQ_END_ID			2
#define SYM_ID				3
#define MODE_ID				4
#define START_ID			5

/********************************************************************************************************************/

char win_autosrch_pat_freq[] = "f32"; //integer width is 5,  fraction width is 2
char win_autosrch_pat_sym[] = "s5";


UINT16 scan_model_type_ids[] =
{
    RS_CONSTELLATION_16,
    RS_CONSTELLATION_32,
    RS_CONSTELLATION_64,
    RS_CONSTELLATION_128,
    RS_CONSTELLATION_256,

};

/********************************************************************************************************************/
/*LDEF_MENU_ITEM_EDT(WIN, autosrch_item1, &autosrch_item2, autosrch_txt1, autosrch_edt1, 1, 5, 2,  \
	CON_L, CON_T + (CON_H + CON_GAP) *0, CON_W, CON_H, RS_INSTALLATION_SFREQ, autoscan_edit_str[0], win_autosrch_pat_freq)

LDEF_MENU_ITEM_EDT(WIN, autosrch_item2, &autosrch_item3, autosrch_txt2, autosrch_edt2, 2, 1, 3,  \
	CON_L, CON_T + (CON_H + CON_GAP) *1, CON_W, CON_H, RS_INSTALLATION_EFREQ, autoscan_edit_str[1], win_autosrch_pat_freq)

LDEF_MENU_ITEM_EDT(WIN, autosrch_item3, &autosrch_item4, autosrch_txt3, autosrch_edt3, 3, 2, 4,  \
	CON_L, CON_T + (CON_H + CON_GAP) *2, CON_W, CON_H, RS_INFO_SYMBOL, autoscan_edit_str[2], win_autosrch_pat_sym)

LDEF_MENU_ITEM_SEL(WIN, autosrch_item4, &autosrch_item5, autosrch_txt4, autosrch_sel4, 4, 3, 5,  \
	CON_L, CON_T + (CON_H + CON_GAP) *3, CON_W, CON_H, RS_INSTALLATION_SCAN_MODE, STRING_ID, 0, 5, scan_model_type_ids)

LDEF_MENU_ITEM_TXT(WIN, autosrch_item5, NULL, autosrch_txt5, 5, 4, 1,  \
	CON_L, CON_T + (CON_H + CON_GAP) *4, CON_W, CON_H, RS_COMMON_SEARCH)

DEF_CONTAINER(autoscan_con, NULL, &autosrch_item1, C_ATTR_ACTIVE, 0,  \
	0, 0, 0, 0, 0, W_CON_L, W_CON_T, W_CON_W, W_CON_H, CON_IDX, CON_IDX, CON_IDX, CON_IDX,  \
	NULL, NULL,  \
	NULL, 1, 0)

LDEF_WIN(WIN, &autoscan_con, WIN_L, WIN_T, WIN_W, WIN_H, 1)*/

static UINT32 as_sfreq = 0x2B5C;//111.00   //0x12D9; // 48.25
static UINT32 as_efreq = 0x150B8; // 862.00
static UINT32 as_symbol = 6875;
static UINT32 as_constellation = QAM64;

/*******************************************************************************
 *	local function definition
 *******************************************************************************/

static INT32 check_freq_value(UINT16 *str)
{
	INT32 ret = 0;
	UINT32 val;
	UINT8 dot_pos;

	/* FREQ */
	wincom_mbs_to_i_with_dot(str, &val, &dot_pos);
	if (val < 4825 || val > 86200)
		ret =  - 1;
	else
		ret = val;

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
	else
		ret = val;

	return ret;
}

static void dvbc_autoscan_set_def_val(void)
{
	INT8 rule[20];

	P_NODE p_node;
	T_NODE t_node;

	/* SFREQ */
	wincom_i_to_mbs_with_dot(autoscan_edit_str[DVBC_AUTOSCAN_SFREQ],
	                         as_sfreq, DVBC_AUTOSCAN_EDIT_LEN - 1/*without dot*/, 0x02/*xxx.xx*/);

	/* EFREQ */
	wincom_i_to_mbs_with_dot(autoscan_edit_str[DVBC_AUTOSCAN_EFREQ],
	                         as_efreq, DVBC_AUTOSCAN_EDIT_LEN - 1/*without dot*/, 0x02/*xxx.xx*/);

	/* SYMBOL */
	wincom_i_to_mbs(autoscan_edit_str[DVBC_AUTOSCAN_SYMBOL],
	                as_symbol, DVBC_AUTOSCAN_EDIT_LEN - 1);

	OSD_SetMultiselSel(&autosrch_sel4, as_constellation - QAM16);
}

static void auto_scan_save_val()
{
	UINT8 dot_pos;
	UINT32 s_freq, e_freq, symbol, constellation;

	//star freq
	wincom_mbs_to_i_with_dot(autoscan_edit_str[0], &s_freq, &dot_pos);
	//end freq
	wincom_mbs_to_i_with_dot(autoscan_edit_str[1], &e_freq, &dot_pos);
	// SYMBOL
	symbol = wincom_mbs_to_i(autoscan_edit_str[2]);
	// scan mode
	constellation = QAM16 + OSD_GetMultiselSel(&autosrch_sel4);
	//change channel

	if ((as_sfreq != s_freq) || (as_efreq != e_freq) || (as_symbol != symbol) || (as_constellation != constellation))
	{
		as_sfreq = s_freq;
		as_efreq = e_freq;
		as_symbol = symbol;
		as_constellation = constellation;
	}


}

static PRESULT full_band_message_proc(UINT32 msg_type, UINT32 msg_code)
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

/*******************************************************************************
 *	key mapping and event callback definition
 *******************************************************************************/
#define VACT_POP_UP	(VACT_PASS + 1)

static VACTION autosrch_item_edit_keymap(POBJECT_HEAD pObj, UINT32 key)
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


static VACTION autosrch_item_sel_keymap(POBJECT_HEAD pObj, UINT32 key)
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
		case V_KEY_ENTER:
			act = VACT_POP_UP;
			break;
		default:
			act = VACT_PASS;
	}

	return act;

}


static PRESULT autosrch_item_sel_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{

	PRESULT ret = PROC_PASS;

	return ret;


}


static VACTION autosrch_item_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act;
	switch (key)
	{
		case V_KEY_ENTER:
		case V_KEY_RIGHT:
			act = VACT_ENTER;
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}

extern void SetSearchTitleID(UINT16 ID);
static PRESULT autosrch_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{

	PRESULT ret = PROC_PASS;
	UINT8 bID;
	VACTION unact;
	POBJECT_HEAD submenu;
	UINT8 back_saved;

	bID = OSD_GetObjID(pObj);

	switch (event)
	{
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			if ((unact == VACT_ENTER) && (bID == START_ID))
			{
				auto_scan_save_val();
				if (as_check_value() == 0)
				//input invalid
					break;
				//SetSearchTitleID(RS_FULL_BAND_SEARCH);
				//win_as_set_search_param();
				win_set_search_param(2,0,0,0,0);
				//SetSearchParamMode(2);
				submenu = (POBJECT_HEAD) &g_win_search;
				if (OSD_ObjOpen(submenu, 0xFFFFFFFF) != PROC_LEAVE)
					menu_stack_push(submenu);
				ret = PROC_LOOP;
			}
         
			break;
	}

	return ret;

}
static BOOL AutoFullbandScan=FALSE;
BOOL getAutoFullbandScan()
{
	return AutoFullbandScan;
}
void setAutoFullbandScan(BOOL input)
{
	AutoFullbandScan=input;
}
////////////////////////////////////
//fullbandsearch   0, autosearch ;   1, fullbandsearch
//needcheck   0, no need check auto start  ;   1, need check if start search
////////////////////////
void nochannel_startScan(UINT8 fullbandsearch,UINT8 needcheck)
{
		POBJECT_HEAD submenu;
        UINT8 back_saved;
        win_popup_choice_t popup_ret;
        if(needcheck)
        {
			win_compopup_init(WIN_POPUP_TYPE_OKNO);
			win_compopup_set_msg(NULL, NULL, RS_NEED_SEARCHING);
			win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
			popup_ret = win_compopup_open_timer(5000,&back_saved);
			if (popup_ret == WIN_POP_CHOICE_NO)
			{
				return;
			}
        }
        if(fullbandsearch)
        {
    		dvbc_autoscan_set_def_val(); 
    		auto_scan_save_val();
    		if (as_check_value() == 0)
    			return;
    		win_set_search_param(2,0,0,0,0);
        }else
        {
            win_set_search_param(0, 0, 0, 0, 1);
        }
		setAutoFullbandScan(TRUE);
		submenu = (POBJECT_HEAD) &g_win_search;
		if (OSD_ObjOpen(submenu, 0xFFFFFFFF) != PROC_LEAVE)
			menu_stack_push(submenu);

}

static VACTION autosrch_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	UINT32 ret1, ret2, ret3;
	UINT8 save;
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_UP:
			act = VACT_CURSOR_UP;
			break;
		case V_KEY_DOWN:
			act = VACT_CURSOR_DOWN;
			break;
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		case V_KEY_EXIT:
			auto_scan_save_val();
			if(get_last_searched_prog()!=-1)
			{
				api_play_channel(get_last_searched_prog(),TRUE,FALSE,FALSE);
				set_last_searched_prog(-1);
			}
			BackToFullScrPlay();
			system_state = SYS_STATE_NORMAL;
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}

struct help_item_resource as_help[] =
{
    {1,IM_HELP_ICON_LR,RS_CHANGTYPE},
    {1,IM_HELP_ICON_TB,RS_SELECT_EDIT_ITEM},
    {0,RS_MENU,RS_HELP_BACK},
    {0,RS_HELP_EXIT,RS_HELP_EXIT},  
};

static PRESULT autosrch_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	MULTISEL *msel;
	UINT16 title_id;
	OSD_RECT rect;
	POBJECT_HEAD lastitem;

	switch (event)
	{
		case EVN_PRE_OPEN:
			wincom_open_title_ext(RS_FULL_BAND_SEARCH, IM_TITLE_ICON_SYSTEM);
			wincom_open_help(as_help, 4);

			dvbc_autoscan_set_def_val(); //Ryan
			system_state = SYS_STATE_SEARCH_PROG;
			break;
		case EVN_POST_OPEN:
			break;
		case EVN_PRE_CLOSE:
			auto_scan_save_val();
			if(get_last_searched_prog()!=-1)
			{
				api_play_channel(get_last_searched_prog(),TRUE,FALSE,FALSE);
				set_last_searched_prog(-1);
			}
			/* Make OSD not flickering */
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;			
			break;
		case EVN_POST_CLOSE:
			system_state = SYS_STATE_NORMAL;
			break;
		case EVN_MSG_GOT:
			ret = full_band_message_proc(param1, param2);
			break;

	}

	return ret;
}


void get_search_param(UINT32 *freq_s,UINT32 *freq_e,UINT8 *dot_pos,UINT32 *symbol,UINT32 *constellation)
{   
    wincom_mbs_to_i_with_dot(autoscan_edit_str[0], freq_s, dot_pos);
	wincom_mbs_to_i_with_dot(autoscan_edit_str[1], freq_e, dot_pos);
	// SYMBOL
	*symbol = wincom_mbs_to_i(autoscan_edit_str[2]);
	// scan mode
	*constellation = QAM16 + OSD_GetMultiselSel(&autosrch_sel4);
}
/*static void win_as_set_search_param(void)
{
	UINT32 search_mode;
	UINT32 prog_type;
	S_NODE s_node;
	struct as_service_param param;
	struct vdec_device *vdec;
	struct vdec_io_get_frm_para_advance vfrm_param;
	UINT32 addr, len;
	T_NODE t_node;

	UINT8 dot_pos;
	UINT32 freq_s, freq_e, symbol, constellation;


	MEMSET(&param, 0, sizeof(struct as_service_param));
	MEMSET(&s_node, 0, sizeof(S_NODE));
	MEMSET(&t_node, 0, sizeof(T_NODE));
	MEMSET(&vfrm_param, 0, sizeof(struct vdec_io_get_frm_para_advance));

	api_stop_play(1);
	osal_task_sleep(100);

	//get the parameters for the search
	//freq
	wincom_mbs_to_i_with_dot(autoscan_edit_str[0], &freq_s, &dot_pos);
	wincom_mbs_to_i_with_dot(autoscan_edit_str[1], &freq_e, &dot_pos);
	// SYMBOL
	symbol = wincom_mbs_to_i(autoscan_edit_str[2]);
	// scan mode
	constellation = QAM16 + OSD_GetMultiselSel(&autosrch_sel4);

	//add the TP node
	//recreate_sat_view(VIEW_ALL, 0);
	//add_node(TYPE_SAT_NODE, 0, &s_node);
	s_node.sat_id = 1;

	//tv/radio/all
	prog_type = P_SEARCH_TV | P_SEARCH_RADIO | P_SEARCH_DATA;
	search_mode = P_SEARCH_FTA | P_SEARCH_SCRAMBLED;

	param.as_prog_attr = prog_type | search_mode; 


	param.as_from = freq_s;
	param.as_to = freq_e;
	param.as_method = AS_METHOD_FFT;


	param.as_frontend_type = FRONTEND_TYPE_C;
	param.as_sat_cnt = 1;
	param.sat_ids[0] = s_node.sat_id;
	param.ft.c_param.sym = symbol;
	param.ft.c_param.constellation = constellation;

	param.as_p_add_cfg = PROG_ADD_REPLACE_OLD;
	param.as_handler = NULL;

	// set param
	win_search_set_param(&param);

	vdec = (struct vdec_device*)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
	vfrm_param.ufrm_mode = VDEC_UN_DISPLAY;
	vfrm_param.request_frm_number = 1;
	vdec_io_control(vdec, VDEC_IO_GET_FRM_ADVANCE, (UINT32) &vfrm_param);
	if (vfrm_param.return_frm_number > 0)
	{
		addr = vfrm_param.tFrmInfo[0].uC_Addr;
		len = 720 * 576 * 3 / 2;
		db_search_init((UINT8*)addr, len);
	}
	else
		ASSERT(0);
}
*/

//1 valid 0 invalid
static UINT8 as_check_value()
{
	INT32 ret1, ret2, ret3;
	UINT8 save;

	ret1 = check_freq_value(autoscan_edit_str[0]);
	ret2 = check_freq_value(autoscan_edit_str[1]);
	ret3 = check_symbol_value(autoscan_edit_str[2]);
	if (ret1 ==  - 1 || ret2 ==  - 1 || ret3 ==  - 1 || (ret1 > ret2))
	{
		win_compopup_init(WIN_POPUP_TYPE_OK);
		win_compopup_set_msg(NULL, NULL, RS_MSG_INVALID_INPUT_CONTINUE);
		win_compopup_set_frame(150, 150, 300, 100);
		win_compopup_open_ext(&save);
		return 0;
	}
	else
		return 1;
}

inline UINT32 get_symbol_from_full_ban()
{
	return as_symbol;
}

inline UINT32 get_qam_from_full_ban()
{
	return as_constellation;
}

void reset_search_param()
{
	as_sfreq = 0x2B5C;//111,0x12D9; // 48.25
	as_efreq = 0x150B8; // 862.00
	as_symbol = 6875;
	as_constellation = QAM64;
}
