/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *  (C)
 *  File: png_out.c
 *  (I)
 *  Description: image and ge operation related routine
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2008.2.1			Sam		Create
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
#include <api/libmp/png.h>
#include "png_core.h"
#include "png_debug.h"

//#define ALPHA_7BITS

#define Y_COF_R		0x41CA//0.257<<16	
#define Y_COF_G		0x8106//0.504<<16	
#define Y_COF_B		0x1916//0.098<<16
#define C_COF_Cb	0x2049b// 2.018<<16
#define C_COF_Cr		0x19893// 1.596<<16

#define CB_COF_R	0x25e3
#define CB_COF_G	0x4A7E
#define CB_COF_B	0x7062
#define CR_COF_R	0x7062
#define CR_COF_G	0x5e35
#define CR_COF_B	0x122D

static void rgb2yuv(UINT8 *data)
{
	UINT8 *dst = data;
	int blue = *data++;
	int green = *data++;
	int red = *data++;
#ifdef ALPHA_7BITS
	UINT8 alpha = (*data)>>1;
#else
	UINT8 alpha = *data;
#endif
	int y,cb,cr;

	y = (red*Y_COF_R + green*Y_COF_G + blue*Y_COF_B);
	cb = - red*CB_COF_R - green*CB_COF_G + blue*CB_COF_B;
	cr = red*CR_COF_R - green*CR_COF_G - blue*CR_COF_B;
	y = (y>>16) + 16;
	cb = (cb>>16) + 128;
	cr = (cr>>16) + 128;	
	if(y<16)
		y = 16;
	else if(y > 235)
		y = 235;
	if(cb<16)
		cb = 16;
	else if(cb>240)
		cb = 240;
	if(cr<16)
		cr = 16;
	else if(cr>240)
		cr = 240;

	*(UINT32 *)dst = (alpha<<24) | (cr<<16) | (y<<8) | cb; 
}

PNG_INLINE void color0(UINT8 *in,UINT8 *out,UINT32 w,UINT32 bits, UINT32 plte)
{
	if(8 == bits)
	{
		while(w--)
		{
			*(out + 2) = *in++;
			*(out + 1) = 0x80;
			*out = 0x80;
			*(out + 3) = 0xFF;
			out += 4;
		}
	}
	else if(16 == bits)
	{
		while(w--)
		{
			*(out + 2) = *in;in += 2;
			*(out + 1) = 0x80;
			*out = 0x80;
			*(out + 3) =  0xFF;
			out += 4;
		}
	}	
}

PNG_INLINE void color2(UINT8 *in,UINT8 *out,UINT32 w,UINT32 bits, UINT32 plte)
{
	if(8 == bits)
	{
		while(w--)
		{
			*(out + 2) = *in++;
			*(out + 1) = *in++;
			*out = *in++;
			*(out + 3) = 0xFF;
			rgb2yuv(out);
			out += 4;
		}
	}
	else if(16 == bits)
	{
		while(w--)
		{
			*(out + 2) = *in;in += 2;
			*(out + 1) = *in;in += 2;
			*out = *in;in += 2;
			*(out + 3) = 0xFF;
			rgb2yuv(out);
			out += 4;
		}
	}	
	
}

PNG_INLINE void color3(UINT8 *in,UINT8 *out,UINT32 w, UINT32 bits, UINT32 plte)
{
	UINT32 data = 0;

	if(8 == bits)
	{	
		while(w--)
		{
			data = *(((UINT32 *)plte) + *in++); 
			*out = data&0xFF;
			*(out + 1) = (data>>8)&0xFF;
			*(out + 2) = (data>>16)&0xFF;
			*(out + 3) = (data>>24)&0xFF;
			rgb2yuv(out);
			out += 4;
		}
	}
	else if(4 == bits)
	{
		UINT8 idx = 0;
		int cnt = 0;
		
		while(w--)
		{
			if(0 == cnt)
			{
				idx = *in++;
				cnt = 8;
			}
			cnt -= 4;
			data = *(((UINT32 *)plte) + ((idx>>cnt)&0xF)); 
			*out = data&0xFF;
			*(out + 1) = (data>>8)&0xFF;
			*(out + 2) = (data>>16)&0xFF;
			*(out + 3) = (data>>24)&0xFF;
			rgb2yuv(out);
			out += 4;
		}		
	}
	else if(2 == bits)
	{
		UINT8 idx = 0;
		int cnt = 0;
		
		while(w--)
		{
			if(0 == cnt)
			{
				idx = *in++;
				cnt = 8;
			}
			cnt -= 2;
			data = *(((UINT32 *)plte) + ((idx>>cnt)&0x3)); 
			*out = data&0xFF;
			*(out + 1) = (data>>8)&0xFF;
			*(out + 2) = (data>>16)&0xFF;
			*(out + 3) = (data>>24)&0xFF;
			rgb2yuv(out);
			out += 4;
		}			
	}
}

