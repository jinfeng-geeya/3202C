#include <types.h>
#include <sys_config.h>
#include <api/libc/list.h>
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

#define SIM_DEBUG_LEVEL		0
#if (SIM_DEBUG_LEVEL>0)
#define SIM_PRINTF			libc_printf
#else
#define SIM_PRINTF(...)		do{}while(0)
#endif


#define PSI_INVALID_ID 0x1FFF	//invalid SI/PSI section PID

#define PAT_VALID_BIT	16		//PAT table valid bid index
#define CAT_VALID_BIT	17		//CAT table valid bid index
#define NIT_VALID_BIT	18		//NIT table valid bid index
#define SDT_VALID_BIT	19		//SDT table valid bid index
#define BAT_VALID_BIT	20		//BAT table valid bid index

#define MAX_SIM_NCB_CNT			5	//max notify callback for monitor core
#ifdef CI_PLUS_SUPPORT
#define MAX_SIM_SCB_CNT			32	//max callback function support for extension monitor
#else
#define MAX_SIM_SCB_CNT			5	//max callback function support for extension monitor
#endif
#define TABLE_COUNT 	(TABLE_TYPE_COUNT+PMT_MAX_COUNT-1)//all table count of one DMX

#define SHORT_BUF_CNT 10	//buffer count for filter

//#if(defined(_MHEG5_V20_ENABLE_))
UINT16 si_get_cur_channel(void);
//#endif

static INT32 monitor_pat_callback(UINT8 *section, INT32 length, UINT32 param);
static INT32 pmt_callback(UINT8 *section, INT32 length, UINT32 param);

static sie_status_t __si_monitor_on_receive_PMT(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter, 
	UINT8 reason, UINT8 *buff, INT32 length);
static sie_status_t __si_monitor_on_receive_PAT(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter, 
	UINT8 reason, UINT8 *buff, INT32 length);
static sie_status_t __si_monitor_on_receive_CAT(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter, 
	UINT8 reason, UINT8 *buff, INT32 length);
static sie_status_t __si_monitor_on_receive_NIT(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter, 
	UINT8 reason, UINT8 *buff, INT32 length);
static sie_status_t __si_monitor_on_receive_SDT(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter, 
	UINT8 reason, UINT8 *buff, INT32 length);
static sie_status_t __si_monitor_on_receive_BAT(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter, 
	UINT8 reason, UINT8 *buff, INT32 length);

OSAL_ID sm_semaphore = OSAL_INVALID_ID;

struct DMX_PSI_INFO dmx_psi_info[DMX_COUNT];

/* Read Only, No need mutex protect */
struct PID_TID_PARSER pid_tid_parser[TABLE_TYPE_COUNT] =
{
	{PSI_INVALID_ID,PSI_PMT_TABLE_ID,__si_monitor_on_receive_PMT}, //pmt
	{PSI_PAT_PID,PSI_PAT_TABLE_ID,__si_monitor_on_receive_PAT}, //pat
	{0x01,0x01,__si_monitor_on_receive_CAT},//cat
	{PSI_INVALID_ID,PSI_NIT_TABLE_ID,__si_monitor_on_receive_NIT},//nit
	{PSI_SDT_PID,PSI_SDT_TABLE_ID,__si_monitor_on_receive_SDT},//sdt
	{PSI_INVALID_ID,PSI_BAT_TABLE_ID,__si_monitor_on_receive_BAT},//bat
};

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT

#define DMX_NUM 2
#define EACH_DMX_MONITOR_NUM 2

UINT8 g_pid_change_flag[DMX_NUM][EACH_DMX_MONITOR_NUM]={0};

#endif

static sim_notify_callback sim_ncb[DMX_COUNT][TABLE_COUNT][MAX_SIM_NCB_CNT];
static sim_section_callback sim_scb[DMX_COUNT][TABLE_COUNT][MAX_SIM_SCB_CNT];
static UINT32 sec_cb_priv[DMX_COUNT][TABLE_COUNT][MAX_SIM_SCB_CNT];


UINT32 sb_used[SHORT_BUF_CNT];
static UINT8 short_buff[SHORT_BUF_CNT][PSI_SHORT_SECTION_LENGTH];


UINT8 monitor_exist[MAX_SIM_COUNT];
struct sim_cb_param simcb_array[MAX_SIM_COUNT];
struct restrict mask_array[MAX_SIM_COUNT];

UINT32 monitor_id_valid(UINT32 monitor_id)
{
	if(monitor_id <MAX_SIM_COUNT)//if(monitor_id>=0 && monitor_id <MAX_SIM_COUNT)/*always is true ,clean warning*/
		return 1;
	return 0;
}

INT32 sim_get_monitor_param(UINT32 monitor_id, struct sim_cb_param *param)
{
	if(param == NULL)
		return ERR_FAILUE;

	if(!monitor_id_valid(monitor_id))
		return ERR_FAILUE;
	
	if(monitor_exist[monitor_id] != 1)
		return ERR_FAILUE;
	
	MEMCPY(param, &(simcb_array[monitor_id]), sizeof(struct sim_cb_param));

	return SUCCESS;
}

static INT16 get_dmx_index(struct dmx_device *dmx)
{
	INT16 index=0;

	if(dmx==NULL)
	{
		return -1;
	}
	index = (dmx->type)&HLD_DEV_ID_MASK;
	return index;
}

UINT32 check_monitor(struct dmx_device *dmx, enum MONITE_TB table, UINT16 PID,UINT32 param)
{
	int i;
	
	if(dmx == NULL)
		return SIM_INVALID_MON_ID;
	
	for(i=0; i<MAX_SIM_COUNT; i++)
	{
		if(monitor_exist[i] == 0)
			continue;
		if(table == MONITE_TB_PMT)
		{
			if( simcb_array[i].table== table && get_dmx_index(dmx) == simcb_array[i].dmx_idx && PID == simcb_array[i].sec_pid
				&& param == simcb_array[i].param )
			{
				return i;
			}
		}
		else
		{
			if( simcb_array[i].table== table && get_dmx_index(dmx) == simcb_array[i].dmx_idx && PID == simcb_array[i].sec_pid )
			{
				return i;
			}
		}
	}
	return SIM_INVALID_MON_ID;
}

UINT8 get_free_buffer(UINT32 monitor_id)
{
	UINT8 i;
    if( !monitor_id_valid(monitor_id) )
        return 0xFF;

    for(i=0; i<SHORT_BUF_CNT; i++)
    {
        if(sb_used[i]==monitor_id)
            return 0xFF;
    }
        
	for(i = 0; i<SHORT_BUF_CNT;i++)
	{
		if(sb_used[i]==SIM_INVALID_MON_ID)
		{
			sb_used[i]= monitor_id;
			return i;
		}
	}
	return 0xFF;
}

UINT8 release_buffer(UINT32 monitor_id)
{
	UINT8 i;

	for(i=0; i<SHORT_BUF_CNT; i++)
	{
		//if(&(short_buff[i][0]) == buf )
		if(sb_used[i]==monitor_id)
		{
			sb_used[i] = SIM_INVALID_MON_ID;
			return i;
		}
	}
	return 0xFF;
}

UINT32 get_free_monitor_id()
{
	UINT32 i;
	static UINT32 monitor_id = 0;
	UINT32 free_id = SIM_INVALID_MON_ID;
	for(i = 0; i<MAX_SIM_COUNT;i++)
	{
		if(monitor_exist[monitor_id]==0)
		{
			monitor_exist[monitor_id]= 1;
			free_id = monitor_id;
			monitor_id = (monitor_id+1)%MAX_SIM_COUNT;
			return free_id;
		}
		monitor_id = (monitor_id+1)%MAX_SIM_COUNT;
	}
	return SIM_INVALID_MON_ID;
}

void release_monitor_id(UINT32 monitor_id)
{
	if( monitor_id_valid(monitor_id) )
		monitor_exist[monitor_id] = 0;
}

//pmt 0-16, pat 16, cat 17, nit 18, sdt 19
UINT8 get_table_index(UINT16 dmx_idx, enum MONITE_TB table, UINT16 prog_number)
{
	UINT8 tbl_idx = 0xFF;

	if(dmx_idx >= DMX_COUNT)
		return tbl_idx;

	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
	if(table==MONITE_TB_PMT)
	{
		for(tbl_idx=0;tbl_idx<PMT_MAX_COUNT;tbl_idx++)
		{
			if((dmx_psi_info[dmx_idx].valid & (1<<tbl_idx)) && 
				dmx_psi_info[dmx_idx].pmt[tbl_idx].prog_number==prog_number )
				break;
		}
		if(tbl_idx==PMT_MAX_COUNT)
			tbl_idx=0xFF;
	}
	else
	{
		tbl_idx = table-MONITE_TB_PAT+PMT_MAX_COUNT;
	}
	osal_semaphore_release(sm_semaphore);
	
	return tbl_idx;
}

void run_callback(UINT32 monitor_id,UINT8 *buff, INT32 length)
{
	UINT8 tbl_idx = 0xFF;
	int i;
	struct sim_cb_param param;
	UINT16 dmx_idx;

	if(!monitor_id_valid(monitor_id) || (monitor_exist[monitor_id] != 1))
		return;

	if(NULL == buff)
		return;
	
	dmx_idx = simcb_array[monitor_id].dmx_idx;
	tbl_idx = get_table_index(dmx_idx,simcb_array[monitor_id].table,simcb_array[monitor_id].param);
	if(tbl_idx==0xFF)
		return;

	MEMCPY(&param, &(simcb_array[monitor_id]), sizeof(struct sim_cb_param));

	//run call back
	//osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
	for(i=0; i<MAX_SIM_NCB_CNT; i++)
	{
		if(sim_ncb[dmx_idx][tbl_idx][i] != NULL)
			sim_ncb[dmx_idx][tbl_idx][i]((UINT32)(&param));
	}
	for(i=0; i<MAX_SIM_SCB_CNT; i++)
	{
		if(sim_scb[dmx_idx][tbl_idx][i] != NULL)
		{
			param.priv = (void*)(sec_cb_priv[dmx_idx][tbl_idx][i]);
			sim_scb[dmx_idx][tbl_idx][i](buff, length, (UINT32)(&param));
		}
	}
	//osal_semaphore_release(sm_semaphore);
	return;
}

