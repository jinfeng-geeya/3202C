#ifndef __LIB_STB_INFO_H__
#define __LIB_STB_INFO_H__

#include <types.h>
#include <retcode.h>
#include <sys_config.h>

#define SAVE_STB_INFO_FAILED		-1
#define SAVE_STB_INFO_OUTRANGE	-2
#define LOAD_STB_INFO_FAILED		-3
#define ERASE_STB_INFO_FAILED		-4

#define STB_INFO_START_ID		0x12345678
#define STB_INFO_END_ID		0x87654321

INT32 save_stbinfo_data(UINT8 *buff,UINT16 buff_len);
INT32 load_stbinfo_data(UINT8 *buff,UINT16 buff_len);
INT32 erase_stbinfo_sector();
INT32 init_stbinfo_info(UINT32 base_addr, UINT32 len);



#endif /* __LIB_STB_INFO_H__ */

