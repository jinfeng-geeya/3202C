#include <types.h>
#include <sys_config.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/decv/decv.h>
#include <hld/nim/nim_dev.h>
#include <api/libsi/si_module.h>
#include <api/libtsi/sie.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libtsi/si_types.h>
#include <api/libchar/lib_char.h>

#include "bat_private.h"

//#define BAT_DEBUG
#ifdef BAT_DEBUG
#define BAT_PRINTF	libc_printf
#define BAT_EVENT_PRINTF libc_printf
#else
#define BAT_PRINTF(...)	        do{}while(0)
#define BAT_EVENT_PRINTF(...)	do{}while(0)
#endif

static ID bat_semaphore_id = OSAL_INVALID_ID;

static struct bat_sec_info *p_bat_section = NULL;
static UINT8 bat_get_finish = 0;

static bat_service_item bat_services[BOUQUET_NUM_MAX];
static UINT16 bouquet_cnt = 0;

static UINT16 bouquet_ids[BOUQUET_CNT] = 
{
	0x0001,0x0002,0x0003,0x0004,0x0005,0x0006,0x0007,0x0008,\
	0x0009,0x000A,0x000B
};

static BOOL bat_event(UINT16 pid, struct si_filter_t *filter,UINT8 reason, UINT8 *data, INT32 len);
static INT32 bat_parse(UINT8 *data, INT32 len);
static void ts_des_parse(UINT8 *head,UINT16 bouquet_id,UINT16 ts_id);
static void bouquet_des_parse(UINT8 *head,UINT16 bouquet_id);
static BOOL save_service_bouquet_name(UINT16 bouquet_id, UINT8 *name, UINT8 name_len);
static BOOL save_service_bouquet_id(UINT16 bouquet_id,UINT16 ts_id,UINT16 service_id);
static INT32 bat_get_section(UINT8 *buf, UINT16 buf_len);
static BOOL get_service_bouquet_id(UINT16 ts_id,UINT16 service_id,UINT16 *bouquet_id);


static BOOL bat_event(UINT16 pid, struct si_filter_t *filter,UINT8 reason, UINT8 *data, INT32 len)
{
    UINT16 i;
    struct bat_hitmap *hitmap = NULL;
    
    if(p_bat_section == NULL)
    {
        BAT_EVENT_PRINTF("!!!ERR,p_bat_section == NULL\n");
        return FALSE;
    }
    BAT_EVENT_PRINTF("**b_id=0x%x,sec_num=%d,_last_sec=%d\n",(data[3]<<8)|data[4],data[6],data[7]);

    for(i=0;i<BOUQUET_CNT;i++)
    {
        if(((UINT16)(data[3]<<8)|data[4]) == bouquet_ids[i])
            break;
    }

    if(i == BOUQUET_CNT)//Not the correct bat.
        return FALSE;
    
    hitmap = &p_bat_section->hitmap[i];
    //compute all section hit value
    if(hitmap->all_hit_value==0)
    {
        for(i = 0; i <= data[7]; i++)
            hitmap->all_hit_value |= (1<<i) | 1;
    }
    
    if(hitmap->all_hit_again==0)
    {
        //this section got already
        if((1<<data[6])&hitmap->hit)
        {
            //check if all section hit
            if(hitmap->all_hit_value==hitmap->hit)
                hitmap->all_hit_again= 1;
            return FALSE;
        }
        else
        {
            hitmap->hit |= 1<<data[6];
            return TRUE;
        }
    }
    //after all_hit_again flag=1, this section got again, all bat section shoule got
    else
    {
        if((1<<data[6])&hitmap->hit)
        {
            for(i=0;i<BOUQUET_CNT;i++)
            {
                if(p_bat_section->hitmap[i].all_hit_again != 1)
                    return FALSE;
            }

            if(BOUQUET_CNT == i)
                bat_get_finish = 1;
            //NOTE!!! return true to make bat handler free filter
            return TRUE;
        }
        return FALSE;
    }
  
}

