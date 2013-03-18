/*-----------------------------------------------------------------------------
*
*	Copyright (C) 2008 ALI Corp. All rights reserved.
*	File: osd_plugin.c
*	Content: 
*		Add compatible function interface with OSD_DRV based library funtions.
*	History: 
*		2008/07/13 by Shine Zhou
*		2009/11/03 by Shine Zhou
*		Build for GE library
*-----------------------------------------------------------------------------*/
#include <sys_config.h>
#include <api/libge/osd_lib.h>
#include <api/libge/osd_window.h>
#include <api/libge/osd_primitive.h>
#include <api/libge/ge_draw_primitive.h>
#include <api/libge/osd_common_draw.h>
#include <api/libge/osd_plugin.h>
#include "osd_lib_internal.h"

/*!!!!!!!!!!!!!!!!!!!!!ALERT!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*only for old ge drv compatible API inteface,if using 3329D,3602f and later on chipset*/
/*please make sure do not using the interface*/
UINT8 g_rgn_idx;
SURF_HANDLE g_surf_handle;
extern GUI_LAYER g_layer_tbl[];

void OSD_DrawPicture(UINT16 x, UINT16 y, UINT16 idPicture,ID_RSC RscLibId, UINT32 Style, PGUI_VSCR pVscr)
{
	GUI_POINT pos;

	pos.uX = x;
	pos.uY = y;
	OSD_DrawBmp2Vscr(&pos,idPicture,pVscr);
}

void OSD_DrawPixel(UINT16 x, UINT16 y, UINT32 color, PGUI_VSCR pVscr)
{
	OSD_DrawFill(x,y,2,2,color,pVscr);
}

void OSD_DrawHorLine(UINT16 x, UINT16 y, UINT16 w, UINT32 color, PGUI_VSCR pVscr)
{
	if(wstyle_colorshow(color))
		OSD_DrawFill(x, y, w,2,color,pVscr);
}

void OSD_DrawVerLine(UINT16 x, UINT16 y, UINT16 h, UINT32 color, PGUI_VSCR pVscr)
{
	if(wstyle_colorshow(color))
		OSD_DrawFill(x,y,2,h,color,pVscr);
}

void OSD_DrawFill(UINT16 x, UINT16 y, UINT16 w, UINT16 h, UINT32 color,PGUI_VSCR pVscr)
{
	GUI_RECT frm;

	frm.uLeft = x;
	frm.uTop = y;
	frm.uWidth = w;
	frm.uHeight = h;
	OSD_DrawRect2Vscr(&frm,(UINT32)color,pVscr);
}

RET_CODE OSD_RegionFill(PGUI_RECT pFrame,UINT32 color)
{
	UINT8 i;
	PGUI_REGION prgn;
	
	for(i  = GUI_GMA1_SURF;i < GUI_MAX_SURF;i++)
	{
		if(g_layer_tbl[i].hsurf == g_surf_handle)
			break;
	}
	
	if(i < GUI_MAX_SURF)
	{
		prgn = gedraw_getdstrgn(i,g_rgn_idx);
		gedraw_fillrect(prgn,pFrame,color);
	}
}

void OSD_ShowOnOff(UINT8 onoff)
{
	UINT8 i;
	PGUI_REGION prgn;
	
	for(i  = GUI_GMA1_SURF;i < GUI_MAX_SURF;i++)
	{
		if(g_layer_tbl[i].hsurf == g_surf_handle)
			break;
	}

	if(i < GUI_MAX_SURF)
	{
		gedraw_surf_showonoff(&g_layer_tbl[i],onoff);
#if 0
		if(1 >= g_layer_tbl[i].rgn_onlayer)
		{
			gedraw_surf_showonoff(&g_layer_tbl[i],onoff);
		}
		else
		{
			prgn = gedraw_getdstrgn(i,g_rgn_idx);
			gedraw_rgn_showonoff(prgn,onoff);
		}
#endif
	}
}

void OSD_SetRectOnScreen(PGUI_RECT rect)
{
	UINT8 i;
	PGUI_REGION prgn;
	
	for(i  = GUI_GMA1_SURF;i < GUI_MAX_SURF;i++)
	{
		if(g_layer_tbl[i].hsurf == g_surf_handle)
			break;
	}

	if(i < GUI_MAX_SURF)
	{
		prgn = gedraw_getdstrgn(i,g_rgn_idx);
		gedraw_setrgnpos(prgn,rect);
	}
}

