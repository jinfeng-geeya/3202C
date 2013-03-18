/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *  (C)
 *  File: gif_main.c
 *  (I)
 *  Description:  The core file of the gif decoder. Implement the decoder routine
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
#include "gif_list.h"
#include "gif_info.h"
#include "gif_in.h"
#include "gif_out.h"
#include "gif_lzw.h"

extern UINT8 g_gif_dec_first_pic;
extern GIF_RET (*g_gif_out_init)(struct gif_context *,struct gif_dec_out_par *);
extern GIF_RET (*g_gif_out_enter)(struct gif_out *,struct gif_lzw *);
extern void (*g_gif_out_img)(struct gif_context *);

static int get_original_line(int line, int total)
{
	int f1, f2, f3, f4;
	int idx;

	f1 = (total>>3);
	if((f1<<3) < total)
		f1++;

	if(line < f1)
	{
		idx = (line<<3);
		goto EXIT;
	}

	f2 = ((total - 4)>>3);
	if((f2<<3) < (total - 4))
		f2++;

	if(line < (f1 + f2))
	{
		idx = 4 + ((line - f1)<<3);
		goto EXIT;
	}

	f3 = ((total - 2)>>2);
	if((f3<<2) < (total - 2))
		f3++;

	if(line < (f1 + f2 + f3))
	{
		idx = 2 + ((line - f1 - f2)<<2);
		goto EXIT;
	}

	f4 = ((total - 1)>>1);
	if((f4<<1) < (total - 1))
		f4++;

	if(line < (f1 + f2 + f3 + f4))
	{
		idx = 1 + ((line - f1 - f2 - f3)<<1);
		goto EXIT;
	}

	return -1;
	
EXIT:
	return idx;
}

void interlace2normal_image(struct gif_context *text)
{
	struct gif_core_buf *buf = &(text->core.buf);
	struct gif_lzw *lzw = text->core.lzw;
	struct gif_out *out = text->out;
	int h = lzw->sum_lines;
	char *out_buf = NULL;
	char *in_buf = NULL;	
	int pitch = 0;	
	int i = 0, j = 0;

	switch(text->out->frmt)
	{
		case GIF_OUT_32BITS:
			pitch = lzw->rect.w<<2;			
			break;
		case GIF_OUT_16BITS:
			pitch = lzw->rect.w<<1;
			break;
		case GIF_OUT_8BITS:
			pitch = lzw->rect.w;
			break;
		default:
			pitch = lzw->rect.w<<2;
			break;
	}
	
	if(pitch * h <= buf->size)
	{
		UINT32 buf_start = buf->start;

		MEMCPY((void *)buf_start, (void *)(lzw->line_buf + pitch), pitch * (h - 1));
		in_buf = (void *)buf_start;
		for(i = 1;i < h;i++)
		{
			j = get_original_line(i, h);
			if(j < 0)
			{
				GIF_PRINTF("original line for interlace image fail\n");
				return;
			}
	
			out_buf = (void *)(lzw->line_buf + j * pitch);
			MEMCPY(out_buf , in_buf, pitch);
			in_buf += pitch;
		}
	}
	else
	{
		GIF_PRINTF("no engouh buf for interlace convert operation\n");
	}
}

GIF_INLINE GIF_RET check_info_integrity(struct gif_core *core)
{
	return (GIF_INFO_MUST == (core->status.flag&GIF_INFO_MUST)) \
		?GIF_TRUE:GIF_FALSE;
}

GIF_INLINE GIF_RET check_show_delay(struct gif_core *core)
{
	struct gif_lzw *lzw = core->lzw;
	UINT32 ticks = osal_get_tick() - lzw->tick_start;
	UINT32 delay = lzw->delay_num;

	if(delay <= GIF_DELAY_THRESHOLD)
		goto SHOW_OK;
	if(ticks < delay - GIF_DELAY_THRESHOLD)
	{
		lzw->delay_num -= ticks;
		lzw->tick_start += ticks;
		return GIF_FALSE;
	}
SHOW_OK:
	lzw->delay_num = 0;
	return GIF_TRUE;
}

GIF_INLINE void core_enter_delay(struct gif_context *text)
{
	struct gif_status *status = &text->core.status;

	status->step = GIF_SHOW_DELAY;
}

GIF_INLINE void core_next_img(struct gif_context *text)
{
	struct gif_in *in = &text->in;
	struct gif_status *status = &text->core.status;

	status->flag &= ~(GIF_INFO_IMGDES | GIF_END_IMG_DECODE);
	status->step = GIF_PARSE_INFO;
	gif_in_init_bit(in);
}

