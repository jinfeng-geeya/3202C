#ifndef __SI_MODULE_H__
#define __SI_MODULE_H__
#include <types.h>
#include <sys_config.h>

#include <api/libc/list.h>
#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <api/libtsi/p_search.h>
#include <api/libsi/si_config.h>

/*
 * PSI module parameters, this section will define module specified parameter,
 * and could be modified according to each different system.
 */
#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
enum psi_module_parameter {
	PSI_MODULE_COMPONENT_NUMBER	= 256,
	PSI_MODULE_CONCURRENT_FILTER	= MAX_FILTER_NUMBER,
	PSI_MODULE_MAX_PROGRAM_NUMBER	= 128,
	PSI_MODULE_MAX_TP_NUMBER	= 512,
	PSI_MODULE_NAME_SIZE		= 48000,

	PSI_MODULE_RETRY_CNT		= 1,
	PSI_MODULE_PAT_TIMEOUT		= 4000,//2500,
	PSI_MODULE_SDT_TIMEOUT		= 5000,//10000,
	PSI_MODULE_SDT_OTH_TIMEOUT	= 15000,//10000,
	PSI_MODULE_BAT_TIMEOUT		= 15000,//10000,
	PSI_MODULE_EIT_TIMEOUT		= 30000,
#ifdef _LCN_ENABLE_
#ifndef INDIA_LCN
	PSI_MODULE_NIT_TIMEOUT		= 30000,
#else
	PSI_MODULE_NIT_TIMEOUT		= 10000,
#endif
#else
#ifdef TFN_SCAN	
	PSI_MODULE_NIT_TIMEOUT		= 30000,//6000,
#else
	PSI_MODULE_NIT_TIMEOUT		= 20000,//6000,
#endif
#endif
	PSI_MODULE_PMT_TIMEOUT		= 4000,//2500,
	PSI_MODULE_TIMEOUT_EXTENTION	= 2000,//1000,
	PSI_MODULE_PMT_TIMEOUT_MAX	= 5500,//4500,
	PSI_MODULE_INVALID_LCN		= 0xFFFF,
};
#endif
enum {
	PSI_MODULE_MAX_PAT_EXT_NUMBER	= 1,
	PSI_MODULE_MAX_SDT_EXT_NUMBER	= 1,
	PSI_MODULE_MAX_BAT_EXT_NUMBER	= 16,
	PSI_MODULE_MAX_NIT_EXT_NUMBER	= 1,
	PSI_MODULE_MAX_EIT_EXT_NUMBER	= PSI_MODULE_MAX_PROGRAM_NUMBER,

	PSI_MODULE_MSECTION_NUMBER	= 256,
	PSI_MODULE_EIT_SECTION_NUMBER	= 2,

	PSI_MODULE_MAX_BAT_GROUP_NUMBER	= 128,
	PSI_MODULE_MAX_BAT_SINFO_NUMBER	= 512,
#ifdef SUPPORT_FRANCE_HD
	PSI_MODULE_MAX_NIT_SINFO_NUMBER = 512,
#else
	PSI_MODULE_MAX_NIT_SINFO_NUMBER = 256,
#endif
};

/*
 * PSI section parameters, this section will define DVB spec. related parameter,
 * so basically it should only be dependent with the DVB and ISO-13818 spec.
 */
enum psi_section_parameter {
	PSI_LONG_SECTION_LENGTH		= 4096,
	PSI_SHORT_SECTION_LENGTH	= 1024,

	PSI_PAT_PID			= 0x0000,
	PSI_NIT_PID			= 0x0010,
	PSI_SDT_PID			= 0x0011,
	PSI_BAT_PID			= 0x0011,
	PSI_EIT_PID			= 0x0012,
	PSI_TDT_PID			= 0x0014,
	PSI_SDTT_PID			= 0x0023,

	PSI_STUFF_PID			= 0x1FFF,

	/**
	 * ISO-13818 defined table ids.
	 */
	PSI_PAT_TABLE_ID		= 0x00,
	PSI_PMT_TABLE_ID		= 0x02,

	/**
	 * EN 300 468 defined table ids.
	 */
	PSI_NIT_TABLE_ID		= 0x40,
	PSI_SDT_TABLE_ID		= 0x42,

