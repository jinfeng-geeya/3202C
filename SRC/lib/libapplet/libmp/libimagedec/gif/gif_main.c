/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *  (C)
 *  File: gif_main.c
 *  (I)
 *  Description:  Control progress work flow overall. According this, API call the other modules of the GIF decoder.
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
#include "gif_debug.h"

//global variables
//extern
extern struct gif_res g_gif_ins_res[GIF_MAX_INSTANCE];

//internal for debug
struct gif_main *g_gif_main[GIF_MAX_INSTANCE];

#ifdef GIF_DYNAMIC_MEMORY
struct gif_dec_block_t gif_free_dec_block[GIF_MAX_SUPPORT_FILES];
struct gif_dec_block_t gif_used_dec_block[GIF_MAX_SUPPORT_FILES];
UINT32 free_dec_block_cnt = 0;
UINT32 used_dec_block_cnt = 0;
struct gif_bit_block_t gif_bit_block[GIF_MAX_SUPPORT_FILES];
UINT32 gif_bit_block_cnt = 0;
int gif_bit_block_cur = 0;
#endif

extern GIF_RET (*g_gif_out_init)(struct gif_context *,struct gif_dec_out_par *);
extern GIF_RET (*g_gif_out_enter)(struct gif_out *,struct gif_lzw *);
extern void (*g_gif_out_img)(struct gif_context *);
extern GIF_RET (*g_gif_out_exit)(struct gif_context *);

#ifndef GIF_DYNAMIC_MEMORY
GIF_INLINE void res_init(struct gif_res *res)
{
	UINT32 bit = (UINT32)res->par.bit_buf_start,dec = (UINT32)res->par.dec_buf_start;
	UINT32 b_size = res->par.bit_buf_size,d_size = res->par.dec_buf_size;
	UINT32 unit_dec_size = 0;
	int i = 0;

	if(!res->active)
		return;

	if((0 != res->par.max_width) && (0 != res->par.max_height))
	{
		unit_dec_size = res->par.max_width * res->par.max_height;
		if(res->par.only_dec_first_pic)
			unit_dec_size = (unit_dec_size<<2) + 0xB000;
		else
			unit_dec_size = (unit_dec_size<<3) + 0xB000;
	}
	else
	{
		unit_dec_size = GIF_DEC_BUF_SIZE;
	}
		
	// set the dec buff as 4-bytes aligned
	dec = (dec + 3) & ~3;
	d_size -= dec - (UINT32)res->par.dec_buf_start;

	for(i = 0;i < GIF_MAX_SUPPORT_FILES;i++)
	{
		res->block[i].bit_buf_start = bit;//(UINT8 *)(((UINT32)bit & ~(7<<28)) | (1<<31));
		res->block[i].dec_buf_start = dec;//(UINT8 *)(((UINT32)dec & ~(7<<28)) | (1<<31));
		if(b_size >= GIF_BIT_BUF_SIZE &&  d_size >= unit_dec_size)
		{
			res->block[i].bit_buf_size = GIF_BIT_BUF_SIZE;
			res->block[i].dec_buf_size = unit_dec_size;	
			bit += GIF_BIT_BUF_SIZE;
			dec += unit_dec_size;
			b_size -= GIF_BIT_BUF_SIZE;
			d_size -= unit_dec_size;
		}
		else
		{
			res->block[i].bit_buf_size = b_size;
			res->block[i].dec_buf_size = d_size;				
			break;
		}
	}
}
#else
GIF_INLINE void res_init(struct gif_res *res)
{
	UINT32 bit = (UINT32)res->par.bit_buf_start,dec = (UINT32)res->par.dec_buf_start;
	UINT32 b_size = res->par.bit_buf_size,d_size = res->par.dec_buf_size;
	UINT32 unit_dec_size = 0;
	int i = 0;

	if(!res->active)
		return;

	// set the dec buff as 4-bytes aligned
	dec = (dec + 3) & ~3;
	d_size -= dec - (UINT32)res->par.dec_buf_start;
	
	gif_free_dec_block[0].address = dec;
	gif_free_dec_block[0].size = d_size;
	free_dec_block_cnt = 1;
	used_dec_block_cnt = 0;
	gif_bit_block_cnt = 0;
	GIF_PRINTF("[res_init]: gif_free_dec_block[0].address=0x%x, size=0x%x.\n", gif_free_dec_block[0].address, gif_free_dec_block[0].size);
	
	for(i = 0;i < GIF_MAX_SUPPORT_FILES;i++)
	{
		if(b_size >= GIF_BIT_BUF_SIZE)
		{
			gif_bit_block[i].address = bit;
			gif_bit_block[i].size = GIF_BIT_BUF_SIZE;
			gif_bit_block[i].used = 0;
			bit += GIF_BIT_BUF_SIZE;
			b_size -= GIF_BIT_BUF_SIZE;
			gif_bit_block_cnt++;
			GIF_PRINTF("[res_init]: gif_bit_block[%d].address = 0x%x, size = 0x8000.\n", i, gif_bit_block[i].address);
		}
		else
		{
			gif_bit_block[i].address = 0;
			gif_bit_block[i].size = 0;
			gif_bit_block[i].used = -1;
			GIF_PRINTF("[res_init]: gif_bit_block_cnt=%d.\n", gif_bit_block_cnt);
			break;
		}
	}
	
}
#endif

