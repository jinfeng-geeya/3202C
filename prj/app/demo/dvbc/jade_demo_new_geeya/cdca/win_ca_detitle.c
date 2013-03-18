#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include "win_cas_com.h"
#include "win_ca_detitle.h"
/*******************************************************************************
*	Objects definition
*******************************************************************************/

#if 0
extern CONTAINER g_win_detitle;
extern CONTAINER win_detitle_con;

extern TEXT_FIELD detitle_title_index;
extern TEXT_FIELD detitle_title_status;
extern TEXT_FIELD detitle_title_code;

extern OBJLIST	 detitle_olist;

extern CONTAINER detitle_item0;
extern CONTAINER detitle_item1;
extern CONTAINER detitle_item2;
extern CONTAINER detitle_item3;
extern CONTAINER detitle_item4;

extern TEXT_FIELD detitle_index0;
extern TEXT_FIELD detitle_index1;
extern TEXT_FIELD detitle_index2;
extern TEXT_FIELD detitle_index3;
extern TEXT_FIELD detitle_index4;

extern TEXT_FIELD detitle_status0;
extern TEXT_FIELD detitle_status1;
extern TEXT_FIELD detitle_status2;
extern TEXT_FIELD detitle_status3;
extern TEXT_FIELD detitle_status4;

extern TEXT_FIELD detitle_code0;
extern TEXT_FIELD detitle_code1;
extern TEXT_FIELD detitle_code2;
extern TEXT_FIELD detitle_code3;
extern TEXT_FIELD detitle_code4;

/*help bar define*/
extern CONTAINER		detitle_helpbar;
extern BITMAP		detitle_del_bmp;
extern TEXT_FIELD		detitle_del_txt;
extern BITMAP		detitle_del_all_bmp;
extern TEXT_FIELD		detitle_del_all_txt;

static VACTION win_detitle_list_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT win_detitle_list_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION win_detitle_list_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT win_detitle_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION win_detitle_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT win_detitle_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

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
#define ITEM_NAME_W  280

#define LDEF_CON(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    win_detitle_list_item_con_keymap,win_detitle_list_item_con_callback,  \
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

#define LDEF_LIST_ITEM(root,varCon,varTxtidx,varTxtRcd,varTxtName,ID,l,t,w,h,idxstr,rcdid,namestr)	\
	LDEF_CON(&root,varCon,NULL,ID,ID,ID,ID,ID,l,t,w,h,&varTxtidx,1)	\
	LDEF_TXTIDX(&varCon,varTxtidx,&varTxtRcd ,0,0,0,0,0,l + ITEM_IDX_L, t,ITEM_IDX_W,h,0,idxstr)	\
	LDEF_TXTNAME(&varCon,varTxtRcd,&varTxtName,1,1,1,1,1,l + ITEM_RCD_L, t,ITEM_RCD_W,h,rcdid,NULL)\
	LDEF_TXTNAME(&varCon,varTxtName,NULL,1,1,1,1,1,l + ITEM_NAME_L, t,ITEM_NAME_W,h,0,namestr)

#define LDEF_OL(root,varOl,nxtObj,l,t,w,h,style,dep,count,flds,sb,mark,selary)	\
  DEF_OBJECTLIST(varOl,&root,nxtObj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_HL_IDX,LST_SL_IDX,LST_GRY_IDX,   \
    win_detitle_list_keymap,win_detitle_list_callback,    \
    flds,sb,mark,style,dep,count,selary)

#define LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    win_detitle_keymap,win_detitle_callback,  \
    nxtObj, focusID,0)

LDEF_TXTIDX(&g_win_detitle, detitle_title_index,&detitle_title_status,0,0,0,0,0,\
    ITEM_L+ITEM_IDX_L, ITEM_T,ITEM_IDX_W,ITEM_H,RS_DETITLE_INDEX,NULL)

LDEF_TXTIDX(&g_win_detitle, detitle_title_status, &detitle_title_code,0,0,0,0,0,\
    ITEM_L+ITEM_RCD_L, ITEM_T,ITEM_RCD_W,ITEM_H,RS_DETITLE_STATUS,NULL)

LDEF_TXTIDX(&g_win_detitle, detitle_title_code,&detitle_olist,0,0,0,0,0,\
    ITEM_L+ITEM_NAME_L, ITEM_T,ITEM_NAME_W,ITEM_H,RS_DETITLE_CODE,NULL)

