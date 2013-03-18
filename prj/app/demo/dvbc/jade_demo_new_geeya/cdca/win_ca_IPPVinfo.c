#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include "win_cas_com.h"

#define IPPVINFO_PRINT libc_printf
//#define IPPVinfo_debug 
#if 0
/*******************************************************************************
*	Objects definition
*******************************************************************************/

extern CONTAINER g_win_IPPVinfo;
extern CONTAINER win_IPPVinfo_con;

extern OBJLIST	 IPPVinfo_olist;

extern CONTAINER IPPVinfo_item0;
extern CONTAINER IPPVinfo_item1;
extern CONTAINER IPPVinfo_item2;
extern CONTAINER IPPVinfo_item3;
extern CONTAINER IPPVinfo_item4;
extern CONTAINER IPPVinfo_item5;
extern CONTAINER IPPVinfo_item6;
extern CONTAINER IPPVinfo_item7;
extern CONTAINER IPPVinfo_item8;

extern TEXT_FIELD IPPVinfo_title;

extern TEXT_FIELD IPPVinfo_list0;
extern TEXT_FIELD IPPVinfo_list1;
extern TEXT_FIELD IPPVinfo_list2;
extern TEXT_FIELD IPPVinfo_list3;
extern TEXT_FIELD IPPVinfo_list4;
extern TEXT_FIELD IPPVinfo_list5;
//extern TEXT_FIELD IPPVinfo_list6;
//extern TEXT_FIELD IPPVinfo_list7;
//extern TEXT_FIELD IPPVinfo_list8;

extern SCROLL_BAR IPPVinfo_scb;

//static VACTION IPPVinfo_list_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT IPPVinfo_list_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION IPPVinfo_list_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT IPPVinfo_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION IPPVinfo_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT IPPVinfo_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

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
#define ITEM_TXT_W1  100
#define ITEM_TXT_W2  100
#define ITEM_TXT_W3  100
#define ITEM_TXT_W4  80
#define ITEM_TXT_W5  80
#define ITEM_TXT_W6  120


#define ITEM_IDX_L  2
#define ITEM_IDX_W  60


#define LDEF_CON(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    NULL,/*IPPVinfo_list_item_con_keymap,*/IPPVinfo_list_item_con_callback,  \
    conobj, 1,1)
    
#define LDEF_TXTIDX(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXTI_SH_IDX,TXTI_HL_IDX,TXTI_SL_IDX,TXTI_GRY_IDX,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,resID,str)

#define LDEF_LIST_ITEM(root,varCon,varTx1,varTx2,varTx3,varTx4,varTx5, varTx6,ID,l,t,w,h,idxstr1,idxstr2,idxstr3,idxstr4,idxstr5, idxstr6)	\
	LDEF_CON(&root,varCon,NULL,ID,ID,ID,ID,ID,l,t,w,h,&varTx1,1)	\
	LDEF_TXTIDX(&varCon,varTx1,&varTx2 ,0,0,0,0,0,(ITEM_L+ITEM_IDX_L), t,ITEM_TXT_W1,ITEM_H,0,idxstr1)	\
	LDEF_TXTIDX(&varCon,varTx2,&varTx3 ,0,0,0,0,0,(ITEM_L+ITEM_IDX_L+ITEM_TXT_W1 ), t,ITEM_TXT_W2,ITEM_H,0,idxstr2)	\
	LDEF_TXTIDX(&varCon,varTx3,&varTx4 ,0,0,0,0,0,(ITEM_L+ITEM_IDX_L+ITEM_TXT_W1+ITEM_TXT_W2), t,ITEM_TXT_W3,ITEM_H,idxstr3,NULL)	\
	LDEF_TXTIDX(&varCon,varTx4,&varTx5 ,0,0,0,0,0,(ITEM_L+ITEM_IDX_L+ITEM_TXT_W1+ITEM_TXT_W2+ITEM_TXT_W3), t,ITEM_TXT_W4,ITEM_H,0,idxstr4)\
	LDEF_TXTIDX(&varCon,varTx5,&varTx6 ,0,0,0,0,0,(ITEM_L+ITEM_IDX_L+ITEM_TXT_W1+ITEM_TXT_W2+ITEM_TXT_W3+ITEM_TXT_W4), t,ITEM_TXT_W5,ITEM_H,0,idxstr5)\
	LDEF_TXTIDX(&varCon,varTx6,NULL ,0,0,0,0,0,(ITEM_L+ITEM_IDX_L+ITEM_TXT_W1+ITEM_TXT_W2+ITEM_TXT_W3+ITEM_TXT_W4+ITEM_TXT_W5), t,ITEM_TXT_W6,ITEM_H,0,idxstr6)