#ifndef GIF_DYNAMIC_MEMORY
GIF_INLINE GIF_RET core_buf_init(struct gif_core *core,struct gif_res *res)
{
	struct gif_block *block = res->block;
	UINT8 free_idx = 0xFF;
	int i = 0;
	
	for(i = 0;i < GIF_MAX_SUPPORT_FILES;i++)
	{
		if(!block->busy)
		{	
			block->busy = GIF_TRUE;
			free_idx = i;
			break;
		}
		block++;
	}
	if(0xFF != free_idx)
	{
		core->buf.id = free_idx;
		core->buf.valid = GIF_TRUE;
		core->buf.start = res->block[free_idx].dec_buf_start;
		core->buf.size = res->block[free_idx].dec_buf_size;
		return GIF_TRUE;
	}
	return GIF_FALSE;
}
#else
GIF_INLINE GIF_RET core_buf_init(struct gif_core *core,struct gif_res *res,struct gif_dec_par *par)
{
	int i = 0, j = 0;
	UINT32 gif_required_dec_size = 0;
	struct gif_dec_out_par *out_par = &par->out_par;
	struct gif_dec_in_par *in_par = &par->in_par;
	UINT32 gif_dec_used_size = 0;
	
	//allocate bit block
	gif_bit_block_cur = -1;
	for(i = 0; i < gif_bit_block_cnt; i++)
	{
		if(gif_bit_block[i].used == 0)
		{
			gif_bit_block_cur = i;
			core->buf.id = i;
			gif_bit_block[i].used = 1;
			GIF_PRINTF("[core_buf_init]: gif_bit_block_cur=%d.\n", gif_bit_block_cur);
			break;
		}
	}
	if(gif_bit_block_cur == -1)
	{
		GIF_PRINTF("[core_buf_init]: allocate bit block failed.\n");
		return GIF_FALSE;
	}
	
	out_par->size.w = (out_par->size.w + 3) & ~3;
	gif_required_dec_size = out_par->size.w * out_par->size.h;
	if(res->par.only_dec_first_pic)
		gif_required_dec_size = (gif_required_dec_size<<2) + 0xBC00;
	else
		gif_required_dec_size = (gif_required_dec_size<<3) + 0xBC00;

	if(in_par->frame_num > 1)
	{	
		//allocate for buffer hardware surface for transparent display
		gif_dec_used_size = gif_required_dec_size;
		gif_required_dec_size += out_par->size.w * out_par->size.h * 2;
	}
	//allocate dec block
	for(i = 0; i < free_dec_block_cnt; i++)
	{
		if(gif_required_dec_size <= gif_free_dec_block[i].size)
		{
			GIF_PRINTF("[core_buf_init]: find the suited gif_free_dec_block to allocate a new dec block.\n");

			core->buf.valid = GIF_TRUE;
			core->buf.start = gif_free_dec_block[i].address;
			core->buf.size = gif_required_dec_size;
			core->buf.start_backup = core->buf.start;
			gif_used_dec_block[used_dec_block_cnt].address = gif_free_dec_block[i].address;
			gif_used_dec_block[used_dec_block_cnt].size = gif_required_dec_size;
			used_dec_block_cnt++;
			if(in_par->frame_num > 1)
				core->buf.hard_surface_buf = core->buf.start + gif_dec_used_size;
			GIF_PRINTF("[core_buf_init]: gif_used_dec_block[%d].address=0x%x, size=0x%x, used_dec_block_cnt=%d.\n", used_dec_block_cnt-1, gif_used_dec_block[used_dec_block_cnt-1].address, gif_used_dec_block[used_dec_block_cnt-1].size, used_dec_block_cnt);

			if(gif_required_dec_size == gif_free_dec_block[i].size)
			{
				for(j = i; j < free_dec_block_cnt-1; j++)
				{
					gif_free_dec_block[j].address = gif_free_dec_block[j+1].address;
					gif_free_dec_block[j].size = gif_free_dec_block[j+1].size;
				}
				free_dec_block_cnt--;
				GIF_PRINTF("[core_buf_init]: suited size gif_free_dec_block, free_dec_block_cnt=%d.\n", free_dec_block_cnt);
			}
			else
			{
				gif_free_dec_block[i].address += gif_required_dec_size;
				gif_free_dec_block[i].size -= gif_required_dec_size;
				GIF_PRINTF("[core_buf_init]: gif_free_dec_block[%d].address=0x%x, size=0x%x.\n", i, gif_free_dec_block[0].address, gif_free_dec_block[0].size);
			}

			return GIF_TRUE;
		}
	}

	GIF_PRINTF("[core_buf_init]: allocate new dec block failed.\n");
	return GIF_FALSE;
}
#endif