static sie_status_t __si_monitor_on_receive_PMT(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter, 
	UINT8 reason, UINT8 *buff, INT32 length)
{
	UINT8 tbl_idx = 0xFF;
	struct prog_info *prog = NULL;
	UINT32 monitor_id = SIM_INVALID_MON_ID;
	UINT16 prog_number;

	SIM_PRINTF("SIM Receive PMT\n");

	if((buff==NULL)||(reason==SIE_REASON_FILTER_TIMEOUT))
		return sie_started;

	prog_number = (buff[3]<<8)|buff[4];
	
	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
	monitor_id = check_monitor(info->dmx, MONITE_TB_PMT, pid, prog_number);
	osal_semaphore_release(sm_semaphore);
	if(monitor_id==SIM_INVALID_MON_ID)
		return sie_started;

	tbl_idx = get_table_index(simcb_array[monitor_id].dmx_idx,MONITE_TB_PMT,prog_number);
	if(tbl_idx==0xFF)
		return sie_started;

	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

	prog = &(info->pmt[tbl_idx]);
	MEMSET(prog, 0, sizeof(struct prog_info));	
	prog->pmt_pid = pid;
	prog->prog_number = prog_number;
	   
	if (psi_pmt_parser(buff, prog,PSI_MODULE_COMPONENT_NUMBER)==SI_SUCCESS)
	{
        ;//prog->video_pid &= 0x1fff;		
	}
	osal_semaphore_release(sm_semaphore);

	run_callback(monitor_id,buff,length);
	return sie_started;
}


static sie_status_t __si_monitor_on_receive_PAT(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter, 
	UINT8 reason, UINT8 *buff, INT32 length)
{
	struct PAT_TABLE_INFO *pat = NULL;
	UINT16 prog_map_len = 0;
	UINT8 *prog_map = NULL;
	UINT16 prog_number;
	UINT32 monitor_id = -1;
	
	if((buff == NULL) || (reason == SIE_REASON_FILTER_TIMEOUT))
		return sie_started;

	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

	pat = &(info->pat);
	pat->max_map_nr = PSI_MODULE_MAX_PROGRAM_NUMBER;
	pat->map_counter = 0;

	pat->ts_id = (buff[3]<<8)|buff[4];

	prog_map_len = (((buff[1]&0xF)<<8)|buff[2]) - 9;
	prog_map = buff + 8;
	while(prog_map_len>0)
	{
		prog_number = (prog_map[0]<<8)|prog_map[1];
		if(prog_number==0)
		{
			pat->nit_pid = ((prog_map[2]&0x1F)<<8)|prog_map[3];
		}
		else
		{
			if(pat->map_counter < pat->max_map_nr)
			{
				pat->map[pat->map_counter].pm_number = prog_number;
				pat->map[pat->map_counter].pm_pid = ((prog_map[2]&0x1F)<<8)|prog_map[3];
				pat->map_counter++;
			}
		}
		prog_map_len -= 4;
		prog_map += 4;
	}
	
	monitor_id = check_monitor(info->dmx, MONITE_TB_PAT, pid, 0);
	
	osal_semaphore_release(sm_semaphore);

	if(monitor_id==SIM_INVALID_MON_ID)
		return sie_started;
	
	run_callback(monitor_id,buff,length);

	return sie_started;	
}

static sie_status_t __si_monitor_on_receive_CAT(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter, 
	UINT8 reason, UINT8 *buff, INT32 length)
{
	struct CAT_TABLE_INFO *cat = NULL;
	UINT16 cat_length = 0;
	UINT16 index = 0;
	UINT32 monitor_id = -1;

	if((buff == NULL) || (reason == SIE_REASON_FILTER_TIMEOUT))
		return sie_started;

	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

	if(!(info->valid & (1<<CAT_VALID_BIT)))
	{
		osal_semaphore_release(sm_semaphore);
		return sie_started;
	}
	
	cat = &(info->cat);
	cat->emm_count = 0;

	cat_length = ((buff[1]&0x0F)<<8) | buff[2];
	index = 8;
	cat_length -= 9;
	while(cat_length>0)
	{
		if(buff[index] == 0x09)
		{
			if(cat->emm_count<SIM_EMM_MAX_COUNT)
			{
				cat->ca_sysid_array[cat->emm_count] = buff[index+2] | buff[index+3];
				cat->emm_pid_array[cat->emm_count] = ((buff[index+4]&0x1F)<<8) | buff[index+5];
				cat->emm_count++;
			}
			else
				SIM_PRINTF("emm_count overflow\n");
		}
		cat_length -= 2+buff[index+1];
		index += 2+buff[index+1];
	}
	monitor_id = check_monitor(info->dmx, MONITE_TB_CAT, pid, 0);
	
	osal_semaphore_release(sm_semaphore);	

	if(monitor_id==SIM_INVALID_MON_ID)
		return sie_started;

	run_callback(monitor_id,buff,length);

	return sie_started;	
}

static sie_status_t __si_monitor_on_receive_NIT(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter, 
	UINT8 reason, UINT8 *buff, INT32 length)
{
	UINT32 monitor_id = -1;
	
	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
	monitor_id = check_monitor(info->dmx, MONITE_TB_NIT, pid, 0);
	osal_semaphore_release(sm_semaphore);

	if(monitor_id==SIM_INVALID_MON_ID)
		return sie_started;

	run_callback(monitor_id, buff, length);

	return sie_started;
}

static sie_status_t __si_monitor_on_receive_SDT(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter, 
	UINT8 reason, UINT8 *buff, INT32 length)
{
	UINT32 monitor_id = -1;
	
	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

#ifdef DYNAMIC_SERVICE_SUPPORT
	struct SDT_TABLE_INFO *sdt = &info->sdt;
	struct sdt_stream_info *ss_info;
	int i, descriptors_length;

	if (buff[0] != PSI_SDT_TABLE_ID)
	{
		osal_semaphore_release(sm_semaphore);
		return sie_started;
	}
		
	MEMSET(sdt, 0, sizeof(struct SDT_TABLE_INFO));
	sdt->t_s_id = (buff[3] << 8) | buff[4];
	sdt->onid = (buff[8] << 8) | buff[9];
	sdt->sdt_version = (buff[5] & 0x3E) >> 1;
	
	for (i = sizeof(struct sdt_section) - PSI_SECTION_CRC_LENGTH;
	     i < length - PSI_SECTION_CRC_LENGTH;
	     i += sizeof(struct sdt_stream_info) + descriptors_length)
	{
    	ss_info = (struct sdt_stream_info *)(buff + i);
    	descriptors_length = SI_MERGE_HL8(ss_info->descriptor_loop_length);
		sdt->sd[sdt->sd_count].program_number = SI_MERGE_UINT16(ss_info->service_id);

		sdt->sd_count++;
	}
#endif
	
	monitor_id = check_monitor(info->dmx, MONITE_TB_SDT, pid, 0);		
	osal_semaphore_release(sm_semaphore);

	if(monitor_id==SIM_INVALID_MON_ID)
		return sie_started;
	
	run_callback(monitor_id, buff, length);
	
	return sie_started;
}

static sie_status_t __si_monitor_on_receive_BAT(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter, 
	UINT8 reason, UINT8 *buff, INT32 length)
{
	UINT32 monitor_id = -1;
	
	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
	monitor_id = check_monitor(info->dmx, MONITE_TB_NIT, pid, 0);
	osal_semaphore_release(sm_semaphore);

	if(monitor_id==SIM_INVALID_MON_ID)
		return sie_started;

	run_callback(monitor_id, buff, length);

	return sie_started;
}


static table_section_parser get_table_parser(UINT8 table_id)
{
	int table_index;

	/* Read only Global variable, no need protect */
	for(table_index=0; table_index<TABLE_TYPE_COUNT; table_index++)	{
		if(pid_tid_parser[table_index].tid == table_id)
			return pid_tid_parser[table_index].parser;
	}
	return NULL;
}

/*! SI monitor's fparam.section_parser
 * No corresponse fparam.section_event except France HD project
 * 
 * Receive sections that being monitored, and call response parser
 * by table_id.
 */
static sie_status_t __si_monitor_on_receive_sec(UINT16 pid, struct si_filter_t *filter, 
	UINT8 reason, UINT8 *buff, INT32 length)
{
	struct dmx_device *dmx;
	UINT8 table_id;
	UINT16 dmx_index;
	table_section_parser section_parser;

	if((buff==NULL)||(filter==NULL)||(reason==SIE_REASON_FILTER_TIMEOUT))
		return sie_started;
	
	if(length == 0)
		return sie_started;

	table_id = buff[0];
	dmx = filter->dmx;
	dmx_index = get_dmx_index(dmx);

	if(dmx_index >= DMX_COUNT)
		return sie_started;

	/* Call the section parser to process section data */
	section_parser = get_table_parser(table_id);
	section_parser(&dmx_psi_info[dmx_index],pid,filter,reason,buff,length);

	return sie_started;
}

