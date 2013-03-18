
#ifndef _PNG_INFLATE_
#define _PNG_INFLATE_

#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>

//MACRO
#define PNG_INF_LEN_HUF_MIN_BITS					(7)
#define PNG_INF_CODE_HUF_MIN_BITS					(9)
#define PNG_INF_DIS_HUF_MIN_BITS					(8)

#define PNG_INF_BLK_NONE_COMMPRESSION			(0x0)
#define PNG_INF_BLK_FIXED_HUF_TABLE				(0x1)
#define PNG_INF_BLK_DYN_HUF_TABLE					(0x2)

#define PNG_MAX_WIN_SIZE							(0x8000)
#define PNG_MAX_LEN									(258)
#define PNG_MAX_BITS								(15)

#define INF_CTRL_SUB_BITS_BASE							(0x80)
#define INF_CTRL_CHAR									(0xFE)
#define INF_CTRL_BLK_END								(0xFF)

#define INF_HUF_TAB_CODE_LEN							(1)
#define INF_HUF_TAB_CODE								(2)
#define INF_HUF_TAB_DIS								(3)


enum PNG_INF_STATUS
{
	INF_BLK_HEADER,
	INF_TAB_LEN,
	INF_LEN_TABLE,
	INF_DEC_CODE_LEN,
	INF_DEC_DIS_LEN,
	INF_CODE_TABLE,
	INF_DIS_TABLE,
	INF_LITER_CODE,
	INF_LEN_CODE,
	INF_DIS_CODE,
	INF_DIS_EXTRA,
	INF_WRITE_DATA,
	INF_NO_COM_LEN,
	INF_NO_COM,
};

//STRUCTURE
struct png_blk_pars
{
	UINT32 BFINAL:1;
	UINT32 BTYPE:2;
	UINT32 RES:29;

	UINT16 LEN;
	UINT16 NLEN;
};

struct png_inf_table
{
	// 32 - 16: literal or relative start address of the sub tree or base value 
	// 15 - 8  : bits, real bit num of the code
	// 7 - 0    : control, extra bits value -- (0 ~ 13), bits left into the sub tree -- (-1 ~ -10), 128 -- literal code
	UINT32 *code; 
	UINT32 size;
};

struct png_inf_scan
{
	UINT8 *write;
	UINT8 *read;
	UINT8 *start;
	UINT32 size;
	UINT32 finish;

	UINT32 count;
	UINT32 line_size;
	UINT32 line_num;

	UINT32 bpp;
};

struct png_inflate
{
	enum PNG_INF_STATUS status;
	
	struct png_inf_table sym_table;
	struct png_inf_table dis_table;
	struct png_inf_table len_table;
	
	struct png_blk_pars pars;
	struct png_inf_scan scan;

	struct png_cfg cfg;

	UINT8 *code_len;
	UINT8 *dis_len;
	UINT8 *len_len;
	
	UINT32 len;
	UINT32 dis;
	UINT16 len_base;
	UINT16 dis_base;
	UINT8 dis_ext_bits;
	UINT8 len_ext_bits;
	
	UINT8 len_tab_bits;
	UINT8 len_tab_num;
	
	UINT32 code_table_num;
	UINT32 dis_table_num;

	UINT8 code_table_bits;
	UINT8 dis_table_bits;

	UINT8 pre_len;
	UINT32 count;

	UINT32 bit_value;
	UINT32 bit_left;

	UINT32 *table;

	UINT32 deflate_blk_num;
};
//ROUTINE


#endif

