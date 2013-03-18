/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *  (C)
 *  File: gif_info.c
 *  (I)
 *  Description: parse the gif file header. deal with the control block
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2008.1.16			Sam		Create
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
#include <api/libmp/gif.h>
#include "gif_main.h"
#include "gif_info.h"
#include "gif_debug.h"

extern UINT8 g_gif_dec_first_pic;
extern UINT8 g_gif_hw_support_rgb;
extern int g_gif_ignore_bk_color;

//CONSTANT STRING for FILE FORMAT IDENTIFICATION
static UINT8 gif_file_signature[] = {"GIF"};
static UINT8 gif_file_version[][4] = {{"87a"},{"89a"}};

GIF_INLINE void rgb2yuv(UINT8 *data,UINT8 frmt)
{
	int blue = *data;
	int green = *(data + 1);
	int red = *(data + 2);
	int y,cb,cr;

	y = (red*Y_COF_R + green*Y_COF_G + blue*Y_COF_B);
	cb = - red*CB_COF_R - green*CB_COF_G + blue*CB_COF_B;
	cr = red*CR_COF_R - green*CR_COF_G - blue*CR_COF_B;
	y = (y>>16) + 16;
	cb = (cb>>16) + 128;
	cr = (cr>>16) + 128;	
	if(y < 16)
		y = 16;
	else if(y > 235)
		y = 235;
	if(cb < 16)
		cb = 16;
	else if(cb > 240)
		cb = 240;
	if(cr < 16)
		cr = 16;
	else if(cr>240)
		cr = 240;

	if(0 == frmt)
		*(UINT32 *)data = (0xFF000000) | (cr<<16) | (y<<8) | cb; 		
	else
		*(UINT32 *)data = (0xFF000000) | (cr<<16) | (cb<<8) | y; 
}

GIF_INLINE void rd_plt_32(UINT8 *plt,struct gif_in *in,INT32 num,UINT8 YUV) 
{	
	INT32 i = 0;

	for(;i < num;i++)
	{		
		*(plt + 2) = (UINT8)gif_in_read_byte(in);
		*(plt + 1) = (UINT8)gif_in_read_byte(in);
		*plt = (UINT8)gif_in_read_byte(in);
		*(plt + 3) = 0xFF;
		plt += 4;
		if(1 == YUV)
			rgb2yuv(plt - 4,YUV);
	}
}

GIF_INLINE void rd_plt_16(UINT8 *plt,struct gif_in *in,INT32 num)
{
	INT32 i = 0;
	UINT16 RGB16 = 0,R = 0, G = 0, B = 0;
	
	for(;i < num;i++)
	{
		R = (UINT16)gif_in_read_byte(in);
		G = (UINT16)gif_in_read_byte(in);
		B = (UINT16)gif_in_read_byte(in);
		RGB16 = 0x8000 | (B>>3) | ((G>>3)<<5) | ((R>>3)<<10);
		*(UINT16 *)plt = RGB16;
		*(UINT16 *)(plt + 2) = 0;
		plt += 4;
	}	
}

GIF_INLINE GIF_RET gif_header(struct gif_context *text)
{
	struct gif_in *in = &text->in;
	struct gif_info_header *header = &(text->core.info)->header;

	header->sig0 = (UINT8)gif_in_read_byte(in);
	header->sig1 = (UINT8)gif_in_read_byte(in);
	header->sig2 = (UINT8)gif_in_read_byte(in);
	header->ver0 = (UINT8)gif_in_read_byte(in);
	header->ver1 = (UINT8)gif_in_read_byte(in);
	header->ver2 = (UINT8)gif_in_read_byte(in);
	if(gif_in_next_byte(in) < 0)
		goto HEADER_FAIL;
	if(header->sig0 != gif_file_signature[0] || header->sig1 != gif_file_signature[1]\
		|| header->sig2 != gif_file_signature[2])
	{
		goto HEADER_FAIL;
	}
	if(header->ver0 != gif_file_version[0][0] || header->ver1 != gif_file_version[0][1]\
		|| header->ver2 != gif_file_version[0][2])
	{
		if(header->ver0 != gif_file_version[1][0] || header->ver1 != gif_file_version[1][1]\
			|| header->ver2 != gif_file_version[1][2])
			goto HEADER_FAIL;
	}
	GIF_PRINTF("\n\n\nFind GIF file header flag\n");
	return GIF_TRUE;
HEADER_FAIL:
	GIF_PRINTF("parse header fail\n");
	return GIF_FALSE;
}

