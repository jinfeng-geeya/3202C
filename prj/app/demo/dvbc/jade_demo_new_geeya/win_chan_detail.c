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
#include "win_chan_detail_vega.h"

/*******************************************************************************
 *	Objects definition
 *******************************************************************************/
CONTAINER g_win_chan_detail;

CONTAINER chan_detail_con;

CONTAINER chanlist_grp_con;
TEXT_FIELD chanlist_grp_name;

OBJLIST chan_detail_prg_ol;
SCROLL_BAR chan_detail_prg_scb;

CONTAINER chan_detail_prg_item1;
CONTAINER chan_detail_prg_item2;
CONTAINER chan_detail_prg_item3;
CONTAINER chan_detail_prg_item4;
CONTAINER chan_detail_prg_item5;
CONTAINER chan_detail_prg_item6;
CONTAINER chan_detail_prg_item7;
CONTAINER chan_detail_prg_item8;
CONTAINER chan_detail_prg_item9;
CONTAINER chan_detail_prg_item10;

TEXT_FIELD chan_detail_prg_idx1;
TEXT_FIELD chan_detail_prg_idx2;
TEXT_FIELD chan_detail_prg_idx3;
TEXT_FIELD chan_detail_prg_idx4;
TEXT_FIELD chan_detail_prg_idx5;
TEXT_FIELD chan_detail_prg_idx6;
TEXT_FIELD chan_detail_prg_idx7;
TEXT_FIELD chan_detail_prg_idx8;
TEXT_FIELD chan_detail_prg_idx9;
TEXT_FIELD chan_detail_prg_idx10;

TEXT_FIELD chan_detail_prg_name1;
TEXT_FIELD chan_detail_prg_name2;
TEXT_FIELD chan_detail_prg_name3;
TEXT_FIELD chan_detail_prg_name4;
TEXT_FIELD chan_detail_prg_name5;
TEXT_FIELD chan_detail_prg_name6;
TEXT_FIELD chan_detail_prg_name7;
TEXT_FIELD chan_detail_prg_name8;
TEXT_FIELD chan_detail_prg_name9;
TEXT_FIELD chan_detail_prg_name10;

static void chan_detail_prg_set_display(void);
static PRESULT chan_detail_unact_proc(VACTION act);
static UINT16 chan_detail_prg_list_load(void);
static void chan_detail_set_cur_service(void);


POBJECT_HEAD chan_detail_prg_items[] =
{
    (POBJECT_HEAD) &chan_detail_prg_item1,
    (POBJECT_HEAD) &chan_detail_prg_item2,
    (POBJECT_HEAD) &chan_detail_prg_item3,
    (POBJECT_HEAD) &chan_detail_prg_item4,
    (POBJECT_HEAD) &chan_detail_prg_item5,
    (POBJECT_HEAD) &chan_detail_prg_item6,
    (POBJECT_HEAD) &chan_detail_prg_item7,
    (POBJECT_HEAD) &chan_detail_prg_item8,
    (POBJECT_HEAD) &chan_detail_prg_item9,
    (POBJECT_HEAD) &chan_detail_prg_item10,
};

/*******************************************************************************
 *	Local variable & function declare
 *******************************************************************************/
static date_time present_end_time;

static UINT8 last_valid_grp_idx = 0;	/* Last valid group index */

static UINT8 pre_sch_cnt = 0;

static UINT16 title_str_id = 0;
static UINT16 title_bmp_id = 0;

/*******************************************************************************
 *	Key mapping and event callback definition
 *******************************************************************************/

static BOOL mode_changed_flag = FALSE;

#define VACT_PLAY_CH		(VACT_PASS+4)

static VACTION chan_detail_keymap(POBJECT_HEAD pObj, UINT32 key)
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
		case V_KEY_LEFT:
			act = VACT_CLOSE;
			break;
		case V_KEY_RIGHT:
			BackToFullScrPlay();
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}
 
