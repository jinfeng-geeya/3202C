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

static void OSD_ScrollBarGetMidRect(PSCROLL_BAR b,GUI_RECT* MidRect)
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


void GetThumbRect(PSCROLL_BAR b, PGUI_RECT pRect)
{
	UINT32 orient;
	GUI_RECT mid_rect;
	UINT16 bw,bh;
	PWINSTYLE  	lpWinSty;
	UINT16 minLoca, maxLoca, minThumbSize,start_pos,end_pos,page_width;

	OSD_ScrollBarGetMidRect(b,&mid_rect);

	if ( (b->bStyle & BAR_HORI_AUTO) || (b->bStyle & BAR_HORI_ORIGINAL))
	{
		minLoca = mid_rect.uStartX;
		maxLoca = mid_rect.uStartX + mid_rect.uWidth;
		orient = 0;
	}
	else// if ( (b->bStyle & BAR_VERT_AUTO) || (b->bStyle & BAR_VERT_ORIGINAL) )
	{
		minLoca = mid_rect.uStartY;
		maxLoca = mid_rect.uStartY + mid_rect.uHeight;
		orient = 1;
	}

	if(OSD_GetLibInfoByWordIdx(b->wThumbID, LIB_ICON, &bw, &bh))
	{
		//icon
		if(orient == 0)
			minThumbSize = bw;
		else
			minThumbSize = bh;
	}
	else
	{
		lpWinSty = g_gui_rscfunc.osd_get_win_style(b->wThumbID);
		minThumbSize = 0;
        
		if(((lpWinSty->bWinStyleType) & C_WS_TYPE_MASK) != C_WS_PIC_DRAW)
			minThumbSize = 2;
		else
		{
			if(orient == 0)
			{
                if(OSD_GetLibInfoByWordIdx(lpWinSty->wLeftTopIdx, lpWinSty->IconLibIdx, &bw, &bh))
                	minThumbSize += bw;
                else if(OSD_GetLibInfoByWordIdx(lpWinSty->wLeftLineIdx, lpWinSty->IconLibIdx, &bw, &bh))
                	minThumbSize += bw;

                if(OSD_GetLibInfoByWordIdx(lpWinSty->wRightTopIdx, lpWinSty->IconLibIdx, &bw, &bh))
                	minThumbSize += bw;
                else if(OSD_GetLibInfoByWordIdx(lpWinSty->wRightLineIdx, lpWinSty->IconLibIdx, &bw, &bh))
                	minThumbSize += bw;
			}
			else
			{
				if(OSD_GetLibInfoByWordIdx(lpWinSty->wLeftTopIdx, lpWinSty->IconLibIdx, &bw, &bh))
					minThumbSize += bh;
				else if(OSD_GetLibInfoByWordIdx(lpWinSty->wTopLineIdx, lpWinSty->IconLibIdx, &bw, &bh))
					minThumbSize += bh;

				if(OSD_GetLibInfoByWordIdx(lpWinSty->wLeftButtomIdx, lpWinSty->IconLibIdx, &bw, &bh))
					minThumbSize += bh;
				else if(OSD_GetLibInfoByWordIdx(lpWinSty->wBottomLineIdx, lpWinSty->IconLibIdx, &bw, &bh))
					minThumbSize += bh;
			}

            if(minThumbSize == 0)
                minThumbSize = 2;
		}
	}

	if(b->bPage == 0)
	{
		if(b->nMax > 0)
			start_pos = minLoca + b->nPos * (maxLoca - minLoca) / b->nMax;
		else
			start_pos = minLoca;
		end_pos = minLoca + minThumbSize;		
	}
 	else if(b->bPage< b->nMax )
	{
	    page_width = (maxLoca  - minLoca+ 1)*b->bPage/b->nMax;
	    if(page_width<minThumbSize)
	        page_width = minThumbSize;
	    //else
	    //    page_width -= minThumbSize;
	    if( (b->nMax - b->nPos) <  b->bPage )
	        start_pos = maxLoca - page_width;
	    else
	        start_pos = minLoca + (maxLoca - minLoca  + 1 - page_width)*b->nPos/ (b->nMax -  b->bPage);
	    end_pos = start_pos + page_width;
	}
	else
	{
	    start_pos   = minLoca;
	    end_pos     = maxLoca;
	}

	if(orient == 0)
	{
		pRect->uStartX 	= start_pos;
		pRect->uStartY  	= mid_rect.uStartY;
		pRect->uWidth 	= end_pos - start_pos + 1;
        if(pRect->uWidth > minThumbSize && b->bStyle & BAR_HORI_ORIGINAL)
            pRect->uWidth = minThumbSize;
		pRect->uHeight 	= mid_rect.uHeight;		
	}
	else
	{
		pRect->uStartX 	= mid_rect.uStartX;
		pRect->uStartY  	= start_pos;
		pRect->uWidth 	= mid_rect.uWidth;
		pRect->uHeight 	= end_pos - start_pos + 1;
        if(pRect->uHeight > minThumbSize && b->bStyle & BAR_VERT_ORIGINAL)
            pRect->uHeight = minThumbSize;
	}
}


