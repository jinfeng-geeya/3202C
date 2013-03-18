#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
//#include <api/libosd/osd_lib.h>
#include <api/libcas/gy/cas_gy.h>
#include <api/libcas/gy/gyca_interface.h>
#include <api/libcas/gy/gyca_def.h>


#include "win_cas_com.h"

/*******************************************************************************
*	Objects definition THIS is the POPUP IPP window
*******************************************************************************/
extern CONTAINER g_win_ippbuy;

extern CONTAINER ippbuy_item0;
extern CONTAINER ippbuy_item1;
extern CONTAINER ippbuy_item2;

extern TEXT_FIELD ippbuy_idx0;
extern TEXT_FIELD ippbuy_idx1;
extern TEXT_FIELD ippbuy_idx2;

extern TEXT_FIELD ippbuy_name0;
extern TEXT_FIELD ippbuy_name1;
extern EDIT_FIELD ipp_name3;

extern CONTAINER ippbuy_detail_con;
extern TEXT_FIELD ippbuy_detail_label;
extern TEXT_FIELD ippbuy_detail_info;
extern TEXT_FIELD ippbuy_help_info;


static VACTION ippbuy_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT ippbuy_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION ippbuy_con_period_keymap(POBJECT_HEAD pObj, UINT32 key);
static VACTION ippbuy_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT ippbuy_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
static UINT16 ipp_period[1];

#define WIN_SH_IDX	  WSTL_COMMON_BACK_2
#define WIN_HL_IDX	  WSTL_COMMON_BACK_2
#define WIN_SL_IDX	  WSTL_COMMON_BACK_2
#define WIN_GRY_IDX  WSTL_COMMON_BACK_2

#define CON_IDX	        WSTL_WIN_2

#define LST_SH_IDX		WSTL_NOSHOW_IDX//WSTL_BUTTON_01
#define LST_HL_IDX		WSTL_NOSHOW_IDX//WSTL_BUTTON_01
#define LST_SL_IDX		WSTL_NOSHOW_IDX//WSTL_BUTTON_01
#define LST_GRY_IDX		WSTL_NOSHOW_IDX//WSTL_BUTTON_01

#define CON_SH_IDX   WSTL_BUTTON_2
#define CON_HL_IDX   WSTL_BUTTON_1
#define CON_SL_IDX   WSTL_BUTTON_2
#define CON_GRY_IDX  WSTL_BUTTON_2

#define TXTI_SH_IDX   WSTL_TXT_4
#define TXTI_HL_IDX   WSTL_TXT_5//WSTL_TXT_5
#define TXTI_SL_IDX   WSTL_TXT_4
#define TXTI_GRY_IDX  WSTL_TXT_10

#define TXTN_SH_IDX   WSTL_TXT_7//WSTL_TXT_4
#define TXTN_HL_IDX   WSTL_TXT_5
#define TXTN_SL_IDX   WSTL_TXT_4
#define TXTN_GRY_IDX  WSTL_TXT_10

#define W_L		66
#define W_T		90
#define W_W		604
#define W_H     	400

#define LST_L	(W_L + 14)
#define LST_T	(W_T + 10)
#define LST_W	(W_W - 22)
#define LST_H	300
#define LST_GAP 4

#define ITEM_L	(LST_L + 0)
#define ITEM_T	(LST_T + 2)
#define ITEM_W	(LST_W - 0)
#define ITEM_H	28
#define ITEM_GAP	2

#define ITEM_IDX_L  2
#define ITEM_IDX_W  160
#define ITEM_NAME_L  (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_NAME_W  400//180

#define LDEF_CON(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    ippbuy_con_keymap, ippbuy_con_callback, conobj, 2,1)//1,1)
    
#define LDEF_TXTIDX(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXTI_SH_IDX,TXTI_HL_IDX,TXTI_SL_IDX,TXTI_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,resID,str)

#define LDEF_TXTNAME(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXTI_SH_IDX,TXTI_HL_IDX,TXTI_SL_IDX,TXTI_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 10,0,resID,str)

#define LDEF_EDIT(root, varNum, nxtObj, ID, l, t, w, h,style,cursormode,pat,sub,str)	\
    DEF_EDITFIELD(varNum,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,ID,ID,ID,ID, l,t,w,h, TXTI_SH_IDX,WSTL_BUTTON_3,TXTI_SL_IDX,TXTI_GRY_IDX,   \
    ippbuy_con_period_keymap, NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,style,pat,10,cursormode,NULL,sub,str)
