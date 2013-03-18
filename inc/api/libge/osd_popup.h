/*-----------------------------------------------------------------------------
*	Copyright (C) 2010 ALI Corp. All rights reserved.
*	GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef _OSD_POPUP_H
#define _OSD_POPUP_H

#include <basic_types.h>
#include <api/libc/string.h>
#include "osd_lib.h"
#include "osd_primitive.h"

void OSD_RectBackUp(PGUI_VSCR lpVscr,UINT8 *buf,PGUI_RECT pRc);
void OSD_RectRestore(PGUI_VSCR lpVscr,UINT8 *buf,PGUI_RECT pRc);

RET_CODE OSD_UpdateMsgPopup(void);
RET_CODE OSD_ClearMsgPopup(void);//msg popup is blend with main UI scrn
RET_CODE OSD_ModalPopupInit(UINT32 param);
RET_CODE OSD_ModalPopupRelease(void);//modal popup is backup bg UI scrn rect,then restore back
void OSD_Attach_Popcb(void);
void OSD_Detach_Popcb(void);

#endif

