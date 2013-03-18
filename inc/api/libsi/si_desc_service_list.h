#ifndef __SI_DESC_SERVICE_LIST_H__
#define __SI_DESC_SERVICE_LIST_H__
#include <types.h>
#include <sys_config.h>

#include "si_module.h"
#include "si_nit.h"

#ifdef __cplusplus
extern "C"{ 
#endif

INT32 nit_on_service_list_desc(UINT8 tag, UINT8 len, UINT8 *data, void *priv);
INT32 nit_on_logical_channel_desc(UINT8 tag, UINT8 len, UINT8 *data, void *priv);

#ifdef __cplusplus
}
#endif

#endif /* __SI_DESC_SERVICE_LIST_H__ */

