/*====================================================================
 *
 *  Copyright (C) 2008 ALi.  All Rights Reserved.
 *
 *  File:   gelib_effects.c
 *  History:      Build for new GElib by Shine Zhou@2010/03/09
 *********************************************************************/
#include <sys_config.h>
#include <api/libge/osd_lib.h>
#include <api/libge/osd_primitive.h>
#include <api/libge/ge_draw_primitive.h>
#include <api/libge/gecmd_primitive.h>
#include <api/libge/osd_common_draw.h>
#include <api/libge/osd_popup.h>

#include "gelib_effects.h"
/*##########################################################################*/
static UINT16 delayticks = 0;

#define GELIB_EFFECT_DELAY(uticks)	do{}while(0)
/*##########################################################################*/
UINT32 gelib_win_slideshow(PGUI_RECT rctslide,UINT32 param)
{
	UINT16 i,ugap,ysld,uflg,udelay;
	GUI_RECT rctgma,rctmem;
	PGUI_VSCR pVscr;
	PGUI_REGION pgma,prgn;
	PGUI_EFFCT_CFG pcfg;
	
	pcfg = (PGUI_EFFCT_CFG)(&param);	
	uflg = pcfg->type;
	ugap = MAX(2,pcfg->param2);
	udelay = MIN(4,pcfg->delay);
	
	pVscr = OSD_GetTaskVscr(osal_task_get_current_id());
	prgn = gelib_getdstrgn(pVscr->root_layer,pVscr->root_rgn);
	pgma = gelib_getdstrgn(pVscr->dst_layer,pVscr->dst_rgn);

	ysld = (rctslide->uStartY+rctslide->uHeight);
	MEMCPY(&rctgma,rctslide,sizeof(GUI_RECT));
	MEMCPY(&rctmem,rctslide,sizeof(GUI_RECT));
	for(i=ugap;i<=(rctslide->uHeight);i+=ugap)
	{
		if(uflg)//slide clear
		{
			rctgma.uHeight = i;
			rctgma.uStartY = rctslide->uStartY;
			gelib_fillrect(pgma,&rctgma,(transcolor_const[pgma->color_mode]));
			
			rctmem.uHeight = (rctslide->uHeight) -i;
			rctgma.uHeight = (rctslide->uHeight) -i;
			rctgma.uStartY = (rctslide->uStartY) + i;
		}
		else
		{
			rctgma.uHeight = i;
			rctgma.uStartY = ysld-i;
			rctmem.uHeight = i;
		}

		gelib_flush_rgn(pgma,prgn,&rctgma,&rctmem,0);
		GELIB_EFFECT_DELAY(0);
	}

	if(uflg)
	{
		rctgma.uHeight = (rctslide->uHeight);
		rctgma.uStartY = rctslide->uStartY;
		gelib_fillrect(pgma,&rctgma,(transcolor_const[pgma->color_mode]));
	}
	else
	{
		rctgma.uHeight = (rctslide->uHeight);
		rctgma.uStartY = rctslide->uStartY;
		rctmem.uHeight = (rctslide->uHeight);
		gelib_flush_rgn(pgma,prgn,&rctgma,&rctmem,0);
	}
	
	return 0;
}

