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
#include <hld/ge/ge.h>
#include <api/libmp/gif.h>
#include "gif_main.h"
#include "gif_out.h"
#include "gif_lzw.h"
#include "gif_debug.h"

extern UINT8 g_gif_dec_first_pic;
extern int g_gif_ignore_bk_color;
extern UINT32 g_gif_trans_color;
extern int g_gif_trans_color_flag;

extern GIF_RET (*g_gif_out_init)(struct gif_context *,struct gif_dec_out_par *);
extern GIF_RET (*g_gif_out_enter)(struct gif_out *,struct gif_lzw *);
extern void (*g_gif_out_img)(struct gif_context *);

static ge_surface_desc_t *virtual_surface;
static ge_surface_desc_t *virtual_surface2;
static ge_surface_desc_t *virtual_surface3;
static ge_surface_desc_t *virtual_surface4;
static UINT8 vir_region_id = 0;

GIF_INLINE GIF_RET check_out_frmt(ge_region_pars_t *par,UINT8 *frmt)
{
	switch(par->pixel_fmt)
	{
		case GE_PF_AYCBCR8888:
			*frmt = GIF_OUT_32BITS;
			break;
		case GE_PF_ARGB1555:
			*frmt = GIF_OUT_32BITS;//GIF_OUT_16BITS;
			break;
		case GE_PF_CLUT8:
			*frmt = GIF_OUT_8BITS;
			break;			
			break;
		default:
			GIF_PRINTF("Gif don't support osd frmt %d\n",par->pixel_fmt);
			GIF_ASSERT(GIF_ERR_UNSUP_OSD_FORMAT);
			return GIF_FALSE;
	}
	
	return GIF_TRUE;
}

GIF_INLINE enum GE_PIXEL_FORMAT FRM_GIF2GE(enum GIF_OUT_FRMT frmt)
{
	enum GE_PIXEL_FORMAT ge_frmt = 0;
	
	switch(frmt)
	{
		case GIF_OUT_32BITS:
			ge_frmt = GE_PF_ARGB8888;//GE_PF_AYCBCR8888;			
			break;
		case GIF_OUT_16BITS:
			ge_frmt = GE_PF_ARGB1555;			
			break;
		case GIF_OUT_8BITS:
				ge_frmt = GE_PF_CLUT8;		
			break;
		default:
			GIF_ASSERT(GIF_ERR_UNSUP_OSD_FORMAT);
			ge_frmt = GE_PF_CLUT8;
	}

	return ge_frmt;
}
	
GIF_INLINE GIF_RET init_surface_par(struct gif_out *out)
{
	struct ge_device *ge = (struct ge_device *)out->out_par.dev;
	ge_surface_desc_t *surface = (ge_surface_desc_t *)out->out_par.handle;
	ge_region_pars_t reg_par;
	UINT8 frmt;

	if(RET_FAILURE == ge_get_region_pars(ge,surface,surface->cur_region,&reg_par))
	{
		return GIF_FALSE;
	}

	if(GIF_FALSE == check_out_frmt(&reg_par,&frmt))
	{
		return GIF_FALSE;
	}
	MEMCPY((void *)out->par1,(void *)surface,sizeof(ge_surface_desc_t));
	MEMCPY((void *)out->par2,(void *)surface,sizeof(ge_surface_desc_t));
	MEMCPY((void *)out->par3,(void *)surface,sizeof(ge_surface_desc_t));	
	MEMCPY((void *)out->par4,(void *)surface,sizeof(ge_surface_desc_t));
	((ge_surface_desc_t *)out->par1)->surf_id = GMA_MEM_SURFACE;
	((ge_surface_desc_t *)out->par2)->surf_id = GMA_MEM_SURFACE;
	((ge_surface_desc_t *)out->par3)->surf_id = GMA_MEM_SURFACE;	
	((ge_surface_desc_t *)out->par4)->surf_id = GMA_MEM_SURFACE;
	ge_set_cur_region(ge,(ge_surface_desc_t *)out->par1,vir_region_id);
	ge_set_cur_region(ge,(ge_surface_desc_t *)out->par2,vir_region_id);	
	ge_set_cur_region(ge,(ge_surface_desc_t *)out->par3,vir_region_id);		
	ge_set_cur_region(ge,(ge_surface_desc_t *)out->par4,vir_region_id);	
	if(reg_par.pixel_fmt != FRM_GIF2GE(frmt))
	{
		((ge_surface_desc_t *)out->par1)->region[vir_region_id].pixel_fmt = FRM_GIF2GE(frmt);
		((ge_surface_desc_t *)out->par2)->region[vir_region_id].pixel_fmt = FRM_GIF2GE(frmt);
		((ge_surface_desc_t *)out->par3)->region[vir_region_id].pixel_fmt = FRM_GIF2GE(frmt);		
		((ge_surface_desc_t *)out->par4)->region[vir_region_id].pixel_fmt = GE_PF_ARGB1555;
		if(GE_PF_AYCBCR8888 == reg_par.pixel_fmt)
			out->yuv2rgb = 1;
	}
	out->frmt = frmt;
	return GIF_TRUE;
}