//LDEF_LIST_ITEM(g_win_ippbuy,ippbuy_item0, &ippbuy_item1, ippbuy_idx0,ippbuy_name0,  1,3,2,\

#define LDEF_LIST_ITEM(root,varCon,nextcon,varTxtidx,varTxtName,ID,IDu,IDd,l,t,w,h,idx,namestr)	\
	LDEF_CON(&root,varCon,nextcon,ID,ID,ID,IDu,IDd,l,t,w,h,&varTxtidx,1)	\
	LDEF_TXTIDX(&varCon,varTxtidx,&varTxtName ,0,0,0,0,0,l + ITEM_IDX_L, t,ITEM_IDX_W,h,idx,NULL)	\
	LDEF_TXTNAME(&varCon,varTxtName,NULL,1,1,1,1,1,l + ITEM_NAME_L, t,ITEM_NAME_W,h,0,namestr)

#define LDEF_LIST_ITEM2(root,varCon,nextcon,varTxtidx,varTxtName,ID,IDu,IDd,l,t,w,h,idx,style,curMode,pat,namestr)	\
	LDEF_CON(&root,varCon,nextcon,ID,ID,ID,IDu,IDd,l,t,w,h,&varTxtidx,1)	\
	LDEF_TXTIDX(&varCon,varTxtidx,&varTxtName ,0,0,0,0,0,l + ITEM_IDX_L, t,ITEM_IDX_W,h,idx,NULL)	\
	LDEF_EDIT(&varCon,varTxtName,NULL,1,l + ITEM_NAME_L, t,ITEM_NAME_W,h,style,curMode,pat,NULL,namestr)

#define LDEF_DETAIL_ITEM(root,varCon,varTxtidx,varTxtName,ID,l,t,w,h,idx,namestr)	\
	LDEF_CON(&root,varCon,NULL,ID,ID,ID,ID,ID,l,t,w,h,&varTxtidx,1)	\
	LDEF_TXTIDX(&varCon,varTxtidx,&varTxtName ,0,0,0,0,0,l + ITEM_IDX_L, t,ITEM_IDX_W,h,idx,NULL)	\
	LDEF_TXTNAME(&varCon,varTxtName,NULL,1,1,1,1,1,l + ITEM_NAME_L, t,ITEM_NAME_W,h,0,namestr)

#define LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, CON_IDX,CON_IDX,CON_IDX,CON_IDX,   \
    ippbuy_keymap,ippbuy_callback,  \
    nxtObj, focusID,0)

#if 0
CONTAINER g_win_progname = {
	.head = {
		.bType = OT_CONTAINER,
		.bAttr = C_ATTR_ACTIVE, .bFont = 0,
		.bID = 0, .bLeftID = 0, .bRightID = 0, .bUpID = 0, .bDownID = 0,
		.frame = {72, 386, 566, 144},
		.style = {
			.bShowIdx = WSTL_PROG_NAME, .bHLIdx = WSTL_PROG_NAME, .bSelIdx = WSTL_PROG_NAME, .bGrayIdx = WSTL_PROG_NAME
		},
		.pfnKeyMap = win_progname_keymap, .pfnCallback = win_progname_callback,
		.pNext = (POBJECT_HEAD)NULL, .pRoot = (POBJECT_HEAD)NULL
	},
	.pNextInCntn = (POBJECT_HEAD)&prog_bmp,
	.FocusObjectID = 0,
	.bHiliteAsWhole = 1,
};

TEXT_FIELD prog_name = {
	.head = {
		.bType = OT_TEXTFIELD,
		.bAttr = C_ATTR_ACTIVE, .bFont = 0,
		.bID = 0, .bLeftID = 0, .bRightID = 0, .bUpID = 0, .bDownID = 0,
		.frame = {156, 386, 220, 36},
		.style = {
			.bShowIdx = WSTL_TXT_PROG_NAME, .bHLIdx = WSTL_TXT_PROG_NAME, .bSelIdx = WSTL_TXT_PROG_NAME, .bGrayIdx = WSTL_TXT_PROG_NAME
		},
		.pfnKeyMap = NULL, .pfnCallback = NULL,
		.pNext = (POBJECT_HEAD)&prog_time, .pRoot = (POBJECT_HEAD)&g_win_progname
	},
	.bAlign = C_ALIGN_LEFT|C_ALIGN_BOTTOM,
	.bX = 0, .bY = 0,
	.wStringID = 0,
	.pString = display_strs[20]
};
#endif
/*1,3,2,\*/
/*//2,1,3,\*/
/*//3,2,1,\*/
static char ipp_name3_pat[] = "r0~99";
LDEF_LIST_ITEM(g_win_ippbuy,ippbuy_item0, &ippbuy_item1, ippbuy_idx0,ippbuy_name0,  1,1,1,\ 
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*2,ITEM_W, ITEM_H, RS_IPP_PRICE_TYPE, display_strs[1])

