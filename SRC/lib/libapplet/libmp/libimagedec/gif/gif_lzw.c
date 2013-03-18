/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *  (C)
 *  File: gif_lzw.c
 *  (I)
 *  Description: implement lzw algorithm of gif decoder
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
#include "gif_in.h"
#include "gif_out.h"
#include "gif_lzw.h"
#include "gif_debug.h"

extern int g_gif_force_show_pic;
extern GIF_RET (*g_gif_out_init)(struct gif_context *,struct gif_dec_out_par *);
extern GIF_RET (*g_gif_out_enter)(struct gif_out *,struct gif_lzw *);
extern void (*g_gif_out_img)(struct gif_context *);

static UINT16 lzw_clear_code = 0;
static UINT16 lzw_end_code = 0;

static UINT32 *lzw_table = NULL;
static UINT32 lzw_table_idx = 0;
static UINT32 lzw_code_size = 0;
static UINT32 *lzw_idx_pallete = NULL;
static volatile UINT32 lzw_line_read = 0;
static UINT16 (*write_code)(UINT16,UINT32);

static UINT32 *lzw_stack_start = NULL;
static UINT32 *lzw_stack_ptr = NULL;

GIF_INLINE UINT16 write_code_32(UINT16 code,UINT32 line);
GIF_INLINE UINT16 write_code_16(UINT16 code,UINT32 line);
GIF_INLINE UINT16 write_code_8(UINT16 code,UINT32 line);

GIF_INLINE void lzw_enter(struct gif_context *text)
{
	struct gif_lzw *lzw = text->core.lzw;
	
	lzw_code_size = lzw->code_size;
	lzw_clear_code = lzw->clear_code;
	lzw_end_code = lzw->end_code;
	lzw_table_idx = lzw->table_idx;
	lzw_idx_pallete = lzw->pallete.local_abs_pal;
	lzw_table = lzw->table_start;
	lzw_line_read = lzw->line_read;
	lzw_stack_start = lzw->stack_start;
	lzw_stack_ptr = lzw->stack_ptr;
	
	switch(text->out->frmt)
	{
		case GIF_OUT_32BITS:
			write_code = write_code_32;
			break;			
		case GIF_OUT_16BITS:
			write_code = write_code_16;
			break;			
		case GIF_OUT_8BITS:
			write_code = write_code_8;
			break;
		default:
			GIF_PRINTF("GIF out frmt fail %d\n",text->out->frmt);
	}		
}

GIF_INLINE void lzw_exit(struct gif_lzw *lzw)
{
	lzw->code_size = lzw_code_size;
	lzw->clear_code= lzw_clear_code;
	lzw->end_code = lzw_end_code;
	lzw->table_idx = lzw_table_idx;
	lzw->line_read = lzw_line_read;
	lzw->stack_ptr = lzw_stack_ptr;	
}

GIF_INLINE void lzw_table_init(void)
{
	int cnt = 1<<(lzw_code_size - 1);
	int i = 0;

	MEMSET((void *)lzw_table,0,GIF_TABLE_LEN<<2);
	MEMSET((void *)lzw_stack_start,0,GIF_TABLE_LEN<<2);
	for(i = 0;i < cnt;i++)
	{
		*(lzw_table + i) = i + 1;
	}
	*(lzw_table + i ) = 1;
}

GIF_INLINE UINT16 first_char(UINT16 code)
{
	UINT16 data = 0;
	
	while(1)
	{
		if(0 == (*(lzw_table + code)&0xffff0000))
		{
			data =  (UINT16)*(lzw_table + code);
			break;
		}
		else
			code = (*(lzw_table + code))>>16;
	}
	return data;
}

GIF_INLINE GIF_RET is_code_exist(UINT16 code)
{
	if(0 == (*(lzw_table + code)&0xffff))
	{
		return GIF_FALSE;
	}
	return GIF_TRUE;
}

GIF_INLINE UINT16 write_code_32(UINT16 code,UINT32 line)
{
	UINT16 num = 1;
#if 1
	while(1)
	{
		*lzw_stack_ptr++ = (UINT32)(*(lzw_idx_pallete + ((UINT8)(*(lzw_table + code)) - 1)));
		if(0 == (*(lzw_table + code)&0xffff0000))
			break;

		num++;
		code = (UINT16)((*(lzw_table + code))>>16);
	}

	while(lzw_stack_ptr != lzw_stack_start)
	{
		*(UINT32 *)lzw_line_read = (UINT32)(*(--lzw_stack_ptr));
		lzw_line_read += 4;
	}
#else
	if(0 == (*(lzw_table + code)&0xffff0000))
	{
		*(UINT32 *)line = *(lzw_idx_pallete + ((UINT8)(*(lzw_table + code)) - 1));
		lzw_line_read += 4;
		return 1;
	}
	else
	{
		num += write_code_32((UINT16)((*(lzw_table + code))>>16),line);
		*(((UINT32 *)line) + num - 1) = *(lzw_idx_pallete + ((UINT8)(*(lzw_table + code)) - 1));
		lzw_line_read += 4;
	}
#endif
	
	return num;
}

