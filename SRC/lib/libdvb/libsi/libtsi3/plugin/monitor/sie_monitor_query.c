#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libc/alloc.h>

#include <api/libtsi/sie.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/si_section.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libsi/si_module.h>
#include <api/libsi/desc_loop_parser.h>
#include "sie_monitor_core.h"
#include <api/libsi/sie_monitor.h>
#ifdef DYNAMIC_SERVICE_SUPPORT
#include <api/libsi/si_sdt.h>
#endif

extern OSAL_ID sm_semaphore;
extern struct DMX_PSI_INFO dmx_psi_info[DMX_COUNT];

extern UINT8 monitor_exist[MAX_SIM_COUNT];
extern struct sim_cb_param simcb_array[MAX_SIM_COUNT];
extern struct restrict mask_array[MAX_SIM_COUNT];


UINT32 sim_get_prog_pmt_pid(UINT32 monitor_id, UINT16 prog_num, UINT16 *pmt_pid)
{
	INT16 dmx_idx;
	UINT32 match_count = 0;
	UINT16 i;

	if( !monitor_id_valid(monitor_id) )
		return 0;

	if( monitor_exist[monitor_id] != 1)
		return 0;

	if(simcb_array[monitor_id].table != MONITE_TB_PAT)
		return 0;
	
	dmx_idx = simcb_array[monitor_id].dmx_idx;

	for(i=0; i<dmx_psi_info[dmx_idx].pat.map_counter; i++)
	{
		if(prog_num == dmx_psi_info[dmx_idx].pat.map[i].pm_number)
		{
			*pmt_pid = dmx_psi_info[dmx_idx].pat.map[i].pm_pid;
			match_count++;
		}
	}	

	return match_count;
}

INT32 sim_get_ca_info(UINT32 monitor_id, CA_INFO *ca_ptr, UINT8 *max_ca_count)
{
	INT16 dmx_idx;
	UINT16 prog_number;
	UINT8 tbl_idx = 0xFF;

	if( !monitor_id_valid(monitor_id) )
		return ERR_FAILUE;

	if( monitor_exist[monitor_id] != 1)
		return ERR_FAILUE;

	dmx_idx = simcb_array[monitor_id].dmx_idx;
	prog_number = simcb_array[monitor_id].param;
	
	tbl_idx = get_table_index(dmx_idx, MONITE_TB_PMT, prog_number);
	if(tbl_idx==0xFF)
		return ERR_FAILUE;

	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

	if(ca_ptr!=NULL && *max_ca_count>0)
	{
		if(*max_ca_count > dmx_psi_info[dmx_idx].pmt[tbl_idx].CA_count)
		{
			*max_ca_count = dmx_psi_info[dmx_idx].pmt[tbl_idx].CA_count;
		}

		MEMCPY(ca_ptr, dmx_psi_info[dmx_idx].pmt[tbl_idx].CA_info,(*max_ca_count)*sizeof(CA_INFO));
	}
	else
	{
		*max_ca_count = dmx_psi_info[dmx_idx].pmt[tbl_idx].CA_count;
	}

	osal_semaphore_release(sm_semaphore);


	return SUCCESS;
}

INT32 sim_get_ecm_pid(UINT32 monitor_id, UINT16 *pid_buffer_ptr, UINT8 *max_ecm_count)
{
	INT16 dmx_idx;
	UINT16 prog_number;
	UINT8 tbl_idx = 0xFF;
	int i = 0;

	if( !monitor_id_valid(monitor_id) )
		return ERR_FAILUE;

	if( monitor_exist[monitor_id] != 1)
		return ERR_FAILUE;

	dmx_idx = simcb_array[monitor_id].dmx_idx;
	prog_number = simcb_array[monitor_id].param;
	
	tbl_idx = get_table_index(dmx_idx, MONITE_TB_PMT, prog_number);
	if(tbl_idx==0xFF)
		return ERR_FAILUE;

	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
	
	if(pid_buffer_ptr!=NULL && *max_ecm_count>0)
	{
		for(i=0; i<dmx_psi_info[dmx_idx].pmt[tbl_idx].CA_count && i<*max_ecm_count; i++)
		{
			pid_buffer_ptr[i] = dmx_psi_info[dmx_idx].pmt[tbl_idx].CA_info[i].CA_pid;
		}
		*max_ecm_count = i;
	}
	else
	{
		*max_ecm_count = dmx_psi_info[dmx_idx].pmt[tbl_idx].CA_count;
	}

	osal_semaphore_release(sm_semaphore);
	
	return SUCCESS;
}

