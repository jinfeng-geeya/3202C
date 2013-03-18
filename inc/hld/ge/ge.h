/*
* Copyright (C) 2005 ALi Corp. All Rights Reserved.
* All rights reserved.
*
* File: ge_s3601.h
* Description£º This file define the struct of graphic engine device.
*
* Revision£º 1.0
* Author£º Leimei Yang
* Data£º Oct,14,2005
*
* History:
* -------------------------------------------------
* Revision Author Data Reason
* -------------------------------------------------
* 1.0 Leimei Yang Oct,14,2005 Create file.
*
*/
#ifndef _GE_H_
#define _GE_H_

//#include <hld/osd/osddrv_dev.h>
#include <hld/hld_dev.h>
#include <basic_types.h>

#define  SURFACE_MAX_REGION	6	/* max region for one surface */
#define  SURFACE_MAX_BLOCKS	36   /* max blocks for one surface */
#define	PALLET_SIZE		256*4			// pallete size, 256 RGB colors

#define	GE_PALLET_RGB			0x00
#define	GE_PALLET_YCBCR			0x01

/****************************************************************************
 *
 * 	define caller ID when create surface
 *    GMA_HW_SURFACE_2 is on top of GMA_HW_SURFACE_1
 *
 ****************************************************************************/
#define GMA_HW_SURFACE_1		0       
#define GMA_HW_SURFACE_2		1
#define GMA_MEM_SURFACE		2
#define GMA_HW_SURFACE_DEO	3       

/****************************************************************************
 *
 * GRAPHICS LIBRARY BLT FLAGS
 *
 ****************************************************************************/
/* source1 is selected */
#define GE_BLT_SRC1_VALID				0x00000001
/* source2 is selected */
#define GE_BLT_SRC2_VALID				0x00000002
 /*enable direct copy function */
#define GE_BLT_DIRECT_COPY				0x00000004
/* reserved */
#define GE_BLT_RESERVED				0x00000008

 /*enable color fill function*/
#define GE_BLT_RESERVED2				0x00000010
 /*do color fill with source1*/
#define GE_BLT_S1_COL_FILL				0x00000020
 /*do color fill with source2*/
#define GE_BLT_S2_COL_FILL				0x00000040
/* reserved */
#define GE_BLT_RESERVED3				0x00000080

  /*enable record function */
#define GE_BLT_RECORD					0x00000100
/*do mb copy */
#define GE_BLT_MBCOPY					0x00000200
/*enable resize function */
#define GE_BLT_RESIZE          			       0x00000400
 /*enable flicker filter function  */
#define GE_BLT_FLICK_FILT				0x00000800

 /*enable RGB to YUV conversion function */
#define GE_BLT_RGB2YUV					0x00001000
 /*enable YUV to RGB conversion function */
#define GE_BLT_YUV2RGB					0x00002000
 /*enable RGB to YUV conversion function */
#define GE_BLT_YUV422TO444				0x00004000
 /*enable YUV to RGB conversion function */
#define GE_BLT_YUV444TO422				0x00008000
 /*enable rectangle clipping function*/

#define GE_BLT_RECT_CLIP				0x00010000
 /*enable color expansion */
#define GE_BLT_COL_EXPAN				0x00020000
 /*enable color correction */
#define GE_BLT_COL_CORRECT				0x00040000
 /*enable color reduction */
#define GE_BLT_COL_REDUCT				0x00080000

 /*enable plane mask function */
#define GE_BLT_PLAN_MASK				0x00100000
 /*enable line drawing function */
#define GE_BLT_LINE_DRAW				0x00200000
 /*enable video line number  triggered */
#define GE_BLT_TRIG_EN					0x00400000
/* flag is reserved */
//#define GE_BLT_RESERVED				0x00800000

 /*enable color keying function */
#define GE_BLT_COLOR_KEY				0x01000000
/*enable alpha blending function */
#define GE_BLT_ALPHA_BLEND				0x02000000
 /*enable raster operation function */
#define GE_BLT_ROP						0x04000000
 /*enable color bool and alpha blending function */
#define GE_BLT_COLBOOL_ABLEND			0x08000000
 /*enable alpha remain function */
#define GE_BLT_COLBOOL_AREMAIN		0x10000000
 /*enable alpha color merge function */
#define GE_BLT_ALPHA_MERGE_COLOR		0x20000000

/* Do this blt mirroring the surface left to right.  Spin the
 * surface around its y-axis. */
#define GE_BLT_MIRROR_LEFTRIGHT		0x40000000
/* Do this blt mirroring the surface up and down.  Spin the surface
 * around its x-axis. */
#define GE_BLT_MIRROR_UPDOWN	       	0x80000000

/*ioctrl IO cmd list*/
#define OSD_IO_16M_MODE_29E					0x01
#define GE_IO_SCALE_OSD_29E					0x02
#define GE_IO_UPDATE_PALETTE_29E				0x03
#define GE_IO_ANTI_FLICK_29E					0x04
#define GE_IO_GLOBAL_ALPHA_29E				0x05
#define GE_IO_SET_TRANS_COLOR               			0x06
#define GE_IO_SWITCH_OPERATION_MODE			0x07
#define GE_IO_SET_CUR_REGION					0x08
#define GE_IO_SET_REG_PARS						0x09
#define GE_IO_GET_REG_PARS						0x0A
#define GE_IO_DRAW_BITMAP						0x0B
#define GE_IO_DRAW_LINE						0x0C
#define GE_IO_SET_ANTIFLK_PARA				0x0D
#define GE_IO_SET_DISPLAY_ADDR				0x0E    
#define GE_IO_SET_SCALE_MODE					0x0F   
#define GE_IO_SET_LAYER2_GLOBAL_ALPHA		0x10   
#define GE_IO_SCALE_OSD_DEO					0x11  
#define GE_IO_SET_EDGE_ALPHA_CTRL					0x12  


#define GE_IO_CTRL_BASE_GMA     				0x1000 // don't conflict with GE_IO_CTRL cmd

#define GE_IO_UPDATE_PALLETTE					(GE_IO_CTRL_BASE_GMA + 1) // Call it in DE ISR
#define GE_IO_RESPOND_API	    					(GE_IO_CTRL_BASE_GMA + 2)  // Call it in DE ISR
#define GE_IO_ENABLE_ANTIFLICK					(GE_IO_CTRL_BASE_GMA + 3)  // Enable/Disable GMA anti-flicker. io_param is TRUE or FLASE
#define GE_IO_SET_GLOBAL_ALPHA  				(GE_IO_CTRL_BASE_GMA + 4)  // Set GMA layer global alpha. io_param [0x00, 0xff], set 0xff will disable global alpha

#define GE_IO_GET_LAYER_ON_OFF  				(GE_IO_CTRL_BASE_GMA + 5)  // Get GMA layer show or hide(io_param is UINT32 *)

//#define GE_IO_SET_TRANS_COLOR   0x86 // Transparent color index for CLUT, default use 0xff; RGB will use 0 as transparent color

#define GE_IO_SET_AUTO_CLEAR_REGION 			(GE_IO_CTRL_BASE_GMA + 7)    /* Enable/Disable filling transparent color in ge_create_region().
                                              						After ge_open(), default is TRUE. Set it before ge_create_region().*/
#define GE_IO_SET_START_MODE        				(GE_IO_CTRL_BASE_GMA + 8)
#define GE_IO_SET_LAYER_DIS_SEQUENCE			(GE_IO_CTRL_BASE_GMA + 9)
#define GE_IO_SET_LATENCY_CNT					(GE_IO_CTRL_BASE_GMA + 0x0A)
#define GE_IO_SET_SYNC_MODE        				(GE_IO_CTRL_BASE_GMA + 0x0B)   /* enum GE_CMD_SYNC_MODE, default use interrupt sync mode */

/* Enable/Disable GE ouput YCBCR format to DE when source is CLUT8, clut8->ycbcr, not do color reduction
     only used when output is 576i/p or 480i/p*/
#define GE_IO_SET_YCBCR_OUT					0x00000100 

#define GE_IO_LOCK_DEV							0x00010000
#define GE_IO_UNLOCK_DEV						0x00020000

enum GE_V_SCAN_ORDER
{
	GE_TOP_TO_BOTTOM,
	GE_BOTTOM_TO_TOP
};

