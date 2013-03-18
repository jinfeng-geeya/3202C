/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *  (C)
 *  File: gif_in.c
 *  (I)
 *  Description: Bit stream operation
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
#include "gif_in.h"
#include "gif_debug.h"


GIF_INLINE GIF_RET gif_prepare_in_buf(struct gif_str *str,struct gif_file *file)
{	
	UINT32 suc_size = 0;
	
	if(0 != str->left)
	{
		GIF_ASSERT("GIF_ERR_OTHERS");
		return GIF_FALSE;
	}
	else
	{
		if(!file->finish)
		{
			suc_size = file->fread(file->handle,str->start,str->size);
			GIF_PRINTF("read file suc size %d start addr %x\n",suc_size,str->start);
			if(0 != suc_size && suc_size <= str->size)
			{
				str->read = str->start;
				str->left = suc_size;
				if(suc_size < str->size)
				{
					GIF_PRINTF("file finish %s\n",__FUNCTION__);
					file->finish = 1;
				}
				return GIF_TRUE;
			}
			else
				file->finish = 1;
		}
		return GIF_FALSE;
	}
}

GIF_INLINE GIF_RET gif_read_sub_size(struct gif_in *in)
{
	INT16 byte = gif_in_read_byte(in);
	
#if 1
	if(byte <= 0)
	{
		GIF_PRINTF("file finish %s\n",__FUNCTION__);
		in->file.finish = 1;
		return GIF_FALSE;
	}
#endif
	in->sub.left = (UINT8)byte;
	in->sub.size = in->sub.left;
	//GIF_PRINTF("sub block size %d \n",in->sub.left);
	return GIF_TRUE;
}

UINT8 gif_in_file_end(struct gif_in *in)
{
	return (UINT8)in->file.finish;
}

UINT32 gif_in_file_offset(struct gif_in *in)
{
	return in->file.offset;
}

GIF_RET gif_in_file_set_offset(struct gif_in *in,UINT32 offset)
{
	struct gif_file *file = &in->file;
	
	if(file->fseek(file->handle,offset, SEEK_SET) < 0)
	{
		return GIF_FALSE;
	}
	file->offset = offset;
	file->finish = 0;
	in->str.left = 0;
	MEMSET((void *)&in->bit,0,sizeof(struct gif_bit));
	return GIF_TRUE;
}

//return the data ,don't move the postion
INT16 gif_in_next_byte(struct gif_in *in)
{
	struct gif_str *str = &in->str;
	struct gif_file *file = &in->file;
	UINT16 byte = 0;
	
	if(0 == str->left)
	{
		if(!gif_prepare_in_buf(str,file))
			return -1;
	}
	byte = (UINT16)(*(str->read));
	return (INT16)byte;	
}

//return data at current position and move to the next position
//when error happen, return -1
INT16 gif_in_read_byte(struct gif_in *in)
{
	INT16 byte = 0;

	byte = gif_in_next_byte(in);
	if(byte >= 0)
	{
		in->str.left--;
		in->str.read++;
		in->file.offset++;
	}
	return byte;
}

//range of num: 2 -- 12 bits
void gif_in_init_bit(struct gif_in *in)
{
	in->bit.bits = 0;
	in->bit.value = 0;
	in->sub.left = 0;
	in->sub.size = 0;
}

INT16 gif_in_read_bits(struct gif_in *in,UINT8 num)
{
	struct gif_bit *bit = &in->bit;
	UINT16 data = 0;
	INT16 byte_c = 0;

	while(1)
	{
		if(bit->bits < num)
		{
			if(0 == in->sub.left)
			{
				if(gif_read_sub_size(in))
					continue;
				else
					return -1;
			}
			else
			{		
				byte_c = gif_in_read_byte(in);
				in->sub.left--;
				if(byte_c < 0)
					return -1;
				bit->value &= ~(0xff<<bit->bits);
				bit->value |= byte_c<<bit->bits;
				bit->bits += 8;
				if(0 == in->sub.left)
					continue;
				byte_c = gif_in_read_byte(in);			
				in->sub.left--;
				if(byte_c < 0)
					continue;
				bit->value &= ~(0xff<<bit->bits);
				bit->value |= byte_c<<bit->bits;		
				bit->bits += 8;
				continue;
			}
		}
		else
		{
			data = (UINT16)(bit->value & ((1<<num) - 1));
			bit->value >>= num;
			bit->bits -= num;
			break;
		}
	}
	return (INT16)data;
}

#ifndef GIF_DYNAMIC_MEMORY
GIF_RET gif_in_init(struct gif_context *text,struct gif_dec_in_par *par,struct gif_res *res)
{
	GIF_RET ret = GIF_FALSE;
	struct gif_in *in = &text->in;
	struct gif_core_buf *buf = &(text->core.buf);
	UINT8 idx = text->core.buf.id;
	
	MEMSET((void *)in,0,sizeof(struct gif_in));
	in->file.handle = par->file;
	in->file.fread = par->fread_callback;
	in->file.fseek = par->fseek_callback;
	in->file.ftell = par->ftell_callback;
	in->file.offset = 0;
	in->str.start = (UINT8 *)res->block[idx].bit_buf_start;
	in->str.read = in->str.start;
	in->str.size = res->block[idx].bit_buf_size;
	in->str.left = 0;
	ret = gif_in_file_set_offset(in,0);
	if(GIF_FALSE == ret)
		gif_core_set_error(text,GIF_ERR_IN_INIT);
	
	GIF_PRINTF("In init done\n");
	return ret;
}
#else
GIF_RET gif_in_init(struct gif_context *text,struct gif_dec_in_par *par,struct gif_res *res)
{
	GIF_RET ret = GIF_FALSE;
	struct gif_in *in = &text->in;
	struct gif_core_buf *buf = &(text->core.buf);
	UINT8 idx = text->core.buf.id;
	UINT32 bit_buf_start;
	UINT32 bit_buf_size;

	if(gif_get_bit_block(&bit_buf_start, &bit_buf_size) == GIF_FALSE)
	{
		GIF_PRINTF("[gif_in_init]: gif_get_bit_block failed.\n");
		return GIF_FALSE;
	}
	
	MEMSET((void *)in,0,sizeof(struct gif_in));
	in->file.handle = par->file;
	in->file.fread = par->fread_callback;
	in->file.fseek = par->fseek_callback;
	in->file.ftell = par->ftell_callback;
	in->file.offset = 0;
	in->str.start = (UINT8 *)bit_buf_start;
	in->str.read = in->str.start;
	in->str.size = bit_buf_size;
	in->str.left = 0;
	in->frame_num = par->frame_num;
	ret = gif_in_file_set_offset(in,0);
	if(GIF_FALSE == ret)
		gif_core_set_error(text,GIF_ERR_IN_INIT);
	
	GIF_PRINTF("In init done\n");
	return ret;
}
#endif


