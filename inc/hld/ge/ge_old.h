
#ifndef _GE_OLD_H_
#define _GE_OLD_H_

RET_CODE ge_io_ctrl(struct ge_device *dev,UINT32 dwCmd,UINT32 dwParam);

RET_CODE ge_region_show_onoff(struct ge_device *dev, ge_surface_desc_t *surface, UINT8 uRegionId,BOOL bOn);

/*
  Sets the palette to the values stored in the specified palette.
*/
RET_CODE ge_set_pallette(struct ge_device *dev, ge_surface_desc_t *surface,UINT8 *pallette,UINT16 color_num, UINT8 type,enum GE_PALLET_ALPHA_LEVEL alpha_level);
/*
  Retrieves the palette and places it in the specified palette. 
*/
RET_CODE ge_get_pallette(struct ge_device *dev, ge_surface_desc_t *surface, UINT8 *pallette,UINT16 color_num, UINT8 type);
RET_CODE ge_modify_pallette(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 index,UINT8 y,UINT8 cb,UINT8 cr,UINT32 alpha,enum GE_PALLET_ALPHA_LEVEL alpha_level);

RET_CODE ge_show_onoff(struct ge_device *dev, ge_surface_desc_t *surface, UINT8 on_off);

/* 
  Create a new surface structure.
  The structure is initialised with a set of default parameters.
 */
ge_surface_desc_t *ge_create_surface(struct ge_device *dev, ge_surface_desc_t *para, UINT32 flag, UINT8 gma_layer);

/*
 Release the specified surface
*/
RET_CODE ge_release_surface(struct ge_device *dev, ge_surface_desc_t  *surface);

/*
 Creates a new region structure on the specified surface.
 The structure is initialised with a set of default parameters.
*/
RET_CODE ge_create_region(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id,ge_rect_t *rect,ge_region_t *para);

/*
 Deletes the region structure with the specified ID.
*/ 
RET_CODE ge_delete_region(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id);

/*
  Sets the region position on the TV srceen
*/
RET_CODE ge_set_region_pos(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id, ge_rect_t *rect);

/*
  Gets the region position
*/
RET_CODE ge_get_region_pos(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id, ge_rect_t *rect);

/*
   Draw a pixel to the specified surface
*/
RET_CODE ge_draw_pixel(struct ge_device *dev, ge_surface_desc_t *surface, int x, int y, UINT32 pixel);

/*
   Read a pixel from the specified surface
*/
UINT32      ge_read_pixel(struct ge_device *dev, ge_surface_desc_t *surface, int x, int y);

/*
  Draw one horizontal line on the specified surface
*/
RET_CODE ge_draw_hor_line(struct ge_device *dev, ge_surface_desc_t *surface, int x, int y, int w, UINT32 pixel);

/*
  Draw one vertical line on the specified surface
*/
RET_CODE ge_draw_ver_line(struct ge_device *dev, ge_surface_desc_t *surface, int x, int y, int h, UINT32 pixel);

/*
  Draw a filled rectangle of the specified dimensions and position on the specified surface
*/
RET_CODE ge_fill_rect(struct ge_device *dev, ge_surface_desc_t *surface,  ge_rect_t *rect, UINT32 color);

/* 
   Performs a bit block transfer (blit). 
*/
RET_CODE ge_blt(struct ge_device *dev, ge_surface_desc_t *dst,   ge_surface_desc_t *src,  ge_rect_t *dst_rect, ge_rect_t *src_rect, UINT32 op_flag);
RET_CODE ge_blt_ex(struct ge_device *dev, ge_surface_desc_t *dst,   ge_surface_desc_t *src1,  ge_surface_desc_t *src2, ge_rect_t *dst_rect, ge_rect_t *src1_rect, ge_rect_t *src2_rect, UINT32 op_flag);


