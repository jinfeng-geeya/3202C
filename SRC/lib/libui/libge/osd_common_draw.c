/*-----------------------------------------------------------------------------
*
*	Copyright (C) 2010 ALI Corp. All rights reserved.
*
*	File: osd_common_draw.c
*
*	Content: 
*		support basic GUI draw function
*	History: 
*		2010/01/09 by Shine Zhou
*		Created this file. Inherited from new osd lib
*-----------------------------------------------------------------------------*/
#include <sys_config.h>

#include <api/libge/osd_lib.h>
#include <api/libc/string.h>
#include <api/libchar/lib_char.h>
#include <api/libge/osd_primitive.h>
#include <api/libge/osd_common_draw.h>
#include <api/libge/ge_primitive_init.h>
#include "osd_lib_internal.h"
/*####################################################################################*/
/*Draw detail element to VSCR,Draw bitmap,font,frame etc.*/
void OSD_DrawRect2Vscr(PGUI_RECT pfrm, UINT32 bg_color, PGUI_VSCR pvscr)
{
	UINT8 rgn_id;
	PGUI_REGION prgn;

	if((NULL != pvscr) && (NULL != pvscr->lpbuff))
	{
		if(gui_rctvalid(pfrm))
		{
			prgn = gelib_getdstrgn((UINT8)pvscr->root_layer,(UINT8)pvscr->root_rgn);//fill vscr rgn
			gelib_fillrect(prgn,pfrm,bg_color);
		}
	}
	else
	{
		if(NULL == pvscr)
			prgn = gelib_getdstrgn(vlayer_idx,vrgn_idx);
		else
			prgn = gelib_getdstrgn((UINT8)pvscr->dst_layer,(UINT8)pvscr->dst_rgn);

		if(NULL != prgn)
			gelib_fillrect(prgn,pfrm,bg_color);
	}
}

void OSD_DrawBmp2Vscr(PGUI_POINT pos,UINT32 styleidx,PGUI_VSCR pvscr)
{
	UINT8	*pbmp;
	UINT16 	rsc_id,cndraw;
	GUI_RECT rct;
	OBJECTINFO RscLibInfo;

	rsc_id = (styleidx &((UINT16)(~0))); 
	cndraw = ((styleidx&C_MIXER) == C_MIXER) ? 1 : 0;
	(pbmp = (UINT8*)g_gui_rscfunc.osd_get_rsc_data(LIB_ICON, rsc_id, &RscLibInfo));
	if ( pbmp != NULL)	
	{
		GUI_BMP bmp;
		PGUI_REGION prgn;

		if(NULL == pvscr)
			prgn = gelib_getdstrgn((UINT8)vlayer_idx,(UINT8)vrgn_idx);
		else if(NULL == pvscr->lpbuff)
			prgn = gelib_getdstrgn((UINT8)pvscr->dst_layer,(UINT8)pvscr->dst_rgn);
		else
			prgn = gelib_getdstrgn((UINT8)pvscr->root_layer,(UINT8)pvscr->root_rgn);

		OSD_SetRect(&rct,pos->uX,pos->uY,RscLibInfo.m_objAttr.m_wActualWidth, RscLibInfo.m_objAttr.m_wHeight);
		MEMSET(&bmp,0x0,sizeof(GUI_BMP));
		bmp.color_mode = prgn->color_mode;
		bmp.stride = RscLibInfo.m_objAttr.m_wWidth;
		bmp.pdata = pbmp;
		bmp.bmp_size.uWidth = RscLibInfo.m_objAttr.m_wActualWidth;
		bmp.bmp_size.uHeight = RscLibInfo.m_objAttr.m_wHeight;
		bmp.bg_color = gui_keycolor[bmp.color_mode];
		bmp.rsc_rle = 1;
		if(cndraw)
			gelib_drawbmp(prgn,&rct,(UINT32)&bmp);
		else
			gelib_renderbmp(prgn,&rct,(UINT32)&bmp);

		g_gui_rscfunc.osd_rel_rsc_data(pbmp, &RscLibInfo);
	}
}

static void OSD_GetCharByOffset(UINT8 *data,OBJECTINFO *info,INT32 offset)
{
    UINT16 i=0,j=0;
    UINT16 src_stride=0,dst_stride=0;
    UINT32 temp_data=0,abs_offset;

    if((offset==0)
        ||(offset>32)
        ||(offset<-32))
    {
        return;
    }

    src_stride = info->m_objAttr.m_wWidth/8;

    if(offset>0)
        abs_offset = offset;
    else
        abs_offset = -offset;
    
    dst_stride = (info->m_objAttr.m_wWidth+7-abs_offset)/8;
    
    if(src_stride>4)
    {
        //libc_printf("Do not support the case that width more than 32.");
        return;
    }
    
    for(i=0;i<info->m_objAttr.m_wHeight;i++)
    {
        temp_data = 0;
        for(j=0;j<src_stride;j++)
        {
            temp_data  = (temp_data<<8) | data[i*src_stride+j];
        }

        if(offset>0)//Delete the left part of a char.
        {
            temp_data = temp_data << abs_offset;
        }
        else//Delete the right part of a char.
        {
            temp_data = temp_data&(0xffffffff<<abs_offset);
        }

        for(j=0;j<dst_stride;j++)
        {
            data[i*dst_stride+j]=(temp_data>>8*(src_stride-1-j))&0xff;
        }
    }

    info->m_objAttr.m_wActualWidth-=abs_offset;
    info->m_objAttr.m_wWidth = (info->m_objAttr.m_wWidth+7-abs_offset)/8*8;
}

UINT16 OSD_DrawChar2Vscr(UINT16 x, UINT16 y, UINT32 fg_color, UINT32 bg_color, UINT16 ch, UINT8 font, PGUI_VSCR pvscr)
{
	UINT8*	pBitMapData;
	UINT16	height,width;
	ID_RSC	RscLibId;
	OBJECTINFO	ObjInfo;

	height = 0;
	width = 0;
	if(OSD_GetCharWidthHeight(ch, font,&width,&height) == FALSE)
		return 0;

	RscLibId = g_gui_rscfunc.osd_get_font_lib(ch) | font;
	if(NULL !=(pBitMapData =(UINT8*)g_gui_rscfunc.osd_get_rsc_data(RscLibId, ch, &ObjInfo)))
	{
		PGUI_REGION prgn;
		GUI_RECT rect_fill;
		GUI_FONT font_bmp;
	
		MEMSET(&font_bmp,0x0,sizeof(GUI_FONT));
		font_bmp.rsc_rle = 0;
		font_bmp.pdata = (void *)pBitMapData;
		font_bmp.fnt_size.uWidth = ObjInfo.m_objAttr.m_wActualWidth;
		font_bmp.fnt_size.uHeight = ObjInfo.m_objAttr.m_wHeight;
		font_bmp.stride = ObjInfo.m_objAttr.m_wWidth;

		if((ge_version > GE_MODULE_NEW) && (pvscr->color_mode==GUI_ARGB1555))
		{
			font_bmp.fg_color = gelib_color_expend(fg_color);
			font_bmp.bg_color = gelib_color_expend(bg_color);
		}
		else
		{
			font_bmp.fg_color = fg_color;
			font_bmp.bg_color = wstyle_noshow(bg_color)?gui_keycolor[pvscr->color_mode]:bg_color;
		}
		
		rect_fill.uStartX = x;
		rect_fill.uStartY = y;
		rect_fill.uWidth = width;
		rect_fill.uHeight = height;

		if(NULL == pvscr)
			prgn = gelib_getdstrgn((UINT8)vlayer_idx,(UINT8)vrgn_idx);
		if(NULL == pvscr->lpbuff)
			prgn = gelib_getdstrgn((UINT8)pvscr->dst_layer,(UINT8)pvscr->dst_rgn);
		else
			prgn = gelib_getdstrgn((UINT8)pvscr->root_layer,(UINT8)pvscr->root_rgn);

		gelib_drawfont(prgn,&rect_fill,(UINT32)&font_bmp);
	}

	return width;
}

