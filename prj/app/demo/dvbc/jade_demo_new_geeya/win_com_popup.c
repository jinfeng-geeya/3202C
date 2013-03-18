#include <sys_config.h>
#include <types.h>
#include <api/libc/printf.h>
#include <osal/osal.h>
//#include <api/libosd/osd_lib.h>
#include <math.h>
//#include <api/libosd/osd_vkey.h>

#include "osdobjs_def.h"

#include "osd_config.h"
#include "string.id"
#include "images.id"

#include "key.h"
#include "control.h"

#include "win_com.h"
#include "win_com_popup.h"

#define ABEL_PRINTF(...)

#define POP_PRINTF PRINTF
#define POPUP_MAX_TITLE_LEN	30
#if (defined(MULTI_CAS)&&(CAS_TYPE == CAS_ABEL))
#define POPUP_MAX_MSG_LEN	512
#else
#define POPUP_MAX_MSG_LEN	200
#endif

/*******************************************************************************
 *	WINDOW's objects declaration
 *******************************************************************************/
extern CONTAINER g_win_popup;
extern TEXT_FIELD win_popup_title_txt;
extern MULTI_TEXT win_popup_msg_mbx;
extern TEXT_FIELD win_popup_yes;
extern TEXT_FIELD win_popup_no;
extern TEXT_FIELD win_popup_cancel;

UINT16 msg_title[POPUP_MAX_MSG_LEN];
UINT16 msg_str[POPUP_MAX_MSG_LEN];
TEXT_CONTENT popup_mtxt_content =
{
	STRING_ID, 0
};

/*
TEXT_CONTENT popup_mtxt_content[] =
{
{STRING_UNICODE, msg_str},
};
 */

static VACTION popup_btn_text_keymap(POBJECT_HEAD pObj, UINT32 vkey);
static PRESULT popup_btn_text_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION popup_con_keymap(POBJECT_HEAD pObj, UINT32 vkey);
static PRESULT popup_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
static UINT8 pop_win_sel = 0; //0, in the style of WIN_POPUP_TYPE_SMSG
							  //1, in the style of WIN_POPUP_TYPE_OK
							  //2, DVN CA use the new OSD displaying content to replace the old one
							  
UINT8 win_compopup_get_status()
{
	return pop_win_sel;
}

void win_compopup_set_status(UINT8 flag)
{
	pop_win_sel = flag;
}


/*******************************************************************************
 *	WINDOW's objects defintion MACRO
 *******************************************************************************/

/////////style
#define POPUPWIN_IDX	WSTL_MY_HELP_ALL///WSTL_POP_WIN_1  WSTL_MY_HELP_ALL
#define POPUPSH_IDX	WSTL_BULE_ITEM
#define POPUPHL_IDX	WSTL_YELLO_ITEM///WSTL_YELLO_ITEM
#define POPUP_TITLE_SH_IDX	WSTL_TXT_6
#define POPUPSTR_IDX  	WSTL_TXT_6

#define LDEF_TXT_TITLE(root,varTxt,nxtObj,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, POPUP_TITLE_SH_IDX, POPUP_TITLE_SH_IDX, 0,0,   \
    NULL, NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,resID,str)

#define LDEF_MULTITEXT(root,varMsg,nxtObj,l,t,w,h)  \
    DEF_MULTITEXT(varMsg,root,nxtObj,C_ATTR_ACTIVE,0,   \
        0,0,0,0,0, l,t,w,h,POPUPSTR_IDX,POPUPSTR_IDX,POPUPSTR_IDX,POPUPSTR_IDX,  \
        NULL,NULL,  \
        C_ALIGN_CENTER | C_ALIGN_VCENTER,1, 0,0,w,h, NULL,&popup_mtxt_content)

#define LDEF_TXT_BTM(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, POPUPSH_IDX, POPUPHL_IDX, 0,0,   \
    popup_btn_text_keymap, popup_btn_text_callback,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,resID,str)

#define LDEF_CONTAINER(varCon,nxtObj,l,t,w,h,focusID)   \
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, POPUPWIN_IDX,POPUPWIN_IDX,0,0,   \
    popup_con_keymap,popup_con_callback,  \
    nxtObj, focusID,0)


#define W_L	(TV_OFFSET_L +(OSD_MAX_WIDTH-W_W)/2)
#define W_T	(TV_OFFSET_T+(OSD_MAX_HEIGHT-W_H)/2)
#define W_W 350
#define W_H 150

#define W_MSG_L 184
#define W_MSG_T 152
#define W_MSG_W 320
#define W_MSG_H 120

#define W_CHS_L 150
#define W_CHS_T 155
#define W_CHS_W 350
#define W_CHS_H 150




#define MSG_L (W_L + 5)
#define MSG_T (W_T + 30)
#define MSG_W (W_W - 10)
#define MSG_H	60



#define BTM_L (W_L +  W_W - (BTM_W + BTM_GAP) * 3)
#define BTM_T  (W_T + W_H - BTM_H*2)
#define BTM_W 70
#define BTM_H 30
#define BTM_GAP 20


#define POPUP_YES_ID    1
#define POPUP_NO_ID     2
#define POPUP_CANCEL_ID 3



/*******************************************************************************
 *	WINDOW's objects defintion
 *******************************************************************************/

LDEF_TXT_TITLE ( &g_win_popup, win_popup_title_txt, &win_popup_msg_mbx, W_L + 10, W_T, W_W + 10, 26, 0, msg_title )
LDEF_MULTITEXT ( &g_win_popup, win_popup_msg_mbx, &win_popup_yes, MSG_L, MSG_T, MSG_W, MSG_H )
LDEF_TXT_BTM ( &g_win_popup, win_popup_yes, &win_popup_no, 	1, 3, 2, 1, 1, BTM_L, BTM_T, BTM_W, BTM_H,  RS_COMMON_YES, NULL )
LDEF_TXT_BTM ( &g_win_popup, win_popup_no, &win_popup_cancel, 2, 1, 3, 2, 2, BTM_L + ( BTM_W + BTM_GAP ) *1, BTM_T, BTM_W, BTM_H,  RS_COMMON_NO, NULL )
LDEF_TXT_BTM ( &g_win_popup, win_popup_cancel, NULL, 			3, 2, 1, 3, 3, BTM_L + ( BTM_W + BTM_GAP ) *2, BTM_T, BTM_W, BTM_H,  RS_COMMON_CANCEL, NULL )

LDEF_CONTAINER(g_win_popup, &win_popup_title_txt, W_L, W_T, W_W, W_H, 1)


/*******************************************************************************
 *	Local Variable & Function declare
 *******************************************************************************/
static win_popup_type_t win_popup_type = WIN_POPUP_TYPE_SMSG;
static win_popup_choice_t win_popup_choice = WIN_POP_CHOICE_NULL;
static UINT8 save_back_ok;

#define MAX_LEN_ONE_LINE    350
#define MAX_LEN_CONAX_ONE_LINE    450
#define MIN_WIDTH_2BTN      250
#define MIN_WIDHT_1ORNOBTN   200

