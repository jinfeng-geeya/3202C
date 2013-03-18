/*-----------------------------------------------------------------------------
*
*	Copyright (C) 2005 ALI Corp. All rights reserved.
*
*	File: osd_common_draw.c
*
*	Content: 
*		support basic GUI draw function
*	History: 
*		2006/01/09 by Sunny Yin
*		Created this file. Inherited from new osd lib
*-----------------------------------------------------------------------------*/
#include <sys_config.h>
#include <api/libloader/ld_osd_lib.h>
#include "ld_osd_lib_internal.h"

static HANDLE	 ghOSDDrvHandle;
static UINT8 g_cur_region_id = 0;
UINT8	osd_vscr_buffer_temp[OSD_VSRC_MEM_MAX_SIZE+32];

#define osd_vscr_buffer (((UINT32)osd_vscr_buffer_temp+31)&0xffffffe0)

VSCR 	g_vscr;

typedef	UINT16	(*OSD_GET_LANG_ENV)(void);
typedef	BOOL	(*OSD_GET_OBJ_INFO)(UINT16 wLibClass,UINT16 uIndex,lpOBJECTINFO pObjectInfo);
typedef	UINT8* 	(*OSD_GET_RSC_DATA)(UINT16 wLibClass,UINT16 uIndex,lpOBJECTINFO objectInfo);
typedef 	ID_RSC 	(*OSD_GET_FONT_LIB)(UINT16 uChar);
typedef 	ID_RSC    (*OSD_GET_STR_LIB_ID)(UINT16 EnvID);

OSD_GET_LANG_ENV	osd_get_lang_env;
OSD_GET_OBJ_INFO	osd_get_obj_info;
OSD_GET_RSC_DATA	osd_get_rsc_data;
OSD_GET_FONT_LIB 	osd_get_font_lib;
OSD_GET_STR_LIB_ID	osd_get_str_lib_id;

void OSD_rsc_reg_cb(OSD_GET_LANG_ENV langenv, OSD_GET_OBJ_INFO objinfo,\
						OSD_GET_RSC_DATA rscdata, OSD_GET_FONT_LIB fontlib, \
						OSD_GET_STR_LIB_ID strlibid)
{
	osd_get_lang_env = langenv;
	osd_get_obj_info = objinfo;
	osd_get_rsc_data= rscdata;
	osd_get_font_lib = fontlib;
	osd_get_str_lib_id = strlibid;
}
/*-------------------------------------------------------------------
Name: OSD_SetDeviceHandle
Description: 
    set current OSD Driver handle.
Parameters: 
    HANDLE dev: the OSD Driver handle.
Return: 
    none
-------------------------------------------------------------------*/
void OSD_SetDeviceHandle(HANDLE dev)
{
	ghOSDDrvHandle = dev;	
}

BOOL OSDGetLocalVscr(lpVSCR lpVscr,UINT16 x,UINT16 y,UINT16 w,UINT16 h)
{
	OSD_SetRect(&lpVscr->vR, x,y,w,h);	

	if(OSD_GET_VSCR_SIZE(&lpVscr->vR) > OSD_VSRC_MEM_MAX_SIZE )
	{
		lpVscr->lpbScr = NULL;
		return FALSE;
	}

	if((g_vscr.updatePending)&&(NULL != g_vscr.lpbScr))
		OSDDrv_RegionWrite(ghOSDDrvHandle, g_cur_region_id, &g_vscr, &g_vscr.vR);
		
	lpVscr->lpbScr = (UINT8 *)osd_vscr_buffer;
	OSD_SetRect2(&g_vscr.vR,  &lpVscr->vR);
	
	if(OSD_INVALID_REGION != g_cur_region_id)
		OSDDrv_RegionRead(ghOSDDrvHandle, g_cur_region_id, lpVscr, &lpVscr->vR);

	return TRUE;
}

