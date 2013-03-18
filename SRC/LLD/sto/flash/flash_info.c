/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2005 Copyright (C)
 *
 *  File: flash_info.c
 *
 *  Description: Provide flash information functions.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2005.5.28   Liu Lan     0.1.000  Initial
 *  2.  2006.4.28   Justin Wu   0.2.000  Re-arch
 *
 ****************************************************************************/

#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <bus/flash/flash.h>
#include "flash_data.h"

extern unsigned sflash_reg_addr;
extern unsigned int SFLASH_IDENTIFY_MODE;

unsigned int flash_sector_size(unsigned int sector)
{
	UINT8 fls_idx;
    unsigned char *p;

	if (sector >= flash_info.flash_sectors)
		sector = flash_info.flash_sectors - 1;

	if(multi_flash_info.multi_chip)
	{
		UINT8 fls_idx=0;
		for(fls_idx=0; fls_idx<4; fls_idx++)
		{
			if (sector==multi_flash_info.multi_chip_info[fls_idx].flash_sectors)
			{
				sector=multi_flash_info.multi_chip_info[fls_idx].flash_sectors-1;
				break;
			}
			else if(sector<multi_flash_info.multi_chip_info[fls_idx].flash_sectors)
			{
				break;
			}
			else
			{
				sector-=multi_flash_info.multi_chip_info[fls_idx].flash_sectors;
			}
		}
		p = (unsigned char *)flash_sector_map + \
		  	flash_sector_begin[multi_flash_info.multi_chip_info[fls_idx].flash_id] * 2;
	}
	else
	{
		p = (unsigned char *)flash_sector_map + \
			flash_sector_begin[flash_info.flash_id] * 2;
	}
	while (sector >= p[0])
	{
		sector -= p[0];
		p += 2;
	}
	return 1 << p[1];
}

unsigned int flash_sector_align(unsigned long addr)
{
    unsigned int i;
    unsigned int sector_size;

    for (i = 0; i < flash_info.flash_sectors; i++)
    {
        sector_size = flash_sector_size(i);
        if (addr < sector_size)
            break;
        addr -= sector_size;
    }
    return i;
}

unsigned long flash_sector_start(unsigned int sector_no)
{
    unsigned long addr = 0;
    while (sector_no--)
        addr += flash_sector_size(sector_no);
    return addr;
}

