/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *  (C)
 *  File: png_info.c
 *  (I)
 *  Description: parsing the png file. identify the file and parse the different chunks
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

static UINT8 SIG_FILE[] = {137,80,78,71,13,10,26,10};

#define  SIG_IHDR		0x49484452
#define  SIG_PLTE		0x504c5445
#define  SIG_IDAT 		0x49444154
#define  SIG_IEND   		0x49454E44
#define  SIG_TRNS		0x74524E53

PNG_INLINE INT32 READ4BYTES(struct png_in *in)
{
	INT32 data = 0;
	INT16 byte = 0;
	int i = 3;

	do
	{
		byte = png_in_read_byte(in);
		data |= (byte & 0xFF)<<(i<<3);
	}while(i--);
	return (byte < 0)?-1:data;
}

PNG_INLINE INT16 READ2BYTES(struct png_in *in)
{
	INT16 data = 0;
	INT16 byte = 0;
	int i = 1;

	do
	{
		byte = png_in_read_byte(in);
		data |= (byte & 0xFF)<<(i<<3);
	}while(i--);
	return (byte < 0)?-1:data;
}

PNG_INLINE UINT32 read_IHDR(struct png_in *in,struct png_ihdr *IHDR)
{
	IHDR->width = READ4BYTES(in);
	IHDR->height = READ4BYTES(in);
	IHDR->bit_depth = png_in_read_byte(in);
	IHDR->color_type = png_in_read_byte(in);
	IHDR->com_mtd = png_in_read_byte(in);
	IHDR->fit_mtd = png_in_read_byte(in);
	IHDR->int_mtd = png_in_read_byte(in);

	if(IHDR->width > PNG_MAX_PIC_WIDTH || IHDR->height > PNG_MAX_PIC_HEIGHT)
	{
		PNG_PRINTF("pic with too big size width %d height %d\n",IHDR->width,IHDR->height);
		return 0;
	}

	PNG_PRINTF("PNG info w %d h %d bits %d color %d\n",IHDR->width,IHDR->height,IHDR->bit_depth\
		,IHDR->color_type);
	return (png_in_next_byte(in) < 0) ? 0:13;
}
PNG_INLINE void read_PLTE(struct png_in *in,struct png_plte *plte)
{
	UINT32 num = plte->num;
	UINT8 *buf = plte->palette;

	while(num--)
	{
		*(buf + 2) = png_in_read_byte(in);
		*(buf + 1) = png_in_read_byte(in);
		*buf = png_in_read_byte(in);
		*(buf + 3) = 0xFF;
		buf += 4;
	}
}

PNG_INLINE void update_PLTE(UINT32 t_num, struct png_in *in,struct png_plte *plte)
{
	UINT32 num = ((plte->num > t_num)? t_num: plte->num);
	UINT8 *buf = plte->palette;

	while(num--)
	{
		*(buf + 3) = png_in_read_byte(in);	
		buf += 4;
	}
}

PNG_INLINE UINT32 parse_CMF(struct png_in *in,struct png_CMF *cmf)
{
	UINT32 cnt = 0;

	PNG_PRINTF("Start a libz stream\n");
	PNG_ENTER;
	cmf->CM = png_in_read_byte(in);
	cmf->FLG = png_in_read_byte(in);
	if(8 != (cmf->CM & 0xF) || !((UINT32)(cmf->CM<<8 + cmf->FLG) % 31))
	{
		PNG_PRINTF("Don't support this CMF\n");
		return 0;
	}

	cnt = 2;
	if(cmf->FLG & 0x20)
	{
		cmf->DICT = READ4BYTES(in);
		cnt += 2;
	}
	PNG_EXIT;
	return cnt;
}

PNG_INLINE PNG_RET inf_scan_init(struct png_core *core)
{
	struct png_cfg *cfg = &core->cfg;
	struct png_ihdr *ihdr = &core->info.IHDR;
	struct png_inf_scan *scan = &core->inf.scan;

	if(ihdr->com_mtd > 0 || ihdr->int_mtd > 0)
	{
		PNG_PRINTF("Don't support interlaced or other compression method features\n");
		return PNG_FALSE;
	}

	core->inf.deflate_blk_num = 0;
	
	switch(ihdr->color_type)
	{
		case 0:
			scan->bpp = ihdr->bit_depth;
			scan->line_size = (ihdr->bit_depth * ihdr->width + 7)>>3;
			break;
		case 2:
			if(!(8 == ihdr->bit_depth || 16 == ihdr->bit_depth))
				return PNG_FALSE;
			scan->bpp = ihdr->bit_depth * 3;
			scan->line_size = (ihdr->bit_depth * ihdr->width * 3)>>3;
			break;
		case 3:
			if(16 == ihdr->bit_depth)
				return PNG_FALSE;
			scan->bpp = 8;
			scan->line_size = (ihdr->width * ihdr->bit_depth + 7)>>3;
			break;			
		case 4:
			if(!(8 == ihdr->bit_depth || 16 == ihdr->bit_depth))
				return PNG_FALSE;
			scan->bpp = ihdr->bit_depth<<1;
			scan->line_size = (ihdr->bit_depth * ihdr->width * 2)>>3;
			break;
		case 6:
			if(!(8 == ihdr->bit_depth || 16 == ihdr->bit_depth))
				return PNG_FALSE;
			scan->bpp = ihdr->bit_depth<<2;
			scan->line_size = (ihdr->bit_depth * ihdr->width * 4)>>3;
			break;			
		default:
			return PNG_FALSE;
	}

	//add the position of filter type of the scan line
	scan->line_size++;
	scan->line_num = ihdr->height;
	scan->finish = 0;
	scan->count = 0;
#ifdef PNG_MEM_DOWN
	scan->size = PNG_MAX_WIN_SIZE<<2;
#else
	scan->size = scan->line_size*ihdr->height;
#endif
	scan->read = scan->write = scan->start = (UINT8 *)PNG_MALLOC(cfg,scan->size);
	if(NULL == scan->start || NULL == scan->start)
	{
		PNG_ASSERT(PNG_ERR_MALLOC);
		return PNG_FALSE;
	}
	PNG_PRINTF("scan start %x size %d\n",scan->write,scan->size);
	return PNG_TRUE;
}

