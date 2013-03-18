#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include "win_cas_com.h"
#include "win_ca_stbid_list.h"
#if 0
/*******************************************************************************
*	Objects definition
*******************************************************************************/

extern CONTAINER g_win_STBIDlst;
extern CONTAINER win_STBIDlist_con;

extern OBJLIST	 STBIDlist_olist;

extern CONTAINER STBIDlist_item0;
extern CONTAINER STBIDlist_item1;
extern CONTAINER STBIDlist_item2;
extern CONTAINER STBIDlist_item3;
extern CONTAINER STBIDlist_item4;

extern TEXT_FIELD STBIDlist_idx0;
extern TEXT_FIELD STBIDlist_idx1;
extern TEXT_FIELD STBIDlist_idx2;
extern TEXT_FIELD STBIDlist_idx3;
extern TEXT_FIELD STBIDlist_idx4;

extern TEXT_FIELD STBIDlist_name0;
extern TEXT_FIELD STBIDlist_name1;
extern TEXT_FIELD STBIDlist_name2;
extern TEXT_FIELD STBIDlist_name3;
extern TEXT_FIELD STBIDlist_name4;

extern TEXT_FIELD STBIDlist_index;
extern TEXT_FIELD STBIDlist_stbids;
extern TEXT_FIELD STBIDlist_info;
extern TEXT_FIELD STBIDlist_info2;

static VACTION win_STBIDlist_keymap ( POBJECT_HEAD pObj, UINT32 key );
static PRESULT win_STBIDlist_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 );
static PRESULT win_STBIDlist_list_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 );


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
#define W_CON_H     	264

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
#define ITEM_IDX_W  280
#define ITEM_NAME_L  (ITEM_IDX_L + ITEM_IDX_W)
#define ITEM_NAME_W  280//180

#define LDEF_CON(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    NULL,NULL,  \
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

#define LDEF_LIST_ITEM(root,varCon,varTxtidx,varTxtName,ID,l,t,w,h,idxstr,namestr)	\
	LDEF_CON(&root,varCon,NULL,ID,ID,ID,ID,ID,l,t,w,h,&varTxtidx,1)	\
	LDEF_TXTIDX(&varCon,varTxtidx,&varTxtName ,0,0,0,0,0,l + ITEM_IDX_L, t,ITEM_IDX_W,h,0,idxstr)	\
	LDEF_TXTNAME(&varCon,varTxtName,NULL,1,1,1,1,1,l + ITEM_NAME_L, t,ITEM_NAME_W,h,0,namestr)

#define LDEF_OL(root,varOl,nxtObj,l,t,w,h,style,dep,count,flds,sb,mark,selary)	\
  DEF_OBJECTLIST(varOl,&root,nxtObj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h,LST_SH_IDX,LST_HL_IDX,LST_SL_IDX,LST_GRY_IDX,   \
    NULL,win_STBIDlist_list_callback,    \
    flds,sb,mark,style,dep,count,selary)

#define LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_SH_IDX,WIN_HL_IDX,WIN_SL_IDX,WIN_GRY_IDX,   \
    win_STBIDlist_keymap,win_STBIDlist_callback,  \
    nxtObj, focusID,0)

LDEF_TXTIDX(&g_win_STBIDlst, STBIDlist_index,&STBIDlist_stbids,0,0,0,0,0,\
    ITEM_L+ITEM_IDX_L, ITEM_T,ITEM_IDX_W,ITEM_H,RS_DETITLE_INDEX,NULL)

LDEF_TXTIDX(&g_win_STBIDlst, STBIDlist_stbids,&STBIDlist_olist,0,0,0,0,0,\
    ITEM_L+ITEM_NAME_L, ITEM_T,ITEM_NAME_W,ITEM_H,RS_CASINFO_STBID,NULL)

LDEF_TXTIDX(&g_win_STBIDlst, STBIDlist_info,NULL,0,0,0,0,0,\
    100, 114,400,28,RS_CA_STBID_PAIRED,NULL)
    
LDEF_TXTIDX(&g_win_STBIDlst, STBIDlist_info2,NULL,0,0,0,0,0,\
    100, 144,400,28,0,display_strs[10])

LDEF_LIST_ITEM ( STBIDlist_olist, STBIDlist_item0, STBIDlist_idx0, STBIDlist_name0,  1, \
                 ITEM_L, ITEM_T + ( ITEM_H + ITEM_GAP ) *1, ITEM_W, ITEM_H, display_strs[0], display_strs[5] )

