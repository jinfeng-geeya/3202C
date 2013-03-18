#ifndef __PSI_PAT_H__
#define __PSI_PAT_H__
#include <types.h>
#include <sys_config.h>

#include "si_module.h"

struct pat_section_info {
	INT16 max_map_nr;
	INT16 map_counter;
	UINT16 ts_id;
	UINT16 nit_pid;
	struct extension_info ext[1];
	struct program_map *map;
};

#ifdef __cplusplus
extern "C"{
#endif

INT32 psi_pat_parser(UINT8 *pat, INT32 len, struct section_parameter *param);

#ifdef __cplusplus
}
#endif

#endif /* __PSI_PAT_H__ */
