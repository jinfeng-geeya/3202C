#ifndef __SI_NIT_H__
#define __SI_NIT_H__
#include <types.h>
#include <sys_config.h>
#include <api/libtsi/p_search.h>

#include "si_module.h"
#include "desc_loop_parser.h"

struct nit_service_info {
	UINT16 onid;
	UINT16 tsid;
	UINT16 sid;
	UINT16 lcn;		//MHEG need LCN in NIT
	
#ifdef BASE_TP_HALF_SCAN_SUPPORT
	UINT8 service_type;
	UINT8 name_len;
	UINT8 *service_name;
#endif
};

struct service_attrib_info {
	UINT16 onid;
	UINT16 tsid;
	UINT16 sid;
	UINT8 nsf;
	UINT8 vsf;
};

struct nit_section_info {
	UINT16 xp_nr;
	UINT16 nvod_nr;
	
	UINT16 onid;
	UINT16 tsid;
		
	INT32 lp1_nr;
	INT32 lp2_nr;
	struct desc_table *loop1;
	struct desc_table *loop2;

	struct extension_info ext[1];
	
	TP_INFO xp[PSI_MODULE_MAX_TP_NUMBER];
	UINT16 nvod_tsid[PSI_MODULE_MAX_TP_NUMBER];

#if(defined(_SERVICE_ATTRIBUTE_ENABLE_) || defined(_LCN_ENABLE_))
	UINT32 priv_spec;
#endif

#if(defined( _MHEG5_ENABLE_) ||defined(_LCN_ENABLE_) || defined( _MHEG5_V20_ENABLE_) || defined(NETWORK_ID_ENABLE))
	UINT16 net_id;	/* network_id */
	UINT16 s_nr;	//count of s_info
	struct nit_service_info s_info[PSI_MODULE_MAX_NIT_SINFO_NUMBER];
#endif

#ifdef BASE_TP_HALF_SCAN_SUPPORT
	UINT32 name_pos;
	UINT8 names[PSI_MODULE_NAME_SIZE];
	UINT16 name_nr;
#endif

#ifdef _SERVICE_ATTRIBUTE_ENABLE_	
	UINT16 attrib_nr;
	struct service_attrib_info attrib_info[PSI_MODULE_MAX_NIT_SINFO_NUMBER];
#endif

};

#ifdef __cplusplus
extern "C"{
#endif

INT32 si_nit_parser(UINT8 *nit, INT32 len, struct section_parameter *param);

//to get LCN from NIT
#if(defined( _MHEG5_ENABLE_) ||defined(_LCN_ENABLE_))
INT32 si_on_lcn_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv);
INT32 si_nit_get_info(struct nit_section_info *nsi, UINT16 onid, UINT16 tsid, 
	PROG_INFO *pg);
#endif

#if(defined(_LCN_ENABLE_) ||defined(_SERVICE_ATTRIBUTE_ENABLE_))
INT32 si_on_priv_spec_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv);
#endif
#ifdef _SERVICE_ATTRIBUTE_ENABLE_
INT32 si_on_attribute_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __SI_NIT_H__ */