static void OSDVscr2Bitmap(lpVSCR pbVscr,BitMap_t* bmp)
{
	bmp->uLeft	= pbVscr->vR.uLeft;
	bmp->uTop	= pbVscr->vR.uTop;
	bmp->uWidth 	= pbVscr->vR.uWidth;
	bmp->uHeight = pbVscr->vR.uHeight;
	bmp->pData 	= pbVscr->lpbScr;

	bmp->stride	= OSD_GET_VSCR_STIDE(pbVscr);//(pbVscr->vR.uWidth)>>FACTOR;
	bmp->bitsPerPix = BIT_PER_PIXEL;
}

BOOL OSD_GetCharWidthHeight(UINT16 uCh,UINT8 font, UINT16* width, UINT16* height)
{
	OBJECTINFO	ObjInfo;
	ID_RSC		RscLibId;
	BOOL	ret;

	if(osd_get_font_lib!=NULL)
		RscLibId = osd_get_font_lib(uCh) | font;
	else
		RscLibId = OSD_GetDefaultFontLib(uCh) | font;

	if(osd_get_obj_info!=NULL)
		ret = osd_get_obj_info( RscLibId, uCh,&ObjInfo);
	else
		ret = OSD_GetObjInfo( RscLibId, uCh,&ObjInfo);
	if(ret)
	{
		*width	= ObjInfo.m_objAttr.m_wActualWidth;
		*height = ObjInfo.m_objAttr.m_wHeight;
		return TRUE;
	}
	else
		return FALSE;

}

UINT16 OSD_MultiFontLibStrMaxHW(UINT8* pString,UINT8 font, UINT16 *wH, UINT16 *wW,UINT16 strLen)
{
	UINT16 		wMaxH=0, wMaxW=0, wLen=0,w,h;
	ID_RSC		RscLibId;
	UINT16 		fontSize;
	UINT16		uString;
	UINT16		i = 0;
	//ID_RSC	    RscCharID = OSDExt_GetMsgLibId(OSD_GetLangEnvironment());

	while( 0 != (uString = ComMB16ToWord(pString)))	// = NULL?
	{				
		if(OSD_GetCharWidthHeight(uString,font,&w,&h))
		{
			wMaxH = (wMaxH > h)? wMaxH : h;
			wMaxW = (wMaxW > w)? wMaxW : w;
			wLen	+= w;
		}
		pString += 2;
		i++;
		if(strLen>0 && i==strLen)
		{
			wMaxH = h;
			wMaxW = w;				
			break;
		}
	}
	*wH = wMaxH;
	*wW = wMaxW;
	return wLen;
}

static void OSDIconCharData2Bitmap(FONTICONPROP* pObjAttr,UINT8* pDtaBitmap,BitMap_t* bmp, ID_RSC RscLibId)
{

	bmp->uWidth  = pObjAttr->m_wActualWidth;//RscLibInfo.m_objAttr.m_wActualWidth
	bmp->uHeight = pObjAttr->m_wHeight;

	bmp->stride = pObjAttr->m_wWidth*pObjAttr->m_bColor/8;

	bmp->pData   = pDtaBitmap;
	bmp->bitsPerPix = pObjAttr->m_bColor;
}