enum GE_H_SCAN_ORDER
{
	GE_LEFT_TO_RIGHT,
	GE_RIGHT_TO_LEFT
};

/* define color space */
enum GE_COLOR_SPACE
{
	GE_CS_RGB,		// rgb color space
	GE_CS_YCBCR	// ycbcr color space
};

/* define pixel endian */
enum GE_PIXEL_ENDIAN
{
	GE_PIXEL_LITTLE_ENDIAN, 	// little endian
	GE_PIXEL_BIG_ENDIAN		// big endian
};

#define GE_ALPHA_RANGE_BASE_36F	0x00

/* define alpha range */
enum GE_ALPHA_RANGE
{
	GE_LITTLE_ALPHA, 		// alpha is in 0-127
	GE_LARGE_ALPHA,		// alpha is in 0-255

	GE_ALPHA_RANGE_0_255 = GE_ALPHA_RANGE_BASE_36F,
	GE_ALPHA_RANGE_0_127,
	GE_ALPHA_RANGE_0_15,    // GE not support, only for GMA pallette
};

/* define resize mode */
enum GE_RSZ_MODE
{
	GE_RSZ_DISABLE =0,
	GE_RSZ_DIRECT_RESIZE=4,  /* copy pixels directly */ 
	GE_RSZ_ALPHA_ONLY =5,     /* filter active only on alpha channel */
	GE_RSZ_COLOR_ONLY =6,     /* filter active only on color channel */
	GE_RSZ_ALPHA_COLOR =7   /* filter active on alpha and color channel */
};

/* define flicker filter mode */
enum GE_FF_MODE
{
	GE_FF_FILTER0,    
	GE_FF_ADAPTIVE	
};

/* define CLUT operation mode */
enum  GE_CRDC_MODE
{
//	GE_COLOR_EXPAND, 	// color expansion mode ((A)CLUTn -> true color)
//	GE_COLOR_CORRECT,   	// color correction (true color -> true color)
	GE_CRDC_NORM=2,	// color reduction normal mode (true color -> CLUTn)
	GE_CRDC_SIMP=3 	// color reduction simple mode (true color -> CLUTn)
};

/* define color reduction weight */
enum  GE_CRDC_WEIGHT
{
	GE_CR_WEIGHT_0,    //  0%(disable)
	GE_CR_WEIGHT_25,  // 25%
	GE_CR_WEIGHT_50,  // 50%
	GE_CR_WEIGHT_100 // 100%
};


/* define color keying channel match mode */
enum GE_CKEY_MATCH_MODE
{
	GE_CKEY_ALWAYS_MATCH,	
	GE_CKEY_IN_RANGE,			//channel match when channel heigh >= channel value >= channel low value
	GE_CKEY_OUT_RANGE		//channel match when  channel value >= channel heigh or channel value<= channel low value
};

/* define color key input selection */
enum GE_CKEY_INPUT_SEL
{
	GE_CKEY_DESTINAION,   // destination color key
	GE_CKEY_SOURCE   // source color key
};

/* define alpha blending source data mode */
enum GE_AB_SRC_MODE
{
	GE_AB_SRC_PREMULTI,	// source color is alhpha premultipied, format is AR, AG and AB
	GE_AB_SRC_NOT_PREMULTI // source color is not alhpha premultipied
};

#define GE_ROP_MODE_BASE_36F 			0x00

/* define ROP operation mode */
 enum GE_ROP_MODE
{
	GE_ROP_SETBLACK		=0,		// set black
	GE_ROP_AND 			=0x01,	// S1 AND S2
	GE_ROP_ANDINV			=0x02,	// (NOT S2) AND S1
	GE_ROP_UPDATE			=0x03,	// S2
	GE_ROP_INVAND			=0x04,	// (NOT S1) AND S2
	GE_ROP_REMAIN			=0x05,	// S1
	GE_ROP_XOR				=0x06,	// S1 XOR S2
	GE_ROP_OR				=0x07,	// S1 OR S2
	GE_ROP_NOR			=0x08,	// S1 NOR S2
	GE_ROP_XORINV			=0x09,	// S1 XOR (NOT)S2
	GE_ROP_INVERT			=0x0A,	// NOT S1
	GE_ROP_INVOR			=0x0B,	// (NOT) S1 OR S2
	GE_ROP_UPDATEINV		=0x0C,	// NOT S2
	GE_ROP_ORINV			=0x0D,	// S1 OR (NOT) S2
	GE_ROP_NAND			=0x0E,	// NOT (S1 AND S2)
	GE_ROP_SETWHITE		=0x0F,	// set white

	// 36f new ROP mode ext
	GE_ROP_SRC_BYPASS 	= GE_ROP_MODE_BASE_36F,  // No ROP, PTN data on the same position will be ignored
	GE_ROP_PTN_BYPASS 	,      // No ROP, SRC data on the same position will be ignored
	GE_ROP_ALPHA_BLENDING,  // Enable alpha-blending between SRC and PTN
	GE_ROP_BOOL,            // Both alpha and color are operated by boolean operation
	GE_ROP_BOOL_ALPHA_BLEND// Only color are operated by boolean operation, 
	                                // Alpha is operated by alpha-blending	
};

/* define macroblock displaying selection */
enum GE_MB_FIELD
{
		GE_MB_TOP,			// select top field
		GE_MB_BOTTOM,		// select bottom field
		GE_MB_TOP_BOT		// select top and bottom field
};

//#if (SYS_CHIP_MODULE==ALI_S3602)		
/* define Macroblock mode */
enum GE_MB_MODE
{
		GE_MB_MPEG32X16,			// MPEG2 MB format is 32*16
		GE_MB_H26416X32			// h264 MB format is 16*32
};
//#endif

/* define color conversion system */
enum GE_CC_SYSTEM
{
	GE_CC_BT601,  		//ITU-R BT.601 
	GE_CC_BT709		//ITU-R BT.709
};

/* define color conversion matrix */
enum GE_CC_MATRIX
{
	GE_CC_GRAPHIC, 	// graphic matrix
	GE_CC_VIDEO	 	// vedio matrix
};

/* define color conversion chroma format */
enum GE_CC_CHROM_FMT
{
	GE_CC_OFFSET_BIN, 	   // offset binaray
	GE_CC_TWO_COMPLE	   // two's complement, signed 
};

/* define color conversion type */
enum GE_CC_TYPE
{
	GE_CC_YCBCR2RGB = 0x01,		 // convert YCBCR888  to RGB888
	GE_CC_RGB2YCBCR = 0x02, 	// convert RGB888 to YCBCR888
	GE_CC_YCBCR422TO444 = 0x04, // convert YCBCR422  to YCBCR888
	GE_CC_YCBCR444TO422 = 0x08  //convert YCBCR888  to YCBCR422
};

/* define line drawing mode */
enum GE_LINE_MODE
{
	GE_LINE_XY,		// draw one point with color get from source surface 
	GE_LINE_XYL,     	// draw line with color get from source surface
	GE_LINE_XYC,    	// draw one point with assigned color 
	GE_LINE_XYLC  	// draw line with assigned color
};

#define GE_PIXEL_FORMAT_MAX		(0x96)

// define pixel format
enum GE_PIXEL_FORMAT
{
	// used by 3602
	GE_PF_RGB565		=0x00,
	GE_PF_RGB888		=0x01,
	GE_PF_RGB555		=0x02,
	GE_PF_RGB444		=0x03,
	GE_PF_ARGB565		=0x04,
	GE_PF_ARGB8888	=0x05,
	GE_PF_ARGB1555	=0x06,
	GE_PF_ARGB4444	=0x07,
	GE_PF_CLUT1		=0x08,
	GE_PF_CLUT2		=0x09,
	GE_PF_CLUT4		=0x0A,	
	GE_PF_CLUT8		=0x0B,
	GE_PF_ACLUT88		=0x0C,
	GE_PF_YCBCR888	=0x10,
	GE_PF_YCBCR422	=0x12,
	GE_PF_YCBCR422MB	=0x13,
	GE_PF_YCBCR420MB	=0x14,
	GE_PF_AYCBCR8888	=0x15,
	GE_PF_A1			=0x18,
	GE_PF_A8			=0x19,
	GE_PF_CK_CLUT2       =0x89,
	GE_PF_CK_CLUT4       =0x8A,
	GE_PF_CK_CLUT8       =0x8B,
	GE_PF_ABGR1555       =0x90, /* GE not support, but DE support in M3202*/
	GE_PF_ABGR4444       =0x91, /* GE not support, but DE support in M3202*/
	GE_PF_BGR565           =0x92, /* GE not support, but DE support in M3202*/