#define LDEF_LISTBAR(root,varScb,page,l,t,w,h)	\
	DEF_SCROLLBAR(varScb, &root, NULL, C_ATTR_ACTIVE, 0, \
		0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
		NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
		0, 10, w, h - 20, 100, 1)

#define LDEF_OL(root,varOl,nxtObj,l,t,w,h,style,dep,count,flds,sb,mark,selary)	\
  DEF_OBJECTLIST(varOl,&root,nxtObj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_HL_IDX,LST_SL_IDX,LST_GRY_IDX,   \
    IPPVinfo_list_keymap,IPPVinfo_list_callback,    \
    flds,sb,mark,style,dep,count,selary)
    
#define LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    IPPVinfo_keymap,IPPVinfo_callback,  \
    nxtObj, focusID,0)

LDEF_TXTIDX(&g_win_IPPVinfo,IPPVinfo_title,&IPPVinfo_olist,0,0,0,0,0,\
    ITEM_L+ITEM_IDX_L, ITEM_T,ITEM_IDX_W,ITEM_H,RS_SLOT_ID,display_strs[0])

LDEF_LIST_ITEM(IPPVinfo_olist,IPPVinfo_item0,IPPVinfo_list0,IPPVinfo_list1, IPPVinfo_list2, IPPVinfo_list3, IPPVinfo_list4,IPPVinfo_list5,1,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*1,ITEM_W, ITEM_H, display_strs[1],display_strs[2],RS_IPPV_STATUS_BOOKING,display_strs[4],display_strs[5],display_strs[6])

LDEF_CON(&IPPVinfo_olist,IPPVinfo_item1,NULL,2,2,2,2,2,ITEM_L,ITEM_T + (ITEM_H + ITEM_GAP)*2,ITEM_W, ITEM_H,&IPPVinfo_list0,1)	\
LDEF_CON(&IPPVinfo_olist,IPPVinfo_item2,NULL,3,3,3,3,3,ITEM_L,ITEM_T + (ITEM_H + ITEM_GAP)*3,ITEM_W, ITEM_H,&IPPVinfo_list0,1)	\
LDEF_CON(&IPPVinfo_olist,IPPVinfo_item3,NULL,4,4,4,4,4,ITEM_L,ITEM_T + (ITEM_H + ITEM_GAP)*4,ITEM_W, ITEM_H,&IPPVinfo_list0,1)	\
LDEF_CON(&IPPVinfo_olist,IPPVinfo_item4,NULL,5,5,5,5,5,ITEM_L,ITEM_T + (ITEM_H + ITEM_GAP)*5,ITEM_W, ITEM_H,&IPPVinfo_list0,1)	\
LDEF_CON(&IPPVinfo_olist,IPPVinfo_item5,NULL,6,6,6,6,6,ITEM_L,ITEM_T + (ITEM_H + ITEM_GAP)*6,ITEM_W, ITEM_H,&IPPVinfo_list0,1)	\
LDEF_CON(&IPPVinfo_olist,IPPVinfo_item6,NULL,7,7,7,7,7,ITEM_L,ITEM_T + (ITEM_H + ITEM_GAP)*7,ITEM_W, ITEM_H,&IPPVinfo_list0,1)	\
LDEF_CON(&IPPVinfo_olist,IPPVinfo_item7,NULL,8,8,8,8,8,ITEM_L,ITEM_T + (ITEM_H + ITEM_GAP)*8,ITEM_W, ITEM_H,&IPPVinfo_list0,1)	\
LDEF_CON(&IPPVinfo_olist,IPPVinfo_item8,NULL,9,9,9,9,9,ITEM_L,ITEM_T + (ITEM_H + ITEM_GAP)*9,ITEM_W, ITEM_H,&IPPVinfo_list0,1)	\

