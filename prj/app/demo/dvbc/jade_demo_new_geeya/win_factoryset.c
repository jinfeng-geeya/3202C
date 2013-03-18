#include <sys_config.h>

#include <types.h>
#include <basic_types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/osd/osddrv_dev.h>
//#include <api/libge/osd_lib.h>
#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"

#include "control.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include <api/libstbinfo/stb_info_data.h>

//include the header from xform 
#include "win_factoryset_vega.h"

static void reset_factory_setting();

/*******************************************************************************
*	Callback and Keymap Functions
*******************************************************************************/
static VACTION factoryset_keymap(POBJECT_HEAD pObj, UINT32 key)
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

static PRESULT factoryset_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;

	switch (event)
	{
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;//可消除闪屏现象---#JingJin
			break;
	}

	return ret;
}

static VACTION factoryset_con_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
		default:
			act = VACT_PASS;
	}

	return act;

}
static PRESULT factoryset_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;
	MULTISEL *m;
	m = &factory_state;
	INT16 state_value = OSD_GetMultiselSel(m);
	switch (event)
	{
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			if(unact == VACT_ENTER)
			{
				if(state_value == 0)
				{
					ret = PROC_LEAVE;
				}
				else if(state_value == 1)
				{
					reset_factory_setting();
					#ifdef NOCHANNEL_AUTOSEARCH
				    		ForceToScrnDonothing();
                    			ret = PROC_PASS;               
				   		nochannel_startScan(0,0);
					#endif
				}
				else{;}
			}
			break;
	}
	return ret;
}

static VACTION factoryset_multisel_keymap(POBJECT_HEAD pObj, UINT32 key)
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
static void reset_factory_setting()
{
	UINT8 bID, back_saved;
	win_popup_choice_t choice;
	UINT32 chunk_id, default_db_addr, default_db_len;
	
	CONTAINER *factory_msg_con;
	factory_msg_con = &fac_msg_con;
	TEXT_FIELD *factory_msg_txt;
	factory_msg_txt = &factoryset_msg;

	MULTISEL *factory_multisel;
	factory_multisel = &factory_state;
	api_stop_play(1);
	epg_reset();
	
	OSD_SetTextFieldContent(factory_msg_txt, STRING_ID, (UINT32)RS_FACTORY_RESTORING);
	OSD_DrawObject((POBJECT_HEAD)factory_msg_txt, C_UPDATE_ALL);
	OSD_DrawObject((POBJECT_HEAD)factory_msg_con, C_UPDATE_ALL);
	//老版本弹出恢复出厂信息
	//win_compopup_init(WIN_POPUP_TYPE_SMSG);
	//win_compopup_set_msg(NULL, NULL, RS_FACTORY_RESTORING);
	//win_compopup_open_ext(&back_saved);	
	
	sys_data_factroy_init();
	sys_data_set_factory_reset(TRUE);
	sys_data_set_cur_chan_mode(TV_CHAN);
	sys_data_save(TRUE);
	sys_data_load();

	chunk_id = STB_CHUNK_DEFAULT_DB_ID;
	api_get_chuck_addlen(chunk_id, &default_db_addr, &default_db_len);
	set_default_value(DEFAULT_ALL_PROG, default_db_addr + 16);
    
	reset_search_param(); /*reset the local static param in full_band_search*/
	if (GetMuteState())
		SetMuteOnOff(FALSE);
	if (GetPauseState())
		SetPauseOnOff(FALSE);
#if(defined(MIS_AD) || defined(MIS_AD_NEW))
	MIS_SetOSDAlpha(0x00);
#endif
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_DVN)
	ap_cas_set_messageID(0);
	cas_flags_reset();
#elif(CAS_TYPE==CAS_IRDETO)
	setFirstIn(1);
#elif (CAS_TYPE == CAS_CDCA)
	api_mcas_stop_transponder();
#endif				
#endif
	setMtxt(0);
	//win_compopup_smsg_restoreback();消除完成恢复出厂设置后出现的小黑框现象---#JingJin
	
	OSD_SetTextFieldContent(factory_msg_txt, STRING_ID, (UINT32)RS_FACTORY_DONE);
	OSD_DrawObject((POBJECT_HEAD)factory_msg_txt, C_UPDATE_ALL);
	
	OSD_SetTextFieldContent(factory_msg_txt, STRING_ID, (UINT32)RS_FACTORY_USER_SURE);

	OSD_SetMultiselSel(factory_multisel, 0);
	OSD_TrackObject((POBJECT_HEAD)factory_multisel, C_DRAW_TYPE_HIGHLIGHT);
	
	//老版本弹出恢复出厂信息
	//win_compopup_init(WIN_POPUP_TYPE_SMSG);
	//win_compopup_set_msg(NULL, NULL, RS_FACTORY_DONE);
	//win_compopup_open_ext(&back_saved);	
}
