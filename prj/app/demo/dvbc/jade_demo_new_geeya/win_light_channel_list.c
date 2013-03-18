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

#include <api/libdb/db_return_value.h>
#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
#include <api/libdb/db_node_c.h>
#else
#include <api/libdb/db_node_s.h>
#endif
#include <api/libdb/db_node_api.h>
#include <api/libdb/db_interface.h>

//#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "win_com.h"
#include "menus_root.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

#include "win_light_channel_list_obj.h"
/*******************************************************************************
 *	Objects definition
 *******************************************************************************/
#define LIST_CNT		8

static PRESULT lcl_unact_proc(VACTION act);
static PRESULT lcl_message_proc(UINT32 msg_type, UINT32 msg_code);

static PRESULT lcl_list_unact_proc(VACTION act);
static void lcl_list_set_display(void);
static UINT16 lcl_prg_list_load(void);

/*******************************************************************************
 *	Local variables define
 *******************************************************************************/
static UINT8 only_fav_grp;				/* Flag of only show fav group */
static UINT8 last_valid_grp_idx = 0;	/* Last valid group index */

/*******************************************************************************
 *	Key mapping and event callback definition
 *******************************************************************************/
#define VACT_PLAY_CH		(VACT_PASS + 1)
#define VACT_TV_RADIO_SW	(VACT_PASS + 2)
#define VACT_GRP_INC		(VACT_PASS + 3)
#define VACT_GRP_DEC		(VACT_PASS + 4)


static VACTION lcl_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_MENU:
		case V_KEY_EXIT:
		case V_KEY_FAV:
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}

static PRESULT lcl_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	UINT8 av_mode, back_saved;
	UINT16 ch_cnt = 0;
	UINT16 channel;
	UINT16 popup_res;
	UINT8 cur_grp_idx;


	switch (event)
	{
		case EVN_PRE_OPEN:
#ifdef MULTI_CAS
#if ((CAS_TYPE != CAS_CONAX) && (CAS_TYPE != CAS_ABEL))
			ap_cas_message_close();
#endif
#if(CAS_TYPE==CAS_IRDETO)
			if(IRCA_BAN_GetWinpopShow()==TRUE)
				win_compopup_close();
#endif
#endif
			display_strs_init(10, 17);
			cur_grp_idx = sys_data_get_cur_group_index();
			last_valid_grp_idx = cur_grp_idx;
			av_mode = sys_data_get_cur_chan_mode();
			only_fav_grp = 0;
			popup_res = 0;
			if ((param2 & MENU_OPEN_TYPE_MASK) == MENU_OPEN_TYPE_KEY)
			{
				if ((param2 & MENU_OPEN_PARAM_MASK) == V_KEY_FAV)
					only_fav_grp = 1;
			}

			if (only_fav_grp)
			{
				sys_data_get_group_channel(FAV_GROUP_TYPE, &channel, av_mode);
				popup_res = RS_MSG_NO_PROGRAM_FAV;
			}
			else
			{
				sys_data_get_cur_group_channel(&channel, av_mode);
				if (channel == P_INVALID_ID)
				{
					sys_data_change_group(ALL_GROUP_TYPE);
					sys_data_get_cur_group_channel(&channel, av_mode);
				}
				popup_res = RS_MSG_NO_CHANNELS;
			}

			if (channel == P_INVALID_ID)
			{
					win_compopup_init(WIN_POPUP_TYPE_SMSG);
					win_compopup_set_msg(NULL, NULL, popup_res);
					win_compopup_open_ext(&back_saved);
					osal_task_sleep(1000);
					win_compopup_smsg_restoreback();
					return PROC_LEAVE;
			}

			if (only_fav_grp)
			{
				sys_data_change_group(FAV_GROUP_TYPE);
				cur_grp_idx = FAV_GROUP_TYPE;
			}

			lcl_prg_list_load();
			group_get_name(cur_grp_idx, OSD_GetTextFieldStrPoint(&lcl_title));
			#ifdef MIS_AD
			MIS_ShowEpgAdv(3);
			#endif
            		//ad_show(AD_OKLIST);
			break;
		case EVN_PRE_CLOSE:
			if (last_valid_grp_idx != sys_data_get_cur_group_index())
				sys_data_change_group(last_valid_grp_idx);
			sys_data_save(1);
            		//ad_stop(AD_OKLIST);
			#ifdef MIS_AD
			MIS_HideEpgAdv();
			#endif
			break;
		case EVN_POST_CLOSE:
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_IRDETO)
			if(IRCA_BAN_GetBannerShow())
			{
				//ap_cas_call_back(0x00000020);
				//ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_MCAS,0x00200000 , FALSE);//CA_BANNER_E<<16
				ap_send_msg_expand(CTRL_MSG_SUBTYPE_STATUS_MCAS,0x00000020 ,0, FALSE);
			}
