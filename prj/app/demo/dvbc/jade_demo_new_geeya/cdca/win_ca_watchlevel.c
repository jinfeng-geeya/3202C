#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
//#include <api/libosd/osd_lib.h>

#include <api/libcas/gy/gyca_def.h>
#include <api/libcas/gy/gyca_interface.h>
#include <api/libcas/gy/cas_gy.h>

#include "win_cas_com.h"

/*******************************************************************************
*	Objects definition
*******************************************************************************/

#if 0
extern CONTAINER g_win_maturity_rating;
extern CONTAINER win_maturity_rating_con;

extern CONTAINER maturity_rating_item_con1;//input rate
extern CONTAINER maturity_rating_item_con2;//input pwd
extern TEXT_FIELD	maturity_rating_txt1;
extern TEXT_FIELD	maturity_rating_txt2;
extern EDIT_FIELD	maturity_rating_edt1;
extern TEXT_FIELD  maturity_rating_edt2;

static VACTION maturity_rating_item_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT maturity_rating_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION maturity_rating_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT maturity_rating_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

#define CON_SH_IDX   WSTL_BUTTON_2
#define CON_HL_IDX   WSTL_BUTTON_1
#define CON_SL_IDX   WSTL_BUTTON_2
#define CON_GRY_IDX  WSTL_BUTTON_2

#define TXTS_SH_IDX   WSTL_TXT_4
#define TXTS_HL_IDX   WSTL_BUTTON_5//WSTL_TXT_5
#define TXTS_SL_IDX   WSTL_TXT_4
#define TXTS_GRY_IDX  WSTL_TXT_10

#define TXTN_SH_IDX   WSTL_TXT_4
#define TXTN_HL_IDX   WSTL_TXT_5
#define TXTN_SL_IDX   WSTL_TXT_4
#define TXTN_GRY_IDX  WSTL_TXT_10

#define WIN_SH_IDX	    WSTL_COMMON_BACK_2
#define CON_IDX	        WSTL_WIN_2

#define CON_L 84
#define CON_T 142
#define CON_W 560
#define CON_H 30
#define CON_GAP 10

#define TXTN_L_OF 0
#define TXTN_T_OF ((CON_H-TXTN_H)/2)
#define TXTN_W 300
#define TXTN_H 24

#define TXTS_L_OF   (CON_W-TXTS_W)
#define TXTS_T_OF   TXTN_T_OF
#define TXTS_W      180
#define TXTS_H      TXTN_H

#define W_L		0
#define W_T		90
#define W_W		720
#define W_H		400

#define W_CON_L		66
#define W_CON_T	102
#define W_CON_W	604
#define W_CON_H     	264


#define LDEF_CON(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    maturity_rating_item_keymap,maturity_rating_item_callback,  \
    conobj, ID,1)
    
#define LDEF_TXT(root,varTxt,nxtObj,l,t,w,h,resID,str,style,x)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    style, x,0,resID,str)

#define LDEF_EDIT(root, varNum, nxtObj, ID, l, t, w, h,style,cursormode,pat,sub,str)	\
    DEF_EDITFIELD(varNum,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, TXTS_SH_IDX,TXTS_HL_IDX,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    maturity_rating_item_keymap,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 20,0,style,pat,10,cursormode,NULL,sub,str)

#define LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,WIN_SH_IDX,   \
    maturity_rating_keymap,maturity_rating_callback,  \
    nxtObj, focusID,0)

#define LDEF_MENU_ITEM_EDT(root,varCon,nxtObj,varTxt,varNum,ID,IDu,IDd,\
						l,t,w,h,resID,style,curMode,pat,sub,str) \
    LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,&varTxt,ID)   \
    LDEF_TXT(&varCon,varTxt,&varNum,l + TXTN_L_OF,t + TXTN_T_OF,TXTN_W,TXTN_H,resID,NULL,C_ALIGN_LEFT | C_ALIGN_VCENTER,0)    \
    LDEF_EDIT(&varCon,varNum,NULL   ,ID, l + TXTS_L_OF ,t + TXTS_T_OF,TXTS_W,TXTS_H,style,curMode,pat,sub,str)

