/*-----------------------------------------------------------------------------
*	Copyright (C) 2010 ALI Corp. All rights reserved.
*	GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef _GE_DRAW_PRIMITIVE_H_
#define _GE_DRAW_PRIMITIVE_H_

#include <hld/ge/ge.h>
#include "ge_primitive_init.h"
#include "osd_primitive.h"

typedef struct gui_app_cfg{
	GUI_DEV	hdev;
	GUI_RECT layer_rect[GUI_MAX_SURF];
	UINT8 layer_color[GUI_MAX_SURF];
	UINT8 rgn_cnt;
	UINT8 direct_draw;
	UINT8 animation_support;
}GUI_APP_CFG,*PGUI_APP_CFG;

typedef struct layer_app_cfg{
	GUI_DEV hdev;
	GUI_RECT rect;
	UINT32 color_mode:8;
	UINT32 slvgma:8;
	UINT32 rsv:16;
}GUI_LAYER_CFG,*PGUI_LAYER_CFG;

typedef struct rgn_app_cfg{
	UINT8	layer_idx;
	UINT8	rgn_idx;
	GUI_RECT rect;
}GUI_RGN_CFG,*PGUI_RGN_CFG;

typedef struct gui_layer
{
	GUI_DEV	hdev;
	SURF_HANDLE	hsurf;
	GUI_RECT rect;
	UINT8	surf_mode;
	UINT8	color_mode;
	UINT8	rgn_onlayer;
	UINT8	rsvd;
	UINT32	rsv;
}GUI_LAYER,*PGUI_LAYER;

typedef struct ge_gma_scale
{//M3602 only
	UINT32 vmul;
	UINT32 vdiv;
	
	UINT32 hmul;
	UINT32 hdiv;	
}GUI_SCALE_FACTOR;

extern GUI_LAYER 	g_layer_tbl[];
extern GEDRAW_MSGPOPUP_BLEND gedraw_up2scrn_cb;
/*##########################################################################*/
UINT8 app_gui_init(PGUI_RGN_CFG prgnlst,UINT32 param);
UINT8 app_popup_init(UINT32 param);
UINT8 app_popup_release(void);

PGUI_LAYER gedraw_getdstlayer(UINT8 layer_type);
PGUI_REGION gedraw_getdstrgn(UINT8 layer_type,UINT8 rgn_idx);

RET_CODE gedraw_drawpixel(PGUI_REGION prgn,PGUI_POINT pPos,UINT32 param);
RET_CODE gedraw_drawline(PGUI_REGION prgn,PGUI_POINT pStart,PGUI_POINT pEnd,UINT32 param);
RET_CODE gedraw_lineto(PGUI_REGION prgn,PGUI_POINT pStart,PGUI_POINT pEnd,UINT32 param);
RET_CODE gedraw_fillrect(PGUI_REGION prgn,PGUI_RECT pRect,UINT32 param);
RET_CODE gedraw_fillroundrect(PGUI_REGION prgn,PGUI_RECT pFrm,PGUI_RECT pCorner,UINT32 param);
RET_CODE gedraw_drawbmp(PGUI_REGION prgn,PGUI_RECT pFrm,UINT32 param);
RET_CODE gedraw_renderbmp(PGUI_REGION prgn,PGUI_RECT pfrm,UINT32 param);
RET_CODE gedraw_drawfont(PGUI_REGION prgn,PGUI_RECT pFrm,UINT32 param);

RET_CODE gedraw_update2scrn(PGUI_VSCR pvscr,PGUI_RECT prect_w);
RET_CODE gedraw_retrievescrn(PGUI_VSCR pvscr,PGUI_RECT prect_r);
RET_CODE gedraw_flush_rgn(PGUI_REGION pdst,PGUI_REGION psrc,PGUI_RECT pdst_rct,PGUI_RECT psrc_rct,UINT32 param);

RET_CODE gedraw_setclip(PGUI_REGION prgn,PGUI_RECT prect,UINT32 param);
RET_CODE gedraw_clearclip(PGUI_REGION prgn);
RET_CODE gedraw_colorkey_rgn(PGUI_REGION pdst,PGUI_REGION prgn1,PGUI_REGION prgn2,PGUI_RECT prct1,PGUI_RECT prct2,UINT32 param);
RET_CODE gedraw_blend_rgn(PGUI_REGION pdst,PGUI_REGION prgn1,PGUI_REGION prgn2,PGUI_RECT prct1,PGUI_RECT prct2,UINT32 param);
RET_CODE gedraw_setrgnpos(PGUI_REGION prgn,PGUI_RECT prect);
RET_CODE gedraw_getrgnpos(PGUI_REGION prgn,PGUI_RECT prect);
RET_CODE gedraw_modifypallette(PGUI_REGION prgn,UINT8 idx,UINT8 *pallette);
RET_CODE gedraw_setpallette(PGUI_REGION prgn,UINT8 *pallette);
RET_CODE gedraw_getpallette(PGUI_REGION prgn,UINT8 *pallette);
RET_CODE gedraw_scale(PGUI_REGION prgn,UINT32 cmd,UINT32 param);
RET_CODE gedraw_setgalpha(UINT32 layer,UINT32 gtrans);
RET_CODE gedraw_rgn_showonoff(PGUI_REGION prgn,UINT8 on_off);
RET_CODE gedraw_surf_showonoff(PGUI_LAYER psurf,UINT8 on_off);
RET_CODE gedraw_deletergn(PGUI_REGION prgn);
RET_CODE gedraw_creatergn(UINT32 param);
RET_CODE gedraw_createsurf(UINT32 param);
RET_CODE gedraw_releasesurf(PGUI_LAYER psurf);

UINT32 app_gui_switch(UINT32 layer,GUI_TVSYS tvsys);//M3602 only
#endif