GIF_INLINE UINT16 write_code_16(UINT16 code,UINT32 line)
{
	UINT16 num = 1;

#if 1
	while(1)
	{
		*lzw_stack_ptr++ = (UINT32)((*(lzw_idx_pallete + ((UINT8)(*(lzw_table + code)) & 0xFFFF) - 1)));
		if(0 == (*(lzw_table + code)&0xffff0000))
			break;

		num++;
		code = (UINT16)((*(lzw_table + code))>>16);
	}

	while(lzw_stack_ptr != lzw_stack_start)
	{
		*(UINT16 *)lzw_line_read = (UINT16)((*(--lzw_stack_ptr)) & 0xFFFF);
		lzw_line_read += 2;
	}
#else
	if(0 == (*(lzw_table + code)&0xffff0000))
	{
		*(UINT16 *)line = (UINT16)*(lzw_idx_pallete + ((UINT8)(*(lzw_table + code)) - 1));
		lzw_line_read += 2;
		return 1;
	}
	else
	{
		num += write_code_16((UINT16)((*(lzw_table + code))>>16),line);
		*(((UINT16 *)line) + num - 1) = (UINT16)*(lzw_idx_pallete + ((UINT8)(*(lzw_table + code)) - 1));
		lzw_line_read += 2;
	}
#endif
	
	return num;
}

GIF_INLINE UINT16 write_code_8(UINT16 code,UINT32 line)
{
	UINT16 num = 1;

#if 1
	while(1)
	{
		*lzw_stack_ptr++ = (UINT32)((*(lzw_table + code) & 0xFF) - 1);
		if(0 == (*(lzw_table + code)&0xffff0000))
			break;

		num++;
		code = (UINT16)((*(lzw_table + code))>>16);
	}

	while(lzw_stack_ptr != lzw_stack_start)
	{
		*(UINT8 *)lzw_line_read = (UINT8)((*(--lzw_stack_ptr)) & 0xFF);
		lzw_line_read++;
	}
#else
	if(0 == (*(lzw_table + code)&0xffff0000))
	{
		*(UINT8 *)line = (UINT8)(*(lzw_table + code)) - 1;
		lzw_line_read++;
		return 1;
	}
	else
	{
		num += write_code_8((UINT16)((*(lzw_table + code))>>16),line);
		*(((UINT8 *)line) + num - 1)= (UINT8)(*(lzw_table + code)) - 1;
		lzw_line_read++;
	}
#endif

	return num;
}

#if 0
GIF_INLINE void SWAP32(UINT32 *p,INT32 i,INT32 j)
{
	UINT32 tmp = 0;
	
	tmp = *(p + i);
	*(p + i) = *(p + j);
	*(p + j) = tmp;
}

GIF_INLINE void SWAP8(UINT8 *p,INT32 i,INT32 j)
{
	UINT8 tmp = 0;
	
	tmp = *(p + i);
	*(p + i) = *(p + j);
	*(p + j) = tmp;
}
//pallete structure: high --> low , Y --> Cb --> Cr. Every componet is with 8 bits length
//sort the pallete by the y value in the ascending order
static void gif_sort_pallete(UINT32 *pallete,INT32 num,UINT8 *sort_pal)
{
	//quick sort algorithm
	INT32 idx1 = 1, idx2 = num - 1,idx = 0;
	UINT32 flag = 0;

	if(num <= 1)
		return;

	//choose the middle index as the flag of quick sorting
	idx = (num - 1)>>1;
	flag = 0xFFFFFF & *(pallete + idx);
	if(0 != idx)
	{
		SWAP32(pallete,0,idx);
		SWAP8(sort_pal,0,idx);
	}
	do
	{
		for(;idx2 > 0;idx2--)
		{
			if((0xFFFFFF & *(pallete + idx2)) < flag)
			{
				//soc_printf("idx2 %d value %d flag %d\n",idx2,*(y_value + (idx2<<2)),flag);
				break;
			}
		}
		for(;idx1 < num - 1;idx1++)
		{
			if((0xFFFFFF & *(pallete + idx1)) > flag)
			{
				//soc_printf("idx1 %d value %d flag %d\n",idx1,*(y_value + (idx1<<2)),flag);
				break;			
			}
		}
		if(idx1 >= idx2)
		{
			//soc_printf("finish sort i %d j %d\n",idx1,idx2);
			break;
		}
		else
		{
			//soc_printf("swip i %d j %d\n",idx1,idx2);
			SWAP32(pallete,idx1,idx2);
			SWAP8(sort_pal,idx1,idx2);
			idx1++;idx2--;
		}
	}while(1);
	if(idx2 > 0)
	{
		SWAP32(pallete,0,idx2);
		SWAP8(sort_pal,0,idx2);
	}
	idx = idx2;
	//the left array
	gif_sort_pallete(pallete,idx,sort_pal);
	//the right array
	gif_sort_pallete(pallete + idx + 1,num - idx - 1,sort_pal + idx + 1);
}


