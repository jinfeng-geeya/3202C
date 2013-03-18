
#ifndef _PNG_INFO_
#define _PNG_INFO_
#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>

#define PNG_MAX_CHUNK_NUM	10

struct png_chunk
{
	UINT32 len;
	UINT32 type;
	UINT8 *data;
	UINT32 CRC;
};

struct png_ihdr
{
	UINT32 width;
	UINT32 height;
	UINT8 bit_depth;
	UINT8 color_type;
	UINT8 com_mtd;
	UINT8 fit_mtd;
	UINT8 int_mtd;
};

struct png_CMF
{
	UINT8 CM;
	UINT8 FLG;

	UINT32 DICT;
	UINT32 ADLER32;
};

struct png_plte
{
	UINT8 *palette;
	UINT32 num;
};

struct png_info
{
	struct png_chunk chunk[PNG_MAX_CHUNK_NUM];
	struct png_ihdr IHDR;
	struct png_plte PLTE;
	struct png_CMF CMF;
	UINT8 num;
	UINT8 IHDR_idx;
	UINT8 PLTE_idx;
	UINT8 IDAT_idx;
	UINT8 IEND_idx;
};

#endif