GIF_INLINE GIF_RET gif_lsdes(struct gif_context *text)
{
	struct gif_in *in = &text->in;
	struct gif_lzw *lzw = text->core.lzw;
	struct gif_core_buf *buf = &(text->core.buf);
	struct gif_info_lsdes *lsdes = &((text->core.info)->lsdes);
	INT32 len = 0;
	UINT8 byte = 0;
	INT32 gt_size = 0;
	INT32 i = 0;

	lsdes->w_low = (UINT8)gif_in_read_byte(in);
	lsdes->w_high = (UINT8)gif_in_read_byte(in);
	lsdes->h_low = (UINT8)gif_in_read_byte(in);
	lsdes->h_high = (UINT8)gif_in_read_byte(in);
	byte = (UINT8)gif_in_read_byte(in);
	lsdes->bk_color = (UINT8)gif_in_read_byte(in);
	lsdes->aspec_ratio = (UINT8)gif_in_read_byte(in);
	if(gif_in_next_byte(in) < 0)
		goto LSDES_FAIL;
	lsdes->gt_flag = byte>>7;
	lsdes->sort_flag = (byte>>3)&1;
	lsdes->color_res = (byte>>4)&7;
	if(lsdes->gt_flag)
	{
		lsdes->gt_size = byte&7;
		gt_size = 1<<(lsdes->gt_size + 1);
		lzw->pallete.num = gt_size;
		switch(text->out->frmt)
		{
			case GIF_OUT_32BITS:
				rd_plt_32((UINT8 *)lzw->pallete.global_abs_pal,in,gt_size,0);				
				break;
			case GIF_OUT_8BITS:
				if(GIF_FALSE == g_gif_hw_support_rgb)
				{
					rd_plt_32((UINT8 *)lzw->pallete.global_abs_pal,in,gt_size,1);
					//lzw->pallete.global_abs_pal[lsdes->bk_color] &= 0xFFFFFF;
					lzw->pallete.global_abs_pal[GIF_DEFAULT_TRAN_COLOR] &= 0xFFFFFF;
				}
				else
					rd_plt_32((UINT8 *)lzw->pallete.global_abs_pal,in,gt_size,0);
				break;				
			case GIF_OUT_16BITS:
				rd_plt_16((UINT8 *)lzw->pallete.global_abs_pal,in,gt_size);
				//lzw->pallete.global_abs_pal[lsdes->bk_color] &= 0x7FFF;				
				break;
			default:
				GIF_PRINTF("GIF out frmt fail %d\n",text->out->frmt);
				goto LSDES_FAIL;
		}
		if(gif_in_next_byte(in) < 0)
			goto LSDES_FAIL;
		
		text->out->update = 1;
	}

	
	if((UINT16)((lsdes->w_high<<8) | lsdes->w_low) > GIF_MAX_WIDTH \
		|| (UINT16)((lsdes->h_high<<8) | lsdes->h_low) > GIF_MAX_HEIGHT)
	{
		GIF_PRINTF("gif width %d height %d. Support Max size is %d X %d\n",(UINT16)((lsdes->w_high<<8) | lsdes->w_low),\
			(UINT16)((lsdes->h_high<<8) | lsdes->h_low) > GIF_MAX_HEIGHT,GIF_MAX_WIDTH,GIF_MAX_HEIGHT);
		goto LSDES_FAIL;
	}
	GIF_PRINTF("LSDES width %d height %d\n",(lsdes->w_high<<8) | lsdes->w_low,(lsdes->h_high<<8) | lsdes->h_low);

	len =  (lsdes->w_high<<8) | lsdes->w_low;
	len = (len + 3) & ~3;
	lzw->pixel_pitch = len;
	switch(text->out->frmt)
	{
		case GIF_OUT_32BITS:
			lzw->img_pitch = len<<2; 
			break;			
		case GIF_OUT_16BITS:
			lzw->img_pitch = len<<1; 
			break;			
		case GIF_OUT_8BITS:
			lzw->img_pitch = len; 
			break;
		default:
			GIF_PRINTF("GIF out frmt fail %d\n",text->out->frmt);
			goto LSDES_FAIL;	
	}
	len = lzw->img_pitch * ((lsdes->h_high<<8) | lsdes->h_low);
	if(buf->size < len)
	{
		goto LSDES_FAIL;
	}
	
	if(GIF_FALSE == g_gif_dec_first_pic)
	{
		lzw->bitmap = buf->start;
		lzw->img_size = len;
		//MEMSET((void *)lzw->bitmap,0,len);
		buf->size -= len;
		buf->start += len;
		GIF_PRINTF("Internal bit map size %x addr buf %x %x free %x \n",len,lzw->bitmap,buf->start,buf->size);

		if(g_gif_ignore_bk_color)
		{
			lzw->bk_screen_update = 0;
			lzw->bk_screen_buf = buf->start;
			lzw->bk_screen_size = len;
			
			buf->size -= len;
			buf->start += len;
		}
	}

	lzw->line_buf = buf->start;
	buf->size -= len;
	buf->start += len;	
	//MEMSET((void *)lzw->line_buf,0,len);
	GIF_PRINTF("lzw line buf size %x addr %x buf %x free size %x\n",len,lzw->line_buf,buf->start,buf->size);	

	if(buf->size < 0)
		goto LSDES_FAIL;
	
	return GIF_TRUE;
	
LSDES_FAIL:	
	GIF_PRINTF("parsing lsdes fail\n");
	return GIF_FALSE;
}

