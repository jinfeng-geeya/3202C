#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
//#include <api/libosd/osd_lib.h>

#include "win_cas_com.h"

#if 0
/*******************************************************************************
* Objects declaration
*******************************************************************************/
extern CONTAINER g_win_ippv_popup;

extern CONTAINER ippv_popup_con1;
extern CONTAINER ippv_popup_con2;
extern CONTAINER ippv_popup_con3;
extern CONTAINER ippv_popup_con4;
extern CONTAINER ippv_popup_con5;
extern CONTAINER ippv_popup_con6;
extern CONTAINER ippv_popup_con7;
extern CONTAINER ippv_popup_con8;

extern TEXT_FIELD ippv_popup_txt_name1; //ippv type: ippv free/buy, ippt
extern TEXT_FIELD ippv_popup_txt_name21;//operator id
extern TEXT_FIELD ippv_popup_txt_name22;//slot id
extern TEXT_FIELD ippv_popup_txt_name31;//product ID
extern TEXT_FIELD ippv_popup_txt_name32;//expire datet
extern TEXT_FIELD ippv_popup_txt_name41;//view price
extern TEXT_FIELD ippv_popup_txt_name42;//ape price
extern TEXT_FIELD ippv_popup_txt_name5;//pin pwd

extern TEXT_FIELD ippv_popup_txt_info1;
extern TEXT_FIELD ippv_popup_txt_info21;
extern TEXT_FIELD ippv_popup_txt_info22;
extern TEXT_FIELD ippv_popup_txt_info31;
extern TEXT_FIELD ippv_popup_txt_info32;
extern TEXT_FIELD ippv_popup_txt_info41;
extern TEXT_FIELD ippv_popup_txt_info42;
extern TEXT_FIELD ippv_popup_txt_info5;

extern TEXT_FIELD ippv_popup_btn_view;
extern TEXT_FIELD ippv_popup_btn_tape;
extern TEXT_FIELD ippv_popup_btn_cancel;

static VACTION win_ippv_popup_con_keymap ( POBJECT_HEAD pObj, UINT32 key );
static PRESULT win_ippv_popup_con_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 );

static VACTION win_ippv_popup_keymap ( POBJECT_HEAD pObj, UINT32 key );
static PRESULT win_ippv_popup_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 );

#define WIN_SH_IDX	  WSTL_COMMON_BACK_2
#define WIN_HL_IDX	  WSTL_COMMON_BACK_2
#define WIN_SL_IDX	  WSTL_COMMON_BACK_2
#define WIN_GRY_IDX  WSTL_COMMON_BACK_2

#define CON_IDX	        WSTL_WIN_2

#define CON_SH_IDX   WSTL_BUTTON_2
#define CON_HL_IDX   WSTL_BUTTON_1
#define CON_SL_IDX   WSTL_BUTTON_2
#define CON_GRY_IDX  WSTL_BUTTON_2

#define TXTN_SH_IDX   WSTL_TXT_4
#define TXTN_HL_IDX   WSTL_TXT_5//WSTL_TXT_5
#define TXTN_SL_IDX   WSTL_TXT_4
#define TXTN_GRY_IDX  WSTL_TXT_10

#define TXTS_SH_IDX   WSTL_TXT_4
#define TXTS_HL_IDX   WSTL_TXT_5
#define TXTS_SL_IDX   WSTL_TXT_4
#define TXTS_GRY_IDX  WSTL_TXT_10

#define BTN_SH_IDX   WSTL_TXT_4
#define BTN_HL_IDX   WSTL_TXT_5
#define BTN_SL_IDX   WSTL_TXT_4
#define BTN_GRY_IDX  WSTL_TXT_10

#define W_L		76
#define W_T		102
#define W_W		570
#define W_H     	320

#define CON_L	(W_L + 14)
#define CON_T	(W_T + 2)
#define CON_W	(W_W - 18)
#define CON_H	28
#define CON_GAP	4

