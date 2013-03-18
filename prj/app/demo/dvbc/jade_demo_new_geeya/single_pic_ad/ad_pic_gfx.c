#include <sys_config.h>

#ifdef SHOW_PICS
#if(SHOW_PICS == SHOW_SINGLE_PIC)

#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hld/ge/ge.h>
#include <hld/osd/osddrv_dev.h>

#include "ad_pic_gfx.h"

#define AD_GFX_DEBUG

#ifdef AD_GFX_DEBUG
#define AD_GFX_PRINTF		libc_printf
#else
#define AD_GFX_PRINTF(...)
#endif

static ge_surface_desc_t *dst=NULL,*src=NULL;
static UINT8 region_id = 0;

#if 0
//use array to create several regions
static ge_rect_t ad_region_rects[] = 
{
//	{0,110,720,28},  //reserved for the title info
    {0,160,720,240},
//    {0,420,720,28}, //reserved for the bottom info
};
#endif

enum
{
	picture_view = 0,
	channel_adv,
	main_menu_adv,
	max_adv,
};

static ge_rect_t ad_region_rects[] = 
{
	{0,160,720,360},   //menu system picture view   
	{500,400,178,100}, //channel adv 
	{268,445,178,100},  //main menu adv
};

extern UINT8 creat_ge_surface(void);
extern ge_surface_desc_t * get_ge_dst_surface(void);
extern void set_ge_dst_surface(ge_surface_desc_t *dst);
INT32 ad_pic_gfx_init(UINT8 type)
{
	struct ge_device *ge_dev;
	ge_surface_desc_t surf_para;
	ge_region_t region_para;
	ge_rect_t rect;
	ge_anti_flick_t af;
	UINT8 i = 0;

	if(type > max_adv-1)
		return -1;

	ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
	if (ge_dev == NULL)
	{
		AD_GFX_PRINTF("%s() ge device find failed!\n", __FUNCTION__);
		return -1;
	}


	if (SUCCESS != ge_open(ge_dev))
	{
		AD_GFX_PRINTF("%s() ge_open failed!\n", __FUNCTION__);
		return -1;
	}		
	
	surf_para.width = 720;
	surf_para.height = 576;
	surf_para.pitch =  surf_para.width << 1; 
	surf_para.endian = GE_PIXEL_LITTLE_ENDIAN;
	surf_para.alpha_range = GE_LARGE_ALPHA;
	
	if(get_ge_dst_surface() == NULL)
	{
		creat_ge_surface();
	}

	dst = get_ge_dst_surface();
	if(dst == NULL)
		return -1;

	region_para.pixel_fmt = GE_PF_ARGB1555;
	region_para.glob_alpha_en = 0;
	region_para.glob_alpha = 0x7f;
	region_para.pallet_seg = 0;
	region_para.pitch = surf_para.width << 1;

	surf_para.width = 720;
	surf_para.height = 576;
	surf_para.pitch =  surf_para.width << 1; 
	surf_para.endian = GE_PIXEL_LITTLE_ENDIAN;
	surf_para.alpha_range = GE_LARGE_ALPHA;
	
	src = ge_create_surface(ge_dev, &surf_para, 0, GMA_MEM_SURFACE);
	if (src == NULL)
	{
		AD_GFX_PRINTF("%s() ge_create_surface for src failed!\n", __FUNCTION__);
		return -1;
	}	

	
    for(i=0;i<1;i++)
    {
		if (SUCCESS != ge_create_region(ge_dev, src, 3, &ad_region_rects[type], &region_para))
		{
			AD_GFX_PRINTF("%s() ge_create_region for src failed!\n", __FUNCTION__);
			return -1;
		}
    } 

	ge_lock(ge_dev);
    for(i=0;i<1;i++)
    {
		MEMCPY(&rect,&ad_region_rects[type],sizeof(ge_rect_t));
		//rect.top = 0; 
		ge_set_cur_region(ge_dev,dst,0);       
		ge_fill_rect(ge_dev,dst,&rect,0x0);
		region_id = i;
    }
	ge_unlock(ge_dev);

	if (sys_ic_get_rev_id() >= IC_REV_2)//m3202
	{
		//enable the anti-flick
		af.layer = GMA_HW_SURFACE_1;
		af.valid = 1;
		ge_io_ctrl(ge_dev, GE_IO_ANTI_FLICK_29E, (UINT32)&af);
	}
	//ge_show_onoff(ge_dev, dst, TRUE);
    
	return SUCCESS;
}