/**************************************************************************************
* Function Name:	OSDBitmapChangeColor 
*
* Description:		Change a bitmap one color to another color.
*					
* Arguments:
*		pBmp: 		IN/OUT;		The bitmap for changing color.
*		bOldColor;	IN;			The color index in the bitmap to be changed.
*		bNewColor:	IN;			The color index to be changed to.
*
* Return value:		
*		None
*
* NOTES:
*
**************************************************************************************/
void OSDBitmapChangeColor(BitMap_t* pBmp,UINT8 bOldColor, UINT8 bNewColor)
{
	UINT16 i,j;
	UINT8* pLineData;
	UINT8 pixsPerByte;
	UINT8 data,mask,pixBits,bytePixOffset;
	UINT32 byteOffset;

	//mask = 0;
	pixsPerByte = 8 / pBmp->bitsPerPix;
	//for(i=0;i<pBmp->bitsPerPix;i++)
	//	mask |= 0x01 << i;

	mask = (2<<pBmp->bitsPerPix) - 1;
		
	bOldColor &= mask;
	bNewColor &= mask;
	
	pLineData = pBmp->pData;// + pBmp->stride*i;
	for(i=0;i<pBmp->uHeight;i++)
	{
		for(j=0;j<pBmp->uWidth;j++)
		{
			byteOffset = j/pixsPerByte;
			data = *(pLineData + byteOffset);
			bytePixOffset = (UINT8)(pixsPerByte - 1 -  j % pixsPerByte);
			pixBits = (data >> bytePixOffset) & mask;
			
			if(pixBits==bOldColor)
			{
				data &= ~(mask<<bytePixOffset);
				data |= bNewColor << bytePixOffset;
				*(pLineData + byteOffset) = data;
			}			
		}
		pLineData += pBmp->stride;
	}
}