#define TXTN_L_OF  	14
#define TXTN_W  	110
#define TXTN_H		28
#define TXTN_T_OF	((CON_H - TXTN_H)/2)

#define TXTS_L_OF  	(TXTN_L_OF + TXTN_W)
#define TXTS_W		150
#define TXTS_H		28
#define TXTS_T_OF	((CON_H - TXTS_H)/2)

#define TXTN2_L_OF  (TXTS_L_OF+TXTS_W)
#define TXTN2_W  	110
#define TXTN2_H		28
#define TXTN2_T_OF	((CON_H - TXTN_H)/2)

#define TXTS2_L_OF  	(TXTN2_L_OF + TXTN2_W)
#define TXTS2_W		150
#define TXTS2_H		28
#define TXTS2_T_OF	((CON_H - TXTS_H)/2)

#define LDEF_CON(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,sh,hl,sl,gry,conobj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, sh,hl,sl,gry,   \
    win_ippv_popup_con_keymap,win_ippv_popup_con_callback,  \
    conobj, focusID,1)

#define LDEF_TXTNAME(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,align,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    align | C_ALIGN_VCENTER, 0,0,resID,str)

#define LDEF_BTN(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,sh,hl,sl,gry,align,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h,sh,hl,sl,gry,   \
    NULL,NULL,  \
    align | C_ALIGN_VCENTER, 0,0,resID,str)

#define LDEF_TXTSET(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,align,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXTS_SH_IDX,TXTS_HL_IDX,TXTS_SL_IDX,TXTS_GRY_IDX,   \
    NULL,NULL,  \
    align| C_ALIGN_VCENTER, 20,0,resID,str)

#define LDEF_MM_ITEM(root,varCon,nxtObj,varTxtName1,varTxtset1,varTxtName2,varTxtset2,ID,IDu,IDd,l,t,w,h,align,nameID1,nameID2,setstr1, setstr2)	\
	LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,&varTxtName1,1)	\
	LDEF_TXTNAME(&varCon,varTxtName1,&varTxtset1,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,align,nameID1,NULL)	\
	LDEF_TXTSET(&varCon,varTxtset1, &varTxtName2,0,0,0,0,0,l + TXTS_L_OF, t + TXTS_T_OF,TXTS_W,TXTS_H,align,0,setstr1) \
	LDEF_TXTNAME(&varCon,varTxtName2,&varTxtset2,0,0,0,0,0,l + TXTN2_L_OF, t + TXTN2_T_OF,TXTN2_W,TXTN2_H,align,nameID2,NULL)	\
	LDEF_TXTSET(&varCon,varTxtset2,  NULL,0,0,0,0,0,l + TXTS2_L_OF, t + TXTS2_T_OF,TXTS2_W,TXTS2_H,align,0,setstr2)

#define LDEF_MM_ITEM2(root,varCon,nxtObj,varTxtName,ID,IDu,IDd,l,t,w,h,sh,hl,sl,gry,align,nameID)	\
	LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,&varTxtName,1)	\
	LDEF_BTN(&varCon,varTxtName, NULL,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,w-20,TXTN_H,sh,hl,sl,gry,align, nameID, NULL)
	//LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,TXTS_SH_IDX,TXTS_HL_IDX,TXTS_SL_IDX,TXTS_GRY_IDX,&varTxtName,1)	\

#define LDEF_MM_ITEM3(root,varCon,nxtObj,varTxtName,varTxtset,ID,IDu,IDd,l,t,w,h,align, nameID,setstr)	\
	LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,&varTxtName,1)	\
	LDEF_TXTNAME(&varCon,varTxtName,&varTxtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,align,nameID,NULL)	\
	LDEF_TXTSET(&varCon,varTxtset,  NULL        ,1,1,1,1,1,l + TXTS_L_OF, t + TXTS_T_OF,TXTS_W,TXTS_H,align,0,setstr)