#ifdef GIF_DYNAMIC_MEMORY
GIF_RET gif_get_bit_block(UINT32 *p_bit_buf_start, UINT32 *p_bit_buf_size)
{	
	//called in gif_in_init()
	if(gif_bit_block_cur != -1)
	{
		*p_bit_buf_start = gif_bit_block[gif_bit_block_cur].address;
		*p_bit_buf_size = gif_bit_block[gif_bit_block_cur].size;
		GIF_PRINTF("[gif_get_bit_block]: get bit_buffer_start=0x%x, size=0x%x.\n", gif_bit_block[gif_bit_block_cur].address, gif_bit_block[gif_bit_block_cur].size);
		return GIF_TRUE;
	}
	
	return GIF_FALSE;
}
#endif

#ifndef GIF_DYNAMIC_MEMORY
GIF_INLINE void core_buf_free(struct gif_core *core,struct gif_res *res)
{
	UINT8 id = 0;
	
	if((id = core->buf.id) < GIF_MAX_SUPPORT_FILES)
	{
		res->block[id].busy = GIF_FALSE;
		core->buf.valid = GIF_FALSE;

		if(core->lzw->img_id == 1)//first frame finished
		{
			struct gif_status *status = &core->status;
			
			status->first_frame_done = 1;
			GIF_PRINTF("[gif_core_dec]: the single frame done.\n");
		}
	}
       
}
#else
GIF_INLINE void core_buf_free(struct gif_core *core,struct gif_res *res)
{
	int i = 0, j = 0, k = 0;
	UINT32 temp_address = 0;
	UINT32 temp_size = 0;

	GIF_PRINTF("[core_buf_free]: core->buf.start_backup=0x%x.\n", core->buf.start_backup);
	//free dec block
	for(i = 0; i < used_dec_block_cnt; i++)
	{
		if(core->buf.start_backup == gif_used_dec_block[i].address)
		{
			GIF_PRINTF("[core_buf_free]: find the freeing buf in used_dec_block_cnt, i=%d.\n", i);
			
			//handle gif_free_dec_block
			GIF_PRINTF("[core_buf_free]: Then handle gif_free_dec_block.\n");
			core->buf.valid = GIF_FALSE;
			gif_free_dec_block[free_dec_block_cnt].address = gif_used_dec_block[i].address;
			gif_free_dec_block[free_dec_block_cnt].size= gif_used_dec_block[i].size;
			free_dec_block_cnt++;

			GIF_PRINTF("[core_buf_free]: free_dec_block_cnt=%d.\n", free_dec_block_cnt);
			if(free_dec_block_cnt == 2)
			{
				if(gif_free_dec_block[1].address < gif_free_dec_block[0].address)
				{
					GIF_PRINTF("[core_buf_free]: re-queue the 2 free_dec_block.\n");
					temp_address = gif_free_dec_block[0].address;
					temp_size = gif_free_dec_block[0].size;
					gif_free_dec_block[0].address = gif_free_dec_block[1].address;
					gif_free_dec_block[0].size = gif_free_dec_block[1].size;
					gif_free_dec_block[1].address = temp_address;
					gif_free_dec_block[1].size = temp_size;
				}
				if(gif_free_dec_block[0].address + gif_free_dec_block[0].size == gif_free_dec_block[1].address)
				{
					gif_free_dec_block[0].size += gif_free_dec_block[1].size;
					free_dec_block_cnt--;
					GIF_PRINTF("[core_buf_free]: The 2 free_dec_block combine, free_dec_block_cnt=%d.\n", free_dec_block_cnt);
				}
			}
			else
			{
				j=0;
				while(j < free_dec_block_cnt-2)
				{
					if(gif_free_dec_block[j].address < gif_free_dec_block[free_dec_block_cnt-1].address < gif_free_dec_block[j+1].address)
					{
						GIF_PRINTF("[core_buf_free]: This block insert into the queued-blocks.\n");
						
						temp_address = gif_free_dec_block[j+1].address;
						temp_size = gif_free_dec_block[j+1].size;
						gif_free_dec_block[j+1].address = gif_free_dec_block[free_dec_block_cnt-1].address;
						gif_free_dec_block[j+1].size = gif_free_dec_block[free_dec_block_cnt-1].size;

						if((gif_free_dec_block[j].address + gif_free_dec_block[j].size == gif_free_dec_block[j+1].address)
							&& (gif_free_dec_block[j+1].address + gif_free_dec_block[j+1].size == temp_address))
						{
							GIF_PRINTF("[core_buf_free]: 3 blocks combine to 1.\n");
							
							gif_free_dec_block[j].size += gif_free_dec_block[j+1].size + temp_size;
							for(k = j+1; k < free_dec_block_cnt-2; k++)
							{
								gif_free_dec_block[k].address = gif_free_dec_block[k+1].address;
								gif_free_dec_block[k].size = gif_free_dec_block[k+1].size;
							}
							free_dec_block_cnt -= 2;
						}
						else if(gif_free_dec_block[j].address + gif_free_dec_block[j].size == gif_free_dec_block[j+1].address)
						{
							GIF_PRINTF("[core_buf_free]: up 2 blocks combine to 1.\n");

							gif_free_dec_block[j].size += gif_free_dec_block[j+1].size;
							gif_free_dec_block[j+1].address = temp_address;
							gif_free_dec_block[j+1].size = temp_size;
							free_dec_block_cnt--;
						}
						else if(gif_free_dec_block[j+1].address + gif_free_dec_block[j+1].size == temp_address)
						{
							GIF_PRINTF("[core_buf_free]: down 2 blocks combine to 1.\n");

							gif_free_dec_block[j+1].size += temp_size;
							free_dec_block_cnt--;
	}
						else
						{
							GIF_PRINTF("[core_buf_free]: no combine.\n");
							
							for(k = free_dec_block_cnt-1; k > j+2; k--)
							{
								gif_free_dec_block[k].address = gif_free_dec_block[k-1].address;
								gif_free_dec_block[k].size = gif_free_dec_block[k-1].size;
							}
							gif_free_dec_block[j+2].address = temp_address;
							gif_free_dec_block[j+2].size = temp_size;
						}
						break;
					}
					j++;
				}
			}

			//handle gif_used_dec_block
			GIF_PRINTF("[core_buf_free]: Free gif_used_dec_block[%d].address=0x%x, used_dec_block_cnt=%d.\n", i, gif_used_dec_block[i].address, used_dec_block_cnt-1);
			for(j = i; j < used_dec_block_cnt-1; j++)
			{
				gif_used_dec_block[j].address = gif_used_dec_block[j+1].address;
				gif_used_dec_block[j].size = gif_used_dec_block[j+1].size;
			}
			used_dec_block_cnt--;
		}
	}

	//free bit block
	gif_bit_block[core->buf.id].used = 0;
	GIF_PRINTF("[core_buf_free]: Free gif_bit_block[%d].address=0x%x.\n", core->buf.id, gif_bit_block[core->buf.id].address);

	if(core->lzw->img_id == 1)//first frame finished
	{
		struct gif_status *status = &core->status;
		
		status->first_frame_done = 1;
		GIF_PRINTF("[core_buf_free]: the single frame done.\n");
	}
       
}
#endif

