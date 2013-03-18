#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libcas/gy/gyca_def.h>
#include <api/libcas/gy/gyca_interface.h>
#include <api/libcas/gy/cas_gy.h>

#include "win_cas_com.h"
#include "win_ca_pinpwd.h"
/*******************************************************************************
*	Objects definition
*******************************************************************************
extern CONTAINER g_win_pin_pwd;
extern CONTAINER win_pin_pwd_con;

extern CONTAINER pin_pwd_item_con1;
extern CONTAINER pin_pwd_item_con2;
extern CONTAINER pin_pwd_item_con3;

extern TEXT_FIELD pin_pwd_item_txtname1;
extern TEXT_FIELD pin_pwd_item_txtname2;
extern TEXT_FIELD pin_pwd_item_txtname3;

extern TEXT_FIELD pin_pwd_item_txtset1;
extern TEXT_FIELD pin_pwd_item_txtset2;
extern TEXT_FIELD pin_pwd_item_txtset3;

static VACTION pin_pwd_item_con_keymap ( POBJECT_HEAD pObj, UINT32 key );
static PRESULT pin_pwd_item_con_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 );

static VACTION pin_pwd_con_keymap ( POBJECT_HEAD pObj, UINT32 key );
static PRESULT pin_pwd_con_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 );

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
    pin_pwd_item_con_keymap,pin_pwd_item_con_callback,  \
    conobj, focusID,1)

#define LDEF_TXTNAME(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 0,0,resID,str)

#define LDEF_TXTSET(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXTS_SH_IDX,TXTS_HL_IDX,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 20,0,resID,str)

#define LDEF_MM_ITEM(root,varCon,nxtObj,varTxtName,varTxtset,ID,IDu,IDd,l,t,w,h,nameID,setstr)	\
	LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,&varTxtName,1)	\
	LDEF_TXTNAME(&varCon,varTxtName,&varTxtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,nameID,NULL)	\
	LDEF_TXTSET(&varCon,varTxtset,  NULL        ,1,1,1,1,1,l + TXTS_L_OF, t + TXTS_T_OF,TXTS_W,TXTS_H,0,setstr)

LDEF_MM_ITEM( g_win_pin_pwd, pin_pwd_item_con1, &pin_pwd_item_con2, pin_pwd_item_txtname1, pin_pwd_item_txtset1, 1, 2, 3,	\
                CON_L, CON_T + ( CON_H + CON_GAP ) *0, CON_W, CON_H,	RS_OLD_PIN, display_strs[PIN_POSTION] )
LDEF_MM_ITEM( g_win_pin_pwd, pin_pwd_item_con2, &pin_pwd_item_con3, pin_pwd_item_txtname2, pin_pwd_item_txtset2, 2, 3, 1,	\
                CON_L, CON_T + ( CON_H + CON_GAP ) *1, CON_W, CON_H,	RS_SYSTME_PARENTAL_LOCK_NEW_PASSWORD, display_strs[PIN_POSTION+1] )
LDEF_MM_ITEM( g_win_pin_pwd, pin_pwd_item_con3, NULL, pin_pwd_item_txtname3, pin_pwd_item_txtset3, 3, 1, 2,	\
                CON_L, CON_T + ( CON_H + CON_GAP ) *2, CON_W, CON_H,	RS_SYSTME_PARENTAL_LOCK_CONFIRM_PASSWORD, display_strs[PIN_POSTION+2] )

DEF_CONTAINER ( win_pin_pwd_con, NULL, &pin_pwd_item_con1, C_ATTR_ACTIVE, 0, \
                0, 0, 0, 0, 0, W_CON_L, W_CON_T, W_CON_W, W_CON_H, CON_IDX, CON_IDX, CON_IDX, CON_IDX,   \
                NULL, NULL,  \
                NULL, 1, 0 )
DEF_CONTAINER ( g_win_pin_pwd, NULL, NULL, C_ATTR_ACTIVE, 0, \
                1, 0, 0, 0, 0, W_L, W_T, W_W, W_H, WIN_SH_IDX, WIN_SH_IDX, WIN_SH_IDX, WIN_SH_IDX,   \
                pin_pwd_con_keymap, pin_pwd_con_callback,  \
                ( POBJECT_HEAD ) &win_pin_pwd_con, 1, 0 )
*/

