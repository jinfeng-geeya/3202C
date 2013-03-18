#ifndef __AD_PIC_PNG_H__
#define __AD_PIC_PNG_H__

#include <hld/osd/osddrv_dev.h>
#include <api/libimagedec/imagedec.h>
#include <api/libmp/png.h>

void ad_png_init(void);
INT32 ad_png_draw(UINT8* file, UINT32 file_len, struct OSDRect *rect);
void ad_png_close(void);

#endif
