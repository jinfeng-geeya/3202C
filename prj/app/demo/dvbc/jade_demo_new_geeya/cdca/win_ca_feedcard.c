#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
//#include <api/libge/osd_lib.h>
//#include <api/libosd/osd_lib.h>

#include "win_cas_com.h"

//#define CAS_FEED_DEBUG
#ifdef CAS_FEED_DEBUG
#define CAS_FEED_PRINTF	 libc_printf
#else
#define CAS_FEED_PRINTF(...) do{}while(0)
#endif
#if 0
/*******************************************************************************
* Objects declaration
*******************************************************************************/
extern CONTAINER g_win_feedcard;
extern CONTAINER win_feedcard_con;

extern CONTAINER feedcard_con1;
extern CONTAINER feedcard_con2;
extern CONTAINER feedcard_con3;
extern CONTAINER feedcard_con4;

extern TEXT_FIELD feedcard_txt_name1;
extern TEXT_FIELD feedcard_txt_name2;
extern TEXT_FIELD feedcard_txt_name3;
extern TEXT_FIELD feedcard_txt_name4;

extern TEXT_FIELD feedcard_txt_info1;
extern TEXT_FIELD feedcard_txt_info2;
extern TEXT_FIELD feedcard_txt_info3;
extern TEXT_FIELD feedcard_txt_info4;

