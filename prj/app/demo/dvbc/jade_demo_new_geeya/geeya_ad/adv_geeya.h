
#ifndef __ADV_GEEYA_H__
#define __ADV_GEEYA_H__

#ifdef  __cplusplus
extern "C" {
#endif

#include <basic_types.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <api/libchunk/chunk.h>


#include "GYAD_Interface.h"

#define	GYAD_LOCK_TIMEOUT		0xffffffff
#define GY_ADV_DMX_BUF_SIZE 	60*1024
#define AD_BUF_MAX_LEN			512*1024
#define FLASH_64K				64*1024  /*由于在 FLASH BootPic后面是AdvPic，为了不擦除后面数据块*/
#define SUMCHANNEL				7       /*频道总数*/
#define GYAD_MAX_SEC_SIZE	4096
#define GYAD_MAX_CHAN_NUM	3


typedef enum 
{
	AD_FREE = 0,
	AD_BUSY
}GYAD_STATUS;


typedef enum adv_pic_type{
	ADV_PIC_TYPE_IFRAME = 0,
	ADV_PIC_TYPE_JPG,
	ADV_PIC_TYPE_GIF,
	ADV_PIC_TYPE_BMP,
	ADV_PIC_TYPE_TXT,
	ADV_PIC_TYPE_UNKNOWN,
};

enum     /*广告位索引号*/
{
	SHOW_GYADV_LOGO = 1,
	SHOW_GYADV_MAIN_MENU,
	SHOW_GYADV_CHANNEL_CHANGE,
	SHOW_GYADV_ROLL_TXT,
	SHOW_GYADV_TYPE_MAX,
};



//regid and fid
typedef struct{
	GYADU8	sRegId;
	INT32   sFid;
	INT32	sCid;
}sRegId_Fid;



typedef struct adv_gy_ctx_tag{	
	UINT16  pid;
	UINT32 	timeout;
	struct dmx_device *dmx;
	INT32 cid;
	INT32 fid;
	INT32 ReqID;
	UINT8* buf;
} adv_gy_ctx;

struct ad_channel_status
{
	UINT8 enable;
	UINT32 tick;
	UINT32 timeout;
};
struct gyad_channel
{
	struct get_section_param get_sec_param;
	struct restrict mask_value;
	struct ad_channel_status status[MAX_MV_NUM];
	UINT8 flt_id;
	UINT8 req_id[MAX_MV_NUM];
	UINT8 sec_buf[GYAD_MAX_SEC_SIZE];
};





GYADErrorType_e GYSTBAD_GetBootAd(sGYAdInfo_t *pBootADInfo, GYADU8 u8BootADNum);

GYADVOID GYSTBAD_Start(GYADVOID);

GYADErrorType_e GYADSTB_GetAdInfo(sGYADRelativeInfo_t sAdRelInfo, sGYAdInfo_t *sAdInfo);

INT32 gyad_filter_callback(UINT8 req_id, UINT8* value, UINT8* mask, UINT8 mask_len, UINT16 pid, UINT8 wait_seconds);

GYADErrorType_e GYSTBAD_WriteNvram_Logo_Menu(GYADU8 *data, GYADU32 len);

#ifdef  __cplusplus
}
#endif

#endif

