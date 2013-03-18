#ifndef __EPG_DB_ALLOC_H__
#define  __EPG_DB_ALLOC_H__

#ifdef WIN32
#include "common.h"
typedef UINT32 			OSAL_ID;
#else
#include <types.h>
#include <osal/osal.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

INT32 x_init_mem(OSAL_ID flag, UINT8 *buffer, UINT32 buf_len);
INT32 x_reset_mem();
INT32 x_release_mem();
INT32 x_get_mem_status();

void *x_alloc(UINT32);
INT32 x_free(void *);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif	//__EPG_DB_ALLOC_H__

