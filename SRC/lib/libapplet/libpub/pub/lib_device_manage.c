#include <sys_config.h>
#include <basic_types.h>
#include <retcode.h>
#include <mediatypes.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>
#include <api/libnim/lib_nim.h>
#include <api/libpub/lib_device_manage.h>

/*******************************************************
* macro define
********************************************************/
#define DEV_FRONTEND_MAX_CNT	2
#define DEV_VDEC_MAX_CNT		2
#define DEV_DMX_MAX_CNT		3
#define DEV_MAX_CNT			(DEV_FRONTEND_MAX_CNT+DEV_VDEC_MAX_CNT+DEV_DMX_MAX_CNT)

/*device sub type define*/
#define DEV_MANAG_TYPE_MASK	0x0000FF00
#define DEV_MANAG_TYPE_NULL	0x00


/*******************************************************
* extern  declare
********************************************************/


/*******************************************************
* struct and variable
********************************************************/
struct dev_manag_dev
{
    	void *dev_handle;
    	/*device type HLD_DEV_TYPE*/
    	UINT32 hld_type;
    	/*lib device manage defined subtype*/
    	UINT32 sub_type;
    	/*device manage status*/
    	UINT32 status;
    	/*device config param or pointer to param struct*/
    	UINT32 config;
};


/*******************************************************
* variable and array
********************************************************/
UINT16 dev_manag_sema_id = OSAL_INVALID_ID;
#define ENTER_DEV_MUTEX()  osal_semaphore_capture(dev_manag_sema_id,TMO_FEVR);
#define LEAVE_DEV_MUTEX()  osal_semaphore_release(dev_manag_sema_id)

static struct dev_manag_dev dev_manag_list[DEV_MAX_CNT];
static struct nim_config dev_nim_info[DEV_FRONTEND_MAX_CNT];
static struct dmx_config dev_dmx_info[DEV_DMX_MAX_CNT];


/*******************************************************
* internal function
********************************************************/


/*******************************************************
* external API
********************************************************/
/* Select NIM Device from Device List */
struct nim_device* dev_select_nim_ext(UINT32 sub_type, UINT32 status, device_match_func match_func)
{
    	UINT8 i;
    	struct dev_manag_dev *dev_list = NULL;
    	struct nim_device *ret_dev = NULL;

    	ENTER_DEV_MUTEX();
    	dev_list = &dev_manag_list[0];
    	for(i=0; i <DEV_MAX_CNT; i++)
    	{
        	if((dev_list[i].hld_type==HLD_DEV_TYPE_NIM)&&(dev_list[i].sub_type==sub_type)&&(dev_list[i].status&status))
        	{
            		//Check whether NIM Config Info Match or not
            		if((match_func != NULL)&&(TRUE==match_func(dev_list[i].dev_handle)))
                		ret_dev = (struct nim_device*)dev_list[i].dev_handle;
            		else if(match_func == NULL)
                		ret_dev = (struct nim_device*)dev_list[i].dev_handle;
            		break;
        	}
    	}
    	LEAVE_DEV_MUTEX();
    	return ret_dev;
}

/* Select NIM Device from Device List */
struct nim_device* dev_select_nim(UINT32 sub_type, UINT32 status, struct nim_config *config)
{
    	UINT8 i;
    	struct dev_manag_dev *dev_list = NULL;
    	struct nim_device *ret_dev = NULL;
    	struct nim_config *pnim_info = NULL;

    	ENTER_DEV_MUTEX();
    	dev_list = &dev_manag_list[0];
    	for(i=0; i < DEV_MAX_CNT; i++)
    	{
        	if((dev_list[i].hld_type==HLD_DEV_TYPE_NIM)&&(dev_list[i].sub_type==sub_type)&&(dev_list[i].status&status))
        	{
            		//Check whether NIM Config Info Match or not
            		if((config!=NULL)&&(dev_list[i].config!=0))
            		{
                		pnim_info = (struct nim_config *)(dev_list[i].config);
                		if(0==MEMCMP(config, pnim_info, sizeof(struct nim_config)))
                    			ret_dev = (struct nim_device*)dev_list[i].dev_handle;
            		}
            		else
                		ret_dev = (struct nim_device*)dev_list[i].dev_handle;
           		break;
		}
    	}
    	LEAVE_DEV_MUTEX();   
    	return ret_dev;
}

/* Get NIM Device Config Info */    
INT32 dev_get_nim_config(struct nim_device *nim, UINT32 sub_type, struct nim_config *config)
{
    	INT32 ret = -1;
    	UINT8 i;
    	struct dev_manag_dev *dev_list = NULL;
    
    	ENTER_DEV_MUTEX();
    	dev_list = &dev_manag_list[0];
    	for(i=0; i < DEV_MAX_CNT; i++)
    	{
        	if((dev_list[i].hld_type==HLD_DEV_TYPE_NIM)&&(sub_type==dev_list[i].sub_type)
            		&&(nim==(struct nim_device *)dev_list[i].dev_handle))
        	{
            		if((config!=NULL)&&(dev_list[i].config!=0))
                		MEMCPY(config,(UINT8 *)dev_list[i].config, sizeof(struct nim_config));
            		ret = SUCCESS;
            		break;
        	}
    	}
    	LEAVE_DEV_MUTEX();
    	return ret;
}

