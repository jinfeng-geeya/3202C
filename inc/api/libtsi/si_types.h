#ifndef __SI_TYPES_H__
#define __SI_TYPES_H__

#include <types.h>
#include <sys_config.h>

#ifdef AUTO_OTA
#define DATABROADCASTID_ALIOTA 0x0A

enum si_pmt_param{
    MAX_ES_CNT = 15,
    SIPMT_EXT_NORMAL = 0x0,
    SIPMT_EXT_OTA = 0x01,//parse pmt es for OTA specially
};

enum si_on_change_type{
	SI_SDT_RUNNING_STATUS_CHG		= 0x1,
	SI_SDT_MULTILANG_SERVICE_NAME	= 0x2,
	SI_SDT_SERVICE_NAME_CHG			= 0x3,
	SI_OTA_VERSION_UPDATE			= 0x4,
	SI_OTA_SSUPID_CHANGE				= 0x5,
	SI_OTA_SEARCH_OVER				= 0x6,
	SI_OTA_CHECK_STOPSEARCH_SIGNAL	= 0x7,
};

struct ssu_info
{
	UINT32 frequency;//uint: KHz
	UINT32 bandwidth;//unit: MHz
	UINT16 ssu_pid;
};

typedef struct
{
	UINT16 service_id;
	UINT16 service_pmtpid;
	struct ssu_info ssuinfo;
}SERVICEPID_INFO;

enum si_nit_param{
    MAX_TS_CNT = 10,
};
#endif

enum si_retcode {
	SI_SUCCESS				= 0,
	SI_FBREAK				= -1,
	SI_PBREAK				= -2,
	SI_SBREAK				= -3,
	SI_UBREAK				= -4,
	SI_UPAUSE				= -5,
	SI_STOFULL				= -6,
	SI_TRYAGAIN				= -7,
	SI_EINVAL				= -8,
	SI_VERSION				= -9,
	SI_CRCRETRY				= -10,
	SI_PERROR				= -11,
	SI_TIMEOUT				= -12,
	SI_SKIP					= -13,
#ifdef STAR_PROG_SEARCH_NIT_SDT_SUPPORT	
	SI_EXIT_STAR			= -14,
#endif	
};

/* 
 * PSI parameters, this section will define DVB spec. related parameter,
 * so basically it should only be dependent with the DVB and ISO-13818 spec.
 */
enum si_parameter {
	SI_LONG_SECTION_LENGTH			= 4096, /* for EIT */
	SI_SHORT_SECTION_LENGTH			= 1024, /* for PAT, SDT, PMT, NIT, ... */

	SI_PAT_PID				= 0x0000,
	SI_NIT_PID				= 0x0010,
	SI_BAT_PID				= 0x0011,
	SI_SDT_PID				= 0x0011,
	SI_STUFF_PID				= 0x1FFF,

	SI_PAT_TABLE_ID				= 0x00,
	SI_NIT_TABLE_ID				= 0x40,
	SI_SDT_TABLE_ID				= 0x42,
	SI_BAT_TABLE_ID				= 0x4a,
	SI_PMT_TABLE_ID				= 0x02,

	SI_SECTION_CRC_LENGTH			= 4,

	SI_SERVICE_NAME_SIZE			= 256,
	SI_MAX_SECTION_NUMBER			= 256,
};

enum {
	SI_ATTR_TABLE				= 0x01,
	SI_ATTR_SPECIFIC_ID			= 0x02,
};

enum si_module_parameter {
	/* retrieving parameter */
	SI_MODULE_SECTION_TIMEOUT_PAT		= 1800,
	SI_MODULE_SECTION_TIMEOUT_SDT		= 5500,
	SI_MODULE_SECTION_TIMEOUT_NIT		= 4500,
	SI_MODULE_SECTION_TIMEOUT_PMT		= 2000,

	/* parsing parameter */
	SI_MODULE_SECTION_LOOP_MAX_DESC		= 16,
};

#endif /* __SI_TYPES_H__ */