static INT32 bat_parse(UINT8 *data, INT32 len)
{
	INT16 sec_len = 0;
	INT16 bouquet_loop_len = 0;
	INT16 ts_loop_len =0,service_loop_len = 0;
	INT16 desc_loop_len = 0;
	INT16 i, j;
	INT16 pos = 0;
	UINT16 bouquet_id = 0,ts_id = 0,service_id = 0;

	sec_len = ((data[1]&0xf)<<8) |data[2];
	bouquet_id = (data[3]<<8)|data[4];

	BAT_PRINTF("bouquet id = 0x%x,version = 0x%x",bouquet_id,(data[5]&0x3E)>>1);

	BAT_PRINTF("\n\nBouquet id=0x%x,sec_len=%d,sec_num=%d,last_sec=%d\n",bouquet_id, sec_len,data[6],data[7]);
	
	//empty section
	if(sec_len <= 13 )
		return SUCCESS;
	
	bouquet_loop_len = ((data[8]&0xf)<<8) |data[9];
	pos = 10 + bouquet_loop_len;
	
	//bouquet descriptor loop
	BAT_PRINTF("bouquet descriptor loop\n");
	for(i = 10; (i<pos)&&(i<sec_len-4); i += 2+data[i+1])
	{
		bouquet_des_parse(&data[i],bouquet_id);
	}

	//ts loop
	BAT_PRINTF("ts_stream loop\n");
	ts_loop_len = ((data[pos]&0xf)<<8) |data[pos+1];
	pos += 2+ts_loop_len;
	
	for(i += 2; (i<pos)&&(i<sec_len-4); i += 6+desc_loop_len)
	{
		ts_id = (data[i]<<8)+data[i+1];
		desc_loop_len = ((data[i+4]&0xf)<<8)|data[i+5];
		
		for(j = i+6; (j<i+6+desc_loop_len)&&(j<sec_len-4); j += 2+service_loop_len)
		{
			ts_des_parse(&data[j],bouquet_id,ts_id);
		}
	}
	
	return SUCCESS;
}

static void bouquet_des_parse(UINT8 *head,UINT16 bouquet_id)
{
	UINT16 des_len = 0;

	// Parse bouquet name
	if(head[0]==0x47)
	{
		des_len = head[1];
		save_service_bouquet_name(bouquet_id,&head[2],des_len);
	}
}

static void ts_des_parse(UINT8 *head,UINT16 bouquet_id,UINT16 ts_id)
{
	UINT16 des_len = 0;
	UINT16 service_id = 0;
	UINT16 i = 0;

	// Parse service list
	if(head[0]==0x41)
	{
		des_len = head[1];
		for(i = 2;i<(2+des_len);i +=3)
		{
		    service_id = (head[i]<<8)+head[i+1];
		    save_service_bouquet_id(bouquet_id,ts_id,service_id);
		}
	}
}

static BOOL save_service_bouquet_name(UINT16 bouquet_id, UINT8 *name, UINT8 name_len)
{
	UINT16 i;
	UINT8 n_len;
	
	if(bouquet_cnt >= BOUQUET_NUM_MAX)
		return FALSE;//the buffer full

	for(i = 0;i < bouquet_cnt;i++)
	{
		if (bat_services[i].bouquet_id == bouquet_id)
			return TRUE;
	}

	if (i == bouquet_cnt)
	{
		bat_services[i].bouquet_id = bouquet_id;
		dvb_to_unicode(name, name_len, bat_services[i].name, BOUQUET_NAME_NUM_MAX, 2312);
		bouquet_cnt++;
	}

	return TRUE;
}