LDEF_MM_ITEM2( g_win_ippv_popup, ippv_popup_con1, &ippv_popup_con2, ippv_popup_txt_name1, 0, 0, 0,	\
                CON_L, CON_T +CON_GAP*3, CON_W, CON_H,TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX, \
                C_ALIGN_CENTER,RS_IPPV_FREEVIEW)

LDEF_MM_ITEM( g_win_ippv_popup, ippv_popup_con2, &ippv_popup_con3, ippv_popup_txt_name21, ippv_popup_txt_info21,  \
			ippv_popup_txt_name22, ippv_popup_txt_info22,0, 0, 0, CON_L, CON_T + CON_GAP+( CON_H + CON_GAP ) *2, CON_W, CON_H,	\
			C_ALIGN_LEFT, RS_IPPV_OPER_ID, RS_IPPV_SLOT_ID, display_strs[1], display_strs[2] )

LDEF_MM_ITEM( g_win_ippv_popup, ippv_popup_con3, &ippv_popup_con4, ippv_popup_txt_name31, ippv_popup_txt_info31,  \
			ippv_popup_txt_name32, ippv_popup_txt_info32,0, 0, 0, CON_L, CON_T + CON_GAP+( CON_H + CON_GAP ) *3, CON_W, CON_H,	\
			C_ALIGN_LEFT, RS_IPPV_PRODUCTID, RS_IPPV_EXPTIME, display_strs[3], display_strs[4] )

LDEF_MM_ITEM( g_win_ippv_popup, ippv_popup_con4, &ippv_popup_con5, ippv_popup_txt_name41, ippv_popup_txt_info41,  \
			ippv_popup_txt_name42, ippv_popup_txt_info42,0, 0, 0, CON_L, CON_T + CON_GAP+( CON_H + CON_GAP ) *4, CON_W, CON_H,	\
			C_ALIGN_LEFT, RS_IPPV_VIEW_PRICE, RS_IPPV_TAPE_PRICE, display_strs[5], display_strs[6] )

LDEF_MM_ITEM3( g_win_ippv_popup, ippv_popup_con5, &ippv_popup_con6, ippv_popup_txt_name5, ippv_popup_txt_info5, 1,4, 2,	\
                CON_L, CON_T +CON_GAP+ ( CON_H + CON_GAP ) *6, CON_W, CON_H, C_ALIGN_LEFT,RS_IPPV_PIN, display_strs[PIN_POSTION] )

LDEF_MM_ITEM2( g_win_ippv_popup, ippv_popup_con6, &ippv_popup_con7, ippv_popup_btn_view, 2, 1, 3,	\
                CON_L+CON_W/12, CON_T +CON_GAP+ ( CON_H + CON_GAP ) *8, CON_W/6, CON_H, BTN_SH_IDX,BTN_HL_IDX,BTN_SL_IDX,BTN_GRY_IDX,\
                C_ALIGN_CENTER,RS_IPPV_VIEW)
                
LDEF_MM_ITEM2( g_win_ippv_popup, ippv_popup_con7, &ippv_popup_con8, ippv_popup_btn_tape, 3, 2, 4,	\
                CON_L+CON_W*5/12, CON_T +CON_GAP+ ( CON_H + CON_GAP ) *8, CON_W/6, CON_H,  BTN_SH_IDX,BTN_HL_IDX,BTN_SL_IDX,BTN_GRY_IDX,\
                C_ALIGN_CENTER,RS_IPPV_TAPE)
                
LDEF_MM_ITEM2( g_win_ippv_popup, ippv_popup_con8, NULL, ippv_popup_btn_cancel, 4, 3, 1,	\
                CON_L+CON_W*9/12, CON_T +CON_GAP+ ( CON_H + CON_GAP ) *8, CON_W/6, CON_H,  BTN_SH_IDX,BTN_HL_IDX,BTN_SL_IDX,BTN_GRY_IDX,\
                C_ALIGN_CENTER,RS_COMMON_CANCEL)

