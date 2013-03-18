/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    bootloader.c
*
*    Description:    This file contains all globe micros and functions declare
*		             of Flash boot loader.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Oct.28.2003     Justin Wu       Ver 0.1    Create file.
 *  2.  2006.5.16       Justin Wu       Ver 0.2    STO chunk support & clean up.
*****************************************************************************/
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

#include <api/libota/lib_ota.h>
#include <hld/osd/osddrv.h>

#include <api/libloader/ld_osd_lib.h>
#include <api/libloader/ld_boot_info.h>
#include <api/libosd/osd_vkey.h>
#include <api/libtsi/db_3l.h>
#include <api/libstbinfo/stb_info_data.h>
#include "dev_handle.h"
#include "../copper_common/boot_system_data.h"
#include "../copper_common/stb_data.h"
#include "key.h"
#include "upg_common.h"
#include "string.id"

//void *alloced_buf,*alloced_buf2;

extern UINT16	g_sw_version;

#define SELECT_STATUS 0
#define EDIT_STATUS	1

static UINT16 focus_id;
static UINT16 input_status;


#define INPUT_CON_L	(OSD_MAX_WIDTH-INPUT_CON_W)/2
#define INPUT_CON_T	(OSD_MAX_HEIGHT-INPUT_CON_H)/2
#define INPUT_CON_W 450
#define INPUT_CON_H	300

#define INPUT_TXTN_L	(INPUT_CON_L+30)
#define INPUT_TXTN_T	(INPUT_CON_T+40)
#define INPUT_TXTN_W	240
#define INPUT_TXTN_H	40//26
//#define INPUT_TXTN_W2	300

#define INPUT_TXTS_L	(INPUT_TXTN_L+INPUT_TXTN_W)
#define INPUT_TXTS_T	INPUT_TXTN_T
#define INPUT_TXTS_W	150
#define INPUT_TXTS_H	40//26

#define INPUT_TXT_GAP	0

#define INPUT_TXTSTART_L	(INPUT_CON_L+(INPUT_CON_W-INPUT_TXTSTART_W*2-70)/2)
#define INPUT_TXTSTART_T	(INPUT_TXTN_T+(INPUT_TXTN_H+INPUT_TXT_GAP)*5)
#define INPUT_TXTSTART_W	100
#define INPUT_TXTSTART_H	40//26

#define INPUT_TXTCANCEL_L	(INPUT_TXTSTART_L+INPUT_TXTSTART_W+70)
#define INPUT_TXTCANCEL_T	INPUT_TXTSTART_T

#define INPUT_CON_SH	WHITE_COLOR
#define INPUT_CON_HL	YELLOW_COLOR//BLACK_COLOR
#define INPUT_BACK_SH	GRAY_COLOR
#define INPUT_TXT_SH	BLACK_COLOR
#define INPUT_TXT_HL	BLACK_COLOR//WHITE_COLOR
#define INPUT_START_SH	GRAY_COLOR
#define INPUT_START_HL	YELLOW_COLOR//BLACK_COLOR

#define FREQUENCEY_ID 1
#define SYMBOL_ID	2
#define MODULATION_ID		3
#define PID_ID		4

#define START_ID	5
#define CANCEL_ID	6

struct RANGE
{
	UINT32 min;
	UINT32 max;
	UINT32 len;
};

struct IDNavi id_navi[]=
{
	{FREQUENCEY_ID,START_ID,SYMBOL_ID,0,0},
	{SYMBOL_ID,FREQUENCEY_ID,MODULATION_ID,0,0},
	{MODULATION_ID,SYMBOL_ID,PID_ID,0,0},
	{PID_ID,MODULATION_ID,START_ID,0,0},
	{START_ID,PID_ID,FREQUENCEY_ID,CANCEL_ID,CANCEL_ID},
	{CANCEL_ID,PID_ID,FREQUENCEY_ID,START_ID,START_ID},	
};

#define OTA_PARAM_NUM	(sizeof(id_navi)/sizeof(id_navi[0])-2)

UINT16 input_ids[]=
{
	RS_FREQUENCY,
	RS_SYMBOL,
	RS_MODULATION,
	RS_PID,
	RS_COMMON_START,
	RS_COMMON_CANCEL,
};

UINT8 *modulation_strs[]=
{
	"16-QAM",
	"32-QAM",
	"64-QAM",
	"128-QAM",
	"256-QAM",
};

UINT32 input_data[6];

