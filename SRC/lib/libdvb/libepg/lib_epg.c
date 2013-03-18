/*========================================================================================
 *
 * Copyright (C) 2006 ALi Shanghai Corp.
 *
 * Description:
 *
 * History
 *
 *   Date	    Author		Comment
 *   ========	===========	========================================
 *   ......
 *   2008/12/31  Yun You    Add section parse hitmap & epg full event
 *   2011/7/5    Steven     Simplified for Cable, for total number(<500) of tp & service
 *                          is much less than Sat.
 *   2011/7/6    Steven     Del services'(of other TPs) events & reset handle's hitmap
 *							when DB full.
 *   2011/9/5    Steven     国内cable,有的地方pf version始终=0!只好用CRC比较.
 *
 *=========================================================================================*/
#include <sys_config.h>

#include <retcode.h>
#include <types.h>

#include <osal/osal.h>
#include <api/libc/string.h>
#include <api/libtsi/db_3l.h>
#include <api/libtsi/sie.h>
#include "epg_common.h"
#include <api/libsi/lib_epg.h>
#include <api/libsi/si_eit.h>
#include "eit_parser.h"
#include "epg_db.h"


//#define _DEBUG

static OSAL_ID	lib_epg_flag = OSAL_INVALID_ID;  

#define INVALID_TP_ID	0xFFFFFFFF

#define RESERVED_VER		0x3F
#define RESERVED_CRC		0xFFFFFFFF

//国内cable,有的地方pf version始终=0!只好用CRC比较.
//#define EPG_UPDATE_BY_CRC


//section status
#define VERSION_MASK		0xCF


enum
{
	EQUAL,
	NOT_EQUAL,
};

typedef enum
{
	SECTION_EXIST 		 = 0xFF,
	ERROR_NO_SERVICE 	 = 0x01,
	ERROR_PF_SECTION_NUM = 0x02,
	ERROR_SERVICE_FULL	 = 0x03,

	PF_SECTION_UPDATE 	 = 0x00, // bit 4~5: [0,0]
	PF_VERSION_UPDATE 	 = 0x10,	//[0,1]
	SCH_SECTION_UPDATE 	 = 0x20, //[1,0]
	SCH_VERSION_UPDATE 	 = 0x30, //[1,1]

} section_status;

//sevice section hitmap
struct SERVICE_T
{
	UINT32 tp_id;
	UINT16 service_id;

	//hitmap for event
	UINT8  pf_sec_mask: 2;
#ifdef EPG_UPDATE_BY_CRC
	UINT32 p_crc;	//section 0's crc
#else
	UINT8  pf_ver: 6;
#endif

	//hitmap for handle
	UINT8  pf_sec_mask_h: 2;
#ifdef EPG_UPDATE_BY_CRC
	UINT32 p_crc_h;
#else
	UINT8  pf_ver_h: 6;
#endif

	//hitmap for event
	UINT8  sch_sec_mask[SCHEDULE_TABLE_ID_NUM][32]; // 32 segment , each 8 section.
#ifdef EPG_UPDATE_BY_CRC
	UINT32 sch_crc[SCHEDULE_TABLE_ID_NUM];	//section 0's crc
#else
	UINT8  sch_ver[SCHEDULE_TABLE_ID_NUM];	//a table, a version
#endif

	//hitmap for handle
	UINT8  sch_sec_mask_h[SCHEDULE_TABLE_ID_NUM][32]; // 32 segment , each 8 section.
#ifdef EPG_UPDATE_BY_CRC
	UINT32 sch_crc_h[SCHEDULE_TABLE_ID_NUM];
#else
	UINT8  sch_ver_h[SCHEDULE_TABLE_ID_NUM];	//a table, a version
#endif
};

//epg control block, for sie & ap callback
static struct epg_info_t
{
	UINT8* buffer;	//buffer for sie's filter
	UINT32 buf_len;
	BOOL   internal_buf;	//internal alloced buffer or external buffer

	struct sie_eit_config config;
	UINT32 tp_id;
	UINT16 service_id;

	EIT_CALL_BACK epg_call_back;

	//service list of current tp
	struct SERVICE_T *service;	//以service_id从小到大的顺序排列
	UINT16 service_num;

#ifdef EPG_OTH_SCH_SUPPORT
	UINT16 pre_sat_id;
	UINT16 tp_id_num;
	UINT32 tp_ids[MAX_TP_COUNT][2];		//tp_id table of a sat. 1024 for sat, 128 for cable.
								//[0]: (ts_id, o_net_id)
								//[1]: tp_id
#endif

} epg_info;

//epg module status
static 	enum EPG_STATUS	epg_status = STATUS_NOT_READY;

void epg_enter_mutex()
{
	UINT32 flag;
	osal_flag_wait(&flag, lib_epg_flag, EPG_FLAG_MUTEX, 
				OSAL_TWF_ANDW|OSAL_TWF_CLR, OSAL_WAIT_FOREVER_TIME);
}
void epg_leave_mutex()
{
	osal_flag_set(lib_epg_flag, EPG_FLAG_MUTEX);
}

//reset the epg module, includes db block, mem block & service list
static void reset_epg_internal()
{
	EPG_PRINT("%s\n\n",__FUNCTION__);
	reset_epg_db();
	x_reset_mem();
	epg_info.service_num = 0;
}

//reset all
INT32 epg_reset()
{
	EPG_PRINT("\n>>>>>>>>>>>>>>>>>\n");
	EPG_PRINT("%s\n",__FUNCTION__);

	if (epg_status != STATUS_OFF)
	{
		EPG_PRINT("epg_reset - status: %d error!\n",epg_status);
		return !SUCCESS;
	}
	
	epg_enter_mutex();
	
	reset_epg_internal();

#ifdef EPG_OTH_SCH_SUPPORT
	epg_info.tp_id_num = 0;
	epg_info.pre_sat_id = 0xFFFF;
#endif

	epg_leave_mutex();

	EPG_PRINT("<<<<<<<<<<<<<<<<<<<<\n");
	return SUCCESS;
}