INT32 sim_get_emm_pid(UINT32 monitor_id, UINT16 *pid_buffer_ptr, UINT8 *max_emm_count)
{
	UINT8 i = 0;
	INT32 dmx_idx = -1;

	if( !monitor_id_valid(monitor_id) )
		return ERR_FAILUE;

	if( monitor_exist[monitor_id] != 1)
		return ERR_FAILUE;

	//dmx_idx = (monitor_id&DMX_INDEX_MASK)>>24;
	dmx_idx = simcb_array[monitor_id].dmx_idx;

	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
	if(pid_buffer_ptr!=NULL && *max_emm_count>0)
	{
		for(i=0; i<dmx_psi_info[dmx_idx].cat.emm_count && i<*max_emm_count; i++)
		{
			pid_buffer_ptr[i] = dmx_psi_info[dmx_idx].cat.emm_pid_array[i];
		}
		*max_emm_count = i;
	}
	else
	{
		*max_emm_count  = dmx_psi_info[dmx_idx].cat.emm_count;
	}

	osal_semaphore_release(sm_semaphore);
	
	return SUCCESS;
}

INT32 sim_get_pcr_pid(UINT32 monitor_id, UINT16 *pcr_pid)
{
	INT16 dmx_idx;
	UINT16 prog_number;
	UINT8 tbl_idx;

	if( !monitor_id_valid(monitor_id) )
		return ERR_FAILUE;

	if( monitor_exist[monitor_id] != 1)
		return ERR_FAILUE;

	//dmx_idx = (monitor_id&DMX_INDEX_MASK)>>24;
	//pmt_pid = (monitor_id&SEC_PID_MASK);
	dmx_idx = simcb_array[monitor_id].dmx_idx;
	prog_number = simcb_array[monitor_id].param;

	tbl_idx = get_table_index(dmx_idx, MONITE_TB_PMT, prog_number);
	if(tbl_idx==0xFF)
		return ERR_FAILUE;

	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
	
	*pcr_pid = dmx_psi_info[dmx_idx].pmt[tbl_idx].pcr_pid;

	osal_semaphore_release(sm_semaphore);

	return SUCCESS;
}

#ifdef SCTE_SUBTITLE_SUPPORT
INT32 sim_get_scte_subt_pid(UINT32 monitor_id, UINT8 *pid_count, UINT16 *subt_pid, UINT8 *pid_lan_count, UINT32 lan[][6])
{
	INT16 dmx_idx;
	UINT16 prog_number;
	UINT8 tbl_idx;

	if(pid_count==NULL || subt_pid == NULL || pid_lan_count == NULL || lan == NULL)
		return ERR_FAILURE;

	if( !monitor_id_valid(monitor_id) )
		return ERR_FAILUE;

	if( monitor_exist[monitor_id] != 1)
		return ERR_FAILUE;

	dmx_idx = simcb_array[monitor_id].dmx_idx;
	prog_number = simcb_array[monitor_id].param;

	tbl_idx = get_table_index(dmx_idx, MONITE_TB_PMT, prog_number);
	if(tbl_idx==0xFF)
		return ERR_FAILUE;

	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

	*pid_count = dmx_psi_info[dmx_idx].pmt[tbl_idx].scte_subt_count;
	MEMCPY(subt_pid, dmx_psi_info[dmx_idx].pmt[tbl_idx].scte_subt_pid, *pid_count*sizeof(UINT16));
	MEMCPY(pid_lan_count, dmx_psi_info[dmx_idx].pmt[tbl_idx].scte_subt_lan_cnt, *pid_count*sizeof(UINT8));
	MEMCPY(lan, dmx_psi_info[dmx_idx].pmt[tbl_idx].scte_subt_lan, *pid_count*6*sizeof(UINT32));

	osal_semaphore_release(sm_semaphore);

	return SUCCESS;
}