struct RANGE input_range[] = 
{
	{48,859,3}, //freq
	{1000,7000,4}, //symb
	{0,4,5},//qam
	{1,9999,4}, // pid
};

void bootota_get_ustr(UINT32 index, UINT16 *ustr)
{
	UINT8 str[20];

	if(ustr == NULL)
		return;

	if(index == MODULATION_ID-1)
		ComAscStr2Uni(modulation_strs[input_data[index]], ustr);
	else
	{
		sprintf(str,"%d",input_data[index]);
		ComAscStr2Uni(str, ustr);
	}
}

UINT8 bootota_input_proc()
{
	union UPG_INFO *upg_info;
	struct OSDRect rect;
	lpVSCR 		pVscr;
	UINT8 *pstr;
	UINT8 str[20];
	UINT16 ustr[100];
	UINT16 i;
	UINT16 back_sh,txt_sh,sh;
	UINT32 new_focus_id;
	UINT32 value;
	UINT32 strlen;

	upg_info = (union UPG_INFO *)stb_info_data_get_upginfo();

	input_data[FREQUENCEY_ID-1] = upg_info->c_info.ota_frequency;
	input_data[SYMBOL_ID-1] = upg_info->c_info.ota_symbol;
	input_data[PID_ID-1] = upg_info->c_info.ota_pid;

	if((upg_info->c_info.ota_qam) <= 8 && (upg_info->c_info.ota_qam)>=4)
		input_data[MODULATION_ID-1] = upg_info->c_info.ota_qam-4;
	else
		input_data[MODULATION_ID-1] = 2;

	focus_id = FREQUENCEY_ID;
	input_status = SELECT_STATUS;
	value = 0;

	// draw input frame
	OSD_SetRect(&rect, INPUT_CON_L, INPUT_CON_T, INPUT_CON_W, INPUT_CON_H);	
	OSDDrv_RegionFill((HANDLE)osd_dev,0,&rect,INPUT_CON_SH);

	for(i=0;i<OTA_PARAM_NUM;i++)
	{
		pstr = (UINT8*)OSD_GetUnicodeString(input_ids[i]);
		OSD_SetRect(&rect, INPUT_TXTN_L, INPUT_TXTN_T+(INPUT_TXTN_H+INPUT_TXT_GAP)*i, INPUT_TXTN_W, INPUT_TXTN_H);	
		pVscr = OSD_GetVscr(&rect,OSD_GET_BACK);
		OSD_DrawText(&rect,pstr,INPUT_TXT_SH,C_ALIGN_LEFT|C_ALIGN_VCENTER,0,pVscr);

		if(pVscr->lpbScr != NULL)
			pVscr->updatePending = 1;
		OSD_UpdateVscr(pVscr);
		pVscr->lpbScr = NULL;

		if(focus_id-1 == i)
		{
			back_sh = INPUT_CON_HL;
			txt_sh = INPUT_TXT_HL;
		}
		else
		{
			back_sh = INPUT_CON_SH;
			txt_sh = INPUT_TXT_SH;
		}

		OSD_SetRect(&rect, INPUT_TXTS_L, INPUT_TXTS_T+(INPUT_TXTS_H+INPUT_TXT_GAP)*i, INPUT_TXTS_W, INPUT_TXTS_H);	
		OSDDrv_RegionFill((HANDLE)osd_dev,0,&rect,back_sh);
		pVscr = OSD_GetVscr(&rect,OSD_GET_BACK);
		bootota_get_ustr(i,ustr);
		OSD_DrawText(&rect,ustr,txt_sh,C_ALIGN_CENTER|C_ALIGN_VCENTER,0,pVscr);
		
		if((focus_id-1==i) && (i==MODULATION_ID-1))
		{
			OSD_SetRect(&rect, INPUT_TXTS_L, INPUT_TXTS_T+(INPUT_TXTS_H+INPUT_TXT_GAP)*i, 26, INPUT_TXTS_H);	
			ComAscStr2Uni("<", ustr);
			OSD_DrawText(&rect,ustr,txt_sh,C_ALIGN_CENTER|C_ALIGN_VCENTER,0,pVscr);

			OSD_SetRect(&rect, INPUT_TXTS_L+INPUT_TXTS_W-26, INPUT_TXTS_T+(INPUT_TXTS_H+INPUT_TXT_GAP)*i, 26, INPUT_TXTS_H);	
			ComAscStr2Uni(">", ustr);
			OSD_DrawText(&rect,ustr,txt_sh,C_ALIGN_CENTER|C_ALIGN_VCENTER,0,pVscr);
		}
			
		if(pVscr->lpbScr != NULL)
			pVscr->updatePending = 1;
		OSD_UpdateVscr(pVscr);
		pVscr->lpbScr = NULL;
	}

	// start button
	pstr = (UINT8*)OSD_GetUnicodeString(input_ids[START_ID-1]);
	OSD_SetRect(&rect, INPUT_TXTSTART_L, INPUT_TXTSTART_T, INPUT_TXTSTART_W, INPUT_TXTSTART_H);	
	OSDDrv_RegionFill((HANDLE)osd_dev,0,&rect,INPUT_START_SH);
	pVscr = OSD_GetVscr(&rect,OSD_GET_BACK);
	OSD_DrawText(&rect,pstr,INPUT_TXT_HL,C_ALIGN_CENTER|C_ALIGN_VCENTER,0,pVscr);

	if(pVscr->lpbScr != NULL)
		pVscr->updatePending = 1;
	OSD_UpdateVscr(pVscr);
	pVscr->lpbScr = NULL;
	
	// cancel button
	pstr = (UINT8*)OSD_GetUnicodeString(input_ids[CANCEL_ID-1]);
	OSD_SetRect(&rect, INPUT_TXTCANCEL_L, INPUT_TXTCANCEL_T, INPUT_TXTSTART_W, INPUT_TXTSTART_H);	
	OSDDrv_RegionFill((HANDLE)osd_dev,0,&rect,INPUT_START_SH);
	pVscr = OSD_GetVscr(&rect,OSD_GET_BACK);
	OSD_DrawText(&rect,pstr,INPUT_TXT_HL,C_ALIGN_CENTER|C_ALIGN_VCENTER,0,pVscr);

	if(pVscr->lpbScr != NULL)
		pVscr->updatePending = 1;
	OSD_UpdateVscr(pVscr);
	pVscr->lpbScr = NULL;

	UINT32 hkey;
	UINT32 vkey;
	UINT8 tmpdata;
	while(1)
	{
		hkey = boot_get_key_msg();
		if(hkey == INVALID_HK || hkey == INVALID_MSG)
			continue;

		ap_hk_to_vk(0,hkey, &vkey);

		if(vkey == V_KEY_ENTER)
		{
			if(focus_id != START_ID&&focus_id!=CANCEL_ID)
				continue;
			if(focus_id==CANCEL_ID)
			{
				return 1;
			}
			upg_info->c_info.ota_frequency = input_data[FREQUENCEY_ID-1];
			upg_info->c_info.ota_symbol = input_data[SYMBOL_ID-1];
			upg_info->c_info.ota_qam = input_data[MODULATION_ID-1]+4;
			upg_info->c_info.ota_pid = input_data[PID_ID-1];

			OSD_SetRect(&rect, INPUT_CON_L, INPUT_CON_T, INPUT_CON_W, INPUT_CON_H);	
			OSDDrv_RegionFill((HANDLE)osd_dev,0,&rect,OSD_TRANSPARENT_COLOR);

			return 0;
		}
		else if(vkey == V_KEY_UP || vkey == V_KEY_DOWN)
		{
			if(input_status == EDIT_STATUS)
			{
				input_status = SELECT_STATUS;
				if(value>=input_range[focus_id-1].min && value<=input_range[focus_id-1].max)
					input_data[focus_id-1] = value;
			}

FOCUS_CHANGE:
			if(vkey == V_KEY_UP)
				new_focus_id = id_navi[focus_id-1].upID;
			else if(vkey == V_KEY_DOWN)
				new_focus_id = id_navi[focus_id-1].downID;
			else if(vkey == V_KEY_LEFT)
				new_focus_id = id_navi[focus_id-1].leftID;
			else
				new_focus_id = id_navi[focus_id-1].rightID;

			if(new_focus_id == 0)
				continue;
				
			if(focus_id == START_ID)
			{
				pstr = (UINT8*)OSD_GetUnicodeString(input_ids[START_ID-1]);
				OSD_SetRect(&rect, INPUT_TXTSTART_L, INPUT_TXTSTART_T, INPUT_TXTSTART_W, INPUT_TXTSTART_H);	
				back_sh = INPUT_START_SH;
				txt_sh = INPUT_TXT_HL;
			}
			else if(focus_id == CANCEL_ID)
			{
				pstr = (UINT8*)OSD_GetUnicodeString(input_ids[CANCEL_ID-1]);
				OSD_SetRect(&rect, INPUT_TXTCANCEL_L, INPUT_TXTCANCEL_T, INPUT_TXTSTART_W, INPUT_TXTSTART_H);	
				back_sh = INPUT_START_SH;
				txt_sh = INPUT_TXT_HL;
			}
			else
			{
				bootota_get_ustr(focus_id-1, ustr);
				pstr = (UINT8*)ustr;
				OSD_SetRect(&rect, INPUT_TXTS_L, INPUT_TXTS_T+(INPUT_TXTS_H+INPUT_TXT_GAP)*(focus_id-1), INPUT_TXTS_W, INPUT_TXTS_H);	
				back_sh = INPUT_CON_SH;
				txt_sh = INPUT_TXT_SH;
			}
			OSDDrv_RegionFill((HANDLE)osd_dev,0,&rect,back_sh);
			pVscr = OSD_GetVscr(&rect,OSD_GET_BACK);
			OSD_DrawText(&rect,pstr,txt_sh,C_ALIGN_CENTER|C_ALIGN_VCENTER,0,pVscr);

			if(pVscr->lpbScr != NULL)
			pVscr->updatePending = 1;
			OSD_UpdateVscr(pVscr);
			pVscr->lpbScr = NULL;

			if(new_focus_id == START_ID)
			{
				pstr = (UINT8*)OSD_GetUnicodeString(input_ids[START_ID-1]);
				OSD_SetRect(&rect, INPUT_TXTSTART_L, INPUT_TXTSTART_T, INPUT_TXTSTART_W, INPUT_TXTSTART_H);	
				back_sh = INPUT_START_HL;
				txt_sh = INPUT_TXT_HL;
			}
			else  if(new_focus_id == CANCEL_ID)
			{
				pstr = (UINT8*)OSD_GetUnicodeString(input_ids[CANCEL_ID-1]);
				OSD_SetRect(&rect, INPUT_TXTCANCEL_L, INPUT_TXTCANCEL_T, INPUT_TXTSTART_W, INPUT_TXTSTART_H);	
				back_sh = INPUT_START_HL;
				txt_sh = INPUT_TXT_HL;
			}
			else
			{
				bootota_get_ustr(new_focus_id-1, ustr);
				pstr = (UINT8*)ustr;
				OSD_SetRect(&rect, INPUT_TXTS_L, INPUT_TXTS_T+(INPUT_TXTS_H+INPUT_TXT_GAP)*(new_focus_id-1), INPUT_TXTS_W, INPUT_TXTS_H);	
				back_sh = INPUT_CON_HL;
				txt_sh = INPUT_TXT_HL;
			}
			OSDDrv_RegionFill((HANDLE)osd_dev,0,&rect,back_sh);
			pVscr = OSD_GetVscr(&rect,OSD_GET_BACK);
			OSD_DrawText(&rect,pstr,txt_sh,C_ALIGN_CENTER|C_ALIGN_VCENTER,0,pVscr);

			if(new_focus_id==MODULATION_ID)
			{
				OSD_SetRect(&rect, INPUT_TXTS_L, INPUT_TXTS_T+(INPUT_TXTS_H+INPUT_TXT_GAP)*(new_focus_id-1), 26, INPUT_TXTS_H);	
				ComAscStr2Uni("<", ustr);
				OSD_DrawText(&rect,ustr,txt_sh,C_ALIGN_CENTER|C_ALIGN_VCENTER,0,pVscr);

				OSD_SetRect(&rect, INPUT_TXTS_L+INPUT_TXTS_W-26, INPUT_TXTS_T+(INPUT_TXTS_H+INPUT_TXT_GAP)*(new_focus_id-1), 26, INPUT_TXTS_H);	
				ComAscStr2Uni(">", ustr);
				OSD_DrawText(&rect,ustr,txt_sh,C_ALIGN_CENTER|C_ALIGN_VCENTER,0,pVscr);
			}

			focus_id = new_focus_id;
		}
		else if(/*vkey>=V_KEY_0 && */vkey<=V_KEY_9)
		{
			if(focus_id<FREQUENCEY_ID || focus_id>PID_ID)
				continue;
			
			if(input_status == SELECT_STATUS)
			{
				value = 0;
				input_status = EDIT_STATUS;
			}

			value = value*10 + vkey - V_KEY_0;
			
			sprintf(str,"%d",value);
			strlen = STRLEN(str);

			if(strlen >= input_range[focus_id-1].len)
			{
				input_status = SELECT_STATUS;
				if(value>=input_range[focus_id-1].min && value<=input_range[focus_id-1].max)
					input_data[focus_id-1] = value;
			}
			else
			{
				sprintf(str,"%d_",value);
			}
			ComAscStr2Uni(str, ustr);
			OSD_SetRect(&rect, INPUT_TXTS_L, INPUT_TXTS_T+(INPUT_TXTS_H+INPUT_TXT_GAP)*(focus_id-1), INPUT_TXTS_W, INPUT_TXTS_H);	
			OSDDrv_RegionFill((HANDLE)osd_dev,0,&rect,INPUT_CON_HL);
			pVscr = OSD_GetVscr(&rect,OSD_GET_BACK);
			OSD_DrawText(&rect,ustr,INPUT_TXT_HL,C_ALIGN_CENTER|C_ALIGN_VCENTER,0,pVscr);
		}
		else if(vkey == V_KEY_LEFT || vkey == V_KEY_RIGHT)
		{
			if(focus_id == START_ID || focus_id ==CANCEL_ID)
				goto FOCUS_CHANGE;
			
			if(focus_id==FREQUENCEY_ID || focus_id==PID_ID || focus_id==SYMBOL_ID)
			{
				if(vkey == V_KEY_RIGHT || input_status == SELECT_STATUS)
					continue;

				value = value/10;
				sprintf(str,"%d_",value);
				ComAscStr2Uni(str, ustr);
				OSD_SetRect(&rect, INPUT_TXTS_L, INPUT_TXTS_T+(INPUT_TXTS_H+INPUT_TXT_GAP)*(focus_id-1), INPUT_TXTS_W, INPUT_TXTS_H);	
				OSDDrv_RegionFill((HANDLE)osd_dev,0,&rect,INPUT_CON_HL);
				pVscr = OSD_GetVscr(&rect,OSD_GET_BACK);
				OSD_DrawText(&rect,ustr,INPUT_TXT_HL,C_ALIGN_CENTER|C_ALIGN_VCENTER,0,pVscr);

				if(pVscr->lpbScr != NULL)
				pVscr->updatePending = 1;
				OSD_UpdateVscr(pVscr);
				pVscr->lpbScr = NULL;
			}
			else if(focus_id==MODULATION_ID)
			{
				if(vkey == V_KEY_LEFT)
					value = -1;
				else
					value = 1;

				input_data[focus_id-1] = (input_data[focus_id-1]+value+input_range[focus_id-1].len)%input_range[focus_id-1].len;
				bootota_get_ustr(focus_id-1, ustr);
				OSD_SetRect(&rect, INPUT_TXTS_L, INPUT_TXTS_T+(INPUT_TXTS_H+INPUT_TXT_GAP)*(focus_id-1), INPUT_TXTS_W, INPUT_TXTS_H);	
				back_sh = INPUT_CON_HL;
				txt_sh = INPUT_TXT_HL;
				OSDDrv_RegionFill((HANDLE)osd_dev,0,&rect,back_sh);
				pVscr = OSD_GetVscr(&rect,OSD_GET_BACK);
				OSD_DrawText(&rect,ustr,txt_sh,C_ALIGN_CENTER|C_ALIGN_VCENTER,0,pVscr);

				OSD_SetRect(&rect, INPUT_TXTS_L, INPUT_TXTS_T+(INPUT_TXTS_H+INPUT_TXT_GAP)*(focus_id-1), 26, INPUT_TXTS_H);	
				ComAscStr2Uni("<", ustr);
				OSD_DrawText(&rect,ustr,txt_sh,C_ALIGN_CENTER|C_ALIGN_VCENTER,0,pVscr);

				OSD_SetRect(&rect, INPUT_TXTS_L+INPUT_TXTS_W-26, INPUT_TXTS_T+(INPUT_TXTS_H+INPUT_TXT_GAP)*(focus_id-1), 26, INPUT_TXTS_H);	
				ComAscStr2Uni(">", ustr);
				OSD_DrawText(&rect,ustr,txt_sh,C_ALIGN_CENTER|C_ALIGN_VCENTER,0,pVscr);
			}
		}
		if(pVscr->lpbScr != NULL)
			pVscr->updatePending = 1;
		OSD_UpdateVscr(pVscr);
		pVscr->lpbScr = NULL;
	}
	return 0;
}