/**************************************************************************************
* Function Name:	OSDBitmapFormatTransfer 
*
* Description:		"Copy" a bitmap to another bitmap.
*					
* Arguments:
*		bmpDest: 	IN/OUT;		The destination bitmap for copy.
*		bmpSrc;		IN;			The source bitmap fore copy.
*		transColor:	IN;			tranparent color.
*		foreColor:	IN:			foreground color.
*
* Return value:		
*		None
*
* NOTES:
*	When transColor!=foreColor, destination bitmap is mixed with source bitmap.
*	Otherwise, source bitmap is copyed to destination bitmap directly.
* Limitaion: Source bitmap must has the same format(bits per pix) with  destination 
*			bitmap. Or source bitmap is 2-value bitmap.
**************************************************************************************/
static BOOL OSDBitmapFormatTransfer(BitMap_t* bmpDest,BitMap_t* bmpSrc,UINT8 transColor,UINT8 foreColor)
{
	struct OSDRect destRect,srcRect,crossRect;
	UINT16 i,j;
	UINT8 maskDest,maskSrc;
	UINT8 backByte,foreByte,foreBits;
	UINT8 fBytePixOffset,bBytePixOffset;
	UINT8 pixsPerByteDest,pixsPerByteSrc;
	UINT32 fLineStartPixsOffset,bLineStartPixsOffset,bLineByteOffset;
	
	UINT8	*pForgLineData,*pBackLineData;
	UINT8	*pFgColor;
	BOOL	bMix;
	
	/* If data bitmap data is null, return */
	if((bmpSrc->pData==NULL) || (bmpDest->pData == NULL))
		return FALSE;
	/* Source bitmap bpp must be 1, or with same format withe dest bitmap. */
	if(bmpSrc->bitsPerPix!=1 && bmpSrc->bitsPerPix!=bmpDest->bitsPerPix)
		return FALSE;

	destRect.uLeft   = bmpDest->uLeft;
	destRect.uTop    = bmpDest->uTop;
	destRect.uWidth  = bmpDest->uWidth;
	destRect.uHeight = bmpDest->uHeight;

	srcRect.uLeft   = bmpSrc->uLeft;
	srcRect.uTop    = bmpSrc->uTop;
	srcRect.uWidth  = bmpSrc->uWidth;
	srcRect.uHeight = bmpSrc->uHeight;

	/* Get the cross rectangle of source and dest bitmap. */
	OSD_GetRectsCross(&destRect,&srcRect,&crossRect);

	/* If the cross rectange is emplty return. */
	if((crossRect.uWidth*crossRect.uHeight)==0)
		return FALSE;

	maskDest = (2<<bmpDest->bitsPerPix - 1) - 1;
	maskSrc = (2<<bmpSrc->bitsPerPix- 1) - 1;
	
	transColor &= maskSrc;
	foreColor  &= maskDest;

	/* If tansparenct color is not same with forground color, the mixing is ture,
	   else not mixing. */
	if((transColor!=foreColor) || (foreColor == 0))
		bMix = TRUE;
	else
		bMix = FALSE;

	/* pixs per byte */
	pixsPerByteDest = 8 / bmpDest->bitsPerPix;
	pixsPerByteSrc  = 8 / bmpSrc->bitsPerPix;

	/* The dest copy data area every line's start pix index in the first byte. */
	bLineStartPixsOffset = crossRect.uLeft % pixsPerByteDest;
	fLineStartPixsOffset = (crossRect.uLeft - srcRect.uLeft)% pixsPerByteSrc;

	/* Decide what data are copyed to the dest bitmap */
	if(bmpSrc->bitsPerPix < bmpDest->bitsPerPix)
		pFgColor = &foreColor;	/* 1bit per pix -- font data */
	else
		pFgColor = &foreBits;	/* icon data */
		

	if( NULL != bmpDest->pData)
	{
		pBackLineData = bmpDest->pData + (crossRect.uLeft-destRect.uLeft) *  bmpDest->bitsPerPix / 8
						+bmpDest->stride * (crossRect.uTop - destRect.uTop); 
		pForgLineData = bmpSrc->pData+ (crossRect.uLeft-srcRect.uLeft) * bmpSrc->bitsPerPix / 8
						+ bmpSrc->stride * (crossRect.uTop - srcRect.uTop);
		for(i=0;i<crossRect.uHeight;i++)	/* every line */
		{
			for(j=0;j<crossRect.uWidth;j++)	/* every pix */
			{
				if(maskDest == maskSrc && maskDest==0xFF)
				{
					foreByte = *(pForgLineData + fLineStartPixsOffset + j);
					if(!bMix ||  foreByte != transColor )
						*( pBackLineData + bLineStartPixsOffset + j ) = foreByte;
					
					continue;
				}

			
				foreByte = *(pForgLineData + (fLineStartPixsOffset + j) / pixsPerByteSrc) ;	/* Foreground byte of pix j from fLineStartPixsOffset */
		    		fBytePixOffset = (pixsPerByteSrc - 1 - (fLineStartPixsOffset + j) % pixsPerByteSrc) * bmpSrc->bitsPerPix;				/* Foreground pix of j from fLineStartPixsOffset's data offset in the foreByte */

				foreBits = (foreByte >> fBytePixOffset) & maskSrc ;
				if(!bMix ||  foreBits != transColor )
				{
					bLineByteOffset = (bLineStartPixsOffset + j)/pixsPerByteDest;
												
					bBytePixOffset = (pixsPerByteDest - 1 - (bLineStartPixsOffset + j) % pixsPerByteDest) * bmpDest->bitsPerPix;			/* Background pix of j from fLineStartPixsOffset's data offset in the backByte */
					backByte = *( pBackLineData + bLineByteOffset );								/* Background byte of pix j from bLineStartPixsOffset */

					backByte &= ~(maskDest<<bBytePixOffset);

					backByte |= *pFgColor << bBytePixOffset;

					*(pBackLineData + bLineByteOffset) = backByte;
				}	

			}
			pBackLineData += bmpDest->stride;
			pForgLineData += bmpSrc->stride;
		}
	}
	
	return TRUE;
		
}