LDEF_LIST_ITEM(g_win_ippbuy,ippbuy_item1, /*&ippbuy_item2*/NULL, ippbuy_idx1,ippbuy_name1,2,2,2,\ 
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*3,ITEM_W, ITEM_H, RS_SCINFO_PINCODE, display_strs[PIN_POSTION])

//LDEF_LIST_ITEM2(g_win_ippbuy,ippbuy_item2, NULL, ippbuy_idx2,ipp_name3,3,3,3,\ 
//	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*4,ITEM_W, ITEM_H, RS_IPP_PERIOD, NORMAL_EDIT_MODE,CURSOR_NORMAL,ipp_name3_pat, display_strs[2])

LDEF_DETAIL_ITEM(g_win_ippbuy,ippbuy_detail_con,ippbuy_detail_label,ippbuy_detail_info,0,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*5,ITEM_W, ITEM_H, RS_IPP_ORDERPRICE, display_strs[3])

LDEF_TXTNAME(&g_win_ippbuy,ippbuy_help_info,NULL,0,0,0,0,0,\
    ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*10,ITEM_W, ITEM_H,RS_IPP_TITLE_BUY,NULL)

LDEF_WIN(g_win_ippbuy,&ippbuy_item0,W_L, W_T, W_W, W_H,2)//1)

POBJECT_HEAD ippbuy_items[] =
{
	(POBJECT_HEAD)&ippbuy_item0,
	(POBJECT_HEAD)&ippbuy_item1,
	//(POBJECT_HEAD)&ippbuy_item2,
};

UINT16 win_ippbuy_string[] =
{
	//RS_IPP_PROGNAME,
	NULL,
	RS_IPP_PROGTYPE2,
	RS_IPP_STARTTIME,
	RS_IPP_DURATION,
};

OSD_RECT win_ippbuy_rect[] =
{
	{ITEM_L,ITEM_T + 5*(ITEM_H + ITEM_GAP)+ITEM_GAP,ITEM_W,ITEM_H},
	{ITEM_L,ITEM_T + 6*(ITEM_H + ITEM_GAP)+ITEM_GAP,ITEM_W,ITEM_H},
	{ITEM_L,ITEM_T + 7*(ITEM_H + ITEM_GAP)+ITEM_GAP,ITEM_W,ITEM_H},
	{ITEM_L,ITEM_T + 8*(ITEM_H + ITEM_GAP)+ITEM_GAP,ITEM_W,ITEM_H},
	{ITEM_L,ITEM_T + 10*(ITEM_H + ITEM_GAP)+ITEM_GAP,ITEM_W,ITEM_H},//help tip
	{ITEM_L,ITEM_T + 11*(ITEM_H + ITEM_GAP)+ITEM_GAP,ITEM_W,ITEM_H},//help key
	{ITEM_L,110,ITEM_W,ITEM_H}, //title
};
/*******************************************************************************
*	Local vriable & function declare
*******************************************************************************/
static void win_ippbuy_set_display(void);
static void win_ippbuy_update_detail();
static void win_period_show_or_not();
static void ippbuy_leftright_key_proc( VACTION act);
PRESULT ippbuy_enter_key_proc();
UINT8 verify_period_value();

#define ITEM_PRICE_SELECTION		1
#define ITEM_PIN_PWD_ENTER		2

//static bool ipp_nochoice = FALSE;
//static SDVTCAIpp popup_ipp;
//static UINT8 current_price_sel  = TPP_TAP_PRICE;

