#ifndef __OSDDRV_DEV_H__
#define __OSDDRV_DEV_H__


struct OSDRect
{
	INT16	uLeft;
	INT16	uTop;
	INT16	uWidth;
	INT16	uHeight;
};

#ifdef WIN32
typedef struct OSDRect OSD_RECT, *POSD_RECT;
typedef const struct OSDRect *PCOSD_RECT;
#endif

enum CLIPMode
{
	CLIP_INSIDE_RECT = 0,
	CLIP_OUTSIDE_RECT,
	CLIP_OFF
};

enum OSDSys
{
	OSD_PAL = 0,
	OSD_NTSC
};

enum OSDColorMode{
	OSD_4_COLOR =	0,
	OSD_16_COLOR,
	OSD_256_COLOR,
	OSD_16_COLOR_PIXEL_ALPHA,
	OSD_HD_ACLUT88,//@Alan modify080421
	OSD_HD_RGB565,
	OSD_HD_RGB888,
	OSD_HD_RGB555,
	OSD_HD_RGB444,
	OSD_HD_ARGB565,
	OSD_HD_ARGB8888,
	OSD_HD_ARGB1555,
	OSD_HD_ARGB4444,
	OSD_HD_AYCbCr8888,
	OSD_HD_YCBCR888,
	OSD_HD_YCBCR422,
	OSD_HD_YCBCR422MB,
	OSD_HD_YCBCR420MB,
	OSD_COLOR_MODE_MAX
};

typedef struct tagVScr
{
	struct OSDRect	vR;
	UINT8 	*lpbScr;
	UINT8	bBlockID;
	BOOL	updatePending;
    UINT8   bColorMode;
	UINT8 bDrawMode;
}VSCR,*lpVSCR;

#define ENABLE_CONTIGUOUS_MEMORY    0x80
struct OSDPara
{
	enum OSDColorMode eMode; /* Color mode */
	UINT8 uGAlphaEnable;
	UINT8 uGAlpha;          /* Alpha blending mix ratio */
	UINT8 uPalletteSel;          /* Pallette index */
};

struct OSD_attach_config
{
	BOOL  bOSDVerticalFilterEnable;                /*enable/disable osd vertical filter*/ 
	BOOL  P2NScaleInNormalPlay;      		    /*enable/disable PAL/NTSC scale in normal play mode*/
	BOOL  P2NScaleInSubtitlePlay;		    /*enable/disable PAL/NTSC scale in subtitle play mode*/
};

struct OSD_AF_PAR
{
	UINT8 id;
	UINT8 vd:1;
	UINT8 af:1;
	UINT8 res:6;
};

// Special case for M3602 DE-O
#define  OSD_M3602_NOT_ATTACH_DEO   0x80    // Please set it to OSD_DRIVER_CONFIG.bStaticBlock

typedef struct _OSD_DRIVER_CONFIG
{
	UINT32 uMemBase;        //previous defined as __MM_OSD_START_ADDR, 16 bit align
	UINT32 uMemSize;		//previous defined as __MM_OSD_LEN,  usually 720*576/factor
							//and factor = how many pixels that 1 byte maps to

	UINT8 bStaticBlock;		//these three varialbes are only used for M33xxC
	UINT8 bDirectDraw;
	UINT8 bCacheable;

	UINT8 bVFilterEnable;   		//enable/disable osd vertical filter 
	UINT8 bP2NScaleInNormalPlay; 	//enable/disable PAL/NTSC scale in normal play mode
	UINT8 bP2NScaleInSubtitlePlay;	//enable/disable PAL/NTSC scale in subtitle play mode
	UINT16 uDViewScaleRatio[2][2];	//set multi-view scale ratio, only for M33xxC now
	struct OSD_AF_PAR af_par[4];	
	UINT32 uSDMemBase;      // memory addr for SD osd when OSD dual-output is enabled,
	                    	// and SD osd source size is not same with HD side
	UINT32 uSDMemSize;		// memory size for SD osd when OSD dual-output is enabled
	                   		// and  SD osd source size is not same with HD side

}OSD_DRIVER_CONFIG, *POSD_DRIVER_CONFIG;

