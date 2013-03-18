/*-----------------------------------------------------------------------------
*	Copyright (C) 2010 ALI Corp. All rights reserved.
*	GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef _OSD_PLUGIN_H
#define _OSD_PLUGIN_H

#include <basic_types.h>
#include <api/libc/string.h>
#include "osd_lib.h"
#include "ge_draw_primitive.h"

#define OSD_SYSTEM_PAL		0
#define OSD_SYSTEM_NTSC	1

#define OSD_SetVscrBuff(pvscr,pbuff) do{(pvscr)->lpbuff = pbuff;}while(0)
#define OSD_LibRegionFill OSD_RegionFill
#define OSD_TaskBuffer_Init(...)	do{}while(0)
#define OSD_TaskBuffer_Free(...) do{}while(0)

typedef GUI_RECT OSD_RECT;
typedef PGUI_RECT POSD_RECT;//for compatible with old structure,old name,new member and usage.

void OSD_DrawPicture(UINT16 x, UINT16 y, UINT16 idPicture,ID_RSC RscLibId, UINT32 Style, PGUI_VSCR pVscr);
void OSD_DrawPixel(UINT16 x, UINT16 y, UINT32 color, PGUI_VSCR pVscr);
void OSD_DrawHorLine(UINT16 x, UINT16 y, UINT16 w, UINT32 color, PGUI_VSCR pVscr);
void OSD_DrawVerLine(UINT16 x, UINT16 y, UINT16 h, UINT32 color, PGUI_VSCR pVscr);
void OSD_DrawFill(UINT16 x, UINT16 y, UINT16 w, UINT16 h, UINT32 color,PGUI_VSCR pVscr);
RET_CODE OSD_RegionFill(PGUI_RECT pFrame,UINT32 color);

void OSD_ShowOnOff(UINT8 onoff);
void OSD_SetRectOnScreen(PGUI_RECT rect);
void OSD_GetRectOnScreen(PGUI_RECT rect);
BOOL OSD_GetRegionData(PGUI_VSCR pVscr,PGUI_RECT rect);
void OSD_RegionWrite(PGUI_VSCR pVscr,PGUI_RECT r);
void OSD_SetClipRect(PGUI_RECT pRect,UINT32 param);
void OSD_ClearClipRect(void);
void OSD_SetCurRegion(UINT8 uRegionID);
UINT8 OSD_GetCurRegion();

RET_CODE OSD_Scale(UINT8 uScaleCmd, UINT32 uScaleParam);
RET_CODE OSD_SetPallette(UINT8 *p_pallette,UINT16 wN);
RET_CODE OSD_CreateRegion(UINT8 region_id,PGUI_RECT pRect,UINT32 param);
RET_CODE OSD_DeleteRegion(UINT8 region_id);
UINT8 OSD_GetCurRegion(void);
void OSD_SetCurRegion(UINT8 uRegionID);
void OSD_SetDeviceHandle(HANDLE dev);
void OSD_RegionInit();
void OSD_DrawFrame(PGUI_RECT rct,UINT32 color,PGUI_VSCR pvscr);

UINT8 *OSD_GetTaskVscrBuffer(ID task_id);
#endif

