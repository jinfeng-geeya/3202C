/*****************************************************************************
*    Copyrights(C) 2005 ALi Corp. All Rights Reserved.
*
*    Company confidential and Properietary information.
*    This information may not be disclosed to unauthorized
*    individual.
*
*    File:    imagedec.h
*
*    Description:   
*		             
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	 1.  2007.4.29        john li		  1.0.0      	Create file.
*****************************************************************************/

#ifndef __IMAGE_DECODER_HEADER
#define __IMAGE_DECODER_HEADER

#include <api/libimagedec/imagedec.h>
#include <mediatypes.h>
#include "imagedec_bmp_decoder.h"

#define IMAGEDEC_SINGLE_INPUT_BUF

#if 0
#define jpeg_printf libc_printf
#define	JPEG_ASSERT(expression)				\
	{									\
		if (!(expression))				\
		{								\
			jpeg_printf("assertion(%s) failed: file \"%s\", line %d\n",	\
				#expression, __FILE__, __LINE__);	\
			SDBBP();					\
		}								\
	}
#else
#define jpeg_printf(...) do{}while(0)
#define JPEG_ASSERT(...)	do{}while(0)
#endif

#define JPEG_ENTRY					\
		{\
			jpeg_printf("/***************/");	\
			jpeg_printf("entry <%s>\n",__FUNCTION__);\
		}
		
#define JPEG_EXIT					\
		{\
			jpeg_printf("$***************$");	\
			jpeg_printf("exit <%s>\n",__FUNCTION__);\
		}

#define JPEG_NEW_HUF_TABLE	
#define JPEG_CUT_EDGE
//#define JPEG_DUMP_ORI_FILE

#define TV_MAX_WIDTH		720
#define TV_MAX_HEIGHT		576

#ifdef JPEG_DUMP_ORI_FILE
#define JPEG_DUMP_START_ADDR 0xA0D00000
#endif

#ifdef JPEG_NEW_HUF_TABLE
#define JPEG_ROOT_BITS				(10)
#define JPEG_SHIFT_BITS				(6) 		// 16 - JPEG_ROOT_BITS	
#define JPEG_SHIFT_MSK				(0x3FF) 
#define JPEG_SUB_BASE				(0x80)
#endif

typedef   signed char  schar;       /*  8 bits     */
typedef unsigned char  uchar;       /*  8 bits     */
typedef   signed short int16;       /* 16 bits     */
typedef unsigned short uint16;      /* 16 bits     */
typedef unsigned short ushort;      /* 16 bits     */
typedef unsigned int   uint;        /* 16/32+ bits */
typedef unsigned long  ulong;       /* 32 bits     */
typedef   signed int   int32;       /* 32+ bits    */

#ifndef max
#define max(a,b) (((a)>(b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b) (((a)<(b)) ? (a) : (b))
#endif

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef false
#define false (0)
#endif

#ifndef true
#define true (1)
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#define IMAGEDEC_INLINE static inline
#define IMAGEDEC_STATIC static

typedef int bool;

//#define SEEK_SET	0

#define SYS_MS_CNT 				(SYS_CPU_CLOCK/2000)

#define JPGD_INBUFSIZE (64*1024)   //(32*1024)  

#define JPGD_MAX_HEIGHT 16384
#define JPGD_MAX_WIDTH  16384

#ifdef JPEG_CUT_EDGE
#define JPGD_HWBUFYSIZE 0xE8C00	//896X1064
#define JPGD_HWBUFCSIZE 0xE8C00
#else
#define JPGD_HWBUFYSIZE 0xBB080	//720X1064
#define JPGD_HWBUFCSIZE 0xBB080
#endif

// extend the hw buf size to support max block mum 2496
#define JPGD_HWBUFYSIZE2 ((JPGD_MAX_HEIGHT * JPGD_MAX_WIDTH)>>6)
#define JPGD_HWBUFCSIZE2 JPGD_HWBUFYSIZE2

// May need to be adjusted if support for other colorspaces/sampling factors is added
#define JPGD_MAXBLOCKSPERMCU 10
//------------------------------------------------------------------------------
#define JPGD_MAXHUFFTABLES   8
#define JPGD_MAXQUANTTABLES  4
#define JPGD_MAXCOMPONENTS   4
#define JPGD_MAXCOMPSINSCAN  4
//------------------------------------------------------------------------------
// Increase this if you increase the max width!
#define JPGD_MAXBLOCKSPERROW 6144

//------------------------------------------------------------------------------
// Max. allocated blocks
#define JPGD_MAXBLOCKS    90
//------------------------------------------------------------------------------
/* JPEG specific errors */
#define JPGD_BAD_DHT_COUNTS              -200
#define JPGD_BAD_DHT_INDEX               -201
#define JPGD_BAD_DHT_MARKER              -202
#define JPGD_BAD_DQT_MARKER              -203
#define JPGD_BAD_DQT_TABLE               -204
#define JPGD_BAD_PRECISION               -205
#define JPGD_BAD_HEIGHT                  -206
#define JPGD_BAD_WIDTH                   -207
#define JPGD_TOO_MANY_COMPONENTS         -208
#define JPGD_BAD_SOF_LENGTH              -209
#define JPGD_BAD_VARIABLE_MARKER         -210
#define JPGD_BAD_DRI_LENGTH              -211
#define JPGD_BAD_SOS_LENGTH              -212
#define JPGD_BAD_SOS_COMP_ID             -213
#define JPGD_W_EXTRA_BYTES_BEFORE_MARKER -214
#define JPGD_NO_ARITHMITIC_SUPPORT       -215
#define JPGD_UNEXPECTED_MARKER           -216
#define JPGD_NOT_JPEG                    -217
#define JPGD_UNSUPPORTED_MARKER          -218
#define JPGD_BAD_DQT_LENGTH              -219
#define JPGD_TOO_MANY_BLOCKS             -221
#define JPGD_UNDEFINED_QUANT_TABLE       -222
#define JPGD_UNDEFINED_HUFF_TABLE        -223
#define JPGD_NOT_SINGLE_SCAN             -224
#define JPGD_UNSUPPORTED_COLORSPACE      -225
#define JPGD_UNSUPPORTED_SAMP_FACTORS    -226
#define JPGD_BAD_RESTART_MARKER          -227
#define JPGD_ASSERTION_ERROR             -228
#define JPGD_BAD_SOS_SPECTRAL            -229
#define JPGD_BAD_SOS_SUCCESSIVE          -230
#define JPGD_STREAM_READ                 -231
#define JPGD_NOTENOUGHMEM                -232
#define JPGD_DECODE_ERROR1                -235
#define JPGD_DECODE_ERROR2                -236
#define JPGD_DECODE_ERROR3                -237
#define JPGD_DECODE_ERROR4                -238
#define JPGD_DECODE_ERROR5                -239
#define JPGD_DECODE_ERROR6                -240
#define JPGD_DECODE_ERROR7                -241
#define JPGD_DECODE_ERROR8                -242
#define JPGD_DECODE_ERROR9                -243
#define JPGD_NOT_SUPPORT_SCALE_UP  -244
#define JPGD_IN_BUF_BUSY			  -245
#define JPGD_GET_INFO_ERROR		  -246
#define JPGD_WRONG_FILE_FLAG		  -247
#define JPGD_OSD_BITMAP_ERROR		  -248
#define JPGD_OSD_UNSUPPORTED_PIC_SIZE 	-249
#define JPGD_OSD_NO_ENOUGH_BITMAP_BUF	-250
#define JPGD_OSD_SHOW_RECT_FAIL			-251

//------------------------------------------------------------------------------
#define JPGD_GRAYSCALE 0
#define JPGD_YH1V1     1 //4:4:4
#define JPGD_YH2V1     2 //4:2:0
#define JPGD_YH1V2     3 //manson: no such format
#define JPGD_YH2V2     4 //4:2:2
#define JPGD_YH4V1     5 //4:1:1
//------------------------------------------------------------------------------
// origin forms. 
//static int JPGD_FAILED = -1;
//static int JPGD_DONE = 1;
//static int JPGD_OKAY = 0;
// to avoid warning when compiled. change them to macro. Peter luo 2003-6-10.
#define JPGD_FAILED		-1
#define JPGD_DONE		1
#define JPGD_OKAY		0

#define IMAGEDEC_MAX_MODES	

/*app macro*/
#define IMAGEDEC_MAX_APP  16
#define IMAGEDEC_EXIF_APP  1
#define IMAGEDEC_TIFF_IFD_NUM 2
#define IMAGEDEC_TIFF_IFD_ENTRY_NUM 100
#define IMAGEDEC_TIFF_IFD_ENTRY_SIZE 12

// To avoid deadlock, change the terminate(), incuding that in .h. Peter luo 2003-6-3
#define terminate(status) do {error_code = status; jpeg_printf("error code :%d stop in line=%d fun %x\n", error_code,__LINE__,__FUNCTION__); return ;} while(0)
#define iterminate(status) do {error_code = status; jpeg_printf("error code :%d stop in line=%d fun %x\n", error_code,__LINE__,__FUNCTION__); return 0;} while(0)

typedef void (*Pdecode_block_func)(int, int, int);

//------------------------------------------------------------------------------
typedef enum
{
	BIG_ENDIAN,
	LITTLE_ENDIAN
}imagedec_endian;

typedef enum
{
	RELEASED,
	INITED,
	STOPED,
	STARTED,
	PAUSED
}imagedec_status;

typedef enum
{
  M_SOF0  = 0xC0,
  M_SOF1  = 0xC1,
  M_SOF2  = 0xC2,
  M_SOF3  = 0xC3,

  M_SOF5  = 0xC5,
  M_SOF6  = 0xC6,
  M_SOF7  = 0xC7,

  M_JPG   = 0xC8,
  M_SOF9  = 0xC9,
  M_SOF10 = 0xCA,
  M_SOF11 = 0xCB,

  M_SOF13 = 0xCD,
  M_SOF14 = 0xCE,
  M_SOF15 = 0xCF,

  M_DHT   = 0xC4,

  M_DAC   = 0xCC,

  M_RST0  = 0xD0,
  M_RST1  = 0xD1,
  M_RST2  = 0xD2,
  M_RST3  = 0xD3,
  M_RST4  = 0xD4,
  M_RST5  = 0xD5,
  M_RST6  = 0xD6,
  M_RST7  = 0xD7,

  M_SOI   = 0xD8,
  M_EOI   = 0xD9,
  M_SOS   = 0xDA,
  M_DQT   = 0xDB,
  M_DNL   = 0xDC,
  M_DRI   = 0xDD,
  M_DHP   = 0xDE,
  M_EXP   = 0xDF,

  M_APP0  = 0xE0,
  M_APP1  = 0xE1,/*exif section*/
  M_APP15 = 0xEF,

  M_JPG0  = 0xF0,
  M_JPG13 = 0xFD,
  M_COM   = 0xFE,

  M_TEM   = 0x01,

  M_ERROR = 0x100
} JPEG_MARKER;

#define SEEK_CUR    1
#define SEEK_END    2
#define SEEK_SET    0

//------------------------------------------------------------------------------
#define RST0 0xD0
//------------------------------------------------------------------------------
typedef struct Imagedec_bmp_pallete_t
{
	UINT8 blue;
	UINT8 green;
	UINT8 red;
	UINT8 alpha;
}Imagedec_bmp_pallete,*pImagedec_bmp_pallete;

typedef UINT8 imagedec_marker;
typedef bool (*app_proc)(void*); 
typedef void (* bmp_dec)(void *,void*);
typedef void (*bmp_trans)(void *,void*);
typedef struct huff_tables_tag
{
#ifdef JPEG_NEW_HUF_TABLE
   // field1(0 ~ 7): control info   -- 1 ~  16	: code lenght
   // field2(8 ~ 15): huf code -- 0  ~ 255
   // field3(16 ~ 31): sub tree index from the start position of the root tree
   UINT32 *huf_table;
#else
//  uint  look_up[256];
  int look_up[256];//for clearing the warning added by sam cp chen
  uchar code_size[256];
  uint look_up_and_code_size[256]; //manson: merge them to speed up

  // FIXME: Is 512 tree entries really enough to handle _all_ possible
  // code sets? I think so but not 100% positive.
  uint  tree[512];
 #endif
} huff_tables_t, *Phuff_tables_t;
//------------------------------------------------------------------------------
typedef struct coeff_buf_tag
{
  uchar *Pdata;

  int block_num_x, block_num_y;
  int block_len_x, block_len_y;

  int block_size;

} coeff_buf_t, *Pcoeff_buf_t;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#define QUANT_TYPE int16
#define BLOCK_TYPE int16
typedef struct Imagedec_in_buf_item_t
{
	UINT8 *in_buf;
	int in_buf_len;
	int in_buf_left;
	UINT32 busy:1;
	UINT32 end:1;
	UINT32 error:1;
	UINT32 reserved:29;
}Imagedec_in_buf_item,*pImagedec_in_buf_item;

typedef struct Imagedec_hw_vld_mgr_t
{
	UINT32	uread;	//read pointer
	UINT32	uwrite;	//write pointer
	UINT32	valid_size;	//byte cnt
	UINT32	cfg_offset;	//in byte
	UINT32	cfg_length;	//in byte
	UINT8	cfg_control_bits;
	UINT32   used_bits;

	UINT32	buf_size;
	UINT32	buf_start_addr;
	UINT32	buf_end_addr;
}Imagedec_hw_vld_mgr, *pImagedec_hw_vld_mgr;

typedef struct Imagedec_stream_mgr_t
{
	UINT8* in_buf;
	int total_bytes_read;
	UINT8 *Pin_buf_ofs;
	int in_buf_len;
	int in_buf_left;
	UINT8 buf_idx_using;
	Imagedec_in_buf_item buf_item[2];
	
	bool eof_flag;
	int (*fread)(UINT32 fh,UINT8 *buf, UINT32 size);
	BOOL (*fseek)(UINT32 fh,INT32 offset, UINT32 origin);
	int (*ftell)();
	UINT32 reserved;

	Imagedec_hw_vld_mgr vld_mgr;
}Imagedec_stream_mgr, *pImagedec_stream_mgr;

typedef struct Image_mem_ctrl_t
{
	unsigned long mem_start;
	unsigned long mem_pos;
	unsigned long mem_up_limit;

	void  *blocks[JPGD_MAXBLOCKS];         /* list of all dynamically allocated blocks */
}Image_mem_crl, *pImage_mem_ctrl; 


typedef struct Imagedec_display_info_t
{
	short angel;			// the rotate angle (in degree, such as 360) of the pic displayed.
	unsigned short ratio;	// the Zoom ratio of the pic displayed, only can be > 0.
	unsigned short zoomx;		// the X offset on the zoomed screen.
	unsigned short zoomy;		// the Y offset on the zoomed screen.

	unsigned short width;		// screen resolution width.
	unsigned short height;		// screen resolution height.
	unsigned short stride;		// stride, change to positive.

	unsigned char* y_addr;		// Y buf address.
	unsigned char* c_addr;		// C buf address.

}Imagedec_display_info, pImagedec_display_info;

typedef struct Imagedec_thumb_t
{
	
}Imagedec_thumb,*pImagedec_thumb;

typedef struct Imagedec_exif_t
{
	
}Imagedec_exif,*pImagedec_exif;
typedef struct Imagedec_decoder_t
{
	int   comps_in_frame;                 /* # of components in frame */
	int   comp_h_samp[JPGD_MAXCOMPONENTS];     /* component's horizontal sampling factor */
	int   comp_v_samp[JPGD_MAXCOMPONENTS];     /* component's vertical sampling factor */
	int   comp_quant[JPGD_MAXCOMPONENTS];      /* component's quantization table selector */
	int   comp_ident[JPGD_MAXCOMPONENTS];      /* component's ID */

	int   comp_h_blocks[JPGD_MAXCOMPONENTS];
	int   comp_v_blocks[JPGD_MAXCOMPONENTS];

	int   comps_in_scan;                  /* # of components in scan */
	int   comp_list[JPGD_MAXCOMPSINSCAN];      /* components in this scan */
	int   comp_dc_tab[JPGD_MAXCOMPONENTS];     /* component's DC Huffman coding table selector */
	int   comp_ac_tab[JPGD_MAXCOMPONENTS];     /* component's AC Huffman coding table selector */
	int   spectral_start;                 /* spectral selection start */
	int   spectral_end;                   /* spectral selection end   */
	int   successive_low;                 /* successive approximation low */
	int   successive_high;                /* successive approximation high */

	int   max_mcu_x_size;                 /* MCU's max. X size in pixels */
	int   max_mcu_y_size;                 /* MCU's max. Y size in pixels */

	int   blocks_per_mcu;
	int   max_blocks_per_row;
	int   mcus_per_row, mcus_per_col;

	int   mcu_org[JPGD_MAXBLOCKSPERMCU];

	int   total_lines_left;               /* total # lines left in image */
	int   mcu_lines_left;                 /* total # lines left in this MCU */

	int   dest_bytes_per_scan_line;        /* rounded up */
	int   dest_bytes_per_pixel;            /* currently, 4 (RGB) or 1 (Y) */


	int   image_x_size;
	int   image_y_size;

	int   progressive_flag;

	QUANT_TYPE *quant[JPGD_MAXQUANTTABLES];    /* pointer to quantization tables */

	int   scan_type;                      /* Grey, Yh1v1, Yh1v2, Yh2v1, Yh2v2,
                                           CMYK111, CMYK4114 */
	int   out_type;				
	int   out_blocks_per_mcu;

 void  *blocks[JPGD_MAXBLOCKS];

	bool ready_flag;

	int   restart_interval;
	int   restarts_left;
	int   next_restart_num;

	int   max_mcus_per_row;
	int   max_blocks_per_mcu;

	int   max_mcus_per_col;
	int	mcus_of_col;

	  union 
	  {
		uint bit_buf;
		uint bit_buf_64[2];
	  } bit;
	int   bits_left;

	uint *component[JPGD_MAXBLOCKSPERMCU];   /* points into the lastdcvals table */
	int  last_dc_val[JPGD_MAXCOMPONENTS];

	Phuff_tables_t dc_huff_seg[JPGD_MAXBLOCKSPERMCU];
	Phuff_tables_t ac_huff_seg[JPGD_MAXBLOCKSPERMCU];

//	int *block_max_zag_set;  
	BLOCK_TYPE ** block_seg; 

	Phuff_tables_t h[JPGD_MAXHUFFTABLES];

	Pcoeff_buf_t dc_coeffs[JPGD_MAXCOMPONENTS];
	Pcoeff_buf_t ac_coeffs[JPGD_MAXCOMPONENTS];

	int eob_run;

	int block_y_mcu[JPGD_MAXCOMPONENTS];

	uchar *huff_num[JPGD_MAXHUFFTABLES];  /* pointer to number of Huffman codes per bit size */
	uchar *huff_val[JPGD_MAXHUFFTABLES];  /* pointer to Huffman codes per bit size */

	
	/*add by sam for supporting bmp file*/
	BITMAPFILEHEADER file_header;
	BITMAPINFOHEADER info_header;
	UINT32 file_offset_cnt;
	UINT32 bmp_pallete[256];
	bmp_dec bmp_decoder;
	bmp_trans bmp_transfer;
	UINT32 bmp_real_size_done;
	UINT32 bmp_pitch;
	UINT8 *bmp_dec_row_buf;
	UINT32 bmp_dec_row_buf_len;
	
	UINT32 real_size_done:1;
	UINT32 reserved:31;

	UINT16 *hw_vld_max_code[JPGD_MAXHUFFTABLES];
	UINT8 *hw_vld_valptr[JPGD_MAXHUFFTABLES];
	UINT8 *hw_vld_symbol[JPGD_MAXHUFFTABLES];
}Image_decoder, *pImage_decoder;
typedef struct Imagedec_frm_t
{
	UINT8 *frm_y_addr;
	UINT32 frm_y_len;
	UINT8 *frm_c_addr;
	UINT32 frm_c_len;
	UINT32 busy;
}Imagedec_frm,*pImagedec_frm;
typedef struct Imagedec_logo_info_t
{
	int buf_idx;
	UINT8 *frm_y_addr;
	UINT8 *frm_c_addr;
	int width;
	int height;
	int stride;
}Imagedec_logo_info,*pImagedec_logo_info;

typedef struct Imagedec_output_t
{
	UINT32 image_out_x;
	UINT32 image_out_y;
	UINT32 image_out_w;
	UINT32 image_out_h;
	UINT32 image_output_stride;
	UINT8 *frm_y_addr;
	UINT32 frm_y_len;
	UINT8 *frm_c_addr;
	UINT32 frm_c_len;
	UINT8 frm_idx_using;
	UINT8 frm_idx_last_dis;
	UINT8 pic_idx_total;
	UINT8 dis_idx_total;
	Imagedec_frm frm[2];
	UINT32 frm_mb_type;
	UINT32 reserved;
	void (*y_dup)(int *,uchar *);
	void (*c_dup)(int *,uchar *);
	Imagedec_logo_info logo_info;
	UINT32 hw_acc_frm_y;
	UINT32 hw_acc_frm_c;
	UINT32 hw_acc_stride;
	UINT8 hw_h_pre;
	UINT8 hw_v_pre; 
	struct Rect hw_acce_rect;
	struct Rect sw_scaled_rect;
	struct Rect de_src_rect;//hw scale of Display Engine
	struct Rect de_dst_rect;
	struct Rect de_dst_bk;

	Imagedec_frm old_frm;
	Imagedec_frm new_frm;

	int hw_max_divide_h;
	int hw_max_divide_v;
	int pixel_per_mcu_h;
	int pixel_per_mcu_v;	
	int divide_mcu_row;
	int divide_mcu_col;	
	int last_mcu_row;
	int last_mcu_col;

	// flags for the combining the hw accelerator and sw scale together
	int combine_hw_mcu_col_max;
	int combine_hw_mcu_col_cnt;

	int combine_hw_mcu_row_cnt;
	int combine_hw_mcu_row_max;
}Imagedec_output,*pImagedec_output;

typedef struct Imagedec_setting_t
{
	enum IMAGE_DIS_MODE mode;
	enum IMAGE_ANGLE angle;
	struct Rect src;
	struct Rect dis;
	UINT32 dis_setting_update:1;
	UINT32 hw_acc_flag:1;
	UINT32 show_flag:1;
	UINT32 fill_logo:1;
	UINT32 reserved_bits:28;

	enum IMAGE_SHOW_MODE show_mode;
	UINT8 show_mode_par[IMAGEDEC_SHOW_MODE_PAR_LEN];

	UINT32 fill_color; // ycbcr
}Imagedec_setting,*pImagedec_setting;

typedef struct imagedec_copy_frm_ins_t
{
	UINT32 on:1;
	UINT32 res:31;

	void *logo_buf;
	UINT32 y_buf_size;
	UINT32 c_buf_size;
	UINT16 width;
	UINT16 height;
	
	Imagedec_Io_Cmd_Copy_Frm input_par;
}imagedec_copy_frm_ins,*pimagedec_copy_frm_ins;

typedef struct Imagedec_main_ctrl_t
{
	UINT32 mode_update:1;
	UINT32 zoom_update:1;
	UINT32 hw_act_update:1;
	UINT32 rot_par_update:1;
	UINT32 show_mode_update:1;
	UINT32 fill_logo_update:1;
	UINT32 osd_mode_update:1;
	UINT32 copy_frm_update:1;
	UINT32 reserved:24;
	Imagedec_setting   api_cmd;
	Imagedec_setting	 setting;
	UINT32 (*status)(void *value);
}Imagedec_main_ctrl,*pImagedec_main_ctrl;
typedef struct Imagedec_tiff_ifd_t
{
	UINT32 entry_num;
	void *entry[IMAGEDEC_TIFF_IFD_ENTRY_NUM];
}Imagedec_tiff_ifd,*pImagedec_tiff_ifd;
typedef struct Imagedec_tiff_t
{
	UINT16 imagic_num;//no use
	UINT32 next_ifd_offset;
	Imagedec_tiff_ifd ifd[IMAGEDEC_TIFF_IFD_NUM];
}Imagedec_tiff,*pImagedec_tiff;
typedef struct Imagedec_tmp_bit_info_t
{
	UINT8 *tmp_buf_ofs[2];
	int tmp_in_buf_left[2];
	uint tmp_bit_buf[2];
	int tmp_bits_left[2];
}Imagedec_tmp_bit_info,*pImagedec_tmp_bit_info;
typedef struct Imagedec_app1_exif_t
{
	UINT32 tiff_start_pos;//start position in the buffer
	UINT32 length;
	UINT32 tiff_offset;//unit: byte
	UINT32 reserved;
	Imagedec_tiff tiff;
	Imagedec_tmp_bit_info tmp_bit_info;
	UINT32 image_offset;
	UINT32 image_length;
	UINT8 byte_order:1;/*little(0) or big(1) endian*/
	UINT8 image_find:1;
	UINT8 reserved2:6;
}Imagedec_app1_exif,*pImagedec_app1_exif;
typedef struct Imagedec_app_item_t
{
	app_proc processor;
	void *par;
	bool active;
}Imagedec_app_item,*pImagedec_app_item;
typedef struct Imagedec_app_ctrl_t
{
	Imagedec_app_item item[IMAGEDEC_MAX_APP];
	UINT32 reserved;
}Imagedec_app_ctrl,*pImagedec_app_ctrl;
typedef struct Image_bmp_decoder_t
{
	
}Image_bmp_decoder,*pImage_bmp_decoder;
typedef struct Imagedec_info_t
{
	imagedec_file_format format;
	int precision;
	int width;
	int height;
	int coding_progress;
	int last_called_prog;
}Imagedec_info,*pImagedec_info;
typedef struct Imagedec_handle_t
{
	UINT32 fh;
	imagedec_id id;/*internal id = external id - 1*/
	imagedec_file_format file_format;
	Imagedec_info info;
	imagedec_endian file_endian;
	imagedec_status status;
	Imagedec_output imageout;
	Imagedec_stream_mgr in_stream;
	Image_decoder decoder;
//	Image_bmp_decoder bmp_decoder;
	Image_mem_crl mem;
	Imagedec_main_ctrl main;
	Imagedec_app_ctrl app;
	UINT32 reserved;
}Imagedec_hdl, *pImagedec_hdl;
//color functions
#define FILL_COLOR(y,c,y_s,c_s,c_y,c_b,c_r)	\
	{\
	UINT32 ct = 0,color = 0,*addr;\
	color = c_y&0xFF;addr = (UINT32 *)y;\
	color |= (color<<8)|(color<<16)|(color<<24);\
	for(;ct<(y_s>>2);ct++){\
		*addr++ = color;}\
	color = ((c_r&0xFF)<<8)|(c_b&0xFF);\
	color |= color<<16;addr = (UINT32 *)c;\
	for(ct=0;ct<(c_s>>2);ct++){\
		*addr++ = color;}	\
	}

/*big endian to little endian*/
/*16bits*/
#define BIG_TO_LIT_16(data)		\
	{	\
	data |= data<<16;	\
	data = (data>>8)&0xffff;\
	}
/*32bits*/
#define BIG_TO_LIT_32(data)		\
	data = (data<<24)|((data<<8)&0xff0000)|((data>>8)&0xff00)|((data>>24)&0xff)

#define MULTIP_5(d)				\
	d += (d<<2) 
	
extern Imagedec_hdl g_imagedec_hdl[IMAGEDEC_MAX_INSTANCE];
 extern int error_code;
 extern int tem_flag;

void set_comp (pImagedec_hdl phdl,  int icomp);
int max_ycbcr_comp (pImagedec_hdl phdl );
int IsProgressive (pImagedec_hdl phdl);
int IsPro2_big (void);
inline int IsTerminated (void);

void jpeg_quick_parsing_sof(pImagedec_hdl phdl);
 void prep_in_buffer(void);
 bool app_exif_decoder(void *pointer);
void imagedec_sub_response_to_api(pImagedec_hdl phdl);
void imagedec_fill_in_buffer_task(UINT32 upara1,UINT32 upara2);
void imagedec_jpeg_init(void);
int imagedec_jpeg_begin(void);
int imagedec_jpeg_decode(void);
void* imagedec_malloc(int n );
#endif

