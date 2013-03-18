#include <sys_config.h>
#include <sys_parameters.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libchunk/chunk.h>
#include <bus/sci/sci.h>
#include <bus/flash/flash.h>
#include <hal/hal_gpio.h>
#include <hal/hal_mem.h>
#include <hld/pan/pan_dev.h>
#include <hld/pan/pan.h>
#include <api/libpub/lib_device_manage.h>

#include <hld/osd/osddrv.h>
#include <api/libloader/ld_osd_lib.h>
#include <api/libloader/ld_boot_info.h>

#include <api/libstbinfo/stb_info_data.h>
#include <api/libupg/lib_upg4.h>

#include <api/libosd/osd_vkey.h>

#include "dev_handle.h"
#include "upg_common.h"
#include "../copper_common/boot_system_data.h"
#include "../copper_common/stb_data.h"
#include "key.h"
#include "string.id"

#define UPG_MENU_CON_L	(OSD_MAX_WIDTH-UPG_MENU_CON_W)/2
#define UPG_MENU_CON_T	(OSD_MAX_HEIGHT-UPG_MENU_CON_H)/2
#define UPG_MENU_CON_W 350
#define UPG_MENU_CON_H	200

#define UPG_MENU_ITEM_L	(UPG_MENU_CON_L+50)
#define UPG_MENU_ITEM_T	(UPG_MENU_CON_T+40)
#define UPG_MENU_ITEM_W	(UPG_MENU_CON_W-100)
#define UPG_MENU_ITEM_H	40

#define UPG_MENU_ITEM_GAP	30

#define UPG_MENU_CON_SH	WHITE_COLOR
#define UPG_MENU_CON_HL	BLACK_COLOR
#define UPG_MENU_ITEM_SH	GRAY_COLOR
#define UPG_MENU_ITEM_HL	YELLOW_COLOR
#define UPG_MENU_TXT_SH	BLACK_COLOR
#define UPG_MENU_TXT_HL	BLACK_COLOR


#define UPG_USB_ID 1
#define UPG_OTA_ID	2

struct IDNavi upg_menu_navi[]=
{
	{UPG_USB_ID,UPG_OTA_ID,UPG_OTA_ID,0,0},
	{UPG_OTA_ID,UPG_USB_ID,UPG_USB_ID,0,0},
};

#define UPG_ITEM_NUM	(sizeof(upg_menu_navi)/sizeof(upg_menu_navi[0]))

UINT16 upg_menu_ids[]=
{
	RS_UPG_USB,
	RS_UPG_OTA,
};

void upg_menu_pan_display(UINT16 focus_id)
{
	UINT8 dis_str[5];
	
	if(focus_id == UPG_USB_ID)
		STRCPY(dis_str, "sUSB");
	else if(focus_id == UPG_OTA_ID)
		STRCPY(dis_str, "sOTA");
	else
		STRCPY(dis_str, "sUNK");

	stb_pan_display(dis_str, 4);
}

