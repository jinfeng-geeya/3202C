/*-----------------------------------------------------------------------------
*
*	Copyright (C) 2005 ALI Corp. All rights reserved.
*
*	File: osd_common_draw.h
*
*	Content: 
*		support basic GUI draw function
*	History: 
*		2006/01/09 by Sunny Yin
*		Created this file. Inherited from new osd lib
*-----------------------------------------------------------------------------*/
#ifndef _OSD_COMMON_DRAW_H_
#define _OSD_COMMON_DRAW_H_

#ifdef PC_SIMULATE
#include <osddrv.h>
#else
#include <hld/osd/osddrv.h>
#endif

// bit 0 - 23 is used as RGB color,  to support HD OSD
#define	C_ALPHA0				(0<<24)		// no transparent.
#define	C_ALPHA1				(1<<24)		// half transparent.
#define	C_ALPHA2				(2<<24)		// half transparent.
#define	C_ALPHA3				(3<<24)		// half transparent.
#define	C_ALPHA4				(4<<24)		// half transparent
#define	C_ALPHA5				(5<<24)		// half transparent
#define	C_ALPHA6				(6<<24)		// transparent

#define	C_NOSHOW				(1<<27)		// Only effect Fg
#define	C_MIXER					(1<<28)		// Only effect Bg/Fg
#define	C_DOUBLE_LINE			(1<<29)		// Only effect Line
#define	C_ICON_IDX				(1<<29)		// Only effect Bg in C_WS_PIC_DRAW, shared with C_DOUBLE_LINE, for 16bit or 32bit osd, 
											// use this flag to indicate the bg color is a icon index or color index
#define	C_MULTI_LINE			(1<<30)		// only effect Line
#define	C_TRANS_NODRAW			(1<<31)		// Only effect Fg
#define C_NOSHOW_TEXT			C_NOSHOW	

#define GET_COLOR_IDX(a)		((a)&0x7ffffff)
#define GET_COLOR_ALPHA(a)		(((a)>>24)&0x7)
#define GET_COLOR_STYLE(a)		((a)&0xf8000000)
#define CHECK_COLOR_STYPE(a, attr)	((a)&(attr))

#define	OSD_DIRDRAW			0
#define	OSD_REQBUF			1
#define OSD_GET_CORNER		2
#define OSD_GET_BACK   		4

typedef UINT16 ID_RSC;
#define	OSD_INVALID_REGION		0xFF
#define	INVALID_INDEX			0

#define BE_TO_LOCAL_WORD(val)   ((val))

#ifdef OSD_DRAW_TASK_SUPPORT_NUM
#define OSD_DRAW_TASK_MAX OSD_DRAW_TASK_SUPPORT_NUM
#else
#define OSD_DRAW_TASK_MAX 25
#endif

typedef struct
{
	UINT16 uLeft;
	UINT16 uTop;
	UINT16 uWidth;       /* Width of the pixmap */
	UINT16 uHeight;      /* Height of the pixmap */
	UINT16 stride;      /* Number of bytes per line in the pixmap */
	UINT16 bitsPerPix;	//
	UINT8  *pData;
}BitMap_t;

typedef enum{
	C_WS_LINE_DRAW,
	C_WS_LINE_CIRCL_DRAW,
	C_WS_PIC_DRAW,
	C_WS_USER_DEFINE
} WSTYLE_TYPE;

typedef struct {
	UINT32 bWinStyleType;		// WSTYLE_TYPE: bit0-bit15, OSD color mode, bit16-bit31.
	UINT32 wTopLineIdx;         // color index or icon index
	UINT32 wLeftLineIdx;        // as above
	UINT32 wRightLineIdx;       // as above
	UINT32 wBottomLineIdx;     	// as above
	UINT32 wBgIdx;             	// Different Mode has Different Define
	UINT32 wFgIdx;             	// Different Mode has Different Define

	UINT32 wLeftTopIdx;			// icon index, only for picture mode
	UINT32 wLeftButtomIdx;		// as above
	UINT32 wRightTopIdx; 		// as above
	UINT32 wRightButtomIdx;		// as above
	UINT16 IconLibIdx;
}WINSTYLE, *PWINSTYLE;
typedef const WINSTYLE *PCWINSTYLE;

#define SST_GET_STYLE(a)	((a)&0x0000ffff)
#define SST_GET_COLOR(a)	((a)>>16)
#define SST_SET_COLOR(a) 	((a)<<16)

//--------------------------- Function Prototype ----------------------------//
void OSD_ShowOnOff(UINT8	byOnOff);
void OSD_SetRectOnScreen(struct OSDRect* rect);
void OSD_GetRectOnScreen(struct OSDRect* rect);
BOOL OSD_GetRegionData(VSCR * pVscr, struct OSDRect* rect);
void OSD_ChangeScrPosi(UINT16 wStartCol, UINT16 wStartRow);
RET_CODE OSD_Scale(UINT8 uScaleCmd, UINT32 uScaleParam);
UINT8* OSD_GetRscPallette(UINT16 wPalIdx);

RET_CODE OSDLib_RegionFill(struct OSDRect *pFrame,UINT32 color);
RET_CODE OSDLib_RegionWrite(VSCR *pVscr,struct OSDRect *rect);
RET_CODE OSDLib_RegionRead(VSCR *pVscr,struct OSDRect *rect);
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
void OSD_DrawPixel(UINT16 x, UINT16 y, UINT32 color, lpVSCR pVscr);

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
void OSD_DrawHorLine(UINT16 x, UINT16 y, UINT16 w, UINT32 color, lpVSCR pVscr);

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
void OSD_DrawVerLine(UINT16 x, UINT16 y, UINT16 h, UINT32 color, lpVSCR pVscr);

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
void OSD_DrawFill(UINT16 x, UINT16 y, UINT16 w, UINT16 h, UINT32 color, lpVSCR pVscr);

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
    pVscr - specifies which screen to draw. If NULL, it drawes directly 
            on the physical screen.
