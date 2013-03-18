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
#include <api/libpub/lib_device_manage.h>

#include <api/libloader/ld_boot_info.h>

#include <api/libstbinfo/stb_info_data.h>
#include <api/libupg/lib_upg4.h>
#include <api/libupg/ota_upgrade.h>

#include "dev_handle.h"
#include "../copper_common/stb_data.h"
#include "string.id"
#include "upg_common.h"

#define NIM_WAIT_LOCK_TIME	1000

#define UPG_SWAP_BUF_LEN           		(256*1024)

extern UINT8 upg_stauts;

extern INT32 upg_process_callback(UINT32 param1,UINT32 param2);
extern INT32 upg_chunk_check(UINT32 param1, UINT32 param2);
extern INT32 upg_keydata_restore(UINT32 param1, UINT32 param2, UINT32 param3, UINT32 param4);
	
INT32 ota_check_service(union ft_xpond *xpond_info)
{
	struct nim_config lnb_info;
	UINT32 unlock_timeout;
	UINT8 lock = 0;

	frontend_set_nim(nim_dev, NULL, xpond_info, 1);
	
	unlock_timeout = 0;
	while(1)
	{
		nim_get_lock(nim_dev, &lock);
		if(lock==1 || unlock_timeout>NIM_WAIT_LOCK_TIME)
			break;
		osal_task_sleep(10);
		unlock_timeout++;
	}

	if(lock == 1)
		return SUCCESS;
	else
		return !SUCCESS;
}

void ota_init(void)
{
	UPG_CONFIG cfg;
	int ret = -1;
	UINT8 order[1] = {0};

	// 8M
	cfg.compressed_addr = (UINT8 *)__MM_UPG_COMPRESSED_BUF_ADDR;
	cfg.compressed_len = __MM_UPG_COMPRESSED_BUFFER_LEN;

	// 8M
	cfg.uncompressed_addr = (UINT8 *)__MM_UPG_UNCOMPRESSED_BUF_ADDR;
	cfg.uncompressed_len = __MM_UPG_UNCOMPRESSED_BUFFER_LEN;

	// 8M
	cfg.data_image_addr = (UINT8 *)__MM_UPG_IMAGE_BUF_ADDR;
	cfg.data_image_len = __MM_UPG_IMAGE_BUFFER_LEN;
	

	// 8M
	cfg.data_mirror_addr = cfg.compressed_addr;
	cfg.data_mirror_len = cfg.compressed_len;


	cfg.swap_addr = MALLOC(UPG_SWAP_BUF_LEN);//cfg.data_image_addr;
	cfg.swap_len = UPG_SWAP_BUF_LEN;
	
	cfg.upg_disp_cb = upg_process_callback;
	cfg.upg_version_cb = upg_chunk_check;
	cfg.upg_keydata_cb = upg_keydata_restore;
	
	upg4_config(&cfg);
	
	upg_reg_bootloader_id(STB_CHUNK_BOOTLOADER_ID);
	upg_reg_stbinfo_id(STB_CHUNK_STBINFO_ID);
	
	upg4_config_burn_order(order, 1);

	upg_reg_restore_id(STB_CHUNK_CAS_ID);
	upg_reg_restore_id(STB_CHUNK_USER_DB_ID);	
}

