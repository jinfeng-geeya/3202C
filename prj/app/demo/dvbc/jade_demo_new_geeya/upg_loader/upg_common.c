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

#include "dev_handle.h"
#include "../copper_common/boot_system_data.h"
#include "../copper_common/stb_data.h"
#include "string.id"
#include "../copper_common/stb_hwinfo.h"

BOOL g_upg_preburn_flag = FALSE;

UINT8 upg_stauts=0;	// upg_stauts: 0:download; 1:burn

#define OTA_CON_L	(OSD_MAX_WIDTH-OTA_CON_W)/2
#define OTA_CON_T	(OSD_MAX_HEIGHT-OTA_CON_H)/2
#define OTA_CON_W	400
#define OTA_CON_H	200

#define OTA_TXT_L	(OTA_CON_L+30)
#define OTA_TXT_T	(OTA_CON_T+30)
#define OTA_TXT_W	(OTA_CON_W-30)
#define OTA_TXT_H	30//26

#define OTA_TXT_V_GAP 10

#define OTA_BAR_L	(OTA_CON_L+30)
#define OTA_BAR_T	(OTA_CON_T+120)
#define OTA_BAR_W	290
#define OTA_BAR_H	26

#define OTA_PROGRESS_L	(OTA_BAR_L+OTA_BAR_W+10)
#define OTA_PROGRESS_T	(OTA_BAR_T)
#define OTA_PROGRESS_W	(OTA_CON_L+OTA_CON_W-OTA_PROGRESS_L)
#define OTA_PROGRESS_H	30//26
	
#define OTA_CON_SH	WHITE_COLOR
#define OTA_TXT_SH	BLACK_COLOR
#define OTA_BAR_SH	GRAY_COLOR
#define OTA_THUMB_SH	BLUE_COLOR

#ifdef __MM_UPG_TOP_ADDR
/*return value:  -1 fail else success*/
INT8 Char2Hex(UINT8 ch)
{
	INT8 ret =  - 1;
	if ((ch <= 0x39) && (ch >= 0x30))
	// '0'~'9'
		ret = ch &0xf;
	else if ((ch <= 102) && (ch >= 97))
	//'a'~'f'
		ret = ch - 97+10;
	else if ((ch <= 70) && (ch >= 65))
	//'A'~'F'
		ret = ch - 65+10;

	return ret;
}

UINT32 Str2UINT32(UINT8 *str, UINT8 len)
{
	UINT32 ret = 0;
	UINT8 i;
	INT temp;

	if (str == NULL)
		return 0;

	for (i = 0; i < len; i++)
	{
		temp = Char2Hex(*str++);
		if (temp ==  - 1)
			return 0;

		ret = (ret << 4) | temp;
	}

	return ret;
}

void upg_draw_frame()
{
	struct OSDRect rect;
	
	// draw con frame
	OSD_SetRect(&rect, OTA_CON_L, OTA_CON_T, OTA_CON_W, OTA_CON_H);
	OSDDrv_RegionFill((HANDLE)osd_dev,0,&rect,OTA_CON_SH);

}

void upg_info_update(UINT32 line, UINT8 *pstr, UINT16 str_id)
{
	struct OSDRect rect;
	lpVSCR 		pVscr;
	UINT8 *ustr;
	
	// draw info txt
	OSD_SetRect(&rect, OTA_TXT_L, OTA_TXT_T+(OTA_TXT_H+OTA_TXT_V_GAP)*line, OTA_TXT_W, OTA_TXT_H);	
	OSDDrv_RegionFill((HANDLE)osd_dev,0,&rect,OTA_CON_SH);
	
	pVscr = OSD_GetVscr(&rect,OSD_GET_BACK);

	if(pstr==NULL)
		ustr = (UINT8*)OSD_GetUnicodeString(str_id);
	else
		ustr = pstr;
	
	OSD_DrawText(&rect,ustr,OTA_TXT_SH,C_ALIGN_LEFT,0,pVscr);

	if(pVscr->lpbScr != NULL)
		pVscr->updatePending = 1;
	OSD_UpdateVscr(pVscr);
	pVscr->lpbScr = NULL;
}