#endif
#endif

#if ((TTX_ON == 1)||(SUBTITLE_ON == 1)) 
            api_osd_mode_change(OSD_SUBTITLE);                 
#endif
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			ret = lcl_unact_proc(unact);
			break;
		case EVN_MSG_GOT:
			ret = lcl_message_proc(param1, param2);
			break;
		default:
			break;
	}

	return ret;
}

static PRESULT lcl_unact_proc(VACTION act)
{
	PRESULT ret = PROC_PASS;
	PCONTAINER con = &g_win_light_chanlist;

	if (act >= VACT_NUM_1 && act <= VACT_NUM_9)
	{
		OSD_ChangeFocus((POBJECT_HEAD)con, act - VACT_NUM_0, C_DRAW_SIGN_EVN_FLG | C_UPDATE_FOCUS);
	}
}

static PRESULT lcl_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;
	POBJLIST ol = &lcl_item_list;
	UINT16 play_ch_idx;

	switch (msg_type)
	{
		case CTRL_MSG_SUBTYPE_CMD_PIDCHANGE:

#if (CHAN_NORMAL_PLAY_TYPE == 0)
			play_ch_idx = OSD_GetObjListSingleSelect(ol);
#else
			play_ch_idx = OSD_GetObjListCurPoint(ol);
#endif
			if (play_ch_idx < OSD_GetObjListCount(ol))
				api_play_channel(play_ch_idx, TRUE, TRUE, FALSE);
			break;
		default:
			ret = PROC_PASS;

	}

	return ret;
}

static VACTION lcl_list_keymap(POBJECT_HEAD pObj, UINT32 key)
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
		case V_KEY_TVRADIO:
			act = VACT_TV_RADIO_SW;
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

static PRESULT lcl_list_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;

	switch (event)
	{
		case EVN_PRE_DRAW:
			lcl_list_set_display();
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			ret = lcl_list_unact_proc(unact);
			break;
#ifdef MIS_AD
		case EVN_ITEM_POST_CHANGE:
			api_play_channel(OSD_GetObjListCurPoint(&lcl_item_list), TRUE, TRUE, FALSE);
			OSD_SetObjListSingleSelect(&lcl_item_list, OSD_GetObjListCurPoint(&lcl_item_list));

			MIS_ShowEpgAdv(3);
			//切台泡泡重新计数
			Mis_Set_SameChan_AdvShowOnce(TRUE);
			Mis_Set_EnterAutoShow(FALSE);
			break;
#endif
		default:
			break;
	}

	return ret;
}

