/* $Id: */
/******************************************************************************
*
*       ALI IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS"
*       AS A COURTESY TO YOU, SOLELY FOR USE IN DEVELOPING PROGRAMS AND
*       SOLUTIONS FOR ALI DEVICES.  BY PROVIDING THIS DESIGN, CODE,
*       OR INFORMATION AS ONE POSSIBLE IMPLEMENTATION OF THIS FEATURE,
*       APPLICATION OR STANDARD, ALI IS MAKING NO REPRESENTATION
*       THAT THIS IMPLEMENTATION IS FREE FROM ANY CLAIMS OF INFRINGEMENT,
*       AND YOU ARE RESPONSIBLE FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE
*       FOR YOUR IMPLEMENTATION.  ALI EXPRESSLY DISCLAIMS ANY
*       WARRANTY WHATSOEVER WITH RESPECT TO THE ADEQUACY OF THE
*       IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO ANY WARRANTIES OR
*       REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE FROM CLAIMS OF
*       INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*       FOR A PARTICULAR PURPOSE.
*
*       (c) Copyright 2008-2009 ALI Inc.
*       All rights reserved.
*
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file ipanel_demux.c
 *
 * Implements porting layer for ipanel STB middle layer running on top of ALI
 * chips.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00a joy  08/10/11 First release
 * </pre>
 *
 ******************************************************************************/

/***************************** Include Files *********************************/

#include <sys_config.h>
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
//#include <hal/machine.h>
#include <hld/hld_dev.h>
#include <bus/sci/sci.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <udi/ipanel/ipanel_porting_event.h>
#include <udi/ipanel/ipanel_demux.h>



#define IPANEL_DMX_TOTAL_FILTERS    27

#define IPANEL_DMX_TOTAL_CHANNELS   32

#define IPANEL_DMX_MASK_MAX_LEN     32

#define IPANEL_DMX_MASK_MAX_NUM     32

enum ipanel_dmx_mask_status
{
    IPANEL_DMX_MASK_STATUS_IDLE,
   
    IPANEL_DMX_MASK_STATUS_READY,

    IPANEL_DMX_MASK_STATUS_RUN
};

enum ipanel_dmx_channel_status
{
    IPANEL_DMX_CHANNEL_STATUS_IDLE,
   
    IPANEL_DMX_CHANNEL_STATUS_READY,

    IPANEL_DMX_CHANNEL_STATUS_RUN
};

enum ipanel_dmx_filter_status
{
    IPANEL_DMX_FILTER_STATUS_IDLE,
   
    IPANEL_DMX_FILTER_STATUS_READY,

    IPANEL_DMX_FILTER_STATUS_RUN
};



struct ipanel_sec_param
{
    struct get_section_param ali_sec_param;

    UINT32 ch_idx;

    UINT8 mask_hit_map[IPANEL_DMX_MASK_MAX_NUM];
};

struct ipanel_dmx_mask
{
    enum ipanel_dmx_mask_status status;

    UINT8 *mask;
   
    UINT8 *coef;

    UINT8 *enable;
};


struct ipanel_dmx_filter
{
    enum ipanel_dmx_filter_status status;

    UINT32 ref_cnt;

    UINT8 ali_filter_idx;
 
    struct get_section_param ali_sec_param;

    struct ipanel_dmx_mask mask[IPANEL_DMX_MASK_MAX_NUM];
};

struct ipanel_dmx_channel
{
    enum ipanel_dmx_channel_status status;

    UINT16 pid;

    UINT32 ipanle_filter_idx;

    UINT8 mask_en_map[IPANEL_DMX_MASK_MAX_NUM];
};

struct ipanel_dmx_channel g_ipanel_dmx_channel[IPANEL_DMX_TOTAL_CHANNELS];
struct ipanel_dmx_filter  g_ipanel_dmx_filter[IPANEL_DMX_TOTAL_FILTERS];

UINT8 *g_ipanle_dmx_buf    =  (UINT8 *)DMX_PORTING_BUF_START;
UINT32 g_ipanle_dmx_buf_wr = 0;
UINT32 g_ipanle_dmx_buf_rd = 0;

NOTIFY_FUNC ipanel_demux_section_notify = NULL;

ID g_ipanel_sec_dispatch_task = INVALID_ID;
BOOL g_ipanel_sec_dispatch_task_running = FALSE;