static VACTION win_feedcard_con_keymap (POBJECT_HEAD pObj, UINT32 key);
static PRESULT win_feedcard_con_callback (POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION win_feedcard_keymap (POBJECT_HEAD pObj, UINT32 key);
static PRESULT win_feedcard_callback (POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

#define WIN_SH_IDX	  WSTL_WIN_MIDDLE_BG//WSTL_COMMON_BACK_2
#define WIN_HL_IDX	  WSTL_WIN_MIDDLE_BG//WSTL_COMMON_BACK_2
#define WIN_SL_IDX	  WSTL_WIN_MIDDLE_BG//WSTL_COMMON_BACK_2
#define WIN_GRY_IDX  WSTL_WIN_MIDDLE_BG//WSTL_COMMON_BACK_2

#define CON_IDX	        WSTL_WIN_2

#define LST_SH_IDX		WSTL_NOSHOW_IDX//WSTL_BUTTON_01
#define LST_HL_IDX		WSTL_NOSHOW_IDX//WSTL_BUTTON_01
#define LST_SL_IDX		WSTL_NOSHOW_IDX//WSTL_BUTTON_01
#define LST_GRY_IDX		WSTL_NOSHOW_IDX//WSTL_BUTTON_01

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

#define W_L		0
#define W_T		90
#define W_W		720
#define W_H		400

#define W_CON_L		66
#define W_CON_T	102
#define W_CON_W	604
#define W_CON_H     	320

#define CON_L 84
#define CON_T 142
#define CON_W 560
#define CON_H 30
#define CON_GAP 10

#define TXTN_L_OF 0
#define TXTN_T_OF ((CON_H-TXTN_H)/2)
#define TXTN_W 280
#define TXTN_H 24

#define TXTS_L_OF   (CON_W-TXTS_W)
#define TXTS_T_OF   TXTN_T_OF
#define TXTS_W      280
#define TXTS_H      TXTN_H


#define LDEF_CON(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    win_feedcard_con_keymap,win_feedcard_con_callback,  \
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
    C_ALIGN_LEFT | C_ALIGN_VCENTER, 20,0,resID,str)

#define LDEF_MM_ITEM(root,varCon,nxtObj,varTxtName,varTxtset,ID,IDu,IDd,l,t,w,h,nameID,setstr)	\
	LDEF_CON(&root,varCon,nxtObj,ID,ID,ID,IDu,IDd,l,t,w,h,&varTxtName,1)	\
	LDEF_TXTNAME(&varCon,varTxtName,&varTxtset,0,0,0,0,0,l + TXTN_L_OF, t + TXTN_T_OF,TXTN_W,TXTN_H,nameID,NULL)	\
	LDEF_TXTSET(&varCon,varTxtset,  NULL        ,1,1,1,1,1,l + TXTS_L_OF, t + TXTS_T_OF,TXTS_W,TXTS_H,0,setstr)

LDEF_MM_ITEM(g_win_feedcard, feedcard_con1, &feedcard_con2, feedcard_txt_name1, feedcard_txt_info1, 1, 2, 4,	\
                CON_L, CON_T + (CON_H + CON_GAP)*0, CON_W, CON_H, RS_FEEDCARD_STATUS, display_strs[0])

LDEF_MM_ITEM(g_win_feedcard, feedcard_con2, &feedcard_con3, feedcard_txt_name2, feedcard_txt_info2, 2, 3, 1,	\
                CON_L, CON_T + (CON_H + CON_GAP)*1, CON_W, CON_H, RS_FEEDCARD_PERIOD, display_strs[1])

LDEF_MM_ITEM(g_win_feedcard, feedcard_con3, &feedcard_con4, feedcard_txt_name3, feedcard_txt_info3, 3, 4, 2,	\
                CON_L, CON_T + (CON_H + CON_GAP)*2, CON_W, CON_H, RS_FEEDCARD_LAST_FEEDING, display_strs[2])

LDEF_MM_ITEM(g_win_feedcard, feedcard_con4, NULL, feedcard_txt_name4, feedcard_txt_info4, 4, 1, 3,	\
                CON_L, CON_T + (CON_H + CON_GAP)*3, CON_W, CON_H, RS_FEEDCARD_FEED, display_strs[3])

DEF_CONTAINER (win_feedcard_con, NULL, &feedcard_con1, C_ATTR_ACTIVE, 0, \
                0, 0, 0, 0, 0, W_CON_L, W_CON_T, W_CON_W, W_CON_H, CON_IDX, CON_IDX, CON_IDX, CON_IDX,   \
                NULL, NULL,  \
                NULL, 1, 0)
                
DEF_CONTAINER (g_win_feedcard, NULL, NULL, C_ATTR_ACTIVE, 0, \
                1, 0, 0, 0, 0, W_L, W_T, W_W, W_H, WIN_SH_IDX, WIN_HL_IDX, WIN_SL_IDX, WIN_GRY_IDX,   \
                win_feedcard_keymap, win_feedcard_callback,  \
                (POBJECT_HEAD)&win_feedcard_con, 4, 0)
#endif

//include the header from xform 
#include "win_ca_feedcard.h"

/*******************************************************************************
*	Local functions & variables declare
*******************************************************************************/
#if 0
extern UINT16 operator_num;
extern UINT16 CAS_operator_ID[CDCA_MAXNUM_OPERATOR];
extern UINT16 current_operator_ID;
//extern STFCAOperatorInfo TFCAS_operator_info[TFCA_MAXLEN_OPERATOR];
static UINT8 card_is_child = 0; //0 is parent card, !0 is child card
UINT8 From_op_info_menu = 0;
CDCA_BOOL  pbIsCanFeed;		//wft 2010/08/10
static void feedcard_show_info(UINT32 str_id)
{
	OSD_SetTextFieldContent(&feedcard_info, STRING_ID, str_id);
	OSD_DrawObject((POBJECT_HEAD)&feedcard_info, C_UPDATE_ALL);
	osal_task_sleep(1000);
}

//0: not to update directly, 1 update immediately
static UINT8 win_feedcard_display(UINT8 update)
{
	PTEXT_FIELD pTxt;
	CONTAINER* con;
	OBJLIST* ol;	
	UINT8 refresh = 0;
	UINT16 tmp[30]={0,};
	UINT16 ret, delaytime,strid;
	UINT32 lastfeedtime;
	UINT16 page,index;
    char pParentCardSN[10];
    MEMSET(pParentCardSN,0,sizeof(pParentCardSN));
    //CDCA_BOOL  pbIsCanFeed;

	ol = &feedcard_olst;

	page = OSD_GetObjListPage(ol); 
	if (operator_num != 0)
	{
		for(index=0;index<page;index++)
	    {
	    	
			ret = CDCASTB_GetOperatorChildStatus(CAS_operator_ID[index], &card_is_child, &delaytime, &lastfeedtime,pParentCardSN,&pbIsCanFeed);

	if(card_is_child == 0) pbIsCanFeed = 0;	//wft 2010/08/10 当前智能卡为母卡,喂养控制位必须清零
	CAS_FEED_PRINTF("ret=%d, pParentCardSN=%s, 	pbIsCanFeed=%d\n",ret, pParentCardSN, pbIsCanFeed);	//wft
	
	if ((CDCA_RC_POINTER_INVALID == ret) ||(CDCA_RC_CARD_INVALID == ret))
			{
				//win_popup_msg(NULL, NULL, RS_CAERR_NOSMC);
				refresh = 0;
			}
	else if (CDCA_RC_CARD_NOTSUPPORT == ret)
			{
				//win_popup_msg(NULL, NULL, RS_CAERR_NOTSUPPORT);
				refresh = 0;
			}
	else if (CDCA_RC_DATA_NOT_FIND == ret)
			{
				//win_popup_msg(NULL, NULL, RS_CAERR_DATA_NOT_FOUND);
				refresh = 0;
			}
	else if (CDCA_RC_OK == ret)
			{
				refresh = 1;
			}
			else//wokao, I see "TFCAS_UNKNOWN" in the return value, API doesn't mention it
			{
				//win_popup_msg(NULL, NULL, RS_CAERR_UNKNOWN);
				refresh = 0;
			}

			con = (CONTAINER*)feedcard_olst_ListField[index];

			if (0 == refresh || index>=operator_num)//clear all disp info
			{
				/* operator */
				pTxt = (TEXT_FIELD*)OSD_GetContainerNextObj(con);
				OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)" ");

				/* status */
				pTxt = (TEXT_FIELD*)OSD_GetObjpNext(pTxt); 
				strid = 0;
				pTxt->pString = NULL;
				OSD_SetTextFieldContent(pTxt, STRING_ID, (UINT32)strid);

				/* feed period*/
				pTxt = (TEXT_FIELD*)OSD_GetObjpNext(pTxt);
					OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)" ");

				/* last feeding */
				pTxt = (TEXT_FIELD*)OSD_GetObjpNext(pTxt);
				if (NULL == pTxt->pString)
					pTxt->pString = display_strs[3];
				OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)" ");

				if (update)
				{
					OSD_TrackObject((POBJECT_HEAD)&feedcard_olst,C_UPDATE_ALL);
//					OSD_TrackObject((POBJECT_HEAD)con, C_UPDATE_ALL);
				}
			}
			else
			{
				if (0 == card_is_child)//parent card
				{
					/* operator */
					pTxt = (TEXT_FIELD*)OSD_GetContainerNextObj(con);
					OSD_SetTextFieldContent(pTxt, STRING_NUMBER, (UINT32)CAS_operator_ID[index]);

					/* status */
					pTxt = (TEXT_FIELD*)OSD_GetObjpNext(pTxt); 
					strid = RS_FEEDCARD_TYPE_PARENT;
					pTxt->pString = NULL;
					OSD_SetTextFieldContent(pTxt, STRING_ID, (UINT32)strid);

					/* feed period*/
					pTxt = (TEXT_FIELD*)OSD_GetObjpNext(pTxt);
					OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)"");

					/* last feeding */
					pTxt = (TEXT_FIELD*)OSD_GetObjpNext(pTxt);
					if (NULL == pTxt->pString)
						pTxt->pString = display_strs[3];
					OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)" ");
				}
				else   //child card
				{
					/* operator */
					pTxt = (TEXT_FIELD*)OSD_GetContainerNextObj(con);
					OSD_SetTextFieldContent(pTxt, STRING_NUMBER, (UINT32)CAS_operator_ID[index]);

					/* status */
					pTxt = (TEXT_FIELD*)OSD_GetObjpNext(pTxt); 
					strid = RS_FEEDCARD_TYPE_CHILD;
					pTxt->pString = NULL;
					OSD_SetTextFieldContent(pTxt, STRING_ID, (UINT32)strid);

					/* feed period*/
					pTxt = (TEXT_FIELD*)OSD_GetObjpNext(pTxt);
					OSD_SetTextFieldContent(pTxt, STRING_NUMBER, (UINT32)delaytime);

					/* last feeding */
					pTxt = (TEXT_FIELD*)OSD_GetObjpNext(pTxt);
					if (0 == lastfeedtime)
					{	pTxt->pString = NULL;
						OSD_SetTextFieldContent(pTxt, STRING_ID, (UINT32)RS_FEEDCARD_NEVERFEED);
					}
					else
					{
						if (NULL == pTxt->pString)
							pTxt->pString = display_strs[3];
						YMD_calculate((lastfeedtime>>16),tmp);
						sprintf(tmp, "%s %02d:%02d:%02d", tmp, (lastfeedtime>>11)&0x1f, (lastfeedtime>>5)&0x3f,(lastfeedtime&0x1f)*2);
						OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)tmp);
					}
				}

			}
			if (update)
			{
				OSD_TrackObject((POBJECT_HEAD)&feedcard_olst,C_UPDATE_ALL);
//				OSD_TrackObject((POBJECT_HEAD)con, C_UPDATE_ALL);
			}
		}
	}
}

