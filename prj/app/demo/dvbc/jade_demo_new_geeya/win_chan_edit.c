#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libtsi/sec_pmt.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_hde.h>
#include <hld/dis/vpo.h>
#include <hld/nim/nim.h>
#include <hld/decv/vdec_driver.h>

//#include <api/libdb/db_config.h>
#include <api/libdb/db_return_value.h>
#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
#include <api/libdb/db_node_c.h>
#else
#include <api/libdb/db_node_s.h>
#endif
#include <api/libdb/db_node_api.h>
#include <api/libdb/db_interface.h>

//#include <api/libge/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "win_com.h"
#include "menus_root.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

#define DEB_PRINT	soc_printf

#include "win_chan_edit_obj.h"

#define CHANEDIT_L     0
#define CHANEDIT_T     90
#define CHANEDIT_W     720
#define CHANEDIT_H     400

#define LIST_CON_L     (66+TV_OFFSET_L)
#define LIST_CON_T     (102+TV_OFFSET_T)
#define LIST_CON_W     604
#define LIST_CON_H     439//350---jingjin

#define LIST_L     LIST_CON_L
#define LIST_T     (LIST_CON_T+34)
#define LIST_W     LIST_CON_W
#define LIST_H     (LIST_CON_H-34)

#define ITEM_CON_L  (LIST_L+4)
#define ITEM_CON_T  LIST_T
#define ITEM_CON_W  580
#define ITEM_CON_H  30

#define ITEM_CON_GAP  0

#define SCB_L (ITEM_CON_L + ITEM_CON_W)
#define SCB_T ITEM_CON_T
#define SCB_W 12
#define SCB_H (ITEM_CON_H*10+ITEM_CON_GAP*9)

#define IDX_W     80
#define NAME_W    300
#define FLAG_W    45

#define COLORBTN_L      84
#define COLORBTN_T      460
#define COLORBTN_W      24
#define COLORBTN_GAP    150
#define COLORTXT_W      (24*2)

static PRESULT ChanEditUnkownActProc(VACTION act);
static void ChanEditStrInit(UINT8 num);
static void ChanEditDrawTxt();
static void ChanEditDrawBmp();
static void ChanEditSetDisplay();
static void ChanEditLoadGroup(BOOL update);
static void ChanEditSetDelFlag(UINT32 chan_idx);
static void ChanEditClearDelFlag(UINT32 chan_idx);
static BOOL ChanEditGetDelFlag(UINT32 chan_idx);
static BOOL ChanEditCheckDelFlag(void);
static void ChanEditSwitchDelFlag(UINT32 chan_idx);
static void ChanEditClearAllDelFlag(void);
static void ChanEditMoveDelFlag(POBJLIST ol, UINT32 src_idx, UINT32 dst_idx);
static void RecreateCurTypeView();
//static BOOL ChanEditSaveSetting();
static UINT16 list_index_2_channel_index(OBJLIST *ol, UINT16 list_idx);
static PRESULT ChanEditMsgProc(UINT32 msg_type, UINT32 msg_code);


#define VACT_FAVOR 	            (VACT_PASS + 1)
#define VACT_DELETE 	        (VACT_PASS + 2)
#define VACT_LOCK 	            (VACT_PASS + 3)
#define VACT_MOVE 	            (VACT_PASS + 4)
#define VACT_TV_RADIO_SW 	    (VACT_PASS + 5)
#ifndef JINGJIN
#define VACT_SKIP	(VACT_PASS + 6)
#endif

/*******************************************************************************
 *	Local vriable & function declare
 *******************************************************************************/
#ifndef JINGJIN
UINT16 current_prg_number;
UINT16 current_prg_name[30];
BOOL skip_flag = FALSE;
BOOL favorite_flag = FALSE;
extern UINT8 chan_index_flag;
extern UINT16 chan_action_index;
extern UINT8 back_chan_action_flag;
extern UINT16 chan_old_action_index;
#endif
static UINT16 move_channel_idx = INVALID_POS_NUM;
static UINT16 pre_group_idx = 0;
static UINT32 pre_channel_pid = 0;
static UINT16 pre_mode = 0;
static P_NODE pre_pnode ;

