/*====================================================================
 *
 *  Copyright (C) 2008 ALi.  All Rights Reserved.
 *
 *  File:   osd_popup.c
 *
 *  Contents:   Add popup interface for both Messge notify dialog and modal item select dialog etc.
 *  History:      Build for new GElib by Shine Zhou@2008/07/31
 *********************************************************************/
#include <sys_config.h>
#include <api/libge/osd_lib.h>
#include <api/libge/osd_window.h>
#include <api/libge/osd_primitive.h>
#include <api/libge/ge_draw_primitive.h>
#include <api/libge/osd_common_draw.h>
#include <api/libge/osd_popup.h>
#include "osd_lib_internal.h"

extern UINT8	*vscr_buff;
extern UINT8	*pop_buff;
GEDRAW_MSGPOPUP_BLEND gedraw_up2scrn_cb;

void OSD_RectBackUp(PGUI_VSCR lpVscr,UINT8 *buf,PGUI_RECT pRc)
{
	gedraw_backup_restore(lpVscr,buf,pRc,TRUE);
}

void OSD_RectRestore(PGUI_VSCR lpVscr,UINT8 *buf,PGUI_RECT pRc)
{
	gedraw_backup_restore(lpVscr,buf,pRc,FALSE);
	lpVscr->dirty_flag = 0;
	OSD_SetRect(&lpVscr->frm,0,0,0,0);
}

RET_CODE OSD_UpdateMsgPopup()
{
	UINT8 rgn_id;
	RET_CODE ret;
	PGUI_REGION pdst,prgn,ppop;

	rgn_id = 0;//reserved param
	ret = RET_FAILURE;
	ppop = gelib_getdstrgn((UINT8)GUI_MEMPOP_SURF,(UINT8)0);
	if(NULL != (void *)ppop->hsurf)
	{
		pdst = gelib_getdstrgn((UINT8)GUI_GMA1_SURF,(UINT8)rgn_id);//blending on the first layer
		if(vscr_buff == NULL)
		{
			prgn = pdst;
		}
		else
		{
			prgn = gelib_getdstrgn((UINT8)GUI_MEMVSCR_SURF,(UINT8)0);
		}
			
		ret = gelib_blend_rgn(pdst,prgn,ppop,NULL,NULL,0);
		OSD_Attach_Popcb();
	}

	return ret;
}

RET_CODE OSD_ClearMsgPopup()
{
	UINT8 rgn_id;
	PGUI_REGION pdst,prgn,ppop;
	
	rgn_id = 0;//reserved param
	OSD_Detach_Popcb();
	ppop = gelib_getdstrgn((UINT8)GUI_MEMPOP_SURF,(UINT8)0);
	if(NULL != (void *)ppop->hsurf)
	{
		pdst = gelib_getdstrgn((UINT8)GUI_GMA1_SURF,(UINT8)rgn_id);//blending on the first layer
		if(vscr_buff == NULL)
		{
			return RET_FAILURE;//can not restore back UI
		}
		else
		{
			prgn = gelib_getdstrgn((UINT8)GUI_MEMVSCR_SURF,(UINT8)0);
		}

		gelib_fillrect(ppop,NULL,transcolor_const[ppop->color_mode]);
		return gelib_blend_rgn(pdst,prgn,ppop,NULL,NULL,0);
	}
	else
	{
		return RET_FAILURE;
	}
}

RET_CODE OSD_ModalPopupInit(UINT32 param)
{
	UINT8 rgn_id;
	GUI_RECT rctpop;
	PGUI_LAYER_CFG pcfg;
	PGUI_REGION psrc,ppop;
	
	if(app_popup_init(param)>0)
     return RET_FAILURE;
	rgn_id = 0;
	OSD_Detach_Popcb();
	//app_popup_init(param);
	
	pcfg = (PGUI_LAYER_CFG)param;
	if(ge_version != GE_MODULE_M3602)
		psrc = gelib_getdstrgn((UINT8)GUI_GMA1_SURF,(UINT8)rgn_id);
	else if(pcfg->slvgma == 1)
		psrc = gelib_getdstrgn(GUI_SLVGMA_SURF,rgn_id);
	else
		psrc = gelib_getdstrgn(GUI_MEMVSCR_SURF,rgn_id);
	
	if(pcfg->slvgma == 0)
		ppop = gelib_getdstrgn((UINT8)GUI_MEMPOP_SURF,0);
	else
		ppop = gelib_getdstrgn((UINT8)GUI_SLVPOP_SURF,0);
	
	rctpop.uStartX = rctpop.uStartY = 0;
	rctpop.uWidth = ppop->rect.uWidth;
	rctpop.uHeight = ppop->rect.uHeight;//popup region relative position
	gelib_flush_rgn(ppop,psrc,&rctpop,&ppop->rect,0);

	return RET_SUCCESS;
}

