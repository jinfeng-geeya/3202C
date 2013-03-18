/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: dmx.c
 *
 *  Description: This file define the functions for dmx device management.
 *
 *  History:
 *      Date        Author         Version   Comment
 *      ====        ======         =======   =======
 *  1.  2002.12.19  David Wang     0.1.000   Initial
 *  2.  2003.02.11  David Wang	   0.1.001   Modification for new sw tree
 *  3.  2003.03.07  Zhengdao Li      0.1.002   Modifications in dmx_dev_alloc
 *  4.  2004.04.05  Goliath Peng      0.1.003   Add some new api in hld.
 *  5.  2004.07.18  Goliath Peng      0.1.004   Do some modification in HLD APIs.
 *  6.  2004.08.10  Goliath Peng      0.1.005   Merge the new hld code with old one.
 ****************************************************************************/
#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
/*
str_type: DES_VIDEO
		DES_AUDIO
		DES_TTX //teletext
		DES_SUP //sub title

cw_type: 1 default cw
		2 even cw
		3 odd cw
*/
RET_CODE dmx_cfg_cw(struct dmx_device * dev, enum DES_STR str_type, UINT8 cw_type, UINT32 * cw)
{
	if(NULL!=dev&&NULL!=dev->cfg_cw)
		return dev->cfg_cw(dev, str_type, cw_type, cw);
	return !RET_SUCCESS;
}

RET_CODE dmx_register_service(struct dmx_device * dev, UINT8 filter_idx, struct register_service * reg_serv)
{
	if(NULL == dev)return !RET_SUCCESS;

	if(dev->flags == DMX_STATE_ATTACH)
	{
		//DMX_PRINTF("dmx_register_service: warning -  device %s is not open!\n", dev->name);
		return !RET_SUCCESS;
	}
	if(NULL != dev->register_service) 
	{
		return dev->register_service(dev, filter_idx, reg_serv);
	}
	return !RET_SUCCESS;
}

RET_CODE dmx_unregister_service(struct dmx_device * dev, UINT8 filter_idx)
{
	if(NULL == dev)return !RET_SUCCESS;

	if (dev->flags == DMX_STATE_ATTACH)
	{
		//DMX_PRINTF("dmx_unregister_service: warning -  device %s is not open!\n", dev->name);
		return !RET_SUCCESS;
	}
	if(NULL != dev->unregister_service) 
	{
		return dev->unregister_service(dev, filter_idx);
	}
	return !RET_SUCCESS;
}
__ATTRIBUTE_REUSE_
RET_CODE dmx_open(struct dmx_device *dev)
{
	if(dev == NULL)return !RET_SUCCESS;   
       
	if(dev->flags != DMX_STATE_ATTACH)
	{
		//DMX_PRINTF("dmx_open: warning - device %s is not in ATTACH status!\n", dev->name);
		return !RET_SUCCESS;
	}
	/* Open this device */
	if (NULL!=dev->open)
	{
		return dev->open(dev);
	}
	return  !RET_SUCCESS;
}

/*
 * 	Name		:   demxu_close()
 *	Description	:   Close a dmx device
 *	Parameter	:	struct dmx_device *dev		: Device to be closed
 *	Return		:	RET_CODE 						: Return value
 *
 */
RET_CODE dmx_close(struct dmx_device *dev)
{
	if(dev == NULL)return !RET_SUCCESS;
       
	if (dev->flags != DMX_STATE_IDLE)
	{
		//DMX_PRINTF("dmx_close: warning - device %s is not in IDLE status!\n", dev->name);
		return !RET_SUCCESS;
	}
	/* close device */
	if ( NULL!= dev->close)
	{
		return dev->close(dev);
	}
	return !RET_SUCCESS;
}

/*
 * 	Name		:   dmx_control()
 *	Description	:   Close a dmx device
 *	Parameter	:	struct dmx_device *dev		: Device to be closed
 *	Return		:	RET_CODE 						: Return value
 *
 */
RET_CODE dmx_io_control(struct dmx_device *dev, UINT32 cmd, UINT32 param)
{
	if(NULL == dev)return !RET_SUCCESS;

	if(dev->flags == DMX_STATE_ATTACH)
	{
		//DMX_PRINTF("dmx_io_control: warning -  device %s is not open!\n", dev->name);
		return !RET_SUCCESS;
	}
	if(NULL != dev->ioctl) 
	{
		return dev->ioctl(dev, cmd, param);
	}
	return !RET_SUCCESS;
}