void OSD_GetRectOnScreen(PGUI_RECT rect)
{
	UINT8 i;
	PGUI_REGION prgn;
	
	for(i  = GUI_GMA1_SURF;i < GUI_MAX_SURF;i++)
	{
		if(g_layer_tbl[i].hsurf == g_surf_handle)
			break;
	}

	if(i < GUI_MAX_SURF)
	{
		prgn = gedraw_getdstrgn(i,g_rgn_idx);
		gedraw_getrgnpos(prgn,rect);
	}
}

UINT32 OSD_GetVscrBuffSize(PGUI_VSCR pvscr)
{
	return (((pvscr->frm.uWidth) * (pvscr->frm.uHeight))<<bypp_const[pvscr->color_mode]);
}

BOOL OSD_GetRegionData(PGUI_VSCR pVscr,PGUI_RECT rect)
{
	UINT8 i;
	ge_rect_t  rc1,rc2;
	struct ge_device *dev;
	ge_region_pars_t cfg;
	ge_region_t rgn_para;
	ge_surface_desc_t *pdstsurf,surf_vscr;

	dev = NULL;
	pdstsurf = NULL;
	MEMSET(&cfg,0x0,sizeof(cfg));
	MEMSET(&surf_vscr,0x0,sizeof(surf_vscr));
	MEMSET(&rgn_para,0x0,sizeof(rgn_para));
	for(i  = GUI_GMA1_SURF;i < GUI_MAX_SURF;i++)
	{
		if(g_layer_tbl[i].hsurf == g_surf_handle)
		{
			dev = (struct ge_device *)g_layer_tbl[i].hdev;
			break;
		}
	}

	if(NULL == dev)
		return;//golobal handle is NULL

	surf_vscr.width = rect->uWidth;
	surf_vscr.height = rect->uHeight;
	surf_vscr.endian = GE_PIXEL_LITTLE_ENDIAN;
	surf_vscr.alpha_range = GE_LITTLE_ALPHA;
	
	rgn_para.rect.left = 0;
	rgn_para.rect.top = 0;
	rgn_para.rect.width = rect->uWidth;
	rgn_para.rect.height = rect->uHeight;
	rgn_para.glob_alpha_en = 0;
	rgn_para.glob_alpha = 0x7f;
	rgn_para.pallet_seg = 0;
	if(GUI_CLUT8 == pVscr->color_mode)
	{
		rgn_para.pixel_fmt = GE_PF_CLUT8;
	}
	else if(GUI_ARGB1555 == pVscr->color_mode)
	{
		rgn_para.pixel_fmt = GE_PF_ARGB1555;
	}
	else
	{
		rgn_para.pixel_fmt = GE_PF_AYCBCR8888;
	}
	surf_vscr.pitch = rgn_para.pitch = rect->uWidth << bypp_const[pVscr->color_mode];
	pdstsurf = ge_create_surface(dev,&surf_vscr,0,GMA_MEM_SURFACE);
	ge_create_region(dev,pdstsurf,0,&rgn_para.rect, &rgn_para);

	cfg.addr = pVscr->lpbuff;
	cfg.pitch = rgn_para.pitch;
	ge_set_region_pars(dev,pdstsurf,0,&cfg);

	rc1.left = 0;
	rc1.top = 0;
	rc2.left = rect->uStartX;
	rc2.top = rect->uStartY;
	rc1.width = rc2.width = rect->uWidth;
	rc1.height = rc2.height = rect->uHeight;
	ge_lock(dev);
	ge_fill_rect(dev,pdstsurf,&rc1,(pVscr->color_mode)[transcolor_const]);
	ge_set_cur_region(dev,(ge_surface_desc_t *)g_surf_handle,g_rgn_idx);
	ge_blt(dev,pdstsurf,(ge_surface_desc_t *)g_surf_handle, \
		&rc1,&rc2,GE_BLT_SRC2_VALID);
	ge_unlock(dev);
	ge_release_surface(dev,pdstsurf);
}