#endif

INT32 sim_get_video_pid(UINT32 monitor_id, UINT16 *video_pid)
{
	INT16 dmx_idx;
	UINT16 prog_number;
	UINT8 tbl_idx;

	if( !monitor_id_valid(monitor_id) )
		return ERR_FAILUE;

	if( monitor_exist[monitor_id] != 1)
		return ERR_FAILUE;

	//dmx_idx = (monitor_id&DMX_INDEX_MASK)>>24;
	//pmt_pid = (monitor_id&SEC_PID_MASK);
	dmx_idx = simcb_array[monitor_id].dmx_idx;
	prog_number = simcb_array[monitor_id].param;

	tbl_idx = get_table_index(dmx_idx, MONITE_TB_PMT, prog_number);
	if(tbl_idx==0xFF)
		return ERR_FAILUE;
	
	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
	
	*video_pid = dmx_psi_info[dmx_idx].pmt[tbl_idx].video_pid;

	osal_semaphore_release(sm_semaphore);
	
	return SUCCESS;
}

INT32 sim_get_prog_info(UINT32 monitor_id, struct prog_info **pg_info)
{
	INT16 dmx_idx;
	UINT16 prog_number;
	UINT8 tbl_idx;

	if( !monitor_id_valid(monitor_id) )
		return ERR_FAILUE;

	if( monitor_exist[monitor_id] != 1)
		return ERR_FAILUE;

	//dmx_idx = (monitor_id&DMX_INDEX_MASK)>>24;
	//pmt_pid = (monitor_id&SEC_PID_MASK);
	dmx_idx = simcb_array[monitor_id].dmx_idx;
	prog_number = simcb_array[monitor_id].param;

	tbl_idx = get_table_index(dmx_idx, MONITE_TB_PMT, prog_number);
	if(tbl_idx==0xFF)
		return ERR_FAILUE;
	
	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
	
	*pg_info = &(dmx_psi_info[dmx_idx].pmt[tbl_idx]);

	osal_semaphore_release(sm_semaphore);
	
	return SUCCESS;
}



INT32 sim_get_audio_info(UINT32 monitor_id, UINT16 *audio_pid, UINT8 audio_lang[][3], UINT8 *max_audio_count)
{
	INT16 dmx_idx;
	UINT16 prog_number;
	UINT8 tbl_idx;
	int i;

	if( !monitor_id_valid(monitor_id) )
		return ERR_FAILUE;

	if( monitor_exist[monitor_id] != 1)
		return ERR_FAILUE;
		
	//dmx_idx = (monitor_id&DMX_INDEX_MASK)>>24;
	//pmt_pid = (monitor_id&SEC_PID_MASK);
	dmx_idx = simcb_array[monitor_id].dmx_idx;
	prog_number = simcb_array[monitor_id].param;
	tbl_idx = get_table_index(dmx_idx, MONITE_TB_PMT, prog_number);
	if(tbl_idx==0xFF)
		return ERR_FAILUE;

	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
	if(audio_pid != NULL || audio_lang != NULL)
	{
		for(i=0; i<*max_audio_count&&i<dmx_psi_info[dmx_idx].pmt[tbl_idx].audio_count; i++)
		{
			if(audio_pid != NULL)
				audio_pid[i] = dmx_psi_info[dmx_idx].pmt[tbl_idx].audio_pid[i];
			if(audio_lang != NULL)
				MEMCPY(audio_lang[i], dmx_psi_info[dmx_idx].pmt[tbl_idx].audio_lang[i],3);
		}
		*max_audio_count = i;
	}
	else
	{
		*max_audio_count = dmx_psi_info[dmx_idx].pmt[tbl_idx].audio_count;
	}
	
	osal_semaphore_release(sm_semaphore);

	return SUCCESS;
}

