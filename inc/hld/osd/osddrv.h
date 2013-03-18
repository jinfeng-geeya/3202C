#ifndef	_OSDDRV_H_
#define	_OSDDRV_H_

#include <basic_types.h>
#include <hld/hld_dev.h>
#include "osddrv_dev.h"

/**************************************************************
				The parameter definition of OSD Driver API function
***************************************************************/
#if (SYS_CHIP_MODULE==ALI_M3327C || SYS_PROJECT_FE == PROJECT_FE_ATSC)// ||SYS_CHIP_MODULE==ALI_S3601 || SYS_CHIP_MODULE == ALI_S3602)
#define OSD_BLOCKLINK_SUPPORT		
#endif


#define	OSDDRV_RGB				0x00    // ARGB   byte order is: {B, G , R , A}, A in [0, 255]
#define	OSDDRV_YCBCR			0x01    // AYCbCr byte order is: {Y, Cb, Cr, A}, A in [0, 15 ]

#define 	OSD_BYTE_ALIGNED		
#define 	OSD_VSCR_STRIDE			0
#define   OSD_CACHE					1

#define	OSDDRV_OFF				0x00
#define	OSDDRV_ON				0x01


/**************** the position and size of OSD frame buffer********************/
#define	OSD_MIN_TV_X		0x20
#define	OSD_MIN_TV_Y		0x10
#define	OSD_MAX_TV_WIDTH	0x2B0
#define	OSD_MAX_TV_HEIGHT	0x1F0 //old value 0x1E0


#define P2N_SCALE_DOWN  	0x00
#define N2P_SCALE_UP		0x01
#define OSD_SCALE_MODE		P2N_SCALE_DOWN


//#define P2N_SCALE_IN_NORMAL_PLAY
#define P2N_SCALE_IN_SUBTITLE_PLAY

// scaler
#define OSD_VSCALE_OFF          0x00
#define OSD_VSCALE_TTX_SUBT     0x01
#define OSD_VSCALE_GAME         0x02
#define OSD_VSCALE_DVIEW        0x03
#define OSD_HDUPLICATE_ON       0x04
#define OSD_HDUPLICATE_OFF      0x05
#define OSD_OUTPUT_1080         0x06 // 720x576(720x480)->1920x1080
#define OSD_OUTPUT_720          0x07 // 720x576(720x480)->1280x720
#define OSD_HDVSCALE_OFF        0x08 // 1280x720->720x576(720x480)
#define OSD_HDOUTPUT_1080       0x09 // 1280x720->1920x1080
#define OSD_HDOUTPUT_720        0x0A // 1280x720->1280x720
#define OSD_SET_SCALE_MODE      0x0B // filter mode or duplicate mode
#define OSD_SCALE_WITH_PARAM    0x0C // Suitable for any case. see struct osd_scale_param
#define OSD_VSCALE_CC_SUBT	0X0D // ATSC CC for HD output scale

#define OSD_SCALE_WITH_PARAM_DEO    0x1000 // set scale parameter for sd output when dual output is enabled. see struct osd_scale_param

#define   OSD_SOURCE_PAL        0
#define   OSD_SOURCE_NTSC       1

#define   OSD_SCALE_DUPLICATE   0
#define   OSD_SCALE_FILTER      1


//io_control
#define	OSD_IO_UPDATE_PALLETTE	0x00
#define OSD_IO_ADJUST_MEMORY    0x01
#define OSD_IO_SET_VFILTER      0x02
#define OSD_IO_RESPOND_API	 0X03
#define OSD_IO_DIS_STATE	 0X04
#define OSD_IO_SET_BUF_CACHEABLE 0X05
#define OSD_IO_16M_MODE			0X06
#define OSD_IO_SET_TRANS_COLOR  0x07
#define OSD_IO_SET_ANTI_FLICK_THRE  0x0F
#define   OSD_IO_ENABLE_ANTIFLICK		0x10
#define   OSD_IO_DISABLE_ANTIFLICK		0x11

#define   OSD_IO_SWITCH_DEO_LAYER		0x12
#define   OSD_IO_SET_DEO_AUTO_SWITCH	0x13

#define   OSD_IO_GET_RESIZE_PARAMATER   0x14
#define   OSD_IO_SET_RESIZE_PARAMATER   0x15


#define OSD_IO_ELEPHANT_BASE 0x10000
#define OSD_IO_SWITH_DATA_TRANSFER_MODE (OSD_IO_ELEPHANT_BASE + 0x01)
#define OSD_IO_SET_ANTIFLK_PARA         (OSD_IO_ELEPHANT_BASE + 0x02)

