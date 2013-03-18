/*-----------------------------------------------------------------------------
*
*	Copyright (C) 2008 ALI Corp. All rights reserved.
*
*	File: ge_draw_primitive.c
*
*	Content: 
*		GUI primitive draw function & GE operation functions
*	History: 
*		2008/06/02 by Shine Zhou
*		2009/05/07 revised by Shine Zhou
*		New build for GE drivers directly draw GUI objects.
*-----------------------------------------------------------------------------*/
#include <sys_config.h>
#include <basic_types.h>
#include <hld/osd/osddrv.h>
#include <api/libge/osd_lib.h>
#include <api/libc/string.h>
#include <api/libchar/lib_char.h>
#include <api/libge/osd_primitive.h>
#include <api/libge/ge_primitive_init.h>
#include <api/libge/ge_draw_primitive.h>
#include "osd_lib_internal.h"
/******************************************************************************************
*GUI primitive draw global variances.
******************************************************************************************/
UINT8	*vscr_buff = NULL;
UINT8	*pop_buff = NULL;
UINT8	*slvscr_buff = NULL;
UINT8	*slvpop_buff = NULL;

UINT8 vrgn_idx = 0;
UINT8 vlayer_idx = 0;

#define GELIB_M3602_SCALE//M3602 only
#define GELIB_M3602_DUALOUT

#ifdef GELIB_M3602_DUALOUT
UINT32 g_gelib_m3602_dualoutput;
#endif

#ifdef GELIB_M3602_SCALE
UINT32	g_size_max;//M3602 only
UINT32  g_gui_msize;//M3602 only
GUI_RECT	g_rct_scaleclip;//M3602 only
GUI_TVSYS	g_gui_tvsys =(UINT32)(~0);//M3602 only gavinliu
GUI_SCALE_FACTOR	g_gui_coeff;//M3602 only

const UINT32 gui_max_size[6] = {
	(720<<16)|480,//NTSC
	(720<<16)|576,//PAL
	(1280<<16)|720,//720P
	(1920<<16)|1080,//1080i
	(720<<16)|576,//reserved
	(720<<16)|576,/*reserved*/};
const GUI_SCALE_FACTOR gui_scale_coeffs[12] = {
	{1,1,5,6}, /*576==>480*/
	{1,1,1,1}, /*576==>576*/
	{5,4,16,9}, /*576==>720*/
	{15,8,8,3},/*576==>1080*/
		
	{2,3,9,16}, /*720==>480*/
	{4,5,9,16}, /*720==>576*/
	{1,1,1,1}, /*720==>720*/
	{3,2,3,2}/*720==>1080*/};
#define GUI_SCALE_COEFFS_LEN (sizeof(gui_scale_coeffs)/sizeof(gui_scale_coeffs[0]))

#define GUI_HSCALE(x)	((UINT16)((x)*(g_gui_coeff.hmul)/(g_gui_coeff.hdiv) + 0.5))//M3602 only
#define GUI_VSCALE(y)	((UINT16)((y)*(g_gui_coeff.vmul)/(g_gui_coeff.vdiv) + 0.5))//M3602 only
#define GUI_RECT_SCALE(prct)	do{ \
								(prct)->uStartX = GUI_HSCALE((prct)->uStartX); \
								(prct)->uStartY = GUI_VSCALE((prct)->uStartY); \
								(prct)->uWidth = GUI_HSCALE((prct)->uWidth); \
								(prct)->uHeight = GUI_VSCALE((prct)->uHeight); \
							}while(0)
							
#define GUI_HREVERT(x)	((UINT16)((x)*(g_gui_coeff.hdiv)/(g_gui_coeff.hmul) + 0.5))//M3602 only
#define GUI_VREVERT(y)	((UINT16)((y)*(g_gui_coeff.vdiv)/(g_gui_coeff.vmul) + 0.5))//M3602 only
#define GUI_RCT_SCALERVT(prct)	do{ \
								(prct)->uStartX = GUI_HREVERT((prct)->uStartX); \
								(prct)->uStartY = GUI_VREVERT((prct)->uStartY); \
								(prct)->uWidth = GUI_HREVERT((prct)->uWidth); \
								(prct)->uHeight = GUI_VREVERT((prct)->uHeight); \
							}while(0)
#endif

GUI_DEV		gui_dev;
GUI_VSCR	g_vscr[GUI_MULTITASK_CNT];
GUI_VSCR	g_slvscr[GUI_MULTITASK_CNT];
GUI_REGION	g_rgn_tbl[GUI_MAX_REGION_CNT];//region handles array
PGUI_REGION	g_memrgn_tbl;
GUI_LAYER 	g_layer_tbl[GUI_MAX_LAYER_CNT];//total layers handle

UINT32	gui_keycolor[GUI_COLORMAX] = {0x0,0x1,0xff,0xfc1f,0x0,0xdd6aca,0x0};

const UINT8 bypp_const[GUI_COLORMAX] = {0,0,0,1,1,2,2};
const UINT32	colormask_const[GUI_COLORMAX] = {0x0,0x1,0xff,0xffff,0xffff,0xffffffff,0xffffffff};
UINT32 transcolor_const[GUI_COLORMAX]={0x0,0x1,0xff,0x7fff,0x7fff,0x76ab94,0x0};
/******************************************************************************************
*GMA region & layer etc. operation and management APIs
******************************************************************************************/
PGUI_LAYER gedraw_getdstlayer(UINT8 layer_type)
{
	if(layer_type > GUI_MAX_SURF)
		return NULL;
	else
		return &g_layer_tbl[layer_type];
}

ge_surface_desc_t *gedraw_getdst_surf(UINT8 layer_type)
{
    if(layer_type > GUI_MAX_SURF)
		return NULL;
	else
		return (ge_surface_desc_t *)g_layer_tbl[layer_type].hsurf;
}

void gedraw_link2layertbl(PGUI_LAYER psurf)
{
	UINT8 layer_type;
	
	if(NULL == (void *)psurf->hsurf)
		return;

	layer_type = psurf->surf_mode;
	if(layer_type < GUI_MAX_SURF)
		MEMCPY(&g_layer_tbl[layer_type],psurf,sizeof(GUI_LAYER));
}

void gedraw_link2rgntbl(PGUI_REGION prgn)
{
	UINT8 idx;

	if(NULL == (void *)prgn->hsurf)
		return;

	if((prgn->layer_type) < GUI_MEMVSCR_SURF)
	{
		idx = (prgn->layer_type * GUI_MAX_REGION_ONLAYER) + prgn->rgn_id;
	}
	else
	{
		idx = GUI_MEMVSCR_SURF * GUI_MAX_REGION_ONLAYER +(prgn->layer_type - GUI_MEMVSCR_SURF);
	}

	if(idx < sizeof(g_rgn_tbl)/sizeof(g_rgn_tbl[0]))
	{
		MEMCPY(&g_rgn_tbl[idx],prgn,sizeof(GUI_REGION));
	}
}

static UINT8 gui_dev_init(UINT32 param)
{
	gui_dev = (GUI_DEV)(param);
}

static UINT8 gui_tbl_init()
{
	MEMSET(&g_layer_tbl,0x0,sizeof(GUI_LAYER)*GUI_MAX_LAYER_CNT);
	MEMSET(&g_rgn_tbl,0x0,sizeof(GUI_REGION)*GUI_MAX_REGION_CNT);
	g_memrgn_tbl = (g_rgn_tbl + GUI_GMA_REGION_OFFSET);

	return 0;
}

static UINT8 gui_mem_init(UINT32 mem_size,UINT8 color_mode)
{
	UINT8 i;
	
	if((vscr_buff == NULL) && (mem_size > 0))
	{
		vscr_buff = MALLOC(mem_size);
		ASSERT(NULL != vscr_buff);
	}

	vrgn_idx = 0;
	pop_buff = NULL;
	gedraw_up2scrn_cb = NULL;
	vlayer_idx = GUI_GMA1_SURF;
	g_gelib_m3602_dualoutput = 0;
    if(ge_version == GE_MODULE_M3602)
    {//M3602 only
        switch(gui_design_dimension)
        {
            case GUI_NTSC_LINES:
                g_gui_msize = ((720<<16) | 480);
                break;
            case GUI_PAL_LINES:
                g_gui_msize = ((720<<16) | 576);
                break;
            case GUI_720P_LINES:
                g_gui_msize = ((1280<<16) | 720);
                break;
            case GUI_1080i_LINES:
                g_gui_msize = ((1920<<16) | 1080);
                break;
            default:
                g_gui_msize = ((720<<16) | 576);
        }

		if(1)/*((g_gui_coeff.hdiv == 0) || (g_gui_coeff.hmul== 0) \
			 || (g_gui_coeff.vdiv == 0) || (g_gui_coeff.vmul == 0))*/
		{
	    	g_gui_coeff.hdiv = 1;
	    	g_gui_coeff.vdiv = 1;
	    	g_gui_coeff.hmul = 1;
	    	g_gui_coeff.vmul = 1;
		}
        
        g_gui_tvsys = (UINT32)(~0);
		g_size_max = (720<<16|576);
        MEMSET(&g_rct_scaleclip,0xff,sizeof(g_rct_scaleclip));
    }
	
	MEMSET(g_vscr,0x0,sizeof(g_vscr));
	MEMSET(g_slvscr,0x0,sizeof(g_slvscr));
	for(i = 0; i < GUI_MULTITASK_CNT; i++)
	{
		g_vscr[i].lpbuff = vscr_buff;
		g_vscr[i].color_mode = color_mode;
		g_vscr[i].root_layer = GUI_MEMVSCR_SURF;
		g_vscr[i].root_rgn = 0;
		g_vscr[i].dst_layer = GUI_GMA1_SURF;
		g_vscr[i].dst_rgn = 0;
		
		g_slvscr[i].lpbuff = slvscr_buff;
		g_slvscr[i].color_mode = GUI_CLUT8;
		g_slvscr[i].root_layer = GUI_SLVGMA_SURF;
		g_slvscr[i].root_rgn = 0;
		g_slvscr[i].dst_layer = GUI_SLVGMA_SURF;
		g_slvscr[i].dst_rgn = 0;
	}
	
	return 0;
}