	// new added for the 36f
	GE_PF_ACLUT44        = 0x93,
	GE_PF_YCBCR444       = 0x94,
	GE_PF_YCBCR420 	= 0x95,
	GE_PF_MASK_A1		= GE_PF_A1,
	GE_PF_MASK_A8		= GE_PF_A8,
};

/*define alpha level in pallette */
enum GE_PALLET_ALPHA_LEVEL
{
	GE_PALLET_ALPHA_16,
	GE_PALLET_ALPHA_128,
	GE_PALLET_ALPHA_256,	
};

typedef struct 
{
    	UINT32 ge_start_base;
   	UINT32 GMA1_buf_size;
    	UINT32 GMA2_buf_size;
    	UINT32 temp_buf_size;
	UINT16 cmd_buf_size;
	BOOL    bpolling_cmd_status;
	UINT32 op_mutex_enable:1;
	UINT32 st_on_osd_enable:1;
	UINT32 region_malloc_optimization:1;
	UINT32 res_bits:29;
	UINT32 GMA_deo_buf_addr;
	UINT32 GMA_deo_buf_size;	
}ge_driver_config_t;

typedef struct
{
	UINT16 src_h;
	UINT16 dst_h;
	UINT8 suf_id;
}ge_scale_par_29e_t, ge_scale_par_deo_t;

typedef struct 
{
	UINT8 layer;
	UINT8 valid;
}ge_anti_flick_t;

typedef struct 
{
	UINT8 layer;
	UINT8 edge_ctrl_cfg;
}ge_edge_alpha_ctrl_t;

typedef struct
{
	UINT8 layer;
	UINT8 valid;
	UINT8 value;
}ge_global_alpha_t;

typedef struct 
{
   	UINT8 *addr;	 					// pointer to pallette entry array. 
      	enum GE_COLOR_SPACE	type;	// pallette color space: RGB or YUV
    	BOOL  update;                                 	// flag for pallette update 
} ge_pallet_t;


/*
	structure for YCBCR and RGB conversion
*/
typedef struct
{
	// color conversion
	enum GE_CC_SYSTEM	system;  // BT 601 or 709
	enum GE_CC_MATRIX	matrix;   // grahic or video matrix
	enum GE_CC_CHROM_FMT	chrom_fmt; 
}ge_YUV_RGB_conv_t;


typedef struct 
{
	enum GE_CRDC_MODE mode;	
	enum GE_CRDC_WEIGHT	err_diff_wt;
	UINT8	err_diff_thres;
}ge_col_rdc_t;

/*
 * define resize infomation
 */
typedef struct 
{
	enum GE_RSZ_MODE     h_mode; // horizontal filter mode */
	enum GE_RSZ_MODE     v_mode; //vertical filter mode */
	BOOL     out_clip;  // clip y to 16-235, c to 16-240
	INT8     *coeff; // pointer to resize coefficient array
	UINT8     h_offset ; // initial subposition for HSRC 
	UINT8     v_offset ; //initial subposition for VSRC
}ge_resize_t; 

typedef struct
{
	UINT8 *coeff; // point to flicker filter coefficient array
	enum GE_FF_MODE  mode;		// flicker filter mode
	UINT8  thshold1;
	UINT8  thshold2;
	UINT8  thshold3;
//#if (SYS_CHIP_MODULE==ALI_S3602)		
	BOOL  clip_en;		// do clip to RGB/YCBCR
	BOOL  alpha_en;		// do flicker filter to alpha component
//#endif	
}ge_flick_filter_t;

/*
* define color key info
*/
typedef struct 
{
    UINT32	low_value;	// low boundary of color space that is to be treated as Color Key, inclusive
    UINT32	high_value;	// high boundary of color space that is to be treated as Color Key, inclusive
    enum GE_CKEY_MATCH_MODE	chn0_flags;	// channel 0 match condition
    enum GE_CKEY_MATCH_MODE	chn1_flags;	// channel 1 match condition
    enum GE_CKEY_MATCH_MODE	chn2_flags;	// channel 2 match condition
    enum GE_CKEY_INPUT_SEL		src_select;    // select reference:dst or src 
} ge_color_key_t;


typedef struct
{
	enum GE_AB_SRC_MODE  src_mode;		// src data is alpha premultiplied or not alpha premultiplied 
	BOOL   glob_alpha_en;
	UINT8  glob_alpha;	// global alpha 
//#if (SYS_CHIP_MODULE==ALI_S3602)		
	BOOL 	neg_alpha;	// alpha range: 0-127 or 0-255
//#endif
}ge_alpha_blend_t;


/*
 * define rectangle clipping info
 */
typedef struct 
{
    UINT16  left;
    UINT16  top;
    UINT16  right;
    UINT16  bottom;	
    BOOL	clip_inside;	// update inside window flag
 } ge_clip_t;

typedef struct
{
	UINT32	left;		//X coordinates of the left pixel of the line
	UINT32	top;        //y coordinates of the left pixel of the line
	UINT32	len;		// line length, in pixels unit
	UINT32	color;    // line color
}ge_line_subins_t;

typedef struct{
	ge_line_subins_t	*instr_ptr; // drawing instruction
	enum GE_LINE_MODE	mode; // line mode: 
	UINT16	num; // number of line to draw
}ge_line_draw_t;

typedef struct 
{
	INT32	left;		// start_x position related to base address
	INT32	top;		// start_y position related to base address
	INT32	width;		// width of input surface
	INT32	height;		// height of surface to be created    
}ge_rect_t;

typedef struct{
	void *buf;
	INT32 width;
	INT32 height;
	INT32 pitch; // unit is bits
	
	UINT32 bk_color;
	UINT32 fg_color;
	UINT32 bpp;  // bits per pixel. Only can be 1
	
	UINT32 polarity:1; // 1: bit0 --> fore ground color; 0: bit0 --> back ground color
	UINT32 bk_fill:1;  // 1: fill the back ground color; 0: don't fill the color
	UINT32 endian:1; // 1: big-endian;0: little-endian 
	UINT32 res:29;
}ge_bitmap_t;	

typedef struct{
	// reserved
}ge_line_t;

#if 0
typedef struct 
{
	ge_rect_t rect;
	enum GE_PIXEL_FORMAT	pixel_fmt;	// pixel format description of the surface
}ge_rect_desc_t;
#endif

typedef struct
{
	UINT32 bk_addr;
	UINT32 free:1;
	UINT32 res:31;
}ge_bk_info_t;


typedef struct 
{
	UINT8 *frm_addr;
	UINT8 *block_addr;
	UINT32 block_idx;
	ge_rect_t block_rect;
	ge_rect_t bk_dis_rect;
}ge_block_link_t;

typedef struct{
	union
	{
		UINT8*	addr;		// pointer to the associated surface memory
		UINT8*	top;              // pointer to luma(y) or chroma(c) top memory
	};
	
	union
	{
		UINT32	resved;    	
		UINT8*	bottom;      // pointer to luma(y) or chroma(c) bottom memory
	};
	
	enum GE_PIXEL_FORMAT	pixel_fmt;
	UINT16  pitch;
			
	ge_rect_t rect;
}ge_region_pars_t;

typedef struct 
{
	UINT8*   gma_head;
	BOOL 	valid;
	ge_rect_t  rect;
	union
	{
		UINT8*	addr;		// pointer to the associated surface memory
		UINT8*	top;              // pointer to luma(y) or chroma(c) top memory
	};
	
	union
	{
		UINT32	resved;    	
		UINT8*	bottom;      // pointer to luma(y) or chroma(c) bottom memory
	};
	enum GE_PIXEL_FORMAT	pixel_fmt;
	UINT16  pitch;
	UINT8 	glob_alpha_en;
	UINT8 	glob_alpha;         
	UINT8 	pallet_seg;
	UINT8    pixel_bits;
	union
	{
		ge_rect_t dst_rect_29e; // add it for M3329E osd register setting
		ge_rect_t deo_dis_rect; // add it for M3329E osd register setting
	};

	UINT32 bk_buf_index;

	ge_block_link_t block_link[SURFACE_MAX_REGION];
	UINT32 block_num;
}ge_region_t;