//RET_CODE ge_get_para(struct ge_device *dev,ge_surface_desc_t *surf_blt, struct OSDPara* para);
RET_CODE ge_set_clip(struct ge_device *dev,ge_surface_desc_t *surface, ge_clip_t* clip);
RET_CODE ge_disable_clip(struct ge_device *dev,ge_surface_desc_t *surface);
RET_CODE ge_set_rop(struct ge_device *dev, ge_surface_desc_t *surface, UINT32 rop);
RET_CODE ge_get_rop(struct ge_device *dev, ge_surface_desc_t *surface, UINT32 *rop);
RET_CODE ge_set_colorkey(struct ge_device *dev, ge_surface_desc_t *surface, ge_color_key_t *color_key);
RET_CODE ge_get_colorkey(struct ge_device *dev, ge_surface_desc_t *surface, ge_color_key_t *color_key);
RET_CODE ge_set_alpha(struct ge_device *dev, ge_surface_desc_t *surface, ge_alpha_blend_t *alpha_blending);
RET_CODE ge_get_alpha(struct ge_device *dev, ge_surface_desc_t *surface, ge_alpha_blend_t *alpha_blending);
RET_CODE ge_set_colorredc(struct ge_device *dev, ge_surface_desc_t *surface, ge_col_rdc_t *color_redc);
RET_CODE ge_get_colorredc(struct ge_device *dev, ge_surface_desc_t *surface, ge_col_rdc_t *color_redc);
RET_CODE ge_set_resize(struct ge_device *dev, ge_surface_desc_t *surface, ge_resize_t *resize);
RET_CODE ge_get_resize(struct ge_device *dev, ge_surface_desc_t *surface, ge_resize_t *resize);
RET_CODE ge_set_flickfilt(struct ge_device *dev, ge_surface_desc_t *surface, ge_flick_filter_t *flick_filt);
RET_CODE ge_get_flickfilt(struct ge_device *dev, ge_surface_desc_t *surface, ge_flick_filter_t *flick_filt);
RET_CODE ge_set_planmask(struct ge_device *dev, ge_surface_desc_t *surface, UINT32 plane_mask);
RET_CODE ge_get_planmask(struct ge_device *dev, ge_surface_desc_t *surface, UINT32 *plane_mask);
RET_CODE ge_set_rgb2yuv( struct ge_device *dev, ge_surface_desc_t *surface, ge_YUV_RGB_conv_t *rgb2yuv);
RET_CODE ge_get_rgb2yuv( struct ge_device *dev, ge_surface_desc_t *surface, ge_YUV_RGB_conv_t *rgb2yuv);
RET_CODE ge_set_yuv2rgb( struct ge_device *dev, ge_surface_desc_t *surface, ge_YUV_RGB_conv_t *yuv2rgb);
RET_CODE ge_get_yuv2rgb( struct ge_device *dev, ge_surface_desc_t *surface, ge_YUV_RGB_conv_t *yuv2rgb);

// extended API for the need of lib
RET_CODE    ge_set_cur_region(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id);

// set the chosen region as the current region and its parameters(frame buf, color format, pitch - except region width and height)
RET_CODE ge_set_region_pars(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id,ge_region_pars_t *pars);
RET_CODE ge_get_region_pars(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id, ge_region_pars_t *pars);

// only support 1bit/pixel format resource at 2008.6.2
RET_CODE ge_draw_bitmap(struct ge_device *dev,ge_surface_desc_t *surface, ge_rect_t *rect,ge_bitmap_t *bitmap);

RET_CODE ge_draw_line(struct ge_device *dev,ge_surface_desc_t *surface,ge_line_t *line);

RET_CODE ge_lock(struct ge_device *dev);
RET_CODE ge_unlock(struct ge_device *dev);

