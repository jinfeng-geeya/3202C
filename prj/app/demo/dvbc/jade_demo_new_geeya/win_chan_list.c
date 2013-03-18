#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_hde.h>
#include <hld/dis/vpo.h>
//#include <api/libge/osd_lib.h>
#include <hld/decv/vdec_driver.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "win_com.h"
#include "menus_root.h"
#include "win_com_popup.h"
#include "win_com_list.h"

#include "copper_common/com_epg.h"
#include "win_epg.h"
#include "win_signalstatus.h"

//include the header from xform 
#include "win_chan_list_vega.h"


/*******************************************************************************
 *	Objects definition
 *******************************************************************************/
CONTAINER g_win_chanlist;

CONTAINER chanlist_prg_con;

CONTAINER chanlist_grp_con;
TEXT_FIELD chanlist_grp_name;

OBJLIST chanlist_prg_ol;
SCROLL_BAR chanlist_prg_scb;

CONTAINER chanlist_prg_item0;
CONTAINER chanlist_prg_item1;
CONTAINER chanlist_prg_item2;
CONTAINER chanlist_prg_item3;
CONTAINER chanlist_prg_item4;
CONTAINER chanlist_prg_item5;
#ifndef JINGJIN
	CONTAINER chanlist_prg_item6;
	CONTAINER chanlist_prg_item7;
	CONTAINER chanlist_prg_item8;
	CONTAINER chanlist_prg_item9;
#endif
TEXT_FIELD chanlist_prg_idx0;
TEXT_FIELD chanlist_prg_idx1;
TEXT_FIELD chanlist_prg_idx2;
TEXT_FIELD chanlist_prg_idx3;
TEXT_FIELD chanlist_prg_idx4;
TEXT_FIELD chanlist_prg_idx5;
#ifndef JINGJIN
	TEXT_FIELD chanlist_prg_idx6;
	TEXT_FIELD chanlist_prg_idx7;
	TEXT_FIELD chanlist_prg_idx8;
	TEXT_FIELD chanlist_prg_idx9;
#endif
TEXT_FIELD chanlist_prg_name0;
TEXT_FIELD chanlist_prg_name1;
TEXT_FIELD chanlist_prg_name2;
TEXT_FIELD chanlist_prg_name3;
TEXT_FIELD chanlist_prg_name4;
TEXT_FIELD chanlist_prg_name5;
#ifndef JINGJIN
	TEXT_FIELD chanlist_prg_name6;
	TEXT_FIELD chanlist_prg_name7;
	TEXT_FIELD chanlist_prg_name8;
	TEXT_FIELD chanlist_prg_name9;
#endif
CONTAINER chanlist_sch_con;
TEXT_FIELD chanlist_sch_present;
TEXT_FIELD chanlist_sch_following;
#ifndef JINGJIN
	CONTAINER chanlist_msg_con;
	TEXT_FIELD chanlist_program_num;
	TEXT_FIELD chanlist_program_name;
	TEXT_FIELD chanlist_date;
	BITMAP calendar;
#endif


/*static VACTION chanlist_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT chanlist_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

static VACTION chanlist_list_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT chanlist_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);*/
static void chanlist_prg_set_display(void);
static PRESULT chanlist_unact_proc(VACTION act);
static PRESULT chanlist_message_proc(UINT32 msg_type, UINT32 msg_code);
static UINT16 chanlist_prg_list_load(void);
static UINT32 chanlist_draw_epg_pf(void);
static void chanlist_set_cur_service(void);
static void chanlist_draw_msg_con(void);
static void chanlist_msg_con_update(POBJECT_HEAD pObj);
/*static VACTION chanlist_list_item_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT chanlist_list_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);*/

/*******************************************************************************
 *	Style & rectangle parameters definition
 *******************************************************************************/
/*#define WIN_IDX	  WSTL_SEARCH_BG_CON//WSTL_COMMON_BACK_2

#define PRG_CON_IDX WSTL_WIN_1
#define GRP_CON_IDX WSTL_GRP_TITLE_ARROW

#define LIST_BAR_SH_IDX  WSTL_SCROLLBAR_1
#define LIST_BAR_HL_IDX  WSTL_SCROLLBAR_1

#define LIST_BAR_MID_RECT_IDX  	    WSTL_NOSHOW_IDX
#define LIST_BAR_MID_THUMB_IDX  	WSTL_SCROLLBAR_2

#define TXTI_IDX   WSTL_TXT_4
#define TXTI_HL_IDX   WSTL_TXT_3

#define TITLE_SCH_SH_IDX WSTL_TXT_4

#define CON_SH_IDX   WSTL_BUTTON_2
#define CON_HL_IDX   WSTL_BUTTON_1
#define CON_SL_IDX   WSTL_BUTTON_1
#define CON_GRY_IDX  WSTL_BUTTON_2

#define LST_IDX		WSTL_NOSHOW_IDX*/

#define PREVIEW_IDX WSTL_SUBMENU_PREVIEW