void ad_pic_show_on(void)
{
	struct ge_device *ge_dev;
	
	if(NULL == dst)
    {
        libc_printf("%s(),ERR!!!",__FUNCTION__);
        return -1;
    }
	
	ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
	if (ge_dev == NULL)
	{
		AD_GFX_PRINTF("%s() ge device find failed!\n", __FUNCTION__);
		return ;
	}
	ge_show_onoff(ge_dev, dst, TRUE);
}

INT32 ad_pic_gfx_exit(UINT8 type)
{
	struct ge_device *ge_dev;
	ge_anti_flick_t af;
	ge_rect_t rect;
    UINT8 i;

	ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
	if (ge_dev == NULL)
	{
		AD_GFX_PRINTF("%s() ge device find failed!\n", __FUNCTION__);
		return -1;
	}
	
    //close the selected region 
	ge_lock(ge_dev);
    for(i=0;i<1;i++)
    {
		if(SUCCESS != ge_delete_region(ge_dev, src, 3))
		{
			AD_GFX_PRINTF("%s() ge_delete_region %d for src failed!\n", __FUNCTION__,i);
			return -1;
		}
    }
	ge_unlock(ge_dev);

    
	if(SUCCESS != ge_release_surface(ge_dev, src))
	{
		AD_GFX_PRINTF("%s() ge_release_surface for src failed!\n", __FUNCTION__);
		return -1;
	}

	ge_lock(ge_dev);
    for(i=0;i<1;i++)
    {
    	ge_set_cur_region(ge_dev, dst, 0);
		MEMCPY(&rect,&ad_region_rects[type],sizeof(ge_rect_t));
		//rect.top = 0; 
		ge_set_cur_region(ge_dev,dst,0);        
		ge_fill_rect(ge_dev,dst,&rect,0x0);
    }
	ge_unlock(ge_dev);

	return SUCCESS;
}

/*
INT32 ad_pic_gfx_draw(struct OSDRect *dst_rc, struct OSDRect *src_rc, UINT8 *buf, UINT16 pitch)
{
	ge_rect_t dst_rect, src_rect;
	struct ge_device *ge_dev;
	INT32 ret;
	
	if (buf == NULL || dst_rc == NULL || src_rc == NULL)
	{
		AD_GFX_PRINTF("%s() para illegal 0!\n", __FUNCTION__);
		return -1;
	}

	if (dst_rc->uWidth == 0 || dst_rc->uHeight == 0 || \
		src_rc->uWidth == 0 || src_rc->uHeight == 0)
	{
		AD_GFX_PRINTF("%s() para illegal 1!\n", __FUNCTION__);
		return -1;
	}

	ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
	if (ge_dev == NULL)
	{
		AD_GFX_PRINTF("%s() ge device find failed!\n", __FUNCTION__);
		return -1;
	}
	
	src_rect.left = src_rc->uLeft;
	src_rect.top = src_rc->uTop;
	src_rect.width = src_rc->uWidth;
	src_rect.height = src_rc->uHeight;
	
	dst_rect.left = dst_rc->uLeft;
	dst_rect.top = dst_rc->uTop;
	dst_rect.width = dst_rc->uWidth;
	dst_rect.height = dst_rc->uHeight;

	ge_lock(ge_dev);

	src->region[0].addr = buf;
	if (pitch == 0)
		src->region[0].pitch = src_rc->uWidth << 1;
	else
		src->region[0].pitch = pitch << 1;
	src->region[0].pixel_fmt = GE_PF_ARGB1555;

	ge_set_cur_region(ge_dev, dst, 0);
	ge_set_cur_region(ge_dev, src, 0);

	if (sys_ic_get_rev_id() >= IC_REV_2)//m3202
		ret = ge_blt(ge_dev, dst, src, &dst_rect, &src_rect, GE_BLT_SRC2_VALID );
	else
		ret = ge_blt(ge_dev, dst, src, &dst_rect, &src_rect, GE_BLT_SRC2_VALID | GE_BLT_RGB2YUV);

	ge_unlock(ge_dev);
	
	return ret;
 }
*/
INT32 ad_pic_gfx_draw(struct OSDRect *rect, UINT8 *buf)
{
	ge_rect_t dst_rect, src_rect;
	struct ge_device *ge_dev;
	INT32 ret;

	if((NULL == src) || (NULL == dst))
    {
        libc_printf("%s(),ERR!!!",__FUNCTION__);
        return -1;
    }

	if (buf == NULL)
	{
		AD_GFX_PRINTF("%s() para buf illegal!\n", __FUNCTION__);
		return -1;
	}

	ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
	if (ge_dev == NULL)
	{
		AD_GFX_PRINTF("%s() ge device find failed!\n", __FUNCTION__);
		return -1;
	}
	
	src_rect.left = 0;
	src_rect.top = 0;
	src_rect.width = rect->uWidth;
	src_rect.height = rect->uHeight;
	
	dst_rect.left = rect->uLeft;
	dst_rect.top = rect->uTop;   // bmp, jpg
	dst_rect.width = rect->uWidth;
	dst_rect.height = rect->uHeight;

	src->region[3].addr = buf;
	src->region[3].pitch = dst_rect.width << 1;
	src->region[3].pixel_fmt = GE_PF_ARGB1555;

	ge_lock(ge_dev);

	//ge_set_cur_region(ge_dev, dst, 0);
	//ge_set_cur_region(ge_dev, src, 0);
	ge_set_cur_region(ge_dev, dst, 0);
	ge_set_cur_region(ge_dev, src, 3);
	

	if (sys_ic_get_rev_id() >= IC_REV_2)//m3202
		ret = ge_blt(ge_dev, dst, src, &dst_rect, &src_rect, GE_BLT_SRC2_VALID );
	else
		ret = ge_blt(ge_dev, dst, src, &dst_rect, &src_rect, GE_BLT_SRC2_VALID | GE_BLT_RGB2YUV);

	ge_unlock(ge_dev);
	
	return ret;
 }


