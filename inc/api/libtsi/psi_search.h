#ifndef __SI_SEARCH_H__
#define __SI_SEARCH_H__
#include <types.h>
#include <sys_config.h>
#include <api/libtsi/si_types.h>

#include <hld/dmx/dmx.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/nim/nim.h>

#include <api/libtsi/p_search.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*psi_tuning_t)(void *param);
typedef UINT8 (*psi_event_t)(UINT8 code, void *info);

INT32 psi_module_init(
	struct dmx_device *dmx, 
	struct nim_device *nim, 
	void *work_place, 
	INT32 length);

INT32 psi_search_start(
	UINT16 sat_id, 
	UINT32 tp_id,
	UINT32 search_scheme, 
	UINT32 storage_scheme, 
	psi_event_t on_event,
	psi_tuning_t on_tuning,
	void *tuning_param
);

INT32 psi_search_stop();

#ifdef __cplusplus
}
#endif

#endif /* __SI_SEARCH_H__ */