Return: 
    char width
-------------------------------------------------------------------*/
UINT16 OSD_DrawChar(UINT16 x, UINT16 y, UINT32 fg_color, UINT32 bg_color, UINT16 ch, UINT8 font, lpVSCR pVscr);

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
void OSD_DrawFrame(struct OSDRect* pFrame, UINT32 color, lpVSCR pVscr);

/*-------------------------------------------------------------------
Name: OSD_DrawFrameClip
Description: 
    Draw a frame with clip region returned
Parameters: 
    pFrame - [in]the frame coordination.
    pClip - [out]to get the clip rect for flicker free.
    Color - the frame color param, refer to OSD_COLOR_PARAM.
    Style - the frame style param, refer to OSD_FRAME_PARAM.
    pVscr - specifies which screen to draw. If NULL, it drawes directly 
            on the physical screen.
Return: 
    none
-------------------------------------------------------------------*/
void OSD_DrawFrameClip(struct OSDRect* pFrame, struct OSDRect* pClip, UINT32 Color, UINT32 Style, lpVSCR pVscr);

/*-------------------------------------------------------------------
Name: OSD_DrawPicture
Description: 
    Draw a picture in the specified rectangle.
Parameters: 
    pRect - [in]specifies the rectangle coordination.
    idPicture - picture resource id.
    Style - the picture style param, refer to OSD_PICTURE_PARAM.
    pVscr - specifies which screen to draw. If NULL, it drawes directly 
            on the physical screen.
Return: 
    none
-------------------------------------------------------------------*/
void OSD_DrawPicture(UINT16 x, UINT16 y, UINT16 idPicture,ID_RSC RscLibId, UINT32 Style, lpVSCR pVscr);


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
UINT16 OSD_DrawText(struct OSDRect* pRect, UINT8* pText, UINT32 Color, UINT8 AssignStyle, UINT8 FontSize, lpVSCR pVscr);

/*-------------------------------------------------------------------
Name: OSD_DrawTexture
Description: 
    Draw texture in the specified frame.
Parameters: 
    pFrame - [in]specifies the frame coordination to draw texture.
    bShIdx - window style index.
    pClip - [in]specifes the clip frame offset by uLeft and uTop
                the uWidth is used to specify the fill with when 
                the win style type is C_WS_LINE_DRAW or 
                C_WS_LINE_CIRCL_DRAW, and with valid wins style 
                subtype
            [out]the client frame.
    dwClipParam - [in]the clip frame param, refer to OSD_FRAME_PARAM
    pVscr - specifies which screen to draw. If NULL, it drawes directly 
            on the physical screen.
Return: 
    TRUE: drawing clip region in mixer mode
    FALSE: drawing clip region in overwriting mode
-------------------------------------------------------------------*/
//BOOL OSD_DrawTexture(struct OSDRect* pFrame, UINT8 bShIdx, POSD_RECT pClip, UINT32 dwClipParam, lpVSCR pVscr);

/*-------------------------------------------------------------------
Name: OSD_DDrawRenderRect
Description: 
    Render the bitmap to the specified rectangel.
Parameters: 
    pRect - [in]the rectangel coordiantion.
    idBitmap - bitmap resource id.
    bMode - specifies the render mode.
            C_WS_PIXEL_TO_LINE: use the first pixel of each line to render 
                                the whole line. It is a optimized mode 
                                for direct draw.
            otherwise, render the whole picture in the frame.
    pVscr - specifies which screen to draw. If NULL, it drawes directly 
            on the physical screen.
Return: 
    none.
-------------------------------------------------------------------*/
void OSD_DrawRenderRect(struct OSDRect* pRect, UINT16 idBitmap, UINT8 bMode, lpVSCR pVscr);


/*-------------------------------------------------------------------
Name: OSD_DrawStyleRect
Description: 
    Draw texture in the specified frame without clip region.
Parameters: 
    pFrame - [in]specifies the frame coordination to draw texture.
    bShIdx - window style index.
    pVscr - specifies which screen to draw. If NULL, it drawes directly 
            on the physical screen.
Return: 
    none
-------------------------------------------------------------------*/
void OSD_DrawStyleRect(struct OSDRect* pRect, UINT8 bShIdx, lpVSCR pVscr);

//added
BOOL    OSD_GetCharWidthHeight(UINT16 uString,UINT8 font, UINT16* width, UINT16* height);
BOOL OSD_GetThaiCellWidthHeight(struct thai_cell *cell, UINT8 font, UINT16* width, UINT16* height);
UINT16  OSD_MultiFontLibStrMaxHW(UINT8* pString,UINT8 font, UINT16 *wH, UINT16 *wW,UINT16 strLen);
UINT8*  OSD_GetUnicodeString(UINT16 uIndex);
RET_CODE OSD_SetPallette(UINT8 *p_pallette,UINT16 wN);
RET_CODE OSD_CreateRegion(UINT8 region_id,struct OSDRect *pRect,UINT32 param);
RET_CODE OSD_DeleteRegion(UINT8 region_id);

void OSD_SetClipRect(enum CLIPMode ClipMode, struct OSDRect* pRect);
void OSD_ClearClipRect(void);
void OSD_SetDeviceHandle(HANDLE dev);

void OSD_TaskBuffer_Free(ID task_id,UINT8 *p_config);
void OSD_TaskBuffer_Init(ID task_id,UINT8 *p_config);
void OSD_GolobalVscr_Init(void);
UINT8* OSD_GetTaskVscrBuffer(ID task_id);
lpVSCR OSD_GetTaskVscr(ID task_id);
#endif

