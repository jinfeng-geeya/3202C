/****************************************************************************
 *
 *  ALi Corporation, All Rights Reserved. 2008 Copyright (C)
 *
 *  File: ge_old.c
 *
 *  Description: This file defines old graphics engine API.
 *
 *  History:
 *      Date          Author        Version         Comment
 *      ====          ======        =======         =======
 *  1.  2009.2.6   Sam     0.1.000         Initial
  ****************************************************************************/
/*
 * 	Name		:   ge_lock()
 *	Description	:   Lock a ge device
 *	Parameter	:   struct ge_device *dev		:
 *	Return		:   INT32 					: Result
 *
 */
RET_CODE ge_lock(struct ge_device *dev)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}

	if (dev->ioctl)
	{
		return dev->ioctl(dev,GE_IO_LOCK_DEV,0);
	}	
    return RET_SUCCESS;
}

/*
 * 	Name		:   ge_unlock()
 *	Description	:   Unlock a ge device
 *	Parameter	:   struct ge_device *dev		:
 *	Return		:   INT32 						: Result
 *
 */
RET_CODE ge_unlock(struct ge_device *dev)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}

	if (dev->ioctl)
	{
		return dev->ioctl(dev,GE_IO_UNLOCK_DEV,0);
	}	
    return RET_SUCCESS;
}

/*
   Draw a pixel to the specified surface
*/
RET_CODE ge_draw_pixel(struct ge_device *dev, ge_surface_desc_t *surface, int x, int y, UINT32 pixel)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->draw_pixel)
		return  dev->draw_pixel(dev, surface, x, y, pixel);
	
	return ERR_NO_DEV;
}

/*
   Read a pixel from the specified surface
*/
UINT32      ge_read_pixel(struct ge_device *dev, ge_surface_desc_t *surface, int x, int y)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->read_pixel)
		return  dev->read_pixel(dev, surface, x, y);
	
	return ERR_NO_DEV;
}

/*
  Draw one horizontal line on the specified surface
*/
RET_CODE ge_draw_hor_line(struct ge_device *dev, ge_surface_desc_t *surface, int x, int y, int w, UINT32 pixel)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->draw_hor_line)
		return  dev->draw_hor_line(dev, surface, x, y, w, pixel);
	
	return ERR_NO_DEV;
}

/*
  Draw one vertical line on the specified surface
*/
RET_CODE ge_draw_ver_line(struct ge_device *dev, ge_surface_desc_t *surface, int x, int y, int h, UINT32 pixel)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->draw_ver_line)
		return  dev->draw_ver_line(dev, surface, x, y, h, pixel);
	
	return ERR_NO_DEV;
}

RET_CODE ge_fill_rect(struct ge_device *dev, ge_surface_desc_t *surface,  ge_rect_t *rect, UINT32 color)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->fill_rect)
		return  dev->fill_rect(dev, surface, rect, color);
	
	return ERR_NO_DEV;
}

RET_CODE ge_blt(struct ge_device *dev, ge_surface_desc_t *dst,   ge_surface_desc_t *src,  ge_rect_t *dst_rect, ge_rect_t *src_rect, UINT32 op_flag)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->blt)
		return  dev->blt(dev, dst, src, dst_rect, src_rect, op_flag);
	
	return ERR_NO_DEV;
	
}

RET_CODE ge_blt_ex(struct ge_device *dev, ge_surface_desc_t *dst,   ge_surface_desc_t *src1,  ge_surface_desc_t *src2, ge_rect_t *dst_rect, ge_rect_t *src1_rect, ge_rect_t *src2_rect, UINT32 op_flag)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->blt_ex)
		return  dev->blt_ex(dev, dst, src1, src2, dst_rect, src1_rect, src2_rect, op_flag);
	
	return ERR_NO_DEV;
	
}