#define Y_RANGE_VALUE						10

#define GIF_ABS(u1,u2)	(((u1) > (u2)) ? (u1) - (u2):(u2)-(u1))

GIF_INLINE void reconstruct_idx_pallete(struct gif_out *out,struct gif_pallete *pal)
{
	UINT8 *s_y = ((UINT8 *)out->surface_pallete) + 2;
	UINT8 *l_y = ((UINT8 *)pal->local_abs_pal) + 2;
	UINT8 *s_u = ((UINT8 *)out->surface_pallete) + 1;
	UINT8 *l_u = ((UINT8 *)pal->local_abs_pal) + 1;
	UINT8 *s_v = (UINT8 *)out->surface_pallete;
	UINT8 *l_v = (UINT8 *)pal->local_abs_pal;
	UINT8 *s_sort = out->sort_pal;
	UINT8 *l_sort = pal->sort_pal;
	UINT8 *i_pal = pal->idx_pal;
	UINT32 idx1 = 0,idx2 = 0;
	UINT32 abs_dif1 = 0,abs_dif2 = 0;
	UINT8 cnt = 0;
	UINT16 min_dif = 0xFFF;
	int i = 0,num = pal->num;
	
	for(i = 0;i < num;i++)
	{
		do
		{
			if(*l_y <= *s_y)	
				break;
			if(idx2 >= GIF_MAX_PALLETE_LEN - 1)
				break;
			idx2++;
			s_y += 4;
		}while(1);
#if 0
		idx1 = idx2;
#else
		idx1 = idx2;
		cnt = 0;
		//idx < idx2
		while(*l_y - Y_RANGE_VALUE < *(s_y - (cnt<<2)))
		{
			abs_dif1 = GIF_ABS(*l_u,*(s_u - (cnt<<2)));
			abs_dif2 = GIF_ABS(*l_v,*(s_v - (cnt<<2)));
			if(abs_dif2 > abs_dif1)
				abs_dif1 = abs_dif2;
			if(abs_dif1 < min_dif)
			{
				min_dif = abs_dif1;
				idx1 = idx2 - cnt;
			}
			if(cnt >= idx2)
				break;
			cnt++;
		}
		//idx > idx2
		cnt = 0;
		while(*l_y + Y_RANGE_VALUE > *(s_y + (cnt<<2)))
		{
			abs_dif1 = GIF_ABS(*l_u,*(s_u + (cnt<<2)));
			abs_dif2 = GIF_ABS(*l_v,*(s_v + (cnt<<2)));
			if(abs_dif2 > abs_dif1)
				abs_dif1 = abs_dif2;
			if(abs_dif1 < min_dif)
			{
				min_dif = abs_dif1;
				idx1 = idx2 + cnt;
			}
			if(cnt >= GIF_MAX_PALLETE_LEN - idx2)
				break;
			cnt++;
		}		
#endif
		*(i_pal + *(l_sort + i))= *(s_sort + idx1);
		//GIF_PRINTF("idx %d img color %x osd color %x\n",*(l_sort + i),pal->local_abs_pal[i],out->surface_pallete[idx1]);
		l_y += 4;
	}
}

