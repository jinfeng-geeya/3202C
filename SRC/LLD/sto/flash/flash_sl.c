/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: flash_sl.c
 *
 *  Description: Provide local serial flash driver for sto type device.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2006.4.24   Justin Wu   0.1.000  Initial
 *
 ****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <bus/flash/flash.h>
#include "sto_flash.h"
#include "flash_data.h"

extern void sflash_get_id(UINT32 *result, UINT32 chip_idx);
extern int sflash_open( void *arg );
extern int sflash_close( void *arg );
extern int sflash_erase_chip(void);
extern int sflash_erase_sector(UINT32 sector_addr);
extern int sflash_copy(UINT32 flash_addr, UINT8 *data, UINT32 len);
extern int sflash_read(void *buffer, void *flash_addr, UINT32 len);
extern int sflash_verify(UINT32 flash_addr, UINT8 *data, UINT32 len);

extern unsigned short sflash_devid;

extern unsigned sflash_reg_addr;
extern unsigned int SFLASH_IDENTIFY_MODE;

static char sto_sflash_local_name[HLD_MAX_NAME_SIZE] = "STO_SFLASH_0";

INT32 sto_local_sflash_attach(struct sto_flash_info *param)
{
	struct sto_device *dev;
	struct flash_private *tp;
	unsigned int ret;


	dev = dev_alloc(sto_sflash_local_name, HLD_DEV_TYPE_STO, sizeof(struct sto_device));
	if (dev == NULL)
	{
		PRINTF("Error: Alloc storage device error!\n");
		return ERR_NO_MEM;
	}

	/* Alloc structure space of private */
	if ((tp = MALLOC(sizeof(struct flash_private))) == NULL) {
		dev_free(dev);
		PRINTF("Error: Alloc front panel device priv memory error!\n");
		return ERR_NO_MEM;
	}
	MEMSET(tp, 0, sizeof(struct flash_private));
	dev->priv = tp;

	tp->get_id = sflash_get_id;

	tp->erase_chip = sflash_erase_chip;

	tp->erase_sector = sflash_erase_sector;
	tp->write = sflash_copy;
	tp->read = sflash_read;
	tp->verify = sflash_verify;
	tp->open  = NULL;
	tp->close = NULL;
	tp->io_ctrl = NULL;

	if (param == NULL || param->flash_deviceid_num == 0)
	{
		tp->flash_deviceid = &sflash_deviceid[0];
		tp->flash_id_p     = &sflash_id[0];
		tp->flash_deviceid_num = sflash_deviceid_num;
	} else
	{
		tp->flash_deviceid = param->flash_deviceid;
		tp->flash_id_p     = param->flash_id;
		tp->flash_deviceid_num = param->flash_deviceid_num;
	}

	/* Current operate address */
	MEMSET(dev->curr_addr, 0, STO_TASK_SUPPORT_NUM_MAX * sizeof(UINT32));

	dev->base_addr = SYS_FLASH_BASE_ADDR;   /* Flash base address */

	//MUTEX_ENTER();
	ret = sto_sflash_identify(dev, 0);
	//MUTEX_LEAVE();

	if (ret == 0) 	{
		PRINTF("Error: Unknow Flash type.\n");
		FREE(tp);
		dev_free(dev);
		return ERR_NO_DEV;
	}

	/* Function point init */
	sto_fp_init(dev, sto_local_sflash_attach);

	dev->totol_size = tp->flash_size;

	/* Add this device to queue */
	if (dev_register(dev) != SUCCESS) {
		PRINTF("Error: Register Flash storage device error!\n");
		FREE(tp);
		dev_free(dev);
		return ERR_NO_DEV;
	}

	return SUCCESS;
}

/**************************************************************
 * Function:
 * 	sto_flash_identify()
 * Description:
 * 	Identify the parameters of flash.
 * Inpute:
 *	dev --- Device control block.
 *	mode
 *	   0 --- Ideentify the flash attached to dev
 *	   1 --- Identify the local flash and pass the paramters
 *		 to those of dev
 * Return Values:
 *	0 ---  Fail to identify flash
 *	1 ---  Idenfify successfully.
 ***************************************************************/
