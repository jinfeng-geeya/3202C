#ifndef __SI_DESC_CAB_DELIVERY_H__
#define __SI_DESC_CAB_DELIVERY_H__

#include <types.h>
#include <sys_config.h>
#include "si_module.h"
#include "si_nit.h"

/**
 * name		: cable_delivery_system_descriptor<struct>
 * description	: Cable delivery system data defined in EN 468 300 6.2.13.1
 * tag		: 0x44
 */
struct cable_delivery_system_descriptor {
	UINT8 descriptor_tag;
	UINT8 descriptor_length;
	UINT8 frequency[4];
	UINT8 reserved_future_use1_hi;
#if (SYS_CPU_ENDIAN == ENDIAN_LITTLE)	
	UINT8 FEC_outer				: 4;
	UINT8 reserved_future_use1_lo		: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 reserved_future_use1_lo		: 4;
	UINT8 FEC_outer				: 4;
#else
#error "please check your SYS_CPU_ENDIAN setting in <sys_config.h>"
#endif	
	UINT8 modulation;
	UINT8 symbol_rate[3];
#if (SYS_CPU_ENDIAN == ENDIAN_LITTLE)	
	UINT8 FEC_inner				: 4;
	UINT8 symbol_rate_lo			: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 symbol_rate_lo			: 4;
	UINT8 FEC_inner				: 4;
#endif	
}__attribute__((packed));



#ifdef __cplusplus
extern "C"
#endif

INT32 si_on_cab_delivery_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv);

#ifdef __cplusplus
}
#endif

#endif /* __SI_DESC_CAB_DELIVERY_H__ */

