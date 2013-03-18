/*-----------------------------------------------------------------------------
*	Copyright (C) 2010 ALI Corp. All rights reserved.
*	GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef _OSD_PRIMITIVE_H_
#define _OSD_PRIMITIVE_H_

#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
/*#include <hld/ge/ge.h>*/
/*Should not include any driver definations,only reserved for internal*/

#if 1
#define gui_printf(...) do{}while(0)
#define gui_debug_show() do{}while(0)
#else
extern char gelib_dbstrs[];
#define gui_printf(args...)	do{ \
						sprintf(gelib_dbstrs,##args); \
						sprintf(gelib_dbstrs,"%s@%s %d.",gelib_dbstrs,__FUNCTION__,__LINE__); \
						}while(0)
#define gui_debug_show() do{ \
							if((prgn->layer_type == GUI_MEMVSCR_SURF)&&(prgn->hsurf!=0)){ \
								PGUI_REGION pdst_debug = gelib_getdstrgn(GUI_GMA1_SURF,0); \
								gelib_flush_rgn(pdst_debug,prgn,NULL,NULL,0);} \
								}while(0)
#endif


#ifdef GUI_MULTITASK_MAX
#define GUI_MULTITASK_CNT GUI_MULTITASK_MAX
#else
#define GUI_MULTITASK_CNT 32
#endif

#ifndef GUI_MAX_LAYER_CNT
	#define GUI_MAX_LAYER_CNT GUI_MAX_SURF//hw support 2layers now
	#define GUI_GMA_LAYER_CNT 3//support dual-output
#endif

#ifndef GUI_MAX_REGION_CNT
	#define GUI_MAX_REGION_ONLAYER 6
	#define GUI_GMA_REGION_OFFSET (GUI_MAX_REGION_ONLAYER*GUI_GMA_LAYER_CNT)
	#define GUI_MAX_REGION_CNT (GUI_GMA_REGION_OFFSET + GUI_MAX_SURF - GUI_GMA_LAYER_CNT)//default setting count for how many regions in one layer.
#endif

#ifndef GUI_MAX_WND_CNT
	#define GUI_MAX_WND_CNT 4//default setting count for how many windows in one region
#endif

#ifdef COLOR_N
#define CLUT_COLOR_N COLOR_N
#else
#define CLUT_COLOR_N 256
#endif

typedef enum gui_surf_colormode
{
    GUI_INVALIDFORMAT = 0,
	GUI_CLUT1,//special reserved for font format,1-0 bit format.
	GUI_CLUT8,
	GUI_ARGB1555,
	GUI_ARGB4444,
	GUI_AYCBCR8888,
	GUI_ARGB8888,
	GUI_COLORMAX,
}GUI_COLOR_MODE;

enum gui_surf_type
{
	GUI_GMA1_SURF = 0,
	GUI_GMA2_SURF,
	GUI_SLVGMA_SURF,//dual output SD layer
	GUI_MEMVSCR_SURF,
	GUI_SLVSCR_SURF,
	GUI_MEMPOP_SURF,
	GUI_SLVPOP_SURF,
	GUI_MEMBLEND_SURF,
	GUI_MAX_SURF,
	GUI_INVALID_SURF = 0xff,
	//reserved here for more type;
}GUI_SURF_TYPE;

typedef UINT32 SURF_HANDLE;
typedef UINT32 GUI_DEV;

typedef struct cpoint
{
	union
	{
		UINT16 uX;
		UINT16 uLeft;
	};
	union
	{
		UINT16 uY;
		UINT16 uTop;
	};
}GUI_POINT,*PGUI_POINT;//make compatible with old member names

typedef struct crect
{
#ifdef _MHEG5_ENABLE_
	union
	{
		INT16	uStartX;
		INT16	uLeft;
	};
	union
	{
		INT16	uStartY;
		INT16	uTop;
	};
#else
	union
	{
		UINT16	uStartX;
		UINT16	uLeft;
	};
	union
	{
		UINT16	uStartY;
		UINT16	uTop;
	};
#endif
	UINT16	uWidth;
	UINT16	uHeight;
}GUI_RECT,*PGUI_RECT;
typedef struct RectSize GUI_RECTSIZE,*PGUI_RECTSIZE;
//Above code section is derived from <mediatypes.h>

typedef struct rsc_pen
{
	UINT32 color;
	UINT32 thick:16;
	UINT32 rsvd:16;/*such as pattern,texture etc.*/
}GUI_PEN,*PGUI_PEN;

typedef struct rsc_bmp
{
	UINT32	stride;
	UINT32	bg_color;/*key_color*/
    	UINT32  rsc_rle;
    
	void *	pdata;
    
	GUI_RECTSIZE	bmp_size;
	GUI_COLOR_MODE	color_mode;
}GUI_BMP,*PGUI_BMP;

