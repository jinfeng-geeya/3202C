/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *  (C)
 *  File: gif_out.c
 *  (I)
 *  Description: Output gif image to Display device, such as osd handle
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2008.1.16			Sam			Create
 * 1. 	2008.9.23			Sam			Store the internal buf as the 32bit data when 16bit osd	
 ****************************************************************************/

#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <os/tds2/itron.h>
#include <api/libc/printf.h>
#include <osal/osal_timer.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/osd/osddrv.h>
#include <api/libmp/gif.h>
#include "gif_main.h"
#include "gif_out.h"
#include "gif_lzw.h"
#include "gif_debug.h"

extern UINT8 g_gif_dec_first_pic;

extern GIF_RET (*g_gif_out_init)(struct gif_context *,struct gif_dec_out_par *);
extern GIF_RET (*g_gif_out_enter)(struct gif_out *,struct gif_lzw *);
extern void (*g_gif_out_img)(struct gif_context *);

static void gif_out_img(struct gif_context *text)
{
	struct gif_core_buf *buf = &(text->core.buf);
	struct gif_out *out = text->out;
	struct gif_core *core = &text->core;
	struct gif_info *info = text->core.info;
	struct gif_lzw *lzw = text->core.lzw;
	gif_rect *area = &out->out_par.area;

	struct osd_device *osd = (struct osd_device *)out->out_par.dev;
	struct OSDRect rect;
	UINT8 region_id = (UINT8)out->out_par.region;
	VSCR source;

	rect.uLeft = out->out_par.pos.x + lzw->rect.x;
	rect.uTop = out->out_par.pos.y + lzw->rect.y;
	rect.uWidth = lzw->rect.w;
	rect.uHeight = lzw->rect.h;

	MEMSET((void *)&source,0,sizeof(VSCR));
	
	if(lzw->rect.w + lzw->rect.x > area->w)
		rect.uWidth = area->w - lzw->rect.x;
	if(lzw->rect.h + lzw->rect.y > area->h)
		rect.uHeight = area->h - lzw->rect.y;
	if((lzw->rect.x < area->w) && (lzw->rect.y < area->h))
	{
		source.vR.uLeft = rect.uLeft;
		source.vR.uTop = rect.uTop;
		source.vR.uWidth = lzw->rect.w;
		source.vR.uHeight = lzw->rect.h;
		source.lpbScr = (UINT8 *)lzw->line_buf;
		OSDDrv_RegionWrite((HANDLE)osd,region_id,&source,&rect);
	}
	
	lzw->sum_lines = 0;
	lzw->pixel_num = 0;
	lzw->tick_start = osal_get_tick();
	lzw->delay_num = ((info->gce.dy_time_high<<8) | (info->gce.dy_time_low)) * 10 + GIF_DELAY_EXTENTION;
	if(0 == lzw->delay_num)
		lzw->delay_num = GIF_DELAY_MIN_VALUE;
	GIF_PRINTF("out put width %d height %d delay %d\n",lzw->rect.w,lzw->rect.h,lzw->delay_num);
}

static GIF_RET gif_out_enter(struct gif_out *out,struct gif_lzw *lzw)
{
	struct osd_device *osd = (struct osd_device *)out->out_par.dev;
	struct OSDRect rect;
	UINT8 region_id = (UINT8)out->out_par.region;
	
	if(out->update)
	{
		out->update = 0;		
		OSDDrv_GetRegionPos((HANDLE)osd,region_id,&rect);
		OSDDrv_ShowOnOff((HANDLE)osd,FALSE);
		OSDDrv_SetPallette((HANDLE)osd,(UINT8 *)lzw->pallete.local_abs_pal,(UINT16)lzw->pallete.num,OSDDRV_YCBCR);
		rect.uLeft = rect.uTop = 0;
		OSDDrv_RegionFill((HANDLE)osd,region_id,&rect,GIF_DEFAULT_TRAN_COLOR);
		OSDDrv_ShowOnOff((HANDLE)osd,TRUE);
	}
	return GIF_TRUE;
}

static GIF_RET gif_out_init(struct gif_context * text,struct gif_dec_out_par *par)
{
	GIF_RET ret = GIF_FALSE;
	struct gif_core_buf *buf = &(text->core.buf);
	struct gif_out *out = NULL;
	INT32 len = (sizeof(struct gif_out) + 3) & ~3;

	if(buf->valid)
	{
		if((buf->size -= len) >= 0)
		{
			text->out = (struct gif_out *)buf->start;
			out = text->out;
			buf->start += len;
			MEMSET((void *)text->out,0,len);
			GIF_PRINTF("out len %x addr %x free size %x\n",len,(UINT32)out,buf->size);
			MEMCPY((void *)&out->out_par,(void *)par,sizeof(struct gif_dec_out_par));
			if(GIF_TRUE == g_gif_dec_first_pic)
				ret = GIF_TRUE;
			out->frmt = GIF_OUT_8BITS;
		}
	}
	
	if(GIF_FALSE == ret)
		gif_core_set_error(text,GIF_ERR_OUT_INIT);
	
	return ret;
}

void gif_out_init_osd(void)
{
	g_gif_out_init = gif_out_init;
	g_gif_out_enter = gif_out_enter;
	g_gif_out_img = gif_out_img;
}