UINT16 OSD_DrawChar2Vscr_ext(UINT16 x, UINT16 y, UINT32 fg_color, UINT32 bg_color, UINT16 ch, UINT8 font, PGUI_VSCR pvscr, INT32 offset)
{
	UINT8*	pBitMapData;
	UINT16	height,width;
	ID_RSC	RscLibId;
	OBJECTINFO	ObjInfo;

	height = 0;
	width = 0;
	if(OSD_GetCharWidthHeight(ch, font,&width,&height) == FALSE)
		return 0;

	RscLibId = g_gui_rscfunc.osd_get_font_lib(ch) | font;
	if(NULL !=(pBitMapData =(UINT8*)g_gui_rscfunc.osd_get_rsc_data(RscLibId, ch, &ObjInfo)))
	{
		PGUI_REGION prgn;
		GUI_RECT rect_fill;
		GUI_FONT font_bmp;

		OSD_GetCharByOffset(pBitMapData, &ObjInfo, offset);
		if (offset > 0)
			width -= offset;
		else
			width += offset;

		MEMSET(&font_bmp,0x0,sizeof(GUI_FONT));
		font_bmp.rsc_rle = 0;
		font_bmp.pdata = (void *)pBitMapData;
		font_bmp.fnt_size.uWidth = ObjInfo.m_objAttr.m_wActualWidth;
		font_bmp.fnt_size.uHeight = ObjInfo.m_objAttr.m_wHeight;
		font_bmp.stride = ObjInfo.m_objAttr.m_wWidth;
		if((ge_version > GE_MODULE_NEW) && (pvscr->color_mode==GUI_ARGB1555))
		{
			font_bmp.fg_color = gelib_color_expend(fg_color);
			font_bmp.bg_color = gelib_color_expend(bg_color);
		}
		else
		{
			font_bmp.fg_color = fg_color;
			font_bmp.bg_color = wstyle_noshow(bg_color)?gui_keycolor[pvscr->color_mode]:bg_color;
		}

		rect_fill.uStartX = x;
		rect_fill.uStartY = y;
		rect_fill.uWidth = width;
		rect_fill.uHeight = height;

		if(NULL == pvscr)
			prgn = gelib_getdstrgn(vlayer_idx,vrgn_idx);
		if(NULL == pvscr->lpbuff)
			prgn = gelib_getdstrgn((UINT8)pvscr->dst_layer,(UINT8)pvscr->dst_rgn);
		else
			prgn = gelib_getdstrgn((UINT8)pvscr->root_layer,(UINT8)pvscr->root_rgn);

		gelib_drawfont(prgn,&rect_fill,(UINT32)&font_bmp);
	}

	return width;
}

UINT16 OSD_DrawText(PGUI_RECT pRect, UINT8* pText, UINT32 Color, UINT8 AssignStyle, UINT8 FontSize, PGUI_VSCR pVscr)
{
	UINT16 uMaxHeight,uMaxWidth,chw,chh,uStr,uStrLen,uXpos, uYpos,uCount;

	uCount = 0;
	if((pText == NULL) || (wstyle_noshow(Color)))
		return 0;

	uStrLen = OSD_MultiFontLibStrMaxHW(pText, FontSize, &uMaxHeight, &uMaxWidth,0);
	if (!(uStrLen*uMaxHeight*uMaxWidth))
		return 0;
	
	uXpos = pRect->uStartX;
	uYpos = pRect->uStartY;
	if (pRect->uHeight > uMaxHeight)
	{
		if (GET_VALIGN(AssignStyle) == C_ALIGN_VCENTER)
			uYpos += (pRect->uHeight - uMaxHeight)>>1;
		else if (GET_VALIGN(AssignStyle) == C_ALIGN_BOTTOM)
			uYpos += pRect->uHeight - uMaxHeight;
	}

	if (uStrLen < pRect->uWidth)
	{
		if (GET_HALIGN(AssignStyle) == C_ALIGN_CENTER)
		{
			uXpos += (pRect->uWidth-uStrLen)>>1;				
		}	
		else if(GET_HALIGN(AssignStyle) == C_ALIGN_RIGHT)
		{
			uXpos += pRect->uWidth-uStrLen-4;
		}
	}

	while(0 != (uStr = ComMB16ToWord(pText)))
	{
		if(is_thai_unicode(uStr))
		{
			struct thai_cell cell;
			UINT32 cnt;

			cnt = thai_get_cell(pText, &cell);
			if(cnt == 0)
				break;

			OSD_GetThaiCellWidthHeight(&cell, FontSize,&chw,&chh);
			if(uXpos + chw> pRect->uStartX + pRect->uWidth )
				break;

			uXpos += OSD_DrawThaiCell(uXpos, uYpos, Color, C_NOSHOW, &cell, FontSize, pVscr);
			pText += cnt;
			uCount ++;
		}
		else
		{
			OSD_GetCharWidthHeight(uStr, FontSize,&chw,&chh);
			if(uXpos + chw> pRect->uStartX + pRect->uWidth )
				break;
		
			uXpos += OSD_DrawChar2Vscr(uXpos, uYpos,Color,C_NOSHOW, uStr, FontSize, pVscr);
			pText += 2;
			uCount ++;
		}
			
	}
	return uCount;
}