/*#define W_L		0
#define W_T		90
#define W_W		720
#define W_H		400

#define LSTCON_PRG_L 66
#define LSTCON_PRG_T 102
#define LSTCON_PRG_W 314
#define LSTCON_PRG_H 224

#define LSTCON_SCH_L 66
#define LSTCON_SCH_T 332
#define LSTCON_SCH_W 604
#define LSTCON_SCH_H 110

#define SCH_PRESENT_L (LSTCON_SCH_L+10)
#define SCH_PRESENT_T (LSTCON_SCH_T+39)
#define SCH_PRESENT_W (LSTCON_SCH_W-10*2)
#define SCH_PRESENT_H 24

#define GRP_NAME_CON_L (LSTCON_PRG_L + 8)
#define GRP_NAME_CON_T (LSTCON_PRG_T + 8)
#define GRP_NAME_CON_W (LSTCON_PRG_W - 16)
#define GRP_NAME_CON_H 20

#define GRP_NAME_L (LSTCON_PRG_L + 30)
#define GRP_NAME_T (LSTCON_PRG_T)
#define GRP_NAME_W (LSTCON_PRG_W -60)
#define GRP_NAME_H 32

#define LST_PRG_L (LSTCON_PRG_L+4)
#define LST_PRG_T (LSTCON_PRG_T+34)
#define LST_PRG_W (LSTCON_PRG_W-4*2)
#define LST_PRG_H (ITEM_H*6)

#define ITEM_L	    LST_PRG_L
#define ITEM_PRG_T	LST_PRG_T

#define ITEM_PRG_W	(LST_PRG_W-SCB_PRG_W)

#define ITEM_H	        30
#define ITEM_PRG_GAP	0
#define ITEM_SCH_GAP	0

#define SCB_PRG_L (ITEM_L + ITEM_PRG_W)
#define SCB_PRG_T LST_PRG_T
#define SCB_PRG_W 12
#define SCB_PRG_H (ITEM_H*6+0*4)

#define ITEM_PRG_IDX_L     50
#define ITEM_PRG_IDX_W     60
#define ITEM_PRG_NAME_L    (ITEM_PRG_IDX_L+ITEM_PRG_IDX_W)
#define ITEM_PRG_NAME_W    140*/

#define CHANLIST_PREVIEW_L 384
#define CHANLIST_PREVIEW_T 96
#define CHANLIST_PREVIEW_W 290
#define CHANLIST_PREVIEW_H 234

#define PREVIEW_L 408
#define PREVIEW_T 120
#define PREVIEW_W 240
#define PREVIEW_H 150


/*#define LDEF_WIN(varCon,nxtObj,l,t,w,h,focusID)		\
    DEF_CONTAINER(varCon,NULL,NULL,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WIN_IDX,WIN_IDX,WIN_IDX,WIN_IDX,   \
    chanlist_keymap,chanlist_callback,  \
    nxtObj, focusID,0)

#define LDEF_CON(root, varCon,nxtObj,l,t,w,h,conobj,style)		\
    DEF_CONTAINER(varCon,&root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, style,style,style,style,   \
    NULL,NULL,  \
    conobj, 1,1)

#define LDEF_ITEM_CON(root, varCon,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,conobj,focusID)		\
    DEF_CONTAINER(varCon,&root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, CON_SH_IDX,CON_HL_IDX,CON_SL_IDX,CON_GRY_IDX,   \
    chanlist_list_item_con_keymap,chanlist_list_item_con_callback,  \
    conobj, 1,1)

#define LDEF_TXT1(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str,xAlign)		\
    DEF_TEXTFIELD(varTxt,&root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXTI_IDX,TXTI_HL_IDX,TXTI_HL_IDX,TXTI_IDX,   \
    NULL,NULL,  \
    xAlign  | C_ALIGN_VCENTER, 10,0,resID,str)

#define LDEF_TXT2(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,&root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, TXTI_IDX,TXTI_HL_IDX,TXTI_HL_IDX,TXTI_IDX,   \
    NULL,NULL,  \
    C_ALIGN_RIGHT | C_ALIGN_VCENTER, 10,0,resID,str)

#define LDEF_TXT_GRP_NAME(root,varTxt,nxtObj,l,t,w,h,resID,str)		\
    DEF_TEXTFIELD(varTxt,&root,nxtObj,C_ATTR_ACTIVE,0, \
    0,0,0,0,0, l,t,w,h, WSTL_TXT_GRP_TITLE,WSTL_TXT_GRP_TITLE,WSTL_TXT_GRP_TITLE,WSTL_TXT_GRP_TITLE,\
    NULL,NULL,  \
    C_ALIGN_CENTER| C_ALIGN_VCENTER, 0,0,resID,str)

#define LDEF_LIST_ITEM_PRG(root,varCon,varTxt1,varTxt2,ID,l,t,w,h,str1,str2)	\
	LDEF_ITEM_CON(root,varCon,NULL,ID,ID,ID,ID,ID,l,t,w,h,&varTxt1,1)	\
	LDEF_TXT2(varCon,varTxt1,&varTxt2 ,0,0,0,0,0,l + ITEM_PRG_IDX_L, t,ITEM_PRG_IDX_W,h,0,str1)	\
	LDEF_TXT1(varCon,varTxt2,NULL ,1,1,1,1,1,l + ITEM_PRG_NAME_L, t,ITEM_PRG_NAME_W,h,0,str2,C_ALIGN_CENTER)

#define LDEF_LISTBAR(root,varScb,page,l,t,w,h)	\
	DEF_SCROLLBAR(varScb, &root, NULL, C_ATTR_ACTIVE, 0, \
		0, 0, 0, 0, 0, l, t, w, h, LIST_BAR_SH_IDX, LIST_BAR_HL_IDX, LIST_BAR_SH_IDX, LIST_BAR_SH_IDX, \
		NULL, NULL, BAR_VERT_AUTO | SBAR_STYLE_RECT_STYLE, page, LIST_BAR_MID_THUMB_IDX, LIST_BAR_MID_RECT_IDX, \
		0, 10, w, h - 20, 100, 1)

#define LDEF_OL(root,varOl,nxtObj,id,l,t,w,h,style,dep,count,flds,sb,mark,selary,callback)	\
  DEF_OBJECTLIST(varOl,&root,nxtObj,C_ATTR_ACTIVE,0, \
    id,id,id,id,id, l,t,w,h,LST_IDX,LST_IDX,LST_IDX,LST_IDX,   \
    chanlist_list_keymap,callback,    \
    flds,sb,mark,style,dep,count,selary)

#define LDEF_MTXT(root,varMtxt,nextObj,ID,l,t,w,h,sh,cnt,sb,content)	\
	DEF_MULTITEXT(varMtxt,&root,nextObj,C_ATTR_ACTIVE,0, \
    	ID,ID,ID,ID,ID, l,t,w,h, sh,sh,sh,sh,   \
	    NULL,NULL,  \
    	C_ALIGN_LEFT | C_ALIGN_TOP, cnt,  4,4,w-8,h-8,sb,content)


#define LIST_STYLE (LIST_VER | LIST_NO_SLECT  | LIST_ITEMS_NOCOMPLETE |  LIST_SCROLL | LIST_GRID | LIST_FOCUS_FIRST | LIST_PAGE_KEEP_CURITEM | LIST_FULL_PAGE)*/