UINT8 app_gui_init(PGUI_RGN_CFG prgnlst,UINT32 param)
{
	UINT8 i,surf_cnt;
	UINT32 mem_size;
	PGUI_APP_CFG pcfg;
	PGUI_LAYER psurf;
	PGUI_REGION prgn;
	PGUI_RGN_CFG prgncfg;
	ge_region_pars_t cfg;

	pcfg = (PGUI_APP_CFG)param;
	
	mem_size = (pcfg->layer_rect[GUI_MEMVSCR_SURF].uWidth*pcfg->layer_rect[GUI_MEMVSCR_SURF].uHeight) \
		<<bypp_const[pcfg->layer_color[GUI_MEMVSCR_SURF]];
	if(pcfg->direct_draw > 0)
		mem_size = 0;

	//init global variable
	gui_tbl_init();
	gui_dev_init((UINT32)(pcfg->hdev));
	gui_mem_init(mem_size,pcfg->layer_color[GUI_MEMVSCR_SURF]);

	//create surface
	for(i = 0; i < GUI_MAX_SURF;i++)
	{
		if(GUI_INVALIDFORMAT == pcfg->layer_color[i])
		{
			continue;
		}
		else
		{
			psurf = gedraw_getdstlayer(i);
			psurf->hdev = pcfg->hdev;
			psurf->hsurf = (SURF_HANDLE)NULL;
			psurf->surf_mode = i;
			psurf->rgn_onlayer = 0;
			psurf->color_mode = pcfg->layer_color[i];
			MEMCPY(&(psurf->rect),&(pcfg->layer_rect[i]),sizeof(GUI_RECT));				
			gedraw_createsurf((UINT32)psurf);
			gedraw_surf_showonoff(psurf,1);//set layer as show
		}
	}

	//create region
	surf_cnt = 0;
	prgncfg = prgnlst;
	for(i = 0; i < (pcfg->rgn_cnt);i++)
	{
		psurf = gedraw_getdstlayer(prgncfg->layer_idx);
		prgn = gedraw_getdstrgn(prgncfg->layer_idx,prgncfg->rgn_idx);
		if((psurf->hsurf) != (SURF_HANDLE)NULL)
		{
			prgn->layer_type = prgncfg->layer_idx;
			prgn->rgn_id = prgncfg->rgn_idx;
			prgn->hsurf = psurf->hsurf;
			prgn->color_mode = psurf->color_mode;
			MEMCPY(&(prgn->rect),&(prgncfg->rect),sizeof(GUI_RECT));
			
			gedraw_creatergn((UINT32)prgn);
			gedraw_rgn_showonoff(prgn,1);

			surf_cnt++;
			psurf->rgn_onlayer++;
		}
		prgncfg++;
	}

	prgn = gedraw_getdstrgn(GUI_MEMVSCR_SURF,0);
	if(NULL != (void *)prgn->hsurf)
	{
		MEMSET(&cfg,0x0,sizeof(ge_region_pars_t));
		cfg.addr = vscr_buff;
		cfg.pitch = prgn->rect.uWidth<<bypp_const[prgn->color_mode];
		ge_set_region_pars((struct ge_device *)pcfg->hdev,(ge_surface_desc_t *)prgn->hsurf,0,&cfg);

		if(NULL != vscr_buff)
			gedraw_fillrect(prgn,NULL,transcolor_const[prgn->color_mode]);
	}
	gelib_state = GELIB_READY;
	
	return surf_cnt;
}

UINT8 app_gui_release(UINT32 param)
{
	UINT8 i;
	
	for(i = 0;i < sizeof(g_rgn_tbl)/sizeof(g_rgn_tbl[0]);i++)
	{
		if(NULL != (void *)g_rgn_tbl[i].hsurf)
			gedraw_deletergn(&g_rgn_tbl[i]);
	}

	for(i = 0;i < sizeof(g_layer_tbl)/sizeof(g_layer_tbl[0]);i++)
	{
		if(NULL != (void *)g_layer_tbl[i].hsurf)
			gedraw_releasesurf(&g_layer_tbl[i]);
	}

	if((vscr_buff) && (param == 0))
	{//param is used as if vscr_buff release or not currently
		FREE(vscr_buff);
		vscr_buff = NULL;
	}

	if(pop_buff)
	{
		FREE(pop_buff);
		pop_buff = NULL;
	}
	
	gelib_state = GELIB_WILD;
}

UINT8 app_ge_release(UINT32 param)
{
	UINT8 i;
	
	for(i = 0;i < sizeof(g_rgn_tbl)/sizeof(g_rgn_tbl[0]);i++)
	{
		if(NULL != (void *)g_rgn_tbl[i].hsurf)
			gelib_delete_rgn(&g_rgn_tbl[i]);
	}

	if((vscr_buff) && (param == 0))
	{//param is used as if vscr_buff release or not currently
		FREE(vscr_buff);
		vscr_buff = NULL;
	}

	if(pop_buff)
	{
		FREE(pop_buff);
		pop_buff = NULL;
	}
	
	gelib_state = GELIB_WILD;
}

UINT8 app_popup_init(UINT32 param)
{
	UINT32 mem_size;
	PGUI_LAYER_CFG pcfg;
	PGUI_REGION prgn;
	PGUI_LAYER psurf;

	pcfg = (PGUI_LAYER_CFG)param;
	if(pcfg->slvgma == 0)
		prgn = gedraw_getdstrgn(GUI_MEMPOP_SURF,0);
	else if(gelib_getdual_support())
		prgn = gedraw_getdstrgn(GUI_SLVPOP_SURF,0);
    else
        return 1;

	if((NULL != (void *)prgn->hsurf))
		app_popup_release();

	if(GUI_INVALIDFORMAT == pcfg->color_mode)
		return 2;
	else if(GE_MODULE_NEW< ge_version)
	{
		return gecmd_popup_init(param);
	}
	else
	{
		struct ge_device *dev;
		ge_region_pars_t rgn_cfg;

		mem_size = (pcfg->rect.uWidth * pcfg->rect.uHeight)<<(bypp_const[pcfg->color_mode]);
		if(pcfg->slvgma == 0)
			pop_buff = MALLOC(mem_size);
		else
			slvpop_buff = MALLOC(mem_size);

		psurf = (pcfg->slvgma == 0) ? (&g_layer_tbl[GUI_MEMPOP_SURF]):(&g_layer_tbl[GUI_SLVPOP_SURF]);
		psurf->hdev = (pcfg->hdev == (GUI_DEV)NULL) ? gui_dev:(pcfg->hdev);
		psurf->hsurf = (UINT32)NULL;
		psurf->rgn_onlayer = 0;
		psurf->surf_mode = (pcfg->slvgma == 0) ? GUI_MEMPOP_SURF : GUI_SLVPOP_SURF;
		psurf->color_mode = pcfg->color_mode;
		psurf->rect.uStartX = pcfg->rect.uStartX;
		psurf->rect.uStartY = pcfg->rect.uStartY;
		psurf->rect.uWidth = pcfg->rect.uWidth;
		psurf->rect.uHeight = pcfg->rect.uHeight;
		gedraw_createsurf((UINT32)psurf);

		prgn->hsurf = psurf->hsurf;
		prgn->layer_type = (pcfg->slvgma == 0) ? GUI_MEMPOP_SURF : GUI_SLVPOP_SURF;
		prgn->rgn_id = 0;
		prgn->color_mode = pcfg->color_mode;
		prgn->rect.uStartX = pcfg->rect.uStartX;
		prgn->rect.uStartY = pcfg->rect.uStartY;
		prgn->rect.uWidth = pcfg->rect.uWidth;
		prgn->rect.uHeight = pcfg->rect.uHeight;
		gedraw_creatergn((UINT32)prgn);

		dev = (struct ge_device *)psurf->hdev;
		MEMSET(&rgn_cfg,0,sizeof(rgn_cfg));
		rgn_cfg.addr = (pcfg->slvgma == 0) ? pop_buff : slvpop_buff;
		rgn_cfg.pitch = (pcfg->rect.uWidth)<<(bypp_const[pcfg->color_mode]);
		ge_set_region_pars( dev,(ge_surface_desc_t *)psurf->hsurf,0,&rgn_cfg);
		//gedraw_fillrect(prgn,NULL,(UINT32)transcolor_const[pcfg->color_mode]);
	}
	
	return 0;
}

UINT8 app_popup_release()
{
	PGUI_REGION ppop;
	PGUI_LAYER psurf;

	ppop = gedraw_getdstrgn(GUI_MEMPOP_SURF,0);
	if(GE_MODULE_NEW< ge_version)
	{
		return gecmd_popup_release();
	}
	else if(NULL != (void *)ppop->hsurf)
	{
		psurf = &g_layer_tbl[GUI_MEMPOP_SURF];
		
		gedraw_deletergn(ppop);
		gedraw_releasesurf(psurf);
		FREE(pop_buff);//FREE(rgn_cfg.addr);
		pop_buff = NULL;
	}
	
	ppop = gedraw_getdstrgn(GUI_SLVPOP_SURF,0);
	if(gelib_getdual_support() && (NULL != (void *)ppop->hsurf))
	{
		psurf = &g_layer_tbl[GUI_SLVPOP_SURF];
		
		gedraw_deletergn(ppop);
		gedraw_releasesurf(psurf);
		FREE(slvpop_buff);
		slvpop_buff = NULL;
	}

	return 0;
}

PGUI_REGION gedraw_getdstrgn(UINT8 layer_type,UINT8 rgn_idx)
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

	return (idx >= sizeof(g_rgn_tbl)/sizeof(g_rgn_tbl[0])) ? (PGUI_REGION)NULL : &g_rgn_tbl[idx];
}

