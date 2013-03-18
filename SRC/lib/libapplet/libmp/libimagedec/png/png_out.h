
#ifndef _PNG_OUT_
#define _PNG_OUT_

#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libmp/png.h>

struct png_out
{
	struct png_image *img;
	UINT32 valid_lines;
	void (*write_line)(UINT8 *,UINT8 *,UINT32,UINT32, UINT32);
	UINT8 bits;
	UINT32 plt;
};

#endif

