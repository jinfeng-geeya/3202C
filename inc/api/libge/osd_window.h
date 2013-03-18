/*-----------------------------------------------------------------------------
*	Copyright (C) 2010 ALI Corp. All rights reserved.
*	GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef _OSD_WINDOW_H
#define _OSD_WINDOW_H_

#include <basic_types.h>
#include <api/libc/string.h>
#include "osd_lib.h"
#include "osd_primitive.h"

#ifndef GELIB_WNDRECT_CNT
#define GELIB_WNDRECT_CNT 64
#endif

#define gelib_objwnd_valid(pobj) ((((pobj)->pRoot == NULL)&&((pobj)->bType == OT_CONTAINER)) ? 1:0)

extern UINT32 g_wnd_monitors;
#define gelib_get_wndmonitor() ((g_wnd_monitors>0)?1:0)

extern UINT32 g_wndrects_idx;
#define gelib_wndrects_valid() ((g_wndrects_idx<GELIB_WNDRECT_CNT)?1:0)
#define gelib_wndrects_full() ((g_wndrects_cnt==GELIB_WNDRECT_CNT)?1:0)
#define gelib_wndrects_showon() ((g_wndrects_cnt>0)?1:0)
#define gelib_wndrects_showoff() ((g_wndrects_cnt==0)?1:0)

typedef struct _OBJECT_HEAD     GUI_OBJWND,*PGUI_OBJWND;

typedef struct gui_wnd
{
	UINT8 layer_idx;
	UINT8 rgn_idx;
}GUI_WINDOW,*PGUI_WINDOW;

UINT32 osdwnd_showwnd(PGUI_WINDOW pwnd,UINT32 param);
UINT32 osdwnd_hidewnd(PGUI_WINDOW pwnd,UINT32 param);
UINT32 osdwnd_get_wndshow();
UINT32 gelib_enable_wnd_monitors();
UINT32 gelib_disable_wnd_monitors(UINT32 onoff);
#endif

