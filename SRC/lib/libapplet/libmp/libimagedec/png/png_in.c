/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *  (C)
 *  File: png_in.c
 *  (I)
 *  Description: file operation
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
#include <api/libmp/png.h>
#include "png_core.h"
#include "png_debug.h"


PNG_INLINE PNG_RET png_prepare_in_buf(struct png_str *str,struct png_file *file)
{	
	UINT32 suc_size = 0;
	
	if(0 != str->left)
	{
		PNG_ASSERT("PNG_ERR_OTHERS");
		return PNG_FALSE;
	}
	else
	{
		if(!file->finish)
		{
			suc_size = file->fread(file->handle,str->start,str->size);
			PNG_PRINTF("read file suc size %d start addr %x\n",suc_size,str->start);
			if(0 != suc_size && suc_size <= str->size)
			{
				str->read = str->start;
				str->left = suc_size;
				if(suc_size < str->size)
				{
					PNG_PRINTF("file finish %s\n",__FUNCTION__);
					file->finish = 1;
				}
				return PNG_TRUE;
			}
			else
				file->finish = 1;
		}
		return PNG_FALSE;
	}
}

#if 0
PNG_INLINE PNG_RET png_read_sub_size(struct png_in *in)
{
	INT16 byte = png_in_read_byte(in);
	
#if 1
	if(byte <= 0)
	{
		PNG_PRINTF("file finish %s\n",__FUNCTION__);
		in->file.finish = 1;
		return PNG_FALSE;
	}
#endif
	in->sub.left = (UINT8)byte;
	in->sub.size = in->sub.left;
	//PNG_PRINTF("sub block size %d \n",in->sub.left);
	return PNG_TRUE;
}
#endif

UINT8 png_in_file_end(struct png_in *in)
{
	return (UINT8)in->file.finish;
}

//return the data ,don't move the postion
INT16 png_in_next_byte(struct png_in *in)
{
	struct png_str *str = &in->str;
	struct png_file *file = &in->file;
	UINT16 byte = 0;
	
	if(0 == str->left)
	{
		if(!png_prepare_in_buf(str,file))
			return -1;
	}
	byte = (UINT16)(*(str->read));
	return (INT16)byte;	
}

//return data at current position and move to the next position
//when error happen, return -1
INT16 png_in_read_byte(struct png_in *in)
{
	INT16 byte = 0;

	byte = png_in_next_byte(in);
	if(byte >= 0)
	{
		in->str.left--;
		in->str.read++;
		in->file.offset++;
	}
	return byte;
}


void png_in_write_byte(struct png_in *in,UINT8 data)
{
	in->str.left++;	
	in->file.offset--;
	*(--in->str.read) = data;
}
#if 0
INT16 png_in_read_bits(struct png_in *in,UINT8 num)
{
	struct png_bit *bit = &in->bit;
	UINT16 data = 0;
	INT16 byte_c = 0;

	while(1)
	{
		if(bit->bits < num)
		{
			if(0 == in->sub.left)
			{
				if(png_read_sub_size(in))
					continue;
				else
					return -1;
			}
			else
			{		
				byte_c = png_in_read_byte(in);
				in->sub.left--;
				if(byte_c < 0)
					return -1;
				bit->value &= ~(0xff<<bit->bits);
				bit->value |= byte_c<<bit->bits;
				bit->bits += 8;
				if(0 == in->sub.left)
					continue;
				byte_c = png_in_read_byte(in);			
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
#endif

PNG_RET png_in_init(struct png_instance *png)
{
	struct png_core *core = (struct png_core *)png->core;
	struct png_cfg *cfg = &core->cfg;
	struct png_in *in = &core->in;

	MEMSET((void *)in,0,sizeof(struct png_in));
	MEMCPY((void *)&in->file,(void *)png->par,sizeof(struct png_file));
	in->str.read = in->str.start = (void *)cfg->bit_buf_start;
	in->str.size = cfg->bit_buf_size;
	return PNG_TRUE;
}