typedef struct _OSD_MINI_DRIVER_CONFIG
{
	UINT32 uOsdBkStartAddr;
	UINT32 uOsdBkSize;
	UINT32 uOsdDisHDStartAddr;
	UINT32 uOsdDisHDSize;
	UINT32 uOsdDisSDStartAddr;
	UINT32 uOsdDisSDSize;
	UINT32 uPalletBuf;
	UINT32 uGeTempBuf;	
	UINT32 uGeTempSize;	
	UINT32 uGeCmdBuf;	
	UINT32 uGeCmdSize;	
	UINT8 bP2NScaleInNormalPlay;      		    //enable/disable PAL/NTSC scale in normal play mode
}OSD_MINI_DRIVER_CONFIG, *POSD_MINI_DRIVER_CONFIG;

/*
struct OSD_OpenPara
{
	struct OSDRect	tMaxCaseRect;
	enum OSDSys eOSDSys;
	UINT16  	uColorNum;
	UINT8 	uBitNum;
	BOOL 	bCompressed;
	BOOL 	bUseGlobalK;
	UINT8 	uAlpha;	
};
*/

struct osd_device
{
	/* Common device structure member */
	struct osd_device *next;            /* link to next device */
	UINT32 type;                        /* Interface hardware type */
	INT8 name[HLD_MAX_NAME_SIZE];       /* Device name */

	UINT32  flags;				/* Interface flags, status and ability */

	/* Hardware privative structure */
	void	*priv;				/* pointer to private data */
	UINT32	sema_opert_osd; 
	/* Device related functions */
	RET_CODE (*open)(struct osd_device *, struct OSDPara *);
	RET_CODE (*close)(struct osd_device *);
	RET_CODE (*ioctl)(struct osd_device *,UINT32,UINT32);
	
	RET_CODE (*get_para)(struct osd_device *,struct OSDPara *);
	RET_CODE (*show_onoff)(struct osd_device *,UINT8);
	RET_CODE (*set_pallette)(struct osd_device *,UINT8 *,UINT16,UINT8);
	RET_CODE (*get_pallette)(struct osd_device *,UINT8 *,UINT16,UINT8);
	RET_CODE (*modify_pallette)(struct osd_device *,UINT8 ,UINT8 ,UINT8 ,UINT8 ,UINT8 );

	RET_CODE (*create_region)(struct osd_device *,UINT8 ,struct OSDRect* ,struct OSDPara * );
	RET_CODE (*delete_region)(struct osd_device *,UINT8 );
	
	RET_CODE (*set_region_pos)(struct osd_device *,UINT8 ,struct OSDRect* );
	RET_CODE (*get_region_pos)(struct osd_device *,UINT8 ,struct OSDRect* );

	RET_CODE (*region_show)(struct osd_device *,UINT8, BOOL);

	RET_CODE (*region_write)(struct osd_device *,UINT8,VSCR *,struct OSDRect *);
	RET_CODE (*region_read)(struct osd_device *,UINT8,VSCR *,struct OSDRect *);	
	RET_CODE (*region_fill)(struct osd_device *,UINT8,struct OSDRect *, UINT32);
	RET_CODE (*region_write2)(struct osd_device *,UINT8,UINT8* ,UINT16 ,UINT16 ,struct OSDRect* ,struct OSDRect* );

	RET_CODE (*draw_hor_line)(struct osd_device *, UINT8, UINT32, UINT32, UINT32, UINT32);

	RET_CODE (*scale)(struct osd_device *, UINT32 ,UINT32);
	
	RET_CODE (*set_clip)(struct osd_device *,enum CLIPMode,struct OSDRect*);
	RET_CODE (*clear_clip)(struct osd_device *);
    RET_CODE (*get_region_addr)(struct osd_device *,UINT8 ,UINT16, UINT32 *);
//#ifdef BIDIRECTIONAL_OSD_STYLE
	RET_CODE (*region_write_inverse)(struct osd_device *,UINT8,VSCR *,struct OSDRect *);
	RET_CODE (*region_read_inverse)(struct osd_device *,UINT8,VSCR *,struct OSDRect *);	
	RET_CODE (*region_fill_inverse)(struct osd_device *,UINT8,struct OSDRect *, UINT32);
//#endif
};


#endif /* __OSDDRV_DEV_H__ */