/******************************************************************************************
*GUI scale operations for M3602 only
******************************************************************************************/
static UINT32 gui_gmasurf_switch(UINT32 layer,GUI_TVSYS tvsys)
{
	UINT8 rgnidx,idx;
	GUI_TVSYS	cur_tvsys;
	GUI_RECT	rctrgn;
	PGUI_REGION prgn,pmrgn;
	PGUI_LAYER psurf;
	extern SURF_HANDLE g_surf_handle;

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
			cur_tvsys = NTSC; //20100920
			break;
		case LINE_720_25:
		case LINE_720_30:
			idx = 2;
			cur_tvsys = LINE_720_30;
			break;
		case LINE_1080_25:
		case LINE_1080_30:
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

	g_size_max = gui_max_size[idx];
	if(gui_design_dimension == GUI_720P_LINES)
		idx +=4;
	else if(gui_design_dimension == GUI_1080i_LINES)
		idx +=8;//reserved currently
	else
		idx = idx;

	if(GUI_SCALE_COEFFS_LEN<idx)
		idx = 0;
	
	for(rgnidx = 0;rgnidx<GUI_MAX_REGION_ONLAYER;rgnidx++)
	{//delete old ones
		prgn = gedraw_getdstrgn(layer,rgnidx);
		
		if((prgn != NULL) && (prgn->hsurf != (SURF_HANDLE)NULL))
		{
			gedraw_rgn_showonoff(prgn,0);
			gedraw_deletergn(prgn);
		}
	}
	psurf = gedraw_getdstlayer(layer);
	gedraw_releasesurf(psurf);

	osal_task_sleep(6);
	GUI_RECT_SCALE(&(psurf->rect));
	gedraw_createsurf((UINT32)psurf);
	gedraw_surf_showonoff(psurf,1);
	OSD_SetDeviceHandle((HANDLE)(psurf->hsurf));

	pmrgn = gedraw_getdstrgn(GUI_MEMVSCR_SURF,0);
	for(rgnidx = 0;rgnidx<GUI_MAX_REGION_ONLAYER;rgnidx++)
	{//resize and create it
		prgn = gedraw_getdstrgn(layer,rgnidx);
		
		if((psurf->hsurf != (SURF_HANDLE)NULL) && (prgn != NULL) \
          && (prgn->rect.uWidth > 0) && (prgn->rect.uHeight > 0))
		{
			prgn->hsurf = psurf->hsurf;
			
			GUI_RCT_SCALERVT(&(prgn->rect));//revert to original size
			MEMCPY(&rctrgn,&(prgn->rect),sizeof(rctrgn));//original memrgn size
			
			MEMCPY(&g_gui_coeff,&gui_scale_coeffs[idx],sizeof(g_gui_coeff));
			GUI_RECT_SCALE(&(prgn->rect));
			
			gedraw_creatergn((UINT32)prgn);
			gedraw_flush_rgn(prgn,pmrgn,NULL,&rctrgn,0);
			//gedraw_fillrect(prgn,NULL,transcolor_const[prgn->color_mode]);
			
			gedraw_rgn_showonoff(prgn,1);
		}
	}

	if(1)//enable scale down filter
	{
	        ge_scale_mode_cfg_t scale_mode_cfg;
			
	        scale_mode_cfg.surf = (ge_surface_desc_t *)(psurf->hsurf);
	        scale_mode_cfg.h_scale_mode = GE_RSZ_ALPHA_COLOR;
	        scale_mode_cfg.v_scale_mode = GE_RSZ_ALPHA_COLOR;
	        scale_mode_cfg.reserved = 0;
	        ge_io_ctrl((struct ge_device *)gui_dev,GE_IO_SET_SCALE_MODE, (UINT32)&scale_mode_cfg );
	}

	return 0;
}

#ifdef GELIB_M3602_SCALE
static void ge3602_expand_rect(PGUI_REGION pmemrgn,PGUI_RECT rct_dst)
{
	UINT16 wrct,hrct;
	UINT16 rxrgn,byrgn;
	UINT16 lxrct,rxrct,tyrct,byrct;

	wrct = rct_dst->uWidth;
	hrct = rct_dst->uHeight;
	lxrct = rct_dst->uLeft;
	tyrct = rct_dst->uTop;
	rxrct = (rct_dst->uLeft+rct_dst->uWidth);
	byrct = (rct_dst->uTop+rct_dst->uHeight);
	rxrgn = (pmemrgn->rect.uLeft+pmemrgn->rect.uWidth);
	byrgn = (pmemrgn->rect.uTop+pmemrgn->rect.uHeight);

	if(lxrct>2)
	{
		lxrct -= 2;
		if((rxrct+2)<=rxrgn)
		{
			wrct += 4;
		}
		else if(rxrct<=rxrgn)
		{
			wrct += 2;
		}
		else
		{
			//DO NULL
		}
	}
	else if((rxrct+2)<=rxrgn)
	{
		wrct += 2;
	}
	else
	{
			//DO NULL
	}
	
	if(tyrct>2)
	{
		tyrct -= 2;
		if((byrct+2)<=byrgn)
		{
			hrct += 4;
		}
		else if(byrct<=byrgn)
		{
			hrct += 2;
		}
		else
		{
			//DO NULL
		}
	}
	else if((byrct+2)<=byrgn)
	{
		hrct += 2;
	}
	else
	{
			//DO NULL
	}

	rct_dst->uStartX = lxrct;
	rct_dst->uStartY = tyrct;
	rct_dst->uWidth = wrct;
	rct_dst->uHeight = hrct;
}

static void ge3602_scale_rect(PGUI_RECT rct_src,PGUI_RECT rct_dst)
{
	UINT32 x,y,w,h,offset;
	UINT32 hdiv,hmul,vdiv,vmul;

	x = rct_src->uStartX;
	y = rct_src->uStartY;
	w = rct_src->uWidth;
	h = rct_src->uHeight;
	hdiv = g_gui_coeff.hdiv;
	vdiv = g_gui_coeff.vdiv;
	hmul = g_gui_coeff.hmul;
	vmul = g_gui_coeff.vmul;

	//adjust clip window	
	offset = (x+2) % hdiv;
	if (offset != 0)
	{
		if(x >= offset)
		{
			x -= offset;
			w += offset;
		}
		else
		{
			w += x;	
			x = 0; 
		}
	}
	if(g_rct_scaleclip.uStartX != 0xFFFF)
		g_rct_scaleclip.uStartX =  (x+2) * hmul /hdiv;

	//adjust operation window
	offset = x % hdiv;
	if (offset != 0)
	{
		if(x >= offset)
		{
			x -= offset;
			w += offset;
		}
		else
		{
			w += x;	
			x = 0; 
		}	
	}

	offset = (w+x-2) % hdiv;
	if (offset != 0)
	{
		w += hdiv - offset;
	}
	if(g_rct_scaleclip.uWidth != 0xFFFF)
		g_rct_scaleclip.uWidth = (w+x-2)* hmul /hdiv-1 - g_rct_scaleclip.uStartX;

	offset = w % hdiv;
	if (offset != 0)
	{
		w += hdiv - offset;
	}

	//adjust clip window	
	offset = (y+2) % vdiv;
	if (offset != 0)
	{
		if(y>= offset)
		{
			y -= offset;
			h += offset;
		}
		else
		{
			h+=y;	
			y = 0; 
		}
	}
	if(g_rct_scaleclip.uStartY != 0xFFFF)
		g_rct_scaleclip.uStartY = (y+2) * vmul /vdiv;

	//adjust operation window
	offset = y % vdiv;
	if (offset != 0)
	{
		if(y>= offset)
		{
			y -= offset;
			h += offset;
		}
		else
		{
			h+=y;		
			y = 0; 
		}
	}

	offset = (h+y-2) % vdiv;
	if (offset != 0)
	{
		h += vdiv - offset;
	}
	if(g_rct_scaleclip.uHeight != 0xFFFF)
		g_rct_scaleclip.uHeight = (h+y-2)* vmul /vdiv-1 - g_rct_scaleclip.uStartY;

	offset = h% vdiv;
	if (offset != 0)
	{
		h += vdiv - offset;
	}

	rct_dst->uStartX= x;
	rct_dst->uStartY = y;
	rct_dst->uWidth = w;
	rct_dst->uHeight = h;
}

static void ge3602_scale_rect_max(PGUI_RECT rct_dst, PGUI_RECT rct_src)
{
	UINT32 w,offset; 
	UINT32 hdiv,hmul,vdiv,vmul;

	w = rct_src->uWidth;
	hdiv = g_gui_coeff.hdiv;
	vdiv = g_gui_coeff.vdiv;
	hmul = g_gui_coeff.hmul;
	vmul = g_gui_coeff.vmul;
	
	offset = w % hdiv;
	w -= offset;
	rct_src->uWidth = w;
	rct_dst->uWidth = w * hmul / hdiv;
}
#endif

UINT32 app_gui_switch(UINT32 layer,GUI_TVSYS tvsys)
{
	UINT32 err;
	
	if(g_gui_tvsys == tvsys)
	{
		gui_printf("Tv system keeps same,need not switch");
		err = (UINT32)(-1);
	}
	else if(ge_version > GE_MODULE_M3329D)
	{
		err = gecmd_tvsys_scale(layer,tvsys);
	}
	else if(ge_version == GE_MODULE_M3602)
	{
		err = gui_gmasurf_switch(layer,tvsys);
	}
	else
	{
		gui_printf("Do not support GE_SCALE!");
		err = (UINT32)(-2);
	}
	
	return err;
}
/******************************************************************************************
*GUI primitive draw functions
******************************************************************************************/
RET_CODE gedraw_drawpixel(PGUI_REGION prgn,PGUI_POINT ppos,UINT32 param)
{
	UINT32 color;
	PGUI_PEN pen = (PGUI_PEN)param;

	if(pen->thick >= 2)
	{
		GUI_RECT dot;

		dot.uStartX = ppos->uX;
		dot.uStartY = ppos->uY;
		dot.uWidth = pen->thick;
		dot.uHeight = pen->thick;
		gedraw_fillrect(prgn,&dot,param);
	}
	else
	{
		struct ge_device * dev = (struct ge_device *)g_layer_tbl[prgn->layer_type].hdev;

		color = gui_colormask(prgn,pen->color);
		
		ge_lock(dev);
		ge_set_cur_region(dev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id);
		ge_draw_pixel(dev,(ge_surface_desc_t *)prgn->hsurf,(int)ppos->uX,(int)ppos->uY,color);
		ge_unlock(dev);
	}
	return RET_SUCCESS;
}

RET_CODE gedraw_drawline(PGUI_REGION prgn,PGUI_POINT pstart,PGUI_POINT pend,UINT32 param)
{
/*/////////////////////////////////////////////////////////////////////////////////////
	1. vertical line
	2. horizontal line
	3. diagonal line
	param is the linethick,here it is pixel width
*////////////////////////////////////////////////////////////////////////////////////////////////
	UINT32 color;
	PGUI_PEN pen = (PGUI_PEN)param;

	if((pstart->uX != pend->uX)&&(pstart->uY != pend->uY))
	{
		return gedraw_lineto(prgn,pstart,pend,param);
	}
	else if(pen->thick>=2)
	{
		GUI_RECT frm;

		color = gui_colormask(prgn,pen->color);

		frm.uStartX = pstart->uX;
		frm.uStartY = pstart->uY;
		frm.uWidth = (pstart->uX == pend->uX)? (pen->thick):(pend->uX - pstart->uX);
		frm.uHeight = (pstart->uY == pend->uY)? (pen->thick):(pend->uY - pstart->uY);
		return gedraw_fillrect(prgn,&frm,color);
	}

	if(pstart->uX == pend->uX)
	{
		struct ge_device * dev = (struct ge_device *)g_layer_tbl[prgn->layer_type].hdev;

		color = gui_colormask(prgn,pen->color);

		ge_lock(dev);
		ge_set_cur_region(dev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id);
		ge_draw_ver_line(dev,(ge_surface_desc_t *)prgn->hsurf,(int)pstart->uX,(int)pstart->uY, \
			(int)(pend->uY - pstart->uY), color);
		ge_unlock(dev);
	}
	else
	{
		struct ge_device * dev = (struct ge_device *)g_layer_tbl[prgn->layer_type].hdev;
		
		color = gui_colormask(prgn,pen->color);
		
		ge_lock(dev);
		ge_set_cur_region(dev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id);
		ge_draw_hor_line(dev,(ge_surface_desc_t *)prgn->hsurf,(int)pstart->uX,(int)pstart->uY, \
			(int)(pend->uX - pstart->uX),color);
		ge_unlock(dev);
	}
	
	return RET_SUCCESS;
};