GIF_INLINE void slice_generate(struct gif_main *main)
{
	struct gif_list_unit *unit = main->unit;
	struct gif_context *text = &unit->text;
	UINT32 ticks = gif_lzw_cur_ticks(text);
	UINT32 delay = gif_lzw_delay_slice(text);

	if(ticks >= delay)
		unit->slice = 0;
	else
		unit->slice = delay - ticks;

	unit = main->alive.head;
	do
	{
		if(1 != unit->busy)
			break;
		
		if(unit != main->unit)
		{
			text = &unit->text;
			ticks = gif_lzw_cur_ticks(text);
			delay = gif_lzw_delay_slice(text);
			if(ticks >= delay)
				unit->slice = 0;
			else
				unit->slice = delay - ticks;		
		}
		
		unit = unit->next;			
	}while(unit != main->unit);
}

//list operation in the main task
GIF_INLINE UINT8 wait_check(struct gif_main *main)
{
	struct gif_list_unit *punit = NULL;
	UINT8 cnt = 0;

	do
	{
		punit = (struct gif_list_unit *)gif_list_fetch_unit(&main->wait);
		if(NULL == punit)
			break;
		if(gif_list_add_unit(&main->alive,punit))
		{
			cnt++;
		}
	}while(1);
	return cnt;
}

