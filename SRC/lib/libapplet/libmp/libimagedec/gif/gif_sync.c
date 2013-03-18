/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *  (C)
 *  File: gif_main.c
 *  (I)
 *  Description:  Control the flag for the sync flow
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

static UINT8 sync_res_init = 0;
static struct gif_sync gif_sync[GIF_MAX_INSTANCE];

GIF_INLINE void sync_init(struct gif_sync *sync)
{
	int i = 0;
	
	for(i = 0;i < GIF_MAX_INSTANCE;i++)
	{
		MEMSET((void *)sync,0,sizeof(struct gif_sync));
		sync++;
	}
}

GIF_RET gif_sync_alloc(struct gif_main *main)
{
	int i = 0,j = 0;
	
	if(0 == sync_res_init)
	{
		sync_res_init = 1;
		sync_init(gif_sync);
	}
	
	for(i = 0;i < GIF_MAX_INSTANCE;i++)
	{
		if(0 == gif_sync[i].busy)
		{
			gif_sync[i].busy = 1;
			main->sync_id = i;
			MEMSET((void *)(gif_sync + i),0,sizeof(struct gif_sync));
			return GIF_TRUE;
		}
	}
	return GIF_FALSE;
}

void gif_sync_free(struct gif_main *main)
{
	if(main->sync_id < GIF_MAX_INSTANCE)
		gif_sync[main->sync_id].busy = 0;
}

GIF_RET gif_sync_wait(struct gif_main *main,UINT32 magic_num,UINT32 time_out)
{
	struct gif_sync *sync = gif_sync + main->sync_id;
	struct gif_sync_ptns *ptns = sync->ptns;
	ER s_flag;
	UINT32 ptn_flag,PTN;
	int i = 0;
	
	for(i = 0;i < GIF_FLAG_SYNC_MAX_NUM;i++,ptns++)
	{
		if(0 == ptns->busy)
		{
			break;
		}
	}
	
	if(i < GIF_FLAG_SYNC_MAX_NUM)
	{
		ptns->busy = 1;
		ptns->magic_num = magic_num;
		PTN = GIF_FLAG_SYNC_PTNS<<i;	
		s_flag = E_FAILURE;
		ptn_flag = 0;
		GIF_PRINTF("wait start... magic num %d \n",magic_num);
		s_flag = osal_flag_wait(&ptn_flag,(OSAL_ID)main->flag_id,PTN,TWF_ANDW,time_out);
		ptns->busy = 0;
		if(E_OK == s_flag)
		{
			GIF_PRINTF("wait ok ptn %x magic num %d\n",PTN,magic_num);
			osal_flag_clear(main->flag_id,PTN);
			return GIF_TRUE;
		}
		else
			GIF_PRINTF("wait fail flag %d magic num %d\n",s_flag,magic_num);
	}
	return GIF_FALSE;
}

void gif_sync_set(struct gif_main *main,UINT32 magic_num)
{
	struct gif_sync *sync = gif_sync + main->sync_id;
	struct gif_sync_ptns *ptns = sync->ptns;	
	UINT32 PTN = 0;
	ER s_flag;
	int i = 0;

	for(i = 0;i < GIF_FLAG_SYNC_MAX_NUM;i++,ptns++)
	{
		if(1 == ptns->busy && magic_num == ptns->magic_num)
		{
			GIF_PRINTF("Set the flag %d\n",i);
			break;
		}
	}

	if(i < GIF_FLAG_SYNC_MAX_NUM)
	{
		PTN = GIF_FLAG_SYNC_PTNS<<i;		
		s_flag = osal_flag_set((OSAL_ID)main->flag_id,PTN);
		GIF_PRINTF("set flag %d <%s> PTN %x magic num %d\n",s_flag,__FUNCTION__,PTN,magic_num);
	}
}