static VACTION chanedit_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	UINT8 group_type = 0, group_pos = 0;
	UINT16 channel = 0;

	switch (key)
	{
		#ifdef JINGJIN
		case V_KEY_TVRADIO:
			act = VACT_TV_RADIO_SW;
			break;
		#endif
		case V_KEY_EXIT:
		case V_KEY_MENU:
			#ifndef JINGJIN
			OSD_ClearObject((POBJECT_HEAD)&g_win_chanedit, C_UPDATE_ALL);//防止CHAN_EDIT覆盖，但会出现闪屏
			#endif
			act = VACT_CLOSE; 	
			break;
		#ifdef JINGJIN
		case V_KEY_LIST:
			sys_data_save(1);
			//ChanEditSaveSetting();
			move_channel_idx = INVALID_POS_NUM;
			sys_data_get_cur_mode_group_infor(pre_group_idx, &group_type, &group_pos, &channel);
			if (channel != INVALID_ID)
				sys_data_change_group(pre_group_idx);
			BackToFullScrPlay();
			break;
		#endif
		default:
			act = VACT_PASS;
	}
	return act;
}

UINT32 del_channel_idx[MAX_PROG_NUM / 32+1];
#ifdef JINGJIN
struct help_item_resource chan_edit_help[] =
{
	{1,IM_HELP_ICON_TB,RS_SELECT_EDIT_ITEM},
    {0,RS_MENU,RS_HELP_BACK},
    {0,RS_HELP_EXIT,RS_HELP_EXIT},
};
#endif
static PRESULT chanedit_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	//P_NODE p_node;
	UINT16 pre_channel_index = 0;
	UINT8 group_type = 0, group_pos = 0;
	UINT16 channel = 0;	

	switch (event)
	{
		case EVN_PRE_OPEN:
			pre_mode = sys_data_get_cur_chan_mode();
			pre_channel_index = sys_data_get_cur_group_cur_mode_channel();
			memset(&pre_pnode, 0, sizeof(pre_pnode));
			get_prog_at(pre_channel_index, &pre_pnode);
			pre_channel_pid = pre_pnode.prog_id;
			ChanEditStrInit(20);
			#ifdef JINGJIN
			wincom_open_title_ext(RS_CHANNEL_EDIT, IM_TITLE_ICON_SYSTEM);
			wincom_open_help(chan_edit_help, 3);
			#endif
			ChanEditLoadGroup(FALSE);
			break;
		case EVN_POST_OPEN:
			#ifdef JINGJIN
			ChanEditDrawTxt();
			ChanEditDrawBmp();
			#endif
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			#ifdef JINGJIN
			sys_data_save(1);
			//ChanEditSaveSetting();
			#endif
			break;
		case EVN_POST_CLOSE:
			move_channel_idx = INVALID_POS_NUM;
			sys_data_get_cur_mode_group_infor(pre_group_idx, &group_type, &group_pos, &channel);
			if (channel != INVALID_ID)
				sys_data_change_group(pre_group_idx);
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			ret = ChanEditUnkownActProc(unact);
			break;
		case EVN_MSG_GOT:
			#ifdef JINGJIN
			ret = ChanEditMsgProc ( param1, param2 );
			break;
			#endif
        	default:
            		;
	}

	return ret;
}

static PRESULT ChanEditMsgProc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;
	switch (msg_type)
	{
		case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
			#ifdef JINGJIN
			if (is_time_inited())
			{
				wincom_draw_title_time();
			}
			#endif
            break;
		default:
			break;
	}
}

