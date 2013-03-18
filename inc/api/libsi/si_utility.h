#ifndef __SI_UTILITY_H__
#define __SI_UTILITY_H__
#include <types.h>
#include <sys_config.h>

#define SI_DEBUG_LEVEL				0

#ifdef __cplusplus
extern "C" {
#endif
/**
 * bcd2int
 *
 * convert bcd code to interger.
 */
UINT32 bcd2integer(UINT8 *ptr, UINT8 digits, INT32 offset);


#if (SI_DEBUG_LEVEL>0)
void DUMP_SECTION(UINT8 *data, INT32 len, INT32 level);
#else
#define DUMP_SECTION(...)			do{}while(0)
#endif
#ifdef __cplusplus
}
#endif

#endif /* __SI_UTILITY_H__ */