GIF_INLINE GIF_RET gif_lzw_buildup_pallete(struct gif_context *text)
{
	struct gif_pallete *pal = &((text->core.lzw)->pallete);
	struct gif_info *info = text->core.info;
	struct gif_out *out = text->out;
	UINT8 *idx_pal = pal->idx_pal;
	int i = 0;

	if(info->imgdes.lt_flag){
		pal->num = 1<<(info->imgdes.lt_size + 1);
	}else if(info->lsdes.gt_flag){
		pal->num = 1<<(info->lsdes.gt_size + 1);		
	}else
		goto BUILD_PAL_FAIL;
	
	GIF_PRINTF("gif pallete size %d\n",pal->num);
	//init the idx pallete
	for(i = 0;i < pal->num;i++)
	{
		*idx_pal++ = (UINT8)i;
	}
	if(out->valid)
	{
		if(!out->sort)
		{
			for(i = 0;i < GIF_MAX_PALLETE_LEN;i++)
			{
				*(out->sort_pal + i)= (UINT8)i;
			}
			
			gif_sort_pallete(out->surface_pallete,GIF_MAX_PALLETE_LEN,out->sort_pal);
			out->sort = 1;
		}
	}
	else
		goto BUILD_PAL_FAIL;
	
	for(i = 0;i < pal->num;i++)
	{
		*(pal->sort_pal + i) = (UINT8)i;
	}	
	
	gif_sort_pallete(pal->local_abs_pal,pal->num,pal->sort_pal);
	reconstruct_idx_pallete(out,pal);
	return GIF_TRUE;
	
BUILD_PAL_FAIL:
	gif_core_set_error(text,GIF_ERR_CORE_LOST_PALLETE);
	return GIF_FALSE;
}
#endif

GIF_INLINE void lzw_first_stream(struct gif_lzw *lzw)
{
	lzw_code_size = lzw->min_code_size;
	lzw_clear_code = 1<<(lzw_code_size- 1);
	lzw_end_code = lzw_clear_code + 1;
	lzw_table_idx = lzw_clear_code + 2;
	lzw_idx_pallete = lzw->pallete.local_abs_pal;
	lzw_table = lzw->table_start;
}

GIF_RET gif_lzw_make_ready(struct gif_context *text)
{
	struct gif_lzw *lzw = text->core.lzw;
	struct gif_info *info = text->core.info;
	INT16 byte = 0;

	lzw->flag &= ~GIF_LZW_FIRST_STREAM;

	byte = (INT16)gif_in_read_byte(&text->in);
	if(byte < 0)	
		return GIF_FALSE;
	
	lzw->min_code_size = 1 + (UINT8)byte;
	lzw->line_read = lzw->line_buf;
	lzw->rect.w = (info->imgdes.w_high<<8) | info->imgdes.w_low;
	lzw->rect.h = (info->imgdes.h_high<<8) | info->imgdes.h_low;
	lzw->rect.x = (info->imgdes.x_high<<8) | info->imgdes.x_low;
	lzw->rect.y = (info->imgdes.y_high<<8) | info->imgdes.y_low;			
	lzw->sum_lines = 0;
	lzw->pixel_num = 0;
	lzw->flag = 0;
	lzw->img_id++;
	
	lzw->stack_ptr = lzw->stack_start;
	
	if(text->out->update)
	{
		if(GIF_OUT_8BITS == text->out->frmt)
			text->out->bk_clor = info->lsdes.bk_color;
		else
			text->out->bk_clor = lzw->pallete.local_abs_pal[info->lsdes.bk_color];
	}
	return g_gif_out_enter(text->out,lzw);
}

GIF_RET gif_lzw_image_done(struct gif_context *text)
{
	struct gif_lzw *lzw = text->core.lzw;
	
	if(lzw->rect.h <= lzw->sum_lines)
	{
		return GIF_TRUE;
	}
	return GIF_FALSE;
}

UINT32 gif_lzw_delay_slice(struct gif_context *text)
{
	return text->core.lzw->delay_num;
}

UINT32 gif_lzw_cur_ticks(struct gif_context *text)
{
	return (UINT32)(osal_get_tick() - (text->core.lzw->tick_start));
}

