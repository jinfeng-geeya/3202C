/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *  (C)
 *  File: gif.c
 *  (I)
 *  Description: Entry of the gif decoder library.It contains all the APIs
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2008.1.16			Sam			Create
 * 1. 	2008.9.12			Sam			Release the first version. Only support ARGB1555
 * 2.        2008.9.17                    Sam			Support Clut8 and AYCBCR8888 format OSD
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

//global variables
struct gif_res g_gif_ins_res[GIF_MAX_INSTANCE];

UINT8 g_gif_dec_first_pic = 0;
UINT8 g_gif_hw_support_rgb = 0;
int g_gif_ignore_bk_color = 0;
UINT32 g_gif_trans_color = 0;
int g_gif_trans_color_flag = 0;
int g_gif_force_show_pic = 0;

GIF_RET (*g_gif_out_init)(struct gif_context *,struct gif_dec_out_par *);
GIF_RET (*g_gif_out_enter)(struct gif_out *,struct gif_lzw *);
void (*g_gif_out_img)(struct gif_context *);
GIF_RET (*g_gif_out_exit)(struct gif_context *);

GIF_RET gif_init(struct gif_cfg *init)
{
	UINT8 *bit = (UINT8 *)((UINT32)(init->bit_buf_start) & ~(7<<28));
	UINT8 *dec = (UINT8 *)((UINT32)(init->dec_buf_start) & ~(7<<28));
	UINT32 bit_size = init->bit_buf_size;
	UINT32 dec_size = init->dec_buf_size;
	UINT8 id = 0;

	GIF_ENTER;
	g_gif_dec_first_pic = GIF_FALSE;
	g_gif_hw_support_rgb = GIF_FALSE;
	g_gif_out_init = NULL;
	g_gif_out_enter = NULL;
	g_gif_out_img = NULL;
       g_gif_out_exit = NULL;
	MEMSET((void *)&g_gif_ins_res,GIF_FALSE,GIF_MAX_INSTANCE * sizeof(struct gif_res));
	for(id = 0;id < GIF_MAX_INSTANCE;id++)
	{
		g_gif_ins_res[id].id = id;
		g_gif_ins_res[id].active = GIF_FALSE;
		g_gif_ins_res[id].par.bit_buf_start = bit;
		g_gif_ins_res[id].par.dec_buf_start = dec;
		g_gif_ins_res[id].par.max_width = init->max_width;
		g_gif_ins_res[id].par.max_height = init->max_height;
		g_gif_ins_res[id].par.only_dec_first_pic = init->only_dec_first_pic;
		if((bit_size >= GIF_INS_BIT_BUF_SIZE) && (dec_size >= GIF_INS_DEC_BUF_SIZE))
		{
			MEMCPY((void *)&(g_gif_ins_res[id].par),(void *)init,sizeof(struct gif_cfg));
			g_gif_ins_res[id].par.bit_buf_size = GIF_INS_BIT_BUF_SIZE;
			g_gif_ins_res[id].par.dec_buf_size = GIF_INS_DEC_BUF_SIZE;		
			bit += GIF_INS_BIT_BUF_SIZE;
			dec += GIF_INS_DEC_BUF_SIZE;
			bit_size -= GIF_INS_BIT_BUF_SIZE;
			dec_size -= GIF_INS_DEC_BUF_SIZE;
		}
		else
		{
			g_gif_ins_res[id].par.bit_buf_size = bit_size;
			g_gif_ins_res[id].par.dec_buf_size = dec_size;				
			GIF_PRINTF("no enough memory allocated for gif decoder\n");
			return GIF_FALSE;
		}
	}
	
	GIF_EXIT;
	return GIF_TRUE;	
}

struct gif_instance *gif_open(void)
{
	struct gif_instance *gif = NULL;
	UINT8 id = 0;

	GIF_ENTER;
	gif = MALLOC(sizeof(struct gif_instance));
	if(NULL == gif)
	{
		GIF_ASSERT(GIF_ERR_MALLOC);
		goto CRE_FAIL;
	}
	MEMSET((void *)gif,0,sizeof(struct gif_instance));
	for(id = 0;id < GIF_MAX_INSTANCE;id++)
	{
		if(!g_gif_ins_res[id].active)
		{
			g_gif_ins_res[id].active = GIF_TRUE;
			break;
		}
	}
	if(id >= GIF_MAX_INSTANCE)
		goto CRE_FAIL;
	gif->id = id;
	if(gif_main_init(gif))
	{
		GIF_EXIT;
		return gif;
	}
CRE_FAIL:		
	GIF_PRINTF("open gif instance fail\n");
	return NULL;
}

GIF_RET gif_close(struct gif_instance *gif)
{
	GIF_ENTER;
	if(NULL != gif)
	{
		if(NULL != (void *)gif->ins)
		{
			g_gif_ins_res[gif->id].active = GIF_FALSE;
			gif_main_free(gif);
		}
		FREE(gif);
	}
	GIF_EXIT;
	return GIF_TRUE;
}

GIF_RET gif_dec(struct  gif_instance *gif,struct gif_dec_par *par)
{
	GIF_ENTER;
	if(NULL != par && NULL != gif)
	{
		gif->par = (UINT32)par;
		if(gif_main_start(gif))
		{
			GIF_EXIT;
			return GIF_TRUE;
		}
	}
	GIF_PRINTF("decode gif fail\n");
	return GIF_FALSE;
}

GIF_RET gif_stop(struct gif_instance *gif,gif_file file)
{
	GIF_ENTER;
	if(NULL != gif)
	{	
		gif->par = (UINT32)&file;
		if(gif_main_stop(gif))
		{
			GIF_EXIT;
			return GIF_TRUE;
		}
	}
	GIF_PRINTF("stop gif fail\n");
	return GIF_FALSE;
}

GIF_RET gif_io_ctrl(struct gif_instance *gif,UINT32 io_cmd,UINT32 io_par)
{
	switch(io_cmd)
	{
		case GIF_IO_CMD_DEC_FIRST_PIC:
			if(GIF_FALSE == io_par)
				g_gif_dec_first_pic = GIF_FALSE;
			else 
				g_gif_dec_first_pic = GIF_TRUE;
			
			break;
		case GIF_IO_CMD_IGNORE_BACKGROUND_COLOR:
			if(GIF_FALSE == io_par)
				g_gif_ignore_bk_color = GIF_FALSE;
			else
				g_gif_ignore_bk_color = GIF_TRUE;
			break;
		case GIF_IO_CMD_GET_STATUS_INFO:
			{
				struct gif_status_info *pinfo = (struct gif_status_info *)io_par;
				
				pinfo->first_pic = ((struct gif_main *)(gif->ins))->info.first_pic;
				break;
			}
		case GIF_IO_CMD_SET_TRANS_COLOR:
			{
				g_gif_trans_color = io_par;
				g_gif_trans_color_flag = 1;
				break;
			}
		case GIF_IO_CMD_FORCE_SHOW_PIC:
			g_gif_force_show_pic = 1;
			break;
		default:
			return GIF_TRUE;
	}
	
	return GIF_TRUE;
}