/*******************************************************************************
*	Local functions & variables define
*******************************************************************************/
#define PIN_PWD_OLD_PIN     1
#define PIN_PWD_NEW_PIN     2
#define PIN_PWD_CONFIRM_PIN 3


CONTAINER* pin_pwd_items[] =
    {
        &pin_pwd_con_item1,
        &pin_pwd_con_item2,
        &pin_pwd_con_item3,
    };

POBJECT_HEAD pin_pwd_item_name[] =
    {
        ( POBJECT_HEAD ) &pin_pwd_item_txtname1,
        ( POBJECT_HEAD ) &pin_pwd_item_txtname2,
        ( POBJECT_HEAD ) &pin_pwd_item_txtname3,
    };

POBJECT_HEAD pin_pwd_item_set[] =
    {
        ( POBJECT_HEAD ) &pin_pwd_item_txtset1,
        ( POBJECT_HEAD ) &pin_pwd_item_txtset2,
        ( POBJECT_HEAD ) &pin_pwd_item_txtset3,
    };

void win_pin_pwd_switch_confirm ( BOOL flag ,UINT8 bID)
{
	UINT8 action;

	if ( flag )
		action = C_ATTR_ACTIVE;
	else
		action = C_ATTR_INACTIVE;

	if ( !OSD_CheckAttr ( ( POBJECT_HEAD )pin_pwd_items[bID-1], action ) )
	{
		OSD_SetAttr ( ( POBJECT_HEAD )pin_pwd_items[bID-1], action );
		OSD_SetAttr ( ( POBJECT_HEAD )pin_pwd_item_name[bID-1], action );
		OSD_SetAttr ( ( POBJECT_HEAD )pin_pwd_item_set[bID-1], action );
		OSD_DrawObject ( ( POBJECT_HEAD ) pin_pwd_items[bID-1], C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL );
	}

}


/*******************************************************************************
*	key mapping and event callback definition
*******************************************************************************/
#define VACT_DRAWBACK (VACT_PASS+1)
static VACTION pin_pwd_item_keymap ( POBJECT_HEAD pObj, UINT32 key )
{
	VACTION act;
	act = VACT_PASS;
	switch ( key )
	{
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
		case V_KEY_LEFT:
			act = VACT_DRAWBACK;
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
			act = VACT_NUM_0 + ( key - V_KEY_0 );
			break;
		default:
			act = VACT_PASS;
			break;
	}

	return act;

}


//0 failure, 1 success or to return from current window
extern UINT8	pin_pwd_password_str[MAX_PIN_PWD_ARRAY][PIN_PWD_LEN];
UINT8 win_pin_pwd_enter_proc(UINT8 bID)
{
	UINT8 ret = 0, id = 0;
	INT32 pin_ret,i;

	pin_ret = pin_pwd_is_same(PIN_PWD_NEW_PIN-1,PIN_PWD_CONFIRM_PIN-1,PIN_PWD_LEN);
	if (FALSE == pin_ret)
	{
		win_popup_msg(NULL, NULL, RS_PWD_CONFIRM_ERROR);

		pin_pwd_set_status(0, FALSE);
		pin_pwd_set_str(PIN_ARRAY_1);
		pin_pwd_set_str(PIN_ARRAY_2);
		win_pin_pwd_switch_confirm ( TRUE,PIN_PWD_NEW_PIN);
		win_pin_pwd_switch_confirm ( FALSE ,PIN_PWD_CONFIRM_PIN);
		OSD_ChangeFocus ( ( POBJECT_HEAD ) &g_win_pin_pwd, PIN_PWD_NEW_PIN, C_UPDATE_ALL );
		OSD_DrawObject ( ( POBJECT_HEAD ) pin_pwd_items[2], C_UPDATE_ALL );
		OSD_TrackObject ( ( POBJECT_HEAD ) pin_pwd_items[1], C_UPDATE_ALL );
		return 0;
	}
	pin_ret = GYCA_ChangePin(pin_pwd_password_str[PIN_PWD_OLD_PIN-1], pin_pwd_password_str[PIN_PWD_NEW_PIN-1]);
	if ((GYCA_CARD_ERR == pin_ret)||(GYCA_ERR == pin_ret))
	{
		win_popup_msg(NULL, NULL, RS_CAERR_NOSMC);
		return 1; //return from current window
	}
	else if (GYCA_PIN_ERR == pin_ret)
	{
		win_popup_msg(NULL, NULL, RS_CAERR_PIN_ERROR);
		//recover
		
		pin_pwd_set_status(0, FALSE);
		pin_pwd_set_str(PIN_ARRAY_0);
		pin_pwd_set_str(PIN_ARRAY_1);
		pin_pwd_set_str(PIN_ARRAY_2);
		OSD_ChangeFocus ( ( POBJECT_HEAD ) &g_win_pin_pwd, PIN_PWD_OLD_PIN, 0 );
		OSD_DrawObject ( ( POBJECT_HEAD ) pin_pwd_items[1], C_UPDATE_ALL );
		OSD_DrawObject ( ( POBJECT_HEAD ) pin_pwd_items[2], C_UPDATE_ALL );
		OSD_TrackObject ( ( POBJECT_HEAD ) pin_pwd_items[0], C_UPDATE_ALL );
		return 0;
	}
	else if (GYCA_OK == pin_ret)
	{
		win_popup_msg(NULL, NULL, RS_CAERR_SET_SUCCESS);
		return 1;
	}
	else
	{
		win_popup_msg(NULL, NULL, RS_CAERR_UNKNOWN);
		return 0;
	}
	
    return ret;
}