void gif_lzw_decompress(struct gif_context *text)
{
	struct gif_lzw *lzw = text->core.lzw;
	struct gif_in *in = &(text->in);
	INT32 width = lzw->rect.w;
	UINT16 code = 0,prefix = lzw->prefix, suffix = lzw->suffix,num = 0;

LZW_ENTER:	
	lzw_enter(text);
	do
	{
		if(!(lzw->flag & GIF_LZW_FIRST_STREAM))
		{
			//clear code operation and init list
			lzw->flag |= GIF_LZW_FIRST_STREAM;
			lzw_first_stream(lzw);
			lzw_table_init();
			//read the first code and ready for the next lzw decoding
			prefix = code = gif_in_read_bits(in,lzw_code_size);
			if(code == lzw_clear_code)
			{
				lzw->flag &= ~GIF_LZW_FIRST_STREAM;
				continue;	
			}
			else if(code < lzw_clear_code)
			{
				num = write_code(prefix, lzw_line_read);//lzw->line_read);
				if(1 == num)
				{
					//lzw->line_read += num;
					lzw->pixel_num += num;
					continue;
				}
			}
			gif_core_set_error(text,GIF_ERR_CORE_FIRST_CODE);
				return;
		}
		//read the code
		suffix = code = gif_in_read_bits(in,lzw_code_size);
		if((INT16)code < 0)
		{
			GIF_PRINTF("file end or error\n");
			if(g_gif_force_show_pic == 1)
				text->core.status.flag |= GIF_END_IMG_DECODE;
			else
				text->core.status.flag |= GIF_END_FILE_STR;
			return;
		}else if(code == lzw_clear_code){
			lzw->flag &= ~GIF_LZW_FIRST_STREAM;
			GIF_PRINTF("clear code code size %d table idx %d \n",lzw_code_size,lzw_table_idx);
			continue;
		}else if(code == lzw_end_code){
			text->core.status.flag |= GIF_END_IMG_DECODE;
			GIF_PRINTF("lzw image finish\n");
			return;
		}
		prefix = (0 == prefix)?lzw_clear_code:prefix;
		if(is_code_exist(code))
		{
			*(lzw_table+ lzw_table_idx) = prefix<<16;
			*(lzw_table + lzw_table_idx) |= first_char(suffix);		
		}
		else
		{
			*(lzw_table+ lzw_table_idx) = prefix<<16;
			*(lzw_table + lzw_table_idx) |= first_char(prefix);
		}
		//write the prefix into the line buffer
		num = write_code(code,lzw_line_read);//lzw->line_read);
		//lzw->line_read += num;
		lzw->pixel_num += num;
		lzw_table_idx++;
		if((lzw_table_idx == (UINT32)(1<<lzw_code_size)) && (GIF_MAX_BIT_LEN != lzw_code_size))
		{
			lzw_code_size++;
			GIF_PRINTF("increase code size %d table idx %d\n",lzw_code_size,lzw_table_idx);
		}
		prefix = code;
LZW_EXIT:		
		//exit this decompress process when finish one line
		if(lzw->pixel_num >= width)
		{
			lzw->sum_lines++;
			//libc_printf("Lzw Line %d\n",lzw->sum_lines);
			lzw->pixel_num -= width;
			lzw->prefix = prefix;
			lzw_exit(lzw);
			return;	
		}
	}while(1);
}

GIF_RET gif_lzw_init(struct gif_context * text)
{
	struct gif_core_buf *buf = &(text->core.buf);
	INT32 len = (sizeof(struct gif_lzw) + 3) & ~3;

	if(buf->valid)
	{
		if((buf->size -= len) >= 0)
		{
			text->core.lzw = (struct gif_lzw *)buf->start;
			buf->start += len;
			MEMSET((void *)text->core.lzw,0,len);
			GIF_PRINTF("lzw len %x addr %x free size %x \n",len,text->core.lzw,buf->size);
		}
		else
			goto LZW_INIT_FAIL;
#if 0
		switch(text->out->frmt)
		{
			case GIF_OUT_32BITS:
				len = GIF_MAX_HEIGHT * GIF_MAX_WIDTH * 4;
				break;			
			case GIF_OUT_16BITS:
				len = GIF_MAX_HEIGHT * GIF_MAX_WIDTH * 2;
				break;			
			case GIF_OUT_8BITS:
				len = GIF_MAX_HEIGHT * GIF_MAX_WIDTH;
				break;
			default:
				GIF_PRINTF("GIF out frmt fail %d\n",text->out->frmt);
				goto LZW_INIT_FAIL;
		}
		len = (len + 3)&~3;
		if((buf->size -= len) >= 0)
		{
			text->core.lzw->line_buf = buf->start;
			buf->start += len;	
			MEMSET((void *)text->core.lzw->line_buf,0,len);
			GIF_PRINTF("lzw line buf size %x addr %x free size %x\n",len,text->core.lzw->line_buf,buf->size);
			return GIF_TRUE;
		}
#else
		return GIF_TRUE;
#endif		
	}
	
LZW_INIT_FAIL:
	gif_core_set_error(text,GIF_ERR_LZW_INIT);
	return GIF_FALSE;
}


