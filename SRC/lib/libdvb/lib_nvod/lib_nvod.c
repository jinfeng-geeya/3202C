#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#include <api/libsi/si_module.h>
#include <api/libtsi/sie.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/p_search.h>
#include <api/libsi/si_tdt.h>

#include <api/libsi/lib_nvod.h>
#include <api/libdb/db_node_c.h>

#include <hld/deca/deca_dev.h>
#include <hld/decv/decv.h>
#include <hld/nim/nim_dev.h>
#include <hld/snd/snd_dev.h>

#include <api/libpub/lib_as.h>
#include <api/libpub/lib_frontend.h>

#include <api/libsi/si_desc_cab_delivery.h>
#include <api/libsi/si_desc_service_list.h>
#include <api/libsi/si_nit.h>

//#define NVOD_DEBUG
#ifdef NVOD_DEBUG
#define NVOD_PRINTF	libc_printf
#define NVOD_EVENT_PRINTF libc_printf
#define NVOD_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) NVOD_PRINTF(" 0x%02x",*((data)+i)); \
                         NVOD_PRINTF("\n"); }
#else
#define NVOD_PRINTF(...)	do{}while(0)
#define NVOD_EVENT_PRINTF(...)	do{}while(0)
#define NVOD_DUMP(data,len) ;
#endif

void *nvod_buf = NULL;
#define NVOD_BUF_LEN 0x48000

#define NVOD_VIEW_MUTEX		0x00000001
#define NVOD_REF_EVENT_MUTEX	0x00000002
#define NVOD_EVENT_MUTEX		0x00000004
#define NVOD_PROG_MUTEX		0x00000008
#define NVOD_TP_MUTEX			0x00000010
#define NVOD_NIT_MUTEX              0x00000020

#define NVOD_ENTER_MUTEX(flag)		{UINT32 flgptn;\
	osal_flag_wait(&flgptn,nvod_flg_id,flag,TWF_ORW|TWF_CLR,OSAL_WAIT_FOREVER_TIME);}
#define NVOD_LEAVE_MUTEX(flag)		{osal_flag_set(nvod_flg_id, flag);}


static ID nvod_flg_id = OSAL_INVALID_ID;
static struct nvod_info *nvod_param = NULL;

static UINT8 nvod_module_state = NVOD_STATUS_NULL;

static UINT8 *nvod_search_buf = NULL;

P_NODE nvod_pre_pnode;

//function declare
void back_to_last_nvod_tp(P_NODE * pnode);


/******internal api for get and parse eit sections****************************************/
static BOOL nvod_dmx_event(UINT16 pid, struct si_filter_t *filter,UINT8 reason, UINT8 *eit, INT32 len)
{
	INT8 i;
	struct nvod_tp *info = &nvod_param->nvod_tp;
	struct subtable_hitmap *hitmap = NULL;
	struct eit_hitmap *hitmap2 = NULL;
	UINT16 service_id = (eit[3]<<8)|eit[4];
	UINT8 version = (eit[5]&0x3e)>>1;
	UINT8 sec_num = eit[6];
	UINT8 last_sec_num = eit[7];
	UINT8 seg_last_sec_num = eit[12];
	
	UINT8 tmp = 0;
	UINT8 seg_idx, sec_pos_in_seg;

	static UINT8 pre_last_sec_num = 0;
	
 /*send empty section to parse, hitmap and parsemap will be set*/
	//empty section
	//if(len <= 18)
	//	return FALSE;
	//for 0x4e, only get ref service's section
	if(eit[0] == 0x4e)
	{
		for(i = 0; i < info->ref_service_cnt; i++)
		{
			if(service_id==info->ref_service_id[i])
			{
				hitmap = &info->ref_eit_hitmap[i];
				break;
			}
		}
		if(i==info->ref_service_cnt)
			return FALSE;
	}
	//check in nvod services, if not find, return; 
	else if(eit[0] >=0x50)
	{
		for(i = 0; i < info->service_cnt; i++)
		{
			if(service_id==info->service_id[i].sid)
			{
				hitmap2 = &info->service_eit_hitmap[i];
				hitmap2->last_tb_id = eit[13];
				hitmap = &hitmap2->tables[eit[0]-0x50];
			
				break;
			}
		}
		if(i==info->service_cnt)
			return FALSE;
	}
		
	//version change
	if(version != hitmap->version)  
	{
		hitmap->seg_flg = 0;
		hitmap->seg_full_value = 0;
		MEMSET(hitmap->sec_flg, 0, sizeof(hitmap->sec_flg));
		hitmap->version = version;
	}
		
	seg_idx = sec_num>>3;
	sec_pos_in_seg = sec_num&0x7;
	//section already hit
	if((1<<sec_pos_in_seg) & hitmap->sec_flg[seg_idx])
		return FALSE;
	else
	{
	#ifdef NVOD_DEBUG
		if(eit[0]==0x4e)
		{
			NVOD_PRINTF("tid[0x%x], len[%d],sid[%d], ver[%d],sec[%d],tick[%d],",eit[0],len,
				service_id,(eit[5]&0x3e)>>1,eit[6],osal_get_tick());
			NVOD_PRINTF("last_sec[%d],seg_last_sec[%d], last_tid[0x%x]\n",eit[7],eit[12],eit[13]);
		}
		else //if(service_id==2081 || service_id==2071)
		{
			//NVOD_PRINTF("\neit sec hit, tick=%d\n",osal_get_tick());
			NVOD_PRINTF("tid[0x%x], len[%d],sid[%d], ver[%d],tick=%d,sec[%d],",eit[0],len,service_id,(eit[5]&0x3e)>>1,osal_get_tick(),eit[6]);
			NVOD_PRINTF("last_sec[%d],seg_last_sec[%d], last_tid[0x%x]\n",eit[7],eit[12],eit[13]);
		}
	#endif

		hitmap->sec_flg[seg_idx] |= 1<<sec_pos_in_seg;
		//check if segment section all hit
		sec_pos_in_seg = seg_last_sec_num&0x7;

		/****patch for Xuyi special case********/
		//some eit section has wrong info, seg_last_sec_num=last_sec_num
		if(seg_last_sec_num==last_sec_num)
		{
			UINT8 last_seg_idx = last_sec_num>>3;
			//for segment that is not last segment, should have 8 sections
			if(seg_idx < last_seg_idx)
				sec_pos_in_seg = 0x7;
			//for last segment, should have (seg_last_sec_num&0x7)+1 sections
		}
		/********************************/
		
		tmp = 1;
		for(i=0; i<sec_pos_in_seg; i++)
			tmp = (tmp<<1)|1;
		if(hitmap->sec_flg[seg_idx] == tmp)
			hitmap->seg_flg |= 1<<seg_idx;
		
		//sub-table's all segment hit value
		//some dvbc eit has wrong info, section_number>last_section_number
		if(sec_num > last_sec_num)
			//hitmap->seg_full_value = 0xFFFFFFFF;
			last_sec_num = sec_num;
		if((last_sec_num != pre_last_sec_num) ||(hitmap->seg_full_value == 0))
		{
			hitmap->seg_full_value = 1;
			tmp = last_sec_num>>3;
			for(i=0; i<tmp; i++)
				hitmap->seg_full_value = (hitmap->seg_full_value<<1)|1;
			pre_last_sec_num = last_sec_num;
		}
		return TRUE;
	}
}

static void nvod_add_ref_event(struct nvod_ref_event *ref)
{
	INT16 i;
	struct nvod_info *info = nvod_param;
	UINT8 update_flg = 0;
	UINT16 pos = 0;

	if(ref==NULL || info==NULL)
		return;
	
	NVOD_ENTER_MUTEX(NVOD_REF_EVENT_MUTEX);
	if(info->ref_event_cnt==NVOD_MAX_REF_EVNET_CNT)
	{
		NVOD_LEAVE_MUTEX(NVOD_REF_EVENT_MUTEX);
		NVOD_PRINTF("nvod ref event to max cnt, can't add!!!\n");
		return;
	}
	for(i = 0; i < info->ref_event_cnt; i++)
	{
		if((ref->t_s_id==info->ref_event[i].t_s_id)
			&&(ref->ref_service_id==info->ref_event[i].ref_service_id)
			&& (ref->event_id==info->ref_event[i].event_id))
			break;
	}
	if(i <  info->ref_event_cnt)
	{
		if(MEMCMP(ref, &info->ref_event[i], sizeof(struct nvod_ref_event))!=0)
		{
			MEMCPY(&info->ref_event[i], ref, sizeof(struct nvod_ref_event));
			update_flg = 1;
			pos = i;
			NVOD_PRINTF("nvod ref event[%d] content update, ref_service[%d]\n",ref->event_id,ref->ref_service_id);
		}
	}
	else if(i == info->ref_event_cnt)
	{
		MEMCPY(&info->ref_event[info->ref_event_cnt++], ref, sizeof(struct nvod_ref_event));
		update_flg = 1;
		pos = i;
		NVOD_PRINTF("nvod add new ref event[%d], ref_service[%d]\n",ref->event_id,ref->ref_service_id);
	}
		
	NVOD_LEAVE_MUTEX(NVOD_REF_EVENT_MUTEX);
	if(info->call_back && update_flg)
	{
		//info->call_back(NVOD_UPDATE_REF_EVENT, (ref->ref_service_id<<16)|ref->event_id);
		info->call_back(NVOD_UPDATE_REF_EVENT, pos);
		NVOD_PRINTF("send NVOD_UPDATE_REF_EVENT\n");
	}

}


static void nvod_add_event(struct nvod_event *event)
{
	UINT16 i;
	struct nvod_info *info = nvod_param;

	if(event==NULL || info==NULL)
		return;
	
	NVOD_ENTER_MUTEX(NVOD_EVENT_MUTEX);
	if(info->event_cnt==NVOD_MAX_EVENT_CNT)
	{
		NVOD_LEAVE_MUTEX(NVOD_EVENT_MUTEX);
		NVOD_PRINTF("nvod event to max cnt, can't add!!!\n");
		return;
	}
	for(i = 0; i < info->event_cnt; i++)
	{
		if((event->event_id==info->event[i].event_id)&&(event->service_id==info->event[i].service_id)
			&&(event->t_s_id==info->event[i].t_s_id))
			break;
	}
	if(i < info->event_cnt)
	{
		if(MEMCMP(event, &info->event[i], sizeof(struct nvod_event))!=0)
		{
			MEMCPY(&info->event[i], event, sizeof(struct nvod_event));
			NVOD_PRINTF("nvod event[%d] content update, service[%d], ref event[%d], ref service[%d]\n",
				event->event_id,event->service_id,event->ref_event_id,event->ref_service_id);
		}
	}
	else
	{
		MEMCPY(&info->event[info->event_cnt], event, sizeof(struct nvod_event));
		info->event_cnt++;
	}
	NVOD_LEAVE_MUTEX(NVOD_EVENT_MUTEX);

}


static INT32 saveorload_tp_eitinfo(UINT8 direction,T_NODE *tp)
{
    UINT16 i;
    struct nvod_info *pinfo = nvod_param;

    if(pinfo==NULL)
    	return -1;
    NVOD_ENTER_MUTEX(NVOD_TP_MUTEX);
    //if(MEMCMP(tp, &pinfo->cur_tp, sizeof(T_NODE))==0)
    //{
    //	NVOD_LEAVE_MUTEX(NVOD_TP_MUTEX);
    //	return SUCCESS; 
    //}
    
    for(i = 0; i < pinfo->tp_cnt; i++)
    	if((pinfo->tp[i].frq==tp->frq)&&(pinfo->tp[i].sym==tp->sym)
    		&&(pinfo->tp[i].FEC_inner==tp->FEC_inner))
    		break;

    if(i==pinfo->tp_cnt)
    {
        NVOD_LEAVE_MUTEX(NVOD_TP_MUTEX);
        NVOD_PRINTF("%s(): not find tp: frq[%d],sym[%d],qam[%d] in nvod tp array!\n",
        	__FUNCTION__,tp->frq,tp->sym,tp->FEC_inner);
        return -1;
    }
  
    //save tp eitbitmap 
    if(direction==0)
        MEMCPY(&pinfo->tp_eitbitmap[i], &pinfo->nvod_tp, sizeof(struct nvod_tp));
    //load tp and tp eitbitmap
    else
    {
        pinfo->tp_eitbitmap[i].eit_timeout_flag = 0;
        MEMCPY(&pinfo->nvod_tp, &pinfo->tp_eitbitmap[i], sizeof(struct nvod_tp));
    	//MEMCPY(&pinfo->cur_tp, &pinfo->tp[i], sizeof(T_NODE));
    }

    NVOD_LEAVE_MUTEX(NVOD_TP_MUTEX);

    return SUCCESS;

}


