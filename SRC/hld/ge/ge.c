/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: ge.c
 *
 *  Description: This file define the functions for graphic engine device management.
 *
 *  History:
 *      Date           Author             Version         Comment
 *      ====         ======         =======    =======
 *  1.  2005.10.17  Leimei Yang     0.1.000          Initial
  ****************************************************************************/
#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/ge/ge.h>

#include "ge_old.c"
#include "ge_new.c"

/*
 * 	Name		:   ge_io_control()
 *	Description	:   ge IO control command
 *	Parameter	:	struct ge_device *dev		:
 *					INT32 cmd					: IO command
 *					UINT32 param				: Param
 *	Return		:	INT32 						: Result
 *
 */
RET_CODE ge_io_ctrl(struct ge_device *dev,UINT32 dwCmd,UINT32 dwParam)
{
	if(NULL == dev)return RET_FAILURE;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->ioctl)
	{
		return dev->ioctl(dev,dwCmd,dwParam);
	}
	
	return RET_FAILURE;
}

/*
 * 	Name		:	ge_open()
 *	Description	:	Open a grpahic engine device
 *	Parameter	:	struct ge_device *dev: Device to be openned
 *	Return		:	RET_FAILURE 	: open device failure 
 *					RET_SUCCESS: open device success
 */
RET_CODE ge_open(struct ge_device *dev)
{
	INT32 result=RET_FAILURE;
	
	/* If openned already, exit */
	if (dev->flags & HLD_DEV_STATS_UP)
	{
		PRINTF("ge_open: warning - device %s openned already!\n", dev->name);
		return RET_SUCCESS;
	}
	
	/* Open this device */
	if (dev->open)
	{
		result = dev->open(dev);
	}
	
	/* Setup init work mode */
	if (result == RET_SUCCESS)
	{
		dev->flags |= (HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
		
	}
	
	return result;
}

/*
 * 	Name		:   ge_close()
 *	Description	:   Close a ge device
 *	Parameter	:   struct ge_device *dev		: Device to be closed
 *	Return		:   RET_FAILURE 	: open device failure 
 *				    RET_SUCCESS: open device success
 *
 */
RET_CODE ge_close(struct ge_device *dev)
{
	INT32 result=RET_FAILURE;

	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		PRINTF("ge_close: warning - device %s closed already!\n", dev->name);
		return RET_SUCCESS;
	}
	
	/* Stop device */
	if (dev->close)
	{
		result = dev->close(dev);
	}
	
	/* Update flags */
	dev->flags &= ~(HLD_DEV_STATS_UP | HLD_DEV_STATS_RUNNING);
	
	return result;
}