#define MSG_VOFFSET_NOBTN      24
#define MSG_VOFFSET_WITHBTN      16
#define MSG_LOFFSET             10
#define MSG_BUTTON_TOPOFFSET     16
#define MSG_BUTTON_BOTOFFSET     20//24

#define DEFAULT_FONT_H      24
#define TITLE_H					26

/********************************************************************
*   ABEL CA MMI API
*********************************************************************/
#if(defined(MULTI_CAS)&&(CAS_TYPE == CAS_ABEL))	
#include "menus_root.h"
#include <api/libcas/abel/abel.h>
#include <api/libcas/abel/abel_mmi.h>

void win_compopup_set_enter_wrong_pin_mmi_status(BOOL value);

extern TEXT_FIELD win_popup_item_name;
extern TEXT_FIELD win_popup_item_val;
UINT16 msg_popup_item_name[POPUP_MAX_MSG_LEN];
UINT16 msg_popup_item_val[POPUP_MAX_MSG_LEN];

#define ITEM_TXT_L	W_L + MSG_LOFFSET
#define ITEM_TXT_T	BTM_T-BTM_H
#define ITEM_TXT_W	(W_W - MSG_LOFFSET * 2)/2
#define ITEM_TXT_H 	TITLE_H

#define ITEM_VAL_L	ITEM_TXT_L+ITEM_TXT_W
#define ITEM_VAL_T 	(W_T + W_H - BTM_H*2)
#define ITEM_VAL_W	ITEM_TXT_W
#define ITEM_VAL_H	ITEM_TXT_H

#define LDEF_TXT_ITEM(root,varTxt,nxtObj,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WSTL_TXT_WIN_POP, WSTL_TXT_WIN_POP, 0,0,   \
    NULL, NULL,  \
     C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,resID,str)

LDEF_TXT_ITEM ( &g_win_popup, win_popup_item_name, &win_popup_yes, ITEM_TXT_L, ITEM_TXT_T, ITEM_TXT_W, ITEM_TXT_H, 0, NULL )
LDEF_TXT_ITEM ( &g_win_popup, win_popup_item_val, &win_popup_yes, ITEM_VAL_L, ITEM_VAL_T, ITEM_VAL_W, ITEM_VAL_H, 0, NULL )


#define VACT_TVPIN_DEL_ONE_CHAR (VACT_PASS+1)

UINT8			g_input_tvpin_pos=0;
UINT8			g_input_tvpin_chars[8];
UINT8			g_target_tvpin_for_confirm_ui[8];
UINT8			g_target_parental_control_level_for_confirm_ui;
UINT8			g_tvpin_display[16];
BOOL			g_tvpin_confirm_mode=FALSE;
BOOL			g_tvpin_confirm_proc_exec_result=FALSE;
UINT16 			val_Xoffset = 0;
BOOL                    g_enter_wrong_pin_mmi = FALSE;

void _update_popup_item_tvpin_display(void)
{

	switch(g_input_tvpin_pos)
	{
		case 0:
			sprintf(g_tvpin_display,"----");
			break;			
		case 1:
			sprintf(g_tvpin_display,"*---");
			break;			
		case 2:
			sprintf(g_tvpin_display,"**--");
			break;			
		case 3:
			sprintf(g_tvpin_display,"***-");
			break;
		case 4:
			sprintf(g_tvpin_display,"****");
			break;
	}
	OSD_SetTextFieldContent(&win_popup_item_val, STRING_ANSI, (UINT32)g_tvpin_display);
	OSD_DrawObject((POBJECT_HEAD)&win_popup_item_val,C_UPDATE_ALL);
}

#endif



static void win_popup_Init();
static UINT8 win_popup_proc(UINT32 msgtype, UINT32 msgcode);

//static UINT8 win_popup_callback(WINDOW* w,UINT8 evt,OBJECTHEAD* focusObj,void* para);

static const UINT8 win_popup_btm_num[] =
{
        0,
#if(defined(MULTI_CAS)&&(CAS_TYPE == CAS_ABEL))	
	 0,			
#endif
        1,
        2,
        3
};

static OBJECT_HEAD *win_popup_btm_objs[] =
{
        ( OBJECT_HEAD* ) &win_popup_yes,
        ( OBJECT_HEAD* ) &win_popup_no,
        ( OBJECT_HEAD* ) &win_popup_cancel
};

static UINT16 btn_str_ids[] =
{
        RS_COMMON_YES,
        RS_COMMON_NO,
        RS_COMMON_CANCEL

};

static UINT16 mp_btn_ids[] = 
{
	RS_MP_MUSIC_PLAYLIST,
	RS_MP_IMAGE_PLAYLIST,
	RS_COMMON_CANCEL
};

/*******************************************************************************
 *	Window's keymap, proc and  callback
 *******************************************************************************/

static VACTION popup_con_keymap(POBJECT_HEAD pObj, UINT32 vkey)
{
	VACTION act = VACT_PASS;

	if (win_popup_type != WIN_POPUP_TYPE_SMSG)
	{
		switch (vkey)
		{
			case V_KEY_MENU:
			case V_KEY_EXIT:
				win_popup_choice = WIN_POP_CHOICE_NO;
				act = VACT_CLOSE;
				break;
			case V_KEY_LEFT:
	#if(defined(MULTI_CAS) &&(CAS_TYPE == CAS_ABEL) )
			 if(win_popup_type == WIN_POPUP_TYPE_INPUTPIN)
			 	act = VACT_TVPIN_DEL_ONE_CHAR;
			 else
			 	act = VACT_CURSOR_LEFT;
	#else
				act = VACT_CURSOR_LEFT;
	#endif				
				break;
			case V_KEY_RIGHT:
				act = VACT_CURSOR_RIGHT;
				break;
	#if(defined(MULTI_CAS) &&(CAS_TYPE == CAS_ABEL) )
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
				act = VACT_NUM_0 + ( vkey - V_KEY_0 );
				break;
	#endif		
			default:
				break;
		}
	}
	else
	{
		if ((vkey == V_KEY_EXIT) || (vkey == V_KEY_MENU))
			act = VACT_CLOSE;
	}
	return act;
}

