/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *  (C)
 *  File: png_filter.c
 *  (I)
 *  Description: de-filter operation to the one pass image
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

PNG_INLINE void FIL_NULL(UINT8 *in,UINT8 *out,UINT32 size,UINT8 bpp,UINT8 *prior)
{
	MEMCPY((void *)out,(void *)in,size);
}

PNG_INLINE void FIL_SUB(UINT8 *in,UINT8 *out,UINT32 size,UINT8 bpp,UINT8 *prior)
{
	UINT8 ref[PNG_MAX_BPP];
	int cnt = 0;

	if(0 != size%bpp)
	{
		PNG_PRINTF("Filter fail %s\n",__FUNCTION__);
		return;
	}
	
	MEMSET((void *)ref,0,PNG_MAX_BPP);
	while(size > 0)
	{
		do
		{
			ref[cnt] = ref[cnt] + *in++;
			*out++ = ref[cnt];
			cnt++;
		}while(cnt < bpp);
		size -= bpp;
		cnt = 0;
	}
}

PNG_INLINE void FIL_UP(UINT8 *in,UINT8 *out,UINT32 size,UINT8 bpp,UINT8 *prior)
{
	if(0 != size%bpp)
	{
		PNG_PRINTF("Filter fail %s\n",__FUNCTION__);
		return;
	}
	
	while(size--)
	{
		(*out++) = (*in++) + (*prior++);		
	}
}

PNG_INLINE void FIL_AVG(UINT8 *in,UINT8 *out,UINT32 size,UINT8 bpp,UINT8 *prior)
{
	UINT8 ref[PNG_MAX_BPP];
	UINT16 avg = 0;
	int cnt = 0;

	if(0 != size%bpp)
	{
		PNG_PRINTF("Filter fail %s\n",__FUNCTION__);
		return;
	}
	
	MEMSET((void *)ref,0,PNG_MAX_BPP);
	while(size > 0)
	{
		do
		{
			avg = (UINT16)(*prior++ + ref[cnt]);
			avg >>= 1;
			ref[cnt] = (UINT8)avg + *in++;
			*out++ = ref[cnt];
			cnt++;
		}while(cnt < bpp);
		size -= bpp;
		cnt = 0;
	}
}

PNG_INLINE void FIL_PAE(UINT8 *in,UINT8 *out,UINT32 size,UINT8 bpp,UINT8 *prior)
{
	UINT8 ref[PNG_MAX_BPP];
	UINT8 ref_p[PNG_MAX_BPP];
	UINT8 b = 0;
	INT16 init_e = 0;
	INT16 dis_a = 0,dis_b = 0,dis_c = 0;
	int cnt = 0;

	if(0 != size%bpp)
	{
		PNG_PRINTF("Filter fail %s\n",__FUNCTION__);
		return;
	}
	
	MEMSET((void *)ref,0,PNG_MAX_BPP);
	MEMSET((void *)ref_p,0,PNG_MAX_BPP);
	while(size > 0)
	{
		do
		{
			b = *prior++;
			init_e = (INT16)(b + ref[cnt]);
			init_e -= (INT16)ref_p[cnt];
			dis_a = PNG_ABS(init_e - (INT16)ref[cnt]);
			dis_b = PNG_ABS(init_e - (INT16)b);
			dis_c = PNG_ABS(init_e - (INT16)ref_p[cnt]);
			if(dis_a <= dis_b && dis_a <= dis_c)
				ref[cnt] += *in++;
			else if(dis_b <= dis_c)
				ref[cnt] = b + *in++;
			else
				ref[cnt] = ref_p[cnt] + *in++;
			ref_p[cnt] = b;
			*out++ = ref[cnt];
			cnt++;
		}while(cnt < bpp);
		size -= bpp;
		cnt = 0;
	}	
}

PNG_INLINE void (*filter_routine[5])(UINT8 *,UINT8 *,UINT32,UINT8,UINT8 *) = {FIL_NULL,FIL_SUB,FIL_UP,FIL_AVG,FIL_PAE};

#ifndef PNG_MEM_DOWN
PNG_RET png_filter_one_pass(struct png_core *core)
{
	struct png_inf_scan *scan = &core->inf.scan;
	struct png_filter *filter = &core->filter;
	UINT8 *bak_addr = NULL;
	
	MEMSET((void *)filter->line_out,0,filter->line_size);
	MEMSET((void *)filter->prior_line,0,filter->line_size);
	
	while(scan->line_num--)
	{
		filter->type = *filter->line_in++;
		if(filter->type < 5)
			filter_routine[filter->type](filter->line_in,filter->line_out,filter->line_size,filter->pixel_bytes,filter->prior_line);
		png_out_line(core);
		bak_addr = filter->prior_line;
		filter->prior_line = filter->line_out;
		filter->line_out = bak_addr;
		filter->line_in += filter->line_size;
		filter->cur_line++;
	}
	
	return PNG_TRUE;
}

#else
PNG_RET png_filter_scan_line(struct png_core *core)
{
	struct png_inf_scan *scan = &core->inf.scan;
	struct png_filter *filter = &core->filter;
	UINT8 *bak_addr = NULL;

	filter->line_in = scan->read;
	filter->type = *filter->line_in++;
	if(filter->type < 5)
		filter_routine[filter->type](filter->line_in,filter->line_out,filter->line_size,filter->pixel_bytes,filter->prior_line);
	png_out_line(core);
	bak_addr = filter->prior_line;
	filter->prior_line = filter->line_out;
	filter->line_out = bak_addr;
	filter->cur_line++;
	scan->read += scan->line_size;
	if((scan->size - scan->count) <= (scan->line_size + PNG_MAX_LEN)){
		PNG_MEMCPY((void *)scan->start,(void *)(scan->write - PNG_MAX_WIN_SIZE),PNG_MAX_WIN_SIZE);
		scan->write = scan->start + PNG_MAX_WIN_SIZE;
		scan->read -= scan->count - PNG_MAX_WIN_SIZE;
		scan->count = PNG_MAX_WIN_SIZE;
	}
	return PNG_TRUE;
}

#endif
PNG_RET png_filter_init(struct png_core *core)
{
	struct png_cfg *cfg = &core->cfg;
	struct png_ihdr *ihdr = &core->info.IHDR;
	struct png_inf_scan *scan = &core->inf.scan;
	struct png_filter *filter = &core->filter;

	if(ihdr->fit_mtd > 0)
	{
		PNG_PRINTF("Don't support other filter method\n");
		goto FIL_INIT_FAIL;
	}
	MEMSET((void *)filter,0,sizeof(struct png_filter));
	filter->pixel_bytes = (scan->bpp + 7)>>3;
	filter->cur_line = 0;
	filter->line_in = scan->start;
	filter->line_size = scan->line_size - 1;
	filter->line_out = (UINT8 *)PNG_MALLOC(cfg,filter->line_size);
	filter->prior_line = (UINT8 *)PNG_MALLOC(cfg,filter->line_size);
	if(NULL == filter->line_out || NULL == filter->prior_line)
	{
		goto FIL_INIT_FAIL;
	}
	MEMSET((void *)filter->line_out,0,filter->line_size);
	MEMSET((void *)filter->prior_line,0,filter->line_size);
	return PNG_TRUE;

FIL_INIT_FAIL:	
	PNG_ASSERT(PNG_ERR_MALLOC);
	return PNG_FALSE;
}

