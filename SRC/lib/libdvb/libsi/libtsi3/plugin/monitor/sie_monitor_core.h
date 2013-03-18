#ifndef __SIE_MONITOR_CORE_H
#define __SIE_MONITOR_CORE_H


#include <types.h>
#include <hld/dmx/dmx_dev.h>
#include <api/libtsi/p_search.h>
#include <api/libsi/si_module.h>
#include <api/libsi/si_nit.h>

#define MAX_SIM_COUNT 64	//support max count of monitor object

struct PAT_PROGRAM {		//store PAT table data
	UINT16 pm_number;
	UINT16 pm_pid;
//	UINT16 pm_retry;
//	UINT16 pm_stat;
//	UINT16 pm_nvod;
};

struct PAT_TABLE_INFO {
	UINT16 ts_id;
	UINT16 nit_pid;	
	INT16 max_map_nr;
	INT16 map_counter;	
	struct PAT_PROGRAM map[PSI_MODULE_MAX_PROGRAM_NUMBER];
	//struct extension_info ext[1];
};

#define SIM_EMM_MAX_COUNT 32	//support max count of emm pid
struct CAT_TABLE_INFO
{
	UINT8 emm_count;
	UINT16 ca_sysid_array[SIM_EMM_MAX_COUNT];
	UINT16 emm_pid_array[SIM_EMM_MAX_COUNT];
};

struct mult_service_name{
UINT32 ISO_639_language_code:24;
UINT8 service_provider_name_length;
UINT8 *service_provider_name;
UINT8 service_name_length;
UINT8 *service_name;
};

struct program_data{
	UINT16 program_number;
	
	UINT8 service_provider_length;
	UINT8 *service_provider;
	UINT8 service_name_length;
	UINT8 *service_name;
	
	UINT8 service_type;
	UINT8 timeshift_flag;
	UINT16 ref_service_id;
	
	
	UINT8 mult_name_count;
	struct mult_service_name ml_name[10];
	
	INT32 name_pos;
	UINT8 names[512];
};

struct SDT_TABLE_INFO
{
	UINT16 sid;
	UINT16 t_s_id;
	UINT16 onid;
	UINT8 sdt_version;

	UINT8 sd_count;
	
	struct program_data sd[PSI_MODULE_MAX_PROGRAM_NUMBER];
};

#define DMX_COUNT 3		//support max count of dmx

#define TABLE_TYPE_COUNT 6	//support table type count


#define PMT_MAX_COUNT 16	//one dmx support max count of pmt monitor

struct DMX_PSI_INFO
{
	struct dmx_device *dmx;
	UINT32 valid; //pmt: 0-15 pat 16,cat 17,nit 18,sdt 19
	struct prog_info pmt[PMT_MAX_COUNT];
	struct PAT_TABLE_INFO pat;
	struct CAT_TABLE_INFO cat;
//	struct nit_section_info nit;
#ifdef DYNAMIC_SERVICE_SUPPORT
	struct SDT_TABLE_INFO sdt;
#endif
};

typedef sie_status_t (*table_section_parser)(struct DMX_PSI_INFO *info,UINT16 pid, struct si_filter_t *filter,UINT8 reason, UINT8 *buff, INT32 length);

struct PID_TID_PARSER
{
	UINT16 pid;
	UINT8 tid;
	table_section_parser parser;
};

#define SIM_INVALID_MON_ID	0xFFFFFFFF	//invalid si monitor object ID

#endif


