/*-----------------------------------------------------------------------------
    Copyright (C) 2010 ALi Corp. All rights reserved.
    File: obj_XXX.c
-----------------------------------------------------------------------------*/
#include <sys_config.h>

#include <api/libc/string.h>
#include <api/libge/osd_lib.h>
#include <api/libge/osd_common_draw.h>
#include <api/libge/osd_primitive.h>
#include "osd_lib_internal.h"

#define BAR_PRINTF(...)

void OSD_SetProgressBarTick(PPROGRESS_BAR b, UINT16 wTickBg, UINT16 wTickFg)
{
	b->wTickBg =wTickBg;
	b->wTickFg= wTickFg;
}
void OSD_SetProgressBarRange(PPROGRESS_BAR b, INT16 nMin, INT16 nMax)
{
	b->nMin=nMin;
	b->nMax=nMax;
}

BOOL OSD_SetProgressBarValue(PPROGRESS_BAR b, INT16 value)
{
	if((value <= b->nMax) && (value >= b->nMin))
	{
		b->nPos = value;
		return TRUE;
	}
	else
		return FALSE;
}

static BOOL OSD_ProgressBarCheckValid(PPROGRESS_BAR b)
{
	if(b->nMin<0||b->nMax<0||b->nBlocks<0)
		return FALSE;
	if(b->nMin>b->nMax)
		return FALSE;
	return TRUE;
}
static void OSD_ProgressBarGetMidRect(PPROGRESS_BAR b,GUI_RECT* MidRect)
{
	MidRect->uStartX  =b->head.frame.uStartX+b->rcBar.uStartX;
	MidRect->uStartY=b->head.frame.uStartY+b->rcBar.uStartY;
	MidRect->uWidth=b->rcBar.uWidth;
	MidRect->uHeight=b->rcBar.uHeight;

	if(MidRect->uStartX>b->head.frame.uStartX+b->head.frame.uWidth)
		MidRect->uStartX =b->head.frame.uStartX+b->head.frame.uWidth;
	if(MidRect->uStartY>b->head.frame.uStartY+b->head.frame.uHeight)
		MidRect->uStartY =b->head.frame.uStartY+b->head.frame.uHeight;
	if(MidRect->uStartX+MidRect->uWidth>b->head.frame.uStartX+b->head.frame.uWidth)
		MidRect->uWidth =b->head.frame.uStartX+b->head.frame.uWidth-MidRect->uStartX;
	if(MidRect->uStartY+MidRect->uHeight>b->head.frame.uStartY+b->head.frame.uHeight)
		MidRect->uHeight =b->head.frame.uStartY+b->head.frame.uHeight-MidRect->uStartY;
}

void OSD_DrawProgressBarTick(PPROGRESS_BAR b,PGUI_RECT r, UINT16 bg_idx,PGUI_VSCR pVscr)
{
	UINT16	wIconWidth = 0, wIconHeight = 0;
	if(PBAR_STYLE_RECT_STYLE & b->bStyle)
		OSD_DrawStyleRect(r, b->wTickFg, pVscr);//ERR_LIBGE,b->wTickBg is a style not color
	else
	{
		if(OSD_GetLibInfoByWordIdx(b->wTickFg, LIB_ICON, &wIconWidth, &wIconHeight))//get mid rect ,check if icon
			OSDDrawPicSolidRectangle(pVscr, r, b->wTickFg,C_MIXER);//ERR_LIBGE,b->wTickBg is a style not color
		else//color index
			OSD_DrawRect2Vscr(r,b->wTickFg,pVscr);//ERR_LIBGE,b->wTickBg is a style not color
	}
}

