#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libpub/lib_pub.h>
//#include <api/libosd/osd_lib.h>
#include <hld/decv/vdec_driver.h>
#include <udi/ipanel/ipanel_base.h>

#include "../osdobjs_def.h"
#include "../string.id"
#include "../images.id"
#include "../osd_config.h"
#include "../win_com.h"
#include "../menus_root.h"

#include "../win_com_popup.h"
#include "../win_com_list.h"

/*******************************************************************************
 *	Objects definition
 *******************************************************************************/
CONTAINER g_win_ipanel;

TEXT_FIELD ipanel_servicelist_name;

CONTAINER ipanel_servicelist_con;
OBJLIST ipanel_servicelist_ol;
SCROLL_BAR ipanel_servicelist_scb;

CONTAINER ipanel_servicelist_item0;
CONTAINER ipanel_servicelist_item1;
CONTAINER ipanel_servicelist_item2;
CONTAINER ipanel_servicelist_item3;
CONTAINER ipanel_servicelist_item4;
CONTAINER ipanel_servicelist_item5;
CONTAINER ipanel_servicelist_item6;
CONTAINER ipanel_servicelist_item7;
CONTAINER ipanel_servicelist_item8;

TEXT_FIELD ipanel_servicelist_idx0;
TEXT_FIELD ipanel_servicelist_idx1;
TEXT_FIELD ipanel_servicelist_idx2;
TEXT_FIELD ipanel_servicelist_idx3;
TEXT_FIELD ipanel_servicelist_idx4;
TEXT_FIELD ipanel_servicelist_idx5;
TEXT_FIELD ipanel_servicelist_idx6;
TEXT_FIELD ipanel_servicelist_idx7;
TEXT_FIELD ipanel_servicelist_idx8;

TEXT_FIELD ipanel_servicelist_name0;
TEXT_FIELD ipanel_servicelist_name1;
TEXT_FIELD ipanel_servicelist_name2;
TEXT_FIELD ipanel_servicelist_name3;
TEXT_FIELD ipanel_servicelist_name4;
TEXT_FIELD ipanel_servicelist_name5;
TEXT_FIELD ipanel_servicelist_name6;
TEXT_FIELD ipanel_servicelist_name7;
TEXT_FIELD ipanel_servicelist_name8;

CONTAINER ipanel_scr_size_frame;
CONTAINER ipanel_scr_size_con;
TEXT_FIELD ipanel_scr_size_id;
TEXT_FIELD ipanel_scr_size_value;


static VACTION ipanel_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT ipanel_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION ipanel_list_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT ipanel_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION ipanel_list_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT ipanel_list_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
static PRESULT ipanel_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION ipanel_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static void LoadScrSize(BOOL b);
static UINT16 ipanelLoadServiceList();

extern INT32 ali_ipanel_enter(UINT16 service_id, UINT32 freq, UINT32 sym, 
    UINT8 qam, UINT32 w, UINT32 h);

#define WIN_IDX	  WSTL_COMMON_BACK_2
#define SERVICELIST_CON_IDX WSTL_WIN_1

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_1
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_1

#define LIST_BAR_MID_RECT_IDX  	    WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX  	WSTL_SCROLLBAR_2

#define TXTI_IDX   WSTL_TXT_4
#define TXTI_HL_IDX   WSTL_TXT_3

#define TITLE_SIZE_SH_IDX WSTL_TXT_4

#define CON_SH_IDX   WSTL_BUTTON_2
#define CON_HL_IDX   WSTL_BUTTON_1
#define CON_SL_IDX   WSTL_BUTTON_1
#define CON_GRY_IDX  WSTL_BUTTON_2

#define LST_IDX		WSTL_NOSHOW_IDX

#define PREVIEW_IDX WSTL_SUBMENU_PREVIEW

#define W_L     	0
#define W_T     	90
#define W_W		    720
#define W_H     	400

