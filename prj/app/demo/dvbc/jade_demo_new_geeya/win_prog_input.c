#include <sys_config.h>
#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/decv/vdec_driver.h>
#include <api/libpub/lib_hde.h>
#include <api/libpub/lib_pub.h>


//#include <api/libosd/osd_lib.h>

#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"
#include "win_com.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

#include "win_prog_input_obj.h"

/*******************************************************************************
 *	Objects definition
 *******************************************************************************/
extern BITMAP pro_input_bmap1;
extern BITMAP pro_input_bmap3;
extern BITMAP pro_input_bmap2;
 
//extern TEXT_FIELD g_win_proginput;
#if 0
#define WIN_SH_IDX	WSTL_INFO_CHINPUT_01

#define W_L  (TV_OFFSET_L + 50)
#define W_T  (TV_OFFSET_T + 40)
#define W_W  120
#define W_H  40

static VACTION win_proginput_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT win_proginput_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

DEF_TEXTFIELD(g_win_proginput, NULL, NULL, C_ATTR_ACTIVE, 2,  \
	0, 0, 0, 0, 0, W_L, W_T, W_W, W_H, WIN_SH_IDX, WIN_SH_IDX, WIN_SH_IDX, WIN_SH_IDX,  \
	win_proginput_keymap, win_proginput_callback,  \
	C_ALIGN_RIGHT | C_ALIGN_VCENTER, 15, 0, 0, display_strs[0])
#endif

/*******************************************************************************
 *	Local vriable & function declare
 *******************************************************************************/

static UINT32 input_ch_number;
static UINT8 input_cancle;
static ID input_ch_timer = OSAL_INVALID_ID;
#define CHANNELINPUT_TIMER_TIME	3000  //ms
#define CHANNELINPUT_TIMER_NAME 	"chi"
UINT16 mm_pro_input_bmp_ids[] =
{IM_DIGIT_NUM0,	IM_DIGIT_NUM1,IM_DIGIT_NUM2,IM_DIGIT_NUM3,IM_DIGIT_NUM4,IM_DIGIT_NUM5,IM_DIGIT_NUM6,IM_DIGIT_NUM7,IM_DIGIT_NUM8,IM_DIGIT_NUM9};

void prog_input_timer_func(UINT unused)
{
	api_stop_timer(&input_ch_timer);
	ap_send_msg(CTRL_MSG_SUBTYPE_CMD_PLAY_PROGRAM, input_ch_number, TRUE);
}

static VACTION win_proginput_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act;
	switch (key)
	{
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
			act = key - V_KEY_0 + VACT_NUM_0;
			break;
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
		case V_KEY_EXIT:
		case V_KEY_MENU:
			input_cancle = 1;
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
	}

	return act;
}

static PRESULT win_proginput_unkown_act_proc(VACTION act);

static PRESULT win_proginput_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT16 ch_idx;
	UINT8 back_saved;
	UINT8 av_mode;

	switch (event)
	{
		case EVN_PRE_DRAW:/*
			if(sys_data_get_cur_chan_mode())
				OSD_SetColor((POBJECT_HEAD)&g_win_proginput, WSTL_INFO_CHINPUT_01, WSTL_INFO_CHINPUT_01, WSTL_INFO_CHINPUT_01, WSTL_INFO_CHINPUT_01);
			else
				OSD_SetColor((POBJECT_HEAD)&g_win_proginput, WSTL_INFO_CHINPUT_02, WSTL_INFO_CHINPUT_02, WSTL_INFO_CHINPUT_02, WSTL_INFO_CHINPUT_02);
               */
			break;
		case EVN_PRE_OPEN:	
			input_ch_number = 0;
			input_cancle = 0;
			break;
		case EVN_POST_OPEN:
			input_ch_timer = api_start_timer(CHANNELINPUT_TIMER_NAME, CHANNELINPUT_TIMER_TIME, prog_input_timer_func);
			break;
		case EVN_PRE_CLOSE:
			api_stop_timer(&input_ch_timer);
			break;
		case EVN_POST_CLOSE:
			if (input_cancle)
				break;
			//if(input_ch_number< get_node_num(TYPE_PROG_NODE, NULL) )
			av_mode = sys_data_get_cur_chan_mode();
			if (input_ch_number <= get_prog_num(VIEW_ALL | av_mode, 0))
			{
				if (input_ch_number != 0)
				{
					ch_idx = input_ch_number - 1;
					ap_clear_all_message();
					api_play_channel(ch_idx, TRUE, TRUE, FALSE);
					#ifdef MULTI_CAS
					#if(CAS_TYPE==CAS_CDCA)
					show_finger_print(0, 0);
					#elif(CAS_TYPE==CAS_DVT)
					ap_cas_fingerprint_proc(0, 1);
					#endif
					#endif
					ap_send_msg(CTRL_MSG_SUBTYPE_CMD_ENTER_ROOT, (UINT32) &g_win_progname, FALSE);
				}
				/*ch_idx = get_prog_num(VIEW_ALL|av_mode, 0) - 1;*/
			}
			else
			{
				win_compopup_init(WIN_POPUP_TYPE_OK);
				win_compopup_set_msg(NULL, NULL, RS_MSG_NO_SUCH_CHANNEL);
				win_compopup_open_ext(&back_saved);
#if ((SUBTITLE_ON == 1)||(TTX_ON == 1)) 
                api_osd_mode_change(OSD_SUBTITLE);
#endif
			}
			break;
		case EVN_UNKNOWNKEY_GOT:
			break;
		case EVN_UNKNOWN_ACTION:
			ret = win_proginput_unkown_act_proc((VACTION)(param1 >> 16));
			
			break;
		case EVN_MSG_GOT:
			if (param1 == CTRL_MSG_SUBTYPE_CMD_PLAY_PROGRAM)
				ret = PROC_LEAVE;
			break;
		default:
			;
	}

	return ret;
}

static PRESULT win_proginput_unkown_act_proc(VACTION act)
{
	PRESULT ret = PROC_LOOP;
	CONTAINER *txt;
	txt = &g_win_proginput;

	api_stop_timer(&input_ch_timer);

	if (act >= VACT_NUM_0 && act <= VACT_NUM_9)
	{
		input_ch_number *= 10;
		input_ch_number += act - VACT_NUM_0;
		if (input_ch_number > 999)
			input_ch_number %= 1000;
		act=input_ch_number%10;   //----yuanlin
	    OSD_SetBitmapContent(&pro_input_bmap3, mm_pro_input_bmp_ids[act]); //---yuanlin
		act=input_ch_number%100/10;
		OSD_SetBitmapContent(&pro_input_bmap2, mm_pro_input_bmp_ids[act]);  //---yuanlin
		act=input_ch_number/100;   
		OSD_SetBitmapContent(&pro_input_bmap1, mm_pro_input_bmp_ids[act]);  //---yuanlin
		//ComInt2UniStr(display_strs[0], (UINT32)input_ch_number, 4);
		OSD_DrawObject((POBJECT_HEAD)txt, C_UPDATE_ALL);
		input_ch_timer = api_start_timer(CHANNELINPUT_TIMER_NAME, CHANNELINPUT_TIMER_TIME, prog_input_timer_func);
	}
	else if (act == VACT_ENTER)
	{
		ret = PROC_LEAVE;
	}

	return ret;

}