static PRESULT popup_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
#if(defined(MULTI_CAS) &&(CAS_TYPE == CAS_ABEL) )
	UINT8	unact, input, save;
	UINT8	id = OSD_GetObjID ( pObj );
	UINT16 	vkey;
	POBJECT_HEAD	newMenu;
	UINT8 i;
	UINT8 ret;

	vkey = (UINT16)param1;

	if(vkey==V_KEY_EXIT||vkey ==  V_KEY_MENU)
	{
		//libc_printf("V_KEY_EXIT!\n");
		win_compopup_set_enter_wrong_pin_mmi_status(FALSE);
		return PROC_LEAVE;
	}

	//libc_printf("popup_con_callback(pObj=0x%08X, event=0x%08X, param1=0x%08X, param2=0x%08X)\n", pObj, event, param1, param2);
	switch ( event )
	{
		case EVN_FOCUS_PRE_GET:
			break;

		case EVN_FOCUS_PRE_LOSE:
			break;

		case EVN_UNKNOWN_ACTION:
			unact = ( VACTION ) ( param1 >> 16 ) ;
			if(g_tvpin_confirm_mode)
			{
				if ( unact <= VACT_NUM_9 )
				{
					if(g_input_tvpin_pos<=3)
					{
						g_input_tvpin_chars[g_input_tvpin_pos]=unact-VACT_NUM_0+'0';
						g_input_tvpin_pos++;
						_update_popup_item_tvpin_display();
					}
					
					if(g_input_tvpin_pos == 4)
					{
						// issue!!
						if(api_abel_mmi_check_mainmenu_on_top())
						{
							if(menu_stack_get_top()==(POBJECT_HEAD)&g_win_change_tv_pin)
							{
								ABEL_PRINTF("-[UI] Change TV PIN\n");

								for(i=0; i<8; i++)
								{
									ABEL_PRINTF("g_target_tvpin_for_confirm_ui[%d]=%c (%d)\n", i, g_target_tvpin_for_confirm_ui[i], g_target_tvpin_for_confirm_ui[i]);
								}

								if(kms_change_pin(g_input_tvpin_chars, g_target_tvpin_for_confirm_ui)==SUCCESS)					
								{
									g_tvpin_confirm_proc_exec_result=TRUE;
									ABEL_PRINTF("kms_change_pin()::SUCCESS\n");
									ret = SUCCESS;
								}
								else
								{
									g_tvpin_confirm_proc_exec_result=FALSE;
									ABEL_PRINTF("kms_change_pin()::FAIL\n");
									ret = !SUCCESS;
								}
							}
							else if(menu_stack_get_top()==(POBJECT_HEAD)&g_win_parental_control)
							{
								ABEL_PRINTF("-[UI] Parental Control\n");
							
								if(parental_control(g_input_tvpin_chars, g_target_parental_control_level_for_confirm_ui)==SUCCESS)
								{
									g_tvpin_confirm_proc_exec_result=TRUE;
									ABEL_PRINTF("kms_change_pin()::SUCCESS\n");
									ret = SUCCESS;
								}
								else
								{
									g_tvpin_confirm_proc_exec_result=FALSE;
									ABEL_PRINTF("kms_change_pin()::FAIL\n");
									ret = !SUCCESS;
								}					
							}
							else
							{
								ABEL_PRINTF("-[UI] Unknown (should be preview menu)\n");
						
								if(user_confirm(TV_PIN, g_input_tvpin_chars)==SUCCESS)					
								{
									g_tvpin_confirm_proc_exec_result=TRUE;
									ABEL_PRINTF("kms_change_pin()::SUCCESS\n");							
									ret = SUCCESS;
								}
								else
								{
									g_tvpin_confirm_proc_exec_result=FALSE;
									ABEL_PRINTF("kms_change_pin()::FAIL\n");
									ret = !SUCCESS;
								}
							}
						}
						else
						{
							ABEL_PRINTF("-[UI] Full Screen Display\n");
							if(user_confirm(TV_PIN, g_input_tvpin_chars)==SUCCESS)					
							{
								g_tvpin_confirm_proc_exec_result=TRUE;
								ABEL_PRINTF("user_confirm()::SUCCESS\n");
								ret = SUCCESS;
							}
							else
							{
								g_tvpin_confirm_proc_exec_result=FALSE;
								ABEL_PRINTF("user_confirm()::FAIL\n");
								ret = !SUCCESS;
							}
						}

						if(ret != SUCCESS)
						{
							g_input_tvpin_pos = 0;
							_update_popup_item_tvpin_display();
							return PROC_LOOP;
						}
						
						return PROC_LEAVE;
					}
					
				}
				else if(unact == VACT_TVPIN_DEL_ONE_CHAR)
				{					
					if(g_input_tvpin_pos>0)
					{
						g_input_tvpin_pos--;
						g_input_tvpin_chars[g_input_tvpin_pos]=0;
						_update_popup_item_tvpin_display();
					}
				}
			}
			return PROC_LOOP;
			break;
		case EVN_PRE_CLOSE:
			break;
		case EVN_POST_CLOSE:
#if 0			
			if(g_tvpin_confirm_mode&&g_tvpin_input_success)
			{
				if(	menu_stack_get_top()==&g_win_parental_control	||
					menu_stack_get_top()==&g_win_change_tv_pin)
				{
					menu_stack_pop();
					newMenu = menu_stack_get_top();
					if(newMenu != NULL)	/* Open the top menu */
						OSD_ObjOpen(newMenu, MENU_OPEN_TYPE_STACK);		
				}
			}
#endif			
			on_event_cmsg_popup_is_closed();
			win_compopup_reset_target_tvpin_value();
			break;			
	}

#endif

	return PROC_PASS;
}

/*******************************************************************************
 *	BUTTON --- proc
 ********************************************************************************/

static VACTION popup_btn_text_keymap(POBJECT_HEAD pObj, UINT32 vkey)
{
	VACTION act = VACT_PASS;

	if (vkey == V_KEY_ENTER)
		act = VACT_ENTER;

	return act;
}


static PRESULT popup_btn_text_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	VACTION act;
	UINT16 vkey;
	UINT8 id;

	if (event == EVN_UNKNOWN_ACTION)
	{
		id = pObj->bID;
		act = (UINT8)(param1 >> 16);
		vkey = (UINT16)param1;

		if (act == VACT_ENTER)
		{
			if (id == POPUP_YES_ID)
				win_popup_choice = WIN_POP_CHOICE_YES;
			else if (id == POPUP_NO_ID)
				win_popup_choice = WIN_POP_CHOICE_NO;
			else if (id == POPUP_CANCEL_ID)
				win_popup_choice = WIN_POP_CHOICE_CANCEL;
			else
				win_popup_choice = WIN_POP_CHOICE_NULL;

			return PROC_LEAVE;
		}
	}

	return PROC_PASS;
}

/*********************************************************************************
 *
 *	Window  open,proc
 *
 **********************************************************************************/
#ifdef AD_SHARE_UI_OSD_LAYER
extern UINT8 ad_valid;
#endif
static void win_popup_Init()
{
	CONTAINER *con = &g_win_popup;
#ifdef AD_SHARE_UI_OSD_LAYER
    ad_valid = 0;
#endif
	OSD_TrackObject((POBJECT_HEAD)con, C_UPDATE_ALL);
#ifdef AD_SHARE_UI_OSD_LAYER
    ad_valid = 1;
#endif
}

/*********************************************************************************
 *
 *	exported APIs
 *
 **********************************************************************************/