LDEF_LISTBAR(IPPVinfo_olist,IPPVinfo_scb,9,SCB_L,SCB_T, SCB_W, SCB_H)

POBJECT_HEAD IPPVinfo_items[] =
{
	(POBJECT_HEAD)&IPPVinfo_item0,
	(POBJECT_HEAD)&IPPVinfo_item1,
	(POBJECT_HEAD)&IPPVinfo_item2,
	(POBJECT_HEAD)&IPPVinfo_item3,
	(POBJECT_HEAD)&IPPVinfo_item4,
	(POBJECT_HEAD)&IPPVinfo_item5,
	(POBJECT_HEAD)&IPPVinfo_item6,
	(POBJECT_HEAD)&IPPVinfo_item7,
	(POBJECT_HEAD)&IPPVinfo_item8,
};
#define LIST_STYLE (LIST_VER | LIST_NO_SLECT     | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)

LDEF_OL(g_win_IPPVinfo,IPPVinfo_olist,NULL, LST_L,LST_T,LST_W,LST_H,LIST_STYLE, 9, 9,IPPVinfo_items,&IPPVinfo_scb,NULL,NULL)

DEF_CONTAINER (win_IPPVinfo_con, NULL, &IPPVinfo_title, C_ATTR_ACTIVE, 0, \
                0, 0, 0, 0, 0, W_CON_L, W_CON_T, W_CON_W, W_CON_H, CON_IDX, CON_IDX, CON_IDX, CON_IDX,   \
                NULL, NULL,  \
                NULL, 1, 0 )
                
LDEF_WIN(g_win_IPPVinfo,&win_IPPVinfo_con, W_L,W_T,W_W, W_H, 1)

PTEXT_FIELD IPPVinfo_listtxt[] =
{
	&IPPVinfo_list0,
	&IPPVinfo_list1,
	&IPPVinfo_list2,
	&IPPVinfo_list3,
	&IPPVinfo_list4,
	&IPPVinfo_list5,
};

#define IPPVINFO_ITEM_TXT_CNT (sizeof(IPPVinfo_listtxt))/(sizeof(IPPVinfo_listtxt[0]))

UINT16 win_ippv_title_string[] =
{
	RS_SLOT_ID,
	RS_IPPV_PROGRAM_ID,
	RS_IPPV_STATUS,
	RS_IPPV_PRICE,
	RS_RECORD_ENABLE,
	RS_EXPIRED_TIME
};

OSD_RECT win_ippv_title_rect[] =
{
	{ITEM_L+ITEM_IDX_L,ITEM_T,100,ITEM_H},
	{ITEM_L+ITEM_IDX_L+100,ITEM_T,100,ITEM_H},
	{ITEM_L+ITEM_IDX_L+200,ITEM_T,100,ITEM_H},
	{ITEM_L+ITEM_IDX_L+300,ITEM_T,80,ITEM_H},
	{ITEM_L+ITEM_IDX_L+380,ITEM_T,80,ITEM_H},
	{ITEM_L+ITEM_IDX_L+460,ITEM_T,120,ITEM_H}
};
#endif

//include the header from xform 
#include "win_ca_IPPVinfo.h"

PTEXT_FIELD IPPVinfo_listtxt[] =
{
	&IPPVinfo_op0,
	&IPPVinfo_wal0,
	&IPPVinfo_prog0,
	&IPPVinfo_st0,
	&IPPVinfo_rec0,
	&IPPVinfo_price0,
	&IPPVinfo_expire0,
};

#define IPPVINFO_ITEM_TXT_CNT (sizeof(IPPVinfo_listtxt))/(sizeof(IPPVinfo_listtxt[0]))

/*******************************************************************************
*	Local vriable & function declare
*******************************************************************************/
extern void win_init_pstring(UINT8 num);
static void win_IPPVinfo_list_display(void);
static void ippv_txt_info_display(UINT16 info_idx,PCONTAINER pCon);
static INT32 IPPVinfo_got_info(void);
static void win_operator_IPPV_draw(OSD_RECT* pRect,UINT16 strid);
void	win_ippv_draw_title(void);
void win_IPPVinfo_open_title(POBJECT_HEAD pObj, UINT16 num);