static void reset_hitmap(struct SERVICE_T *service)
{
	service->pf_sec_mask_h= 0;
#ifdef EPG_UPDATE_BY_CRC
	service->p_crc_h= RESERVED_CRC;
#else
	service->pf_ver_h= RESERVED_VER;
#endif
	MEMSET(service->sch_sec_mask_h, 0, 32 * SCHEDULE_TABLE_ID_NUM);
#ifdef EPG_UPDATE_BY_CRC
	INT i;
	for (i=0;i<SCHEDULE_TABLE_ID_NUM;i++)
		service->sch_crc_h[i] = RESERVED_CRC;
#else
	MEMSET(service->sch_ver_h,RESERVED_VER, SCHEDULE_TABLE_ID_NUM);
#endif
}

//Del service's events whose tp_id != tp_id & !active service, reset service hitmap.
//Events deleted, service still exists.
//When epg db full, epg module will call this function to del other services's events.
static INT32 epg_del_events(UINT32 tp_id, UINT32 param)
{
	INT32 i;
	INT32 count = 0;	//del count
	struct SERVICE_T *service;

	EPG_PRINT("[epg] %s: tp ==/!= 0x%X by param %d\n",__FUNCTION__,tp_id,param);
	if (param == EQUAL)
	{
		count = del_events(tp_id, 0xFFFF, 0, PRESENT_EVENT_MASK|FOLLOWING_EVENT_MASK|SCHEDULE_EVENT_MASK);
	}
	else if (param == NOT_EQUAL)
	{
		count = del_events(tp_id, 0, 0, PRESENT_EVENT_MASK|FOLLOWING_EVENT_MASK|SCHEDULE_EVENT_MASK);
	}
	else
	{
		ERR_PRINT("%s: param %d error!\n",__FUNCTION__,param);
		return 0;
	}

	for (i=0; i< epg_info.service_num; i++)
	{
		service = &epg_info.service[i];
		if ((param == NOT_EQUAL && service->tp_id != tp_id)
			|| (param == EQUAL && service->tp_id == tp_id))
		{
			//if (!epg_check_active_service(service->tp_id,service->service_id))
			{
				reset_hitmap(service);
			}
		}
	}

	if (count == 0)
	{
		ERR_PRINT("\n!@#$ [epg] error: del 0 event, pls check buffer size(shall > one tp of events)!!! ^&*()\n\n");
	}
	else
	{
		EPG_PRINT("[epg] total del %d events\n",count);
	}

	return count;
}

//remove one service's hitmap & all events.
INT32 epg_del_service(UINT32 tp_id, UINT16 service_id)
{
	INT32 i, j;
	INT32 count = 0;	//del count
	struct SERVICE_T *service;

	if (epg_status != STATUS_OFF)
	{
		EPG_PRINT("epg_del_service - status: %d error!\n",epg_status);
		return !SUCCESS;
	}

	EPG_PRINT("[epg] %s: tp = 0x%X, service = 0x%X\n",__FUNCTION__,tp_id,service_id);
	
	epg_enter_mutex();

	for (i=0; i< epg_info.service_num; i++)
	{
		service = &epg_info.service[i];
		if (service->tp_id == tp_id && service->service_id == service_id)
		{
			//EPG_PRINT("%s: del events of tp id 0x%X, service id 0x%X\n",__FUNCTION__,service->tp_id,service->service_id);
			count += del_events(service->tp_id,service->service_id, SI_EIT_TABLE_ID_APF&0x0F, PRESENT_EVENT_MASK|FOLLOWING_EVENT_MASK);
#ifdef EPG_OTH_SCH_SUPPORT
			count += del_events(service->tp_id,service->service_id, SI_EIT_TABLE_ID_OPF&0x0F, PRESENT_EVENT_MASK|FOLLOWING_EVENT_MASK);
#endif
			for (j=0; j<SCHEDULE_TABLE_ID_NUM; j++)
				count += del_events(service->tp_id,service->service_id, j, SCHEDULE_EVENT_MASK);

			//remove it from service list
			if (epg_info.service_num-1-i != 0)
				MEMMOVE(service, service+1, (epg_info.service_num-1-i)*sizeof(struct SERVICE_T));

			epg_info.service_num --;

			epg_leave_mutex();
			EPG_PRINT("[epg] %s: del %d events, left %d service\n",__FUNCTION__,count,epg_info.service_num);
			return SUCCESS;
		}
	}

	epg_leave_mutex();
	ERR_PRINT("[epg] %s: not find service (tp 0x%X, service 0x%X)\n",__FUNCTION__,tp_id,service_id);
	return !SUCCESS;
}

//get service struct from service list
static struct SERVICE_T* get_service(UINT32 tp_id, UINT16 service_id, UINT16 *idx)
{
	int m, n, mid;

	if (epg_info.service_num == 0)
		goto EXIT;
	if (tp_id < epg_info.service[0].tp_id
		|| (tp_id == epg_info.service[0].tp_id && service_id < epg_info.service[0].service_id))
		goto EXIT;
	if (tp_id > epg_info.service[epg_info.service_num-1].tp_id
		|| (tp_id == epg_info.service[epg_info.service_num-1].tp_id && service_id > epg_info.service[epg_info.service_num-1].service_id))
		goto EXIT;
	if (epg_info.service_num == 1)
	{
		if (tp_id == epg_info.service[0].tp_id
			&& service_id == epg_info.service[0].service_id)
		{
			*idx = 0;
			return &epg_info.service[0];
		}
		else
			goto EXIT;
	}

	//2分法快速搜索
	m = 0;
	n = epg_info.service_num - 1;
	if (tp_id == epg_info.service[m].tp_id
		&& service_id == epg_info.service[m].service_id)
	{
		*idx = m;
		return &epg_info.service[m];
	}
	if (tp_id == epg_info.service[n].tp_id
		&& service_id == epg_info.service[n].service_id)
	{
		*idx = n;
		return &epg_info.service[n];
	}
	do
	{
		mid = (m + n) / 2;

		if (mid == m || mid == n)
			break;

		if (tp_id == epg_info.service[mid].tp_id
			&& service_id == epg_info.service[mid].service_id)
		{
			*idx = mid;
			return &epg_info.service[mid];
		}
		else if (tp_id > epg_info.service[mid].tp_id
				|| (tp_id == epg_info.service[mid].tp_id && service_id > epg_info.service[mid].service_id))
			m = mid;
		else
			n = mid;

	} while (1);

EXIT:
	MSG_PRINT("get_service: not find tp 0x%X, service 0x%X\n",tp_id,service_id);
	return NULL;
}

