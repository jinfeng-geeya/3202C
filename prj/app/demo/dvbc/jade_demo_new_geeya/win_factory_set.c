/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2003 Copyright (C)
 *
 *  File: win_upgrade_menu.c
 *
 *  Description:
 *
 *  History:
 *      Date        	Author             	Version     	Comment
 *      ====        	======        	=======   	=======
 *
 ****************************************************************************/
#include <sys_config.h>
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_as.h>
#include <hld/decv/decv.h>

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

#include "win_factory_set_obj.h"
#include <api/libstbinfo/stb_info_data.h>

#define BTN_YES_ID	1
#define BTN_NO_ID	2

/*******************************************************************************
 *	Local functions & variables declare
 *******************************************************************************/

TEXT_CONTENT factoryset_mtxt_content[] =
{
	{
		STRING_ID, RS_MSG_THIS_OPERATION_WILL_ERASE_ALL
        },
};
/*******************************************************************************
 *	Window's keymap, proc and  callback
 *******************************************************************************/

static PRESULT factory_set_message_proc(UINT32 msg_type, UINT32 msg_code)
{
	PRESULT ret = PROC_LOOP;
	switch (msg_type)
	{
		case CTRL_MSG_SUBTYPE_CMD_TIMEDISPLAYUPDATE:
			if (is_time_inited())
			{
				wincom_draw_title_time();
			}
            break;
		default:
			break;
	}
}

static VACTION factoryset_btn_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	if (key == V_KEY_ENTER)
		act = VACT_ENTER;

	return act;
}

static PRESULT factoryset_btn_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	//PRESULT ret = PROC_LEAVE;
	VACTION unact;
	UINT8 bID, back_saved;
	win_popup_choice_t choice;
	UINT32 chunk_id, default_db_addr, default_db_len;

	bID = OSD_GetObjID(pObj);

	switch (event)
	{
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			if (unact == VACT_ENTER)
			{
				ret = PROC_LEAVE;
				if (bID == BTN_NO_ID)
				{
					break;
				}

				//win_compopup_init(WIN_POPUP_TYPE_OKNO);
				//win_compopup_set_msg(NULL, NULL, RS_DISPLAY_SURE_TO_DELETE);
				//win_compopup_set_default_choice(WIN_POP_CHOICE_NO);
				//choice = win_compopup_open_ext(&back_saved);
				//if(choice == WIN_POP_CHOICE_YES)
				//{
					api_stop_play(1);
					epg_reset();
					win_compopup_init(WIN_POPUP_TYPE_SMSG);
					win_compopup_set_msg(NULL, NULL, RS_MSG_SAVING_DATA);
					win_compopup_open_ext(&back_saved);					

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
					//ap_set_signal_status(TRUE);//TFCAS_SIGNAL_STATUS = TRUE
					ap_cas_set_messageID(0);
					cas_flags_reset();
					//ap_cas_message_show();
#elif(CAS_TYPE==CAS_IRDETO)
					setFirstIn(1);
#elif (CAS_TYPE == CAS_CDCA)
			    	//ap_set_detitle_icon_status(0);
			        //ap_set_cas_dispstr(0);
			        api_mcas_stop_transponder();
			       // CDCASTB_FormatBuffer(0);
#endif				
#endif
					setMtxt(0);
					win_compopup_smsg_restoreback();
					// PRESULT ret = PROC_LEAVE;
					//ap_send_msg(CTRL_MSG_SUBTYPE_CMD_EXIT_ROOT, (UINT32)&g_win_PALNTSC,FALSE);
				//}
#ifdef NOCHANNEL_AUTOSEARCH
				    ForceToScrnDonothing();
                    ret = PROC_PASS;               
				    nochannel_startScan(0,0);
#endif

			}
			break;
	}

	return ret;
}

static VACTION factoryset_keymap(POBJECT_HEAD pObj, UINT32 vkey)
{
	VACTION act = VACT_PASS;
	switch (vkey)
	{
		case V_KEY_LEFT:
			act = VACT_CURSOR_LEFT;
			break;
		case V_KEY_RIGHT:
			act = VACT_CURSOR_RIGHT;
			break;
		case V_KEY_MENU:
			act = VACT_CLOSE;
			break;
		case V_KEY_EXIT:		
			BackToFullScrPlay();			
	}

	return act;
}

struct help_item_resource factory_set_help[] =
{
    {0,RS_MENU,RS_HELP_BACK},
    {0,RS_HELP_EXIT,RS_HELP_EXIT},
};

static PRESULT factoryset_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	CONTAINER *cont = (CONTAINER*)pObj;
	UINT8 id = OSD_GetObjID(pObj);

	switch (event)
	{
		case EVN_PRE_OPEN:
			OSD_SetContainerFocus(cont, BTN_NO_ID);
			//OSD_SetContainerFocus(cont, 2);
			wincom_open_title_ext(RS_RESTORE_DEFAULT, IM_TITLE_ICON_SYSTEM);
			wincom_open_help(factory_set_help, 2);

			break;

		case EVN_POST_OPEN:
			break;

		case EVN_PRE_CLOSE:
			/* Make OSD not flickering */
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;

			break;

		case EVN_POST_CLOSE:
			break;
		case EVN_MSG_GOT:
			ret = factory_set_message_proc( param1, param2 );
			break;
	}
	return ret;
}