extern cas_ippv_info* gycas_get_ippvmsg_info(void);
INT32 ippbuy_got_info()
{
#if 0
	UINT16 pid;
	cas_ippv_info *ipp_info = NULL;
	MEMSET(&popup_ipp, 0, sizeof(SDVTCAIpp));


	ipp_info = gycas_get_ippvmsg_info();
	popup_ipp.m_wBookedPrice = ipp_info->price;
	popup_ipp.m_byIppStatus = ipp_info->ippvtype;
	
#if 0

	popup_ipp.m_byBookedPriceType = 0;
	popup_ipp.m_wBookedPrice = 1023;
	popup_ipp.m_wCurTppTapPrice = 0;
	popup_ipp.m_wCurTppNoTapPrice = 0;
	popup_ipp.m_wCurCppTapPrice = 0;
	popup_ipp.m_wCurCppNoTapPrice = 0;
	popup_ipp.m_dwDuration = 100;
	popup_ipp.m_tStartTime = 100000;
	popup_ipp.m_byIppStatus |= 0x00;//0x08;
	sprintf(popup_ipp.m_szProdName, "扬智开发软件");

#endif

#endif
	return 1;
}

/*******************************************************************************
*	key mapping and event callback definition
*******************************************************************************/
#define VACT_TYPE_INCREASE		(VACT_PASS+1)
#define VACT_TYPE_DECREASE	(VACT_PASS+2)
static VACTION ippbuy_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act;
	
	switch(key)
	{		
		case V_KEY_RIGHT:
			act = VACT_TYPE_INCREASE;
			break;			
		case V_KEY_LEFT:
			act = VACT_TYPE_DECREASE;
			break;
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
			act = key - V_KEY_0 + VACT_NUM_0;
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}

static VACTION ippbuy_con_period_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act;
	
	switch(key)
	{		
		case V_KEY_LEFT:
			act = VACT_EDIT_LEFT;
			break;
		case V_KEY_RIGHT:
			act = VACT_EDIT_RIGHT;
			break;
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
			act = key - V_KEY_0 + VACT_NUM_0;
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}
static PRESULT ippbuy_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	UINT8 id;
	PRESULT ret = PROC_PASS;
	VACTION act;

	id = OSD_GetObjID ( pObj );

	switch(event)
	{
		case EVN_PRE_DRAW:
			win_ippbuy_set_display();
			break;
		case EVN_POST_DRAW:
			//win_ippbuy_update_detail();
			break;
		case EVN_UNKNOWN_ACTION:
			act = (VACTION)(param1 >> 16);
			if ((act <= VACT_NUM_9) && (ITEM_PIN_PWD_ENTER == id))
			{				
				pin_pwd_input(PIN_ARRAY_0, (UINT8)(act - VACT_NUM_0));
				pin_pwd_set_str(PIN_ARRAY_0);
				OSD_TrackObject (pObj, C_UPDATE_ALL );
			}
			//if ((ITEM_PRICE_SELECTION == id) 
			//	&&((VACT_TYPE_INCREASE == act) || (VACT_TYPE_DECREASE == act)))
			//	ippbuy_leftright_key_proc(act);
			if ((VACT_TYPE_DECREASE == act) && (ITEM_PIN_PWD_ENTER == id))
			{
				pin_pwd_drawback(PIN_ARRAY_0);
				OSD_TrackObject (pObj, C_UPDATE_ALL );
				ret = PROC_LOOP;
			}
			if ((VACT_ENTER == act) && ((ITEM_PIN_PWD_ENTER == id)))
				ret = ippbuy_enter_key_proc();

			break;
		default:
			break;
	}

	return ret;
}

static VACTION ippbuy_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	UINT8 bID = OSD_GetFocusID ( ( POBJECT_HEAD ) &g_win_ippbuy);
	//UINT8 period_check=verify_period_value();

	switch(key)
	{
		case V_KEY_UP:
			if (ITEM_PIN_PWD_ENTER == bID)
			{
				pin_pwd_set_status(0, FALSE);
				pin_pwd_set_str(PIN_ARRAY_0);
			}
			//if(period_check)
			//{
			//	return VACT_PASS;
			//}
			act = VACT_CURSOR_UP;
			break;			
		case V_KEY_DOWN:
			if (ITEM_PRICE_SELECTION == bID)
			{
				pin_pwd_set_status(0, FALSE);
				pin_pwd_set_str(PIN_ARRAY_0);
			}
			//if(period_check)
			//{
			//	return VACT_PASS;
			//}
			act = VACT_CURSOR_DOWN;
			break;	
		case V_KEY_EXIT:
			act = VACT_CLOSE;
#if 0
			DVTCASTB_InquireBookIppOver(DVTSTBCA_GetCurECMPID());
			act = VACT_CLOSE;
#endif
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}