static void ali_get_section_callback
(
    struct get_section_param *param
)
{
    UINT8                    *sec_len;
    UINT8                    *ch_idx;
    UINT8                    *mask_hit_map;
    UINT32                    mask_idx;
    UINT8                    *pid;
    struct get_section_param *ali_sec_param;
	UINT32                    free_block_len;

    ali_sec_param = (struct get_section_param *)param;

    /* Last 0x1000 + 4 + IPANEL_DMX_MASK_MAX_NUM bytes in buffer is used as runback 
     * zone.
	 */
	if ((g_ipanle_dmx_buf_wr + 0x1000 + 4 + IPANEL_DMX_MASK_MAX_NUM) > DMX_PORTING_BUF_SIZE)
	{
		//libc_printf("%s, %d->0\n", __FUNCTION__, g_ipanle_dmx_buf_wr);

        g_ipanle_dmx_buf_wr = 0;
	}

	if (g_ipanle_dmx_buf_wr < g_ipanle_dmx_buf_rd)
	{
        free_block_len = g_ipanle_dmx_buf_rd - g_ipanle_dmx_buf_wr;
	}
    else
    {
        free_block_len = DMX_PORTING_BUF_SIZE - g_ipanle_dmx_buf_wr;
	}

	if (free_block_len < ali_sec_param->sec_tbl_len)
	{
        //libc_printf("%s, buffer overflow, rd:%d, wr:%d, sec len:%d\n",
			        //__FUNCTION__, g_ipanle_dmx_buf_rd, g_ipanle_dmx_buf_wr,
			        //ali_sec_param->sec_tbl_len);

        return;
	}

    sec_len = (UINT8 *)(&ali_sec_param->sec_tbl_len);
    g_ipanle_dmx_buf[g_ipanle_dmx_buf_wr + 0] = sec_len[1];
    g_ipanle_dmx_buf[g_ipanle_dmx_buf_wr + 1] = sec_len[0];

    pid = (UINT8 *)(&ali_sec_param->pid);
    g_ipanle_dmx_buf[g_ipanle_dmx_buf_wr + 2] = pid[1];
    g_ipanle_dmx_buf[g_ipanle_dmx_buf_wr + 3] = pid[0];

    mask_hit_map = ((struct restrict_ext *)(ali_sec_param->mask_value))->multi_mask_hit_ext;

    for (mask_idx = 0; mask_idx < IPANEL_DMX_MASK_MAX_NUM; mask_idx++)
    {
        g_ipanle_dmx_buf[g_ipanle_dmx_buf_wr + 4 + mask_idx] = mask_hit_map[mask_idx];
    }

    MEMCPY(&(g_ipanle_dmx_buf[g_ipanle_dmx_buf_wr + 4 + IPANEL_DMX_MASK_MAX_NUM]), 
           ali_sec_param->buff, ali_sec_param->sec_tbl_len);

    g_ipanle_dmx_buf_wr += (4 + IPANEL_DMX_MASK_MAX_NUM + ali_sec_param->sec_tbl_len);

#if 0
    if ((DMX_PORTING_BUF_SIZE - g_ipanle_dmx_buf_wr) < (0x1000 + 4 + IPANEL_DMX_MASK_MAX_NUM))
    {
        g_ipanle_dmx_buf_wr = 0;
    }
#endif

    return;
}