ge_surface_desc_t *ge_create_surface(struct ge_device *dev, ge_surface_desc_t *para, UINT32 flag, UINT8 gma_layer)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return NULL;
	}
	
	if (dev->create_surface)
	{
		return dev->create_surface(dev, para, flag, gma_layer);
	}

	return NULL;	
}

RET_CODE ge_release_surface(struct ge_device *dev,ge_surface_desc_t *surface)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->release_surface)
	{
		dev->release_surface(dev,surface);
	}
	
	return RET_SUCCESS;
}

RET_CODE ge_show_onoff(struct ge_device *dev, ge_surface_desc_t *surface, UINT8 on_off)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->show_onoff)
	{
		dev->show_onoff(surface, on_off);
	}
	
	return RET_SUCCESS;
}

RET_CODE ge_region_show_onoff(struct ge_device *dev, ge_surface_desc_t *surface, UINT8 uRegionId,BOOL bOn)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->region_show)
	{
		return dev->region_show(dev, surface, uRegionId,bOn);
	}
	
	return RET_SUCCESS;
}
RET_CODE ge_set_pallette(struct ge_device *dev, ge_surface_desc_t *surface,UINT8 *pallette,UINT16 color_num, UINT8 type,enum GE_PALLET_ALPHA_LEVEL alpha_level)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_pallette)
	{
		dev->set_pallette(dev, surface, pallette,color_num,type, alpha_level);
	}
	
	return RET_SUCCESS;
}

RET_CODE ge_get_pallette(struct ge_device *dev, ge_surface_desc_t *surface, UINT8 *pallette,UINT16 color_num, UINT8 type)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_pallette)
	{
		dev->get_pallette(surface, pallette,color_num,type);
	}
	
	return RET_SUCCESS;
}

RET_CODE ge_modify_pallette(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 index,UINT8 y,UINT8 cb,UINT8 cr,UINT32 alpha,enum GE_PALLET_ALPHA_LEVEL alpha_level)
{
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_pallette)
	{
		dev->modify_pallette(dev, surface, index,y, cb, cr, alpha, alpha_level);
	}
	
	return RET_SUCCESS;
}

RET_CODE ge_create_region(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id,ge_rect_t *rect,ge_region_t *para)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->create_region)
	{
		dev->create_region(dev, surface, region_id,rect, para);
	}
	
	return RET_SUCCESS;

}
RET_CODE ge_delete_region(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->delete_region)
	{
		dev->delete_region(dev, surface, region_id);
	}
	
	return RET_SUCCESS;
}
RET_CODE ge_set_region_pos(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id, ge_rect_t *rect)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_region_pos)
	{
		dev->set_region_pos(surface, region_id,rect);
	}
	
	return RET_SUCCESS;
}

RET_CODE ge_get_region_pos(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id, ge_rect_t *rect)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_region_pos)
	{
		dev->get_region_pos(surface, region_id,rect);
	}
	
	return RET_SUCCESS;
}

RET_CODE ge_set_clip(struct ge_device *dev,ge_surface_desc_t *surface, ge_clip_t* clip)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_clip)
	{
		return dev->set_clip(surface, clip);
	}
	
	return RET_SUCCESS;

}

RET_CODE ge_disable_clip(struct ge_device *dev,ge_surface_desc_t *surface)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->disable_clip)
	{
		dev->disable_clip(surface);
	}
	
	return RET_SUCCESS;

}

#if 0
RET_CODE ge_stretch_blt(struct ge_device *dev,ge_surface_t *dest_surface,  ge_surface_t *src1_surface, ge_surface_t *src2_surface, ge_surface_desc_t *surface, ge_blt_t  *blt_fx)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->stretch_blt)
		return  dev->stretch_blt(dev, dest_surface, src1_surface, src2_surface, surface,blt_fx);
	
	return ERR_NO_DEV;
	
}
#endif

RET_CODE ge_set_rop(struct ge_device *dev, ge_surface_desc_t *surface, UINT32 rop)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_rop)
		return  dev->set_rop( surface, rop);
			
	return ERR_NO_DEV;
	
}

