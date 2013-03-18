#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include "win_cas_com.h"
#include "win_ca_walletinfo.h"
/*******************************************************************************
*	Objects definition
*******************************************************************************/
#if 0
extern CONTAINER g_win_walletinfo;
extern CONTAINER win_walletinfo_con;

extern OBJLIST	 walletinfo_olist;

extern CONTAINER walletinfo_item0;
extern CONTAINER walletinfo_item1;
extern CONTAINER walletinfo_item2;
extern CONTAINER walletinfo_item3;
extern CONTAINER walletinfo_item4;
extern CONTAINER walletinfo_item5;
extern CONTAINER walletinfo_item6;
extern CONTAINER walletinfo_item7;
extern CONTAINER walletinfo_item8;

extern TEXT_FIELD walletinfo_title_id;
extern TEXT_FIELD walletinfo_title_credit;
extern TEXT_FIELD walletinfo_title_cost;

extern TEXT_FIELD walletinfo_idx0;
extern TEXT_FIELD walletinfo_idx1;
extern TEXT_FIELD walletinfo_idx2;
extern TEXT_FIELD walletinfo_idx3;
extern TEXT_FIELD walletinfo_idx4;
extern TEXT_FIELD walletinfo_idx5;
extern TEXT_FIELD walletinfo_idx6;
extern TEXT_FIELD walletinfo_idx7;
extern TEXT_FIELD walletinfo_idx8;

extern TEXT_FIELD walletinfo_crdt0;
extern TEXT_FIELD walletinfo_crdt1;
extern TEXT_FIELD walletinfo_crdt2;
extern TEXT_FIELD walletinfo_crdt3;
extern TEXT_FIELD walletinfo_crdt4;
extern TEXT_FIELD walletinfo_crdt5;
extern TEXT_FIELD walletinfo_crdt6;
extern TEXT_FIELD walletinfo_crdt7;
extern TEXT_FIELD walletinfo_crdt8;


extern TEXT_FIELD walletinfo_cost0;
extern TEXT_FIELD walletinfo_cost1;
extern TEXT_FIELD walletinfo_cost2;
extern TEXT_FIELD walletinfo_cost3;
extern TEXT_FIELD walletinfo_cost4;
extern TEXT_FIELD walletinfo_cost5;
extern TEXT_FIELD walletinfo_cost6;
extern TEXT_FIELD walletinfo_cost7;
extern TEXT_FIELD walletinfo_cost8;

extern SCROLL_BAR walletinfo_scb;

static VACTION walletinfo_list_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT walletinfo_list_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION walletinfo_list_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT walletinfo_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION walletinfo_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT walletinfo_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

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

#define TXTN_SH_IDX   WSTL_TXT_4
#define TXTN_HL_IDX   WSTL_TXT_5
#define TXTN_SL_IDX   WSTL_TXT_4
#define TXTN_GRY_IDX  WSTL_TXT_10

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_1
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_1

#define LIST_BAR_MID_RECT_IDX  	    WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX  	WSTL_SCROLLBAR_2


#define W_L		0
#define W_T		90
#define W_W		720
#define W_H		400

#define W_CON_L		66
#define W_CON_T	102
#define W_CON_W	604
#define W_CON_H     	320

#define SCB_L (W_CON_L + 2)
#define SCB_T (W_CON_T )
#define SCB_W 12
#define SCB_H (W_CON_H - 4)

#define LST_L	(SCB_L + SCB_W)
#define LST_T	(W_CON_T + 10)
#define LST_W	(W_CON_W - SCB_W - 8)
#define LST_H	300
#define LST_GAP 4

#define ITEM_L	(LST_L + 0)
#define ITEM_T	(LST_T + 2)
#define ITEM_W	(LST_W - 0)
#define ITEM_H	28
#define ITEM_GAP	2

#define ITEM_IDX_L  2
#define ITEM_IDX_W  150
#define ITEM_RCD_L (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_RCD_W 150
#define ITEM_NAME_L  (ITEM_RCD_L + ITEM_RCD_W)
#define ITEM_NAME_W  280//180

#define LDEF_CON(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    walletinfo_list_item_con_keymap,walletinfo_list_item_con_callback,  \
    conobj, 1,1)
    
#define LDEF_TXTIDX(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXTI_SH_IDX,TXTI_HL_IDX,TXTI_SL_IDX,TXTI_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,resID,str)

#define LDEF_TXTNAME(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXTN_SH_IDX,TXTN_HL_IDX,TXTN_SL_IDX,TXTN_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 10,0,resID,str)

