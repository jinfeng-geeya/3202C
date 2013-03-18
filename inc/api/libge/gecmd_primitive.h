/*-----------------------------------------------------------------------------
*	Copyright (C) 2010 ALI Corp. All rights reserved.
*	GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef _GECMD_PRIMITIVE_H_
#define _GECMD_PRIMITIVE_H_

#include <hld/ge/ge.h>
#include "osd_primitive.h"

PGUI_REGION gecmd_getdstrgn(UINT8 layer_type,UINT8 rgn_idx);
RET_CODE gecmd_init(UINT32 param);
RET_CODE gecmd_release(void);
UINT32 gecmd_tvsys_scale(UINT32 layer,GUI_TVSYS tvsys);
UINT8 gecmd_popup_release(void);
UINT8 gecmd_popup_init(UINT32 param);

RET_CODE gecmd_drawpixel(PGUI_REGION prgn,PGUI_POINT pPos,UINT32 param);
RET_CODE gecmd_drawline(PGUI_REGION prgn,PGUI_POINT pStart,PGUI_POINT pEnd,UINT32 param);
RET_CODE gecmd_lineto(PGUI_REGION prgn,PGUI_POINT pStart,PGUI_POINT pEnd,UINT32 param);
RET_CODE gecmd_fillrect(PGUI_REGION prgn,PGUI_RECT pRect,UINT32 param);
RET_CODE gecmd_fillroundrect(PGUI_REGION prgn,PGUI_RECT pFrm,PGUI_RECT pCorner,UINT32 param);
RET_CODE gecmd_drawbmp(PGUI_REGION prgn,PGUI_RECT pFrm,UINT32 param);
RET_CODE gecmd_renderbmp(PGUI_REGION prgn,PGUI_RECT pfrm,UINT32 param);
RET_CODE gecmd_drawfont(PGUI_REGION prgn,PGUI_RECT pFrm,UINT32 param);

RET_CODE gecmd_update2scrn(PGUI_VSCR pvscr,PGUI_RECT prect_w);
RET_CODE gecmd_retrievescrn(PGUI_VSCR pvscr,PGUI_RECT prect_r);
RET_CODE gecmd_flush_rgn(PGUI_REGION pdst,PGUI_REGION psrc,PGUI_RECT pdst_rct,PGUI_RECT psrc_rct,UINT32 param);
RET_CODE gecmd_colorkey_rgn(PGUI_REGION pdst,PGUI_REGION prgn1,PGUI_REGION prgn2,PGUI_RECT prct1,PGUI_RECT prct2,UINT32 param);
RET_CODE gecmd_blend_rgn(PGUI_REGION pdst,PGUI_REGION prgn1,PGUI_REGION prgn2,PGUI_RECT prct1,PGUI_RECT prct2,UINT32 param);

RET_CODE gecmd_setclip(PGUI_REGION prgn,PGUI_RECT prect,UINT32 param);
RET_CODE gecmd_clearclip(PGUI_REGION prgn);
RET_CODE gecmd_setrgnpos(PGUI_REGION prgn,PGUI_RECT prect);
RET_CODE gecmd_getrgnpos(PGUI_REGION prgn,PGUI_RECT prect);
RET_CODE gecmd_modifypallette(PGUI_REGION prgn,UINT8 idx,UINT8 *pallette);
RET_CODE gecmd_setpallette(PGUI_REGION prgn,UINT8 *pallette);
RET_CODE gecmd_getpallette(PGUI_REGION prgn,UINT8 *pallette);
RET_CODE gecmd_scale(PGUI_REGION prgn,UINT32 cmd,UINT32 param);
RET_CODE gecmd_setgalpha(UINT32 layer,UINT32 gtrans);

RET_CODE gecmd_rgn_showonoff(PGUI_REGION prgn,UINT8 on_off);
RET_CODE gecmd_deletergn(PGUI_REGION prgn);
RET_CODE gecmd_creatergn(UINT32 param);
#endif