UINT16 OSD_DrawText_ext(PGUI_RECT pRect, UINT8* pText, UINT32 Color, UINT8 AssignStyle, UINT8 FontSize, PGUI_VSCR pVscr, UINT16 offset)
{
	UINT16 		uMaxHeight,uMaxWidth,chw,chh,uStr,uStrLen,uXpos, uYpos,uCount;
	UINT32 skip_len = 0;
	INT32 scroll_offset = 0;

	uCount = 0;
	if(pText == NULL)
		return 0;

	uStrLen = OSD_MultiFontLibStrMaxHW(pText, FontSize, &uMaxHeight, &uMaxWidth,0);
	if (!(uStrLen*uMaxHeight*uMaxWidth))
		return 0;
	
	uXpos = pRect->uStartX;
	uYpos = pRect->uStartY;
	if (pRect->uHeight > uMaxHeight)
	{
		if (GET_VALIGN(AssignStyle) == C_ALIGN_VCENTER)
			uYpos += (pRect->uHeight - uMaxHeight)>>1;
		else if (GET_VALIGN(AssignStyle) == C_ALIGN_BOTTOM)
			uYpos += pRect->uHeight - uMaxHeight;
	}

	if (uStrLen < pRect->uWidth)
	{
		if (GET_HALIGN(AssignStyle) == C_ALIGN_CENTER)
		{
			uXpos += (pRect->uWidth-uStrLen)>>1;				
		}	
		else if(GET_HALIGN(AssignStyle) == C_ALIGN_RIGHT)
		{
			uXpos += pRect->uWidth-uStrLen-4;
		}
	}

	while(0 != (uStr = ComMB16ToWord(pText)))
	{
		if(is_thai_unicode(uStr))
		{
			struct thai_cell cell;
			UINT32 cnt;

			cnt = thai_get_cell(pText, &cell);
			if(cnt == 0)
				break;

			OSD_GetThaiCellWidthHeight(&cell, FontSize,&chw,&chh);
			if(uXpos + chw> pRect->uStartX + pRect->uWidth )
				break;

			uXpos += OSD_DrawThaiCell(uXpos, uYpos, Color, C_NOSHOW, &cell, FontSize, pVscr);
			pText += cnt;
			uCount ++;
		}
		else
		{
			OSD_GetCharWidthHeight(uStr, FontSize,&chw,&chh);
			if (uXpos >= pRect->uLeft + pRect->uWidth )
				break;
			//The last char need to draw.
			else if (uXpos + chw > pRect->uLeft + pRect->uWidth )
			{
				scroll_offset = pRect->uLeft+pRect->uWidth-uXpos-chw;
			}
			//Skip the char.
			else if ((skip_len + chw) <= offset && uCount == 0)
			{
				skip_len+=chw;
				pText += 2;
				continue;
			}
			//Draw the first char.
			else if(((skip_len + chw) > offset) &&(skip_len<offset) &&(uCount==0))
			{
				scroll_offset = offset - skip_len;
			}
			//Draw the char normal.
			else
			{
				scroll_offset = 0;
			}

			uXpos += OSD_DrawChar2Vscr_ext(uXpos, uYpos, Color, C_NOSHOW, uStr, FontSize, pVscr, scroll_offset);
			pText += 2;
			uCount ++;
		}
			
	}
	return uCount;
}

/*####################################################################################*/
/*Winstyle etc.*/
void OSDDrawPicSolidRectangle(PGUI_VSCR pvscr,PGUI_RECT r,UINT32 rsc_idx,UINT32 mixer)
{
	UINT8*	pDtaBitmap;
	UINT16	id_rsc,uWidth,uHeight,uStartX,uStartY,uBottom,uRight;
	
	GUI_RECT bmp_rect;
	OBJECTINFO RscLibInfo;

	id_rsc = (UINT16)rsc_idx;
	pDtaBitmap = (UINT8*)g_gui_rscfunc.osd_get_rsc_data(LIB_ICON, id_rsc, &RscLibInfo);
	if(pDtaBitmap == NULL)
		return;

	uRight  = r->uStartX + r->uWidth;
	uBottom = r->uStartY + r->uHeight;
	uWidth   = RscLibInfo.m_objAttr.m_wActualWidth;
	uHeight  = RscLibInfo.m_objAttr.m_wHeight;
	for(uStartY = r->uStartY; uStartY < uBottom; uStartY += uHeight)
	{
		GUI_BMP	rsc_bmp;
		PGUI_REGION prgn;
		
		if( (uStartY + uHeight) > uBottom)
		{
			if(uStartY != r->uStartY) 
				uStartY = uBottom - uHeight;//if overflow,set it overlap with latest one
			else
				uHeight = uBottom - uStartY;//if vertical gap is smaller than picture height,reduce height
		}
		bmp_rect.uStartY = uStartY;
		bmp_rect.uHeight = uHeight;

		for(uStartX =r->uStartX;uStartX < uRight;uStartX += uWidth)
		{
			if( (uStartX + uWidth) > uRight)
			{
				if(uStartX != r->uStartX)
					uStartX = uRight - uWidth;//if overflow,set it overlap with latest one
				else
					uWidth = uRight - uStartX;//if horizontal gap is smaller than picture size,reduce it.                   
			}
			bmp_rect.uStartX  = uStartX;
			bmp_rect.uWidth = uWidth;

			if(NULL == pvscr->lpbuff)
				prgn = gelib_getdstrgn((UINT8)pvscr->dst_layer,(UINT8)pvscr->dst_rgn);
			else
				prgn = gelib_getdstrgn((UINT8)pvscr->root_layer,(UINT8)pvscr->root_rgn);
			
			MEMSET(&rsc_bmp,0x0,sizeof(rsc_bmp));
			rsc_bmp.pdata = pDtaBitmap;
			rsc_bmp.color_mode = pvscr->color_mode;
			rsc_bmp.stride = RscLibInfo.m_objAttr.m_wWidth;
			rsc_bmp.bmp_size.uWidth = RscLibInfo.m_objAttr.m_wActualWidth;
			rsc_bmp.bmp_size.uHeight = RscLibInfo.m_objAttr.m_wHeight;
			rsc_bmp.bg_color = gui_keycolor[pvscr->color_mode];
			rsc_bmp.rsc_rle = 1;

			if(C_MIXER != mixer)
				gelib_drawbmp(prgn,&bmp_rect,(UINT32)&rsc_bmp);
			else
				gelib_renderbmp(prgn,&bmp_rect,(UINT32)&rsc_bmp);
		}
	}

	g_gui_rscfunc.osd_rel_rsc_data(pDtaBitmap, &RscLibInfo);
}