GIF_INLINE GIF_RET gif_imgdes(struct gif_context *text)
{
	struct gif_in *in = &text->in;
	struct gif_info_imgdes *imgdes = &(text->core.info)->imgdes;	
	struct gif_lzw *lzw = text->core.lzw;
	UINT8 byte = 0;
	int gt_size = 0,i = 0;
	UINT16 RGB16 = 0,R = 0, G = 0, B = 0;

	imgdes->x_low = (UINT8)gif_in_read_byte(in);
	imgdes->x_high = (UINT8)gif_in_read_byte(in);
	imgdes->y_low = (UINT8)gif_in_read_byte(in);
	imgdes->y_high = (UINT8)gif_in_read_byte(in);
	imgdes->w_low = (UINT8)gif_in_read_byte(in);
	imgdes->w_high = (UINT8)gif_in_read_byte(in);
	imgdes->h_low = (UINT8)gif_in_read_byte(in);
	imgdes->h_high = (UINT8)gif_in_read_byte(in);
	byte = (UINT8)gif_in_read_byte(in);
	if(gif_in_next_byte(in) < 0)
		goto IMGDES_FAIL;

	{
		struct gif_info_lsdes *lsdes = &((text->core.info)->lsdes);		
		int img_x = imgdes->x_high<<8 | imgdes->x_low;
		int img_y = imgdes->y_high<<8 | imgdes->y_low;
		int img_w = imgdes->w_high<<8 | imgdes->w_low;
		int img_h = imgdes->h_high<<8 | imgdes->h_low;
		int des_w = lsdes->w_high<<8 | lsdes->w_low;
		int des_h = lsdes->h_high<<8 | lsdes->h_low;

		if((img_x + img_w > des_w)
			|| (img_y + img_h > des_h))
		{
			GIF_PRINTF("img rect fail x %d y %d w %d h %d des w %d h %d\n"
				, img_x, img_y, img_w, img_h, des_w, des_h);
			goto IMGDES_FAIL;
		}		
	}
	
	imgdes->lt_flag = byte>>7;
	imgdes->sort_flag = (byte>>5) & 1;
	imgdes->int_flag = (byte>>6) & 1;
	GIF_PRINTF("img x %d y %d w %d h %d\n", (imgdes->x_high<<8 | imgdes->x_low)
		, (imgdes->y_high<<8 | imgdes->y_low), (imgdes->w_high<<8 | imgdes->w_low)
		,(imgdes->h_high<<8 | imgdes->h_low));
	GIF_PRINTF("local pallete addr %x\n",(void *)lzw->pallete.local_abs_pal);

	i = GIF_MAX_PALLETE_LEN;
	while(i--)
	{
		lzw->pallete.local_abs_pal[i] = lzw->pallete.global_abs_pal[i];
	}
	
	if(imgdes->lt_flag)
	{
		imgdes->lt_size = byte&7;
		gt_size = 1<<(imgdes->lt_size + 1);
		switch(text->out->frmt)
		{
			case GIF_OUT_32BITS:
				rd_plt_32((UINT8 *)lzw->pallete.local_abs_pal,in,gt_size,0);				
				break;
			case GIF_OUT_8BITS:
				if(GIF_FALSE == g_gif_hw_support_rgb)
				{
					rd_plt_32((UINT8 *)lzw->pallete.local_abs_pal,in,gt_size,1);
					lzw->pallete.local_abs_pal[GIF_DEFAULT_TRAN_COLOR] &= 0xFFFFFF;					
				}
				else
					rd_plt_32((UINT8 *)lzw->pallete.local_abs_pal,in,gt_size,0);
				break;				
			case GIF_OUT_16BITS:
				rd_plt_16((UINT8 *)lzw->pallete.local_abs_pal,in,gt_size);
				//lzw->pallete.global_abs_pal[(text->core.info)->lsdes.bk_color] &= 0x7FFF;
			default:
				GIF_PRINTF("GIF out frmt fail %d\n",text->out->frmt);
				goto IMGDES_FAIL;
		}
		if(gif_in_next_byte(in) < 0)
			goto IMGDES_FAIL;
		text->out->update = 1;
	}

	// modify the item in the pallete which is the same as tran color in GCE
	if((1 == text->core.info->gce.alpha_flag) && (GIF_OUT_8BITS != text->out->frmt))
	{
		UINT32 *pal = (UINT32 *)lzw->pallete.local_abs_pal;
		UINT32 tran_true_color = 0;
		int i = 0;

		if(0 == gt_size)
			gt_size = lzw->pallete.num;
		tran_true_color = (UINT32)*(pal + text->core.info->gce.alpha_value);	
		for(i = 0;i < gt_size;i++,pal++)
		{
			if(tran_true_color == *pal)
			{
				if(i != text->core.info->gce.alpha_value)
				{
					if(0xFFFFFFFF == *pal)
						*pal -= 1;
					else
						*pal += 1;
				}
				
				if(g_gif_ignore_bk_color)
					*pal &= 0x00FFFFFF;
			}
		}
	}

	if(g_gif_ignore_bk_color && (GIF_OUT_8BITS != text->out->frmt))
	{
		lzw->pallete.local_abs_pal[text->core.info->lsdes.bk_color] &= 0x00FFFFFF;
	}

	GIF_PRINTF("Interlace %d\n",imgdes->int_flag);
	return GIF_TRUE;
IMGDES_FAIL:
	GIF_PRINTF("parse imgdes fail\n");
	return GIF_FALSE;
}