static sie_status_t nvod_eit_parser(UINT16 pid, struct si_filter_t *filter, UINT8 reason, UINT8 *eit, INT32 length)
{
	INT16 len;
	UINT16 service_id;
	UINT8 version;
	UINT8 sec_num;
	UINT8 seg_last_sec_num;
	
	UINT8 tmp = 0;
	UINT8 seg_idx, sec_pos_in_seg;
	INT16 event_len;
	INT8 i;
	
	struct nvod_tp *info = &nvod_param->nvod_tp;
	nvod_callback call_back = nvod_param->call_back;
	nvod_eit_complete eit_complete = nvod_param->eit_complete;
	struct subtable_hitmap *hitmap = NULL;
	struct subtable_hitmap *parsmap = NULL;
	struct eit_hitmap *hitmap2 = NULL;
	struct eit_hitmap *parsmap2 = NULL;

	struct nvod_ref_event ref_event;
	struct nvod_event event;
	UINT8 event_time[8];
	UINT16 event_id;
	UINT8 name_len;
	UINT8 service_pos = 0;
	INT8 tmp2 = 0;
	UINT32 *pservice_flg = NULL;
	//UINT16 t_s_id = nvod_param->cur_tp.t_s_id;
	//some SI info wrong, ts_id in PAT,EIT not match
	UINT16 t_s_id = 0;
	
	if (NULL != eit)
	{
		t_s_id = (eit[8]<<8)|eit[9];
		len = ((eit[1]&0xf)<<8)|eit[2];
		service_id = (eit[3]<<8)|eit[4];
		version = (eit[5]&0x3e)>>1;
		sec_num = eit[6];
		seg_last_sec_num = eit[12];
		if(eit[0] == 0x4e)
		{
			for(i = 0; i < info->ref_service_cnt; i++)
			{
				if(service_id==info->ref_service_id[i])
				{
					parsmap = &info->ref_eit_parsflg[i];
					hitmap = &info->ref_eit_hitmap[i];
					pservice_flg = &info->ref_services_parsflg;
					service_pos = i;
					break;
				}
			}
		}
		else if(eit[0] >=0x50)
		{
			tmp = eit[0] - 0x50;
			for(i = 0; i < info->service_cnt; i++)
			{
				if(service_id==info->service_id[i].sid)
				{
					hitmap2 = &info->service_eit_hitmap[i];
					hitmap = &hitmap2->tables[tmp];
					parsmap2 = &info->service_eit_parsflg[i];
					parsmap2->last_tb_id = eit[13];
					parsmap = &parsmap2->tables[tmp];
					pservice_flg = &info->services_parsflg[tmp];
					service_pos = i;
					break;
				}
			}
		}
		if(parsmap == NULL)
			return sie_started;
	
		if(version != parsmap->version)  
		{
			parsmap->seg_flg = 0;
			MEMSET(parsmap->sec_flg, 0, sizeof(parsmap->sec_flg));
			parsmap->version = version;
			*pservice_flg = (*pservice_flg)&(~(1<<service_pos));
		}
		
		UINT8 *pdata = eit+14;
		len = len - 11- 4;
		while(len > 0)
		{
			event_id = (pdata[0]<<8)|pdata[1];
			//NVOD_PRINTF("\nevent_id[%d], ",event_id);
			if(eit[0] == 0x4e)
			{
				MEMSET(&ref_event, 0, sizeof(struct nvod_ref_event));
				ref_event.event_id = event_id;
				ref_event.ref_service_id = service_id;
				ref_event.t_s_id = t_s_id;
			}
			MEMCPY(event_time, &pdata[2], 8);
                 /* UINT32 time;
                    UINT16 y;
                    UINT8 mon,d,w,h,min,s;
			time = (pdata[2]<<8)|pdata[3];
			mjd_to_ymd(time, &y, &mon, &d, &w);
			bcd_to_hms(pdata+4, &h, &min, &s);
			NVOD_PRINTF("start_time[%d-%d-%d  %d:%d:%d], ",y,mon,d,h,min,s);
			bcd_to_hms(pdata+7,  &h, &min, &s);
			NVOD_PRINTF("duration[%d:%d:%d], runing_status[%d], ",h,min,s,(pdata[10]&0xe0)>>5);
                */
			INT16 dec_loop_len = ((pdata[10]&0xf)<<8)|pdata[11];
			//NVOD_PRINTF("descriptor loop len[%d]\n", dec_loop_len);
			event_len = dec_loop_len;
			pdata += 12;
			while(dec_loop_len > 0)
			{
				//nvod service event
				if((eit[0]>=0x50)&&( pdata[0] == TIME_SHIFTED_EVENT_DESCRIPTOR))
				{
					//NVOD_PRINTF("time_shift_event_dec, event_id[%d],service_id[%d],len[%d],ref_service_id[%d],ref_event_id[%d]\n",
					//	event_id,service_id,pdata[1],(pdata[2]<<8)|pdata[3], (pdata[4]<<8)|pdata[5]);
					
					event.event_id = event_id;
					event.service_id = service_id;
					event.t_s_id = t_s_id;
					event.ref_service_id = (pdata[2]<<8)|pdata[3];
					event.ref_event_id = (pdata[4]<<8)|pdata[5];
					MEMCPY(event.time, event_time, 8);
					//add this event to event buf
					nvod_add_event(&event);
                                NVOD_EVENT_PRINTF("event id[%d],service_id[%d],ref_event id[%d],ref_service_id[%d]\n",
                                    event.event_id,event.service_id,event.ref_event_id,event.ref_service_id);
				}
				//reference service event
				else if((eit[0]==0x4e)&& (pdata[0] == SHORT_EVENT_DESCRIPTOR))
				{
					//NVOD_PRINTF("short_event_dec, len[%d],name_len[%d],txt_len[%d]\n",
					//	pdata[1],pdata[5], pdata[6+pdata[5]]);
					
					MEMCPY(ref_event.lang_code, &pdata[2],3);
					name_len = (pdata[5] < sizeof(ref_event.name)) ? pdata[5]:sizeof(ref_event.name);
					ref_event.name_len = name_len;
					MEMCPY(ref_event.name, &pdata[6],name_len);
					ref_event.txt_len = pdata[6+pdata[5]];
					MEMCPY(ref_event.txt, &pdata[7+pdata[5]],ref_event.txt_len);
					nvod_add_ref_event(&ref_event);
                                NVOD_EVENT_PRINTF("ref_event id[%d],ref_service_id[%d],name len[%d]\n",ref_event.event_id,ref_event.ref_service_id,name_len);
                                NVOD_EVENT_PRINTF("ref_event mjd[%d],start[%x:%x:%x],len[%x:%x:%x]\n",
                                    (event_time[0]<<8)|event_time[1],event_time[2],event_time[3],event_time[4],
                                    event_time[5],event_time[6],event_time[7]);
				}
				/*
				else if(pdata[0] == EXTENDED_EVENT_DESCRIPTOR)
				{
					NVOD_PRINTF("extended_event_dec, len[%d],desc_num[%d],last_desc_num[%d], ",
						pdata[1],(pdata[2]&0xf0)>>4, pdata[2]&0xf);
					NVOD_PRINTF("item_len[%d], txt_len[%d]\n",pdata[6], pdata[7+pdata[6]]);
				}
				*/
				dec_loop_len -= 2+pdata[1];
				pdata += 2+pdata[1];

			}
			len -= 12+event_len;
			
		}
		
		seg_idx = sec_num>>3;
		sec_pos_in_seg = sec_num&0x7;
		//section  parsed flag
		parsmap->sec_flg[seg_idx] |= 1<<sec_pos_in_seg;
		//check if segment section all parsed
		sec_pos_in_seg = seg_last_sec_num&0x7;

		/****patch for Xuyi special case********/
		//some eit section has wrong info, seg_last_sec_num=last_sec_num
		if(seg_last_sec_num==eit[7])
		{
			UINT8 last_seg_idx = eit[7]>>3;
			//for segment that is not last segment, should have 8 sections
			if(seg_idx < last_seg_idx)
				sec_pos_in_seg = 0x7;
			//for last segment, should have (seg_last_sec_num&0x7)+1 sections
		}
		/**********************************/
		
		tmp = 1;
		for(i=0; i<sec_pos_in_seg; i++)
			tmp = (tmp<<1)|1;
		if(parsmap->sec_flg[seg_idx] == tmp)
			parsmap->seg_flg |= 1<<seg_idx;
		
		//sub-table's all segment parsed
		if(parsmap->seg_flg == hitmap->seg_full_value)
		{
			*pservice_flg  |= (1<<service_pos);
			NVOD_PRINTF("$$$table[0x%x], sid[%d],ver[%d] all sec parsed[0x%x],tick=%d!\n",eit[0], 
				service_id,version,parsmap->seg_flg,osal_get_tick());
		}
		else
			return sie_started;
		
		//all ref services all segment parsed
		if(info->ref_services_parsflg != info->ref_services_full_value)
			return sie_started;
		
		tmp2 = info->service_eit_hitmap[0].last_tb_id - 0x50;
		if(info->services_full_value[eit[0]-0x50]!=info->services_parsflg[eit[0]-0x50])
			return sie_started;
		else		
		{
			NVOD_PRINTF("$$$nvod all services table[0x%x] sections parsed!\n",eit[0]);
		      for(i = 0; i <= tmp2; i++)
        		{
        			if(info->services_full_value[i]!=info->services_parsflg[i])
        				break;
        		}
                    //all service all eit table parsed
                    if(i==(tmp2+1))
                    {
                        //stop_tdt();
                        if((nvod_module_state==NVOD_STATUS_IN_SCAN)&&(eit_complete))
                        {
                            saveorload_tp_eitinfo(0, &nvod_param->cur_tp);
                            eit_complete();
                        }
                        if(call_back)
				call_back(NVOD_UPDATE_EVENT_SCH, 0);
						
			//if all services all section parsed, we can close eit filter. when play service, eit will start again
			return sie_freed;
                    }
                    return sie_started;
		}
					
	}
	else if(reason==SIE_REASON_FILTER_TIMEOUT)
	{
            NVOD_PRINTF("$$$nvod get EIT timeout, tick=%d!\n", osal_get_tick());
            info->eit_timeout_flag = 1;
		//stop_tdt();
            if((nvod_module_state==NVOD_STATUS_IN_SCAN)&&(eit_complete))
	      {
                saveorload_tp_eitinfo(0, &nvod_param->cur_tp);
                eit_complete();
            }
            if(call_back)
                call_back(NVOD_TIMEOUT, 0);
            return sie_started;
	}
	
	
}

/*****************************************************************************/

/****internal api for create one ref event's time list schedule*****************************/
static INT32 nvod_create_view(UINT16 ref_event_id, UINT16 ref_service_id, UINT16 t_s_id)
{
	UINT16 i;
	struct nvod_info *info = nvod_param;
	UINT16 mjd = 0;
	UINT16 hms = 0;
	UINT8 h,m,s;

	NVOD_PRINTF("nvod_reate_view(): ref event[%d] ref service[%d]:\n",ref_event_id,ref_service_id);
	NVOD_ENTER_MUTEX(NVOD_EVENT_MUTEX);
	info->view_node_cnt = 0;
	for(i = 0; i < info->event_cnt; i++)
	{
		if((ref_event_id==info->event[i].ref_event_id)
			&&(ref_service_id==info->event[i].ref_service_id)
			&&(t_s_id==info->event[i].t_s_id))
		{
			if(info->view_node_cnt==NVOD_MAX_VIEW_CNT)
			{
				NVOD_PRINTF("nvod view node to max cnt, break!!!\n");
				break;
			}
			//coded event time to uint32
			mjd = (info->event[i].time[0]<<8)|info->event[i].time[1];
			bcd_to_hms(&(info->event[i].time[2]), &h, &m, &s);
			//format:hour 5bit, min 6bit, second 5bit(unit 2 second)
			hms = (h<<11)|(m<<5)|(s>>1);
			info->view_node_time[info->view_node_cnt] = (mjd<<16)|hms;
			info->view[info->view_node_cnt++] = &info->event[i];
			
		#if 0
			NVOD_PRINTF("event[%d] sid[%d], ",info->event[i].event_id,info->event[i].service_id);
			UINT16 y;
			UINT8 mon, d,week_day;

			mjd_to_ymd(mjd, &y, &mon, &d, &week_day);
			NVOD_PRINTF("[%d-%d-%d %d:%d:%d]\n",y,mon,d,h,m, s);
		#endif
		}
	}
	NVOD_LEAVE_MUTEX(NVOD_EVENT_MUTEX);
	NVOD_PRINTF("nvod view node cnt[%d]\n",info->view_node_cnt);
	return SUCCESS;
}

static void shellpass(UINT32 *array, INT16 step, INT16 from, INT16 to,struct nvod_event **pevent)
{
	INT16 i, j;
	UINT32 key = 0;
	struct nvod_event *tmp = NULL;
	//struct nvod_event **pevent = &nvod_param->view[0];
		
	for(i = from+step; i <= to; i++)
	{
		if(array[i] < array[i-step])
		{
			tmp = pevent[i];
			key = array[i]; j = i-step;
		
			do{
				array[j+step] = array[j];
				pevent[j+step] = pevent[j];
				j -= step;

				}while(j>=from && array[j] > key);
			array[j+step] = key;
			pevent[j+step] = tmp;
		}
	}
}

