/*-----------------------------------------------------------------------------
*
*	Copyright (C) 2009 ALI Corp. All rights reserved.
*
*	File: ge_primitive_init.c
*
*	Content: 
*		GUI primitive draw function & GE operation functions init
*	History: 
*		2009/03/04 by Shine Zhou
*		New build for M3202C new GE drivers directly draw GUI objects.
*		New GE Drv using cmdlst method to draw primitive.
*-----------------------------------------------------------------------------*/
#include <sys_config.h>
#include <basic_types.h>
#include <hld/osd/osddrv.h>
#include <api/libge/osd_lib.h>
#include <api/libc/string.h>
#include <api/libchar/lib_char.h>
#include <api/libge/osd_primitive.h>
#include <api/libge/ge_primitive_init.h>
#include <api/libge/gecmd_primitive.h>
#include "osd_lib_internal.h"

#define GELIB_DEBUG_STRS	256

UINT32	gelib_state;
UINT32	ge_version;
UINT32	gui_design_dimension;
char gelib_dbstrs[GELIB_DEBUG_STRS];
/*##########################################################################*/
/*GUI related primitive draw function APIs.*/
GUI_GETDSTRGN	gelib_getdstrgn;/*Should not get rgn handle if no initilized*/

GUI_DRAWFONT	gelib_drawfont;
GUI_RENDERBMP	gelib_renderbmp;
GUI_DRAWBMP	gelib_drawbmp;
GUI_FILLROUNDRECT	gelib_fillroundrect;
GUI_FILLRECT	gelib_fillrect;
GUI_LINETO	gelib_lineto;
GUI_DRAWLINE	gelib_drawline;

GUI_COLORKEY_RGN	gelib_colorkey_rgn;
GUI_BLEND_RGN	gelib_blend_rgn;
GUI_FLUSH_RGN	gelib_flush_rgn;
GUI_RETRIEVESCRN	gelib_retrievescrn;
GUI_UPDATE2SCRN	gelib_update2scrn;

GUI_GETPALLETTE	gelib_get_pallette;
GUI_SETPALLETTE	gelib_set_pallette;
GUI_MODIFYPALLETTE	gelib_modify_pallette;
GUI_CLEARCLIP	gelib_clearclip;
GUI_SETCLIP	gelib_setclip;

GUI_GET_RGNPOS	gelib_get_rgnpos;
GUI_SET_RGNPOS	gelib_set_rgnpos;
GUI_RGN_SHOWONFF	gelib_rgn_showonoff;
GUI_CREATERGN	gelib_create_rgn;
GUI_DELETERGN	gelib_delete_rgn;
GUI_SCALE	gelib_scale;
GUI_SETGALPHA gelib_setgalpha;
/******************************************************************************************
*Call GE_PRIMITIVE function pointers attach init operations
******************************************************************************************/
RET_CODE gedraw_primitive_attach(void)
{
	gelib_getdstrgn = gedraw_getdstrgn;

	gelib_drawline = gedraw_drawline;
	gelib_lineto = gedraw_lineto;
	gelib_fillrect = gedraw_fillrect;
	gelib_fillroundrect = gedraw_fillroundrect;
	gelib_drawbmp = gedraw_drawbmp;
	gelib_renderbmp = gedraw_renderbmp;
	gelib_drawfont = gedraw_drawfont;
	
	gelib_retrievescrn = gedraw_retrievescrn;
	gelib_flush_rgn = gedraw_flush_rgn;
	gelib_colorkey_rgn = gedraw_colorkey_rgn;
	gelib_blend_rgn = gedraw_blend_rgn;
	gelib_update2scrn = gedraw_update2scrn;
	
	gelib_setclip = gedraw_setclip;
	gelib_clearclip = gedraw_clearclip;
	gelib_modify_pallette = gedraw_modifypallette;
	gelib_set_pallette = gedraw_setpallette;
	gelib_get_pallette = gedraw_getpallette;
	
	gelib_get_rgnpos = gedraw_getrgnpos;
	gelib_set_rgnpos = gedraw_setrgnpos;
	gelib_rgn_showonoff = gedraw_rgn_showonoff;
	gelib_create_rgn = gedraw_creatergn;
	gelib_delete_rgn = gedraw_deletergn;
	gelib_scale = gedraw_scale;
	gelib_setgalpha = gedraw_setgalpha;

	return RET_SUCCESS;
}/*Old GE_DRV APIs,so called "gedraw"*/

RET_CODE gecmd_primitive_attach(void)
{
	gelib_getdstrgn = gecmd_getdstrgn;

	gelib_drawline = gecmd_drawline;
	gelib_lineto = gecmd_lineto;
	gelib_fillrect = gecmd_fillrect;
	gelib_fillroundrect = gecmd_fillroundrect;
	gelib_drawbmp = gecmd_drawbmp;
	gelib_renderbmp = gecmd_renderbmp;
	gelib_drawfont = gecmd_drawfont;
	
	gelib_retrievescrn = gecmd_retrievescrn;
	gelib_flush_rgn = gecmd_flush_rgn;
	gelib_colorkey_rgn = gecmd_colorkey_rgn;
	gelib_blend_rgn = gecmd_blend_rgn;
	gelib_update2scrn = gecmd_update2scrn;
	
	gelib_setclip = gecmd_setclip;
	gelib_clearclip = gecmd_clearclip;
	gelib_modify_pallette = gecmd_modifypallette;
	gelib_set_pallette = gecmd_setpallette;
	gelib_get_pallette = gecmd_getpallette;
	
	gelib_get_rgnpos = gecmd_getrgnpos;
	gelib_set_rgnpos = gecmd_setrgnpos;
	gelib_rgn_showonoff = gecmd_rgn_showonoff;
	gelib_create_rgn = gecmd_creatergn;
	gelib_delete_rgn = gecmd_deletergn;
	gelib_scale = gecmd_scale;
	gelib_setgalpha = gecmd_setgalpha;

	return RET_SUCCESS;
}/*New GE_DRV APIs,so called 'gecmd'*/

/*##########################################################################*/
//TODO:
/*##########################################################################*/

RET_CODE gelib_init(GUI_DEV gedev,PGUI_INIT_PARAM pinit)
{
	gui_dev = gedev;
	ge_version = pinit->ge_version;
	gelib_state = GELIB_INITLIZING;
	gui_design_dimension= pinit->gui_lines;
	
	if(ge_version < GE_MODULE_NEW)
	{/*OLD GE_DRV*/
		gedraw_primitive_attach();
		gui_printf("OLD_GE attach,gedraw_XX");
	}
	else
	{/*NEW GE_DRV*/
		gecmd_init((UINT32)gedev);
		gecmd_primitive_attach();
		gui_printf("NEW_GE attach,gecmd_XX");
	}
}

UINT32 gelib_getstates()
{
	return gelib_state;
}

