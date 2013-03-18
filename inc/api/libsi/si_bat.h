#ifndef __SI_BAT_H__
#define __SI_BAT_H__
#include <types.h>
#include <sys_config.h>

#include "si_module.h"

#define MAX_BOUQ_NAME_LENGTH	15
//#define BAT_MIN_TIME		1500

struct bat_service_info {
	UINT16 ts_id;
	UINT16 orig_netid;
	UINT16 bouq_id;
	UINT16 sid;
	UINT16 lcn;
	UINT8 service_type;
//	UINT8 vol;
//	UINT8 track;
};

/*struct bat_hitmap
{
	UINT32 all_hit_value;
	UINT32 hit;
	UINT32 parse;
};*/

struct bat_section_info {
	//tmp save each section bouquet id
	UINT16 b_id;
	//tmp save one tp's ts_id, origin_netid
	UINT16 ts_id;
	UINT16 orig_netid;

//	UINT8 buf[8*PSI_SHORT_SECTION_LENGTH];

	INT16 bouq_cnt;
//	struct bat_hitmap hitmap[PSI_MODULE_MAX_BAT_EXT_NUMBER];
	UINT16 bouq_id[PSI_MODULE_MAX_BAT_EXT_NUMBER];
	UINT8 bouq_name[PSI_MODULE_MAX_BAT_EXT_NUMBER][2*(MAX_BOUQ_NAME_LENGTH + 1)];
	
	INT16 service_cnt;
	struct bat_service_info s_info[PSI_MODULE_MAX_BAT_SINFO_NUMBER];

	struct extension_info ext[PSI_MODULE_MAX_BAT_EXT_NUMBER];
};


#ifdef __cplusplus
extern "C"{
#endif

INT32 si_bat_parser(UINT8 *bat, INT32 len, struct section_parameter *param);

INT32 si_bat_get_info(struct bat_section_info *bsi, UINT16 onid, UINT16 tsid, PROG_INFO *pg);
UINT16 si_bat_get_bouq_cnt(struct bat_section_info *bat_info, UINT16 *bouq_id, UINT8 *bouq_name, UINT16 max_name_len);

//this interface need CSTM to porting!!!
INT32 si_porting_get_bouquet_id(UINT16 *b_id, INT32 max_cnt);

#ifdef __cplusplus
}
#endif
#endif /* __SI_BAT_H__ */

