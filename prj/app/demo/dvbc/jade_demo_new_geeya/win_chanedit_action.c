#include <sys_config.h>

#include <types.h>
#include <basic_types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/osd/osddrv_dev.h>
//#include <api/libosd/osd_lib.h>
#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"

#include "control.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"

//include the header from xform 
#include "win_chanedit_action_vega.h"

extern UINT8 chan_index_flag;
extern UINT16 current_prg_number;
extern UINT16 current_prg_name[30];
extern BOOL skip_flag;
extern BOOL favorite_flag;

UINT16	CHANEDIT_ACTION_MULTITSEL[]  =
{
	RS_COMMON_ON,
	RS_MP_EQ_NORMAL,
};
UINT8 back_chan_action_flag;
UINT16 chan_old_action_index;
UINT16 chan_action_index;
static UINT16 move_channel_idx = INVALID_POS_NUM;
static UINT16 pre_group_idx = 0;
static UINT32 pre_channel_pid = 0;
static UINT16 pre_mode = 0;
static P_NODE pre_pnode ;

static void RecreateCurTypeView();
static void chan_modify_action();
static BOOL ChanModifySaveSetting();
/*******************************************************************************
*	Callback and Keymap Functions
*******************************************************************************/
static VACTION chanedit_action_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	switch (key)
	{
		case V_KEY_EXIT:
		case V_KEY_MENU:
			back_chan_action_flag = 0;//标志从EIXT/MENU键返回
			chan_index_flag = 0;//标志从win_chanedit_action进入win_chan_edit
			chan_old_action_index = current_prg_number;//为从win_chan_action 返回win_chan_edit提供焦点index--- 通过EXIT/MENU键
			act = VACT_CLOSE;
			break;
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
		default:
			act = VACT_PASS;
	}

	return act;

}
static PRESULT chanedit_action_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	TEXT_FIELD *txt;
	txt = &chan_num;
	TEXT_FIELD *txt1;
	txt1 = &chan_name;
	MULTISEL *s;
	s = &multisel_skip;
	MULTISEL *s1;
	s1 = &multisel_fav;
	UINT8 str[30];
	//INT16 multitsel_point;
	//INT16 multitsel_count;
	switch (event)
	{
		case EVN_PRE_OPEN:
			pre_mode = sys_data_get_cur_chan_mode();
			display_strs_init(20, 1);
			sprintf(str, "%03d", current_prg_number+1);
			OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
			OSD_SetTextFieldContent(txt1, STRING_UNICODE, (UINT32)current_prg_name);

			/*手动配置MULTITSEL组件
			multitsel_point = OSD_GetMultiselSel(s);
			multitsel_count = OSD_GetMultiselCount(s);			
			OSD_SetMultiselSelType(s, STRING_ID);
			OSD_SetMultiselCount(s, 2);
			OSD_SetMultiselSelTable(s, (void*)CHANEDIT_ACTION_MULTITSEL);
			*/
			if(skip_flag)
				OSD_SetMultiselSel(s,1);
			else
				OSD_SetMultiselSel(s,0);
			if(favorite_flag)
				OSD_SetMultiselSel(s1,1);
			else
				OSD_SetMultiselSel(s1,0);
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;//可消除闪屏现象---#JingJin
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			if(unact == VACT_ENTER)
			{
				chan_modify_action();
				chan_index_flag = 0;//标志从win_chanedit_action进入win_chan_edit
				ret = PROC_LEAVE;
			}
			break;
        	default:
				;
	}
	return ret;
}
static VACTION chanedit_action_con_keymap(POBJECT_HEAD pObj, UINT32 key)
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
		default:
			act = VACT_PASS;
	}

	return act;

}
static VACTION chanedit_action_multitsel_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_LEFT:
			act = VACT_DECREASE;
			break;
		case V_KEY_RIGHT:
			act = VACT_INCREASE;
			break;
		default:
			act = VACT_PASS;
	}
	return act;
}
static VACTION chanedit_action_editfield_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_LEFT:
			act = VACT_EDIT_LEFT;
			break;
		case V_KEY_RIGHT:
			act = VACT_EDIT_RIGHT;
			break;
		case V_KEY_0:
		case V_KEY_1:
		case V_KEY_2:
		case V_KEY_3:
		case V_KEY_4:
		case V_KEY_5:
		case V_KEY_6:
		case V_KEY_7:
		case V_KEY_8:
		case V_KEY_9:
			act = VACT_NUM_0 + (key - V_KEY_0);
			break;
		default:
			act = VACT_PASS;
	}
	return act;
}
static void chan_modify_action()
{
	MULTISEL *s;
	s = &multisel_skip;
	MULTISEL *s1;
	s1 = &multisel_fav;
	P_NODE p_node;
	INT16 crt_multitsel_skip;
	INT16 crt_multitsel_fav;
	OBJLIST *ol;
	ol = &chanedit_ol;
	UINT16 sel, sel_ch, ch_cnt;
	EDIT_FIELD *e;
	e = &editfield_insert;
	UINT16 *move_channel_idx;
	UINT16 target_move_idx;
	
	move_channel_idx = (UINT16*)OSD_GetEditFieldContent(e);
	target_move_idx = (UINT16)ComUniStr2Int(move_channel_idx);
	
	chan_action_index = target_move_idx-1;//为从win_chan_action 返回win_chan_edit提供焦点index--- 通过ENTER键
	
	sel = OSD_GetObjListCurPoint(ol);
	ch_cnt = OSD_GetObjListCount(ol);

	back_chan_action_flag = 1;//标志通过ENTER键返回
	
	get_prog_at(current_prg_number, &p_node);//原来是current_prg_number+1出现action错位
	
	crt_multitsel_skip = OSD_GetMultiselSel(s);
	crt_multitsel_fav = OSD_GetMultiselSel(s1);
	
	if(crt_multitsel_skip==0)
	{
		p_node.skip_flag = 0;
		modify_prog(p_node.prog_id, &p_node);
	}
	else if(crt_multitsel_skip==1)
	{
		p_node.skip_flag = 1;
		modify_prog(p_node.prog_id, &p_node);
	}
	else{;}
	
	if(crt_multitsel_fav==0)
	{
		p_node.fav_group[0] = 0;
		modify_prog(p_node.prog_id, &p_node);
	}
	else if(crt_multitsel_fav==1)
	{
		p_node.fav_group[0] = 1;
		modify_prog(p_node.prog_id, &p_node);
	}
	else{;}
	
	if (target_move_idx > ch_cnt)
		target_move_idx = sel;
	else
	{
		if (target_move_idx != sel)
		{
			move_prog(target_move_idx-1, sel);
		}
		target_move_idx = INVALID_POS_NUM;
	}
	
	sys_data_save(1);
	ChanModifySaveSetting();
}
static BOOL ChanModifySaveSetting()
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

	//if (check_node_modified(TYPE_PROG_NODE))
	//{
		//win_compopup_init(WIN_POPUP_TYPE_OKNO);
		//win_compopup_set_msg(NULL, NULL, RS_MSG_ARE_YOU_SURE_TO_SAVE);
		//choice = win_compopup_open_ext(&back_saved);
		//if (choice == WIN_POP_CHOICE_YES)
		//{
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
		//}
		//else
		//{
			//RecreateCurTypeView();
		//}
	//}
	//else
	//{
		//if (cur_mode != pre_mode)
		//{
			//sys_data_set_cur_chan_mode(pre_mode);
			//recreate_prog_view(VIEW_ALL | pre_mode, 0);
		//}
	//}
	//sys_data_change_group ( cur_chan_grp_idx );//abandon current data modification

	return changed;
}

