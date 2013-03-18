#ifndef __AD_PIC_GIF_H__
#define __AD_PIC_GIF_H__

#include <hld/osd/osddrv_dev.h>
#include <api/libmp/gif.h>

void ad_pic_gif_init(void);
INT32 ad_pic_gif_draw(gif_file fh, UINT8* file, UINT32 file_len, struct OSDRect *rect, BOOL animation);
void ad_pic_gif_stop(gif_file file);
void ad_pic_gif_close(void);

#endif