extern UINT16 current_operator_ID;
SCDCAIppvInfo IPPV_program_info[CDCA_MAXNUM_IPPVP];
UINT16 total_IPPV_number = 0;

/*******************************************************************************
*	key mapping and event callback definition
*******************************************************************************/

static PRESULT IPPVinfo_list_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	UINT8 bID;
	PRESULT ret = PROC_PASS;
	VACTION unact;

	bID = OSD_GetObjID(pObj);
	switch(event)
	{
		case EVN_PRE_DRAW:
			ippv_txt_info_display((UINT16)bID,(PCONTAINER)pObj);
			break;
		case EVN_POST_DRAW:
			break;
		case EVN_UNKNOWN_ACTION:
			break;
		default:
			break;
	}
	return ret;
}

static VACTION IPPVinfo_list_keymap(POBJECT_HEAD pObj, UINT32 key)
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
	default:
		act = VACT_PASS;
	}

	return act;
}

static PRESULT IPPVinfo_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	OBJLIST* ol;

	UINT16 cnt;

	ol = &IPPVinfo_olist;
	cnt = OSD_GetObjListCount(ol);

	switch(event)
	{
		case EVN_PRE_DRAW:
			if (cnt==0)
				ret = PROC_LOOP;
			break;
		case EVN_POST_DRAW:
			break;
		default:
			break;

	}
	return ret;
}

static VACTION IPPVinfo_keymap(POBJECT_HEAD pObj, UINT32 key)
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