static void OSDDrawBmp(UINT8 *pDtaBitmap,OBJECTINFO*RscLibInfo, lpVSCR pbVscr, struct OSDRect* r, ID_RSC RscLibId,UINT16 dwColorFlag)
{
	UINT8 		bTransColor,bForgColor;
	BitMap_t	destBmp,srcBmp;

	if((NULL == pbVscr) || (NULL == pbVscr->lpbScr))
		return;

	bForgColor	= (UINT8)(dwColorFlag);

	srcBmp.uLeft = r->uLeft;
	srcBmp.uTop  = r->uTop;
	OSDIconCharData2Bitmap(&RscLibInfo->m_objAttr,pDtaBitmap,&srcBmp, RscLibId);
	srcBmp.uWidth = r->uWidth > srcBmp.uWidth ? srcBmp.uWidth : r->uWidth;
	srcBmp.uHeight = r->uHeight > srcBmp.uHeight ? srcBmp.uHeight : r->uHeight;
	
	if(1 == srcBmp.bitsPerPix)
		bTransColor = (dwColorFlag & C_MIXER) ? 0x1: 0x0;
	else
		bTransColor = (dwColorFlag & C_MIXER) ? OSD_TRANSPARENT_COLOR: bForgColor;
	OSDVscr2Bitmap(pbVscr,&destBmp);
	OSDBitmapFormatTransfer(&destBmp,&srcBmp,bTransColor,bForgColor);

	if(((UINT8)(dwColorFlag>>16)) != ((UINT8)(dwColorFlag>>24)))
	{
		srcBmp.stride = destBmp.stride;
		srcBmp.bitsPerPix	= destBmp.bitsPerPix;
		srcBmp.pData		= destBmp.pData 
			+ (srcBmp.uTop - destBmp.uTop) * destBmp.stride
			+ (srcBmp.uLeft - destBmp.uLeft) * destBmp.bitsPerPix/8;

		OSDBitmapChangeColor(&srcBmp,(UINT8)(dwColorFlag>>24),(UINT8)(dwColorFlag>>16));
	}
}

/*-------------------------------------------------------------------
Name: OSD_DrawPixel
Description: 
    Draw a pixel on screen.
Parameters: 
    x - x coordination, from left to right
    y - y coordiantion, from top to bottom
    color - the pixel color
    pVscr - specifies which screen to draw. If NULL, it drawes directly 
            on the physical screen.
Return: 
    none
-------------------------------------------------------------------*/
void OSD_DrawPixel(UINT16 x, UINT16 y, UINT8 color, lpVSCR pVscr)
{
	OSD_DrawFill(x, y, 1, 2, color, pVscr);
}

/*-------------------------------------------------------------------
Name: OSD_DrawHorLine
Description: 
    Draw a horizontal line.
Parameters: 
    x, y - the starting point coordiantion.
    w - the line length in pixel.
    color - line color
    pVscr - specifies which screen to draw. If NULL, it drawes directly 
            on the physical screen.
Return: 
    none
-------------------------------------------------------------------*/
void OSD_DrawHorLine(UINT16 x, UINT16 y, UINT16 w, UINT8 color, lpVSCR pVscr)
{
	if(!(color&C_NOSHOW))
		OSD_DrawFill(x, y, w, 1, color, pVscr);
}

/*-------------------------------------------------------------------
Name: OSD_DrawVerLine
Description: 
    Draw a vertical line.
Parameters: 
    x, y - the starting point coordiantion.
    h - the line length in pixel.
    color - line color
    pVscr - specifies which screen to draw. If NULL, it drawes directly 
            on the physical screen.
Return: 
    none
-------------------------------------------------------------------*/
void OSD_DrawVerLine(UINT16 x, UINT16 y, UINT16 h, UINT8 color, lpVSCR pVscr)
{
	if(!(color&C_NOSHOW))
		OSD_DrawFill(x, y, 1, h, color, pVscr);
}

/*-------------------------------------------------------------------
Name: OSD_DrawFill
Description: 
    Fill a rectangle.
Parameters: 
    x, y - the top-left coordination of the rectangle.
    w, h - width and height of the rectangle.
    color - the filling color
    pVscr - specifies which screen to draw. If NULL, it drawes directly 
            on the physical screen.
Return: 
    none
-------------------------------------------------------------------*/
void OSD_DrawFill(UINT16 x, UINT16 y, UINT16 w, UINT16 h, UINT8 color, lpVSCR pVscr)
{
	struct OSDRect	r;
	OSD_SetRect(&r, x, y, w, h);
	OSD_DrawFrame(&r, color, pVscr);
}

