#ifndef __AD_PIC_GFX_H__
#define __AD_PIC_GFX_H__

#include <hld/osd/osddrv_dev.h>
#include <hld/ge/ge.h>


#define AD_DST_SURFACE		0
#define AD_SRC_SURFACE		1

INT32 ad_pic_gfx_init(UINT8 type);
//INT32 ad_pic_gfx_draw(struct OSDRect *dst_rc, struct OSDRect *src_rc, UINT8 *buf, UINT16 pitch);
INT32 ad_pic_gfx_draw(struct OSDRect *rect, UINT8 *buf);
INT32 ad_pic_gfx_clear(UINT8 type);
INT32 ad_pic_gfx_exit(UINT8 type);
struct ge_surface_desc_t * ad_pic_get_ge_surface(UINT32 ad_surface);
UINT8 ad_pic_get_ge_region_id(void);
void ad_pic_show_on(void);

#endif