LDEF_LIST_ITEM(detitle_olist,detitle_item0,detitle_index0,detitle_status0, detitle_code0, 1,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*1,ITEM_W, ITEM_H, display_strs[0], 0, display_strs[10])

LDEF_LIST_ITEM(detitle_olist,detitle_item1,detitle_index1,detitle_status1, detitle_code1, 2,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*2,ITEM_W, ITEM_H, display_strs[1], 0, display_strs[11])

LDEF_LIST_ITEM(detitle_olist,detitle_item2,detitle_index2,detitle_status2, detitle_code2, 3,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*3,ITEM_W, ITEM_H, display_strs[2], 0, display_strs[12])

LDEF_LIST_ITEM(detitle_olist,detitle_item3,detitle_index3,detitle_status3, detitle_code3, 4,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*4,ITEM_W, ITEM_H, display_strs[3], 0, display_strs[13])

LDEF_LIST_ITEM(detitle_olist,detitle_item4,detitle_index4,detitle_status4, detitle_code4, 5,\
	ITEM_L, ITEM_T + (ITEM_H + ITEM_GAP)*5,ITEM_W, ITEM_H, display_strs[4], 0, display_strs[14])


POBJECT_HEAD detitle_items[] =
{
	(POBJECT_HEAD)&detitle_item0,
	(POBJECT_HEAD)&detitle_item1,
	(POBJECT_HEAD)&detitle_item2,
	(POBJECT_HEAD)&detitle_item3,
	(POBJECT_HEAD)&detitle_item4,
};

#define LIST_STYLE (LIST_VER | LIST_NO_SLECT     | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)

LDEF_OL(g_win_detitle,detitle_olist,NULL, LST_L,LST_T,LST_W,LST_H,LIST_STYLE, 5, 5,detitle_items,NULL,NULL,NULL)

DEF_CONTAINER (win_detitle_con, NULL, &detitle_title_index, C_ATTR_ACTIVE, 0, \
                0, 0, 0, 0, 0, W_CON_L, W_CON_T, W_CON_W, W_CON_H, CON_IDX, CON_IDX, CON_IDX, CON_IDX,   \
                NULL, NULL,  \
                NULL, 1, 0 )
                
LDEF_WIN(g_win_detitle,&win_detitle_con, W_L,W_T,W_W, W_H, 1)
#endif
/*******************************************************************************
*	Local vriable & function declare
*******************************************************************************/
extern UINT16 current_operator_ID;
static UINT32 Detitle_Chknum[CDCA_MAXNUM_DETITLE+1];
UINT32 del_detitle_idx[CDCA_MAXNUM_DETITLE+1] = {0,};
static BOOL detitle_get_del_flag(UINT32 index)
{
	UINT32 wIndex;
	UINT8 bShiftBit;
    if(index>CDCA_MAXNUM_DETITLE)
        return FALSE;
	if (del_detitle_idx[index]==1)
		return TRUE;
	else
		return FALSE;
}