/*-------------------------------------------------------------------
Name: OSD_DrawChar
Description: 
    Draw a character.
Parameters: 
    x, y - the top-left coordination of the rectangle.
    fg_color - foreground color
    bg_color - background color
    ch - a unicode character
    font - font type. 0 for default font.
Return: 
    char width
-------------------------------------------------------------------*/
UINT16 OSD_DrawChar(UINT16 x, UINT16 y, UINT16 fg_color, UINT16 bg_color, UINT16 ch, UINT8 font, lpVSCR pVscr)
{
	UINT16 		height =0, width =0;
	VSCR	vscr;	
	BOOL	bGetVscr = FALSE;
	OBJECTINFO	ObjInfo;
	ID_RSC		RscLibId;
	UINT8*		pBitMapData;
	struct OSDRect	r;

	if(OSD_GetCharWidthHeight(ch, font,&width,&height) == FALSE)
		return 0;

	if(osd_get_font_lib!=NULL)
		RscLibId = osd_get_font_lib(ch) | font;
	else
		RscLibId = OSD_GetDefaultFontLib(ch) | font;
	
	//get vscr if necessary
	if((NULL == pVscr) || (NULL == pVscr->lpbScr))
	{
		bGetVscr = OSDGetLocalVscr(&vscr, x, y, width, height);
		if(!bGetVscr)
			return 0;
	}
	else
	{
		MEMCPY(&vscr, pVscr, sizeof(VSCR));
	}

	//backgroud
	if( !(C_NOSHOW& bg_color)&& bGetVscr)
	{
		OSD_DrawFill(x, y, width, height, (UINT8)bg_color, &vscr);
	}
	
	// Load bitmap
	if(osd_get_rsc_data!=NULL)
		pBitMapData =(UINT8*)osd_get_rsc_data(RscLibId, ch, &ObjInfo);
	else
		pBitMapData =(UINT8*)OSD_GetRscObjData(RscLibId, ch, &ObjInfo);

	if(NULL !=pBitMapData)
	{
		OSD_SetRect(&r, x, y, width, height);
		OSDDrawBmp(pBitMapData, &ObjInfo, &vscr, &r, RscLibId, fg_color);
	}

	if(bGetVscr)
	{
		OSDDrv_RegionWrite(ghOSDDrvHandle, g_cur_region_id, &vscr, &vscr.vR);
	}

	return width;
}