POBJECT_HEAD chanlist_prg_items[] =
{
    (POBJECT_HEAD) &chanlist_prg_item0,
    (POBJECT_HEAD) &chanlist_prg_item1,
    (POBJECT_HEAD) &chanlist_prg_item2,
    (POBJECT_HEAD) &chanlist_prg_item3,
    (POBJECT_HEAD) &chanlist_prg_item4,
    (POBJECT_HEAD) &chanlist_prg_item5,
    #ifndef JINGJIN
		(POBJECT_HEAD) &chanlist_prg_item6,
	  	(POBJECT_HEAD) &chanlist_prg_item7,
	   	(POBJECT_HEAD) &chanlist_prg_item8,
	    	(POBJECT_HEAD) &chanlist_prg_item9,
    #endif
};

/*LDEF_WIN(g_win_chanlist, &chanlist_prg_con, W_L, W_T, W_W, W_H, 1)

LDEF_CON(g_win_chanlist, chanlist_prg_con, &chanlist_grp_con,LSTCON_PRG_L,LSTCON_PRG_T,LSTCON_PRG_W,LSTCON_PRG_H,NULL,PRG_CON_IDX)

LDEF_CON(g_win_chanlist, chanlist_grp_con, &chanlist_prg_ol, GRP_NAME_CON_L, GRP_NAME_CON_T, GRP_NAME_CON_W, GRP_NAME_CON_H, &chanlist_grp_name, GRP_CON_IDX)

LDEF_TXT_GRP_NAME(chanlist_grp_con,chanlist_grp_name,NULL, GRP_NAME_L,GRP_NAME_T,GRP_NAME_W,GRP_NAME_H,0,display_strs[24])

LDEF_OL(g_win_chanlist, chanlist_prg_ol,NULL, 1,LST_PRG_L, LST_PRG_T, LST_PRG_W, LST_PRG_H, LIST_STYLE, 6, 6, \
        chanlist_prg_items, &chanlist_prg_scb, NULL, NULL,chanlist_list_callback)

LDEF_LISTBAR(chanlist_prg_ol, chanlist_prg_scb, 6, SCB_PRG_L, SCB_PRG_T, SCB_PRG_W, SCB_PRG_H)

LDEF_LIST_ITEM_PRG (chanlist_prg_ol, chanlist_prg_item0, chanlist_prg_idx0, chanlist_prg_name0,1, \
                 ITEM_L, ITEM_PRG_T + (ITEM_H + ITEM_PRG_GAP) *0, ITEM_PRG_W, ITEM_H,\
                 display_strs[10], display_strs[11])

LDEF_LIST_ITEM_PRG (chanlist_prg_ol, chanlist_prg_item1, chanlist_prg_idx1, chanlist_prg_name1,2, \
                 ITEM_L, ITEM_PRG_T + (ITEM_H + ITEM_PRG_GAP) *1, ITEM_PRG_W, ITEM_H,\
                 display_strs[12], display_strs[13])

LDEF_LIST_ITEM_PRG (chanlist_prg_ol, chanlist_prg_item2, chanlist_prg_idx2, chanlist_prg_name2,3, \
                 ITEM_L, ITEM_PRG_T + (ITEM_H + ITEM_PRG_GAP) *2, ITEM_PRG_W, ITEM_H,\
                 display_strs[14], display_strs[15])

LDEF_LIST_ITEM_PRG (chanlist_prg_ol, chanlist_prg_item3, chanlist_prg_idx3, chanlist_prg_name3,4, \
                 ITEM_L, ITEM_PRG_T + (ITEM_H + ITEM_PRG_GAP) *3, ITEM_PRG_W, ITEM_H,\
                 display_strs[16], display_strs[17])

LDEF_LIST_ITEM_PRG (chanlist_prg_ol, chanlist_prg_item4, chanlist_prg_idx4, chanlist_prg_name4,5, \
                 ITEM_L, ITEM_PRG_T + (ITEM_H + ITEM_PRG_GAP) *4, ITEM_PRG_W, ITEM_H,\
                 display_strs[18], display_strs[19])

LDEF_LIST_ITEM_PRG (chanlist_prg_ol, chanlist_prg_item5, chanlist_prg_idx5, chanlist_prg_name5,6, \
                 ITEM_L, ITEM_PRG_T + (ITEM_H + ITEM_PRG_GAP) *5, ITEM_PRG_W, ITEM_H,\
                 display_strs[20], display_strs[21])

LDEF_CON(g_win_chanlist, chanlist_sch_con, NULL,LSTCON_SCH_L,LSTCON_SCH_T,LSTCON_SCH_W,LSTCON_SCH_H,&chanlist_sch_present,PRG_CON_IDX)

LDEF_TXT1(chanlist_sch_con,chanlist_sch_present,&chanlist_sch_following,1,1,1,1,1,\
        SCH_PRESENT_L, SCH_PRESENT_T,SCH_PRESENT_W,SCH_PRESENT_H,0,display_strs[22],C_ALIGN_LEFT)

LDEF_TXT1(chanlist_sch_con,chanlist_sch_following,NULL ,1,1,1,1,1,\
        SCH_PRESENT_L,(SCH_PRESENT_T+SCH_PRESENT_H+10),SCH_PRESENT_W,SCH_PRESENT_H,0,display_strs[23],C_ALIGN_LEFT)*/