static void shellsort(UINT32 *array, INT16 from, INT16 to,struct nvod_event **pevent)
{
	INT16 step = to-from+1;

	if(from == to)
		return;
	do{
		if(step / 3 == 1)
			step = 1;
		else
			step = step / 3 + 1;
		shellpass(array, step, from, to, pevent);

		}while(step > 1);

}


static void ref_event_shellpass(UINT32 *array, INT16 step, INT16 from, INT16 to)
{
	INT16 i, j;
	UINT32 key = 0;
	struct nvod_ref_event tmp;
	struct nvod_ref_event *pevent = &nvod_param->ref_event[0];
		
	for(i = from+step; i <= to; i++)
	{
		if(array[i] < array[i-step])
		{
			MEMCPY(&tmp, &pevent[i],sizeof(tmp));
			key = array[i]; j = i-step;
		
			do{
				array[j+step] = array[j];
				MEMCPY(&pevent[j+step],&pevent[j],sizeof(pevent[j]));
				j -= step;

				}while(j>=from && array[j] > key);
			array[j+step] = key;
			MEMCPY(&pevent[j+step],&tmp,sizeof(tmp));
		}
	}
}

static void ref_event_shellsort(UINT32 *array, INT16 from, INT16 to)
{
	INT16 step = to-from+1;

	if(from == to)
		return;
	do{
		if(step / 3 == 1)
			step = 1;
		else
			step = step / 3 + 1;
		ref_event_shellpass(array, step, from, to);

		}while(step > 1);
}



static void nvod_as_event_handler(UINT8 event_type, UINT32 value)
{
    UINT16 i;
    INT16 j;
    INT8 table_cnt;
    UINT8 last_table_id;
    
	if((event_type==ASM_TYPE_PROGRESS)&&(value==AS_PROGRESS_SCANOVER))
	{
		struct nvod_info *pinfo = nvod_param;

            db_search_exit();
            if(nvod_search_buf!=NULL)
            {
                FREE(nvod_search_buf);
                nvod_search_buf = NULL;
            }
            
            /*if exit when scan not completed, lib_as still send AS_PROGRESS_SCANOVER*/
            //check if all scanned tps' eit completed
            for(i=0; i < pinfo->tp_cnt; i++)
            {
                struct nvod_tp *tp_eit = &pinfo->tp_eitbitmap[i];
                /*some stream has error eit, nvod services listed in sdt not have eit section*/
                last_table_id = 0;
                for(j=0;j<tp_eit->service_cnt;j++)
                {
                    if(tp_eit->service_eit_hitmap[j].last_tb_id!=0)
                    {
                        last_table_id = tp_eit->service_eit_hitmap[j].last_tb_id;
                        break;
                    }
                }
                table_cnt = last_table_id - 0x50 + 1;
                for(j = 0 ; j < table_cnt;j++)
                {
                    //get eit timeout on the tp, but already scan it
                    if(tp_eit->eit_timeout_flag==1)
                        continue;
                    else if((tp_eit->services_full_value[j]==0)||
                        (tp_eit->services_full_value[j]!=tp_eit->services_parsflg[j]))
                        break;
                }
                if(j!=table_cnt)
                    break;
            }
            if((pinfo->tp_cnt>0)&&(i==pinfo->tp_cnt))
            {
                nvod_module_state = NVOD_STATUS_SCAN_OVER;
                if(pinfo->call_back)
				pinfo->call_back(NVOD_SCAN_OVER, 0);
            }
            if((pinfo->ref_event_cnt==0)&&(pinfo->event_cnt==0))
                if(pinfo->call_back)
                    pinfo->call_back(NVOD_NO_INFO, 0);
            
  
	}
}

static void clear_nvod_info()
{
    struct nvod_info *pinfo = nvod_param;
    if(pinfo==NULL)
        return;
    else
    {
        osal_task_dispatch_off();
        MEMSET(pinfo, 0, sizeof(struct nvod_info));
        osal_task_dispatch_on();
    }
}


/***************************************************************/



/****APIs for external use******************************************/

INT32 nvod_start_eit(BOOL search_mode)
{
	INT32 ret;
	struct si_filter_t *filter = NULL;
	struct si_filter_param fparam;

	if((nvod_param==NULL)||(nvod_param->eit_buf==NULL))
		return -1;
	UINT8 * buf = nvod_param->eit_buf;
	UINT32 buf_len = sizeof(nvod_param->eit_buf);

	filter = sie_alloc_filter(PSI_EIT_PID, buf, buf_len, PSI_LONG_SECTION_LENGTH);
	if(NULL == filter)
	{
		NVOD_PRINTF("%s(): alloc filter failed!\n", __FUNCTION__);
		return -1;
	}
	MEMSET(&fparam, 0, sizeof(fparam));
	/*get eit time out should changed according to eit interval of each place*/
	if(search_mode==TRUE)
		fparam.timeout = 15000;//25000;
	else
		fparam.timeout = OSAL_WAIT_FOREVER_TIME;
	fparam.attr[0] = fparam.attr[1] = SI_ATTR_HAVE_CRC32;
	fparam.mask_value.tb_flt_msk = 0x3;
	fparam.mask_value.mask_len = 6;
	fparam.mask_value.multi_mask[0][0] = 0xFF;
	fparam.mask_value.multi_mask[0][1] = 0x80;
	//fparam.mask_value.multi_mask[0][3] = 0xFF;
	//fparam.mask_value.multi_mask[0][4] = 0xFF;
	fparam.mask_value.multi_mask[0][5] = 0x01;
	//only get eit table 0x50~0x51
	fparam.mask_value.multi_mask[1][0] = 0xFE;
	fparam.mask_value.multi_mask[1][1] = 0x80;
	//fparam.mask_value.multi_mask[1][3] = 0xFF;
	//fparam.mask_value.multi_mask[1][4] = 0xFE;
	fparam.mask_value.multi_mask[1][5] = 0x01;
	
	fparam.mask_value.value[0][0] = 0x4E;
	fparam.mask_value.value[0][1] = 0x80;
	//fparam.mask_value.value[0][3] = 0x00;
	//fparam.mask_value.value[0][4] = 0xEE;
	fparam.mask_value.value[0][5] = 0x01;
	fparam.mask_value.value[1][0] = 0x50;
	fparam.mask_value.value[1][1] = 0x80;
	//fparam.mask_value.value[1][3] = 0x00;
	//fparam.mask_value.value[1][4] = 0xB8;
	fparam.mask_value.value[1][5] = 0x01;
	
	fparam.section_event = nvod_dmx_event;
	fparam.section_parser = nvod_eit_parser;
	sie_config_filter(filter, &fparam);
	ret = sie_enable_filter(filter);
	if(ret != SI_SUCCESS)
	{
		NVOD_PRINTF("%s(): enable filter failed!\n", __FUNCTION__);
		return -1;
	}
	NVOD_PRINTF("%s(): tick=%d!\n", __FUNCTION__,osal_get_tick());
	return SUCCESS;
}



INT32 nvod_add_tp(T_NODE *node)
{
	struct nvod_info *pinfo = nvod_param;
	UINT16 i;

	NVOD_ENTER_MUTEX(NVOD_TP_MUTEX);
	if(pinfo->tp_cnt >= NVOD_MAX_TP_CNT)
	{
		NVOD_LEAVE_MUTEX(NVOD_TP_MUTEX);
		NVOD_PRINTF("nvod TP cnt to max number!\n");
		return -1;
	}
	
	for(i = 0; i < pinfo->tp_cnt; i++)
		if((node->frq==pinfo->tp[i].frq)&&(node->sym==pinfo->tp[i].sym)
			&&(node->FEC_inner==pinfo->tp[i].FEC_inner))
			break;
	if(i==pinfo->tp_cnt)
	{
		MEMCPY(&pinfo->tp[pinfo->tp_cnt], node, sizeof(T_NODE));
		pinfo->tp_cnt++;
	}
	
	NVOD_LEAVE_MUTEX(NVOD_TP_MUTEX);
	return SUCCESS;

}



INT32 nvod_scan_start(T_NODE *tp,struct nvod_sdt_info * sdt_info)
{
	struct nvod_tp *nvod = NULL;
	UINT32 tmp = 0;
	UINT8 i;

	if((sdt_info == NULL)||(nvod_param==NULL))
		return -1;
	NVOD_PRINTF("%s(): tick=%d\n",__FUNCTION__,osal_get_tick());
	nvod = &nvod_param->nvod_tp;
	start_tdt();
      /*if load parameter tp fail, means this tp not in nvod info*/      
	if(-1==saveorload_tp_eitinfo(1, tp))
	    MEMSET(nvod, 0, sizeof(struct nvod_tp));
    
	//copy sdt nvod info to nvod struct
	nvod->ref_service_cnt = sdt_info->ref_cnt;
	nvod->ref_services_full_value = 1;
	for(i = 1; i < nvod->ref_service_cnt; i++)
		nvod->ref_services_full_value = (nvod->ref_services_full_value<<1)|1;
	MEMCPY(nvod->ref_service_id, sdt_info->ref_id, sizeof(UINT16)*sdt_info->ref_cnt);

	nvod->service_cnt = sdt_info->scnt;
	tmp = 1;
	for(i = 1; i < nvod->service_cnt; i++)
		tmp = (tmp<<1)|1;
	for(i = 0; i < MAX_EIT_TABLE_CNT; i++)
		nvod->services_full_value[i] = tmp;
	MEMCPY(nvod->service_id, sdt_info->sid, sizeof(struct nvod_service_id)*sdt_info->scnt);
	if(sdt_info->scnt > 0)
	{
		nvod->transport_stream_id = sdt_info->sid[0].t_s_id;
		nvod->origin_netid = sdt_info->sid[0].origin_netid;
		tp->t_s_id = nvod->transport_stream_id;
		nvod_add_tp( tp);
		//set this tp as current tp
		MEMCPY(&nvod_param->cur_tp, tp, sizeof(T_NODE));
	}
		
	return nvod_start_eit(TRUE);
}




UINT16 nvod_get_ref_event_cnt()
{
	UINT16 i = 0;
	struct nvod_info *info = nvod_param;

	if(info==NULL)
		return 0;
	
	NVOD_ENTER_MUTEX(NVOD_REF_EVENT_MUTEX);
	i = info->ref_event_cnt;
	NVOD_LEAVE_MUTEX(NVOD_REF_EVENT_MUTEX);
	return i;

}

//INT32 nvod_get_ref_event(UINT32 id, struct nvod_ref_event *ref)
INT32 nvod_get_ref_event(UINT16 pos, struct nvod_ref_event *ref)
{
	INT16 i = 0;
	struct nvod_info *info = nvod_param;
	//UINT16 ref_service_id = (UINT16)(id>>16); 
	//UINT16 event_id = id&0xFFFF;

	if(ref==NULL || info==NULL)
		return -1;
	
	NVOD_ENTER_MUTEX(NVOD_REF_EVENT_MUTEX);
/*	for(i = 0; i < info->ref_event_cnt; i++)
	{
		if((event_id==info->ref_event[i].event_id)&&(ref_service_id==info->ref_event[i].ref_service_id))
			break;
	}
*/	
	if(pos>= info->ref_event_cnt)
	{
		NVOD_LEAVE_MUTEX(NVOD_REF_EVENT_MUTEX);
		NVOD_PRINTF("%s():ref event index[%d] not found\n",__FUNCTION__, pos);
		return -1;
	}
	
	MEMCPY(ref, &info->ref_event[pos],sizeof(struct nvod_ref_event));
			
	NVOD_LEAVE_MUTEX(NVOD_REF_EVENT_MUTEX);
	return SUCCESS;

}

INT32 nvod_get_ref_event_byid(UINT16 ref_event_id, UINT16 ref_service_id, struct nvod_ref_event *ref)
{
	INT16 i = 0;
	struct nvod_info *info = nvod_param;
	
	if(ref==NULL || info==NULL)
		return -1;
	
	NVOD_ENTER_MUTEX(NVOD_REF_EVENT_MUTEX);
	for(i = 0; i < info->ref_event_cnt; i++)
	{
		if((ref_event_id==info->ref_event[i].event_id)&&(ref_service_id==info->ref_event[i].ref_service_id))
			break;
	}

	if(i>= info->ref_event_cnt)
	{
		NVOD_LEAVE_MUTEX(NVOD_REF_EVENT_MUTEX);
		NVOD_PRINTF("%s():ref event[%d] ref_service[%d] not found\n",__FUNCTION__, ref_event_id,ref_service_id);
		return -1;
	}
	
	MEMCPY(ref, &info->ref_event[i],sizeof(struct nvod_ref_event));
			
	NVOD_LEAVE_MUTEX(NVOD_REF_EVENT_MUTEX);
	return SUCCESS;

}