void OSD_DrawScrollBarCell(PSCROLL_BAR b,UINT8 bStyleIdx,UINT32 nCmdDraw)
{
	
	PGUI_VSCR		pVscr;
	UINT32 style_param;
	GUI_RECT rcThumb,MidRect;
	UINT16 BitmapWith,BitmapHeight;
	BOOL fDrawThumb;
	UINT32 sbarstyle;
	
	if(OSD_GetUpdateType(nCmdDraw) == C_UPDATE_CONTENT)
		sbarstyle = (UINT32)(~0);
	else
		sbarstyle = bStyleIdx;
	
	if(OSD_GetAttrSlave(b))
		pVscr = OSD_DrawSlaveFrame(&(b->head.frame),sbarstyle);
	else
		pVscr=OSD_DrawObjectFrame(&(b->head.frame), sbarstyle);

	OSD_ScrollBarGetMidRect(b,&MidRect);
	if(OSD_GetLibInfoByWordIdx(b->wTickBg, LIB_ICON, &BitmapWith, &BitmapHeight))//get mid rect ,check if icon
		OSDDrawPicSolidRectangle(pVscr, &MidRect, b->wTickBg,C_MIXER);//ERR_LIBGE,b->wTickBg is a style not color
	else
		OSD_DrawStyleRect(&MidRect,b->wTickBg,pVscr);//ERR_LIBGE,b->wTickBg is a style not color
	
	GetThumbRect(b, &rcThumb);
	if(OSD_GetLibInfoByWordIdx(b->wThumbID,LIB_ICON, &BitmapWith,&BitmapHeight))
		OSDDrawPicSolidRectangle(pVscr, &rcThumb, b->wThumbID,C_MIXER);//ERR_LIBGE,b->wTickBg is a style not color
	else
		OSD_DrawStyleRect(&rcThumb, b->wThumbID, pVscr);//ERR_LIBGE,b->wTickBg is a style not color
}

VACTION OSD_ScrollBarKeyMap(POBJECT_HEAD pObj, UINT32 vkey)
{
	VACTION Action = VACT_PASS;
	
	return Action;
}

PRESULT OSD_ScrollBarProc(POBJECT_HEAD pObj, UINT32 msg_type,UINT32 msg,UINT32 param1)
{
	PRESULT Result;

	if(msg_type==MSG_TYPE_KEY)
	{
		VACTION Action;
	    	UINT32 vkey;
	    	BOOL bContinue;
    	
		Result = OSD_ObjCommonProc(pObj,msg,param1,&vkey,&Action,&bContinue);
		if(!bContinue)
	    	goto CHECK_LEAVE;
	    	
		switch(Action)
	    {	
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