static PRESULT ChanEditListUnkownActProc(VACTION act)
{
	PRESULT ret = PROC_LOOP;
	OBJLIST *ol;
	UINT16 sel, sel_ch, ch_cnt;
	P_NODE p_node;
	UINT32 fav_mask;
    UINT16 i;
	ol = &chanedit_ol;
	sel = OSD_GetObjListCurPoint(ol);
	ch_cnt = OSD_GetObjListCount(ol);

	sel_ch = (INVALID_POS_NUM == move_channel_idx) ? sel : list_index_2_channel_index(ol, sel);
	get_prog_at(sel_ch, &p_node);

	switch (act)
	{
		#ifndef JINGJIN
		case VACT_ENTER:
			//获取IDX,NAME,SKIP_FLAG,FAV_FLAG 供下级菜单g_win_chanedit_action使用
			current_prg_number = sel;
			
			if (p_node.ca_mode)
				ComAscStr2Uni("$", current_prg_name);
			ComUniStrCopyChar((UINT8*) &current_prg_name[p_node.ca_mode], p_node.service_name);
			
			fav_mask = 0;
			for (i = 0; i < MAX_FAVGROUP_NUM; i++)
				fav_mask |= (0x01 << i);
			if (p_node.fav_group[0] &fav_mask)
				favorite_flag = TRUE;
			else
				favorite_flag = FALSE;
			if (p_node.skip_flag)
				skip_flag = TRUE;
			else
				skip_flag = FALSE;
			if (OSD_ObjOpen((POBJECT_HEAD)&g_win_chanedit_action, 0xFFFFFFFF) != PROC_LEAVE)
					menu_stack_push((POBJECT_HEAD)&g_win_chanedit_action);
			break;
		#endif
		#ifdef JINGJIN
		case VACT_FAVOR:
			p_node.fav_group[0] ^= 1;
			modify_prog(p_node.prog_id, &p_node);
			OSD_TrackObject((POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
			break;
		case VACT_DELETE:
			ChanEditSwitchDelFlag(sel_ch);
			OSD_TrackObject((POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
			break;
		case VACT_LOCK:
			p_node.lock_flag ^= 1;
			modify_prog(p_node.prog_id, &p_node);
			OSD_TrackObject((POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
			break;
		case VACT_MOVE:
			if (move_channel_idx > ch_cnt)
				move_channel_idx = sel;
			else
			{
				if (move_channel_idx != sel)
				{
					ChanEditMoveDelFlag(ol, move_channel_idx, sel);
					move_prog(sel, move_channel_idx);
				}
				move_channel_idx = INVALID_POS_NUM;
			}
			OSD_TrackObject((POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
			break;
		#endif
		default:
			break;
	}
	return ret;
}


static PRESULT ChanEditUnkownActProc(VACTION act)
{
	PRESULT ret = PROC_LOOP;
	UINT8 av_mode, back_saved;
	UINT16 channel, strID;

	av_mode = sys_data_get_cur_chan_mode();

	switch (act)
	{
		#ifdef JINGJIN
		case VACT_TV_RADIO_SW:
			av_mode = (av_mode == TV_CHAN) ? RADIO_CHAN : TV_CHAN;
			sys_data_get_cur_group_channel(&channel, av_mode);
			if (channel == P_INVALID_ID)
			 /* If the opposite mode has no channel */
			{
				if (av_mode == RADIO_CHAN)
					strID = RS_MSG_NO_RADIO_CHANNEL;
				else
					strID = RS_MSG_NO_TV_CHANNEL;
				win_compopup_init(WIN_POPUP_TYPE_SMSG);
				win_compopup_set_msg(NULL, NULL, strID);
				win_compopup_open_ext(&back_saved);
				osal_task_sleep(500);
				win_compopup_smsg_restoreback();
			}
			else
			{
				sys_data_save(1);
				//ChanEditSaveSetting();
				move_channel_idx = INVALID_POS_NUM;

				sys_data_set_cur_chan_mode(av_mode);
				ChanEditLoadGroup(FALSE);
				OSD_TrackObject((POBJECT_HEAD) &chanedit_ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
			}
			break;
		#endif
		default:
			;
	}
	return ret;
}


static VACTION chanedit_list_keymap(POBJECT_HEAD pObj, UINT32 key)
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
		case V_KEY_RED:
			act = VACT_FAVOR;
			break;
		case V_KEY_GREEN:
			act = VACT_DELETE;
			break;
		case V_KEY_YELLOW:
			act = VACT_LOCK;
			break;
		case V_KEY_BLUE:
			act = VACT_MOVE;
			break;
		#endif
		case V_KEY_EXIT:
			act = VACT_PASS;
			break;
		#ifndef JINGJIN
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
		#endif
		default:
			;
	}
	return act;
}

static PRESULT chanedit_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;

	switch (event)
	{
		case EVN_PRE_DRAW:
			//ret = PROC_LOOP;
			//if (param1 == C_DRAW_TYPE_HIGHLIGHT)
			//{
				//libc_printf("test\n");
				ChanEditSetDisplay();
				//OSD_TrackObject(pObj, C_UPDATE_ALL);
			//}
			break;
		case EVN_POST_DRAW:
			break;
		case EVN_ITEM_PRE_CHANGE:
			break;
		case EVN_ITEM_POST_CHANGE:
			break;
		case EVN_PRE_CHANGE:
			break;
		case EVN_POST_CHANGE:
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			ret = ChanEditListUnkownActProc(unact);
			break;
		case EVN_UNKNOWNKEY_GOT:
			break;
		default:
			;
	}


	return ret;
}

static void ChanEditStrInit(UINT8 num)
{
	UINT8 i = 0;

	for (i = 0; i < num; i++)
	{
		MEMSET(display_strs[i], 0, sizeof(display_strs[i]));
	}
}

#define LIST_CON_TOP_OFFSET 4

static OSD_RECT rc_txt[] =
{
    {ITEM_CON_L,LIST_CON_T+LIST_CON_TOP_OFFSET,IDX_W,24},
    {ITEM_CON_L+IDX_W,LIST_CON_T+LIST_CON_TOP_OFFSET,NAME_W,24},
    {COLORBTN_L+COLORBTN_W+COLORBTN_GAP*0,COLORBTN_T,COLORTXT_W,24},
    {COLORBTN_L+COLORBTN_W+COLORBTN_GAP*1,COLORBTN_T,COLORTXT_W,24},
    {COLORBTN_L+COLORBTN_W+COLORBTN_GAP*2,COLORBTN_T,COLORTXT_W,24},
    {COLORBTN_L+COLORBTN_W+COLORBTN_GAP*3,COLORBTN_T,COLORTXT_W,24}
};

#define TXT_NUM (sizeof(rc_txt)/sizeof(OSD_RECT))

static UINT16 id_txt[] =
{
    RS_SEARIAL_NUM,
    RS_SYSTEM_TIMERSET_CHANNEL,
    RS_FAV,
    RS_HELP_DELETE,
    RS_INFO_SIGNAL_STATUS_LOCKED,
    RS_HELP_MOVE
};

static OSD_RECT rc_bmp[] =
{
    {ITEM_CON_L+IDX_W+NAME_W+FLAG_W*0,LIST_CON_T+LIST_CON_TOP_OFFSET,FLAG_W,24},
    {ITEM_CON_L+IDX_W+NAME_W+FLAG_W*1,LIST_CON_T+LIST_CON_TOP_OFFSET,FLAG_W,24},
    {ITEM_CON_L+IDX_W+NAME_W+FLAG_W*2,LIST_CON_T+LIST_CON_TOP_OFFSET,FLAG_W,24},
    {ITEM_CON_L+IDX_W+NAME_W+FLAG_W*3,LIST_CON_T+LIST_CON_TOP_OFFSET,FLAG_W,24},
    {COLORBTN_L+COLORBTN_GAP*0,COLORBTN_T,COLORBTN_W,24},
    {COLORBTN_L+COLORBTN_GAP*1,COLORBTN_T,COLORBTN_W,24},
    {COLORBTN_L+COLORBTN_GAP*2,COLORBTN_T,COLORBTN_W,24},
    {COLORBTN_L+COLORBTN_GAP*3,COLORBTN_T,COLORBTN_W,24}
};

#define BMP_NUM (sizeof(rc_bmp)/sizeof(OSD_RECT))

static UINT16 id_bmp[] =
{
    IM_TV_FAVORITE,
    IM_TV_DEL,
    IM_TV_LOCK,
    IM_TV_MOVE,
    IM_EPG_COLORBUTTON_RED,
    IM_EPG_COLORBUTTON_GREEN,
    IM_EPG_COLORBUTTON_YELLOW,
    IM_EPG_COLORBUTTON_BLUE
};
#ifdef JINGJIN
static void ChanEditDrawTxt()
{
	UINT8 i = 0;
	TEXT_FIELD *txt;

	txt = &chanedit_txt;

	for (i = 0; i < TXT_NUM; i++)
	{
		OSD_SetRect2(&txt->head.frame, &rc_txt[i]);
		OSD_SetTextFieldContent(txt, STRING_ID, id_txt[i]);
		OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
	}

}

static void ChanEditDrawBmp()
{
	UINT8 i = 0;
	BITMAP *bmp;

	bmp = &chanedit_bmp;

	for (i = 0; i < BMP_NUM; i++)
	{
		OSD_SetRect2(&bmp->head.frame, &rc_bmp[i]);
		OSD_SetBitmapContent(bmp, id_bmp[i]);
		OSD_DrawObject((POBJECT_HEAD)bmp, C_UPDATE_ALL);
	}
}
#endif

#define FAV_ID  1
#define DEL_ID  2
#define LOCK_ID 3
#define MOVE_ID 4
#ifndef JINGJIN
#define SKIP_ID 5
#endif
UINT16 chan_mark_icons[] =
{
    IM_TV_FAVORITE,
    IM_TV_DEL,
    IM_TV_LOCK,
    IM_TV_MOVE,
    #ifndef JINGJIN
    IM_SKIP,
    #endif
};

static void ChanEditSetDisplay()
{
	OBJLIST *ol;
	CONTAINER *item;
	TEXT_FIELD *txt;
	BITMAP *bmp;
	UINT32 i, n;
	UINT32 valid_idx;
	UINT16 top, cnt, page, index, curitem, chan_idx;
	P_NODE p_node;
	UINT16 unistr[30];
	char str[10];
	UINT16 iconID;
	BOOL fav_flag;
	BOOL lock_flag;
	BOOL move_flag;
	BOOL del_flag;
	#ifndef JINGJIN
	BOOL skip_flag;
	#endif
	UINT32 fav_mask;

	ol = &chanedit_ol;

	cnt = OSD_GetObjListCount(ol);
	page = OSD_GetObjListPage(ol);
	top = OSD_GetObjListTop(ol);
	curitem = OSD_GetObjListNewPoint(ol);

	fav_mask = 0;
	for (i = 0; i < MAX_FAVGROUP_NUM; i++)
		fav_mask |= (0x01 << i);

	for (i = 0; i < page; i++)
	{
		//item = (CONTAINER*)chanedit_items[i];
		item = (CONTAINER*)chanedit_ol_ListField[i];
		index = top + i;
		fav_flag = lock_flag = del_flag = move_flag = skip_flag= FALSE;

		valid_idx = (index < cnt) ? 1 : 0;
		if (valid_idx)
		{
			chan_idx = list_index_2_channel_index(ol, index);
			get_prog_at(chan_idx, &p_node);
			if (p_node.fav_group[0] &fav_mask)
				fav_flag = TRUE;
			#ifdef JINGJIN
			if (p_node.lock_flag)
				lock_flag = TRUE;
			if ((INVALID_POS_NUM != move_channel_idx) && (index == curitem))
				move_flag = TRUE;
			del_flag = ChanEditGetDelFlag(chan_idx);
			#endif
			#ifndef JINGJIN
			if(p_node.skip_flag)
				skip_flag = TRUE;
			#endif

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

		/* FAV */
		bmp = (BITMAP*)OSD_GetObjpNext(txt);
		iconID = (fav_flag) ? chan_mark_icons[FAV_ID - 1]: 0;
		OSD_SetBitmapContent(bmp, iconID);
		#ifdef JIGNJIN
		/* Delete */
		bmp = (BITMAP*)OSD_GetObjpNext(bmp);
		iconID = (del_flag) ? chan_mark_icons[DEL_ID - 1]: 0;
		OSD_SetBitmapContent(bmp, iconID);

		/* Lock */
		bmp = (BITMAP*)OSD_GetObjpNext(bmp);
		iconID = (lock_flag) ? chan_mark_icons[LOCK_ID - 1]: 0;
		OSD_SetBitmapContent(bmp, iconID);

		/* Move */
		bmp = (BITMAP*)OSD_GetObjpNext(bmp);
		iconID = (move_flag) ? chan_mark_icons[MOVE_ID - 1]: 0;
		OSD_SetBitmapContent(bmp, iconID);
		#else
		/* SKIP */
		bmp = (BITMAP*)OSD_GetObjpNext(bmp);
		iconID = (skip_flag) ? chan_mark_icons[SKIP_ID - 1]: 0;
		OSD_SetBitmapContent(bmp, iconID);
		#endif
	}
}

static void ChanEditSetDelFlag(UINT32 chan_idx)
{
	UINT32 wIndex;
	UINT8 bShiftBit;

	wIndex = chan_idx / 32;
	bShiftBit = chan_idx % 32;
	del_channel_idx[wIndex] |= (0x00000001 << bShiftBit);
}

static void ChanEditClearDelFlag(UINT32 chan_idx)
{
	UINT32 wIndex;
	UINT8 bShiftBit;

	wIndex = chan_idx / 32;
	bShiftBit = chan_idx % 32;
	del_channel_idx[wIndex] &= ~(0x00000001 << bShiftBit);
}

static BOOL ChanEditGetDelFlag(UINT32 chan_idx)
{
	UINT32 wIndex;
	UINT8 bShiftBit;

	wIndex = chan_idx / 32;
	bShiftBit = (UINT8)(chan_idx % 32);

	if ((del_channel_idx[wIndex]) &(0x00000001 << bShiftBit))
		return TRUE;
	else
		return FALSE;
}

static BOOL ChanEditCheckDelFlag(void)
{
	OBJLIST *ol;
	UINT32 i, n;

	ol = &chanedit_ol;
	n = ol->wCount / 32; /* current channel total number */

	for (i = 0; i < n; i++)
		if ( del_channel_idx[i] > 0 )	return TRUE;

	for (i = n * 32; i < ol->wCount; i++)
		if (ChanEditGetDelFlag(i))
			return TRUE;

	return FALSE;
}

static void ChanEditSwitchDelFlag(UINT32 chan_idx)
{
	UINT32 wIndex;
	UINT8 bShiftBit;

	wIndex = chan_idx / 32;
	bShiftBit = chan_idx % 32;
	del_channel_idx[wIndex] ^= (0x00000001 << bShiftBit);
}

static void ChanEditClearAllDelFlag(void)
{
	UINT32 i, n;

	n = sizeof(del_channel_idx) / sizeof(del_channel_idx[0]);
	for (i = 0; i < n; i++)
		del_channel_idx[i] = 0;
};

static void ChanEditMoveDelFlag(POBJLIST ol, UINT32 src_idx, UINT32 dst_idx)
{
		UINT16 i;
		/* Move direction, TRUE: 2->1, 3->2, 4->3, 1->4, ; FALSE: 4->3, 3->2, 2->1, 1->4 */
		BOOL move_dir = (dst_idx > src_idx) ? TRUE : FALSE; 
		/* Backup sel del flag to avoid overwrite while moving other channel del flag */
		BOOL dst_del_flag = ChanEditGetDelFlag(src_idx);
		/* Reflash del flags between move_channel_idx and sel */
		for (i = src_idx; (move_dir ? (i<dst_idx) : (i>dst_idx)); (move_dir ? (i++) : (i--)))
		{
			if (ChanEditGetDelFlag(list_index_2_channel_index(ol, i)))
				ChanEditSetDelFlag(i);
			else
				ChanEditClearDelFlag(i);
		}

		if (dst_del_flag)
			ChanEditSetDelFlag(dst_idx);
		else
			ChanEditClearDelFlag(dst_idx);
}

static void ChanEditLoadGroup(BOOL update)
{
	UINT8 group_type, group_pos;
	UINT16 channel, ch_cnt, page;
	UINT8 av_flag;
	P_NODE p_node;
	OBJLIST *ol;
	SCROLL_BAR *scb;

	ol = &chanedit_ol;
	scb = &chanedit_scb;

	pre_group_idx = sys_data_get_cur_group_index();
	//change to SATE_GROUP_TYPE
	sys_data_change_group(0);
	sys_data_get_cur_mode_group_infor(0, &group_type, &group_pos, &channel);

	av_flag = sys_data_get_cur_chan_mode();
	ch_cnt = get_prog_num(VIEW_ALL | av_flag, 0);

	page = OSD_GetObjListPage(ol);
	#ifndef JINGJIN
		if(chan_index_flag == 0)//标志是从win_chan_action进入
		{
			if(back_chan_action_flag == 1)//标志通过ENTER返回
				channel = chan_action_index;
			else if(back_chan_action_flag == 0)//标志通过EXIT/MENU返回
				channel = chan_old_action_index;
			else;
		}
	#endif
	if (channel >= ch_cnt)
		channel = 0;

	if (!update)
	{
		get_prog_at(channel, &p_node);
	}
	
	OSD_SetObjListCount(ol, ch_cnt);
	OSD_SetObjListCurPoint(ol, channel);
	OSD_SetObjListNewPoint(ol, channel);
	OSD_SetObjListTop(ol, channel / page * page);

	if (update)
		OSD_TrackObject((POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_ALL);
}

static BOOL ChanEditSaveSetting()
{
	BOOL del_flag = FALSE, changed = FALSE;
	UINT8 back_saved;
	INT32 i, n, ret;
	win_popup_choice_t choice;
	OBJLIST *ol;
	P_NODE p_node;
	UINT16 pos, grp_ch, cur_channel;
	UINT8 grp_type, grp_pos, cur_mode;

	ol = &chanedit_ol;
	n = OSD_GetObjListCount(ol);

	del_flag = ChanEditCheckDelFlag();

	cur_mode = sys_data_get_cur_chan_mode();

	if (del_flag || check_node_modified(TYPE_PROG_NODE))
	{
		win_compopup_init(WIN_POPUP_TYPE_OKNO);
		win_compopup_set_msg(NULL, NULL, RS_MSG_ARE_YOU_SURE_TO_SAVE);
		choice = win_compopup_open_ext(&back_saved);
		if (choice == WIN_POP_CHOICE_YES)
		{
			if (del_flag)
			{
				for (i = n - 1; i >= 0; i--)
				{
					if (ChanEditGetDelFlag(i))
					{
						//sys_data_get_cur_mode_group_infor ( cur_chan_grp_idx, &grp_type, &grp_pos, &grp_ch );
						//if ( grp_type == FAV_GROUP_TYPE )
						if (0)
						{
							get_prog_at((UINT16)i, &p_node);
							p_node.fav_group[0] &= ~(0x1 << grp_pos);
							modify_prog(p_node.prog_id, &p_node);
						}
						else
						{
							ret = find_prg_id_from_index((UINT16)i);
							if (PROG_ID_NOTFOUND != ret)
								del_from_timer((UINT32)ret);

							/*if (SUCCESS == get_prog_at(i, &p_node))
							{
								epg_off();
								epg_del_service(p_node.tp_id, p_node.prog_number);
							}*/

							del_prog_at((UINT16)i);

						}
					}
				}
			}
			update_data();

			if (pre_mode != cur_mode)
			{
				recreate_prog_view(VIEW_ALL | pre_mode, 0);
			}

			if (get_prog_num(VIEW_ALL | pre_mode, 0) == 0)
			{
				cur_mode = (pre_mode == PROG_TV_MODE) ? PROG_RADIO_MODE : PROG_TV_MODE;

				recreate_prog_view(VIEW_ALL | cur_mode, 0);
				if (get_prog_num(VIEW_ALL | cur_mode, 0) > 0)
				{
					sys_data_set_cur_chan_mode(cur_mode);
					sys_data_change_group(0);
					pos = get_prog_pos(pre_channel_pid);
					if (pos != INVALID_POS_NUM)
					{
						/* if the program played before was not delete, so resume play it*/
						sys_data_set_cur_group_channel(pos);
						api_play_channel(pos, TRUE, FALSE, FALSE);
					}
					else
					{
						api_play_channel(0, TRUE, FALSE, FALSE);
					}
					key_pan_display("----", 4);
				}
				else
				{
					/* we got nothing here, so just stop play
					 * but we can not call api_stop_play directly. cause there is nothing in 
					 * each dbview. api_stop_play will do nothing.
					 */
					struct cc_param param;
					MEMSET(&param, 0, sizeof(param));
					api_set_channel_info(&pre_pnode, &param);
					chchg_stop_channel(&param.es, &param.dev_list, TRUE);
					key_pan_display("noCH", 4);
					setMtxt(0);//clear all popups
				}
			}
			else
			{
				sys_data_set_cur_chan_mode(pre_mode);
				pos = get_prog_pos(pre_channel_pid);
				if (pos != INVALID_POS_NUM)
				{
					sys_data_set_cur_group_channel(pos);
					api_play_channel(pos, TRUE, FALSE, FALSE);
				}
				else
				{
					api_play_channel(0, TRUE, FALSE, FALSE);
				}
				key_pan_display("----", 4);
			}

			sys_data_check_channel_groups();

			changed = TRUE;
		}
		else
		{
			RecreateCurTypeView();
		}
	}
	else
	{
		if (cur_mode != pre_mode)
		{
			sys_data_set_cur_chan_mode(pre_mode);
			recreate_prog_view(VIEW_ALL | pre_mode, 0);
		}
	}
	//sys_data_change_group ( cur_chan_grp_idx );//abandon current data modification
	ChanEditClearAllDelFlag();

	return changed;
}

static void RecreateCurTypeView()
{
	UINT8 cur_group, max_group;
	UINT16 cur_channel, max_channel;
	UINT8 group_type, group_pos;
	S_NODE s_node;
	P_NODE p_node;
	UINT32 n;
	SYSTEM_DATA *sys_data;
	UINT8 cur_chan_mode;

	sys_data = sys_data_get();

	max_group = sys_data_get_group_num();
	cur_group = sys_data_get_cur_group_index();
	sys_data_get_cur_mode_group_infor(cur_group, &group_type, &group_pos, &cur_channel);

	cur_chan_mode = sys_data_get_cur_chan_mode();
	if (group_type == ALL_GROUP_TYPE)
	{
		PRINTF("========Change to ALL_SATE_GROUP_TYPE\n");
		recreate_prog_view(VIEW_ALL | cur_chan_mode, 0);
	}
	else if (group_type == DATA_GROUP_TYPE)
	{
		get_sat_at(group_pos, VIEW_ALL, &s_node);
		recreate_prog_view(VIEW_SINGLE_SAT | cur_chan_mode, s_node.sat_id);
		PRINTF("========Change to SATE_GROUP_TYPE(%d,%s)\n", group_pos, s_node.sat_name);
	}
	else
	{
		PRINTF("========Change to FAV_GROUP_TYPE(%d)\n", group_pos);
		recreate_prog_view(VIEW_FAV_GROUP | cur_chan_mode, group_pos);
	}
}

static UINT16 list_index_2_channel_index(OBJLIST *ol, UINT16 list_idx)
{
	char flag = 0;
	UINT16 start_idx, end_idx, cur_idx, chan_idx, new_point;

	/* When moving, the channel displayer order is not same as normal */
	if (move_channel_idx != INVALID_POS_NUM)
	{
		new_point = OSD_GetObjListNewPoint(ol); //ol->wNewPoint;
		start_idx = (new_point > move_channel_idx) ? move_channel_idx : new_point;
		end_idx = (new_point > move_channel_idx) ? new_point : move_channel_idx;

		if (list_idx < start_idx || list_idx > end_idx)
			flag = 0;
		else
		{
			if (new_point == move_channel_idx)
				flag = 0;
			else if (new_point < move_channel_idx)
			{
				if (list_idx > new_point && list_idx <= move_channel_idx)
					flag =  - 1;
			}
			else
			{
				if (list_idx >= move_channel_idx && list_idx < new_point)
					flag = 1;
			}
		}

		if (list_idx == new_point)
		 /* foucs item show the channel in moving */
			chan_idx = move_channel_idx;
		else
			chan_idx = list_idx + flag;
	}
	else
	 /* not in moving state*/
		chan_idx = list_idx;

	return chan_idx;
}