PNG_INLINE void color3_o(UINT8 *in,UINT8 *out,UINT32 w, UINT32 bits, UINT32 plte)
{
	UINT32 data = 0;

	if(8 == bits)
	{	
		while(w--)
			*out++ = *in++; 
	}
	else if(4 == bits)
	{
		UINT8 idx = 0;
		int cnt = 0;
		
		while(w--)
		{
			if(0 == cnt)
			{
				idx = *in++;
				cnt = 8;
			}
			cnt -= 4;
			*out++ = ((idx>>cnt) & 0xF);
		}		
	}
	else if(2 == bits)
	{
		UINT8 idx = 0;
		int cnt = 0;
		
		while(w--)
		{
			if(0 == cnt)
			{
				idx = *in++;
				cnt = 8;
			}
			cnt -= 2;
			*out++ = (idx>>cnt)&0x3;
		}			
	}
}

PNG_INLINE void color4(UINT8 *in,UINT8 *out,UINT32 w,UINT32 bits, UINT32 plte)
{
	if(8 == bits)
	{
		while(w--)
		{
			*(out + 2) = *in++;
			*(out + 1) = 0x80;
			*out = 0x80;
#ifdef ALPHA_7BITS
			*(out + 3) = (*in++)>>1;
#else
			*(out + 3) = *in++;
#endif
			out += 4;
		}
	}
	else if(16 == bits)
	{
		while(w--)
		{
			*(out + 2) = *in;in += 2;
			*(out + 1) = 0x80;
			*out = 0x80;
#ifdef ALPHA_7BITS
			*(out + 3) = (*in)>>1;
#else
			*(out + 3) = *in;
#endif
			in += 2;
			out += 4;
		}
	}	
}

PNG_INLINE void color6(UINT8 *in,UINT8 *out,UINT32 w,UINT32 bits, UINT32 plte)
{
	if(8 == bits)
	{
		while(w--)
		{
			*(out + 2) = *in++;
			*(out + 1) = *in++;
			*out = *in++;
			*(out + 3) = *in++;
			rgb2yuv(out);
			out += 4;
		}
	}
	else if(16 == bits)
	{
		while(w--)
		{
			*(out + 2) = *in;in += 2;
			*(out + 1) = *in;in += 2;
			*out = *in;in += 2;
			*(out + 3) = *in;in += 2;
			rgb2yuv(out);
			out += 4;
		}
	}		
}

#if 1
void png_out_line(struct png_core *core)
{
	struct png_out *out = &(core->out);
	struct png_image *img = out->img;
	UINT8 *out_line = img->graphic_buf + (out->valid_lines * img->pitch);
	
	out->write_line(core->filter.line_out,out_line,img->size.w,out->bits, out->plt);
	out->valid_lines++;
}
#else

extern ge_surface_desc_t *g_ge_surface;

void png_out_line(struct png_core *core)
{
	struct png_out *out = &(core->out);
	struct png_image *img = out->img;
	UINT8 *out_line = NULL;

	out_line = g_ge_surface->region[0].addr;
	out_line += ((out->valid_lines + 100)*g_ge_surface->region[0].pitch) + 400;
	out->write_line(core->filter.line_out,out_line,img->size.w,out->bits);
	out->valid_lines++;
}


