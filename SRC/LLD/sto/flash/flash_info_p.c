/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: flash_info_p.c
 *
 *  Description: Provide paral flash information functions.
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

extern void pflash_get_id(unsigned long *, unsigned long);
extern int pflash_erase_chip(void);
extern int pflash_verify(unsigned long, unsigned char *, unsigned long);
extern int pflash_erase_sector(unsigned long);
extern int pflash_copy(unsigned long, unsigned char *, unsigned long);
extern int pflash_read( void *des, void* src, UINT32 len);

void flash_info_pl_init()
{
	flash_info.flash_type=1;  //parallel flash;
	flash_info.flash_deviceid = &pflash_deviceid[0];
	flash_info.flash_deviceid_ex = NULL;
	flash_info.flash_id_p = &pflash_id[0];
	flash_info.flash_id_p_ex = NULL;
	flash_info.flash_deviceid_num = pflash_deviceid_num;
	flash_info.flash_deviceid_num_ex = 0;	
	flash_info.get_id = pflash_get_id;
	flash_info.erase_chip = pflash_erase_chip;
	flash_info.erase_sector = pflash_erase_sector;
	flash_info.verify = pflash_verify;
	flash_info.write = pflash_copy;
	flash_info.read = pflash_read;
}
