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
 * 0.		2008.2.12			Sam			Create
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

#define GE36F_MAX_CMD_NUM 10
extern UINT8 g_gif_dec_first_pic;
extern UINT8 g_gif_hw_support_rgb;
extern GIF_RET (*g_gif_out_init)(struct gif_context *,struct gif_dec_out_par *);
extern GIF_RET (*g_gif_out_enter)(struct gif_out *,struct gif_lzw *);
extern void (*g_gif_out_img)(struct gif_context *);
extern GIF_RET (*g_gif_out_exit)(struct gif_context *);
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

GIF_INLINE GIF_RET init_region_par(struct gif_out *out)
{
	struct ge_device *ge = (struct ge_device *)out->out_par.dev;
	ge_gma_region_t *pregion = (ge_gma_region_t *)out->sub_handle;
	UINT32 layer_id = (UINT32)out->out_par.handle;

	if(RET_FAILURE == ge_gma_get_region_info(ge,layer_id,(UINT32)out->out_par.region,pregion))
	{
		return GIF_FALSE;
	}
	if(GE_PF_ARGB8888 == pregion->color_format || GE_PF_RGB888 == pregion->color_format
		|| GE_PF_AYCBCR8888 == pregion->color_format || GE_PF_YCBCR888 == pregion->color_format)
		out->frmt = GIF_OUT_32BITS;
	else
		out->frmt = GIF_OUT_8BITS;
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

	struct ge_device *ge = (struct ge_device *)out->out_par.dev;
	ge_operating_entity entity;
	ge_gma_region_t *pregion = (ge_gma_region_t *)out->sub_handle;
	UINT32 layer_id = (UINT32)out->out_par.handle;
	UINT32 region_id = (UINT32)out->out_par.region;
	UINT32 cmd_hdl = 0;
	ge_rect_t dst_rect,src_rect;
	
	UINT32 * local_pal = lzw->pallete.local_abs_pal;
	UINT32 tran_true_color = 0;
	UINT8 tran_flag = 0;

	int disposal_m = 0;
	ge_rect_t disp_rect;

	if(lzw->img_id == 1)	
	{
		dst_rect.width = (info->lsdes.w_high<<8) | info->lsdes.w_low;
		dst_rect.height= (info->lsdes.h_high<<8) | info->lsdes.h_low;
		dst_rect.left = 0;
		dst_rect.top = 0;
		if(GIF_OUT_32BITS == out->frmt)
			tran_true_color = (UINT32)*(local_pal +  info->lsdes.bk_color);
		else
			tran_true_color =  info->lsdes.bk_color;		

		cmd_hdl = ge_cmd_begin(ge, out->cmd_list, GE_FILL_RECT_BACK_COLOR);
		{
			entity.color_format = FRM_GIF2GE(out->frmt);
			entity.base_address = lzw->bitmap;
			entity.data_decoder = GE_DECODER_DISABLE;
			entity.pixel_pitch = lzw->pixel_pitch;
			entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
			ge_set_operating_entity(ge,cmd_hdl,GE_DST,&entity);
			// set the background color to be filled in the rect
			ge_set_back_color(ge, cmd_hdl, tran_true_color);
			// set the rect position
			ge_set_xy(ge, cmd_hdl, GE_DST, dst_rect.left, dst_rect.top);
			// set the rect size
			ge_set_wh(ge, cmd_hdl, GE_DST, dst_rect.width, dst_rect.height);
			// end this command. if the mode of cmd list is GE_COMPILE_AND_EXECUTE
			// , then this cmd will be executed by GE immediately
		}
		ge_cmd_end(ge, cmd_hdl);		

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
			if(GIF_OUT_32BITS == out->frmt)
				tran_true_color = (UINT32)*(local_pal + out->prior_lsdes.bk_color);
			else
				tran_true_color = out->prior_lsdes.bk_color;
	
			cmd_hdl = ge_cmd_begin(ge, out->cmd_list, GE_FILL_RECT_BACK_COLOR);
			{
				entity.color_format = FRM_GIF2GE(out->frmt);
				entity.base_address = lzw->bitmap;
				entity.data_decoder = GE_DECODER_DISABLE;
				entity.pixel_pitch = lzw->pixel_pitch;
				entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
				ge_set_operating_entity(ge,cmd_hdl,GE_DST,&entity);
				// set the background color to be filled in the rect
				ge_set_back_color(ge, cmd_hdl, tran_true_color);
				// set the rect position
				ge_set_xy(ge, cmd_hdl, GE_DST, dst_rect.left, dst_rect.top);
				// set the rect size
				ge_set_wh(ge, cmd_hdl, GE_DST, dst_rect.width, dst_rect.height);
				// end this command. if the mode of cmd list is GE_COMPILE_AND_EXECUTE
				// , then this cmd will be executed by GE immediately
			}
			ge_cmd_end(ge, cmd_hdl);		

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
			
			cmd_hdl = ge_cmd_begin(ge,out->cmd_list,GE_PRIM_DISABLE);
			{
				entity.color_format = FRM_GIF2GE(out->frmt);
				entity.base_address = lzw->bitmap;
				entity.data_decoder = GE_DECODER_DISABLE;
				entity.pixel_pitch = lzw->pixel_pitch;
				entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
				ge_set_operating_entity(ge,cmd_hdl,GE_DST,&entity);	
				entity.color_format = FRM_GIF2GE(out->frmt);
				entity.base_address = out->bak_pic;
				entity.data_decoder = GE_DECODER_DISABLE;
				entity.pixel_pitch = out->pitch;
				entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
				ge_set_operating_entity(ge,cmd_hdl,GE_SRC,&entity);				
				ge_set_src_mode(ge,cmd_hdl,GE_SRC_DIRECT_COPY);
				ge_set_xy(ge,cmd_hdl,GE_DST,dst_rect.left,dst_rect.top);
				ge_set_xy(ge,cmd_hdl,GE_SRC,src_rect.left,src_rect.top);
				ge_set_wh(ge,cmd_hdl,GE_DST,dst_rect.width,dst_rect.height);	
			}
			ge_cmd_end(ge,cmd_hdl);
			
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
			UINT8 red = 0,green = 0,blue = 0;
			
			tran_flag = 1;
			if(GIF_OUT_32BITS == out->frmt)
				tran_true_color = (UINT32)*(local_pal + info->gce.alpha_value);				
			else
				tran_true_color = (UINT32)info->gce.alpha_value;

			if(GIF_FALSE == g_gif_dec_first_pic)
			{
				if(1 == lzw->img_id)
				{
					// clear the bitmap buf
					dst_rect.left = lzw->rect.x;
					dst_rect.top = lzw->rect.y;
					dst_rect.width = lzw->rect.w;
					dst_rect.height = lzw->rect.h;
					
					cmd_hdl = ge_cmd_begin(ge, out->cmd_list, GE_FILL_RECT_BACK_COLOR);
					{
						entity.color_format = FRM_GIF2GE(out->frmt);
						entity.base_address = lzw->bitmap;
						entity.data_decoder = GE_DECODER_DISABLE;
						entity.pixel_pitch = lzw->pixel_pitch;
						entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
						ge_set_operating_entity(ge,cmd_hdl,GE_DST,&entity);
						// set the background color to be filled in the rect
						ge_set_back_color(ge, cmd_hdl, out->bk_clor);
						// set the rect position
						ge_set_xy(ge, cmd_hdl, GE_DST, dst_rect.left, dst_rect.top);
						// set the rect size
						ge_set_wh(ge, cmd_hdl, GE_DST, dst_rect.width, dst_rect.height);
						// end this command. if the mode of cmd list is GE_COMPILE_AND_EXECUTE
						// , then this cmd will be executed by GE immediately
					}
					ge_cmd_end(ge, cmd_hdl);					
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
			out->pitch = out->width;
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
				
				cmd_hdl = ge_cmd_begin(ge,out->cmd_list,GE_PRIM_DISABLE);
				{
					entity.color_format = FRM_GIF2GE(out->frmt);
					entity.base_address = out->bak_pic;
					entity.data_decoder = GE_DECODER_DISABLE;
					entity.pixel_pitch = out->pitch;
					entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
					ge_set_operating_entity(ge,cmd_hdl,GE_DST,&entity);	
					entity.color_format = FRM_GIF2GE(out->frmt);
					entity.base_address = lzw->bitmap;
					entity.data_decoder = GE_DECODER_DISABLE;
					entity.pixel_pitch = lzw->pixel_pitch;
					entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
					ge_set_operating_entity(ge,cmd_hdl,GE_SRC,&entity);						
					ge_set_src_mode(ge,cmd_hdl,GE_SRC_DIRECT_COPY);
					ge_set_xy(ge,cmd_hdl,GE_DST,dst_rect.left,dst_rect.top);
					ge_set_xy(ge,cmd_hdl,GE_SRC,src_rect.left,src_rect.top);
					ge_set_wh(ge,cmd_hdl,GE_DST,dst_rect.width,dst_rect.height);
				}
				ge_cmd_end(ge,cmd_hdl);
				
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

	//MEMSET((void *)&dst_rect,0,sizeof(ge_rect_t));
	//MEMSET((void *)&src_rect,0,sizeof(ge_rect_t));		
	dst_rect.left = lzw->rect.x;
	dst_rect.top = lzw->rect.y;
	dst_rect.width = lzw->rect.w;
	dst_rect.height = lzw->sum_lines;
	
	src_rect.left = src_rect.top = 0;
	src_rect.width = lzw->rect.w;
	src_rect.height = lzw->sum_lines;

	if(GIF_FALSE == g_gif_dec_first_pic)
	{
	       if(tran_flag)
		{
                    cmd_hdl = ge_cmd_begin(ge,out->cmd_list,GE_DRAW_BITMAP);
                    {
                        entity.color_format = FRM_GIF2GE(out->frmt);
                        entity.base_address = lzw->bitmap;
                        entity.data_decoder = GE_DECODER_DISABLE;
                        entity.pixel_pitch = lzw->pixel_pitch;
                        entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
                        ge_set_operating_entity(ge,cmd_hdl,GE_DST,&entity);	
                        entity.color_format = FRM_GIF2GE(out->frmt);
                        entity.base_address = lzw->bitmap;
                        entity.data_decoder = GE_DECODER_DISABLE;
                        entity.pixel_pitch = lzw->pixel_pitch;
                        entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
                        ge_set_operating_entity(ge,cmd_hdl,GE_SRC,&entity);	
                        entity.color_format = FRM_GIF2GE(out->frmt);
                        entity.base_address = lzw->line_buf;
                        entity.data_decoder = GE_DECODER_DISABLE;
                        entity.pixel_pitch = src_rect.width;
                        entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
                        ge_set_operating_entity(ge,cmd_hdl,GE_PTN,&entity);			
                        ge_set_src_mode(ge,cmd_hdl,GE_SRC_BITBLT);
                        ge_set_xy(ge,cmd_hdl,GE_DST,dst_rect.left,dst_rect.top);
                        ge_set_xy(ge,cmd_hdl,GE_SRC,dst_rect.left,dst_rect.top);            
                        ge_set_xy(ge,cmd_hdl,GE_PTN,src_rect.left,src_rect.top);         
                        ge_set_wh(ge,cmd_hdl,GE_DST_PTN,dst_rect.width,dst_rect.height);

                        ge_set_colorkey_mode(ge, cmd_hdl, GE_CKEY_PTN_PRE_CLUT);
                        if(GIF_OUT_32BITS == out->frmt)
                        {
                            ge_set_colorkey_match_mode(ge, cmd_hdl, GE_CKEY_CHANNEL_R, GE_CKEY_MATCH_IN_RANGE);
                            ge_set_colorkey_match_mode(ge, cmd_hdl, GE_CKEY_CHANNEL_G, GE_CKEY_MATCH_IN_RANGE);
                            ge_set_colorkey_match_mode(ge, cmd_hdl, GE_CKEY_CHANNEL_B, GE_CKEY_MATCH_IN_RANGE);
                        }
                        else
                        {
                            ge_set_colorkey_match_mode(ge, cmd_hdl, GE_CKEY_CHANNEL_R, GE_CKEY_MATCH_ALWAYS);
                            ge_set_colorkey_match_mode(ge, cmd_hdl, GE_CKEY_CHANNEL_G, GE_CKEY_MATCH_ALWAYS);
                            ge_set_colorkey_match_mode(ge, cmd_hdl, GE_CKEY_CHANNEL_B, GE_CKEY_MATCH_IN_RANGE);			
                        }
                        ge_set_colorkey_range(ge, cmd_hdl, tran_true_color, tran_true_color);
	             }
                    ge_cmd_end(ge,cmd_hdl);	
	       }
           else
           {
			cmd_hdl = ge_cmd_begin(ge,out->cmd_list,GE_PRIM_DISABLE);
			{
				entity.color_format = FRM_GIF2GE(out->frmt);
				entity.base_address = lzw->bitmap;
				entity.data_decoder = GE_DECODER_DISABLE;
				entity.pixel_pitch = lzw->pixel_pitch;
				entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
				ge_set_operating_entity(ge,cmd_hdl,GE_DST,&entity);	
				entity.color_format = FRM_GIF2GE(out->frmt);
				entity.base_address = lzw->line_buf;
				entity.data_decoder = GE_DECODER_DISABLE;
				entity.pixel_pitch = src_rect.width;
				entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
				ge_set_operating_entity(ge,cmd_hdl,GE_SRC,&entity);						
				ge_set_src_mode(ge,cmd_hdl,GE_SRC_DIRECT_COPY);
				ge_set_xy(ge,cmd_hdl,GE_DST,dst_rect.left,dst_rect.top);
				ge_set_xy(ge,cmd_hdl,GE_SRC,src_rect.left,src_rect.top);
				ge_set_wh(ge,cmd_hdl,GE_DST,dst_rect.width,dst_rect.height);
			}
			ge_cmd_end(ge,cmd_hdl);            
           }
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

	if((lzw->rect.x < area->w) && (lzw->rect.y < area->h))
	{
		UINT32 flag = GE_BLT_SRC2_VALID;

		ge_cmd_list_new(ge,out->cmd_list,GE_COMPILE_AND_EXECUTE);
		ge_gma_set_region_to_cmd_list(ge,layer_id,region_id,out->cmd_list);

		if(GIF_FALSE == g_gif_dec_first_pic)
		{
			entity.color_format = FRM_GIF2GE(out->frmt);
			entity.base_address = lzw->bitmap;
			entity.data_decoder = GE_DECODER_DISABLE;
			entity.pixel_pitch = lzw->pixel_pitch;
		}
		else
		{
			entity.color_format = FRM_GIF2GE(out->frmt);
			entity.base_address = lzw->line_buf;
			entity.data_decoder = GE_DECODER_DISABLE;
			entity.pixel_pitch = src_rect.width;
		}
				
		if(GIF_OUT_32BITS != out->frmt)
		{
			cmd_hdl = ge_cmd_begin(ge,out->cmd_list,GE_DRAW_BITMAP);
			{	
				entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
				ge_set_operating_entity(ge,cmd_hdl,GE_PTN,&entity);	
				ge_set_clut_rgb_order(ge,cmd_hdl,GE_RGB_ORDER_ARGB);
				ge_set_xy(ge,cmd_hdl,GE_DST,dst_rect.left,dst_rect.top);
				ge_set_xy(ge,cmd_hdl,GE_SRC,dst_rect.left,dst_rect.top);
				ge_set_xy(ge,cmd_hdl,GE_PTN,src_rect.left,src_rect.top);                
				ge_set_wh(ge,cmd_hdl,GE_DST_PTN,dst_rect.width,dst_rect.height);
			    	ge_set_clut_mode(ge, cmd_hdl, GE_CLUT_COLOR_EXPANSION, FALSE);				
				ge_set_clut_addr(ge, cmd_hdl, (UINT32)local_pal);
	    			ge_set_clut_update(ge, cmd_hdl, TRUE);
			}
			ge_cmd_end(ge,cmd_hdl);
		}
		else
		{
			cmd_hdl = ge_cmd_begin(ge,out->cmd_list,GE_PRIM_DISABLE);
			{				
				entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
				ge_set_operating_entity(ge,cmd_hdl,GE_SRC,&entity);						
				ge_set_src_mode(ge,cmd_hdl,GE_SRC_DIRECT_COPY);
				ge_set_xy(ge,cmd_hdl,GE_DST,dst_rect.left,dst_rect.top);
				ge_set_xy(ge,cmd_hdl,GE_SRC,src_rect.left,src_rect.top);
				ge_set_wh(ge,cmd_hdl,GE_DST,dst_rect.width,dst_rect.height);
			}
			ge_cmd_end(ge,cmd_hdl);
		}	
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
	struct ge_device *ge = (struct ge_device *)out->out_par.dev;
	ge_gma_region_t *pregion = (ge_gma_region_t *)out->sub_handle;
	UINT32 layer_id = (UINT32)out->out_par.handle;
	ge_rect_t rect;

	if(out->update)
	{
		out->update = 0;
		if(GE_PF_CK_CLUT8 == pregion->color_format)
			return GIF_FALSE;
	}
    
	if(0 == out->cmd_list)
	{
		out->cmd_list = (UINT32)ge_cmd_list_create(ge,GE36F_MAX_CMD_NUM);
		if(0 == out->cmd_list)
			return GIF_FALSE;
	}
    
	ge_cmd_list_new(ge,out->cmd_list,GE_COMPILE_AND_EXECUTE);
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

			len = (sizeof(ge_gma_region_t) + 3) & ~3;
			if((buf->size -= len) >= 0)
			{
				out->sub_handle = buf->start;
				buf->start += len;
				ret = init_region_par(out);
			}
		}
	}
	if(GIF_FALSE == ret)
		gif_core_set_error(text,GIF_ERR_OUT_INIT);
	
	return ret;
}

static GIF_RET gif_out_exit(struct gif_context *text)
{
	struct gif_out *out = text->out;

        if(0 != out->cmd_list)
        {
            ge_cmd_list_destroy((struct ge_device *)out->out_par.dev, out->cmd_list);
        }
}   

// in 3202c gif solution, only support true color ge
void gif_out_init_ge36f(void)
{
	g_gif_hw_support_rgb = GIF_TRUE;
		
	g_gif_out_init = gif_out_init;
	g_gif_out_enter = gif_out_enter;
	g_gif_out_img = gif_out_img;
       g_gif_out_exit = gif_out_exit;
}

