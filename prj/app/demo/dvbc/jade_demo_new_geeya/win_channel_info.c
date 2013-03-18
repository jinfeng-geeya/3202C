#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

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

#include "win_channel_info_obj.h"

/*rectangle parameters & style define for channel_info_menu*/
#define CON_MENU_IDX   	WSTL_WIN_1
#define CON_MENU_L      (TV_OFFSET_L+66)
#define CON_MENU_T      (TV_OFFSET_T+102)
#define CON_MENU_W		604
#define CON_MENU_H      220

/*rectangle parameters & style define for TEXTS*/
#define TXT_TITLE_IDX		WSTL_TXT_4
#define TXT_ITEM_IDX		WSTL_TXT_3

#define TXT_NEME_L			(CON_MENU_L+20)
#define TXT_NEME_W			120		
#define TXT_VALUE_L			(CON_MENU_L+TXT_NEME_W)	
#define TXT_VALUE_W			120	
#define TXT_VERTICAL_GAP	10
#define TXT_HORIZONTAL_GAP	210

#define TXT_T				(CON_MENU_T)
#define TXT_H				30

struct help_item_resource win_CI_help[] =
{
   	 	{0,RS_MENU,RS_HELP_BACK},
    	{0,RS_HELP_EXIT,RS_HELP_EXIT},

};

UINT16 audio_mode_str_id[] =
{
        RS_AUDIO_MODE_LEFT_CHANNEL,
        RS_AUDIO_MODE_RIGHT_CHANNEL,
        RS_AUDIO_MODE_STEREO,
        RS_AUDIO_MODE_MONO,
};

UINT16 items_name_str_id[] =
{
		RS_INFO_PID_VIDEO_PID,
		RS_INFO_PID_AUDIO_PID,
		RS_INFO_PID_PCR_PID,
		RS_INFO_PID_TEXT_PID,
		RS_SEARCH_COD_FREQ,
		RS_SEARCH_COD_SYMBOL,
		RS_SEARCH_COD_COMSEL,

};

UINT16 *items_value_str_id[] =
{
	display_strs[41],  //video pid
	display_strs[42],  //audio pid
	display_strs[43],  //freq
	display_strs[44],  //constellation
	display_strs[45],  //pcr pid
	display_strs[46],  //text pid
	display_strs[47],  //symbol

};
UINT32 constellation_str[] =
{
		16,
		32,
		64,
		128,
		256,
};
/*******************************************************************************
 *	HELPER FUNCTIONS DECLARATION
 *******************************************************************************/
static PRESULT channel_info_message_proc(UINT32 msg_type, UINT32 msg_code);
static void win_channel_info_load_data();
static void win_draw_text_content(OSD_RECT *pRect, char *str, UINT16 *unistr, UINT32 strtype, UINT8 shidx);

/*******************************************************************************
 *	GENERAL FUNCTIONS DEFINE
 *******************************************************************************/
static VACTION win_channel_info_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		case V_KEY_EXIT:
			win_signal_close();
			BackToFullScrPlay();
			break;
		default:
			break;
	}
	return act;
}

static PRESULT win_channel_info_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT psult = PROC_PASS;

	switch (event)
	{
		case EVN_PRE_OPEN:
			wincom_open_title_ext(RS_CHANNEL_INFO, IM_TITLE_ICON_SYSTEM);
			wincom_open_help(win_CI_help, 2);
			win_clear_trash(display_strs[40],8);
			break;
		case EVN_POST_OPEN:
			win_signal_open(pObj);
			win_channel_info_load_data();
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			win_signal_close();
			break;
		case EVN_MSG_GOT:
			channel_info_message_proc(param1, param2);
			break;
	}
	return psult;
}


/*******************************************************************************
 *	HELPER FUNCTIONS DEFINE
 *******************************************************************************/

static PRESULT channel_info_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;

	switch (msg_type)
	{
		case CTRL_MSG_SUBTYPE_STATUS_SIGNAL:
			win_signal_update();
			break;
		case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
			if (is_time_inited())
			{
				wincom_draw_title_time();
			}
            break;
	}

	return ret;

}