DEF_CONTAINER ( g_win_ippv_popup, NULL, NULL, C_ATTR_ACTIVE, 0, \
                1, 0, 0, 0, 0, W_L, W_T, W_W, W_H, CON_IDX, CON_IDX, CON_IDX, CON_IDX,   \
                win_ippv_popup_keymap, win_ippv_popup_callback,  \
                ( POBJECT_HEAD ) &ippv_popup_con1, 1, 0 )
#endif

//include the header from xform 
#include "win_IPPV_popup.h"
/*******************************************************************************
*	Local functions & variables declare
*******************************************************************************/
#define IPPVPOP_PWD_ITEM		1
#define IPPVPOP_BTNVIEW_ITEM		2
#define IPPVPOP_BTNTAPE_ITEM		3
#define IPPVPOP_BTNCANCEL_ITEM	4

UINT8 view_id = 0;
UINT8 tape_id = 0;
static UINT8	ippvpop_status = FALSE;
static SCDCAIppvBuyInfo buyinfo_prev = {0,0,0,0,{0,0,0,0,0,0},0};
static UINT32 ippvstatus = 0xff;
static UINT8 exit_order = FALSE;

/*To read/set the ippv popup status
 * rorw: 0-read, 1 write, value: FALSE-closed, TRUE-opening
 */
void win_ippv_popup_status(UINT8 rorw, UINT8 *value)
{
	if (rorw)
	{
		ippvpop_status = *value;
	}
	else
		*value = ippvpop_status;

	return;
}

/*call this function to see if the request window has the same info with previous opening window,
/if the same, don't open it again
*/
UINT8  win_ippv_check_request()
{
	UINT8 tmp;
	UINT32 status;
	SCDCAIppvBuyInfo buyinfo;
	CDSTBCA_GetIppvBuyInfo(&buyinfo, &status);
	win_ippv_popup_status(0, &tmp);
	if ((TRUE == tmp) //current is opening
		&&(0 == MEMCMP(&buyinfo_prev, &buyinfo, sizeof(SCDCAIppvBuyInfo)))//same information
		&&(status == ippvstatus))//same segment
	{
		return 0;
	}
	else
		return 1;
}