#define LDEF_MENU_ITEM_TXT(root,varCon,nxtObj,varTxt,varTxtset,ID,IDu,IDd,l,t,w,h,nameID,setstr)	\
    LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,&varTxt,ID)	\
    LDEF_TXT(&varCon,varTxt,&varTxtset,l + TXTN_L_OF,t + TXTN_T_OF,TXTN_W,TXTN_H,nameID,NULL,C_ALIGN_LEFT| C_ALIGN_VCENTER,0)	\
    LDEF_TXT(&varCon,varTxtset,NULL,l + TXTS_L_OF,t + TXTS_T_OF,TXTS_W,TXTS_H,0,setstr,C_ALIGN_CENTER | C_ALIGN_VCENTER,20)

char maturity_pat[] = "r0~15";
#endif

UINT16 maturity_sub[10];

#define DVTCAERR_BASE_STB						0xF1000000
#define DVTCA_OK								0 
#define DVTCAERR_STB_DATA_LEN_ERROR				(DVTCAERR_BASE_STB + 2)		//输入的数据长度不合法
#define DVTCAERR_STB_PIN_LOCKED					(DVTCAERR_BASE_STB + 7)		//PIN码被锁定


#include "win_ca_watchlevel.h"

typedef struct _CDCAPin{

	BYTE	m_byLen;

	BYTE	m_byszPin[PIN_PWD_LEN];

}CDCAPin;
#if 0
typedef struct gui_vscr
{
	union
	{
		GUI_RECT frm;
		GUI_RECT vR;
	};
	
	UINT32	root_layer;
	UINT32	root_rgn;
	UINT32	dst_layer;
	UINT32	dst_rgn;
	
	union
	{
		UINT8	dirty_flag;
		UINT8	updatePending;
	};
	
	UINT8	color_mode;
	UINT8	suspend;
	
	union
	{
		UINT8*	lpbuff;
		UINT8*	lpbScr;
	};

	UINT32 rsv;
}GUI_VSCR,*PGUI_VSCR;

#endif

#if 0

LDEF_MENU_ITEM_EDT(g_win_maturity_rating,maturity_rating_item_con1,&maturity_rating_item_con2,maturity_rating_txt1,maturity_rating_edt1, 1, 2, 2, \
		CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_SCINFO_WATCHLEVEL, NORMAL_EDIT_MODE, CURSOR_NORMAL, maturity_pat,maturity_sub, display_strs[0])

LDEF_MENU_ITEM_TXT(g_win_maturity_rating, maturity_rating_item_con2, NULL, maturity_rating_txt2, maturity_rating_edt2, 2, 1, 1,	\
    CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H,  RS_SCINFO_PINCODE, display_strs[PIN_POSTION])


DEF_CONTAINER (win_maturity_rating_con, NULL, &maturity_rating_item_con1, C_ATTR_ACTIVE, 0, \
                0, 0, 0, 0, 0, W_CON_L, W_CON_T, W_CON_W, W_CON_H, CON_IDX, CON_IDX, CON_IDX, CON_IDX,   \
                NULL, NULL,  \
                NULL, 1, 0 )
                
LDEF_WIN(g_win_maturity_rating,&win_maturity_rating_con,W_L, W_T, W_W, W_H,1)

#endif

UINT8 maturity_rating = 0;

/*******************************************************************************
*	key mapping and event callback definition
*******************************************************************************/
#define MATURITY_RATING_RAT_SET	1
#define MATURITY_RATING_PWD		2
#define DVTCA_CANCEL_PROMTMSG	0XFE   