#define LSTCON_L 66
#define LSTCON_T 102
#define LSTCON_W 604//314
#define LSTCON_H 324//224
/*
#define SIZE_FRAME_L 66
#define SIZE_FRAME_T 332
#define SIZE_FRAME_W LSTCON_W//604
#define SIZE_FRAME_H 110

#define SIZE_CON_L (SIZE_FRAME_L+5)
#define SIZE_CON_T (SIZE_FRAME_T+40)
#define SIZE_CON_W (LSTCON_W-5*2)
#define SIZE_CON_H 30

#define SIZE_IDX_L SIZE_CON_L
#define SIZE_IDX_T SIZE_CON_T
#define SIZE_IDX_W 200
#define SIZE_IDX_H SIZE_CON_H

#define SIZE_VALUE_L (SIZE_IDX_L+SIZE_IDX_W)
#define SIZE_VALUE_T SIZE_IDX_T
#define SIZE_VALUE_W (SIZE_CON_W-SIZE_IDX_W)
#define SIZE_VALUE_H SIZE_CON_H
*/
#define SERVICELIST_NAME_L (LSTCON_L+10)
#define SERVICELIST_NAME_T (LSTCON_T+4)
#define SERVICELIST_NAME_W (LSTCON_W-10*2)
#define SERVICELIST_NAME_H 24

#define LST_L (LSTCON_L+4)
#define LST_T (LSTCON_T+34)
#define LST_W (LSTCON_W-4*2)
#define LST_H (ITEM_H*9)

#define ITEM_L	    LST_L
#define ITEM_T	LST_T

#define ITEM_W	(LST_W-SCB_W)

#define ITEM_H	        30
#define ITEM_GAP	0
#define ITEM_SIZE_GAP	0

#define SCB_L (ITEM_L + ITEM_W)
#define SCB_T LST_T
#define SCB_W 12
#define SCB_H (ITEM_H*9+0*4)

#define ITEM_IDX_L     50
#define ITEM_IDX_W     60
#define ITEM_NAME_L    (ITEM_IDX_L+ITEM_IDX_W)
#define ITEM_NAME_W    140

#define SERVICELIST_ID 1
#define SCR_SIZE_ID 2

#define LDEF_CON_SERVICELIST(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    ipanel_list_item_con_keymap,ipanel_list_item_con_callback,  \
    conobj, 1,1)

#define LDEF_CONT(root, varCon,nxtObj,id,uID,dID,l,t,w,h,conobj,sh,hl,sl,gry)		\
    DEF_CONTAINER(varCon,root,nxtObj,C_ATTR_ACTIVE,0, \
    id,0,0,uID,dID, l,t,w,h, sh,hl,sl,gry,   \
    ipanel_con_keymap,ipanel_con_callback,  \
    conobj, 1,1)

#define LDEF_SERVICELIST_NAME(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, WSTL_VOLUME_CON_BG,WSTL_VOLUME_CON_BG,WSTL_VOLUME_CON_BG,WSTL_VOLUME_CON_BG,   \
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 10,0,resID,str)

#define LDEF_TXT1(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str,xAlign,hl)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXTI_IDX,hl,TXTI_HL_IDX,TXTI_IDX,   \
    NULL,NULL,  \
    xAlign  | C_ALIGN_VCENTER, 10,0,resID,str)

#define LDEF_TXT2(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXTI_IDX,TXTI_HL_IDX,TXTI_HL_IDX,TXTI_IDX,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT | C_ALIGN_VCENTER, 10,0,resID,str)

#define LDEF_LIST_ITEM(root,varCon,varTxt1,varTxt2,ID,l,t,w,h,str1,str2)	\
	LDEF_CON_SERVICELIST(&root,varCon,NULL,ID,ID,ID,ID,ID,l,t,w,h,&varTxt1,1)	\
	LDEF_TXT2(&varCon,varTxt1,&varTxt2 ,0,0,0,0,0,l + ITEM_IDX_L, t,ITEM_IDX_W,h,0,str1)	\
	LDEF_TXT1(&varCon,varTxt2,NULL ,1,1,1,1,1,l + ITEM_NAME_L, t,ITEM_NAME_W,h,0,str2,C_ALIGN_CENTER,TXTI_HL_IDX)

#define LDEF_LISTBAR(root,varScb,page,l,t,w,h)	\
	DEF_SCROLLBAR(varScb, &root, NULL, C_ATTR_ACTIVE, 0, \
		0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
		NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
		0, 10, w, h - 20, 100, 1)

