/*-----------------------------------------------------------------------------
*
*	Copyright (C) 2008 ALI Corp. All rights reserved.
*
*	File: osd_window.c
*
*	Content: 
*		using window to manage region & surface etc.
*	History: 
*		2008/06/03 by Shine Zhou
*		Build for GE library
*-----------------------------------------------------------------------------*/
#include <sys_config.h>
#include <api/libge/osd_lib.h>
#include <api/libge/osd_window.h>
#include <api/libge/osd_primitive.h>
#include "osd_lib_internal.h"

UINT32	g_osdwnd_onoff;
UINT32	g_wndrects_idx;
UINT32	g_wndrects_cnt;
UINT32	g_wnd_monitors;
GUI_RECT g_wnd_rects[GELIB_WNDRECT_CNT];
extern GUI_WINDOW 	g_scrn_wnd[GUI_MAX_WND_CNT];//show on the screen,not total wnd count
extern GUI_REGION		g_rgn_tbl[GUI_MAX_REGION_CNT];//region handles array

static UINT32 osdwnd_reclaim_rects();

PGUI_REGION osdwnd_getwndregion(PGUI_WINDOW pwnd,UINT32 param)
{
	UINT8 rgn_idx;
	
	if(pwnd == NULL)
	{
		return NULL;
	}
	else
	{
		rgn_idx = pwnd->rgn_idx;
		return &g_rgn_tbl[rgn_idx];
	}
}

UINT32 osdwnd_releasewndregion(PGUI_WINDOW pwnd,UINT32 param)
{
	return 0;
}

UINT32 osdwnd_createwnd(UINT32 wnd_id,UINT32 param)
{
	return 0;
}

UINT32 osdwnd_destroywnd(UINT32 wnd_id,UINT32 param)
{
	return 0;
}

UINT32 osdwnd_showwnd(PGUI_WINDOW pwnd,UINT32 param)
{
	UINT32 i,bfind;
	PGUI_REGION 	prgn;
	PGUI_VSCR	pvscr;
	PGUI_OBJWND wndobj;
	
	i = 0;
	if(gelib_get_wndmonitor())
	{
		bfind = 0;
		wndobj = (PGUI_OBJWND)param;
		if(g_wndrects_cnt>0)
		{
			for(i=0;i<g_wndrects_idx;i++)
			{
				if(MEMCMP(&(wndobj->frame),&(g_wnd_rects[i]),sizeof(GUI_RECT)) == 0)
				{
					bfind = 1;
					break;
				}
			}
		}

		if((gelib_wndrects_valid())&&( bfind==0))
		{
			MEMCPY(&(g_wnd_rects[g_wndrects_idx]),&(wndobj->frame),sizeof(GUI_RECT));
			g_wndrects_idx++;
			g_wndrects_cnt++;
		}

		if(gelib_wndrects_full())
		{//wndrects is not enough
			gelib_disable_wnd_monitors(1);
		}
		else if(gelib_wndrects_valid()==0)
		{
			osdwnd_reclaim_rects();
		}
		
		if(gelib_wndrects_showon() && (g_osdwnd_onoff == 0))
		{
			pvscr = OSD_GetTaskVscr(osal_task_get_current_id());
			prgn = gelib_getdstrgn(pvscr->dst_layer,pvscr->dst_rgn);
			ge_show_onoff((struct ge_device *)gui_dev,(ge_surface_desc_t *)prgn->hsurf,1);

			if(gelib_getdual_support())
			{
				prgn = gelib_getdstrgn(GUI_SLVGMA_SURF,0);
				ge_show_onoff_deo((struct ge_device *)gui_dev,(ge_surface_desc_t *)prgn->hsurf,1);
			}

			g_osdwnd_onoff = 1;
		}
	}

	return i;
}