void OSD_DrawProgressBarCell(PPROGRESS_BAR b,UINT8 bStyleIdx,UINT32 nCmdDraw)
{ 
	PGUI_VSCR		pVscr;
	INT16 	wLeft, wDig,wDigWidth,wDigHeight,wCount;
	GUI_RECT	r,iconrect;
	GUI_RECT MidRect;
	PWINSTYLE  	lpWinSty;
	UINT32 pbarstyle;
	UINT16 wIconWidth,wIconHeight;
	INT32 start_pos,end_pos,page_width,reverse;
	
	wIconWidth = 0;
	wIconHeight = 0;
	lpWinSty = g_gui_rscfunc.osd_get_win_style(bStyleIdx);
	if(OSD_GetUpdateType(nCmdDraw) == C_UPDATE_CONTENT)
		pbarstyle = (UINT32)(~0);
	else
		pbarstyle = bStyleIdx;
	
	OSD_SetRect2(&r, &b->head.frame);
	if(OSD_GetAttrSlave(b))
		pVscr=OSD_DrawSlaveFrame(&r, bStyleIdx);
	else
		pVscr=OSD_DrawObjectFrame(&r, pbarstyle);
	OSD_ProgressBarGetMidRect(b,&MidRect);
	OSD_DrawStyleRect(&MidRect, b->wTickBg, pVscr);//ERR_LIBGE,b->wTickBg is a style not color
	
	if(!OSD_ProgressBarCheckValid(b))//check data 
		return;

	if((GetProgressBarStyle(b) & PROGRESSBAR_HORI_NORMAL)||(GetProgressBarStyle(b) & PROGRESSBAR_HORI_REVERSE))
	{
		if(b->bX==0)//analog bar
		{
			if(b->nPos<b->nMin)
				wDigWidth = 0;
			else
				wDigWidth = MidRect.uWidth*(b->nPos-b->nMin)/(b->nMax-b->nMin);

			if(GetProgressBarStyle(b)&PROGRESSBAR_HORI_NORMAL)
				OSD_SetRect(&iconrect,MidRect.uStartX,MidRect.uStartY,wDigWidth,MidRect.uHeight);
			else //reverse type
				OSD_SetRect(&iconrect,MidRect.uStartX+MidRect.uWidth-wDigWidth,MidRect.uStartY,wDigWidth,MidRect.uHeight);

			OSD_DrawProgressBarTick(b, &iconrect, lpWinSty->wBgIdx, pVscr);
		}
		else//digital bar
		{
			INT32 i ;
			if((b->nBlocks-1)*b->bX>=MidRect.uWidth)// Interval width is too large
				return;
			wDig = MidRect.uWidth/b->nBlocks;//wDig is the total tick width
			if(wDig<=1)
				return;
			wDigWidth = (MidRect.uWidth-(b->nBlocks-1)*b->bX)/b->nBlocks;//wDigWidth is the width for Foreback winstyle draw
			if(wDigWidth==0)
				wDigWidth =1;//at least is 1
			wCount = b->nMax-b->nMin;
			if(GetProgressBarStyle(b)&PROGRESSBAR_HORI_NORMAL)
			{
				for(i = b->nMin,wLeft=MidRect.uStartX; (i < b->nPos)&&(wLeft<MidRect.uStartX+MidRect.uWidth); wLeft += wDig, i += wCount/b->nBlocks)
				{/*i += wCount/b->nBlocks may cause divisibility problem,So we should assure right blocks,nMax,nMin value when using digital bar*/
					if(wLeft+wDigWidth-wDig>MidRect.uStartX+MidRect.uWidth)
						OSD_SetRect(&iconrect,wLeft,MidRect.uStartY,MidRect.uStartX+MidRect.uWidth-wLeft,MidRect.uHeight);
					else if (wLeft+wDigWidth>MidRect.uStartX+MidRect.uWidth)
						OSD_SetRect(&iconrect,wLeft,MidRect.uStartY,MidRect.uStartX+MidRect.uWidth-wLeft,MidRect.uHeight);
					else
						OSD_SetRect(&iconrect,wLeft,MidRect.uStartY,wDigWidth,MidRect.uHeight);

					OSD_DrawProgressBarTick(b, &iconrect, lpWinSty->wBgIdx, pVscr);
				}
			}
			else//PROGRESSBAR_HORI_REVERSE
			{
				wLeft = MidRect.uStartX+MidRect.uWidth-wDig;
				for(i = b->nMin,wLeft=MidRect.uStartX+MidRect.uWidth-wDig; (i < b->nPos)&&(wLeft >MidRect.uStartX); wLeft -= wDig, i += wCount/b->nBlocks)
				{
					if(wLeft+wDigWidth-wDig<MidRect.uStartX)
						OSD_SetRect(&iconrect,MidRect.uStartX,MidRect.uStartY,wLeft-MidRect.uStartX,MidRect.uHeight);
					else if (wLeft-wDigWidth<MidRect.uStartX)
						OSD_SetRect(&iconrect,MidRect.uStartX,MidRect.uStartY,wLeft-MidRect.uStartX,MidRect.uHeight);
					else
						OSD_SetRect(&iconrect,wLeft,MidRect.uStartY,wDigWidth,MidRect.uHeight);

					OSD_DrawProgressBarTick(b, &iconrect, lpWinSty->wBgIdx, pVscr);
				}
			}
		}
	}
	else
	{
		if(b->bY==0)//analog bar
		{
			if(b->nPos<b->nMin)
				wDigHeight = 0;
			else
				wDigHeight = MidRect.uHeight*(b->nPos-b->nMin)/(b->nMax-b->nMin);
			if(GetProgressBarStyle(b)&PROGRESSBAR_VERT_NORMAL)
				OSD_SetRect(&iconrect,MidRect.uStartX,MidRect.uStartY,MidRect.uWidth,wDigHeight);
			else
				OSD_SetRect(&iconrect,MidRect.uStartX,MidRect.uStartY+MidRect.uHeight-wDigHeight,MidRect.uWidth,wDigHeight);				

			OSD_DrawProgressBarTick(b, &iconrect, lpWinSty->wBgIdx, pVscr);
		}
		else//digital bar
		{
			INT32 i ;
			INT32 wRectTop;
			if((b->nBlocks-1)*b->bY>=MidRect.uHeight)//Interval width is too large
				return;
			wDig = MidRect.uHeight/b->nBlocks;
			if(wDig<=1)
				return;
			wDigHeight = (MidRect.uHeight-(b->nBlocks-1)*b->bY)/b->nBlocks;
			if(wDigHeight==0)
				wDigHeight=1;
			wCount = b->nMax-b->nMin;
			if(GetProgressBarStyle(b)&PROGRESSBAR_VERT_NORMAL)
			{
				for(i = b->nMin,wRectTop=MidRect.uStartY; (i < b->nPos)&&(wRectTop<MidRect.uStartY+MidRect.uHeight); wRectTop += wDig, i += wCount/b->nBlocks)
				{
					if(wRectTop+wDig-wDigHeight>MidRect.uStartY+MidRect.uHeight)
						OSD_SetRect(&iconrect,MidRect.uStartX,wRectTop,MidRect.uWidth,MidRect.uStartY+MidRect.uHeight-wRectTop);
					else if (wRectTop+wDigHeight>MidRect.uStartY+MidRect.uHeight)
						OSD_SetRect(&iconrect,MidRect.uStartX,wRectTop,MidRect.uWidth,MidRect.uStartY+MidRect.uHeight-wRectTop);
					else
						OSD_SetRect(&iconrect,MidRect.uStartX,wRectTop,MidRect.uWidth,wDigHeight);
					OSD_DrawProgressBarTick(b, &iconrect, lpWinSty->wBgIdx, pVscr);
				}
			}
			else
			{
				for(i = b->nMin,wRectTop=MidRect.uStartY+MidRect.uHeight-wDig; (i < b->nPos)&&(wRectTop>MidRect.uStartY); wRectTop -= wDig, i += wCount/b->nBlocks)
				{
					if(wRectTop+wDigHeight-wDig<MidRect.uStartY)
						OSD_SetRect(&iconrect,MidRect.uStartX,MidRect.uStartY,MidRect.uWidth,wRectTop-MidRect.uStartY);
					else if(wRectTop-wDigHeight<MidRect.uStartY)
						OSD_SetRect(&iconrect,MidRect.uStartX,MidRect.uStartY,MidRect.uWidth,wRectTop-MidRect.uStartY);
					else
						OSD_SetRect(&iconrect,MidRect.uStartX,wRectTop,MidRect.uWidth,wDigHeight);
					OSD_DrawProgressBarTick(b, &iconrect, lpWinSty->wBgIdx, pVscr);
				}
			}
		}
	}
}