static PRESULT chan_detail_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
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
			ch_cnt = get_prog_num(VIEW_ALL | av_mode, 0);
			if (ch_cnt == 0)
			{
				sys_data_change_group(ALL_GROUP_TYPE);
			}
			
			display_strs_init(10, 30);
			MEMSET(len_display_str, 0, sizeof(len_display_str));
			
			cur_grp_idx = sys_data_get_cur_group_index();
			last_valid_grp_idx = cur_grp_idx;
			chan_detail_prg_list_load();//设置好滑动条的属性
			
			get_local_time(&present_end_time);
			break;
		case EVN_POST_OPEN:	
			if (mode_changed_flag)//判断是否播放当前节目
			{
				UINT16 curChannel;
				curChannel = sys_data_get_cur_group_cur_mode_channel();
				api_play_channel(curChannel, TRUE, TRUE, FALSE);
				mode_changed_flag = FALSE;
			}
			break;
		case EVN_PRE_CLOSE:
			//*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;//可消除闪屏现象---#JingJin
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			break;
	}

	return ret;
}

static VACTION program_list_keymap(POBJECT_HEAD pObj, UINT32 key)
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
			act = VACT_PLAY_CH;
			break;
		default:
			act = VACT_PASS;
			break;
	}

	return act;
}

static PRESULT program_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;

	switch (event)
	{
		case EVN_PRE_DRAW:			//原始为EVN_PRE_DRAW 会出现死机现象
			chan_detail_prg_set_display();
			break;	
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			chan_detail_unact_proc(unact);
			break;
	}
	return ret;
}

static void chan_detail_prg_set_display(void)//显示chan_detail_prg_ol列表数据
{
	POBJLIST ol = &chan_detail_prg_ol;
	PCONTAINER item;
	PTEXT_FIELD txt;
	UINT16 i;
	UINT32 valid_idx;
	UINT16 top, cnt, page, index, curitem;
	P_NODE p_node;
	UINT16 unistr[30];
	UINT8 str[30];
	UINT32 eventNum = 0;
	UINT16 *s = NULL;
	UINT16 present_name[33];
	UINT16 len = 0;
	date_time start_time, end_time;
	eit_event_info_t *sch_event = NULL;
	INT32 h=0, m=0, sec=0;
	
	get_local_time(&start_time);
	
	cnt = OSD_GetObjListCount(ol);
	page = OSD_GetObjListPage(ol);
	top = OSD_GetObjListTop(ol);
	
	display_strs_init(10, 30);
	for (i = 0; i < page; i++)
	{
		item = (CONTAINER*)chan_detail_prg_items[i];
		index = top + i;

		valid_idx = (index < cnt) ? 1 : 0;
		if (valid_idx)
		{
			sch_event=epg_get_cur_service_event(index, SCHEDULE_EVENT,NULL,NULL, &eventNum, FALSE);//根据INDEX获取EPG信息----重要
			get_prog_at(index, &p_node);//从FLASH中提取DATABASE(idx,server_name)存入P_NODE结构体中供列表显示和EPG数据提取
		}
		else
		{
			STRCPY(str, "");
			unistr[0] = 0;
		}

		/* IDX show the program number #jingjin*/
		txt = (TEXT_FIELD*)OSD_GetContainerNextObj(item);//获取当前对象的第一个子对象first sub-object(chan_detail_prg_idx*)
		if (valid_idx)
			sprintf(str, "%03d", index + 1);
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);

		/* Name show the program name #jingjin*/
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);//获取上一个对象的相邻的下一个对象(chan_detail_prg_name*)
		if (valid_idx)
		{
			if (p_node.ca_mode)
				ComAscStr2Uni("$", unistr);
			ComUniStrCopyChar((UINT8*) &unistr[p_node.ca_mode], p_node.service_name);
			OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)unistr);
		}
		else
			OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
		
		/* EPG show the present name #jingjin*/
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);//获取上一个对象的相邻的下一个对象(chan_detail_pre_prg*)
		wincom_scroll_textfield_stop(txt);
		if (valid_idx)
		{
			if(sch_event != NULL)
			{
				//get the information from the EIT form#jingjin
				MEMSET(str, 0, sizeof(str));
				MEMSET(len_display_str, 0, sizeof(len_display_str));

				get_STC_offset(&h, &m, &sec);
				get_event_start_time(sch_event, &start_time);
				get_event_end_time(sch_event, &end_time);
				convert_time_by_offset(&start_time, &start_time, h, m);
				convert_time_by_offset(&end_time, &end_time, h, m);
						
				sprintf(str, "%02d:%02d:%02d~%02d:%02d:%02d  ", start_time.hour, start_time.min, start_time.sec,  \
						end_time.hour, end_time.min, end_time.sec);
				ComAscStr2Uni(str, len_display_str);
				len = ComUniStrLen(len_display_str);
				
				s = epg_get_event_name(sch_event, present_name, 32);
				if (s != NULL)
				{
					ComUniStrCopy(&len_display_str[len], present_name);
					OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)len_display_str);
				}
				else
				{
					MEMSET(str, 0, sizeof(str));
					OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
				}
			}
		}
		else
		{
			MEMSET(str, 0, sizeof(str));
			OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
		}
	}
}