static void win_channel_info_load_data()
{
	UINT8 group_type, av_mode, styleidx;
	UINT16 cur_channel;
	P_NODE p_node;
	T_NODE tp;
	UINT32 ret;
	char str[MAX_SERVICE_NAME_LENGTH+5]={0};
	char prog_name[MAX_SERVICE_NAME_LENGTH + 1];
	UINT16 video_pid, audio_pid, pcr_pid, ttx_id, sub_title_id;
	UINT32 audio_pid_idx;
	UINT32 strlen;

	UINT32 freq, symbol;
	UINT8 modulation;
	UINT32 id = 0;

	struct nim_device *nim_dev = (struct nim_device*)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);

	TEXT_FIELD *txt = NULL;
	OSD_RECT rect;


	/*get the info of the current video */
	av_mode = sys_data_get_cur_chan_mode();
	cur_channel = sys_data_get_cur_group_cur_mode_channel();
	ret = get_prog_at(cur_channel, &p_node);
	if(SUCCESS == ret)
	{
		/*draw channel name and index */
		sprintf(str, "%04d", cur_channel + 1);

		rect.uLeft = TXT_NEME_L;
		rect.uTop = TXT_T;
		rect.uWidth = 80;
		rect.uHeight = 30;
		styleidx = TXT_TITLE_IDX;
		win_draw_text_content(&rect, str, NULL, 0, styleidx);

		rect.uLeft = TXT_NEME_L + 80;
		rect.uWidth = 200;

		if (p_node.ca_mode == 0)
			STRCPY(str, "");
		else
			STRCPY(str, "$");

		ComAscStr2Uni(str, len_display_str);
		strlen = ComUniStrLen(len_display_str);
		ComUniStrCopyChar((UINT8*) &len_display_str[strlen], p_node.service_name);
		win_draw_text_content(&rect, NULL, len_display_str, 0, styleidx);

		rect.uLeft = (TV_OFFSET_L + 536);
		rect.uTop = TXT_T;
		rect.uWidth = 150;
		win_draw_text_content(&rect, NULL, NULL, (UINT32)audio_mode_str_id[p_node.audio_channel], styleidx);

		/*fill velues for each display_strs[]*/
		sprintf((UINT8 *)items_value_str_id[id++], "%04d", p_node.video_pid);
		if (p_node.audio_select)
			sprintf((UINT8 *)items_value_str_id[id++], "%04d", p_node.audio_pid[p_node.cur_audio] & 0x1fff);
		else
			sprintf((UINT8 *)items_value_str_id[id++], "%04d", p_node.audio_pid[0] & 0x1fff);			
		sprintf((UINT8 *)items_value_str_id[id++], "%04d", p_node.pcr_pid);
		sprintf((UINT8 *)items_value_str_id[id++], "%04d", p_node.teletext_pid);

		if (SUCCESS == get_tp_by_id((UINT16)p_node.tp_id, &tp))
		{
			sprintf((UINT8 *)items_value_str_id[id++], "%03d.%02dMHz", tp.frq / 100, tp.frq % 100);
			sprintf((UINT8 *)items_value_str_id[id++], "%04dKbaud", tp.sym);
			sprintf((UINT8 *)items_value_str_id[id++], "%dQAM", constellation_str[tp.FEC_inner - 4]);
		}
	}
	else
	{
		sprintf((UINT8 *)items_value_str_id[id++], "%04d", 0);
		sprintf((UINT8 *)items_value_str_id[id++], "%04d", 0);
		sprintf((UINT8 *)items_value_str_id[id++], "%04d", 0);
		sprintf((UINT8 *)items_value_str_id[id++], "%04d", 0);	
		sprintf((UINT8 *)items_value_str_id[id++], "000.00MHz");
		sprintf((UINT8 *)items_value_str_id[id++], "0000Kbaud");
		sprintf((UINT8 *)items_value_str_id[id++], "00QAM");
	}



	/*now we can show them all*/
	styleidx = TXT_ITEM_IDX;
	rect.uWidth = TXT_NEME_W;
	id = 0;
	for (id = 0; id < 4; id++)
	{
		rect.uLeft = TXT_NEME_L;
		rect.uTop = TXT_T + (id + 1)*(TXT_H + TXT_VERTICAL_GAP);
		win_draw_text_content(&rect, NULL, NULL, (UINT32)items_name_str_id[id], styleidx);

		rect.uLeft += TXT_NEME_W;
		win_draw_text_content(&rect, (char*)items_value_str_id[id], NULL, 0, styleidx);
	}
	/*draw value */
	id = 0;
	for (; id < 3; id++)
	{
		rect.uLeft = TXT_NEME_L + TXT_NEME_W + TXT_HORIZONTAL_GAP;
		rect.uTop = TXT_T + (id + 1)*(TXT_H + TXT_VERTICAL_GAP);
		win_draw_text_content(&rect, NULL, NULL, (UINT32)items_name_str_id[id + 4], styleidx);

		rect.uLeft += TXT_NEME_W;
		win_draw_text_content(&rect, (char*)items_value_str_id[id + 4], NULL, 0, styleidx);

	}
	return ;

}


static void win_draw_text_content(OSD_RECT *pRect, char *str, UINT16 *unistr, UINT32 strtype, UINT8 shidx)
{
	TEXT_FIELD *txt;

	txt = &txt_items_name;
	OSD_SetRect2(&txt->head.frame, pRect);
	if (NULL == str && NULL == unistr)
	{
		txt->pString = NULL;
		OSD_SetTextFieldContent(txt, STRING_ID, (UINT32)strtype);
	}
	else if (str != NULL)
	{
		txt->pString = display_strs[40];
		OSD_SetTextFieldContent(txt, STRING_ANSI, (UINT32)str);
	}
	else if (unistr != NULL)
	{
		txt->pString = display_strs[40];
		OSD_SetTextFieldContent(txt, STRING_UNICODE, (UINT32)unistr);
	}
	OSD_SetColor(txt, shidx, shidx, shidx, shidx);
	OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
}