UINT32 __si_start_monitor(struct dmx_device *dmx,UINT16 PID, UINT16 table_id, enum MONITE_TB table, UINT32 param)
{
	INT32 ret = SUCCESS;
	struct si_filter_t *filter = NULL;
	struct si_filter_param fparam;
	UINT16 dmx_index = 0;
	UINT8 buf_idx;
	UINT32 monitor_id = SIM_INVALID_MON_ID;

	if(dmx == NULL)
		return SIM_INVALID_MON_ID;

	dmx_index = get_dmx_index(dmx);
	
	if(dmx_index >= DMX_COUNT)
		return SIM_INVALID_MON_ID;

	MEMSET(&fparam, 0, sizeof(struct si_filter_param));
	fparam.timeout = OSAL_WAIT_FOREVER_TIME;
	fparam.attr[0] = SI_ATTR_HAVE_CRC32;
	fparam.section_event = NULL;
	fparam.section_parser = (si_handler_t)__si_monitor_on_receive_sec;
	if(table_id==PSI_PMT_TABLE_ID)
	{
		fparam.mask_value.mask_len = 6;
		fparam.mask_value.mask[0] = 0xFF;
		fparam.mask_value.mask[1] = 0x80;
		fparam.mask_value.mask[3] = 0xFF;
		fparam.mask_value.mask[4] = 0xFF;
		fparam.mask_value.mask[5] = 0x01;		
		fparam.mask_value.value_num = 1;
		fparam.mask_value.value[0][0] = table_id;
		fparam.mask_value.value[0][1] = 0x80;
		fparam.mask_value.value[0][3] = (param>>8)&0xFF;
		fparam.mask_value.value[0][4] = param&0xFF;
		fparam.mask_value.value[0][5] = 0x01;
	}
	else
	{		
		fparam.mask_value.mask_len = 6;
		fparam.mask_value.mask[0] = 0xFF;
		fparam.mask_value.mask[1] = 0x80;
		fparam.mask_value.mask[5] = 0x01;
		fparam.mask_value.value_num = 1;
		fparam.mask_value.value[0][0] = table_id;
		fparam.mask_value.value[0][1] = 0x80;
		fparam.mask_value.value[0][5] = 0x01;
	}

	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
	
    monitor_id = get_free_monitor_id();
	if(monitor_id == SIM_INVALID_MON_ID)
	{
        osal_semaphore_release(sm_semaphore);
		return SIM_INVALID_MON_ID;
	}
	
	buf_idx = get_free_buffer(monitor_id);

	if(buf_idx==0xFF)
	{
        release_monitor_id(monitor_id);
		osal_semaphore_release(sm_semaphore);
		return SIM_INVALID_MON_ID;
	}

	osal_semaphore_release(sm_semaphore);
	filter = sie_alloc_filter_ext(dmx, PID, short_buff[buf_idx], 1024,1024,0);
		
	if(filter==NULL)
	{
		osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
        release_monitor_id(monitor_id);
		release_buffer(monitor_id);
		osal_semaphore_release(sm_semaphore);
		return SIM_INVALID_MON_ID;
	}

	sie_config_filter(filter,&fparam);	

	ret = sie_enable_filter(filter);

	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
	if(ret!=SI_SUCCESS)
	{
        release_monitor_id(monitor_id);
		release_buffer(monitor_id);
		osal_semaphore_release(sm_semaphore);
		return SIM_INVALID_MON_ID;		
	}

	//backup restrict
	MEMCPY(&(mask_array[monitor_id]), &(fparam.mask_value), sizeof(struct restrict));
	simcb_array[monitor_id].sim_id = monitor_id;
	simcb_array[monitor_id].dmx_idx  = dmx_index;
	simcb_array[monitor_id].table = table;
	simcb_array[monitor_id].sec_pid = PID;
	simcb_array[monitor_id].param = param;
	simcb_array[monitor_id].priv = NULL;

	if(dmx_psi_info[dmx_index].valid==0)//no data valid
	{
		MEMSET(&dmx_psi_info[dmx_index],0,sizeof(struct DMX_PSI_INFO));
		dmx_psi_info[dmx_index].dmx = dmx;
	}

	if(table_id==PSI_PMT_TABLE_ID)
	{
		int tmp_idx = 0;
		for(tmp_idx=0;tmp_idx<PMT_MAX_COUNT;tmp_idx++)
		{
			if((dmx_psi_info[dmx_index].valid & (1<<tmp_idx))== 0)
				break;
		}
		if(tmp_idx<PMT_MAX_COUNT)
		{
			MEMSET(&(dmx_psi_info[dmx_index].pmt[tmp_idx]), 0, sizeof(struct prog_info));				
			dmx_psi_info[dmx_index].pmt[tmp_idx].pmt_pid = PID;
			dmx_psi_info[dmx_index].pmt[tmp_idx].prog_number = param;
			dmx_psi_info[dmx_index].valid |= (1<<tmp_idx);
		}
	}
	else if(table_id==PSI_PAT_TABLE_ID)
	{
		MEMSET(&(dmx_psi_info[dmx_index].pat), 0, sizeof(struct PAT_TABLE_INFO));
		dmx_psi_info[dmx_index].valid |= (1<<PAT_VALID_BIT);
	}
	else if(table_id==0x01)//cat
	{
		MEMSET(&(dmx_psi_info[dmx_index].cat), 0, sizeof(struct CAT_TABLE_INFO));
		dmx_psi_info[dmx_index].valid |= (1<<CAT_VALID_BIT);
	}

	else if(table_id==PSI_NIT_TABLE_ID)
	{
		//MEMSET(&(dmx_psi_info[dmx_index].nit), 0, sizeof(struct nit_section_info));
		dmx_psi_info[dmx_index].valid |= (1<<NIT_VALID_BIT);
	}
	else if(table_id==PSI_SDT_TABLE_ID)
	{
		//MEMSET(&(dmx_psi_info[dmx_index].sdt), 0, sizeof(struct SDT_TABLE_INFO));
		dmx_psi_info[dmx_index].valid |= (1<<SDT_VALID_BIT);
	}
	else if(table_id==PSI_BAT_TABLE_ID)
	{
		//MEMSET(&(dmx_psi_info[dmx_index].sdt), 0, sizeof(struct SDT_TABLE_INFO));
		dmx_psi_info[dmx_index].valid |= (1<<BAT_VALID_BIT);
	}

	osal_semaphore_release(sm_semaphore);	
	return monitor_id;
}

void sim_open_monitor(UINT32 praram)
{
	INT32 i,j,k;

	SIM_PRINTF("Init monitor\n");
		
	sm_semaphore = osal_semaphore_create(1);
	//if (sm_semaphore == OSAL_INVALID_ID)
	//{
	//	return;
	//}

	for(i=0; i<DMX_COUNT; i++)
	{
		for(j=0; j<TABLE_COUNT; j++)
		{
			for(k=0; k<MAX_SIM_NCB_CNT; k++)
			{
				sim_ncb[i][j][k] = NULL;
			}
			for(k=0; k<MAX_SIM_SCB_CNT; k++)
			{
				sim_scb[i][j][k] = NULL;
				sec_cb_priv[i][j][k] = 0;
			}
		}
	}
	for(i=0; i<SHORT_BUF_CNT; i++)
	{
		sb_used[i] = SIM_INVALID_MON_ID;
	}
	for(i=0; i<MAX_SIM_COUNT;i++)
	{
		monitor_exist[i] = 0;
	}

	MEMSET(dmx_psi_info, 0 , sizeof(struct DMX_PSI_INFO)*DMX_COUNT);
	return;
}

void sim_close_monitor(UINT32 praram)
{
	INT32 i,j,k;

	SIM_PRINTF("close monitor\n");

	osal_semaphore_delete(sm_semaphore);
	sm_semaphore = OSAL_INVALID_ID;

	for(i=0; i<DMX_COUNT; i++)
	{
		for(j=0; j<TABLE_COUNT; j++)
		{
			for(k=0; k<MAX_SIM_NCB_CNT; k++)
			{
				sim_ncb[i][j][k] = NULL;
			}
			for(k=0; k<MAX_SIM_SCB_CNT; k++)
			{
				sim_scb[i][j][k] = NULL;
				sec_cb_priv[i][j][k] = 0;
			}
		}
	}
	for(i=0; i<SHORT_BUF_CNT; i++)
	{
		sb_used[i] = SIM_INVALID_MON_ID;
	}
	for(i=0; i<MAX_SIM_COUNT;i++)
	{
		monitor_exist[i] = 0;
	}

	MEMSET(dmx_psi_info, 0 , sizeof(struct DMX_PSI_INFO)*DMX_COUNT);
	return;
}

static INT32 pmt_callback(UINT8 *section, INT32 length, UINT32 param);

UINT32 sim_start_monitor(struct dmx_device *dmx, enum MONITE_TB table, UINT16 PID,UINT32 param)
{
	UINT16 pid;
	UINT16 table_id;
	UINT32 monitor_id = SIM_INVALID_MON_ID;
	
	SIM_PRINTF("sim start monitor\n");

	if(dmx==NULL)
		return SIM_INVALID_MON_ID;

	if(PID>=8191)
		return SIM_INVALID_MON_ID;

	//check pmt pid
	if(table ==MONITE_TB_PMT && (PID ==PSI_PAT_PID||PID ==0x01||PID ==PSI_SDT_PID))
		return SIM_INVALID_MON_ID;

	monitor_id = check_monitor(dmx, table, PID, param);
	if( monitor_id != SIM_INVALID_MON_ID)
		return monitor_id;

	if(pid_tid_parser[table].pid==PSI_INVALID_ID)
		pid = PID;
	else
		pid = pid_tid_parser[table].pid;
	table_id = pid_tid_parser[table].tid;
	monitor_id = __si_start_monitor(dmx, pid, table_id,table,param);

	if((table == MONITE_TB_PMT) && (STRCMP(dmx->name, "DMX_S3601_2") != 0))//not allow parse ca pmt when playback
		sim_register_scb(monitor_id, pmt_callback, NULL);//add for ca
	
	return monitor_id;
}

