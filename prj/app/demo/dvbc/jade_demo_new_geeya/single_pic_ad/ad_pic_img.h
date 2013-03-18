#ifndef __AD_PIC_JPG_H__
#define __AD_PIC_JPG_H__

#include <hld/osd/osddrv_dev.h>
#include <api/libimagedec/imagedec.h>

INT32 ad_pic_img_draw(UINT8* file, UINT32 file_len, struct OSDRect *rect);
void ad_pic_img_close(void);
void ad_pic_img_clear(void);
    
#endif