typedef struct rsc_font
{
	UINT32	stride;
	UINT32	fg_color;
	UINT32	bg_color;
	UINT32  rsc_rle;
    
	void *pdata;
	
	GUI_RECTSIZE	fnt_size;
}GUI_FONT,*PGUI_FONT;

typedef struct gui_vscr
{
	union
	{
		GUI_RECT frm;
		GUI_RECT vR;
	};
	
	UINT32	root_layer:8;
	UINT32	root_rgn:8;
	UINT32	dst_layer:8;
	UINT32	dst_rgn:8;
	
	union
	{
		UINT8	dirty_flag;
		UINT8	updatePending;
	};
	
	UINT8	color_mode;
	UINT8	suspend;
	
	union
	{
		UINT8*	lpbuff;
		UINT8*	lpbScr;
	};

	UINT32 rsv;
}GUI_VSCR,*PGUI_VSCR;

typedef struct gui_vscr_cfg
{
	UINT32	root_layer:8;
	UINT32	root_rgn:8;
	UINT32	dst_layer:8;
	UINT32	dst_rgn:8;
}GUI_VSCR_CFG,*PGUI_VSCR_CFG;

typedef struct gui_rgn
{
	SURF_HANDLE hsurf;
	//for memsurf in new GE_DRV,it will assigned as buffer address
	
	GUI_RECT rect;
	UINT8	layer_type;
	UINT8	rgn_id;
	UINT8	color_mode;
	UINT8	rsvd;
	UINT32	rsv;
}GUI_REGION,*PGUI_REGION;

typedef enum TVSystem GUI_TVSYS;

extern UINT32	gui_keycolor[];
extern const UINT8 bypp_const[];
extern const UINT32 colormask_const[];
extern UINT32 transcolor_const[];

extern UINT8	*vscr_buff;
extern UINT8	*pop_buff;
extern UINT8	*slvscr_buff;
extern UINT8	*slvpop_buff;

extern UINT8 vrgn_idx;
extern UINT8 vlayer_idx;

extern UINT32	gelib_state;
extern UINT32	ge_version;
extern UINT32	gui_design_dimension;
extern GUI_TVSYS		g_gui_tvsys;
extern UINT32 g_gelib_m3602_dualoutput;

extern GUI_DEV		gui_dev;
extern GUI_VSCR	g_vscr[];
extern GUI_VSCR	g_slvscr[];
extern GUI_REGION	g_rgn_tbl[];
extern PGUI_REGION	g_memrgn_tbl;

typedef RET_CODE (*GEDRAW_MSGPOPUP_BLEND)(UINT32);

typedef PGUI_REGION (*GUI_GETDSTRGN)(UINT8,UINT8);

typedef RET_CODE (*GUI_DRAWPIXEL)(PGUI_REGION,PGUI_POINT,UINT32);
typedef RET_CODE (*GUI_DRAWLINE)(PGUI_REGION,PGUI_POINT,PGUI_POINT,UINT32 param);
typedef RET_CODE (*GUI_LINETO)(PGUI_REGION prgn,PGUI_POINT pStart,PGUI_POINT pEnd,UINT32 param);
typedef RET_CODE (*GUI_FILLRECT)(PGUI_REGION prgn,PGUI_RECT pRect,UINT32 param);
typedef RET_CODE (*GUI_FILLROUNDRECT)(PGUI_REGION prgn,PGUI_RECT pFrm,PGUI_RECT pCorner,UINT32 param);
typedef RET_CODE (*GUI_DRAWBMP)(PGUI_REGION prgn,PGUI_RECT pFrm,UINT32 param);
typedef RET_CODE (*GUI_RENDERBMP)(PGUI_REGION prgn,PGUI_RECT pfrm,UINT32 param);
typedef RET_CODE (*GUI_DRAWFONT)(PGUI_REGION prgn,PGUI_RECT pFrm,UINT32 param);

typedef RET_CODE (*GUI_UPDATE2SCRN)(PGUI_VSCR pvscr,PGUI_RECT prect_w);
typedef RET_CODE (*GUI_RETRIEVESCRN)(PGUI_VSCR pvscr,PGUI_RECT prect_r);
typedef RET_CODE (*GUI_FLUSH_RGN)(PGUI_REGION pdst,PGUI_REGION psrc,PGUI_RECT pdst_rct,PGUI_RECT psrc_rct,UINT32 param);
typedef RET_CODE (*GUI_COLORKEY_RGN)(PGUI_REGION pdst,PGUI_REGION prgn1,PGUI_REGION prgn2,PGUI_RECT prct1,PGUI_RECT prct2,UINT32 param);
typedef RET_CODE (*GUI_BLEND_RGN)(PGUI_REGION pdst,PGUI_REGION prgn1,PGUI_REGION prgn2,PGUI_RECT prct1,PGUI_RECT prct2,UINT32 param);