unsigned int sto_sflash_identify(struct sto_device *dev, UINT32 mode)
{
	unsigned short s,i,j;
	unsigned long id, id_buf[3];
	unsigned char flash_did;
	UINT8 multi_chip=FALSE;
	UINT8 flash_identified = FALSE;
	struct flash_private *tp = (struct flash_private *)dev->priv;

	for (j = 0; j < 4; j++)//max flash chip 4
	{
		id_buf[0]=id_buf[1]=id_buf[2]=0xffffffff;
		if( 0 == mode )
		{
			tp->get_id(id_buf, j);
		}
		else {
			/*
			In the mode, one Master STB will upgrade many Slave STBs
			in one time. So we identify local flash and assume that
			all Slave STBs have the same type of flashes. Also, we
			should inform remote_flash driver of the correct flash
			command address.
			*/
			sflash_get_id(id_buf, j);
			/* *below operation only applying to remote_flash* */
			if(tp->io_ctrl!=NULL)
				tp->io_ctrl(FLASH_SET_CMDADDR,sflash_devid);
		}
		if(((0xffffffff==id_buf[0])||(0==id_buf[0]))
                &&((0xffffffff==id_buf[1])||(0==id_buf[1]))
                &&((0xffffffff==id_buf[2])||(0==id_buf[1])))
		{
			//PRINTF("flash ID invalid\n");
			continue;
		}
		for (i = 0; i < (tp->flash_deviceid_num)*2; i += 2)//check id in table
		{
			s = (tp->flash_deviceid)[i + 1];
			id = id_buf[s >> 5];
			s &= 0x1F;
			flash_did = (tp->flash_deviceid)[i];
			if (((id >> s) & 0xFF) == flash_did)//device_id
			{
				tp->flash_id = (tp->flash_id_p)[i >> 1];//get id in sflash_id
				/* special treatment for SST39VF088 */
				if (tp->flash_id == FLASHTYPE_39080 && j == 0)
					tp->flash_id = FLASHTYPE_39088;
				break;
			}
		}
		if (i < (tp->flash_deviceid_num)*2 )
		{
			tp->flash_sectors = (unsigned int) \
				(tflash_sectors[tp->flash_id]);
			tp->flash_size=sto_flash_sector_start(dev,tp->flash_sectors);
			//tp->flash_cmdaddr = tflash_cmdaddr[j];
			flash_identified=TRUE;
			if(j>0)
			{
				multi_flash_info.multi_chip=TRUE;
			}
			if(SFLASH_IDENTIFY_MODE != 1)
			{
			multi_flash_info.chip_num++;
			multi_flash_info.total_sectors+=tp->flash_sectors;
			multi_flash_info.total_size+=tp->flash_size;
			multi_flash_info.multi_chip_info[j].base_addr=SYS_FLASH_BASE_ADDR-(j*0x1000000);
			multi_flash_info.multi_chip_info[j].flash_id=tp->flash_id;
			multi_flash_info.multi_chip_info[j].flash_io=1;
			multi_flash_info.multi_chip_info[j].flash_sectors=tp->flash_sectors;
			multi_flash_info.multi_chip_info[j].flash_size=tp->flash_size;
			}
			continue;
		}		
	}
	
	tp->flash_sectors=multi_flash_info.total_sectors;
	tp->flash_size=multi_flash_info.total_size;
	
//set flash size
	*((volatile UINT32*)(sflash_reg_addr + 0x98)) &= ~0xc0000000;
	if (tp->flash_size <= 0x200000)
		*((volatile UINT32*)(sflash_reg_addr + 0x98)) |= 0;
	else if (tp->flash_size > 0x200000 && tp->flash_size <= 0x400000)
		*((volatile UINT32*)(sflash_reg_addr + 0x98)) |= 0x40000000;
	else if (tp->flash_size > 0x400000 && tp->flash_size <= 0x800000)
		*((volatile UINT32*)(sflash_reg_addr + 0x98)) |= 0x80000000;
	else
		*((volatile UINT32*)(sflash_reg_addr + 0x98)) |= 0xc0000000;
	
	SFLASH_IDENTIFY_MODE = 1;
	return flash_identified;
}