/* Add for M3602 OSD dual output */
RET_CODE ge_region_show_onoff_deo(struct ge_device *dev, ge_surface_desc_t *surface, UINT8 uRegionId,BOOL bOn);
RET_CODE ge_set_pallette_deo(struct ge_device *dev, ge_surface_desc_t *surface,UINT8 *pallette,UINT16 color_num, UINT8 type,enum GE_PALLET_ALPHA_LEVEL alpha_level);
RET_CODE ge_get_pallette_deo(struct ge_device *dev, ge_surface_desc_t *surface, UINT8 *pallette,UINT16 color_num, UINT8 type);
RET_CODE ge_modify_pallette_deo(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 index,UINT8 y,UINT8 cb,UINT8 cr,UINT32 alpha,enum GE_PALLET_ALPHA_LEVEL alpha_level);
RET_CODE ge_show_onoff_deo(struct ge_device *dev, ge_surface_desc_t *surface, UINT8 on_off);
RET_CODE ge_io_ctrl_deo(struct ge_device *dev,UINT32 dwCmd,UINT32 dwParam);
ge_surface_desc_t *ge_create_surface_deo(struct ge_device *dev, ge_surface_desc_t *para, UINT32 flag, UINT8 gma_layer);
RET_CODE ge_release_surface_deo(struct ge_device *dev, ge_surface_desc_t  *surface);
RET_CODE ge_create_region_deo(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id,ge_rect_t *rect,ge_region_t *para);
RET_CODE ge_delete_region_deo(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id);
RET_CODE ge_set_region_pos_deo(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id, ge_rect_t *rect);
RET_CODE ge_get_region_pos_deo(struct ge_device *dev,ge_surface_desc_t *surface, UINT8 region_id, ge_rect_t *rect);
RET_CODE ge_set_clip_deo(struct ge_device *dev,ge_surface_desc_t *surface, ge_clip_t* clip);
RET_CODE ge_disable_clip_deo(struct ge_device *dev,ge_surface_desc_t *surface);
/* Add end */

#if 0
/****************************************************************************
 *
 * GRAPHICS LIBRARY SURFACE CAPABILITY FLAGS
 *
 ****************************************************************************/

/*
 * This bit is reserved. It should not be specified.
 */
#define GL_SCAPS_RESERVED1			0x00000001
/*
 * Indicates that this surface contains alpha-only information.
 * (To determine if a surface is RGBA/YUVA, the pixel format must be
 * interrogated.)
 */
#define GL_SCAPS_ALPHA				0x00000002
/*
 * Indicates that this surface is a backbuffer.	 It is generally
 * set by CreateSurface when the GL_SCAPS_FLIP capability bit is set.
 * It indicates that this surface is THE back buffer of a surface
 * flipping structure.	Graphics library supports N surfaces in a
 * surface flipping structure.	Only the surface that immediately
 * precedeces the GL_SCAPS_FRONTBUFFER has this capability bit set.
 * The other surfaces are identified as back buffers by the presence
 * of the GL_SCAPS_FLIP capability, their attachment order, and the
 * absence of the GL_SCAPS_FRONTBUFFER and GL_SCAPS_BACKBUFFER
 * capabilities.  The bit is sent to CreateSurface when a standalone
 * back buffer is being created.  This surface could be attached to
 * a front buffer and/or back buffers to form a flipping surface
 * structure after the CreateSurface call.  See AddAttachments for
 * a detailed description of the behaviors in this case.
 */
#define GL_SCAPS_BACKBUFFER			0x00000004
/*
 * Indicates a complex surface structure is being described.  A
 * complex surface structure results in the creation of more than
 * one surface.	 The additional surfaces are attached to the root
 * surface.  The complex structure can only be destroyed by
 * destroying the root.
 */
#define GL_SCAPS_COMPLEX			0x00000008
/*
 * Indicates that this surface is a part of a surface flipping structure.
 * When it is passed to CreateSurface the GL_SCAPS_FRONTBUFFER and
 * GL_SCAP_BACKBUFFER bits are not set.	They are set by CreateSurface
 * on the resulting creations.	The dwBackBufferCount field in the
 * GL_SURFACEDESC structure must be set to at least 1 in order for
 * the CreateSurface call to succeed.  The GL_SCAPS_COMPLEX capability
 * must always be set with creating multiple surfaces through CreateSurface.
 */