GIF_INLINE GIF_RET gif_gce(struct gif_context *text)
{
	struct gif_in *in = &text->in;
	struct gif_info_gce *gce = &(text->core.info)->gce;
	UINT8 byte = 0;

	gce->block_size = (UINT8)gif_in_read_byte(in);
	byte = (UINT8)gif_in_read_byte(in);
	gce->dy_time_low = (UINT8)gif_in_read_byte(in);
	gce->dy_time_high = (UINT8)gif_in_read_byte(in);
	gce->alpha_value = (UINT8)gif_in_read_byte(in);
	gce->block_tm = (UINT8)gif_in_read_byte(in);
	if(gif_in_next_byte(in) < 0)
		goto GCE_FAIL;
	gce->alpha_flag = byte & 1;
	gce->user_in = (byte>>1) & 1;
	gce->disposal_m = (byte>>2) & 7;
	if(GIF_BLOCK_TER_FLAG != gce->block_tm)
		goto GCE_FAIL;
	GIF_PRINTF("Disposal method %d alpha flag %d value %d\n",gce->disposal_m
		, gce->alpha_flag, gce->alpha_value);
	return GIF_TRUE;
	
GCE_FAIL:
	GIF_PRINTF("parse gce fail\n");
	return GIF_FALSE;
}