static void feedcard_proc(UINT16 operator_id,UINT8 child)
{
	UINT8 save, tmp, len, feeddata[256+1] = {0,};
	UINT16 ret;

	UINT16 delaytime2;	//wft 2010/08/09 子母卡喂养周期最大为720小时,delaytime更改为双字节
	UINT32 lastfeedtime2;
	UINT8 start_from_child = 1;		//wft 2010/09/16 喂养是从子卡开始吗  1: yes   0: no

	char pParentCardSN2[20];
	MEMSET(pParentCardSN2,0,sizeof(pParentCardSN2));

	CAS_FEED_PRINTF("feed proc!\n");
	//wft 2010/09/16 喂养开始的时候判断是否能够喂养
	if(child)//wft 2010/08/10 母卡在机顶盒内的时候,开始喂养直接读取母卡数据,然后提示插入子卡,所以提示插入母卡只针对子卡在的情形
	{
		CAS_FEED_PRINTF("child feed!\n");
		start_from_child = 1;
		//ask to insert parent card first
		win_compopup_init ( WIN_POPUP_TYPE_OK );
		
		if(pbIsCanFeed!=0)		//可以喂养提示插入母卡
		{
			CAS_FEED_PRINTF("insert parent!\n");
			win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_INSERT_PARENT);
			win_compopup_open_ext ( &save );
		}
		else
		{
			CAS_FEED_PRINTF("child can't feed!\n");
			win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_CHILD_FAIL);
			win_compopup_open_ext ( &save );
			return;
		}
	}
	else start_from_child = 0;	//wft 2010/09/16 直接从母卡读数据
	
	//step 1. read data from parent card
	ret = CDCASTB_ReadFeedDataFromParent(current_operator_ID, feeddata, &len);
	if(CDCA_RC_OK == ret)
	{
		//step 2: ask to insert child card
		win_compopup_init (WIN_POPUP_TYPE_OK);
		win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_INSERT_CHILD);
		win_compopup_open_ext (&save);


		if(start_from_child == 0)
		{
			ret = CDCASTB_GetOperatorChildStatus(current_operator_ID, &card_is_child, &delaytime2, &lastfeedtime2,pParentCardSN2,&pbIsCanFeed);
			if(card_is_child && (pbIsCanFeed == 0))
			{
				CAS_FEED_PRINTF("child can't to feed!\n");
				win_compopup_init ( WIN_POPUP_TYPE_OK );
				win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_CHILD_FAIL);
				win_compopup_open_ext ( &save );
				return;
			}
		}
		//write to child card
		ret = CDCASTB_WriteFeedDataToChild(feeddata, len);
		if (CDCA_RC_OK == ret)
		{
/*			win_compopup_init (WIN_POPUP_TYPE_OK);
			win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_OK);
			win_compopup_open_ext (&save);*/
			feedcard_show_info(RS_FEEDCARD_OK);
			return;
		}
		else
		{
/*			win_compopup_init (WIN_POPUP_TYPE_OK);
			win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_FAIL);
			win_compopup_open_ext (&save);*/
			feedcard_show_info(RS_FEEDCARD_FAIL);
			return;
		}
		
	}
	else
	{
/*		win_compopup_init (WIN_POPUP_TYPE_OK);
		win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_FEED_FAIL);
		win_compopup_open_ext (&save);*/
		feedcard_show_info(RS_FEEDCARD_FEED_FAIL);
		return;
	}
}