unsigned int flash_identify(void)
{
    unsigned long id;
    unsigned long id_buf[3];
    unsigned int s;
    unsigned short i, j;
    unsigned char flash_did;
    unsigned short flash_did_ex;
    if(1==flash_info.flash_type)
    {
        for (j = 0; j < flash_cmdaddr_num; j++)
        {
            flash_get_id(id_buf, tflash_cmdaddr[j]);
            flash_info.flash_id = FLASHTYPE_UNKNOWN;
            for (i = 0; i < flash_info.flash_deviceid_num * 2; i += 2)
            {
                s = flash_info.flash_deviceid[i + 1];
                id = id_buf[s >> 5];
                s &= 0x1F;
                flash_did = flash_info.flash_deviceid[i];
                if (((id >> s) & 0xFF) == flash_did)
                {
                    flash_info.flash_id = flash_info.flash_id_p[i >> 1];
                    /* special treatment for SST39VF088 */
                    if (flash_info.flash_id == FLASHTYPE_39080 && j == 0)
                        flash_info.flash_id = FLASHTYPE_39088;
                    if(0x1a == flash_did&&72==flash_info.flash_deviceid[i + 1])
                    {
                        if(0!=(id&0xffff0000)&&flash_info.flash_id==FLASHTYPE_S29AL032D_04)
                            flash_info.flash_id=FLASHTYPE_S29AL032D_03;
                    }
                    break;
                }
            }
            if (i < flash_info.flash_deviceid_num * 2)
            {
                flash_info.flash_sectors = (unsigned int) \
                      (tflash_sectors[flash_info.flash_id]);
                flash_info.flash_size = flash_sector_start(flash_info.flash_sectors);
                break;
            }
            for (i = 0; i < flash_info.flash_deviceid_num_ex * 2; i += 2)
            {
                s = flash_info.flash_deviceid_ex[i + 1];
                id = id_buf[(s & 0xff) >> 5];
                flash_did_ex = flash_info.flash_deviceid_ex[i];
               if ((unsigned char)((id >> (s & 0x1F)) & 0xFF) == (unsigned char)(flash_did_ex & 0xFF) && \
                    (unsigned char)((id >> ((s >> 8) & 0x1F)) & 0xFF) == (unsigned char)((flash_did_ex >> 8) & 0xFF))
                {
                    flash_info.flash_id = flash_info.flash_id_p_ex[i >> 1];
                    break;
                }
            }
            if (i < flash_info.flash_deviceid_num_ex * 2)
            {
                flash_info.flash_sectors = (unsigned int) \
                      (tflash_sectors[flash_info.flash_id]);
                flash_info.flash_size = flash_sector_start(flash_info.flash_sectors);
                break;
            }
        }
    
        return j < flash_cmdaddr_num;
    }
    else
    {
        UINT8 flash_identified=FALSE;
        for (j = 0; j < 4; j++)
        {
            id_buf[0]=id_buf[1]=id_buf[2]=0xffffffff;
            flash_get_id(id_buf, j);
            if(((0xffffffff==id_buf[0])||(0==id_buf[0]))
                &&((0xffffffff==id_buf[1])||(0==id_buf[1]))
                &&((0xffffffff==id_buf[2])||(0==id_buf[1])))
            {
                //PRINTF("flash ID invalid\n");
                continue;
            }
            flash_info.flash_id = FLASHTYPE_UNKNOWN;
            for (i = 0; i < flash_info.flash_deviceid_num * 2; i += 2)
            {
                s = flash_info.flash_deviceid[i + 1];
                id = id_buf[s >> 5];
                s &= 0x1F;
                flash_did = flash_info.flash_deviceid[i];
                if (((id >> s) & 0xFF) == flash_did)
                {
                    flash_info.flash_id = flash_info.flash_id_p[i >> 1];
                    /* special treatment for SST39VF088 */
                    if (flash_info.flash_id == FLASHTYPE_39080 && j == 0)
                        flash_info.flash_id = FLASHTYPE_39088;
                    if(0x1a == flash_did&&72==flash_info.flash_deviceid[i + 1])
                    {
                        if(0!=(id&0xffff0000)&&flash_info.flash_id==FLASHTYPE_S29AL032D_04)
                            flash_info.flash_id=FLASHTYPE_S29AL032D_03;
                    }
                    break;
                }
            }
            if (i < flash_info.flash_deviceid_num * 2)
            {
                flash_info.flash_sectors = (unsigned int) \
                      (tflash_sectors[flash_info.flash_id]);
                flash_info.flash_size = flash_sector_start(flash_info.flash_sectors);
                flash_identified=TRUE;

                if(j>0)
                {
                    multi_flash_info.multi_chip=TRUE;
                }
		if(SFLASH_IDENTIFY_MODE != 1)
               { 
               multi_flash_info.chip_num++;
                multi_flash_info.total_sectors+=flash_info.flash_sectors;
                multi_flash_info.total_size+=flash_info.flash_size;
                multi_flash_info.multi_chip_info[j].base_addr=SYS_FLASH_BASE_ADDR-(j*0x1000000);
                multi_flash_info.multi_chip_info[j].flash_id=flash_info.flash_id;
                multi_flash_info.multi_chip_info[j].flash_io=1;
                multi_flash_info.multi_chip_info[j].flash_sectors=flash_info.flash_sectors;
                multi_flash_info.multi_chip_info[j].flash_size=flash_info.flash_size;
		}
                continue;
            }
            if(flash_identified)
            {
                break;
            }
            for (i = 0; i < flash_info.flash_deviceid_num_ex * 2; i += 2)
            {
                s = flash_info.flash_deviceid_ex[i + 1];
                id = id_buf[(s & 0xff) >> 5];
                flash_did_ex = flash_info.flash_deviceid_ex[i];
               if ((unsigned char)((id >> (s & 0x1F)) & 0xFF) == (unsigned char)(flash_did_ex & 0xFF) && \
                    (unsigned char)((id >> ((s >> 8) & 0x1F)) & 0xFF) == (unsigned char)((flash_did_ex >> 8) & 0xFF))
                {
                    flash_info.flash_id = flash_info.flash_id_p_ex[i >> 1];
                    break;
                }
            }
            if (i < flash_info.flash_deviceid_num_ex * 2)
            {
                flash_info.flash_sectors = (unsigned int) \
                      (tflash_sectors[flash_info.flash_id]);
                flash_info.flash_size = flash_sector_start(flash_info.flash_sectors);
                break;
            }
        }
        flash_info.flash_sectors=multi_flash_info.total_sectors;
        flash_info.flash_size=multi_flash_info.total_size;

	*((volatile UINT32*)(sflash_reg_addr + 0x98)) &= ~0xc0000000;
	if (flash_info.flash_size <= 0x200000)
		*((volatile UINT32*)(sflash_reg_addr + 0x98)) |= 0;
	else if (flash_info.flash_size > 0x200000 && flash_info.flash_size <= 0x400000)
		*((volatile UINT32*)(sflash_reg_addr + 0x98)) |= 0x40000000;
	else if (flash_info.flash_size > 0x400000 && flash_info.flash_size <= 0x800000)
		*((volatile UINT32*)(sflash_reg_addr + 0x98)) |= 0x80000000;
	else
		*((volatile UINT32*)(sflash_reg_addr + 0x98)) |= 0xc0000000;

	SFLASH_IDENTIFY_MODE = 1;
        return flash_identified;
    }
}