const UINT8 circle_dscr[] = { 5,3, 2,1,1};
void OSDDrawCircleRectangle (PGUI_RECT r, PWINSTYLE lpWinSty, PGUI_VSCR pvscr)
{
	UINT8	i,offset;
	UINT16	n;
	UINT32	color;
	GUI_PEN pen_dot;
	GUI_RECT	tmp_rect;
	GUI_POINT strt,end;
	PGUI_REGION prgn;

	if(NULL == pvscr->lpbuff)
		prgn = gelib_getdstrgn((UINT8)pvscr->dst_layer,(UINT8)pvscr->dst_rgn);
	else
		prgn = gelib_getdstrgn((UINT8)pvscr->root_layer,(UINT8)pvscr->root_rgn);
	
	if (wstyle_colorshow(lpWinSty->wBgIdx))
	{
		color = (lpWinSty->wBgIdx);
		for(i=0; i<CIRCLE_PIX; i++)
		{
			offset = circle_dscr[i];
			strt.uX = r->uStartX + offset;
			end.uX = r->uStartX + offset + r->uWidth - offset*2;
			strt.uY = end.uY = i + r->uStartY;
			MEMSET(&pen_dot,0,sizeof(GUI_PEN));
			pen_dot.thick = 2;
			pen_dot.color = color;
			gelib_drawline(prgn,&strt,&end,(UINT32)&pen_dot);
		}
		OSD_SetRect(&tmp_rect, r->uStartX, r->uStartY + CIRCLE_PIX, r->uWidth, r->uHeight-2*CIRCLE_PIX);
		OSD_DrawRect2Vscr(&tmp_rect, color, pvscr);

		for(i=0;i<CIRCLE_PIX;i++)
		{
			offset = circle_dscr[i];
			strt.uX = r->uStartX +  offset;
			end.uX = r->uStartX + offset + r->uWidth - (offset<<1);
			strt.uY = end.uY = r->uStartY + r->uHeight - i - 1;
			MEMSET(&pen_dot,0,sizeof(GUI_PEN));
			pen_dot.thick = 2;
			pen_dot.color = color;
			gelib_drawline(prgn,&strt,&end,(UINT32)&pen_dot);
		}        
	}

	if (wstyle_colorshow(lpWinSty->wTopLineIdx))
	{
		color = (lpWinSty->wTopLineIdx);
		n = wstyle_getline(lpWinSty->bWinStyleType);
		offset= circle_dscr[0];
		MEMSET(&pen_dot,0,sizeof(GUI_PEN));
		pen_dot.thick = 2;
		pen_dot.color = color;
		for(i=0;i<n;i++)
		{
			strt.uX = r->uStartX + offset;
			end.uX = r->uStartX + offset + r->uWidth - offset*2;
			strt.uY = end.uY =r->uStartY + i;
			gelib_drawline(prgn,&strt,&end,(UINT32)&pen_dot);
		}
		
		for(i=1;i<CIRCLE_PIX;i++)
		{
			offset = circle_dscr[i];
			strt.uX =r->uStartX + offset;
			end.uX = r->uStartX + offset + n;
			strt.uY = end.uY = r->uStartY + i;
			gelib_drawline(prgn,&strt,&end,(UINT32)&pen_dot);
			strt.uX = r->uStartX + r->uWidth - n - offset;
			end.uX =r->uStartX + r->uWidth - n - offset + n;
			strt.uY = end.uY = r->uStartY + i;
			gelib_drawline(prgn,&strt,&end,(UINT32)&pen_dot);
		}
	}

	if (wstyle_colorshow(lpWinSty->wBottomLineIdx))
	{
		color = (lpWinSty->wBottomLineIdx);
		n = wstyle_getline(lpWinSty->bWinStyleType);
		MEMSET(&pen_dot,0,sizeof(GUI_PEN));
		pen_dot.thick = 2;
		pen_dot.color = color;
		offset= circle_dscr[0];
		for(i=0;i<n;i++)
		{
			strt.uX = r->uStartX + offset;
			end.uX =r->uStartX + offset + r->uWidth - offset*2;
			strt.uY = end.uY = r->uStartY+r->uHeight- i - 1;
			gelib_drawline(prgn,&strt,&end,(UINT32)&pen_dot);
		}
		for(i=1;i<CIRCLE_PIX;i++)
		{
			offset = circle_dscr[i];
			strt.uX =r->uStartX + offset;
			end.uX = r->uStartX + offset + n;
			strt.uY = end.uY = r->uStartY+r->uHeight- i - 1;
			gelib_drawline(prgn,&strt,&end,(UINT32)&pen_dot);
			strt.uX = r->uStartX + r->uWidth - n - offset;
			end.uX =r->uStartX + r->uWidth - n - offset + n;
			strt.uY = end.uY = r->uStartY+r->uHeight- i - 1;
			gelib_drawline(prgn,&strt,&end,(UINT32)&pen_dot);
		}
	}

	if (wstyle_colorshow(lpWinSty->wLeftLineIdx))
	{
		color = (lpWinSty->wLeftLineIdx);
		n = wstyle_getline(lpWinSty->bWinStyleType)>>1;
		MEMSET(&pen_dot,0,sizeof(GUI_PEN));
		pen_dot.thick = 2;
		pen_dot.color = color;
		for(i=0;i<n;i++)
		{
			strt.uX = end.uX = r->uStartX + i;
			strt.uY = r->uStartY + CIRCLE_PIX;
			end.uY = r->uStartY + CIRCLE_PIX + r->uHeight - CIRCLE_PIX*2;			
			gelib_drawline(prgn,&strt,&end,(UINT32)&pen_dot);
		}
	}

	if (wstyle_colorshow(lpWinSty->wRightLineIdx))
	{
		color = (lpWinSty->wRightLineIdx);
		n = wstyle_getline(lpWinSty->bWinStyleType)>>1;
		MEMSET(&pen_dot,0,sizeof(GUI_PEN));
		pen_dot.thick = 2;
		pen_dot.color = color;
		for(i=0;i<n;i++)
		{
			strt.uX = end.uX = r->uStartX+r->uWidth- i - 1;
			strt.uY =  r->uStartY + CIRCLE_PIX;
			end.uY =  r->uStartY + CIRCLE_PIX + r->uHeight - CIRCLE_PIX*2;			
			gelib_drawline(prgn,&strt,&end,(UINT32)&pen_dot);
		}
	}
}

void OSDDrawSolidRectangle (PGUI_RECT r, PWINSTYLE lpWinSty, PGUI_VSCR pvscr)
{
	GUI_RECT tmp_rect;
	UINT16	tmp_size;
	
	// Fill Solid Box
	if (wstyle_colorshow(lpWinSty->wBgIdx))
	{
		OSD_DrawRect2Vscr(r, lpWinSty->wBgIdx, pvscr);
	}
	
	//left line
	if (wstyle_colorshow(lpWinSty->wLeftLineIdx))
	{
		tmp_size = wstyle_getline(lpWinSty->bWinStyleType);
		OSD_SetRect(&tmp_rect, r->uStartX, r->uStartY, tmp_size, r->uHeight);
		OSD_DrawRect2Vscr(&tmp_rect, lpWinSty->wLeftLineIdx, pvscr);
	}

	//right line
	if (wstyle_colorshow(lpWinSty->wRightLineIdx))
	{
		tmp_size = wstyle_getline(lpWinSty->bWinStyleType);
		OSD_SetRect(&tmp_rect, r->uStartX+r->uWidth-tmp_size, r->uStartY, tmp_size, r->uHeight);
		OSD_DrawRect2Vscr(&tmp_rect, lpWinSty->wRightLineIdx, pvscr);
	}

	//top line
	if (wstyle_colorshow(lpWinSty->wTopLineIdx))
	{
		tmp_size = wstyle_getline(lpWinSty->bWinStyleType);
		OSD_SetRect(&tmp_rect, r->uStartX, r->uStartY, r->uWidth, tmp_size);
		OSD_DrawRect2Vscr(&tmp_rect, lpWinSty->wTopLineIdx, pvscr);
	}

	//bottom line
	if (wstyle_colorshow(lpWinSty->wBottomLineIdx))
	{
		tmp_size = wstyle_getline(lpWinSty->bWinStyleType);
		OSD_SetRect(&tmp_rect, r->uStartX, r->uStartY + r->uHeight - tmp_size, r->uWidth, tmp_size);
		OSD_DrawRect2Vscr(&tmp_rect, lpWinSty->wBottomLineIdx, pvscr);
	}
}