INT32 IPPVinfo_got_info()
{
	UINT16 ret;
	UINT16 total_num = CDCA_MAXNUM_IPPVP;
	SCDCAIppvInfo  total_program_info[CDCA_MAXNUM_IPPVP];
	
#if 1	
	MEMSET(total_program_info, 0, sizeof(total_program_info));
	ret = CDCASTB_GetIPPVProgram(current_operator_ID, total_program_info, &total_num);
	if (CDCA_RC_CARD_INVALID== ret)
	{
		win_popup_msg(NULL, NULL, RS_CAERR_NOSMC);
		return 0;
	}
	else if (CDCA_RC_DATA_NOT_FIND == ret)
	{
		win_popup_msg(NULL, NULL, RS_CAERR_DATA_NOT_FOUND);
		return 0;	
	}
	else if (CDCA_RC_DATASPACE_SMALL == ret)
	{
		win_popup_msg(NULL, NULL, RS_CAERR_BUFFER_SMALL);
		return 0;
	}
	else if (CDCA_RC_OK == ret)
	{
		//success, sort the IPPV prog info by Select Sorting Method
		UINT32 i, j, k;
		SCDCAIppvInfo tmp;

		for (i=0; i<total_num; i++)
		{
			k = i;
			for (j=i+1; j<total_num; j++)
			{
				/*if ((total_program_info[j].m_bySlotID < total_program_info[k].m_bySlotID)
					||((total_program_info[j].m_bySlotID == total_program_info[k].m_bySlotID) 
						&&(total_program_info[j].m_dwProductID<total_program_info[k].m_dwProductID)))*/
				if (total_program_info[j].m_dwProductID<total_program_info[k].m_dwProductID)
					k = j;
			}
			if (k != i)
			{
				MEMCPY(&tmp, &total_program_info[i], sizeof(SCDCAIppvInfo));
				MEMCPY(&total_program_info[i], &total_program_info[k], sizeof(SCDCAIppvInfo));
				MEMCPY(&total_program_info[k], &tmp, sizeof(SCDCAIppvInfo));
			}
		}
		//just display the viewed records
		MEMSET(IPPV_program_info, 0, sizeof(IPPV_program_info));
		j= 0;
		for(i=0; i<total_num; i++)
		{
			if (CDCA_IPPVSTATUS_VIEWED == total_program_info[i].m_byBookEdFlag) //viewed
			{
				MEMCPY(&IPPV_program_info[j], &total_program_info[i], sizeof(SCDCAIppvInfo));
				j++;
			}
		}
		total_IPPV_number = j;
	}
	else
	{
		win_popup_msg(NULL, NULL, RS_CAERR_UNKNOWN);
		return 0;
	}
#else // for self -testing 
	UINT32 i, j, k;
	SCDCAIppvInfo tmp;
	total_num = 201;

	MEMSET(total_program_info, 0, sizeof(total_program_info));
	for (k=0;k<total_num;k++)
	{
		i = total_num - k;
		total_program_info[i].m_dwProductID = 1200000290+i;
		total_program_info[i].m_bCanTape = i%2;
		total_program_info[i].m_byBookEdFlag = (i%3) +1;
		total_program_info[i].m_bySlotID = i%5;
		total_program_info[i].m_wPrice = i*3;
	}


		for (i=0; i<total_num; i++)
		{
			k = i;
			for (j=i+1; j<total_num; j++)
			{
				/*if ((total_program_info[j].m_bySlotID < total_program_info[k].m_bySlotID)
					||((total_program_info[j].m_bySlotID == total_program_info[k].m_bySlotID) 
						&&(total_program_info[j].m_dwProductID<total_program_info[k].m_dwProductID)))*/
				if (total_program_info[j].m_dwProductID<total_program_info[k].m_dwProductID)
					k = j;
			}
			if (k != i)
			{
				MEMCPY(&tmp, &total_program_info[i], sizeof(SCDCAIppvInfo));
				MEMCPY(&total_program_info[i], &total_program_info[k], sizeof(SCDCAIppvInfo));
				MEMCPY(&total_program_info[k], &tmp, sizeof(SCDCAIppvInfo));
			}
		}
		//just display the viewed records
		MEMSET(IPPV_program_info, 0, sizeof(IPPV_program_info));
		j= 0;
		for(i=0; i<total_num; i++)
		{
			if (CDCA_IPPVSTATUS_VIEWED == total_program_info[i].m_byBookEdFlag) //viewed
			{
				MEMCPY(&IPPV_program_info[j], &total_program_info[i], sizeof(SCDCAIppvInfo));
				j++;
			}
		}
		total_IPPV_number = j;
#endif	
	return 1;

}
extern struct help_item_resource win_opinfo_help[];
static PRESULT IPPVinfo_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT32 cnt = 0;
	INT32 iret;

	switch(event)
	{
	case EVN_PRE_OPEN:
		total_IPPV_number = 0;
		
        	iret = IPPVinfo_got_info();
		if(0 == iret)
		{
			ret = PROC_LEAVE;
			break;
		}
		OSD_SetObjListCount(&IPPVinfo_olist,total_IPPV_number);
		//for different card display
		if (total_IPPV_number <= OSD_GetObjListCurPoint(&IPPVinfo_olist))
		{
			//OSD_SetObjListNewPoint(&IPPVinfo_olist, 0);
			OSD_SetObjListCurPoint (&IPPVinfo_olist, 0);
			OSD_SetObjListNewPoint (&IPPVinfo_olist, 0);
			OSD_SetObjListTop (&IPPVinfo_olist, 0);
		}
		wincom_open_title_ext(RS_IPPV_PROGRAM, IM_TITLE_ICON_SYSTEM);
		wincom_open_help(win_opinfo_help, 2);
        	win_init_pstring(9);
		break;
	case EVN_POST_OPEN:
//		win_ippv_draw_title();
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
/*
static void win_operator_IPPV_draw(GUI_RECT* pRect,UINT16 strid)
{
	TEXT_FIELD* txt;
	UINT16 *string;
	
	//index: 0 for title, 1~9 for list item
	txt = &IPPVinfo_title;
	OSD_SetRect2(&txt->head.frame,pRect);
	
	if (txt->pString != NULL)
	{
		txt->pString = NULL;
	}
	OSD_SetTextFieldContent(txt, STRING_ID, (UINT32)strid);
	
	OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
}

void	win_ippv_draw_title(void)
{
	UINT8 i;

	for (i=0;i<6;i++)
	{
		win_operator_IPPV_draw(&win_ippv_title_rect[i], win_ippv_title_string[i]);
	}
}
*/
static void ippv_txt_info_display(UINT16 info_idx,PCONTAINER pCon)	
{
	UINT8 i,j,date[11];
	UINT16 top,index, curitem,count,strid=0;
	PTEXT_FIELD pTxt;
	OBJLIST *ol = &IPPVinfo_olist;
	PCONTAINER ppcon;
	
	curitem = OSD_GetObjListCurPoint(ol);
	count = OSD_GetObjListCount(ol);
	top  = OSD_GetObjListTop(ol);
	index = top +info_idx -1; //for container index is 1,2,....9

	ppcon = pCon;
	if (count == 0)
	{
		IPPVINFO_PRINT("@@@@ippv_txt_info_display: list count is 0!\n ");
	}
	else if (index<count)
	{
		for(i = 0;i < IPPVINFO_ITEM_TXT_CNT; i++)
		{
			pTxt= IPPVinfo_listtxt[i];
			OSD_SetObjRoot(pTxt,ppcon);
			OSD_SetContainerFocus(ppcon,1);
			OSD_SetObjRect(pTxt, pTxt->head.frame.uLeft, ppcon->head.frame.uTop, pTxt->head.frame.uWidth, pTxt->head.frame.uHeight);
					
			if (i==0)//operator id
			{
				OSD_SetTextFieldContent(pTxt, STRING_NUMBER, (UINT32)current_operator_ID);
				OSD_SetContainerNextObj(ppcon, pTxt);
			}

			if (i==1)//slot id
			{
				OSD_SetTextFieldContent(pTxt, STRING_NUMBER, (UINT32)IPPV_program_info[index].m_bySlotID);
			}
			if (i==2)//prog id
			{
				OSD_SetTextFieldContent(pTxt, STRING_NUMBER, (UINT32)IPPV_program_info[index].m_dwProductID);
			}
			if (i==3)//booking status
			{
				switch(IPPV_program_info[index].m_byBookEdFlag)
				{
					case CDCA_IPPVSTATUS_BOOKING:  // 1 for booking
						strid = 0;
						break;
					case 2: // 2 for expired????? (how to confirm??)
						strid = 0;
						break;
					case CDCA_IPPVSTATUS_VIEWED: // 3 for viewed
						strid = RS_IPPV_STATUS_VIEWED;
						break;
				}
				OSD_SetTextFieldContent(pTxt, STRING_ID, (UINT32)strid);
			}
			if (i==4)//tape
			{
				OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)((IPPV_program_info[index].m_bCanTape)?"Yes":"No"));
			}
			if (i==5)//price
			{
				OSD_SetTextFieldContent(pTxt, STRING_NUMBER, (UINT32)IPPV_program_info[index].m_wPrice);
			}
			if (i==6)//expired time
			{
				YMD_calculate(IPPV_program_info[index].m_wExpiredDate, date);
				OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)date);
			}
		}
		
		if ((curitem%9) == (info_idx-1))
			OSD_TrackObject((POBJECT_HEAD)ppcon, C_UPDATE_ALL);
		else
			OSD_DrawObject((POBJECT_HEAD)ppcon, C_UPDATE_ALL);
	}

	else 
	{
		for(i = 0;i < IPPVINFO_ITEM_TXT_CNT; i++)
		{
			pTxt= IPPVinfo_listtxt[i];
			OSD_SetObjRoot(pTxt,ppcon);
			OSD_SetObjRect(pTxt, pTxt->head.frame.uLeft, ppcon->head.frame.uTop, pTxt->head.frame.uWidth, pTxt->head.frame.uHeight);
			if (3==i)
				OSD_SetTextFieldContent(pTxt, STRING_ID, (UINT32)0);
			else
				OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)"");
		}
	}
}

void win_IPPVinfo_open_title(POBJECT_HEAD pObj, UINT16 num)
{
	UINT8 *p, info[20];
	UINT16 temp[20],title[50];

	if (NULL == title)
		return;
	MEMSET(temp, 0, sizeof(temp));
	MEMSET(title, 0, sizeof(title));
	p = OSD_GetUnicodeString(RS_IPPV_PROGRAM);
	ComUniStrCopyChar((UINT8 *)title, p);
	
	sprintf(info, " (NUM: %d)",num);
	ComAscStr2Uni(info, temp);
	ComUniStrCat(title,temp);
	
	wincom_open_title_ext(pObj, NULL, title,0);
}

