/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: flash_pr.c
 *
 *  Description: Provide remote paral flash driver for sto type device.
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
#include <bus/erom/erom.h>
#include "sto_flash.h"
#include "flash_data.h"


/* Name for the remote FLASH, the last character must be Number */
static char sto_remote_flash_name[HLD_MAX_NAME_SIZE] = "STO_RM_FLASH_0";

extern int remote_flash_open( void *arg);
extern int remote_flash_close ( void *arg);
extern int remote_flash_control(unsigned long ctrl_cmd, unsigned long ctrl_val);
extern void remote_flash_get_id(unsigned long *id_buf, unsigned long cmd_addr);
extern int remote_flash_erase_chip(void);
extern int remote_flash_erase_sector(unsigned long sector_addr);
extern int remote_flash_copy(unsigned long flash_addr, unsigned char *data, unsigned long len);
extern int remote_flash_read2( void *des, void* src, UINT32 len);
extern int remote_flash_verify(unsigned long flash_addr, unsigned char *data, unsigned long len);

/**************************************************************
 * Function:
 * 	sto_flash_init2()
 * Description:
 * 	Create and init specific flash device.
 * Inpute:
 *	arg
 *	    NULL --- Indicate to create and init local flash
 *		     device.
 *	    Non-NULL --- Indicate to create and init remote
 *                   flash device.
 * Return Values:
 *	Return SUCCESS for successfully create and init the
 *      device.
 ***************************************************************/
INT32 sto_remote_flash_attach(struct sto_flash_info *param)
{
	struct sto_device *dev;
	struct flash_private *tp;
	unsigned int ret;
	unsigned char sw,mode;

	dev = dev_alloc(sto_remote_flash_name, \
			 HLD_DEV_TYPE_STO,sizeof(struct sto_device));
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

	tp->get_id 	= remote_flash_get_id ;
	tp->erase_chip 	= remote_flash_erase_chip ;
	tp->erase_sector= remote_flash_erase_sector ;
	tp->write 	= remote_flash_copy ;
	tp->read 	= remote_flash_read2 ;
	tp->verify 	= remote_flash_verify ;
	tp->open 	= remote_flash_open ;
	tp->close 	= remote_flash_close ;
	tp->io_ctrl 	= remote_flash_control ;

	//reuse those data of paralell flash
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
	
	
	
	// config uart bitrate & config R/W of slave flash
	if( SUCCESS != uart_high_speed_config(param->uart_baudrate) ||
		SUCCESS != tp->io_ctrl( UPG_MODE_CONTROL, param->mode ) ||
			SUCCESS != tp->io_ctrl(FLASH_CONTROL,param->flash_ctrl) ) {
		PRINTF("Error: Config UART baudrate error!\n");
		FREE(tp);
		dev_free(dev);
		return ERR_NO_DEV;
	}

	if( param->mode == M2S_MODE )
		mode = 1;
	else
		mode = 0;
	
	//MUTEX_ENTER();
	ret = sto_pflash_identify(dev,mode);
	//MUTEX_LEAVE();

	if (ret == 0) 	{
		PRINTF("Error: Unknow Flash type.\n");
		FREE(tp);
		dev_free(dev);
		return ERR_NO_DEV;
	}

	/* Function point init */
	sto_fp_init(dev,sto_remote_flash_attach);

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