/*-------------------------------------------------------------------
Name: OSD_DrawFrame
Description: 
    Fill a rectangle with specified color. It is just like
    OSD_DrawFill, but with different arguments.
Parameters: 
    pFrame - [in]specifies the frame coordination
    Color - color used to fill the frame.
Return: 
    none
-------------------------------------------------------------------*/
void OSD_DrawFrame(struct OSDRect* pFrame, UINT8 color, lpVSCR pVscr)
{
	VSCR	vscr;
	BOOL	bGetVscr = FALSE;
	//get vscr if necessary
	if((NULL == pVscr) || (NULL == pVscr->lpbScr))
	{
		bGetVscr = OSDGetLocalVscr(&vscr, pFrame->uLeft, pFrame->uTop, pFrame->uWidth, pFrame->uHeight);
	}
	else
	{
		MEMCPY(&vscr, pVscr, sizeof(VSCR));
	}

	//if the vscr is valid
	if(NULL != vscr.lpbScr)
	{
		struct OSDRect	target_rect;
		//only draw the cross rectangle if target rect outof the vscr rect
		OSD_GetRectsCross(pFrame, &vscr.vR, &target_rect);
		if(target_rect.uHeight|target_rect.uWidth)
		{
			UINT8		mask = COLOR_N -1;
			UINT8		pixsPerByte;
			UINT16	i,j;
			UINT8*	pLineData;
			UINT8		tmp_data;
            UINT16      tmp_len;
			
			pixsPerByte = 8 / BIT_PER_PIXEL;

			//get every byte data
			//color mode		bit per pixel		pixel per byte		if color = 1
			//256				8					1					data = 00 00 00 01
			//16				4					2					data = 00 01 00 01 = 1<<4 | 1
			// 4				2					4					data = 01 01 01 01 = 1<<6 | 1 << 4 | 1 << 2 | 1
			// 2				1					8					data = 11 11 11 11 = ......
			for(i = 8-BIT_PER_PIXEL; i > 0; i -= BIT_PER_PIXEL)
				color |= (color&mask)<<i;

			for(i = 0; i < target_rect.uHeight; i++)
			{
				pLineData = vscr.lpbScr + (target_rect.uTop +  i - vscr.vR.uTop) * OSD_GET_VSCR_STIDE(&vscr)
					+ (target_rect.uLeft-vscr.vR.uLeft / pixsPerByte);					

				//the head of the line
				tmp_len = target_rect.uLeft % pixsPerByte;
				
				if(tmp_len > 0)
				{
					tmp_data = *pLineData;
					//当width<pixsPerByte 情况较为复杂，要考虑j + target_rect.uWidth > tmp_len
					for(j = tmp_len ; (j >0) && (j + target_rect.uWidth > tmp_len); j --)
					{
						*pLineData &= (~(mask<<(j * BIT_PER_PIXEL)));
						*pLineData |= color & (mask<<(j * BIT_PER_PIXEL));
					}						
					pLineData++;
				}

				tmp_len = (target_rect.uWidth-target_rect.uLeft % pixsPerByte)/pixsPerByte;
				if(tmp_len > 0)
				{
					MEMSET(pLineData, color, tmp_len);
					pLineData += tmp_len;
				}

				//the tail of the line
				tmp_len = (target_rect.uWidth-target_rect.uLeft % pixsPerByte)%pixsPerByte;
				if(tmp_len > 0)
				{
					tmp_data = *pLineData;
					for(j = 8-(tmp_len) * BIT_PER_PIXEL ; j < 8 ; j += BIT_PER_PIXEL)
					{
						*pLineData &= (~(mask<<j));
						*pLineData |= color & (mask<<j);
					}						
				}
			}
			if(bGetVscr)
				OSDDrv_RegionWrite(ghOSDDrvHandle, g_cur_region_id, &vscr, &target_rect);
			return;
		}
	}
	else
		OSDDrv_RegionFill(ghOSDDrvHandle, g_cur_region_id, pFrame, color);	

}

UINT8* OSD_GetUnicodeString(UINT16 uIndex)
{
	OBJECTINFO	RscLibInfo;    
	ID_RSC	    RscId=0;
	UINT16	EnvID = 0;

	if(osd_get_lang_env!=NULL)
		EnvID = osd_get_lang_env();
	
	if(osd_get_str_lib_id!=NULL)
		RscId = osd_get_str_lib_id(EnvID);    

	if(osd_get_rsc_data!=NULL)
		return osd_get_rsc_data(RscId, uIndex, &RscLibInfo);

	return NULL;	
}