GIF_INLINE UINT8 dead_check(struct gif_main *main)
{
	struct gif_list_unit *punit1 = NULL,*punit2 = NULL;
	UINT8 cnt = 0;

	do
	{
		punit1 = gif_list_fetch_unit(&main->dead);
		if(NULL == punit1)
			break;
		if(gif_list_delete_unit(&main->alive,punit1,&punit2))
		{
			gif_sync_set(main,punit2->magic_num);
			core_buf_free(&(punit2->text.core),(struct gif_res *)main->resource);
                     if(NULL != g_gif_out_exit)
                     {
                        g_gif_out_exit(&punit2->text);
                     }
			cnt++;
		}
	}while(1);
	return cnt;	
}

GIF_INLINE GIF_RET alive_check(struct gif_main *main,UINT32 *idle,UINT32 *time)
{
	struct gif_list_unit *punit = NULL;
	struct gif_list_unit *phead = NULL;
	struct gif_list_unit *min_slice_unit = NULL;

	*idle = *time = 0;
	do
	{
		punit = gif_list_next_unit(&main->alive);
		if(NULL == punit)
		{
			main->unit = punit;
			*idle = 1;
			*time = GIF_FALG_IDLE_TIME;
			return GIF_FALSE;
		}

		if(punit == phead)
		{
			if(0 != *time)
			{
				*idle = 1;
				while(punit != min_slice_unit)
				{
					punit = gif_list_next_unit(&main->alive);
				}
				main->cur = &(punit->text);
				main->unit = punit;
				return GIF_FALSE;
			}
			break;
		}
		if(NULL == phead)
			phead = punit;
		
		if(0 == punit->slice)
			break;
		else
		{
			if(*time > punit->slice || 0 == *time)
			{
				*time = punit->slice;
				min_slice_unit = punit;
			}
		}
	}while(1);
	main->cur = &(punit->text);
	main->unit = punit;
	*idle = *time = 0;
	//GIF_PRINTF("start code decoder %d\n",punit->id);
	return GIF_TRUE;
}

GIF_INLINE GIF_RET main_list_check(struct gif_main *main,UINT32 *idle,UINT32 *time)
{
	UINT8 valid_num = 0;
		
	if(NULL == main)
		return GIF_FALSE;
	
	//check the wait list. If some entries exit, add those into alive list
	if(0 == (valid_num = wait_check(main)))
	{
		//GIF_PRINTF("wait list is empty\n");
	}

	//check the dead list. If some entries exit, delete them in the alive list
	if(0 == (valid_num = dead_check(main)))
	{
		//GIF_PRINTF("check dead list fail\n");
	}
	return alive_check(main,idle,time);
}

UINT32 list_time = 0;

