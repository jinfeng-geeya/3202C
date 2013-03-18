#ifndef _DEV_HANDLE_H_
#define _DEV_HANDLE_H_

#include "upg_common.h"

extern struct pan_device  *g_pan_dev;
extern struct sto_device *g_sto_dev;
extern struct nim_device*nim_dev;
extern struct dmx_device *dmx_dev;
extern struct osd_device *osd_dev;

extern struct vdec_device*  g_decv_dev;
extern struct vpo_device*   g_vpo_dev;
extern struct vpo_device*   g_sd_vpo_dev;

extern struct ge_device	*g_ge_dev;

#ifdef NAND_FLASH_SUPPORT
extern struct ali_nand_device *g_nflash_dev;
#endif

#endif//_DEV_HANDLE_H_