/*-------------------------------------------------------------------
Name: OSD_DrawText
Description: 
    Draw a text string in the specified rectangle.
Parameters: 
    pRect - [in]specifies the rectangle coordination.
    pszText - [in]specifies the text string.
    Color - the text color param, refer to OSD_COLOR_PARAM.
    Style - the text style param, refer to OSD_TEXT_PARAM.
    pVscr - specifies which screen to draw. If NULL, it drawes directly 
            on the physical screen.
Return: 
    none
-------------------------------------------------------------------*/
UINT16 OSD_DrawText(struct OSDRect* pRect, UINT8* pText, UINT16 Color, UINT8 AssignStyle, UINT8 FontSize, lpVSCR pVscr)
{
	UINT16 		uMaxHeight, uDetHeight, uMaxWidth,chw,chh;
	UINT16		uLen=0, uStrLen;
	UINT16		uXpos, uYpos, uCount = 0;
	UINT16		uStr;

	if(Color&C_NOSHOW)
		return 0;
	if(pText == NULL)
		return 0;

	uStrLen = OSD_MultiFontLibStrMaxHW(pText, FontSize, &uMaxHeight, &uMaxWidth,0);
	if (!(uStrLen*uMaxHeight*uMaxWidth))
		return 0;
	uXpos = pRect->uLeft;
	uYpos = pRect->uTop;
	//	Adjust Y Coordinate
	if (pRect->uHeight > uMaxHeight)
	{
		if (GET_VALIGN(AssignStyle) == C_ALIGN_VCENTER)
			uYpos += (pRect->uHeight - uMaxHeight)>>1;
		else if (GET_VALIGN(AssignStyle) == C_ALIGN_BOTTOM)
			uYpos += pRect->uHeight - uMaxHeight;
	}
	//	Adjust X Coordinate
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

	while(0 != (uStr = ComMB16ToWord(pText)))	// == NULL?
	{
		OSD_GetCharWidthHeight(uStr, FontSize,&chw,&chh);
		if(uXpos + chw> pRect->uLeft + pRect->uWidth )
			break;
		
		uXpos += OSD_DrawChar(uXpos, uYpos, Color, C_NOSHOW, uStr, FontSize, pVscr);
		pText += 2;
		uCount ++;
	}
	return uCount;
}

////////////////////////////////////////////////////////////////////////
// virtual screen related functions

void OSD_UpdateVscr(VSCR* pVscr)
{	
	if(pVscr->lpbScr!=NULL)
	{
		if(pVscr->updatePending)
		{
			//DDrv_WriteData2Frm(ghOSDDrvHandle,pVscr,&pVscr->vR);
			OSDDrv_RegionWrite(ghOSDDrvHandle, g_cur_region_id, pVscr,&pVscr->vR);
			pVscr->updatePending = 0;
		}
	}
}

VSCR* OSD_GetVscr(struct OSDRect* pRect, UINT32 bFlag)
{
	VSCR		*pGVscr;
	UINT16 stride;
	UINT32 mem_size_r;
	struct OSDRect corner;

	pGVscr = &g_vscr;	

	mem_size_r = OSD_GET_VSCR_SIZE(pRect);
			
	if(pGVscr->lpbScr) 	/* Global virtual screen is valid. */
	{
		if(OSD_RectInRect(&pGVscr->vR,pRect)) /* Global's virtual screen cover the area that requied */
			return pGVscr;
		else	/* Global's virtual screen can't cover the area that requied */
		{
			OSD_UpdateVscr(pGVscr);
			pGVscr->lpbScr = NULL;
		}
	}
	
	OSD_SetRect2(&pGVscr->vR, pRect);
	if(bFlag == OSD_DIRDRAW)
		pGVscr->lpbScr = NULL;
	else
	{
		if(mem_size_r > OSD_VSRC_MEM_MAX_SIZE)
			pGVscr->lpbScr = NULL;
		else
			pGVscr->lpbScr = (UINT8 *)osd_vscr_buffer;
	}
	
	if(pGVscr->lpbScr != NULL)
	{
		if(bFlag & OSD_GET_BACK)
			OSDDrv_RegionRead(ghOSDDrvHandle,g_cur_region_id,pGVscr, pRect);
		else if(bFlag & OSD_GET_CORNER)
		{
			OSD_SetRect(&corner, pRect->uLeft, pRect->uTop, CIRCLE_PIX*2, pRect->uHeight);
			OSDDrv_RegionRead(ghOSDDrvHandle,g_cur_region_id,pGVscr, &corner);
			
			corner.uLeft = pRect->uLeft + pRect->uWidth - CIRCLE_PIX*2;
			OSDDrv_RegionRead(ghOSDDrvHandle,g_cur_region_id,pGVscr, &corner);
		}				
		else
			MEMSET(pGVscr->lpbScr,OSD_TRANSPARENT_COLOR_BYTE, mem_size_r);
	}
	
	pGVscr->updatePending = FALSE;
	
	return pGVscr;
}

