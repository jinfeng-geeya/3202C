#ifndef _GIF_OUT_
#define _GIF_OUT_
#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libmp/gif.h>
#include "gif_lzw.h"
#include "gif_info.h"

enum GIF_OUT_FRMT
{
	GIF_OUT_8BITS,
	GIF_OUT_16BITS,
	GIF_OUT_32BITS
};

struct gif_out
{
	//output parameters
	struct gif_dec_out_par out_par;

	UINT32 valid:1;
	UINT32 sort:1;
	UINT32 frmt:3;
	UINT32 update:1;
	UINT32 yuv2rgb:1;
	UINT32 res:25;
	UINT32 bk_clor;

	struct gif_info_gce prior_gce;
	struct gif_info_imgdes prior_img;
	struct gif_info_lsdes prior_lsdes;

	UINT32 bak_pic;
	UINT32 pitch;
	UINT32 width;
	UINT32 height;
	INT32 size;

	UINT32 sub_handle;
	UINT32 cmd_list;

	void *par1;
	void *par2;
	void *par3;
	void *par4;
};
#endif