void ipanel_sec_dispatch_task(UINT32 para1,UINT32 para2)
{
    UINT8   hit;
    UINT8  *sec_data_addr;
    UINT32  ch_idx;
    UINT16  sec_len;
    UINT32  flt_idx;
    UINT32  mask_idx;

    UINT8  *p_pid;
    UINT8  *p_sec_len;

    UINT16 pid;

    struct ipanel_dmx_channel *channel;

    p_pid = (UINT8 *)(&pid);

    p_sec_len = (UINT8 *)(&sec_len);

    while (g_ipanel_sec_dispatch_task_running)
    { 
        if (NULL == ipanel_demux_section_notify)
        {
            continue;
        }  
    
        while (g_ipanle_dmx_buf_rd != g_ipanle_dmx_buf_wr && g_ipanel_sec_dispatch_task_running == TRUE)
        {
            /* Last 0x1000 + 4 + IPANEL_DMX_MASK_MAX_NUM bytes in buffer is used as runback 
             * zone.
        	 */
	        if ((g_ipanle_dmx_buf_rd + 0x1000 + 4 + IPANEL_DMX_MASK_MAX_NUM) >
                DMX_PORTING_BUF_SIZE)
        	{
				//libc_printf("%s, %d->0\n", __FUNCTION__, g_ipanle_dmx_buf_rd);

                g_ipanle_dmx_buf_rd = 0;
        	}

            p_pid[0] = g_ipanle_dmx_buf[g_ipanle_dmx_buf_rd + 3];
            p_pid[1] = g_ipanle_dmx_buf[g_ipanle_dmx_buf_rd + 2];

            p_sec_len[0] = g_ipanle_dmx_buf[g_ipanle_dmx_buf_rd + 1];
            p_sec_len[1] = g_ipanle_dmx_buf[g_ipanle_dmx_buf_rd + 0];

            sec_data_addr = &(g_ipanle_dmx_buf[g_ipanle_dmx_buf_rd + 4 + 
                              IPANEL_DMX_MASK_MAX_NUM]);

            for (ch_idx = 0; ch_idx < IPANEL_DMX_TOTAL_CHANNELS; ch_idx++)
            {
                channel = &(g_ipanel_dmx_channel[ch_idx]);

                if (!((channel->status == IPANEL_DMX_CHANNEL_STATUS_RUN) && 
                     (channel->pid == pid)))
                {
                    continue;
                }

                for (mask_idx = 0; mask_idx < IPANEL_DMX_MASK_MAX_NUM; mask_idx++)
                {
                    hit = g_ipanle_dmx_buf[g_ipanle_dmx_buf_rd + 4 + mask_idx];
 
                    if (0 != (channel->mask_en_map[mask_idx] & hit))
                    //if (0 != hit)
                    {
                        flt_idx = (ch_idx << 16) | mask_idx;
    
                        //libc_printf(">, ch:0x%x, flt:0x%x, pid:%d, len:%d\n", ch_idx, flt_idx, pid, sec_len);

    					ipanel_demux_section_notify(ch_idx, flt_idx, 
                                                    (UINT32)sec_data_addr, sec_len);
                    }
                 }   
            }
                
            g_ipanle_dmx_buf_rd += (4 + IPANEL_DMX_MASK_MAX_NUM + sec_len);

#if 0
            if ((DMX_PORTING_BUF_SIZE - g_ipanle_dmx_buf_rd) < (0x1000 + 4 + IPANEL_DMX_MASK_MAX_NUM))
            {
                g_ipanle_dmx_buf_rd = 0;
            }
#endif
        }

		osal_task_sleep(50);
    }

	g_ipanel_sec_dispatch_task =OSAL_INVALID_ID;
	osal_task_exit(0);
}

INT32 ipanel_porting_set_demux_notify(NOTIFY_FUNC func)
{
	ipanel_demux_section_notify = func;
	return 0;
}


UINT32 ipanel_porting_create_demux_channel
(
    INT32  pool_size,
    UINT16 pid
)
{

    UINT32                     ch_idx;
    UINT32                     flt_idx;
    struct ipanel_dmx_channel *channel;
    struct ipanel_dmx_filter  *filter;
    RET_CODE           ret;
	struct dmx_device *dmx_dev;

    //libc_printf("%s, pid:%d", __FUNCTION__, pid);

    for (ch_idx = 0; ch_idx < IPANEL_DMX_TOTAL_CHANNELS; ch_idx++)
    {
        channel = &(g_ipanel_dmx_channel[ch_idx]);

        if (IPANEL_DMX_CHANNEL_STATUS_IDLE == channel->status)
        {
		    break;
        }
    }    

    if (ch_idx >= IPANEL_DMX_TOTAL_CHANNELS)
    {
        //libc_printf("%s, no idle channel!\n", __FUNCTION__);
    
        return(0xFFFFFFFF);
    }

    channel->status = IPANEL_DMX_CHANNEL_STATUS_READY;

    for (flt_idx = 0; flt_idx < IPANEL_DMX_TOTAL_FILTERS; flt_idx++)
    {
        filter = &(g_ipanel_dmx_filter[flt_idx]);

        if ((IPANEL_DMX_FILTER_STATUS_IDLE != filter->status) &&
            (filter->ali_sec_param.pid == pid))
        {
            break;
        }
    }

    if (flt_idx >= IPANEL_DMX_TOTAL_FILTERS)
    {
        for (flt_idx = 0; flt_idx < IPANEL_DMX_TOTAL_FILTERS; flt_idx++)
        {
            filter = &(g_ipanel_dmx_filter[flt_idx]);
    
            if (IPANEL_DMX_FILTER_STATUS_IDLE == filter->status)
            {
            	dmx_dev = (struct dmx_device*)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
            
                if (NULL == dmx_dev)
                {
                    //libc_printf("%s, dev_get_by_id() failed!\n", __FUNCTION__, channel->pid);
            		
                    return(-1);
                }
    
                filter->ali_sec_param.pid = pid;

                ret = dmx_async_req_section(dmx_dev, &(filter->ali_sec_param),
                                            &(filter->ali_filter_idx));
            
                if (RET_SUCCESS != ret)
                {
                    //libc_printf("%s, pid:%d dmx_async_req_section() failed!\n", __FUNCTION__, channel->pid);
            
            	    return(-1);
                }

                filter->status = IPANEL_DMX_FILTER_STATUS_RUN;
    
                break;
            }
        }
    }

    if (flt_idx >= IPANEL_DMX_TOTAL_FILTERS)
    {
        //libc_printf("%s, no idle filter!\n", __FUNCTION__);
    
        return(0xFFFFFFFF);
    }

    channel->ipanle_filter_idx = flt_idx;

    //channel->status = IPANEL_DMX_CHANNEL_STATUS_READY;

    channel->pid = pid;

    //libc_printf(" ch_idx:%d\n", ch_idx);

    return(ch_idx);
}