static void gif_out_img(struct gif_context *text)
{
	struct gif_core_buf *buf = &(text->core.buf);
	struct gif_out *out = text->out;
	struct gif_core *core = &text->core;
	struct gif_info *info = text->core.info;
	struct gif_lzw *lzw = text->core.lzw;
	gif_rect *area = &out->out_par.area;
	struct gif_in *in = &text->in;

	struct ge_device *ge = (struct ge_device *)out->out_par.dev;
	ge_surface_desc_t *surface = (ge_surface_desc_t *)out->out_par.handle;
	ge_region_pars_t reg_par;
	ge_rect_t dst_rect,src_rect;
	
	UINT32 * local_pal = lzw->pallete.local_abs_pal;
	UINT32 tran_true_color = 0;
	UINT8 tran_flag = 0;
	UINT8 red = 0,green = 0,blue = 0;
	
	ge_color_key_t color_key;

	int disposal_m = 0;
	ge_rect_t disp_rect;
				
	ge_lock(ge);	

	virtual_surface = (ge_surface_desc_t *)out->par1;
	virtual_surface2 = (ge_surface_desc_t *)out->par2;
	virtual_surface3 = (ge_surface_desc_t *)out->par3;
	virtual_surface4 = (ge_surface_desc_t *)out->par4;

	if((lzw->img_id == 1) && (g_gif_dec_first_pic == FALSE))	
	{
		// fill the screen with background color
		dst_rect.width = (info->lsdes.w_high<<8) | info->lsdes.w_low;
		dst_rect.height = (info->lsdes.h_high<<8) | info->lsdes.h_low;
		dst_rect.left = 0;
		dst_rect.top = 0;
		tran_true_color = (UINT32)*(local_pal + info->lsdes.bk_color);

		reg_par.addr = (UINT8 *)lzw->bitmap; 
		reg_par.pitch = lzw->img_pitch;
		reg_par.pixel_fmt = FRM_GIF2GE(out->frmt);
		ge_set_region_pars(ge,virtual_surface,vir_region_id,&reg_par);
		if(in->frame_num > 1)
			ge_fill_rect(ge,virtual_surface,&dst_rect,0);
		else
			ge_fill_rect(ge,virtual_surface,&dst_rect,tran_true_color);	

		disposal_m = 1;
		MEMCPY((void *)&disp_rect, (void *)&dst_rect, sizeof(dst_rect));		
	}
	
	if(core->status.flag & GIF_INFO_GCE_PRIOR)
	{
		core->status.flag &= ~GIF_INFO_GCE_PRIOR;
		dst_rect.width = (out->prior_img.w_high<<8) | out->prior_img.w_low;
		dst_rect.height= (out->prior_img.h_high<<8) | out->prior_img.h_low;
		dst_rect.left = ((out->prior_img.x_high<<8) | out->prior_img.x_low);
		dst_rect.top = ((out->prior_img.y_high<<8) | out->prior_img.y_low);
		if(2 == out->prior_gce.disposal_m)
		{	
			tran_true_color = (UINT32)*(local_pal + out->prior_lsdes.bk_color);
			reg_par.addr = (UINT8 *)lzw->bitmap; 
			reg_par.pitch = lzw->img_pitch;
			reg_par.pixel_fmt = FRM_GIF2GE(out->frmt);
			ge_set_region_pars(ge,virtual_surface,vir_region_id,&reg_par);
			if(in->frame_num > 1)
				ge_fill_rect(ge,virtual_surface,&dst_rect,0);
			else
				ge_fill_rect(ge,virtual_surface,&dst_rect,tran_true_color);

			disposal_m = 1;
			MEMCPY((void *)&disp_rect, (void *)&dst_rect, sizeof(dst_rect));
			
			GIF_PRINTF("Disposal method 2 fill color %x idx %d\n",tran_true_color,out->prior_lsdes.bk_color);
		}
		else if(3 == out->prior_gce.disposal_m)
		{
			src_rect.left = 0;
			src_rect.top = 0;
			src_rect.width = dst_rect.width;
			src_rect.height = dst_rect.height;

			reg_par.addr = (UINT8 *)out->bak_pic; 
			reg_par.pitch = out->pitch;
			reg_par.pixel_fmt = FRM_GIF2GE(out->frmt);	
			ge_set_region_pars(ge,virtual_surface2,vir_region_id,&reg_par);
			reg_par.addr = (UINT8 *)lzw->bitmap; 
			reg_par.pitch = lzw->img_pitch;
			reg_par.pixel_fmt = FRM_GIF2GE(out->frmt);	
			ge_set_region_pars(ge,virtual_surface,vir_region_id,&reg_par);
			ge_blt(ge,virtual_surface,virtual_surface2,&dst_rect,&src_rect,GE_BLT_SRC2_VALID);

			buf->start -= out->size;
			buf->size += out->size;

			disposal_m = 1;
			MEMCPY((void *)&disp_rect, (void *)&dst_rect, sizeof(dst_rect));
			
			GIF_PRINTF("Disposal method 3 recover last image\n");			
		}
	}

	if(core->status.flag & GIF_INFO_GCE)
	{
		core->status.flag &= ~GIF_INFO_GCE;
		if(info->gce.alpha_flag)
		{
			tran_flag = 1;
			tran_true_color = (UINT32)*(local_pal + info->gce.alpha_value);				
			if(GIF_OUT_16BITS == out->frmt)
			{
				// transfer the 16 bit RGB555 to RGB888
				red = (tran_true_color & 0x7C00)>>7;
				red |= (red>>5)&0x7;
				green = (tran_true_color &  0x3e0)>>2;
				green |= (green>>5)&0x7;
				blue = (tran_true_color & 0x1F)<<3;
				blue |= (blue>>5)&0x07;
				tran_true_color = 0;
				tran_true_color |= (red<<16) | (green <<8) | blue;
			}
			if(GIF_FALSE == g_gif_dec_first_pic)
			{
				if(1 == lzw->img_id)
				{
					// clear the bitmap buf
					dst_rect.left = lzw->rect.x;
					dst_rect.top = lzw->rect.y;
					dst_rect.width = lzw->rect.w;
					dst_rect.height = lzw->rect.h;
					reg_par.addr = (UINT8 *)lzw->bitmap; 
					reg_par.pitch = lzw->img_pitch;
					reg_par.pixel_fmt = FRM_GIF2GE(out->frmt);	
					ge_set_region_pars(ge,virtual_surface,vir_region_id,&reg_par);	
					if(in->frame_num > 1)
						ge_fill_rect(ge,virtual_surface,&dst_rect,0);
					else
						ge_fill_rect(ge,virtual_surface,&dst_rect,out->bk_clor);
				}
			}
			GIF_PRINTF("Trans true color 32 bits %x\n",tran_true_color);
		}
		if(3 == info->gce.disposal_m)
		{
			out->width = (lzw->rect.w + 3) & ~3;
			out->height = lzw->rect.h;
			switch(out->frmt)
			{
				case GIF_OUT_32BITS:	
					out->pitch = out->width<<2;
					break;
				case GIF_OUT_16BITS:	
					out->pitch = out->width<<1;
					break;
				case GIF_OUT_8BITS:	
					out->pitch = out->width;
					break;	
				default:
					GIF_ASSERT(GIF_ERR_UNSUP_OSD_FORMAT);
					return;					
			}
			out->size = out->pitch * out->height;

			if(out->size <= buf->size)
			{
				out->bak_pic =buf->start;
				buf->size -= out->size;
				buf->start += out->size;
				GIF_PRINTF("Disposal_m3: addr %x size %x free %x\n",out->bak_pic,out->size,buf->size);
				
				dst_rect.left = dst_rect.top = 0;
				dst_rect.width = out->width;
				dst_rect.height = out->height;
				src_rect.left = lzw->rect.x;
				src_rect.top = lzw->rect.y;
				src_rect.width = lzw->rect.w;
				src_rect.height = lzw->sum_lines;	

				reg_par.addr = (UINT8 *)lzw->bitmap; 
				reg_par.pitch = lzw->img_pitch;
				reg_par.pixel_fmt = FRM_GIF2GE(out->frmt);	
				ge_set_region_pars(ge,virtual_surface2,vir_region_id,&reg_par);			
				reg_par.addr = (UINT8 *)out->bak_pic; 
				reg_par.pitch = out->pitch;
				reg_par.pixel_fmt = FRM_GIF2GE(out->frmt);	
				ge_set_region_pars(ge,virtual_surface,vir_region_id,&reg_par);
				ge_blt((struct ge_device *)out->out_par.dev,virtual_surface,virtual_surface2,&dst_rect,&src_rect,GE_BLT_SRC2_VALID);
					
				core->status.flag |= GIF_INFO_GCE_PRIOR;					
			}
			else
			{
				GIF_PRINTF("Disposal_m3: allocate back pic buff fail size %x free %x\n",out->size,buf->size);
			}
		}
		else if(2 == info->gce.disposal_m)
			core->status.flag |= GIF_INFO_GCE_PRIOR;
		MEMCPY((void *)&out->prior_gce,(void *)&info->gce,sizeof(struct gif_info_gce));
		MEMCPY((void *)&out->prior_img,(void *)&info->imgdes,sizeof(struct gif_info_imgdes));
		MEMCPY((void *)&out->prior_lsdes,(void *)&info->lsdes,sizeof(struct gif_info_lsdes));
	}

	MEMSET((void *)&dst_rect,0,sizeof(ge_rect_t));
	MEMSET((void *)&src_rect,0,sizeof(ge_rect_t));		
	dst_rect.left = lzw->rect.x;
	dst_rect.top = lzw->rect.y;
	dst_rect.width = lzw->rect.w;
	dst_rect.height = lzw->sum_lines;
	
	src_rect.left = src_rect.top = 0;
	src_rect.width = lzw->rect.w;
	src_rect.height = lzw->sum_lines;

	switch(out->frmt)
	{
		case GIF_OUT_32BITS:
			reg_par.addr = (UINT8 *)lzw->line_buf;
			reg_par.pitch = dst_rect.width * 4;
			reg_par.pixel_fmt = FRM_GIF2GE(out->frmt);	
			ge_set_region_pars(ge,virtual_surface2,vir_region_id,&reg_par);			
			reg_par.addr = (UINT8 *)lzw->bitmap; 
			reg_par.pitch = lzw->img_pitch;
			reg_par.pixel_fmt = FRM_GIF2GE(out->frmt);	
			ge_set_region_pars(ge,virtual_surface,vir_region_id,&reg_par);			
			break;
		case GIF_OUT_16BITS:
			reg_par.addr = (UINT8 *)lzw->line_buf;
			reg_par.pitch = dst_rect.width * 2;
			reg_par.pixel_fmt = FRM_GIF2GE(out->frmt);	
			ge_set_region_pars(ge,virtual_surface2,vir_region_id,&reg_par);			
			reg_par.addr = (UINT8 *)lzw->bitmap; 
			reg_par.pitch = lzw->img_pitch;
			reg_par.pixel_fmt = FRM_GIF2GE(out->frmt);	
			ge_set_region_pars(ge,virtual_surface,vir_region_id,&reg_par);				
			break;
		case GIF_OUT_8BITS:
			reg_par.addr = (UINT8 *)lzw->line_buf;
			reg_par.pitch = dst_rect.width;
			reg_par.pixel_fmt = FRM_GIF2GE(out->frmt);	
			ge_set_region_pars(ge,virtual_surface2,vir_region_id,&reg_par);			
			reg_par.addr = (UINT8 *)lzw->bitmap; 
			reg_par.pitch = lzw->img_pitch;
			reg_par.pixel_fmt = FRM_GIF2GE(out->frmt);	
			ge_set_region_pars(ge,virtual_surface,vir_region_id,&reg_par);			
			break;
		default:
			GIF_ASSERT(GIF_ERR_UNSUP_OSD_FORMAT);
			return;
	}

	if(GIF_FALSE == g_gif_dec_first_pic)
	{
		if(tran_flag)
		{	
			if(in->frame_num > 1)
				tran_true_color = (UINT32)*(local_pal + info->gce.alpha_value);
			MEMSET((void *)&color_key,0,sizeof(ge_color_key_t));
			if(GIF_OUT_8BITS == out->frmt)
			{
				color_key.low_value = color_key.high_value = info->gce.alpha_value;
				color_key.chn0_flags = GE_CKEY_ALWAYS_MATCH;
				color_key.chn1_flags = GE_CKEY_ALWAYS_MATCH;
				color_key.chn2_flags = GE_CKEY_IN_RANGE;
				color_key.src_select = GE_CKEY_SOURCE;			
			}
			else
			{
				color_key.low_value = color_key.high_value = tran_true_color;
				color_key.chn0_flags = GE_CKEY_IN_RANGE;
				color_key.chn1_flags = GE_CKEY_IN_RANGE;
				color_key.chn2_flags = GE_CKEY_IN_RANGE;
				color_key.src_select = GE_CKEY_SOURCE;
			}
			ge_set_colorkey(ge,virtual_surface,&color_key);
			ge_blt_ex(ge,virtual_surface,virtual_surface,virtual_surface2,\
				&dst_rect,&dst_rect,&src_rect,GE_BLT_SRC1_VALID | GE_BLT_SRC2_VALID | GE_BLT_COLOR_KEY\
				);
		}
		else
			ge_blt(ge,virtual_surface,virtual_surface2,&dst_rect,\
				&src_rect,GE_BLT_SRC2_VALID);
	}


	if(disposal_m == 1)	
	{
		ge_rect_t tmp_rect;

		dst_rect.left = lzw->rect.x;
		dst_rect.top = lzw->rect.y;
		dst_rect.width = lzw->rect.w;
		dst_rect.height = lzw->rect.h;
		tmp_rect.left = (disp_rect.left < dst_rect.left) ? disp_rect.left : dst_rect.left;
		tmp_rect.top = (disp_rect.top < dst_rect.top) ? disp_rect.top : dst_rect.top;
		tmp_rect.width = disp_rect.left + disp_rect.width;
		if(tmp_rect.width < dst_rect.left + dst_rect.width)
			tmp_rect.width = dst_rect.left + dst_rect.width;
		tmp_rect.width = tmp_rect.width - tmp_rect.left;
		tmp_rect.height= disp_rect.top + disp_rect.height;
		if(tmp_rect.height < dst_rect.top + dst_rect.height)
			tmp_rect.height = dst_rect.top + dst_rect.height;
		tmp_rect.height = tmp_rect.height - tmp_rect.top;
		
		dst_rect.left = out->out_par.pos.x + tmp_rect.left;
		dst_rect.top = out->out_par.pos.y + tmp_rect.top;
		dst_rect.width = tmp_rect.width;
		dst_rect.height = tmp_rect.height;
		if(tmp_rect.width + tmp_rect.left > area->w)
			dst_rect.width = area->w - tmp_rect.left;
		if(tmp_rect.height + tmp_rect.top > area->h)
			dst_rect.height = area->h - tmp_rect.top;
		src_rect.left = tmp_rect.left;
		src_rect.top = tmp_rect.top;
		src_rect.width = dst_rect.width;
		src_rect.height = dst_rect.height;			
	}
	else
	{
		dst_rect.left = out->out_par.pos.x + lzw->rect.x;
		dst_rect.top = out->out_par.pos.y + lzw->rect.y;
		dst_rect.width = lzw->rect.w;
		dst_rect.height = lzw->rect.h;
		if(lzw->rect.w + lzw->rect.x > area->w)
			dst_rect.width = area->w - lzw->rect.x;
		if(lzw->rect.h + lzw->rect.y > area->h)
			dst_rect.height = area->h - lzw->rect.y;
		src_rect.left = lzw->rect.x;
		src_rect.top = lzw->rect.y;
		src_rect.width = dst_rect.width;
		src_rect.height = dst_rect.height;
	}

	UINT32 flag = GE_BLT_SRC2_VALID;
		
	if((GIF_FALSE == g_gif_dec_first_pic) && (GIF_TRUE == g_gif_ignore_bk_color))
	{	
		if(!lzw->bk_screen_update)
		{
			ge_rect_t s_dst, s_src;

			flag = GE_BLT_SRC2_VALID;
		
			s_src.left = out->out_par.pos.x;
			s_src.top = out->out_par.pos.y;
			s_src.width = info->lsdes.w_high<<8 | info->lsdes.w_low;
			s_src.height = info->lsdes.h_high<<8 | info->lsdes.h_low;

			s_dst.left = s_dst.top = 0;
			s_dst.width = s_src.width;
			s_dst.height = s_src.height;
			
			reg_par.addr = (UINT8 *)(lzw->bk_screen_buf + 0x20000000);
			reg_par.pitch =s_dst.width * 4;
			reg_par.pixel_fmt = FRM_GIF2GE(out->frmt);	
			ge_set_region_pars(ge,virtual_surface3,vir_region_id,&reg_par);	
			ge_set_cur_region(ge,surface,out->out_par.region);		
			if(out->yuv2rgb)
				flag |= GE_BLT_RGB2YUV;
			ge_blt(ge,virtual_surface3, surface,&s_dst,&s_src,flag);
			
			lzw->bk_screen_update = 1;
		}
	}

	ge_alpha_blend_t alpha;
				
	if((lzw->rect.x < area->w) && (lzw->rect.y < area->h))
	{	
		flag = GE_BLT_SRC2_VALID;
		ge_set_cur_region(ge,surface,out->out_par.region);
		if(out->yuv2rgb)
			flag |= GE_BLT_YUV2RGB;
		if(GIF_FALSE == g_gif_dec_first_pic)
		{
			if(g_gif_trans_color_flag)
			{
				MEMSET((void *)&color_key,0,sizeof(ge_color_key_t));
				if(GIF_OUT_8BITS == out->frmt)
				{
					color_key.low_value = color_key.high_value = (g_gif_trans_color & 0xFF);
					color_key.chn0_flags = GE_CKEY_ALWAYS_MATCH;
					color_key.chn1_flags = GE_CKEY_ALWAYS_MATCH;
					color_key.chn2_flags = GE_CKEY_IN_RANGE;
					color_key.src_select = GE_CKEY_SOURCE;			
				}
				else
				{
					color_key.low_value = color_key.high_value = g_gif_trans_color;
					color_key.chn0_flags = GE_CKEY_IN_RANGE;
					color_key.chn1_flags = GE_CKEY_IN_RANGE;
					color_key.chn2_flags = GE_CKEY_IN_RANGE;
					color_key.src_select = GE_CKEY_SOURCE;
				}
				ge_set_colorkey(ge,surface,&color_key);
				ge_blt_ex(ge,surface,surface,virtual_surface,\
					&dst_rect,&dst_rect,&src_rect,GE_BLT_SRC1_VALID | GE_BLT_SRC2_VALID | GE_BLT_COLOR_KEY\
					);				
			}
			else if(g_gif_ignore_bk_color)
			{	
				ge_rect_t dst_rect2;

				dst_rect2.left = lzw->rect.x;
				dst_rect2.top = lzw->rect.y;
				dst_rect2.width = dst_rect.width;
				dst_rect2.height = dst_rect.height;
				MEMSET((void *)&alpha, 0, sizeof(ge_alpha_blend_t));	
				alpha.neg_alpha = 0;
				alpha.glob_alpha_en = 0;
				alpha.glob_alpha = 0x7F;
				alpha.src_mode = GE_AB_SRC_NOT_PREMULTI;
				ge_set_alpha(ge, surface, &alpha);
				ge_blt_ex(ge,surface,virtual_surface3,virtual_surface,\
					&dst_rect,&dst_rect2,&src_rect,GE_BLT_SRC1_VALID | GE_BLT_SRC2_VALID 
					| GE_BLT_ALPHA_BLEND);
			}
			else
			{
				if(in->frame_num > 1)
				{
					MEMSET((void *)&alpha, 0, sizeof(ge_alpha_blend_t));	
					alpha.neg_alpha = 0;
					alpha.glob_alpha_en = 0;
					alpha.glob_alpha = 0x7F;
					alpha.src_mode = GE_AB_SRC_NOT_PREMULTI;
					ge_set_alpha(ge, surface, &alpha);
					ge_blt_ex(ge,surface,virtual_surface4,virtual_surface,\
						&dst_rect,&src_rect,&src_rect,GE_BLT_SRC1_VALID | GE_BLT_SRC2_VALID 
						| GE_BLT_ALPHA_BLEND);
				}
				else
					ge_blt(ge,surface,virtual_surface,&dst_rect,&src_rect,flag);
			}
		}
		else
		{
			if(g_gif_trans_color_flag)
			{
				MEMSET((void *)&color_key,0,sizeof(ge_color_key_t));
				if(GIF_OUT_8BITS == out->frmt)
				{
					color_key.low_value = color_key.high_value = (g_gif_trans_color & 0xFF);
					color_key.chn0_flags = GE_CKEY_ALWAYS_MATCH;
					color_key.chn1_flags = GE_CKEY_ALWAYS_MATCH;
					color_key.chn2_flags = GE_CKEY_IN_RANGE;
					color_key.src_select = GE_CKEY_SOURCE;			
				}
				else
				{
					color_key.low_value = color_key.high_value = g_gif_trans_color;
					color_key.chn0_flags = GE_CKEY_IN_RANGE;
					color_key.chn1_flags = GE_CKEY_IN_RANGE;
					color_key.chn2_flags = GE_CKEY_IN_RANGE;
					color_key.src_select = GE_CKEY_SOURCE;
				}
				ge_set_colorkey(ge,surface,&color_key);
				ge_blt_ex(ge,surface,surface,virtual_surface2,\
					&dst_rect,&dst_rect,&src_rect,GE_BLT_SRC1_VALID | GE_BLT_SRC2_VALID | GE_BLT_COLOR_KEY\
					);						
			}
			else if(g_gif_ignore_bk_color)
			{				
				MEMSET((void *)&alpha, 0, sizeof(ge_alpha_blend_t));	
				alpha.neg_alpha = 0;
				alpha.glob_alpha_en = 0;
				alpha.glob_alpha = 0x7F;
				alpha.src_mode = GE_AB_SRC_NOT_PREMULTI;
				ge_set_alpha(ge, surface, &alpha);
				ge_blt_ex(ge,surface,surface,virtual_surface2,\
					&dst_rect,&dst_rect,&src_rect,GE_BLT_SRC1_VALID | GE_BLT_SRC2_VALID 
					| GE_BLT_ALPHA_BLEND);
			}
			else
				ge_blt(ge,surface,virtual_surface2,&dst_rect,&src_rect,flag);		
		}
	}
	
	ge_unlock(ge);

	lzw->sum_lines = 0;
	lzw->pixel_num = 0;
	lzw->tick_start = osal_get_tick();
	if(out->out_par.gif_delay_num != 0)
	{
		lzw->delay_num = out->out_par.gif_delay_num;
	}
	else
	{
		lzw->delay_num = ((info->gce.dy_time_high<<8) | (info->gce.dy_time_low)) * 10 + GIF_DELAY_EXTENTION;
	}
	
	if(0 == lzw->delay_num)
		lzw->delay_num = GIF_DELAY_MIN_VALUE;
	GIF_PRINTF("out put width %d height %d delay %d\n",lzw->rect.w,lzw->rect.h,lzw->delay_num);
}