RET_CODE gedraw_lineto(PGUI_REGION prgn,PGUI_POINT pstart,PGUI_POINT pend,UINT32 param)
{
	//TODO:reserved for future use.
	gui_printf("Do not support yet");
	return RET_SUCCESS;
}

RET_CODE gedraw_fillroundrect(PGUI_REGION prgn,PGUI_RECT pFrm,PGUI_RECT pCorner,UINT32 param)
{
	//reserved for future use
	gui_printf("Do not support yet");
	return RET_SUCCESS;
}

RET_CODE gedraw_fillrect(PGUI_REGION prgn,PGUI_RECT prect,UINT32 param)
{
	UINT32 color;
	RET_CODE ret;
	ge_rect_t frm;
	struct ge_device *dev;
	
	if(NULL == prect)
	{
		MEMSET(&frm,0,sizeof(ge_rect_t));
		frm.width = prgn->rect.uWidth;
		frm.height = prgn->rect.uHeight;		
	}
	else
	{
		frm.left = prect->uStartX;
		frm.top = prect->uStartY;
		frm.width = prect->uWidth;
		frm.height = prect->uHeight;
	}
	dev = (struct ge_device *)g_layer_tbl[prgn->layer_type].hdev;
	color = gui_colormask(prgn,param);
	
	ge_lock(dev);
	ge_set_cur_region(dev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id);
	ret = ge_fill_rect(dev,(ge_surface_desc_t *)prgn->hsurf,&frm,color);
	ge_unlock(dev);

	return ret;
}

RET_CODE gedraw_drawbmp(PGUI_REGION prgn,PGUI_RECT pfrm,UINT32 param)
{
	UINT32	op_flag;
	RET_CODE error_ret;
	PGUI_BMP 	pbmp;
	struct ge_device * dev;
	ge_surface_desc_t surf_bmp,*psurf;
	ge_region_t rgn_para;
	ge_rect_t dst_rect,src_rect;
	ge_region_pars_t rgn_cfg;
	
	op_flag = GE_BLT_SRC2_VALID;
	pbmp = (PGUI_BMP)param;
	src_rect.left = 0;
	src_rect.top = 0;
	dst_rect.left = pfrm->uStartX;
	dst_rect.top = pfrm->uStartY;
	dst_rect.width = src_rect.width =MIN(pfrm->uWidth,pbmp->bmp_size.uWidth);
	dst_rect.height = src_rect.height = MIN(pfrm->uHeight,pbmp->bmp_size.uHeight);
	MEMSET(&surf_bmp,0x0,sizeof(surf_bmp));
	surf_bmp.width = MIN(pfrm->uWidth,pbmp->bmp_size.uWidth);
	surf_bmp.height = MIN(pfrm->uHeight,pbmp->bmp_size.uHeight);
	surf_bmp.pitch = pbmp->stride; 
	surf_bmp.endian = GE_PIXEL_LITTLE_ENDIAN;
	surf_bmp.alpha_range = GE_LARGE_ALPHA;
	
	MEMSET(&rgn_para,0x0,sizeof(rgn_para));
	rgn_para.rect.left = 0;
	rgn_para.rect.top = 0;
	rgn_para.rect.width = surf_bmp.width;
	rgn_para.rect.height = surf_bmp.height;
	rgn_para.glob_alpha_en = 0;
	rgn_para.glob_alpha = 0x7f;
	rgn_para.pallet_seg = 0;
	rgn_para.pitch = pbmp->stride; 
	if(GUI_CLUT8 == pbmp->color_mode)
	{
		rgn_para.pixel_fmt = GE_PF_CLUT8;
	}
	else if(GUI_ARGB1555 == pbmp->color_mode)
	{
		rgn_para.pixel_fmt = GE_PF_ARGB1555;
		
		if(GUI_AYCBCR8888 == prgn->color_mode)
			op_flag = op_flag | GE_BLT_RGB2YUV;
	}
	else if(GUI_ARGB8888 == pbmp->color_mode)
	{
		rgn_para.pixel_fmt = GE_PF_ARGB8888;
		if(GUI_AYCBCR8888 == prgn->color_mode)
			op_flag = op_flag | GE_BLT_RGB2YUV;
	}    
	else
	{
		rgn_para.pixel_fmt = GE_PF_AYCBCR8888;
	}

	dev = (struct ge_device *)g_layer_tbl[prgn->layer_type].hdev;
	psurf = ge_create_surface(dev,&surf_bmp,0,GMA_MEM_SURFACE);
	ge_create_region(dev,psurf,0, &rgn_para.rect, &rgn_para);
	MEMSET(&rgn_cfg,0x0,sizeof(ge_region_pars_t));
	rgn_cfg.addr = pbmp->pdata;
	rgn_cfg.pitch = pbmp->stride;
	ge_set_region_pars(dev,psurf,0,&rgn_cfg);
	
	ge_lock(dev);
	ge_set_cur_region(dev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id);
	error_ret = ge_blt(dev,(ge_surface_desc_t *)prgn->hsurf,psurf,&dst_rect,&src_rect,op_flag);
	ge_unlock(dev);
	ge_release_surface(dev,psurf);

	return error_ret;
}

RET_CODE gedraw_renderbmp(PGUI_REGION prgn,PGUI_RECT pfrm,UINT32 param)
{
	UINT32	op_flag;
	PGUI_BMP 	pbmp;
	struct ge_device * dev;
	ge_rect_t dst_rct,src_rct;
	ge_color_key_t color_key;
	ge_region_pars_t rgn_cfg;
	ge_surface_desc_t surf_bmp,*psurf;
	ge_region_t rgn_para;
	RET_CODE err_ret;
	
	pbmp = (PGUI_BMP)param;
	src_rct.left = 0;
	src_rct.top = 0;
	dst_rct.left = pfrm->uStartX;
	dst_rct.top = pfrm->uStartY;
	dst_rct.width = src_rct.width = MIN(pfrm->uWidth,pbmp->bmp_size.uWidth);
	dst_rct.height = src_rct.height = MIN(pfrm->uHeight,pbmp->bmp_size.uHeight);
	MEMSET(&surf_bmp,0x0,sizeof(surf_bmp));
	surf_bmp.width = MIN(pfrm->uWidth,pbmp->bmp_size.uWidth);
	surf_bmp.height = MIN(pfrm->uHeight,pbmp->bmp_size.uHeight);
	surf_bmp.pitch = pbmp->stride; 
	surf_bmp.endian = GE_PIXEL_LITTLE_ENDIAN;
	surf_bmp.alpha_range = GE_LARGE_ALPHA;
	
	MEMSET(&rgn_para,0x0,sizeof(rgn_para));
	rgn_para.rect.left = 0;
	rgn_para.rect.top = 0;
	rgn_para.rect.width = surf_bmp.width;
	rgn_para.rect.height = surf_bmp.height;
	rgn_para.glob_alpha_en = 0;
	rgn_para.glob_alpha = 0x7f;
	rgn_para.pallet_seg = 0;
	rgn_para.pitch = pbmp->stride; 
	if(GUI_CLUT8 == pbmp->color_mode)
	{
		rgn_para.pixel_fmt = GE_PF_CLUT8;
	}
	else if(GUI_ARGB1555 == pbmp->color_mode)
	{
		rgn_para.pixel_fmt = GE_PF_ARGB1555;
	}
	else if(GUI_ARGB8888 == pbmp->color_mode)
	{
		rgn_para.pixel_fmt = GE_PF_ARGB8888;
	}    
	else
	{
		rgn_para.pixel_fmt = GE_PF_AYCBCR8888;
	}

	dev = (struct ge_device *)g_layer_tbl[prgn->layer_type].hdev;
	psurf = ge_create_surface(dev,&surf_bmp,0,GMA_MEM_SURFACE);
	ge_create_region(dev,psurf,0, &rgn_para.rect, &rgn_para);

	MEMSET(&rgn_cfg,0,sizeof(rgn_cfg));
	rgn_cfg.pitch = pbmp->stride;//pitch & adr is essential
	rgn_cfg.addr = (UINT8 *)pbmp->pdata;
	ge_set_region_pars(dev,psurf,0,&rgn_cfg);

	MEMSET(&color_key,0,sizeof(color_key));
	if(GUI_CLUT8 == pbmp->color_mode)
	{
		color_key.low_value = gui_colormask(prgn,pbmp->bg_color);
		color_key.high_value= gui_colormask(prgn,pbmp->bg_color);
		color_key.chn0_flags = GE_CKEY_ALWAYS_MATCH;
		color_key.chn1_flags = GE_CKEY_ALWAYS_MATCH;
		color_key.chn2_flags = GE_CKEY_IN_RANGE;
	}
	else if(GUI_ARGB1555 == pbmp->color_mode)
	{
		UINT32 r,g,b;
		
		r = (pbmp->bg_color>>10)&0x1f;
		g = (pbmp->bg_color>>5)&0x1f;
		b = pbmp->bg_color&0x1f;
		color_key.high_value = color_key.low_value = (((r<<3)+(r>>2))<<16)+
			(((g<<3)+(g>>2))<<8)+((b<<3)+(b>>2));		
		color_key.chn0_flags = GE_CKEY_IN_RANGE;
		color_key.chn1_flags = GE_CKEY_IN_RANGE;
		color_key.chn2_flags = GE_CKEY_IN_RANGE;
	}
	else
	{
		color_key.low_value = gui_colormask(prgn,pbmp->bg_color);
		color_key.high_value= gui_colormask(prgn,pbmp->bg_color);
		color_key.chn0_flags = GE_CKEY_IN_RANGE;
		color_key.chn1_flags = GE_CKEY_IN_RANGE;
		color_key.chn2_flags = GE_CKEY_IN_RANGE;
	}
	color_key.src_select = GE_CKEY_SOURCE;

	op_flag = GE_BLT_SRC1_VALID | GE_BLT_SRC2_VALID | GE_BLT_COLOR_KEY;
	if(((GUI_ARGB1555 == pbmp->color_mode) || (GUI_ARGB8888 == pbmp->color_mode))&&\
	        (GUI_AYCBCR8888 == prgn->color_mode))
		op_flag = op_flag | GE_BLT_RGB2YUV;
	
	ge_lock(dev);
	ge_set_cur_region(dev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id);
	ge_set_colorkey(dev,(ge_surface_desc_t *)prgn->hsurf,&color_key);
	err_ret = ge_blt_ex(dev,(ge_surface_desc_t *)prgn->hsurf,(ge_surface_desc_t *)prgn->hsurf, \
		psurf,&dst_rct,&dst_rct,&src_rct,op_flag);
	ge_unlock(dev);
	ge_release_surface(dev,psurf);
	
	return err_ret;
}