/*******************************************************************************
*	Window's keymap, proc and  callback
*******************************************************************************/
static VACTION feedcard_list_keymap(POBJECT_HEAD pObj, UINT32 key)
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
	case V_KEY_ENTER:
		act = VACT_ENTER;
		break;
	default:
		act = VACT_PASS;
	}

	return act;
}

static PRESULT feedcard_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	OBJLIST* ol;
	UINT16 curitem;
	VACTION unact;
	
	ol = &feedcard_olst;
	curitem = OSD_GetObjListCurPoint(ol);

	switch(event)
	{
	case EVN_PRE_DRAW:
		break;
	case EVN_UNKNOWN_ACTION:
		unact = (VACTION)(param1>>16);
		if(unact == VACT_ENTER) 
		{
			feedcard_proc(CAS_operator_ID[curitem],card_is_child);
			win_feedcard_display(1);//update the windows
		}
		break;
	}
	return ret;
}

static VACTION win_feedcard_keymap (POBJECT_HEAD pObj, UINT32 vkey)
{
	VACTION act = VACT_PASS;

	switch (vkey)
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

struct help_item_resource win_feedcard_help[] =
{
    {0,RS_HELP_ENTER,RS_FEEDCARD_FEED},
};
static PRESULT win_feedcard_callback (POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT32 retval;
	UINT16 channel;

	switch (event)
	{
		case EVN_PRE_OPEN:
			operator_num = 0;
			MEMSET(CAS_operator_ID,0, sizeof(CAS_operator_ID));
			//MEMSET(TFCAS_operator_info, 0, sizeof(TFCAS_operator_info));
			if(From_op_info_menu == 0)
			{
		        retval = operator_got_info();
				if (0 == retval)
				{
					ret = PROC_LEAVE;
					break;
				}
			}
			else
			{
				operator_num = 1;
				CAS_operator_ID[0] = current_operator_ID;
				From_op_info_menu = 0;
			}
			wincom_open_title_ext(RS_PARENT_CARD, IM_TITLE_ICON_SYSTEM);
			wincom_open_help(win_feedcard_help, 1);
			win_feedcard_display(0);//update by osd lib
			OSD_SetTextFieldContent(&feedcard_info, STRING_ID, (UINT32)0);
			OSD_SetObjListCount(&feedcard_olst, operator_num);	
			break;
		case EVN_POST_OPEN:
			break;
		case EVN_PRE_DRAW:
			break;
		case EVN_PRE_CLOSE:
        	*((UINT32*)param2)&= ~C_CLOSE_CLRBACK_FLG;				
			break;
		case EVN_POST_CLOSE:
			break;
	}
	return ret;
}
#endif
extern UINT16 current_operator_ID;
static UINT8 card_is_child = 0; //0 is parent card, !0 is child card

static UINT8 bfeedOver = 0;

CDCA_BOOL  pbIsCanFeed;		//wft 2010/08/10
//0: not to update directly, 1 update immediately
static UINT8 win_feedcard_display(UINT8 update)
{
	PTEXT_FIELD pTxt;
	UINT8 refresh = 0;
	UINT8 tmp[30]={0,};
	UINT16 ret, strid ,delaytime;	//wft 2010/08/09 子母卡喂养周期最大为720小时,delaytime更改为双字节
	UINT32 lastfeedtime;
	char pParentCardSN[20];
	MEMSET(pParentCardSN,0,sizeof(pParentCardSN));
	//CDCA_BOOL  pbIsCanFeed;
	ret = CDCASTB_GetOperatorChildStatus(current_operator_ID, &card_is_child, &delaytime, &lastfeedtime,pParentCardSN,&pbIsCanFeed);

	if(card_is_child == 0) pbIsCanFeed = 0;	//wft 2010/08/10 当前智能卡为母卡,喂养控制位必须清零
	libc_printf("ret=%d, pParentCardSN=%s, 	pbIsCanFeed=%d\n",ret, pParentCardSN, pbIsCanFeed);	//wft
	if ((CDCA_RC_POINTER_INVALID == ret) ||(CDCA_RC_CARD_INVALID == ret))
	{
		//win_popup_msg(NULL, NULL, RS_CAERR_NOSMC);
		refresh = 0;
	}
	else if (CDCA_RC_CARD_NOTSUPPORT == ret)
	{
		//win_popup_msg(NULL, NULL, RS_CAERR_NOTSUPPORT);
		refresh = 0;
	}
	else if (CDCA_RC_DATA_NOT_FIND == ret)
	{
		//win_popup_msg(NULL, NULL, RS_CAERR_DATA_NOT_FOUND);
		refresh = 0;
	}
	else if (CDCA_RC_OK == ret)
	{
		refresh = 1;
	}
	else//wokao, I see "CDCA_RC_UNKNOWN" in the return value, API doesn't mention it
	{
		//win_popup_msg(NULL, NULL, RS_CAERR_UNKNOWN);
		refresh = 0;
	}
	if (0 == refresh)//clear all disp info
	{
		strid = 0;
		pTxt = &feedcard_txt_info1;
		pTxt->pString = NULL;
		OSD_SetTextFieldContent(pTxt, STRING_ID, (UINT32)strid);

		pTxt = &feedcard_txt_info5;
		OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)"");
        
		pTxt = &feedcard_txt_info2;
		OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)"");

		pTxt = &feedcard_txt_info3;
		if (NULL == pTxt->pString)
			pTxt->pString = display_strs[2];
		OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)" ");

		pTxt = &feedcard_txt_info4;
		OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)"");

		pTxt = &feedcard_txt_info6;
		OSD_SetTextFieldContent(pTxt, STRING_NUMBER, (UINT32)"");
		
		if (update)
		{
			OSD_DrawObject((POBJECT_HEAD)&g_win_feedcard,C_UPDATE_ALL);
			OSD_TrackObject((POBJECT_HEAD)&feedcard_con4, C_UPDATE_ALL);
		}
		return 0;
	}
	else
	{
		if (0 == card_is_child) //parent card
		{
			strid = RS_FEEDCARD_TYPE_PARENT;
			pTxt = &feedcard_txt_info1;
			pTxt->pString = NULL;
			OSD_SetTextFieldContent(pTxt, STRING_ID, (UINT32)strid);

			pTxt = &feedcard_txt_info5;
			OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)"");
        
			pTxt = &feedcard_txt_info2;
			OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)"");

			pTxt = &feedcard_txt_info3;
			if (NULL == pTxt->pString)
				pTxt->pString = display_strs[2];
			OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)" ");

			pTxt = &feedcard_txt_info4;
			OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)"");
			
			pTxt = &feedcard_txt_info6;
			OSD_SetTextFieldContent(pTxt, STRING_NUMBER, (UINT32)current_operator_ID);
		}
		else   //child card
		{
			strid = RS_FEEDCARD_TYPE_CHILD;
			pTxt = &feedcard_txt_info1;
			pTxt->pString = NULL;
			OSD_SetTextFieldContent(pTxt, STRING_ID, (UINT32)strid);

			pTxt = &feedcard_txt_info5;
			OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)pParentCardSN);
        
			pTxt = &feedcard_txt_info2;
			OSD_SetTextFieldContent(pTxt, STRING_NUMBER, (UINT32)delaytime);

			pTxt = &feedcard_txt_info3;
			if (0 == lastfeedtime)
			{	pTxt->pString = NULL;
				OSD_SetTextFieldContent(pTxt, STRING_ID, (UINT32)RS_FEEDCARD_NEVERFEED);
			}
			else
			{
				if (NULL == pTxt->pString)
					pTxt->pString = display_strs[2];
				MEMSET(tmp, 0, sizeof(tmp));
				YMD_calculate((lastfeedtime>>16),tmp);
				sprintf(tmp, "%s %02d:%02d:%02d", tmp, (lastfeedtime>>11)&0x1f, (lastfeedtime>>5)&0x3f,(lastfeedtime&0x1f)*2);
				OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)tmp);
			}

			pTxt = &feedcard_txt_info4;
			OSD_SetTextFieldContent(pTxt, STRING_ANSI, (UINT32)"");
			
			pTxt = &feedcard_txt_info6;
			OSD_SetTextFieldContent(pTxt, STRING_NUMBER, (UINT32)current_operator_ID);
		}

	}
	if (update)
	{
		OSD_DrawObject((POBJECT_HEAD)&g_win_feedcard,C_UPDATE_ALL);
		OSD_TrackObject((POBJECT_HEAD)&feedcard_con4, C_UPDATE_ALL);
	}
	return 1;
}