static UINT8 win_com_popup_frame_set;
void win_compopup_init(win_popup_type_t type)
{
	OBJECT_HEAD *obj;
	UINT8 i, btn_num;
	UINT16 btn_left;
	TEXT_FIELD *txt;
	CONTAINER *win = &g_win_popup;
	POBJECT_HEAD pObjNext;

	msg_str[0] = 0x0000;
	
#if(defined(MULTI_CAS)&&(CAS_TYPE == CAS_ABEL))
	on_event_win_compopup_init();
	g_input_tvpin_pos=0;
	g_tvpin_confirm_mode				=	FALSE;
	g_tvpin_confirm_proc_exec_result	=	FALSE;
	memset(g_input_tvpin_chars, 0xFF, 8);	
#endif


	win_popup_type = type;

	win_popup_type = (type <= WIN_POPUP_TYPE_OKNOCANCLE) ? type : WIN_POPUP_TYPE_SMSG;
	win_popup_choice = WIN_POP_CHOICE_NULL;

	btn_num = win_popup_btm_num[win_popup_type];

	POP_PRINTF("%s:win_popup_type=%d,btn_num=%d\n", __FUNCTION__, win_popup_type, btn_num);

	OSD_SetContainerFocus(win, 1);
	win->pNextInCntn = (OBJECT_HEAD*) &win_popup_msg_mbx;

	/* Link Window's objects */
	obj = (OBJECT_HEAD*) &win_popup_msg_mbx;
	pObjNext = (btn_num <= 0) ? NULL : (POBJECT_HEAD)win_popup_btm_objs[0];
	OSD_SetObjpNext(obj, pObjNext);

	obj = win_popup_btm_objs[0];
	pObjNext = (btn_num <= 1) ? NULL : (POBJECT_HEAD)win_popup_btm_objs[1];
	OSD_SetObjpNext(obj, pObjNext);

	txt = (TEXT_FIELD*)win_popup_btm_objs[0];

#if 0
	if (btn_num == 1)
		txt->wStringID = 0;
	else
		txt->wStringID = 0;
#endif

	obj = win_popup_btm_objs[1];
	pObjNext = (btn_num <= 2) ? NULL : (POBJECT_HEAD)win_popup_btm_objs[2];
	OSD_SetObjpNext(obj, pObjNext);

	if (btn_num == 1)
		OSD_SetID(win_popup_btm_objs[0], 1, 1, 1, 1, 1);
	else if (btn_num == 2)
	{
		OSD_SetID(win_popup_btm_objs[0], 1, 2, 2, 1, 1);
		OSD_SetID(win_popup_btm_objs[1], 2, 1, 1, 2, 2);
	}
	else if (btn_num == 3)
	{
		OSD_SetID(win_popup_btm_objs[0], 1, 3, 2, 1, 1);
		OSD_SetID(win_popup_btm_objs[1], 2, 1, 3, 2, 2);
		OSD_SetID(win_popup_btm_objs[2], 3, 2, 1, 3, 3);
	}

	for (i = 0; i < 3; i++)
	{
		txt = (TEXT_FIELD*)win_popup_btm_objs[i];
		OSD_SetTextFieldContent(txt, STRING_ID, (UINT32)btn_str_ids[i]);
	}


	win_com_popup_frame_set = 0;

	win_compopup_set_frame(W_MSG_L, W_MSG_T, W_MSG_W, W_MSG_H);


	/* Set Window & object's default pos and size */
	//win_compopup_set_frame(W_L,W_T,W_W,W_H);

	win_com_popup_frame_set = 1;

}

void win_compopup_set_title(char *str, char *unistr, UINT16 strID)
{
	TEXT_FIELD *txt;
	CONTAINER *win = &g_win_popup;

	txt = &win_popup_title_txt;
	OSD_SetTextFieldStrPoint(txt, msg_title);
	if (str != NULL)
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
	else if (unistr != NULL)
		OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)unistr);
	else
	{
		if (strID != 0)
		{
			OSD_SetTextFieldStrPoint(txt, NULL);
			OSD_SetTextFieldContent(txt, STRING_ID, (UINT32)strID);
		}
		else
		{
			win->pNextInCntn = (OBJECT_HEAD*) &win_popup_msg_mbx;
			return ;
		}
	}

	OSD_SetContainerNextObj(win, txt);
	//	win->pNextInCntn = (OBJECT_HEAD*)txt;

}


void win_compopup_set_msg(char *str, char *unistr, UINT16 strID)
{
	UINT8 i, btn_num, lines;
	UINT8 *pstr;
	POBJECT_HEAD txt = (POBJECT_HEAD) &win_popup_msg_mbx;
	UINT16 totalw, mw, mh;
	UINT16 l, t, w, h;
	UINT8 title_flag;
	CONTAINER *win = &g_win_popup;

	title_flag = (win->pNextInCntn == txt) ? 0 : 1;

	popup_mtxt_content.bTextType = STRING_UNICODE;
	popup_mtxt_content.text.pString = msg_str;

	if (str != NULL)
		ComAscStr2Uni(str, msg_str);
	else if (unistr != NULL)
		ComUniStrCopyChar((UINT8*)msg_str, unistr);
	else
	{
		popup_mtxt_content.bTextType = STRING_ID;
		popup_mtxt_content.text.wStringID = strID;
	}

	btn_num = win_popup_btm_num[win_popup_type];
	if (str != NULL || unistr != NULL)
		pstr = (UINT8*)msg_str;
	else
		pstr = OSD_GetUnicodeString(strID);

	totalw = OSD_MultiFontLibStrMaxHW(pstr, txt->bFont, &mw, &mh, 0);

	totalw += MSG_LOFFSET * 2;


	if (btn_num <= 1)
		w = MIN_WIDHT_1ORNOBTN;
	else
		w = MIN_WIDTH_2BTN;

CHECK_LINES:
	lines = ( totalw + w - 1 ) / w;
	if (lines <= 1)
		lines = 1;
	else
	{
		if (w != MAX_LEN_ONE_LINE)
		{
			w = MAX_LEN_ONE_LINE;
			goto CHECK_LINES;
		}
	}

	if (mh < 24)
		mh = 24;
	h = mh * lines + (lines - 1) *4;
	if (btn_num == 0)
		h += MSG_VOFFSET_NOBTN * 2;
	else
		h += (MSG_VOFFSET_WITHBTN + MSG_BUTTON_TOPOFFSET + MSG_BUTTON_BOTOFFSET + BTM_H);

	if (title_flag)
		h += TITLE_H + 10;

	l = (OSD_MAX_WIDTH - w) >> 1;
	t = (OSD_MAX_HEIGHT - h) >> 1;

	t = (t + 1) / 2 * 2;

	win_compopup_set_frame(l, t, w, h);
}


void win_compopup_set_msg_ext(char *str, char *unistr, UINT16 strID)
{
	UINT8 *pstr;
	POBJECT_HEAD txt = (POBJECT_HEAD) &win_popup_msg_mbx;
	UINT8 title_flag;
	CONTAINER *win = &g_win_popup;


	popup_mtxt_content.bTextType = STRING_UNICODE;
	popup_mtxt_content.text.pString = msg_str;

	if (str != NULL)
		ComAscStr2Uni(str, msg_str);
	else if (unistr != NULL)
		ComUniStrCopyChar((UINT8*)msg_str, unistr);
	else
	{
		popup_mtxt_content.bTextType = STRING_ID;
		popup_mtxt_content.text.wStringID = strID;
	}

	if (str != NULL || unistr != NULL)
		pstr = (UINT8*)msg_str;
	else
		pstr = OSD_GetUnicodeString(strID);
}