void upg_progress_update(UINT32 param)
{
	struct OSDRect rect;
	lpVSCR 		pVscr;
	UINT8 str[5];
	UINT16 ustr[5];
	INT32 part_id, progress;

	progress = param&0xFF;
	part_id = param>>24;

	if((upg_stauts != 0) && (part_id == -1))
	{
		return;
	}

	if(progress>100)
		return;//progress = 0;

	// draw process bar back
	OSD_SetRect(&rect, OTA_BAR_L, OTA_BAR_T, OTA_BAR_W, OTA_BAR_H);
	OSDDrv_RegionFill((HANDLE)osd_dev,0,&rect,OTA_BAR_SH);

	// draw process bar thumb
	OSD_SetRect(&rect, OTA_BAR_L+2, OTA_BAR_T+2, (OTA_BAR_W-4)*progress/100, OTA_BAR_H-4);
	OSDDrv_RegionFill((HANDLE)osd_dev,0,&rect,OTA_THUMB_SH);

	// draw process txt
	OSD_SetRect(&rect, OTA_PROGRESS_L, OTA_PROGRESS_T, OTA_PROGRESS_W, OTA_PROGRESS_H);
	OSDDrv_RegionFill((HANDLE)osd_dev,0,&rect,OTA_CON_SH);
	sprintf(str,"%d%%", (progress>=99) ? 100 : progress);
	ComAscStr2Uni(str, ustr);
	pVscr = OSD_GetVscr(&rect,OSD_GET_BACK);
	OSD_DrawText(&rect,ustr,OTA_TXT_SH,C_ALIGN_LEFT,0,pVscr);
#ifdef PANEL_DISPLAY
	if(upg_stauts == 0)
	{
		sprintf(str,"d%03d",progress);
		if(progress == 90)
		{
			sprintf(str,"PREb");			
		}
	}
	else
	{
		sprintf(str,"%1xP%02d\n",part_id,progress);
	}
	stb_pan_display(str, 4);
#endif				

	if(pVscr->lpbScr != NULL)
		pVscr->updatePending = 1;
	OSD_UpdateVscr(pVscr);
	pVscr->lpbScr = NULL;
}

INT32 upg_process_callback(UINT32 param1,UINT32 param2)
{
	INT32 ret = 0;
	UINT8 *pstr;
	UINT8 str[100];
	UINT16 ustr[100];
	UINT32 part_cnt, part_idx;
	UINT32 str_len;
	UINT16 str_id=0;

	switch(param1)
	{
	case UPG_PART_INFO:
		part_idx = param2 & 0xffff;
		part_cnt = (param2>>16) & 0xffff;

		sprintf(str, " %d/%d.", part_idx, part_cnt);

		pstr = (UINT8*)OSD_GetUnicodeString(RS_UPDATE_PART);
		
		ComUniStrCopyChar((UINT8*) ustr, pstr);
	
		str_len = ComUniStrLen(ustr);
		ComAscStr2Uni(str, &ustr[str_len]);

		upg_info_update(0, (UINT8 *)ustr, 0);
		break;
	case UPG_STATUS_INFO:
		if(param2 == UPG_BACKUP_MIRROR_FLAG)
			str_id = RS_BACKUP_DATA;
		else if(param2 == UPG_BURN_PART_FLAG)
			str_id = RS_BURNING_FLASH;
		else 
			str_id = param2;
		
		upg_info_update(1, NULL, str_id);
		break;
	case UPG_PROCESS_NUM:
		upg_progress_update(param2);
		break;
	default:
		break;
	}

    return ret;
}
#endif

/*param1: upg chunk addr; param2: mirror chunk addr
return 0: not upgrade this chunk; 1: upgrade this chunk; -1: invalid ,exit upgrade*/
INT32 upg_chunk_check(UINT32 param1, UINT32 param2)
{
	UINT8 *upg_pointer, *mirror_pointer;
	CHUNK_HEADER upg_head, mirror_head;
	UINT16 version1,version2;
	INT32 ret = -1;

	upg_pointer = (UINT8*)param1;
	mirror_pointer = (UINT8*)param2;

	get_chunk_header_by_addr(upg_pointer, &upg_head);
	get_chunk_header_by_addr(mirror_pointer, &mirror_head);
	
	version1 = Str2UINT32(upg_head.version, 4);
	version2 = Str2UINT32(mirror_head.version, 4);
		
	if (version1 > version2)
		ret = 1;	// 1: upgrade this chunk
	else
		ret = 0;	// 0: not upgrade this chunk

	return ret;

}

/*param1: image part addr; param2: image part len; param3: mirror part addr; param4: mirror part len
return 0: success; -1: fail */
INT32 upg_keydata_restore(UINT32 param1, UINT32 param2, UINT32 param3, UINT32 param4)
{
	UINT8 *image_pointer, *mirror_pointer;
	UINT32 image_len, mirror_len;
	UINT8* image_chunk, *mirror_chunk;
	CHUNK_HEADER image_head, mirror_head;

	image_pointer = (UINT8*)param1;
	image_len = param2;
	mirror_pointer = (UINT8*)param3;
	mirror_len = param4;

	get_chunk_header_by_addr(image_pointer, &image_head);
	get_chunk_header_by_addr(mirror_pointer, &mirror_head);
	
	if(image_head.id != STB_CHUNK_BOOTLOADER_ID)
	{
		return 0;
	}

	/* save back mac/stbid... info */
	MEMCPY((UINT8*)image_pointer+STB_HWINFO_BASE_ADDR, (UINT8*)mirror_pointer+STB_HWINFO_BASE_ADDR, STB_HWINFO_LEN);

	return 0;

}

