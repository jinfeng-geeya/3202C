/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2005 Copyright (C)
 *
 *  File: smc.c
 *
 *  Description: This file contains all functions definition
 *		             of smart card reader interface driver.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  0.                 Victor Chen            Ref. code
 *  1. 2005.9.8  Gushun Chen     0.1.000    Initial
 *
 ****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>

#include <hld/smc/smc.h>

//#define SMC_RW_DEBUG
#ifdef SMC_RW_DEBUG
#define SMC_RW_PRINTF	libc_printf
#endif
/*
 * 	Name		:   smc_open()
 *	Description	:   Open a smc device
 *	Parameter	:	struct smc_device *dev		: Device to be openned
 *	Return		:	INT32 						: Return value
 *
 */
INT32 smc_open(struct smc_device *dev, void (*callback)(UINT32 param))
{
	INT32 result = SUCCESS;

	/* If openned already, exit */
	if (dev->flags & HLD_DEV_STATS_UP)
	{
		PRINTF("smc_open: warning - device %s openned already!\n", dev->name);
		return SUCCESS;
	}

	/* Open this device */
	if (dev->open)
	{
		result = dev->open(dev, callback);
	}

	/* Setup init work mode */
	if (result == SUCCESS)
	{
		dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

	}

	return result;
}

/*
 * 	Name		:   smc_close()
 *	Description	:   Close a smc device
 *	Parameter	:	struct smc_device *dev		: Device to be closed
 *	Return		:	INT32 						: Return value
 *
 */
INT32 smc_close(struct smc_device *dev)
{
	INT32 result =  SUCCESS;

	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		PRINTF("smc_close: warning - device %s closed already!\n", dev->name);
		return SUCCESS;
	}

	/* Close device */
	if (dev->close)
	{
		result = dev->close(dev);
	}

	/* Update flags */
	dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

	return result;
}

/*
 * 	Name		:   smc_card_exist()
 *	Description	:   Reset smart card
 *	Parameter	:	struct smc_device *dev		: Device structuer
 *	Return		:	INT32 						: SUCCESS or FAIL
 *
 */
INT32 smc_card_exist(struct smc_device *dev)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->card_exist)
	{
		return dev->card_exist(dev);
	}
}

/*
 * 	Name		:   smc_reset()
 *	Description	:   Reset smart card
 *	Parameter	:	struct smc_device *dev		: Device structuer
 *					UINT8 *buffer				: Buffer pointer
 *					UINT16 *atr_size				: Size of ATR
 *	Return		:	INT32 						: SUCCESS or FAIL
 *
 */
INT32 smc_reset(struct smc_device *dev, UINT8 *buffer, UINT16 *atr_size)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->reset)
	{
		return dev->reset(dev, buffer, atr_size);
	}
}

/*
 * 	Name		:   smc_reset()
 *	Description	:   Reset smart card
 *	Parameter	:	struct smc_device *dev		: Device structuer
 *					UINT8 *buffer				: Buffer pointer
 *					UINT16 *atr_size				: Size of ATR
 *	Return		:	INT32 						: SUCCESS or FAIL
 *
 */
INT32 smc_deactive(struct smc_device *dev)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->deactive)
	{
		return dev->deactive(dev);
	}

	return SUCCESS;
}

/*
 * 	Name		:   smc_raw_read()
 *	Description	:   Read data from smart card
 *	Parameter	:	struct smc_device *dev			: Device structuer
 *					UINT8 *buffer					: Buffer pointer
 *					INT16 size					: Number of read in data
  *					INT16 *actlen					: Number of actual read in data
 *	Return		:	INT32 						: SUCCESS or FAIL
 *
 */
INT32 smc_raw_read(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->raw_read)
	{
		INT32 rlt;
		rlt = dev->raw_read(dev, buffer, size, actlen);
	#ifdef SMC_RW_DEBUG
		INT16 len = *actlen;
		INT16 k;
		size = *actlen;
		if(len>5)
			len = 5;
		SMC_RW_PRINTF("R: ");
		for(k=0; k<len; k++)
			SMC_RW_PRINTF("%02x ", buffer[k]);
		if(size>5)
			SMC_RW_PRINTF(". . . %02x %02x %02x ", buffer[size-3], buffer[size-2], buffer[size-1]);
		SMC_RW_PRINTF("\n");
	#endif
		return rlt;
	}

	return SUCCESS;
}

/*
 * 	Name		:   smc_raw_write()
 *	Description	:   Write data to smart card
 *	Parameter	:	struct smc_device *dev			: Device structuer
 *					UINT8 *buffer					: Buffer pointer
 *					INT16 size					: Number of write out data
 *					INT16 *actlen					: Number of actual write out data 
 *	Return		:	INT32 						: SUCCESS or FAIL
 *
 */