INT32 sim_stop_monitor(UINT32 monitor_id)
{
	UINT16 dmx_index = -1;
	UINT8 tbl_index = -1;
	int pmt_index = -1;
	int i;
	UINT8 *buf;
	struct dmx_device *dmx = NULL;
	enum MONITE_TB table;
	UINT16 PID;

	if( !monitor_id_valid(monitor_id) )
		return ERR_FAILUE;

	if( monitor_exist[monitor_id] != 1)
		return ERR_FAILUE;

	dmx_index = simcb_array[monitor_id].dmx_idx; //(monitor_id&DMX_INDEX_MASK)>>24;
	table = simcb_array[monitor_id].table; //(monitor_id&TABLE_MASK)>>16;
	PID = simcb_array[monitor_id].sec_pid; //(monitor_id&SEC_PID_MASK);
	dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_index);
	
	SIM_PRINTF("SIM STOP monitor\n");

	tbl_index = get_table_index(dmx_index, table, simcb_array[monitor_id].param);
	if(tbl_index==0xFF)
		return ERR_FAILUE;

	if(sie_abort_ext(dmx, &buf, PID, &(mask_array[monitor_id])) != SI_SUCCESS)
	{
		return ERR_FAILUE;
	}

	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

	release_buffer(monitor_id);

	release_monitor_id(monitor_id);

	for(i=0; i<MAX_SIM_NCB_CNT; i++)
	{
		sim_ncb[dmx_index][tbl_index][i] = NULL;
	}

	//dmx_index = get_dmx_index(dmx);

	if(table==MONITE_TB_PMT)
	{
		int tmp_idx = 0;
		for(tmp_idx=0;tmp_idx<PMT_MAX_COUNT;tmp_idx++)
		{
			if((dmx_psi_info[dmx_index].valid & (1<<tmp_idx)) && 
				dmx_psi_info[dmx_index].pmt[tmp_idx].pmt_pid==PID )
				break;
		}
		if(tmp_idx<PMT_MAX_COUNT)
		{
			MEMSET(&(dmx_psi_info[dmx_index].pmt[tmp_idx]), 0, sizeof(struct prog_info));				
			dmx_psi_info[dmx_index].valid &= ~(1<<tmp_idx);
		}
	}
	else if(table==MONITE_TB_PAT)
	{
		MEMSET(&(dmx_psi_info[dmx_index].pat), 0, sizeof(struct PAT_TABLE_INFO));
		dmx_psi_info[dmx_index].valid &= ~(1<<PAT_VALID_BIT);
	}
	else if(table==MONITE_TB_CAT)//cat
	{
		MEMSET(&(dmx_psi_info[dmx_index].cat), 0, sizeof(struct CAT_TABLE_INFO));
		dmx_psi_info[dmx_index].valid &= ~(1<<CAT_VALID_BIT);
	}

	else if(table==MONITE_TB_NIT)
	{
		//MEMSET(&(dmx_psi_info[dmx_index].nit), 0, sizeof(struct nit_section_info));
		dmx_psi_info[dmx_index].valid &= ~(1<<NIT_VALID_BIT);
	}
	else if(table==MONITE_TB_SDT)
	{
		//MEMSET(&(dmx_psi_info[dmx_index].sdt), 0, sizeof(struct SDT_TABLE_INFO));
		dmx_psi_info[dmx_index].valid &= ~(1<<SDT_VALID_BIT);
	}
	else if(table==MONITE_TB_BAT)
	{
		//MEMSET(&(dmx_psi_info[dmx_index].sdt), 0, sizeof(struct SDT_TABLE_INFO));
		dmx_psi_info[dmx_index].valid &= ~(1<<BAT_VALID_BIT);
	}

	if(dmx_psi_info[dmx_index].valid==0)
		dmx_psi_info[dmx_index].dmx = NULL;

	osal_semaphore_release(sm_semaphore);

	return SUCCESS;
}

INT32 sim_register_ncb(UINT32 monitor_id, sim_notify_callback callback)
{
	INT32 i;
	INT32 n = -1;
	enum MONITE_TB table;
	UINT16 dmx_idx;
	UINT8 tbl_idx;

	SIM_PRINTF("sim_register_ncb\n");

	if( !monitor_id_valid(monitor_id) )
		return ERR_FAILUE;

	if( monitor_exist[monitor_id] != 1)
		return ERR_FAILUE;

	dmx_idx = simcb_array[monitor_id].dmx_idx;
	table = simcb_array[monitor_id].table;

	tbl_idx = get_table_index(dmx_idx, table, simcb_array[monitor_id].param);
	if(tbl_idx==0xFF)
		return ERR_FAILUE;

	if (callback == NULL)
	{
		return ERR_FAILUE;
	}

	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
	for (i=0; i<MAX_SIM_NCB_CNT; i++)
	{
		if (sim_ncb[dmx_idx][tbl_idx][i] == callback)
		{
			osal_semaphore_release(sm_semaphore);
			return SUCCESS;
		}
		else if (sim_ncb[dmx_idx][tbl_idx][i] == NULL && n == -1)
		{
			n = i;
		}
	}

	if (n >= 0 && n < MAX_SIM_NCB_CNT)
	{
		sim_ncb[dmx_idx][tbl_idx][n] = callback;
		osal_semaphore_release(sm_semaphore);
		return SUCCESS;
	}

	osal_semaphore_release(sm_semaphore);
	return ERR_FAILUE;
}

INT32 sim_unregister_ncb(UINT32 monitor_id, sim_notify_callback callback)
{
	INT32 i;
	enum MONITE_TB table;
	UINT16 dmx_idx;
	UINT8 tbl_idx;

	SIM_PRINTF("sim_unregister_ncb\n");

	if( !monitor_id_valid(monitor_id) )
		return ERR_FAILUE;

	if( monitor_exist[monitor_id] != 1)
		return ERR_FAILUE;

	dmx_idx = simcb_array[monitor_id].dmx_idx;
	table = simcb_array[monitor_id].table;

	tbl_idx = get_table_index(dmx_idx, table, simcb_array[monitor_id].param);
	if(tbl_idx==0xFF)
	{
		return ERR_FAILUE;
	}

	if (callback == NULL)
	{
		return ERR_FAILUE;
	}

	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
	for (i=0; i<MAX_SIM_NCB_CNT; i++)
	{
		if (sim_ncb[dmx_idx][tbl_idx][i] == callback)
		{
			sim_ncb[dmx_idx][tbl_idx][i] = NULL;
			osal_semaphore_release(sm_semaphore);
			return SUCCESS;
		}
	}
	
	osal_semaphore_release(sm_semaphore);
	return ERR_FAILUE;

}

INT32 sim_register_scb(UINT32 monitor_id, sim_section_callback callback, void* priv)
{
	INT32 i;
	INT32 n = -1;
	enum MONITE_TB table;
	UINT16 dmx_idx;
	UINT8 tbl_idx;

	SIM_PRINTF("sim_register_scb\n");

	if( !monitor_id_valid(monitor_id) )
		return ERR_FAILUE;

	if( monitor_exist[monitor_id] != 1)
		return ERR_FAILUE;

	dmx_idx = simcb_array[monitor_id].dmx_idx;
	table = simcb_array[monitor_id].table;

	tbl_idx = get_table_index(dmx_idx, table, simcb_array[monitor_id].param);
	if(tbl_idx==0xFF)
		return ERR_FAILUE;

	if (callback == NULL)
		return ERR_FAILUE;

	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);

	for (i=0; i<MAX_SIM_SCB_CNT; i++)
	{
		if (sim_scb[dmx_idx][tbl_idx][i] == callback)
		{
			osal_semaphore_release(sm_semaphore);
			return SUCCESS;
		}
		else if (sim_scb[dmx_idx][tbl_idx][i] == NULL && n == -1)
		{
			n = i;
		}
	}

	if (n >= 0 && n < MAX_SIM_SCB_CNT)
	{
		sim_scb[dmx_idx][tbl_idx][n] = callback;
		sec_cb_priv[dmx_idx][tbl_idx][n] = (UINT32)priv;
		osal_semaphore_release(sm_semaphore);
		return SUCCESS;
	}

	osal_semaphore_release(sm_semaphore);
	return ERR_FAILUE;
}

INT32 sim_unregister_scb(UINT32 monitor_id, sim_section_callback callback)
{
	INT32 i;
	enum MONITE_TB table;
	UINT16 dmx_idx;
	UINT8 tbl_idx;

	SIM_PRINTF("sim_unregister_scb\n");

	if( !monitor_id_valid(monitor_id) )
		return ERR_FAILUE;

	if( monitor_exist[monitor_id] != 1)
		return ERR_FAILUE;

	dmx_idx = simcb_array[monitor_id].dmx_idx;
	table = simcb_array[monitor_id].table;

	tbl_idx = get_table_index(dmx_idx, table, simcb_array[monitor_id].param);

	if(tbl_idx==0xFF)
		return ERR_FAILUE;

	if (callback == NULL)
		return ERR_FAILUE;

	osal_semaphore_capture(sm_semaphore, OSAL_WAIT_FOREVER_TIME);
	for (i=0; i<MAX_SIM_SCB_CNT; i++)
	{
		if (sim_scb[dmx_idx][tbl_idx][i] == callback)
		{
			sim_scb[dmx_idx][tbl_idx][i] = NULL;
			sec_cb_priv[dmx_idx][tbl_idx][i] = 0;
			osal_semaphore_release(sm_semaphore);
			return SUCCESS;
		}
	}

	osal_semaphore_release(sm_semaphore);
	return ERR_FAILUE;

}





/**********************for old api******************************************/

UINT16 monitor_dmx_id = 0;

void si_set_default_dmx(UINT16 id)
{
	monitor_dmx_id = id;
}


