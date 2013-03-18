/*
 * SI_SDT.H
 *
 * SI SDT parser declaration file
 *
 * 1. 2006.07.10	Zhengdao Li	0.01		Initialize.
 */
#ifndef __SI_SDT_H__
#define __SI_SDT_H__
#include <types.h>
#include <sys_config.h>

#include "si_module.h"
#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)	
#include <api/libsi/lib_nvod.h>


#ifdef PSI_SEARCH_SDT_OTH_SUPPORT
#define PSI_SEARCH_SDT_OTH_COUNT	31	//64
#else
#define PSI_SEARCH_SDT_OTH_COUNT	0
#endif


struct sdt_private_data{
	UINT16 logic_chan_number;
	UINT16 reserve1;
	UINT8 volumn_set;
	UINT8 audio_track;
	UINT8 reserve2;
	UINT8 reserve3;
};

#endif

struct sdt_section_info {
	UINT16 sid;
	UINT16 t_s_id;
	UINT16 onid;

	UINT8 service_type;
	UINT8 sdt_version;

	INT32 max_name_len;
	INT32 name_pos;

	INT16 max_sd_nr;
	INT16 sd_nr;

	INT16 max_nd_nr;
	INT16 nd_nr;

	INT16 max_lcn_nr;
	INT16 lcn_nr;
#if(MW_MODULE == MW_AVIT)
    INT16 db_id;
#endif
	struct extension_info ext[PSI_SEARCH_SDT_OTH_COUNT + 1];
	//struct extension_info ext[1];

	UINT8 names[PSI_MODULE_NAME_SIZE];	
	UINT8 service_name[PSI_SERVICE_NAME_SIZE];
	UINT8 service_provider_name[PSI_SERVICE_NAME_SIZE];
	
#ifdef PSI_SEARCH_SDT_OTH_SUPPORT
	struct service_data sd[PSI_MODULE_MAX_PROGRAM_NUMBER*3];	//>300
#else
	struct service_data sd[PSI_MODULE_MAX_PROGRAM_NUMBER];
#endif

#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)	
	struct nvod_sdt_info nvod;

	struct sdt_private_data private[PSI_MODULE_MAX_PROGRAM_NUMBER];
#endif

};


#ifdef __cplusplus
extern "C"{
#endif

INT32 si_sdt_parser(UINT8 *sdt, INT32 len, struct section_parameter *param);

INT32 psi_get_service(struct sdt_section_info *info,
	UINT16 program_number, 	UINT8 *service_name, UINT8 *provider_name);

#ifdef __cplusplus
}
#endif

#endif /* __SI_SDT_H__ */