#define GL_SCAPS_FLIP				0x00000010
/*
 * Indicates that this surface is THE front buffer of a surface flipping
 * structure.  It is generally set by CreateSurface when the GL_SCAPS_FLIP
 * capability bit is set.
 * If this capability is sent to CreateSurface then a standalonw front buffer
 * is created.	This surface will not have the GL_SCAPS_FLIP capability.
 * It can be attached to other back buffers to form a flipping structure.
 * See AddAttachments for a detailed description of the behaviors in this
 * case.
 */
#define GL_SCAPS_FRONTBUFFER		0x00000020
/*
 * Indicates that this surface is any offscreen surface that is not an overlay,
 * texture, zbuffer, front buffer, back buffer, or alpha surface.  It is used
 * to identify plain vanilla surfaces.
 */
#define GL_SCAPS_OFFSCREENPLAIN		0x00000040
/*
 * Indicates that changes made to this surface are immediately visible.
 * It is always set for the primary surface and is set for overlays while
 * they are being overlayed and texture maps while they are being textured.
#define GL_SCAPS_VISIBLE			0x00008000
 */

/****************************************************************************
 *
 * GRAPHICS LIBRARY PIXELFORMAT FLAGS
 *
 ****************************************************************************/

/* The surface has alpha channel information in the pixel format. */
#define GE_PF_ALPHAPIXELS		0x00000001
/* The pixel format contains alpha only information */
#define GE_PF_ALPHA				0x00000002
/* pixel format is in premultiplied alpha format. */
#define GE_PF_ALPHAPREMULT      0x00008000
/* The FourCC code is valid. */
#define GE_PF_FOURCC			0x00000004
/* The surface is 1-bit color indexed. */
#define GE_PF_PALETTEINDEXED1	0x00000800
/* The surface is 2-bit color indexed. */
#define GE_PF_PALETTEINDEXED2	0x00001000
/* The surface is 4-bit color indexed. */
#define GE_PF_PALETTEINDEXED4	0x00000008
/* The surface is 8-bit color indexed. */
#define GE_PF_PALETTEINDEXED8	0x00000020
/* The RGB data in the pixel format structure is valid. */
#define GE_PF_RGB				0x00000040
/* pixel format is YUV - YUV data in pixel format struct is valid */
#define GE_PF_YUV				0x00000200
/* pixel format is MacroBlock mode */
#define GE_PF_MB				0x00100000
/* pixel format is in 422 mode */
#define GE_PF_422				0x00200000
/* pixel format is in 420 mode */
#define GE_PF_420				0x00400000
/* pixel format is in 888 mode */
#define GE_PF_888				0x00800000
/* pixel format is in 565 mode */
#define GE_PF_565				0x01000000

/****************************************************************************
 *
 * GRAPHICS LIBRARY SURFACE FLAGS
 *
 ****************************************************************************/