void win_compopup_set_btnstr(UINT32 btn_idx, UINT16 str_id)
{
	TEXT_FIELD *txt;

	if (btn_idx < 3)
	{
		txt = (TEXT_FIELD*)win_popup_btm_objs[btn_idx];
		OSD_SetTextFieldContent(txt, STRING_ID, (UINT32)str_id);
	}
}

void win_compopup_set_frame(UINT16 x, UINT16 y, UINT16 w, UINT16 h)
{
	OBJECT_HEAD *obj;
	UINT8 i, btn_num;
	UINT16 btn_left, btn_top, btm_width;
	UINT16 topoffset, botoffset;

	UINT16 msgwidth, wMaxHeight, wMaxWidth;
	OBJECT_HEAD *objmbx;
	MULTI_TEXT *mbx;
	UINT8 title_flag;
	CONTAINER *win = &g_win_popup;
	TEXT_FIELD *txt_title = &win_popup_title_txt;



	title_flag = (OSD_GetContainerNextObj(win) == (POBJECT_HEAD)txt_title) ? 1 : 0;
	if (title_flag)
		OSD_SetRect(&txt_title->head.frame, x + 10, y + 6, w - 20, TITLE_H);


	// if(win_com_popup_frame_set) return;


	/* Set Window & object's default pos and size */
	obj = (OBJECT_HEAD*) &g_win_popup;
	OSD_SetRect(&obj->frame, x, y, w, h);

	btn_num = win_popup_btm_num[win_popup_type];


	objmbx = (OBJECT_HEAD*) &win_popup_msg_mbx;
	mbx = &win_popup_msg_mbx;
	if (btn_num == 0)
	{
	#if (defined(MULTI_CAS)&&(CAS_TYPE == CAS_ABEL))
		if(win_popup_type == WIN_POPUP_TYPE_INPUTPIN)
			mbx->bAlign = C_ALIGN_CENTER | C_ALIGN_TOP;
		else
			mbx->bAlign = C_ALIGN_CENTER | C_ALIGN_VCENTER;
	#else
		mbx->bAlign = C_ALIGN_CENTER | C_ALIGN_VCENTER;
	#endif
		topoffset = MSG_VOFFSET_NOBTN;
		if (h - topoffset * 2 < DEFAULT_FONT_H)
			topoffset = (h - DEFAULT_FONT_H) / 2;

		botoffset = topoffset;
		if (title_flag)
			topoffset += TITLE_H;
	}
	else
	{
		mbx->bAlign = C_ALIGN_CENTER | C_ALIGN_TOP;
		topoffset = MSG_VOFFSET_WITHBTN; //h/5;
		botoffset = topoffset + BTM_H;
		if (title_flag)
			topoffset += TITLE_H;
	}
	//OSD_SetRect(&objmbx->frame,  x + MSG_LOFFSET,y + topoffset,w - MSG_LOFFSET*2,h - botoffset - topoffset);
	OSD_SetObjRect(objmbx, x + MSG_LOFFSET, y + topoffset, w - MSG_LOFFSET * 2, h - botoffset - topoffset);

	mbx->rcText.uWidth = objmbx->frame.uWidth;
	mbx->rcText.uHeight = objmbx->frame.uHeight;


	for (i = 0; i < btn_num; i++)
	{
		obj = win_popup_btm_objs[i];

		//btm_width = w/5;
		//if(btm_width>BTM_W)
		btm_width = BTM_W;

		if (btn_num == 1)
			btn_left = x + (w - btm_width) / 2;
		else if (btn_num == 2)
			btn_left = x + (w - btm_width * btn_num - BTM_GAP) / 2+(btm_width + BTM_GAP) *i;
		else
			btn_left = x + w - (btm_width + BTM_GAP)*(btn_num - i) - 40;

		btn_top = h - MSG_BUTTON_BOTOFFSET - BTM_H;

		OSD_SetRect(&obj->frame, btn_left, (y + btn_top), btm_width, BTM_H);
	}
	
#if(defined(MULTI_CAS)&&(CAS_TYPE == CAS_ABEL))	
	TEXT_FIELD *txt_item_name 	= &win_popup_item_name;
	TEXT_FIELD *txt_item_val 		= &win_popup_item_val;

	if(OSD_GetObjpNext(objmbx)==(POBJECT_HEAD)&win_popup_item_name)
	{	
		OSD_SetRect(&txt_item_name->head.frame, x + MSG_LOFFSET, y + h - MSG_BUTTON_BOTOFFSET - BTM_H*2+6 , (w - MSG_LOFFSET * 2)/2, TITLE_H);

		obj = (POBJECT_HEAD)&win_popup_item_name;
		if(OSD_GetObjpNext(obj)==(POBJECT_HEAD)&win_popup_item_val)
		{
			OSD_SetRect(&txt_item_val->head.frame, x + MSG_LOFFSET+(w - MSG_LOFFSET * 2)/2 - val_Xoffset+10, y + h - MSG_BUTTON_BOTOFFSET - BTM_H*2+6, (w - MSG_LOFFSET * 2)/2-4, TITLE_H);	

		}
	}
	else if(OSD_GetObjpNext(objmbx)==(POBJECT_HEAD)&win_popup_item_val)
	{
		OSD_SetRect(&txt_item_val->head.frame, x + MSG_LOFFSET+(w - MSG_LOFFSET * 2)/2, y + h - MSG_BUTTON_BOTOFFSET - BTM_H*2+6, (w - MSG_LOFFSET * 2)/2-4, TITLE_H);	
	}
#endif

}

void win_compopup_set_pos(UINT16 x, UINT16 y)
{
	OSD_MoveObject(&g_win_popup, x, y, FALSE);
}

void win_compopup_set_default_choice(win_popup_choice_t choice)
{
	win_popup_choice = choice;
	g_win_popup.FocusObjectID = win_popup_choice;
}


win_popup_choice_t win_compopup_open(void)
{
	CONTAINER *con = &g_win_popup;
	PRESULT bResult = PROC_LOOP;
	UINT32 hkey;
	UINT32 msg;
	//UIMsg_t	uiMsg;

	win_popup_Init();

	if (win_popup_type != WIN_POPUP_TYPE_SMSG)
	{
        pop_win_sel = 1;
		while (bResult != PROC_LEAVE)
		{
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_DVN)
            if(2 == pop_win_sel )
            {
                ap_cas_callback(CAS_DISP_OSD, 0);
                win_popup_choice = WIN_POP_CHOICE_YES;
                return win_popup_choice;
            }
#endif
#endif

			hkey = ap_get_key_msg();
			//bValidKey = UIGetKeyFromMsg(&uiMsg);
			if (hkey == INVALID_HK || hkey == INVALID_MSG)
			{
				POP_PRINTF("Unvalid hkey\n");
				continue;
			}
			POP_PRINTF("!!!!!hkey==%d\n", hkey);
			//msg = OSD_GetOSDMessage(MSG_TYPE_KEY,hkey);

			bResult = OSD_ObjProc((POBJECT_HEAD)con, (MSG_TYPE_KEY << 16), hkey, 0); //w->head.pProc((OBJECTHEAD *)w, msgcode, 0, save_back_ok);
			POP_PRINTF("bResult==%d\n", bResult);
		}

	}
    pop_win_sel = 0;