RET_CODE ge_get_rop(struct ge_device *dev, ge_surface_desc_t *surface, UINT32 *rop)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_rop)
		return  dev->get_rop( surface, rop);
			
	return ERR_NO_DEV;
	
}

RET_CODE ge_set_resize(struct ge_device *dev, ge_surface_desc_t *surface, ge_resize_t *resize)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_resize)
		return  dev->set_resize( surface, resize);
			
	return ERR_NO_DEV;
	
}

RET_CODE ge_get_resize(struct ge_device *dev, ge_surface_desc_t *surface, ge_resize_t *resize)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_resize)
		return  dev->get_resize( surface, resize);
			
	return ERR_NO_DEV;
	
}

RET_CODE ge_set_colorkey(struct ge_device *dev, ge_surface_desc_t *surface, ge_color_key_t *color_key)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_colorkey)
		return  dev->set_colorkey(surface, color_key);
			
	return ERR_NO_DEV;
}

RET_CODE ge_get_colorkey(struct ge_device *dev, ge_surface_desc_t *surface, ge_color_key_t *color_key)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_colorkey)
		return  dev->get_colorkey(surface, color_key);
			
	return ERR_NO_DEV;
}

RET_CODE ge_set_alpha(struct ge_device *dev, ge_surface_desc_t *surface, ge_alpha_blend_t *alpha_blending)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_alpha)
		return  dev->set_alpha(surface, alpha_blending);
			
	return ERR_NO_DEV;
}

RET_CODE ge_get_alpha(struct ge_device *dev, ge_surface_desc_t *surface, ge_alpha_blend_t *alpha_blending)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_alpha)
		return  dev->get_alpha(surface, alpha_blending);
			
	return ERR_NO_DEV;
}

RET_CODE ge_set_colorredc(struct ge_device *dev, ge_surface_desc_t *surface, ge_col_rdc_t *color_redc)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_colorredc)
		return  dev->set_colorredc(surface, color_redc);
			
	return ERR_NO_DEV;
}
RET_CODE ge_get_colorredc(struct ge_device *dev, ge_surface_desc_t *surface, ge_col_rdc_t *color_redc)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_colorredc)
		return  dev->get_colorredc(surface, color_redc);
			
	return ERR_NO_DEV;
}

RET_CODE ge_set_flickfilt(struct ge_device *dev, ge_surface_desc_t *surface, ge_flick_filter_t *flick_filt)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_flickfilt)
		return  dev->set_flickfilt(surface, flick_filt);
			
	return ERR_NO_DEV;
}
RET_CODE ge_get_flickfilt(struct ge_device *dev, ge_surface_desc_t *surface, ge_flick_filter_t *flick_filt)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_flickfilt)
		return  dev->get_flickfilt(surface, flick_filt);
			
	return ERR_NO_DEV;
}

RET_CODE ge_set_rgb2yuv( struct ge_device *dev, ge_surface_desc_t *surface, ge_YUV_RGB_conv_t *rgb2yuv)
{
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_rgb2yuv)
		return  dev->set_rgb2yuv(surface, rgb2yuv);
			
	return ERR_NO_DEV;
}

RET_CODE ge_get_rgb2yuv( struct ge_device *dev, ge_surface_desc_t *surface, ge_YUV_RGB_conv_t *rgb2yuv)
{
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_rgb2yuv)
		return  dev->get_rgb2yuv(surface, rgb2yuv);
			
	return ERR_NO_DEV;
}

RET_CODE ge_set_yuv2rgb( struct ge_device *dev, ge_surface_desc_t *surface, ge_YUV_RGB_conv_t *yuv2rgb)
{
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_yuv2rgb)
		return  dev->set_yuv2rgb(surface, yuv2rgb);
			
	return ERR_NO_DEV;
}