typedef struct
{
	UINT32 start;
	UINT32 end;
	INT32 len;
}ge_line_info_t;

typedef struct 
{
	UINT32 buf_start;
	UINT32 pitch;

	// free lines
	UINT32 free_num;
	ge_line_info_t free[SURFACE_MAX_BLOCKS];
}ge_surf_buf_info_t;

typedef struct 
{
	UINT8 enable;
	UINT8 layer;
	UINT8 no_temp_buf; 
	UINT8 reserved;
}ge_clut_ycbcr_out_t;

typedef struct 
{
	UINT8 layer;  //ge display layer number
	UINT8 region_id;
	UINT16 reserved;
	UINT32 disp_addr; // to be dislplayed buffer address
}ge_disp_addr_cfg_t;

typedef struct 
{
	UINT16	width;		// width of input surface
	UINT16	height;		// height of surface to be created    
	INT32	pitch;		// distance to start of next line (return value only)
   	enum GE_PIXEL_ENDIAN	endian;      		// pixel endian(0: little endian, 1: big endian)
	enum GE_ALPHA_RANGE	alpha_range;	// alpha range: 0-127 or 0-255
//	UINT8     *base_addr;
	UINT32	fill_color;				// color to be filled
	UINT32	plane_mask_val;		// plane mask value
	UINT32	trig_num;				//expected video line number to trigger
	ge_YUV_RGB_conv_t	yuv2rgb;	 	// color space conversion information
	ge_YUV_RGB_conv_t	rgb2yuv;	 	// color space conversion information
	ge_col_rdc_t 	color_rdc;  	// information of color expansion or correction or reduction
	ge_resize_t		resize;			//  resize information
	ge_flick_filter_t	flick_filt;
	ge_color_key_t	color_key;		// color key value
	ge_alpha_blend_t	alpha_blend;	// alpha blending information
	enum GE_ROP_MODE	rop;		// ROP operations
	ge_line_draw_t	line_draw;
	enum GE_MB_FIELD mb_field;	
//#if (SYS_CHIP_MODULE==ALI_S3602)		
	enum GE_MB_MODE mb_mode;	
//#endif
	ge_pallet_t     pallet;	// pallette information
	
       BOOL  do_clip;
	ge_clip_t		clipper;		       // rectangle clipping information 
	
	ge_region_t region[SURFACE_MAX_REGION];
	UINT8 	surf_id;        //indicate HW or memory surface
	UINT8 	cur_region; //region id for current operation
	ge_bk_info_t bk_info[SURFACE_MAX_BLOCKS];

	ge_surf_buf_info_t buf_info;
}ge_surface_desc_t;

typedef struct
{
	ge_surface_desc_t *surf;
	enum GE_RSZ_MODE h_scale_mode; 
	enum GE_RSZ_MODE v_scale_mode; 
	UINT32 reserved;
}ge_scale_mode_cfg_t;

typedef struct 
{
	UINT32 par0;
	UINT32 par1;
	UINT32 par2;
	UINT32 par3;
}ge_io_cmd_par_t;

// added for the new ge module
#define NEW_GE_ARCH

/* GE Data Flow: SRC + PTN/PRIM + MSK --> DST

   Case 1(DST == SRC):   SRC + PTN/PRIM + MSK --> SRC, SRC mode will always be BITBLT
   Case 2(NO PTN & MSK): SRC --> DST, SRC mode could be BITBLT, Direct-Copy, Direct-Fill or Normal Color Fill.

   OSD model:    OSD + PTN/PRIM + MSK --> OSD
                 SRC == DST == OSD
*/

enum GE_DATA_PATH_TYPE
{
	GE_DST, // DST is always display buffer or one copy of display buffer.
	GE_SRC, // SRC could be same as DST, or it could be another bitmap or one single color.
	GE_PTN, // PTN is the data path for primitives(line, rectangle, filled rectangle, font, bitmap texture);
	GE_MSK, // MSK 1bpp is the mask data for some PTN (filled rectangle and bitmap texture) to change shape;
	        // MSK 8bpp is the mask data for PTN bitmap to do alpha-blending with/without a SRC bitmap;


	/* The following macros only for ge_set_xy(cmd_pos, data_path, x, y) and ge_set_wh(cmd_pos, data_path, w, h)
	For example: 
	    ge_set_xy(cmd_pos, GE_DST_SRC, x, y);
	same as:
	    ge_set_xy(cmd_pos, GE_DST, x, y);
	    ge_set_xy(cmd_pos, GE_SRC, x, y);
	*/

	//#define GE_DST_SRC  (GE_DST + (GE_SRC << 4))
	//#define GE_DST_PTN  (GE_DST + (GE_PTN << 4))
	//#define GE_DST_MSK  (GE_DST + (GE_MSK << 4))

	GE_DST_SRC = (GE_DST + (GE_SRC << 4)),
	GE_DST_PTN = (GE_DST + (GE_PTN << 4)),
	GE_DST_MSK = (GE_DST + (GE_MSK << 4)),
	GE_SRC_PTN = (GE_SRC + (GE_PTN << 4)),
	GE_SRC_MSK = (GE_SRC + (GE_MSK << 4)),
	GE_PTN_MSK = (GE_PTN + (GE_MSK << 4)),
};

// Primitive + PTN
enum GE_PRIMITIVE_MODE
{
	GE_PRIM_DISABLE,        // No Primitive and PTN, SRC mode is BITBLT by default
	GE_DRAW_RECT_FRAME,
	GE_DRAW_RECT_FRAME_FILL,
	GE_FILL_RECT_BACK_COLOR,
	GE_FILL_RECT_DRAW_COLOR,
	GE_DRAW_FONT,
	GE_DRAW_FONT_FILL_BACK_COLOR,
	GE_DRAW_BITMAP,
	GE_DRAW_BITMAP_ALPHA_BLENDING,

	// for points and lines, pen width is always 1
	GE_DRAW_POINT,          // Use primitive and draw color
	GE_DRAW_LINE,           // Use primitive and draw color
	GE_DIRECT_DRAW_POINT,   // Use SRC direct fill and back color
	GE_DIRECT_DRAW_LINE,    // Use SRC direct fill and back color
	GE_PRIM_SCALING,
};

enum GE_SRC_MODE
{
	GE_SRC_DISABLE,
	GE_SRC_BITBLT,
	GE_SRC_DIRECT_COPY,
	GE_SRC_DIRECT_FILL,
	GE_SRC_NORMAL_FILL,
};

enum GE_MSK_MODE
{
	GE_MSK_DISABLE,
	GE_MSK_ENABLE,
};

enum GE_GLOBAL_ALPHA_SEL
{
	GE_USE_GALPHA_MULTIPLY_PTN_ALPHA,
	GE_USE_GALPHA,
};

enum GE_ALPHA_OUT_MODE
{
    GE_OUTALPHA_FROM_BLENDING,
    GE_OUTALPHA_FROM_SRC,
    GE_OUTALPHA_FROM_PTN,
    GE_OUTALPHA_FROM_REG,
};

enum GE_BITMAP_ALPHA_MODE
{
    GE_BITMAP_PRE_MUL_ALPHA,
    GE_BITMAP_NON_PRE_MUL_ALPHA,
};

enum GE_GLOBAL_ALPHA_LAYER
{
    GE_GALPHA_LAYER_PTN,
    GE_GALPHA_LAYER_SRC,
};

enum GE_ALPHA_BLEND_MODE
{
    GE_ALPHA_BLEND_SRC_OVER,
    GE_ALPHA_BLEND_DST_OVER,
    GE_ALPHA_BLEND_SRC,
    GE_ALPHA_BLEND_DST,
    GE_ALPHA_BLEND_SRC_IN,
    GE_ALPHA_BLEND_DST_IN,
    GE_ALPHA_BLEND_SRC_OUT,
    GE_ALPHA_BLEND_DST_OUT,
    GE_ALPHA_BLEND_SRC_ATOP,
    GE_ALPHA_BLEND_DST_ATOP,
    GE_ALPHA_BLEND_XOR,
    GE_ALPHA_BLEND_CLEAR,
};