PNG_INLINE PNG_RET pass_init(struct png_core *core)
{
	if(inf_scan_init(core))
	{
		PNG_PRINTF("Init scan information ok\n");
		if(png_filter_init(core) && png_out_init(core))
		{
			PNG_PRINTF("Init filter and out ok\n");
			core->inf.status = INF_BLK_HEADER;
			return PNG_TRUE;
		}
	}
	return PNG_FALSE;
}
// when meet the IEND and IDAT chunk, operation parsing the chunk return the routine
PNG_RET png_info_parse_chunk(struct png_core *core)
{
	struct png_in *in = &(core->in);
	struct png_info *info = &(core->info);
	struct png_chunk *chunk = info->chunk + info->num;
	UINT32 len = 0;

	PNG_ENTER;
	if((SIG_IDAT == (chunk - 1)->type) && (info->num > 0))
	{
		if(!(core->flag & PNG_INFO_CMF))
			info->CMF.ADLER32 = READ4BYTES(in);
		(chunk - 1)->CRC = READ4BYTES(in);
	}
	
	do{
		chunk->len = READ4BYTES(in);
		chunk->type = READ4BYTES(in);
		//chunk->data
		if(png_in_next_byte(in) < 0)
			goto CHUNK_FAIL;
		switch(chunk->type)
		{
			case SIG_IHDR:
				PNG_PRINTF("IHDR chunk\n");
				core->flag |= PNG_INFO_IHDR;
				info->IHDR_idx = info->num;
				if(chunk->len != read_IHDR(in,&info->IHDR))
					goto CHUNK_FAIL;
				break;
			case SIG_PLTE:
				PNG_PRINTF("PLTE chunk\n");
				core->flag |= PNG_INFO_PLTE;
				info->PLTE_idx = info->num;
				if(0 != chunk->len%3)
					goto CHUNK_FAIL;
				info->PLTE.num = chunk->len/3;
				info->PLTE.palette = (UINT8 *)PNG_MALLOC(&core->cfg,info->PLTE.num<<2);
				if(NULL == info->PLTE.palette)
				{
					PNG_ASSERT(PNG_ERR_MALLOC);
					goto CHUNK_FAIL;
				}
				read_PLTE(in,&info->PLTE);
				break;
			case SIG_TRNS:
				PNG_PRINTF("TRNS chunk\n");
				//core->flag |= PNG_INFO_TRNS;
				if(0 < chunk->len)
				{					
					struct png_ihdr *ihdr = &core->info.IHDR;
					if((NULL != info->PLTE.palette) && (3 == ihdr->color_type))
						update_PLTE(chunk->len, in, &info->PLTE);
					else
						goto IGNORE_DATA;
				}
				break;	
			case SIG_IDAT:
				PNG_PRINTF("IDAT chunk len %x\n",chunk->len);
				if(!(core->flag & PNG_INFO_IHDR))
					goto CHUNK_FAIL;
				if(!(core->flag & PNG_INFO_CMF))
				{
					core->flag |= PNG_INFO_CMF; 
					if(0 == (len = parse_CMF(in,&info->CMF)))
					{
						PNG_PRINTF("Parse CMF fail\n");
						goto CHUNK_FAIL;
					}
					chunk->len -= len;
					if(!pass_init(core))
						goto CHUNK_FAIL;
				}
				core->flag |= PNG_INFO_IDAT;	
				info->IDAT_idx = info->num;
				goto CHUNK_SUC;
			case SIG_IEND:
				PNG_PRINTF("IEND chunk\n");
				core->flag = 0;
				core->flag |= PNG_INFO_IEND;
				info->IEND_idx = info->num;
				goto CHUNK_SUC;
			default:
IGNORE_DATA:				
				len = chunk->len;
				while(len)
				{
					if(png_in_read_byte(in) < 0)
						goto CHUNK_FAIL;	
					len--;
				}
				break;
		}
		chunk->CRC = READ4BYTES(in);
		info->num++;
		chunk++;
	}while(1);
	
CHUNK_SUC:
	PNG_EXIT;
	info->num++;
	return PNG_TRUE;

CHUNK_FAIL:
	png_core_set_error(core,PNG_ERR_FILE);
	return PNG_FALSE;
}

PNG_RET png_info_file_identification(struct png_core *core)
{
	struct png_in *in = &(core->in);
	INT16 byte = 0;
	int i = 0;

	do{
		byte = png_in_read_byte(in);
		if((byte < 0) || ((UINT8)byte != SIG_FILE[i]))
			return PNG_FALSE;
		i++;
	}while(i < 8);
	MEMSET((void *)&core->info,0,sizeof(struct png_info));
	PNG_PRINTF("\n\n\n\nIdentify png file ok\n");
	return PNG_TRUE;
}


