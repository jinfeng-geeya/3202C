/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 - 2003 Copyright (C)
*
*    File:    sto.c
*
*    Description:    This file contains all functions definition
*		             of storage device driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	May.29.2003      Justin Wu       Ver 0.1    Create file.
*****************************************************************************/

#include <types.h>
#include <retcode.h>
#include <api/libc/printf.h>
#include <hld/hld_dev.h>

#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>


OSAL_ID sto_mutex_id = OSAL_INVALID_ID;

void sto_mutex_enter(void)
{
#ifdef STO_PROTECT_BY_MUTEX
	if(OSAL_INVALID_ID==sto_mutex_id)
	{
		sto_mutex_id = osal_mutex_create();
		ASSERT(sto_mutex_id!=OSAL_INVALID_ID);
	}
	osal_mutex_lock(sto_mutex_id, OSAL_WAIT_FOREVER_TIME);
#else
	osal_task_dispatch_off();
#endif
}

void sto_mutex_exit(void)
{
#ifdef STO_PROTECT_BY_MUTEX
	osal_mutex_unlock(sto_mutex_id);
#else
	osal_task_dispatch_on();
#endif
}


/*
 * 	Name		:   sto_open()
 *	Description	:   Open a storage device
 *	Parameter	:	struct sto_device *dev		: Device to be openned
 *	Return		:	INT32 						: Return value
 *
 */
INT32 sto_open(struct sto_device *dev)
{
	INT32 result = SUCCESS;

	/* If openned already, exit */
	if (dev->flags & HLD_DEV_STATS_UP)
	{
		//PRINTF("sto_open: warning - device %s openned already!\n", dev->name);
		return SUCCESS;
	}

	/* Open this device */
	if (dev->open)
	{
		result = dev->open(dev);
	}

	/* Setup init work mode */
	if (result == SUCCESS)
	{
		dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

	}

	return result;
}

/*
 * 	Name		:   sto_close()
 *	Description	:   Close a storage device
 *	Parameter	:	struct sto_device *dev		: Device to be closed
 *	Return		:	INT32 						: Return value
 *
 */
INT32 sto_close(struct sto_device *dev)
{
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		PRINTF("sto_close: warning - device %s closed already!\n", dev->name);
		return SUCCESS;
	}

	/* close device */
	if (dev->close)
	{
		dev->close(dev);
	}

	/* Update flags */
	dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);

	return SUCCESS;
}

/*
 * 	Name		:   sto_lseek()
 *	Description	:   Long seek current operation point
 *	Parameter	:	struct sto_device *dev		: Device
 *					INT32 offset				: Offset of seek
 *					int start					: Start base position
 *	Return		:	INT32 						: Postion
 *
 */
INT32 sto_lseek(struct sto_device *dev, INT32 offset, int origin)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->lseek)
	{
		return dev->lseek(dev, offset, origin);
	}

	return SUCCESS;
}

/*
 * 	Name		:   sto_write()
 *	Description	:   Write data into storage
 *	Parameter	:	struct sto_device *dev		: Device
 *					UINT8 *data					: Data to be write
 *					UINT32 len					: Data length
 *	Return		:	INT32 						: Write data length
 *
 */
INT32 sto_write(struct sto_device *dev, UINT8 *data, INT32 len)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->write)
	{
		return dev->write(dev, data, len);
	}

	return SUCCESS;
}

/*
 * 	Name		:   sto_read()
 *	Description	:   Read data from storage
 *	Parameter	:	struct sto_device *dev		: Device
 *					UINT8 *data					: Data read out
 *					UINT32 len					: Data length
 *	Return		:	INT32 						: Read data length
 *
 */
INT32 sto_read(struct sto_device *dev, UINT8 *data, INT32 len)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->read)
	{
		return dev->read(dev, data, len);
	}

	return SUCCESS;
}

/*
 * 	Name		:   sto_io_control()
 *	Description	:   Do IO control
 *	Parameter	:	struct sto_device *dev		: Device
 *					INT32 cmd					: IO command
 *					UINT32 param				: Param
 *	Return		:	INT32 						: Result
 *
 */
INT32 sto_io_control(struct sto_device *dev, INT32 cmd, UINT32 param)
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

/*
 * 	Name		:   sto_put_data()
 *	Description	:   Write data into storage
 *	Parameter	:	struct sto_device *dev		: Device
 *					UINT8 *data					: Data to be write
 *					UINT32 len					: Data length
 *	Return		:	INT32 						: Write data length
 *
 */
INT32 sto_put_data(struct sto_device *dev, UINT32 offset, UINT8 *data, INT32 len)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->put_data)
	{
		return dev->put_data(dev, offset, data, len);
	}

	return SUCCESS;
}

/*
 * 	Name		:   sto_get_data()
 *	Description	:   Read data from storage
 *	Parameter	:	struct sto_device *dev		: Device
 *					UINT32 len					: Data length
 *					UINT8 *data					: Data to be read
 *	Return		:	INT32 						: Read data length
 *
 */
INT32 sto_get_data(struct sto_device *dev, UINT8 *data, UINT32 offset, INT32 len)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return ERR_DEV_ERROR;
	}

	if (dev->get_data)
	{
		return dev->get_data(dev, data, offset, len);
	}

	return SUCCESS;
}