#define LDEF_LIST_ITEM(root,varCon,varTxtidx,varTxtRcd,varTxtName,ID,l,t,w,h,idxstr,rcdstr,namestr)	\
	LDEF_CON(&root,varCon,NULL,ID,ID,ID,ID,ID,l,t,w,h,&varTxtidx,1)	\
	LDEF_TXTIDX(&varCon,varTxtidx,&varTxtRcd ,0,0,0,0,0,l + ITEM_IDX_L, t,ITEM_IDX_W,h,0,idxstr)	\
	LDEF_TXTNAME(&varCon,varTxtRcd,&varTxtName,1,1,1,1,1,l + ITEM_RCD_L, t,ITEM_RCD_W,h,0,rcdstr)\
	LDEF_TXTNAME(&varCon,varTxtName,NULL,1,1,1,1,1,l + ITEM_NAME_L, t,ITEM_NAME_W,h,0,namestr)

#define LDEF_LISTBAR(root,varScb,page,l,t,w,h)	\
	DEF_SCROLLBAR(varScb, &root, NULL, C_ATTR_ACTIVE, 0, \
		0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
		NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
		0, 10, w, h - 20, 100, 1)

#define LDEF_OL(root,varOl,nxtObj,l,t,w,h,style,dep,count,flds,sb,mark,selary)	\
  DEF_OBJECTLIST(varOl,&root,nxtObj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_HL_IDX,LST_SL_IDX,LST_GRY_IDX,   \
    walletinfo_list_keymap,walletinfo_list_callback,    \
    flds,sb,mark,style,dep,count,selary)

#define LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    walletinfo_keymap,walletinfo_callback,  \
    nxtObj, focusID,0)

LDEF_TXTIDX(&g_win_walletinfo,walletinfo_title_id,&walletinfo_title_credit,0,0,0,0,0,\
    ITEM_L+ITEM_IDX_L, ITEM_T,ITEM_IDX_W,ITEM_H,RS_SLOT_ID,NULL)

LDEF_TXTIDX(&g_win_walletinfo,walletinfo_title_credit,&walletinfo_title_cost,0,0,0,0,0,\
    ITEM_L+ITEM_RCD_L, ITEM_T,ITEM_RCD_W,ITEM_H,RS_SLOT_CREDIT,NULL)

LDEF_TXTIDX(&g_win_walletinfo,walletinfo_title_cost,&walletinfo_olist,0,0,0,0,0,\
    ITEM_L+ITEM_NAME_L, ITEM_T,ITEM_NAME_W,ITEM_H,RS_SLOT_BALANCE,NULL)

LDEF_LIST_ITEM(walletinfo_olist,walletinfo_item0,walletinfo_idx0,walletinfo_crdt0, walletinfo_cost0, 1,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*1,ITEM_W, ITEM_H, display_strs[0], display_strs[9], display_strs[18])

LDEF_LIST_ITEM(walletinfo_olist,walletinfo_item1,walletinfo_idx1,walletinfo_crdt1, walletinfo_cost1, 2,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*2,ITEM_W, ITEM_H, display_strs[1], display_strs[10], display_strs[19])

LDEF_LIST_ITEM(walletinfo_olist,walletinfo_item2,walletinfo_idx2,walletinfo_crdt2, walletinfo_cost2, 3,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*3,ITEM_W, ITEM_H, display_strs[2], display_strs[11], display_strs[20])

LDEF_LIST_ITEM(walletinfo_olist,walletinfo_item3,walletinfo_idx3,walletinfo_crdt3, walletinfo_cost3, 4,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*4,ITEM_W, ITEM_H, display_strs[3], display_strs[12], display_strs[21])

LDEF_LIST_ITEM(walletinfo_olist,walletinfo_item4,walletinfo_idx4,walletinfo_crdt4, walletinfo_cost4, 5,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*5,ITEM_W, ITEM_H, display_strs[4], display_strs[13], display_strs[22])

LDEF_LIST_ITEM(walletinfo_olist,walletinfo_item5,walletinfo_idx5,walletinfo_crdt5, walletinfo_cost5, 6,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*6,ITEM_W, ITEM_H, display_strs[5], display_strs[14], display_strs[23])

LDEF_LIST_ITEM(walletinfo_olist,walletinfo_item6,walletinfo_idx6,walletinfo_crdt6, walletinfo_cost6, 7,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*7,ITEM_W, ITEM_H, display_strs[6], display_strs[15], display_strs[24])

LDEF_LIST_ITEM(walletinfo_olist,walletinfo_item7,walletinfo_idx7,walletinfo_crdt7, walletinfo_cost7, 8,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*8,ITEM_W, ITEM_H, display_strs[7], display_strs[16], display_strs[25])

