/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: flash_info_s.c
 *
 *  Description: Provide serial flash information functions.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2005.5.28   Liu Lan     0.1.000  Initial
 *  2.  2006.4.28   Justin Wu   0.2.000  Re-arch
 *
 ****************************************************************************/
#include <bus/flash/flash.h>
#include "flash_data.h"

extern void sflash_get_id(UINT32 *result, UINT32 cmdaddr);
extern int sflash_erase_chip(void);
extern int sflash_erase_sector(UINT32 sector_addr);
extern int sflash_copy(UINT32 flash_addr, UINT8 *data, UINT32 len);
extern int sflash_read(void *buffer, void *flash_addr, UINT32 len);
extern int sflash_verify(UINT32 flash_addr, UINT8 *data, UINT32 len);

void flash_info_sl_config(UINT8  *pdeviceid, UINT8  *pflashid, UINT16 flash_id_num, UINT16  *pdeviceid_ex, UINT8  *pflashid_ex, UINT16 flash_id_num_ex)
{
	flash_info.flash_type=0;  //serial flash;
	if(pdeviceid != NULL)
		flash_info.flash_deviceid = pdeviceid;
	else
		flash_info.flash_deviceid = &sflash_deviceid[0];
	if(pflashid != NULL)	
		flash_info.flash_id_p = pflashid;
	else
		flash_info.flash_id_p = &sflash_id[0];
	flash_info.flash_deviceid_ex = NULL;	
	flash_info.flash_id_p_ex = NULL;
	if(flash_id_num != 0)	
		flash_info.flash_deviceid_num = flash_id_num;
	else
		flash_info.flash_deviceid_num = sflash_deviceid_num;
	flash_info.flash_deviceid_num_ex = 0;	
	flash_info.get_id = sflash_get_id;
	flash_info.erase_chip = sflash_erase_chip;
	flash_info.erase_sector = sflash_erase_sector;
	flash_info.verify = sflash_verify;
	flash_info.write = sflash_copy;
	flash_info.read = sflash_read;	
}

void flash_info_sl_init()
{
	flash_info.flash_type=0;  //serial flash;
	flash_info.flash_deviceid = &sflash_deviceid[0];
	flash_info.flash_deviceid_ex = NULL;	
	flash_info.flash_id_p = &sflash_id[0];
	flash_info.flash_id_p_ex = NULL;	
	flash_info.flash_deviceid_num = sflash_deviceid_num;
	flash_info.flash_deviceid_num_ex = 0;	
	flash_info.get_id = sflash_get_id;
	flash_info.erase_chip = sflash_erase_chip;
	flash_info.erase_sector = sflash_erase_sector;
	flash_info.verify = sflash_verify;
	flash_info.write = sflash_copy;
	flash_info.read = sflash_read;
}
