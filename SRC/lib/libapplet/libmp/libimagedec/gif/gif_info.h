#ifndef _GIF_INFO_
#define _GIF_INFO_
#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>

//MACRO
#define GIF_IMAGE_SEPARATOR				0x2C
#define GIF_EXT_INTRODUCER					0x21
#define GIF_BLOCK_TER_FLAG					0x00

#define GIF_FILE_TRAILER					0x3B


//extention lable
#define GIF_GRAPHIC_CTRL					0xF9

/*RGB2YCbCr coefficient*/
#define Y_COF_R		0x41CA//0.257<<16	
#define Y_COF_G		0x8106//0.504<<16	
#define Y_COF_B		0x1916//0.098<<16
#define C_COF_Cb	0x2049b// 2.018<<16
#define C_COF_Cr		0x19893// 1.596<<16

#define CB_COF_R	0x25e3
#define CB_COF_G	0x4A7E
#define CB_COF_B	0x7062
#define CR_COF_R	0x7062
#define CR_COF_G	0x5e35
#define CR_COF_B	0x122D

//STRUCTURE
struct gif_info_header
{
	//signature
	UINT8 sig0;
	UINT8 sig1;
	UINT8 sig2;
	//version
	UINT8 ver0;
	UINT8 ver1;
	UINT8 ver2;
};

struct gif_info_lsdes
{
	UINT8 w_low;
	UINT8 w_high;
	UINT8 h_low;
	UINT8 h_high;
	UINT8 gt_size:3;//global table size
	UINT8 sort_flag:1;
	UINT8 color_res:3;//color resolution
	UINT8 gt_flag:1;//global table flag
	UINT8 bk_color;
	UINT8 aspec_ratio;
};

struct gif_info_imgdes
{
	UINT8 x_low;
	UINT8 x_high;
	UINT8 y_low;
	UINT8 y_high;	
	UINT8 w_low;
	UINT8 w_high;
	UINT8 h_low;
	UINT8 h_high;
	UINT8 lt_size:3;//local table size
	UINT8 res:2;
	UINT8 sort_flag:1;
	UINT8 int_flag:1;//interlace flag
	UINT8 lt_flag:1;//local table flag
	UINT16 iteration_num;
};

struct gif_info_gce
{
	UINT8 block_size;
	UINT8 alpha_flag:1;//transparent color flag
	UINT8 user_in:1;
	UINT8 disposal_m:3;
	UINT8 res:3;
	UINT8 dy_time_low;
	UINT8 dy_time_high;
	UINT8 alpha_value;
	UINT8 block_tm;//block terminater
};

struct gif_info_off
{
	UINT32 first_img;
};

struct gif_info 
{
	struct gif_info_header header;
	struct gif_info_lsdes lsdes;
	struct gif_info_imgdes imgdes;
	struct gif_info_gce gce;
	struct gif_info_off offset;
	UINT32 flag;
};

#endif