static void detitle_set_del_flag(UINT32 index)
{
	if (del_detitle_idx[index]==1)
	{
        del_detitle_idx[index]=0;    
	}else
	    del_detitle_idx[index]=1; 
}
static void detitle_set_alldel_flag()
{
    OBJLIST* ol;
    UINT16 cnt,i=0;
    ol = &detitle_olist;
	cnt = OSD_GetObjListCount(ol);
	for(i=0;i<cnt;i++)
	{
       del_detitle_idx[i]=1;
	}
}
void init_detitle_flag()
{
    MEMSET(del_detitle_idx,0,sizeof(del_detitle_idx));
}
void save_detitle()
{
    UINT8 back_saved,deldtl,i;
    UINT16 ret2;
    for(i=0;i<CDCA_MAXNUM_DETITLE+1;i++)
    {
        if(del_detitle_idx[i]==1)
            break;     
    }
    if(i==CDCA_MAXNUM_DETITLE+1)
        return;
    win_compopup_init(WIN_POPUP_TYPE_OKNO);
	win_compopup_set_msg(NULL, NULL, RS_DETITLE_DELPOP_CURRENT);
	win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
	ret2= win_compopup_open_ext(&back_saved);
	if (ret2 == WIN_POP_CHOICE_YES)
	{
		//to do
		for(i=0;i<CDCA_MAXNUM_DETITLE+1;i++)
		{
            if(del_detitle_idx[i]==1)
            {
    		    deldtl = CDCASTB_DelDetitleChkNum(current_operator_ID, Detitle_Chknum[i]);
            	if (FALSE == deldtl)
            	{
            		win_popup_msg(NULL, NULL, RS_OPERATION_ERROR);
                    break;
            	}
            }
		}
	}
}
UINT8 detitle_get_info(UINT16 *num)
{
	UINT16 ret, i, tmp=0;
	
	for (i=0;i<=CDCA_MAXNUM_DETITLE; i++)
	{
		Detitle_Chknum[i] = 0;
	}
	
	ret = CDCASTB_GetDetitleChkNums(current_operator_ID, Detitle_Chknum);
	if ((CDCA_RC_POINTER_INVALID == ret)||(CDCA_RC_CARD_INVALID == ret))
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
		//not sure how to keep the total detitle number
		for (i=0; i<CDCA_MAXNUM_DETITLE; i++)
		{
			if (Detitle_Chknum[i] != 0)
				tmp = i+1;
		}
		*num = tmp;
		return 1;
	}
	else
	{
		win_popup_msg(NULL, NULL, RS_CAERR_UNKNOWN);
	}

	return 0; // in case other error
}
void win_detitle_list_display()
{
	CONTAINER* con;
	TEXT_FIELD* text;
	OBJLIST* ol;		
	UINT16 index, cnt,iconID;
	BITMAP *bmp;
	ol = &detitle_olist;
	cnt = OSD_GetObjListCount(ol);
    BOOL del_flag;
	//argly :), may remove status item later
	OSD_SetTextFieldContent(&detitle_status0, STRING_ID, (UINT32)0);
	OSD_SetTextFieldContent(&detitle_status1, STRING_ID, (UINT32)0);
	OSD_SetTextFieldContent(&detitle_status2, STRING_ID, (UINT32)0);
	OSD_SetTextFieldContent(&detitle_status3, STRING_ID, (UINT32)0);
	OSD_SetTextFieldContent(&detitle_status4, STRING_ID, (UINT32)0);
	OSD_SetTextFieldContent(&detitle_status5, STRING_ID, (UINT32)0);
	OSD_SetTextFieldContent(&detitle_status6, STRING_ID, (UINT32)0);
	OSD_SetTextFieldContent(&detitle_status7, STRING_ID, (UINT32)0);
	OSD_SetTextFieldContent(&detitle_status8, STRING_ID, (UINT32)0);
	OSD_SetBitmapContent(&ca_detitle_delete_0, 0);
    OSD_SetBitmapContent(&ca_detitle_delete_1, 0);
    OSD_SetBitmapContent(&ca_detitle_delete_2, 0);
    OSD_SetBitmapContent(&ca_detitle_delete_3, 0);
    OSD_SetBitmapContent(&ca_detitle_delete_4, 0);
    OSD_SetBitmapContent(&ca_detitle_delete_5, 0);
    OSD_SetBitmapContent(&ca_detitle_delete_6, 0);
    OSD_SetBitmapContent(&ca_detitle_delete_7, 0);
    OSD_SetBitmapContent(&ca_detitle_delete_8, 0);
	for(index=0;index<cnt;index++)
    {
		con = (CONTAINER*)detitle_olist_ListField[index];
        
		del_flag = detitle_get_del_flag(index);
		
		/* index */
		text = (TEXT_FIELD*)OSD_GetContainerNextObj(con); 
		OSD_SetTextFieldContent(text, STRING_NUMBER, Detitle_Chknum[index]);

		/* status**/
		text = (TEXT_FIELD*)OSD_GetObjpNext(text);
		OSD_SetTextFieldContent(text, STRING_ID, (UINT32)RS_DETITLE_STATUS_READED);
		
		/* detitle confirm code**/
		//text = (TEXT_FIELD*)OSD_GetObjpNext(text);
		//OSD_SetTextFieldContent(text, STRING_NUMBER, (UINT32)Detitle_Chknum[index]);
		/* Delete */
		bmp = (BITMAP*)OSD_GetObjpNext(text);
		iconID = (del_flag) ?IM_TV_DEL: 0;
		OSD_SetBitmapContent(bmp, iconID);
	}
	
}

/*******************************************************************************
*	key mapping and event callback definition
*******************************************************************************/
static VACTION win_detitle_list_item_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	
	return act;
}
static PRESULT win_detitle_list_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;

	return ret;
}