static PRESULT maturity_rating_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8	unact;
	UINT32 	rating, err;
	CDCAPin pin;
		INT32 temp;
	UINT8	id = OSD_GetObjID ( pObj );
    	//PGUI_VSCR lpVscr;
	
	switch ( event )
	{
		case EVN_FOCUS_PRE_GET:
			break;

		case EVN_FOCUS_PRE_LOSE:
			break;

		case EVN_UNKNOWN_ACTION:
			unact = ( VACTION ) ( param1 >> 16 ) ;
			if ( unact <= VACT_NUM_9 )
			{
				if(id == MATURITY_RATING_PWD)
				{
					pin_pwd_input(PIN_ARRAY_0, (UINT8)(unact - VACT_NUM_0));
					pin_pwd_set_str(PIN_ARRAY_0);
					OSD_TrackObject (pObj, C_UPDATE_ALL );
					//lpVscr = OSD_GetTaskVscr(osal_task_get_current_id());
 					//OSD_UpdateVscr (lpVscr);
					//lpVscr->lpbScr = NULL;
					ret = PROC_LOOP;
					break;
				}
			}
			else if(unact == VACT_ENTER)
			{
				if(id == MATURITY_RATING_RAT_SET)
				{
					OSD_SetContainerFocus(&win_maturity_rating_con,MATURITY_RATING_PWD);
					OSD_TrackObject((POBJECT_HEAD)&win_maturity_rating_con,C_UPDATE_ALL);
					ret = PROC_LOOP;
				}
				else if(id == MATURITY_RATING_PWD)
				{
					OSD_GetEditFieldIntValue(&maturity_rating_edt1,&temp);
					maturity_rating=temp;
					if( ( maturity_rating <4) ||(maturity_rating >18))
					{
						win_popup_msg(NULL, NULL, RS_CAERR_RATING_INVALID_TF);
						OSD_SetContainerFocus(&win_maturity_rating_con,MATURITY_RATING_RAT_SET);
						OSD_TrackObject((POBJECT_HEAD)&win_maturity_rating_con,C_UPDATE_ALL);
						ret = PROC_LOOP;
						break;
					}

					
					err = GYCA_SetParentControl(pin_pwd_password_str[PIN_ARRAY_0], maturity_rating);
					if ((GYCA_CARD_ERR == err)||(GYCA_ERR == err))
					{
						win_popup_msg(NULL, NULL, RS_CAERR_NOSMC);
						ret = PROC_LEAVE; //return from current window
						break;
					}
					else if (GYCA_PIN_ERR == err)
					{
						win_popup_msg(NULL, NULL, RS_CAERR_PIN_ERROR);
						pin_pwd_init(1UL<<PIN_ARRAY_0);
						pin_pwd_set_status(0, FALSE);
						pin_pwd_set_str(PIN_ARRAY_0);
						OSD_TrackObject ( pObj, C_UPDATE_ALL );
						ret = PROC_LOOP;
						break;
					}
					else if (GYCA_OK == err)
					{
						win_popup_msg(NULL, NULL, RS_CAERR_SET_SUCCESS);
						UINT16 msg_id = ap_get_cas_dispstr(0);
						if(( msg_id== RS_SHOWMSG_WATCHLEVEL)||(msg_id == RS_SHOWMSG_RATINGTOOLOW))
						{
							ap_cas_message_setid(DVTCA_CANCEL_PROMTMSG);	//close rating msg
						}
						ret = PROC_LEAVE; 
						break;
					}
					else if (GYCA_PARENTLEVEL_ERR == err)
					{
						win_popup_msg(NULL, NULL, RS_CAERR_RATING_INVALID_TF);						
						ret = PROC_LOOP;
						break;
					}
					else
					{
						win_popup_msg(NULL, NULL, RS_CAERR_UNKNOWN);
						ret = PROC_LOOP;
						break;
					}   
				}
			}
			else if ((unact == VACT_EDIT_LEFT) && (id == MATURITY_RATING_PWD))
			{
				pin_pwd_drawback(PIN_ARRAY_0);
				OSD_TrackObject (pObj, C_UPDATE_ALL );
				ret = PROC_LOOP;
			} 
			break;
		default:
			break;
	}
	return ret;

}

static VACTION maturity_rating_item_keymap(POBJECT_HEAD pObj, UINT32 key)
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
		case V_KEY_ENTER:
			Action = VACT_ENTER;
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

