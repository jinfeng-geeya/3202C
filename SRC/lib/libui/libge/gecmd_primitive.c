/*-----------------------------------------------------------------------------
*
*	Copyright (C) 2009 ALI Corp. All rights reserved.
*
*	File: gecmd_primitive.c
*
*	Content: 
*		GUI primitive draw function & GE operation functions
*	History: 
*		2009/03/12 by Shine Zhou
*		New build for M3329D & M3202C chipsets new GE drivers directly draw GUI objects.
*		New GE Drv using cmdlst method to draw primitive.
*-----------------------------------------------------------------------------*/
#include <sys_config.h>
#include <basic_types.h>
#include <hld/ge/ge.h>
#include <api/libge/osd_lib.h>
#include <api/libc/string.h>
#include <api/libchar/lib_char.h>
#include <api/libge/gecmd_primitive.h>
#include <api/libge/ge_primitive_init.h>
#include "osd_lib_internal.h"

/*0xff as the invalid format,clut8 as default*/
const UINT8 colorcvt_tbl[GUI_COLORMAX+1] = { \
                            (UINT8)(~0),GE_PF_CLUT1,GE_PF_CLUT8, \
                            GE_PF_ARGB1555,GE_PF_ARGB4444,GE_PF_AYCBCR8888, \
                            GE_PF_ARGB8888,GE_PF_CLUT8};
static UINT32 gui_pallette[CLUT_COLOR_N];

extern GUI_TVSYS	g_gui_tvsys;
extern GUI_SCALE_FACTOR gui_scale_coeffs[];

#define GUICOLOR_2_GECOLOR(color_mode) ((color_mode >= GUI_COLORMAX)?(UINT8)(~0):colorcvt_tbl[color_mode])
/*#########################################################################################################*/
PGUI_REGION gecmd_getdstrgn(UINT8 layer_type,UINT8 rgn_idx)
{
	UINT8 idx;

	if(layer_type < GUI_MEMVSCR_SURF)
	{
		idx = layer_type * GUI_MAX_REGION_ONLAYER + rgn_idx;
	}
	else
	{
		idx = GUI_MEMVSCR_SURF * GUI_MAX_REGION_ONLAYER +(layer_type - GUI_MEMVSCR_SURF);
	}

	return (idx >= GUI_MAX_REGION_CNT) ? NULL : &g_rgn_tbl[idx];
}

static RET_CODE gecmd_rgninit(UINT32 lstcnt,UINT32 param)
{
	UINT32 idx;
	g_memrgn_tbl = (g_rgn_tbl + GUI_GMA_REGION_OFFSET);

	for(idx = 0;idx<lstcnt;idx++)
	{
		MEMSET(&g_rgn_tbl[idx],0x0,sizeof(g_rgn_tbl[0]));
	}

    	MEMSET(&g_vscr,0x0,sizeof(g_vscr[0])*GUI_MULTITASK_CNT);
	for(idx = 0;idx < GUI_MULTITASK_CNT;idx++)
	{
		g_vscr[idx].lpbuff = NULL;
		g_vscr[idx].root_layer = GUI_MEMVSCR_SURF;
		g_vscr[idx].root_rgn = 0;
		g_vscr[idx].dst_layer = GUI_GMA1_SURF;
		g_vscr[idx].dst_rgn = 0;        
	}
}
/****************************************************************************************************************************************************
*New GE global variance and APIs encapsules etc.
****************************************************************************************************************************************************/
static ge_cmd_list_hdl gelib_cmdlst = 0;/*cmd list only used in the gecmd_primitive.c files*/
RET_CODE gecmd_init(UINT32 param)
{
	UINT32 cmd_hdl;
	struct ge_device * gelib_dev;

	cmd_hdl = 0;
	gelib_state = GELIB_INITLIZING;
	gecmd_rgninit(GUI_MAX_REGION_CNT,0);

	gelib_dev = (struct ge_device*)param;
	if(NULL == gelib_dev)
		gelib_dev = (struct ge_device*)dev_get_by_id(HLD_DEV_TYPE_GE, 0);

	gelib_cmdlst  = ge_cmd_list_create(gelib_dev, 1);
	gelib_state = GELIB_READY;
}

RET_CODE gecmd_release(void)
{
	struct ge_device * gelib_dev;

	gelib_dev = (struct ge_device *)gui_dev;
	ge_cmd_list_destroy(gelib_dev,gelib_cmdlst);
	ge_close(gelib_dev);
	gui_dev = (GUI_DEV)NULL;
	gelib_cmdlst = 0;
}

/*#########################################################################################################*/
UINT8 gecmd_popup_init(UINT32 param)
{
	PGUI_LAYER_CFG pcfg;
	PGUI_REGION prgn;

	pcfg = (PGUI_LAYER_CFG)param;
	prgn = gelib_getdstrgn(GUI_MEMPOP_SURF,0);

	prgn->rgn_id = 0;
	prgn->layer_type = GUI_MEMPOP_SURF;
	prgn->color_mode = pcfg->color_mode;
	prgn->rect.uStartX = pcfg->rect.uStartX;
	prgn->rect.uStartY = pcfg->rect.uStartY;
	prgn->rect.uWidth = pcfg->rect.uWidth;
	prgn->rect.uHeight = pcfg->rect.uHeight;
	gelib_create_rgn((UINT32)prgn);
	
	pop_buff = (UINT8 *)(prgn->hsurf);
	return 0;
}

UINT8 gecmd_popup_release(void)
{
	PGUI_REGION ppop;

	ppop= gelib_getdstrgn(GUI_MEMPOP_SURF,0);
	gelib_delete_rgn(ppop);
	pop_buff = NULL;
	return 0;
}
/*#########################################################################################################*/
UINT32 gecmd_tvsys_scale(UINT32 layer,GUI_TVSYS tvsys)
{
	UINT8 idx;
	UINT32 err_code;
	GUI_TVSYS	cur_tvsys;
	ge_scale_param_t mgma_param;

	if(layer > GUI_GMA2_SURF)
		return (UINT32)(-3);
	
	switch(tvsys)
	{
		case PAL:
		case PAL_M:
		case PAL_N:
		case PAL_60:
			idx = 1;
			cur_tvsys = PAL;
			break;
		case NTSC:
		case NTSC_443:
		case SECAM:
		case MAC:
			idx = 0;
			//cur_tvsys = PAL;
			cur_tvsys = NTSC;//20100920
			break;
		case LINE_720_25:
		case LINE_720_30:
			idx = 2;
			cur_tvsys = LINE_720_30;
			break;
			
		case LINE_1080_24: //gavin
		case LINE_1080_25:
		case LINE_1080_30:
		case LINE_1080_50:
		case LINE_1080_60:
			idx = 3;
			cur_tvsys = LINE_1080_30;
			break;
		default:
			idx = 1;
			cur_tvsys = g_gui_tvsys;//unkown tv-out
	}

	if(cur_tvsys == g_gui_tvsys)
		return (UINT32)(-4);//need not scale region and layers
	else
		g_gui_tvsys = cur_tvsys;

	if(gui_design_dimension == GUI_720P_LINES)
		idx +=4;
	else if(gui_design_dimension == GUI_1080i_LINES)
		idx +=8;//reserved currently
	else
		idx = idx;

	idx = (idx>=12) ? 0:idx;
	MEMSET(&mgma_param,0x0,sizeof(mgma_param));
	mgma_param.tv_sys = g_gui_tvsys;
	mgma_param.h_div = gui_scale_coeffs[idx].hdiv;
	mgma_param.h_mul = gui_scale_coeffs[idx].hmul;
	mgma_param.v_div = gui_scale_coeffs[idx].vdiv;
	mgma_param.v_mul = gui_scale_coeffs[idx].vmul;
	err_code = ge_gma_scale((struct ge_device *)gui_dev, layer, GE_SET_SCALE_PARAM,(UINT32)&mgma_param);

	return err_code;
}
/****************************************************************************************************************************************************
*GUI primitive draw functions
****************************************************************************************************************************************************/
RET_CODE gecmd_drawpixel(PGUI_REGION prgn,PGUI_POINT ppos,UINT32 param)
{
	UINT8 *pbuff;
	UINT16 offsetx,offsety;
	UINT32 cmd_hdl,color;
	RET_CODE cmd_ret;
	PGUI_PEN pen;
	ge_rect_t dst_rect;
	struct ge_device * gelib_dev;
	
	pen = (PGUI_PEN)param;
	if(pen->thick >= 2)
	{
		GUI_RECT dot;

		dot.uStartX = ppos->uX;
		dot.uStartY = ppos->uY;
		dot.uWidth = pen->thick;
		dot.uHeight = pen->thick;
		gecmd_fillrect(prgn,&dot,param);/*In fact it draws a small square*/
	}
	
	pbuff = (UINT8 *)prgn->hsurf;
	gelib_dev = (struct ge_device *)gui_dev;
	ge_cmd_list_new(gelib_dev, gelib_cmdlst, GE_COMPILE_AND_EXECUTE);

	if(prgn->layer_type < GUI_MEMVSCR_SURF)
	{
		offsetx = offsety = 0;
		ge_gma_set_region_to_cmd_list(gelib_dev,(UINT32)prgn->layer_type,(UINT32)prgn->rgn_id,gelib_cmdlst);
		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst, GE_DIRECT_DRAW_POINT);
	}
	else
	{/*For mem virtual screen operations*/
		ge_operating_entity entity;
		
		MEMSET(&entity,0x0,sizeof(entity));
		entity.color_format = GUICOLOR_2_GECOLOR(prgn->color_mode);
		entity.base_address = (UINT32)pbuff;
		entity.data_decoder = GE_DECODER_DISABLE;
		entity.pixel_pitch = gelib_get_rgnpitch(prgn);
		entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
		
		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst, GE_DRAW_POINT);
		ge_set_operating_entity(gelib_dev,cmd_hdl,GE_DST,&entity);
		ge_set_color_format(gelib_dev,cmd_hdl,entity.color_format);

		offsetx = prgn->rect.uStartX;
		offsety = prgn->rect.uStartY;
	}

	color = gui_colormask(prgn,(pen->color));
	ge_set_draw_color(gelib_dev, cmd_hdl, color);
	ge_set_xy(gelib_dev,cmd_hdl,GE_DST,(INT32)(ppos->uX - offsetx),(INT32)(ppos->uX - offsety));
	ge_cmd_end(gelib_dev, cmd_hdl);

	cmd_ret = ge_cmd_list_end(gelib_dev,gelib_cmdlst);
	gui_debug_show();
	return cmd_ret;
}