GIF_INLINE void gif_main_task(UINT32 para1,UINT32 para2)
{
	struct gif_main *main = (struct gif_main *)para1;
	GIF_RET list_busy = GIF_FALSE;
	ER s_flag;
	UINT32 ptn_flag;
	UINT32 idle = 0,time = 0;

	while(1)
	{
MAIN_BEGIN:	
		gif_list_enter_mutex();
		list_busy = main_list_check(main,&idle,&time);
		gif_list_exit_mutex();
		if(list_busy)
		{
			gif_core_dec(main);		
			if(gif_core_error(main->cur))
			{
				struct gif_list_unit unit;

				MEMSET((void *)&unit,0,sizeof(struct gif_list_unit));
				unit.magic_num = (UINT32)main->cur->in.file.handle;
				gif_list_enter_mutex();
				gif_list_add_unit(&main->dead,&unit);
				gif_list_exit_mutex();
				GIF_PRINTF("gif error happen file %d \n",unit.magic_num);
				goto MAIN_BEGIN;
			}	
		}
		else
		{
			if(1 == idle)
			{
				//enter the idle status
				GIF_PRINTF("main enter idle %d \n",time);
				//list_time = read_tsc();	
				s_flag = E_FAILURE;
				ptn_flag = 0;
				s_flag = osal_flag_wait(&ptn_flag,main->flag_id,GIF_FLAG_START_PTN,TWF_ANDW,time);
				if(E_OK == s_flag)
					osal_flag_clear(main->flag_id,GIF_FLAG_START_PTN);
			//	list_time = (read_tsc() - list_time)/108000;
			//	libc_printf("List time %d\n",list_time);
			}		
		}	
		if(NULL != main->unit)
			slice_generate(main);
	}
}

GIF_RET gif_main_init(struct gif_instance *gif)
{
	OSAL_ID	tsk_id = OSAL_INVALID_ID;
	OSAL_T_CTSK t_ctsk;
	struct gif_main *main = NULL;
	
	main = MALLOC(sizeof(struct gif_main));
	if(NULL == main)
	{
		GIF_ASSERT(GIF_ERR_MALLOC);
		return GIF_FALSE;
	}
	MEMSET((void *)main,0,sizeof(struct gif_main));
	main->resource = (UINT32)&g_gif_ins_res[gif->id];
	res_init(&g_gif_ins_res[gif->id]);
	gif_list_init(main);	
	gif_sync_alloc(main);
	gif->ins = (UINT32)main;
	g_gif_main[gif->id] = main;
	
	if(NULL == g_gif_out_init || NULL == g_gif_out_enter || NULL == g_gif_out_img)
		gif_out_init_ge();
		
	/*create the main task and flag of the gif decoder*/
	main->flag_id = osal_flag_create(0);
	
	t_ctsk.itskpri = OSAL_PRI_NORMAL;
	t_ctsk.stksz = 0x4000;
	t_ctsk.quantum = 10;
	t_ctsk.para1 = (UINT32)main;
	t_ctsk.para2 = (UINT32)NULL;
	t_ctsk.name[0] = 'G';
	t_ctsk.name[1] = 'F';
	t_ctsk.name[2] = gif->id + 49;
	t_ctsk.task = gif_main_task;
	main->task_id = osal_task_create(&t_ctsk);
	if(OSAL_INVALID_ID == main->task_id || OSAL_INVALID_ID == main->flag_id)
	{
		GIF_PRINTF("create task or flag fail %s \n",__FUNCTION__);
		return GIF_FALSE;
	}
	return GIF_TRUE;
}

void gif_main_free(struct gif_instance *gif)
{
	struct gif_main *main = (struct gif_main *)(gif->ins);

	if(NULL != main)
	{
		osal_task_delete(main->task_id);
		osal_flag_delete(main->flag_id);
		main->flag_id = OSAL_INVALID_ID;
		main->task_id = OSAL_INVALID_ID;
		gif_list_free(main);
		FREE(main);
	}
	gif->ins = 0;
}

//creat core decoder and add it into the wait list
GIF_RET gif_main_start(struct gif_instance *gif)
{
	struct gif_main *main = (struct gif_main *)gif->ins;
	struct gif_dec_par *par = (struct gif_dec_par *)gif->par;
	struct gif_list_unit unit;
	struct gif_context *text = &(unit.text);
	GIF_RET ret = 0;

	MEMSET((void *)&unit,0,sizeof(struct gif_list_unit));
	unit.magic_num = (UINT32)par->in_par.file;
	
	//check whether this file is being decoded or the alive list is full	
	gif_list_enter_mutex();
	ret = gif_list_check_unit(&(main->alive),&unit);
	gif_list_exit_mutex();
	if(GIF_FALSE == ret)
		goto MAIN_START_FAIL;
#ifndef GIF_DYNAMIC_MEMORY
	if(!core_buf_init(&text->core,&g_gif_ins_res[gif->id]))
	{
		gif_core_set_error(text,GIF_ERR_ALLOCATE_CORE_BUF);
	}
#else
	if(!core_buf_init(&text->core,&g_gif_ins_res[gif->id],par))
	{
		gif_core_set_error(text,GIF_ERR_ALLOCATE_CORE_BUF);
	}
#endif
	else
	{
		gif_in_init(text,&par->in_par,&g_gif_ins_res[gif->id]);	
		g_gif_out_init(text,&par->out_par);
		gif_lzw_init(text);
		gif_info_init(text);
		main->info.first_pic = 0;		
	}
	if(!gif_core_error(text))
	{
		ER s_flag;
		
		gif_list_enter_mutex();
		gif_list_add_unit(&main->wait,&unit);
		gif_list_exit_mutex();
		
		s_flag = osal_flag_set(main->flag_id,GIF_FLAG_START_PTN);
		if(E_OK != s_flag)
		{
			GIF_PRINTF("set flag error<%s>\n",__FUNCTION__);
		}
		return GIF_TRUE;
	}
MAIN_START_FAIL:
	GIF_PRINTF("main start fail\n");
	return GIF_FALSE;
}