/* ddsCaps field is valid.	*/
#define GL_SD_CAPS				0x00000001	// default
/* dwTV_X is valid		*/
//#define GL_SD_TV_X				0x00100000
/* dwTV_Y is valid		*/
//#define GL_SD_TV_Y				0x00200000
/* dwHeight field is valid.	*/
#define GL_SD_HEIGHT			0x00000002
/* dwWidth field is valid.	*/
#define GL_SD_WIDTH				0x00000004
/* lPitch is valid.			*/
#define GL_SD_PITCH				0x00000008
/* dwAlphaBitDepth is valid.	*/
#define GL_SD_ALPHABITDEPTH		0x00000080
/* lpSurface is valid.		*/
#define GL_SD_LPSURFACE			0x00000800
/* ddpfPixelFormat is valid.	*/
#define GL_SD_PIXELFORMAT		0x00001000
/* ddckCKDestBlt is valid.		*/
#define GL_SD_CKDESTBLT			0x00004000
/* ddckCKSrcBlt is valid.		*/
#define GL_SD_CKSRCBLT			0x00010000
/* dwLinearSize is valid		*/
#define GL_SD_LINEARSIZE		0x00080000
/* lpAlphaSurface is valid		*/
#define GL_SD_ALPHASURFACE		0x00400000
/* lpClipperInfo is valid		*/
#define GL_SD_CLIPPERINFO		0x00800000
/* lpPaletteInfo is valid		*/
#define GL_SD_PALETTEINFO		0x01000000 
/* dwBackBufferCount is valid.	*/
//#define GL_SD_BACKBUFFERCOUNT	0x00000020

/* All input fields are valid.	xxx */
//#define GL_SD_ALL				0x000ff9ee

/****************************************************************************
 *
 * GRAPHICS LIBRARY BLT FLAGS
 *
 ****************************************************************************/

/* Use the alpha information in the pixel format or the alpha channel surface
 * attached to the destination surface as the alpha channel for this blt. */
#define GL_BLT_ALPHADEST					0x00000001
/* Use the dwConstAlphaDest field in the GL_BLTFX structure as the alpha channel
 * for the destination surface for this blt. */
#define GL_BLT_ALPHADESTCONSTOVERRIDE		0x00000002
/* Use the lpGL_SAlphaDest field in the GL_BLTFX structure as the alpha
 * channel for the destination for this blt. */
#define GL_BLT_ALPHADESTSURFACEOVERRIDE		0x00000008
/* Use the alpha information in the pixel format or the alpha channel surface
 * attached to the source surface as the alpha channel for this blt. */
#define GL_BLT_ALPHASRC						0x00000020
/* Use the dwConstAlphaSrc field in the GL_BLTFX structure as the alpha channel
 * for the source for this blt. */
#define GL_BLT_ALPHASRCCONSTOVERRIDE		0x00000040
/* Use the lpGL_SAlphaSrc field in the GL_BLTFX structure as the alpha channel
 * for the source for this blt. */
#define GL_BLT_ALPHASRCSURFACEOVERRIDE		0x00000100
/* Do this blt asynchronously through the FIFO in the order received.  If
 * there is no room in the hardware FIFO fail the call. */
#define GL_BLT_ASYNC						0x00000200
/* Uses the dwFillColor field in the GL_BLTFX structure as the RGB color
 * to fill the destination rectangle on the destination surface with. */
#define GL_BLT_COLORFILL					0x00000400
/* Uses the dwGL_FX field in the GL_BLTFX structure to specify the effects
 * to use for the blt. */
#define GL_BLT_GL_FX						0x00000800
/* Uses the dwGL_ROPS field in the GL_BLTFX structure to specify the ROPS
 * that are not part of the Win32 API. 
#define GL_BLT_GL_ROPS						0x00001000*/
/* Use the color key associated with the destination surface. */
#define GL_BLT_KEYDEST						0x00002000
/* Use the dckDestColorkey field in the GL_BLTFX structure as the color key
 * for the destination surface. */
#define GL_BLT_KEYDESTOVERRIDE				0x00004000
/* Use the color key associated with the source surface. */
#define GL_BLT_KEYSRC						0x00008000
/* Use the dckSrcColorkey field in the GL_BLTFX structure as the color key
 * for the source surface. */
#define GL_BLT_KEYSRCOVERRIDE				0x00010000
/* Use the dwROP field in the GL_BLTFX structure for the raster operation
 * for this blt.  These ROPs are the same as the ones defined in the Win32 API. */
#define GL_BLT_ROP							0x00020000
/* wait until the device is ready to handle the blt
 * this will cause blt to not return GL_ERR_WASSTILLDRAWING */