void OSD_RegionWrite(PGUI_VSCR pVscr,PGUI_RECT rect)
{
	UINT8 i;
	ge_rect_t  rc1,rc2;
	struct ge_device *dev;
	ge_region_pars_t cfg;
	ge_region_t rgn_para;
	ge_surface_desc_t *psrcsurf,surf_vscr;

	dev = NULL;
	psrcsurf = NULL;
	MEMSET(&cfg,0x0,sizeof(cfg));
	MEMSET(&surf_vscr,0x0,sizeof(surf_vscr));
	MEMSET(&rgn_para,0x0,sizeof(rgn_para));
	for(i  = GUI_GMA1_SURF;i < GUI_MAX_SURF;i++)
	{
		if(g_layer_tbl[i].hsurf == g_surf_handle)
		{
			dev = (struct ge_device *)g_layer_tbl[i].hdev;
			break;
		}
	}

	if(NULL == dev)
		return;//golobal handle is NULL

	surf_vscr.width = rect->uWidth;
	surf_vscr.height = rect->uHeight;
	surf_vscr.endian = GE_PIXEL_LITTLE_ENDIAN;
	surf_vscr.alpha_range = GE_LITTLE_ALPHA;
	
	rgn_para.rect.left = 0;
	rgn_para.rect.top = 0;
	rgn_para.rect.width = rect->uWidth;
	rgn_para.rect.height = rect->uHeight;
	rgn_para.glob_alpha_en = 0;
	rgn_para.glob_alpha = 0x7f;
	rgn_para.pallet_seg = 0;
	if(GUI_CLUT8 == pVscr->color_mode)
	{
		rgn_para.pixel_fmt = GE_PF_CLUT8;
	}
	else if(GUI_ARGB1555 == pVscr->color_mode)
	{
		rgn_para.pixel_fmt = GE_PF_ARGB1555;
	}
	else
	{
		rgn_para.pixel_fmt = GE_PF_AYCBCR8888;
	}
	surf_vscr.pitch = rgn_para.pitch = rect->uWidth << bypp_const[pVscr->color_mode];
	psrcsurf = ge_create_surface(dev,&surf_vscr,0,GMA_MEM_SURFACE);
	ge_create_region(dev,psrcsurf,0,&rgn_para.rect, &rgn_para);

	cfg.addr = pVscr->lpbuff;
	cfg.pitch = rgn_para.pitch;
	ge_set_region_pars(dev,psrcsurf,0,&cfg);

	rc2.left = 0;
	rc2.top = 0;
	rc1.left = rect->uStartX;
	rc1.top = rect->uStartY;
	rc1.width = rc2.width = rect->uWidth;
	rc1.height = rc2.height = rect->uHeight;
	
	ge_lock(dev);
	ge_set_cur_region(dev,(ge_surface_desc_t *)g_surf_handle,g_rgn_idx);
	ge_blt(dev,(ge_surface_desc_t *)g_surf_handle,psrcsurf, \
		&rc1,&rc2,GE_BLT_SRC2_VALID);
	ge_unlock(dev);
	
	ge_release_surface(dev,psrcsurf);
}

RET_CODE OSD_Scale(UINT8 uScaleCmd, UINT32 uScaleParam)
{
	UINT8 i;
	PGUI_REGION prgn;
	ge_scale_par_29e_t scale;
    	 UINT32 *uScale=(UINT32*)uScaleParam;
	for(i  = GUI_GMA1_SURF;i < GUI_MAX_SURF;i++)
	{
		if(g_layer_tbl[i].hsurf == g_surf_handle)
			break;
	}

	if(i < GUI_MAX_SURF)
	{
		prgn = gedraw_getdstrgn(i,g_rgn_idx);
	}
	else
	{
		prgn = NULL;
		return RET_FAILURE;
	}

	if (*uScale == OSD_SYSTEM_PAL)
	{
		scale.src_h = 576;
		scale.dst_h = 576;
	}
	else if (*uScale == OSD_SYSTEM_NTSC)
	{
		scale.src_h = 576;
		scale.dst_h = 480;
	}
	else
	{
		scale.src_h = 576;
		scale.dst_h = 576;
	}	
	scale.suf_id = i;	//GUI_GMA1_SURF;
	
	return gedraw_scale(prgn,GE_IO_SCALE_OSD_29E, (UINT32)&scale);
}

RET_CODE OSD_SetPallette(UINT8 *p_pallette,UINT16 wN)
{
	UINT8 i;
	PGUI_REGION prgn;
	
	for(i  = GUI_GMA1_SURF;i < GUI_MAX_SURF;i++)
	{
		if(g_layer_tbl[i].hsurf == g_surf_handle)
			break;
	}
	
	if(i < GUI_MAX_SURF)
	{
		prgn = gedraw_getdstrgn(i,g_rgn_idx);
		gedraw_setpallette(prgn,p_pallette);
	}
}