LDEF_LIST_ITEM(walletinfo_olist,walletinfo_item8,walletinfo_idx8,walletinfo_crdt8, walletinfo_cost8, 9,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*9,ITEM_W, ITEM_H, display_strs[8], display_strs[17], display_strs[26])

LDEF_LISTBAR(walletinfo_olist,walletinfo_scb,9,SCB_L,SCB_T, SCB_W, SCB_H)

POBJECT_HEAD walletinfo_items[] =
{
	(POBJECT_HEAD)&walletinfo_item0,
	(POBJECT_HEAD)&walletinfo_item1,
	(POBJECT_HEAD)&walletinfo_item2,
	(POBJECT_HEAD)&walletinfo_item3,
	(POBJECT_HEAD)&walletinfo_item4,
	(POBJECT_HEAD)&walletinfo_item5,
	(POBJECT_HEAD)&walletinfo_item6,
	(POBJECT_HEAD)&walletinfo_item7,
	(POBJECT_HEAD)&walletinfo_item8,
};

#define LIST_STYLE (LIST_VER | LIST_NO_SLECT     | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)

LDEF_OL(g_win_walletinfo,walletinfo_olist,NULL, LST_L,LST_T,LST_W,LST_H,LIST_STYLE, 9, 9,walletinfo_items,&walletinfo_scb,NULL,NULL)

DEF_CONTAINER (win_walletinfo_con, NULL, &walletinfo_title_id, C_ATTR_ACTIVE, 0, \
                0, 0, 0, 0, 0, W_CON_L, W_CON_T, W_CON_W, W_CON_H, CON_IDX, CON_IDX, CON_IDX, CON_IDX,   \
                NULL, NULL,  \
                NULL, 1, 0 )
                
LDEF_WIN(g_win_walletinfo,&win_walletinfo_con, W_L,W_T,W_W, W_H, 1)

#endif
/*******************************************************************************
*	Local vriable & function declare
*******************************************************************************/
extern void win_init_pstring(UINT8 num);
static void win_walletinfo_set_display(void);

UINT8 slot_number;
UINT8 slot_ID[CDCA_MAXNUM_SLOT+1];
SCDCATVSSlotInfo slot_info[CDCA_MAXNUM_SLOT+1];


/*******************************************************************************
*	key mapping and event callback definition
*******************************************************************************/
static VACTION walletinfo_list_item_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	
	return act;
}

static PRESULT walletinfo_list_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;

	return ret;
}

#define VACT_walletinfo_DETAIL (VACT_PASS + 1)

static VACTION walletinfo_list_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act;
	
	switch(key)
	{
	case V_KEY_UP:
		act = VACT_CURSOR_UP;
		break;			
	case V_KEY_DOWN:
		act = VACT_CURSOR_DOWN;
		break;			
	case V_KEY_P_UP:
		act = VACT_CURSOR_PGUP;
		break;			
	case V_KEY_P_DOWN:
		act = VACT_CURSOR_PGDN;
		break;	
		break;		
	default:
		act = VACT_PASS;
	}

	return act;
}

static PRESULT walletinfo_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	OBJLIST* ol;
	UINT16 curitem;

	ol = &walletinfo_olist;

	switch(event)
	{
	case EVN_PRE_DRAW:
		win_walletinfo_set_display();
		break;
	}
	return ret;
}