void OSDDrawPICRectangle (PGUI_RECT r, PWINSTYLE lpWinSty, PGUI_VSCR pvscr)
{
	UINT16	IconLibIdx,wWidth,uHeight;
	UINT16	wLeftTopWidth, wLeftTopHeight, wRightTopWidth, wRightTopHeight;
	UINT16	wRightButtomWidth, wRightButtomHeight, wLeftButtomWidth, wLeftButtomHeight;
	UINT16	borders_w[4];
	UINT32	styleidx;
	GUI_RECT	borders_r;
	GUI_POINT	pos;

	MEMSET(borders_w,0x0,sizeof(borders_w));
	IconLibIdx = (UINT16)((lpWinSty->IconLibIdx!= INVALID_INDEX)? lpWinSty->IconLibIdx : LIB_ICON);
	OSD_GetLibInfoByWordIdx(lpWinSty->wLeftTopIdx, IconLibIdx, &wLeftTopWidth, &wLeftTopHeight);
	OSD_GetLibInfoByWordIdx(lpWinSty->wRightTopIdx, IconLibIdx, &wRightTopWidth, &wRightTopHeight);
	OSD_GetLibInfoByWordIdx(lpWinSty->wRightButtomIdx, IconLibIdx, &wRightButtomWidth, &wRightButtomHeight);
	OSD_GetLibInfoByWordIdx(lpWinSty->wLeftButtomIdx, IconLibIdx, &wLeftButtomWidth, &wLeftButtomHeight);
    
	//  Background
	if (wstyle_colorshow(lpWinSty->wBgIdx))
	{	
		OSD_GetLibInfoByWordIdx(lpWinSty->wTopLineIdx, IconLibIdx, &wWidth, &uHeight);
		borders_w[0] = uHeight;
		OSD_GetLibInfoByWordIdx(lpWinSty->wBottomLineIdx, IconLibIdx, &wWidth, &uHeight);
		borders_w[1] = uHeight;
		OSD_GetLibInfoByWordIdx(lpWinSty->wLeftLineIdx, IconLibIdx, &wWidth, &uHeight);
		borders_w[2] = wWidth;
		OSD_GetLibInfoByWordIdx(lpWinSty->wRightLineIdx, IconLibIdx, &wWidth, &uHeight);
		borders_w[3] = wWidth;     

		if((r->uWidth > borders_w[2] + borders_w[3]) && (r->uHeight > borders_w[0] + borders_w[1]))
		{
			OSD_SetRect(&borders_r, r->uStartX + borders_w[2], r->uStartY + borders_w[0], r->uWidth - borders_w[2] - borders_w[3], r->uHeight - borders_w[0] - borders_w[1]);

			if (OSD_GetLibInfoByWordIdx((UINT16)(lpWinSty->wBgIdx), IconLibIdx, &wWidth, &uHeight))
	    			OSDDrawPicSolidRectangle(pvscr, &borders_r, (lpWinSty->wBgIdx),(wstyle_getmixer(lpWinSty->bWinStyleType,WID_BGMIX)));
			else
				OSD_DrawRect2Vscr(&borders_r, lpWinSty->wBgIdx, pvscr);
		}
	}
        
	//  TopLine
	if ((INVALID_INDEX !=lpWinSty->wTopLineIdx) && (r->uWidth > (wLeftTopWidth + wRightTopWidth)))
	{
		if (OSD_GetLibInfoByWordIdx(lpWinSty->wTopLineIdx, IconLibIdx, &wWidth, &uHeight))
		{
			uHeight = uHeight > r->uHeight ? r->uHeight : uHeight;
			OSD_SetRect(&borders_r, r->uStartX+wLeftTopWidth, r->uStartY, r->uWidth -wLeftTopWidth - wRightTopWidth, uHeight);
			OSDDrawPicSolidRectangle(pvscr, &borders_r, lpWinSty->wTopLineIdx,(wstyle_getmixer(lpWinSty->bWinStyleType,WID_BGMIX)));
		}
		else if(wstyle_colorshow(lpWinSty->wTopLineIdx))
		{
			uHeight = wstyle_getline(lpWinSty->bWinStyleType);
			OSD_SetRect(&borders_r, r->uStartX+wLeftTopWidth, r->uStartY, r->uWidth -wLeftTopWidth - wRightTopWidth, uHeight);
			OSD_DrawRect2Vscr(&borders_r, lpWinSty->wTopLineIdx, pvscr);
		}

		if(0 != wLeftTopHeight) 
			wLeftTopHeight = uHeight;
		if(0 != wRightTopHeight)    
			wRightTopHeight = uHeight;
	}
	
	//  BottomLine
	if((INVALID_INDEX != lpWinSty->wBottomLineIdx) && (r->uWidth > (wLeftTopWidth + wRightTopWidth)))
	{
		if (OSD_GetLibInfoByWordIdx(lpWinSty->wBottomLineIdx, IconLibIdx, &wWidth, &uHeight))
		{    
			UINT16	uStartY = uHeight > r->uHeight ? r->uStartY : r->uStartY+r->uHeight-uHeight;
			uHeight = uHeight > r->uHeight ? r->uHeight : uHeight;
			
			OSD_SetRect(&borders_r, r->uStartX+wLeftTopWidth, uStartY, r->uWidth -wLeftTopWidth - wRightTopWidth, uHeight);
			OSDDrawPicSolidRectangle(pvscr, &borders_r, lpWinSty->wBottomLineIdx,(wstyle_getmixer(lpWinSty->bWinStyleType,WID_BGMIX)));
		}
		else if(wstyle_colorshow(lpWinSty->wBottomLineIdx))
		{
			uHeight = wstyle_getline(lpWinSty->bWinStyleType);
			OSD_SetRect(&borders_r, r->uStartX + wLeftButtomWidth, r->uStartY+ r->uHeight - 1, r->uWidth -wLeftButtomWidth - wRightButtomWidth, uHeight);
			OSD_DrawRect2Vscr(&borders_r, lpWinSty->wBottomLineIdx,pvscr);
		}    

		if(!wLeftButtomHeight) 
			wLeftButtomHeight = uHeight;
		if(!wRightButtomHeight)    
			wRightButtomHeight = uHeight;
	}
	//  LeftLine
	if ((INVALID_INDEX !=lpWinSty->wLeftLineIdx ) && (r->uHeight > (wLeftTopHeight + wLeftButtomHeight)))
	{
		if (OSD_GetLibInfoByWordIdx(lpWinSty->wLeftLineIdx, IconLibIdx, &wWidth, &uHeight))
		{
			wWidth = wWidth > r->uWidth ? r->uWidth : wWidth;			
			OSD_SetRect(&borders_r, r->uStartX, r->uStartY + wLeftTopHeight, wWidth, r->uHeight - wLeftTopHeight - wLeftButtomHeight);
			OSDDrawPicSolidRectangle(pvscr, &borders_r,lpWinSty->wLeftLineIdx,(wstyle_getmixer(lpWinSty->bWinStyleType,WID_BGMIX)));//if need mixer,specify in winstyle
		}
		else if(wstyle_colorshow(lpWinSty->wLeftLineIdx))
		{
			wWidth = wstyle_getline(lpWinSty->bWinStyleType);
			OSD_SetRect(&borders_r, r->uStartX, r->uStartY+ wLeftTopHeight, wWidth, r->uHeight- wLeftTopHeight- wLeftButtomHeight);
			OSD_DrawRect2Vscr(&borders_r, lpWinSty->wLeftLineIdx,pvscr);
		}
	}
	
	//  RightLine
	if ((INVALID_INDEX !=lpWinSty->wRightLineIdx ) && (r->uHeight > (wLeftTopHeight + wLeftButtomHeight)))
	{
		if (OSD_GetLibInfoByWordIdx(lpWinSty->wRightLineIdx, IconLibIdx, &wWidth, &uHeight))
		{
			UINT16	uLeft = wWidth > r->uWidth ? r->uStartX : r->uStartX+r->uWidth-wWidth;
			uHeight = wWidth > r->uWidth ? r->uWidth : wWidth;
			OSD_SetRect(&borders_r, uLeft,r->uStartY + wRightTopHeight,wWidth,r->uHeight - wRightTopHeight - wRightButtomHeight);
			OSDDrawPicSolidRectangle(pvscr, &borders_r, lpWinSty->wRightLineIdx,(wstyle_getmixer(lpWinSty->bWinStyleType,WID_BGMIX)));//if need mixer,specify in winstyle
		}
		else if(wstyle_colorshow(lpWinSty->wRightLineIdx))
		{
			wWidth = wstyle_getline(lpWinSty->bWinStyleType);
			OSD_SetRect(&borders_r, r->uStartX+r->uWidth-1, r->uStartY+ wRightTopHeight, wWidth, r->uHeight- wRightTopHeight- wRightButtomHeight);
			OSD_DrawRect2Vscr(&borders_r, lpWinSty->wRightLineIdx,pvscr);
		}
	}
	
    //  LeftTop
	if (INVALID_INDEX !=lpWinSty->wLeftTopIdx)
	{
		pos.uX = r->uStartX;
		pos.uY = r->uStartY;

		styleidx = lpWinSty->wLeftTopIdx;
		if(wstyle_getmixer(lpWinSty->bWinStyleType,WID_CNDRW))
			styleidx |= C_MIXER;
		OSD_DrawBmp2Vscr(&pos,styleidx,pvscr);
	}
        //  LeftButtom
	if (INVALID_INDEX !=lpWinSty->wLeftButtomIdx)
	{
		if(r->uHeight > wLeftButtomHeight)
		{
			pos.uX = r->uStartX;
			pos.uY = r->uStartY+r->uHeight-wLeftButtomHeight;
			
			styleidx = lpWinSty->wLeftButtomIdx;
			if(wstyle_getmixer(lpWinSty->bWinStyleType,WID_CNDRW))
				styleidx |= C_MIXER;
			OSD_DrawBmp2Vscr(&pos,styleidx,pvscr);
		}
	}
        //  RightTop
	if (INVALID_INDEX !=lpWinSty->wRightTopIdx)
	{
		if(r->uWidth > wRightTopWidth)
		{
			pos.uX = r->uStartX+r->uWidth-wRightTopWidth;
			pos.uY = r->uStartY;

			styleidx = lpWinSty->wRightTopIdx;
			if(wstyle_getmixer(lpWinSty->bWinStyleType,WID_CNDRW))
				styleidx |= C_MIXER;
			OSD_DrawBmp2Vscr(&pos,styleidx,pvscr);
		}
	}
        //  RightButtom
	if (INVALID_INDEX !=lpWinSty->wRightButtomIdx)
	{
		if((r->uWidth > wRightButtomWidth) && (r->uHeight > wRightButtomHeight))
		{
			pos.uX = r->uStartX+r->uWidth-wRightButtomWidth;
			pos.uY = r->uStartY+r->uHeight-wRightButtomHeight;
			
			styleidx = lpWinSty->wRightButtomIdx;
			if(wstyle_getmixer(lpWinSty->bWinStyleType,WID_CNDRW))
				styleidx |= C_MIXER;
			OSD_DrawBmp2Vscr(&pos,styleidx,pvscr);
		}
	}
}