typedef RET_CODE (*GUI_SETCLIP)(PGUI_REGION prgn,PGUI_RECT prect,UINT32 param);
typedef RET_CODE (*GUI_CLEARCLIP)(PGUI_REGION prgn);
typedef RET_CODE (*GUI_MODIFYPALLETTE)(PGUI_REGION prgn,UINT8 idx,UINT8 *pallette);
typedef RET_CODE (*GUI_SETPALLETTE)(PGUI_REGION prgn,UINT8 *pallette);
typedef RET_CODE (*GUI_GETPALLETTE)(PGUI_REGION prgn,UINT8 *pallette);

typedef RET_CODE (*GUI_DELETERGN)(PGUI_REGION prgn);
typedef RET_CODE (*GUI_CREATERGN)(UINT32 param);
typedef RET_CODE (*GUI_RGN_SHOWONFF)(PGUI_REGION prgn,UINT8 on_off);
typedef RET_CODE (*GUI_SET_RGNPOS)(PGUI_REGION prgn,PGUI_RECT prect);
typedef RET_CODE (*GUI_GET_RGNPOS)(PGUI_REGION prgn,PGUI_RECT prect);
typedef RET_CODE (*GUI_SCALE)(PGUI_REGION prgn,UINT32 cmd,UINT32 param);
typedef RET_CODE (*GUI_SETGALPHA)(UINT32 layer,UINT32 param);

extern GUI_GETDSTRGN	gelib_getdstrgn;

extern GUI_DRAWFONT	gelib_drawfont;
extern GUI_RENDERBMP	gelib_renderbmp;
extern GUI_DRAWBMP	gelib_drawbmp;
extern GUI_FILLROUNDRECT	gelib_fillroundrect;
extern GUI_FILLRECT	gelib_fillrect;
extern GUI_LINETO	gelib_lineto;
extern GUI_DRAWLINE	gelib_drawline;

extern GUI_COLORKEY_RGN	gelib_colorkey_rgn;
extern GUI_BLEND_RGN	gelib_blend_rgn;
extern GUI_FLUSH_RGN	gelib_flush_rgn;
extern GUI_RETRIEVESCRN	gelib_retrievescrn;
extern GUI_UPDATE2SCRN	gelib_update2scrn;

extern GUI_GETPALLETTE	gelib_get_pallette;
extern GUI_SETPALLETTE	gelib_set_pallette;
extern GUI_MODIFYPALLETTE	gelib_modify_pallette;
extern GUI_CLEARCLIP	gelib_clearclip;
extern GUI_SETCLIP	gelib_setclip;

extern GUI_GET_RGNPOS	gelib_get_rgnpos;
extern GUI_SET_RGNPOS	gelib_set_rgnpos;
extern GUI_RGN_SHOWONFF	gelib_rgn_showonoff;
extern GUI_CREATERGN	gelib_create_rgn;
extern GUI_DELETERGN	gelib_delete_rgn;
extern GUI_SCALE	gelib_scale;
extern GUI_SETGALPHA gelib_setgalpha;

#define gelib_settranscolor(color_mode,trscolor)do{(transcolor_const[color_mode] = trscolor);}while(0)
#define gelib_set_slaveout(on_off) (g_gelib_m3602_dualoutput = (on_off))
#define gelib_getdual_support()	((g_gelib_m3602_dualoutput>0)?1:0)
#define gelib_setkeycolor(color_mode,keycolor) (gui_keycolor[color_mode] = keycolor)
#define gelib_get_rgnsize(prgn) ((prgn->rect.uWidth * prgn->rect.uHeight)<<bypp_const[prgn->color_mode])
//#define gelib_get_rgnpitch(prgn) ((prgn->rect.uWidth)<<bypp_const[prgn->color_mode])
#define gelib_get_rgnpitch(prgn) ((prgn->rect.uWidth))/*GE_DRV does the work*/
#define gelib_get_posdiff(ps1,ps2) ((ps1) > (ps2) ? ((ps1)-(ps2)) : ((ps2)-(ps1)))
#define gelib_color_expend(argb1555) (((UINT32)(0xff000000))|((UINT32)(argb1555&0x7c00)<<9)|((UINT32)(argb1555&0x3e0)<<6)|((UINT32)(argb1555&0x1f)<<3))
#define BE_TO_LOCAL_WORD(val)   ((val))
#define MAX(a, b)		((a) > (b) ? (a) : (b))
#define MIN(a, b)		((a) > (b) ? (b) : (a))

#define gui_colormask(prgn,val)	(val & colormask_const[prgn->color_mode])
#define gui_rctvalid(rct)		(((((rct)->uWidth)>0) && (((rct)->uHeight)>0))?1:0)
#define gui_rctsize(rct1,rct2) (((((rct1)->uWidth) == ((rct2)->uWidth)) && (((rct1)->uHeight) == ((rct2)->uHeight)))?1:0)
#endif