void png_out_write_img(struct png_image *img,struct png_view_par *par)
{
	ge_surface_desc_t *dst = (ge_surface_desc_t *)par->handle1,src;
	ge_rect_t dst_rect,src_rect;
	UINT32 op_flag = 0;

	MEMCPY((void *)&src,(void *)dst,sizeof(ge_surface_desc_t));

	src_rect.top = src_rect.left = 0;
	src_rect.width = img->size.w;
	src_rect.height = img->size.h;

	dst_rect.left = par->pos.x;
	dst_rect.top = par->pos.y;
	dst_rect.width = src_rect.width;
	dst_rect.height = src_rect.height;

	src.cur_region = 0;
	src.region[0].addr = img->graphic_buf; 
	src.region[0].pitch = img->pitch;
	osal_cache_flush(img->graphic_buf,img->pitch * img->size.h);
	op_flag = GE_BLT_SRC2_VALID | GE_BLT_SRC1_VALID | GE_BLT_ALPHA_BLEND;
	if(PNG_AYCBCR8888 == img->color)
		src.region[0].pixel_fmt = GE_PF_AYCBCR8888;
/*	else if(PNG_ARGB8888 == img->color)
	{
		op_flag |= GE_BLT_RGB2YUV;
		src.region[0].pixel_fmt = GE_PF_ARGB8888;
	}
*/	else
	{
		PNG_PRINTF("Don't suppot other output color format\n");
		return;
	}
		
	dst->cur_region = (UINT8)par->handle2;
	dst->alpha_blend.glob_alpha_en = 0;
	dst->alpha_blend.glob_alpha = 0x7f;
	dst->alpha_blend.src_mode = GE_AB_SRC_NOT_PREMULTI;
	
	ge_blt_ex((struct ge_device *)par->dev,dst,dst,&src,&dst_rect,&dst_rect,&src_rect,op_flag);
}
#endif

PNG_RET png_out_init(struct png_core *core)
{
	struct png_cfg *cfg = &core->cfg;
	struct png_out *out = &(core->out);
	struct png_image *img = out->img;
	struct png_ihdr *ihdr = &core->info.IHDR;
	struct png_inf_scan *scan = &core->inf.scan;

	out->bits = ihdr->bit_depth;

#if 0	
	if(out->bits < 8)
	{
		PNG_PRINTF("Don't support sample bit depth less than 8\n");
		return PNG_FALSE;
	}
#endif
	if(PNG_CF_DEFINED_AUTOMATICALLY != img->cf_defined)
	{
		switch(ihdr->color_type)
		{
			case 0:
				img->color = PNG_AYCBCR8888;
				out->write_line = color0;
				break;
			case 2:
				img->color = PNG_ARGB8888;
				out->write_line = color2;
				break;			
			case 3:
				img->color = PNG_ARGB8888;
				out->write_line = color3;
				out->plt = (UINT32)core->info.PLTE.palette;
				break;			
			case 4:
				img->color = PNG_AYCBCR8888;
				out->write_line = color4;
				break;			
			case 6:
				img->color = PNG_ARGB8888;
				out->write_line = color6;
				break;
			default:
				return PNG_FALSE;
		}

		img->color = PNG_AYCBCR8888;
	}
	else
	{
		switch(ihdr->color_type)
		{
			case 0:
				img->color = PNG_ARGB8888;
				out->write_line = color0;
				break;
			case 2:
				img->color = PNG_ARGB8888;
				out->write_line = color2;
				break;			
			case 3:
				img->color = PNG_ACLUT8;
				img->pallete = (void *)core->info.PLTE.palette;
				img->pal_size = core->info.PLTE.num;				
				out->write_line = color3_o;
				out->plt = (UINT32)core->info.PLTE.palette;
				break;			
			case 4:
				img->color = PNG_AYCBCR8888;
				out->write_line = color4;
				break;			
			case 6:
				img->color = PNG_ARGB8888;
				out->write_line = color6;
				break;
			default:
				return PNG_FALSE;
		}		
	}
	
	out->valid_lines = 0;
	img->size.w = ihdr->width;
	img->size.h = ihdr->height;
	
	if(PNG_ACLUT8 == img->color)
	{
		img->pitch = img->size.w;

		int i = 0;
		UINT8 *addr = (UINT8 *)img->pallete;
		UINT32 data = 0;

		for(i = 0;i < (int)img->pal_size;i++)
		{
			rgb2yuv(addr);
			data = *addr<<8;
			data |= *(addr + 1);
			data |= *(addr + 2)<<16;
			data |= *(addr + 3)<<24;

			*addr = data&0xFF;
			*(addr + 1) = (data>>8) & 0xFF;
			*(addr + 2) = (data>>16) & 0xFF;
			*(addr + 3) = (data>>24) & 0xFF;//0x7F;
			
			addr += 4;
		}
	}
	else
		img->pitch = (img->size.w<<2);
	
	img->graphic_buf = (UINT8 *)PNG_MALLOC(cfg,img->pitch * img->size.h);
	if(NULL == img->graphic_buf)
	{
		PNG_ASSERT(PNG_ERR_MALLOC);
		return PNG_FALSE;
	}
	return PNG_TRUE;
}