static void feedcard_proc(UINT8 child)
{
	UINT8 save, tmp, len, feeddata[256+1] = {0,};
	UINT16 ret;

	UINT16 delaytime2;	//wft 2010/08/09 子母卡喂养周期最大为720小时,delaytime更改为双字节
	UINT32 lastfeedtime2;
	UINT8 start_from_child = 1;		//wft 2010/09/16 喂养是从子卡开始吗  1: yes   0: no

	char pParentCardSN2[20];
	MEMSET(pParentCardSN2,0,sizeof(pParentCardSN2));

	//wft 2010/09/16 喂养开始的时候判断是否能够喂养
	if(child)//wft 2010/08/10 母卡在机顶盒内的时候,开始喂养直接读取母卡数据,然后提示插入子卡,所以提示插入母卡只针对子卡在的情形
	{
		start_from_child = 1;
		//ask to insert parent card first
		win_compopup_init ( WIN_POPUP_TYPE_OK );
		
		if(pbIsCanFeed!=0)		//可以喂养提示插入母卡
		{
			win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_INSERT_PARENT);
			win_compopup_open_ext ( &save );
		}
		else
		{
			win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_CHILD_FAIL);
			win_compopup_open_ext ( &save );
			return;
		}
	}
	else start_from_child = 0;	//wft 2010/09/16 直接从母卡读数据
	
	//step 1. read data from parent card
	ret = CDCASTB_ReadFeedDataFromParent(current_operator_ID, feeddata, &len);
	if(CDCA_RC_OK == ret)
	{
		//step 2: ask to insert child card
		win_compopup_init ( WIN_POPUP_TYPE_OK );
		win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_INSERT_CHILD);
		win_compopup_open_ext ( &save );


		if(start_from_child == 0)
		{
			ret = CDCASTB_GetOperatorChildStatus(current_operator_ID, &card_is_child, &delaytime2, &lastfeedtime2,pParentCardSN2,&pbIsCanFeed);
			if(card_is_child && (pbIsCanFeed == 0))
			{
				win_compopup_init ( WIN_POPUP_TYPE_OK );
				win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_CHILD_FAIL);
				win_compopup_open_ext ( &save );
				return;
			}
		}

		//write to child card
		ret = CDCASTB_WriteFeedDataToChild(feeddata, len);
		if (CDCA_RC_OK == ret)
		{		
			win_compopup_init ( WIN_POPUP_TYPE_OK );
			win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_OK);
			win_compopup_open_ext ( &save );
			return;
		}
		else
		{			
			win_compopup_init ( WIN_POPUP_TYPE_OK );
			win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_FAIL);
			win_compopup_open_ext ( &save );
			return;
		}
		
	}
	else
	{		
		win_compopup_init ( WIN_POPUP_TYPE_OK );
		win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_FEED_FAIL);
		win_compopup_open_ext ( &save );
		return;
	}
