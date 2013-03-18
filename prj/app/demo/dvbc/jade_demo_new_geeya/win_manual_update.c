#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/nim/nim.h>
//#include <api/libosd/osd_lib.h>
#include "osdobjs_def.h"

#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "osd_rsc.h"

#include "copper_common/system_data.h"
#include "copper_common/menu_api.h"
//#include "win_com_menu_define.h"
#include "win_com.h"
#include "win_signal.h"

#include "win_manual_update_vega.h"

#define DVBC_QUICKSCAN_EDIT_LEN  0x06 // add .xx
//this values is restore the values setting
static UINT32 i_qs_freq = 0; //0x74CC;
static UINT32 i_qs_symbol = 0; //6875;
static UINT8 i_qs_mode = 0; //QAM64;
static UINT16 i_qs_areacode = 0;
/*******************************************************************************
*	local variable define
*******************************************************************************/
static void win_upgrade_save_data();
static void win_upgrade_load_data();
#define VACT_SET_DECREASE	(VACT_PASS + 1)
#define VACT_SET_INCREASE	(VACT_PASS + 2)
static void dvbc_quickscan_set_def_val(void);
static void win_manual_upgrade_update_param(BOOL save);
static PRESULT main_req_message_proc(UINT32 msg_type, UINT32 msg_code);
static UINT8 qs_check_value();
static INT32 check_freq_value(UINT16 *str);
static INT32 check_symbol_value(UINT16 *str);
static INT32 check_upgrade_pid_value(UINT16 *str);

typedef struct tp_info
{
	UINT32 frequency;
	UINT16 symbol_rate;
	UINT16 modulation;
	UINT32 pid;
} UPGRADE_PARAM;
void manual_upgrade_get_param(UINT32 *fre,UINT16 *sysbol,UINT16 *mode,UINT32 *pid );
void manual_upgrade_set_param(UPGRADE_PARAM *upgrade_param_temp);
UINT16 manual_upgrade_get_param_pid(void);
UPGRADE_PARAM upgrade_info_param;
/*******************************************************************************
 *	helper functions define
 *******************************************************************************/

static VACTION win_common_item_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		case V_KEY_EXIT:
			dm_set_onoff(1);
			win_manual_upgrade_update_param(1);
			//win_upgrade_save_data();
			sys_data_save(1);
			act = VACT_CLOSE;
			break;
		default:
			break;
	}

	return act;
}

static PRESULT win_main_freq_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	SYSTEM_DATA *pSysData = NULL;

	pSysData = sys_data_get();

	switch (event)
	{
		case EVN_PRE_OPEN:
			MEMSET(&upgrade_info_param,0,sizeof(UPGRADE_PARAM));
			win_upgrade_load_data();//read data from system_data
			dvbc_quickscan_set_def_val();
			dm_set_onoff(0);
			break;
		case EVN_POST_OPEN:
			//win_manual_upgrade_update_param(1);
			//win_signal_open(pObj);
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		case EVN_POST_CLOSE:
			dm_set_onoff(1);
			//win_signal_close();
			win_manual_upgrade_update_param(1);
			//win_upgrade_save_data();
			sys_data_save(1);
			break;

	}

	return ret;
}


static VACTION con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_LEFT:
			act = VACT_SET_DECREASE;
			break;
		case V_KEY_RIGHT:
			act = VACT_SET_INCREASE;
			break;
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
			break;
	}
	return act;
}


static PRESULT con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{

	PRESULT ret = PROC_PASS;
	VACTION act;
	//UINT8 id= OSD_GetFocusID(pObj);


	switch (event)
	{
		case EVN_ITEM_POST_CHANGE:
			if (1 == qs_check_value())
			//win_manual_upgrade_update_param(1);
				break;
		case EVN_UNKNOWN_ACTION:
			act = (VACTION)(param1 >> 16);
			if (act == VACT_ENTER)
			{
				if (1 == qs_check_value())
				{
					win_manual_upgrade_update_param(1);
					manual_upgrade_set_param(&upgrade_info_param);
					ap_send_msg(CTRL_MSG_SUBTYPE_STATUS_UPGRADE_MANUAL, 0, FALSE);
				}
				ret = PROC_LOOP;
			}
			break;
		default:
			break;
	}
	return ret;
}


static VACTION main_freq_item_edit_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION Action = VACT_PASS;

	switch (key)
	{
		case V_KEY_LEFT:
			Action = VACT_EDIT_LEFT;
			break;
		case V_KEY_RIGHT:
			Action = VACT_EDIT_RIGHT;
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
			Action = key - V_KEY_0 + VACT_NUM_0;
			break;
		default:
			break;
	}

	return Action;
}

static VACTION main_freq_item_sel_keymap(POBJECT_HEAD pObj, UINT32 key)
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
		case V_KEY_ENTER:
			break;
		default:
			act = VACT_PASS;
			break;
	}

	return act;
}

