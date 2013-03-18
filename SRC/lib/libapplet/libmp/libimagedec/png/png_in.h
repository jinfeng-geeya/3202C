
#ifndef _LIB_PNG_IN_
#define _LIB_PNG_IN_

#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libmp/png.h>

struct png_str
{
	UINT8 *start;
	UINT8 *read;
	UINT32 left;
	UINT32 size;
};

struct png_bit
{
	UINT32 value;
	INT32 bits;
};

struct png_sub
{
	UINT8 size;
	UINT8 left;
};

struct png_in
{
	struct png_str str;
//	struct png_bit bit;
	struct png_file file;
//	struct png_sub sub;
};

#endif