static PRESULT chan_detail_unact_proc(VACTION act)
{
	PRESULT ret = PROC_PASS;
	UINT8 cur_grp_idx, grp_idx, back_saved;
	UINT8 av_mode, nxt_av_mode;
	POBJLIST ol = &chan_detail_prg_ol;
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
	if (act == VACT_PLAY_CH)//input the key:enter
	{
		sel = OSD_GetObjListCurPoint(ol);
		chan_detail_set_cur_service();
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
		}
	}
	return ret;
}

static UINT16 chan_detail_prg_list_load(void)//设置chan_detail_prg_ol的属性样式
{
	UINT8 group_type, group_pos;
	UINT16 channel, ch_cnt = 0, page, top;
	UINT8 av_flag;
	POBJLIST ol = &chan_detail_prg_ol;

	channel = sys_data_get_cur_group_cur_mode_channel();//获取当前GROUNP中节目的number  ---#jingjin

	av_flag = sys_data_get_cur_chan_mode();
	ch_cnt = get_prog_num(VIEW_ALL | av_flag, 0);//获取当前GROUNP所有的节目数量---#jingjin

	if (channel >= ch_cnt)
		channel = 0;

	page = OSD_GetObjListPage(ol);//获取每一页的ITEM 数量---#jingjin

	top = channel / page * page;//获取 当前页最顶层节目的ID number	---#jingjin
	if (ch_cnt > page)//当页数超过一页时
	{
		if ((top + page) > ch_cnt)//当前结节目在最后一页时
			top = ch_cnt - page;//设置当前节目在最后一页时的顶层number	---#jingjin
	}

	OSD_SetObjListCount(ol, ch_cnt);
	OSD_SetObjListCurPoint(ol, channel);
	OSD_SetObjListNewPoint(ol, channel);
	OSD_SetObjListTop(ol, top);

	return ch_cnt;
}
static void chan_detail_set_cur_service(void)
{
	struct ACTIVE_SERVICE_INFO service;
	P_NODE pNode;
	POBJLIST ol = &chan_detail_prg_ol;
	UINT16 curitem;
	curitem = OSD_GetObjListNewPoint(ol);

	if (SUCCESS == get_prog_at(curitem, &pNode))
	{
		service.tp_id = pNode.tp_id;
		service.service_id = pNode.prog_number;
		epg_set_active_service(&service, 1);
	}
}
static VACTION program_list_item_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	return act;
}
static PRESULT program_list_item_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	return ret;
}