#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_ABEL)
	on_event_able_mmi_close();
#endif
#endif
	return win_popup_choice;
}

void win_compopup_close(void)
{
	OBJECT_HEAD *obj = (OBJECT_HEAD*) &g_win_popup;

#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_ABEL)
	on_event_able_mmi_close();
#endif
#endif
	OSD_ObjClose(obj, C_CLOSE_CLRBACK_FLG);
}

win_popup_choice_t win_com_popup_open(win_popup_type_t type, char *str, UINT16 strID)
{
	win_compopup_init(type);
	win_compopup_set_msg(str, NULL, strID);
	return win_compopup_open();
}

extern UINT8 *osd_vscr_buffer[];

static UINT8 *g_back_buf = NULL;
static UINT32 g_back_buf_size;

#define MAX_BACK_BUF_SIZE (2*1024)
static UINT8 back_buf[MAX_BACK_BUF_SIZE];

#ifdef USE_LIB_GE
INT32 win_com_backup_rect(OSD_RECT *prc)
{
	UINT8 *pBuf = NULL;
	GUI_VSCR *lpVscr;

	lpVscr = OSD_GetTaskVscr(osal_task_get_current_id());
	
	if(((prc->uLeft+prc->uWidth)>OSD_MAX_WIDTH) ||
		((prc->uTop+prc->uHeight)>OSD_MAX_HEIGHT))
	{
		libc_printf("\npopup rect invalid!\n");
		return -1;
	}

	g_back_buf_size = (prc->uWidth*prc->uHeight)<<1;
	
	if (g_back_buf_size < MAX_BACK_BUF_SIZE)
	{
		pBuf = back_buf;
		save_back_ok = 1;
	}
	else
	{
		if(g_back_buf!=NULL)
		{
			FREE(g_back_buf);
			g_back_buf = NULL;
		}

		pBuf = (UINT8*)MALLOC(g_back_buf_size);
		MEMSET(pBuf,0,g_back_buf_size);
			
		if (pBuf != NULL)
		{
			g_back_buf = pBuf;
			save_back_ok = 1;
		}
		else
		{
			libc_printf("\npop up malloc fail!\n");
			save_back_ok = 0;
			return -2;
		}
	}

	if (save_back_ok)
	{
		OSD_RectBackUp(lpVscr,pBuf,prc);
	}	

	return 0;
}

void win_com_restore_rect(OSD_RECT *prc)
{
	UINT8 *pBuf = NULL;
	GUI_VSCR *lpVscr;

	if (!save_back_ok)
		return;
	
	lpVscr = OSD_GetTaskVscr(osal_task_get_current_id());	
	
	if(g_back_buf != NULL)
		pBuf = g_back_buf;
	else
		pBuf = back_buf;

	OSD_RectRestore(lpVscr,pBuf,prc);

	if (g_back_buf != NULL)
	{
		FREE(g_back_buf);
		g_back_buf = NULL;
	}	
}
#endif

win_popup_choice_t win_compopup_open_ext(UINT8 *back_saved)
{
	CONTAINER *win;
	UINT32 vscr_size;
	//struct OSDRect *frame;
	OSD_RECT *frame;
	VSCR vscr, *lpVscr;;
	win_popup_choice_t ret;
	UINT8 *pbuf = NULL;

	win = &g_win_popup;
	frame = &win->head.frame;

	lpVscr = OSD_GetTaskVscr(osal_task_get_current_id());
	OSD_UpdateVscr(lpVscr);
#ifdef USE_LIB_GE
    vscr_size= (frame->uWidth*frame->uHeight)<<1;
    if (vscr_size < MAX_BACK_BUF_SIZE)
	{
		pbuf = back_buf;
		save_back_ok = 1;
	}
#else
    lpVscr->lpbScr = NULL;
	vscr_size = (((frame->uWidth) >> FACTOR) + 1) *frame->uHeight;
	if (vscr_size < OSD_VSRC_MEM_MAX_SIZE / 2)
	{
		pbuf = &osd_vscr_buffer[osal_task_get_current_id() - 1][vscr_size];
		save_back_ok = 1;
	}
#endif
	else
	{
		if (g_back_buf != NULL && g_back_buf_size >= vscr_size)
			pbuf = g_back_buf;
		else
		{
			if (g_back_buf != NULL)
				FREE(g_back_buf);
			pbuf = (UINT8*)MALLOC(vscr_size);
			g_back_buf = pbuf;
			if (g_back_buf != NULL)
				g_back_buf_size = vscr_size;
		}
		if (pbuf != NULL)
			save_back_ok = 1;
		else
			save_back_ok = 0;
	}

	if (save_back_ok)
	{
#ifdef USE_LIB_GE
       OSD_RectBackUp(lpVscr,pbuf,frame);
#else
		OSD_SetRect2(&vscr.vR, frame);
		vscr.lpbScr = pbuf;
		OSD_GetRegionData(&vscr, &vscr.vR);
#endif
	}
    
#ifdef USE_LIB_GE
        OSD_SetCloseClrbackFlag(FALSE);
#endif
	ret = win_compopup_open();
#ifdef USE_LIB_GE
        OSD_ResumeCloseClrbackFlag();
#endif

	if (save_back_ok && win_popup_type != WIN_POPUP_TYPE_SMSG)
	{
#ifdef USE_LIB_GE
        OSD_RectRestore(lpVscr,pbuf,frame);
#else
		OSD_RegionWrite(&vscr, &vscr.vR);
#endif
		if (g_back_buf != NULL)
		{
			FREE(g_back_buf);
			g_back_buf = NULL;
		}
	}

#ifndef USE_LIB_GE
	lpVscr->lpbScr = NULL;
#endif

	*back_saved = save_back_ok;

	return ret;
}

void win_compopup_smsg_restoreback(void)
{
	UINT32 vscr_size;
	CONTAINER *win;
	//struct OSDRect *frame;
	OSD_RECT *frame;
	
    UINT8 *pBuf = NULL;
    
	if (!save_back_ok)
		return ;

	win = &g_win_popup;
	frame = &win->head.frame;
#ifdef USE_LIB_GE
    VSCR *vscr;
    vscr= OSD_GetTaskVscr(osal_task_get_current_id());	
	if(g_back_buf != NULL)
		pBuf = g_back_buf;
	else
		pBuf = back_buf;
	if(pBuf!=NULL)
	OSD_RectRestore(vscr,pBuf,frame);
#else
    VSCR vscr;
	vscr_size = (((frame->uWidth) >> FACTOR) + 1) *frame->uHeight;

	if ((vscr_size >= OSD_VSRC_MEM_MAX_SIZE / 2) && (g_back_buf == NULL))
		return;

	if (vscr_size < OSD_VSRC_MEM_MAX_SIZE / 2)
	{
		vscr.lpbScr = &osd_vscr_buffer[osal_task_get_current_id() - 1][vscr_size];
	}
	else if (g_back_buf != NULL)
	{
		if (vscr_size > g_back_buf_size)
		{
			FREE(g_back_buf);
			g_back_buf = NULL;
			return ;
		}
		vscr.lpbScr = g_back_buf;
	}

	OSD_SetRect2(&vscr.vR, frame);
	OSD_RegionWrite(&vscr, &vscr.vR);
#endif

	if (g_back_buf != NULL)
	{
		FREE(g_back_buf);
		g_back_buf = NULL;
	}

}