#define MAX_SI_CALLBACK_CNT			5	// CI + CA + ...
static section_parse_cb_t si_pmt_callback[MAX_SI_CALLBACK_CNT] = {NULL,NULL,NULL,NULL,NULL};
#ifdef GET_CAT_BY_SIM
static section_parse_cb_t si_cat_callback[MAX_SI_CALLBACK_CNT] = {NULL,NULL,NULL,NULL,NULL};
#endif
static INT32 register_cb(section_parse_cb_t *callback_tab, INT max_cnt, section_parse_cb_t callback)
{
	INT32 i;
	INT32 n = -1;

	if (callback == NULL)
	{
		SIM_PRINTF("%s: callback is NULL!\n",__FUNCTION__);
		return !SUCCESS;
	}

	for (i=0; i<max_cnt; i++)
	{
		if (callback_tab[i] == callback)
		{
			SIM_PRINTF("%s: callback 0x%X already exist!\n",__FUNCTION__,callback);
			return SUCCESS;
		}
		else if (callback_tab[i] == NULL && n == -1)
		{
			n = i;
		}
	}

	if (n >= 0 && n < max_cnt)
	{
		SIM_PRINTF("%s: callback %d registered to 0x%X\n",__FUNCTION__,n,callback);
		callback_tab[n] = callback;
		return SUCCESS;
	}
	else
	{
		SIM_PRINTF("%s: callback table is full!!!\n",__FUNCTION__);
		return !SUCCESS;
	}
}

/*
 * Internal function, for unregister a callback func from 
 * the callback_tab.
 */
static INT32 unregister_cb(section_parse_cb_t *callback_tab, INT max_cnt, section_parse_cb_t callback)
{
	INT32 i;

	if (callback == NULL)
	{
		SIM_PRINTF("%s: callback is NULL!\n",__FUNCTION__);
		return !SUCCESS;
	}

	for (i=0; i<max_cnt; i++)
	{
		if (callback_tab[i] == callback)
		{
			callback_tab[i] = NULL;

			SIM_PRINTF("%s: callback %d unregistered!\n",__FUNCTION__,i);
			return SUCCESS;
		}
	}

	SIM_PRINTF("%s: callback %X not found!\n",__FUNCTION__,callback);
	return !SUCCESS;
}

//run all callback function
static void callback_run(section_parse_cb_t *callback_tab, INT max_cnt,
						UINT8 *buf, INT32 length, UINT32 param)
{
	INT i;
	
	for (i=0; i<max_cnt; i++)
	{
		if (callback_tab[i] != NULL)
		{
			callback_tab[i](buf, length, param);
		}
	}
}

static on_pid_change_t on_change = NULL;

static on_pid_change_t pre_change = NULL;

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
static on_rec_pid_change_t rec_on_change = NULL;
#endif
static P_NODE program;
UINT32 monitor_id = SIM_INVALID_MON_ID;
static UINT16 m_CurChanIndex = 0xffff;
static INT32 notify_ap(UINT32 param)
{
	P_NODE node, back_node;
	struct prog_info info;
	BOOL db_changed = FALSE;
	UINT16 pcr_pid;
	UINT16 video_pid;
	UINT16 audio_pid[P_MAX_AUDIO_NUM];
	UINT8 audio_count = P_MAX_AUDIO_NUM;
	UINT32 pos;

	SIM_PRINTF("notify AP\n");
	
	MEMCPY(&back_node, &program, sizeof(P_NODE));

	sim_get_pmt(monitor_id, &info);
	
	sim_get_pcr_pid(monitor_id, &pcr_pid);
	sim_get_video_pid(monitor_id, &video_pid);
	video_pid &= 0x1fff;
	sim_get_audio_info(monitor_id, audio_pid, NULL, &audio_count);

	if(pcr_pid != back_node.pcr_pid || video_pid != back_node.video_pid || 
		audio_count != back_node.audio_count || 
		MEMCMP(back_node.audio_pid, audio_pid, sizeof(UINT16)*audio_count)!=0)
		db_changed = TRUE;

	if(db_changed)
	{
		if(pre_change)
			pre_change(TRUE);	
		
		psi_monitor_pg2db(&node, &info);
		if(update_stream_info(&back_node, &node, &pos) == SUCCESS)
	{
		SIM_PRINTF("PID Changed, v: %d, pcr: %d, ac: %d, a1: %d\n",video_pid,pcr_pid,audio_count,audio_pid[0]);
		MEMCPY(&program, &node, sizeof(P_NODE));
		if(on_change)
		{
			on_change(TRUE);
		}
	}
	}
	return 0;	
}

#if(defined(TTX_ON)&&(TTX_ON==1))
static UINT8 prev_ttx[256];
#endif
#if( SUBTITLE_ON == 1)
static UINT8 prev_sub[256];
#endif


#ifdef NO_TTX_DESCRIPTOR
UINT8 ttxflag = FALSE;
#endif

static UINT8 ttx_reset_flag = 0;
static UINT8 subt_reset_flag = 0;

void ttx_descriptor_reset()
{
	ttx_reset_flag = 1;
}


void subt_descriptor_reset()
{
	subt_reset_flag = 1;
}

static BOOL hold_ttx_info = FALSE;
void hold_ttx_enable(BOOL enable)
{
	hold_ttx_info = enable;
}
BOOL ttx_is_holded()
{
	return hold_ttx_info;
}

INT32 ttx_handler(UINT8 tag, UINT8 length, UINT8 *content, void *priv)
{
	if( ttx_is_holded() )
		return SI_SUCCESS;
#if(defined(TTX_ON)&&(TTX_ON==1))
	if (ttx_reset_flag || (MEMCMP(content, prev_ttx, length) != 0)||(prev_ttx[255]==0xFF))
	{
		ttx_reset_flag = 0;
		MEMSET(prev_ttx, 0, sizeof(prev_ttx));
		MEMCPY(prev_ttx, content, length);
		ttx_descriptor(tag, length, content, priv);
	}
#ifdef NO_TTX_DESCRIPTOR //cloud
    else if(length == 0 && content!=NULL && ttxflag == FALSE) //for some special stream without ttx desc
    {
        ttx_descriptor(tag, length, content, priv);
        ttxflag = TRUE;
    }
#endif    
#endif    
	return SI_SUCCESS;
}

INT32 sub_handler(UINT8 tag, UINT8 length, UINT8 *content, void *priv)
{
#if(SUBTITLE_ON == 1)
	if (subt_reset_flag || (MEMCMP(content, prev_sub, length) != 0)||(prev_sub[255]==0xFF))
	{
		subt_reset_flag = 0;
		MEMSET(prev_sub, 0, sizeof(prev_sub));
		MEMCPY(prev_sub, content, length);
		subt_descriptor(tag, length, content, priv);
	}
#endif
	return SI_SUCCESS;
}
struct desc_table es_desc_info[] = {
/* Add switch:NEW_DEMO_FRAME
 * Reason: In project Sabbat dual, monitor id manage by APP Layer
 * and ttx, subt callback register by APP according to monitor id, when
 * PMT receeved, subt/ttxt callback will call by run_callback. so here
 * the PMT data path to ttx/subt (ttx_handler/sub_handler) should be blocked,
 * otherwise there two data paths to ttx/subt, and cause some problems.
 * In Glass project, monitor id manage by Middle Layer, APP not use so as
 * ttx_register, subt_register, so here have to use ttx_handler/sub_handler
 */
#ifndef NEW_DEMO_FRAME
	{
		TELTEXT_DESCRIPTOR,
		0,
		ttx_handler,
	},
	{
		SUBTITLE_DESCRIPTOR,
		0,
		sub_handler,
	},
#endif
};



#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT

UINT32  rec_monitor_id[DMX_NUM][EACH_DMX_MONITOR_NUM] = {{SIM_INVALID_MON_ID,SIM_INVALID_MON_ID},{SIM_INVALID_MON_ID,SIM_INVALID_MON_ID}};
UINT32  rec_monitor_id_prog[DMX_NUM][EACH_DMX_MONITOR_NUM]={{SIM_INVALID_MON_ID,SIM_INVALID_MON_ID},{SIM_INVALID_MON_ID,SIM_INVALID_MON_ID}};
P_NODE rec_monitor_id_prog_node[DMX_NUM][EACH_DMX_MONITOR_NUM]={{0,0},{0,0}};
struct sim_cb_param g_monitor_pmt_sec[DMX_NUM][EACH_DMX_MONITOR_NUM];

UINT8 get_dynamic_pid_program(UINT16 dmx_idx,UINT32 pg_id,P_NODE *node)
{
	UINT8 result=TRUE;
	if(rec_monitor_id_prog_node[dmx_idx][0].prog_id==pg_id)
		MEMCPY(node,&rec_monitor_id_prog_node[dmx_idx][0],sizeof(P_NODE));
	else if(rec_monitor_id_prog_node[dmx_idx][1].prog_id==pg_id)
		MEMCPY(node,&rec_monitor_id_prog_node[dmx_idx][1],sizeof(P_NODE));
	else
		result=FALSE;
	return result;
}