RET_CODE OSD_ModalPopupRelease()
{
	UINT8 rgn_id;
	GUI_RECT rctpop;
	PGUI_REGION ppop,pdst,pmvsr,pslv,pslvpop;

	rgn_id = 0;
	pmvsr = NULL;
	if(ge_version == GE_MODULE_M3602)
		pdst = gelib_getdstrgn(GUI_MEMVSCR_SURF,rgn_id);
	else
	{
		pdst = gelib_getdstrgn((UINT8)GUI_GMA1_SURF,(UINT8)rgn_id);
		pmvsr = gelib_getdstrgn((UINT8)GUI_MEMVSCR_SURF,(UINT8)rgn_id);
	}
	
	ppop = gelib_getdstrgn((UINT8)GUI_MEMPOP_SURF,(UINT8)0);
	rctpop.uStartX = rctpop.uStartY = 0;
	rctpop.uWidth = ppop->rect.uWidth;
	rctpop.uHeight = ppop->rect.uHeight;
	if(NULL != (void*)ppop->hsurf)
	{
		gelib_flush_rgn(pdst,ppop,&ppop->rect,&rctpop,0);
	}
	
	if(ge_version == GE_MODULE_M3602)
	{
		GUI_VSCR mvsr;

		MEMSET(&mvsr,0x0,sizeof(mvsr));
		mvsr.dirty_flag = 1;
		mvsr.root_rgn = 0;
		mvsr.dst_rgn = 0;
		mvsr.dst_layer = GUI_GMA2_SURF;
		mvsr.root_layer = GUI_MEMVSCR_SURF;
		mvsr.color_mode = pdst->color_mode;

		mvsr.lpbuff = vscr_buff;
		mvsr.frm.uStartX = ppop->rect.uStartX;
		mvsr.frm.uStartY = ppop->rect.uStartY;
		mvsr.frm.uWidth = ppop->rect.uWidth;
		mvsr.frm.uHeight = ppop->rect.uHeight;
		gelib_update2scrn(&mvsr,&(mvsr.frm));
	}
	else if(NULL != (void*)ppop->hsurf)
	{
		gelib_flush_rgn(pmvsr,ppop,&ppop->rect,&rctpop,0);
	}

	if(gelib_getdual_support())
	{
		pslv = gelib_getdstrgn(GUI_SLVGMA_SURF,rgn_id);
		pmvsr = gelib_getdstrgn(GUI_SLVSCR_SURF,0);
		pslvpop = gelib_getdstrgn((UINT8)GUI_SLVPOP_SURF,(UINT8)0);
		rctpop.uStartX = rctpop.uStartY = 0;
		rctpop.uWidth = pslvpop->rect.uWidth;
		rctpop.uHeight = pslvpop->rect.uHeight;//popup region relative position
		if((NULL != (void*)pslvpop->hsurf) && (NULL != (void*)pslv->hsurf))
		{
			gelib_flush_rgn(pslv,pslvpop,&pslvpop->rect,&rctpop,0);
			gelib_flush_rgn(pmvsr,pslvpop,&pslvpop->rect,&rctpop,0);
		}
	}
	
	app_popup_release();
	return RET_SUCCESS;
}

static RET_CODE OSD_PopupBlend(UINT32 param)
{
	RET_CODE	ret;
	GUI_RECT	cross_rct;
	PGUI_VSCR	pvscr;
	PGUI_REGION	ppop,pdst,prgn;

	ret = RET_FAILURE;
	pvscr = (PGUI_VSCR)param;
	ppop = gelib_getdstrgn((UINT8)GUI_MEMPOP_SURF,(UINT8)0);
	prgn = gelib_getdstrgn((UINT8)pvscr->root_layer,(UINT8)pvscr->root_rgn);
	pdst = gelib_getdstrgn((UINT8)pvscr->dst_layer,(UINT8)pvscr->dst_rgn);
	
	if((GUI_MEMVSCR_SURF == pvscr->root_layer) &&  (NULL != (void *)ppop->hsurf))
	{
		MEMSET(&cross_rct,0x0,sizeof(cross_rct));
		OSD_GetRectsCross(&ppop->rect,&pvscr->frm,&cross_rct);
		if((0 != cross_rct.uHeight) && (0 != cross_rct.uWidth))
		{
			ge_alpha_blend_t alpha_cfg;

			MEMSET(&alpha_cfg,0,sizeof(alpha_cfg));
			alpha_cfg.glob_alpha = 0;
			alpha_cfg.glob_alpha_en = 0;
			alpha_cfg.neg_alpha = 0;
			alpha_cfg.src_mode = GE_AB_SRC_NOT_PREMULTI;
			ret = gelib_blend_rgn(pdst,prgn,ppop,&cross_rct,&cross_rct,(UINT32)&alpha_cfg);

			gelib_setclip(pdst,&cross_rct,0);
		}
	}//pre-blending

	return ret;
}

void OSD_Attach_Popcb()
{
	gedraw_up2scrn_cb = OSD_PopupBlend;
}

void OSD_Detach_Popcb()
{
	gedraw_up2scrn_cb = NULL;
}