static BOOL ipp_user_exit = TRUE;
static PRESULT ippbuy_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT32 cnt = 0, retval;
	
	switch(event)
	{
		case EVN_PRE_OPEN:
			retval = ippbuy_got_info();
			if (0 == retval)
			{
				ret = PROC_LEAVE;
				break;
			}

			ap_cas_disp_blackscreen();
			if( -1 != window_pos_in_stack((POBJECT_HEAD)&g_win_progname))
			{
				OSD_ObjClose(&g_win_progname, C_CLOSE_CLRBACK_FLG);
				if ((POBJECT_HEAD)&g_win_progname == menu_stack_get_top())
					menu_stack_pop();
			}
			
			ap_cas_message_close();
			win_init_pstring(10);
			pin_pwd_init(1UL<<PIN_ARRAY_0);
			pin_pwd_set_status(0, FALSE);
			pin_pwd_set_str(PIN_ARRAY_0);
			//OSD_SetContainerFocus(&g_win_ippbuy,ITEM_PRICE_SELECTION);
			OSD_SetContainerFocus(&g_win_ippbuy,ITEM_PIN_PWD_ENTER);
			//OSD_SetEditFieldContent(&ipp_name3, STRING_NUMBER, (UINT32)period_number);
			win_period_show_or_not();
			break;
		case EVN_POST_OPEN:
			win_ippbuy_update_detail();
			break;
		case EVN_PRE_CLOSE:
			break;
		case EVN_POST_CLOSE:
			if(ipp_user_exit == TRUE)
				GYCA_IPPVStopBuyDlg(GYFALSE,0x1fff,NULL,NULL );
			ipp_user_exit = TRUE;
			break;
	}

	return ret;
}


static void  win_ippbuy_set_display(void)
{
	UINT8 i, info[10]={0,};
	UINT8 *p;
	UINT16 strid= 0, price=0, priceinfo[50]={0,}, temp[30]={0,};
	TEXT_FIELD *text = &ippbuy_name0;

	cas_ippv_info *ipp_info = NULL;
	ipp_info = gycas_get_ippvmsg_info();

    
	//other normal cases
	if (ipp_info->ippvtype== CAS_IPPV_PAY) //IPPV
	{
		cas_sprintf(priceinfo, "(%d.%02d%I)", ipp_info->price/100, ipp_info->price%100, 
			RS_PRICE_YUAN);
	}
	else
	{
		cas_sprintf(priceinfo, "(%d.%02d%I/%I)", ipp_info->price/100,ipp_info->price%100, 
			RS_PRICE_YUAN,RS_TIME_MINUTES);
	}
	OSD_SetTextFieldContent(text, STRING_UNICODE, (UINT32)priceinfo);
}

static void win_ippbuy_update_help()
{
	UINT8 index = 4;
	UINT16 temp[30] ={0,};
	TEXT_FIELD *text = &ippbuy_help_info;

	OSD_SetObjRect(text, text->head.frame.uLeft, win_ippbuy_rect[index].uTop, text->head.frame.uWidth, text->head.frame.uHeight);
	OSD_SetTextFieldContent(text, STRING_ID, RS_IPP_HELP_TIP);
	OSD_DrawObject((POBJECT_HEAD)text, C_UPDATE_ALL);

	OSD_SetObjRect(text, text->head.frame.uLeft, win_ippbuy_rect[index+1].uTop, text->head.frame.uWidth, text->head.frame.uHeight);
	OSD_SetTextFieldContent(text, STRING_ID, RS_IPP_HELP_KEY);
	OSD_DrawObject((POBJECT_HEAD)text, C_UPDATE_ALL);

	OSD_SetObjRect(text, text->head.frame.uLeft, win_ippbuy_rect[index+2].uTop, text->head.frame.uWidth, text->head.frame.uHeight);
	OSD_SetTextFieldContent(text, STRING_ID, RS_IPP_TITLE_BUY);
	OSD_DrawObject((POBJECT_HEAD)text, C_UPDATE_ALL);
}

static void win_period_show_or_not()
{
	UINT16 strid=0;
	cas_ippv_info *ipp_info = NULL;
	return ;
	ipp_info = gycas_get_ippvmsg_info();
	strid = (ipp_info->ippvtype == CAS_IPPV_PAY)?RS_IPP_IPPV:RS_IPP_IPPT;
	if(RS_IPP_IPPV==strid)
	{
		OSD_SetAttr((POBJECT_HEAD) &ippbuy_item2, C_ATTR_HIDDEN);
	}
	else
	{
		OSD_SetAttr((POBJECT_HEAD) &ippbuy_item2, C_ATTR_ACTIVE);
	}

}