void OSD_DrawStyleRect(PGUI_RECT pRect, UINT32 bShIdx, PGUI_VSCR pvscr)
{
	PWINSTYLE	lpWinSty;

	if(gui_rctvalid(pRect)==0)
		return;

	lpWinSty= g_gui_rscfunc.osd_get_win_style(bShIdx);		
	switch((lpWinSty->bWinStyleType) & C_WS_TYPE_MASK)
	{
		case (C_WS_LINE_DRAW& C_WS_TYPE_MASK):
			OSDDrawSolidRectangle(pRect, lpWinSty,pvscr);
			break;
		case (C_WS_LINE_CIRCL_DRAW& C_WS_TYPE_MASK):
			OSDDrawCircleRectangle(pRect, lpWinSty,pvscr);
			break;
		case C_WS_PIC_DRAW:
			OSDDrawPICRectangle(pRect, lpWinSty,pvscr);
			break;
		case C_WS_USER_DEFINE:
			break;
	}
}

/*####################################################################################*/
/*Strings,Text etc.*/
UINT16 OSD_DrawThaiCell(UINT16 x, UINT16 y, UINT32 fg_color, UINT32 bg_color, struct thai_cell *cell, UINT8 font, PGUI_VSCR pVscr)
{
	UINT8*	pBitMapData;
	UINT16	height,width;
	GUI_RECT r;
	GUI_VSCR	vscr;
	OBJECTINFO	ObjInfo;
	ID_RSC		RscLibId;
		
	height = width =0;
	if((cell == NULL) || (cell->char_num == 0))
		return 0;
		
	if(OSD_GetThaiCellWidthHeight(cell, font,&width,&height) == FALSE)
		return 0;

	MEMCPY(&vscr, pVscr, sizeof(GUI_VSCR));
	if(wstyle_colorshow(bg_color))
	{
		OSD_SetRect(&r,x,y,width,height);
		OSD_DrawRect2Vscr(&r,bg_color,&vscr);
	}
	
	RscLibId = g_gui_rscfunc.osd_get_font_lib(cell->unicode[0]) | font;
	if(g_gui_rscfunc.osd_get_thai_font_data!=NULL)
	{
		(pBitMapData =(UINT8*)g_gui_rscfunc.osd_get_thai_font_data(RscLibId, cell, &ObjInfo));		
		if(NULL != pBitMapData)
		{
			PGUI_REGION prgn;
			GUI_BMP	rsc_bmp;
		
			OSD_SetRect(&r, x, y, width, height);
			if(NULL == pVscr->lpbuff)
				prgn = gelib_getdstrgn((UINT8)pVscr->dst_layer,(UINT8)pVscr->dst_rgn);
			else
				prgn = gelib_getdstrgn((UINT8)pVscr->root_layer,(UINT8)pVscr->root_rgn);
			
			MEMSET(&rsc_bmp,0x0,sizeof(rsc_bmp));
			rsc_bmp.pdata = pBitMapData;
			rsc_bmp.color_mode = pVscr->color_mode;
			rsc_bmp.stride = ObjInfo.m_objAttr.m_wWidth;
			rsc_bmp.bmp_size.uWidth = ObjInfo.m_objAttr.m_wActualWidth;
			rsc_bmp.bmp_size.uHeight = ObjInfo.m_objAttr.m_wHeight;
			rsc_bmp.bg_color = gui_keycolor[pVscr->color_mode];
			gelib_renderbmp(prgn,&r,(UINT32)&rsc_bmp);
		}
	}

	return width;
}