/* Set NIM Device Config Info */
INT32 dev_set_nim_config(struct nim_device *nim, UINT32 sub_type, struct nim_config *config)
{
    	INT32 ret = -1;
    	UINT8 i;
    	struct dev_manag_dev *dev_list = NULL;
    
    	ENTER_DEV_MUTEX();
    	dev_list = &dev_manag_list[0];
    	for(i=0; i < DEV_MAX_CNT; i++)
    	{
        	if((dev_list[i].hld_type==HLD_DEV_TYPE_NIM)&&(sub_type==dev_list[i].sub_type)
            		&&(nim==(struct nim_device *)dev_list[i].dev_handle))
        	{
           		if((config!=NULL)&&(dev_list[i].config!=0))
                		MEMCPY((UINT8 *)dev_list[i].config, config,sizeof(struct nim_config));
            		ret = SUCCESS;
            		break;
        	}
    	}
    	LEAVE_DEV_MUTEX();
    	return ret;
}

/* Get Dmx Device Config Info */
INT32 dev_get_dmx_config(struct dmx_device *dmx, UINT32 sub_type, struct dmx_config *config)
{
    	INT32 ret = -1;
    	UINT8 i;
    	struct dev_manag_dev *dev_list = NULL;
    
    	ENTER_DEV_MUTEX();
    	dev_list = &dev_manag_list[0];
    	for(i=0; i < DEV_MAX_CNT; i++)
    	{
        	if((dev_list[i].hld_type==HLD_DEV_TYPE_DMX)&&(sub_type==dev_list[i].sub_type)
            		&&(dmx==(struct dmx_device *)dev_list[i].dev_handle))
        	{
			if((config!=NULL)&&(dev_list[i].config!=0))
            		{
            			//For DVB-S, Get Dmx Config Info From TS Route Info
		   		//if(sub_type == FRONTEND_TYPE_S)
		   		{
			 		_ts_route_get_dmx_config(dmx, (UINT8 *)dev_list[i].config);
		   		}
                		MEMCPY(config,(UINT8 *)dev_list[i].config, sizeof(struct dmx_config));
            		}
            		ret = SUCCESS;
            		break;
        	}
    	}
    	LEAVE_DEV_MUTEX();
    	return ret;
}

/* Get Device Status */
INT32 dev_get_device_status(UINT32 hld_type, UINT32 sub_type, void *dev, UINT32 *status)
{
    	INT32 ret = -1;
    	UINT8 i;
    	struct dev_manag_dev *dev_list = NULL;
    
    	ENTER_DEV_MUTEX();
    	dev_list = &dev_manag_list[0];
    	for(i=0; i < DEV_MAX_CNT; i++)
    	{
        	if((dev_list[i].hld_type==hld_type)&&(sub_type==dev_list[i].sub_type)
            		&&(dev==dev_list[i].dev_handle))
        	{
        		//For DVB-S, Get Device Status Info From TS Route Info
			//if(sub_type == FRONTEND_TYPE_S)
			{
				_ts_route_get_dev_state(hld_type, dev, &dev_list[i].status);
			}
            		*status = dev_list[i].status;
            		ret = SUCCESS;
            		break;
        	}
    	}
    	LEAVE_DEV_MUTEX();
    	return ret;
}

/* Get Number of one special type Device*/ 
UINT8 dev_get_device_cnt(UINT32 hld_type, UINT32 sub_type)
{
    	UINT16 i;
    	UINT8 cnt = 0;
    	struct dev_manag_dev *dev_list = NULL;

    	ENTER_DEV_MUTEX();
    	dev_list = &dev_manag_list[0];
    	for(i=0;i < DEV_MAX_CNT; i++)
    	{
        	if((dev_list[i].hld_type==hld_type)&&(dev_list[i].sub_type==sub_type)
            		&&(dev_list[i].dev_handle!=NULL))
            	cnt++;
    	}
    	LEAVE_DEV_MUTEX();
    	return cnt;
}

/* Register one special type Device */
INT32 dev_register_device(void *dev, UINT32 hld_type,UINT8 id,UINT32 sub_type)
{
    	UINT8 i;
    	INT32 ret = -1;
    	struct dev_manag_dev *dev_list = NULL;

	//Device Config Info Initialization
    	if(dev_manag_sema_id==OSAL_INVALID_ID)
    	{
        	dev_manag_sema_id = osal_semaphore_create(1);
        	MEMSET(dev_manag_list, 0, sizeof(dev_manag_list));
        	MEMSET(dev_nim_info, 0, sizeof(dev_nim_info));
        	MEMSET(dev_dmx_info, 0, sizeof(dev_dmx_info));
   	}
    	if(dev_manag_sema_id==OSAL_INVALID_ID)
        	return ret;

    	ENTER_DEV_MUTEX();
    	dev_list = &dev_manag_list[0];
    	for(i=0;i < DEV_MAX_CNT; i++)
    	{
        	if((dev_list[i].hld_type==0)&&(dev_list[i].sub_type==0)
            		&&(dev_list[i].dev_handle==NULL))
        	{
            		dev_list[i].dev_handle = dev;
            		dev_list[i].hld_type = hld_type;
            		dev_list[i].sub_type = sub_type;
            		dev_list[i].status = DEV_STATUS_NULL;
            		if(hld_type==HLD_DEV_TYPE_NIM)
                		dev_list[i].config = (UINT32)&dev_nim_info[id];
            		else if(hld_type==HLD_DEV_TYPE_DMX)
                		dev_list[i].config = (UINT32)&dev_dmx_info[id];	
            		ret = 0;
            		break;
        	}
    	}
    	LEAVE_DEV_MUTEX();
    	return ret;
}