static void win_ippbuy_update_detail()
{
	UINT16 strid, temp[50] ={0,};
	UINT32 index, time;
	UINT8 *p, temp2[30]={0,};
	CONTAINER* con;
	TEXT_FIELD *text1, *text2;

	cas_ippv_info *ipp_info = NULL;
	ipp_info = gycas_get_ippvmsg_info();

	text1 = &ippbuy_detail_label;
	text2 = &ippbuy_detail_info;
	con = &ippbuy_detail_con;
	
	for(index=1;index<3;index++)
	{
		OSD_SetObjRect(con, win_ippbuy_rect[index].uLeft, win_ippbuy_rect[index].uTop, win_ippbuy_rect[index].uWidth, win_ippbuy_rect[index].uHeight);
		OSD_SetObjRect(text1, text1->head.frame.uLeft, win_ippbuy_rect[index].uTop, text1->head.frame.uWidth, text1->head.frame.uHeight);
		OSD_SetObjRect(text2, text2->head.frame.uLeft, win_ippbuy_rect[index].uTop, text2->head.frame.uWidth, text2->head.frame.uHeight);

		if(index == 3)
		{
			//OSD_SetTextFieldContent(text1, STRING_ID,win_ippbuy_string[index]);
			UINT16 tmpt[20];
			MEMSET(tmpt, 0, 40);
			convert_gb2312_to_unicode("结束时间", STRLEN("结束时间"), tmpt, 20);
			OSD_SetTextFieldContent(text1, STRING_UNICODE, (UINT32)tmpt);
		}
		else
			OSD_SetTextFieldContent(text1, STRING_ID,win_ippbuy_string[index]);
/*
		if (0 == index) //program name
		{
			convert_gb2312_to_unicode(popup_ipp.m_szProdName, DVTCA_MAXLEN_PRODUCT_NAME,
				temp, DVTCA_MAXLEN_PRODUCT_NAME);
			OSD_SetTextFieldContent(text2, STRING_UNICODE, (UINT32)temp);
		}
*/
		if (1 == index) // program type
		{
			strid = (ipp_info->ippvtype== CAS_IPPV_PAY)?RS_IPP_IPPV:RS_IPP_IPPT;
			p = OSD_GetUnicodeString(strid);
			ComUniStrCopyChar((UINT8 *)temp, p);
			OSD_SetTextFieldContent(text2, STRING_UNICODE, (UINT32)temp);
		}
		if (2 == index) //start time
		{
			UINT8 asc_time[20];
			sprintf(asc_time, "%4d/%02d/%02d %02d:%02d:%02d",ipp_info->startdate.date.year,
				ipp_info->startdate.date.month,ipp_info->startdate.date.day,
				ipp_info->startdate.time.hour,ipp_info->startdate.time.minute,ipp_info->startdate.time.second);
			wincom_AscToMBUni(asc_time,temp);
			//convert_sysdate_to_uni(popup_ipp.m_tStartTime, temp);
			OSD_SetTextFieldContent(text2, STRING_UNICODE, (UINT32)temp);
		}
		if (3 == index) //duration
		{
		#if 0
			time = popup_ipp.m_dwDuration;
			MEMSET(temp, 0, sizeof(temp));
			cas_sprintf(temp, "%d%I%d%I%d%I%d%I",time/(3600*24), RS_TIME_DAY, time%(3600*24)/3600, RS_TIME_HOUR,
					time%3600/60,RS_TIME_MIN, time%60,RS_TIME_SECOND);
			OSD_SetTextFieldContent(text2, STRING_UNICODE, (UINT32)temp);
		#endif
			UINT8 asc_time[20];
			sprintf(asc_time, "%4d/%02d/%02d %02d:%02d:%02d",ipp_info->expiredate.date.year,
				ipp_info->expiredate.date.month,ipp_info->expiredate.date.day,
				ipp_info->expiredate.time.hour,ipp_info->expiredate.time.minute,ipp_info->expiredate.time.second);
			wincom_AscToMBUni(asc_time,temp);
			//convert_sysdate_to_uni(popup_ipp.m_tStartTime, temp);
			OSD_SetTextFieldContent(text2, STRING_UNICODE, (UINT32)temp);
		}
		OSD_DrawObject((POBJECT_HEAD)con, C_UPDATE_ALL);
	}

	win_ippbuy_update_help();
}