	PSI_SDT_OTHER_TABLE_ID= 0x46,

	PSI_BAT_TABLE_ID		= 0x4a,

	PSI_EIT_APF_TABLE_ID		= 0x4E,
	PSI_EIT_OPF_TABLE_ID		= 0x4F,
	PSI_EIT_ASCH_TABLE_ID		= 0x50,
	PSI_TDT_TABLE_ID 			= 0x70,
	PSI_TOT_TABLE_ID 			= 0x73,
	PSI_SDTT_TABLE_ID			= 0xC3,
	PSI_SECTION_CRC_LENGTH		= 4,

	PSI_SERVICE_NAME_SIZE		= 256,
};

/*
 * PSI search internal flag bitfield definition.
 */
enum psi_flag_stat {
	PSI_FLAG_STAT_SECTION		= 0x00000001,

	PSI_FLAG_STAT_PAT		= 0x00000002,
	PSI_FLAG_STAT_SDT		= 0x00000004,
	PSI_FLAG_STAT_NIT		= 0x00000008,
	PSI_FLAG_STAT_BAT		= 0x00000010,
	PSI_FLAG_STAT_EIT_APF		= 0x00000020,
	PSI_FLAG_STAT_EIT_ASCH		= 0x00000040,

	PSI_FLAG_STAT_MUTEX		= 0x00000200,
	PSI_FLAG_STAT_ABORT		= 0x00000400,
	PSI_FLAG_STAT_STOFULL		= 0x00000800,
	PSI_FLAG_STAT_PMTALL		= 0x00001000,
	PSI_FLAG_STAT_RELEASE		= 0x00002000,
};

enum program_map_stat {
	PSI_PMS_NEW			= 0,
	PSI_PMS_QUEUED			= 1,
	PSI_PMS_RETURNED		= 2,
};

enum restrict_config {
	PSI_RC_SECTION_0_3,
	PSI_RC_DUPLICATE,
};

enum {
	PSI_NIT_SECTION_INFO		= 0,
	PSI_PAT_SECTION_INFO		= 1,
	PSI_SDT_SECTION_INFO		= 2,
	PSI_BAT_SECTION_INFO		= 3,
	PSI_EIT_SECTION_INFO		= 4,
};

enum {
	PSI_PROGRESS_ALL_READY		= 60,
	PSI_PROGRESS_ALL_LEFT		= 35,
};

/*
 * name		: program_map <struct>
 * description	: represent a program number <-> program mapping relation.
 * field define	:
 * pm_list	: a double-linked list storing program number that has the same PID.
 * pm_number	: the program number value.
 * pm_retry	: the retry counter for this program map.
 * pm_stat	: the status of this program map, see enum program_map_stat.
 * PROG_INFO	: the program information for this PMT section.
 */
struct program_map {
	struct list_head pm_list;
	UINT16 pm_number;
	UINT16 pm_pid;
	UINT16 pm_retry;
	UINT16 pm_stat;
	UINT16 pm_nvod;
	PROG_INFO pm_info; /* TODO: use a smaller structure to store */
};

struct lcn_map {
	UINT16 service_id;
	UINT16 lcn;
};

struct service_data {
	UINT16 program_number;
	UINT8 service_name_length;
	UINT8 service_provider_length;
	UINT8 service_type;
	UINT8 timeshift_flag;
	UINT16 ref_service_id;
	UINT8 *service_name;
	UINT8 *service_provider;
};


struct section_parameter;
typedef INT32(*si_parser_t)(UINT8 *, INT32, struct section_parameter *);

struct extension_info {
	UINT16 id;
	UINT8 version;
	UINT8 last_section_number;
	UINT16 hit;
	UINT8 *flag;
//	UINT16 hit_times;
};

struct section_parameter {
	struct section_info *info;
	/* retrieving parameter */
	UINT16 retry_cnt;
	UINT16 ext_nr;
	struct extension_info *ext;
	si_parser_t parser;
	void *priv;
};

struct section_info {
	UINT16 pid;
	UINT8 table_id;
	UINT8 table_mask;
	UINT16 flag;
	UINT16 buflen;
	UINT32 timeout;
	si_parser_t parser;
};


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)			((INT32)(sizeof(x)/sizeof(x[0])))
#endif

#endif /* __SI_MODULE_H__ */

