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
 
#include "win_child_lock_list_vega.h"

static void ChildLockStrInit(UINT8 num); 
static void ChildLockListDisplay();
static void ChildLockLoadGroup(BOOL update);
 static UINT16 ChildLock_list_index_2_channel_index(OBJLIST *ol, UINT16 list_idx);
 static BOOL ChildLockModifySaveSetting();

#define VACT_LOCK 	            (VACT_PASS + 3) 

/*******************************************************************************
 *	Local vriable & function declare
 *******************************************************************************/
 
static UINT16 move_channel_idx = INVALID_POS_NUM;
static UINT16 pre_group_idx = 0;
static UINT32 pre_channel_pid = 0;
static UINT16 pre_mode = 0;
static P_NODE pre_pnode ;

static VACTION childlock_list_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	switch (key)
	{
		case V_KEY_EXIT:
		case V_KEY_MENU:
			//OSD_ClearObject((POBJECT_HEAD)&g_win_childlock_list, C_UPDATE_ALL);//防止g_win_childlock_list覆盖，但会出现闪屏
			act = VACT_CLOSE; 	
			break;
		default:
			act = VACT_PASS;
	}
	return act;
}

static PRESULT childlock_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact; 
	UINT16 pre_channel_index = 0;
	UINT8 group_type = 0, group_pos = 0;
	UINT16 channel = 0;	
	TEXT_FIELD *childlock_name;
	childlock_name = &childlock_list_title_name;
	switch (event)
	{
		case EVN_PRE_OPEN:
			pre_mode = sys_data_get_cur_chan_mode();
			if(pre_mode == TV_CHAN)
			{
				OSD_SetTextFieldContent(childlock_name, STRING_ID, RS_TV_CHAN_LOCK);
			}
			else if(pre_mode == RADIO_CHAN)
			{
				OSD_SetTextFieldContent(childlock_name, STRING_ID, RS_RD_CHAN_LOCK);
			}
			else{;}
			pre_channel_index = sys_data_get_cur_group_cur_mode_channel();
			memset(&pre_pnode, 0, sizeof(pre_pnode));
			get_prog_at(pre_channel_index, &pre_pnode);
			pre_channel_pid = pre_pnode.prog_id;
			ChildLockStrInit(20);
			ChildLockLoadGroup(FALSE);
			break;
		case EVN_PRE_CLOSE:
			sys_data_save(1);
			ChildLockModifySaveSetting();
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			move_channel_idx = INVALID_POS_NUM;
			sys_data_get_cur_mode_group_infor(pre_group_idx, &group_type, &group_pos, &channel);
			if (channel != INVALID_ID)
				sys_data_change_group(pre_group_idx);
			break;
        	default:
            		break;
	}

	return ret;
}
static PRESULT ChildLockListUnkownActProc(VACTION act)
{
	PRESULT ret = PROC_LOOP;
	OBJLIST *ol;
	UINT16 sel,sel_ch;
	P_NODE p_node; 
	ol = &childlock_list_ol;
	
	sel = OSD_GetObjListCurPoint(ol);
	//sel_ch = (INVALID_POS_NUM == move_channel_idx) ? sel : ChildLock_list_index_2_channel_index(ol, sel);
	get_prog_at(sel, &p_node);

	switch (act)
	{
		case VACT_LOCK:
			p_node.lock_flag ^= 1;
			modify_prog(p_node.prog_id, &p_node);
			OSD_TrackObject((POBJECT_HEAD)ol, C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);
			break;
		default:
			break;
	}
	return ret;
}

static VACTION childlock_list_item_keymap(POBJECT_HEAD pObj, UINT32 key)
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
		case V_KEY_ENTER:
			act = VACT_LOCK;
			break;
		default:
			break;
	}
	return act;
}

static PRESULT childlock_list_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	
	switch (event)
	{
		case EVN_PRE_DRAW:
			ChildLockListDisplay();
			break;
		case EVN_PRE_CLOSE:
			sys_data_save(1);
			ChildLockModifySaveSetting();
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			ret = ChildLockListUnkownActProc(unact);
			break;
		default:
			break;
	}

	return ret;
}

static void ChildLockStrInit(UINT8 num)
{
	UINT8 i = 0;

	for (i = 0; i < num; i++)
	{
		MEMSET(display_strs[i], 0, sizeof(display_strs[i]));
	}
}
 
static void ChildLockListDisplay()
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
	BOOL lock_flag;
	
	ol = &childlock_list_ol;

	cnt = OSD_GetObjListCount(ol);
	page = OSD_GetObjListPage(ol);
	top = OSD_GetObjListTop(ol);
	curitem = OSD_GetObjListNewPoint(ol);
 
	for (i = 0; i < page; i++)
	{ 
		item = (CONTAINER*)childlock_list_ol_ListField[i];
		index = top + i;
		lock_flag = FALSE;

		valid_idx = (index < cnt) ? 1 : 0;
		if (valid_idx)
		{
			chan_idx = index;
			get_prog_at(chan_idx, &p_node);
 
			if (p_node.lock_flag)
				lock_flag = TRUE; 
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
		
		/* Lock */
		bmp = (BITMAP*)OSD_GetObjpNext(txt);
		iconID = (lock_flag) ? IM_TV_LOCK : 0;
		OSD_SetBitmapContent(bmp, iconID); 
		
	}
}
 
static void ChildLockLoadGroup(BOOL update)
{
	UINT8 group_type, group_pos;
	UINT16 channel, ch_cnt, page;
	UINT8 av_flag;
	P_NODE p_node;
	OBJLIST *ol;
	SCROLL_BAR *scb;

	ol = &childlock_list_ol;
	scb = &childlock_list_scb;

	pre_group_idx = sys_data_get_cur_group_index();
	//change to SATE_GROUP_TYPE
	sys_data_change_group(0);
	sys_data_get_cur_mode_group_infor(0, &group_type, &group_pos, &channel);

	av_flag = sys_data_get_cur_chan_mode();
	ch_cnt = get_prog_num(VIEW_ALL | av_flag, 0);

	page = OSD_GetObjListPage(ol); 
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

 static UINT16 ChildLock_list_index_2_channel_index(OBJLIST *ol, UINT16 list_idx)
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
static BOOL ChildLockModifySaveSetting()
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

	cur_mode = sys_data_get_cur_chan_mode();
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
				//api_play_channel(pos, TRUE, FALSE, FALSE);
			}
			else
			{
				//api_play_channel(0, TRUE, FALSE, FALSE);
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
			//api_play_channel(pos, TRUE, FALSE, FALSE);//防止点击确认后重新播台---JingJing
		}
		else
		{
			//api_play_channel(0, TRUE, FALSE, FALSE);
		}
		key_pan_display("----", 4);
	}
	sys_data_check_channel_groups();
	changed = TRUE;
	return changed;
}