RET_CODE gedraw_drawfont(PGUI_REGION prgn,PGUI_RECT pfrm,UINT32 param)
{
	UINT32 bg_color,fg_color;
	RET_CODE ret;
	ge_rect_t frm;
	ge_bitmap_t fnt;
	PGUI_FONT	pmfnt;
	struct ge_device *dev;

	frm.left = pfrm->uStartX;
	frm.top = pfrm->uStartY;
	frm.width = pfrm->uWidth;
	frm.height = pfrm->uHeight;
	dev = (struct ge_device *)g_layer_tbl[prgn->layer_type].hdev;

	pmfnt = (PGUI_FONT)param;
	MEMSET(&fnt,0x0,sizeof(fnt));
	fnt.endian = 1;
	fnt.buf = pmfnt->pdata;
	fnt.bpp = 1;
	fnt.pitch = (INT32)pmfnt->stride;
	fnt.height = (INT32)pmfnt->fnt_size.uHeight;
	fnt.width = (INT32)pmfnt->fnt_size.uWidth;

	fg_color = gui_colormask(prgn,pmfnt->fg_color);
	bg_color = gui_colormask(prgn,pmfnt->bg_color);
	fnt.fg_color = fg_color;
	fnt.bk_color = bg_color;
	
	ge_lock(dev);
	ge_set_cur_region(dev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id);
	ret = ge_draw_bitmap(dev,(ge_surface_desc_t *)prgn->hsurf,&frm,&fnt);
	ge_unlock(dev);

	return ret;
}

RET_CODE gedraw_update2scrn(PGUI_VSCR pvscr,PGUI_RECT prect)
{
	if(pvscr->suspend > 0)
		return RET_FAILURE;//do not update to screen

	if(pvscr->dst_layer < GUI_MEMVSCR_SURF)
	{//dst region must be display surface,otherwise need not submit vscr
		UINT16	rctmask;
		UINT32	op_flag;
		RET_CODE ret;
		GUI_RECT	rctscale;
		PGUI_REGION	pdst,prgn;
		struct ge_device *dev;
		ge_rect_t	rct_dst,rct_src;

		prgn = gedraw_getdstrgn(pvscr->root_layer,pvscr->root_rgn);
		pdst = gedraw_getdstrgn(pvscr->dst_layer,pvscr->dst_rgn);

		if(NULL != gedraw_up2scrn_cb)
		{
			GUI_VSCR blend_vscr;

			MEMCPY(&blend_vscr,pvscr,sizeof(GUI_VSCR));
			MEMCPY(&blend_vscr.frm,prect,sizeof(GUI_RECT));
			gedraw_up2scrn_cb((UINT32)&blend_vscr);
		}

		op_flag = GE_BLT_SRC2_VALID;
		dev = (struct ge_device *)g_layer_tbl[pdst->layer_type].hdev;
		if(((GUI_ARGB1555 == pvscr->color_mode)|| (GUI_ARGB8888 == pvscr->color_mode)) && \
			(GUI_AYCBCR8888 == pdst->color_mode))
			op_flag = op_flag | GE_BLT_RGB2YUV;

		rct_src.left = rct_dst.left = prect->uStartX;
		rct_src.top = rct_dst.top = prect->uStartY;
		rct_src.width = rct_dst.width = prect->uWidth;
		rct_src.height = rct_dst.height = prect->uHeight;
		if((ge_version == GE_MODULE_M3602) && (pvscr->dst_layer < GUI_SLVGMA_SURF) \
			&& ((g_gui_coeff.hdiv + g_gui_coeff.vdiv)>3))
		{
			rctmask = (UINT16)(~0);

			//SZMK
			MEMCPY(&rctscale,prect,sizeof(GUI_RECT));
			ge3602_expand_rect(prgn,&rctscale);
			ge3602_scale_rect(&rctscale,&rctscale);//adjust rect for GE scale fileter & max size
			rctscale.uWidth = ((rctscale.uStartX + rctscale.uWidth)>((g_gui_msize>>16)&rctmask)) \
				? ((g_gui_msize>>16)&rctmask -rctscale.uStartX):(rctscale.uWidth);
			rctscale.uHeight = ((rctscale.uStartY + rctscale.uHeight)>(g_gui_msize&rctmask)) \
				? ((g_gui_msize&rctmask) -rctscale.uStartY):(rctscale.uHeight);
				
			rct_src.left = rct_dst.left = rctscale.uStartX;
			rct_src.top = rct_dst.top = rctscale.uStartY;
			rct_src.width = rct_dst.width = rctscale.uWidth;
			rct_src.height = rct_dst.height = rctscale.uHeight;
			
			rct_dst.left = GUI_HSCALE(rct_dst.left);
			rct_dst.width = GUI_HSCALE(rct_dst.width);
			rct_dst.top = GUI_VSCALE(rct_dst.top);
			rct_dst.height = GUI_VSCALE(rct_dst.height);//scale dst
			op_flag = op_flag | GE_BLT_RESIZE;//non-directly DMA,need scale
		}

		ge_lock(dev);
   #ifdef AD_SHARE_UI_OSD_LAYER
        extern INT32 ad_pic_memsurf_cpy(ge_surface_desc_t *dest);
        ad_pic_memsurf_cpy((ge_surface_desc_t *)prgn->hsurf);
   #endif
		ge_set_cur_region(dev,(ge_surface_desc_t *)pdst->hsurf,pdst->rgn_id);
		ret = ge_blt(dev,(ge_surface_desc_t *)pdst->hsurf,(ge_surface_desc_t *)prgn->hsurf, \
			&rct_dst,&rct_src,op_flag);
		ge_unlock(dev);
		
		if(NULL != gedraw_up2scrn_cb)
			gedraw_clearclip(pdst);
		
		return ret;
	}
	else
	{
		return RET_FAILURE;
	}
}

RET_CODE gedraw_retrievescrn(PGUI_VSCR pvscr,PGUI_RECT prect_r)
{
	RET_CODE ret;

	ret = RET_FAILURE;
	if(((pvscr->dst_layer) < GUI_MEMVSCR_SURF) && (NULL != pvscr->lpbuff))
	{
		UINT32 op_flag;
		PGUI_REGION pdst,prgn;
		ge_rect_t 	rct_dst,rct_src;
		struct ge_device *dev;

		if(((pvscr->dst_layer)<GUI_SLVGMA_SURF)&&(ge_version == GE_MODULE_M3602))
			return ret;

		pdst = gedraw_getdstrgn(pvscr->dst_layer,pvscr->dst_rgn);
		prgn = gedraw_getdstrgn(pvscr->root_layer,pvscr->root_rgn);
		dev = (struct ge_device *)g_layer_tbl[pdst->layer_type].hdev;
		
		rct_src.left = prect_r->uStartX;
		rct_src.top = prect_r->uStartY;
		rct_dst.left = prect_r->uStartX;
		rct_dst.top = prect_r->uStartY;
		rct_src.width = rct_dst.width = prect_r->uWidth;
		rct_src.height = rct_dst.height = prect_r->uHeight;

		op_flag = GE_BLT_SRC2_VALID;
		if(((GUI_ARGB1555 == pvscr->color_mode) || (GUI_ARGB8888 == pvscr->color_mode)) && \
	            (GUI_AYCBCR8888 == pdst->color_mode))
			op_flag = op_flag | GE_BLT_YUV2RGB;
		
		ge_lock(dev);
		ge_set_cur_region(dev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id);
		ret = ge_blt(dev,(ge_surface_desc_t *)prgn->hsurf,(ge_surface_desc_t *)pdst->hsurf, \
			&rct_dst,&rct_src,op_flag);
		ge_unlock(dev);
	}

	return ret;
}

void gedraw_backup_restore(PGUI_VSCR lpVscr,UINT8 *buf,PGUI_RECT pRc,BOOL backup_flag)
{
	PGUI_REGION prgn;
	struct ge_device *dev;
	ge_rect_t 	rc1,rc2;
	ge_region_pars_t cfg;
	ge_region_t rgn_para;
	ge_surface_desc_t *psurf,surf_vscr;
	
	psurf = NULL;
	dev = (struct ge_device *)g_layer_tbl[lpVscr->dst_layer].hdev;
	MEMSET(&surf_vscr,0x0,sizeof(surf_vscr));
	surf_vscr.width = pRc->uWidth;
	surf_vscr.height = pRc->uHeight;
	surf_vscr.endian = GE_PIXEL_LITTLE_ENDIAN;
	surf_vscr.alpha_range = GE_LITTLE_ALPHA;
	
	MEMSET(&rgn_para,0x0,sizeof(rgn_para));
	rgn_para.rect.left = 0;
	rgn_para.rect.top = 0;
	rgn_para.rect.width = surf_vscr.width;
	rgn_para.rect.height = surf_vscr.height;
	rgn_para.glob_alpha_en = 0;
	rgn_para.glob_alpha = 0x7f;
	rgn_para.pallet_seg = 0;
	if(GUI_CLUT8 == lpVscr->color_mode)
	{
		rgn_para.pixel_fmt = GE_PF_CLUT8;
	}
	else if(GUI_ARGB1555 == lpVscr->color_mode)
	{
		rgn_para.pixel_fmt = GE_PF_ARGB1555;
	}
	else if(GUI_ARGB8888 == lpVscr->color_mode)
	{
		rgn_para.pixel_fmt = GE_PF_ARGB8888;
	}    
	else     
	{
		rgn_para.pixel_fmt = GE_PF_AYCBCR8888;
	}
	surf_vscr.pitch = rgn_para.pitch = pRc->uWidth << bypp_const[lpVscr->color_mode];
	psurf = ge_create_surface(dev,&surf_vscr,0,GMA_MEM_SURFACE);
	ge_create_region(dev,psurf,0, &rgn_para.rect, &rgn_para);

	MEMSET(&cfg,0,sizeof(cfg));
	cfg.addr = buf;
	cfg.pitch = rgn_para.pitch;
	ge_set_region_pars(dev,psurf,0,&cfg);//call interface function to retrieve struture member dataa

	rc2.left = 0;
	rc2.top = 0;
	rc1.left = pRc->uStartX;
	rc1.top = pRc->uStartY;
	rc1.width = rc2.width = pRc->uWidth;
	rc1.height = rc2.height = pRc->uHeight;
	prgn = gedraw_getdstrgn(lpVscr->dst_layer/*GUI_MEMVSCR_SURF*/,0);
	
	if(backup_flag)
	{
		ge_lock(dev);
		ge_blt(dev,psurf,(ge_surface_desc_t *)prgn->hsurf, \
			&rc2,&rc1,GE_BLT_SRC2_VALID);
		ge_unlock(dev);
	}
	else
	{
		PGUI_REGION pdst;

		ge_lock(dev);
		ge_set_cur_region(dev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id);
    #ifdef AD_SHARE_UI_OSD_LAYER
        INT32 ad_pic_memsurf_clip_cpy(ge_surface_desc_t *dest, ge_rect_t *rect);
        ad_pic_memsurf_clip_cpy(psurf,&rc1);
    #endif
		ge_blt(dev,(ge_surface_desc_t *)prgn->hsurf,psurf,\
			&rc1,&rc2,GE_BLT_SRC2_VALID);
		ge_unlock(dev);
        
		pdst = gedraw_getdstrgn(GUI_MEMVSCR_SURF,0);
		ge_lock(dev);
		ge_set_cur_region(dev,(ge_surface_desc_t *)pdst->hsurf,pdst->rgn_id);
		ge_blt(dev,(ge_surface_desc_t *)pdst->hsurf,psurf,\
			&rc1,&rc2,GE_BLT_SRC2_VALID);
		ge_unlock(dev);//keep sync
	}
	
	if(prgn->layer_type == GUI_SLVGMA_SURF)
	{
		ge_release_surface_deo(dev,psurf);
	}
	else
	{
		ge_release_surface(dev,psurf);
	}
}