LDEF_LIST_ITEM ( STBIDlist_olist, STBIDlist_item1, STBIDlist_idx1, STBIDlist_name1, 2, \
                 ITEM_L, ITEM_T + ( ITEM_H + ITEM_GAP ) *2, ITEM_W, ITEM_H, display_strs[1], display_strs[6] )

LDEF_LIST_ITEM ( STBIDlist_olist, STBIDlist_item2, STBIDlist_idx2, STBIDlist_name2, 3, \
                 ITEM_L, ITEM_T + ( ITEM_H + ITEM_GAP ) *3, ITEM_W, ITEM_H, display_strs[2], display_strs[7] )

LDEF_LIST_ITEM ( STBIDlist_olist, STBIDlist_item3, STBIDlist_idx3, STBIDlist_name3, 4, \
                 ITEM_L, ITEM_T + ( ITEM_H + ITEM_GAP ) *4, ITEM_W, ITEM_H, display_strs[3], display_strs[8] )

LDEF_LIST_ITEM ( STBIDlist_olist, STBIDlist_item4, STBIDlist_idx4, STBIDlist_name4, 5, \
                 ITEM_L, ITEM_T + ( ITEM_H + ITEM_GAP ) *5, ITEM_W, ITEM_H, display_strs[4], display_strs[9] )

POBJECT_HEAD STBIDlist_items[] =
    {
        ( POBJECT_HEAD ) &STBIDlist_item0,
        ( POBJECT_HEAD ) &STBIDlist_item1,
        ( POBJECT_HEAD ) &STBIDlist_item2,
        ( POBJECT_HEAD ) &STBIDlist_item3,
        ( POBJECT_HEAD ) &STBIDlist_item4,
    };

#define LIST_STYLE (LIST_VER | LIST_NO_SLECT     | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)

LDEF_OL ( g_win_STBIDlst, STBIDlist_olist, NULL, LST_L, LST_T, LST_W, LST_H, LIST_STYLE, 5, 5, STBIDlist_items, NULL, NULL, NULL )

DEF_CONTAINER (win_STBIDlist_con, NULL, &STBIDlist_index, C_ATTR_ACTIVE, 0, \
                0, 0, 0, 0, 0, W_CON_L, W_CON_T, W_CON_W, W_CON_H, CON_IDX, CON_IDX, CON_IDX, CON_IDX,   \
                NULL, NULL,  \
                NULL, 1, 0 )
                
LDEF_WIN ( g_win_STBIDlst, &win_STBIDlist_con, W_L, W_T, W_W, W_H, 0 )
#endif

/*******************************************************************************
*	Local vriable & function declare
*******************************************************************************/
#define SKYTFCA_MAXBCMATCH_NUM 	5
static void  win_STBIDlist_set_display ( void );
static UINT8 ParNum = SKYTFCA_MAXBCMATCH_NUM;
static UINT8 ParStbNoList[SKYTFCA_MAXBCMATCH_NUM*6]; //max 5 ID, every id 6 bytes

/*******************************************************************************
*	key mapping and event callback definition
*******************************************************************************/
static PRESULT win_STBIDlist_list_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 )
{
	PRESULT ret = PROC_PASS;

	switch(event)
	{
		case EVN_PRE_DRAW:
			win_STBIDlist_set_display();
			break;
		default:
			break;
	}

	return ret;
}