/*

	if(child && (pbIsCanFeed==0))	//wft 2010/08/10 子卡且喂养时间未到不允许喂养
	{
		win_compopup_init ( WIN_POPUP_TYPE_OK );
		win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_CHILD_FAIL);
		win_compopup_open_ext ( &save );
		return;
	}

	if(child)//wft 2010/08/10 母卡在机顶盒内的时候,开始喂养直接读取母卡数据,然后提示插入子卡,所以提示插入母卡只针对子卡在的情形
	{
		//ask to insert parent card first
		win_compopup_init ( WIN_POPUP_TYPE_OK );
		win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_INSERT_PARENT);
		win_compopup_open_ext ( &save );
	}
	
	//step 1. read data from parent card
	ret = CDCASTB_ReadFeedDataFromParent(current_operator_ID, feeddata, &len);
	if(CDCA_RC_OK == ret)
	{
		//step 2: ask to insert child card
		win_compopup_init ( WIN_POPUP_TYPE_OK );
		win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_INSERT_CHILD);
		win_compopup_open_ext ( &save );

		//write to child card

		//* wft 2010/08/10 在对子卡进行数据写的时候,判断当前子卡是否可以进行喂养 
		ret = CDCASTB_GetOperatorChildStatus(current_operator_ID, &card_is_child, &delaytime2, &lastfeedtime2,pParentCardSN2,&pbIsCanFeed);
		
		if(card_is_child && (pbIsCanFeed ==0))
		{
			win_compopup_init ( WIN_POPUP_TYPE_OK );
			win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_CHILD_FAIL);
			win_compopup_open_ext ( &save );
			return;
		}

		ret = CDCASTB_WriteFeedDataToChild(feeddata, len);
		if (CDCA_RC_OK == ret)
		{
			win_compopup_init ( WIN_POPUP_TYPE_OK );
			win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_OK);
			win_compopup_open_ext ( &save );
			return;
		}
		else
		{
			win_compopup_init ( WIN_POPUP_TYPE_OK );
			win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_FAIL);
			win_compopup_open_ext ( &save );
			return;
		}
		
	}
	else
	{
		win_compopup_init ( WIN_POPUP_TYPE_OK );
		win_compopup_set_msg (NULL, NULL, RS_FEEDCARD_FEED_FAIL);
		win_compopup_open_ext ( &save );
		return;
	}*/
	
}