// When color-key enabled, PTN is the color-key source, SRC(==DST) is the color-key destination
enum GE_COLOR_KEY_MODE
{
	GE_CKEY_DISABLE,
	GE_CKEY_DST,            // specified color in SRC(==DST) will not be erased
	GE_CKEY_PTN_POST_CLUT,  // specified color in PTN will not be written to DST
	GE_CKEY_PTN_PRE_CLUT,   // specified color in PTN will not be written to DST
};

enum GE_COLOR_KEY_MATCH_MODE
{
	GE_CKEY_MATCH_ALWAYS,
	GE_CKEY_MATCH_IN_RANGE,     // LOW <= value <= HIGH
	GE_CKEY_MATCH_OUT_RANGE,    // value < LOW or value > HIGH
};

enum GE_COLOR_KEY_CHANNEL
{
	GE_CKEY_CHANNEL_R,
	GE_CKEY_CHANNEL_G,
	GE_CKEY_CHANNEL_B,
	GE_CKEY_CHANNEL_A,  // C3603 support alpha channel
};

/* define boolean operation mode */
enum GE_BOP_MODE
{
	GE_BOP_SETBLACK     =0x00,  // set data to 0 (black)
	GE_BOP_AND          =0x01,  // PTN AND SRC
	GE_BOP_ANDINV       =0x02,  // PTN AND (NOT SRC)
	GE_BOP_UPDATE       =0x03,  // copy PTN
	GE_BOP_INVAND       =0x04,  // (NOT PTN) AND SRC
	GE_BOP_REMAIN       =0x05,  // SRC (no update)
	GE_BOP_XOR          =0x06,  // PTN XOR SRC
	GE_BOP_OR           =0x07,  // PTN OR SRC
	GE_BOP_NOR          =0x08,  // NOT (PTN OR SRC)
	GE_BOP_INVXOR       =0x09,  // (NOT PTN) XOR SRC
	GE_BOP_INVERT       =0x0A,  // NOT SRC
	GE_BOP_ORINV        =0x0B,  // PTN OR (NOT SRC)
	GE_BOP_UPDATEINV    =0x0C,  // NOT PTN
	GE_BOP_INVOR        =0x0D,  // (NOT PTN) OR SRC
	GE_BOP_NAND         =0x0E,  // NOT (PTN AND SRC)
	GE_BOP_SETWHITE     =0x0F   // set data to 1 (white)
};

enum GE_RGB_ORDER
{
	GE_RGB_ORDER_ARGB, // RGB/ARGB
	GE_RGB_ORDER_ABGR, // BGR/ABGR
	GE_RGB_ORDER_RGBA,
	GE_RGB_ORDER_BGRA,

	GE_RGB_ORDER_AYCbCr = GE_RGB_ORDER_ARGB,
	GE_RGB_ORDER_ACrCbY = GE_RGB_ORDER_ABGR,
	GE_RGB_ORDER_YCbCrA = GE_RGB_ORDER_RGBA,
	GE_RGB_ORDER_CrCbYA = GE_RGB_ORDER_BGRA,
};

enum GE_RGB_COLOR_EXPANSION
{
	GE_RGB_EXPAN_MSB_TO_LSB,
	GE_RGB_EXPAN_PAD0_TO_LSB,
};

enum GE_ALPHA_POLARITY
{
	GE_ALPHA_POLARITY_0,    // 0 - transparent, 1 - not transparent
	GE_ALPHA_POLARITY_1,    // 0 - not transparent, 1 - transparent
};

enum GE_BYTE_ENDIAN
{
	GE_BYTE_ENDIAN_LITTLE,
	GE_BYTE_ENDIAN_BIG,
};

enum GE_SUBBYTE_ENDIAN
{
	GE_SUBBYTE_RIGHT_PIXEL_MSB, // most right pixel in most significant bits
	GE_SUBBYTE_RIGHT_PIXEL_LSB, // most right pixel in least significant bits
};

enum GE_FONT_DATA_FORMAT
{
	GE_FONT_DATA_BYTE,          // font data in byte aligned
	GE_FONT_DATA_DWORD,         // font data in dword aligned
};

enum GE_CLIP_MODE
{
	GE_CLIP_DISABLE,
	GE_CLIP_INSIDE,
	GE_CLIP_OUTSIDE,
};

#define GE_FONT_STRETCH_DISABLE     0
#define GE_FONT_STRETCH_ENABLE      1   // pixel replicate to 2 multiple size output

/* define color conversion system */
enum GE_COLOR_CVT_SYSTEM
{
	GE_COLOR_CVT_BT601,  		//ITU-R BT.601 
	GE_COLOR_CVT_BT709		//ITU-R BT.709
};

/* define color conversion matrix */
enum GE_COLOR_SPACE_MATRIX
{
	GE_COLOR_SPACE_GRAPHIC_MATRIX,
	GE_COLOR_SPACE_VIDEO_MATRIX,
};

enum GE_CLUT_MODE
{
	GE_CLUT_DISABLE,
	GE_CLUT_COLOR_EXPANSION,
	GE_CLUT_COLOR_CORRECTION,
	GE_CLUT_COLOR_REDUCTION   // not support
};

// GE scan order
#define GE_SCAN_TOP_TO_BOTTOM    0
#define GE_SCAN_BOTTOM_TO_TOP    1
#define GE_SCAN_LEFT_TO_RIGHT    0
#define GE_SCAN_RIGHT_TO_LEFT    1

enum GE_BASE_ADDR_SEL
{
	GE_BASE_ADDR,
	GE_BASE_ADDR1,
	GE_BASE_ADDR2,
	GE_BASE_ADDR3,
	GE_BASE_ADDR4,
	GE_BASE_ADDR5,
	GE_BASE_ADDR6,
	GE_BASE_ADDR7,
	GE_BASE_ADDR8,
	GE_BASE_ADDR9,
	GE_BASE_ADDR10,
	GE_BASE_ADDR11,
	GE_BASE_ADDR12,
};

enum GE_DATA_DECODER
{
	GE_DECODER_DISABLE,
	GE_DECODER_RLE,
};

#define GE_BA_FLAG_ALL      0x0f    // modify the whole struct ge_base_addr_t

#define GE_BA_FLAG_ADDR     0x01    // modify base_address + data_decoder
#define GE_BA_FLAG_SEL      0x02    // modify base_addr_sel only
#define GE_BA_FLAG_PITCH    0x04    // modify pixel_pitch only
#define GE_BA_FLAG_FORMAT   0x08    // modify color_format only

typedef struct ge_base_addr_t
{
	UINT16                modify_flags;
	UINT16                pixel_pitch;
	UINT32                base_address;
	enum GE_BASE_ADDR_SEL base_addr_sel;
	enum GE_PIXEL_FORMAT  color_format;
	enum GE_DATA_DECODER  data_decoder;  // only can be enabled for PTN or MSK
}ge_base_addr_t; 
typedef struct ge_base_addr_t ge_operating_entity;

enum GE_START_MODE
{
	GE_IO_START_MODE,   // IO start
	GE_CMD_START_MODE,  // Command Q
};

enum GE_CMD_LIST_MODE
{
	GE_COMPILE,
	GE_COMPILE_AND_EXECUTE,
};

enum GE_CMD_SYNC_MODE
{
	GE_SYNC_MODE_INTERRUPT, // default use interrupt sync mode, better efficiency
	GE_SYNC_MODE_POOLING,   // if task dispatching off, please change to use pooling mode.
};

#define GE_IO_REG       0   // Special ge_cmd_list_hdl for IO start mode

// GE api based on cmd list
#define GE_MAX_CMD_LIST_NUM     0xff
#define GE_MAX_CMD_NUM          0xffff
#define GE_INVALID_CMD          0xffff

typedef UINT32 ge_cmd_list_hdl;

#if 0
typedef struct _ge_rect_t
{
    union
    {
        UINT16 uLeft;
        UINT16 left;
    };
    union
    {
        UINT16 uTop;
        UINT16 top;
    };
    union
    {
        UINT16 uWidth;
        UINT16 width;
    };
    union
    {
        UINT16 uHeight;
        UINT16 height;
    };
}ge_rect_t, *pge_rect_t;
#endif