static UINT16 OSD_ShiftBarPoint(PROGRESS_BAR* b, INT16  bShift)
{
	//BAR_PRINTF("(%s) bMultiple:%x\n", __FUNCTION__, bMultiple);
	//BAR_PRINTF("(%s) bMultiple:%x\n", __FUNCTION__, bMultiple);
	if(bShift>=0)
		if((b->nPos + bShift) < b->nMax && b->nMax > b->nMin)
			b->nPos += bShift;
		else if((b->nPos - bShift) > b->nMax && b->nMax < b->nMin)
			b->nPos -= bShift;
		else
			b->nPos = b->nMax;
	else
		if((b->nPos + bShift) > b->nMin && b->nMax > b->nMin)
			b->nPos += bShift;


		else if((b->nPos - bShift) < b->nMin && b->nMax < b->nMin)
			b->nPos -= bShift;
		else
			b->nPos = b->nMin;

	return b->nPos;
}

VACTION OSD_ProgressBarKeyMap(POBJECT_HEAD pObj,	UINT32 vkey)
{
	VACTION        Vact = VACT_PASS;
	PROGRESS_BAR* b = (PROGRESS_BAR*)pObj;
	UINT8 bStyle = GetProgressBarStyle(b);
	switch(vkey)
	{
	case V_KEY_UP:
		if(bStyle&PROGRESSBAR_VERT_NORMAL)
			Vact = VACT_DECREASE;
		if(bStyle&PROGRESSBAR_VERT_REVERSE)
			Vact = VACT_INCREASE;
		break;
	case V_KEY_DOWN:
		if(bStyle&PROGRESSBAR_VERT_NORMAL)
			Vact = VACT_INCREASE;
		if(bStyle&PROGRESSBAR_VERT_REVERSE)
			Vact = VACT_DECREASE;
		break;
	case V_KEY_RIGHT:
		if(bStyle&PROGRESSBAR_HORI_NORMAL)
			Vact = VACT_INCREASE;
		if(bStyle&PROGRESSBAR_HORI_REVERSE)
			Vact = VACT_DECREASE;
		break;
	case V_KEY_LEFT:
		if(bStyle&PROGRESSBAR_HORI_NORMAL)
			Vact = VACT_DECREASE;
		if(bStyle&PROGRESSBAR_HORI_REVERSE)
			Vact = VACT_INCREASE;
		break;
	default:
		break;
	}
	return Vact;
}
	
