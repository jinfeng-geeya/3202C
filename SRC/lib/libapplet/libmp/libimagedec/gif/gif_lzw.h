#ifndef _GIF_LZW_
#define _GIF_LZW_
#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libmp/gif.h>

#define GIF_MAX_BIT_LEN					12
#define GIF_TABLE_LEN						0x1000

#define GIF_MAX_PALLETE_LEN				256
#define GIF_MAX_SUB_BLOCKS_NUM			20
#define GIF_MAX_SUB_BLOCK_SIZE			256

//lzw control flag
#define GIF_LZW_FIRST_STREAM				0x00000001

enum GIF_LZW_STEP
{
	lZW_NEW_SUB,
	LZW_IN_SUB
};

struct gif_pallete
{
	INT32 num;
	UINT32 global_abs_pal[GIF_MAX_PALLETE_LEN];//index -> true color
	UINT32 local_abs_pal[GIF_MAX_PALLETE_LEN];
#if 0	
	UINT8 sort_pal[GIF_MAX_PALLETE_LEN];
	UINT8 idx_pal[GIF_MAX_PALLETE_LEN];//index -> index
#endif	
};

struct gif_lzw
{
	//lzw parameters
	UINT32 min_code_size;
	UINT32 code_size;
	UINT32 table_idx;
	UINT16 clear_code;
	UINT16 end_code;
	UINT16 prefix;
	UINT16 suffix;
	UINT32 table_start[GIF_TABLE_LEN];
	UINT32 stack_start[GIF_TABLE_LEN];
	UINT32 *stack_ptr;

	//lzw flow step control
	UINT32 flag;

	// line information
	UINT32 line_buf;
	UINT32 line_read;
	INT32 pixel_num;
	INT32 sum_lines;// count the num of lines successfully decompressed

	//pallete
	struct gif_pallete pallete;

	//parametes about the current image
	UINT8 img_id;
	UINT8 last_img;
	gif_rect rect;
	UINT32 tick_start;
	UINT32 delay_num;

	UINT32 bitmap;
	UINT32 pixel_pitch;
	UINT32 img_pitch;
	UINT32 img_size;

	UINT32 bk_screen_buf;
	UINT32 bk_screen_size;
	int bk_screen_update;

	UINT16 iteration_idx;
};

#endif