GIF_INLINE void core_reshow_file(struct gif_context *text)
{
	struct gif_core *core = &text->core;
	struct gif_in *in = &text->in;
	struct gif_status *status = &core->status;

	core->lzw->tick_start = 0;
	core->lzw->img_id = 0;
	status->flag &= ~(GIF_INFO_IMGDES | GIF_END_FILE_STR \
		| GIF_END_IMG_DECODE | GIF_INFO_GCE_PRIOR);
	status->step = GIF_PARSE_INFO;

	GIF_PRINTF("%s : line %d\n", __FUNCTION__, __LINE__);
		
#if 0
	MEMSET((void *)core->lzw->bitmap,0,core->lzw->img_size);
	osal_cache_flush((void *)core->lzw->bitmap,core->lzw->img_pitch);
#endif	
	gif_in_init_bit(in);
	gif_in_file_set_offset(in,core->info->offset.first_img);
}

void gif_core_set_error(struct gif_context *text ,INT32 num)
{
	GIF_PRINTF("core set error num %d\n",num);
	text->core.status.error = num;
}

UINT32 gif_core_error(struct gif_context *text)
{
	return text->core.status.error;
}

void gif_core_dec(struct gif_main *main)
{
	struct gif_context *text = main->cur;
	struct gif_core *core = &text->core;
	struct gif_in *in = &text->in;
	struct gif_status *status = &core->status;
	UINT8 img_done_flag = 0;

CORE_BEGIN:
	switch(text->core.status.step)
	{
		case GIF_PARSE_INFO:
			if(gif_info_parse(text))
			{
				if(check_info_integrity(core))
				{
					if(gif_lzw_make_ready(text))
					{
						status->step = GIF_LZW_IMAGE;		
						goto CORE_BEGIN;
					}
					else
						break;
				}
				gif_core_set_error(text,GIF_ERR_CORE_LOST_HEADER_INFO);
			}
			break;
		case GIF_LZW_IMAGE:
			// decode one sub data block and free the resource for another file if it exists
			gif_lzw_decompress(text);
			break;
		case GIF_SHOW_DELAY:
			if(check_show_delay(core))
				break;
			else
				goto CORE_EXIT;
		default:
			GIF_ASSERT(GIF_ERR_CORE_STATUS);
	}

	//check error
	if(gif_core_error(text))
			return;

	//check flag and image finish
	if(status->flag & GIF_END_FILE_STR)
	{
		if(check_show_delay(core))
		{
			core_reshow_file(text);
			
			core->lzw->iteration_idx++;
			GIF_PRINTF("[gif_core_dec]: iteration_idx=%d.\n", core->lzw->iteration_idx);
			if(in->frame_num > 1)
			{
				if((core->lzw->iteration_idx >= core->info->imgdes.iteration_num) && (core->info->imgdes.iteration_num != 0xffffffff))
				{
					gif_core_set_error(text,GIF_ERR_OTHERS);
					return;
				}
			}
		}
		else
		{
			core_enter_delay(text);
			goto CORE_EXIT;
		}
		if(core->lzw->sum_lines > 0)
			img_done_flag = 1;
	}
	else if(gif_lzw_image_done(text) || (status->flag & GIF_END_IMG_DECODE))
	{
		GIF_PRINTF("\n\n\nCurrent image %d\n",core->lzw->img_id);

		if(check_show_delay(core))
		{
			core_next_img(text);
			img_done_flag = 1;
		}
		else
		{
			core_enter_delay(text);
			goto CORE_EXIT;
		}
	}
	
	if(img_done_flag)
	{
		if(core->lzw->img_id == core->lzw->last_img)
		{
			gif_core_set_error(text,GIF_ERR_OTHERS);
		}
		else
		{
			img_done_flag = 0;
			if(core->info->imgdes.int_flag == 1)
			{
				GIF_PRINTF("convert interlace image to normal\n");
				interlace2normal_image(text);
			}
			
			g_gif_out_img(text);
			core->lzw->last_img = core->lzw->img_id;

			if(core->lzw->img_id == 1)//first frame finished
			{
				if(in->frame_num > 1)
				{
					status->first_frame_done = 1;
					GIF_PRINTF("[gif_core_dec]: first frame has done.\n");
				}
			}

			main->info.first_pic = 1;
			// only show the first pic of the gif file
			if(GIF_TRUE == g_gif_dec_first_pic)
			{
				gif_core_set_error(text,GIF_ERR_OTHERS);					
				return;
			}
		}
	}
	
CORE_EXIT:	
	
	if(GIF_TRUE == g_gif_dec_first_pic)
		goto CORE_BEGIN;
	
	return;
}