//0: not to update directly, 1 update immediately
static UINT8 win_ippv_popup_display(UINT8 update)
{
	UINT8 i;
	UINT8 temp[20] = {0,};
	UINT16 viewprice=0, tapeprice=0;
	UINT32 status, strid;
	PTEXT_FIELD ptxt;
	SCDCAIppvBuyInfo buyinfo;

	CDSTBCA_GetIppvBuyInfo(&buyinfo, &status);
	//save the current status
	MEMCPY(&buyinfo_prev, &buyinfo, sizeof(SCDCAIppvBuyInfo));
	ippvstatus = status;
	//get current IPPV stage
	switch(status&0xff)
	{
		case CDCA_IPPV_FREEVIEWED_SEGMENT:
			strid = RS_IPPV_FREEVIEW;
			break;
		case CDCA_IPPV_PAYVIEWED_SEGMENT:
			strid = RS_IPPV_PAYVIEW;
			break;
		case CDCA_IPPT_PAYVIEWED_SEGMENT:
			strid = RS_IPPT_PAYVIEW;
			break;
		default:
			strid = 0;
			break;
	}

	//get the price
	for (i=0;i<buyinfo.m_byPriceNum;i++)
	{
		if(CDCA_IPPVPRICETYPE_TPPVVIEW == buyinfo.m_Price[i].m_byPriceCode)
			viewprice = buyinfo.m_Price[i].m_wPrice;
		else if (CDCA_IPPVPRICETYPE_TPPVVIEWTAPING == buyinfo.m_Price[i].m_byPriceCode)
			tapeprice = buyinfo.m_Price[i].m_wPrice;
	}
	
	ptxt = &ippv_popup_txt_name1;
	OSD_SetTextFieldContent(ptxt, STRING_ID, strid);

	ptxt= &ippv_popup_txt_info21;
	OSD_SetTextFieldContent(ptxt, STRING_NUMBER, (UINT32)buyinfo.m_wTvsID);

	ptxt= &ippv_popup_txt_info22;
	OSD_SetTextFieldContent(ptxt, STRING_NUMBER, (UINT32)buyinfo.m_bySlotID);

	ptxt= &ippv_popup_txt_info31;
	OSD_SetTextFieldContent(ptxt, STRING_NUMBER, (UINT32)buyinfo.m_dwProductID);

	ptxt = &ippv_popup_txt_name32;
	if (CDCA_IPPT_PAYVIEWED_SEGMENT == (status&0xff))
	{
		OSD_SetTextFieldContent(ptxt, STRING_ID, (UINT32)RS_IPPV_PRICEUNIT);
	}
	else
	{
		OSD_SetTextFieldContent(ptxt, STRING_ID, (UINT32)RS_IPPV_EXPTIME);
	}
	
	ptxt= &ippv_popup_txt_info32;
	if(CDCA_IPPT_PAYVIEWED_SEGMENT == (status&0xff))
	{
		sprintf(temp, "%d Min", buyinfo.m_wExpiredDate);
		OSD_SetTextFieldContent(ptxt, STRING_ANSI, (UINT32)temp);
	}
	else
	{
		YMD_calculate(buyinfo.m_wExpiredDate,temp);
		OSD_SetTextFieldContent(ptxt, STRING_ANSI, (UINT32)temp);
	}

	ptxt= &ippv_popup_txt_info41;
	if (0 == viewprice)
		OSD_SetTextFieldContent(ptxt, STRING_ANSI, (UINT32)"");
	else
		OSD_SetTextFieldContent(ptxt, STRING_NUMBER, (UINT32)viewprice);

	ptxt= &ippv_popup_txt_info42;
	if (0 == tapeprice)
		OSD_SetTextFieldContent(ptxt, STRING_ANSI, (UINT32)"");
	else
		OSD_SetTextFieldContent(ptxt, STRING_NUMBER, (UINT32)tapeprice);
	
	if (update)
	{
		OSD_DrawObject((POBJECT_HEAD)&g_win_ippv_popup,C_UPDATE_ALL);
		OSD_TrackObject((POBJECT_HEAD)&ippv_popup_con5, C_UPDATE_ALL);
	}
	return 1;
}

extern UINT8 pin_pwd_password_str[MAX_PIN_PWD_ARRAY][MAX_PIN_PWD_LEN];