RET_CODE gecmd_drawline(PGUI_REGION prgn,PGUI_POINT pstart,PGUI_POINT pend,UINT32 param)
{
/*/////////////////////////////////////////////////////////////////////////////////////
	1. vertical line
	2. horizontal line
	3. diagonal line
	param is the linethick,here it is pixel width,in this function thick will be set as default
*////////////////////////////////////////////////////////////////////////////////////////////////
	UINT8 *pbuff;
	UINT16 offsetx,offsety;
	UINT32 cmd_hdl,color;
	RET_CODE cmd_ret;
	PGUI_PEN pen;
	ge_rect_t dst_rect;
	struct ge_device * gelib_dev;
	
	pen = (PGUI_PEN)param;
	if((pstart->uX != pend->uX)&&(pstart->uY != pend->uY))
	{
		return gelib_lineto(prgn,pstart,pend,param);
	}
	else if(pen->thick>=2)
	{
		GUI_RECT frm;

		frm.uStartX = pstart->uX;
		frm.uStartY = pstart->uY;
		frm.uWidth = (pstart->uX == pend->uX)? (pen->thick): gelib_get_posdiff(pend->uX,pstart->uX);
		frm.uHeight = (pstart->uY == pend->uY)? (pen->thick): gelib_get_posdiff(pend->uY,pstart->uY);
		return gecmd_fillrect(prgn,&frm,pen->color);
	}
	
	pbuff = (UINT8 *)prgn->hsurf;
	gelib_dev = (struct ge_device *)gui_dev;
	ge_cmd_list_new(gelib_dev, gelib_cmdlst, GE_COMPILE_AND_EXECUTE);

	if(prgn->layer_type < GUI_MEMVSCR_SURF)
	{
		offsetx = offsety = 0;
		ge_gma_set_region_to_cmd_list(gelib_dev,(UINT32)prgn->layer_type,(UINT32)prgn->rgn_id,gelib_cmdlst);
		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst, GE_DIRECT_DRAW_LINE);
	}
	else
	{/*For mem virtual screen operations*/
		ge_operating_entity entity;
		
		MEMSET(&entity,0x0,sizeof(entity));
		entity.color_format = GUICOLOR_2_GECOLOR(prgn->color_mode);
		entity.base_address = (UINT32)pbuff;
		entity.data_decoder = GE_DECODER_DISABLE;
		entity.pixel_pitch = gelib_get_rgnpitch(prgn);
		entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
		
		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst, GE_DRAW_LINE);
		ge_set_operating_entity(gelib_dev,cmd_hdl,GE_DST,&entity);
		ge_set_color_format(gelib_dev,cmd_hdl,entity.color_format);

		offsetx = 0;//prgn->rect.uStartX;
		offsety = 0;//prgn->rect.uStartY;
	}

	color = gui_colormask(prgn,(pen->color));
	ge_set_draw_color(gelib_dev, cmd_hdl, color);
	ge_set_xy2(gelib_dev, cmd_hdl, GE_DST,(INT32)(pstart->uX - offsetx),(INT32)(pstart->uY - offsety), \
		(INT32)(pend->uX - offsetx),(INT32)(pend->uY - offsety));
	ge_cmd_end(gelib_dev, cmd_hdl);
	
	cmd_ret = ge_cmd_list_end(gelib_dev,gelib_cmdlst);
	gui_debug_show();
	return cmd_ret;
};

RET_CODE gecmd_lineto(PGUI_REGION prgn,PGUI_POINT pstart,PGUI_POINT pend,UINT32 param)
{
	UINT8 *pbuff;
	UINT16 offsetx,offsety;
	UINT32 cmd_hdl,color;
	RET_CODE cmd_ret;
	PGUI_PEN pen;
	ge_rect_t dst_rect;
	struct ge_device * gelib_dev;
	
	pen = (PGUI_PEN)param;
	pbuff = (UINT8 *)prgn->hsurf;
	gelib_dev = (struct ge_device *)gui_dev;
	ge_cmd_list_new(gelib_dev, gelib_cmdlst, GE_COMPILE_AND_EXECUTE);

	if(prgn->layer_type < GUI_MEMVSCR_SURF)
	{
		offsetx = offsety = 0;
		ge_gma_set_region_to_cmd_list(gelib_dev,(UINT32)prgn->layer_type,(UINT32)prgn->rgn_id,gelib_cmdlst);
		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst, GE_DIRECT_DRAW_LINE);
	}
	else
	{/*For mem virtual screen operations*/
		ge_operating_entity entity;
		
		MEMSET(&entity,0x0,sizeof(entity));
		entity.color_format = GUICOLOR_2_GECOLOR(prgn->color_mode);
		entity.base_address = (UINT32)pbuff;
		entity.data_decoder = GE_DECODER_DISABLE;
		entity.pixel_pitch = gelib_get_rgnpitch(prgn);
		entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
		
		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst, GE_DRAW_LINE);
		ge_set_operating_entity(gelib_dev,cmd_hdl,GE_DST,&entity);
        		ge_set_color_format(gelib_dev,cmd_hdl,entity.color_format);

		offsetx = 0;//prgn->rect.uStartX;
		offsety = 0;//prgn->rect.uStartY;
	}


	color = gui_colormask(prgn,(pen->color));
	ge_set_draw_color(gelib_dev, cmd_hdl, color);
	ge_set_xy2(gelib_dev, cmd_hdl, GE_DST,(INT32)(pstart->uX - offsetx),(INT32)(pstart->uY - offsety), \
		(INT32)(pend->uX - offsetx),(INT32)(pend->uY - offsety));
	
	ge_cmd_end(gelib_dev, cmd_hdl);
	
	cmd_ret = ge_cmd_list_end(gelib_dev,gelib_cmdlst);
	gui_debug_show();
	return cmd_ret;
}

RET_CODE gecmd_fillroundrect(PGUI_REGION prgn,PGUI_RECT prect,PGUI_RECT pcorner,UINT32 param)
{
	//reserved for future use
	return RET_SUCCESS;
	
	UINT8 *pbuff;
	UINT16 offsetx,offsety;
	UINT32 cmd_hdl;
	RET_CODE cmd_ret;
	ge_rect_t dst_rect;
	struct ge_device * gelib_dev;

	pbuff = (UINT8 *)prgn->hsurf;
	gelib_dev = (struct ge_device *)gui_dev;
	ge_cmd_list_new(gelib_dev, gelib_cmdlst, GE_COMPILE_AND_EXECUTE);

	if(prgn->layer_type < GUI_MEMVSCR_SURF)
	{
		offsetx = offsety = 0;
		ge_gma_set_region_to_cmd_list(gelib_dev,(UINT32)prgn->layer_type,(UINT32)prgn->rgn_id,gelib_cmdlst);
		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst, GE_FILL_RECT_BACK_COLOR);
	}
	else
	{/*For mem virtual screen operations*/
		ge_operating_entity entity;
		MEMSET(&entity,0x0,sizeof(entity));

		entity.color_format = GUICOLOR_2_GECOLOR(prgn->color_mode);
		entity.base_address = (UINT32)pbuff;
		entity.data_decoder = GE_DECODER_DISABLE;
		entity.pixel_pitch = gelib_get_rgnpitch(prgn);
		entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst, GE_FILL_RECT_BACK_COLOR);
		ge_set_operating_entity(gelib_dev,cmd_hdl,GE_DST,&entity);
        		ge_set_color_format(gelib_dev,cmd_hdl,entity.color_format);

		offsetx = 0;//prgn->rect.uStartX;
		offsety = 0;//prgn->rect.uStartY;
	}

	dst_rect.left = (INT32)(prect->uStartX - offsetx);
	dst_rect.top = (INT32)(prect->uStartY - offsety);
	dst_rect.height = (INT32)(prect->uHeight);
	dst_rect.width = (INT32)(prect->uWidth);
    
	ge_set_back_color(gelib_dev, cmd_hdl, param);
	ge_set_xy(gelib_dev, cmd_hdl, GE_DST, dst_rect.left, dst_rect.top);
	ge_set_wh(gelib_dev, cmd_hdl, GE_DST, dst_rect.width, dst_rect.height);
	ge_cmd_end(gelib_dev, cmd_hdl);
	
	cmd_ret = ge_cmd_list_end(gelib_dev,gelib_cmdlst);
	gui_debug_show();
	return cmd_ret;
}

