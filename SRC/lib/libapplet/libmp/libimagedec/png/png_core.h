
#ifndef _PNG_CORE_
#define _PNG_CORE_
#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libmp/png.h>
#include "png_in.h"
#include "png_inflate.h"
#include "png_crc32.h"
#include "png_filter.h"
#include "png_info.h"
#include "png_out.h"

//MACRO & ENUM
#define PNG_INLINE						static

#define PNG_MEM_DOWN

#define PNG_MAX_PIC_WIDTH				(500)
#define PNG_MAX_PIC_HEIGHT				(400)

#define PNG_FLAG_DECOMPRESS_SYNC				(0x00000001)

#define PNG_DECOMPRESS_SYNC_TIME_OUT			(50)

#define PNG_INFO_IHDR							(0x00000001)
#define PNG_INFO_IDAT							(0x00000002)
#define PNG_INFO_IEND							(0x00000004)
#define PNG_INFO_PLTE							(0x00000008)
#define PNG_INFO_TRNS							(0x00000010)
#define PNG_INFO_CMF							(0x00010000)

//STRUCTURE
struct png_core
{
	struct png_inflate inf;
	struct png_info info;
	struct png_out out;
	struct png_filter filter;
	struct png_cfg cfg;
	struct png_cfg cfg_bk;
	struct png_in in;
	
	enum PNG_STATUS status;
	UINT32 flag;
	UINT32 error;
	OSAL_ID task_id;
	OSAL_ID flag_id;
};

//ROUTINE

#endif