GIF_RET gif_main_pause(struct gif_instance *gif)
{
	return GIF_FALSE;
}

GIF_RET gif_main_stop(struct gif_instance *gif)
{
	struct gif_main *main = (struct gif_main *)(gif->ins);
	struct gif_list_unit unit;
	gif_file *file = (gif_file *)gif->par;
	ER s_flag;

	MEMSET((void *)&unit,0,sizeof(struct gif_list_unit));
	unit.magic_num = (UINT32)*file;
	gif_list_enter_mutex();
	if(GIF_FALSE == gif_list_check_unit(&main->alive, &unit))
	{
		gif_list_add_unit(&main->dead,&unit);
		gif_list_exit_mutex();
		s_flag = osal_flag_set(main->flag_id,GIF_FLAG_START_PTN);
		if(E_OK != s_flag)
		{
			GIF_PRINTF("set flag error<%s>\n",__FUNCTION__);
		}	
		return gif_sync_wait(main,unit.magic_num,GIF_FLAG_STOP_TIME);
	}
	gif_list_exit_mutex();	

	return GIF_FALSE;
}

GIF_RET gif_check_first_frame(struct gif_instance *gif, gif_file file)
{
	struct gif_main *main = (struct gif_main *)gif->ins;
	struct gif_context *text = main->cur;
	struct gif_core *core = &text->core;
	struct gif_status *status = &core->status;
	
	while(1)
	{
		if(main->cur != NULL)
		{
			if(main->cur->in.file.handle == file)
			{
				GIF_PRINTF("[gif_check_first_frame]: file = 0x%x.\n", file);
				break;
			}
		}
		
		if((main->alive.tail)->magic_num == file)
		{
			GIF_PRINTF("[gif_check_first_frame]: file 0x%x deleted to alive tail up while.\n", file);
			return GIF_TRUE;
		}
		
		osal_task_sleep(50);
	}
	
	while(1)
	{
		if(main->cur->core.status.first_frame_done == 1)
		{
			main->cur->core.status.first_frame_done = 0;
			return GIF_TRUE;
		}
		
		if((main->alive.tail)->magic_num == file)
		{
			GIF_PRINTF("[gif_check_first_frame]: file 0x%x deleted to alive tail down while.\n", file);
			return GIF_TRUE;
		}
		
		osal_task_sleep(50);
	}
}

#ifdef GIF_DYNAMIC_MEMORY
INT16 gif_block_search(struct gif_in *in)
{
	INT16 bytes_in_block = 0;
	UINT32 offset;
	
	gif_in_read_byte(in);
	gif_in_read_byte(in);
	
	bytes_in_block = gif_in_read_byte(in);
	while(bytes_in_block > 0)
	{
		offset = gif_in_file_offset(in);
		offset += (UINT8)bytes_in_block;
		gif_in_file_set_offset(in, offset);

		bytes_in_block = gif_in_read_byte(in);
	}

	return bytes_in_block;
}