#define OSD_IO_SET_GLOBAL_ALPHA         (OSD_IO_ELEPHANT_BASE + 0x03) /* dwParam [0x00, 0xff] */

#define OSD_IO_GET_ON_OFF               (OSD_IO_ELEPHANT_BASE + 0x04) /* OSD layer show or hide(dwParam is UINT32 *) */
#define OSD_IO_SET_AUTO_CLEAR_REGION    (OSD_IO_ELEPHANT_BASE + 0x05) /* Enable/Disable filling transparent color in OSDDrv_CreateRegion().
                                                                         After Open(), default is TRUE. Set it before OSDDrv_CreateRegion().*/
/* Enable/Disable GE ouput YCBCR format to DE when source is CLUT8, clut8->ycbcr, not do color reduction
     only used when output is 576i/p or 480i/p*/
#define OSD_IO_SET_YCBCR_OUTPUT    (OSD_IO_ELEPHANT_BASE + 0x06) 

#define OSD_IO_SET_DISPLAY_ADDR    (OSD_IO_ELEPHANT_BASE + 0x07) 
#define OSD_IO_SET_MAX_PIXEL_PITCH	   (OSD_IO_ELEPHANT_BASE + 0x08)
#define OSD_IO_WRITE2_SUPPORT_HD_OSD   (OSD_IO_ELEPHANT_BASE + 0x09)
#define OSD_IO_SUBT_RESOLUTION     (OSD_IO_ELEPHANT_BASE + 0x0A)
#define OSD_IO_CREATE_REGION            (OSD_IO_ELEPHANT_BASE + 0x0b) /* Create region with external buffer, dwParam is pcosd_region_param */
#define OSD_IO_MOVE_REGION              (OSD_IO_ELEPHANT_BASE + 0x0c) /* Move the region created by OSD_IO_CREATE_REGION, dwParam is pcosd_region_param */
#define OSD_IO_GET_REGION_INFO          (OSD_IO_ELEPHANT_BASE + 0x0d) /* Get the region information , dwParam is posd_region_param */

enum OSD_SUBT_RESOLUTION{
	OSD_SUBT_RESO_720X576 = 1,
	OSD_SUBT_RESO_720X480,
	OSD_SUBT_RESO_1280X720,
	OSD_SUBT_RESO_1920X1080
};

typedef struct _osd_scale_param
{
    UINT16 tv_sys;       // enum TVSystem
    UINT16 h_div;
    UINT16 v_div;
    UINT16 h_mul;
    UINT16 v_mul;
} osd_scale_param, *posd_scale_param;
typedef const osd_scale_param *pcosd_scale_param;

typedef struct _osd_resize_param
{
    INT32 h_mode;
    INT32 v_mode;
} osd_resize_param, *posd_resize_param;

typedef struct 
{
	UINT8 enable;
	UINT8 layer;
	UINT8 no_temp_buf; // not use temp buffer
	UINT8 reserved;
}osd_clut_ycbcr_out; /*output ycbcr to DE, source is clut format*/

typedef struct 
{
	UINT8 region_id; // region id
	UINT8 reserved[3];
	UINT32 disp_addr;  // buffer address to be displayed 
}osd_disp_addr_cfg;

#define OSD_Resize_Param    osd_resize_param
#define POSD_Resize_Param   posd_resize_param

typedef struct _osd_region_param
{
    UINT8   region_id;      // The region id to create/move/get info

    UINT8   color_format;   // enum OSDColorMode
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

                            // ge_gma_create_region(): bitmap_addr and pixel_pitch determines the region bitmap address, total 4 cases:
                            // Case 1: if bitmap_addr is 0, and pixel_pitch is 0, it will use region_w as pixel_pitch,
                            //     and region bitmap addr will be allocated from uMemBase dynamically.
                            // Case 2: if bitmap_addr is 0, and pixel_pitch is not 0, the region bitmap addr will be fixed:
                            //     uMemBase + (pixel_pitch * bitmap_y + bitmap_x) * byte_per_pixel

                            // Case 3: if bitmap_addr is not 0, and pixel_pitch is 0, the region bitmap addr will be:
                            //     bitmap_addr + (bitmap_w * bitmap_y + bitmap_x) * byte_per_pixel
                            // Case 4: if bitmap_addr is not 0, and pixel_pitch is not 0, the region bitmap addr will be:
                            //     bitmap_addr + (pixel_pitch * bitmap_y + bitmap_x) * byte_per_pixel

                            // ge_gma_move_region(): region using internal memory can only change region_x, region_y, pal_sel;
                            // ge_gma_move_region(): region using external memory can change everyting in ge_gma_region_t;

    UINT32  bitmap_x;       // x offset from the top_left pixel in bitmap_addr or internal memory
    UINT32  bitmap_y;       // y offset from the top_left pixel in bitmap_addr or internal memory
    UINT32  bitmap_w;       // bitmap_w must >= bitmap_x + region_w, both for internal memory or external memory
    UINT32  bitmap_h;       // bitmap_h must >= bitmap_y + region_h, both for internal memory or external memory
} osd_region_param, *posd_region_param;
typedef const osd_region_param *pcosd_region_param;