static BOOL save_service_bouquet_id(UINT16 bouquet_id,UINT16 ts_id,UINT16 service_id)
{
	UINT16 i,j,k;

	if(bouquet_cnt >= BOUQUET_NUM_MAX)
		return FALSE;//the buffer full
	
	for(i = 0;i < bouquet_cnt;i++)
	{
		if (bat_services[i].bouquet_id == bouquet_id)
		{
			for(j = 0;j < TS_IN_BOUQUET_NUM_MAX;j++)
			{
				if(bat_services[i].ts_cnt >= TS_IN_BOUQUET_NUM_MAX)
					return FALSE;
				
				if(bat_services[i].ts_id[j] == ts_id)
				{
					for(k = 0;k < SERVICE_IN_TS_NUM_MAX;k++)
					{
						if(bat_services[i].service_cnt[bat_services[i].ts_cnt-1] >= SERVICE_IN_TS_NUM_MAX)
							return FALSE;
						
						if(bat_services[i].service_id[j][k] == service_id)
						{
							return TRUE;
						}
					}
					if (k == SERVICE_IN_TS_NUM_MAX)
					{
						bat_services[i].service_id[bat_services[i].ts_cnt-1][bat_services[i].service_cnt[bat_services[i].ts_cnt-1]] = service_id;
						bat_services[i].service_cnt[bat_services[i].ts_cnt-1]++;
						return TRUE;
					}
				}
			}
			if (j == TS_IN_BOUQUET_NUM_MAX)
			{
				bat_services[i].ts_id[bat_services[i].ts_cnt] = ts_id;
				bat_services[i].service_id[bat_services[i].ts_cnt][0] = service_id;
				bat_services[i].service_cnt[bat_services[i].ts_cnt]++;
				bat_services[i].ts_cnt++;
				return TRUE;
			}
		}
	}

	if(i == bouquet_cnt)//do not find the service
	{
		bat_services[i].bouquet_id = bouquet_id;
		bat_services[i].ts_id[0] = ts_id;
		bat_services[i].service_id[0][0] = service_id;
		bat_services[i].service_cnt[0]++;
		bat_services[i].ts_cnt++;
		return TRUE;
	}

	return TRUE;
}

static sie_status_t bat_handler(UINT16 pid, struct si_filter_t *filter, UINT8 reason, UINT8 *data, INT32 len)
{
    sie_status_t ret = sie_started;

    if(reason == SIE_REASON_FILTER_TIMEOUT)
    {
        BAT_PRINTF("BAT get time out\n");
        ret = sie_freed;
        osal_semaphore_release(bat_semaphore_id);
        //bat_priv_unlock();
    }
    else if(data != NULL)
    {
        //all bouquet id sections should got and parsed already
        if(bat_get_finish==1)
        {
            if(p_bat_section!=NULL)
            {
                FREE(p_bat_section);
                p_bat_section = NULL;
            }
            bat_get_finish = 0;
            osal_semaphore_release(bat_semaphore_id);
            //bat_priv_unlock();
            return sie_freed;
        }
        else
        {
            //bat parse
            bat_parse(data, len);
        }
        
    }
    return ret;
}

static INT32 bat_get_section(UINT8 *buf, UINT16 buf_len)
{
    INT32 ret;
    struct si_filter_t *filter = NULL;
    struct si_filter_param fparam;
    
      filter = sie_alloc_filter(PSI_BAT_PID, buf, buf_len, PSI_SHORT_SECTION_LENGTH);
	if(NULL == filter)
	{
		BAT_PRINTF("%s(): alloc filter failed!\n", __FUNCTION__);
		return -1;
	}

	MEMSET(&fparam, 0, sizeof(fparam));
	fparam.timeout = 15000;
	fparam.attr[0] = SI_ATTR_HAVE_CRC32;
	fparam.mask_value.tb_flt_msk = 0x01;
	fparam.mask_value.mask_len = 6;
	fparam.mask_value.multi_mask[0][0] = 0xFF;
	fparam.mask_value.multi_mask[0][1] = 0x80;
	//fparam.mask_value.multi_mask[0][3] = 0x00;
	//fparam.mask_value.multi_mask[0][4] = 0x00;
	fparam.mask_value.multi_mask[0][5] = 0x01;
	fparam.mask_value.value[0][0] = PSI_BAT_TABLE_ID;
	fparam.mask_value.value[0][1] = 0x80;
	//fparam.mask_value.value[0][3] = 0;
	//fparam.mask_value.value[0][4] = 0;
	fparam.mask_value.value[0][5] = 0x01;
		
	fparam.section_event = bat_event;
	fparam.section_parser = bat_handler;
	sie_config_filter(filter, &fparam);
	ret = sie_enable_filter(filter);
	if(ret != SI_SUCCESS)
	{
		BAT_PRINTF("%s(): enable filter failed!\n", __FUNCTION__);
		return -1;
	}
	
	return SUCCESS;
}