RET_CODE gecmd_fillrect(PGUI_REGION prgn,PGUI_RECT prect,UINT32 param)
{
	UINT8 *pbuff;
	UINT16 offsetx,offsety;
	UINT32 cmd_hdl;
	RET_CODE cmd_ret,colour;
	ge_rect_t dst_rect;
    	ge_operating_entity entity;
	struct ge_device * gelib_dev;

	pbuff = (UINT8 *)prgn->hsurf;
	gelib_dev = (struct ge_device *)gui_dev;
	ge_cmd_list_new(gelib_dev, gelib_cmdlst, GE_COMPILE_AND_EXECUTE);

	if(prgn->layer_type < GUI_MEMVSCR_SURF)
	{
		offsetx = offsety = 0;
		ge_gma_set_region_to_cmd_list(gelib_dev,(UINT32)prgn->layer_type,(UINT32)prgn->rgn_id,gelib_cmdlst);
		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst,GE_FILL_RECT_BACK_COLOR);
	}
	else
	{/*For mem virtual screen operations*/
		MEMSET(&entity,0x0,sizeof(entity));
		entity.color_format = GUICOLOR_2_GECOLOR(prgn->color_mode);
		entity.base_address = (UINT32)pbuff;
		entity.data_decoder = GE_DECODER_DISABLE;
		entity.pixel_pitch = gelib_get_rgnpitch(prgn);
		entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
		
		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst,GE_FILL_RECT_BACK_COLOR);
		ge_set_operating_entity(gelib_dev,cmd_hdl,GE_DST,&entity);
        		ge_set_color_format(gelib_dev,cmd_hdl,entity.color_format);

		offsetx = 0;//prgn->rect.uStartX;
		offsety = 0;//prgn->rect.uStartY;
	}

	if(NULL != prect)
	{
		dst_rect.left = (INT32)(prect->uStartX - offsetx);
		dst_rect.top = (INT32)(prect->uStartY - offsety);
		dst_rect.height = (INT32)(prect->uHeight);
		dst_rect.width = (INT32)(prect->uWidth);
	}
	else
	{
		dst_rect.left = 0;
		dst_rect.top = 0;
		dst_rect.height = (INT32)(prgn->rect.uHeight);
		dst_rect.width = (INT32)(prgn->rect.uWidth);
	}
	colour = gui_colormask(prgn,param);
	ge_set_back_color(gelib_dev,cmd_hdl,colour);
	ge_set_xy(gelib_dev,cmd_hdl,GE_DST,dst_rect.left,dst_rect.top);
	ge_set_wh(gelib_dev,cmd_hdl,GE_DST,dst_rect.width,dst_rect.height);
    
	ge_cmd_end(gelib_dev,cmd_hdl);
	cmd_ret = ge_cmd_list_end(gelib_dev,gelib_cmdlst);
	
	gui_debug_show();
	return cmd_ret;
}

RET_CODE gecmd_drawbmp(PGUI_REGION prgn,PGUI_RECT pfrm,UINT32 param)
{
	UINT8 *pbuff;
	UINT16 offsetx,offsety;
	UINT32 cmd_hdl;
	RET_CODE cmd_ret;
	ge_rect_t dst_rect;
	PGUI_BMP    pbmp;
	ge_operating_entity entity;
	ge_gma_region_t gma_info;
	struct ge_device * gelib_dev;

	pbmp = (PGUI_BMP)param;	
	pbuff = (UINT8 *)prgn->hsurf;
	gelib_dev = (struct ge_device *)gui_dev;
	MEMSET(&entity,0x0,sizeof(entity));
    
	ge_cmd_list_new(gelib_dev, gelib_cmdlst, GE_COMPILE_AND_EXECUTE);
	if(prgn->layer_type < GUI_MEMVSCR_SURF)
	{
		offsetx = offsety = 0;
		ge_gma_set_region_to_cmd_list(gelib_dev,(UINT32)prgn->layer_type,(UINT32)prgn->rgn_id,gelib_cmdlst);
		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst,GE_DRAW_BITMAP);

		ge_gma_get_region_info(gelib_dev,(UINT32)prgn->layer_type,(UINT32)prgn->rgn_id,&gma_info);
		entity.color_format = gma_info.color_format;
		entity.base_address = (UINT32)gma_info.bitmap_addr;
		entity.data_decoder = GE_DECODER_DISABLE;
		entity.pixel_pitch = gma_info.pixel_pitch;
		entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
		ge_set_operating_entity(gelib_dev,cmd_hdl,GE_SRC,&entity);
	}
	else
	{/*For mem virtual screen operations*/
		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst,GE_DRAW_BITMAP);
        
		entity.color_format = GUICOLOR_2_GECOLOR(prgn->color_mode);
		entity.base_address = (UINT32)pbuff;
		entity.data_decoder = GE_DECODER_DISABLE;
		entity.pixel_pitch = gelib_get_rgnpitch(prgn);
		entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
		ge_set_operating_entity(gelib_dev,cmd_hdl,GE_SRC,&entity);
        
		entity.color_format = GUICOLOR_2_GECOLOR(prgn->color_mode);
		entity.base_address = (UINT32)pbuff;
		entity.data_decoder = GE_DECODER_DISABLE;
		entity.pixel_pitch = gelib_get_rgnpitch(prgn);
		entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
		ge_set_operating_entity(gelib_dev,cmd_hdl,GE_DST,&entity);
		ge_set_color_format(gelib_dev,cmd_hdl,entity.color_format);

		offsetx = 0;//prgn->rect.uStartX;
		offsety = 0;//prgn->rect.uStartY;
	}

	entity.color_format = (enum GE_PIXEL_FORMAT)GUICOLOR_2_GECOLOR(pbmp->color_mode);
	entity.base_address = (UINT32)pbmp->pdata;
	entity.data_decoder = (pbmp->rsc_rle>0)?1:0;
	entity.pixel_pitch = pbmp->stride;
	entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
	ge_set_operating_entity(gelib_dev,cmd_hdl,GE_PTN,&entity);			

	dst_rect.left = (INT32)(pfrm->uStartX - offsetx);
	dst_rect.top = (INT32)(pfrm->uStartY - offsety);
	dst_rect.height = (INT32)MIN(pfrm->uHeight,pbmp->bmp_size.uHeight);
	dst_rect.width = (INT32)MIN(pfrm->uWidth,pbmp->bmp_size.uWidth);
	osal_cache_flush((void *)pbmp->pdata,pbmp->stride*dst_rect.height*4);
    
	ge_set_src_mode(gelib_dev,cmd_hdl,GE_SRC_BITBLT);           
	ge_set_xy(gelib_dev,cmd_hdl,GE_PTN,0,0); 
	ge_set_xy(gelib_dev,cmd_hdl,GE_SRC,dst_rect.left,dst_rect.top); 
	ge_set_xy(gelib_dev, cmd_hdl, GE_DST, dst_rect.left, dst_rect.top);
	ge_set_wh(gelib_dev, cmd_hdl, GE_DST_PTN, dst_rect.width, dst_rect.height);
    
	ge_cmd_end(gelib_dev, cmd_hdl);
	
	cmd_ret = ge_cmd_list_end(gelib_dev,gelib_cmdlst);
	gui_debug_show();
	return cmd_ret;
}