RET_CODE ge_get_yuv2rgb( struct ge_device *dev, ge_surface_desc_t *surface, ge_YUV_RGB_conv_t *yuv2rgb)
{
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_yuv2rgb)
		return  dev->set_yuv2rgb(surface, yuv2rgb);
			
	return ERR_NO_DEV;
}

RET_CODE ge_set_planmask(struct ge_device *dev, ge_surface_desc_t *surface, UINT32 plane_mask)
{
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_planmask)
		return  dev->set_planmask(surface, plane_mask);
			
	return ERR_NO_DEV;

}

RET_CODE ge_get_planmask(struct ge_device *dev, ge_surface_desc_t *surface, UINT32 *plane_mask)
{
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_planmask)
		return  dev->get_planmask(surface, plane_mask);
			
	return ERR_NO_DEV;

}

// extended APIs
RET_CODE    ge_set_cur_region(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id)
{
	ge_io_cmd_par_t io_cmd_par;
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}	

	io_cmd_par.par0 = (UINT32)surface;
	io_cmd_par.par1 = (UINT32)&region_id;

	if (dev->ioctl)
	{
		return dev->ioctl(dev,GE_IO_SET_CUR_REGION,(UINT32)&io_cmd_par);
	}
			
	return ERR_NO_DEV;
}

// set the chosen region as the current region and its parameters(frame buf, color format, pitch - except region width and height)
RET_CODE ge_set_region_pars(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id,ge_region_pars_t *pars)
{
	ge_io_cmd_par_t io_cmd_par;
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	io_cmd_par.par0 = (UINT32)surface;
	io_cmd_par.par1 = (UINT32)&region_id;
	io_cmd_par.par2 = (UINT32)pars;
	
	if (dev->ioctl)
	{
		return dev->ioctl(dev,GE_IO_SET_REG_PARS,(UINT32)&io_cmd_par);
	}
			
	return ERR_NO_DEV;
}

RET_CODE ge_get_region_pars(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id, ge_region_pars_t *pars)
{
	ge_io_cmd_par_t io_cmd_par;
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}

	io_cmd_par.par0 = (UINT32)surface;
	io_cmd_par.par1 = (UINT32)&region_id;
	io_cmd_par.par2 = (UINT32)pars;	

	if (dev->ioctl)
	{
		return dev->ioctl(dev,GE_IO_GET_REG_PARS,(UINT32)&io_cmd_par);
	}
			
	return ERR_NO_DEV;
}

// only support 1bit/pixel format resource at 2008.6.2
RET_CODE ge_draw_bitmap(struct ge_device *dev,ge_surface_desc_t *surface, ge_rect_t *rect,ge_bitmap_t *bitmap)
{
	ge_io_cmd_par_t io_cmd_par;
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}

	io_cmd_par.par0 = (UINT32)surface;
	io_cmd_par.par1 = (UINT32)rect;
	io_cmd_par.par2 = (UINT32)bitmap;
	
	if (dev->ioctl)
	{
		return dev->ioctl(dev,GE_IO_DRAW_BITMAP,(UINT32)&io_cmd_par);
	}
			
	return ERR_NO_DEV;
}

RET_CODE ge_draw_line(struct ge_device *dev,ge_surface_desc_t *surface,ge_line_t *line)
{
	ge_io_cmd_par_t io_cmd_par;
	
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}

	io_cmd_par.par0 = (UINT32)surface;
	io_cmd_par.par1 = (UINT32)line;
	
	if (dev->ioctl)
	{
		return dev->ioctl(dev,GE_IO_DRAW_LINE,(UINT32)&io_cmd_par);
	}
			
	return ERR_NO_DEV;
}

/* ========Add for M3602 OSD dual output ============*/
ge_surface_desc_t *ge_create_surface_deo(struct ge_device *dev, ge_surface_desc_t *para, UINT32 flag, UINT8 gma_layer)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return NULL;
	}
	
	if (dev->create_surface_deo)
	{
		return dev->create_surface_deo(dev, para, flag, gma_layer);
	}

	return NULL;	
}