INT32 nvod_create_refevent_timelist(UINT16 ref_event_id, UINT16 ref_service_id, 
	UINT16 t_s_id, UINT16 *cnt)
{
	date_time dt1, dt2;
	UINT32 time;
	UINT8 h, m, s;
	UINT16 i;
	INT32 ret = SUCCESS;
	struct nvod_info *pinfo = nvod_param;
	struct nvod_event *pevent = NULL;
   
   
	if(pinfo==NULL)
		return -1;
	NVOD_PRINTF("\n%s(): start, tick[%d]\n",__FUNCTION__,osal_get_tick());
	ret = nvod_create_view(ref_event_id, ref_service_id, t_s_id);
	if(ret != SUCCESS)
		return ret;
	if(pinfo->view_node_cnt == 0)
	{
		*cnt = 0;
		NVOD_PRINTF("nvod no event schedule for ref event[%d] ref service[%d] !\n",ref_event_id,ref_service_id);
		return SUCCESS;
	}
	shellsort(pinfo->view_node_time, 0, pinfo->view_node_cnt-1,&pinfo->view[0]);

#if 0
	UINT16 y;
	UINT8 mon, d,week_day;
	UINT32 mjd;
	
	NVOD_PRINTF("after sort, ref event[%d] ref service[%d] timelist:\n",ref_event_id, ref_service_id);
	for(i = 0; i < pinfo->view_node_cnt; i++)
	{
		pevent = pinfo->view[i];
		mjd = (pevent->time[0]<<8)|pevent->time[1];
		bcd_to_hms(&(pevent->time[2]), &h, &m, &s);
		mjd_to_ymd(mjd, &y, &mon, &d, &week_day);
		NVOD_PRINTF("event[%d] sid[%d] [%d-%d-%d %d:%d:%d]\n", pevent->event_id,pevent->service_id,y,mon,d,h,m,s);
	}
#endif
	
	for(i = 0; i < pinfo->view_node_cnt; i++)
	{
            get_UTC(&dt1);
            pevent = pinfo->view[i];
            bcd_to_hms(&pevent->time[5], &h, &m, &s);
            convert_time_by_offset2(&dt2, &dt1, 0-h,0-m,0-s);
            //32 bit time format:16bit mjd, 5bit hour, 6bit min, 5bit second(unit 2s)
            time = (dt2.mjd<<16)|(dt2.hour<<11)|(dt2.min<<5)|(dt2.sec>>1);
            if(pinfo->view_node_time[i] >= time)
			break;
	}
	if(i==pinfo->view_node_cnt)
	{
		*cnt = pinfo->view_node_cnt = 0;
		NVOD_PRINTF("nvod all events in view start time+duration < UTC !\n");
		return -1;
	}
	MEMMOVE(&pinfo->view_node_time[0], &pinfo->view_node_time[i], sizeof(UINT32)*(pinfo->view_node_cnt-i));
	MEMMOVE(&pinfo->view[0], &pinfo->view[i], sizeof(struct nvod_event *)*(pinfo->view_node_cnt-i));
	pinfo->view_node_cnt -= i;
	*cnt = pinfo->view_node_cnt;
	NVOD_PRINTF("ref event[%d] ref service[%d] timelist has [%d] sechdule!\n",ref_event_id,ref_service_id,*cnt);
	NVOD_PRINTF("%s(): finished, tick[%d]\n",__FUNCTION__,osal_get_tick());
	return SUCCESS;
}


INT32 nvod_get_refevent_timelist(UINT16 start_pos, UINT16 cnt, struct nvod_event *buf, UINT16 *actual_cnt)
{
	UINT16 i;
	UINT16 count;
	struct nvod_info *pinfo = nvod_param;

	if(pinfo==NULL || cnt==0 || buf==NULL || actual_cnt==NULL)
		return -1;

	NVOD_ENTER_MUTEX(NVOD_EVENT_MUTEX);
	if(( pinfo->view_node_cnt==0)||(start_pos >= pinfo->view_node_cnt))
	{
		NVOD_LEAVE_MUTEX(NVOD_EVENT_MUTEX);
		*actual_cnt = 0;
		return -1;
	}
	if((start_pos+cnt) > pinfo->view_node_cnt)
		count = pinfo->view_node_cnt - start_pos;
	else
		count = cnt;
	*actual_cnt = count;
	for(i = 0; i < count; i++)
		MEMCPY(&buf[i], pinfo->view[start_pos+i], sizeof(struct nvod_event));
	NVOD_LEAVE_MUTEX(NVOD_EVENT_MUTEX);
	
	return SUCCESS;
}


INT32 nvod_get_cur_event(UINT16 service_id, UINT16 ref_service_id,
   UINT16 t_s_id,struct nvod_event *event)
{
    UINT16 i;
    UINT16 count;
    date_time utc_dt, event_dt;
    UINT8 duration_h, duration_m, duration_s;
    INT32 d_offset, h_offset, m_offset, s_offset;
    struct nvod_info *pinfo = nvod_param;
    struct nvod_event *pevent;
    INT32 ret = SUCCESS;
    INT32 tmp1 = -1;
    INT32 tmp2 = -1;

    if(pinfo==NULL || event==NULL)
        return -1;
    
    NVOD_ENTER_MUTEX(NVOD_EVENT_MUTEX);
    count = pinfo->event_cnt;
    pevent = &pinfo->event[0];
    for(i = 0; i < count; i++)
    {
        if((pevent[i].service_id==service_id)&&(pevent[i].t_s_id==t_s_id)
            &&(pevent[i].ref_service_id==ref_service_id))
        {
            event_dt.mjd = (pevent[i].time[0]<<8)|pevent[i].time[1];
            bcd_to_hms(&pevent[i].time[2], &event_dt.hour, &event_dt.min, &event_dt.sec);
            bcd_to_hms(&pevent[i].time[5], &duration_h, &duration_m, &duration_s);
            get_UTC(&utc_dt);
            d_offset = (INT32)(utc_dt.mjd - event_dt.mjd);
            h_offset = (INT8)(utc_dt.hour - event_dt.hour);
            m_offset = (INT8)(utc_dt.min - event_dt.min);
            s_offset = (INT8)(utc_dt.sec - event_dt.sec);
            if(d_offset >= 0)
            {
                tmp1 = (d_offset*24+h_offset)*60+m_offset;
            }
            //tmp1==0 means the event just begin
            if(tmp1 >= 0)
                tmp2 = duration_h*60+duration_m;
            if((tmp1>=0)&&(tmp1<tmp2))
                break;
        }

    }
    if(i < count)
        MEMCPY(event, &pevent[i], sizeof(struct nvod_event));
    else 
    {
        ret = -1;
        MEMSET(&event, 0, sizeof(struct nvod_event));
        NVOD_PRINTF("%s(): service_id[],ref_sid[%d] not find matched event!\n",__FUNCTION__,
        service_id, ref_service_id);
    }
    

    NVOD_LEAVE_MUTEX(NVOD_EVENT_MUTEX);

    return ret;
}


INT32 nvod_get_event_byid(UINT16 event_id, UINT16 service_id,   struct nvod_event *event)
{
    UINT16 i;
    UINT16 count;
    struct nvod_info *pinfo = nvod_param;
    struct nvod_event *pevent;
    INT32 ret = SUCCESS;
   
    if(pinfo==NULL || event==NULL)
        return -1;
    
    NVOD_ENTER_MUTEX(NVOD_EVENT_MUTEX);
    count = pinfo->event_cnt;
    pevent = &pinfo->event[0];
    for(i = 0; i < count; i++)
    {
        if((pevent[i].event_id==event_id)&&(pevent[i].service_id==service_id))
        {
            break;
        }

    }
    if(i < count)
        MEMCPY(event, &pevent[i], sizeof(struct nvod_event));
    else 
    {
        ret = -1;
        MEMSET(&event, 0, sizeof(struct nvod_event));
        NVOD_PRINTF("%s(): event_id[%d],service_id[%d] not find matched event!\n",__FUNCTION__,
        event_id, service_id);
    }
    

    NVOD_LEAVE_MUTEX(NVOD_EVENT_MUTEX);

    return ret;
}


/*sort reference event by ts_id and service_id*/
INT32 nvod_sort_ref_event()
{
	INT16 i = 0;
	struct nvod_info *info = nvod_param;
	UINT32 key_array[NVOD_MAX_REF_EVNET_CNT];
	
	if(info==NULL)
		return -1;
	if(info->ref_event_cnt == 0)
		return SUCCESS;
	
	NVOD_ENTER_MUTEX(NVOD_REF_EVENT_MUTEX);
	
	MEMSET(key_array, 0, sizeof(key_array));
	for(i=0; i<info->ref_event_cnt;i++)
		key_array[i] = (info->ref_event[i].t_s_id<<16)|info->ref_event[i].ref_service_id;
	
	ref_event_shellsort(key_array, 0, info->ref_event_cnt-1);
			
	NVOD_LEAVE_MUTEX(NVOD_REF_EVENT_MUTEX);
	return SUCCESS;

}

INT32 nvod_rearrange_event()
{
    date_time utc_time, event_time;
    UINT8 duration_h, duration_m, duration_s;
    INT32 d_offset, h_offset, m_offset, s_offset;
    INT32 tmp1 = 0;
    INT16 i, j;
    struct nvod_info *pinfo = nvod_param;
    UINT8 ref_event_flag[NVOD_MAX_REF_EVNET_CNT];

    if((nvod_module_state!=NVOD_STATUS_SCAN_OVER)||(pinfo==NULL))
    	return -1;
    MEMSET(ref_event_flag, 0, sizeof(ref_event_flag));
    get_UTC(&utc_time);
    NVOD_EVENT_PRINTF("UTC [%d-%d-%d] [%d:%d:%d]\n",utc_time.year,utc_time.month,utc_time.day,
		utc_time.hour,utc_time.min,utc_time.sec);
    NVOD_ENTER_MUTEX(NVOD_EVENT_MUTEX|NVOD_REF_EVENT_MUTEX);
    
    for(i = 0; i < pinfo->event_cnt; )
    {
        event_time.mjd = (pinfo->event[i].time[0]<<8)|pinfo->event[i].time[1];
        bcd_to_hms(&(pinfo->event[i].time[2]), &event_time.hour, &event_time.min, &event_time.sec);
        bcd_to_hms(&(pinfo->event[i].time[5]), &duration_h, &duration_m, &duration_s);
        d_offset = event_time.mjd - utc_time.mjd;
        h_offset = event_time.hour - utc_time.hour;
        m_offset = event_time.min - utc_time.min;
        tmp1 = (d_offset*24+h_offset+duration_h)*60+m_offset+duration_m;
		
        //event start time+duration < utc time
        if(tmp1 < 0)
        {
            NVOD_EVENT_PRINTF("event indx[%d], id[%d], ser_id[%d],ref_event_id[%d],ref_ser_id[%d]\n",
                i,pinfo->event[i].event_id,pinfo->event[i].service_id,pinfo->event[i].ref_event_id,pinfo->event[i].ref_service_id);
            NVOD_EVENT_PRINTF("event mjd[%d],start time[%d:%d:%d],len[%d:%d:%d], end before UTC!\n\n",
                event_time.mjd,event_time.hour,event_time.min,event_time.sec,duration_h,duration_m,duration_s);
            if(i!=(pinfo->event_cnt-1))
                MEMMOVE(&pinfo->event[i], &pinfo->event[i+1], (pinfo->event_cnt-1-i)*sizeof(struct nvod_event));
            pinfo->event_cnt--;
        }
        else
        {
#ifdef NVOD_SERVICE_ONE_DAY
			date_time lt, let;
			INT32 h, m, s;

			get_STC_offset(&h, &m, &s);
			convert_time_by_offset(&lt, &utc_time, h, m);
			mjd_to_ymd(event_time.mjd, &event_time.year, &event_time.month, &event_time.day, &event_time.weekday);
			convert_time_by_offset(&let, &event_time, h, m);
			
			if (lt.mjd != let.mjd)
			{
				if(i!=(pinfo->event_cnt-1))
					MEMMOVE(&pinfo->event[i], &pinfo->event[i+1], (pinfo->event_cnt-1-i)*sizeof(struct nvod_event));
				pinfo->event_cnt--;
			}
			else
#endif
            i++;
        }
    }
    //check if ref event has valid event
    for(i = 0; i < pinfo->ref_event_cnt; i++)
    {
        for(j = 0; j < pinfo->event_cnt; j++)
        {
            if((pinfo->ref_event[i].event_id==pinfo->event[j].ref_event_id)
                &&(pinfo->ref_event[i].ref_service_id==pinfo->event[j].ref_service_id)
                &&(pinfo->ref_event[i].t_s_id==pinfo->event[j].t_s_id))
            {
                ref_event_flag[i] = 1;
                break;
            }
        }
    }
    //delete those ref event who has no valid event
    for(i = 0; i < pinfo->ref_event_cnt;)
    {
        if(ref_event_flag[i] == 0)
        {
            if(i != (pinfo->ref_event_cnt-1))
            {    
                NVOD_EVENT_PRINTF("nvod ref_event id[%d],ref_ser_id[%d] no valid event!!!\n",
                    pinfo->ref_event[i].event_id,pinfo->ref_event[i].ref_service_id);
                MEMMOVE(&pinfo->ref_event[i], &pinfo->ref_event[i+1], (pinfo->ref_event_cnt-1-i)*sizeof(struct nvod_ref_event));
                MEMMOVE(&ref_event_flag[i], &ref_event_flag[i+1], (pinfo->ref_event_cnt-1-i)*sizeof(UINT8));
            }
            pinfo->ref_event_cnt--;
        }
        else
            i++;
    }

    NVOD_LEAVE_MUTEX(NVOD_EVENT_MUTEX|NVOD_REF_EVENT_MUTEX);

    #if 0
	NVOD_EVENT_PRINTF("After rearrange: \n");
	for(i=0;i<pinfo->ref_event_cnt;i++)
	{
	    NVOD_EVENT_PRINTF("nvod ref_event[%d] id[%d],ref_ser_id[%d]\n",i,
                    pinfo->ref_event[i].event_id,pinfo->ref_event[i].ref_service_id);
	}
	#endif
    
    return SUCCESS;
}    


