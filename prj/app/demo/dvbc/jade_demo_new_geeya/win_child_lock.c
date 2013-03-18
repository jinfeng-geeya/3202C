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
#include "win_child_lock_vega.h"

extern UINT8 new_channel_change_flag;

POBJECT_HEAD windows_childlock_list[3] = 
{
	(POBJECT_HEAD)&g_win_childlock_list,
	(POBJECT_HEAD)&g_win_childlock_list,
	(POBJECT_HEAD)&g_win_pwd,
};

#define TV_ID 1
#define RD_ID 2
#define POPUP_MSG_LEFT	400
#define POPUP_MSG_TOP 210
#define POPUP_MSG_WIDTH	150
#define POPUP_MSG_HIGH	50
/*******************************************************************************
*	Callback and Keymap Functions
*******************************************************************************/
static VACTION childlock_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	
	switch (key)
	{
		case V_KEY_EXIT:
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		default:
			act = VACT_PASS;
	}

	return act;

}

static PRESULT childlock_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;

	switch (event)
	{
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		default:
			break;
	}
	return ret;
}

static VACTION childlock_con_item_keymap(POBJECT_HEAD pObj, UINT32 key)
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
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
		default:
			act = VACT_PASS;
	}

	return act;

}
static PRESULT childlock_con_item_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	UINT8 av_mode;
	UINT8 id = 0;
	UINT8 back_saved;
	switch (event)
	{
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			if(unact == VACT_ENTER)
			{
				id = OSD_GetFocusID(pObj);
				av_mode = sys_data_get_cur_chan_mode();
				new_channel_change_flag = 0;
				if(id == TV_ID)
				{
					if (TV_CHAN != av_mode)
						recreate_prog_view(VIEW_ALL | TV_CHAN, 0);		
                        
					sys_data_set_cur_chan_mode(TV_CHAN);
					if (get_prog_num(VIEW_ALL | TV_CHAN, 0) > 0)
					{	
						if(TV_CHAN != av_mode)
						{
							sys_data_set_cur_chan_mode(TV_CHAN);
							sys_data_change_group(0);
						}
					}
					else
					{
						win_compopup_init(WIN_POPUP_TYPE_SMSG);
						win_compopup_set_msg(NULL, NULL, RS_MSG_NO_PROGRAM_TV);
						win_compopup_set_frame(POPUP_MSG_LEFT, POPUP_MSG_TOP, POPUP_MSG_WIDTH, POPUP_MSG_HIGH);
						win_compopup_open_ext(&back_saved);
						osal_task_sleep(1000);
						win_compopup_smsg_restoreback();
						key_pan_display("noCH", 4);
						ret = PROC_LOOP;
						return ret;
					}
				}
				else if(id == RD_ID)
				{
					if (RADIO_CHAN != av_mode)
						recreate_prog_view(VIEW_ALL | RADIO_CHAN, 0);
					new_channel_change_flag = 1;						
					sys_data_set_cur_chan_mode(RADIO_CHAN); // Add code by xc.liu
					if(get_prog_num(VIEW_ALL | RADIO_CHAN, 0) > 0)
					{

						if (RADIO_CHAN != av_mode)
						{
							sys_data_set_cur_chan_mode(RADIO_CHAN);
							sys_data_change_group(0);
						}
					}
					else
					{
						win_compopup_init(WIN_POPUP_TYPE_SMSG);
						win_compopup_set_msg(NULL, NULL, RS_MSG_NO_RADIO_CHANNEL);
						win_compopup_set_frame(POPUP_MSG_LEFT, POPUP_MSG_TOP, POPUP_MSG_WIDTH, POPUP_MSG_HIGH);
						win_compopup_open_ext(&back_saved);
						osal_task_sleep(1000);
						win_compopup_smsg_restoreback();
						key_pan_display("noCH", 4);
						ret = PROC_LOOP;
						return ret;								
					}
				}
				else{;}
				if(NULL != windows_childlock_list[id - 1])
				{
					if(OSD_ObjOpen(windows_childlock_list[id - 1], 0xFFFFFFFF) != PROC_LEAVE)
						menu_stack_push(windows_childlock_list[id - 1]);
				}
			}
			break;
		default:
			break;
	}
	return ret;
}