/*##########################################################################*/

RET_CODE gedraw_setclip(PGUI_REGION prgn,PGUI_RECT prect,UINT32 param)
{
	RET_CODE ret;

	ret = RET_FAILURE;
	if((NULL != prgn) && (NULL != (void *)prgn->hsurf))
	{
		ge_clip_t clip;
		struct ge_device * dev;

		clip.left = prect->uStartX;
		clip.top = prect->uStartY;
		clip.right = prect->uStartX + prect->uWidth;
		clip.bottom = prect->uStartY + prect->uHeight;
		clip.clip_inside = (BOOL)param;
		dev = (struct ge_device *)g_layer_tbl[prgn->layer_type].hdev;
		
		ge_lock(dev);
		if(prgn->layer_type == GUI_SLVGMA_SURF)
		{
			ret = ge_set_clip(dev,(ge_surface_desc_t *)prgn->hsurf,&clip);
		}
		else
		{
			ge_set_cur_region(dev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id);
			ret = ge_set_clip(dev,(ge_surface_desc_t *)prgn->hsurf,&clip);
		}
		ge_unlock(dev);
	}

	return ret;
}

RET_CODE gedraw_clearclip(PGUI_REGION prgn)
{
	RET_CODE ret;

	ret = RET_FAILURE;
	if((NULL != prgn) && (NULL != (void *)prgn->hsurf))
	{
		struct ge_device * dev;
		
		dev = (struct ge_device *)g_layer_tbl[prgn->layer_type].hdev;
		
		ge_lock(dev);
		if(prgn->layer_type == GUI_SLVGMA_SURF)
		{
			ret = ge_disable_clip_deo(dev,(ge_surface_desc_t *)prgn->hsurf);
		}
		else
		{
			ge_set_cur_region(dev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id);
			ret = ge_disable_clip(dev,(ge_surface_desc_t *)prgn->hsurf);
		}
		ge_unlock(dev);
	}

	return ret;
}

RET_CODE gedraw_colorkey_rgn(PGUI_REGION pdst,PGUI_REGION prgn1,PGUI_REGION prgn2,PGUI_RECT prct1,PGUI_RECT prct2,UINT32 param)
{
	UINT32 op_flag;
	RET_CODE ret;
	GUI_RECT cross_rct;
	PGUI_RECT psrc1_rct,psrc2_rct;
	ge_rect_t op_rct;
	ge_color_key_t *pcfg;
	struct ge_device *dev;

	pcfg = (ge_color_key_t *)param;
	dev = (struct ge_device *)g_layer_tbl[pdst->layer_type].hdev;

	if(NULL == prct1)
		psrc1_rct = &(prgn1->rect);
	else
		psrc1_rct = prct1;
	if(NULL == prct2)
		psrc2_rct = &(prgn2->rect);
	else
		psrc2_rct = prct2;
	MEMSET(&cross_rct,0x0,sizeof(cross_rct));
	OSD_GetRectsCross(psrc1_rct,psrc2_rct,&cross_rct);
	if((cross_rct.uHeight == 0) || (cross_rct.uWidth == 0))
		return RET_FAILURE;//no overlap rect to do colorkey
	
	op_rct.left = cross_rct.uStartX;
	op_rct.top = cross_rct.uStartY;
	op_rct.width = cross_rct.uWidth;
	op_rct.height = cross_rct.uHeight;
	op_flag = GE_BLT_SRC1_VALID | GE_BLT_SRC2_VALID | GE_BLT_COLOR_KEY;

	ge_lock(dev);
	ge_set_cur_region(dev,(ge_surface_desc_t *)pdst->hsurf,pdst->rgn_id);
	ge_set_colorkey(dev,(ge_surface_desc_t *)pdst->hsurf,pcfg);
	ret = ge_blt_ex(dev,(ge_surface_desc_t *)pdst->hsurf,(ge_surface_desc_t *)prgn1->hsurf, \
		(ge_surface_desc_t *)prgn2->hsurf,&op_rct,&op_rct,&op_rct,op_flag);
	ge_unlock(dev);
	
	return ret;
}

RET_CODE gedraw_blend_rgn(PGUI_REGION pdst,PGUI_REGION prgn1,PGUI_REGION prgn2,PGUI_RECT prct1,PGUI_RECT prct2,UINT32 param)
{
	RET_CODE ret;

	ret = RET_FAILURE;
	if((GUI_CLUT8 == prgn1->color_mode)&&(GUI_CLUT8 == prgn2->color_mode))
	{
		ge_color_key_t colorkey_cfg;
		
		//TODO:CLUT8 format do not support alpha blend,so just colorkey it.
		MEMSET(&colorkey_cfg,0,sizeof(colorkey_cfg));
		colorkey_cfg.src_select = GE_CKEY_SOURCE;
		colorkey_cfg.chn0_flags = GE_CKEY_ALWAYS_MATCH;
		colorkey_cfg.chn1_flags = GE_CKEY_ALWAYS_MATCH;
		colorkey_cfg.chn2_flags = GE_CKEY_IN_RANGE;
		colorkey_cfg.high_value = gui_keycolor[GUI_CLUT8];
		colorkey_cfg.low_value = gui_keycolor[GUI_CLUT8];
		
		ret =  gedraw_colorkey_rgn(pdst,prgn1,prgn2,prct1,prct2,(UINT32)&colorkey_cfg);
	}
	else if(prgn1->color_mode != prgn2->color_mode)
	{
		//region format mismatch,can not blending
		gui_printf("Illegel operations,region format mismatch");
	}
	else
	{
		UINT32 op_flag;
		GUI_RECT cross_rct;
		PGUI_RECT psrc1_rct,psrc2_rct;
		ge_rect_t op_rct;
		struct ge_device *dev;
		ge_alpha_blend_t *pcfg;

		pcfg = (ge_alpha_blend_t *)param;
		dev = (struct ge_device *)g_layer_tbl[0].hdev;
		
		if(NULL == prct1)
			psrc1_rct = &(prgn1->rect);
		else
			psrc1_rct = prct1;
		if(NULL == prct2)
			psrc2_rct = &(prgn2->rect);
		else
			psrc2_rct = prct2;
		OSD_GetRectsCross(psrc1_rct,psrc2_rct,&cross_rct);
		if((cross_rct.uHeight == 0) || (cross_rct.uWidth == 0))
			return RET_FAILURE;//no overlapped rect to blending

		op_rct.left = cross_rct.uStartX;
		op_rct.top = cross_rct.uStartY;
		op_rct.width = cross_rct.uWidth;
		op_rct.height = cross_rct.uHeight;
		op_flag = GE_BLT_SRC1_VALID | GE_BLT_SRC2_VALID | GE_BLT_ALPHA_BLEND;
		
		ge_lock(dev);
		ge_set_cur_region(dev,(ge_surface_desc_t *)pdst->hsurf,pdst->rgn_id);
		ge_set_alpha(dev,(ge_surface_desc_t *)prgn1->hsurf,pcfg);
		ret = ge_blt_ex(dev,(ge_surface_desc_t *)pdst->hsurf,(ge_surface_desc_t *)prgn1->hsurf, \
			(ge_surface_desc_t *)prgn2->hsurf,&op_rct,&op_rct,&op_rct,op_flag);
		ge_unlock(dev);
	}

	return ret;
}

RET_CODE gedraw_flush_rgn(PGUI_REGION pdst,PGUI_REGION psrc,PGUI_RECT pdst_rct,PGUI_RECT psrc_rct,UINT32 param)
{
	UINT16 ww,hh;
	UINT32 op_flag;
	RET_CODE ret;
	GUI_RECT overlap_rct;
	ge_rect_t dst_rct,src_rct;
	struct ge_device *dev;

	if(NULL == pdst_rct)
		pdst_rct = &(pdst->rect);
	if(NULL == psrc_rct)
		psrc_rct = &(psrc->rect);

	if(ge_version == GE_MODULE_M3602)
	{//with scale functions
		if(pdst->layer_type < GUI_SLVGMA_SURF)
		{
			ww = GUI_HREVERT(pdst_rct->uWidth);
			hh = GUI_VREVERT(pdst_rct->uHeight);
		}
		else
		{
			ww = pdst_rct->uWidth;
			hh = pdst_rct->uHeight;
		}
		overlap_rct.uWidth = MIN(ww,psrc_rct->uWidth);
		overlap_rct.uHeight = MIN(hh,psrc_rct->uHeight);
	}
	else
	{//1:1 copy
		overlap_rct.uWidth = MIN(pdst_rct->uWidth,psrc_rct->uWidth);
		overlap_rct.uHeight = MIN(pdst_rct->uHeight,psrc_rct->uHeight);
	}

	ret = RET_FAILURE;
	if((0 != overlap_rct.uHeight) ||(0 != overlap_rct.uWidth))
	{
		if((NULL == (void*)pdst->hsurf)||(NULL == (void*)psrc->hsurf))
			return RET_FAILURE;
		
		op_flag = GE_BLT_SRC2_VALID;
		dev = (struct ge_device *)g_layer_tbl[pdst->layer_type].hdev;
		dst_rct.left = pdst_rct->uStartX;
		dst_rct.top = pdst_rct->uStartY;
		src_rct.left = psrc_rct->uStartX;
		src_rct.top = psrc_rct->uStartY;
		
		if((ge_version == GE_MODULE_M3602)&&(pdst->layer_type < GUI_SLVGMA_SURF))
		{
			dst_rct.width = pdst_rct->uWidth;
			dst_rct.height = pdst_rct->uHeight;
			src_rct.width = overlap_rct.uWidth;
			src_rct.height = overlap_rct.uHeight;

			if(dst_rct.height != src_rct.height )
				op_flag |= GE_BLT_RESIZE;
		}
		else
		{
			dst_rct.width = src_rct.width = overlap_rct.uWidth;
			dst_rct.height = src_rct.height = overlap_rct.uHeight;
		}

		ge_lock(dev);
		ge_set_cur_region(dev,(ge_surface_desc_t *)pdst->hsurf,pdst->rgn_id);
		ret = ge_blt(dev,(ge_surface_desc_t *)pdst->hsurf,(ge_surface_desc_t *)psrc->hsurf, \
			&dst_rct,&src_rct,op_flag);
		ge_unlock(dev);
	}

	return ret;
}

