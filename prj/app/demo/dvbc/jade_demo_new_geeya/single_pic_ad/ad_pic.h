#ifndef __AD_PIC_H__
#define __AD_PIC_H__

//#include <hld/osd/osddrv_dev.h>
#include <hld/osd/osddrv_dev.h>
/*
typedef struct
{
	UINT32 file;
	UINT8 *buf;  //void *buf;
	UINT32 len;
	struct OSDRect rect;
} ad;

*/

typedef struct
{
	UINT8 flag; //1 means the pic is showing, 0 means no pic is showing
	struct OSDRect rect;
} AD_FILE;

void ad_pic_init(UINT8 type);
void ad_pic_exit();
static void ad_np_scale();
static void ad_pic_show(UINT8 num,UINT8 *buf,UINT32 len,struct OSDRect rect);
void show_flash_pic(UINT8 num);
void close_ad_pic(UINT8 type);
int show_channel_ad(UINT8 adv_type,UINT16 sid ,UINT8 *pic_type);

#endif