RET_CODE gecmd_renderbmp(PGUI_REGION prgn,PGUI_RECT pfrm,UINT32 param)
{
	UINT8 *pbuff;
	UINT16 offsetx,offsety;
	UINT32 cmd_hdl,tcolour,lcolour;
	RET_CODE cmd_ret;
	PGUI_BMP    pbmp;
	ge_rect_t dst_rect;
	ge_operating_entity entity;
	ge_gma_region_t gma_info;
	struct ge_device * gelib_dev;

	pbmp = (PGUI_BMP)param;	
	pbuff = (UINT8 *)prgn->hsurf;
	gelib_dev = (struct ge_device *)gui_dev;
	MEMSET(&entity,0x0,sizeof(entity));
    
	ge_cmd_list_new(gelib_dev, gelib_cmdlst, GE_COMPILE_AND_EXECUTE);
	if(prgn->layer_type < GUI_MEMVSCR_SURF)
	{
		offsetx = offsety = 0;
		ge_gma_set_region_to_cmd_list(gelib_dev,(UINT32)prgn->layer_type,(UINT32)prgn->rgn_id,gelib_cmdlst);
		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst,GE_DRAW_BITMAP);

	        ge_gma_get_region_info(gelib_dev,(UINT32)prgn->layer_type,(UINT32)prgn->rgn_id,&gma_info);
		entity.color_format = gma_info.color_format;
		entity.base_address = (UINT32)gma_info.bitmap_addr;
		entity.data_decoder = GE_DECODER_DISABLE;
		entity.pixel_pitch = gma_info.pixel_pitch;
		entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
		ge_set_operating_entity(gelib_dev,cmd_hdl,GE_SRC,&entity);
	}
	else
	{/*For mem virtual screen operations*/
		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst,GE_DRAW_BITMAP);
        
		entity.color_format = GUICOLOR_2_GECOLOR(prgn->color_mode);
		entity.base_address = (UINT32)pbuff;
		entity.data_decoder = GE_DECODER_DISABLE;
		entity.pixel_pitch = gelib_get_rgnpitch(prgn);
		entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
		ge_set_operating_entity(gelib_dev,cmd_hdl,GE_SRC,&entity);
        
		entity.color_format = GUICOLOR_2_GECOLOR(prgn->color_mode);
		entity.base_address = (UINT32)pbuff;
		entity.data_decoder = GE_DECODER_DISABLE;
		entity.pixel_pitch = gelib_get_rgnpitch(prgn);
		entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
		ge_set_operating_entity(gelib_dev,cmd_hdl,GE_DST,&entity);
		ge_set_color_format(gelib_dev,cmd_hdl,entity.color_format);

		offsetx = 0;//prgn->rect.uStartX;
		offsety = 0;//prgn->rect.uStartY;
	}

	entity.color_format = (enum GE_PIXEL_FORMAT)GUICOLOR_2_GECOLOR(pbmp->color_mode);
	entity.base_address = (UINT32)pbmp->pdata;
	entity.data_decoder = (pbmp->rsc_rle>0)?1:0;
	entity.pixel_pitch = pbmp->stride;
	entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
	ge_set_operating_entity(gelib_dev,cmd_hdl,GE_PTN,&entity);			

	dst_rect.left = (INT32)(pfrm->uStartX - offsetx);
	dst_rect.top = (INT32)(pfrm->uStartY - offsety);
	dst_rect.height = (INT32)MIN(pfrm->uHeight,pbmp->bmp_size.uHeight);
	dst_rect.width = (INT32)MIN(pfrm->uWidth,pbmp->bmp_size.uWidth);
    
	ge_set_src_mode(gelib_dev,cmd_hdl,GE_SRC_BITBLT);           
	ge_set_xy(gelib_dev,cmd_hdl,GE_PTN,0,0); 
	ge_set_xy(gelib_dev,cmd_hdl,GE_SRC,dst_rect.left,dst_rect.top); 
	ge_set_xy(gelib_dev, cmd_hdl, GE_DST, dst_rect.left, dst_rect.top);
	ge_set_wh(gelib_dev, cmd_hdl, GE_DST_PTN, dst_rect.width, dst_rect.height);

	ge_set_colorkey_mode(gelib_dev,cmd_hdl,GE_CKEY_PTN_PRE_CLUT);
	if(prgn->color_mode == GUI_CLUT8)
	{
		tcolour = gui_colormask(prgn,pbmp->bg_color);
		lcolour = gui_colormask(prgn,pbmp->bg_color);
		ge_set_colorkey_match_mode(gelib_dev,cmd_hdl, GE_CKEY_CHANNEL_R,GE_CKEY_MATCH_ALWAYS);
		ge_set_colorkey_match_mode(gelib_dev,cmd_hdl, GE_CKEY_CHANNEL_G,GE_CKEY_MATCH_ALWAYS);
	}
	else
	{
		tcolour = gui_colormask(prgn,pbmp->bg_color);
		lcolour = gui_colormask(prgn,pbmp->bg_color);
		ge_set_colorkey_match_mode(gelib_dev,cmd_hdl, GE_CKEY_CHANNEL_R,GE_CKEY_MATCH_IN_RANGE);
		ge_set_colorkey_match_mode(gelib_dev,cmd_hdl, GE_CKEY_CHANNEL_G,GE_CKEY_MATCH_IN_RANGE);
	}
	ge_set_colorkey_match_mode(gelib_dev,cmd_hdl, GE_CKEY_CHANNEL_B,GE_CKEY_MATCH_IN_RANGE);
	ge_set_colorkey_range(gelib_dev,cmd_hdl,tcolour,lcolour);
	ge_cmd_end(gelib_dev, cmd_hdl);
	
	cmd_ret = ge_cmd_list_end(gelib_dev,gelib_cmdlst);
	
	gui_debug_show();
	return cmd_ret;
}

#define gelib_get_fontbytes(pfont) ((pfont->stride * pfont->fnt_size.uHeight)>>3)
RET_CODE gecmd_drawfont(PGUI_REGION prgn,PGUI_RECT pfrm,UINT32 param)
{
	UINT8 *pbuff;
	UINT16 offsetx,offsety;
	UINT32 cmd_hdl,bg_colour,fg_colour;
	RET_CODE cmd_ret;
	PGUI_FONT pfont;
	ge_rect_t dst_rect;
	ge_operating_entity entity;
	ge_gma_region_t gma_info;
	struct ge_device * gelib_dev;

	pfont = (PGUI_FONT)param;
	pbuff = (UINT8 *)prgn->hsurf;
	gelib_dev = (struct ge_device *)gui_dev;
	MEMSET(&entity,0x0,sizeof(entity));
    
	dst_rect.left = (INT32)pfrm->uStartX;
	dst_rect.top = (INT32)pfrm->uStartY;
	dst_rect.width = MIN(pfrm->uWidth,pfont->fnt_size.uWidth);
	dst_rect.height = MIN(pfrm->uHeight,pfont->fnt_size.uHeight);
	
	ge_cmd_list_new(gelib_dev, gelib_cmdlst, GE_COMPILE_AND_EXECUTE);
	if(prgn->layer_type < GUI_MEMVSCR_SURF)
	{//GMA surface
		offsetx = offsety = 0;
		ge_gma_set_region_to_cmd_list(gelib_dev,(UINT32)prgn->layer_type,(UINT32)prgn->rgn_id,gelib_cmdlst);
		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst, GE_DRAW_FONT);     
	}
	else
	{/*For mem virtual screen operations*/
		offsetx = 0;//prgn->rect.uStartX;
		offsety = 0;//prgn->rect.uStartY;

		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst, GE_DRAW_FONT);
		entity.color_format = GUICOLOR_2_GECOLOR(prgn->color_mode);
		entity.base_address = (UINT32)pbuff;
		entity.data_decoder = GE_DECODER_DISABLE;
		entity.pixel_pitch = gelib_get_rgnpitch(prgn);
		entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
		ge_set_operating_entity(gelib_dev,cmd_hdl,GE_DST,&entity);
		ge_set_operating_entity(gelib_dev,cmd_hdl,GE_SRC,&entity);
		ge_set_subbyte_endian(gelib_dev,cmd_hdl,GE_SRC,GE_SUBBYTE_RIGHT_PIXEL_LSB);
		ge_set_subbyte_endian(gelib_dev,cmd_hdl,GE_PTN,GE_SUBBYTE_RIGHT_PIXEL_LSB);   
	}
    
	osal_cache_flush((void *)pfont->pdata,gelib_get_fontbytes(pfont));
	entity.color_format = GE_PF_ARGB8888;
	entity.base_address = (UINT32)pfont->pdata;
	entity.data_decoder = (pfont->rsc_rle>0)?1:0;
	entity.pixel_pitch = pfont->stride;
	entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
	ge_set_operating_entity(gelib_dev,cmd_hdl,GE_PTN,&entity);

	/*Set Font attribute*/
	fg_colour = gui_colormask(prgn,pfont->fg_color);
	bg_colour = gui_colormask(prgn,pfont->bg_color);
	/*if(prgn->color_mode == GUI_CLUT8)
	{
		fg_colour = gui_pallette[fg_colour];
		bg_colour = gui_pallette[bg_colour];
	}*/
	ge_set_font_color(gelib_dev,cmd_hdl,fg_colour);
	ge_set_back_color(gelib_dev,cmd_hdl,bg_colour);
	ge_set_color_format(gelib_dev,cmd_hdl,GE_PF_ARGB8888);
	ge_set_font_data_format(gelib_dev,cmd_hdl,GE_FONT_DATA_BYTE);
	ge_set_byte_endian(gelib_dev,cmd_hdl,GE_PTN,GE_BYTE_ENDIAN_LITTLE);
	ge_set_xy(gelib_dev,cmd_hdl,GE_PTN,0,0);		
	ge_set_xy(gelib_dev,cmd_hdl,GE_DST_SRC,(dst_rect.left+offsetx),(dst_rect.top+offsety));
	ge_set_wh(gelib_dev,cmd_hdl,GE_DST_PTN,dst_rect.width,dst_rect.height);
	ge_set_wh(gelib_dev,cmd_hdl,GE_SRC,dst_rect.width,dst_rect.height);

	ge_cmd_end(gelib_dev, cmd_hdl);
	cmd_ret = ge_cmd_list_end(gelib_dev,gelib_cmdlst);
	
	gui_debug_show();
	return cmd_ret;
}

