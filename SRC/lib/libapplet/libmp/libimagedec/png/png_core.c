/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *  (C)
 *  File: png_core.c
 *  (I)
 *  Description: The core file of the png decoder. Implement the decoder routine
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

UINT32 png_core_error(struct png_core *core)
{
	return core->error;
}

void png_core_set_error(struct png_core *core,UINT32 code)
{
	PNG_PRINTF("PNG error %d\n",code);
	core->error = code;
}

PNG_INLINE void core_routine(UINT32 par1,UINT32 par2)
{
	struct png_core *core = (struct png_core *)par1;
	enum PNG_MODE mode = (enum PNG_MODE)par2;

	PNG_ENTER;
	if(!png_info_file_identification(core))
	{
		PNG_PRINTF("png file identify fail\n");
		goto CORE_EXIT;
	}
	
	do
	{
		if(png_info_parse_chunk(core))
		{
			if(core->flag & PNG_INFO_IDAT)
			{
				core->flag &= ~PNG_INFO_IDAT;
				png_inflate_dec(core);
#ifndef PNG_MEM_DOWN
				if(png_inflate_pass_done(core))
				{
					PNG_PRINTF("inflate one pass ok. start filter\n");
					png_filter_one_pass(core);
					PNG_PRINTF("stop filter\n");
				}
#endif
			}
			else if(core->flag & PNG_INFO_IEND)
			{
				goto CORE_EXIT;
			}
		}
	}while(!png_core_error(core));
	
CORE_EXIT:	
	PNG_EXIT;
	core->status = PNG_IDLE;
}

void *PNG_MALLOC(struct png_cfg *cfg,UINT32 size)
{
	size = (size + 3) & ~0x3;
	if(cfg->dec_buf_size > size)
	{
		cfg->used_size += size;
		cfg->dec_buf_size -= size;
		cfg->dec_buf_start += size;
		PNG_PRINTF("Malloc buffer size %d total %d\n",size,cfg->used_size);
		return (void *)(cfg->dec_buf_start - size);
	}
	return NULL;
}

void PNG_MEMCPY(void *dst,void *src,UINT32 len)
{
	UINT16 *s = (UINT16 *)src;
	UINT16 *d = (UINT16 *)dst;
	UINT32 l = (len>>1);

	if((!dst&0x01) && (!src&0x01) && (len>1)){
		while(l--){
			*d++ = *s++;
		}
		if(len&1){
			*(UINT8 *)d = *(UINT8 *)s;
		}
	}else
		MEMCPY(dst,src,len);	
}

PNG_RET png_core_status(struct png_instance *png,enum PNG_STATUS *status)
{
	struct png_core *core = (struct png_core *)png->core;
	static UINT32 png_time_out = 0;
	static UINT32 png_time = 0;

	*status = PNG_IN_JOB;
	
	osal_task_dispatch_off();	
	if(PNG_IDLE == core->status)
	{
		*status = PNG_IDLE;
		png_time_out = png_time = 0;
	}
	else
	{
		if(PNG_DECOMPRESS_SYNC_TIME_OUT <= png_time_out)
		{
			osal_task_delete(core->task_id);	
			core->status = PNG_IDLE;
			*status = PNG_IDLE;
			png_time_out = png_time = 0;
			PNG_PRINTF("png dec force finish\n");
		}
		else
		{
			if(0 != png_time)
			{
				png_time_out += osal_get_tick() - png_time;
			}
			png_time = osal_get_tick();
		}
	}
	osal_task_dispatch_on();
	
	return PNG_TRUE;
}

PNG_RET png_core_start(struct png_instance *png,struct png_image *img,enum PNG_MODE mode)
{
	struct png_core *core = (struct png_core *)png->core;

	osal_task_dispatch_off();
	if(core->status > PNG_IDLE)
		goto CORE_START_FAIL;
	core->status = PNG_IN_JOB;
	osal_task_dispatch_on();

	MEMSET((void *)&core->inf,0,sizeof(struct png_inflate));
	MEMCPY((void *)&core->cfg,(void *)&core->cfg_bk,sizeof(struct png_cfg));
	png_in_init(png);
	core->flag = 0;
	core->out.img = img;
	if(PNG_POLL == mode)
	{
		OSAL_T_CTSK t_ctsk;
		ER s_flag;
		UINT32 ptn_flag;
		
		t_ctsk.itskpri = OSAL_PRI_NORMAL;
		t_ctsk.stksz = 0x4000;
		t_ctsk.quantum = 10;
		t_ctsk.para1 = (UINT32)core;
		t_ctsk.para2 = (UINT32)mode;
		t_ctsk.name[0] = 'P';
		t_ctsk.name[1] = 'N';
		t_ctsk.name[2] = 'G';
		t_ctsk.task = core_routine;
		core->task_id = OSAL_INVALID_ID;
		core->task_id = osal_task_create(&t_ctsk);
		if(OSAL_INVALID_ID == core->task_id)
		{
			core->status = PNG_IDLE;
			PNG_PRINTF("create task fail %s \n",__FUNCTION__);
			goto CORE_START_FAIL;
		}
	}
	else
	{
		core_routine((UINT32)core,(UINT32)mode);
		core->status = PNG_IDLE;
	}
	
	return PNG_TRUE;

CORE_START_FAIL:
	osal_task_dispatch_on();
	return PNG_FALSE;
}

void png_core_show_img(struct png_instance *png,struct png_image *img)
{
	if(png->view_routine)
		png->view_routine(img, (struct png_view_par *)png->par);
}

void png_core_finish(struct png_instance *png)
{
	struct png_core *core = (struct png_core *)png->core;
	
	osal_task_dispatch_off();
	if(PNG_IDLE != core->status)
		osal_task_delete(core->task_id);	
	core->status = PNG_IDLE;
	osal_task_dispatch_on();
}

PNG_RET png_core_init(struct png_instance *png)
{
	struct png_core *core = NULL;
	struct png_cfg *cfg = (struct png_cfg *)png->par;
	UINT32 len = (sizeof(struct png_core) + 3) & ~3;

	cfg->bit_buf_start &= ~(7<<28);
	cfg->dec_buf_start &= ~(7<<28);
	if(NULL != (core = (struct png_core *)PNG_MALLOC(cfg,len)))
	{
		png->core = (UINT32)core;
		MEMSET((void *)core,0,len);
		MEMCPY((void *)&core->cfg,(void *)cfg,sizeof(struct png_cfg));
		MEMCPY((void *)&core->cfg_bk,(void *)cfg,sizeof(struct png_cfg));
		core->flag_id = OSAL_INVALID_ID;
		core->flag_id = osal_flag_create(0);
		if(OSAL_INVALID_ID == core->flag_id)
		{
			PNG_PRINTF("create flag fail %s \n",__FUNCTION__);
			return PNG_FALSE;
		}
		PNG_PRINTF("Core info len %d\n",len);
		return PNG_TRUE;
	}
	PNG_ASSERT(PNG_ERR_MALLOC);
	return PNG_FALSE;
}

void png_core_free(struct png_instance *png)
{
	png_core_finish(png);
	osal_flag_delete(((struct png_core *)png->core)->flag_id);
}