GIF_RET gif_check_frame_num(struct gif_dec_par *par)
{
	struct gif_in in;
	INT16 byte;
	UINT16 width, height, left_pos, top_pos;
	UINT8 w_low, w_high, h_low, h_high;
	UINT8 i = 0, back_idx = 0;
	UINT8 frame_num = 0;
	UINT32 bit_buf_start;
	UINT32 bit_buf_size;
	

	for(i = 0; i < gif_bit_block_cnt; i++)
	{
		if(gif_bit_block[i].used == 0)
		{
			bit_buf_start = gif_bit_block[i].address;
			bit_buf_size = gif_bit_block[i].size;
			break;
		}
		if(i == gif_bit_block_cnt-1)
		{
			GIF_PRINTF("[gif_check_frame_num]: gif_get_bit_block failed.\n");
			return GIF_FALSE;
		}
	}
	
	MEMSET((void *)&in,0,sizeof(struct gif_in));
	in.file.handle = par->in_par.file;
	in.file.fread = par->in_par.fread_callback;
	in.file.fseek = par->in_par.fseek_callback;
	in.file.ftell = par->in_par.ftell_callback;
	in.file.offset = 0;
	in.str.start = (UINT8 *)bit_buf_start;
	in.str.read = in.str.start;
	in.str.size = bit_buf_size;
	in.str.left = 0;
	gif_in_file_set_offset(&in,0);

	//1. gif header pass
	for(i=0; i<6; i++)
	{
		gif_in_read_byte(&in);
	}

	//2. gif lsdes for check
	w_low = (UINT8)gif_in_read_byte(&in);
	w_high = (UINT8)gif_in_read_byte(&in);
	h_low = (UINT8)gif_in_read_byte(&in);
	h_high = (UINT8)gif_in_read_byte(&in);

	par->out_par.size.w = w_high<<8 | w_low;
	par->out_par.size.h = h_high<<8 | h_low;
	if(par->out_par.size.w < par->out_par.area.w || par->out_par.size.h < par->out_par.area.h)
	{
		GIF_PRINTF("[gif_check_frame_num]: Attention buffer--because size<area, %d, %d.\n", par->out_par.size.w, par->out_par.size.h);
	}

	byte = gif_in_read_byte(&in);
	while(byte >= 0)
	{
		if((byte == 0) && ((UINT8)gif_in_next_byte(&in) == GIF_IMAGE_SEPARATOR))
		{
			gif_in_read_byte(&in);//GIF_IMAGE_SEPARATOR
			
			//read left pos
			byte = gif_in_read_byte(&in);
			if(byte < 0)
			{
				//file end
				GIF_PRINTF("111, 0x%x.\n", byte);
				goto FILE_END;
				
			}
			if(gif_in_next_byte(&in) < 0)
			{
				//file end
				goto FILE_END;
				
			}
			left_pos = ((UINT8)gif_in_read_byte(&in)<<8) | (UINT8)byte;

			//read top pos
			byte = gif_in_read_byte(&in);
			if(byte < 0)
			{
				//file end
				GIF_PRINTF("222, 0x%x.\n", byte);
				goto FILE_END;
				
			}
			if(gif_in_next_byte(&in) < 0)
			{
				//file end
				goto FILE_END;
				
			}
			top_pos = ((UINT8)gif_in_read_byte(&in)<<8) | (UINT8)byte;
			
			//read width
			byte = gif_in_read_byte(&in);
			if(byte < 0)
			{
				//file end
				GIF_PRINTF("333, 0x%x.\n", byte);
				goto FILE_END;
				
			}
			if(gif_in_next_byte(&in) < 0)
			{
				//file end
				goto FILE_END;
				
			}
			width = ((UINT8)gif_in_read_byte(&in)<<8) | (UINT8)byte;

			if(left_pos + width <= (w_high<<8 | w_low))
			{
				//read height
				byte = gif_in_read_byte(&in);
				if(byte < 0)
				{
					//file end
					GIF_PRINTF("444, 0x%x.\n", byte);
					goto FILE_END;
					
				}
				if(gif_in_next_byte(&in) < 0)
				{
					//file end
					goto FILE_END;
					
				}
				height = ((UINT8)gif_in_read_byte(&in)<<8) | (UINT8)byte;
				
				if(top_pos + height <= (h_high<<8 | h_low))
				{
					frame_num++;
					GIF_PRINTF("[gif_check_frame_num]: find 1 frame, frame_num = %d.\n", frame_num);
					if(frame_num >= 2)
						break;
#if 0
					if(gif_block_search(&in) == 0)
					{
						GIF_PRINTF("[gif_check_frame_num]: this frame data over.\n");
						byte = gif_in_read_byte(&in);
						if((UINT8)byte == GIF_FILE_TRAILER)
							goto FILE_END;
					}
#else
					byte = gif_in_read_byte(&in);
#endif
				}
			}
		}
		else
		{
			byte = gif_in_read_byte(&in);
		}
	}

FILE_END:
	par->in_par.frame_num = frame_num;
	return GIF_TRUE;
}
#endif