/*##########################################################################*/
RET_CODE gecmd_update2scrn(PGUI_VSCR pvscr,PGUI_RECT prect)
{
	UINT8 *pdbuff,*psbuff;
	UINT32 cmd_hdl;
	PGUI_REGION pdst,pvrgn;
	ge_operating_entity entity;
	ge_rect_t src_rect,dst_rect;
	struct ge_device * gelib_dev;

	if((pvscr->suspend > 0)||(pvscr->root_layer < GUI_MEMVSCR_SURF))
		return RET_SUCCESS;
	
	gelib_dev = (struct ge_device *)gui_dev;
	pvrgn = gelib_getdstrgn(pvscr->root_layer,pvscr->root_rgn);
	pdst = gelib_getdstrgn(pvscr->dst_layer,pvscr->dst_rgn);
	pdbuff = (UINT8 *)pdst->hsurf;
	psbuff = (UINT8 *)pvrgn->hsurf;

	if(NULL != prect)
	{
	    dst_rect.left = prect->uStartX;
	    dst_rect.top = prect->uStartY;
	    dst_rect.width = MIN(prect->uWidth,pvscr->frm.uWidth);
	    dst_rect.height = MIN(prect->uHeight,pvscr->frm.uHeight);
	    //dst_rect.width = MIN((MIN(prect->uWidth,pvscr->frm.uWidth)),(pdst->frm.uStartX+pdst->frm.uWidth-prect->uStartX));
	    //dst_rect.height = MIN((MIN(prect->uHeight,pvscr->frm.uHeight)),(pdst->frm.uStartY+pdst->frm.uHeight-prect->uStartY));
	}
	else
	{
		dst_rect.left = pvscr->frm.uStartX;
		dst_rect.top = pvscr->frm.uStartY;
		dst_rect.width = pvscr->frm.uWidth;
		dst_rect.height = pvscr->frm.uHeight;
	}
	MEMCPY(&src_rect,&dst_rect,sizeof(ge_rect_t));

	ge_cmd_list_new(gelib_dev, gelib_cmdlst, GE_COMPILE_AND_EXECUTE);
	if(pdst->layer_type < GUI_MEMVSCR_SURF)
	{
		ge_gma_set_region_to_cmd_list(gelib_dev,(UINT32)pdst->layer_type,(UINT32)pdst->rgn_id,gelib_cmdlst);
		cmd_hdl = ge_cmd_begin(gelib_dev,gelib_cmdlst,GE_PRIM_DISABLE);
	}
	else
	{
		entity.color_format = GUICOLOR_2_GECOLOR(pvscr->color_mode);
		entity.base_address = (UINT32)pdbuff;
		entity.data_decoder = GE_DECODER_DISABLE;
		entity.pixel_pitch = gelib_get_rgnpitch(pdst);
		entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

		cmd_hdl = ge_cmd_begin(gelib_dev,gelib_cmdlst,GE_PRIM_DISABLE);
		ge_set_operating_entity(gelib_dev,cmd_hdl,GE_DST,&entity);
		ge_set_color_format(gelib_dev,cmd_hdl,entity.color_format);	
	}

	entity.color_format = GUICOLOR_2_GECOLOR(pvscr->color_mode);
	entity.base_address = (UINT32)psbuff;
	entity.data_decoder = GE_DECODER_DISABLE;
	entity.pixel_pitch = gelib_get_rgnpitch(pvrgn);
	entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
	ge_set_operating_entity(gelib_dev,cmd_hdl,GE_SRC,&entity);	

	ge_set_src_mode(gelib_dev,cmd_hdl,GE_SRC_DIRECT_COPY);
	ge_set_xy(gelib_dev,cmd_hdl,GE_DST,dst_rect.left,dst_rect.top);
	ge_set_wh(gelib_dev,cmd_hdl,GE_DST,dst_rect.width,dst_rect.height);	
	ge_set_xy(gelib_dev,cmd_hdl,GE_SRC,src_rect.left,src_rect.top);
	ge_cmd_end(gelib_dev, cmd_hdl);
	
	return ge_cmd_list_end(gelib_dev,gelib_cmdlst);
}

RET_CODE gecmd_retrievescrn(PGUI_VSCR pvscr,PGUI_RECT prect)
{/*Retrieve data from screen,that is retrieve memory buffer with GMA display data*/
	UINT8 *pdbuff,*psbuff;
	UINT32 cmd_hdl;
	PGUI_REGION psrc,pvrgn;
	ge_operating_entity entity;
	ge_rect_t src_rect,dst_rect;
	struct ge_device * gelib_dev;

	if((pvscr->suspend>0) || (ge_version > GE_MODULE_M3329D))
		return RET_SUCCESS;

	gelib_dev = (struct ge_device *)gui_dev;	
	pvrgn = gelib_getdstrgn(pvscr->root_layer,pvscr->root_rgn);
	psrc = gelib_getdstrgn(pvscr->dst_layer,pvscr->dst_rgn);
	pdbuff = (UINT8 *)pvrgn->hsurf;
	psbuff = (UINT8 *)psrc->hsurf;

	if(NULL != prect)
	{
		GUI_RECT crct;

		OSD_GetRectsCross(prect,&(pvscr->frm),&crct);
		if((crct.uWidth != 0)&&(crct.uHeight != 0))
		{
		src_rect.left = crct.uStartX;
		src_rect.top = crct.uStartY;
		src_rect.width = MIN(crct.uWidth,pvscr->frm.uWidth);
		src_rect.height = MIN(crct.uHeight,pvscr->frm.uHeight);
		}
		else
			return RET_FAILURE;
		}
	else
	{
		src_rect.left = pvscr->frm.uStartX;
		src_rect.top = pvscr->frm.uStartY;
		src_rect.width = pvscr->frm.uWidth;
		src_rect.height = pvscr->frm.uHeight;
	}
	MEMCPY(&dst_rect,&src_rect,sizeof(ge_rect_t));

	ge_cmd_list_new(gelib_dev, gelib_cmdlst, GE_COMPILE_AND_EXECUTE);
	if(psrc->layer_type < GUI_MEMVSCR_SURF)
	{
		ge_gma_set_region_to_cmd_list(gelib_dev,(UINT32)psrc->layer_type,(UINT32)psrc->rgn_id,gelib_cmdlst);
		cmd_hdl = ge_cmd_begin(gelib_dev,gelib_cmdlst,GE_PRIM_DISABLE);
	}
	else
	{
		entity.color_format = GUICOLOR_2_GECOLOR(pvscr->color_mode);
		entity.base_address = (UINT32)psbuff;
		entity.data_decoder = GE_DECODER_DISABLE;
		entity.pixel_pitch = gelib_get_rgnpitch(psrc);
		entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
		
		cmd_hdl = ge_cmd_begin(gelib_dev,gelib_cmdlst,GE_PRIM_DISABLE);
		ge_set_operating_entity(gelib_dev,cmd_hdl,GE_SRC,&entity);
		ge_set_color_format(gelib_dev,cmd_hdl,entity.color_format);
	}
	
	entity.color_format = GUICOLOR_2_GECOLOR(pvscr->color_mode);
	entity.base_address = (UINT32)pdbuff;
	entity.data_decoder = GE_DECODER_DISABLE;
	entity.pixel_pitch = gelib_get_rgnpitch(pvrgn);
	entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
	ge_set_operating_entity(gelib_dev,cmd_hdl,GE_DST,&entity);	

	ge_set_src_mode(gelib_dev,cmd_hdl,GE_SRC_DIRECT_COPY);
	ge_set_xy(gelib_dev,cmd_hdl,GE_DST,dst_rect.left,dst_rect.top);
	ge_set_wh(gelib_dev,cmd_hdl,GE_DST,dst_rect.width,dst_rect.height);	
	ge_set_xy(gelib_dev,cmd_hdl,GE_SRC,src_rect.left,src_rect.top);
	ge_cmd_end(gelib_dev, cmd_hdl);
	
	return ge_cmd_list_end(gelib_dev,gelib_cmdlst);
}