void win_compopup_set_mpbtn(BOOL bSet)
{
	UINT8 i;
	TEXT_FIELD *txt;

	for(i=0;i<3;i++)
	{
		txt = (TEXT_FIELD*)win_popup_btm_objs[i];
		if(bSet)
			OSD_SetTextFieldContent(txt, STRING_ID, (UINT32)mp_btn_ids[i]);
		else
			OSD_SetTextFieldContent(txt, STRING_ID, (UINT32)btn_str_ids[i]);
	}
}

extern OSAL_ID control_mbf_id;

static UINT32 defaultca_key_exit[] = 
{
	V_KEY_EXIT,V_KEY_MENU,V_KEY_LEFT,V_KEY_RIGHT,V_KEY_UP,V_KEY_DOWN
};

static ID wincom_popup_timer_id = OSAL_INVALID_ID;
static BOOL popup_expire = FALSE;

static void win_compopup_timerhandler()
{
	if(wincom_popup_timer_id!= OSAL_INVALID_ID)
	{
		popup_expire = TRUE;
		win_popup_choice = WIN_POP_CHOICE_YES;
	}
}

win_popup_choice_t win_compopup_open_timer(UINT32 timer,UINT8* back_saved)
{
	CONTAINER* win;
	UINT32 vscr_size;
	struct OSDRect*		frame;
	VSCR vscr,*lpVscr;
	win_popup_choice_t ret;
	UINT8* pbuf = NULL;
	PRESULT bResult = PROC_LOOP;
	UINT32 hkey = INVALID_HK;
	UINT32 vkey = V_KEY_NULL;
	struct pan_device*pan_dev = (struct pan_device*)dev_get_by_id(HLD_DEV_TYPE_PAN,0);
	UINT16 key;	
	 
	win =  &g_win_popup;
	frame = &win->head.frame;

	lpVscr = OSD_GetTaskVscr(osal_task_get_current_id());
	OSD_UpdateVscr(lpVscr);
	
#ifdef USE_LIB_GE
    vscr_size= (frame->uWidth*frame->uHeight)<<1;
    if (vscr_size < MAX_BACK_BUF_SIZE)
	{
		pbuf = back_buf;
		save_back_ok = 1;
	}
#else
      lpVscr->lpbScr = NULL;
	vscr_size = (((frame->uWidth) >> FACTOR) + 1) *frame->uHeight;
	if (vscr_size < OSD_VSRC_MEM_MAX_SIZE / 2)
	{
		pbuf = &osd_vscr_buffer[osal_task_get_current_id() - 1][vscr_size];
		save_back_ok = 1;
	}
#endif	
	else
	{
		if (g_back_buf != NULL && g_back_buf_size >= vscr_size)
			pbuf = g_back_buf;
		else
		{
			if (g_back_buf != NULL)
				FREE(g_back_buf);
			pbuf = (UINT8*)MALLOC(vscr_size);
			g_back_buf = pbuf;
			if (g_back_buf != NULL)
				g_back_buf_size = vscr_size;
		}
		if (pbuf != NULL)
			save_back_ok = 1;
		else
			save_back_ok = 0;
	}

	if(save_back_ok)
	{
#ifdef USE_LIB_GE
        OSD_RectBackUp(lpVscr, pbuf, frame);
#else
		OSD_SetRect2(&vscr.vR, frame);
		vscr.lpbScr = pbuf;
		OSD_GetRegionData(&vscr, &vscr.vR);
#endif
	}

	popup_expire = FALSE;
	if(win_popup_type!=WIN_POPUP_TYPE_SMSG)
		wincom_popup_timer_id = api_start_timer("popup", timer, win_compopup_timerhandler);

	//ret = win_compopup_open();
	win_popup_Init();

	if(win_popup_type!=WIN_POPUP_TYPE_SMSG)
	{
		while((bResult != PROC_LEAVE)&&(popup_expire==FALSE))
		{
			if(system_state == SYS_STATE_DATACAST)
			{
				//if(get_input_key(pan_dev,&key,&hkey)==0)
					hkey = INVALID_HK;
			}
			else
			{
				hkey = ap_get_key_msg();
			}
			
			if(hkey == INVALID_HK || hkey == INVALID_MSG)
			{
				POP_PRINTF("Unvalid hkey\n");
				continue;
			}
			POP_PRINTF("!!!!!hkey==%d\n",hkey);

			bResult = OSD_ObjProc((POBJECT_HEAD)&g_win_popup, (MSG_TYPE_KEY<<16),hkey, 0);
			POP_PRINTF("bResult==%d\n",bResult);
		}
//		if(popup_expire==FALSE)
//			ap_send_msg(CTRL_MSG_SUBTYPE_KEY, hkey, FALSE);

		api_stop_timer(&wincom_popup_timer_id);
	}

	ret = win_popup_choice;
	
	if (save_back_ok && win_popup_type != WIN_POPUP_TYPE_SMSG)
	{
#ifdef USE_LIB_GE
        OSD_RectRestore(lpVscr, pbuf, frame);
#else
		OSD_RegionWrite(&vscr, &vscr.vR);
#endif	
		if (g_back_buf != NULL)
		{
			FREE(g_back_buf);
			g_back_buf = NULL;
		}
	}
#ifndef USE_LIB_GE
	lpVscr->lpbScr = NULL;
#endif
	*back_saved = save_back_ok;

	return ret;
}

