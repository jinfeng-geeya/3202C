/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    rfm_dev.h
*
*    Description:    This file contains rfm_device structure define in HLD.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Aug.14.2003      Justin Wu       Ver 0.1    Create file.
*****************************************************************************/

#ifndef __HLD_RFM_DEV_H__
#define __HLD_RFM_DEV_H__

#include <types.h>
#include <hld/hld_dev.h>

/* IO control command */
enum rfm_device_ioctrl_command
{
	RFM_DRIVER_ATTACH	= 0,		/* RF modulator driver attach command */
	RFM_DRIVER_SUSPEND	= 1,		/* RF modulator driver suspend command */
	RFM_DRIVER_RESUME	= 2,		/* RF modulator driver resume command */
	RFM_DRIVER_DETACH	= 3,		/* RF modulator driver detach command */
};

struct rfm_device_stats
{
	UINT32 frequence;
};

/*
 *  Structure rfm_device, the basic structure between HLD and LLD of RF
 *  modulator device.
 */

struct rfm_device
{
	struct hld_device   *next;		/* Next device structure */
	UINT32		type;						/* Interface hardware type */
	INT8		name[HLD_MAX_NAME_SIZE];	/* Device name */

	UINT16		flags;				/* Interface flags, status and ability */

	/* Hardware privative structure */
	void		*priv;				/* pointer to private data */

	UINT32		base_addr;			/* Device base address */
	UINT32		system;				/* Output system mode in RF modulator */
	UINT32		channel;			/* Output channel */
	UINT32		step;
	UINT16		base_ch;
	UINT32		base_freq;
	UINT32		lo_low:1;
	UINT32 		i2c_type_id_cfg:1;
	UINT32		reserved:30;
	UINT32		i2c_type_id;
/*
 *  Functions of this RF modulator device
 */
	INT32	(*init)();

	INT32	(*open)(struct rfm_device *dev);

	INT32	(*stop)(struct rfm_device *dev);

	INT32	(*do_ioctl)(struct rfm_device *dev, INT32 cmd, UINT32 param);

	INT32	(*system_set)(struct rfm_device *dev, UINT32 system);

	INT32 	(*channel_set)(struct rfm_device *dev, UINT16 channel);

	struct rfm_device_stats* (*get_stats)(struct rfm_device *dev);
};

#endif  /* __HLD_RFM_DEV_H__ */
