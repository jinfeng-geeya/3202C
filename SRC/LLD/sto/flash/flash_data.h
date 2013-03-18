/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2005 Copyright (C)
 *
 *  File: flash_data.c
 *
 *  Description: Provide all flash data declare.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2005.5.28   Liu Lan     0.1.000  Initial
 *
 ****************************************************************************/

#ifndef _FLASH_DATA_H_
#define _FLASH_DATA_H_

extern const unsigned int tflash_cmdaddr[];
extern const unsigned short tflash_sectors[];
extern const unsigned char flash_sector_begin[];
extern const unsigned char flash_sector_map[];
extern const unsigned char flash_cmdaddr_num;

/* Declare for paral flash */
extern const unsigned char pflash_deviceid[];
extern const unsigned char pflash_id[];
extern const unsigned short pflash_deviceid_num;
extern const unsigned short pflash_deviceid_ex[];
extern const unsigned char pflash_id_ex[];
extern const unsigned short pflash_deviceid_num_ex;

/* Declare for serial flash */
extern const unsigned char sflash_deviceid[];
extern const unsigned char sflash_id[];
extern const unsigned short sflash_deviceid_num;

#endif