// order: 1--order, 0-- cancel
static PRESULT IPPVPOP_program_order(UINT8 order, UINT8 tape)
{
	UINT8 buy = TRUE, i, type;
	UINT16 ecmpid, ret;
	UINT32 status;
	SCDCAIPPVPrice price;
	SCDCAIppvBuyInfo buyinfo;


	CDSTBCA_GetIppvBuyInfo(&buyinfo, &status);

	if (order)
	{
		type = (tape)?CDCA_IPPVPRICETYPE_TPPVVIEWTAPING:CDCA_IPPVPRICETYPE_TPPVVIEW;
		for (i=0;i<buyinfo.m_byPriceNum;i++)
		{
			if(type == buyinfo.m_Price[i].m_byPriceCode)
				break;
		}
		if (i == buyinfo.m_byPriceNum)
		{
			//No proper price, return and notify the user
			win_popup_msg(NULL, NULL, RS_IPPV_MSG_NOVIEWPRICE);
			return PROC_LOOP;
		}
		else
		{
			price.m_byPriceCode = type;
			price.m_wPrice = buyinfo.m_Price[i].m_wPrice;
		}
		
		ecmpid = ((status>>8)&0xffff);
		ret = CDCASTB_StopIPPVBuyDlg(buy, ecmpid, pin_pwd_password_str[PIN_ARRAY_0], &price);

		if ((CDCA_RC_POINTER_INVALID == ret ) || (CDCA_RC_CARD_INVALID == ret))
		{
			win_popup_msg(NULL, NULL, RS_CAERR_NOSMC);
			return PROC_LOOP;
		}
		else if (CDCA_RC_CARD_NO_ROOM == ret)
		{
			win_popup_msg(NULL, NULL, RS_CAERR_NO_ROOM);
			return PROC_LOOP;
		}
		else if (CDCA_RC_PROG_STATUS_INVALID == ret)	
		{
			win_popup_msg(NULL, NULL, RS_CAERR_PROG_STATUS_INVALID);
			return PROC_LOOP;
		}
		else if (CDCA_RC_PIN_INVALID == ret)
		{
			win_popup_msg(NULL, NULL, RS_CAERR_PIN_ERROR_TF);
			return PROC_LOOP;
		}
		else if (CDCA_RC_DATA_NOT_FIND == ret)
		{
			win_popup_msg(NULL, NULL, RS_CAERR_DATA_NOT_FOUND);
			return PROC_LOOP;
		}
		else if (CDCA_RC_OK == ret)
		{
			win_popup_msg(NULL, NULL, RS_CAERR_SET_SUCCESS);
			return PROC_LEAVE;
		}
		else
		{
			win_popup_msg(NULL, NULL, RS_CAERR_UNKNOWN);
			return PROC_LOOP;
		}
	}
	else
	{
		ecmpid = ((status>>8)&0xffff);
		buy = FALSE;
		CDCASTB_StopIPPVBuyDlg(buy, ecmpid, NULL, NULL);

		return PROC_LEAVE;
	}
	
}

/*******************************************************************************
*	Window's keymap, proc and  callback
*******************************************************************************/
static VACTION win_ippv_popup_con_keymap ( POBJECT_HEAD pObj, UINT32 key )
{
	VACTION act;

	switch ( key )
	{
		case V_KEY_ENTER:
			act = VACT_ENTER;
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
	}

	return act;
}
static UINT32 key_exit[] ={V_KEY_EXIT};
static PRESULT win_ippv_popup_con_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 )
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	UINT8 tmp, bitnum, status;

	UINT8 bID = OSD_GetFocusID ( ( POBJECT_HEAD ) &g_win_ippv_popup); 
	
	switch(event)
	{
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1>>16);
			if(unact == VACT_ENTER) 
			{
				if (IPPVPOP_BTNCANCEL_ITEM == bID)
				{
					ret = IPPVPOP_program_order(FALSE, 0);
				}
				else if (IPPVPOP_BTNTAPE_ITEM == bID)
				{
					ret = IPPVPOP_program_order(TRUE, 1);
				}
				else if (IPPVPOP_BTNVIEW_ITEM == bID) 
				{
					ret = IPPVPOP_program_order(TRUE, 0);
				}

				if (PROC_LOOP == ret)
				{
					pin_pwd_init(1UL<<PIN_ARRAY_0);
					pin_pwd_set_status(0, FALSE);
					pin_pwd_set_str(PIN_ARRAY_0);
					//set the focus on password
					OSD_ChangeFocus ( ( POBJECT_HEAD ) &g_win_ippv_popup, 1, C_UPDATE_ALL );
					OSD_TrackObject ((POBJECT_HEAD)&ippv_popup_con5, C_UPDATE_ALL );
					OSD_DrawObject ((POBJECT_HEAD)&ippv_popup_con6, C_UPDATE_ALL );
					OSD_DrawObject ((POBJECT_HEAD)&ippv_popup_con7, C_UPDATE_ALL );
					OSD_DrawObject ((POBJECT_HEAD)&ippv_popup_con8, C_UPDATE_ALL );
				}

				break;
			}
			else if ((unact <= VACT_NUM_9 )&&(IPPVPOP_PWD_ITEM == bID))
			{	
				pin_pwd_get_status(&bitnum, &status);
				if ((status == TRUE ) && ( bitnum == 5 ) )
				{
					pin_pwd_input(PIN_ARRAY_0, ( UINT8 ) ( unact - VACT_NUM_0 ) );
					pin_pwd_set_str(PIN_ARRAY_0);
					OSD_ChangeFocus ( ( POBJECT_HEAD ) &g_win_ippv_popup, IPPVPOP_BTNVIEW_ITEM, C_UPDATE_ALL );
					OSD_DrawObject (pObj, C_UPDATE_ALL );
					OSD_TrackObject ((POBJECT_HEAD)&ippv_popup_con6, C_UPDATE_ALL );
					break;
				}
				pin_pwd_input(PIN_ARRAY_0, (UINT8) (unact - VACT_NUM_0) );
				pin_pwd_set_str(PIN_ARRAY_0);
				OSD_TrackObject ( pObj, C_UPDATE_ALL );	
				ret = PROC_LOOP;
			}
			break;
		default:
			break;
	}

	return ret;
}