INT32 ipanel_porting_start_demux_channel(UINT32 ch_idx)
{
    UINT32             flt_idx;
    RET_CODE           ret;
	struct dmx_device *dmx_dev;
    struct ipanel_dmx_channel *channel;
    struct ipanel_dmx_filter  *filter;

    //libc_printf("%s, ch_idx:%d\n", __FUNCTION__, ch_idx);

    channel = &(g_ipanel_dmx_channel[ch_idx]);

	dmx_dev = (struct dmx_device*)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

    if (NULL == dmx_dev)
    {
        //libc_printf("%s, dev_get_by_id() failed!\n", __FUNCTION__, channel->pid);

        return(-1);
    }

    if (IPANEL_DMX_CHANNEL_STATUS_READY != channel->status)
    {
        //libc_printf("%s, pid:%d is already running!\n", __FUNCTION__, channel->pid);

	    return(-1);
    }

    channel->status = IPANEL_DMX_CHANNEL_STATUS_RUN;

    return(0);
}




INT32 ipanel_porting_stop_demux_channel(UINT32 ch_idx)
{
    struct ipanel_dmx_channel *channel;

    struct ipanel_dmx_filter  *filter;

    //libc_printf("%s, ch_idx:%d\n", __FUNCTION__, ch_idx);

    channel = &(g_ipanel_dmx_channel[ch_idx]);



    if (IPANEL_DMX_CHANNEL_STATUS_RUN != channel->status)
    {
        //libc_printf("%s, pid:%d is not running!\n", __FUNCTION__, channel->pid);

	    return(-1);
    }

    channel->status = IPANEL_DMX_CHANNEL_STATUS_READY;

    return(0);
}




INT32 ipanel_porting_destroy_demux_channel(UINT32 ch_idx)
{
    UINT32 flt_idx;
    RET_CODE           ret;

    struct ipanel_dmx_channel *channel;
    struct ipanel_dmx_filter  *filter;
    struct ipanel_dmx_channel *cmp_channel;
    UINT32 cmp_ch_idx;
	struct dmx_device *dmx_dev;

    //libc_printf("%s, ch_idx:%d\n", __FUNCTION__, ch_idx);

    channel = &(g_ipanel_dmx_channel[ch_idx]);

    if (IPANEL_DMX_CHANNEL_STATUS_READY != channel->status)
    {
        //libc_printf("%s, pid:%d is not READY!\n", __FUNCTION__, channel->pid);

	    return(-1);
    }

	dmx_dev = (struct dmx_device*)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

    if (NULL == dmx_dev)
    {
        //libc_printf("%s, dev_get_by_id() failed!\n", __FUNCTION__, channel->pid);

        return(-1);
    }
    for (cmp_ch_idx = 0; cmp_ch_idx < IPANEL_DMX_TOTAL_CHANNELS; cmp_ch_idx++)
    {
        if (ch_idx == cmp_ch_idx)
        {
            continue;
        }

        cmp_channel = &(g_ipanel_dmx_channel[cmp_ch_idx]);

        if ((IPANEL_DMX_CHANNEL_STATUS_IDLE != cmp_channel->status) &&
            (channel->ipanle_filter_idx == cmp_channel->ipanle_filter_idx))
        {
		    break;
        }
    }

    filter = &(g_ipanel_dmx_filter[channel->ipanle_filter_idx]);

    if ((cmp_ch_idx >= IPANEL_DMX_TOTAL_CHANNELS) &&
        (IPANEL_DMX_FILTER_STATUS_RUN == filter->status))
    {
        ret = dmx_io_control(dmx_dev, IO_ASYNC_CLOSE, filter->ali_filter_idx);
    
        if (RET_SUCCESS != ret)
        {
            //libc_printf("%s, pid:%d IO_ASYNC_CLOSE() failed!\n", __FUNCTION__, channel->pid);
    
    	    return(-1);
        }

        filter->status = IPANEL_DMX_FILTER_STATUS_IDLE;
    }

    g_ipanel_dmx_channel[ch_idx].status = IPANEL_DMX_CHANNEL_STATUS_IDLE;

    return(0);
}