//sort service list by tp_id & service_id
static INT32 insert_service(struct SERVICE_T *service, struct SERVICE_T *service_list, UINT16 cnt)
{
	int i;

	if (service == NULL || service_list == NULL)
		return ERR_FAILURE;

	if (cnt == 0)
	{
		MEMCPY(&service_list[0], service, sizeof(struct SERVICE_T));
		EPG_PRINT("%s: [0x%X-0x%X] to service(%d)\n",__FUNCTION__,service->tp_id,service->service_id,cnt);
		return SUCCESS;
	}

	// > last one
	if (service->tp_id > service_list[cnt-1].tp_id
		|| (service->tp_id == service_list[cnt-1].tp_id
			&& service->service_id > service_list[cnt-1].service_id))
	{
		MEMCPY(&service_list[cnt], service, sizeof(struct SERVICE_T));
		EPG_PRINT("%s: [0x%X-0x%X] to service(%d)\n",__FUNCTION__,service->tp_id,service->service_id,cnt);
		return SUCCESS;
	}

	for (i=0; i<cnt; i++)
	{
		if (service->tp_id < service_list[i].tp_id
			|| (service->tp_id == service_list[i].tp_id
				&& service->service_id < service_list[i].service_id))
		{
			//insert
			//move [i] -> [i+1]
			MEMMOVE(&service_list[i+1], &service_list[i], (cnt-i)*sizeof(struct SERVICE_T));
			MEMCPY(&service_list[i], service, sizeof(struct SERVICE_T));
			EPG_PRINT("%s: [0x%X-0x%X] to service(%d)\n",__FUNCTION__,service->tp_id,service->service_id,i);
			return SUCCESS;
		}
	}

	ERR_PRINT("%s: can not insert service [0x%X-0x%X]\n",__FUNCTION__,service->tp_id,service->service_id);
#ifdef _DEBUG
	ASSERT(0);
#endif
	return ERR_FAILURE;
}

//check if the section already exists
static section_status find_section(UINT32 tp_id, UINT16 service_id, UINT8 table_id, UINT8 ver, UINT8 sec_num, UINT16 *idx, UINT32 crc)
{
	UINT8 table_num;	//table number(0-15)
	UINT8 seg;			//segment number(8 section is 1 segment)
	UINT8 seg_sec_num;	//section number within segment(0-7)
	struct SERVICE_T *service;

	service = get_service(tp_id, service_id, idx);

	if (service == NULL)
	{
		return ERROR_NO_SERVICE;
	}

	if (IS_PF(table_id))
	{
#ifdef _DEBUG
		if (sec_num > 1)
		{
			EPG_PRINT("[%s] sec_num: %d overflow!\n",__FUNCTION__,sec_num);
			return ERROR_PF_SECTION_NUM;
		}
#endif

#ifdef EPG_UPDATE_BY_CRC
		if (sec_num == 0 && crc != service->p_crc)
		{
			if (service->p_crc != RESERVED_CRC)
			{
				EPG_PRINT("service: 0x%X, pf: CRC 0x%X update to 0x%X\n",service_id,service->p_crc,crc);
				service->p_crc = crc;
				service->pf_sec_mask = 0;
				service->pf_sec_mask |= (1<<sec_num);
				return PF_VERSION_UPDATE;
			}

			service->p_crc = crc;
		}
#else
		if (ver != service->pf_ver)
		{
			if (service->pf_ver != RESERVED_VER)
			{
				EPG_PRINT("service: 0x%X, pf: version %d update to %d\n",service_id,service->pf_ver,ver);
				service->pf_ver = ver;
				service->pf_sec_mask = 0;
				service->pf_sec_mask |= (1<<sec_num);
				return PF_VERSION_UPDATE;
			}

			service->pf_ver = ver;
		}
#endif
		if (((1<<sec_num) & service->pf_sec_mask) == 0)
		{
			service->pf_sec_mask |= (1<<sec_num);
			MSG_PRINT("service: 0x%X, pf: section %d\n",service_id,sec_num);
			return PF_SECTION_UPDATE;
		}
	}
	else	//sch
	{
		table_num = table_id & 0x0F;

		if (table_num < SCHEDULE_TABLE_ID_NUM)
		{
#ifdef EPG_UPDATE_BY_CRC
			if (sec_num == 0 && crc != service->sch_crc[table_num])
			{
				if (service->sch_crc[table_num] != RESERVED_CRC)
				{
					EPG_PRINT("service: 0x%X, sch[%X]: crc 0x%X update to 0x%X\n",service_id,table_id,service->sch_crc[table_num],crc);
					service->sch_crc[table_num] = crc;
					MEMSET(service->sch_sec_mask[table_num], 0, 32);
					seg = sec_num >> 3;
					seg_sec_num = sec_num & 0x07;
					service->sch_sec_mask[table_num][seg] |= (1<<seg_sec_num);
					return SCH_VERSION_UPDATE;
				}
				
				service->sch_crc[table_num] = crc;
			}
#else
#ifdef EPG_TEST_CASE_SEC_VER_UPDATE
			static int sec_num = 0;
			sec_num ++;
			if ((sec_num % 20) == 0)
#else
			if (ver != service->sch_ver[table_num])
#endif
			{
				if (service->sch_ver[table_num] != RESERVED_VER)
				{
					EPG_PRINT("service: 0x%X, sch[%X]: version %d update to %d\n",service_id,table_id,service->sch_ver[table_num],ver);
					service->sch_ver[table_num] = ver;
					MEMSET(service->sch_sec_mask[table_num], 0, 32);
					seg = sec_num >> 3;
					seg_sec_num = sec_num & 0x07;
					service->sch_sec_mask[table_num][seg] |= (1<<seg_sec_num);
					return SCH_VERSION_UPDATE;	
				}
				
				service->sch_ver[table_num] = ver;
			}
#endif
			seg = sec_num >> 3;
			seg_sec_num = sec_num & 0x07;
			if (((1<<seg_sec_num) & service->sch_sec_mask[table_num][seg]) == 0)
			{
				service->sch_sec_mask[table_num][seg] |= (1<<seg_sec_num);
				//libc_printf("e[%d] t=%d, s=%d\n", *idx, table_num,sec_num);
				MSG_PRINT("service: 0x%X, table: 0x%X, sch: section %d\n",service_id, table_id, sec_num);
				return SCH_SECTION_UPDATE;
			}
		}
		else
		{
			ERR_PRINT("[%s] table_id 0x%X overflow!\n",__FUNCTION__,table_id);
		}
	}

	return SECTION_EXIST;
}