/*******************************************************************************
 *	Local variable & function declare
 *******************************************************************************/
#ifndef JINGJIN
extern UINT16 weekday_id[];
extern UINT8 new_channel_change_flag;
#endif
static date_time present_end_time;

static UINT8 last_valid_grp_idx = 0;	/* Last valid group index */

static UINT8 pre_sch_cnt = 0;

static UINT16 title_str_id = 0;
static UINT16 title_bmp_id = 0;

struct help_item_resource chanlist_help[] =
{
	{1,IM_HELP_ICON_LR,RS_HELP_GROUP},
	{0,RS_MENU,RS_HELP_BACK},
	{0,RS_HELP_EXIT,RS_FULL_PLAY}, 
};

/*******************************************************************************
 *	Key mapping and event callback definition
 *******************************************************************************/
void ChanListSetTitleID(UINT16 strID, UINT16 bmpID)
{
	title_str_id = strID;
	title_bmp_id = bmpID;
}

BOOL mode_changed_flag = FALSE;
void ChanListSetModeChangedFlag(BOOL flag)
{
	mode_changed_flag = flag;
}


#define VACT_GRP_INC		(VACT_PASS+1)
#define VACT_GRP_DEC		(VACT_PASS+2)
#define VACT_TV_RADIO_SW	(VACT_PASS+3)
#define VACT_PLAY_CH		(VACT_PASS+4)

static VACTION chanlist_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_EXIT:
			if (last_valid_grp_idx != sys_data_get_cur_group_index())
				sys_data_change_group(last_valid_grp_idx);
			BackToFullScrPlay();
			break;
		case V_KEY_MENU:
			if (last_valid_grp_idx != sys_data_get_cur_group_index())
				sys_data_change_group(last_valid_grp_idx);
			act = VACT_CLOSE;
			break;
		case V_KEY_LEFT:
			act = VACT_GRP_DEC;
			break;
		case V_KEY_RIGHT:
			act = VACT_GRP_INC;
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}