PRESULT OSD_ProgressBarProc(POBJECT_HEAD pObj, UINT32 msg_type, UINT32 msg,UINT32 param1)
{
	PRESULT Result = PROC_PASS;
	PROGRESS_BAR* b = (PROGRESS_BAR*)pObj;
	INT16 len,shift;
	
	if(msg_type==MSG_TYPE_KEY)
	{
		VACTION Action;
		UINT32 vkey;
		BOOL bContinue;
    	
		Result = OSD_ObjCommonProc(pObj,msg,param1,&vkey,&Action,&bContinue);
		if(!bContinue)
	    		goto CHECK_LEAVE;
	    	
		len = (b->nMax - b->nMin)/b->nBlocks/*nStep*/;

		switch(Action)
		{
		case VACT_INCREASE:
		case VACT_DECREASE:
		{
			INT16	cur_pos = b->nPos;
			if(Action == VACT_INCREASE)
				shift = len;
			else
				shift = -len;				
										
			OSD_ShiftBarPoint(b, shift);
			Result = OSD_SIGNAL(pObj,EVN_PRE_CHANGE,(UINT32)&(b->nPos),cur_pos);
			if(Result!=PROC_PASS)
				goto CHECK_LEAVE;
				
			OSD_TrackObject(pObj, C_DRAW_SIGN_EVN_FLG);			
			Result = OSD_SIGNAL(pObj,EVN_POST_CHANGE,b->nPos,cur_pos);
			if(Result!=PROC_PASS)
				goto CHECK_LEAVE;
			
			Result = PROC_LOOP;							
			break;
		}
		case VACT_CLOSE:
CLOSE_OBJECT:			
    		Result = OSD_ObjClose(pObj,C_CLOSE_CLRBACK_FLG);
    		goto EXIT;
    		break;			
		default:
			Result = OSD_SIGNAL(pObj, EVN_UNKNOWN_ACTION, (Action<<16) | vkey, param1);
			break;
		}
	}
	else// if(msg_type==MSG_TYPE_EVNT)
		Result = OSD_SIGNAL(pObj, msg_type, msg, param1);
	
CHECK_LEAVE:
	if(!(EVN_PRE_OPEN == msg_type))
		CHECK_LEAVE_RETURN(Result,pObj);
	
EXIT:		
	return Result;
}