//copy hitmap from handle to event
static void restore_hitmap(struct SERVICE_T *service, UINT32 num)
{
	UINT32 i;

	for(i=0; i<num; i++)
	{
		service[i].pf_sec_mask = service[i].pf_sec_mask_h;
#ifdef EPG_UPDATE_BY_CRC
		service[i].p_crc = service[i].p_crc_h;
#else
		service[i].pf_ver = service[i].pf_ver_h;
#endif
		MEMCPY(service[i].sch_sec_mask, service[i].sch_sec_mask_h, 32 * SCHEDULE_TABLE_ID_NUM);
#ifdef EPG_UPDATE_BY_CRC
		MEMCPY(service[i].sch_crc, service[i].sch_crc_h, sizeof(UINT32)*SCHEDULE_TABLE_ID_NUM);
#else
		MEMCPY(service[i].sch_ver, service[i].sch_ver_h, SCHEDULE_TABLE_ID_NUM);
#endif
	}
}

//set handle hitmap in SIE task.
static void set_hitmap(UINT8 table_id, UINT8 ver, UINT8 sec_num, UINT16 service_idx, UINT32 crc)
{
	UINT8 table_num;	//table number(0-15)
	UINT8 seg;			//segment number(8 section is 1 segment)
	UINT8 seg_sec_num;	//section number within segment(0-7)
	struct SERVICE_T *service;

	service = &epg_info.service[service_idx];
	//ASSERT(service_idx<epg_info.service_num);

	if (IS_PF(table_id))
	{
#ifdef EPG_UPDATE_BY_CRC
		if (sec_num == 0 && crc != service->p_crc_h)
		{
			if (service->p_crc_h != RESERVED_CRC)
			{
				service->pf_sec_mask_h = 0;
			}
			service->p_crc_h = crc;
		}
#else
		if (ver != service->pf_ver_h)
		{
			if (service->pf_ver_h != RESERVED_VER)
			{
				service->pf_sec_mask_h = 0;
			}
			service->pf_ver_h = ver;
		}
#endif
		service->pf_sec_mask_h |= (1<<sec_num);
	}
	else	//sch
	{
		table_num = table_id & 0x0F;
		//ASSERT(table_num<SCHEDULE_TABLE_ID_NUM);

		if (table_num < SCHEDULE_TABLE_ID_NUM)
		{
#ifdef EPG_UPDATE_BY_CRC
			if (sec_num == 0 && crc != service->sch_crc_h[table_num])
			{
				if (service->sch_crc_h[table_num] != RESERVED_CRC)
				{
					MEMSET(service->sch_sec_mask_h[table_num], 0, 32);
				}
				service->sch_crc_h[table_num] = crc;
			}
#else
			if (ver != service->sch_ver_h[table_num])
			{
				if (service->sch_ver_h[table_num] != RESERVED_VER)
				{
					MEMSET(service->sch_sec_mask_h[table_num], 0, 32);
				}
				service->sch_ver_h[table_num] = ver;
			}
#endif
			seg = sec_num >> 3;
			seg_sec_num = sec_num & 0x07;

			service->sch_sec_mask_h[table_num][seg] |= (1<<seg_sec_num);
			//libc_printf("h[%d] t=%d, s=%d\n", service_idx, table_num,sec_num);
		}
		else
		{
			ERR_PRINT("[%s] table_id 0x%X overflow!\n",__FUNCTION__,table_id);
		}
	}
}

//clear event hitmap
//NOTE: be carefull that this function is running in SIE task, but DMX task will
//		access & set this hitmap too!!!
static void clear_hitmap(UINT8 table_id, UINT8 ver, UINT8 sec_num, UINT16 service_idx)
{
	UINT8 table_num;	//table number(0-15)
	UINT8 seg;			//segment number(8 section is 1 segment)
	UINT8 seg_sec_num;	//section number within segment(0-7)
	struct SERVICE_T *service;

	service = &epg_info.service[service_idx];
	//ASSERT(service_idx<epg_info.service_num);

	if (IS_PF(table_id))
	{
		osal_task_dispatch_off();
		service->pf_sec_mask &= ~(1<<sec_num);	//clear
		osal_task_dispatch_on();
	}
	else	//sch
	{
		table_num = table_id & 0x0F;
		//ASSERT(table_num<SCHEDULE_TABLE_ID_NUM);

		if (table_num < SCHEDULE_TABLE_ID_NUM)
		{
			seg = sec_num >> 3;
			seg_sec_num = sec_num & 0x07;

			osal_task_dispatch_off();
			service->sch_sec_mask[table_num][seg] &= ~(1<<seg_sec_num);
			osal_task_dispatch_on();
			//libc_printf("h[%d] t=%d, s=%d\n", service_idx, table_num,sec_num);
		}
		else
		{
			ERR_PRINT("[%s] table_id 0x%X overflow!\n",__FUNCTION__,table_id);
		}
	}
}