/*##########################################################################*/
RET_CODE gecmd_colorkey_rgn(PGUI_REGION pdst,PGUI_REGION prgn1,PGUI_REGION prgn2,PGUI_RECT prct1,PGUI_RECT prct2,UINT32 param)
{/*Do colorkey operation,colorkey small region on large one,consider small one as bitmap*/
	UINT8 *srcbuff1,*srcbuff2,*dstbuff;
	UINT32 cmd_hdl,colour;
	GUI_RECT cross_rct,*srct1,*srct2;
	ge_operating_entity entity;
	ge_rect_t src_rct1,src_rct2,dst_rect;
	struct ge_device * gelib_dev;

	if(prct1 == NULL)
		srct1 = &(prgn1->rect);
	else
		srct1 = prct1;

	if(prct2 == NULL)
		srct2 = &(prgn2->rect);
	else
		srct2 = prct2;
	MEMSET(&cross_rct,0x0,sizeof(cross_rct));
	OSD_GetRectsCross(srct1,srct2,&cross_rct);
	if((cross_rct.uWidth == 0) || (cross_rct.uHeight == 0))
		return RET_FAILURE;/*No overlapped region position*/

	src_rct1.left = (INT32)cross_rct.uStartX;
	src_rct1.top = (INT32)cross_rct.uStartY;
	src_rct1.width = (INT32)cross_rct.uWidth;
	src_rct1.height = (INT32)cross_rct.uHeight;
	MEMCPY(&src_rct2,&src_rct1,sizeof(src_rct2));
	MEMCPY(&dst_rect,&src_rct1,sizeof(dst_rect));
	
	dstbuff = (UINT8 *)pdst->hsurf;
	srcbuff1 = (UINT8 *)prgn1->hsurf;
	srcbuff2 = (UINT8 *)prgn2->hsurf;
	gelib_dev = (struct ge_device *)gui_dev;
	ge_cmd_list_new(gelib_dev, gelib_cmdlst, GE_COMPILE_AND_EXECUTE);

	if(pdst->layer_type < GUI_MEMVSCR_SURF)
	{
		ge_gma_set_region_to_cmd_list(gelib_dev,(UINT32)pdst->layer_type,(UINT32)pdst->rgn_id,gelib_cmdlst);
		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst, GE_DRAW_BITMAP);
	}
	else
	{
		entity.color_format = GUICOLOR_2_GECOLOR(pdst->color_mode);
		entity.base_address = (UINT32)dstbuff;
		entity.data_decoder = GE_DECODER_DISABLE;
		entity.pixel_pitch = gelib_get_rgnpitch(pdst);
		entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
		
		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst, GE_DRAW_BITMAP);
		ge_set_operating_entity(gelib_dev,cmd_hdl,GE_DST,&entity);
		ge_set_color_format(gelib_dev,cmd_hdl,entity.color_format);
	}
	entity.color_format = GUICOLOR_2_GECOLOR(prgn1->color_mode);
	entity.base_address = (UINT32)srcbuff1;
	entity.data_decoder = GE_DECODER_DISABLE;
	entity.pixel_pitch = gelib_get_rgnpitch(prgn1);
	entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
	ge_set_operating_entity(gelib_dev,cmd_hdl,GE_SRC,&entity);	
	entity.color_format = GUICOLOR_2_GECOLOR(prgn2->color_mode);
	entity.base_address = (UINT32)srcbuff2;
	entity.data_decoder = GE_DECODER_DISABLE;
	entity.pixel_pitch = gelib_get_rgnpitch(prgn2);
	entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
	ge_set_operating_entity(gelib_dev,cmd_hdl,GE_PTN,&entity);
	
	ge_set_src_mode(gelib_dev,cmd_hdl,GE_SRC_BITBLT);
	ge_set_xy(gelib_dev,cmd_hdl,GE_DST,dst_rect.left,dst_rect.top);
	ge_set_xy(gelib_dev,cmd_hdl,GE_SRC,dst_rect.left,dst_rect.top);            
	ge_set_xy(gelib_dev,cmd_hdl,GE_PTN,src_rct1.left,src_rct1.top);
	ge_set_wh(gelib_dev,cmd_hdl,GE_DST_PTN,dst_rect.width,dst_rect.height);

	colour = gui_colormask(prgn1,param);
	ge_set_colorkey_mode(gelib_dev,cmd_hdl,GE_CKEY_PTN_PRE_CLUT);
	ge_set_colorkey_match_mode(gelib_dev,cmd_hdl, GE_CKEY_CHANNEL_R,GE_CKEY_MATCH_IN_RANGE);
	ge_set_colorkey_match_mode(gelib_dev,cmd_hdl, GE_CKEY_CHANNEL_G,GE_CKEY_MATCH_IN_RANGE);
	ge_set_colorkey_match_mode(gelib_dev,cmd_hdl, GE_CKEY_CHANNEL_B,GE_CKEY_MATCH_IN_RANGE);
	ge_set_colorkey_range(gelib_dev,cmd_hdl,colour,colour);
    
	ge_cmd_end(gelib_dev,cmd_hdl);					
	
	return ge_cmd_list_end(gelib_dev,gelib_cmdlst);
}

RET_CODE gecmd_blend_rgn(PGUI_REGION pdst,PGUI_REGION prgn1,PGUI_REGION prgn2,PGUI_RECT prct1,PGUI_RECT prct2,UINT32 param)
{/*Consider the small region as a bitmap data,blending the two into the GMA region or dest region*/
	UINT8 *srcbuff1,*srcbuff2,*dstbuff;
	UINT32 cmd_hdl;
	GUI_RECT cross_rct,*srct1,*srct2;
	ge_operating_entity entity;
	ge_rect_t src_rct1,src_rct2,dst_rect;
	struct ge_device * gelib_dev;

	if(prct1 == NULL)
		srct1 = &(prgn1->rect);/*default parameter value*/
	else
		srct1 = prct1;

	if(prct2 == NULL)
		srct2 = &(prgn2->rect);/*default parameter value*/
	else
		srct2 = prct2;
	MEMSET(&cross_rct,0x0,sizeof(cross_rct));
	OSD_GetRectsCross(srct1,srct2,&cross_rct);
	if((cross_rct.uWidth == 0) || (cross_rct.uHeight == 0))
		return RET_FAILURE;/*No overlapped region position*/

	src_rct1.left = (INT32)cross_rct.uStartX;
	src_rct1.top = (INT32)cross_rct.uStartY;
	src_rct1.width = (INT32)cross_rct.uWidth;
	src_rct1.height = (INT32)cross_rct.uHeight;
	MEMCPY(&src_rct2,&src_rct1,sizeof(src_rct2));
	MEMCPY(&dst_rect,&src_rct1,sizeof(dst_rect));/*Get the cross rect*/
	
	dstbuff = (UINT8 *)pdst->hsurf;
	srcbuff1 = (UINT8 *)prgn1->hsurf;
	srcbuff2 = (UINT8 *)prgn2->hsurf;
	gelib_dev = (struct ge_device *)gui_dev;
	ge_cmd_list_new(gelib_dev, gelib_cmdlst, GE_COMPILE_AND_EXECUTE);

	if(pdst->layer_type < GUI_MEMVSCR_SURF)
	{
		ge_gma_set_region_to_cmd_list(gelib_dev,(UINT32)pdst->layer_type,(UINT32)pdst->rgn_id,gelib_cmdlst);
		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst, GE_DRAW_BITMAP_ALPHA_BLENDING);
	}
	else
	{
		entity.color_format = GUICOLOR_2_GECOLOR(pdst->color_mode);
		entity.base_address = (UINT32)dstbuff;
		entity.data_decoder = GE_DECODER_DISABLE;
		entity.pixel_pitch = gelib_get_rgnpitch(pdst);
		entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
		
		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst, GE_DRAW_BITMAP_ALPHA_BLENDING);
		ge_set_operating_entity(gelib_dev,cmd_hdl,GE_DST,&entity);
		ge_set_color_format(gelib_dev,cmd_hdl,entity.color_format);
	}
	
	entity.color_format = GUICOLOR_2_GECOLOR(prgn1->color_mode);
	entity.base_address = (UINT32)srcbuff1;
	entity.data_decoder = GE_DECODER_DISABLE;
	entity.pixel_pitch = gelib_get_rgnpitch(prgn1);
	entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
	ge_set_operating_entity(gelib_dev,cmd_hdl,GE_SRC,&entity);	
	entity.color_format = GUICOLOR_2_GECOLOR(prgn2->color_mode);
	entity.base_address = (UINT32)srcbuff2;
	entity.data_decoder = GE_DECODER_DISABLE;
	entity.pixel_pitch = gelib_get_rgnpitch(prgn2);
	entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
	ge_set_operating_entity(gelib_dev,cmd_hdl,GE_PTN,&entity);
	
	ge_set_xy(gelib_dev, cmd_hdl, GE_DST_SRC,dst_rect.left,dst_rect.top);
	ge_set_wh(gelib_dev, cmd_hdl, GE_DST_PTN,dst_rect.width,dst_rect.height);

    ge_set_global_alpha(gelib_dev, cmd_hdl,param);
	//ge_set_global_alpha_mode(gelib_dev,cmd_hdl,0);
    ge_set_global_alpha_sel(gelib_dev,cmd_hdl,GE_USE_GALPHA_MULTIPLY_PTN_ALPHA);

	ge_cmd_end(gelib_dev,cmd_hdl);	
	return ge_cmd_list_end(gelib_dev,gelib_cmdlst);
}