#define GL_BLT_WAIT                         0x01000000
/* Returns without blitting and also returns GL_ERR_WASSTILLDRAWING
 * if the blitter is busy */
#define GL_BLT_DONOTWAIT					0x08000000
#endif

#if 0
/*
	Obtains the blitter status.
*/
INT32 ge_get_blt_status(UINT32 dwFlags);

/*
	Retrieves the color key value
*/
INT32 ge_get_colorkey(ge_surface_desc_t lpSurface, UINT32 dwFlags, ge_color_key_t lpColorkey);

/*
	Sets the color key value
*/
INT32 ge_set_colorkey(ge_surface_desc_t lpSurface, UINT32 dwFlags, ge_color_key_t lpColorkey);

/*
	The Flip method makes the surface memory associated with the 
	GL_SCAPS_BACKBUFFER surface become associated with the front-buffer surface.
*/
INT32 ge_flip(ge_surface_desc_t lpSurface, ge_surface_desc_t lpGL_SurfaceTargetOverrid, UINT32 flags);

/*
	The GetFlipStatus method indicates whether the surface
	has finished its flipping process
*/
INT32 ge_get_flip_status(UINT32 dwFlags);

/*
	The GetClipper method retrieves the Clipper Rectangle associated with
	this surface
*/
INT32 ge_get_clipper(ge_surface_desc_t surface,ge_clip_info_t clip_info);

/*
	The SetClipper method attaches a clipper Rectangle to, or deletes
	one from, a surface
*/
INT32 ge_set_clipper(ge_surface_desc_t lpSurface, ge_clip_info_t lpClipper);

/*
	The GetPalette method retrieves the Palette structure associated with
	this surface.
*/
INT32 ge_get_palette(ge_surface_desc_t lpSurface, ge_palette_info_t lpPalette);

/*
	The SetPalette method attaches a palette structure to (or detaches one from)
	a surface. The surface uses this palette for all subsequent operations.
	The palette change takes place immediately, without regard to refresh timing.
*/
INT32 ge_set_palette(ge_surface_desc_t lpSurface, ge_palette_info_t lpPalette);

/*
	CreateSurface method creates a GRAPHICS LIBRARY SURFACE structure
*/
INT32 ge_create_surface(ge_palette_info_t lpSurfaceDesc,ge_surface_desc_t **lplpSurface);

/*
	ReleaseSurface method release a Surface structure
*/
void ge_release_surface(ge_surface_desc_t lpSurface);

/*
	The GetSurfaceDesc method retrieves a description of the surface in
	its current condition.
*/
INT32 GetSurfaceDesc(ge_surface_desc_t lpSurface, ge_palette_info_t lpSurfaceDesc);


/*
   The BltColorFill method performs a bit block transfer 
   (blit) of a single color to the specified destination rectangle. 
*/
//INT32 BltColorFill(PRECT lpDestRect, PGL_SURFACE lpDestSurface, UINT32 fillvalue);

/*
   The BltFast method performs a source copy blit or 
   transparent blit by using a source color key or destination color key.
*/
//INT32 BltFast(PGL_SURFACE lpDestSurface, PRECT lpSrcRect, PGL_SURFACE lpSrcSurface, UINT32 dwTrans);
/****************************************************************************
 *
 * BLTFAST FLAGS
 *
 ****************************************************************************/

#define GL_BLTFAST_NOCOLORKEY                    0x00000000
#define GL_BLTFAST_SRCCOLORKEY                   0x00000001
#define GL_BLTFAST_DESTCOLORKEY                  0x00000002
#define GL_BLTFAST_WAIT                          0x00000010


/****************************************************************************
 *
 * GRAPHICS LIBRARY GETBLTSTATUS FLAGS
 *
 ****************************************************************************/

/*
 * is it OK to blt now?
 */
#define GL_GBS_CANBLT			0x00000001

/*
 * is the blt to the surface finished?
 */