//create service list
static UINT16 create_service_list(UINT32 tp_id, struct SERVICE_T* service, UINT16 last_cnt, UINT16 max_cnt)
{
	UINT16 cnt = last_cnt;
	UINT32 pos = 0;
	P_NODE p_node;
	UINT16 idx;
	struct SERVICE_T tmp_service;
	INT i;

	if (service == NULL)
		return 0;

	//init
	tmp_service.pf_sec_mask = 0;
#ifdef EPG_UPDATE_BY_CRC
	tmp_service.p_crc = RESERVED_CRC;
#else
	tmp_service.pf_ver = RESERVED_VER;
#endif
	tmp_service.pf_sec_mask_h = 0;
#ifdef EPG_UPDATE_BY_CRC
	tmp_service.p_crc_h = RESERVED_CRC;
#else
	tmp_service.pf_ver_h = RESERVED_VER;
#endif
	MEMSET(tmp_service.sch_sec_mask, 0, 32 * SCHEDULE_TABLE_ID_NUM);
#ifdef EPG_UPDATE_BY_CRC
	for (i=0; i<SCHEDULE_TABLE_ID_NUM; i++)
		tmp_service.sch_crc[i] = RESERVED_CRC;
#else
	MEMSET(tmp_service.sch_ver, RESERVED_VER, SCHEDULE_TABLE_ID_NUM);
#endif
	MEMSET(tmp_service.sch_sec_mask_h, 0, 32 * SCHEDULE_TABLE_ID_NUM);
#ifdef EPG_UPDATE_BY_CRC
	for (i=0; i<SCHEDULE_TABLE_ID_NUM; i++)
		tmp_service.sch_crc_h[i] = RESERVED_CRC;
#else
	MEMSET(tmp_service.sch_ver_h, RESERVED_VER, SCHEDULE_TABLE_ID_NUM);
#endif

	//find all
	while (cnt < max_cnt && SUCCESS == get_prog_at(pos, &p_node))
	{
#ifndef EPG_MULTI_TP
		//add all services for EPG_MULTI_TP
		//add services of this tp for undef EPG_MULTI_TP
		if (p_node.tp_id == tp_id)	//add all services
#endif
		{
			if (last_cnt == 0
				|| NULL == get_service(p_node.tp_id, p_node.prog_number, &idx))	//not exist
			{
				tmp_service.tp_id = p_node.tp_id;
				tmp_service.service_id = p_node.prog_number;

				insert_service(&tmp_service, service, cnt);

				cnt ++;
			}
		}
		pos ++;
	}

	if (cnt != last_cnt)
	{
		EPG_PRINT("\ncreate_service_list: total %d service\n",cnt);
		for (i=0;i<cnt;i++)
		{
			EPG_PRINT("		service(%d) - [0x%X-0x%X]\n",i,service[i].tp_id,service[i].service_id);
		}
	}
	return cnt;
}

#ifdef EPG_OTH_SCH_SUPPORT
//create tp list table of sat_id, except current tp
static void create_tp_list(UINT16 sat_id, UINT32 tp_id)
{
	//static UINT16 pre_sat_id = 0;
	INT32 i, j, k, n = 0;
	UINT16 tp_num;
	T_NODE t_node;
	UINT32 id1, id2;
	
	if (sat_id == epg_info.pre_sat_id && tp_id == epg_info.tp_id)
		return;

	epg_info.pre_sat_id = sat_id;

	tp_num = get_tp_num_sat(sat_id);

	//get tp info from DB
	i = 0;
	while (i < tp_num && n < MAX_TP_COUNT)	//128 for cable
	{
		if ((get_tp_at(sat_id, i, &t_node) == SUCCESS)
			&&(t_node.tp_id != tp_id)) /*except current tp*/
		{
			//(ts_id, o_net_id)
			if (t_node.t_s_id || t_node.network_id)
			{
				epg_info.tp_ids[n][0] = (t_node.t_s_id<<16)|t_node.network_id;
				epg_info.tp_ids[n][1] = t_node.tp_id;
				n ++;
			}
			else
			{
				//fake tp
			}
		}
		i ++;
	}
	epg_info.tp_id_num = n;
	EPG_PRINT("\ncreate_tp_list: sat = 0x%X, tp != 0x%X, tp num = %d\n",sat_id,tp_id,n);

	//sort
	for (i=0; i<n-1; i++)
	{
		k = i;
		for (j=i+1; j<n; j++)
		{
			if (epg_info.tp_ids[j][0] < epg_info.tp_ids[k][0])
				k = j;
		}
		if (k != i)
		{
			id1 = epg_info.tp_ids[i][0];
			id2 = epg_info.tp_ids[i][1];
			epg_info.tp_ids[i][0] = epg_info.tp_ids[k][0];
			epg_info.tp_ids[i][1] = epg_info.tp_ids[k][1];
			epg_info.tp_ids[k][0] = id1;
			epg_info.tp_ids[k][1] = id2;
		}
	}

	for (i=0; i<n; i++)
	{
		EPG_PRINT("		[ts|net]: 0x%08X, [tp]: 0x%08X\n",epg_info.tp_ids[i][0],epg_info.tp_ids[i][1]);
	}
}

//get tp_id by (ts_id, o_net_id)
static UINT32 get_tp_id(UINT16 ts_id, UINT16 o_net_id)
{
	int m, n, mid;
	UINT32 id = (ts_id<<16)|o_net_id;

//test all event
//	return ((ts_id<<16)|o_net_id);

	if (epg_info.tp_id_num == 0)
		goto EXIT;
	if (id < epg_info.tp_ids[0][0])
		goto EXIT;
	if (id > epg_info.tp_ids[epg_info.tp_id_num-1][0])
		goto EXIT;

	//2分法快速搜索
	m = 0;
	n = epg_info.tp_id_num - 1;
	if (id == epg_info.tp_ids[m][0])
	{
		MSG_PRINT("get_tp_id: [ts 0x%X, net 0x%X] -> tp 0x%X\n",ts_id,o_net_id,epg_info.tp_ids[m][1]);
		return epg_info.tp_ids[m][1];
	}
	else if (id == epg_info.tp_ids[n][0])
	{
		MSG_PRINT("get_tp_id: [ts 0x%X, net 0x%X] -> tp 0x%X\n",ts_id,o_net_id,epg_info.tp_ids[n][1]);
		return epg_info.tp_ids[n][1];
	}

	do
	{
		mid = (m + n) / 2;

		if (mid == m || mid == n)
			break;

		if (id == epg_info.tp_ids[mid][0])
		{
			MSG_PRINT("get_tp_id: [ts 0x%X, net 0x%X] -> tp 0x%X\n",ts_id,o_net_id,epg_info.tp_ids[mid][1]);
			return epg_info.tp_ids[mid][1];
		}
		else if (id > epg_info.tp_ids[mid][0])
			m = mid;
		else
			n = mid;

	} while (1);

EXIT:
	MSG_PRINT("get_tp_id: not find [ts 0x%X, net 0x%X]\n",ts_id,o_net_id);
	return INVALID_TP_ID;
}
#endif

