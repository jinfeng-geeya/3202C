/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    This is the demo implenment of the Graphics Porting APIs needed by iPanel MiddleWare.
    Maybe you should modify it accorrding to Platform.

    Note: the "int" in the file is 32bits

    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/
#include <udi/ipanel/ipanel_base.h>
#include <udi/ipanel/ipanel_graphics.h>

static struct ge_device* g_ge_dev;
static ge_surface_desc_t gfx_src,*gfx_dst = NULL;
unsigned char *gfx_buf = NULL;

#define ALI_SCREENW  720
#define ALI_SCREENH  576
#define ALI_GE_PITCH	(4 * ALI_SCREENW)

#define IPANEL_GRAPHICS_SURFACE GMA_HW_SURFACE_1


int ipanel_ge_open(void)
{
	g_ge_dev = (struct ge_device*)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
	if (RET_SUCCESS != ge_open(g_ge_dev))
		return  - 1;

	return 0;
}

int ipanel_ge_close(void)
{
	if (SUCCESS != ge_close(g_ge_dev))
		return  - 1;
	return 0;
}

static void ipanel_np_scale(void)
{
	enum TVSystem tvsys;
	struct vpo_device *vpo_dev;
	struct ge_device* ge_dev;
	ge_scale_par_29e_t par;
	
	vpo_dev = (struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
	vpo_ioctl(vpo_dev, VPO_IO_GET_OUT_MODE, (UINT32)(&tvsys));

	if (tvsys == NTSC || tvsys == NTSC_443)
		par.dst_h = 480;
	else
		par.dst_h = 576;

	par.src_h = 576;
	par.suf_id = 0;

	ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
	ge_io_ctrl(ge_dev, GE_IO_SCALE_OSD_29E, (UINT32)&par);
}


int ipanel_gfx_init(void)
{
	ge_region_t region_para;
	ge_rect_t dst_rect;
	INT32 pitch = 0;
	ge_anti_flick_t af;

        ipanel_ge_open();
	gfx_dst = NULL;
	gfx_buf = NULL;
	MEMSET((UINT8 *)&gfx_src, 0, sizeof(ge_surface_desc_t));

	if (sys_ic_get_rev_id() >= IC_REV_2)//m3202
	{
        pitch = 2*ALI_SCREENW;
        region_para.pixel_fmt = GE_PF_ARGB1555;
	}
	else
	{
        pitch = 4*ALI_SCREENW;
	    region_para.pixel_fmt = GE_PF_AYCBCR8888;//Only this color format can be used as the ge display frame
	}
    
	gfx_src.width = ALI_SCREENW;//IPANEL_SCREENW;
	gfx_src.height = ALI_SCREENH;//IPANEL_SCREENH;
	gfx_src.pitch = pitch; 
	gfx_src.endian = GE_PIXEL_LITTLE_ENDIAN;
	gfx_src.alpha_range = GE_LARGE_ALPHA;	
	/*A surface is a real display layer in HW*/
	gfx_dst = ge_create_surface(g_ge_dev, &gfx_src, 0, IPANEL_GRAPHICS_SURFACE);	

    if(NULL==gfx_dst)
    {
        dprintf("%s() ge_create_surface fail!\n", __FUNCTION__);
        return  - 1;        
    }
    
	dst_rect.left = 0;
	dst_rect.top = 0;
	dst_rect.width = ALI_SCREENW;
	dst_rect.height = ALI_SCREENH;
	region_para.glob_alpha_en = 0;
	region_para.glob_alpha = 0x7f;
	region_para.pallet_seg = 0;
	region_para.pitch = pitch;
	
	/*Create a region for displaying in the created surface. In the same surface ,there may has several regions. 
	But all the region should have the same color format and the area of every region can't coexist at the same row*/
	if(RET_SUCCESS!=ge_create_region(g_ge_dev, gfx_dst, 0, &dst_rect, &region_para))
	{
        dprintf("%s() ge_create_region fail!\n", __FUNCTION__);
        return  - 1;        
	}

	if (sys_ic_get_rev_id() >= IC_REV_2)//m3202
	{
        //enable the anti-flick
        af.layer = IPANEL_GRAPHICS_SURFACE;
        af.valid = 1;
        ge_io_ctrl(g_ge_dev,GE_IO_ANTI_FLICK_29E,(UINT32)&af);
        
        ge_fill_rect(g_ge_dev,gfx_dst,&dst_rect,0);
	}
    else
    {
    	/*Fill the rectangle of this region with defined color. the color data sequence is A-Cr-Y-Cb from high to low when 32bits*/
    	ge_fill_rect(g_ge_dev,gfx_dst,&dst_rect,0x801080);
    }


	MEMCPY((UINT8 *)&gfx_src, gfx_dst, sizeof(ge_surface_desc_t));
	gfx_src.surf_id = 2;//GMA_MEM_SURFACE

	ipanel_np_scale();
	ge_show_onoff(g_ge_dev,gfx_dst, 1);

	//gfx_buf = (unsigned char*)MALLOC(IPANEL_SCREENW*IPANEL_SCREENH*2);
//	gfx_buf = (unsigned char*)ipanel_porting_malloc(IPANEL_SCREENW*IPANEL_SCREENH*2);
	//MEMSET(gfx_buf,0,(IPANEL_SCREENW*IPANEL_SCREENH*2));//Set the buffer to transparent and black as default.

	return 0;
}

int ipanel_gfx_exit(void)
{
	ge_anti_flick_t af;

    if (sys_ic_get_rev_id() >= IC_REV_2)//m3202
	{
        af.layer = IPANEL_GRAPHICS_SURFACE;
        af.valid = 0;
        ge_io_ctrl(g_ge_dev,GE_IO_ANTI_FLICK_29E,(UINT32)&af);
    }
    
	//if(gfx_buf)
	//	FREE(gfx_buf);
//		ipanel_porting_free(gfx_buf);

	if(RET_SUCCESS!=ge_delete_region(g_ge_dev,gfx_dst,0))
	{
		dprintf("[ipanel_gfx_exit]ge_delete_region failed..\n");
		return -1;
	}

	if(RET_SUCCESS!=ge_release_surface(g_ge_dev,gfx_dst))
	{
		dprintf("[ipanel_gfx_exit]ge_release_surface failed..\n");
		return -1;
	}

        ipanel_ge_close();
    return 0;
}

/******************************************************************/
/*Description: Get Display information.                           */
/*             width,height: VOD/IPTV output area availability,   */
/*             bufWidth,bufHeight: show the actual buffer size,   */
/*pBuffer: Return to the VOD/IPTV directly output the buffer area */
/*pointer address£»if *pBuffer = 0; VOD/IPTV can auto-allocate the*/
/*buffer area£¨buffer£©£¬then later call eis_draw_image output out*/
/* to the actual display buffer area £¨buffer£©£»if *pBuffer =    */
/* actual display buffer area address£¬VOD/IPTV shall directly    */
/* output data to buffer area£¬and at the same time eis_draw_image*/
/* can also be invoked.                                           */
/*Input      : Address of wid, hgt, FrameBuffer, bufW, bufH       */
/*Output     : width, height, FB, bufWidth, bufHeight             */
/*Return     : 0 -- success, -1 -- fail.                          */
/******************************************************************/
int ipanel_porting_graphics_getInfo(int *width,
                                   int *height,
                                void **pbuffer,
                                 int *bufWidth,
                                int *bufHeight)
{
    *width  = *bufWidth  = IPANEL_SCREENW;
    *height = *bufHeight = IPANEL_SCREENH;
    *pbuffer= NULL;

    return 0;
}

/******************************************************************/
/*Description: Set Palette, Only 8bpp or less 8bpp need the func. */
/*Input      : pal[] -- color Array, format 0x00RRGGBB,           */
/*     npals -- counts of colors in Palette, 16-4bpp 256-8bpp etc */
/*Output     : Install Palette datas to STB Palette               */
/*Return     : 0 -- success, -1 -- fail.                          */
/******************************************************************/
int ipanel_porting_install_palette(unsigned int *pal, int npals)
{
    int ret = -1;

    return ret;
}

/******************************************************************/
/*Description: Draw a rectangle image.                            */
/*Input      : start point(x,y),width(w),height(h);               */
/*             color data(bits), and data width(w_src) per line;  */
/*Output     : No                                                 */
/*Return     : 0 -- success, Other fail.                          */
/******************************************************************/
/**Note: segguest OSD address is  0xC0000000 in following.        */
int ipanel_porting_draw_image(int x, int y, int w, int h,
                                     unsigned char *bits,
                                               int w_src)
{
	ge_rect_t dst_rect, src_rect;
	int i=0,j=0;
	unsigned short *src = (unsigned short *)bits;
	unsigned short *dst = (unsigned short *)gfx_buf;

	dst_rect.left = x+DISPLAY_POS_X;
	dst_rect.top = y+DISPLAY_POS_Y;
	dst_rect.width = w;
	dst_rect.height = h;
	src_rect.left = 0;
	src_rect.top = 0;
	src_rect.width = w;
	src_rect.height = h;

//	dprintf("[ipanel_porting_draw_image]x=%d,y=%d,w=%d,h=%d,w_src=%d..\n",x,y,w,h,w_src);
/*	
	for(i=0;i<h;i++){
		for(j=0;j<w;j++){
			*dst = *src;
			dst++;
			src++;			
		}
		src +=(w_src-w);
	}
    */
       //osal_cache_flush((void *)gfx_buf,w*h*2);
       osal_cache_flush((void *)bits,w_src*h*2);
	//gfx_src.region[0].addr = (unsigned char *)gfx_buf; 
	gfx_src.region[0].addr = bits; 
	gfx_src.region[0].pitch = w_src<<1;//w<<1;
	gfx_src.region[0].pixel_fmt = GE_PF_ARGB1555;//GE_PF_RGB444;
	/*resource format, support GE_PF_ARGB4444,GE_PF_ARGB1555,GE_PF_ACLUT88*/

	if (sys_ic_get_rev_id() >= IC_REV_2)//m3202
	{
        ge_blt(g_ge_dev,gfx_dst,&gfx_src,&dst_rect,&src_rect,GE_BLT_SRC2_VALID);
	}
	else
	{
    	/*Write a chosen picture into the ge display frame with the RGB2YUV operation*/
    	ge_blt(g_ge_dev,gfx_dst,&gfx_src,&dst_rect,&src_rect,GE_BLT_SRC2_VALID|GE_BLT_RGB2YUV);
	}

    return 0;
}

#if 1
int ipanel_show_test(int flag)
{
	if(flag==1)
		ge_show_onoff(g_ge_dev,gfx_dst, 1);
	else
		ge_show_onoff(g_ge_dev,gfx_dst, 0);
	return 0;
}
#endif