/*####################################################################################*/
/* virtual screen related functions*/
static RET_CODE OSD_UpdateSlvscr(PGUI_VSCR pVscr);
RET_CODE OSD_UpdateVscr(PGUI_VSCR pVscr)
{
	RET_CODE ret;

	ret = RET_FAILURE;
	if(gui_rctvalid(&(pVscr->frm))==0)
	{
		//gui_printf("Rct NULL");
	}
	else if(pVscr->suspend> 0)
	{//SZMK
		//gui_printf("suspend operations!");
	}
	else if(pVscr->dst_layer >= GUI_SLVGMA_SURF)
	{
		//gui_printf("invalid vscr operation handle.");
	}
	else if((pVscr->dirty_flag > 0) && (pVscr->lpbuff != NULL) && (pVscr->dst_layer < GUI_MEMVSCR_SURF))
	{
		if(RET_SUCCESS == gelib_update2scrn(pVscr,&pVscr->frm))
		{
			pVscr->dirty_flag = 0;
			MEMSET(&(pVscr->frm),0x0,sizeof(GUI_RECT));
			ret = RET_SUCCESS;
		}
	}

	if(gelib_getdual_support())
	{
		OSD_UpdateSlvscr(NULL);
	}

	return ret;
}

RET_CODE OSD_UpdateSlvscr(PGUI_VSCR pVscr)
{
	RET_CODE ret;
	PGUI_VSCR	pslvscr;
	PGUI_REGION pslvrgn;
	
	ret = RET_FAILURE;
	if(NULL == pVscr)
	{
		pslvscr = &g_slvscr[(osal_task_get_current_id())-1];
		pslvrgn = gelib_getdstrgn(GUI_SLVSCR_SURF,0);
	}
	else
	{
		pslvscr = pVscr;
		pslvrgn = gelib_getdstrgn(pVscr->root_layer,pVscr->root_rgn);
	}
	
	if((gui_rctvalid(&(pslvscr->frm))==0))
	{
		//gui_printf("Slave rct NULL");
	}
	else if(pslvscr->suspend > 0)
	{//SZMK
		//gui_printf("suspend operations!");
	}
	else if(pslvscr->dst_layer != GUI_SLVGMA_SURF)
	{
		//gui_printf("invalid vscr operation handle.");
	}
	else if((void *)pslvrgn->hsurf != NULL)
	{
		if(gedraw_update2slave(pslvscr,&(pslvscr->frm)) == RET_SUCCESS)
		{
			pslvscr->dirty_flag = 0;
			MEMSET(&(pslvscr->frm),0x0,sizeof(GUI_RECT));
			ret = RET_SUCCESS;
		}
	}

	return ret;
}

static GUI_RECT rctvscr;
PGUI_RECT OSD_ClearVscr(PGUI_VSCR pvscr)
{
	MEMSET(&rctvscr,0x0,sizeof(rctvscr));
	
	if(pvscr  != NULL)
	{
		if(gui_rctvalid(&(pvscr->frm))==1)
		{
			MEMCPY(&rctvscr,&(pvscr->frm),sizeof(rctvscr));
		}
		
		pvscr->dirty_flag = 0;
		MEMSET(&(pvscr->frm),0x0,sizeof(GUI_RECT));
	}

	return (&rctvscr);
}

PGUI_VSCR OSD_GetVscr(PGUI_RECT pRect, UINT32 bFlag)
{
	UINT8	suspend_flag;
	PGUI_VSCR pGVscr;
	GUI_RECT	corner;
	PGUI_REGION prgn;

	pGVscr = OSD_GetTaskVscr(osal_task_get_current_id());
	if(GUI_MEMPOP_SURF == pGVscr->dst_layer)
	{
		bFlag = VSCR_DIRECTDRAW;
		pGVscr->lpbuff = pop_buff;
	}
	else
	{
		pGVscr->lpbuff = vscr_buff;
	}
	
	if(OSD_RectInRect(&pGVscr->frm,pRect))
	{
		return pGVscr;
	}
	else if(gui_rctvalid(&pGVscr->frm))
	{//SZTODO
		OSD_UpdateVscr(pGVscr);//SZMK,return value check
	}
	else
	{
	}
	
	pGVscr->dirty_flag = 0;
	OSD_SetRect2(&pGVscr->frm, pRect);

	if(NULL == pGVscr->lpbuff)
	{
		return pGVscr;
	}
	else if(VSCR_GET_BACK == bFlag)
	{
		gelib_retrievescrn(pGVscr, pRect);
	}
	else if(VSCR_GET_CORNER == bFlag)
	{
		OSD_SetRect(&corner, pRect->uStartX, pRect->uStartY,(CIRCLE_PIX<<1), pRect->uHeight);
		gelib_retrievescrn(pGVscr, &corner);
		corner.uStartX = pRect->uStartX + pRect->uWidth - (CIRCLE_PIX<<1);
		gelib_retrievescrn(pGVscr, &corner);
	}
	else if(VSCR_FILL_BACK == bFlag)
	{
		PGUI_REGION prgn;

		prgn = gelib_getdstrgn((UINT8)pGVscr->root_layer,(UINT8)pGVscr->root_rgn);
		gelib_fillrect(prgn,pRect,transcolor_const[prgn->color_mode]);
	}
	else//VSCR_NULL
	{
		//TODO:
	}
	
	return pGVscr;
}