INT32 sim_get_pmt(UINT32 monitor_id,struct prog_info* info)
{
	UINT16 dmx_idx;
	UINT16 prog_number;
	UINT8 tbl_idx;
	
	if(info==NULL)
		return ERR_FAILUE;

	if( !monitor_id_valid(monitor_id) )
		return ERR_FAILUE;

	if( monitor_exist[monitor_id] != 1)
		return ERR_FAILUE;

	//dmx_idx = (monitor_id&DMX_INDEX_MASK)>>24;
	//pmt_pid = (monitor_id&SEC_PID_MASK);
	dmx_idx = simcb_array[monitor_id].dmx_idx;
	prog_number = simcb_array[monitor_id].param;
	tbl_idx = get_table_index(dmx_idx, MONITE_TB_PMT, prog_number);
	
	if(tbl_idx==0xFF)
		return ERR_FAILUE;

	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
	
	MEMCPY(info, &(dmx_psi_info[dmx_idx].pmt[tbl_idx]), sizeof(struct prog_info));

	osal_semaphore_release(sm_semaphore);
	return SUCCESS;
}

/*UINT32 si_get_service_name(UINT32 monitor_id,  UINT16 program_number, UINT8 *name, UINT8 *max_name_len)
{
	return ERR_FAILUE;
}

UINT32 si_get_sprovider_name(UINT32 monitor_id,  UINT16 program_number, UINT8 *name, UINT8 *max_name_len)
{
	return ERR_FAILUE;
}*/

INT32 si_get_service_new_orinid(UINT32 monitor_id,  UINT16 program_number, UINT16 *new_orinid)
{
	return ERR_FAILUE;
}

INT32 si_get_service_new_tsid(UINT32 monitor_id,  UINT16 program_number, UINT16 *new_tsid)
{
	return ERR_FAILUE;
}



//for old api
extern UINT32 monitor_id;
extern UINT32 cat_monitor_id;

UINT32 sie_get_ecm_pid(UINT16 *pid_buffer_ptr)
{
	UINT8 max_count = 16;

	if(monitor_id==SIM_INVALID_MON_ID || pid_buffer_ptr == NULL)
		return 0;

	sim_get_ecm_pid(monitor_id, pid_buffer_ptr, &max_count);
		
	return max_count;
}

void sie_get_ecm_emm_cnt(UINT8 *ecm_count_ptr, UINT8 *emm_count_ptr)
{
	*ecm_count_ptr = 0;
	*emm_count_ptr = 0;
	
	if(monitor_id!=SIM_INVALID_MON_ID)
		sim_get_ecm_pid(monitor_id, NULL, ecm_count_ptr);
	
	if(cat_monitor_id!=SIM_INVALID_MON_ID)
		sim_get_emm_pid(cat_monitor_id, NULL, emm_count_ptr);
}

UINT32 sie_get_emm_pid(UINT16 *pid_buffer_ptr)
{
	UINT8 max_count = 32;

	if(cat_monitor_id==SIM_INVALID_MON_ID || pid_buffer_ptr == NULL)
		return 0;

	sim_get_emm_pid(cat_monitor_id, pid_buffer_ptr, &max_count);
		
	return max_count;
}

#ifdef DYNAMIC_SERVICE_SUPPORT
INT32 sim_get_sdt_info(UINT32 monitor_id, struct sdt_info *info)
{
	struct SDT_TABLE_INFO *sdt = NULL;
	INT16 dmx_idx;
	UINT8 i;

	if (!monitor_id_valid(monitor_id))
		return ERR_FAILUE;

	if (monitor_exist[monitor_id] != 1)
		return ERR_FAILUE;

	if (simcb_array[monitor_id].table != MONITE_TB_SDT)
		return ERR_FAILUE;

	dmx_idx = simcb_array[monitor_id].dmx_idx;
	sdt = &dmx_psi_info[dmx_idx].sdt;

	info->t_s_id = sdt->t_s_id;
	info->onid = sdt->onid;
	info->sdt_version = sdt->sdt_version;
	info->service_cnt = sdt->sd_count;
	for (i = 0; i < sdt->sd_count; i++)
	{
		info->ssi[i].service_id = sdt->sd[i].program_number;
	}	

	return SUCCESS;
}
#endif