RET_CODE ge_release_surface_deo(struct ge_device *dev,ge_surface_desc_t *surface)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->release_surface_deo)
	{
		dev->release_surface_deo(dev,surface);
	}
	
	return RET_SUCCESS;
}

RET_CODE ge_show_onoff_deo(struct ge_device *dev, ge_surface_desc_t *surface, UINT8 on_off)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->show_onoff_deo)
	{
		dev->show_onoff_deo(surface, on_off);
	}
	
	return RET_SUCCESS;
}

RET_CODE ge_region_show_onoff_deo(struct ge_device *dev, ge_surface_desc_t *surface, UINT8 uRegionId,BOOL bOn)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->region_show_deo)
	{
		return dev->region_show_deo(dev, surface, uRegionId,bOn);
	}
	
	return RET_SUCCESS;
}
RET_CODE ge_set_pallette_deo(struct ge_device *dev, ge_surface_desc_t *surface,UINT8 *pallette,UINT16 color_num, UINT8 type,enum GE_PALLET_ALPHA_LEVEL alpha_level)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_pallette_deo)
	{
		dev->set_pallette_deo(dev, surface, pallette,color_num,type, alpha_level);
	}
	
	return RET_SUCCESS;
}

RET_CODE ge_get_pallette_deo(struct ge_device *dev, ge_surface_desc_t *surface, UINT8 *pallette,UINT16 color_num, UINT8 type)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_pallette_deo)
	{
		dev->get_pallette_deo(surface, pallette,color_num,type);
	}
	
	return RET_SUCCESS;
}

RET_CODE ge_modify_pallette_deo(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 index,UINT8 y,UINT8 cb,UINT8 cr,UINT32 alpha,enum GE_PALLET_ALPHA_LEVEL alpha_level)
{
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_pallette_deo)
	{
		dev->modify_pallette_deo(dev, surface, index,y, cb, cr, alpha, alpha_level);
	}
	
	return RET_SUCCESS;
}

RET_CODE ge_create_region_deo(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id,ge_rect_t *rect,ge_region_t *para)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->create_region_deo)
	{
		dev->create_region_deo(dev, surface, region_id,rect, para);
	}
	
	return RET_SUCCESS;

}
RET_CODE ge_delete_region_deo(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->delete_region)
	{
		dev->delete_region_deo(dev, surface, region_id);
	}
	
	return RET_SUCCESS;
}
RET_CODE ge_set_region_pos_deo(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id, ge_rect_t *rect)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_region_pos_deo)
	{
		dev->set_region_pos_deo(surface, region_id,rect);
	}
	
	return RET_SUCCESS;
}

RET_CODE ge_get_region_pos_deo(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id, ge_rect_t *rect)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->get_region_pos_deo)
	{
		dev->get_region_pos_deo(surface, region_id,rect);
	}
	
	return RET_SUCCESS;
}

RET_CODE ge_set_clip_deo(struct ge_device *dev,ge_surface_desc_t *surface, ge_clip_t* clip)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->set_clip_deo)
	{
		return dev->set_clip_deo(surface, clip);
	}
	
	return RET_SUCCESS;

}

RET_CODE ge_disable_clip_deo(struct ge_device *dev,ge_surface_desc_t *surface)
{
	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->disable_clip_deo)
	{
		dev->disable_clip_deo(surface);
	}
	
	return RET_SUCCESS;

}

RET_CODE ge_io_ctrl_deo(struct ge_device *dev,UINT32 dwCmd,UINT32 dwParam)
{
	if(NULL == dev)return RET_FAILURE;

	/* If device not running, exit */
	if ((dev->flags & HLD_DEV_STATS_UP) == 0)
	{
		return RET_FAILURE;
	}
	
	if (dev->ioctl_deo)
	{
		return dev->ioctl_deo((ge_surface_desc_t *)dev,dwCmd,dwParam);
	}
	
	return RET_FAILURE;
}
/* Add end */