UINT32 gelib_win_fadeout(UINT32 galpha,UINT32 param)
{
	UINT16 i,gvalue,ugap,uflg,udelay;
	PGUI_VSCR pVscr;
	PGUI_EFFCT_CFG pcfg;
	
	pcfg = (PGUI_EFFCT_CFG)(&param);
	uflg = pcfg->type;
	ugap = MAX(2,pcfg->param2);
	udelay = MAX(2,pcfg->delay);

	if(uflg)//fade in
		gvalue = galpha;
	else
		gvalue = 0;
	pVscr = OSD_GetTaskVscr(osal_task_get_current_id());
	gelib_setgalpha(pVscr->dst_layer,gvalue);
	
	OSD_SetVscrModified(pVscr);
	OSD_UpdateVscr(pVscr);
    
	for(i=0;i<=galpha;i += ugap)
	{
		gelib_setgalpha(pVscr->dst_layer,gvalue);
		if(uflg)
			gvalue = (gvalue>ugap)?(gvalue-ugap):0;
		else
			gvalue = ((gvalue+ugap)>0xff)?0xff:(gvalue+ugap);

		GELIB_EFFECT_DELAY(udelay);
	}
	gelib_setgalpha(pVscr->dst_layer,(UINT8)gvalue);
	
	return 0;
}
/*##########################################################################*/
#define ABS(x) ((x) < 0 ? -(x) : (x))
UINT32 gelib_item_focuslide(POBJECT_HEAD psobj,POBJECT_HEAD ptobj,UINT32 param)
{//start and terminal point
	UINT16 i,yy,pyy,tyy,byy,hy,urevert,uborder;
	GUI_RECT rcvscr,rctop,rctborder,*psrct,*ptrct;
	PGUI_VSCR pvscr;
	PGUI_REGION pgma,pmem;

	psrct = &psobj->frame;
	ptrct = &ptobj->frame;
	if(gui_rctsize(psrct,ptrct) == 0)
		return (UINT32)(-1);//do not support focus slide
	else if(ge_version <= GE_MODULE_M3329D)
		return (UINT32)(-2);

	urevert = 0;
	uborder = 0;
	if((psrct->uStartX) != (ptrct->uStartX))
		return (UINT32)(-3);//do not support horizontal focus slide
	else if(ABS((psobj->bID) - (ptobj->bID))>1)
		return (UINT32)(-4);//uborder = 1;//return (UINT32)(-4);
	else if((psrct->uStartY)>(ptrct->uStartY))
		urevert = 1;
	
	hy = (ptrct->uHeight);
	tyy = MIN((psrct->uStartY),(ptrct->uStartY));
	byy = MAX((psrct->uStartY),(ptrct->uStartY))+(ptrct->uHeight);
	rcvscr.uStartX = (ptrct->uStartX);
	rcvscr.uStartY = tyy;
	rcvscr.uWidth = (ptrct->uWidth);
	rcvscr.uHeight = (byy - tyy);
	rctop.uHeight = hy;
	rctop.uStartX = (ptrct->uStartX);
	rctop.uWidth = (ptrct->uWidth);

	pvscr = OSD_GetTaskVscr(osal_task_get_current_id());
	OSD_SetVscrModified(pvscr);
	OSD_UpdateVscr(pvscr);	
	OSD_SetVscrSuspend(pvscr);
	
	pmem = gelib_getdstrgn(pvscr->root_layer,pvscr->root_rgn);
	pgma = gelib_getdstrgn(pvscr->dst_layer,pvscr->dst_rgn);

	if(uborder == 1)
	{
		if((psrct->uStartY)<(ptrct->uStartY))
			urevert = 1;
		else
			urevert = 0;
		
		rctop.uStartY = (psrct->uStartX);
		rctop.uStartY = (psrct->uStartY);
		rctop.uWidth = (psrct->uWidth);
		rctop.uHeight = (psrct->uHeight);
		rctborder.uStartX = (ptrct->uStartX);
		rctborder.uStartY = (ptrct->uStartY);
		rctborder.uWidth = (ptrct->uWidth);
		rctborder.uHeight = (ptrct->uHeight);
		for(i = 0;i < (rctop.uHeight); i += 2)
		{
			if(urevert)
			{
				rctop.uHeight -= 2;
				rctborder.uStartY = (ptrct->uStartY + ptrct->uHeight -i);
			}
			else
			{
				rctop.uStartY += 2;
				rctop.uHeight -= 2;
			}
			
			rctborder.uHeight = i;
			OSD_DrawStyleRect(&rctop,psobj->style.bHLIdx,pvscr);
			OSD_DrawStyleRect(&rctborder,ptobj->style.bHLIdx,pvscr);
			
			OSD_DrawObject(psobj,C_UPDATE_CONTENT);
			OSD_DrawObject(ptobj,C_UPDATE_CONTENT);
			gelib_flush_rgn(pgma,pmem,&rcvscr,&rcvscr,0);
			
			OSD_DrawStyleRect(&rctop,psobj->style.bShowIdx,pvscr);
			OSD_DrawStyleRect(&rctborder,ptobj->style.bShowIdx,pvscr);
			GELIB_EFFECT_DELAY(0);
		}
	}
	else
	{
		for(i = tyy;i < (byy-hy); i += 2)
		{
			if(urevert)
				rctop.uStartY = (byy-hy-i + tyy);
			else
				rctop.uStartY = i;
			OSD_DrawStyleRect(&rctop,psobj->style.bHLIdx,pvscr);
			
			OSD_DrawObject(psobj,C_UPDATE_CONTENT);
			OSD_DrawObject(ptobj,C_UPDATE_CONTENT);
			gelib_flush_rgn(pgma,pmem,&rcvscr,&rcvscr,0);
			
			OSD_DrawStyleRect(&rctop,psobj->style.bShowIdx,pvscr);
			GELIB_EFFECT_DELAY(0);
		}
	}
	
	pvscr = OSD_GetVscr(&rcvscr,VSCR_NULL);	
	OSD_DrawObject(psobj,C_UPDATE_ALL);
	OSD_TrackObject(ptobj,C_UPDATE_ALL);
	OSD_ClearVscrSuspend(pvscr);
	OSD_SetVscrModified(pvscr);
	OSD_UpdateVscr(pvscr);
	
	return 0;
}