static PRESULT main_freq_item_sel_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 bID;
	VACTION unact;

	//bID = OSD_GetObjID(pObj);

	switch (event)
	{
		case EVN_POST_CHANGE:
			//if ((bID == MODE_ID))
			//win_manual_upgrade_update_param(1);
			break;
		default:
			break;

	}


	return ret;
}

static void dvbc_quickscan_set_def_val(void)
{
	INT8 rule[20];
	UINT32 freq, symbol, constellation;
	P_NODE p_node;
	T_NODE t_node;
	UINT32 prog_num;
	UINT16 cur_channel;

	/* FREQ */
	wincom_i_to_mbs_with_dot(display_strs[20],
	                         i_qs_freq, DVBC_QUICKSCAN_EDIT_LEN - 1/*without dot*/, 0x02/*xxx.xx*/);
	ComAscStr2Uni("MHz", display_strs[30]);
	/* SYMBOL */
	wincom_i_to_mbs(display_strs[21],
	                i_qs_symbol, DVBC_QUICKSCAN_EDIT_LEN - 1);
	ComAscStr2Uni("KBaud", display_strs[31]);

	OSD_SetMultiselSel(&txt_mu_item_value3, i_qs_mode - QAM16);
	wincom_i_to_mbs(display_strs[22],
	                i_qs_areacode, 4);
}


// 0 not save, 1 save the parameter
static void win_manual_upgrade_update_param(BOOL save)
{
	UINT8 dot_pos;
	UINT32 freq, symbol, constellation, areacode;

	//freq
	wincom_mbs_to_i_with_dot(display_strs[20], &freq, &dot_pos);
	// SYMBOL
	symbol = wincom_mbs_to_i(display_strs[21]);
	// scan mode
	constellation = QAM16 + OSD_GetMultiselSel(&txt_mu_item_value3);

	//areacode = wincom_mbs_to_i(display_strs[22]);
	 wincom_mbs_to_i_with_dot(display_strs[22],&areacode,&dot_pos);
	//change channel
	//UIChChgNimcc(freq, symbol, constellation);

	if (1 == save)
	{
		i_qs_freq = freq;
		i_qs_symbol = symbol;
		i_qs_mode = constellation;
		i_qs_areacode = areacode;
	}
}

// 1 valid 0 invalid
static UINT8 qs_check_value()
{
	UINT32 ret1, ret2;
	UINT8 save;

	ret1 = check_freq_value(display_strs[20]);
	ret2 = check_symbol_value(display_strs[21]);
	if (ret1 != 0 || ret2 != 0)
	{
		win_popup_msg(NULL, NULL, RS_MSG_INVALID_INPUT_CONTINUE);
		return 0;
	}
	else
		return 1;
}


static INT32 check_freq_value(UINT16 *str)
{
	INT32 ret = 0;
	UINT32 val;
	UINT8 dot_pos;

	/* FREQ */
	wincom_mbs_to_i_with_dot(str, &val, &dot_pos);
	if (val < 4825 || val > 85875)
		ret =  - 1;

	return ret;
}

static INT32 check_symbol_value(UINT16 *str)
{
	INT32 ret = 0;
	UINT32 val;

	/* SYMBOL */
	val = wincom_mbs_to_i(str);
	if (val < 1000 || val > 7000)
		ret =  - 1;


	return ret;
}

static INT32 check_upgrade_pid_value(UINT16 *str)
{
	INT32 ret = 0;
	UINT32 val;
	val = wincom_mbs_to_i(str);
	if (val < 1000 || val > 7000)
		ret =  - 1;


	return ret;
}
static void win_upgrade_save_data()/////this function is not used,Be care !!!! 
{
	SYSTEM_DATA *sys_data = &system_config;

	sys_data->main_frequency = i_qs_freq;
	sys_data->main_symbol = i_qs_symbol;
	sys_data->main_qam = i_qs_mode;
	//sys_data->ad_data.mis_areacode = i_qs_areacode;
}

static void win_upgrade_load_data()
{
	SYSTEM_DATA *sys_data = &system_config;

	i_qs_freq = sys_data->main_frequency;
	i_qs_symbol = sys_data->main_symbol;
	i_qs_mode = sys_data->main_qam;
//	i_qs_areacode = sys_data->ad_data.mis_areacode;
}
void manual_upgrade_set_param(UPGRADE_PARAM *upgrade_param_temp)
{
	upgrade_param_temp ->frequency = i_qs_freq;
	upgrade_param_temp ->symbol_rate = i_qs_symbol;
	upgrade_param_temp ->modulation = i_qs_mode;
	upgrade_param_temp ->pid = i_qs_areacode;
	
}
void manual_upgrade_get_param(UINT32 *fre,UINT16 *sysbol,UINT16 *mode,UINT32 *pid )
{
	*fre = upgrade_info_param.frequency;
	*sysbol = upgrade_info_param.symbol_rate;
	*mode = upgrade_info_param.modulation;
	*pid = upgrade_info_param.pid;
}

UINT16 manual_upgrade_get_param_pid(void)
{
	return upgrade_info_param.pid;
}