/****************************************************************/


/****api for  play nvod service**********************************************/
/*
static void nvod_channel_act(struct nvod_cc_cmd *pCmd)
{
	INT32 ret = SUCCESS;
	struct io_param io_parameter;
	struct dmx_device *dmx_dev = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
   	struct deca_device *deca_dev = (struct deca_device *)dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
    	struct snd_device *snd_dev = (struct snd_device *)dev_get_by_id(HLD_DEV_TYPE_SND, 0);
    	struct nim_device *nim_dev = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
	struct vdec_device *vdec_dev = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);

	if(hde_get_mode()==VIEW_MODE_PREVIEW)
		pCmd->cmd_bits &= ~CC_CMD_CLOSE_VPO;
		
	if(pCmd->cmd_bits & CC_CMD_CLOSE_VPO)
	{
		ret = vpo_win_onoff((struct vpo_device *)dev_get_by_id(HLD_DEV_TYPE_DIS, 0),FALSE);
		if (ret != RET_SUCCESS)
			NVOD_PLAY_PRINTF("vpo off fail\n");
	}
	if(pCmd->cmd_bits & CC_CMD_STOP_AUDIO)
	{
		ret = deca_stop(deca_dev,0,ADEC_STOP_IMM);
		if (ret != RET_SUCCESS)
			NVOD_PLAY_PRINTF("deca_stop() fail\n");
	}
	if(pCmd->cmd_bits & CC_CMD_STOP_VIDEO)
	{
		ret = vdec_stop(vdec_dev,FALSE,FALSE);
		if (ret == RET_BUSY)
		{
			ret = vdec_stop(vdec_dev,FALSE,FALSE);
		}
		if (ret != RET_SUCCESS)
			NVOD_PLAY_PRINTF("vdec_stop() fail \n");
	}

	if(pCmd->cmd_bits & CC_CMD_STOP_DMX)
	{
		ret = dmx_io_control(dmx_dev,IO_STREAM_DISABLE,(UINT32)&io_parameter);
		if (ret != RET_SUCCESS)
			NVOD_PLAY_PRINTF("Dmx disable stream fail \n");
	}
	if(pCmd->cmd_bits & CC_CMD_FULL_FB)
	{
		if(pCmd->plogo == NULL)
		{
			// no logo ,just full black screen
			struct YCbCrColor tColor;
			tColor.uY = 0x10;
			tColor.uCb = 0x80;
			tColor.uCr = 0x80;
			vdec_io_control(vdec_dev, VDEC_IO_FILL_FRM, (UINT32)(&tColor));
		}
	}
	if(pCmd->cmd_bits & CC_CMD_SET_XPOND)
	{
		struct NIM_Channel_Change cc_param;
		MEMSET(&cc_param, 0, sizeof(cc_param));
		cc_param.freq = pCmd->frq;
		cc_param.sym = pCmd->sym;
		cc_param.modulation = pCmd->qam;
		nim_ioctl_ext(nim_dev, NIM_DRIVER_CHANNEL_CHANGE, (void *)(&cc_param));
	}
	if(pCmd->cmd_bits & CC_CMD_START_AUDIO)
	{
		ret = deca_start(deca_dev,0);
		if(ret != RET_SUCCESS)
			NVOD_PLAY_PRINTF("deca_start() fail \n");
	 	enum SndDupChannel channel;
		switch(pCmd->audio_chan)
		{
			case AUDIO_CH_L:
				channel = SND_DUP_L;
				break;
			case AUDIO_CH_R:
				channel = SND_DUP_R;
				break;
			case AUDIO_CH_MONO:
				channel = SND_DUP_MONO;
				break;
			case AUDIO_CH_STEREO:
			default:
				channel = SND_DUP_NONE;
		}
		snd_set_duplicate(snd_dev, channel);
	}

	if(pCmd->cmd_bits & CC_CMD_START_VIDEO)
	{
		ret = vdec_start(vdec_dev);
		if(ret != RET_SUCCESS)
			NVOD_PLAY_PRINTF("vdec_start() fail\n");
	}

	if(pCmd->cmd_bits & CC_CMD_START_DMX)
	{
		UINT16 PID_list[3];
		
		io_parameter.io_buff_in = (UINT8 *)PID_list;
		PID_list[0] = pCmd->video_pid;
		PID_list[1] = pCmd->audio_pid;
		PID_list[2] = pCmd->pcr_pid;
				
		ret = dmx_io_control(dmx_dev,IO_CREATE_AV_STREAM,(UINT32)&io_parameter);
	    	if(ret != RET_SUCCESS)
			NVOD_PLAY_PRINTF("dmx create AV stream fail\n");
	        deca_set_sync_mode(deca_dev, ADEC_SYNC_PTS);
	  		
	    	ret = dmx_io_control(dmx_dev,IO_STREAM_ENABLE,(UINT32)&io_parameter);
	    	if(ret != RET_SUCCESS)
			NVOD_PLAY_PRINTF("dmx enable stream fail\n");
	}
	MEMCPY(&nvod_cc_config, pCmd, sizeof(struct nvod_cc_cmd));

}

//direction 0 save tp eitbitmap info; 1 load tp eitbitmap info
static INT32 nvod_saveorload_tp_eitbitmap(UINT8 direction,T_NODE *tp)
{
	UINT16 i;
	struct nvod_info *pinfo = nvod_param;

	NVOD_ENTER_MUTEX(NVOD_TP_MUTEX);
	for(i = 0; i < pinfo->tp_cnt; i++)
		if((pinfo->tp[i].frq==tp->frq)&&(pinfo->tp[i].sym==tp->sym)
			&&(pinfo->tp[i].FEC_inner==tp->FEC_inner))
			break;
	
	if(i==pinfo->tp_cnt)
	{
		NVOD_LEAVE_MUTEX(NVOD_TP_MUTEX);
		NVOD_PRINTF("%s(): not find tp: frq[%d],sym[%d],qam[%d] in nvod tp array!\n",
			__FUNCTION__,tp->frq,tp->sym,tp->FEC_inner);
		return -1;
	}
	//save tp eitbitmap 
	if(direction==0)
	{	
		MEMCPY(&pinfo->tp_eitbitmap[i], &pinfo->nvod_tp, sizeof(struct nvod_tp));
		
	}
	//load tp and tp eitbitmap
	else
	{
		MEMCPY(&pinfo->nvod_tp, &pinfo->tp_eitbitmap[i], sizeof(struct nvod_tp));
		MEMCPY(&pinfo->cur_tp, &pinfo->tp[i], sizeof(T_NODE));
	}

	NVOD_LEAVE_MUTEX(NVOD_TP_MUTEX);
	
	return SUCCESS;
}


INT32 nvod_play_channel(UINT16 service_id)
{
	P_NODE prog;
	T_NODE tp;
	UINT16 i;
	struct nvod_info *pinfo = nvod_param;
	struct nvod_cc_cmd cc_cmd;
	
	if(pinfo==NULL || pinfo->status!=NVOD_STATUS_SCAN_OVER)
		return -1;
	//find the program
	NVOD_ENTER_MUTEX(NVOD_PROG_MUTEX);
	for(i = 0; i < pinfo->service_cnt; i++)
	{
		if(pinfo->service[i].prog_number==service_id)
			break;
	}
	if(i==pinfo->service_cnt)
	{
		NVOD_LEAVE_MUTEX(NVOD_PROG_MUTEX);
		return -1;
	}
	MEMCPY(&prog, &pinfo->service[i], sizeof(P_NODE));
	NVOD_LEAVE_MUTEX(NVOD_PROG_MUTEX);

	if(SUCCESS != get_tp_by_id(prog.tp_id, &tp))
		return -1;
	
	//set cc cmd
	MEMSET(&cc_cmd, 0, sizeof(cc_cmd));
	cc_cmd.ca_mode = prog.ca_mode;
	cc_cmd.av_flag = prog.av_flag;
	cc_cmd.audio_chan = prog.audio_channel;
	cc_cmd.volumn = prog.audio_volume<<2;//volumn*4
	cc_cmd.service_id = prog.prog_number;
	cc_cmd.video_pid = prog.video_pid;
	cc_cmd.pcr_pid = prog.pcr_pid;
	cc_cmd.audio_pid = prog.audio_pid[prog.cur_audio];
	cc_cmd.t_s_id = tp.t_s_id;
	cc_cmd.frq = tp.frq;
	cc_cmd.sym = tp.sym;
	cc_cmd.qam = tp.FEC_inner;
	cc_cmd.cmd_bits = 0;
	if((cc_cmd.frq!=nvod_cc_config.frq)||(cc_cmd.sym!=nvod_cc_config.sym)
		||(cc_cmd.qam!=nvod_cc_config.qam))
		cc_cmd.cmd_bits |= CC_CMD_SET_XPOND;

	cc_cmd.cmd_bits |= CC_CMD_FULL_FB|CC_CMD_CLOSE_VPO;
	cc_cmd.cmd_bits |= CC_CMD_STOP_AUDIO|CC_CMD_STOP_VIDEO|CC_CMD_STOP_DMX;
	cc_cmd.cmd_bits |= CC_CMD_START_AUDIO|CC_CMD_START_VIDEO|CC_CMD_START_DMX;

	//tp change, close eit filter
	if(cc_cmd.cmd_bits&CC_CMD_SET_XPOND)
	{
		sie_abort(PSI_EIT_PID, NULL);
		if(SUCCESS!=nvod_saveorload_tp_eitbitmap(0, &pinfo->cur_tp))
			return -1;
	}
	
	nvod_channel_act(&cc_cmd);
	
	//after tp tuning, start eit filter
	if(cc_cmd.cmd_bits&CC_CMD_SET_XPOND)
	{
		if(SUCCESS!=nvod_saveorload_tp_eitbitmap(1, &tp))
			return -1;
		nvod_start_eit(FALSE);
	}
}


INT32 nvod_stop_channel(UINT8 black_screen)
{
	if(nvod_param==NULL || nvod_param->status != NVOD_STATUS_SCAN_OVER)
		return -1;

	MEMSET(&nvod_cc_config, 0, sizeof(struct nvod_cc_cmd));
	nvod_cc_config.cmd_bits = CC_CMD_STOP_AUDIO|CC_CMD_STOP_VIDEO|CC_CMD_STOP_DMX;
						
	if(black_screen)
		    	nvod_cc_config.cmd_bits |= CC_CMD_FULL_FB;

	nvod_channel_act(&nvod_cc_config);
}


INT32 nvod_reset_cur_channel()
{
	struct nvod_info *pinfo = nvod_param;
	T_NODE tp;

	if(pinfo==NULL || pinfo->status!=NVOD_STATUS_SCAN_OVER)
		return -1;

	nvod_cc_config.cmd_bits |= CC_CMD_FULL_FB|CC_CMD_SET_XPOND;
	MEMSET(&tp, 0, sizeof(T_NODE));
	tp.frq = nvod_cc_config.frq;
	tp.sym = nvod_cc_config.sym;
	tp.FEC_inner = nvod_cc_config.qam;

	//tp change, close eit filter
	sie_abort(PSI_EIT_PID, NULL);
	if(SUCCESS!=nvod_saveorload_tp_eitbitmap(0, &pinfo->cur_tp))
		return -1;
		
	nvod_channel_act(&nvod_cc_config);
	
	//after tp tuning, start eit filter
	if(SUCCESS!=nvod_saveorload_tp_eitbitmap(1, &tp))
		return -1;
	nvod_start_eit(FALSE);
	

}


INT32 nvod_get_channel(UINT16 service_id, P_NODE *prog)
{
    UINT16 i;
	struct nvod_info *pinfo = nvod_param;
		
	if(pinfo==NULL || pinfo->status!=NVOD_STATUS_SCAN_OVER)
		return -1;
    MEMSET(prog, 0, sizeof(P_NODE));
	//find the program
	NVOD_ENTER_MUTEX(NVOD_PROG_MUTEX);
	for(i = 0; i < pinfo->service_cnt; i++)
	{
		if(pinfo->service[i].prog_number==service_id)
			break;
	}
	if(i==pinfo->service_cnt)
	{
		NVOD_LEAVE_MUTEX(NVOD_PROG_MUTEX);
		return -1;
	}
	MEMCPY(prog, &pinfo->service[i], sizeof(P_NODE));
	NVOD_LEAVE_MUTEX(NVOD_PROG_MUTEX);
}


INT32 nvod_modify_channel(UINT16 service_id, P_NODE *prog)
{
	UINT16 i;
	struct nvod_info *pinfo = nvod_param;

	if(pinfo==NULL || prog==NULL)
		return -1;
	//find the program
	NVOD_ENTER_MUTEX(NVOD_PROG_MUTEX);
	for(i = 0; i < pinfo->service_cnt; i++)
	{
		if(pinfo->service[i].prog_number==service_id)
			break;
	}
	if(i==pinfo->service_cnt)
	{
		NVOD_LEAVE_MUTEX(NVOD_PROG_MUTEX);
		return -1;
	}
	MEMCPY(&pinfo->service[i], prog, sizeof(P_NODE));
	NVOD_LEAVE_MUTEX(NVOD_PROG_MUTEX);
	
	return SUCCESS;
}

*/
    