//on section hit event. only check if the section exists.
static BOOL eit_event(UINT16 pid, struct si_filter_t *filter,
				UINT8 reason, UINT8 *buf, INT32 length)
{
	UINT32 tp_id;
	UINT8  table_id;
	UINT16 sec_len;
	UINT16 service_id;
	UINT8  ver;
	UINT8  sec_num;
	section_status ret;
	UINT16 idx;
	UINT32 crc;
#ifdef EPG_OTH_SCH_SUPPORT
	UINT16 ts_id;
	UINT16 o_net_id;
#endif
    static int n = 0;

#ifdef _DEBUG
	static UINT16 pre_service_id = 0xFFFF;
	static UINT8  pre_table_id;
	static UINT8  pre_sec_num;
	UINT32 tick = osal_get_tick();
#endif
	if (buf == NULL || length < 18 || length > 4096)
	{
		EPG_PRINT("%s: buffer is NULL\n",__FUNCTION__);
		return FALSE;
	}

	table_id = buf[0];
#ifdef _DEBUG
	if (!IS_PF(table_id) && !IS_SCH(table_id))
	{
		EPG_PRINT("%s: invalid table id 0x%X\n",__FUNCTION__,table_id);
		ASSERT(0);
	}
	else if (IS_SCH(table_id) && (table_id&0x0F)>=SCHEDULE_TABLE_ID_NUM)
	{
		EPG_PRINT("%s: table id 0x%X overflow!\n",__FUNCTION__,table_id);
		ASSERT(0);
	}
#endif

	sec_len = ((buf[1]&0x0F)<<8) | buf[2];
	if (sec_len + 3 > length || sec_len < 15 || sec_len > 4093)
	{
		EPG_PRINT("%s: length %d < section length %d\n",__FUNCTION__,length, sec_len + 3);
		return FALSE;
	}
	service_id = (buf[3]<<8) | buf[4];
   	ver = (buf[5]>>1)&0x1f;
	sec_num = buf[6];
	crc = (buf[3+sec_len-4]<<24) | (buf[3+sec_len-3]<<16) | (buf[3+sec_len-2]<<8) | buf[3+sec_len-1];

/*	if (IS_PF(table_id) && sec_num == 0 && service_id == 0x66)
	{
		libc_printf("0x66: ver %d, crc 0x%X\n",ver,crc);
	}*/

#ifdef EPG_OTH_SCH_SUPPORT
	if (IS_OTH_SCH(table_id)||IS_OTH_PF(table_id))
	{
		//(ts_id, o_net_id) -> tp_id
		ts_id = (buf[8]<<8)|buf[9];
		o_net_id = (buf[10]<<8)|buf[11];
		tp_id = get_tp_id(ts_id, o_net_id);
		if (tp_id == INVALID_TP_ID)
		{
			return FALSE;
		}

		//tp_id -> (ts_id, o_net_id)
		buf[8] = (tp_id>>24)&0xFF;
		buf[9] = (tp_id>>16)&0xFF;
		buf[10] = (tp_id>>8)&0xFF;
		buf[11] = tp_id&0xFF;
	}
	else
#endif
	{
		tp_id = epg_info.tp_id;
	}

	//if this section not exists, let handle parse it.
	ret = find_section(tp_id, service_id, table_id, ver, sec_num, &idx, crc);

	//last_sec_num -> service idx
	buf[7] = idx&0xFF;		//low byte
	buf[12] = idx>>8;	//[segment_last_section_number] high byte	//!!!

	if (ret == PF_VERSION_UPDATE || ret == SCH_VERSION_UPDATE)
	{
		//use byte 1/bit4,5(reserved in si spec) to tell handle the section status.
		//if the buf is protected by sie, then no need mutex to protect it.
		buf[1] = (buf[1] & VERSION_MASK) | ret;

		if ((n++) % 50 == 0)
			EPG_PRINT("hit %d, %X, %X, %d (@ tick %d)\n",n,buf[0],service_id,sec_num,osal_get_tick());

		return TRUE;
	}
	else if (ret == PF_SECTION_UPDATE || ret == SCH_SECTION_UPDATE)
	{
		buf[1] = (buf[1] & VERSION_MASK) | ret;

		if ((n++) % 50 == 0)
			EPG_PRINT("hit %d, %X, %X, %d (@ tick %d)\n",n,buf[0],service_id,sec_num,osal_get_tick());

		return TRUE;
	}
	else if (ret == ERROR_NO_SERVICE)
	{
		MSG_PRINT("!@#$%^&___ %s: epg not find service!!! ___&*()*(_\n",__FUNCTION__);
		return FALSE;
	}
	else
		return FALSE;
}