INT32 ad_pic_gfx_fill(struct OSDRect *rect, UINT16 color)
{
	ge_rect_t dst_rect;
	struct ge_device *ge_dev;

	if((NULL == src) || (NULL == dst))
    {
        libc_printf("%s(),ERR!!!",__FUNCTION__);
        return -1;
    }

	if (rect == NULL)
	{
		AD_GFX_PRINTF("%s() para illegal 0!\n", __FUNCTION__);
		return -1;
	}

	if (rect->uWidth == 0 || rect->uHeight == 0)
	{
		AD_GFX_PRINTF("%s() para illegal 1!\n", __FUNCTION__);
		return -1;
	}


	ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
	if (ge_dev == NULL)
	{
		AD_GFX_PRINTF("%s() ge device find failed!\n", __FUNCTION__);
		return -1;
	}

	dst_rect.left = rect->uLeft;
	dst_rect.top = rect->uTop;
	dst_rect.width = rect->uWidth;
	dst_rect.height = rect->uHeight;

	ge_lock(ge_dev);
	ge_set_cur_region(ge_dev, dst, 0);
	ge_fill_rect(ge_dev, dst, &dst_rect, color);
	ge_unlock(ge_dev);

	return SUCCESS;
}


INT32 ad_pic_gfx_clear(UINT8 type)
{
	ge_rect_t rect;
	struct ge_device *ge_dev;

	if((NULL == src) || (NULL == dst))
    {
        libc_printf("%s(),ERR!!!",__FUNCTION__);
        return -1;
    }
	
	rect.left = ad_region_rects[type].left;
	rect.top = ad_region_rects[type].top;//0;
	rect.width= ad_region_rects[type].width;
	rect.height= ad_region_rects[type].height;

	ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
	if (ge_dev == NULL)
	{
		AD_GFX_PRINTF("%s() ge device find failed!\n", __FUNCTION__);
		return -1;
	}
	ge_lock(ge_dev);
	ge_set_cur_region(ge_dev, dst, 0);
	ge_fill_rect(ge_dev, dst, &rect, 0x0);
	ge_unlock(ge_dev);
}

struct ge_surface_desc_t * ad_pic_get_ge_surface(UINT32 ad_surface)
{
	if (ad_surface == AD_DST_SURFACE)
		return dst;
	else
		return src;
}

UINT8 ad_pic_get_ge_region_id(void)
{
	return region_id;
}

#endif
#endif