static UINT32 osdwnd_reclaim_rects()
{
	UINT32 i,j,ufree;
	PGUI_RECT prct_tmp;

	ufree = 0;
	for(i = 0;i<GELIB_WNDRECT_CNT;i++)
	{
		if(gui_rctvalid(&(g_wnd_rects[i]))==0)
		{//a blank position
			for(j=i+1;j<GELIB_WNDRECT_CNT;j++)
			{
				if(gui_rctvalid(&(g_wnd_rects[j]))==1)
				{
					MEMCPY(&(g_wnd_rects[i]),&(g_wnd_rects[j]),sizeof(GUI_RECT));
					MEMSET(&(g_wnd_rects[j]),0x0,sizeof(GUI_RECT));
					ufree++;
					break;
				}
			}

			if(j==GELIB_WNDRECT_CNT)
				break;
		}
		else
			continue;
	}
	
	g_wndrects_idx = g_wndrects_cnt;
	
	return ufree;
}


UINT32 osdwnd_hidewnd(PGUI_WINDOW pwnd,UINT32 param)
{//clear wnd
	UINT32 i;
	PGUI_VSCR	pvscr;
	PGUI_REGION 	prgn;
	PGUI_OBJWND wndobj;
	//TODO:windows clear code
	
	if(gelib_get_wndmonitor()&&g_wndrects_cnt)
	{
		wndobj = (PGUI_OBJWND)param;
		for(i = 0;i < g_wndrects_idx;i++)
		{
			if(MEMCMP(&(wndobj->frame),&(g_wnd_rects[i]),sizeof(GUI_RECT)) == 0)
			{
				MEMSET(&(g_wnd_rects[i]),0x0,sizeof(GUI_RECT));

				if(g_wndrects_cnt > 0)
					g_wndrects_cnt--;
			}
		}

		if(gelib_wndrects_showoff() && g_osdwnd_onoff)
		{
			pvscr = OSD_GetTaskVscr(osal_task_get_current_id());
			prgn = gelib_getdstrgn(pvscr->dst_layer,pvscr->dst_rgn);
			ge_show_onoff((struct ge_device *)gui_dev,(ge_surface_desc_t *)prgn->hsurf,0);
			if(gelib_getdual_support())
			{
				prgn = gelib_getdstrgn(GUI_SLVGMA_SURF,0);
				ge_show_onoff_deo((struct ge_device *)gui_dev,(ge_surface_desc_t *)prgn->hsurf,0);
			}			
			
			g_osdwnd_onoff = 0;
		}
	}
	else if((g_wndrects_cnt == 0)&&(g_wnd_monitors == 1))
	{
		if(gelib_wndrects_showoff() && g_osdwnd_onoff)
		{
			pvscr = OSD_GetTaskVscr(osal_task_get_current_id());
			prgn = gelib_getdstrgn(pvscr->dst_layer,pvscr->dst_rgn);
			ge_show_onoff((struct ge_device *)gui_dev,(ge_surface_desc_t *)prgn->hsurf,0);
			
			g_osdwnd_onoff = 0;
		}
	}
	
	return 0;
}

UINT32 osdwnd_get_wndshow()
{
	if(gelib_get_wndmonitor())
		return g_wndrects_cnt;
	else
		return 1;
}

UINT32 gelib_enable_wnd_monitors()
{
	g_wnd_monitors = 1;

	//TODO: clear wnd rects informations
	g_wndrects_idx = 0;
	g_wndrects_cnt = 0;
	g_osdwnd_onoff = 1;
	MEMSET(g_wnd_rects,0x0,sizeof(g_wnd_rects));
	
	return 0;
}

UINT32 gelib_disable_wnd_monitors(UINT32 onoff)
{
	PGUI_VSCR	pvscr;
	PGUI_REGION 	prgn;
	
	g_wnd_monitors = 0;
	
	g_wndrects_idx = 0;
	g_wndrects_cnt = 0;
	g_osdwnd_onoff = 1;
	MEMSET(g_wnd_rects,0x0,sizeof(g_wnd_rects));
	
	pvscr = OSD_GetTaskVscr(osal_task_get_current_id());
	prgn = gelib_getdstrgn(pvscr->dst_layer,pvscr->dst_rgn);
	ge_show_onoff((struct ge_device *)gui_dev,(ge_surface_desc_t *)prgn->hsurf,onoff);

	if(gelib_getdual_support())
	{
		prgn = gelib_getdstrgn(GUI_SLVGMA_SURF,0);
		ge_show_onoff_deo((struct ge_device *)gui_dev,(ge_surface_desc_t *)prgn->hsurf,1);
	}
	return 0;
}