#define GL_GBS_ISBLTDONE		0x00000002

/****************************************************************************
 *
 * GRAPHICS LIBRARY Flip FLAGS
 *
 ****************************************************************************/
#define GL_FLIP_WAIT		0x00000001
#define GL_FLIP_EVEN		0x00000002 /* only valid for overlay */
#define GL_FLIP_ODD			0x00000004 /* only valid for overlay */
#define GL_FLIP_DONOTWAIT	0x00000020
/*
#define GL_FLIP_NOVSYNC		0x00000008
#define GL_FLIP_STEREO		0x00000010
#define DDFLIP_INTERVAL2	0x02000000
#define DDFLIP_INTERVAL3	0x04000000
#define DDFLIP_INTERVAL4	0x08000000
*/
/****************************************************************************
 *
 * GRAPHICS LIBRARY GETFLIPSTATUS FLAGS
 *
 ****************************************************************************/

/*
 * is it OK to flip now?
 */
#define GL_GFS_CANFLIP			0x00000001

/*
 * is the last flip finished?
 */
#define GL_GFS_ISFLIPDONE		0x00000002

/****************************************************************************
 *
 * GRAPHICS LIBRARY SURFACE SET/GET COLOR KEY FLAGS
 *
 ****************************************************************************/

/* Set if the structure contains a color space.	 Not set if the structure
 * contains a single color key.  */
#define GL_CKEY_COLORSPACE	0x00000001
/* Set if the structure specifies a color key or color space which is to be
 * used as a destination color key for blt operations. */
#define GL_CKEY_DESTBLT		0x00000002
/* Set if the structure specifies a color key or color space which is to be
 * used as a destination color key for overlay operations. 
#define GL_CKEY_DESTOVERLAY	0x00000004		*/
/* Set if the structure specifies a color key or color space which is to be
 * used as a source color key for blt operations. */
#define GL_CKEY_SRCBLT		0x00000008
/* Set if the structure specifies a color key or color space which is to be
 * used as a source color key for overlay operations. 
#define GL_CKEY_SRCOVERLAY	0x00000010		*/

/*
 * GL_Surface Capabilities
 */
typedef struct _GL_SCAPS
{
    UINT32	dwCaps;		// capabilities of surface wanted
} GL_SCAPS;


typedef struct _GL_SURFACEDESC
{
	UINT32		dwSize;			// size of the GL_SURFACEDESC structure
	UINT32		dwFlags;		// determines what fields are valid
//	UINT32		TV_X;			// start_x position on TV
//	UINT32		TV_Y;			// start_y position on TV
	UINT32		dwHeight;		// height of surface to be created
	UINT32		dwWidth;		// width of input surface
    union
    {
        INT32		lPitch;			// distance to start of next line (return value only)
        UINT32       dwLinearSize;           // Formless late-allocated optimized surface size
    };
    UINT32		dwBackBufferCount;	// number of back buffers requested
    UINT32		dwAlphaBitDepth;	// depth of alpha buffer requested
    UINT32		dwReserved;		// reserved
    UINT8 *		lpSurface;		// pointer to the associated surface memory
    UINT8 *		lp_y_top;
    UINT8 *      lp_y_bottom;
    UINT8 *      lp_c_top;
    UINT8 *      lp_c_bottom;
	//    GL_COLORKEY		ddckCKDestOverlay;	// color key for destination overlay use
    GL_COLORKEY		ddckCKDestBlt;		// color key for destination blt use
	//    GL_COLORKEY		ddckCKSrcOverlay;	// color key for source overlay use
    GL_COLORKEY		ddckCKSrcBlt;		// color key for source blt use
    GL_PIXELFORMAT	ddpfPixelFormat;	// pixel format description of the surface
    GL_SCAPS		ddsCaps;		// graphics library surface capabilities
} GL_SURFACEDESC, *PGL_SURFACEDESC;
#endif
#endif