void OSD_SetClipRect(PGUI_RECT pRect,UINT32 param)
{
	UINT8 i;
	PGUI_REGION prgn;
	
	for(i  = GUI_GMA1_SURF;i < GUI_MAX_SURF;i++)
	{
		if(g_layer_tbl[i].hsurf == g_surf_handle)
			break;
	}
	
	if(i < GUI_MAX_SURF)
	{
		prgn = gedraw_getdstrgn(i,g_rgn_idx);
		gedraw_setclip(prgn,pRect,param);
	}
}

void OSD_ClearClipRect(void)
{
	UINT8 i;
	PGUI_REGION prgn;
	
	for(i  = GUI_GMA1_SURF;i < GUI_MAX_SURF;i++)
	{
		if(g_layer_tbl[i].hsurf == g_surf_handle)
			break;
	}

	if(i < GUI_MAX_SURF)
	{
		prgn = gedraw_getdstrgn(i,g_rgn_idx);
		gedraw_clearclip(prgn);
	}
}


RET_CODE OSD_CreateRegion(UINT8 region_id,PGUI_RECT pRect,UINT32 param)
{
	UINT8 i;
	RET_CODE ret;
	PGUI_REGION prgn;

	ret = RET_FAILURE;
	for(i  = GUI_GMA1_SURF;i < GUI_MAX_SURF;i++)
	{
		if(g_layer_tbl[i].hsurf == g_surf_handle)
			break;
	}

	if(i < GUI_MAX_SURF)
	{
		prgn = gedraw_getdstrgn(i,region_id);
		prgn->color_mode = GUI_CLUT8;
		prgn->hsurf = (SURF_HANDLE)g_surf_handle;
		prgn->layer_type = i;
		prgn->rgn_id = region_id;
		MEMCPY(&prgn->rect,pRect,sizeof(GUI_RECT));
		ret = gedraw_creatergn((UINT32)prgn);

	}
	
	return ret;
}

RET_CODE OSD_DeleteRegion(UINT8 region_id)
{
	UINT8 i;
	RET_CODE ret;
	PGUI_REGION prgn;
	PGUI_LAYER psurf;

	ret = RET_FAILURE;
	for(i = GUI_GMA1_SURF; i < GUI_MAX_SURF;i++)
	{
		if(g_surf_handle == (SURF_HANDLE)g_layer_tbl[i].hsurf)
			break;
	}

	if(i < GUI_MAX_SURF)
	{
		prgn = gedraw_getdstrgn(i,region_id);
		ret = gedraw_deletergn(prgn);

		psurf = gedraw_getdstlayer(i);
		psurf->rgn_onlayer--;
		//if(0 == psurf->rgn_onlayer)
			//gedraw_releasesurf(psurf);
	}
	
	return ret;
}

void OSD_SetCurRegion(UINT8 uRegionID)
{
	g_rgn_idx = uRegionID;
}

UINT8 OSD_GetCurRegion()
{
	return g_rgn_idx;
}

void OSD_SetDeviceHandle(HANDLE dev)
{
	g_surf_handle = (SURF_HANDLE)dev;
}

void OSD_RegionInit()
{
	OSD_SetDeviceHandle((HANDLE)g_layer_tbl[GUI_GMA1_SURF].hsurf);
	OSD_SetCurRegion(0);
}
void OSD_RegionInit2()
{
	OSD_SetDeviceHandle((HANDLE)g_layer_tbl[GUI_GMA2_SURF].hsurf);
	OSD_SetCurRegion(0);
}
UINT8 *OSD_GetTaskVscrBuffer(ID task_id)
{
	return NULL;
}

void OSD_DrawFrame(PGUI_RECT rct,UINT32 color,PGUI_VSCR pvscr)
{
	UINT8 i;
	PGUI_REGION prgn;
	
	for(i  = GUI_GMA1_SURF;i < GUI_MAX_SURF;i++)
	{
		if(g_layer_tbl[i].hsurf == g_surf_handle)
			break;
	}
	
	if(NULL == pvscr)
	{
		prgn = gedraw_getdstrgn(i,g_rgn_idx);
	}
	else
	{
		prgn = gedraw_getdstrgn(pvscr->root_layer,pvscr->root_rgn);
	}
	gedraw_fillrect(prgn,rct,color);
}