INT32 smc_raw_write(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->raw_write)
	{
	#ifdef SMC_RW_DEBUG
		INT16 len = size;
		INT16 k;
		if(len>5)
			len = 5;
		SMC_RW_PRINTF("W: ");
		for(k=0; k<len; k++)
			SMC_RW_PRINTF("%02x ", buffer[k]);
		if(size>5)
			SMC_RW_PRINTF(". . . %02x %02x %02x ", buffer[size-3], buffer[size-2], buffer[size-1]);
		SMC_RW_PRINTF("\n");
	#endif
		return dev->raw_write(dev, buffer, size, actlen);
	}

	return SUCCESS;
}

INT32 smc_raw_fifo_write(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->raw_fifo_write)
	{
		return dev->raw_fifo_write(dev, buffer, size, actlen);
	}

	return SUCCESS;
}
/*
 * 	Name		:   smc_iso_transfer()
 *	Description	:   Combines the functionality of both write and read.
 *					Implement ISO7816-3 command transfer. 
 *	Parameter	:	struct smc_device *dev		: Device structuer
 *					UINT8 *command				: ISO7816 command buffer pointer
 *					INT16 num_to_write			: Number to write 
 *					UINT8 *response, 				: Response data buffer pointer
 *					INT16 num_to_read			: Number to read
 *					INT16 *actual_size				: actual size got
 *	Return		:	INT32 						: SUCCESS or FAIL
 *
 */
INT32 smc_iso_transfer(struct smc_device *dev, UINT8 *command, INT16 num_to_write, UINT8 *response, INT16 num_to_read, INT16 *actual_size)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->iso_transfer)
	{
		return dev->iso_transfer(dev, command, num_to_write, response, num_to_read, actual_size);
	}
}

/*
 * 	Name		:   smc_iso_transfer_t1()
 *	Description	:   Combines the functionality of both write and read.
 *					Implement ISO7816-3 command transfer. 
 *	Parameter	:	struct smc_device *dev		: Device structuer
 *					UINT8 *command				: ISO7816 command buffer pointer
 *					INT16 num_to_write			: Number to write 
 *					UINT8 *response, 				: Response data buffer pointer
 *					INT16 num_to_read			: Number to read
 *					INT32 *actual_size				: Actually returned data size
 *	Return		:	INT32 						: SUCCESS or FAIL
 *
 */
INT32 smc_iso_transfer_t1(struct smc_device *dev, UINT8 *command, INT16 num_to_write, UINT8 *response, INT16 num_to_read,INT32 *actual_size)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->iso_transfer_t1)
	{
		return dev->iso_transfer_t1(dev, command, num_to_write, response, num_to_read, actual_size);
	}
}

/*
 * 	Name		:   smc_iso_transfer_t14()
 *	Description	:   Combines the functionality of both write and read.
 *					Implement ISO7816-3 command transfer. 
 *	Parameter	:	struct smc_device *dev		: Device structuer
 *					UINT8 *command				: ISO7816 command buffer pointer
 *					INT16 num_to_write			: Number to write 
 *					UINT8 *response, 				: Response data buffer pointer
 *					INT16 num_to_read			: Number to read
 *					INT32 *actual_size				: Actually returned data size
 *	Return		:	INT32 						: SUCCESS or FAIL
 *
 */
INT32 smc_iso_transfer_t14(struct smc_device *dev, UINT8 *command, INT16 num_to_write, UINT8 *response, INT16 num_to_read,INT32 *actual_size)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->iso_transfer_t1)
	{
		return dev->iso_transfer_t1(dev, command, num_to_write, response, num_to_read, actual_size);
	}
}

/*
 * 	Name		:   smc_io_control()
 *	Description	:   Smart card control command
 *	Parameter	:	struct sto_device *dev		: Device
 *					INT32 cmd					: IO command
 *					UINT32 param				: Param
 *	Return		:	INT32 						: Result
 *
 */
INT32 smc_io_control(struct smc_device *dev, INT32 cmd, UINT32 param)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->do_ioctl)
	{
		return dev->do_ioctl(dev, cmd, param);
	}

	return SUCCESS;
}

INT32 smc_t1_transfer(struct smc_device*dev, UINT8 dad, const void *snd_buf, UINT32 snd_len, void *rcv_buf, UINT32 rcv_len)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->t1_transfer)
	{
		return dev->t1_transfer(dev,dad, snd_buf, snd_len, rcv_buf, rcv_len);
	}
}

INT32 smc_t1_xcv(struct smc_device *dev, UINT8*sblock, UINT32 slen, UINT8 *rblock, UINT32 rmax, UINT32 *ractual)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->t1_xcv)
	{
		return dev->t1_xcv(dev,sblock, slen,rblock, rmax, ractual);
	}
}

INT32 smc_t1_negociate_ifsd(struct smc_device*dev, UINT32 dad, INT32 ifsd)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}
	if (dev->t1_negociate_ifsd)
	{
		return dev->t1_negociate_ifsd(dev,dad,ifsd);
	}
}