#define LDEF_OL(root,varOl,nxtObj,id,l,t,w,h,style,dep,count,flds,sb,mark,selary,callback)	\
  DEF_OBJECTLIST(varOl,&root,nxtObj,C_ATTR_ACTIVE,0, \
    id,id,id,id,id, l,t,w,h,LST_IDX,LST_IDX,LST_IDX,LST_IDX,   \
    ipanel_list_keymap,callback,    \
    flds,sb,mark,style,dep,count,selary)

#define LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_IDX,WIN_IDX,WIN_IDX,WIN_IDX,   \
    ipanel_keymap,ipanel_callback,  \
    nxtObj, focusID,0)


LDEF_LIST_ITEM ( ipanel_servicelist_ol, ipanel_servicelist_item0, ipanel_servicelist_idx0, ipanel_servicelist_name0,1, \
                 ITEM_L, ITEM_T + ( ITEM_H + ITEM_GAP ) *0, ITEM_W, ITEM_H,\
                 display_strs[0], display_strs[1])

LDEF_LIST_ITEM ( ipanel_servicelist_ol, ipanel_servicelist_item1, ipanel_servicelist_idx1, ipanel_servicelist_name1,2, \
                 ITEM_L, ITEM_T + ( ITEM_H + ITEM_GAP ) *1, ITEM_W, ITEM_H,\
                 display_strs[2], display_strs[3])

LDEF_LIST_ITEM ( ipanel_servicelist_ol, ipanel_servicelist_item2, ipanel_servicelist_idx2, ipanel_servicelist_name2,3, \
                 ITEM_L, ITEM_T + ( ITEM_H + ITEM_GAP ) *2, ITEM_W, ITEM_H,\
                 display_strs[4], display_strs[5])

LDEF_LIST_ITEM ( ipanel_servicelist_ol, ipanel_servicelist_item3, ipanel_servicelist_idx3, ipanel_servicelist_name3,4, \
                 ITEM_L, ITEM_T + ( ITEM_H + ITEM_GAP ) *3, ITEM_W, ITEM_H,\
                 display_strs[6], display_strs[7])

LDEF_LIST_ITEM ( ipanel_servicelist_ol, ipanel_servicelist_item4, ipanel_servicelist_idx4, ipanel_servicelist_name4,5, \
                 ITEM_L, ITEM_T + ( ITEM_H + ITEM_GAP ) *4, ITEM_W, ITEM_H,\
                 display_strs[8], display_strs[9])

LDEF_LIST_ITEM ( ipanel_servicelist_ol, ipanel_servicelist_item5, ipanel_servicelist_idx5, ipanel_servicelist_name5,6, \
                 ITEM_L, ITEM_T + ( ITEM_H + ITEM_GAP ) *5, ITEM_W, ITEM_H,\
                 display_strs[10], display_strs[11])

LDEF_LIST_ITEM ( ipanel_servicelist_ol, ipanel_servicelist_item6, ipanel_servicelist_idx6, ipanel_servicelist_name6,7, \
                 ITEM_L, ITEM_T + ( ITEM_H + ITEM_GAP ) *6, ITEM_W, ITEM_H,\
                 display_strs[12], display_strs[13])

LDEF_LIST_ITEM ( ipanel_servicelist_ol, ipanel_servicelist_item7, ipanel_servicelist_idx7, ipanel_servicelist_name7,8, \
                 ITEM_L, ITEM_T + ( ITEM_H + ITEM_GAP ) *7, ITEM_W, ITEM_H,\
                 display_strs[14], display_strs[15])

LDEF_LIST_ITEM ( ipanel_servicelist_ol, ipanel_servicelist_item8, ipanel_servicelist_idx8, ipanel_servicelist_name8,9, \
                 ITEM_L, ITEM_T + ( ITEM_H + ITEM_GAP ) *8, ITEM_W, ITEM_H,\
                 display_strs[16], display_strs[17])

LDEF_LISTBAR ( ipanel_servicelist_ol, ipanel_servicelist_scb, 9, SCB_L, SCB_T, SCB_W, SCB_H )

