/*-----------------------------------------------------------------------------
*	Copyright (C) 2010 ALI Corp. All rights reserved.
*	GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef _GE_PRIMITIVE_INIT_H_
#define _GE_PRIMITIVE_INIT_H_

#include"ge_draw_primitive.h"

typedef enum gelib_states
{
	GELIB_WILD = 0,
	GELIB_INITLIZING,
	GELIB_READY,
	GELIB_RUNNING,
	GELIB_WAITING,
}GELIB_STATES;

typedef enum ge_versions
{
	GE_MODULE_M3329E = 0x0,
	GE_MODULE_M3202 = 0x01,
	GE_MODULE_M3602 = 0xf0,

	GE_MODULE_NEW=0xff,
	
	GE_MODULE_M3329D=0x100,
	GE_MODULE_M3602F=0x1f0,
	GE_MODULE_M3603=0x1f1,
	GE_MODULE_M3606=0x1f2,
	GE_MODULE_M3701C=0x1f3,
	GE_MODULE_MAX = 0xffffffff,
}GE_VERSION_CONST;

typedef enum gui_tvsys_resolution
{
	GUI_NTSC_LINES = 480,//480*720 
	GUI_PAL_LINES = 576,//576*720,4:3size
	GUI_720P_LINES = 720,//720*1280
	GUI_1080i_LINES = 1080,//1080*1920,16:9size
}GUI_DIMENSION;

typedef struct gui_init_param
{
	GE_VERSION_CONST ge_version;
	GUI_DIMENSION	gui_lines;
	UINT32 ini_rsvd;
}GUI_INIT_PARAM,*PGUI_INIT_PARAM;

UINT32 gelib_getstates();
RET_CODE gelib_init(GUI_DEV gedev,PGUI_INIT_PARAM pinit);

#endif