static GIF_RET gif_out_enter(struct gif_out *out,struct gif_lzw *lzw)
{
	struct ge_device *ge = (struct ge_device *)out->out_par.dev;
	ge_surface_desc_t *surface = (ge_surface_desc_t *)out->out_par.handle;
	ge_region_pars_t reg_par;
	ge_rect_t rect;

	if(out->update)
	{
		out->update = 0;
		if(GIF_OUT_8BITS == out->frmt)
		{	
			ge_show_onoff(ge,surface,FALSE);
			if(RET_SUCCESS != ge_set_pallette(ge,surface,(UINT8 *)lzw->pallete.local_abs_pal
				,(UINT16)lzw->pallete.num,GE_CS_YCBCR,0))
				return GIF_FALSE;
			if(RET_FAILURE == ge_get_region_pars(ge,surface,surface->cur_region,&reg_par))
				return GIF_FALSE;
			rect.left = rect.top = 0;
			rect.width = reg_par.rect.width;
			rect.height = reg_par.rect.height;
			ge_lock(ge);
			ge_set_cur_region(ge,surface,out->out_par.region);
			ge_fill_rect(ge,surface,&rect,GIF_DEFAULT_TRAN_COLOR);//out->bk_clor);
			ge_unlock(ge);
		}
		ge_show_onoff(ge,surface,TRUE);
	}
	return GIF_TRUE;
}

