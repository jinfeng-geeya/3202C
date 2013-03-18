#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

//#include <api/libosd/osd_lib.h>
#include <api/libchunk/chunk.h>
#include <api/libstbinfo/stb_info_data.h>

#include "osdobjs_def.h"
#include "string.id"
#include "images.id"
#include "osd_config.h"
#include "win_com.h"
#include "menus_root.h"
#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_signalstatus.h"

#include "win_sys_information_obj.h"
static void win_sysinfo_load_data();

extern TEXT_CONTENT txt_info_item8_text_content[];
UINT16 *sysinfo_value_str[] =
{
	display_strs[40],  //hardware version
	display_strs[41],  //software version
	display_strs[42],  //serial num
	display_strs[43],  //download date
};

/*******************************************************************************
 *	Objects & callback functions declaration
 *******************************************************************************/
 
#define ITERM_COUNT sizeof(sysinfo_value_str)/sizeof(UINT32)

static VACTION win_sysinfo_keymap(POBJECT_HEAD pObj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_MENU:
		case V_KEY_EXIT:
			act = VACT_CLOSE;
			break;
		default:
			break;
	}
	return act;
}

static PRESULT win_sysinfo_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT psult = PROC_PASS;
	MULTI_TEXT *serial_num;
	serial_num = &txt_info_item8;
		char str[100]={0};
	UINT16 uni_str[100]={0}; 
	switch (event)
	{
		case EVN_PRE_OPEN:
			win_clear_trash(display_strs[40],ITERM_COUNT);
			break;
		case EVN_PRE_DRAW:
			win_sysinfo_load_data();
			break;
		case EVN_POST_OPEN:
			//在此获取机顶盒SERIAL，方可显示
			api_sn_get(str); 
			//load serail number
			ComAscStr2Uni(str,uni_str);
			txt_info_item8_text_content[0].text.pString = uni_str;
			OSD_SetMultiTextContent(serial_num, (PTEXT_CONTENT)&txt_info_item8_text_content);		//serail number
			OSD_DrawObject((POBJECT_HEAD)serial_num, C_UPDATE_ALL);
			break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;
		default:
			break;
	}
	return psult;
}

static void win_sysinfo_load_data()
{
	char str[100]={0};
	UINT16 uni_str[100]={0}; 
	UINT32 hw_str[100]={0};
	UINT32 sw_str[100]={0};
	STB_INFO_DATA *stb_data;
	HW_SW_INFO hwswInfo;
	
	TEXT_FIELD *pub_date;
	pub_date = &txt_info_item5;
	TEXT_FIELD *hw_info;
	hw_info = &txt_info_item6;
	TEXT_FIELD *sw_info;
	sw_info = &txt_info_item7;
	MULTI_TEXT *serial_num;
	serial_num = &txt_info_item8;
	
	api_hwsw_Info_get(&hwswInfo);
	//load hw version 
	sprintf(hw_str, "0x%.4x", hwswInfo.hw_version);
	OSD_SetTextFieldContent(hw_info, STRING_ANSI, hw_str);// hw version
	
	//load sw version
	sprintf(sw_str, "0x%.4x", hwswInfo.sw_version);
	OSD_SetTextFieldContent(sw_info, STRING_ANSI, sw_str);// sw version

	/*api_sn_get(str); 
	//load serail number
	ComAscStr2Uni(str,uni_str);
	txt_info_item8_text_content[0].text.pString = uni_str;
	OSD_SetMultiTextContent(serial_num, (PTEXT_CONTENT)&txt_info_item8_text_content);		//serail number*/
	
	//load download date
	stb_data = stb_info_data_get();
	OSD_SetTextFieldContent(pub_date, STRING_ANSI, (UINT32)(stb_data->download_date)); //download data
}