RET_CODE gedraw_setrgnpos(PGUI_REGION prgn,PGUI_RECT prect)
{
	RET_CODE ret;

	ret = RET_FAILURE;
	if((NULL != prgn) && (NULL != (void *)prgn->hsurf) && (gui_rctvalid(prect)))
	{
		ge_rect_t rect;
		struct ge_device * dev = (struct ge_device *)g_layer_tbl[prgn->layer_type].hdev;

		rect.left = prect->uStartX;
		rect.top = prect->uStartY;
		rect.width = prect->uWidth;
		rect.height = prect->uHeight;
		
		if(prgn->layer_type == GUI_SLVGMA_SURF)
			ret = ge_set_region_pos_deo(dev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id,&rect);
		else
			ret = ge_set_region_pos(dev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id,&rect);
	}
	
	return ret;
}

RET_CODE gedraw_getrgnpos(PGUI_REGION prgn,PGUI_RECT prect)
{
	RET_CODE ret;

	ret = RET_FAILURE;
	if((NULL != prgn) && (NULL != (void *)prgn->hsurf))
	{
		ge_rect_t rect;
		struct ge_device * dev = (struct ge_device *)gui_dev;

		MEMSET(&rect,0,sizeof(ge_rect_t));
		if(prgn->layer_type == GUI_SLVGMA_SURF)
			ret = (ge_get_region_pos_deo(dev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id,&rect));
		else
			ret = (ge_get_region_pos(dev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id,&rect));
		
		prect->uStartX = rect.left;
		prect->uStartY = rect.top;
		prect->uWidth = rect.width;
		prect->uHeight = rect.height;
	}
	
	return ret;
}

RET_CODE gedraw_modifypallette(PGUI_REGION prgn,UINT8 idx,UINT8 *pallette)
{
	if((NULL != prgn) && (NULL != (void *)prgn->hsurf) && (GUI_CLUT8 == prgn->color_mode))
	{
		UINT32 alpha;
		struct ge_device * dev = (struct ge_device *)g_layer_tbl[prgn->layer_type].hdev;

		alpha = pallette[3];
		if(prgn->layer_type == GUI_SLVGMA_SURF)
			return ge_modify_pallette_deo(dev,(ge_surface_desc_t *)prgn->hsurf,idx,pallette[0],pallette[1],pallette[2],alpha,GE_PALLET_ALPHA_16);
		else
			return ge_modify_pallette(dev,(ge_surface_desc_t *)prgn->hsurf,idx,pallette[0],pallette[1],pallette[2],alpha,GE_PALLET_ALPHA_16);
	}
	else
	{
		return RET_FAILURE;
	}
}

static UINT8 gelib_sdpallette[4*256];
RET_CODE gedraw_setpallette(PGUI_REGION prgn,UINT8 *pallette)
{
	if((NULL != prgn) && (NULL != (void *)prgn->hsurf) && (GUI_CLUT8 == prgn->color_mode))
	{
		struct ge_device * dev = (struct ge_device *)g_layer_tbl[prgn->layer_type].hdev;
		
		if(prgn->layer_type == GUI_SLVGMA_SURF)
		{
			MEMCPY(gelib_sdpallette,pallette,1024*sizeof(UINT8));
			return ge_set_pallette_deo((struct ge_device *)dev,(ge_surface_desc_t *)prgn->hsurf,gelib_sdpallette,(UINT16)256,OSDDRV_YCBCR,GE_PALLET_ALPHA_16);
		}
		else
			return ge_set_pallette((struct ge_device *)dev,(ge_surface_desc_t *)prgn->hsurf,pallette,(UINT16)256,OSDDRV_YCBCR,GE_PALLET_ALPHA_16);
	}
	else
	{
		return RET_FAILURE;
	}
}

RET_CODE gedraw_getpallette(PGUI_REGION prgn,UINT8 *pallette)
{
	if((NULL != prgn) && (NULL != (void *)prgn->hsurf) && (GUI_CLUT8 == prgn->color_mode))
	{
		struct ge_device * dev = (struct ge_device *)g_layer_tbl[prgn->layer_type].hdev;

		if(prgn->layer_type == GUI_SLVGMA_SURF)
			return ge_get_pallette_deo((struct ge_device *) dev,(ge_surface_desc_t *)prgn->hsurf,pallette,(UINT16)COLOR_N,(UINT8)OSDDRV_YCBCR);
		else
			return ge_get_pallette((struct ge_device *) dev,(ge_surface_desc_t *)prgn->hsurf,pallette,(UINT16)COLOR_N,(UINT8)OSDDRV_YCBCR);
	}
	else
	{
		return RET_FAILURE;
	};
}

RET_CODE gedraw_setgalpha(UINT32 layer,UINT32 gtrans)
{
	ge_global_alpha_t galpha_setting;
	
	if((layer == GUI_GMA2_SURF) && (GE_MODULE_M3602 == ge_version))
		return ge_io_ctrl((struct ge_device *)gui_dev,GE_IO_SET_LAYER2_GLOBAL_ALPHA,(UINT32)gtrans);
	else if(layer < GUI_MEMVSCR_SURF)
	{
		MEMSET(&galpha_setting,0x0,sizeof(galpha_setting));
		galpha_setting.layer = layer;
		galpha_setting.valid = 1;
		galpha_setting.value = gtrans;
		
		return ge_io_ctrl((struct ge_device *)gui_dev,GE_IO_GLOBAL_ALPHA_29E,(UINT32)&galpha_setting);
	}
	else
		return RET_FAILURE;
}

RET_CODE gedraw_scale(PGUI_REGION prgn,UINT32 cmd,UINT32 param)
{
	struct ge_device *dev = (struct ge_device *)g_layer_tbl[prgn->layer_type].hdev;

	if(NULL != dev)
	{
		if(prgn->layer_type == GUI_SLVGMA_SURF)
			return ge_io_ctrl_deo(dev,cmd,param);
		else
			return ge_io_ctrl(dev,cmd,param);
	}
	else
		return RET_FAILURE;
}

RET_CODE gedraw_rgn_showonoff(PGUI_REGION prgn,UINT8 on_off)
{		
	if((NULL != prgn) && (NULL != (void *)prgn->hsurf)&&(prgn->layer_type <= GUI_SLVGMA_SURF))
	{		
		struct ge_device * dev = (struct ge_device *)g_layer_tbl[prgn->layer_type].hdev;

		if(prgn->layer_type== GUI_SLVGMA_SURF)
			return ge_region_show_onoff_deo(dev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id,on_off);
		else
			return ge_region_show_onoff(dev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id,on_off);
	}
	else
	{
		return RET_FAILURE;
	}
}

RET_CODE gedraw_surf_showonoff(PGUI_LAYER psurf,UINT8 on_off)
{
	if((NULL != psurf) && (NULL != (void *)psurf->hdev) && (NULL != (void *)psurf->hsurf) && (psurf->surf_mode <= GUI_SLVGMA_SURF))
	{
		if(psurf->surf_mode == GUI_SLVGMA_SURF)
			return ge_show_onoff_deo((struct ge_device *)psurf->hdev,(ge_surface_desc_t *)psurf->hsurf,on_off);
		else
			return ge_show_onoff((struct ge_device *)psurf->hdev,(ge_surface_desc_t *)psurf->hsurf,on_off);
	}
	else
	{
		return RET_FAILURE;
	}
}

RET_CODE gedraw_deletergn(PGUI_REGION prgn)
{
	RET_CODE ret;

	ret = RET_FAILURE;
	if((NULL != prgn) && (NULL != (void *)prgn->hsurf))
	{
		if(prgn->layer_type == GUI_SLVGMA_SURF)
			ret = ge_delete_region_deo((struct ge_device *)g_layer_tbl[prgn->layer_type].hdev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id);
		else
			ret = ge_delete_region((struct ge_device *)g_layer_tbl[prgn->layer_type].hdev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id);

		//MEMSET(prgn,0x0,sizeof(GUI_REGION));
		prgn->hsurf = (SURF_HANDLE)NULL;
	}

	return ret;
}

RET_CODE gedraw_creatergn(UINT32 param)
{
	UINT8 idx,rgn_idx,*p_pallette;
	RET_CODE ret_error;
	PGUI_REGION prgn;
	ge_region_t rgn_para;

	prgn = (PGUI_REGION)param;
	idx = prgn->layer_type;
	MEMSET(&rgn_para,0x0,sizeof(rgn_para));
	rgn_para.rect.left = prgn->rect.uStartX;
	rgn_para.rect.top = prgn->rect.uStartY;
	rgn_para.rect.width = prgn->rect.uWidth;
	rgn_para.rect.height = prgn->rect.uHeight;
	rgn_para.pitch = prgn->rect.uWidth<<bypp_const[prgn->color_mode]; 
	rgn_para.glob_alpha_en = 0;
	rgn_para.glob_alpha = 0x7f;
	rgn_para.pallet_seg = 0;

	if(GUI_CLUT8 == prgn->color_mode)
	{
		rgn_para.pixel_fmt = GE_PF_CLUT8;
	}
	else if(GUI_ARGB1555 == prgn->color_mode)
	{
		rgn_para.pixel_fmt = GE_PF_ARGB1555;
	}
	else if(GUI_ARGB8888 == prgn->color_mode)
	{
		rgn_para.pixel_fmt = GE_PF_ARGB8888;
	}
	else
	{
		rgn_para.pixel_fmt = GE_PF_AYCBCR8888;
	}

	g_layer_tbl[idx].rgn_onlayer++;
	if(prgn->layer_type == GUI_SLVGMA_SURF)
	{
		prgn->color_mode = GUI_CLUT8;
		rgn_para.pixel_fmt = GE_PF_CLUT8;
		ret_error = ge_create_region_deo((struct ge_device *)g_layer_tbl[idx].hdev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id, &rgn_para.rect, &rgn_para);
	}
	else
	{
		ret_error = ge_create_region((struct ge_device *)g_layer_tbl[idx].hdev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id, &rgn_para.rect, &rgn_para);
	}

	if((prgn->color_mode == GUI_CLUT8) && (prgn->layer_type <= GUI_SLVGMA_SURF))
	{
		p_pallette = OSD_GetRscPallette(0x4080 | 0);
		gedraw_setpallette(prgn,p_pallette);
	}
	
	if(prgn->layer_type <= GUI_SLVGMA_SURF)
	{
		gedraw_fillrect(prgn,NULL,transcolor_const[prgn->color_mode]);
	}
	
	return ret_error;
}