static VACTION maturity_rating_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	UINT8 bID;

	bID = OSD_GetFocusID ( ( POBJECT_HEAD ) &win_maturity_rating_con);
	switch(key)
	{
		case V_KEY_UP:
			if (MATURITY_RATING_PWD == bID)
			{
				pin_pwd_set_status(0, FALSE);
				pin_pwd_set_str(PIN_ARRAY_0);
			}
			act = VACT_CURSOR_UP;
			break;
		case V_KEY_DOWN:
			if (MATURITY_RATING_PWD == bID)
			{
				pin_pwd_set_status(0, FALSE);
				pin_pwd_set_str(PIN_ARRAY_0);
			}
			act = VACT_CURSOR_DOWN;
			break;
		default:
		act = VACT_PASS;
	}

	return act;	
}

static VACTION maturity_rating_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	UINT8 bID;

	bID = OSD_GetFocusID ( ( POBJECT_HEAD ) &g_win_maturity_rating);
	switch(key)
	{
/*		case V_KEY_UP:
			if (MATURITY_RATING_PWD == bID)
			{
				pin_pwd_set_status(0, FALSE);
				pin_pwd_set_str(PIN_ARRAY_0);
			}
			act = VACT_CURSOR_UP;
			break;
		case V_KEY_DOWN:
			if (MATURITY_RATING_PWD == bID)
			{
				pin_pwd_set_status(0, FALSE);
				pin_pwd_set_str(PIN_ARRAY_0);
			}
			act = VACT_CURSOR_DOWN;
			break;
*/		case V_KEY_EXIT:
			BackToFullScrPlay();
			break;
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		//case V_KEY_LEFT:
			//act = VACT_CLOSE;
			//break;
		default:
		act = VACT_PASS;
	}

	return act;	
}

static PRESULT maturity_rating_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 rating;
  	UINT32 rating_ret;
	struct help_item_resource win_mr_help[] =
	{
		{1,IM_HELP_ICON_TB,RS_HELP_SELECT},
		{0,RS_HELP_ENTER,RS_HELP_ENTER},
		{0,RS_MENU,RS_HELP_BACK},
		{0,RS_HELP_EXIT,RS_FULL_PLAY},   
	};
    
	switch(event)
	{
	case EVN_PRE_OPEN:
		rating_ret = GYCA_GetParentControl(&rating);
		/*
		if ((GYCA_CARD_ERR == rating_ret)|| (GYCA_ERR ==rating_ret))
		{
			win_popup_msg(NULL, NULL, RS_CAERR_NOSMC);
			ret = PROC_LEAVE;
			return ret;
		}
		
		else if (CDCA_RC_OK == rating_ret)
		{
			maturity_rating = rating;
		}
		else
		{
			win_popup_msg(NULL, NULL, RS_CAERR_UNKNOWN);
			ret = PROC_LEAVE;
			return ret;
		}
		*/
		//wincom_open_title_ext(RS_CA_MATURITY_RATING, IM_TITLE_ICON_SYSTEM);
		//wincom_open_help(win_mr_help, 4);
		OSD_SetEditFieldContent(&maturity_rating_edt1, STRING_NUMBER, (UINT32)rating);
		pin_pwd_init(1UL<<PIN_ARRAY_0);
		pin_pwd_set_status(0, FALSE);
		pin_pwd_set_str(PIN_ARRAY_0);
		OSD_SetContainerFocus(&g_win_maturity_rating,MATURITY_RATING_RAT_SET);
		break;
	case EVN_PRE_DRAW:
		rating = 0;
		rating_ret = GYCA_GetParentControl(&rating);
		if (DVTCA_OK == rating_ret)
		{
			OSD_SetEditFieldContent(&maturity_rating_edt1, STRING_NUMBER, (UINT32)rating);
		}
		
		pin_pwd_init(1UL<<PIN_ARRAY_0);
		pin_pwd_set_status(0, FALSE);
		pin_pwd_set_str(PIN_ARRAY_0);
		break;
	case EVN_PRE_CLOSE:
		/* Make OSD not flickering */
		*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
		break;
	case EVN_POST_CLOSE:
		break;
	case EVN_MSG_GOT:
		//ret = preview_casmsg_proc(param1, param2);
		break;
	}

	return ret;
}