typedef struct _ge_pal_attr_t
{
	UINT8 pal_type;         // GE_PAL_RGB or GE_PAL_YCBCR
	UINT8 rgb_order;        // enum GE_RGB_ORDER
	UINT8 alpha_range;      // enum GE_ALPHA_RANGE
	UINT8 alpha_pol;        // enum GE_ALPHA_POLARITY
} ge_pal_attr_t, *pge_pal_attr_t;
typedef const ge_pal_attr_t *pcge_pal_attr_t;

typedef const ge_rect_t *pcge_rect_t;


// added for new gma module
// gma hw layer ID
#define GMA_HW_LAYER0       0
#define GMA_HW_LAYER1       1
#define GMA_HW_LAYER2       2

// pallette type
#define	GE_PAL_RGB              0x00
#define	GE_PAL_YCBCR            0x01

// scale mode
#define GE_SCALE_DUPLICATE      0
#define GE_SCALE_FILTER         1

// Begin ge_gma_scale(dev, layer_id, scale_cmd, scale_param)
#define GE_VSCALE_OFF           0x01 // scale_param: enum TVSystem
#define GE_VSCALE_TTX_SUBT      0x02 // scale_param: enum TVSystem

#define GE_H_DUPLICATE_ON_OFF   0x03 // scale_param: TRUE or FALSE
#define GE_SET_SCALE_MODE       0x04 // GE_SCALE_FILTER or GE_SCALE_DUPLICATE

#define GE_SET_SCALE_PARAM      0x05 // Set arbitrary scale param. see struct gma_scale_param_t
#define GE_GET_SCALE_PARAM      0x06 // Get scale param. see struct ge_scale_param_t

typedef struct _ge_scale_param_t
{
	UINT16 tv_sys;       // enum TVSystem
	UINT16 h_div;
	UINT16 v_div;
	UINT16 h_mul;
	UINT16 v_mul;
} ge_scale_param_t, *pge_scale_param_t;
typedef const ge_scale_param_t *pcge_scale_param_t;
// End ge_gma_scale(dev, layer_id, scale_cmd, scale_param)

typedef struct 
{
	BOOL blend_info_en;
	UINT32 blend_info_addr;
	BOOL color_premuled; // 0: color is not alpha pre-multiplied, 1: color is alpha pre-multiplied
} ge_scale_info_t, *pge_scale_info_t;

// for create/move/get region
typedef struct _ge_gma_region_t
{
	UINT8   color_format;   // enum GE_PIXEL_FORMAT
	UINT8   galpha_enable;  // 0 - use color by color alpha; 1 - enable global alpha for this region
	UINT8   global_alpha;   // If global alpha enable, please set global_alpha [0x00, 0xff]
	UINT8   pallette_sel;   // pallette index for CLUT4

	UINT16  region_x;       // x offset of the region, from screen top_left pixel
	UINT16  region_y;       // y offset of the region, from screen top_left pixel
	UINT16  region_w;
	UINT16  region_h;

	UINT32  bitmap_addr;    // 0 - use uMemBase(internal memory) which is set by ge_attach(ge_layer_config_t *);
	                        // bitmap_addr not 0 - use external memory address as region bitmap addr
	UINT32  pixel_pitch;    // pixel pitch(not byte pitch) for internal memory or bitmap_addr

	                        // ge_create_region(): bitmap_addr and pixel_pitch determines the region bitmap address, total 4 cases:
	                        // Case 1: if bitmap_addr is 0, and pixel_pitch is 0, it will use region_w as pixel_pitch,
	                        //     and region bitmap addr will be allocated from uMemBase dynamically.
	                        // Case 2: if bitmap_addr is 0, and pixel_pitch is not 0, the region bitmap addr will be fixed:
	                        //     uMemBase + (pixel_pitch * bitmap_y + bitmap_x) * byte_per_pixel

	                        // Case 3: if bitmap_addr is not 0, and pixel_pitch is 0, the region bitmap addr will be:
	                        //     bitmap_addr + (bitmap_w * bitmap_y + bitmap_x) * byte_per_pixel
	                        // Case 4: if bitmap_addr is not 0, and pixel_pitch is not 0, the region bitmap addr will be:
	                        //     bitmap_addr + (pixel_pitch * bitmap_y + bitmap_x) * byte_per_pixel

	                        // ge_move_region(): region using internal memory can only change region_x, region_y, pal_sel;
	                        // ge_move_region(): region using external memory can change everyting in ge_region_t;

	UINT32  bitmap_x;       // x offset from the top_left pixel in bitmap_addr or internal memory
	UINT32  bitmap_y;       // y offset from the top_left pixel in bitmap_addr or internal memory
	UINT32  bitmap_w;       // bitmap_w must >= bitmap_x + region_w, both for internal memory or external memory
	UINT32  bitmap_h;       // bitmap_h must >= bitmap_y + region_h, both for internal memory or external memory
} ge_gma_region_t, *pge_gma_region_t;
typedef const ge_gma_region_t *pcge_region_t;

// ge_driver_config_t for ge_attach(dev, ge_driver_config_t *, layer_number);
typedef struct _ge_layer_config_t
{
	UINT32 mem_base;
	UINT32 mem_size;
	UINT8  hw_layer_id;         // application can switch hw layer id
	UINT8  color_format;        // default region color format, enum GE_PIXEL_FORMAT
	UINT16 width, height;       // default region width and height
	UINT16 pixel_pitch;         // default region pixel pitch

	UINT32 bScaleFilterEnable       :1;  // enable/disable GMA scale filter
	UINT32 bP2NScaleInNormalPlay    :1;  // enable/disable PAL/NTSC scale in normal play mode
	UINT32 bP2NScaleInSubtitlePlay  :1;	 // enable/disable PAL/NTSC scale in subtitle play mode
	UINT32 bDirectDraw              :1;  // for CPU direct draw, no GE draw
	UINT32 bCacheable               :1;  // for CPU direct draw, no GE draw
	UINT32 reserved                 :29; // reserved for future use
} ge_layer_config_t, *pge_layer_config_t;

#define GE_MAX_LAYER_NUM		2

typedef struct
{
	int layer_num;
	int dis_layer[GE_MAX_LAYER_NUM];
}ge_layer_dis_seq_t, *pge_layer_dis_seq_t;

struct ge_device
{
	struct ge_device  *next;  /*next device */
       INT32  type;
	INT8  name[HLD_MAX_NAME_SIZE];
	INT32  flags;

	void *priv;		/* Used to be 'private' but that upsets C++ */
	
	// common ge api
	RET_CODE (*attach) ();
	RET_CODE (*detach) (struct ge_device *);
	RET_CODE (*open)(struct ge_device *);
	RET_CODE (*close) (struct ge_device*);
	RET_CODE (*ioctl) (struct ge_device*,UINT32,UINT32); 

	RET_CODE (*show_onoff)(ge_surface_desc_t *, UINT8 );
	RET_CODE (*region_show)(struct ge_device*, ge_surface_desc_t *, UINT8 ,BOOL);
	RET_CODE (*set_pallette)(struct ge_device*,ge_surface_desc_t *,UINT8 *,UINT16 , UINT8,enum GE_PALLET_ALPHA_LEVEL);
	RET_CODE (*get_pallette)(ge_surface_desc_t *, UINT8 *,UINT16 , UINT8 type);
	RET_CODE (*modify_pallette)(struct ge_device*, ge_surface_desc_t *, UINT8 ,UINT8 ,UINT8 ,UINT8 ,UINT32 ,enum GE_PALLET_ALPHA_LEVEL);

	// old ge api
 	ge_surface_desc_t  * ((*create_surface)(struct ge_device *, ge_surface_desc_t *, UINT32, UINT8));		
 	RET_CODE (*release_surface)(struct ge_device *, ge_surface_desc_t *);	
	
	RET_CODE (*create_region)(struct ge_device *,ge_surface_desc_t *, UINT8 ,ge_rect_t *,ge_region_t *);
	RET_CODE (*delete_region)(struct ge_device *, ge_surface_desc_t *, UINT8 );
	
	RET_CODE (*set_region_pos)(ge_surface_desc_t *, UINT8 ,ge_rect_t* );
	RET_CODE (*get_region_pos)(ge_surface_desc_t *, UINT8 ,ge_rect_t* );

