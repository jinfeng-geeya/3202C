#ifndef _STB_DATA_H_
#define _STB_DATA_H_

#include <sys_config.h>
#include <types.h>
#include <mediatypes.h>
#include <math.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libstbinfo/stb_info.h>
#include <api/libstbinfo/stb_info_data.h>
#include <api/libchunk/chunk.h>
//#include <api/libupg/lib_upg4.h>

#ifdef NAND_FLASH_SUPPORT
#include <bus/nand/ali_nand_dev.h>
#endif

#define STB_NOR_FLASH_SIZE SYS_FLASH_SIZE

#ifdef NAND_FLASH_SUPPORT
#define STB_NAND_FLASH_SIZE (128*1024*1024)
#endif

#define OTA_FREQUENCY			498
#define OTA_SYM_RATE			6875

/* 0 - Normal, write both to LED and buf
1 - write to LED only
2 - write to buf only
 */
typedef enum
{
	PAN_DISP_LED_BUF=0,
	PAN_DISP_LED_ONLY,
	PAN_DISP_BUF_ONLY,
 }PanDispType_t;

typedef struct
{	
///////////////////////////////////////////////////////////
//	BEGIN
//	the first 64 bytes is exchange space between maincode and bootloader
//	please do not modify !!!!!!!
///////////////////////////////////////////////////////////
	STB_INFO_DATA stb_info;
///////////////////////////////////////////////////////////
//	END
///////////////////////////////////////////////////////////

	
}STB_DATA;


extern STB_DATA system_stb_data;

#endif //_STB_DATA_H_