POBJECT_HEAD ipanel_servicelist_items[] =
{
    ( POBJECT_HEAD ) &ipanel_servicelist_item0,
    ( POBJECT_HEAD ) &ipanel_servicelist_item1,
    ( POBJECT_HEAD ) &ipanel_servicelist_item2,
    ( POBJECT_HEAD ) &ipanel_servicelist_item3,
    ( POBJECT_HEAD ) &ipanel_servicelist_item4,
    ( POBJECT_HEAD ) &ipanel_servicelist_item5,
    ( POBJECT_HEAD ) &ipanel_servicelist_item6,
    ( POBJECT_HEAD ) &ipanel_servicelist_item7,
    ( POBJECT_HEAD ) &ipanel_servicelist_item8,
};

#define LIST_STYLE (LIST_VER | LIST_NO_SLECT     | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)

LDEF_SERVICELIST_NAME(NULL,ipanel_servicelist_name,NULL ,1,1,1,1,1,\
        SERVICELIST_NAME_L,SERVICELIST_NAME_T,SERVICELIST_NAME_W,SERVICELIST_NAME_H,RS_SERVICE_LIST,NULL)

/*LDEF_TXT1(&ipanel_scr_size_con,ipanel_scr_size_value,NULL ,1,1,1,1,1,\
        SIZE_VALUE_L,SIZE_VALUE_T,SIZE_VALUE_W,SIZE_VALUE_H,0,display_strs[18],C_ALIGN_CENTER,WSTL_BUTTON_5)

LDEF_TXT1(&ipanel_scr_size_con,ipanel_scr_size_id,&ipanel_scr_size_value,1,1,1,1,1,\
        SIZE_IDX_L, SIZE_IDX_T,SIZE_IDX_W,SIZE_IDX_H,RS_SCR_RESOLUTION,NULL,C_ALIGN_CENTER,TXTI_HL_IDX)

LDEF_CONT(&g_win_ipanel,ipanel_scr_size_con,NULL,SCR_SIZE_ID,SERVICELIST_ID,SERVICELIST_ID,
    SIZE_CON_L,SIZE_CON_T,SIZE_CON_W,SIZE_CON_H,&ipanel_scr_size_id,CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX)

/*LDEF_CONT(&g_win_ipanel,ipanel_scr_size_frame,&ipanel_scr_size_con,0,0,0,SIZE_FRAME_L,SIZE_FRAME_T,SIZE_FRAME_W,SIZE_FRAME_H,NULL,
    WSTL_WIN_1,WSTL_WIN_1,WSTL_WIN_1,WSTL_WIN_1)
*/
LDEF_OL ( g_win_ipanel, ipanel_servicelist_ol,NULL, 1,LST_L, LST_T, LST_W, LST_H, LIST_STYLE, 9, 9, \
        ipanel_servicelist_items, &ipanel_servicelist_scb, NULL, NULL,ipanel_list_callback)

LDEF_CONT(&g_win_ipanel,ipanel_servicelist_con,NULL/*&ipanel_scr_size_frame*/,SERVICELIST_ID,SCR_SIZE_ID,SCR_SIZE_ID,
    LSTCON_L,LSTCON_T,LSTCON_W,LSTCON_H,&ipanel_servicelist_ol,WSTL_WIN_1,WSTL_WIN_1,WSTL_WIN_1,WSTL_WIN_1)

LDEF_WIN ( g_win_ipanel, &ipanel_servicelist_con, W_L, W_T, W_W, W_H, 1 )

#define VACT_CHANGE_SCR_SIZE    (VACT_PASS+1)
#define VACT_CHANGE_FOCUS       (VACT_PASS+2)
#define VACT_ENTER_IPANEL     (VACT_PASS+3)

static VACTION ipanel_list_item_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

    switch(key)
    {
        case V_KEY_ENTER:
            act = VACT_ENTER_IPANEL;
            break;
        default:
            break;
    }

	return act;
}