/****************************************************************/



/****api for save nvod services into memory*********************************/
/*
static void nvod_psi2program(P_NODE *pg, PROG_INFO *info)
{
	INT32 i;
	MEMSET(pg,0,sizeof(P_NODE));

	pg->audio_volume = AUDIO_DEFAULT_VOLUME;
	pg->audio_count = info->audio_count;
	pg->audio_channel = AUDIO_CH_STEREO;

	MEMCPY(pg->audio_pid, info->audio_pid, info->audio_count*sizeof(info->audio_pid[0]));
	for(i=0;i<(INT32)info->audio_count;i++)
		get_audio_lang2b(info->audio_lang[i], (UINT8 *)&pg->audio_lang[i]);
	
	pg->av_flag = info->av_flag;

	
	if(info->CA_count >0) 
		pg->ca_mode = 1;
	
#ifdef DB_CAS_SUPPORT
		//cas count
		pg->cas_count = (info->CA_count < MAX_CAS_CNT)?info->CA_count : MAX_CAS_CNT;
		for(i = 0; i < pg->cas_count; i++)
			pg->cas_sysid[i] = info->CA_info[i].CA_system_id;
#endif	

 	pg->pmt_pid = info->pmt_pid;
	pg->pcr_pid = info->pcr_pid;
	pg->prog_number = info->prog_number;
	pg->service_type = info->service_type;
	pg->video_pid = info->video_pid;
	pg->teletext_pid = info->teletext_pid;
	pg->subtitle_pid = info->subtitle_pid;
	pg->sat_id = info->sat_id;
	pg->tp_id = info->tp_id;
	pg->nvod_sid = info->ref_sid;
	
}


INT32 nvod_save_program(PROG_INFO *info)
{
	P_NODE node;
	struct nvod_info *pinfo = nvod_param;
	UINT16 i;

	nvod_psi2program(&node, info);

	NVOD_ENTER_MUTEX(NVOD_PROG_MUTEX);
	if(pinfo->service_cnt >= NVOD_MAX_SERVICE_CNT)
	{
		NVOD_LEAVE_MUTEX(NVOD_PROG_MUTEX);
		NVOD_PRINTF("nvod service cnt to max number!\n");
		return -1;
	}
	
	for(i = 0; i < pinfo->service_cnt; i++)
		if(node.prog_number==pinfo->service[i].prog_number)
			break;
	if(i==pinfo->service_cnt)
	{
		MEMCPY(&pinfo->service[pinfo->service_cnt], &node, sizeof(P_NODE));
		pinfo->service_cnt++;
	}
	NVOD_LEAVE_MUTEX(NVOD_PROG_MUTEX);
	return SUCCESS;
}

*/

static void nvod_psi2program(P_NODE *pg, PROG_INFO *info)
{
	INT32 i;
	MEMSET(pg,0,sizeof(P_NODE));

	pg->audio_volume = AUDIO_DEFAULT_VOLUME;
	pg->audio_count = info->audio_count;
	//pg->audio_channel = AUDIO_CH_STEREO;
	//if(info->track != 0)
	//default audio tack AUDIO_CH_L==0
		pg->audio_channel = info->track;
	if(info->volume != 0)
		pg->audio_volume = info->volume;
#ifdef MULTI_BOUQUET_ID_SUPPORT
    pg->bouquet_count = 1;
    pg->bouquet_id[0] = info->bouquet_id;
#else        
	pg->bouquet_id = info->bouquet_id;
#endif
	pg->logical_channel_num = info->logical_channel_num;

	MEMCPY(pg->audio_pid, info->audio_pid, info->audio_count*sizeof(info->audio_pid[0]));
	for(i=0;i<(INT32)info->audio_count;i++)
		get_audio_lang2b(info->audio_lang[i], (UINT8 *)&pg->audio_lang[i]);
	
	pg->av_flag = info->av_flag;

	/* check CA descriptor */
	if(info->CA_count >0) 
		pg->ca_mode = 1;
	
#ifdef DB_CAS_SUPPORT
		//cas count
		pg->cas_count = (info->CA_count < MAX_CAS_CNT)?info->CA_count : MAX_CAS_CNT;
		for(i = 0; i < pg->cas_count; i++)
			pg->cas_sysid[i] = info->CA_info[i].CA_system_id;
#endif	

 	pg->pmt_pid = info->pmt_pid;
	pg->pcr_pid = info->pcr_pid;
	pg->prog_number = info->prog_number;
	pg->service_type = info->service_type;
	pg->video_pid = info->video_pid&0x1fff;;
	pg->teletext_pid = info->teletext_pid;
	pg->subtitle_pid = info->subtitle_pid;
	pg->sat_id = info->sat_id;
	pg->tp_id = info->tp_id;
	pg->nvod_sid = info->ref_sid;

	DB_STRCPY((DB_ETYPE *)pg->service_name,(DB_ETYPE *)info->service_name);
#if (SERVICE_PROVIDER_NAME_OPTION>0)
	DB_STRCPY((DB_ETYPE *)pg->service_provider_name,(DB_ETYPE *)info->service_provider_name);
#endif
	
}


INT32 nvod_save_program(PROG_INFO *info, T_NODE *cur_tp)
{
	P_NODE node;
	T_NODE tp;
	struct nvod_info *pinfo = nvod_param;
	UINT16 i;
	INT32 ret;

	if(pinfo==NULL)
		return -1;
	nvod_psi2program(&node, info);
	if(db_search_lookup_node(TYPE_PROG_NODE, (void *)&node)!=SUCCESS)
	{
		ret = add_node(TYPE_PROG_NODE, node.tp_id, (void *)&node);
		if ((DBERR_FLASH_FULL==ret) || (DBERR_MAX_LIMIT)==ret) 
			return SI_STOFULL;
	}
	get_tp_by_id(node.tp_id, &tp);
	NVOD_ENTER_MUTEX(NVOD_PROG_MUTEX);
	for(i = 0; i < pinfo->service_cnt; i++)
	{
		if((node.prog_number==pinfo->service[i].service_id)&&(tp.t_s_id==cur_tp->t_s_id))
		{
			pinfo->service[i].db_prog_id = node.prog_id;
			break;
		}
	}
	
	if((i==pinfo->service_cnt)&&(pinfo->service_cnt<NVOD_MAX_SERVICE_CNT))
	{
		pinfo->service[i].db_prog_id = node.prog_id;
		pinfo->service[i].service_id = node.prog_number;
		pinfo->service[i].t_s_id = cur_tp->t_s_id;
		pinfo->service_cnt++;
	}
	
	NVOD_LEAVE_MUTEX(NVOD_PROG_MUTEX);
	return SUCCESS;
}


//direction 0 save tp eitbitmap info; 1 load tp eitbitmap info
INT32 nvod_saveorload_tp_eit(UINT8 direction,T_NODE *tp)
{
	//save tp eitbitmap 
	if(direction==0)
		sie_abort(PSI_EIT_PID, NULL);
		
	saveorload_tp_eitinfo(direction, tp);	
	
	//load tp and tp eitbitmap
	if(direction==1)
	    nvod_start_eit(FALSE);
	
	return SUCCESS;
}



INT32 nvod_get_channel_progid(UINT16 service_id, UINT16 t_s_id,UINT32 *prog_id)
{
	struct nvod_info *pinfo = nvod_param;
	UINT16 i;
	INT32 ret;

	if(pinfo==NULL)
		return -1;
	NVOD_ENTER_MUTEX(NVOD_PROG_MUTEX);
	for(i = 0; i < pinfo->service_cnt; i++)
	{
		if((service_id==pinfo->service[i].service_id)&&(t_s_id==pinfo->service[i].t_s_id))
		{
			*prog_id = pinfo->service[i].db_prog_id;
			break;
		}
	}
	if(i==pinfo->service_cnt)
		*prog_id = 0xFFFFFFFF;
	NVOD_LEAVE_MUTEX(NVOD_PROG_MUTEX);

	return SUCCESS;
}

static BOOL buf_malloc=FALSE;
INT32 nvod_open(UINT8 mode, struct tp_param *t_param, nvod_callback callback, 
	nvod_eit_complete eit_complete)
{
    INT32 ret = SUCCESS;
    struct nvod_info *pinfo = nvod_param;
    struct vdec_device *vdec;
    struct vdec_io_get_frm_para vfrm_param;
    UINT32 addr=0, len=0;
    buf_malloc=FALSE;
    if((nvod_module_state<NVOD_STATUS_INITED)||(pinfo==NULL))
    	return -1;

#ifndef __MM_VBV_START_ADDR
/*	if(NULL == nvod_buf)
	{
		nvod_buf = MALLOC(NVOD_BUF_LEN);
	}*/
#ifdef DATA_BROADCAST
#if(DATA_BROADCAST==DATA_BROADCAST_IPANEL)
	nvod_buf = (UINT8*) IPANEL_MW_BUF_START;
#elif(DATA_BROADCAST==DATA_BROADCAST_ENREACH)
    nvod_buf =(UINT8*) ENREACH_BROWSER_BUF_START;
#else
    buf_malloc=TRUE;
    if(NULL == nvod_buf)
	{
		nvod_buf = MALLOC(NVOD_BUF_LEN);
	}
#endif
#else
    buf_malloc=TRUE;
    if(NULL == nvod_buf)
	{
		nvod_buf = MALLOC(NVOD_BUF_LEN);
	}
#endif
#else
	nvod_buf =(UINT8*) __MM_VBV_START_ADDR;
#endif	
	
    stop_tdt();
    enable_time_parse();
    if(callback != NULL)
    	pinfo->call_back = callback;
    if(eit_complete != NULL)
    	pinfo->eit_complete = eit_complete;
      if(nvod_module_state==NVOD_STATUS_INITED)
        nvod_module_state = NVOD_STATUS_FIRST_OPEN;

    struct as_service_param param;
    T_NODE tp;

    MEMSET(&tp, 0, sizeof(T_NODE));
    tp.ft_type = FRONTEND_TYPE_C;
    tp.sat_id = 1;
    tp.frq = t_param->freq;
    tp.sym = t_param->symbol;
    tp.FEC_inner = t_param->constellation;
    recreate_tp_view( VIEW_SINGLE_SAT, 1);
    ret = lookup_node(TYPE_TP_NODE, &tp, 1);
    if (ret != SUCCESS)
    {
    	add_node(TYPE_TP_NODE, 1, &tp);
    	update_data();
    }
    /*first open nvod, do search*/
    if(nvod_module_state==NVOD_STATUS_FIRST_OPEN)
    {
        MEMSET(&param, 0, sizeof(struct as_service_param));
        param.as_frontend_type = FRONTEND_TYPE_C;
        param.as_prog_attr = P_SEARCH_TV|P_SEARCH_FTA|P_SEARCH_SCRAMBLED|P_SEARCH_NVOD;
        param.as_p_add_cfg = PROG_ADD_REPLACE_OLD;
        param.as_method = AS_METHOD_NVOD;
        if(mode==NVOD_MODE_MULTI_TP)
        	param.as_method = AS_METHOD_NVOD_MULTI_TP;
        param.as_sat_cnt = 1;
        param.sat_ids[0] = tp.sat_id;
        param.as_from = tp.tp_id;
        param.as_to = 0;
        param.as_handler = nvod_as_event_handler;
        param.ft.c_param.sym = tp.sym;
        param.ft.c_param.constellation = tp.FEC_inner;
        /*why on hd service,can not get frm buf after stop play?
        vdec = (struct vdec_device *)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
        vfrm_param.ufrm_mode = VDEC_UN_DISPLAY;
        vdec_io_control(vdec, VDEC_IO_GET_FRM, (UINT32)&vfrm_param);
        addr = vfrm_param.tFrmInfo.uC_Addr;
        len = (vfrm_param.tFrmInfo.uHeight * vfrm_param.tFrmInfo.uWidth * 3) / 2;
        */
        if(nvod_search_buf==NULL)
        {
            nvod_search_buf = (UINT8*)MALLOC(0x20000);
            if(nvod_search_buf==NULL)
                return -1;
            addr = (UINT32)nvod_search_buf;
            len = 0x20000;
        }

        db_search_init((UINT8 *)addr, len);
        
        as_service_start(&param);
        nvod_module_state = NVOD_STATUS_IN_SCAN;
    }
    /*if scaned already, not search, only get eit*/
    else if(nvod_module_state==NVOD_STATUS_SCAN_OVER)
    {
		/* nvod_pre_pnode hold the last played nvod pnode, so we change to 
		 * the crosponding tp can always success.
		 */
		back_to_last_nvod_tp(&nvod_pre_pnode);

        recreate_prog_view(VIEW_ALL|PROG_NVOD_MODE, 0);
        start_tdt();
        nvod_saveorload_tp_eit(1, &tp);
        if(pinfo->call_back)
		pinfo->call_back(NVOD_DIRECT_UPDATE_EVENT, 0);
    }
    return SUCCESS;
}