UINT8 upg_menu_proc()
{
	struct OSDRect rect;
	lpVSCR 		pVscr;
	UINT8 *pstr;
	UINT16 i;
	UINT16 back_sh,txt_sh;
	UINT32 new_focus_id=0;
	UINT16 focus_id;

	focus_id = UPG_USB_ID;
		
	upg_menu_pan_display(focus_id);
		
	// draw input frame
	OSD_SetRect(&rect, UPG_MENU_CON_L, UPG_MENU_CON_T, UPG_MENU_CON_W, UPG_MENU_CON_H);	
	OSDDrv_RegionFill((HANDLE)osd_dev,0,&rect,UPG_MENU_CON_SH);

	for(i=0;i<UPG_ITEM_NUM;i++)
	{
		if(focus_id-1 == i)
		{
			back_sh = UPG_MENU_ITEM_HL;
			txt_sh = UPG_MENU_TXT_HL;
		}
		else
		{
			back_sh = UPG_MENU_ITEM_SH;
			txt_sh = UPG_MENU_TXT_SH;
		}

		pstr = (UINT8*)OSD_GetUnicodeString(upg_menu_ids[i]);
		OSD_SetRect(&rect, UPG_MENU_ITEM_L, UPG_MENU_ITEM_T+(UPG_MENU_ITEM_H+UPG_MENU_ITEM_GAP)*i, UPG_MENU_ITEM_W, UPG_MENU_ITEM_H);	
		OSDDrv_RegionFill((HANDLE)osd_dev,0,&rect,back_sh);
		pVscr = OSD_GetVscr(&rect,OSD_GET_BACK);
		OSD_DrawText(&rect,pstr,txt_sh,C_ALIGN_CENTER|C_ALIGN_VCENTER,0,pVscr);

		if(pVscr->lpbScr != NULL)
			pVscr->updatePending = 1;
		OSD_UpdateVscr(pVscr);
		pVscr->lpbScr = NULL;
	}

	UINT32 hkey;
	UINT32 vkey;
	while(1)
	{
		hkey = boot_get_key_msg();
		if(hkey == INVALID_HK || hkey == INVALID_MSG)
			continue;

		ap_hk_to_vk(0,hkey, &vkey);

		if(vkey == V_KEY_ENTER)
		{
			return focus_id;
		}
		else if(vkey == V_KEY_UP || vkey == V_KEY_DOWN)
		{
			if(vkey == V_KEY_UP)
				new_focus_id = upg_menu_navi[focus_id-1].upID;
			else if(vkey == V_KEY_DOWN)
				new_focus_id = upg_menu_navi[focus_id-1].downID;

			if(new_focus_id == 0)
				continue;

			upg_menu_pan_display(new_focus_id);

			/* not highlight old item */
			pstr = (UINT8*)OSD_GetUnicodeString(upg_menu_ids[focus_id-1]);
			OSD_SetRect(&rect, UPG_MENU_ITEM_L, UPG_MENU_ITEM_T+(UPG_MENU_ITEM_H+UPG_MENU_ITEM_GAP)*(focus_id-1), UPG_MENU_ITEM_W, UPG_MENU_ITEM_H);	
			back_sh = UPG_MENU_ITEM_SH;
			txt_sh = UPG_MENU_TXT_SH;
			OSDDrv_RegionFill((HANDLE)osd_dev,0,&rect,back_sh);
			pVscr = OSD_GetVscr(&rect,OSD_GET_BACK);
			OSD_DrawText(&rect,pstr,txt_sh,C_ALIGN_CENTER|C_ALIGN_VCENTER,0,pVscr);

			if(pVscr->lpbScr != NULL)
			pVscr->updatePending = 1;
			OSD_UpdateVscr(pVscr);
			pVscr->lpbScr = NULL;

			/* highlight new item */
			pstr = (UINT8*)OSD_GetUnicodeString(upg_menu_ids[new_focus_id-1]);
			OSD_SetRect(&rect, UPG_MENU_ITEM_L, UPG_MENU_ITEM_T+(UPG_MENU_ITEM_H+UPG_MENU_ITEM_GAP)*(new_focus_id-1), UPG_MENU_ITEM_W, UPG_MENU_ITEM_H);	
			back_sh = UPG_MENU_ITEM_HL;
			txt_sh = UPG_MENU_TXT_HL;
			OSDDrv_RegionFill((HANDLE)osd_dev,0,&rect,back_sh);
			pVscr = OSD_GetVscr(&rect,OSD_GET_BACK);
			OSD_DrawText(&rect,pstr,txt_sh,C_ALIGN_CENTER|C_ALIGN_VCENTER,0,pVscr);

			if(pVscr->lpbScr != NULL)
			pVscr->updatePending = 1;
			OSD_UpdateVscr(pVscr);
			pVscr->lpbScr = NULL;

			focus_id = new_focus_id;
		}
		
		if(pVscr->lpbScr != NULL)
			pVscr->updatePending = 1;
		OSD_UpdateVscr(pVscr);
		pVscr->lpbScr = NULL;
	}
	return 0;
}