#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_CONAX)
 void win_mmicompopup_set_msg(char* str,char* unistr,UINT16 strID)
{
	UINT8 i,btn_num,lines;
	UINT8* pstr;
	POBJECT_HEAD txt = (POBJECT_HEAD)&win_popup_msg_mbx;
	UINT16 totalw,mw,mh;
	UINT16 l,t,w,h;
	UINT8	title_flag;
	CONTAINER* win = &g_win_popup;

	title_flag = (win->pNextInCntn == txt)? 0 : 1;

	popup_mtxt_content.bTextType = STRING_UNICODE;
	popup_mtxt_content.text.pString   = msg_str;
	
	if(str!=NULL)
		ComAscStr2Uni(str, msg_str);
	else if(unistr!=NULL)
		ComUniStrCopyChar( (UINT8*)msg_str,unistr);
	else
	{
		popup_mtxt_content.bTextType        = STRING_ID;
		popup_mtxt_content.text.wStringID   = strID;
	}

	btn_num = win_popup_btm_num[win_popup_type];
	if(str != NULL || unistr!= NULL)
		pstr = (UINT8*)msg_str;
	else
		pstr = OSD_GetUnicodeString(strID);
	totalw = OSD_MultiFontLibStrMaxHW(pstr,txt->bFont,&mw,&mh,0);

	totalw += MSG_LOFFSET*2;

	if(btn_num <= 1)
		w = MIN_WIDHT_1ORNOBTN;
	else
		w = MIN_WIDTH_2BTN;
CHECK_LINES:
	lines = (totalw + w - 1) / w;
	if(lines <= 1)
		lines = 1;
	else
	{
		if(w != MAX_LEN_CONAX_ONE_LINE)
		{
			w = MAX_LEN_CONAX_ONE_LINE;
			goto CHECK_LINES;
		}
	}

	if(mh < DEFAULT_FONT_H)
		mh = DEFAULT_FONT_H; 
	h =  mh * lines + (lines  - 1)*4 ;
	if(btn_num == 0)
		h += MSG_VOFFSET_NOBTN*2;
	else
		h += (MSG_VOFFSET_WITHBTN + MSG_BUTTON_TOPOFFSET + MSG_BUTTON_BOTOFFSET + BTM_H);

	//for jade hd osd
	if(h<144)
		h = 144;

	if(title_flag)
		h += TITLE_H + 10;
	l = (OSD_MAX_WIDTH - w)>>1;
	t = (OSD_MAX_HEIGHT - h)>>1;
	t = (t + 1) /2 * 2 - MSG_VOFFSET_NOBTN;
	win_compopup_set_frame(l,t,w,h);
}

extern BOOL signal_stataus_showed;
void win_mmipopup_open()
{
	#if(CAS_TYPE == CAS_CONAX)
        ap_set_cas_show_flag(TRUE);
	#endif
	if(get_mmi_showed()==5)
	{
		OSD_SetColor((POBJECT_HEAD)&g_win_popup,WSTL_NOSHOW_IDX,WSTL_NOSHOW_IDX,WSTL_NOSHOW_IDX,WSTL_NOSHOW_IDX);
		OSD_SetColor((POBJECT_HEAD)&win_popup_msg_mbx,WSTL_TXT_9,WSTL_TXT_9,WSTL_TXT_9,WSTL_TXT_9);
	}

	OSD_DrawObject((POBJECT_HEAD)&g_win_popup, C_UPDATE_ALL);

	if(get_mmi_showed()==5)
	{
		OSD_SetColor((POBJECT_HEAD)&g_win_popup,POPUPWIN_IDX,POPUPWIN_IDX,POPUPWIN_IDX,POPUPWIN_IDX);
		OSD_SetColor((POBJECT_HEAD)&win_popup_msg_mbx,POPUPSTR_IDX,POPUPSTR_IDX,POPUPSTR_IDX,POPUPSTR_IDX);
	}
}

void win_mmipopup_close()
{
#if(CAS_TYPE == CAS_CONAX)
        ap_set_cas_show_flag(FALSE);

	if(GetSignalStatausShow())
		ShowSignalStatusOSDOnOff(0);
#endif
	if(get_mmi_showed()!=3&&get_mmi_showed()!=4)
		OSD_ObjClose((POBJECT_HEAD)&g_win_popup,C_CLOSE_CLRBACK_FLG);
}

#elif (CAS_TYPE == CAS_ABEL)
void win_compopup_set_enter_wrong_pin_mmi_status(BOOL value)
{
	g_enter_wrong_pin_mmi = value;
}

BOOL  win_compopup_get_enter_wrong_pin_mmi_status()
{
	return g_enter_wrong_pin_mmi;
}

BOOL win_compopup_set_target_parental_control_level(UINT8 target_parental_control_level_code)
{
	g_target_parental_control_level_for_confirm_ui=target_parental_control_level_code;
}


BOOL win_compopup_set_target_tvpin_value(UINT8* target_tvpin)
{
	memcpy(g_target_tvpin_for_confirm_ui, target_tvpin, 8);
}

BOOL win_compopup_reset_target_tvpin_value(void)
{
	memset(g_target_tvpin_for_confirm_ui, 0xFF, 8);
}

BOOL win_compopup_get_confirm_proc_exec_result(void)
{
	return g_tvpin_confirm_proc_exec_result;
}

BOOL win_compopup_reset_confirm_proc_exec_result(void)
{
	g_tvpin_confirm_proc_exec_result=FALSE;
}

void win_compopup_set_tvpin_input_confirm_mode(BOOL bMode)
{
	g_tvpin_confirm_mode = bMode;
}

void win_compopup_set_item_name(char *str, char *unistr, UINT16 strID)
{
	TEXT_FIELD *txt;
	OBJECT_HEAD *obj= (OBJECT_HEAD*)&win_popup_msg_mbx;
	OBJECT_HEAD *pObjNext;

	//libc_printf("win_compopup_set_item_name()\n");
	txt = &win_popup_item_name;
	OSD_SetTextFieldStrPoint(txt, msg_popup_item_name);
	if (str != NULL)
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
	else if (unistr != NULL)
		OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)unistr);
	else
	{
		if (strID != 0)
		{
			OSD_SetTextFieldStrPoint(txt, NULL);
			OSD_SetTextFieldContent(txt, STRING_ID, (UINT32)strID);
		}
		else
		{
			return ;
		}
	}
	OSD_SetObjpNext(obj, txt);

	pObjNext = (win_popup_btm_num[win_popup_type] <= 0) ? NULL : (POBJECT_HEAD)win_popup_btm_objs[0];
	//libc_printf("-msg_popup_item_name->pObjNext=%d\n", pObjNext);
	OSD_SetObjpNext(txt, pObjNext);

}

void win_compopup_set_item_val(char *str, char *unistr, UINT16 strID, UINT16 val_offset)
{
	TEXT_FIELD *txt;	
	OBJECT_HEAD *obj= (OBJECT_HEAD *)&win_popup_msg_mbx;
	OBJECT_HEAD *pObjNext=(OBJECT_HEAD *)&win_popup_item_name;
	
	//libc_printf("win_compopup_set_item_val()\n");
	txt = &win_popup_item_val;
	OSD_SetTextFieldStrPoint(txt, msg_popup_item_val);
	if (str != NULL)
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
	else if (unistr != NULL)
		OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)unistr);
	else
	{
		if (strID != 0)
		{
			OSD_SetTextFieldStrPoint(txt, NULL);
			OSD_SetTextFieldContent(txt, STRING_ID, (UINT32)strID);
		}
		else
		{	
			return ;
		}
	}
	
	obj=(OBJECT_HEAD *)&win_popup_msg_mbx;	
	if(OSD_GetObjpNext(obj)==(POBJECT_HEAD)&win_popup_item_name)
	{
		obj=(OBJECT_HEAD *)&win_popup_item_name;	
		OSD_SetObjpNext(obj, txt);
	}
	else
	{
		OSD_SetObjpNext(obj, txt);
	}

	//	win->pNextInCntn = (OBJECT_HEAD*)txt;

	pObjNext = (win_popup_btm_num[win_popup_type] <= 0) ? NULL : (POBJECT_HEAD)win_popup_btm_objs[0];
	OSD_SetObjpNext(txt, pObjNext);
	val_Xoffset = val_offset;
}
#endif
#endif