static PRESULT chanlist_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	UINT8 av_mode, back_saved;
	OSD_RECT rc_con, rc_preview;
	UINT16 ch_cnt = 0;
	UINT8 cur_grp_idx;

	switch (event)
	{
		case EVN_PRE_OPEN:
			av_mode = sys_data_get_cur_chan_mode();
			#ifndef JINGJIN
				new_channel_change_flag = 0;
			#endif
			ch_cnt = get_prog_num(VIEW_ALL | av_mode, 0);
			if (ch_cnt == 0)
			{
				sys_data_change_group(ALL_GROUP_TYPE);
			}
			/*
			ch_cnt = get_prog_num(VIEW_ALL | av_flag, 0);
			if (ch_cnt == 0)
			{
				win_compopup_init(WIN_POPUP_TYPE_SMSG);
				win_compopup_set_msg(NULL, NULL, RS_MSG_NO_CHANNELS);
				win_compopup_open_ext(&back_saved);
				osal_task_sleep(1000);
				win_compopup_smsg_restoreback();
			}*/
			#ifdef JINGJIN
				display_strs_init(10, 15);
			#else
				display_strs_init(10, 25);
			#endif
			MEMSET(len_display_str, 0, sizeof(len_display_str));
			
			cur_grp_idx = sys_data_get_cur_group_index();
			last_valid_grp_idx = cur_grp_idx;
			chanlist_prg_list_load();
			
			#ifdef JINGJIN
			if(!title_str_id || !title_bmp_id)
			{
				if((av_mode == PROG_TV_MODE))
				{
					ChanListSetTitleID(RS_DIGITAL_TV,IM_TITLE_ICON_TV);
				}

				if((av_mode == PROG_RADIO_MODE))
				{
					ChanListSetTitleID(RS_RADIO,IM_TITLE_ICON_RADIO);
				}
			}
			wincom_open_title_ext(title_str_id, title_bmp_id);
			wincom_open_help(chanlist_help, 3);
			group_get_name(cur_grp_idx, OSD_GetTextFieldStrPoint(&chanlist_grp_name));
			get_local_time(&present_end_time);
			#endif
			//wincom_close_preview();
			break;
		case EVN_POST_OPEN:
			
			#ifdef JINGJIN
			rc_con.uLeft = CHANLIST_PREVIEW_L;
			rc_con.uTop = CHANLIST_PREVIEW_T;
			rc_con.uWidth = CHANLIST_PREVIEW_W;
			rc_con.uHeight = CHANLIST_PREVIEW_H;
			rc_preview.uLeft = PREVIEW_L;
			rc_preview.uTop = PREVIEW_T;
			rc_preview.uWidth = PREVIEW_W;
			rc_preview.uHeight = PREVIEW_H;
			wincom_open_preview_ext(rc_con, rc_preview, PREVIEW_IDX);
			#endif
			
			/*
			vdec_io_control((struct vdec_device*)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_GET_STATUS, (UINT32) &CurStatus);
			if (!CurStatus.uFirstPicShowed)
			{
				//show blank screen in preview box
				struct YCbCrColor tColor;
				tColor.uY = 0x10;
				tColor.uCb = 0x80;
				tColor.uCr = 0x80;
				vdec_io_control((struct vdec_device*)dev_get_by_id(HLD_DEV_TYPE_DECV, 0), VDEC_IO_FILL_FRM, (UINT32)(&tColor));
				vpo_win_onoff((struct vpo_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_DIS), TRUE);
			}
			*/
			chanlist_draw_epg_pf();
			if (mode_changed_flag)
			{
				UINT16 curChannel;
				curChannel = sys_data_get_cur_group_cur_mode_channel();
				api_play_channel(curChannel, TRUE, TRUE, FALSE);
				mode_changed_flag = FALSE;
			}
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			break;
		case EVN_MSG_GOT:
			ret = chanlist_message_proc(param1, param2);
			break;
		case EVN_PRE_CLOSE:
			if ((POBJECT_HEAD) &g_win_mainmenu == menu_stack_get(0))
				*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			//wincom_close_preview();
			
			if ((POBJECT_HEAD)&g_win_mainmenu != menu_stack_get(0))
			{
				wincom_close_title();
				wincom_close_help();
			}
			// av_mode = (av_mode == TV_CHAN) ? RADIO_CHAN : TV_CHAN;

			/*added for show pwd window if the program is locked   *******vincent*/
			/* sys_data_get_cur_group_channel (&chan_idx, av_mode );
			get_prog_at (chan_idx, &p_node );
			if (sys_data_get_channel_lock() && (p_node.lock_flag || p_node.provider_lock))
			{
			api_play_channel(chan_idx,  TRUE,  TRUE,  FALSE);
			}
			 */
			break;
	}

	return ret;
}

static VACTION chanlist_list_keymap(POBJECT_HEAD pObj, UINT32 key)
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
			
		#ifdef JINGJIN	
			case V_KEY_LEFT:
				act = VACT_GRP_DEC;
				break;
			case V_KEY_RIGHT:
				act = VACT_GRP_INC;
				break;
		#else
			case V_KEY_RIGHT:
				if (OSD_ObjOpen((POBJECT_HEAD)&g_win_chan_detail, 0xFFFFFFFF) != PROC_LEAVE)
					menu_stack_push((POBJECT_HEAD)&g_win_chan_detail);
				break;
		#endif
		
		case V_KEY_TVRADIO:
			act = VACT_TV_RADIO_SW;
			break;
		case V_KEY_ENTER:
			act = VACT_PLAY_CH;
			break;
		default:
			act = VACT_PASS;
			break;
	}

	return act;
}