#if 0
static void ippbuy_leftright_key_proc( VACTION act)
{
	UINT8 i, type;
	UINT8 *p;
	UINT16 strid= 0, price=0;
	UINT16 priceinfo[50]={0,}, temp[30]={0,};
	TEXT_FIELD *text = &ippbuy_name0;

//	if (TRUE == ipp_nochoice) //IPPT and bookedPrice is not zero
	{
		//win_popup_msg(NULL, NULL, RS_IPP_NOPRICESEL);
		return;
	}
	
	type = popup_ipp.m_byBookedPriceType;
	
	for (i=0; i<4; i++)
	{
		price = 0;
		if (VACT_TYPE_INCREASE == act)
		{
			type = (type+1)%4;
		}
		else
		{
			type = (type+4-1)%4;
		}
		switch(type)
		{
			case TPP_TAP_PRICE:
				//price =  popup_ipp.m_wCurTppTapPrice;
				//strid = RS_IPP_PRICE0;
				price = 1;
				strid = RS_IPP_PRICE1;
				break;
			case TPP_NOTAP_PRICE:
				//price =  popup_ipp.m_wCurTppNoTapPrice;
				//strid = RS_IPP_PRICE1;
				price = 1;
				strid = RS_IPP_PRICE1;
				break;
			case CPP_TAP_PRICE:
				//price =  popup_ipp.m_wCurCppTapPrice;
				//strid = RS_IPP_PRICE2;
				price = 1;
				strid = RS_IPP_PRICE1;
				break;
			case CPP_NOTAP_PRICE:
				//price =  popup_ipp.m_wCurCppNoTapPrice;
				//strid = RS_IPP_PRICE3;
				price = 1;
				strid = RS_IPP_PRICE1;
				break;
		}
		if (price)
		{
			popup_ipp.m_wBookedPrice = price;
			popup_ipp.m_byBookedPriceType = type;
			break;
		}
	}

	if (i>=4)//no valid date is found
	{
		popup_ipp.m_wBookedPrice = 0xff;//invalid data
		popup_ipp.m_byBookedPriceType = 0;
		p = OSD_GetUnicodeString(RS_CAERR_PROG_STATUS_INVALID);
		ComUniStrCopyChar((UINT8 *)temp, p);
		OSD_SetTextFieldContent(text, STRING_UNICODE, (UINT32)temp);
		return;
	}

	/*p = OSD_GetUnicodeString(strid);
	ComUniStrCopyChar((UINT8 *)priceinfo, p);
	display_yuan_min(price, popup_ipp.m_byCurInterval, temp);
	ComUniStrCat(priceinfo, temp);
	*/
	if (1)//(popup_ipp.m_byIppStatus & 0x08) //IPPV
	{
		cas_sprintf(priceinfo, "%I(%d.%02d%I)", strid, price/100, price%100, 
			RS_PRICE_YUAN);
	}
	else
	{
		cas_sprintf(priceinfo, "%I(%d.%02d%I/%d%I)", strid, price/100, price%100, 
			RS_PRICE_YUAN,popup_ipp.m_byCurInterval,RS_TIME_MINUTES);
	}
	OSD_SetTextFieldContent(text, STRING_UNICODE, (UINT32)priceinfo);
	OSD_TrackObject((POBJECT_HEAD)&ippbuy_item0, C_UPDATE_ALL);
	return;
}
#endif