#define VACT_DEL_SINGLE_DETITLE		(VACT_PASS+1)
#define VACT_DEL_ALL_DETITLE			(VACT_PASS+2)
static VACTION win_detitle_list_keymap(POBJECT_HEAD pObj, UINT32 key)
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
		case V_KEY_RED:
			act = VACT_DEL_SINGLE_DETITLE;
			break;
		case V_KEY_GREEN:
			act = VACT_DEL_ALL_DETITLE;
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}
static PRESULT win_detitle_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	OBJLIST* ol;
	POBJECT_HEAD temp;
	UINT8 back_saved, deldtl;

	UINT16 cnt,curitem,top,ret2,i;

	ol = &detitle_olist;
	curitem = OSD_GetObjListCurPoint(ol);
	cnt = OSD_GetObjListCount(ol);

	switch(event)
	{
		case EVN_PRE_DRAW:
			win_detitle_list_display();
			break;
		case EVN_POST_DRAW:
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1>>16);

			if (unact == VACT_DEL_SINGLE_DETITLE)
			{
                if(cnt!=0)
                {
                    detitle_set_del_flag(curitem);
                    OSD_TrackObject((POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
                }
				/*win_compopup_init(WIN_POPUP_TYPE_OKNO);
				win_compopup_set_msg(NULL, NULL, RS_DETITLE_DELPOP_CURRENT);
				win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
				ret2= win_compopup_open_ext(&back_saved);
				if (ret2 == WIN_POP_CHOICE_YES)
				{
					//to do
					deldtl = CDCASTB_DelDetitleChkNum(current_operator_ID, Detitle_Chknum[curitem]);
					if (FALSE == deldtl)
					{
						win_popup_msg(NULL, NULL, RS_OPERATION_ERROR);
					}
					ret = PROC_LEAVE;
				}*/
			}
			else if(unact == VACT_DEL_ALL_DETITLE)
			{
                detitle_set_alldel_flag();
                OSD_TrackObject((POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
				/*win_compopup_init(WIN_POPUP_TYPE_OKNO);
				win_compopup_set_msg(NULL, NULL, RS_DETITLE_DELPOP_ALL);
				win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
				ret2= win_compopup_open_ext(&back_saved);
				if (ret2 == WIN_POP_CHOICE_YES)
				{
					//to do
					deldtl = CDCASTB_DelDetitleChkNum(current_operator_ID, 0);
					if (FALSE == deldtl)
					{
						win_popup_msg(NULL, NULL, RS_OPERATION_ERROR);
					}
					ret = PROC_LEAVE;
				}*/
			}
			break;

	}
	return ret;
}

static VACTION win_detitle_keymap(POBJECT_HEAD pObj, UINT32 key)
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
extern struct help_item_resource win_opinfo_help[];
static PRESULT win_detitle_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
 	OBJLIST* ol = &detitle_olist;
	UINT16 tmp, num=0;
	struct help_item_resource detitle_help[] =
	{
		{1,IM_EPG_COLORBUTTON_RED,RS_HELP_DELETE},
		{1,IM_EPG_COLORBUTTON_GREEN,RS_DELETE_ALL},
		{1,IM_HELP_ICON_TB,RS_HELP_SELECT},
	};
	switch(event)
	{
		case EVN_PRE_OPEN:
            init_detitle_flag();
			tmp = detitle_get_info(&num);
			if (0 == tmp)
			{
				ret = PROC_LEAVE;
				break;
			}
			OSD_SetObjListCount(ol, num);
			if (num <= OSD_GetObjListCurPoint(&detitle_olist))
			{
				OSD_SetObjListCurPoint (&detitle_olist, 0);
				OSD_SetObjListNewPoint (&detitle_olist, 0);
				OSD_SetObjListTop (&detitle_olist, 0);
				//OSD_SetObjListNewPoint(&detitle_olist, 0);		
			}
			wincom_open_title_ext(RS_DETITLE, IM_TITLE_ICON_SYSTEM);
			wincom_open_help(detitle_help, 3);
        	win_init_pstring(18);
			break;
		case EVN_POST_OPEN:
			break;
		case EVN_PRE_CLOSE:
			/* Make OSD not flickering */
            save_detitle();
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;	
			break;
		case EVN_POST_CLOSE:
			//TFCAS_WriteFlash();//write to flash at once, ignore the delay
			break;
	}

	return ret;
}

