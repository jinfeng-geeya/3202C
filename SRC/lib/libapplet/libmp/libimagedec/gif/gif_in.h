
#ifndef _GIF_IN_
#define _GIF_IN_
#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libmp/gif.h>

struct gif_file
{
	gif_file handle;
	int (*fread)(gif_file,UINT8 *, UINT32);
	BOOL (*fseek)(gif_file,INT32, UINT32);
	int (*ftell)(gif_file);
	UINT32 offset;
	UINT32 finish:1;
	UINT32 res:31;
};
struct gif_str
{
	UINT8 *start;
	UINT8 *read;
	UINT32 left;
	UINT32 size;
};

struct gif_bit
{
	UINT32 value;
	INT32 bits;
};

struct gif_sub
{
	UINT8 size;
	UINT8 left;
};

struct gif_in
{
	struct gif_str str;
	struct gif_bit bit;
	struct gif_file file;
	struct gif_sub sub;
	int frame_num;
};

#endif