static PRESULT pin_pwd_item_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 )
{
	PRESULT ret = PROC_PASS;
	UINT8	unact, bitnum=0, status=0;
	UINT8	id = OSD_GetFocusID ( ( POBJECT_HEAD ) &g_win_pin_pwd);
	//SYSTEM_DATA* p_sys_data = sys_data_get();

	switch ( event )
	{
		case EVN_FOCUS_PRE_GET:
			break;

		case EVN_FOCUS_PRE_LOSE:
			break;

		case EVN_UNKNOWN_ACTION:
			unact = ( VACTION ) ( param1 >> 16 ) ;
			if(unact == VACT_ENTER)
			{
				if(win_pin_pwd_enter_proc(id))
				return PROC_LEAVE;
			}
			else if (unact == VACT_DRAWBACK)
			{
				pin_pwd_drawback(id-1);
				OSD_TrackObject (pObj, C_UPDATE_ALL );
			}
			else if ( unact <= VACT_NUM_9 )
			{
				pin_pwd_get_status(&bitnum, &status);
				if ( ( status == TRUE ) && ( bitnum == 5 ) )
				{
					if ((PIN_PWD_OLD_PIN == id) ||(PIN_PWD_NEW_PIN == id))
					{
						pin_pwd_input( id-1, ( UINT8 ) ( unact - VACT_NUM_0 ) );
						pin_pwd_set_str( id-1 );
						win_pin_pwd_switch_confirm ( TRUE,id+1);
						pin_pwd_set_status(0, FALSE);
						pin_pwd_init(1<<id);
						pin_pwd_set_str(id);
						OSD_ChangeFocus ( ( POBJECT_HEAD ) &g_win_pin_pwd, id+1, C_UPDATE_ALL);
						OSD_DrawObject ( ( POBJECT_HEAD ) pin_pwd_items[id-1], C_UPDATE_ALL );
						OSD_TrackObject ( ( POBJECT_HEAD ) pin_pwd_items[id], C_UPDATE_ALL );
					}
					else if (PIN_PWD_CONFIRM_PIN == id)
					{
						pin_pwd_input( id-1, ( UINT8 ) ( unact - VACT_NUM_0 ) );
						pin_pwd_set_str ( id-1 );
						OSD_TrackObject ( ( POBJECT_HEAD ) pin_pwd_items[id-1], C_UPDATE_ALL );
					}
				}
				else
				{
					pin_pwd_input(id - 1, ( UINT8 ) ( unact - VACT_NUM_0 ) );
					pin_pwd_set_str ( id - 1);
					OSD_TrackObject ( pObj, C_UPDATE_ALL );
				}
			}
			ret = PROC_LOOP;
			break;
	}
	return ret;

}