UINT32 ipanel_porting_create_demux_filter
(
    UINT32  ch_idx, 
    UINT32  wide,
    UINT8  *coef,
    UINT8  *mask,
    UINT8  *excl
)
{

    UINT32                     mask_idx;
    UINT32                     byte_idx;
    UINT32                     ret_flt_idx;
    struct ipanel_dmx_channel *channel;
    struct ipanel_dmx_filter  *filter;
    struct ipanel_dmx_mask    *flt_mask;

    //libc_printf("%s, ch_idx:%d", __FUNCTION__, ch_idx);


    if (wide > IPANEL_DMX_MASK_MAX_LEN)
    {
        //libc_printf("%s, wide:%d too long!\n", __FUNCTION__, wide);

	    return(0xFFFFFFFF);    
    }

    channel = &(g_ipanel_dmx_channel[ch_idx]);

    if (IPANEL_DMX_CHANNEL_STATUS_IDLE == channel->status)
    {
        //libc_printf("%s, chanel %d is IDLE!\n", __FUNCTION__, channel);

	    return(0xFFFFFFFF);    
    }

    filter = &(g_ipanel_dmx_filter[channel->ipanle_filter_idx]);

    for (mask_idx = 0; mask_idx < IPANEL_DMX_MASK_MAX_NUM; mask_idx++)
    {
        flt_mask = &(filter->mask[mask_idx]);

        if (IPANEL_DMX_MASK_STATUS_IDLE == flt_mask->status)
        {
            MEMSET(flt_mask->mask, 0, IPANEL_DMX_MASK_MAX_LEN);

            MEMSET(flt_mask->coef, 0, IPANEL_DMX_MASK_MAX_LEN);

			for(byte_idx = 0; byte_idx < wide; byte_idx++)
            {
                if (0 == byte_idx)
                {
                    flt_mask->mask[byte_idx] = mask[byte_idx];
                    flt_mask->coef[byte_idx] = coef[byte_idx];
                }
                else
                {
                    flt_mask->mask[byte_idx + 2] = mask[byte_idx];
                    flt_mask->coef[byte_idx + 2] = coef[byte_idx];
                }
			}

            flt_mask->status = IPANEL_DMX_MASK_STATUS_READY;

            ret_flt_idx = (ch_idx << 16) | mask_idx;

            //libc_printf(" flt_idx:0x%x\n", ret_flt_idx);

		    return(ret_flt_idx);
        }
    }

    //libc_printf("%s, chanel %d has no idle filter!\n", __FUNCTION__, channel);

	return(0xFFFFFFFF);    
}


