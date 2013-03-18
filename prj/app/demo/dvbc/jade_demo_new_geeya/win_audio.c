#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_as.h>

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
#if (CAS_TYPE == CAS_IRDETO)
#include <api/libcas/irca/irca_sys.h>
#endif
#define DEB_PRINT	soc_printf

//include the header from xform 
#include "win_audio_obj.h"

/*******************************************************************************
 *	Local vriable & function declare
 *******************************************************************************/
static void win_audio_track_display(void);
static void win_audio_track_change_mode(void);

/*******************************************************************************
*	Callback and Keymap Functions
*******************************************************************************/
static VACTION audio_track_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_EXIT:
		case V_KEY_MENU:
		case V_KEY_AUDIO:
			BackToFullScrPlay();
			//act = VACT_CLOSE;
			break;
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
		default:
			act = VACT_PASS;
	}

	return act;

}
static PRESULT audio_track_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;

	switch (event)
	{
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;//可消除闪屏现象---#JingJin
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			if(unact == VACT_ENTER)
			{
				win_audio_track_change_mode();	
				BackToFullScrPlay();
				//ret = PROC_LEAVE;
			}
			break;
	}
	return ret;
}
static VACTION audio_track_con_keymap(POBJECT_HEAD pObj, UINT32 key)
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
static PRESULT audio_track_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	VACTION unact;

	switch (event)
	{
		case EVN_PRE_DRAW:
			win_audio_track_display();
			break;
	}
	return ret;
}

static VACTION audio_track_multisel_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;
	switch(key)
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

static void win_audio_track_display()
{
	P_NODE p_node;
	INT32 audio_ch;
	UINT16 channel;
	PMULTISEL cur_multisel_mode = &cur_audio_mode;
	
	channel = sys_data_get_cur_group_cur_mode_channel();
	
	get_prog_at(channel, &p_node);
	audio_ch = p_node.audio_channel;
	
	OSD_SetMultiselSel(cur_multisel_mode, audio_ch);
	OSD_DrawObject((POBJECT_HEAD)cur_multisel_mode, C_UPDATE_ALL);
}

static void win_audio_track_change_mode()
{
	PMULTISEL cur_multisel_mode = &cur_audio_mode;
	P_NODE p_node;
	INT16 cur_audio_ch;
	UINT16 channel;
	
	cur_audio_ch = OSD_GetMultiselSel(cur_multisel_mode);
	
	channel = sys_data_get_cur_group_cur_mode_channel();
	
	get_prog_at(channel, &p_node);
	
	p_node.audio_channel = cur_audio_ch;
	modify_prog(p_node.prog_id, &p_node);
	update_data();
}