RET_CODE gedraw_createsurf(UINT32 param)
{
	UINT8 gam_layer;
	ge_surface_desc_t  surf_para;
	PGUI_LAYER psurf = (PGUI_LAYER)param;

	MEMSET(&surf_para,0x0,sizeof(surf_para));
	surf_para.width = psurf->rect.uWidth;
	surf_para.height = psurf->rect.uHeight; 
	surf_para.pitch = surf_para.width <<  bypp_const[psurf->color_mode];
	surf_para.endian = GE_PIXEL_LITTLE_ENDIAN;
	surf_para.alpha_range = GE_LARGE_ALPHA;

	if(GUI_SLVGMA_SURF == psurf->surf_mode)
		gam_layer = GMA_HW_SURFACE_DEO;
	else if(GUI_SLVGMA_SURF > psurf->surf_mode)
		gam_layer = psurf->surf_mode;
	else
		gam_layer = GMA_MEM_SURFACE;

	if(gam_layer == GMA_HW_SURFACE_DEO)
	{
		psurf->hsurf = (SURF_HANDLE)ge_create_surface_deo((struct ge_device *)psurf->hdev,&surf_para,0,gam_layer);
	}
	else
	{
		psurf->hsurf = (SURF_HANDLE)ge_create_surface((struct ge_device *)psurf->hdev,&surf_para,0,gam_layer);
	}
	
	if(NULL == (void *)psurf->hsurf)
		return RET_FAILURE;
	else
		return RET_SUCCESS;
}

RET_CODE gedraw_releasesurf(PGUI_LAYER psurf)
{
	RET_CODE ret;

	ret = RET_FAILURE;
	if((NULL != psurf) && (NULL != (void *)psurf->hdev) && (NULL != (void *)psurf->hsurf))
	{
		if(psurf->surf_mode == GUI_SLVGMA_SURF)
			ret = ge_release_surface_deo((struct ge_device *)psurf->hdev,(ge_surface_desc_t *)psurf->hsurf);
		else
			ret = ge_release_surface((struct ge_device *)psurf->hdev,(ge_surface_desc_t *)psurf->hsurf);

		psurf->hsurf = (SURF_HANDLE)NULL;
	}

	return ret;
}

#ifdef GELIB_M3602_DUALOUT
RET_CODE app_dualoutput_init(PGUI_RGN_CFG prgnlst,UINT32 param)
{
	UINT8 i,vsr_enable;
	PGUI_LAYER psurf;
	PGUI_REGION prgn;
	ge_region_pars_t cfg;

	vsr_enable = param&0xff;
	if(ge_version!=GE_MODULE_M3602)
		return (-1);
	
	g_gelib_m3602_dualoutput = 1;
	psurf = gedraw_getdstlayer(GUI_SLVGMA_SURF);
	psurf->hdev = gui_dev;
	psurf->hsurf = (SURF_HANDLE)NULL;
	psurf->surf_mode = GUI_SLVGMA_SURF;
	psurf->rgn_onlayer = 0;
	psurf->color_mode = GUI_CLUT8;//only support clut8
	MEMCPY(&(psurf->rect),&(prgnlst->rect),sizeof(GUI_RECT));				
	gedraw_createsurf((UINT32)psurf);
	gedraw_surf_showonoff(psurf,1);//set layer as show
	
	prgnlst->layer_idx = GUI_SLVGMA_SURF;//frce into DEO layer
	prgn = gedraw_getdstrgn(prgnlst->layer_idx,prgnlst->rgn_idx);
	if((psurf->hsurf) != (SURF_HANDLE)NULL)
	{
		prgn->layer_type = prgnlst->layer_idx;
		prgn->rgn_id = prgnlst->rgn_idx;
		prgn->hsurf = psurf->hsurf;
		prgn->color_mode = GUI_CLUT8;
		MEMCPY(&(prgn->rect),&(prgnlst->rect),sizeof(GUI_RECT));
		
		gedraw_creatergn((UINT32)prgn);
		gedraw_rgn_showonoff(prgn,1);
		psurf->rgn_onlayer++;
	}
	
	if(vsr_enable)
	{
		psurf = gedraw_getdstlayer(GUI_SLVSCR_SURF);
		psurf->hdev = gui_dev;
		psurf->hsurf = (SURF_HANDLE)NULL;
		psurf->surf_mode = GUI_SLVSCR_SURF;
		psurf->rgn_onlayer = 0;
		psurf->color_mode = GUI_CLUT8;//only support clut8
		MEMCPY(&(psurf->rect),&(prgnlst->rect),sizeof(GUI_RECT));				
		gedraw_createsurf((UINT32)psurf);

		prgn = gedraw_getdstrgn(GUI_SLVSCR_SURF,0);
		if((psurf->hsurf) != (SURF_HANDLE)NULL)
		{
			prgn->layer_type = GUI_SLVSCR_SURF;
			prgn->rgn_id = 0;
			prgn->hsurf = psurf->hsurf;
			prgn->color_mode = GUI_CLUT8;
			MEMCPY(&(prgn->rect),&(prgnlst->rect),sizeof(GUI_RECT));
			
			gedraw_creatergn((UINT32)prgn);
			psurf->rgn_onlayer++;
		}

		slvscr_buff = MALLOC((prgn->rect.uWidth)*(prgn->rect.uHeight));
		prgn = gedraw_getdstrgn(GUI_SLVSCR_SURF,0);
		if(NULL != (void *)prgn->hsurf)
		{
			MEMSET(&cfg,0x0,sizeof(ge_region_pars_t));
			cfg.addr = slvscr_buff;
			cfg.pitch = (prgn->rect.uWidth);
			ge_set_region_pars((struct ge_device *)gui_dev,(ge_surface_desc_t *)prgn->hsurf,0,&cfg);
		
			if(NULL != slvscr_buff)
				gedraw_fillrect(prgn,NULL,transcolor_const[prgn->color_mode]);
		}

		for(i = 0; i < GUI_MULTITASK_CNT; i++)
		{
			g_slvscr[i].lpbuff = slvscr_buff;
			g_slvscr[i].root_layer = GUI_SLVSCR_SURF;//vscr buffer mode
		}
	}

	return 0;
}

UINT32 gelib_dualoutput_onoff(UINT32 param)
{
	g_gelib_m3602_dualoutput = param;
}

RET_CODE app_dualoutput_release(UINT32 param)
{
	UINT8 i;
    PGUI_REGION prgn;
	
	if(ge_version!=GE_MODULE_M3602)
		return (-1);
	
	for(i = 0;i < GUI_MAX_REGION_ONLAYER;i++)
	{
        prgn = gedraw_getdstrgn(GUI_SLVGMA_SURF,i);

		if((void *)prgn->hsurf != NULL)
			gedraw_deletergn(prgn);
	}

    prgn = gedraw_getdstrgn(GUI_SLVSCR_SURF,0);
	if((void *)prgn->hsurf != NULL)
		gedraw_deletergn(prgn);
	if(NULL != (void *)g_layer_tbl[GUI_SLVSCR_SURF].hsurf)
		gedraw_releasesurf(&g_layer_tbl[GUI_SLVSCR_SURF]);    
	if(NULL != (void *)g_layer_tbl[GUI_SLVGMA_SURF].hsurf)
		gedraw_releasesurf(&g_layer_tbl[GUI_SLVGMA_SURF]);

	if(NULL != slvscr_buff)
	{
		FREE(slvscr_buff);
		slvscr_buff = NULL;
	}

	g_gelib_m3602_dualoutput = 0;	
	return 0;
}

RET_CODE gedraw_update2slave(PGUI_VSCR pvscr,PGUI_RECT prect)
{
	if((pvscr->suspend > 0) || (pvscr->dirty_flag == 0))
		return RET_FAILURE;//do not update to screen
		
	if((pvscr->dst_layer == GUI_SLVGMA_SURF) && (pvscr->root_layer == GUI_SLVSCR_SURF))
	{//dst region must be display surface,otherwise need not submit vscr
		UINT16	rctmask;
		UINT32	op_flag;
		RET_CODE ret;
		GUI_RECT	rctscale;
		PGUI_REGION	pdst,prgn;
		struct ge_device *dev;
		ge_rect_t	rct_dst,rct_src;

		dev = (struct ge_device *)gui_dev;
		prgn = gedraw_getdstrgn(pvscr->root_layer,pvscr->root_rgn);
		pdst = gedraw_getdstrgn(pvscr->dst_layer,pvscr->dst_rgn);
		
		op_flag = GE_BLT_SRC2_VALID;
		rct_src.left = rct_dst.left = prect->uStartX;
		rct_src.top = rct_dst.top = prect->uStartY;
		rct_src.width = rct_dst.width = prect->uWidth;
		rct_src.height = rct_dst.height = prect->uHeight;
		
		ge_lock(dev);
		ge_set_cur_region(dev,(ge_surface_desc_t *)pdst->hsurf,pdst->rgn_id);
		ret = ge_blt(dev,(ge_surface_desc_t *)pdst->hsurf,(ge_surface_desc_t *)prgn->hsurf, \
			&rct_dst,&rct_src,op_flag);
		ge_unlock(dev);
		return ret;
	}
	else
	{
		return RET_FAILURE;
	}
}

RET_CODE gedraw_retrieveslave(PGUI_VSCR pvscr,PGUI_RECT prect_r)
{
	RET_CODE ret;

	ret = RET_FAILURE;
	if((pvscr->dst_layer == GUI_SLVGMA_SURF) && (pvscr->root_layer == GUI_SLVSCR_SURF))
	{
		UINT32 op_flag;
		PGUI_REGION pdst,prgn;
		ge_rect_t 	rct_dst,rct_src;
		struct ge_device *dev;
		
		pdst = gedraw_getdstrgn(pvscr->dst_layer,pvscr->dst_rgn);
		prgn = gedraw_getdstrgn(pvscr->root_layer,pvscr->root_rgn);
		dev = (struct ge_device *)gui_dev;
		
		op_flag = GE_BLT_SRC2_VALID;
		rct_src.left = prect_r->uStartX;
		rct_src.top = prect_r->uStartY;
		rct_dst.left = prect_r->uStartX;
		rct_dst.top = prect_r->uStartY;
		rct_src.width = rct_dst.width = prect_r->uWidth;
		rct_src.height = rct_dst.height = prect_r->uHeight;

		ge_lock(dev);
		ge_set_cur_region(dev,(ge_surface_desc_t *)prgn->hsurf,prgn->rgn_id);
		ret = ge_blt(dev,(ge_surface_desc_t *)prgn->hsurf,(ge_surface_desc_t *)pdst->hsurf, \
			&rct_dst,&rct_src,op_flag);
		ge_unlock(dev);
	}

	return ret;
}
#endif