static PRESULT lcl_list_unact_proc(VACTION act)
{
	PRESULT ret = PROC_PASS;
	POBJLIST ol  = &lcl_item_list;
	UINT8 back_saved;
	UINT8 av_mode, nxt_av_mode;
	UINT16 channel;
	UINT8 cur_grp_idx, grp_idx;
	UINT16 sel;
	UINT16 strID;
	P_NODE p_node;
	static UINT8 last_grp =  ALL_GROUP_TYPE;
    last_grp = last_valid_grp_idx;
	cur_grp_idx = sys_data_get_cur_group_index();
#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_IRDETO)
			if(getStopChannelChange())//check whether stop channel change
				return ret ;
#endif
#endif
	
	switch (act)
	{
	case VACT_PLAY_CH:
		sel = OSD_GetObjListCurPoint(ol);
		SetPauseStatus(FALSE);
		if (last_grp != cur_grp_idx|| sel != OSD_GetObjListSingleSelect(ol))
		{
			last_grp = cur_grp_idx;
			OSD_SetObjListSingleSelect(ol, sel);
			
			if (get_prog_at((UINT16)sel, &p_node) != SUCCESS)
			{
				return PROC_LOOP;
			}
#if ((TTX_ON == 1)||(SUBTITLE_ON == 1)) 
//clear the subtitle info
#if (SUBTITLE_ON == 1)
				subt_show_onoff(FALSE);
#endif	
#if (TTX_ON ==1)
				TTXEng_ShowOnOff(FALSE);
#endif 
                api_play_channel(sel, TRUE, TRUE, FALSE);
//                api_osd_mode_change(OSD_SUBTITLE);                
#else
            api_play_channel(sel, TRUE, TRUE, FALSE);
#endif
			last_valid_grp_idx = cur_grp_idx;
		}
		else
			ret = PROC_LEAVE;
		break;
	case VACT_TV_RADIO_SW:
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
			group_get_name(grp_idx, OSD_GetTextFieldStrPoint(&lcl_title));
			OSD_DrawObject((POBJECT_HEAD)&lcl_title, C_UPDATE_ALL);
			lcl_prg_list_load();
			sel = OSD_GetObjListCurPoint(ol);
			api_play_channel(sel, TRUE, TRUE, FALSE);
			OSD_TrackObject((POBJECT_HEAD)ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
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
		break;
	case VACT_GRP_INC:
	case VACT_GRP_DEC:
		if (only_fav_grp == 0)
		{
			grp_idx = sys_data_cur_group_inc_dec((act == VACT_GRP_INC), GRP_WITH_FAV);
			if (cur_grp_idx != grp_idx)
			{
				sys_data_change_group(grp_idx);
				group_get_name(grp_idx, OSD_GetTextFieldStrPoint(&lcl_title));
				OSD_DrawObject((POBJECT_HEAD)&lcl_title, C_UPDATE_ALL);
				lcl_prg_list_load();
				OSD_TrackObject((POBJECT_HEAD)ol, C_UPDATE_ALL | C_DRAW_SIGN_EVN_FLG);
			}
		}
		break;
	default:
		break;
	}

	return ret;
}

void lcl_list_set_display(void)
{
	UINT32 i;
	UINT16 top, cnt, page, chan_idx;
	POBJLIST ol = &lcl_item_list;
	PBITMAP bmp;
	PTEXT_FIELD txt;
	PCONTAINER item;
	P_NODE p_node;
	UINT32 valid_idx;
	UINT16 unistr[50];

	cnt = OSD_GetObjListCount(ol);
	page = OSD_GetObjListPage(ol);
	top = OSD_GetObjListTop(ol);
	
	for (i = 0; i < page; i++)
	{
		item = (CONTAINER*)lcl_item_list_ListField[i];

		chan_idx = top + i;

		if (chan_idx < cnt)
			valid_idx = 1;
		else
			valid_idx = 0;

		if (valid_idx)
			get_prog_at(chan_idx, &p_node);

		/*IDX */
		txt = (TEXT_FIELD*)OSD_GetContainerNextObj(item);
		if (valid_idx)
			OSD_SetTextFieldContent(txt, STRING_NUMBER, chan_idx + 1);
		else
			OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)"");

		/*Name*/
		txt = (TEXT_FIELD*)OSD_GetObjpNext(txt);
		if (valid_idx)
		{
			if (p_node.ca_mode)
				ComAscStr2Uni("$", unistr);
			ComUniStrCopyChar((UINT8*) &unistr[p_node.ca_mode], p_node.service_name);
			OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)unistr);
		}
		else
			OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)"");
	}
}

static UINT16 lcl_prg_list_load(void)
{
	UINT8 group_type, group_pos;
	UINT16 channel, ch_cnt = 0, page, top;
	UINT8 av_flag;
	POBJLIST ol = &lcl_item_list;

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
#if (CHAN_NORMAL_PLAY_TYPE == 0)
	OSD_SetObjListSingleSelect(ol, channel);
#else
	OSD_SetObjListSingleSelect(ol, INVALID_POS_NUM);
#endif

	return ch_cnt;
}

static VACTION lcl_list_item_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		default:
			act = VACT_PASS;
	}

	return act;
}

static PRESULT lcl_list_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;

	return ret;
}