static PRESULT ipanel_list_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
    VACTION vact;
    UINT16 ID;
    P_NODE pNode;
    T_NODE tNode;
    SYSTEM_DATA *sys = sys_data_get();
    UINT32 w = 0,h = 0;
    UINT8 av_flag=0,back_saved=0;
    struct browser_entry entry;		

    switch(event)
    {
        case EVN_UNKNOWN_ACTION:
            vact = (VACTION)(param1 >> 16);
            if(vact == VACT_ENTER_IPANEL)
            {
                ID = OSD_GetObjID(pObj);
                get_prog_at(ID-1,&pNode);
                get_tp_by_id(pNode.tp_id,&tNode);

		entry.service_id =pNode.prog_number;
		entry.ts_id = tNode.t_s_id;
		entry.freq = tNode.frq;
		ali_ipanel_enter_mw(1, (UINT32)&entry);
		
		if (sys_data_get_sate_group_num(av_flag) > 0)
			key_pan_display("----", 4);
		else
			key_pan_display("noCH", 4); 
		OSD_ObjOpen((POBJECT_HEAD) &g_win_ipanel, C_UPDATE_ALL);
		

		ret = PROC_LOOP;
            }
            break;
        default:
            break;
    }

	return ret;
}

static VACTION ipanel_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;


	switch (key)
	{
		case V_KEY_LEFT:
		case V_KEY_RIGHT:
            act = VACT_CHANGE_SCR_SIZE;
			break;
		default:
			act = VACT_PASS;
	}
    
	return act;
}

static PRESULT ipanel_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
    VACTION unact;
    UINT8 id;
    SYSTEM_DATA *sys = sys_data_get();

    id = pObj->bID;

    switch(event)
    {
        case EVN_UNKNOWN_ACTION:
           /* if(id == SCR_SIZE_ID)
            {
                unact = (VACTION)(param1 >> 16);
                if(unact == VACT_CHANGE_SCR_SIZE)
                {
                    if(sys->ipanel_scr_size == 1)
                        sys->ipanel_scr_size = 0;
                    else
                        sys->ipanel_scr_size = 1;
                    
                    LoadScrSize(TRUE);
                }
            }*/
            break;
    }

	return ret;
}

static VACTION ipanel_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	INT8 shift = 1;

	switch (key)
	{
		case V_KEY_EXIT:
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		case V_KEY_RED:
            act = VACT_CHANGE_FOCUS;
			break;
		default:
			act = VACT_PASS;
	}

	return act;

}

struct help_item_resource ipanel_help[] =
{
    ////{1,IM_EPG_COLORBUTTON_RED,RS_SWITCH_WINDOW},
    {0,RS_MENU,RS_HELP_EXIT},
    {0,RS_HELP_EXIT,RS_HELP_EXIT},
};

static PRESULT ipanel_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	UINT8 back_saved;
    UINT8 i;
	static UINT16 curChannel;
	static UINT8 pre_chan_mode;
	static UINT8 groupIdx=0;    

	switch (event)
	{
		case EVN_PRE_OPEN:
            curChannel = sys_data_get_cur_group_cur_mode_channel();
			pre_chan_mode = sys_data_get_cur_chan_mode();
			groupIdx = sys_data_get_cur_group_index();
  			api_stop_play(TRUE);
            osal_task_sleep(400);
            if(0==ipanelLoadServiceList())
            {
				win_compopup_init(WIN_POPUP_TYPE_SMSG);
				win_compopup_set_msg(NULL, NULL, RS_DATA_BROADCAST_NO_INFO);
				win_compopup_open_ext(&back_saved);
				osal_task_sleep(500);
				win_compopup_smsg_restoreback();
    			/*return to the previous db view, group, channel,*/
				sys_data_set_cur_chan_mode(pre_chan_mode);
    			sys_data_change_group ( groupIdx );
    			sys_data_set_cur_group_channel(curChannel);
    			api_play_channel(curChannel, TRUE, FALSE, FALSE);
                ret = PROC_LEAVE;
            }
            else
            {
    			wincom_open_title_ext(RS_DATA_BROADCAST, IM_TITLE_ICON_SEARCH);
    			wincom_open_help(ipanel_help, 2);

            	for (i = 0; i <= 18; i++)
            	{
            		MEMSET(display_strs[i], 0, sizeof(display_strs[i]));
            	}            
                
                //LoadScrSize(FALSE);
               // g_win_ipanel.FocusObjectID = 1; 
            }
			break;
		case EVN_POST_OPEN:
            OSD_DrawObject((POBJECT_HEAD)&ipanel_servicelist_name,C_UPDATE_ALL);
			break;
		case EVN_UNKNOWNKEY_GOT:
			break;
		case EVN_UNKNOWN_ACTION:
			
			break;
		case EVN_MSG_GOT:
            sys_data_save(1);
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			/*return to the previous db view, group, channel,*/
			sys_data_set_cur_chan_mode(pre_chan_mode);
			sys_data_change_group ( groupIdx );
			sys_data_set_cur_group_channel(curChannel);
            
			api_play_channel(curChannel, TRUE, FALSE, FALSE);
            
			break;
		case EVN_POST_CLOSE:
			break;
	}

	return ret;
}