//parse it
static sie_status_t eit_handle(UINT16 pid, struct si_filter_t *filter,
						UINT8 reason, UINT8 *buf, INT32 length)
{
	UINT32 tp_id;
	UINT16 service_id;
	UINT8  table_id;
	UINT16 sec_len;
	UINT8  ver;
	UINT8  sec_num;
	UINT8  sec_mask;
	UINT32 crc;
	INT32  ret;

	if (buf == NULL || reason == SIE_REASON_FILTER_TIMEOUT)
		return sie_started;

#ifdef EPG_OTH_SCH_SUPPORT
	if (IS_OTH_SCH(buf[0])||IS_OTH_PF(buf[0]))
	{
		tp_id = (buf[8]<<24)|(buf[9]<<16)|(buf[10]<<8)|buf[11];
	}
	else
#endif
	{
		tp_id = epg_info.tp_id;
	}

	sec_mask = buf[1]&(~VERSION_MASK);
	//pf version update, del pf events first
	if (sec_mask==PF_VERSION_UPDATE)
	{
		service_id = (buf[3]<<8) | buf[4];
		//it maybe SI_EIT_TABLE_ID_APF or SI_EIT_TABLE_ID_OPF got before.
		//del_events(tp_id, service_id, buf[0]&0x0F, PRESENT_EVENT_MASK|FOLLOWING_EVENT_MASK);
		del_events(tp_id, service_id, SI_EIT_TABLE_ID_APF&0x0F, PRESENT_EVENT_MASK|FOLLOWING_EVENT_MASK);
#ifdef EPG_OTH_SCH_SUPPORT
		del_events(tp_id, service_id, SI_EIT_TABLE_ID_OPF&0x0F, PRESENT_EVENT_MASK|FOLLOWING_EVENT_MASK);
#endif
	}
	else if (sec_mask==SCH_VERSION_UPDATE)	//sch version update, del sch events first
	{
		service_id = (buf[3]<<8) | buf[4];
		del_events(tp_id, service_id, buf[0]&0x0F, SCHEDULE_EVENT_MASK);
	}

#ifdef _DEBUG
	UINT32 tick = osal_get_tick();
#endif

	if (get_epg_db_status() == STATUS_UNAVAILABLE)
	{
		ERR_PRINT("!@#$%^&___ %s: epg db is full & unavailable!!! ___&*()*(_\n",__FUNCTION__);

		//drop sch other
		if (IS_OTH_SCH(buf[0])||IS_OTH_PF(buf[0]))
		{
			EPG_PRINT("drop eit other!\n");
			return sie_started;
		}

		EPG_PRINT("\nepg del events tick 1: %u\n",osal_get_tick());
		if (0 == epg_del_events(epg_info.tp_id, NOT_EQUAL))	//del other tp's events
		{
		    if (0 == epg_del_events(epg_info.tp_id, EQUAL)) //del current tp's events
    		{
    		    //db is full, no need do any parsing!
#ifdef _DEBUG
    			ASSERT(0);
#endif
				return sie_started;
	        }
		}
		EPG_PRINT("epg del events tick 2: %u\n",osal_get_tick());
	}

	ret = eit_sec_parser(tp_id, buf, length, epg_info.epg_call_back);

	table_id = buf[0];
	ver = (buf[5]>>1)&0x1f;
	sec_num = buf[6];
	sec_len = ((buf[1]&0x0F)<<8) | buf[2];
	crc = (buf[3+sec_len-4]<<24) | (buf[3+sec_len-3]<<16) | (buf[3+sec_len-2]<<8) | buf[3+sec_len-1];

	if (ret == ERR_DB_FULL)	//epg db full
	{
		ERR_PRINT("!@#$%^&___ %s: epg db is full!!! ___&*()*(_\n",__FUNCTION__);

		/* to del other events next section, not this time! */
		//epg_del_service_events(epg_info.tp_id, NOT_EQUAL);	//del other tp's events

		//clear event hitmap, but some event maybe repeat times!
		clear_hitmap(table_id, ver, sec_num, (buf[12]<<8)|buf[7]);
	}
	else
	{
		if (ret == SUCCESS)
		//if (1)
		{
		}
		else if (ret == ERR_NO_MEM)
		{
			//TO DO...
			ERR_PRINT("!@#$%^&___ %s: epg no mem!!! ___&*()*(_\n",__FUNCTION__);
		}
		else
		{
			//ERR_PRINT("!@#$%^&___ %s: eit parse return error (%d)!!! ___&*()*(_\n",__FUNCTION__,ret);

			/* do not clear event hitmap, for it's bad or empty section. */
			//clear_hitmap(table_id, ver, sec_num, (buf[12]<<8)|buf[7]);
		}

		//set hitmap of handle itself
		set_hitmap(table_id, ver, sec_num, (buf[12]<<8)|buf[7], crc);
	}

#ifdef _DEBUG
	libc_printf("%d/",osal_get_tick()-tick);
#endif

	return sie_started;
}

/*
 * start to get epg data
 */
void epg_on_ext(void *dmx, UINT16 sat_id, UINT32 tp_id, UINT16 service_id)
{
	T_NODE t_node;

	EPG_PRINT("%s:\n",__FUNCTION__);
	if (epg_status != STATUS_OFF)
	{
		EPG_PRINT("_epg_on - status: %d error!\n",epg_status);
		return;
	}

	if (sat_id == 0)
	{
		get_tp_by_id(tp_id, &t_node);
		EPG_PRINT("sat_id: 0x%X\n",t_node.sat_id);
		sat_id = t_node.sat_id;
	}

	if (epg_info.buffer == NULL)
		return;

	epg_enter_mutex();

#ifdef EPG_OTH_SCH_SUPPORT
	create_tp_list(sat_id, tp_id);
#endif

#ifndef EPG_MULTI_TP
	//a tp a unit. when change tp, do reset all
	if (tp_id != epg_info.tp_id)
		reset_epg_internal();
#endif

	//add new services into the struct list of this TP.
	epg_info.service_num = create_service_list(tp_id, epg_info.service, epg_info.service_num, MAX_TP_SERVICE_COUNT);

	epg_info.tp_id = tp_id;
	epg_info.service_id = service_id;

	epg_info.config.cur_service_id = service_id;

	//copy handle service to event
	restore_hitmap(epg_info.service, epg_info.service_num);

	epg_status = STATUS_ON;
	//epg mutex shall NOT nest sie api(called by start_eit)!
	epg_leave_mutex();

	if (dmx)
		start_eit_ext((struct dmx_device*)dmx, &epg_info.config, eit_event, eit_handle);
	else
		start_eit(&epg_info.config, eit_event, eit_handle);
}

void epg_on(UINT16 sat_id, UINT32 tp_id, UINT16 service_id)
{
	EPG_PRINT("%s:\n",__FUNCTION__);
	epg_on_ext(NULL, sat_id, tp_id, service_id);
}

/*
 * stop getting epg data
 */
void epg_off()
{
	EPG_PRINT("%s:\n",__FUNCTION__);
	if (epg_status != STATUS_ON)
	{
		EPG_PRINT("epg_off - status: %d error!\n",epg_status);
		return;
	}

	epg_enter_mutex();
	epg_status = STATUS_OFF;
	epg_leave_mutex();

	stop_eit();
}