void osd_m36f_attach(char *name, OSD_DRIVER_CONFIG *attach_config);

RET_CODE OSDDrv_Open(HANDLE hDev,struct OSDPara *ptPara);
RET_CODE OSDDrv_Close(HANDLE hDev);
RET_CODE OSDDrv_IoCtl(HANDLE hDev,UINT32 dwCmd,UINT32 dwParam);
RET_CODE OSDDrv_GetPara(HANDLE hDev,struct OSDPara* ptPara);
RET_CODE OSDDrv_ShowOnOff(HANDLE hDev,UINT8 uOnOff);
RET_CODE OSDDrv_SetPallette(HANDLE hDev,UINT8 *puPallette,UINT16 uColorNum,UINT8 uType);
RET_CODE OSDDrv_GetPallette(HANDLE hDev,UINT8 *puPallette,UINT16 uColorNum,UINT8 uType);
RET_CODE OSDDrv_ModifyPallette(HANDLE hDev,UINT8 uIndex,UINT8 uY,UINT8 uCb,UINT8 uCr,UINT8 uK);

RET_CODE OSDDrv_CreateRegion(HANDLE hDev,UINT8 uRegionId,struct OSDRect* rect,struct OSDPara*pOpenPara);
RET_CODE OSDDrv_DeleteRegion(HANDLE hDev,UINT8 uRegionId);

RET_CODE OSDDrv_SetRegionPos(HANDLE hDev,UINT8 uRegionId,struct OSDRect* rect);
RET_CODE OSDDrv_GetRegionPos(HANDLE hDev,UINT8 uRegionId,struct OSDRect* rect);

RET_CODE OSDDrv_RegionShow(HANDLE hDev,UINT8 uRegionId,BOOL bOn);

RET_CODE OSDDrv_RegionWrite(HANDLE hDev,UINT8 uRegionId,VSCR *pVscr,struct OSDRect *rect);
RET_CODE OSDDrv_RegionRead(HANDLE hDev,UINT8 uRegionId,VSCR *pVscr,struct OSDRect *rect);
RET_CODE OSDDrv_RegionFill(HANDLE hDev,UINT8 uRegionId,struct OSDRect *rect, UINT32 uColorData);
//#ifdef BIDIRECTIONAL_OSD_STYLE
RET_CODE OSDDrv_RegionWriteInverse(HANDLE hDev,UINT8 uRegionId,VSCR *pVscr,struct OSDRect *rect);
RET_CODE OSDDrv_RegionReadInverse(HANDLE hDev,UINT8 uRegionId,VSCR *pVscr,struct OSDRect *rect);
RET_CODE OSDDrv_RegionFillInverse(HANDLE hDev,UINT8 uRegionId,struct OSDRect *rect, UINT32 uColorData);
//#endif

#define OSDDrv_RegionFill32 OSDDrv_RegionFill
//#ifdef BIDIRECTIONAL_OSD_STYLE
#define OSDDrv_RegionFill32Inverse OSDDrv_RegionFillInverse
//#endif
RET_CODE OSDDrv_RegionWrite2(HANDLE hDev,UINT8 uRegionId,UINT8* pSrcData,UINT16 uSrcWidth,UINT16 uSrcHeight,struct OSDRect* pSrcRect,struct OSDRect* pDestRect);

RET_CODE OSDDrv_DrawHorLine(HANDLE hDev, UINT8 uRegionId, UINT32 x, UINT32 y, UINT32 width, UINT32 color);

RET_CODE OSDDrv_Scale(HANDLE hDev, UINT32 uScaleCmd,UINT32 uScaleParam);
RET_CODE OSDDrv_SetClip(HANDLE hDev,enum CLIPMode clipmode,struct OSDRect *pRect);
RET_CODE OSDDrv_ClearClip(HANDLE hDev);

UINT8 osd_get_byte_per_pixel(UINT8 color_mode);
UINT16 osd_get_pitch(UINT8 color_mode, UINT16 width);
#define osd_get_pitch_by_color_mode osd_get_pitch

#endif
