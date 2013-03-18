/*====================================================================
 *
 *  Copyright (C) 2010 ALi.  All Rights Reserved.
 *
 *  File:   gelib_effects.h
 *
 *  Contents:    Provide GElib animation task management functions.
 *  History:      Build for new GElib by Shine Zhou@2010/03/10
 *********************************************************************/
#ifndef _GELIB_EFFECTS_H_
#define _GELIB_EFFECTS_H_

UINT32 gelib_win_slideshow(PGUI_RECT rctslide,UINT32 param);
UINT32 gelib_win_fadeout(UINT32 galpha,UINT32 param);

UINT32 gelib_item_focuslide(POBJECT_HEAD psobj,POBJECT_HEAD ptobj,UINT32 param);
UINT32 gelib_olst_focuslide(POBJECT_HEAD psobj,POBJECT_HEAD ptobj,UINT32 param);

typedef struct gelib_effect_cfg
{
	UINT32 type:8;
	UINT32 delay:8;
	UINT32 param1:8;
	UINT32 param2:8;
}GUI_EFFCT_CFG,*PGUI_EFFCT_CFG;

#endif//_GELIB_EFFECTS_H_