INT32 ipanel_porting_enable_demux_filter(UINT32 ipanle_flt_idx)
{
    UINT32 ch_idx;
    UINT32 mask_idx;
    struct ipanel_dmx_channel *channel;
    struct ipanel_dmx_filter  *filter;
    struct ipanel_dmx_mask    *mask;    

    ch_idx = (ipanle_flt_idx & 0xFFFF0000) >> 16;

    mask_idx = ipanle_flt_idx & 0x0000FFFF;

    //libc_printf("%s, ch_idx:%d, mask_idx:%d\n", __FUNCTION__, ch_idx, mask_idx);

    channel = &(g_ipanel_dmx_channel[ch_idx]);

    filter = &(g_ipanel_dmx_filter[channel->ipanle_filter_idx]);

    mask = &(filter->mask[mask_idx]);

    if (IPANEL_DMX_MASK_STATUS_READY != mask->status)
    {
        //libc_printf("%s, mask %d is not ready!\n", __FUNCTION__, mask_idx);

	    return(-1);
    }

    channel->mask_en_map[mask_idx] = 1;
    
    mask->status = IPANEL_DMX_MASK_STATUS_RUN;

    *(mask->enable) = 1;

    return(0);   
}





INT32 ipanel_porting_disable_demux_filter(UINT32 ipanle_flt_idx)
{
    UINT32 ch_idx;
    UINT32 mask_idx;
    struct ipanel_dmx_channel *channel;
    struct ipanel_dmx_filter  *filter;
    struct ipanel_dmx_mask    *mask;

    ch_idx = (ipanle_flt_idx & 0xFFFF0000) >> 16;

    mask_idx = ipanle_flt_idx & 0x0000FFFF;

    //libc_printf("%s, ch_idx:%d, mask_idx:%d\n", __FUNCTION__, ch_idx, mask_idx);

    channel = &(g_ipanel_dmx_channel[ch_idx]);

    filter = &(g_ipanel_dmx_filter[channel->ipanle_filter_idx]);

    mask = &(filter->mask[mask_idx]);

    if (IPANEL_DMX_MASK_STATUS_RUN != mask->status)
    {
        //libc_printf("%s, mask %d is not running!\n", __FUNCTION__, mask_idx);

	    return(-1);
    }

    channel->mask_en_map[mask_idx] = 0;

    mask->status = IPANEL_DMX_MASK_STATUS_READY;

    *(mask->enable) = 0;

    return(0);   
}


INT32 ipanel_porting_destroy_demux_filter(UINT32 ipanle_flt_idx)
{
    UINT32 ch_idx;
    UINT32 mask_idx;
    struct ipanel_dmx_channel *channel;
    struct ipanel_dmx_filter  *filter;
    struct ipanel_dmx_mask    *mask;

    ch_idx = (ipanle_flt_idx & 0xFFFF0000) >> 16;

    mask_idx = ipanle_flt_idx & 0x0000FFFF;

    //libc_printf("%s, ch_idx:%d, mask_idx:%d\n", __FUNCTION__, ch_idx, mask_idx);

    channel = &(g_ipanel_dmx_channel[ch_idx]);

    filter = &(g_ipanel_dmx_filter[channel->ipanle_filter_idx]);

    mask = &(filter->mask[mask_idx]);

    if (IPANEL_DMX_MASK_STATUS_READY != mask->status)
    {
        //libc_printf("%s, mask %d is not stopped!\n", __FUNCTION__, mask_idx);

	    return(-1);
    }

    mask->status = IPANEL_DMX_MASK_STATUS_IDLE;

    channel->mask_en_map[mask_idx] = 0;

    *(mask->enable) = 0;

    return(0); 
}