/*******************************************************************************
*	Window's keymap, proc and  callback
*******************************************************************************/
static VACTION win_feedcard_con_keymap ( POBJECT_HEAD pObj, UINT32 key )
{
	VACTION act;

	switch ( key )
	{
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}
static PRESULT win_feedcard_con_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 )
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	UINT8 tmp;

	UINT8 bID = OSD_GetFocusID ( ( POBJECT_HEAD ) &g_win_feedcard); 
	switch(event)
	{
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1>>16);
			if((unact == VACT_ENTER)) //only response on "feed" item &&(4 == bID)
			{
				//to do
				if(!bfeedOver)
				{
					bfeedOver = 1;
					feedcard_proc(card_is_child);
					win_feedcard_display(1);//update the windows
					bfeedOver = 0;
				}
			}
			break;
		default:
			break;
	}

	return ret;
}

static VACTION win_feedcard_keymap ( POBJECT_HEAD pObj, UINT32 vkey )
{
	VACTION act = VACT_PASS;

	switch ( vkey )
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

struct help_item_resource win_feedcard_help[] =
{
	{0,RS_HELP_ENTER,RS_FEEDCARD_FEED},
	{0,RS_MENU,RS_HELP_BACK},
	{0,RS_HELP_EXIT,RS_FULL_PLAY},
};
static PRESULT win_feedcard_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 )
{
	PRESULT ret = PROC_PASS;
	UINT8 tmp = 0, value;

	switch ( event )
	{
		case EVN_PRE_OPEN:
			tmp = win_feedcard_display(0);//update by osd lib
			bfeedOver = 0;
			if (0 == tmp)
			{
				ret= PROC_LEAVE;
				break;
			}
			/*@@@@CAS UI need modify*/
			//wincom_open_title(RS_CHILD_PARENT_CARD, 0);
			wincom_open_title_ext(RS_PARENT_CARD, IM_TITLE_ICON_SYSTEM);
			wincom_open_help(win_feedcard_help, 3);
			break;
		case EVN_POST_OPEN:
			break;
		case EVN_PRE_DRAW:
			break;
		case EVN_PRE_CLOSE:
				*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;	
			break;
		case EVN_POST_CLOSE:
	
			break;
	}
	return ret;
}