void epg_off_ext()
{
	EPG_PRINT("%s:\n",__FUNCTION__);
	if (epg_status != STATUS_ON)
	{
		EPG_PRINT("epg_off_ext - status: %d error!\n",epg_status);
		return;
	}

	epg_enter_mutex();
	epg_status = STATUS_OFF;
	epg_leave_mutex();

	stop_eit_ext();
}

/*
 * init the epg module
 * mode: IN, 
 *		SIE_EIT_SINGLE_SERVICE: 345k buffer at least
 *		SIE_EIT_WHOLE_TP: 585k buffer at least
 * buf: IN, includes sie filter buffer + tp service table + event db + alloc buf
 * call_back: IN, call back when parse one pf/sch section
 */
INT32 epg_init(UINT8 mode, UINT8* buf, UINT32 len,
				EIT_CALL_BACK call_back
				)
{
	UINT32 service_buf_len;
	UINT32 db_buf_len;
	UINT32 x_mem_len;

	if (epg_status != STATUS_NOT_READY)
	{
		EPG_PRINT("epg_init - status: %d error!\n",epg_status);
		return !SUCCESS;
	}
	
	MEMSET(&epg_info, 0, sizeof(epg_info));
	if(lib_epg_flag == OSAL_INVALID_ID)
	{
		lib_epg_flag = osal_flag_create(EPG_FLAG_MUTEX);
		if (lib_epg_flag == OSAL_INVALID_ID)
		{
			EPG_PRINT("epg flag create failure!\n");
			return !SUCCESS;
		}
	}

	if (buf != NULL)
		epg_info.internal_buf = FALSE;
	else
	{
		epg_info.internal_buf = TRUE;
		buf = MALLOC(len);
	}

	if (buf == NULL/* || len < EIT_FILTER_BUFFER_LEN*/)
	{
		EPG_PRINT("epg_init - mem not enough!!!\n");
		return !SUCCESS;
	}
	
	epg_info.epg_call_back = call_back;

	epg_info.buffer = buf;
	epg_info.buf_len = len;

	//SIE_EIT_SINGLE_SERVICE or SIE_EIT_WHOLE_TP
	epg_info.config.eit_mode = mode;
	EPG_PRINT("epg_init -- buffer: 0x%X, len: %d, mode: %d, max tp num: %d, max service num: %d\n",
				buf, len, mode, MAX_TP_COUNT, MAX_TP_SERVICE_COUNT);

	//si filter buffer, 32k or 128k
	epg_info.config.buffer = buf;
	if (mode == SIE_EIT_SINGLE_SERVICE)
	{
		epg_info.config.buf_len = 32*1024;	//si filter buffer
		db_buf_len = 48*1024;	//epg db buffer
	}
	else
	{
		epg_info.config.buf_len = 64*1024;	//si filter buffer

#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
		if (len >= 0x700000)
			db_buf_len = 1224*1024;	//jilin: ~50000
		else if (len >= 0x400000)
			db_buf_len = 512*1024;	//~20000
		else if (len >= 0x200000)
			db_buf_len = 256*1024;	//~10000
		else	//<=0x100000
			db_buf_len = 128*1024;	//~5000
#else
#error "error: project type not support for epg!!!"
#endif
	}
	epg_info.service = (struct SERVICE_T*)(buf + epg_info.config.buf_len);
	//service table of tp, 64 * 136 = 8704(9k)
	service_buf_len = MAX_TP_SERVICE_COUNT * sizeof(struct SERVICE_T);
	EPG_PRINT("epg_init - size of service struct: %d, service buffer len: %d\n",sizeof(struct SERVICE_T),service_buf_len);

	x_mem_len = len - epg_info.config.buf_len - service_buf_len - db_buf_len;

	//epg db buffer
	init_epg_db(lib_epg_flag, buf+epg_info.config.buf_len+service_buf_len, db_buf_len);

	//x_alloc buffer, 256k at least
	x_init_mem(lib_epg_flag, buf+epg_info.config.buf_len+service_buf_len+db_buf_len, x_mem_len);

	epg_status = STATUS_OFF;
	return SUCCESS;
}

/*
 * release the epg module
 */
INT32 epg_release()
{
	if (epg_status != STATUS_OFF)
	{
		EPG_PRINT("epg_release - status: %d error!\n",epg_status);
		return !SUCCESS;
	}

#ifdef EPG_CACHE_ENABLE
	//cache pf event
	free_epg_cache();
	if(SUCCESS == init_epg_cache())
		cache_db();
#endif

	if (epg_info.internal_buf
		&& epg_info.buffer != NULL)
		FREE(epg_info.buffer);

	MEMSET(&epg_info, 0, sizeof(epg_info));

	release_epg_db();
	x_release_mem();
	
	epg_status = STATUS_NOT_READY;
	return SUCCESS;
}

//get current playing tp id
UINT32 epg_get_cur_tp_id()
{
	return epg_info.tp_id;
}

/*
//get current playing service id
UINT16 epg_get_cur_service_id()
{
	return epg_info.service_id;
}*/

//set the active services
static UINT8 active_service_cnt = 0;
static struct ACTIVE_SERVICE_INFO active_service[MAX_ACTIVE_SERVICE_CNT];
INT32 epg_set_active_service(struct ACTIVE_SERVICE_INFO *service, UINT8 cnt)
{
	EPG_PRINT("epg_set_active_service: active service count %d\n",cnt);

	epg_enter_mutex();

	if (cnt > MAX_ACTIVE_SERVICE_CNT)
	{
		ERR_PRINT("epg_set_active_service - ACTIVE SERVICE COUNT %d overflow!\n",cnt);
		cnt = MAX_ACTIVE_SERVICE_CNT;
	}

	if (service != NULL)
		MEMCPY(active_service, service, cnt*sizeof(struct ACTIVE_SERVICE_INFO));

	active_service_cnt = cnt;

	epg_leave_mutex();
	return cnt;
}

//check active service
BOOL epg_check_active_service(UINT32 tp_id, UINT16 service_id)
{
	int i;

	epg_enter_mutex();

	for (i=0; i<active_service_cnt; i++)
	{
		if (tp_id == active_service[i].tp_id && service_id == active_service[i].service_id)
		{
			epg_leave_mutex();
			return TRUE;
		}
	}
	epg_leave_mutex();
	return FALSE;
}