INT32 nvod_close()
{
#ifndef __MM_VBV_START_ADDR
        if(buf_malloc&&NULL != nvod_buf)
        {
            FREE(nvod_buf);
            nvod_buf = NULL;
        }        	
		nvod_buf = NULL;
#endif

    if(nvod_module_state<=NVOD_STATUS_FIRST_OPEN)
        return SUCCESS;
    //close eit filter
    sie_abort(PSI_EIT_PID, NULL);
    if(nvod_module_state==NVOD_STATUS_IN_SCAN)
        as_service_stop();
    /*nvod scan not complete, clear got nvod info*/
    if(nvod_module_state!=NVOD_STATUS_SCAN_OVER)
    {
        nvod_module_state = NVOD_STATUS_INITED;
        //clear nvod info
        clear_nvod_info();
    }
    
    return SUCCESS;
}


/****add functions to find nvod tp from main tp's NIT**********************/
static UINT32 nit_all_sec_value = 0;
static UINT32 nit_hit_map = 0;
static UINT32 nit_parse_hitmap = 0;
struct nit_section_info *nit_sec_info = NULL;


static struct desc_table nit_loop2[] = {
	{
		.tag = SERVICE_LIST_DESCRIPTOR,
		.bit = 0,
		.parser = nit_on_service_list_desc,
	},
	{
		.tag = CABLE_DELIVERY_SYSTEM_DESCRIPTOR,
		.bit = 0,
		.parser = si_on_cab_delivery_desc,
	},
};



static BOOL nit_event(UINT16 pid, struct si_filter_t *filter,UINT8 reason, UINT8 *data, INT32 len)
{
     //section already hit
    if(((1<<data[6])&nit_hit_map) != 0)
        return FALSE;
    else
    {
        nit_hit_map |= (1<<data[6]);
        return TRUE;
    }
}


static sie_status_t nit_handler(UINT16 pid, struct si_filter_t *filter, UINT8 reason, UINT8 *data, INT32 len)
{
    INT8 i;
    struct section_parameter sec_param; 
    sie_status_t ret = sie_started;

    
    if(reason == SIE_REASON_FILTER_TIMEOUT)
    {
        NVOD_PRINTF("NVOD get NIT time out\n");
        NVOD_LEAVE_MUTEX(NVOD_NIT_MUTEX);
        ret = sie_freed;
    }
    else if(data != NULL)
    {
        if(nit_all_sec_value==0)
        {
            nit_all_sec_value = 1;
            for(i = 0; i <= data[7]; i++)
                nit_all_sec_value = (1<<i) | 1;
		//nit section 1st hit, clear buf for tp info
            nit_sec_info = (struct nit_section_info *)nvod_buf;	
            MEMSET(nit_sec_info, 0, sizeof(struct nit_section_info));			
        }
      
        if(nit_sec_info != NULL)
        {
            MEMSET(&sec_param, 0, sizeof(struct section_parameter));
            nit_sec_info->lp1_nr = 0;
            nit_sec_info->lp2_nr = 2;
            nit_sec_info->loop2 = nit_loop2;
            sec_param.priv = (void *)nit_sec_info;
            si_nit_parser(data, len, &sec_param);
            //all section hit
            if(nit_hit_map==nit_all_sec_value)
            {
		NVOD_LEAVE_MUTEX(NVOD_NIT_MUTEX);
                ret = sie_freed;
            }
        }
    }
    return ret;
}


static INT32 nvod_get_nit(UINT8 *buf, UINT16 buf_len)
{
    INT32 ret;
    struct si_filter_t *filter = NULL;
    struct si_filter_param fparam;
    
      filter = sie_alloc_filter(PSI_NIT_PID, buf, buf_len, PSI_SHORT_SECTION_LENGTH);
	if(NULL == filter)
	{
		NVOD_PRINTF("%s(): alloc filter failed!\n", __FUNCTION__);
		return -1;
	}
	MEMSET(&fparam, 0, sizeof(fparam));
	fparam.timeout = 10000;
	fparam.attr[0] = SI_ATTR_HAVE_CRC32;
	fparam.mask_value.tb_flt_msk = 0x01;
	fparam.mask_value.mask_len = 6;
	fparam.mask_value.multi_mask[0][0] = 0xFF;
	fparam.mask_value.multi_mask[0][1] = 0x80;
	fparam.mask_value.multi_mask[0][5] = 0x01;
	fparam.mask_value.value[0][0] = PSI_NIT_TABLE_ID;
	fparam.mask_value.value[0][1] = 0x80;
	fparam.mask_value.value[0][5] = 0x01;
		
	fparam.section_event = nit_event;
	fparam.section_parser = nit_handler;
	sie_config_filter(filter, &fparam);
	ret = sie_enable_filter(filter);
	if(ret != SI_SUCCESS)
	{
		NVOD_PRINTF("%s(): enable filter failed!\n", __FUNCTION__);
		return -1;
	}
	NVOD_PRINTF("%s(): tick=%d!\n", __FUNCTION__,osal_get_tick());
	return SUCCESS;
}


sie_status_t nvod_nit_parse(UINT16 pid, struct si_filter_t *filter, UINT8 reason, UINT8 *data, INT32 len)
{
	UINT16 i, j;
	struct section_parameter sec_param; 
	sie_status_t ret = sie_started;
	struct nit_section_info *nit_sec_param = NULL;	

	if((data==NULL)||(reason==SIE_REASON_FILTER_TIMEOUT))
	{
	    return sie_freed;
	}
	if(nit_all_sec_value==0)
	{
		nit_all_sec_value = 1;
		for(i = 0; i <= data[7]; i++)
			nit_all_sec_value = (1<<i) | 1;
		
		//nit section 1st hit, clear buf for tp info
		nit_sec_param = (struct nit_section_info *)nvod_buf;	
		MEMSET(nit_sec_param, 0, sizeof(struct nit_section_info));	
	}
	//this section already parsed
	if(((1<<data[6])&nit_parse_hitmap) != 0)
		return ret;
	else
	{
		nit_parse_hitmap |= (1<<data[6]);
		
	}
	//loval variable nit_sec_param need to be set value if nit has multi-section
	nit_sec_param = (struct nit_section_info *)nvod_buf;	
	MEMSET(&sec_param, 0, sizeof(struct section_parameter));
	nit_sec_param->lp1_nr = 0;
	nit_sec_param->lp2_nr = 2;
	nit_sec_param->loop2 = nit_loop2;
	sec_param.priv = (void *)nit_sec_param;
	si_nit_parser(data, len, &sec_param);
	//all section hit
	if(nit_parse_hitmap==nit_all_sec_value)
	{
		T_NODE tp;
		UINT16 tpcnt;
		
		 if(nit_sec_param->xp_nr==0)
		{
			NVOD_PRINTF("%s(): in nit  tp cnt=0!\n",__FUNCTION__);
			return sie_freed;
		}
		recreate_tp_view(VIEW_SINGLE_SAT, 1);
		tpcnt = get_tp_num_sat(1);
		 /*check old nvod tp, clear nvod flag*/
		for(i = 0; i < tpcnt; i++)
		{
		   if(get_tp_by_pos(i, &tp)==SUCCESS)
		       if(tp.nvod_flg==1)
		       {
		           tp.nvod_flg = 0;
		           modify_tp(tp.tp_id, &tp);
		       }
		}
		//update nvod tp into db
		for(i=0; i < nit_sec_param->xp_nr; i++)
		{
		    for(j=0; j<nit_sec_param->nvod_nr; j++)
		    {
		        if (nit_sec_param->xp[i].common.tsid == nit_sec_param->nvod_tsid[j]) 
		        {
		            	MEMSET(&tp, 0, sizeof(tp));
				tp.sat_id = 1;
		                tp.ft_type = FRONTEND_TYPE_C;
		                tp.t_s_id = nit_sec_param->xp[i].c_info.tsid;
		                tp.network_id = nit_sec_param->xp[i].c_info.onid;
		                tp.frq = nit_sec_param->xp[i].c_info.frequency;
		                tp.sym = nit_sec_param->xp[i].c_info.symbol_rate;
		                tp.FEC_inner = nit_sec_param->xp[i].c_info.modulation;
		                tp.nvod_flg = 1;
		                if(SUCCESS != lookup_node(TYPE_TP_NODE, &tp, 1))
		                {
		                    add_node(TYPE_TP_NODE, 1, &tp);
		                 }
		                else
		                    modify_tp(tp.tp_id, &tp);
		            	break;
		        }
		    }
		}
		update_data();
		
		ret = sie_freed;
	}

	return ret;
}


INT32 nvod_init(UINT32 frq, UINT32 symbol, UINT8 qam)
{
    struct nim_device *nim;
    UINT16 i, j;
    UINT8 lock;
    UINT8 *nit_buf;
    struct nit_section_info *nit_info_buf;
    T_NODE tp;
    UINT16 tpcnt;
    UINT32 time_out = osal_get_tick()+1000;

    if((nvod_param!=NULL)&&(nvod_module_state>=NVOD_STATUS_INITED))
        return SUCCESS;
    

    /*create nvod flag */
    if(nvod_flg_id == OSAL_INVALID_ID)
    {
        nvod_flg_id = osal_flag_create(0);
        if(nvod_flg_id == OSAL_INVALID_ID)
        {
        	NVOD_PRINTF("NVOD create flag failed\n");
        	return !SUCCESS;
        }
    }
       
    osal_flag_clear(nvod_flg_id, 0xffffffff);
    osal_flag_set(nvod_flg_id, NVOD_VIEW_MUTEX|NVOD_REF_EVENT_MUTEX
		|NVOD_EVENT_MUTEX|NVOD_PROG_MUTEX|NVOD_TP_MUTEX);
#if 0       
    /*tune nim to main tp, check if lock*/
    nim = dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
    if(nim==NULL)
        return !SUCCESS;
    nim_channel_change(nim, frq, symbol, qam);
    while(osal_get_tick()<time_out)
    {
        lock = 0;
        nim_get_lock(nim, &lock);
        if(lock) break;
        else osal_task_sleep(2);
    }
    if(lock==0)
    {
        NVOD_PRINTF("%s(), try 1second nim unlock at frq[%d],sym[%d],qam[%d]!!!\n",
            __FUNCTION__,frq,symbol,qam);
        return !SUCCESS;
    }
    
    /*init parameter for nit*/
    nit_all_sec_value = 0;
    nit_hit_map = 0;
    nit_parse_hitmap = 0;	
    nit_sec_info = NULL;
    nit_buf = (UINT8 *)MALLOC(PSI_SHORT_SECTION_LENGTH);
    
    if(nit_buf==NULL)
    {
        NVOD_PRINTF("%s(), malloc memory failed!!!\n");
        return !SUCCESS;
    }
    
    /*get nit on main tp*/
    nvod_get_nit(nit_buf, PSI_SHORT_SECTION_LENGTH);
    NVOD_ENTER_MUTEX(NVOD_NIT_MUTEX);
    osal_flag_clear(nvod_flg_id, NVOD_NIT_MUTEX);
       
    /*check old nvod tp, clear nvod flag*/
       recreate_tp_view(VIEW_SINGLE_SAT, 1);
       tpcnt = get_tp_num_sat(1);
       for(i = 0; i < tpcnt; i++)
       {
           if(get_tp_by_pos(i, &tp)==SUCCESS)
               if(tp.nvod_flg==1)
               {
                   tp.nvod_flg = 0;
                   modify_tp(tp.tp_id, &tp);
               }
        }
	
    nit_info_buf = (struct nit_section_info *)nvod_buf;	
    //after parsed all nit sections, add nvod tp into db
    for(i=0; i < nit_info_buf->xp_nr; i++)
    {
        for(j=0; j<nit_info_buf->nvod_nr; j++)
        {
            if (nit_info_buf->xp[i].common.tsid == nit_info_buf->nvod_tsid[j]) 
            {
                	MEMSET(&tp, 0, sizeof(tp));
                    tp.ft_type = FRONTEND_TYPE_C;
                    tp.t_s_id = nit_info_buf->xp[i].c_info.tsid;
                    tp.network_id = nit_info_buf->xp[i].c_info.onid;
                    tp.frq = nit_info_buf->xp[i].c_info.frequency;
                    tp.sym = nit_info_buf->xp[i].c_info.symbol_rate;
                    tp.FEC_inner = nit_info_buf->xp[i].c_info.modulation;
                    tp.nvod_flg = 1;
                    tp.sat_id = 1;
                    if(SUCCESS != lookup_node(TYPE_TP_NODE, &tp, 1))
                    {
                        add_node(TYPE_TP_NODE, 1, &tp);
                        
                    }
                    else
                        modify_tp(tp.tp_id, &tp);
                	break;
            }
        }
    }


    update_data();
   /*free nit related buffer*/
    if(nit_buf!=NULL)
        FREE(nit_buf);
   
#endif
   
    /*malloc memory for nvod info*/
    if(nvod_param==NULL)
        nvod_param = (struct nvod_info *)MALLOC(sizeof(struct nvod_info));
    if(nvod_param == NULL)
    {
    	NVOD_PRINTF("NVOD alloc memory failed\n");
    	return !SUCCESS;
    }
    MEMSET((UINT8 *)nvod_param, 0, sizeof(struct nvod_info));
    nvod_module_state = NVOD_STATUS_INITED;
    return SUCCESS;
}