extern UINT32 ap_cas_disp_blackscreen();
extern PRESULT ap_cas_ipp_popup_proc(UINT8 open_close);
PRESULT ippbuy_enter_key_proc()
{
	UINT32 err;
	PRESULT ret = PROC_PASS;
	win_popup_choice_t choice;
	gycapin pin;
	T_CA_IPPVPrice price;

	cas_ippv_info *ipp_info = NULL;

	ipp_info = gycas_get_ippvmsg_info();
	price.m_byEventPriceType = (ipp_info->ippvtype==CAS_IPPT_PAY)?1:0;
	price.m_wEventPrice = ipp_info->price;

	if(gy_get_sc_status() == FALSE)
	{
		cas_popup_msg(NULL, NULL, RS_CAERR_INSERT_CARD);
		return ret;
	}

	//if (0 == price.m_wEventPrice)
	//{
	//	cas_popup_yesno_msg(NULL, NULL, RS_IPP_SUB_INVALIDEPRICE);
	//	return ret;
	//}
	pin.m_byLen = PIN_PWD_LEN;
	MEMCPY(pin.m_byszPin, pin_pwd_password_str[PIN_ARRAY_0], PIN_PWD_LEN);
	if (PIN_PWD_LEN != pin_pwd_len(pin.m_byszPin))
		err = PIN_LEN_ERROR;
	
	if (PIN_LEN_ERROR != err)
	{
		choice = cas_popup_yesno_msg(NULL, NULL, RS_IPP_SUB_TIP);
		if (WIN_POP_CHOICE_YES != choice)
		{
			ipp_user_exit = FALSE;
			ret = GYCA_IPPVStopBuyDlg(GYFALSE, ipp_info->ecm_pid, pin.m_byszPin, &price);
			OSD_ObjClose(&g_win_ippbuy, C_CLOSE_CLRBACK_FLG);
			if ((POBJECT_HEAD)&g_win_ippbuy == menu_stack_get_top())
				menu_stack_pop();
			return ret;
		}

		err = GYCA_IPPVStopBuyDlg(GYTRUE, ipp_info->ecm_pid, pin.m_byszPin, &price);
		if (GYCA_OK == err)
		{
			//ap_cas_disp_blackscreen();
			//ap_cas_ipp_popup_proc(0);
			ipp_user_exit = FALSE;
			OSD_ObjClose(&g_win_ippbuy, C_CLOSE_CLRBACK_FLG);
			if ((POBJECT_HEAD)&g_win_ippbuy == menu_stack_get_top())
				menu_stack_pop();
			return ret;
		}
		else if (GYCA_ERR == ret)
		{
			if(gy_get_sc_status())
				win_popup_msg(NULL, NULL, RS_IPP_ERR_MONEYLACK);
			else//the sc in out
	        {
	    		win_popup_msg(NULL, NULL, RS_SHOWMSG_INSERTCARD);
	        }
	   		ret = PROC_LOOP;
		}
		else if (GYCA_PIN_ERR == ret)
		{
	    	win_popup_msg(NULL, NULL, RS_CAERR_PIN_ERROR);
	   		ret = PROC_LOOP;
		}
		else
		{
			win_popup_msg(NULL, NULL, RS_IPP_ERR_OTHER);
			ret = PROC_LOOP;
		}
	}
	else if (PIN_LEN_ERROR == err)
	{
		win_popup_msg(NULL, NULL, RS_CAERR_OLDPIN_ERRLEN);
		ret = PROC_LOOP;
	}
	else
	{
		win_popup_msg(NULL, NULL, RS_IPP_ERR_OTHER);
		ret = PROC_LOOP;
	}
	
#if 0
	else
	{
        if(gy_get_sc_status())
        {
    		win_popup_msg(NULL, NULL, RS_CAERR_PIN_ERROR);
        }
        else//the sc in out
        {
    		win_popup_msg(NULL, NULL, RS_SHOWMSG_INSERTCARD);
        }
   		ret = PROC_LOOP;
	}
#endif

	pin_pwd_init(1UL<<PIN_ARRAY_0);
	pin_pwd_set_status(0, FALSE);
	pin_pwd_set_str(PIN_ARRAY_0);
	//OSD_SetContainerFocus(&g_win_ippbuy,ITEM_PRICE_SELECTION);
	OSD_SetContainerFocus(&g_win_ippbuy,ITEM_PIN_PWD_ENTER);
	OSD_TrackObject((POBJECT_HEAD)&g_win_ippbuy,C_UPDATE_ALL);
	win_ippbuy_update_detail();
	return PROC_PASS;
}

UINT8 verify_period_value()
{
	UINT16 strid=0;
	UINT32 temp=0;
	UINT8 period_number=20;
	cas_ippv_info *ipp_info = NULL;
	return FALSE;
	ipp_info = gycas_get_ippvmsg_info();
	strid = (ipp_info->ippvtype == CAS_IPPV_PAY)?RS_IPP_IPPV:RS_IPP_IPPT;
	if(RS_IPP_IPPT==strid)
	{
		OSD_GetEditFieldIntValue(&ipp_name3,&temp);		
		if(temp<=0)
		{			
			return TRUE;
		}
	}
	return FALSE;
}


