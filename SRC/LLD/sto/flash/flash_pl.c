/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: flash_pl.c
 *
 *  Description: Provide local paral flash driver for sto type device.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2006.4.xx Shipman Yuan  0.1.000  Initial
 *
 ****************************************************************************/
#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <sys_parameters.h>
#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hal/hal_mem.h>
#include <hld/hld_dev.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <bus/flash/flash.h>
#include "sto_flash.h"
#include "flash_data.h"


/* Name for the local FLASH, the last character must be Number */
static char sto_flash_name[HLD_MAX_NAME_SIZE] = "STO_FLASH_0";

extern void pflash_get_id(unsigned long *, unsigned long);
extern int pflash_erase_chip(void);
extern int pflash_verify(unsigned long, unsigned char *, unsigned long);
extern int pflash_erase_sector(unsigned long);
extern int pflash_copy(unsigned long, unsigned char *, unsigned long);
extern int pflash_read( void *des, void* src, UINT32 len);


/**************************************************************
 * Function:
 * 	sto_local_flash_attach()
 * Description:
 * 	Create and init local flash device.
 * Inpute:
 *	None.
 * Return Values:
 *	Return SUCCESS for successfully create and init the
 *      device.
 ***************************************************************/
INT32 sto_local_flash_attach(struct sto_flash_info *param)
{
	struct sto_device *dev;
	struct flash_private *tp;
	unsigned int ret;
	unsigned char sw;

	dev = dev_alloc(sto_flash_name, HLD_DEV_TYPE_STO,sizeof(struct sto_device));
	if (dev == NULL) {
		PRINTF("Error: Alloc storage device error!\n");
		return ERR_NO_MEM;
	}

	/* Alloc structure space of private */
	if ((tp = MALLOC(sizeof(struct flash_private))) == NULL) {
		dev_free(dev);
		PRINTF("Error: Alloc front panel device priv memory error!\n");
		return ERR_NO_MEM;
	}
	dev->priv = tp;

	tp->get_id = pflash_get_id ;
	tp->erase_chip = pflash_erase_chip ;
	tp->erase_sector= pflash_erase_sector ;
	tp->write 	= pflash_copy ;
	tp->read 	= pflash_read ;
	tp->verify 	= pflash_verify ;
	tp->open  	= NULL;
	tp->close 	= NULL;
	tp->io_ctrl 	= NULL;

	if (param == NULL || param->flash_deviceid_num == 0)
	{
		tp->flash_deviceid = &pflash_deviceid[0];
		tp->flash_id_p     = &pflash_id[0];
		tp->flash_deviceid_num = pflash_deviceid_num;
	} else
	{
		tp->flash_deviceid = param->flash_deviceid;
		tp->flash_id_p     = param->flash_id;
		tp->flash_deviceid_num = param->flash_deviceid_num;
	}

	/* Current operate address */
	MEMSET(dev->curr_addr, 0, STO_TASK_SUPPORT_NUM_MAX * sizeof(UINT32));

	dev->base_addr = SYS_FLASH_BASE_ADDR;   /* Flash base address */

	sw = *(unsigned char *)(SYS_FLASH_BASE_ADDR + \
		HW_SET_FLASHSPEED) & 0x3F;
	if (sw == 0)
		sw = HW_FLASH_DEFSPEED;
	//HAL_ROM_MODE_SET(sw, 0, 1, 0);

	//MUTEX_ENTER();
	ret = sto_pflash_identify(dev,0);
	//MUTEX_LEAVE();

	if (ret == 0) 	{
		PRINTF("Error: Unknow Flash type.\n");
		FREE(tp);
		dev_free(dev);
		return ERR_NO_DEV;
	}

	/* Function point init */
	sto_fp_init(dev,sto_local_flash_attach);

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
unsigned int sto_pflash_identify(struct sto_device *dev, UINT32 mode)
{
	unsigned short s,i,j;
	unsigned long id, id_buf[3];
	unsigned char flash_did;
	struct flash_private *tp = (struct flash_private *)dev->priv;

	for (j = 0; j < flash_cmdaddr_num; j++)
	{
		if( 0 == mode )
			tp->get_id(id_buf, tflash_cmdaddr[j]);
		else {
			/*
			In the mode, one Master STB will upgrade many Slave STBs
			in one time. So we identify local flash and assume that
			all Slave STBs have the same type of flashes. Also, we
			should inform remote_flash driver of the correct flash
			command address.
			*/
			pflash_get_id(id_buf, tflash_cmdaddr[j]);
			/* *below operation only applying to remote_flash* */
			tp->io_ctrl(FLASH_SET_CMDADDR,tflash_cmdaddr[j]);
		}

		tp->flash_id = FLASHTYPE_UNKNOWN;
		for (i = 0; i < (tp->flash_deviceid_num)*2; i += 2)
		{
			s = (tp->flash_deviceid)[i + 1];
			id = id_buf[s >> 5];
			s &= 0x1F;
			flash_did = (tp->flash_deviceid)[i];
			if (((id >> s) & 0xFF) == flash_did)
			{
				tp->flash_id = (tp->flash_id_p)[i >> 1];
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
			break;
		}
	}

	return j < flash_cmdaddr_num;
}