INT32 nvod_release()
{
    if(nvod_flg_id != OSAL_INVALID_ID)
    {
        osal_flag_delete(nvod_flg_id);
        nvod_flg_id = OSAL_INVALID_ID;
    }

    if(nvod_param != NULL)
    {
        FREE(nvod_param);
        nvod_param = NULL;
    }
    
    nvod_module_state = NVOD_STATUS_NULL;
    return SUCCESS;

}


/*return value: 0: nvod tp exist and not change, means all nvod tp in database are scaned 
				   already by nvod module;
			 	1: nvod tp changed, means one or more nvod tp in database not scaned by 
			  	   nvod module, nvod module need to be released and init again;			 
			   -1: no nvod tp in database;
*/
INT32 nvod_get_tp_entry(T_NODE *tp)
{
	UINT16 i, tpcnt, db_nvod_tpcnt;
	T_NODE db_tp, nvod_tp;
	UINT16 j, scaned_tp_cnt;
	struct nvod_info *pinfo = nvod_param;

	if((nvod_module_state<NVOD_STATUS_INITED)||(pinfo==NULL))
    		return -1;
	
	scaned_tp_cnt = pinfo->tp_cnt;
	        
	tpcnt = get_tp_num_sat(1);
	db_nvod_tpcnt = 0;
	for(i = 0; i < tpcnt; i++)
	{
		if(get_tp_by_pos(i, &db_tp)==SUCCESS)
		if(db_tp.nvod_flg==1)
		{
			db_nvod_tpcnt++;
			if(scaned_tp_cnt==0)
			{
				/* means we never launched a nvod search,it's the first nvod search,
                 * and later nvod_open need the tp not NULL, so here we return the first tp in db
				 */
				MEMCPY(tp, &db_tp, sizeof(T_NODE));
				return 1;	
			}
			for(j=0; j < scaned_tp_cnt; j++)
			{
				NVOD_ENTER_MUTEX(NVOD_TP_MUTEX);
				MEMCPY(&nvod_tp, &pinfo->tp[j], sizeof(T_NODE));
				NVOD_LEAVE_MUTEX(NVOD_TP_MUTEX);
				//this tp already scaned by nvod module
				if((db_tp.frq==nvod_tp.frq)&&(db_tp.FEC_inner==nvod_tp.FEC_inner))
				{
					break;
				}
			}
			MEMCPY(tp, &db_tp, sizeof(T_NODE));
			//this nvod tp not scaned, means new nvod tp
			if((scaned_tp_cnt>0)&&(j==scaned_tp_cnt))
			{
				return 1;		
			}
			
		}
	}
	if((i==tpcnt)&&(db_nvod_tpcnt==0))
	{
		MEMSET(tp, 0, sizeof(T_NODE));
		return -1;
	}
	return 0;
}

INT32 nvod_get_module_status()
{
	return nvod_module_state;
}

void back_to_last_nvod_tp(P_NODE * pnode)
{
	T_NODE t_node;
	struct nim_device *nim_dev = (struct nim_device*)dev_get_by_id(HLD_DEV_TYPE_NIM, 0);

	if(get_tp_by_id(pnode->tp_id, &t_node)==SUCCESS)
		nim_channel_change(nim_dev, t_node.frq, t_node.sym, t_node.FEC_inner);


}


/****patch for Xuyi special case*******************/
#if 0
//create current service event list and all services now playing event
INT32 nvod_create_playing_eventlist()
{
	date_time utc_time, event_time;
	INT32 d_offset, h_offset, m_offset;
	INT32 tmp1 = 0;
	INT16 i, j;
	struct nvod_info *pinfo = nvod_param;
	struct nvod_event *pevent = NULL;

	if((nvod_module_state!=NVOD_STATUS_SCAN_OVER)||(pinfo==NULL))
		return -1;
    MEMSET(pinfo->playing_event_list, 0, sizeof(struct nvod_event *)*NVOD_MAX_SERVICE_CNT);
	NVOD_EVENT_PRINTF("\n\n%s(), tick=%d\n",__FUNCTION__,osal_get_tick());
	get_UTC(&utc_time);
	NVOD_EVENT_PRINTF("UTC [%d-%d-%d] [%d:%d:%d]\n",utc_time.year,utc_time.month,utc_time.day,
		utc_time.hour,utc_time.min,utc_time.sec);
	NVOD_ENTER_MUTEX(NVOD_EVENT_MUTEX|NVOD_PROG_MUTEX);

	for(i = 0; i < pinfo->event_cnt; i++)
	{
        event_time.mjd = (pinfo->event[i].time[0]<<8)|pinfo->event[i].time[1];
        bcd_to_hms(&(pinfo->event[i].time[2]), &event_time.hour, &event_time.min, &event_time.sec);
        d_offset = event_time.mjd - utc_time.mjd;
        h_offset = event_time.hour - utc_time.hour;
        m_offset = event_time.min - utc_time.min;
        tmp1 = (d_offset*24+h_offset)*60+m_offset;
	    pevent = &pinfo->event[i];
        //start time < utc time, this event now playing
        if(tmp1 < 0)
	    {
            NVOD_EVENT_PRINTF("event index[%d],id[%d], ser_id[%d],ref_event_id[%d],ref_ser_id[%d]\n",
			i,pevent->event_id,pevent->service_id,pevent->ref_event_id,pevent->ref_service_id);
		    NVOD_EVENT_PRINTF("event mjd[%d],start time[%d:%d:%d] now playing!\n\n",
			event_time.mjd,event_time.hour,event_time.min,event_time.sec);
            for(j=0; j<pinfo->service_cnt; j++)
		    {
            if(pevent->service_id==pinfo->service[j].service_id)
				pinfo->playing_event_list[j] = pevent;
		    }
	    }
	    //else start time > utc, this event not start	
	
    }

	NVOD_LEAVE_MUTEX(NVOD_EVENT_MUTEX|NVOD_PROG_MUTEX);
	NVOD_EVENT_PRINTF("%s() finish, tick=%d\n\n",__FUNCTION__,osal_get_tick());
	return SUCCESS;
}


INT32 nvod_create_cur_service_eventlist(UINT16 cur_service_id)
{
	INT16 i, j;
	struct nvod_info *pinfo = nvod_param;
	struct nvod_event *pevent = NULL;
	UINT16 mjd, hms;
	UINT8 h,m,s;

	if((nvod_module_state!=NVOD_STATUS_SCAN_OVER)||(pinfo==NULL))
		return -1;
	NVOD_EVENT_PRINTF("\n\n%s(), tick=%d\n",__FUNCTION__,osal_get_tick());
    pinfo->cur_service_event_cnt = 0;
    MEMSET(pinfo->cur_service_event_list, 0, sizeof(struct nvod_event *)*NVOD_MAX_REF_EVNET_CNT);
    MEMSET(pinfo->cur_service_event_time, 0, sizeof(UINT32)*NVOD_MAX_REF_EVNET_CNT);
	NVOD_ENTER_MUTEX(NVOD_EVENT_MUTEX);

	for(i = 0; i < pinfo->event_cnt; i++)
	{    
	pevent = &pinfo->event[i];
	//put current service's event into cur_service_event_list, according to its reference event 
	if(pevent->service_id==cur_service_id)
	{
		//check if this event is already in cur service event list, maybe unnecessary
		/*for(j=0;j<pinfo->cur_service_event_cnt;j++)
		{
			if((pevent->ref_event_id==pinfo->cur_service_event_list[j]->ref_event_id)
				&&(pevent->ref_service_id==pinfo->cur_service_event_list[j]->ref_service_id))
				break;
		}
		
		//if this event's reference event is a new one, add this event to cur_service_event_cnt
		if(j==pinfo->cur_service_event_cnt)*/
		{
			NVOD_EVENT_PRINTF("event id[%d], ser_id[%d],ref_event_id[%d],ref_ser_id[%d] add to event list\n",
			pevent->event_id,pevent->service_id,pevent->ref_event_id,pevent->ref_service_id);
			
			//coded event time to uint32
			mjd = (pevent->time[0]<<8)|pevent->time[1];
			bcd_to_hms(&(pevent->time[2]), &h, &m, &s);
			//format:hour 5bit, min 6bit, second 5bit(unit 2 second)
			hms = (h<<11)|(m<<5)|(s>>1);
			pinfo->cur_service_event_time[pinfo->cur_service_event_cnt] = (mjd<<16)|hms;
			NVOD_EVENT_PRINTF("event start_time[mjd=%d] [%d:%d:%d]\n",mjd,h,m,s>>1);
			
			pinfo->cur_service_event_list[pinfo->cur_service_event_cnt] = pevent;
			pinfo->cur_service_event_cnt++;
		}
	}
    }
	//sort these event by start time
	shellsort(pinfo->cur_service_event_time, 0, pinfo->cur_service_event_cnt-1,&pinfo->cur_service_event_list[0]);
	#if 0
	NVOD_EVENT_PRINTF("After sort: \n");
	for(i=0;i<pinfo->cur_service_event_cnt;i++)
	{
		UINT32 time = pinfo->cur_service_event_time[i];
		pevent = pinfo->cur_service_event_list[i];
		NVOD_EVENT_PRINTF("event id[%d], ser_id[%d],ref_event_id[%d],ref_ser_id[%d]\n",
			pevent->event_id,pevent->service_id,pevent->ref_event_id,pevent->ref_service_id);
		NVOD_EVENT_PRINTF("event start_time[mjd=%d] [%d:%d:%d]\n",time>>16,
			(time&0xf800)>>11,(time&0x07e0)>>5,time&0x001f);
	}
	#endif
	NVOD_LEAVE_MUTEX(NVOD_EVENT_MUTEX);
	NVOD_EVENT_PRINTF("%s() finish, tick=%d\n\n",__FUNCTION__,osal_get_tick());
	return SUCCESS;
}


//get current service's event list's reference event count
UINT16 nvod_get_cur_service_eventlist_cnt()
{
	struct nvod_info *info = nvod_param;
	
	if(info==NULL)
		return -1;
	return info->cur_service_event_cnt;
}

//get current service's event by pos, then can get its reference event use nvod_get_ref_event_byid()
INT32 nvod_get_cur_service_event(UINT16 pos, struct nvod_event *event)
{
	struct nvod_info *info = nvod_param;
	
	if(event==NULL || info==NULL ||pos>= info->cur_service_event_cnt)
		return -1;
	
	NVOD_ENTER_MUTEX(NVOD_EVENT_MUTEX);
	MEMCPY(event, info->cur_service_event_list[pos],sizeof(struct nvod_event));
	NVOD_LEAVE_MUTEX(NVOD_EVENT_MUTEX);
	return SUCCESS;

}


//get current playing event , then can get its reference event use nvod_get_ref_event_byid()
INT32 nvod_get_playing_event(UINT16 service_id, struct nvod_event *event)
{
	struct nvod_info *info = nvod_param;
    INT16 i;
	
	if(event==NULL || info==NULL)
		return -1;

    for(i=0;i<info->service_cnt;i++)
    {
        if(service_id==info->playing_event_list[i]->service_id)
            break;
    }
    if(i==info->service_cnt)
        return -1;
    
	NVOD_ENTER_MUTEX(NVOD_EVENT_MUTEX);
	MEMCPY(event, info->playing_event_list[i],sizeof(struct nvod_event));
	NVOD_LEAVE_MUTEX(NVOD_EVENT_MUTEX);
	return SUCCESS;

}

#endif