RET_CODE gecmd_flush_rgn(PGUI_REGION pdst,PGUI_REGION psrc,PGUI_RECT pdst_rct,PGUI_RECT psrc_rct,UINT32 param)
{
	UINT8 *pdbuff,*psbuff;
	UINT32 cmd_hdl;
	RET_CODE cmd_ret;
	INT32 w,h;
	ge_rect_t src_rect,dst_rect;
	ge_operating_entity entity;
	struct ge_device * gelib_dev;

	if(NULL != pdst_rct)
	{
		dst_rect.left = pdst_rct->uStartX;
		dst_rect.top = pdst_rct->uStartY;
		dst_rect.width = pdst_rct->uWidth;
		dst_rect.height = pdst_rct->uHeight;
	}
	else
	{
		dst_rect.left = 0;//pdst->rect.uStartX;
		dst_rect.top = 0;//pdst->rect.uStartY;
		dst_rect.width = pdst->rect.uWidth;
		dst_rect.height = pdst->rect.uHeight;
	}

	if(NULL != psrc_rct)
	{
		src_rect.left = psrc_rct->uStartX;
		src_rect.top = psrc_rct->uStartY;
		src_rect.width = psrc_rct->uWidth;
		src_rect.height = psrc_rct->uHeight;
	}
	else
	{
		src_rect.left = 0;//psrc->rect.uStartX;
		src_rect.top = 0;//psrc->rect.uStartY;
		src_rect.width = psrc->rect.uWidth;
		src_rect.height = psrc->rect.uHeight;
	}

	cmd_hdl = 0;
	pdbuff = (UINT8 *)pdst->hsurf;
	psbuff = (UINT8 *)psrc->hsurf;	
	gelib_dev = (struct ge_device *)gui_dev;
	ge_cmd_list_new(gelib_dev, gelib_cmdlst, GE_COMPILE_AND_EXECUTE);
	if(pdst->layer_type < GUI_MEMVSCR_SURF)
	{
		ge_gma_set_region_to_cmd_list(gelib_dev,(UINT32)pdst->layer_type,(UINT32)pdst->rgn_id,gelib_cmdlst);
		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst, GE_PRIM_DISABLE);
	}
	else
	{
		MEMSET(&entity,0x0,sizeof(entity));
		entity.color_format = GUICOLOR_2_GECOLOR(pdst->color_mode);
		entity.base_address = (UINT32)pdbuff;
		entity.data_decoder = GE_DECODER_DISABLE;
		entity.pixel_pitch = gelib_get_rgnpitch(pdst);
		entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;

		cmd_hdl = ge_cmd_begin(gelib_dev, gelib_cmdlst, GE_PRIM_DISABLE);
		ge_set_operating_entity(gelib_dev,cmd_hdl,GE_DST,&entity);
		ge_set_color_format(gelib_dev,cmd_hdl,entity.color_format);	
	}
	
	MEMSET(&entity,0x0,sizeof(entity));
	entity.color_format = GUICOLOR_2_GECOLOR(psrc->color_mode);
	entity.base_address = (UINT32)psbuff;
	entity.data_decoder = GE_DECODER_DISABLE;
	entity.pixel_pitch = gelib_get_rgnpitch(psrc);
	entity.modify_flags = GE_BA_FLAG_ADDR|GE_BA_FLAG_FORMAT|GE_BA_FLAG_PITCH;
	ge_set_operating_entity(gelib_dev,cmd_hdl,GE_SRC,&entity); 
	
	w = MIN(dst_rect.width,src_rect.width);
	h = MIN(dst_rect.height,src_rect.height);
	ge_set_src_mode(gelib_dev,cmd_hdl,GE_SRC_DIRECT_COPY);
	ge_set_xy(gelib_dev,cmd_hdl,GE_DST,dst_rect.left,dst_rect.top);
	ge_set_xy(gelib_dev,cmd_hdl,GE_SRC,src_rect.left,src_rect.top);
	ge_set_wh(gelib_dev,cmd_hdl,GE_DST_SRC,w,h);

	ge_cmd_end(gelib_dev, cmd_hdl);
	return ge_cmd_list_end(gelib_dev,gelib_cmdlst);
}

/*#########################################################################################################*/
RET_CODE gecmd_setclip(PGUI_REGION prgn,PGUI_RECT prect,UINT32 param)
{
	RET_CODE ret;

	ret = RET_FAILURE;
	if((NULL != prgn) && (prgn->layer_type < GUI_MEMVSCR_SURF))
	{
		ge_clip_t clip;
		enum GE_CLIP_MODE clip_mode;
		struct ge_device * dev;

		dev = (struct ge_device *)gui_dev;
		clip_mode = (enum GE_CLIP_MODE)param;/*inside or outside of the rectangular*/
		ge_gma_set_region_clip_mode(dev,(UINT32)prgn->layer_type,clip_mode);
		ret = ge_gma_set_region_clip_rect(dev,(UINT32)prgn->layer_type,(UINT32)prect->uStartX,(UINT32)prect->uStartY,(UINT32)prect->uWidth,(UINT32)prect->uHeight);
	}

	return ret;
}

RET_CODE gecmd_clearclip(PGUI_REGION prgn)
{
	RET_CODE ret;

	ret = RET_FAILURE;
	if((NULL != prgn) && (prgn->layer_type < GUI_MEMVSCR_SURF))
	{
		struct ge_device * dev;
		enum GE_CLIP_MODE clip_mode;

		clip_mode = GE_CLIP_DISABLE;
		dev = (struct ge_device *)gui_dev;		
		ret = ge_gma_set_region_clip_mode(dev,(UINT32)prgn->layer_type,clip_mode);
	}

	return ret;
}

RET_CODE gecmd_setrgnpos(PGUI_REGION prgn,PGUI_RECT prect)
{/*only valid for gma region*/
	ge_gma_region_t rgn_param;

    if((NULL != prgn)&&(prgn->layer_type<GUI_MEMVSCR_SURF))
    {
    	MEMSET(&rgn_param,0x0,sizeof(rgn_param));
    	ge_gma_get_region_info((struct ge_device *)gui_dev,(UINT32)prgn->layer_type,(UINT32)prgn->rgn_id,&rgn_param);

    	rgn_param.region_x = prect->uStartX;
    	rgn_param.region_y = prect->uStartY;
    	rgn_param.region_w = prect->uHeight;
    	rgn_param.region_h = prect->uWidth;
    	return ge_gma_move_region((struct ge_device *)gui_dev,(UINT32)prgn->layer_type,(UINT32)prgn->rgn_id,&rgn_param);
    }
    else
        return RET_FAILURE;
}

RET_CODE gecmd_getrgnpos(PGUI_REGION prgn,PGUI_RECT prect)
{
	RET_CODE ret;
	ge_gma_region_t rgn_param;

    ret = RET_SUCCESS;
    if((NULL != prgn)&&(prgn->layer_type<GUI_MEMVSCR_SURF))
    {
    	MEMSET(&rgn_param,0x0,sizeof(rgn_param));
    	ret = ge_gma_get_region_info((struct ge_device *)gui_dev,(UINT32)prgn->layer_type,(UINT32)prgn->rgn_id,&rgn_param);

    	//set prect
    	prect->uStartX = rgn_param.region_x;
    	prect->uStartY = rgn_param.region_y;
    	prect->uWidth = rgn_param.region_w;
    	prect->uHeight = rgn_param.region_h;
    }
    else
    {
    	prect->uStartX = prgn->rect.uStartX;
    	prect->uStartY = prgn->rect.uStartY;
    	prect->uWidth = prgn->rect.uWidth;
    	prect->uHeight = prgn->rect.uHeight;
    }
	return ret;
}

RET_CODE gecmd_modifypallette(PGUI_REGION prgn,UINT8 idx,UINT8 *pallette)
{
	if((NULL != prgn) && (prgn->layer_type < GUI_MEMVSCR_SURF) && (GUI_CLUT8 == prgn->color_mode))
	{
		UINT8 alpha,y,cb,cr;
		ge_pal_attr_t pal_attr;

		y = *(pallette+0);
		cb = *(pallette+1);
		cr = *(pallette+2);
		alpha = (*pallette+3);
		gui_pallette[idx] = (alpha<<24) + (y<<16) + (cb<<8) + cr;
		
		MEMSET(&pal_attr,0x0,sizeof(pal_attr));
		pal_attr.pal_type = GE_PAL_YCBCR;
		pal_attr.rgb_order = GE_RGB_ORDER_ACrCbY;
		pal_attr.alpha_range = GE_ALPHA_RANGE_0_15;
		pal_attr.alpha_pol = GE_ALPHA_POLARITY_0;
		return ge_gma_modify_pallette((struct ge_device *)gui_dev,(UINT32)prgn->layer_type,idx,alpha,y,cb,cr,&pal_attr);
	}
	else
	{
		return RET_FAILURE;
	}
}