INT32 bat_priv_info_init(UINT32 frq, UINT32 symbol, UINT8 qam,BOOL change_to_mainfreq)
{
	struct nim_device *nim = NULL;
	UINT32 time_out = osal_get_tick()+1000;
	UINT8 lock = 0;
	INT8 ret;

	if(bat_semaphore_id == OSAL_INVALID_ID)
	{
		bat_semaphore_id = osal_semaphore_create(1);
		if(bat_semaphore_id == OSAL_INVALID_ID)
		{
			BAT_PRINTF("BAT create semaphore failed\n");
			return !SUCCESS;
		}
		//make semaphore value=0 to hold control task
		osal_semaphore_capture(bat_semaphore_id, 1000);
	}


	/*tune nim to main tp, check if lock*/
	nim = dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
	if(nim==NULL)
	    return !SUCCESS;

	if(change_to_mainfreq)
		nim_channel_change(nim, frq, symbol, qam);

	/*init parameter for bat*/
	bouquet_cnt = 0;
	p_bat_section = NULL;
	p_bat_section = (struct bat_sec_info *)MALLOC(sizeof(struct bat_sec_info));
	if(p_bat_section==NULL)
	{
		BAT_PRINTF("%s(), malloc memory failed!!!\n",__FUNCTION__);
		return !SUCCESS;
	}
	MEMSET(p_bat_section, 0, sizeof(struct bat_sec_info));
	MEMSET(bat_services,0,sizeof(bat_services));
	
	/*get bat on main tp*/
	while(osal_get_tick()<time_out)
	{
		lock = 0;
		nim_get_lock(nim, &lock);
		if(lock)
			break;
		else
			osal_task_sleep(2);
	}
	if(lock==0)
	{
		BAT_PRINTF("%s(), try 1second nim unlock at frq[%d],sym[%d],qam[%d]!!!\n",
			__FUNCTION__,frq,symbol,qam);
		return !SUCCESS;
	}
	
	bat_get_section(p_bat_section->section_buf, PSI_SHORT_SECTION_LENGTH);
	osal_semaphore_capture(bat_semaphore_id, OSAL_WAIT_FOREVER_TIME);
	//bat_priv_lock();

	return SUCCESS;
}

static BOOL get_service_bouquet_id(UINT16 ts_id,UINT16 service_id,UINT16 *bouquet_id)
{
	UINT16 i,j,k;

	for(i = 0;i < bouquet_cnt;i++)
	{
		for(j = 0;j < TS_IN_BOUQUET_NUM_MAX;j++)
		{
			if(bat_services[i].ts_id[j] == ts_id)
			{
				for(k = 0;k < SERVICE_IN_TS_NUM_MAX;k++)
				{
					if(bat_services[i].service_id[j][k] == service_id)
					{
						*bouquet_id = bat_services[i].bouquet_id;
						return TRUE;
					}
				}
				if (k == SERVICE_IN_TS_NUM_MAX)
				{
					continue;
				}
			}
		}
		if (j == TS_IN_BOUQUET_NUM_MAX)
		{
			continue;
		}
	}

	if(i == bouquet_cnt)
		return FALSE;

	return TRUE;
}

BOOL get_service_bouquet_info(UINT16 *bouquet_num, UINT16 *bouquet_id_list, UINT16 *bouquet_name)
{
	UINT16 i;
	UINT32 offest=0;
	
	if (!(bouquet_num && bouquet_id_list && bouquet_name))
		return FALSE;

	*bouquet_num = bouquet_cnt;
	for (i=0;i<bouquet_cnt;i++)
	{
		bouquet_id_list[i] = bat_services[i].bouquet_id;
		MEMCPY(&bouquet_name[offest], bat_services[i].name, 2*BOUQUET_NAME_NUM_MAX);
		offest += BOUQUET_NAME_NUM_MAX;
	}
	
	return TRUE;
}

void update_services_info(void)//update bouquet_id
{
	UINT16 prog_num = 0,i = 0;
	P_NODE p_node;
	T_NODE t_node;
	UINT16 bouquet_id= 0;
	
	recreate_prog_view(VIEW_ALL | PROG_TVRADIO_MODE, 0);
	prog_num = get_prog_num(VIEW_ALL | PROG_TVRADIO_MODE, 0);  

	for(i = 0;i<prog_num;i++)
	{
		MEMSET((void *)&p_node,0,sizeof(P_NODE));
		get_prog_at(i,&p_node);
		get_tp_by_id(p_node.tp_id,&t_node);
		if(get_service_bouquet_id(t_node.t_s_id,p_node.prog_number,&bouquet_id))
		{
			p_node.bouquet_id = bouquet_id;
			modify_prog(p_node.prog_id, &p_node);
		}
	}
	update_data();
}