static PRESULT chanlist_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;

	switch (event)
	{
		case EVN_PRE_DRAW:
			chanlist_prg_set_display();
			wincom_draw_time_ext(&chanlist_date);		//J.Jing
			#ifndef JINGJIN
			chanlist_draw_msg_con();
			#endif
			
			break;
		case EVN_ITEM_POST_CHANGE:
			chanlist_msg_con_update(pObj);
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			chanlist_unact_proc(unact);
			break;
	}
	return ret;
}
#ifndef JINGJIN
static void chanlist_msg_con_update(POBJECT_HEAD pObj)
{
	UINT16 uni_name[30];
	UINT8 *new_name;
	UINT16 curitem;
	P_NODE pNode;  
	UINT32 i;
	TEXT_FIELD *txt;
	TEXT_FIELD *txt1;
	char string[100];
	INT32 strlen;
	txt = &chanlist_program_num;
	txt1 = &chanlist_program_name;
	//txt->pString = display_strs[33];
	//txt1->pString = display_strs[34];
	
	curitem = OSD_GetObjListNewPoint((OBJLIST*)pObj);
	chanlist_set_cur_service();
	SetPauseStatus(FALSE); 
	if (SUCCESS == get_prog_at(curitem, &pNode))
	{
	/* Init display strings */
		for (i=0; i<2; i++)
			MEMSET(display_strs[33+i], 0, 2*MAX_DISP_STR_LEN);
	
	/* Set prog_num content */
		MEMSET(string, 0, sizeof(string));
		sprintf(string, "%03d", curitem+1);
		ComAscStr2Uni(string, display_strs[33]);
		OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)display_strs[33]);		
	
	/* Set prog_name content */
		MEMSET(string, 0, sizeof(string));
		if (pNode.ca_mode == 0)
			STRCPY(string, "");
		else
			STRCPY(string, "$");
		ComAscStr2Uni(string, display_strs[34]);
		strlen = ComUniStrLen(display_strs[34]);
		ComUniStrCopyChar((UINT8*) &display_strs[34][strlen], pNode.service_name);
		OSD_SetTextFieldContent(txt1, STRING_UNICODE, (UINT32)display_strs[34]);

		OSD_DrawObject((POBJECT_HEAD)&chanlist_program_num, C_UPDATE_ALL);
		OSD_DrawObject((POBJECT_HEAD)&chanlist_program_name, C_UPDATE_ALL);
	}	
}
#endif

static void chanlist_prg_set_display(void)
{
	POBJLIST ol = &chanlist_prg_ol;
	PCONTAINER item;
	PTEXT_FIELD txt;
	UINT32 i;
	UINT32 valid_idx;
	UINT16 top, cnt, page, index, curitem;
	P_NODE p_node;
	UINT16 unistr[30];
	char str[10];

	cnt = OSD_GetObjListCount(ol);
	page = OSD_GetObjListPage(ol);
	top = OSD_GetObjListTop(ol);
	curitem = OSD_GetObjListNewPoint(ol);

	for (i = 0; i < page; i++)
	{
		item = (CONTAINER*)chanlist_prg_items[i];
		index = top + i;

		valid_idx = (index < cnt) ? 1 : 0;
		if (valid_idx)
		{
			get_prog_at(index, &p_node);
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
			if (p_node.ca_mode)
				ComAscStr2Uni("$", unistr);
			ComUniStrCopyChar((UINT8*) &unistr[p_node.ca_mode], p_node.service_name);
			OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)unistr);
		}
		else
			OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

	}
}

static PRESULT chanlist_unact_proc(VACTION act)
{
	PRESULT ret = PROC_PASS;
	UINT8 cur_grp_idx, grp_idx, back_saved;
	UINT8 av_mode, nxt_av_mode;
	POBJLIST ol = &chanlist_prg_ol;
	UINT16 channel, strID;
	P_NODE p_node;
	static UINT8 last_grp = ALL_GROUP_TYPE;
	UINT16 sel;
	enum API_PLAY_TYPE ret_enum;

	cur_grp_idx = sys_data_get_cur_group_index();
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_IRDETO)
			if(getStopChannelChange())//check whether stop channel change
				return ret ;