static VACTION pin_pwd_con_keymap ( POBJECT_HEAD pObj, UINT32 key )
{
	VACTION act = VACT_PASS;
    UINT8 bID;

    bID = OSD_GetFocusID ( ( POBJECT_HEAD ) &g_win_pin_pwd);
	switch ( key )
	{

		case V_KEY_UP:
			if (bID == PIN_PWD_NEW_PIN)
			{
				pin_pwd_set_status(0, FALSE);
				pin_pwd_set_str(PIN_ARRAY_0);
				pin_pwd_set_str(PIN_ARRAY_1);
				win_pin_pwd_switch_confirm ( TRUE,PIN_PWD_OLD_PIN);
				win_pin_pwd_switch_confirm ( FALSE ,PIN_PWD_NEW_PIN);
				OSD_ChangeFocus ( ( POBJECT_HEAD ) &g_win_pin_pwd, PIN_PWD_OLD_PIN, C_UPDATE_ALL );
				OSD_DrawObject ( ( POBJECT_HEAD ) pin_pwd_items[1], C_UPDATE_ALL );
				OSD_TrackObject ( ( POBJECT_HEAD ) pin_pwd_items[0], C_UPDATE_ALL );
				break;
			}
			else if ( bID == PIN_PWD_CONFIRM_PIN)
			{
				pin_pwd_set_status(0, FALSE);
				pin_pwd_set_str(PIN_ARRAY_1);
				pin_pwd_set_str(PIN_ARRAY_2);
				win_pin_pwd_switch_confirm ( TRUE,PIN_PWD_NEW_PIN);
				win_pin_pwd_switch_confirm ( FALSE ,PIN_PWD_CONFIRM_PIN);
				OSD_ChangeFocus ( ( POBJECT_HEAD ) &g_win_pin_pwd, PIN_PWD_NEW_PIN, C_UPDATE_ALL );
				OSD_DrawObject ( ( POBJECT_HEAD ) pin_pwd_items[2], C_UPDATE_ALL );
				OSD_TrackObject ( ( POBJECT_HEAD ) pin_pwd_items[1], C_UPDATE_ALL );
				break;
			}
		case V_KEY_DOWN:
			break;

		case V_KEY_EXIT:
			//BackToFullScrPlay();
			///break;
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
			break;
	}

	return act;
}

struct help_item_resource win_cas_pin_help[] =
{
    {1,IM_HELP_ICON_TB,RS_HELP_MOVE},
    {0,RS_HELP_ENTER,RS_HELP_SELECT},
};

static PRESULT pin_pwd_con_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 )
{
	PRESULT ret = PROC_PASS;
    UINT8 rating;
    UINT16 rating_ret;

	switch ( event )
	{
		case EVN_PRE_OPEN:
            rating_ret = GYCA_GetParentControl(&rating);
			/*
    		if ((CDCA_RC_CARD_INVALID == rating_ret)|| (CDCA_RC_POINTER_INVALID ==rating_ret))
    		{
    			win_popup_msg(NULL, NULL, RS_CAERR_NOSMC);
    		////	    return PROC_LEAVE;
    		}
    		else if (CDCA_RC_OK != rating_ret)
    		{
    			win_popup_msg(NULL, NULL, RS_CAERR_UNKNOWN);
    			    return PROC_LEAVE;
    		}
    		*/
			///wincom_open_title_ext(RS_MODIFY_PIN, IM_TITLE_ICON_SYSTEM);
			//wincom_open_help(win_cas_pin_help, 2);
			win_pin_pwd_switch_confirm ( TRUE,PIN_PWD_OLD_PIN);
			win_pin_pwd_switch_confirm ( FALSE ,PIN_PWD_NEW_PIN);
			win_pin_pwd_switch_confirm ( FALSE ,PIN_PWD_CONFIRM_PIN);
			OSD_ChangeFocus ( ( POBJECT_HEAD ) &g_win_pin_pwd, 1, 0 );//
			pin_pwd_init(1UL<<PIN_ARRAY_0 |1UL<<PIN_ARRAY_1|1UL<<PIN_ARRAY_2);
			pin_pwd_set_status(0, FALSE);
			pin_pwd_set_str(PIN_ARRAY_0);
			pin_pwd_set_str(PIN_ARRAY_1);
			pin_pwd_set_str(PIN_ARRAY_2);
			break;

		case EVN_POST_OPEN:
          ///  wincom_open_preview_default();
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			break;
	}
	return ret;
}


