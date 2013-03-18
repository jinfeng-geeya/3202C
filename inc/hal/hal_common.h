#ifndef _DRIVER_COMMON_H_
#define _DRIVER_COMMON_H_

#include <basic_types.h>

#define SYS_WriteByte(uAddr, bVal) \
							do{*(volatile UINT8 *)(uAddr) = (bVal);}while(0)
#define SYS_WriteWord(uAddr, wVal) \
							do{*(volatile UINT16 *)(uAddr) = (wVal);}while(0)
#define SYS_WriteDWord(uAddr, dwVal) \
							do{*(volatile UINT32 *)(uAddr) = (dwVal);}while(0)
#define SYS_ReadByte(uAddr) \
							({ \
								volatile UINT8 bVal; \
								bVal = (*(volatile UINT8 *)(uAddr)); \
								bVal; \
							})
#define SYS_ReadWord(uAddr) \
							({ \
								volatile UINT16 wVal; \
								wVal = (*(volatile UINT16 *)(uAddr)); \
								wVal; \
							})
							
#define SYS_ReadDWord(uAddr) \
							({ \
								volatile UINT32 dwVal; \
								dwVal = (*(volatile UINT32 *)(uAddr)); \
								dwVal; \
							})


UINT16 SYS_ReadMemW(UINT8 *p);
UINT32 SYS_ReadMemD(UINT8 *p);
UINT16 SYS_WriteMemW(UINT8 *ptr, UINT16 val);
UINT32 SYS_WriteMemD(UINT8 *ptr, UINT32 val);





//VP SYS_MemMallocEx(INT blksz, INT bytealign, BOOL cacheable);
//ER SYS_MemFreeEx(VP blk);



#endif