static void ipanel_servicelist_set_display(void)
{
	OBJLIST *ol;
	CONTAINER *item;
	TEXT_FIELD *txt;
	UINT32 i;
	UINT32 valid_idx;
	UINT16 top, cnt, page, index, curitem;
	P_NODE pNode;
	UINT16 unistr[30];
	char str[10];

	ol = &ipanel_servicelist_ol;

	cnt = OSD_GetObjListCount(ol);
	page = OSD_GetObjListPage(ol);
	top = OSD_GetObjListTop(ol);
	curitem = OSD_GetObjListNewPoint(ol);

	for (i = 0; i < page; i++)
	{
		item = (CONTAINER*)ipanel_servicelist_items[i];
		index = top + i;

		valid_idx = (index < cnt) ? 1 : 0;
		if (valid_idx)
		{
            get_prog_at(index,&pNode);
		}
		else
		{
			STRCPY(str, "");
			unistr[0] = 0;
		}


		/* IDX */
		txt = (TEXT_FIELD*)OSD_GetContainerNextObj(item);
		if (valid_idx)
			sprintf(str, "%03d", index + 1);
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

		/* Name */
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);
		if (valid_idx)
		{
			ComUniStrCopyChar((UINT8*)unistr, pNode.service_name);
			OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)unistr);            
		}
		else
			OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

	}
}

static VACTION ipanel_list_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
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
			break;
	}

	return act;
}

static PRESULT ipanel_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	OBJLIST *ol;
	UINT16 curitem;
	P_NODE p_node;
	SYSTEM_DATA *sys_data = sys_data_get();

	ol = &ipanel_servicelist_ol;
	curitem = OSD_GetObjListNewPoint(ol);

	switch (event)
	{
		case EVN_PRE_DRAW:
			ipanel_servicelist_set_display();
			break;
		case EVN_ITEM_POST_CHANGE:
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			break;
	}
	return ret;
}

static void LoadScrSize(BOOL b)
{
    /*SYSTEM_DATA *sys;

    sys = sys_data_get();

   /* if(sys->ipanel_scr_size == 0)
    {
        ComAscStr2Uni("720 * 576",display_strs[12]);
    }
    else if(sys->ipanel_scr_size == 1)
    {
        ComAscStr2Uni("640 * 526",display_strs[12]);
   // }

    if(b)
    {
        OSD_TrackObject((POBJECT_HEAD)&ipanel_scr_size_value,C_UPDATE_ALL);
    }*/
}

static UINT16 ipanelLoadServiceList()
{
	UINT16 serviceCnt = 0,page;
	OBJLIST *ol;
	SCROLL_BAR *scb;

	ol = &ipanel_servicelist_ol;
	scb = &ipanel_servicelist_scb;

    recreate_prog_view ( VIEW_ALL | PROG_DATA_MODE, 0 );
	serviceCnt = get_prog_num(VIEW_ALL | PROG_DATA_MODE, 0);
	page = OSD_GetObjListPage(ol);

	OSD_SetObjListCount(ol, serviceCnt);
	OSD_SetObjListCurPoint(ol, 0);
	OSD_SetObjListNewPoint(ol, 0);
	OSD_SetObjListTop(ol, 0);

	return serviceCnt;
}


