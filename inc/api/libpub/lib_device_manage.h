#ifndef _LIB_DEVICE_MANAGE_H_
#define _LIB_DEVICE_MANAGE_H_

#include <sys_config.h> 
#include <api/libpub/lib_frontend.h>

/*******************************************************
* macro define
********************************************************/
#define DEV_STATUS_NULL           0x00000000

//attribute, byte0:play or record; byte1~3 reserve
#define DEV_FRONTEND_STATUS_PLAY          0x00000001
#define DEV_FRONTEND_STATUS_RECORD      0x00000002

/*dmx device*/
#define DEV_DMX_STATUS_PLAY          0x00000001
#define DEV_DMX_STATUS_RECORD      0x00000002


/*******************************************************
* struct and define
********************************************************/
struct nim_config
{
    struct ft_antenna antenna;
    union ft_xpond xpond;
};

struct dmx_config
{
	UINT32 tp_id;
 	struct dmx_sim dmx_sim_info[DEV_DMX_SIM_CNT];
};

typedef BOOL (* device_match_func)(void* dev_handle);

/*******************************************************
* API
********************************************************/
struct nim_device* dev_select_nim_ext(UINT32 sub_type, UINT32 status,device_match_func match_func);
struct nim_device* dev_select_nim(UINT32 sub_type, UINT32 status, struct nim_config *config);
struct vdec_device* dev_select_vdec(UINT32 sub_type, UINT32 status, UINT32 param);
struct dmx_device* dev_select_dmx(UINT32 sub_type, UINT32 status, UINT32 param);

INT32 dev_get_nim_config(struct nim_device *nim, UINT32 sub_type, struct nim_config *config);
INT32 dev_set_nim_config(struct nim_device *nim, UINT32 sub_type, struct nim_config *config);
INT32 dev_get_dmx_config(struct dmx_device *dmx, UINT32 sub_type, struct dmx_config *config);
INT32 dev_set_dmx_config(struct dmx_device *dmx, UINT32 sub_type, struct dmx_config *config);
INT32 dev_get_device_status(UINT32 hld_type, UINT32 sub_type, void *dev, UINT32 *status);
INT32 dev_set_device_status(UINT32 hld_type, UINT32 sub_type, void *dev, UINT32 status);

UINT8 dev_get_device_cnt(UINT32 hld_type, UINT32 sub_type);
INT32 dev_register_device(void *dev, UINT32 hld_type,UINT8 id,UINT32 sub_type);




#endif

