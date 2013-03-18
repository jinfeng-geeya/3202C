
#ifndef _PNG_FILTER_
#define _PNG_FILTER_

#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>

#define PNG_MAX_BPP			(8)

#define PNG_FIL_NULL 			(0)
#define PNG_FIL_SUB				(1)
#define PNG_FIL_UP				(2)
#define PNG_FIL_AVG				(3)
#define PNG_FIL_PAE				(4)

#define PNG_ABS(x)				((x)<0 ? -(x):(x))	

struct png_filter
{
	UINT8 *prior_line;
	UINT32 line_size;

	UINT32 cur_line;
	
	UINT8 *line_in;
	UINT8 *line_out;
	UINT8 pixel_bytes;

	UINT8 method;
	UINT8 type;
};

#endif