PGUI_VSCR OSD_GetSlvscr(PGUI_RECT pRect, UINT32 bFlag)
{
	UINT8	suspend_flag;
	UINT32	vscr_idx;
	PGUI_VSCR pGVscr;
	GUI_RECT	corner;
	PGUI_REGION prgn;

	vscr_idx = (UINT32)osal_task_get_current_id();
	if((vscr_idx == OSAL_INVALID_ID) || (vscr_idx > GUI_MULTITASK_CNT))
	{
		gui_printf("Invalid index");
		return NULL;
	}
	
	pGVscr = &g_slvscr[vscr_idx - 1];
	if(GUI_MEMPOP_SURF == pGVscr->dst_layer)
	{
		bFlag = VSCR_DIRECTDRAW;
		pGVscr->lpbuff = slvpop_buff;
	}
	else
	{
		pGVscr->lpbuff = slvscr_buff;
	}
	
	if(OSD_RectInRect(&pGVscr->frm,pRect))
	{
		return pGVscr;
	}
	else if(gui_rctvalid(&pGVscr->frm))
	{
		OSD_UpdateSlvscr(pGVscr);
	}
	else
	{
		//do nothing
	}
	
	pGVscr->dirty_flag = 0;
	OSD_SetRect2(&pGVscr->frm, pRect);

	if(NULL == pGVscr->lpbuff)
	{
		return pGVscr;
	}
	/*else if(VSCR_GET_BACK == bFlag)
	{
		gedraw_retrieveslave(pGVscr, pRect);
	}
	else if(VSCR_GET_CORNER == bFlag)
	{
		OSD_SetRect(&corner, pRect->uStartX, pRect->uStartY,(CIRCLE_PIX<<1), pRect->uHeight);
		gedraw_retrieveslave(pGVscr, &corner);
		corner.uStartX = pRect->uStartX + pRect->uWidth - (CIRCLE_PIX<<1);
		gedraw_retrieveslave(pGVscr, &corner);
	}*/
	else if(VSCR_FILL_BACK == bFlag)
	{
		PGUI_REGION prgn;

		prgn = gelib_getdstrgn((UINT8)pGVscr->root_layer,(UINT8)pGVscr->root_rgn);
		gelib_fillrect(prgn,pRect,transcolor_const[prgn->color_mode]);
	}
	else
	{
	}
	
	return pGVscr;
}

PGUI_VSCR OSD_GetTaskVscr(ID task_id)
{
	if((task_id == OSAL_INVALID_ID)
		|| (task_id > GUI_MULTITASK_CNT))
		return NULL;/*Invalid id or id overflow max supported*/

	return &(g_vscr[task_id - 1]);
}

PGUI_VSCR OSD_GetTaskSlvscr(ID task_id)
{
	if((task_id == OSAL_INVALID_ID)
		|| (task_id > GUI_MULTITASK_CNT))
		return NULL;/*Invalid id or id overflow max supported*/

	return &(g_slvscr[task_id - 1]);
}

void OSD_SetVscrDst(UINT8 dstlayer,UINT8 dstrgn)
{
	vrgn_idx = dstrgn;
	vlayer_idx = dstlayer;
}

RET_CODE OSD_ClearScrn(PGUI_VSCR pvscr)
{
	RET_CODE ret;
	GUI_VSCR vscr_tmp;
	PGUI_REGION proot,pdst;

	if(pvscr == NULL)
	{//default parameters,that is you need not specify pvscr for simple
		MEMSET(&vscr_tmp,0x0,sizeof(vscr_tmp));
		pvscr = &vscr_tmp;

		vscr_tmp.dst_rgn = 0;
		vscr_tmp.root_rgn = 0;
		vscr_tmp.dst_layer = vlayer_idx;
		vscr_tmp.root_layer = GUI_MEMVSCR_SURF;
	}
	
	proot = gelib_getdstrgn((UINT8)pvscr->root_layer,(UINT8)pvscr->root_rgn);
	pdst = gelib_getdstrgn((UINT8)pvscr->dst_layer,(UINT8)pvscr->dst_rgn);
	ret = gelib_fillrect(proot,NULL,transcolor_const[proot->color_mode]);
	ret = gelib_fillrect(pdst,NULL,transcolor_const[pdst->color_mode]);

	if(gelib_getdual_support())
	{
		PGUI_REGION psd;

		psd = gelib_getdstrgn(GUI_SLVGMA_SURF,0);
		ret = gelib_fillrect(psd,NULL,transcolor_const[psd->color_mode]);
		psd = gelib_getdstrgn(GUI_SLVSCR_SURF,0);
		ret = gelib_fillrect(psd,NULL,transcolor_const[psd->color_mode]);
	}

	reset_object_record();
	return ret;
}

RET_CODE OSD_ClearRct(PGUI_VSCR pvscr,PGUI_RECT prct)
{
	RET_CODE ret;
	GUI_VSCR vscr_tmp;
	PGUI_REGION proot,pdst;

	ret = RET_FAILURE;
	if(pvscr == NULL)
	{//default parameters,that is you need not specify pvscr for simple
		MEMSET(&vscr_tmp,0x0,sizeof(vscr_tmp));
		vscr_tmp.dirty_flag = 1;
		vscr_tmp.dst_rgn = 0;
		vscr_tmp.root_rgn = 0;
		vscr_tmp.dst_layer = vlayer_idx;
		vscr_tmp.root_layer = GUI_MEMVSCR_SURF;
		
		proot = gelib_getdstrgn((UINT8)vscr_tmp.root_layer,(UINT8)vscr_tmp.root_rgn);
		ret = gelib_fillrect(proot,prct,transcolor_const[proot->color_mode]);
		
		vscr_tmp.color_mode = proot->color_mode;
		MEMCPY(&(vscr_tmp.frm),prct,sizeof(GUI_RECT));
		gelib_update2scrn(&vscr_tmp,&(vscr_tmp.frm));	
	}
	else
	{
		MEMCPY(&vscr_tmp,pvscr,sizeof(GUI_VSCR));
		MEMCPY(&(vscr_tmp.frm),prct,sizeof(GUI_RECT));
		proot = gelib_getdstrgn((UINT8)vscr_tmp.root_layer,(UINT8)vscr_tmp.root_rgn);
		ret = gelib_fillrect(proot,prct,transcolor_const[proot->color_mode]);

		vscr_tmp.dirty_flag = 1;
		vscr_tmp.suspend = 0;
		if(pvscr->dirty_flag)
			gelib_update2scrn(pvscr,&(pvscr->frm));
	}

	return ret;
}
/*####################################################################################*/