RET_CODE gecmd_setpallette(PGUI_REGION prgn,UINT8 *pallette)
{
	if((NULL != prgn) && (prgn->layer_type < GUI_MEMVSCR_SURF) && (GUI_CLUT8 == prgn->color_mode))
	{
		UINT32 idx;
		UINT8 alpha,y,cb,cr;
		ge_pal_attr_t pal_attr;

		for(idx = 0;idx < 256;idx++)
		{
			y = *(pallette + (idx<<2) + 0);
			cb = *(pallette + (idx<<2) + 1);
			cr = *(pallette + (idx<<2) + 2);
			alpha = *(pallette + (idx<<2) + 3);
			gui_pallette[idx] = (alpha<<24) + (y<<16) + (cb<<8) + cr;
		}
		
		MEMSET(&pal_attr,0x0,sizeof(pal_attr));
		pal_attr.pal_type = GE_PAL_YCBCR;
		pal_attr.rgb_order = GE_RGB_ORDER_ACrCbY;
		pal_attr.alpha_range = GE_ALPHA_RANGE_0_15;
		pal_attr.alpha_pol = GE_ALPHA_POLARITY_0;
		return ge_gma_set_pallette((struct ge_device *)gui_dev,(UINT32)prgn->layer_type,pallette,(UINT16)CLUT_COLOR_N,&pal_attr);
	}
	else
	{
		return RET_FAILURE;
	}
}

RET_CODE gecmd_setgalpha(UINT32 layer,UINT32 gtrans)
{
	ge_io_cmd_par_t param;
	
	if(layer < GUI_MEMVSCR_SURF)
	{
		MEMSET(&param,0x0,sizeof(param));
		param.par0 = layer;
		param.par1 = gtrans;
		return ge_io_ctrl((struct ge_device *)gui_dev,GE_IO_SET_GLOBAL_ALPHA,(UINT32)&param);
	}
	else
		return RET_FAILURE;
}

RET_CODE gecmd_getpallette(PGUI_REGION prgn,UINT8 *pallette)
{
	if((NULL != prgn) && (prgn->layer_type < GUI_MEMVSCR_SURF) && (GUI_CLUT8 == prgn->color_mode))
	{/*Only valid for CLUT8 GMA region type*/
		ge_pal_attr_t pal_attr;
		
		MEMSET(&pal_attr,0x0,sizeof(pal_attr));
		pal_attr.pal_type = GE_PAL_YCBCR;
		pal_attr.rgb_order = GE_RGB_ORDER_ACrCbY;
		pal_attr.alpha_range = GE_ALPHA_RANGE_0_15;
		pal_attr.alpha_pol = GE_ALPHA_POLARITY_0;
		return ge_gma_get_pallette((struct ge_device *)gui_dev,(UINT32)prgn->layer_type,pallette,(UINT16)CLUT_COLOR_N,&pal_attr);
	}
	else
	{
		return RET_FAILURE;
	};
}

RET_CODE gecmd_scale(PGUI_REGION prgn,UINT32 cmd,UINT32 param)
{
	return ge_gma_scale((struct ge_device *)gui_dev,(UINT32)(prgn->layer_type),cmd,param);
}//scale API function pointer declaration should be changed
/*##########################################################################*/
RET_CODE gecmd_rgn_showonoff(PGUI_REGION prgn,UINT8 on_off)
{
	RET_CODE ret;
	
	ret = RET_FAILURE;
	if((NULL != prgn) && (prgn->layer_type < GUI_MEMVSCR_SURF))
	{/*To be test*/
		ge_gma_show_onoff((struct ge_device *)gui_dev,(UINT32)prgn->layer_type,(BOOL)on_off);
		ret = ge_gma_show_region((struct ge_device *)gui_dev,(UINT32)prgn->layer_type,(UINT32)prgn->rgn_id,(BOOL)on_off);
	}

	return ret;
}

RET_CODE gecmd_deletergn(PGUI_REGION prgn)
{
	RET_CODE ret;

	ret = RET_FAILURE;
	if(NULL == prgn)
	{
		return RET_FAILURE;
	}
	else if(prgn->layer_type < GUI_MEMVSCR_SURF)
	{
		ret = ge_gma_delete_region((struct ge_device *)gui_dev,prgn->layer_type,prgn->rgn_id);
	}
	else
	{
		if(NULL != (void *)prgn->hsurf)
		{
			FREE((void *)prgn->hsurf);
		}
		else
			ret = RET_FAILURE;
	}
	MEMSET(prgn,0x0,sizeof(GUI_REGION));

	return ret;
}

RET_CODE gecmd_creatergn(UINT32 param)
{
	UINT8 idx,rgn_idx,*p_pallette;
	RET_CODE ret_error;
	PGUI_REGION prgn,pcreated;
	ge_gma_region_t rgn_para;

	prgn = (PGUI_REGION)param;
	idx = prgn->layer_type;
	rgn_idx = prgn->rgn_id;
	pcreated = gelib_getdstrgn(idx,rgn_idx);
	if(pcreated != prgn)
		MEMCPY(pcreated,prgn,sizeof(GUI_REGION));//sync data with region tables
        
	if(idx < GUI_MEMVSCR_SURF)//GMA surface need create region
	{
		MEMSET(&rgn_para,0x0,sizeof(rgn_para));
		rgn_para.galpha_enable = 1;
		rgn_para.global_alpha = 0xff;
		rgn_para.region_x = (UINT16)(prgn->rect.uStartX);
		rgn_para.region_y = (UINT16)(prgn->rect.uStartY);
		rgn_para.region_w = (UINT16)(prgn->rect.uWidth);
		rgn_para.region_h = (UINT16)(prgn->rect.uHeight);
		//rgn_para.bitmap_addr = __MM_OSD_BK_ADDR1;
		/*rgn_para.bitmap_x = (UINT32)(prgn->rect.uStartX);
		rgn_para.bitmap_y = (UINT32)(prgn->rect.uStartY);
		rgn_para.bitmap_w = (UINT32)(prgn->rect.uWidth);
		rgn_para.bitmap_h = (UINT32)(prgn->rect.uHeight);
		rgn_para.pixel_pitch = (UINT16)gelib_get_rgnpitch(prgn);*/
		switch(prgn->color_mode)
		{
			case GUI_CLUT8:
				rgn_para.color_format = GE_PF_CLUT8;
				break;
			case GUI_ARGB1555:
				rgn_para.color_format = GE_PF_ARGB1555;
				break;
			case GUI_ARGB8888:
				rgn_para.color_format = GE_PF_ARGB8888;
				break;				
			default:
				rgn_para.color_format = GE_PF_ARGB8888;
		}
		
		pcreated->hsurf = 0x0;
		if(RET_SUCCESS == (ret_error = ge_gma_create_region((struct ge_device *)gui_dev,idx,rgn_idx,&rgn_para)))
		{
			ge_gma_get_region_info((struct ge_device *)gui_dev,idx,rgn_idx,&rgn_para);
			pcreated->hsurf = rgn_para.bitmap_addr;
		}
	
		/*Other init operation for region display*/
		if(prgn->color_mode == GUI_CLUT8)
		{
			p_pallette = OSD_GetRscPallette(0x4080 | 0);
			gelib_set_pallette(prgn,p_pallette);
		}
	}
	else
	{
		vscr_buff = (UINT8 *)MALLOC(gelib_get_rgnsize(prgn));
		
		pcreated->hsurf = (SURF_HANDLE)vscr_buff;
		gelib_fillrect(pcreated,NULL,transcolor_const[prgn->color_mode]);
		if((pcreated->hsurf != 0) && (idx == GUI_MEMVSCR_SURF))
		{
			UINT32 vidx;
			extern UINT8 *vscr_buff;

			vscr_buff = (UINT8 *)pcreated->hsurf;
			for(vidx = 0;vidx < GUI_MULTITASK_CNT;vidx++)
			{
				g_vscr[vidx].lpbuff = vscr_buff;
				MEMSET(&(g_vscr[vidx].frm),0x0,sizeof(GUI_RECT));
				g_vscr[vidx].color_mode = (UINT8)pcreated->color_mode;
				//MEMCPY(&(g_vscr[vidx].frm),&(pcreated->rect),sizeof(GUI_RECT));
			}
		}
		ret_error = RET_SUCCESS;
	}
	
	return ret_error;
}
/*##########################################################################*/