#endif
#endif
	if (act == VACT_PLAY_CH)
	{
		sel = OSD_GetObjListCurPoint(ol);
		chanlist_set_cur_service();
		SetPauseStatus(FALSE);
		if (last_grp != cur_grp_idx || sel != OSD_GetObjListSingleSelect(ol))
		{
			last_grp = cur_grp_idx;
			OSD_SetObjListSingleSelect(ol, sel);
			
			if (get_prog_at((UINT16)sel, &p_node) != SUCCESS)
			{
				return PROC_LOOP;
			}
			
			ret_enum = api_play_channel(sel, TRUE, TRUE, FALSE);
			last_valid_grp_idx = cur_grp_idx;
			if(!p_node.lock_flag || API_PLAY_NORMAL == ret_enum)
			{
				chanlist_draw_epg_pf();
			}
			else
			{
				OSD_SetTextFieldContent(&chanlist_sch_present, STRING_ANSI, (UINT32)" ");
				OSD_SetTextFieldContent(&chanlist_sch_following, STRING_ANSI, (UINT32)" ");
				OSD_DrawObject((POBJECT_HEAD)&chanlist_sch_con, C_UPDATE_ALL);
			}
		}
	}
	else if (act == VACT_TV_RADIO_SW)
	{
		grp_idx = cur_grp_idx;
		av_mode = sys_data_get_cur_chan_mode();
		nxt_av_mode = (av_mode == TV_CHAN) ? RADIO_CHAN : TV_CHAN;
		sys_data_set_cur_chan_mode(nxt_av_mode);
		sys_data_change_group(cur_grp_idx);
		sys_data_get_cur_group_channel(&channel, nxt_av_mode);
		if (channel == P_INVALID_ID)
		{
			sys_data_change_group(ALL_GROUP_TYPE);
			grp_idx = ALL_GROUP_TYPE;
			sys_data_get_group_channel(ALL_GROUP_TYPE, &channel, nxt_av_mode);
		}
		if (channel != P_INVALID_ID)
		{
			title_str_id = (title_str_id == RS_DIGITAL_TV) ? RS_RADIO : RS_DIGITAL_TV;
			title_bmp_id = (title_bmp_id == IM_TITLE_ICON_TV)? IM_TITLE_ICON_RADIO : IM_TITLE_ICON_TV;
			wincom_open_title_ext(title_str_id,title_bmp_id);
			group_get_name(grp_idx, OSD_GetTextFieldStrPoint(&chanlist_grp_name));
			OSD_DrawObject((POBJECT_HEAD)&chanlist_grp_name, C_UPDATE_ALL);
			chanlist_prg_list_load();
			sel = OSD_GetObjListCurPoint(ol);
			api_play_channel(sel, TRUE, TRUE, FALSE);
			OSD_TrackObject((POBJECT_HEAD)ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
			chanlist_draw_epg_pf();
			OSD_DrawObject((POBJECT_HEAD) &chanlist_sch_con, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
		}
		else
		{
			if (nxt_av_mode == RADIO_CHAN)
				strID = RS_MSG_NO_RADIO_CHANNEL;
			else
				strID = RS_MSG_NO_TV_CHANNEL;
			win_compopup_init(WIN_POPUP_TYPE_SMSG);
			win_compopup_set_msg(NULL, NULL, strID);
			win_compopup_open_ext(&back_saved);
			sys_data_set_cur_chan_mode(av_mode);
			sys_data_change_group(cur_grp_idx);
			osal_task_sleep(1000);
			win_compopup_smsg_restoreback();
		}
		last_valid_grp_idx = sys_data_get_cur_group_index();
	}
	#ifdef JINGJIN
	else if ((act == VACT_GRP_DEC) || (act == VACT_GRP_INC))
	{
		grp_idx = sys_data_cur_group_inc_dec((act == VACT_GRP_INC), GRP_WITH_FAV);
		if (cur_grp_idx != grp_idx)
		{
			sys_data_change_group(grp_idx);
			group_get_name(grp_idx, OSD_GetTextFieldStrPoint(&chanlist_grp_name));
			OSD_DrawObject((POBJECT_HEAD)&chanlist_grp_name, C_UPDATE_ALL);
			chanlist_prg_list_load();
			OSD_TrackObject((POBJECT_HEAD)ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
			chanlist_draw_epg_pf();
			OSD_DrawObject((POBJECT_HEAD) &chanlist_sch_con, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
		}
	}
	#endif
	return ret;
}

static PRESULT chanlist_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;
	date_time cur_time;
	POBJLIST ol = &chanlist_prg_ol;

	switch (msg_type)
	{
		case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
			#ifdef JINGJIN
			wincom_preview_proc();
			#endif
			break;
		case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
			if (is_time_inited())
			{
				#ifdef JINGJIN
				wincom_draw_title_time()
				#endif
				
				get_local_time(&cur_time);
				if (date_time_compare(&cur_time, &present_end_time))
				{
					chanlist_draw_epg_pf();
				}
			}
			break;
		case CTRL_MSG_SUBTYPE_CMD_EPG_PF_UPDATED:
		case CTRL_MSG_SUBTYPE_CMD_EPG_SCH_UPDATED:
			if (OSD_GetObjListCount(ol) < 2)
				chanlist_draw_epg_pf();
		default:
			break;
	}

	return ret;
}

static UINT16 chanlist_prg_list_load(void)
{
	UINT8 group_type, group_pos;
	UINT16 channel, ch_cnt = 0, page, top;
	UINT8 av_flag;
	POBJLIST ol = &chanlist_prg_ol;

	channel = sys_data_get_cur_group_cur_mode_channel();

	av_flag = sys_data_get_cur_chan_mode();
	ch_cnt = get_prog_num(VIEW_ALL | av_flag, 0);

	if (channel >= ch_cnt)
		channel = 0;

	page = OSD_GetObjListPage(ol);

	top = channel / page * page;
	if (ch_cnt > page)
	{
		if ((top + page) > ch_cnt)
			top = ch_cnt - page;
	}

	OSD_SetObjListCount(ol, ch_cnt);
	OSD_SetObjListCurPoint(ol, channel);
	OSD_SetObjListNewPoint(ol, channel);
	OSD_SetObjListTop(ol, top);

	return ch_cnt;
}

static UINT32  chanlist_draw_epg_pf(void)
{
	eit_event_info_t *pe = NULL,  *fe = NULL;
	UINT8 *s1 = NULL,  *s2 = NULL;
	UINT16 cur_channel;
	UINT16 present_name[33];
	UINT16 following_name[33];
	PTEXT_FIELD txt = NULL;
	UINT8 str[30];
	UINT16 len = 0;
	UINT32 ret = 0;
	INT32 h=0, m=0, sec=0;
	date_time start_time, end_time;
	get_local_time(&start_time);

	cur_channel = sys_data_get_cur_group_cur_mode_channel();

	pe = epg_get_cur_service_event((INT32)cur_channel, PRESENT_EVENT, &start_time, NULL, NULL, TRUE);
	fe = epg_get_cur_service_event((INT32)cur_channel, FOLLOWING_EVENT, &start_time, NULL, NULL, TRUE);
	s1 = (UINT8*)epg_get_event_name(pe, present_name, 32);
	s2 = (UINT8*)epg_get_event_name(fe, following_name, 32);

	if (pe != NULL)
	{
		MEMSET(str, 0, sizeof(str));
		MEMSET(len_display_str, 0, sizeof(len_display_str));

		get_STC_offset(&h, &m, &sec);
		get_event_start_time(pe, &start_time);
		get_event_end_time(pe, &end_time);
		convert_time_by_offset(&start_time, &start_time, h, m);
		convert_time_by_offset(&end_time, &end_time, h, m);
		
		sprintf(str, "%02d:%02d:%02d~%02d:%02d:%02d  ", start_time.hour, start_time.min, start_time.sec,  \
			end_time.hour, end_time.min, end_time.sec);
		ComAscStr2Uni(str, len_display_str);
		len = ComUniStrLen(len_display_str);
		if (s1 != NULL)
		{
			txt = &chanlist_sch_present;
			ComUniStrCopy(&len_display_str[len], present_name);
			OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)len_display_str);
			ret++;
		}
	}
	else
	 /*else show blank*/
	{
		txt = &chanlist_sch_present;
		MEMSET(str, 0, sizeof(str));
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
	}

	if (fe != NULL)
	{
		MEMSET(str, 0, sizeof(str));
		MEMSET(len_display_str, 0, sizeof(len_display_str));

		get_STC_offset(&h, &m, &sec);
		get_event_start_time(fe, &start_time);
		get_event_end_time(fe, &end_time);
		convert_time_by_offset(&start_time, &start_time, h, m);
		convert_time_by_offset(&end_time, &end_time, h, m);
		
		sprintf(str, "%02d:%02d:%02d~%02d:%02d:%02d  ", start_time.hour, start_time.min, start_time.sec,  \
			end_time.hour, end_time.min, end_time.sec);
		ComAscStr2Uni(str, len_display_str);
		len = ComUniStrLen(len_display_str);
		if (s2 != NULL)
		{
			txt = &chanlist_sch_following;
			ComUniStrCopy(&len_display_str[len], following_name);
			OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)len_display_str);
			ret++;
		}
	}
	else
	{
		txt = &chanlist_sch_following;
		MEMSET(str, 0, sizeof(str));
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
	}

	//if ((pe != NULL) || (fe != NULL))
		//OSD_DrawObject((POBJECT_HEAD)&chanlist_sch_con, C_UPDATE_ALL);
		OSD_DrawObject((POBJECT_HEAD)&chanlist_sch_present, C_UPDATE_ALL);
		OSD_DrawObject((POBJECT_HEAD)&chanlist_sch_following, C_UPDATE_ALL);
	return ret;
}
#ifndef JINGJIN
static void chanlist_draw_msg_con(void)
{
	struct ACTIVE_SERVICE_INFO service;
		P_NODE pNode;
		POBJLIST ol = &chanlist_prg_ol;
		UINT16 curitem; 
		UINT32 i;
		date_time dt;
		TEXT_FIELD *txt;
		TEXT_FIELD *txt1;
        	TEXT_FIELD *txt2;
		char string[100];
		INT32 strlen;
		UINT8 *src;
		
		txt = &chanlist_program_num;
		txt1 = &chanlist_program_name;
		txt2 = &chanlist_date;
		//txt->pString = display_strs[33];
		//txt1->pString = display_strs[34];
		get_local_time(&dt);
		curitem = sys_data_get_cur_group_cur_mode_channel();
		chanlist_set_cur_service();
		if (SUCCESS == get_prog_at(curitem, &pNode))
		{
		/* Init display strings */
			for (i=0; i<2; i++)
				MEMSET(display_strs[33+i], 0, 2*MAX_DISP_STR_LEN);
		
		/* Set prog_num content */
				MEMSET(string, 0, sizeof(string));
				sprintf(string, "%03d", curitem+1);
				ComAscStr2Uni(string, display_strs[33]);
		OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)display_strs[33]);		
		
		/* Set prog_name content */
			MEMSET(string, 0, sizeof(string));
			if (pNode.ca_mode == 0)
				STRCPY(string, "");
			else
				STRCPY(string, "$");
			ComAscStr2Uni(string, display_strs[34]);
			strlen = ComUniStrLen(display_strs[34]);
			ComUniStrCopyChar((UINT8*) &display_strs[34][strlen], pNode.service_name);
			OSD_SetTextFieldContent(txt1, STRING_UNICODE, (UINT32)display_strs[34]);

		/* Set prog_time content */
			/*
			MEMSET(string, 0, sizeof(string));
			sprintf(string, "%4d.%02d.%02d ", dt.year, dt.month, dt.day);
			ComAscStr2Uni(string, display_strs[32]);
			strlen = ComUniStrLen(display_strs[32]);
			src = OSD_GetUnicodeString(weekday_id[dt.weekday%7]);
			ComUniStrCopyChar((UINT8*) &display_strs[32][strlen], src);
			OSD_SetTextFieldContent(txt2, STRING_UNICODE, (UINT32)display_strs[32]);
			*/
		}	
}
#endif
static void chanlist_set_cur_service(void)
{
	struct ACTIVE_SERVICE_INFO service;
	P_NODE pNode;
	POBJLIST ol = &chanlist_prg_ol;
	UINT16 curitem;

	curitem = OSD_GetObjListNewPoint(ol);

	if (SUCCESS == get_prog_at(curitem, &pNode))
	{
		service.tp_id = pNode.tp_id;
		service.service_id = pNode.prog_number;
		epg_set_active_service(&service, 1);
	}
}

static VACTION chanlist_list_item_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	return act;
}

static PRESULT chanlist_list_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;

	return ret;
}