UINT32 gelib_olst_focuslide(POBJECT_HEAD psobj,POBJECT_HEAD ptobj,UINT32 param)
{//start and terminal point
	UINT16 i,yy,pyy,tyy,byy,hy,urevert,uborder;
	GUI_RECT rcvscr,rctop,rctborder,*psrct,*ptrct;
	PGUI_VSCR pvscr;
	PGUI_REGION pgma,pmem;

	psrct = &psobj->frame;
	ptrct = &ptobj->frame;
	if(gui_rctsize(psrct,ptrct) == 0)
		return (UINT32)(-1);//do not support focus slide
	else if(ge_version <= GE_MODULE_M3329D)
		return (UINT32)(-2);

	urevert = 0;
	uborder = 0;
	if((psrct->uStartX) != (ptrct->uStartX))
		return (UINT32)(-3);//do not support horizontal focus slide
	else if(ABS((psobj->bID) - (ptobj->bID))>1)
		return (UINT32)(-4);//uborder = 1;//return (UINT32)(-4);
	else if((psrct->uStartY)>(ptrct->uStartY))
		urevert = 1;
	
	hy = (ptrct->uHeight);
	tyy = MIN((psrct->uStartY),(ptrct->uStartY));
	byy = MAX((psrct->uStartY),(ptrct->uStartY))+(ptrct->uHeight);
	rcvscr.uStartX = (ptrct->uStartX);
	rcvscr.uStartY = tyy;
	rcvscr.uWidth = (ptrct->uWidth);
	rcvscr.uHeight = (byy - tyy);
	rctop.uHeight = hy;
	rctop.uStartX = (ptrct->uStartX);
	rctop.uWidth = (ptrct->uWidth);

	pvscr = OSD_GetTaskVscr(osal_task_get_current_id());
	OSD_SetVscrModified(pvscr);
	OSD_UpdateVscr(pvscr);	
	OSD_SetVscrSuspend(pvscr);
	
	pmem = gelib_getdstrgn(pvscr->root_layer,pvscr->root_rgn);
	pgma = gelib_getdstrgn(pvscr->dst_layer,pvscr->dst_rgn);

	if(uborder == 1)
	{
		if((psrct->uStartY)<(ptrct->uStartY))
			urevert = 1;
		else
			urevert = 0;
		
		rctop.uStartY = (psrct->uStartX);
		rctop.uStartY = (psrct->uStartY);
		rctop.uWidth = (psrct->uWidth);
		rctop.uHeight = (psrct->uHeight);
		rctborder.uStartX = (ptrct->uStartX);
		rctborder.uStartY = (ptrct->uStartY);
		rctborder.uWidth = (ptrct->uWidth);
		rctborder.uHeight = (ptrct->uHeight);
		for(i = 0;i < (rctop.uHeight); i += 2)
		{
			if(urevert)
			{
				rctop.uHeight -= 2;
				rctborder.uStartY = (ptrct->uStartY + ptrct->uHeight -i);
			}
			else
			{
				rctop.uStartY += 2;
				rctop.uHeight -= 2;
			}
			
			rctborder.uHeight = i;
			OSD_DrawStyleRect(&rctop,psobj->style.bHLIdx,pvscr);
			OSD_DrawStyleRect(&rctborder,ptobj->style.bHLIdx,pvscr);
			
			OSD_DrawObject(psobj,C_UPDATE_CONTENT);
			OSD_DrawObject(ptobj,C_UPDATE_CONTENT);
			gelib_flush_rgn(pgma,pmem,&rcvscr,&rcvscr,0);
			
			OSD_DrawStyleRect(&rctop,psobj->style.bShowIdx,pvscr);
			OSD_DrawStyleRect(&rctborder,ptobj->style.bShowIdx,pvscr);
			GELIB_EFFECT_DELAY(0);
		}
	}
	else
	{
		for(i = tyy;i < (byy-hy); i += 2)
		{
			if(urevert)
				rctop.uStartY = (byy-hy-i + tyy);
			else
				rctop.uStartY = i;
			OSD_DrawStyleRect(&rctop,psobj->style.bHLIdx,pvscr);
			
			OSD_DrawObject(psobj,C_UPDATE_CONTENT);
			OSD_DrawObject(ptobj,C_UPDATE_CONTENT);
			gelib_flush_rgn(pgma,pmem,&rcvscr,&rcvscr,0);
			
			OSD_DrawStyleRect(&rctop,psobj->style.bShowIdx,pvscr);
			GELIB_EFFECT_DELAY(0);
		}
	}
	
	pvscr = OSD_GetVscr(&rcvscr,VSCR_NULL); 
	OSD_DrawObject(psobj,C_UPDATE_ALL);
	OSD_TrackObject(ptobj,C_UPDATE_ALL);
	OSD_ClearVscrSuspend(pvscr);
	OSD_SetVscrModified(pvscr);
	OSD_UpdateVscr(pvscr);
	
	return 0;
}

/*##########################################################################*/