static VACTION win_ippv_popup_keymap ( POBJECT_HEAD pObj, UINT32 vkey )
{
	VACTION act = VACT_PASS;

	switch ( vkey )
	{
		case V_KEY_UP:
			act = VACT_INCREASE;
			break;
		case V_KEY_DOWN:
			act = VACT_DECREASE;
			break;
		case V_KEY_RIGHT:
			act = VACT_CURSOR_DOWN;
			break;
		case V_KEY_LEFT:
			act = VACT_CURSOR_UP;
			break;
		//case V_KEY_MENU:
		case V_KEY_EXIT:
			exit_order = TRUE;
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}

static PRESULT win_ippv_popup_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 )
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	UINT8 tmp = 0, value=TRUE;
	static UINT8 flag = 0; //flag to re-send up/down key
	UINT32 hkey;
	OSD_RECT title_old = {17, 20, 576, 40};//this is the normal definition of tilte pos
	OSD_RECT title_ippv = {17, 60, 576, 40};// move to a new pos to show mail icon/detitle icon/finger together
	
	switch ( event )
	{
		case EVN_PRE_OPEN:
			ap_cas_message_close();
			
			exit_order = FALSE;
			pin_pwd_init(1UL<<PIN_ARRAY_0);
			pin_pwd_set_status(0, FALSE);
			pin_pwd_set_str(PIN_ARRAY_0);
			//set the focus on password
			OSD_SetContainerFocus(&g_win_ippv_popup, IPPVPOP_PWD_ITEM); 
			
			win_ippv_popup_display(0);
			win_ippv_popup_status(1, &value);
			break;
		case EVN_POST_OPEN:
			break;
		case EVN_PRE_DRAW:
			break;
		case EVN_PRE_CLOSE:
			if (TRUE == exit_order)
			{
				IPPVPOP_program_order(FALSE, 0);
			}
			break;
		case EVN_POST_CLOSE:
			value = FALSE;
			win_ippv_popup_status(1, &value);
			if ((POBJECT_HEAD)&g_win_ippv_popup == menu_stack_get_top())
				menu_stack_pop();
			MEMSET(&buyinfo_prev, 0, sizeof(SCDCAIppvBuyInfo));
			ippvstatus = 0xff;
			if (1 == flag)
			{
				ap_vk_to_hk(0, V_KEY_UP, &hkey);
				ap_send_msg (CTRL_MSG_SUBTYPE_KEY, hkey, FALSE );
			}
			else if (2 == flag)
			{
				ap_vk_to_hk(0, V_KEY_DOWN, &hkey);
				ap_send_msg (CTRL_MSG_SUBTYPE_KEY, hkey, FALSE );
			}
			flag = 0;
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1>>16);
			if (VACT_INCREASE == unact) 
			{
				flag = 1;
			}
			else if (VACT_DECREASE == unact)
			{
				flag = 2;
			}
			ret = PROC_LEAVE;
			break;
	}
	return ret;
}