	RET_CODE (*blt)( struct ge_device*, ge_surface_desc_t *,   ge_surface_desc_t *,  ge_rect_t *, ge_rect_t *, UINT32 );
	RET_CODE (*blt_ex)( struct ge_device*, ge_surface_desc_t *,   ge_surface_desc_t *,  ge_surface_desc_t *, ge_rect_t *, ge_rect_t *, ge_rect_t *, UINT32 );

/* new added */
	RET_CODE (*fill_rect)( struct ge_device*, ge_surface_desc_t *,  ge_rect_t *, UINT32);
	RET_CODE (*draw_pixel)(struct ge_device*, ge_surface_desc_t *, int, int, UINT32);
	UINT32      (*read_pixel)(struct ge_device*, ge_surface_desc_t *, int, int);
	RET_CODE (*draw_hor_line)(struct ge_device*, ge_surface_desc_t *, int, int, int, UINT32);
	RET_CODE (*draw_ver_line)(struct ge_device*, ge_surface_desc_t *, int, int, int, UINT32);
/* add end*/

	RET_CODE (*set_clip)(ge_surface_desc_t *, ge_clip_t* );
	RET_CODE (*disable_clip)(ge_surface_desc_t * );	

	RET_CODE (*set_rop)( ge_surface_desc_t * , UINT32 );
	RET_CODE (*get_rop)( ge_surface_desc_t * , UINT32 *);
	RET_CODE (*set_colorkey)( ge_surface_desc_t * , ge_color_key_t *);
	RET_CODE (*get_colorkey)( ge_surface_desc_t * , ge_color_key_t *);
	RET_CODE (*set_alpha)( ge_surface_desc_t * , ge_alpha_blend_t *);
	RET_CODE (*get_alpha)( ge_surface_desc_t * , ge_alpha_blend_t *);
	RET_CODE (*set_colorredc)( ge_surface_desc_t * , ge_col_rdc_t*);
	RET_CODE (*get_colorredc)( ge_surface_desc_t * , ge_col_rdc_t *);
	RET_CODE (*set_resize)( ge_surface_desc_t * , ge_resize_t *);
	RET_CODE (*get_resize)( ge_surface_desc_t * , ge_resize_t *);
	RET_CODE (*set_flickfilt)( ge_surface_desc_t * , ge_flick_filter_t *);
	RET_CODE (*get_flickfilt)( ge_surface_desc_t * , ge_flick_filter_t *);
	RET_CODE (*set_planmask)( ge_surface_desc_t * , UINT32);
	RET_CODE (*get_planmask)( ge_surface_desc_t * , UINT32 *);
	RET_CODE (*set_yuv2rgb)( ge_surface_desc_t * , ge_YUV_RGB_conv_t *);
	RET_CODE (*get_yuv2rgb)( ge_surface_desc_t * , ge_YUV_RGB_conv_t *);
	RET_CODE (*set_rgb2yuv)( ge_surface_desc_t * , ge_YUV_RGB_conv_t *);
	RET_CODE (*get_rgb2yuv)( ge_surface_desc_t * , ge_YUV_RGB_conv_t *);

	// new ge interface
	ge_cmd_list_hdl (*cmd_list_create)(struct ge_device * , UINT32);
	RET_CODE (*cmd_list_destroy)(struct ge_device * , ge_cmd_list_hdl);
	BOOL (*cmd_list_is_valid)(struct ge_device * , ge_cmd_list_hdl);

	UINT32 (*cmd_list_get_base)(struct ge_device * , ge_cmd_list_hdl);

	RET_CODE (*cmd_list_new)(struct ge_device * , ge_cmd_list_hdl, enum GE_CMD_LIST_MODE);
	RET_CODE (*cmd_list_end)(struct ge_device * , ge_cmd_list_hdl);

	RET_CODE (*cmd_list_set_param)(struct ge_device * , ge_cmd_list_hdl, UINT32);
	UINT32 (*cmd_list_get_param)(struct ge_device * , ge_cmd_list_hdl);

	RET_CODE (*cmd_list_start)(struct ge_device * , ge_cmd_list_hdl, BOOL, BOOL);
	RET_CODE (*cmd_list_stop)(struct ge_device * , ge_cmd_list_hdl, BOOL, BOOL);

	UINT32 (*cmd_begin)(struct ge_device * , ge_cmd_list_hdl, enum GE_PRIMITIVE_MODE);
	RET_CODE (*cmd_end)(struct ge_device * , UINT32);

	RET_CODE (*set_operating_entity)(struct ge_device * , UINT32 , enum GE_DATA_PATH_TYPE, const ge_operating_entity *);
	RET_CODE (*get_operating_entity)(struct ge_device * , UINT32 , enum GE_DATA_PATH_TYPE, ge_operating_entity *);

	RET_CODE (*set_src_mode)(struct ge_device * , UINT32 , enum GE_SRC_MODE );
	RET_CODE (*set_msk_mode)(struct ge_device * , UINT32 , enum GE_MSK_MODE );
	RET_CODE (*get_primitive_mode)(struct ge_device * , UINT32 , enum GE_PRIMITIVE_MODE *);
	RET_CODE (*get_src_mode)(struct ge_device * , UINT32 , enum GE_SRC_MODE *);
	RET_CODE (*get_msk_mode)(struct ge_device * , UINT32 , enum GE_MSK_MODE *);

	RET_CODE (*set_xy)(struct ge_device * , UINT32 , enum GE_DATA_PATH_TYPE, INT32 , INT32 );
	RET_CODE (*set_wh)(struct ge_device * , UINT32 , enum GE_DATA_PATH_TYPE, INT32 , INT32 );
	RET_CODE (*get_xy)(struct ge_device * , UINT32, enum GE_DATA_PATH_TYPE, INT32 *, INT32 *);
	RET_CODE (*get_wh)(struct ge_device * , UINT32 , enum GE_DATA_PATH_TYPE, INT32 *, INT32 *);

	RET_CODE (*set_xy2)(struct ge_device * , UINT32 , enum GE_DATA_PATH_TYPE, INT32 , INT32 , INT32, INT32);
	RET_CODE (*get_xy2)(struct ge_device * , UINT32 , enum GE_DATA_PATH_TYPE, INT32 *, INT32 *, INT32 *, INT32 *);

	RET_CODE (*set_back_color)(struct ge_device * , UINT32 , UINT32 );
	RET_CODE (*set_font_color)(struct ge_device * , UINT32 , UINT32 );
	RET_CODE (*set_draw_color)(struct ge_device * , UINT32 , UINT32 );
	RET_CODE (*get_back_color)(struct ge_device * , UINT32 , UINT32 *);
	RET_CODE (*get_font_color)(struct ge_device * , UINT32 , UINT32 *);
	RET_CODE (*get_draw_color)(struct ge_device * , UINT32 , UINT32 *);
	RET_CODE (*set_color_format)(struct ge_device * , UINT32 , enum GE_PIXEL_FORMAT);

	RET_CODE (*set_rop_mode)(struct ge_device * , UINT32 , enum GE_ROP_MODE);
	RET_CODE (*set_bop_mode)(struct ge_device * , UINT32 , enum GE_BOP_MODE);
	RET_CODE (*get_rop_mode)(struct ge_device * , UINT32 , enum GE_ROP_MODE*);
	RET_CODE (*get_bop_mode)(struct ge_device * , UINT32 , enum GE_BOP_MODE *);

	RET_CODE (*set_global_alpha)(struct ge_device * , UINT32 , UINT32 );
	RET_CODE (*set_global_alpha_mode)(struct ge_device * , UINT32 , UINT32 );
	RET_CODE (*set_global_alpha_sel)(struct ge_device * , UINT32 , enum GE_GLOBAL_ALPHA_SEL);
	RET_CODE (*get_global_alpha)(struct ge_device * , UINT32 , UINT32 *);
	RET_CODE (*get_global_alpha_mode)(struct ge_device * , UINT32 , UINT32 *);
	RET_CODE (*get_global_alpha_sel)(struct ge_device * , UINT32 , enum GE_GLOBAL_ALPHA_SEL *);

	RET_CODE (*set_colorkey_mode)(struct ge_device * , UINT32 , enum GE_COLOR_KEY_MODE);
	RET_CODE (*set_colorkey_match_mode)(struct ge_device * , UINT32 , enum GE_COLOR_KEY_CHANNEL, enum GE_COLOR_KEY_MATCH_MODE);
	RET_CODE (*set_colorkey_range)(struct ge_device * , UINT32 , UINT32 , UINT32 );
	RET_CODE (*get_colorkey_mode)(struct ge_device * , UINT32 , enum GE_COLOR_KEY_MODE *);
	RET_CODE (*get_colorkey_match_mode)(struct ge_device * , UINT32 , enum GE_COLOR_KEY_CHANNEL, enum GE_COLOR_KEY_MATCH_MODE *);
	RET_CODE (*get_colorkey_range)(struct ge_device * , UINT32 , UINT32 *, UINT32 *);