static VACTION walletinfo_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch(key)
	{
		case V_KEY_EXIT:
			BackToFullScrPlay();
			break;
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}
extern UINT16 current_operator_ID;
SCDCAEntitles ca_entitles;
INT32 walletinfo_got_info()
{
	UINT8 back_saved;
	UINT16 ret,i,j;

	ret = CDCASTB_GetSlotIDs(current_operator_ID, slot_ID);
	if ((CDCA_RC_CARD_INVALID== ret)||(CDCA_RC_POINTER_INVALID == ret))
	{
		win_popup_msg(NULL, NULL, RS_CAERR_NOSMC);

		MEMSET(slot_ID, 0, sizeof(slot_ID));
		MEMSET(slot_info, 0, sizeof(slot_info));
		return 0;
	}
	else if (CDCA_RC_DATA_NOT_FIND == ret)
	{
		win_popup_msg(NULL, NULL, RS_CAERR_DATA_NOT_FOUND);
		return 0;	
	}
	else if (CDCA_RC_OK == ret)
	{
		for (i=0; i<CDCA_MAXNUM_SLOT; i++)
		{
			if (0 == slot_ID[i])
				break;
		}
		slot_number = i;
		for(j=0; j<slot_number; j++)
		{
			ret = CDCASTB_GetSlotInfo(current_operator_ID, slot_ID[j], &slot_info[j]);
			if ((CDCA_RC_CARD_INVALID== ret)||(CDCA_RC_POINTER_INVALID == ret))
			{
				win_popup_msg(NULL, NULL, RS_CAERR_NOSMC);
				return 0;
			}
			else if (CDCA_RC_DATA_NOT_FIND == ret)
			{
				win_popup_msg(NULL, NULL, RS_CAERR_DATA_NOT_FOUND);
				return 0;	
			}
			else if (CDCA_RC_OK == ret)
			{
				//WALLET_PRINT("@@@@Got slot info: ID %d, money %d, cost %d\n", slot_ID[j], slot_info[j].m_wCreditLimit, slot_info[j].m_wBalance );
			}	
		}
	}
	else
	{
		win_popup_msg(NULL, NULL, RS_CAERR_UNKNOWN);
		return 0;
	}
#if 0//self-testing use
	UINT32 tmp;
	slot_number = CDCA_MAXNUM_SLOT;
	for (tmp=4; tmp<CDCA_MAXNUM_SLOT; tmp++)
	{
		slot_ID[tmp]=tmp+1;
		slot_info[tmp].m_wCreditLimit = tmp*10;
		slot_info[tmp].m_wBalance = tmp *5;
	}
#endif 

	return 1;
}

extern struct help_item_resource win_opinfo_help[];
static PRESULT walletinfo_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	INT32 tmp;
	
	switch(event)
	{
	case EVN_PRE_OPEN:
		MEMSET(slot_ID, 0, sizeof(slot_ID));
		MEMSET(slot_info, 0, sizeof(slot_info));
        	tmp = walletinfo_got_info();
		if (0==tmp)
		{
			ret = PROC_LEAVE;
			break;
		}
		wincom_open_title_ext(RS_SLOT_INFO, IM_TITLE_ICON_SYSTEM);
		wincom_open_help(win_opinfo_help, 2);
        	win_init_pstring(40);
		OSD_SetObjListCount(&walletinfo_olist, slot_number);
		//wincom_open_title(pObj,RS_SLOT_INFO,0);
		break;
	case EVN_POST_OPEN:
		break;
	case EVN_PRE_CLOSE:
		/* Make OSD not flickering */
		*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;	
		break;
	case EVN_POST_CLOSE:
		break;
	}

	return ret;
}

static void  win_walletinfo_set_display(void)
{
	CONTAINER* con;
	TEXT_FIELD* text;
	OBJLIST* ol;	
	
	UINT16 page,index,top;
	UINT16 date[11];
	UINT32 infoid;
	
	ol = &walletinfo_olist;

	page = OSD_GetObjListPage(ol); 
	top = OSD_GetObjListTop(ol);
	
	if (slot_number != 0)
	{
		for(index=0;index<page;index++)
	    	{
			con = (CONTAINER*)walletinfo_olist_ListField[index];

		        /* operator */
		       text = (TEXT_FIELD*)OSD_GetContainerNextObj(con);
				if ((index+top)< slot_number)
					OSD_SetTextFieldContent(text, STRING_NUMBER, (UINT32)current_operator_ID);
				else
					OSD_SetTextFieldContent(text, STRING_ANSI, (UINT32)"");

		        /* ID */
		       text = (TEXT_FIELD*)OSD_GetObjpNext(text); 

				if ((index+top) < slot_number)
				{
					OSD_SetTextFieldContent(text, STRING_NUMBER, (UINT32)slot_ID[(index+top)]);
				}
				else
					OSD_SetTextFieldContent(text, STRING_ANSI, (UINT32)"");

		 
		        /* credit**/
		        text = (TEXT_FIELD*)OSD_GetObjpNext(text);

				if ((index+top) < slot_number)
					OSD_SetTextFieldContent(text, STRING_NUMBER, (UINT32)(slot_info[(index+top)].m_wCreditLimit));
				else
					OSD_SetTextFieldContent(text, STRING_ANSI, (UINT32)"");

			    /*cost**/
		        text = (TEXT_FIELD*)OSD_GetObjpNext(text);

				if ((index+top) < slot_number)
				{
					OSD_SetTextFieldContent(text, STRING_NUMBER, (UINT32)slot_info[(index+top)].m_wBalance);
				}
				else
					OSD_SetTextFieldContent(text, STRING_ANSI, (UINT32)"");

	        
		}
	}
	
}