void ipanel_porting_dmx_init(void)
{

    struct get_section_param *ali_sec_param;
    struct restrict_ext      *ali_sec_restrict;
    UINT8                    *mask;
    UINT8                    *coef;
    UINT8                    *mask_en;
    UINT8                    *mask_hit;
	T_CTSK                    tsk_param;
    struct ipanel_dmx_channel *channel;
    UINT32 ch_idx;
    UINT32 flt_idx;
    UINT32 mask_idx;
    UINT32 malloc_len = 0;
    UINT8 *malloc_buffer = NULL;

    struct ipanel_dmx_filter  *filter;


    for (ch_idx = 0; ch_idx < IPANEL_DMX_TOTAL_CHANNELS; ch_idx++)
    {
        channel = &(g_ipanel_dmx_channel[ch_idx]);

        channel->status = IPANEL_DMX_CHANNEL_STATUS_IDLE;
    }

    malloc_len = sizeof(struct restrict_ext) + \
        2 * IPANEL_DMX_MASK_MAX_NUM * IPANEL_DMX_MASK_MAX_LEN + \
        2 * IPANEL_DMX_MASK_MAX_NUM + 4096;

    malloc_buffer = MALLOC(IPANEL_DMX_TOTAL_FILTERS * malloc_len);
    ASSERT(malloc_buffer != NULL);

    MEMSET(malloc_buffer, 0, IPANEL_DMX_TOTAL_FILTERS * malloc_len);

    for (flt_idx = 0; flt_idx < IPANEL_DMX_TOTAL_FILTERS; flt_idx++)
    {
        filter = &(g_ipanel_dmx_filter[flt_idx]);

        filter->status = IPANEL_DMX_FILTER_STATUS_IDLE;

        ali_sec_param = &(filter->ali_sec_param);

    	MEMSET(ali_sec_param, 0, sizeof(struct get_section_param));

        ali_sec_restrict = (struct restrict_ext *)((UINT32)malloc_buffer + malloc_len * flt_idx);
        mask = (UINT8 *)ali_sec_restrict + sizeof(struct restrict_ext);
        coef = mask + IPANEL_DMX_MASK_MAX_NUM * IPANEL_DMX_MASK_MAX_LEN;
        mask_en = coef + IPANEL_DMX_MASK_MAX_NUM * IPANEL_DMX_MASK_MAX_LEN;
        mask_hit = mask_en + IPANEL_DMX_MASK_MAX_NUM;
   
        ali_sec_restrict->multi_mask_ext     = mask;
        ali_sec_restrict->multi_value_ext    = coef;
        ali_sec_restrict->multi_mask_en_ext  = mask_en;
        ali_sec_restrict->multi_mask_hit_ext = mask_hit;
        ali_sec_restrict->multi_mask_len_ext = IPANEL_DMX_MASK_MAX_LEN;
        ali_sec_restrict->multi_mask_num_ext = IPANEL_DMX_MASK_MAX_NUM;

        /* Tell demux that will want more than 8 masks from this filter */
        ali_sec_restrict->tb_flt_msk = 0xFF00;
    
    	ali_sec_param->mask_value = (struct restrict *)ali_sec_restrict;

    	ali_sec_param->buff             = mask_hit + IPANEL_DMX_MASK_MAX_NUM;//MALLOC(4096);
    	//ASSERT(ali_sec_param->buff!=NULL);

    	ali_sec_param->buff_len         = 4096;
    	ali_sec_param->continue_get_sec = 1;
    	ali_sec_param->pid              = 0x1FFF;
    	ali_sec_param->get_sec_cb       = ali_get_section_callback;
    	ali_sec_param->retrieve_sec_fmt = RETRIEVE_SEC;
       
        for (mask_idx = 0; mask_idx < IPANEL_DMX_MASK_MAX_NUM; mask_idx++)
        {
            filter->mask[mask_idx].mask = mask + mask_idx * IPANEL_DMX_MASK_MAX_LEN;

            filter->mask[mask_idx].coef = coef + mask_idx * IPANEL_DMX_MASK_MAX_LEN;

            filter->mask[mask_idx].enable = mask_en + mask_idx;

            filter->mask[mask_idx].status = IPANEL_DMX_MASK_STATUS_IDLE;
        }
    }

	g_ipanel_sec_dispatch_task_running = TRUE;

	tsk_param.task = (FP)ipanel_sec_dispatch_task;
	tsk_param.name[0] = 's';
	tsk_param.name[1] = 'c';
	tsk_param.name[2] = 'm';
	tsk_param.quantum = 10;
	tsk_param.itskpri = OSAL_PRI_NORMAL;
	tsk_param.stksz = 0x8000;

	g_ipanel_sec_dispatch_task = osal_task_create(&tsk_param);
	if(INVALID_ID==g_ipanel_sec_dispatch_task)
	{
		//libc_printf("create task g_ipanel_sec_dispatch_task FAILED!\n");

		ASSERT(0);
	}
}

void ipanel_porting_dmx_task_exit(void)
{
	g_ipanel_sec_dispatch_task_running = FALSE;
    
	while(OSAL_INVALID_ID != g_ipanel_sec_dispatch_task)
	{
		osal_task_sleep(50);
	}
}

INT32 ipanel_porting_dmx_exit(void)
{
	UINT32 idx;

	for (idx = 0; idx < IPANEL_DMX_TOTAL_CHANNELS; idx++)
		ipanel_porting_destroy_demux_channel(idx);

	g_ipanle_dmx_buf_rd = 0;
	g_ipanle_dmx_buf_wr = 0;

    FREE(g_ipanel_dmx_filter[0].ali_sec_param.mask_value);
    return 0;
}