	RET_CODE (*set_scan_order)(struct ge_device * , UINT32 , enum GE_DATA_PATH_TYPE, UINT32 , UINT32 );
	RET_CODE (*set_byte_endian)(struct ge_device * , UINT32 , enum GE_DATA_PATH_TYPE, enum GE_BYTE_ENDIAN );
	RET_CODE (*set_subbyte_endian)(struct ge_device * , UINT32 , enum GE_DATA_PATH_TYPE, enum GE_SUBBYTE_ENDIAN );
	RET_CODE (*set_rgb_order)(struct ge_device * , UINT32 , enum GE_DATA_PATH_TYPE, enum GE_RGB_ORDER );
	RET_CODE (*set_alpha_attr)(struct ge_device * , UINT32 , enum GE_DATA_PATH_TYPE, enum GE_ALPHA_RANGE , enum GE_ALPHA_POLARITY);
	RET_CODE (*set_rgb_expansion)(struct ge_device * , UINT32 , enum GE_DATA_PATH_TYPE, enum GE_RGB_COLOR_EXPANSION );

	RET_CODE (*set_font_data_format)(struct ge_device * , UINT32 , enum GE_FONT_DATA_FORMAT );
	RET_CODE (*set_font_stretch)(struct ge_device * , UINT32 , UINT32 , UINT32 );

	RET_CODE (*set_clut_mode)(struct ge_device * , UINT32 , enum GE_CLUT_MODE, BOOL );

	RET_CODE (*set_clut_addr)(struct ge_device * , UINT32 , UINT32 );

	RET_CODE (*set_clut_rgb_order)(struct ge_device * , UINT32 , enum GE_RGB_ORDER );
	RET_CODE (*set_clut_update)(struct ge_device * , UINT32 , BOOL );
	RET_CODE (*set_clut_color_cvt)(struct ge_device * , UINT32 , enum GE_COLOR_CVT_SYSTEM , enum GE_COLOR_SPACE_MATRIX );

	RET_CODE (*set_clip_mode)(struct ge_device * , UINT32 , enum GE_CLIP_MODE);
	RET_CODE (*set_clip_rect)(struct ge_device * , UINT32 , UINT32 , UINT32 , UINT32 , UINT32 );

	RET_CODE (*enable_bitmask)(struct ge_device * , UINT32 , BOOL );
	RET_CODE (*set_bitmask)(struct ge_device * , UINT32 , UINT32 );

	RET_CODE (*enable_dither)(struct ge_device * , UINT32 , BOOL );

	UINT32 (*cmd_list_get_num)(struct ge_device * , ge_cmd_list_hdl );
	UINT32 (*cmd_list_get_first)(struct ge_device * , ge_cmd_list_hdl );
	UINT32 (*cmd_list_get_last)(struct ge_device * , ge_cmd_list_hdl );
	UINT32 (*cmd_list_get_next)(struct ge_device * , UINT32 );
	UINT32 (*cmd_list_get_prev)(struct ge_device * , UINT32 );

	// Begin GMA interfaces
	RET_CODE (*gma_show_onoff)(UINT32, BOOL);
	RET_CODE (*gma_scale)(UINT32, UINT32, UINT32);

	RET_CODE (*gma_set_pallette)(UINT32, const UINT8 *, UINT16, const ge_pal_attr_t *);
	RET_CODE (*gma_get_pallette)(UINT32, UINT8 *,       UINT16, const ge_pal_attr_t *);
	RET_CODE (*gma_modify_pallette)(UINT32, UINT8 , UINT8, UINT8, UINT8, UINT8, const ge_pal_attr_t *);

	RET_CODE (*gma_create_region)(UINT32, UINT32, const ge_gma_region_t *);
	RET_CODE (*gma_delete_region)(UINT32, UINT32);

	RET_CODE (*gma_move_region)(UINT32, UINT32, const ge_gma_region_t *);
	RET_CODE (*gma_show_region)(UINT32, UINT32, BOOL);
	RET_CODE (*gma_get_region_info) (UINT32, UINT32, ge_gma_region_t *);

	RET_CODE (*gma_set_region_to_cmd_list)(UINT32, UINT32, ge_cmd_list_hdl);

	RET_CODE (*gma_set_clip_mode)(UINT32, enum GE_CLIP_MODE);
	RET_CODE (*gma_set_clip_rect)(UINT32, UINT32, UINT32, UINT32, UINT32);
	// End GMA interfaces

	/*====Add for M3602 dual output====*/
	RET_CODE (*ioctl_deo)(ge_surface_desc_t *, UINT32, UINT32);
	RET_CODE (*show_onoff_deo)(ge_surface_desc_t *, UINT8 );
	RET_CODE (*region_show_deo)(struct ge_device*, ge_surface_desc_t *, UINT8 ,BOOL);
	RET_CODE (*set_pallette_deo)(struct ge_device*,ge_surface_desc_t *,UINT8 *,UINT16 , UINT8,enum GE_PALLET_ALPHA_LEVEL);
	RET_CODE (*get_pallette_deo)(ge_surface_desc_t *, UINT8 *,UINT16 , UINT8 type);
	RET_CODE (*modify_pallette_deo)(struct ge_device*, ge_surface_desc_t *, UINT8 ,UINT8 ,UINT8 ,UINT8 ,UINT32 ,enum GE_PALLET_ALPHA_LEVEL);
 	ge_surface_desc_t  * ((*create_surface_deo)(struct ge_device *, ge_surface_desc_t *, UINT32, UINT8));		
 	RET_CODE (*release_surface_deo)(struct ge_device *, ge_surface_desc_t *);	
	
	RET_CODE (*create_region_deo)(struct ge_device *,ge_surface_desc_t *, UINT8 ,ge_rect_t *,ge_region_t *);
	RET_CODE (*delete_region_deo)(struct ge_device *, ge_surface_desc_t *, UINT8 );
	
	RET_CODE (*set_region_pos_deo)(ge_surface_desc_t *, UINT8 ,ge_rect_t* );
	RET_CODE (*get_region_pos_deo)(ge_surface_desc_t *, UINT8 ,ge_rect_t* );
	RET_CODE (*set_clip_deo)(ge_surface_desc_t *, ge_clip_t* );
	RET_CODE (*disable_clip_deo)(ge_surface_desc_t * );	
	/*====Add for M3602 dual output====*/

    /* added for M3603/M3606 */
	RET_CODE (*set_scale_info)(struct ge_device * , UINT32 , ge_scale_info_t*);	
	RET_CODE (*ioctl_ext) (struct ge_device *, UINT32, UINT32, UINT32);
	RET_CODE (*gma_close)(UINT32);
	RET_CODE (*set_alpha_out_mode)(struct ge_device * , UINT32 , enum GE_ALPHA_OUT_MODE);
	RET_CODE (*set_dst_alpha_out)(struct ge_device * , UINT32 , UINT32);
    RET_CODE (*set_global_alpha_layer)(struct ge_device *, UINT32 , enum GE_GLOBAL_ALPHA_LAYER);
    RET_CODE (*set_bitmap_alpha_mode)(struct ge_device *, UINT32 , enum GE_DATA_PATH_TYPE , enum GE_BITMAP_ALPHA_MODE);
    RET_CODE (*get_alpha_out_mode)(struct ge_device * , UINT32 , enum GE_ALPHA_OUT_MODE *);
    RET_CODE (*get_dst_alpha_out)(struct ge_device * , UINT32 , UINT32 *);
    /* added for M3603/M3606 */

};

#ifdef __cplusplus
extern "C"
{
#endif

#include "ge_old.h"
#include "ge_new.h"

RET_CODE ge_detach(struct ge_device *dev);

/*
   Open graphic engine device
*/
RET_CODE ge_open (struct ge_device *dev);
/*
   Close graphic engine device 
*/
RET_CODE ge_close(struct ge_device *dev);


#ifdef __cplusplus
}
#endif

#endif