/*
 * 	Name		:   dmx_req_section()
 *	Description	:   Close a dmx device
 *	Parameter	:   struct dmx_device *dev		: Device to be closed
 *                              struct get_section_param *sec_param
 *	Return		:	RET_CODE 						: Return value
 *
 */
RET_CODE dmx_req_section(struct dmx_device *dev, struct get_section_param *sec_param)
{
	if(NULL == dev)return !RET_SUCCESS;
	if (dev->flags == DMX_STATE_ATTACH)
	{
		//DMX_PRINTF("dmx_req_section: warning -  device %s is not open!\n", dev->name);
		return !RET_SUCCESS;
	}
	if(NULL != dev->get_section)
	{
		return dev->get_section(dev,(void *)sec_param);
	}
	return !RET_SUCCESS;
}

RET_CODE dmx_async_req_section(struct dmx_device *dev, struct get_section_param *sec_param, UINT8 * flt_idx)
{
	if (NULL == dev)
		return !RET_SUCCESS;
	if (dev->flags == DMX_STATE_ATTACH)
	{
		//DMX_PRINTF("dmx_async_req_section: warning -  device %s is not open!\n", dev->name);
		return !RET_SUCCESS;
	}
	if(NULL != dev->async_get_section)
	{
		return dev->async_get_section(dev,(void *)sec_param, flt_idx);
	}
	return !RET_SUCCESS;
}

RET_CODE dmx_start(struct dmx_device *dev)
{
	if(NULL == dev)return !RET_SUCCESS;
     
	if ((dev->flags != DMX_STATE_IDLE)&&(dev->flags != DMX_STATE_PAUSE))
	{
		//DMX_PRINTF("dmx_start: warning -  device %s is not in IDLE or PAUSE status!\n", dev->name);
		return !RET_SUCCESS;
	}
	if(NULL != dev->start)
	{
		return dev->start(dev);
	}
    
	return !RET_SUCCESS;
}

RET_CODE dmx_stop(struct dmx_device *dev)
{
	if(NULL == dev)return !RET_SUCCESS;
     
	if ((dev->flags != DMX_STATE_PLAY)&&(dev->flags != DMX_STATE_PAUSE))
	{
		//DMX_PRINTF("dmx_stop: warning -  device %s is not in PLAY or PAUSE status!\n", dev->name);
		return !RET_SUCCESS;
	}
	if(NULL != dev->stop)
	{
		return dev->stop(dev);
	}
    
	return !RET_SUCCESS;
}

RET_CODE dmx_pause(struct dmx_device * dev)
{
	if(NULL == dev)return !RET_SUCCESS;
     
	if (dev->flags != DMX_STATE_PLAY)
	{
		//DMX_PRINTF("dmx_pause: warning -  device %s is not in PLAY status!\n", dev->name);
		return !RET_SUCCESS;
	}
	if(NULL != dev->pause)
	{
		return dev->pause(dev);
	}
    
	return !RET_SUCCESS;
}

RET_CODE dmx_register_service_new(struct dmx_device * dev, struct register_service_new * reg_serv)
{
	if(NULL == dev)return !RET_SUCCESS;

	if(dev->flags == DMX_STATE_ATTACH)
	{
		//DMX_PRINTF("dmx_register_service: warning -  device %s is not open!\n", dev->name);
		return !RET_SUCCESS;
	}
	if(NULL != dev->register_service_new) 
	{
		return dev->register_service_new(dev, reg_serv);
	}
	return !RET_SUCCESS;
}

RET_CODE dmx_unregister_service_new(struct dmx_device * dev, struct register_service_new * reg_serv)
{
	if(NULL == dev)return !RET_SUCCESS;

	if(dev->flags == DMX_STATE_ATTACH)
	{
		//DMX_PRINTF("dmx_register_service: warning -  device %s is not open!\n", dev->name);
		return !RET_SUCCESS;
	}
	if(NULL != dev->unregister_service_new) 
	{
		return dev->unregister_service_new(dev, reg_serv);
	}
	return !RET_SUCCESS;
}