static VACTION win_STBIDlist_keymap ( POBJECT_HEAD pObj, UINT32 key )
{
	VACTION act = VACT_PASS;

	switch ( key )
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
static UINT16 IsPaired_ret;
static PRESULT win_STBIDlist_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 )
{
	PRESULT ret = PROC_PASS;
	UINT8 str[4],strlen,*src;
	UINT16 unistr[14],unistr2[4];
	struct help_item_resource win_sl_help[] =
	{
    		{0,RS_MENU,RS_HELP_BACK},
    		{0,RS_HELP_EXIT,RS_FULL_PLAY},
	};
	
	switch ( event )
	{
		case EVN_PRE_OPEN:
#if 1
			MEMSET(ParStbNoList, 0, sizeof(ParStbNoList));
			IsPaired_ret = CDCASTB_IsPaired(&ParNum,  (UINT8 *)ParStbNoList);
			if ((CDCA_RC_OK != IsPaired_ret)&&(CDCA_RC_CARD_PAIROTHER != IsPaired_ret))
			{
				//--{{{此处需要增加错误提示信息
				//--}}}
				ret = PROC_LEAVE;
				break;
			}
#else //debug only
			ParNum = 5;
			UINT8 i, j;
			for (i=0; i<5; i++)
			{
				for (j=0; j<6;j++)
					ParStbNoList[i*6+j] = i*6+j+2;
			}
#endif
			wincom_open_title_ext(RS_PAIRING_SETTING, IM_TITLE_ICON_SYSTEM);
			wincom_open_help(win_sl_help, 2);
			win_init_pstring(10);
			if(CDCA_RC_CARD_PAIROTHER == IsPaired_ret)
			{
				OSD_SetTextFieldContent((PTEXT_FIELD)&STBIDlist_title_index, STRING_ID, RS_DETITLE_INDEX);
				OSD_SetTextFieldContent((PTEXT_FIELD)&STBIDlist_title_stbids, STRING_ID, RS_CASINFO_STBID);

				STBIDlist_info.bAlign = STBIDlist_info2.bAlign = C_ALIGN_LEFT | C_ALIGN_VCENTER;
				src = OSD_GetUnicodeString(RS_CA_STBID_NUMBER);
				ComUniStrCopyChar((UINT8*) unistr, src);
				strlen = ComUniStrLen(unistr);
				sprintf(str,"%3d",ParNum);
				ComAscStr2Uni(str, unistr2);               
				ComUniStrCopyChar((UINT8*)&unistr[strlen], (UINT8*)unistr2);
				OSD_SetTextFieldContent((PTEXT_FIELD)&STBIDlist_info2, STRING_UNICODE, (UINT32)unistr);
			}
			else
			{
				OSD_SetTextFieldContent((PTEXT_FIELD)&STBIDlist_title_index, STRING_ID, RS_DETITLE_INDEX);
				OSD_SetTextFieldContent((PTEXT_FIELD)&STBIDlist_title_stbids, STRING_ID, RS_CASINFO_STBID);
				OSD_SetTextFieldContent((PTEXT_FIELD)&STBIDlist_info, STRING_ID, 0);
				sprintf(str," ");
				OSD_SetTextFieldContent((PTEXT_FIELD)&STBIDlist_info2, STRING_ANSI, (UINT32)str);
			}
			OSD_SetObjListCount(&STBIDlist_olist, ParNum);
			break;
		case EVN_POST_OPEN:
	            OSD_DrawObject((POBJECT_HEAD)&STBIDlist_info, C_UPDATE_ALL);
	            OSD_DrawObject((POBJECT_HEAD)&STBIDlist_info2, C_UPDATE_ALL);           
			break;
		case EVN_PRE_CLOSE:
			/* Make OSD not flickering */
			* ( ( UINT32* ) param2 ) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			break;
	}

	return ret;
}


static void  win_STBIDlist_set_display ( void )
{
	CONTAINER* con;
	TEXT_FIELD* text;
	OBJLIST* ol;
	UINT8 temp[50] = {0,};
	UINT16 i,j, top, cnt, page, index, valid_idx;
	SYSTEM_DATA* sys_data = sys_data_get();

	ol = &STBIDlist_olist;

	cnt = OSD_GetObjListCount ( ol );
	page = OSD_GetObjListPage ( ol );
	top = OSD_GetObjListTop ( ol );


	for ( i = 0;i < page;i++ )
	{
		con = ( CONTAINER* ) STBIDlist_olist_ListField[i];
		index = top + i;

		if ( index < cnt )
			valid_idx = 1;
		else
			valid_idx = 0;

		/* IDX */
		text = ( TEXT_FIELD* ) OSD_GetContainerNextObj ( con );
		if ( valid_idx )
			OSD_SetTextFieldContent ( text, STRING_NUMBER, index + 1 );
		else
			OSD_SetTextFieldContent ( text, STRING_ANSI, ( UINT32 ) "" );

		/* Name**/
		text = ( TEXT_FIELD* ) OSD_GetObjpNext ( text );
		if ( valid_idx )
		{
			MEMSET(temp, 0, sizeof(temp));
			sprintf(temp, "%02x%02x%02x%02x%02x%02x", 
					ParStbNoList[i*6 + 0],
					ParStbNoList[i*6 + 1],
					ParStbNoList[i*6 + 2],
					ParStbNoList[i*6 + 3],
					ParStbNoList[i*6 + 4],
					ParStbNoList[i*6 + 5]);
			OSD_SetTextFieldContent ( text, STRING_ANSI, ( UINT32 )temp );
		}
		else
			OSD_SetTextFieldContent ( text, STRING_ANSI, ( UINT32 ) "" );

	}
}