INT32 ota_upgrade(UINT8 force_ota)
{
	union UPG_INFO *upg_info = NULL;
	union ft_xpond xpond_info;
	//struct OSDRect rect;
	static UINT8 ota_step = 0;//0:get info; 1: download; 2: burn.
	UINT32 i;
	UINT32 flgptn = 0;
	struct dl_info ota_dl_info;
	BOOL ret;
	UINT8 ret_input;
	STB_INFO_DATA *stb_data;
	struct ota_config_t ota_cfg;
	UINT8 sw_version_flag;


	/* upg config init */
	ota_init();

	stb_data = stb_info_data_get();
	upg_info = (union UPG_INFO *)stb_info_data_get_upginfo();

	get_stb_hwinfo(&ota_cfg.oui,&ota_cfg.hw_model,&ota_cfg.hw_version);
	ota_cfg.sw_model = stb_data->sw_model;
	ota_cfg.sw_version = stb_data->sw_version;
	ota_cfg.sw_version_chufa = upg_info->c_info.sw_version_trigger;
	ota_cfg.ctrl_flag = upg_info->c_info.upg_control_flag;
	sw_version_flag = upg_info->c_info.sw_version_ctrl;
	
	ota_config(&ota_cfg);

	if( ((upg_info->c_info.upg_trigger_type & STB_INFO_UPG_TRIGGER_BOOTFAIL) && (stb_data->state & STB_INFO_DATA_STATE_LIMIT)) 
		|| (upg_info->c_info.upg_trigger_type & STB_INFO_UPG_TRIGGER_BOOTKEY)
		|| (upg_info->c_info.upg_trigger_type == STB_INFO_UPG_TRIGGER_NONE))
	{
OTA_INFO_INPUT:	
		stb_pan_display(" OTA", 4);
		// if can not get para from maincode, input para first
		boot_clear_all_message();
		ret_input=bootota_input_proc();
		if(ret_input)
		{
			return 2;
		}
	}

	upg_info->c_info.upg_trigger_type = STB_INFO_UPG_TRIGGER_NONE;
	stb_info_data_save();
	
	MEMSET(&xpond_info, 0, sizeof(xpond_info));
	xpond_info.c_info.type = FRONTEND_TYPE_C;
	xpond_info.c_info.frq = upg_info->c_info.ota_frequency*100;
	xpond_info.c_info.sym = upg_info->c_info.ota_symbol;
	xpond_info.c_info.modulation = upg_info->c_info.ota_qam;

	// draw con frame
	upg_draw_frame();

	upg_info_update(0, NULL, RS_COLLECT_INFO);

	if(force_ota == 0)
	{
		/*add delay for hdmi show slow issue*/
		for(i=0;i<200;i++)
			osal_delay(5000);
	}	

	// check service
	ret = ota_check_service(&xpond_info);
	if (ret != SUCCESS)
	{
		upg_info_update(0, NULL, RS_CHECK_SERVICE_FAIL);
		stb_pan_display("er02", 4);

		for(i=0;i<400;i++)
			osal_delay(5000);
		goto OTA_INFO_INPUT;
	}

	// collect download info
	ret = ota_cmd_get_download_info(upg_info->c_info.ota_pid,&ota_dl_info);
	if(ret)
	{
		//sw version check
		if( (sw_version_flag == STB_INFO_SW_VER_LARGE) || (sw_version_flag == STB_INFO_SW_VER_SAME))
		{
			if(ota_dl_info.sw_version <= stb_data->sw_version)
				ret = FALSE;
		}
		else if(sw_version_flag == STB_INFO_SW_VER_DIFFER)
		{
			if(ota_dl_info.sw_version == stb_data->sw_version)
				ret = FALSE;
		}
	}
	if(!ret)
	{
		// get dl info failed
		upg_info_update(0, NULL, RS_DOWNLOAD_INFO_FAIL);
		
		stb_pan_display("er03", 4);

		for(i=0;i<400;i++)
			osal_delay(5000);
		goto OTA_INFO_INPUT;
	}

	// start download
	upg_info_update(0, NULL, RS_DOWNLOADING);
	upg_stauts = 0;	
	upg_progress_update(0);
	ret = ota_cmd_start_download(upg_info->c_info.ota_pid);
	if(!ret)
	{
		// download failed
		upg_info_update(0, NULL, RS_DOWNLOAD_FAIL);
		stb_pan_display("er04", 4);

		for(i=0;i<400;i++)
			osal_delay(5000);

		goto OTA_INFO_INPUT;
	}

	// start burn flash
	upg_info_update(0, NULL, RS_START_UPDATE);
	upg_stauts = 1;		
	upg_progress_update(0);
	ret = ota_cmd_start_burnflash();
	if(!ret)
	{
		char string[16];
		// burn failed
		upg_info_update(0, NULL, RS_BURN_FAIL);
		upg4_get_err_string(&string);
		stb_pan_display(string, 4);

	for(i=0;i<400;i++)
			osal_delay(5000);

		goto OTA_INFO_INPUT;
	}        

	upg_info_update(1, NULL, 0);
	upg_info_update(0, NULL, RS_UPGRADE_SUCCESS);
	stb_pan_display("done", 4);

	for(i=0;i<400;i++)
		osal_delay(5000);

	return 1;
}