INT32 ttx_handler_rec(UINT32 param,UINT8 tag, UINT8 length, UINT8 *content, void *priv)
{
	UINT8 prog_index=0, ttx_index=0;
	P_NODE p_node;
    struct t_ttx_lang *ttx_list = NULL, *p_ttx = NULL;
	UINT8 num;
	UINT16 pid = *((UINT16*)priv);
	
	struct sim_cb_param dynamic_param;
	MEMCPY((&dynamic_param),(struct sim_cb_param *)param,sizeof(struct sim_cb_param));
	
	for(prog_index=0 ; prog_index < 2 ;prog_index++)
	{
		if(rec_monitor_id[dynamic_param.dmx_idx][prog_index]==dynamic_param.sim_id)
			break;
	}
	
	if(prog_index >=2)
		return SI_SUCCESS;
#if(defined(TTX_ON)&&(TTX_ON==1))
	MEMCPY(&p_node,&rec_monitor_id_prog_node[dynamic_param.dmx_idx][prog_index], sizeof(P_NODE));
    if (dynamic_param.table == MONITE_TB_PMT)//&&(p_node.teletext_pid!= pid))
	{
		rec_ttx_descriptor(dynamic_param.dmx_idx,prog_index,tag, length, content, priv);//init rec ttx

		Rec_Monitor_TTXEng_GetInitLang(dynamic_param.dmx_idx,prog_index,&ttx_list, &num);
    		
    	for(ttx_index=0; ttx_index<num; ttx_index++)
    	{
    		p_ttx = &ttx_list[ttx_index];
    		if (p_ttx->pid == pid)
    	    	return SI_SUCCESS;
    	}

		Rec_Monitor_TTXEng_GetSubtLang(dynamic_param.dmx_idx,prog_index,&ttx_list, &num);
    		
    	for(ttx_index=0; ttx_index<num; ttx_index++)
    	{
    		p_ttx = &ttx_list[ttx_index];
    		if (p_ttx->pid == pid)
    	    	return SI_SUCCESS;
    	}
		
		p_node.teletext_pid=pid;
		MEMCPY(&rec_monitor_id_prog_node[dynamic_param.dmx_idx][prog_index], &p_node, sizeof(P_NODE));		
		g_pid_change_flag[dynamic_param.dmx_idx][prog_index]=1;
	}
#ifdef NO_TTX_DESCRIPTOR //cloud
    else if(length == 0 && content!=NULL && ttxflag == FALSE) //for some special stream without ttx desc
    {
        rec_ttx_descriptor(dynamic_param.dmx_idx,prog_index,tag, length, content, priv);
        ttxflag = TRUE;
    }
#endif    
#endif    
	return SI_SUCCESS;
}

INT32 sub_handler_rec(UINT32 param,UINT8 tag, UINT8 length, UINT8 *content, void *priv)
{
	UINT8 prog_index=0,subt_num=0;
	UINT16 subt_index=0;
	P_NODE p_node;
	struct t_subt_lang *subt_list = NULL,*p_subt = NULL;
	UINT16 pid = *((UINT16*)priv);
	
	struct sim_cb_param dynamic_param;
	MEMCPY((&dynamic_param),(struct sim_cb_param *)param,sizeof(struct sim_cb_param));
	
	for(prog_index=0 ; prog_index < 2 ;prog_index++)
	{
		if(rec_monitor_id[dynamic_param.dmx_idx][prog_index]==dynamic_param.sim_id)
			break;
	}
	
	if(prog_index >=2)
		return SI_SUCCESS;
	
#if(SUBTITLE_ON == 1)
	MEMCPY(&p_node,&rec_monitor_id_prog_node[dynamic_param.dmx_idx][prog_index], sizeof(P_NODE));
    	//if ((dynamic_param.table == MONITE_TB_PMT)&&(p_node.subtitle_pid != pid))
	rec_subt_descriptor(dynamic_param.dmx_idx,prog_index,tag, length, content, priv);
    if ((dynamic_param.table == MONITE_TB_PMT))//&&(p_node.subtitle_pid != pid))
	{
    	Rec_Monitor_subt_get_language(dynamic_param.dmx_idx,prog_index,&subt_list, &subt_num);
    		
    	for(subt_index=0; subt_index<subt_num; subt_index++)
    	{
    		p_subt = &subt_list[subt_index];
    		if (p_subt->pid == pid)
    	    	return SI_SUCCESS;
    	}
    	p_node.subtitle_pid=pid;
    	//rec_subt_descriptor(dynamic_param.dmx_idx,prog_index,tag, length, content, priv);
    	MEMCPY(&rec_monitor_id_prog_node[dynamic_param.dmx_idx][prog_index], &p_node, sizeof(P_NODE));
    	g_pid_change_flag[dynamic_param.dmx_idx][prog_index]=1;
	}
#endif
				
	return SI_SUCCESS;
}

struct desc_table rec_es_desc_info[] = {
	{
		TELTEXT_DESCRIPTOR,
		0,
		ttx_handler_rec,
	},
	{
		SUBTITLE_DESCRIPTOR,
		0,
		sub_handler_rec,
	},
};

static void rec_psi_es_info_checkup(UINT32 param,UINT8 *buff, INT32 len, struct desc_table *info, INT32 desc_num)
{
	INT32 prog_info_length;
	INT32 es_info_length;
	INT32 i;
	UINT16 es_pid;
	
	struct pmt_section *pms = (struct pmt_section *)buff;
	struct pmt_stream_info *stream;
	prog_info_length = SI_MERGE_HL8(pms->program_info_length);
	for(i=sizeof(struct pmt_section)+prog_info_length-4; i<len-4; i+=es_info_length+sizeof(struct pmt_stream_info)) {
		stream = (struct pmt_stream_info *)(buff+i);
		es_info_length = SI_MERGE_HL8(stream->es_info_length);
		es_pid = SI_MERGE_HL8(stream->elementary_pid);
		rec_desc_loop_parser(param,stream->descriptor, es_info_length, info, desc_num, NULL, (void *)&es_pid);
	}
}

#endif
static void psi_es_info_checkup(UINT8 *buff, INT32 len, struct desc_table *info, INT32 desc_num)
{
	INT32 prog_info_length;
	INT32 es_info_length;
	INT32 i;
	UINT16 es_pid;
	
	struct pmt_section *pms = (struct pmt_section *)buff;
	struct pmt_stream_info *stream;
	prog_info_length = SI_MERGE_HL8(pms->program_info_length);
	for(i=sizeof(struct pmt_section)+prog_info_length-4; i<len-4; i+=es_info_length+sizeof(struct pmt_stream_info)) {
		stream = (struct pmt_stream_info *)(buff+i);
		es_info_length = SI_MERGE_HL8(stream->es_info_length);
		es_pid = SI_MERGE_HL8(stream->elementary_pid);
		desc_loop_parser(stream->descriptor, es_info_length, info, desc_num, NULL, (void *)&es_pid);
	}
}

static INT32 monitor_pmt_sec(UINT8 *section, INT32 length, UINT32 param)
{
#if (ISDBT_CC == 1)
	psi_es_info_checkup(section, length, es_desc_info, 3);
#else
	psi_es_info_checkup(section, length, es_desc_info, 2);
#endif
	return 0;
}
#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
static INT32 rec_monitor_pmt_sec(UINT8 *section, INT32 length, UINT32 param)
{
	UINT8 prog_index=0;
	P_NODE p_node;
	
	struct sim_cb_param dynamic_param;
	MEMCPY((&dynamic_param),(struct sim_cb_param *)param,sizeof(struct sim_cb_param));
	
	for(prog_index=0 ; prog_index < 2 ;prog_index++)
	{
		if(rec_monitor_id[dynamic_param.dmx_idx][prog_index]==dynamic_param.sim_id)
			 	break;
	}
	if (prog_index ==2)
		return 0;
			
	MEMCPY(&g_monitor_pmt_sec[dynamic_param.dmx_idx][prog_index],(struct sim_cb_param *)param,sizeof(struct sim_cb_param));
	
	rec_psi_es_info_checkup((UINT32)(&g_monitor_pmt_sec[dynamic_param.dmx_idx][prog_index]),section, length, rec_es_desc_info, 2);


	if((rec_on_change)&&(g_pid_change_flag[dynamic_param.dmx_idx][prog_index]==1))
	{
			
			MEMCPY(&p_node,&rec_monitor_id_prog_node[dynamic_param.dmx_idx][prog_index], sizeof(P_NODE));	
	    		//modify_prog(p_node.prog_id, &p_node);
	              //update_data();
			  
			rec_on_change(p_node.prog_id);

			g_pid_change_flag[dynamic_param.dmx_idx][prog_index]=0;
			
	}	
	//osal_task_dispatch_on();	
	return 0;
}
#endif

static INT32 pmt_callback(UINT8 *section, INT32 length, UINT32 param)
{
	callback_run(si_pmt_callback, MAX_SI_CALLBACK_CNT, section, length,
		#ifdef SI_MONITOR_CHOOSE_SERVICE_ID
			program.prog_number
		#else
			param 	//m_CurChanIndex
		#endif
		);
	return 0;
}

UINT32 pat_monitor_id = SIM_INVALID_MON_ID;

INT32 si_monitor_on(UINT32 index)
{
	P_NODE node;
	INT32 ret = SUCCESS;
	
	struct dmx_device * dmx;

	if(monitor_id != SIM_INVALID_MON_ID)
		return SUCCESS;

#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)
	#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
	dmx =(struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, (lib_nimng_get_nim_play()-1));
	#else
	dmx = (struct dmx_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DMX);
	#endif
#else
	dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, monitor_dmx_id);
#endif


	SIM_PRINTF("si monitor on\n");
		
	if(get_prog_at(index, &node) != SUCCESS)
	{
		return ERR_FAILUE;
	}

	m_CurChanIndex = index;
	
	pat_monitor_id = sim_start_monitor(dmx, MONITE_TB_PAT, PSI_PAT_PID, 0);

	if(node.user_modified_flag==0 && node.pmt_pid != 0x0 && node.pmt_pid != 0x1FFF)
	{
		monitor_id = sim_start_monitor(dmx, MONITE_TB_PMT, node.pmt_pid, node.prog_number);

		MEMCPY(&program, &node, sizeof(P_NODE));

		sim_register_ncb(monitor_id, notify_ap);

		sim_register_scb(monitor_id, monitor_pmt_sec, NULL);

		sim_register_scb(monitor_id, pmt_callback, NULL);

	}	
//	else
	{
		/*No PMT PID inside PAT table, turn on PAT monitor to retrieval PMT PID */
		sim_register_scb(pat_monitor_id, monitor_pat_callback, NULL);
	}

	return ret;
}


#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT

UINT8 rec_check_pmt_monitor(UINT32 monitor_offid)
{
	UINT8 i=0,j=0,prog_index=2;
	INT32 ret = TRUE;
	
	for(i=0;i<2;i++)
		{
			for(j=0;j<2;j++)
				{
					if(rec_monitor_id[i][j]==monitor_offid)
						{
							 ret = FALSE;
							break;
						}
				}
		}
	return ret;
}


/*
*	name:	api_sim_callback
*	in	:	param( Struct sim_cb_param )
*	out	:	SUCCESS or ERR_FAILUE
*	function:	to be registered in sim,  send info "CTRL_MSG_SUBTYPE_CMD_PIDCHANGE" at runtime when the PID monitored changed
*/

static UINT32 moni_prog=0;


INT32 api_sim_callback(UINT32 param)
{
	struct sim_cb_param *sim_param = (struct sim_cb_param *)param;

    INT32 ret = ERR_FAILUE;
	UINT16 sim_pcr_pid;
	UINT16 sim_video_pid;
	UINT16 sim_audio_pid[P_MAX_AUDIO_NUM];
	UINT16 sim_audio_count = P_MAX_AUDIO_NUM;
	static BOOL on_process = FALSE;
	struct prog_info p_info;
    /*
    while(on_process)
    {
        osal_task_sleep(10);
    }
    */
    on_process = TRUE;

	//static var to minitor the ecm and emm pids
	static UINT16 ecm_pids[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	
	static UINT16 emm_pids[16]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	UINT16 sie_ecm_pids[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	UINT16 sie_emm_pids[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};	
	UINT8 max_ecm_cnt = 16;
	UINT8 max_emm_cnt = 16;

	//PROG_INFO sim_p_info; // get from sie_monitor
	P_NODE p_node; // Come from get_prog_by_id
	//struct record_prog_param r_p_info; //to update record info
	INT8 sim_idx = -1, i;
	UINT8 prog_index=0;
	

	for(prog_index=0 ; prog_index < 2 ;prog_index++)
	{
		if(rec_monitor_id[sim_param->dmx_idx][prog_index]==sim_param->sim_id)
			break;
	}
	
	if(prog_index >=2)
		return ret;
		
	//moni_prog = rec_monitor_id_prog[sim_param->dmx_idx][prog_index];

    //sim_information* info = api_get_sim_info_slot_by_sim(sim_param->dmx_idx, sim_param->sim_id);
   // moni_prog ++;
    do{
    	if (sim_param->table == MONITE_TB_PMT)
    	{
    		
    		sim_video_pid = sim_pcr_pid = 8191;
    		MEMSET(sim_audio_pid, 8191, P_MAX_AUDIO_NUM);

    		if (ERR_FAILUE == sim_get_video_pid(sim_param->sim_id, &sim_video_pid))
    			break;
			sim_video_pid &= 0x1fff;
    		//sim_get_pmt(sim_param->sim_id, &sim_p_info);
    		if (ERR_FAILUE == sim_get_audio_info(sim_param->sim_id, sim_audio_pid, NULL, &sim_audio_count))	
    			break;
    		if (ERR_FAILUE== sim_get_pcr_pid(sim_param->sim_id, &sim_pcr_pid))
			break;
            
    		if (ERR_FAILUE == sim_get_ecm_pid(sim_param->sim_id,sie_ecm_pids,&max_ecm_cnt))
    			break;

			sim_get_pmt(rec_monitor_id[sim_param->dmx_idx][prog_index], &p_info);
    		//get_prog_by_id(moni_prog, &p_node);
			MEMCPY(&p_node,&rec_monitor_id_prog_node[sim_param->dmx_idx][prog_index], sizeof(P_NODE));
			
            if(p_node.pmt_pid != sim_param->sec_pid)
    		{
               break;
    		}
    		            
			if((sim_video_pid != p_node.video_pid) 
    			|| (sim_pcr_pid != p_node.pcr_pid) 
    			//||(p_info.subtitle_pid != p_node.subtitle_pid) 
    			//||(p_info.teletext_pid!= p_node.teletext_pid) 
    			||(sim_audio_count != p_node.audio_count) 
    			||(MEMCMP(p_node.audio_pid,sim_audio_pid, sizeof(UINT16)*sim_audio_count)!=0))
    		{

     			if(PROG_TV_MODE ==p_node.av_flag)
    				 p_node.video_pid = sim_video_pid;			
    			
				p_node.pcr_pid = sim_pcr_pid;
				p_node.audio_count = sim_audio_count;
				MEMCPY(p_node.audio_pid, sim_audio_pid, sizeof(UINT16)*sim_audio_count);

    			if ((p_node.ca_mode)
    				&&( max_ecm_cnt != 0)
    				&&(MEMCMP(ecm_pids,sie_ecm_pids, sizeof(UINT16)*max_ecm_cnt)!=0))
    			{
    				for (i=0;i<max_ecm_cnt;i++)
    				{
    					libc_printf("ecm pid changed![ %d] -> [ %d]\n",ecm_pids[i],sie_ecm_pids[i]);
    				}
    				libc_printf("Dynamic PID cause ECM changed!\n Need to update code to support it\n");
    				MEMCPY(ecm_pids,sie_ecm_pids, sizeof(UINT16)*max_ecm_cnt);

    			}	
				g_pid_change_flag[sim_param->dmx_idx][prog_index]=1;
				MEMCPY(&rec_monitor_id_prog_node[sim_param->dmx_idx][prog_index], &p_node, sizeof(P_NODE));
    		}
    	}

    		//CA mode to update emm pid  pid
    	if ( sim_param->table == MONITE_TB_CAT )
    	{
    		if (ERR_FAILUE == sim_get_emm_pid(sim_param->sim_id,sie_emm_pids,&max_emm_cnt))
    			break;
    		if ((p_node.ca_mode) 
    			&&(max_emm_cnt != 0)
    			&& (MEMCMP(emm_pids,sie_ecm_pids, sizeof(UINT16)*max_emm_cnt)!=0))
    		{
    			for (i=0;i<max_emm_cnt;i++)
    			{
    				libc_printf("emm pid changed![ %d] -> [ %d]\n",emm_pids[i],sie_emm_pids[i]);
    			}
    			libc_printf("Dynamic PID cause EMM changed!\n Need to update code to support it\n");
    			MEMCPY(emm_pids,sie_emm_pids, sizeof(UINT16)*max_emm_cnt);

    		}
    	}
        ret = SUCCESS;
        
    }while(0);
    on_process = FALSE;
	return ret;
}

INT32 rec_si_monitor_on(UINT8 dmx_id,UINT32 index)
{
	P_NODE node;
	INT32 ret = SUCCESS;
	UINT8 prog_index=0;
	
	struct dmx_device * dmx;

	dmx =(struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, (dmx_id));
		
	if(get_prog_by_id(index, &node) != SUCCESS)
	{
		return ERR_FAILUE;
	}
				
	if(node.user_modified_flag==0 && node.pmt_pid != 0x0 && node.pmt_pid != 0x1FFF)
	{
		if(rec_monitor_id[dmx_id][0]==SIM_INVALID_MON_ID)
		{
			prog_index=0;
			rec_monitor_id[dmx_id][prog_index] = sim_start_monitor(dmx, MONITE_TB_PMT, node.pmt_pid, node.prog_number);
				
			if(rec_monitor_id[dmx_id][prog_index]!=SIM_INVALID_MON_ID)
				rec_monitor_id_prog[dmx_id][prog_index]=node.prog_id;
		}
		else if(rec_monitor_id[dmx_id][1]==SIM_INVALID_MON_ID)
		{
			prog_index=1;
			rec_monitor_id[dmx_id][prog_index] = sim_start_monitor(dmx, MONITE_TB_PMT, node.pmt_pid, node.prog_number);
			if(rec_monitor_id[dmx_id][prog_index]!=SIM_INVALID_MON_ID)
				rec_monitor_id_prog[dmx_id][prog_index]=node.prog_id;				
		}
		else
		{
			prog_index=0;
		}
		
		MEMCPY(&rec_monitor_id_prog_node[dmx_id][prog_index], &node, sizeof(P_NODE));

		sim_register_ncb(rec_monitor_id[dmx_id][prog_index] , api_sim_callback);

		sim_register_scb(rec_monitor_id[dmx_id][prog_index] ,rec_monitor_pmt_sec, NULL);

	}	

	return ret;
}
#endif
void si_monitor_off(UINT32 index)
{
	INT32 ret = SUCCESS;
	UINT16 stuff_pid = 0x1FFF;
	P_NODE node;
	
	if(monitor_id == SIM_INVALID_MON_ID)
		return;

	SIM_PRINTF("si monitor off\n");

	m_CurChanIndex = 0xffff;


#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT
	if(rec_check_pmt_monitor(monitor_id)==TRUE)
		{
			//soc_printf("si_off(m_id==>%d)\n",monitor_id);
	sim_stop_monitor(monitor_id);
		}
	else
		{	//soc_printf("sim_un_ncb=>%d \n",monitor_id);
			sim_unregister_ncb(monitor_id,notify_ap);
			//sim_register_ncb(monitor_id, api_sim_callback);
			sim_unregister_scb(monitor_id,monitor_pmt_sec);
			//sim_register_scb(monitor_id, rec_monitor_pmt_sec, NULL);			
		}
#else
	sim_stop_monitor(monitor_id);
#endif

	monitor_id = SIM_INVALID_MON_ID;

	if ((index==0xFFFFFFFF)||(get_prog_at(index, &node)== SUCCESS)) {
		if(index == 0xFFFFFFFF)
		{
			SIM_PRINTF("	2.index=0xffffffff, call undo_prog_modify()\n");
			undo_prog_modify(program.tp_id, program.prog_number);			
		}
	}

#if(defined(TTX_ON)&&(TTX_ON==1))
		if(ttx_is_holded() == FALSE)
		{
			MEMSET(prev_ttx, 0xFF, sizeof(prev_ttx));
			ttx_descriptor(TELTEXT_DESCRIPTOR, 0, prev_ttx, (void *)&stuff_pid);
		}
#endif
#if( SUBTITLE_ON == 1)
		MEMSET(prev_sub, 0xFF, sizeof(prev_sub));
		subt_descriptor(SUBTITLE_DESCRIPTOR, 0, prev_sub, (void *)&stuff_pid);
#endif

	sim_stop_monitor(pat_monitor_id);
	pat_monitor_id = SIM_INVALID_MON_ID;

	return;
}

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT

UINT8 get_rec_monitor_index(UINT8 dmx_id,UINT32 pro_id)
{
	UINT8 index=2;
	UINT8 i=0;
	P_NODE p_node;
	for(i=0;i<2;i++)
		{
			MEMCPY(&p_node,&rec_monitor_id_prog_node[dmx_id][i], sizeof(P_NODE));
			if(p_node.prog_id==pro_id)
				{
					index=i;
					break;
				}
		}
	return index;
}


void rec_si_monitor_off(UINT8 dmx_id,UINT32 index,UINT8 off_sie)
{
	INT32 ret = SUCCESS;
	UINT16 stuff_pid = 0x1FFF;
	P_NODE node;
	UINT8 i=0,prog_index=2;

	for(i=0;i<2;i++)
		{
			if(rec_monitor_id_prog[dmx_id][i]==index)
				{
					prog_index=i;
					break;
				}
		}
	
	if(prog_index==2)
		return;
	
	if(rec_monitor_id[dmx_id][prog_index] == SIM_INVALID_MON_ID)
		return;

	SIM_PRINTF("=============== \n");

	SIM_PRINTF("rec_si_monitor_off\n");

	SIM_PRINTF("rec_monitor_id[%d][%d] \n",dmx_id,prog_index);
	
	if(off_sie==1)
	{
		sim_stop_monitor(rec_monitor_id[dmx_id][prog_index]);
	}
	else 
	{
		sim_unregister_ncb(rec_monitor_id[dmx_id][prog_index],api_sim_callback);
		sim_unregister_scb(rec_monitor_id[dmx_id][prog_index],rec_monitor_pmt_sec);
	}
	/*! \bug BUG39626 & BUG39491
	 * Description - 3105_glass project: Open Timeshift, then change channel. Each switch ensure timeshift
	 * already begin, do like that about 10 times, SW can't get PMT table correctly.
	 * Reason - Because every time change channel, pmt monitor didn't stop, about 10 times
	 * all resource( short_buff ) exhaust. SI Monitor can't monitor PMT any more.
	 */
	if (rec_monitor_id[dmx_id][prog_index] != monitor_id )
		sim_stop_monitor(rec_monitor_id[dmx_id][prog_index]);
	rec_monitor_id[dmx_id][prog_index] = SIM_INVALID_MON_ID;
	rec_monitor_id_prog[dmx_id][prog_index]=SIM_INVALID_MON_ID;
	MEMSET(&rec_monitor_id_prog_node[dmx_id][prog_index], 0 ,sizeof(P_NODE));

	return;
}
#endif
void si_monitor_register(on_pid_change_t pid_change)
{
	on_change = pid_change;
	return;
}

void si_monitor_pre_register(on_pid_change_t pid_change)
{
	pre_change = pid_change;
}

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT

void rec_si_monitor_register(on_rec_pid_change_t pid_change)
{
	rec_on_change = pid_change;
	return;
}
#endif
INT32 si_monitor_register_pmt_cb(section_parse_cb_t callback)
{
	return register_cb(si_pmt_callback, MAX_SI_CALLBACK_CNT, callback);
}

INT32 si_monitor_unregister_pmt_cb(section_parse_cb_t callback)
{
	return unregister_cb(si_pmt_callback, MAX_SI_CALLBACK_CNT, callback);
}


INT32 pmt_monitor_on(UINT16 pmt_pid, UINT16 prog_number/*service_id*/)
{
	INT32 ret = SUCCESS;
	
	struct dmx_device * dmx;

	if(monitor_id != SIM_INVALID_MON_ID)
		return SUCCESS;

	dmx = (struct dmx_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DMX);

	SIM_PRINTF("si monitor on\n");
		

	monitor_id = sim_start_monitor(dmx, MONITE_TB_PMT, pmt_pid, prog_number);

	sim_register_scb(monitor_id, pmt_callback, NULL);


	return ret;
}

void pmt_monitor_off(UINT16 pmt_pid)
{
	INT32 ret = SUCCESS;

	struct dmx_device * dmx;

	if(monitor_id == SIM_INVALID_MON_ID)
		return;

	SIM_PRINTF("si monitor off\n");

	dmx = (struct dmx_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_DMX);
		
	sim_stop_monitor(monitor_id);

	monitor_id = SIM_INVALID_MON_ID;

	return;
}
#ifdef GET_CAT_BY_SIM
INT32 si_monitor_register_cat_cb(section_parse_cb_t callback)
{
	return register_cb(si_cat_callback, MAX_SI_CALLBACK_CNT, callback);
}

INT32 si_monitor_unregister_cat_cb(section_parse_cb_t callback)
{
	return unregister_cb(si_cat_callback, MAX_SI_CALLBACK_CNT, callback);
}

static INT32 cat_callback(UINT8 *section, INT32 length, UINT32 param)
{
	callback_run(si_cat_callback, MAX_SI_CALLBACK_CNT, section, length,param);
	return 0;
}
#endif
UINT32 cat_monitor_id = SIM_INVALID_MON_ID;

void sie_start_emm_service()
{
	struct dmx_device * dmx;

	dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, monitor_dmx_id);
	
	cat_monitor_id = sim_start_monitor(dmx, MONITE_TB_CAT, 0x01, 0);
	#ifdef GET_CAT_BY_SIM
	sim_register_scb(cat_monitor_id, cat_callback, NULL);
	#endif
}


void sie_stop_emm_service()
{
	sim_stop_monitor(cat_monitor_id);
	cat_monitor_id = SIM_INVALID_MON_ID;
}

//#if(defined(_MHEG5_V20_ENABLE_))
//#if(defined(_MHEG5_V20_ENABLE_))
UINT16 si_get_cur_channel(void)
{
	return m_CurChanIndex;
}
//#endif


static INT8 si_get_service_pid(UINT8 *pbuf, UINT16 serviceid, UINT16 *pservicepid)
{
    UINT16 prognum = pbuf[0]<<8 | pbuf[1];
    if(serviceid == prognum)
    {
        *pservicepid = (pbuf[2]&0x1F)<<8 | pbuf[3];
        return SUCCESS;
    }
    pservicepid = NULL;
    return ERR_FAILUE;
}

static INT32 monitor_pat_callback(UINT8 *section, INT32 length, UINT32 param)
{
	P_NODE node;		
	struct dmx_device * dmx;

	UINT16 prog_map_len;
	UINT16 prog_number;
	UINT8 *prog_map = NULL;
	UINT8 same_prog_number = 0;
	UINT16 pmt_pid = 0;
	UINT8 ret = 0;
	struct sim_cb_param *cb_param = (struct sim_cb_param *)param;

	//if(monitor_id != SIM_INVALID_MON_ID)
	//	return SUCCESS;

	if(get_prog_at(si_get_cur_channel(), &node) != SUCCESS)
	{
		return SUCCESS;
	}

	dmx = (struct dmx_device *)(struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, monitor_dmx_id);

	if (section!=NULL) 
	{

		/*get PMT pid using program number*/
		prog_map_len = (((section[1]&0xF)<<8)|section[2]) - 9;
		prog_map = section + 8;

		/*get PMT pid using program number*/
		while(prog_map_len > 0)
		{               
			if( SUCCESS == si_get_service_pid(prog_map, node.prog_number, &pmt_pid))
			{
				if(node.pmt_pid == pmt_pid)//program's pmt_pid exist, stop parse PAT
				{
					same_prog_number = 0;
				}
				else
				{
					same_prog_number++;
					SIM_PRINTF("-----si_monitor_on_pat_parsing()[si_monitor.c]2.prog_number %d match, pmt pid=%d\n ",
					prog_number,pmt_pid);
				}
			}

			prog_map_len -= 4;
			prog_map += 4;    
		}

		/*Actually, this callback doesn't care multi-section issue casue siae would send section continuously*/
		if(sim_get_prog_pmt_pid(pat_monitor_id,node.prog_number,&pmt_pid) == 1)
		{
			
			/*close pat monitor callback cause pmt pid has been retrevial*/
			//sim_unregister_scb(pat_monitor_id, monitor_pat_callback);
	              //if( monitor_id != SIM_INVALID_MON_ID)
	              if(node.pmt_pid != pmt_pid)
	              {
				  	if(monitor_id != SIM_INVALID_MON_ID)
	              {
						sim_stop_monitor(monitor_id);
						monitor_id = SIM_INVALID_MON_ID;
				  	}
			/*Turn on PMT receive filter*/	
			node.pmt_pid = pmt_pid;

			modify_prog(node.prog_id, &node);
			update_data();

			monitor_id = sim_start_monitor(dmx, MONITE_TB_PMT, node.pmt_pid, node.prog_number);

			MEMCPY(&program, &node, sizeof(P_NODE));

			sim_register_ncb(monitor_id, notify_ap);

			sim_register_scb(monitor_id, monitor_pmt_sec, NULL);
			sim_register_scb(monitor_id, pmt_callback, NULL);

             }
		}
	//	else if(same_prog_number > 1)
	//	{
	//		/*just prevent callback not to close immediately*/
	//		sim_unregister_scb(pat_monitor_id, monitor_pat_callback);
	//	}
 
	}
}