static GIF_RET gif_out_init(struct gif_context * text,struct gif_dec_out_par *par)
{
	GIF_RET ret = GIF_FALSE;
	struct gif_core_buf *buf = &(text->core.buf);
	struct gif_out *out = NULL;
	INT32 len = (sizeof(struct gif_out) + 3) & ~3;
	struct gif_in *in = &text->in;

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

			len = 4 * sizeof(ge_surface_desc_t);
			if((buf->size -= len) >= 0)
			{
				text->out->par1 = (void *)buf->start;
				text->out->par2 = text->out->par1+ sizeof(ge_surface_desc_t);
				text->out->par3 = text->out->par2+ sizeof(ge_surface_desc_t);
				text->out->par4 = text->out->par3+ sizeof(ge_surface_desc_t);
				buf->start += len;
				GIF_PRINTF("out surface len %x addr %x free size %x\n",len,(UINT32)out,buf->size);				
				ret =  init_surface_par(out);

				if(in->frame_num > 1)
				{
					ge_rect_t sur_dst, sur_src;
					ge_region_pars_t reg_par;
					struct ge_device *ge = (struct ge_device *)out->out_par.dev;
					ge_surface_desc_t *surface = (ge_surface_desc_t *)out->out_par.handle;
					virtual_surface4 = (ge_surface_desc_t *)out->par4;
						
					sur_src.left = out->out_par.pos.x;
					sur_src.top = out->out_par.pos.y;
					sur_src.width = out->out_par.area.w;
					sur_src.height = out->out_par.area.h;
					
					sur_dst.left = sur_dst.top = 0;
					sur_dst.width = sur_src.width;
					sur_dst.height = sur_src.height;

					//GIF_PRINTF("[gif_out_init]: buf->hard_surface_buf=0x%x.\n", buf->hard_surface_buf);
					reg_par.addr = (UINT8 *)(buf->hard_surface_buf);
					reg_par.pitch = sur_dst.width * 2;
					reg_par.pixel_fmt = GE_PF_ARGB1555;
					ge_set_region_pars(ge,virtual_surface4,vir_region_id,&reg_par);	
					ge_set_cur_region(ge,surface,out->out_par.region);
					ge_blt(ge,virtual_surface4,surface,&sur_dst,&sur_src,GE_BLT_SRC2_VALID);
				}
				
			}
		}
	}
	if(GIF_FALSE == ret)
		gif_core_set_error(text,GIF_ERR_OUT_INIT);
	
	return ret;
}

void gif_out_init_ge(void)
{
	g_gif_out_init = gif_out_init;
	g_gif_out_enter = gif_out_enter;
	g_gif_out_img = gif_out_img;
}