GIF_INLINE GIF_RET gif_find_imgdes(struct gif_context *text)
{
	struct gif_in *in = &text->in;
	struct gif_info *info = text->core.info;
	struct gif_info_imgdes *imgdes = &info->imgdes;
	struct gif_info_gce *gce = &info->gce;
	UINT8 indicator = 0;
	UINT8 ext_lable = 0;
	UINT8 byte = 0;
	int ext_len = 0;
	int i = 0;
	UINT16 iteration_num = 0;

IMGDES_BEGIN:
	indicator = (UINT8)gif_in_read_byte(in);
	GIF_PRINTF("%s : indicator %x\n", __FUNCTION__, indicator);
	switch(indicator)
	{
		case GIF_FILE_TRAILER:
			goto FIND_IMGDES_FAIL;
		case GIF_IMAGE_SEPARATOR:
			if(gif_imgdes(text))
				break;	
			else
				goto FIND_IMGDES_FAIL;
		case GIF_EXT_INTRODUCER:
			ext_lable = (UINT8)gif_in_read_byte(in);
			if(0xFF == ext_lable)
			{
				ext_len = gif_in_read_byte(in);//11
				for(i = 0;i < ext_len; i++)//NETSCAPE2.0
					gif_in_read_byte(in);
				gif_in_read_byte(in);//3
				gif_in_read_byte(in);//1
				byte = (UINT8)gif_in_read_byte(in);
				iteration_num = ((UINT8)gif_in_read_byte(in)<<8) | byte;
				GIF_PRINTF("[gif_find_imgdes]: iteration_num = %d.\n", iteration_num);
				info->imgdes.iteration_num = iteration_num;
				if(iteration_num == 0)
					info->imgdes.iteration_num = 0xffff;
				gif_in_read_byte(in);//0
			}
			else if(GIF_GRAPHIC_CTRL == ext_lable)
			{
#if 0			
				if(gif_gce(text))
					goto IMGDES_BEGIN;
				else
					goto FIND_IMGDES_FAIL;
#else
				if(gif_gce(text))
					text->core.status.flag |= GIF_INFO_GCE;
				goto IMGDES_BEGIN;
#endif
			}
			else
			{
				do
				{
					ext_len = gif_in_read_byte(in);
					if(ext_len <= 0)
						break;
					for(i = 0;i < ext_len; i++)
						gif_in_read_byte(in);					
				}while(1);
			}
			goto IMGDES_BEGIN;
		default:
#if 0		
			if(1 == in->file.finish)
				break;
#endif			
			while(1)
			{
				if(GIF_BLOCK_TER_FLAG == indicator)
					goto IMGDES_BEGIN;
				else
				{
					if(gif_in_next_byte(in) < 0)	
					{
						goto FIND_IMGDES_FAIL;
					}
					indicator = (UINT8)gif_in_read_byte(in);
				}
			}		
	}
	
	GIF_PRINTF("Find the images successfully w %d h %d\n", (imgdes->w_high<<8) | imgdes->w_low
		, (imgdes->h_high<<8) | imgdes->h_low);
	
	return GIF_TRUE;
	
FIND_IMGDES_FAIL:		
	GIF_PRINTF("Find the image fail\n");
	text->core.status.flag |= GIF_END_FILE_STR;
	return GIF_FALSE;
}

GIF_RET gif_info_parse(struct gif_context *text)
{
	struct gif_info *info = text->core.info;
	struct gif_core *core = &text->core;
	struct gif_status *status = &core->status;

INFO_BEGIN:
	if(!(status->flag & GIF_INFO_HEADER))
	{
		MEMSET((void *)info,0,sizeof(struct gif_info));
		if(gif_header(text))
		{
			status->flag |= GIF_INFO_HEADER;
			if(gif_lsdes(text))
			{
				status->flag |= GIF_INFO_LSDES;
				if(0 == info->offset.first_img)
				{
					info->offset.first_img = gif_in_file_offset(&text->in);// - 1;
					GIF_PRINTF("first img offset %x\n",info->offset.first_img);
				}
				goto INFO_BEGIN;
			}
		}	
		gif_core_set_error(text,GIF_ERR_CORE_LOST_HEADER_INFO);
		return GIF_FALSE;
	}
	else
	{
		if(gif_find_imgdes(text))
			status->flag |= GIF_INFO_IMGDES;
	}	
	return (status->flag&GIF_END_FILE_STR)?GIF_FALSE:GIF_TRUE;
}

GIF_RET gif_info_init(struct gif_context *text)
{
	struct gif_core_buf *buf = &(text->core.buf);
	INT32 len = (sizeof(struct gif_info) + 3) & ~3;
	
	if(buf->valid)
	{
		if((buf->size -= len) >= 0)
		{
			text->core.info = (struct gif_info *)buf->start;
			buf->start += len;
			MEMSET((void *)text->core.info,0,len);
			GIF_PRINTF("info len %x addr %x free size %x \n",len,text->core.info,buf->size);
			return GIF_TRUE;
		}
	}
	GIF_ASSERT(GIF_ERR_INFO_INIT);
	return GIF_FALSE;
}

