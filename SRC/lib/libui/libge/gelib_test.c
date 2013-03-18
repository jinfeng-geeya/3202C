/*====================================================================
 *
 *  Copyright (C) 2008 ALi.  All Rights Reserved.
 *
 *  File:   gelib_test.c
 *  History:      Build for new GElib by Shine Zhou@2010/03/01
 *********************************************************************/
#include <sys_config.h>
#include <api/libge/osd_lib.h>
#include <api/libge/osd_window.h>
#include <api/libge/ge_draw_primitive.h>
#include <api/libge/osd_common_draw.h>
#include <api/libge/osd_popup.h>
#include "osd_lib_internal.h"
#include "gelib_test.h"

#if (1==0)
void app_gelib_init()
{
	struct ge_device *ge_dev;
	GUI_RSC_FNC osd_rsc_info;

	MEMSET(&osd_rsc_info,0x0,sizeof(osd_rsc_info));
	osd_rsc_info.osd_get_lang_env = OSD_GetLangEnvironment;
	osd_rsc_info.osd_get_obj_info = OSD_GetObjInfo;
	osd_rsc_info.osd_get_rsc_data= OSD_GetRscObjData;
	osd_rsc_info.osd_rel_rsc_data= OSD_ReleaseObjData;
	osd_rsc_info.osd_get_font_lib = OSD_GetDefaultFontLib;
	osd_rsc_info.osd_get_win_style = OSDExt_GetWinStyle;
	osd_rsc_info.osd_get_str_lib_id = OSDExt_GetMsgLibId;
	osd_rsc_info.ap_hk_to_vk = ap_hk_to_vk;
	app_rsc_init(&osd_rsc_info);

	ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
	if(RET_SUCCESS!=ge_open(ge_dev))
	{
		PRINTF("ge_open failed!!\n");
		ASSERT(0);
	}
	
	GUI_INIT_PARAM gui_init;

	MEMSET(&gui_init,0x0,sizeof(gui_init));
	gui_init.ge_version = GE_MODULE_M3329D;
	gui_init.gui_lines = GUI_PAL_LINES;
	gelib_init((GUI_DEV)ge_dev,&gui_init);

	GUI_REGION rgn_par;
	MEMSET(&rgn_par,0x0,sizeof(rgn_par));
	rgn_par.layer_type = GUI_GMA1_SURF;
	rgn_par.rgn_id = 0;
	rgn_par.rect.uStartX = (720 - OSD_MAX_WIDTH)>>1;
	rgn_par.rect.uStartY = (576 - OSD_MAX_HEIGHT)>>1;
	rgn_par.rect.uWidth = OSD_MAX_WIDTH;
	rgn_par.rect.uHeight = OSD_MAX_HEIGHT;
	rgn_par.color_mode = GUI_CLUT8;
	gelib_create_rgn((UINT32)&rgn_par);

	MEMSET(&rgn_par,0x0,sizeof(rgn_par));
	rgn_par.layer_type = GUI_MEMVSCR_SURF;
	rgn_par.rgn_id = 0;
	rgn_par.rect.uStartX = (720 - OSD_MAX_WIDTH)>>1;
	rgn_par.rect.uStartY = (576 - OSD_MAX_HEIGHT)>>1;
	rgn_par.rect.uWidth = OSD_MAX_WIDTH;
	rgn_par.rect.uHeight = OSD_MAX_HEIGHT;
	rgn_par.color_mode = GUI_CLUT8;
	gelib_create_rgn((UINT32)&rgn_par);

	PGUI_REGION pgmargn;
	pgmargn = gelib_getdstrgn(GUI_GMA1_SURF,0);
	gelib_rgn_showonoff(pgmargn,1);
}
#endif

#ifdef GELIB_M3701C_CCBN
UINT32 gui_dev_m3701c_attach()
{
	ge_layer_config_t osd_config[1];
	
	MEMSET((void *)&osd_config, 0, sizeof(osd_config));	
	osd_config[0].mem_base = __MM_OSD_BK_ADDR1;
	osd_config[0].mem_size = __MM_OSD_LEN1;
	osd_config[0].bDirectDraw = FALSE;
	osd_config[0].bCacheable = FALSE;
	osd_config[0].bScaleFilterEnable = TRUE;
	osd_config[0].bP2NScaleInNormalPlay = FALSE;
	osd_config[0].bP2NScaleInSubtitlePlay = TRUE;
	
	osd_config[0].hw_layer_id = GMA_HW_LAYER0;
	osd_config[0].color_format = GE_PF_ARGB8888;
	osd_config[0].pixel_pitch = ((OSD_MAX_WIDTH));
	osd_config[0].width = OSD_MAX_WIDTH;
	osd_config[0].height = OSD_MAX_HEIGHT;
	ge_m36f_attach(osd_config, 1);
	g_ge_dev = dev_get_by_id(HLD_DEV_TYPE_GE, 0);
}

void ap_osd_init()
{
	struct ge_device *ge_dev;
	GUI_RSC_FNC osd_rsc_info;

	MEMSET(&osd_rsc_info,0x0,sizeof(osd_rsc_info));
	osd_rsc_info.osd_get_lang_env = OSD_GetLangEnvironment;
	osd_rsc_info.osd_get_obj_info = OSD_GetObjInfo;
	osd_rsc_info.osd_get_rsc_data= OSD_GetRscObjData;
	osd_rsc_info.osd_rel_rsc_data= OSD_ReleaseObjData;
	osd_rsc_info.osd_get_font_lib = OSD_GetDefaultFontLib;
	osd_rsc_info.osd_get_win_style = OSDExt_GetWinStyle;
	osd_rsc_info.osd_get_str_lib_id = OSDExt_GetMsgLibId;
	osd_rsc_info.ap_hk_to_vk = ap_hk_to_vk;
	app_rsc_init(&osd_rsc_info);

	ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
	if(RET_SUCCESS!=ge_open(ge_dev))
	{
		PRINTF("ge_open failed!!\n");
		ASSERT(0);
	}
	
	GUI_INIT_PARAM gui_init;

	MEMSET(&gui_init,0x0,sizeof(gui_init));
	gui_init.ge_version = GE_MODULE_M3603;
	gui_init.gui_lines = GUI_720P_LINES;
	gelib_init((GUI_DEV)ge_dev,&gui_init);

	GUI_REGION rgn_par;
	MEMSET(&rgn_par,0x0,sizeof(rgn_par));
	rgn_par.layer_type = GUI_GMA1_SURF;
	rgn_par.rgn_id = 0;
	rgn_par.rect.uStartX = (1280 - OSD_MAX_WIDTH)>>1;
	rgn_par.rect.uStartY = (720 - OSD_MAX_HEIGHT)>>1;
	rgn_par.rect.uWidth = OSD_MAX_WIDTH;
	rgn_par.rect.uHeight = OSD_MAX_HEIGHT;
	rgn_par.color_mode = GUI_ARGB8888;
	gelib_create_rgn((UINT32)&rgn_par);
	MEMSET(&rgn_par,0x0,sizeof(rgn_par));
	rgn_par.layer_type = GUI_MEMVSCR_SURF;
	rgn_par.rgn_id = 0;
	rgn_par.rect.uStartX = (1280 - OSD_MAX_WIDTH)>>1;
	rgn_par.rect.uStartY = (720 - OSD_MAX_HEIGHT)>>1;
	rgn_par.rect.uWidth = OSD_MAX_WIDTH;
	rgn_par.rect.uHeight = OSD_MAX_HEIGHT;
	rgn_par.color_mode = GUI_ARGB8888;
	gelib_create_rgn((UINT32)&rgn_par);
	
	PGUI_REGION pgmargn,rgnplug;
	pgmargn = gelib_getdstrgn(GUI_GMA1_SURF,0);
	gelib_rgn_showonoff(pgmargn,1);
    
	OSD_RegionInit();//init for plugin compatible APIs
	rgnplug = gelib_getdstrgn(GUI_GMA1_SURF,0);
	OSD_SetDeviceHandle((HANDLE)(rgnplug->hsurf));//Switch to GMA2

	PGUI_VSCR pvsr = OSD_GetTaskVscr(osal_task_get_current_id());
	OSD_SetVscrLayer(pvsr,GUI_MEMVSCR_SURF,0,GUI_GMA1_SURF,0);
	OSD_SetVscrDst(GUI_GMA1_SURF,0);
}


#endif

#ifdef GELIB_M3602_DUALOUT
void app_gelib_test(UINT32 color)
{
	UINT8 *pBitMapData;
	ID_RSC	RscLibId;
	OBJECTINFO	ObjInfo;
	
	GUI_RECT rct;
	PGUI_REGION prgn;

	rct.uStartX = 120;
	rct.uStartY = 120;
	rct.uWidth = 200;
	rct.uHeight = 200;
	prgn = gelib_getdstrgn(GUI_GMA2_SURF,0);

	gelib_fillrect(prgn,&rct,color);
	RscLibId = g_osd_rsc_info.osd_get_font_lib(0x7cfb) | C_FONT_DEFAULT;
	if(NULL !=(pBitMapData =(UINT8*)g_osd_rsc_info.osd_get_rsc_data(RscLibId, 0x7cfb, &ObjInfo)))
	{
		GUI_RECT rect_fill;
		GUI_FONT font_bmp;
	
		MEMSET(&font_bmp,0x0,sizeof(GUI_FONT));
		font_bmp.rsc_rle = 0;
		font_bmp.pdata = (void *)pBitMapData;
		font_bmp.fnt_size.uWidth = ObjInfo.m_objAttr.m_wActualWidth;
		font_bmp.fnt_size.uHeight = ObjInfo.m_objAttr.m_wHeight;
		font_bmp.stride = ObjInfo.m_objAttr.m_wWidth;
		font_bmp.fg_color = 0x7b;
		font_bmp.bg_color = 0xff;
		
		rect_fill.uStartX = 128;
		rect_fill.uStartY = 128;
		rect_fill.uWidth = 64;
		rect_fill.uHeight = 64;
		gelib_drawfont(prgn,&rect_fill,(UINT32)&font_bmp);
	}
}

void app_dualoutput_test(UINT32 color)
{
	UINT8 *pBitMapData;
	ID_RSC	RscLibId;
	OBJECTINFO	ObjInfo;
	
	GUI_RECT rct;
	PGUI_REGION prgn;

	rct.uStartX = 120;
	rct.uStartY = 120;
	rct.uWidth = 200;
	rct.uHeight = 200;
	prgn = gelib_getdstrgn(GUI_SLVGMA_SURF,0);

	gelib_fillrect(prgn,&rct,color);
	
	RscLibId = g_osd_rsc_info.osd_get_font_lib(0x7cfb) | C_FONT_DEFAULT;
	if(NULL !=(pBitMapData =(UINT8*)g_osd_rsc_info.osd_get_rsc_data(RscLibId, 0x7cfb, &ObjInfo)))
	{
		GUI_RECT rect_fill;
		GUI_FONT font_bmp;
	
		MEMSET(&font_bmp,0x0,sizeof(GUI_FONT));
		font_bmp.rsc_rle = 0;
		font_bmp.pdata = (void *)pBitMapData;
		font_bmp.fnt_size.uWidth = ObjInfo.m_objAttr.m_wActualWidth;
		font_bmp.fnt_size.uHeight = ObjInfo.m_objAttr.m_wHeight;
		font_bmp.stride = ObjInfo.m_objAttr.m_wWidth;
		font_bmp.fg_color = 0x7b;
		font_bmp.bg_color = 0xff;
		
		rect_fill.uStartX = 128;
		rect_fill.uStartY = 128;
		rect_fill.uWidth = 64;
		rect_fill.uHeight = 64;
		gelib_drawfont(prgn,&rect_fill,(UINT32)&font_bmp);
	}
}

//sample code for dualoutput
void ap_osd_init()
{
	struct ge_device *ge_dev;
	GUI_APP_CFG osd_cfg;
	GUI_RSC_FNC osd_rsc_info;
	ge_anti_flick_t af;
	GUI_RGN_CFG rgn_list[2],deo_lst;
	PGUI_REGION rgnplug;

	MEMSET(&osd_rsc_info, 0, sizeof(osd_rsc_info));
	osd_rsc_info.osd_get_lang_env = OSD_GetLangEnvironment;
	osd_rsc_info.osd_get_obj_info = OSD_GetObjInfo;
	osd_rsc_info.osd_get_rsc_data= OSD_GetRscObjData;
	osd_rsc_info.osd_rel_rsc_data= OSD_ReleaseObjData;
	osd_rsc_info.osd_get_font_lib = OSD_GetDefaultFontLib;
	osd_rsc_info.osd_get_win_style = OSDExt_GetWinStyle;
	osd_rsc_info.osd_get_str_lib_id = OSDExt_GetMsgLibId;
	osd_rsc_info.ap_hk_to_vk = ap_hk_to_vk;
	
	app_rsc_init(&osd_rsc_info);

	ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
    
	if(RET_SUCCESS!=ge_open(ge_dev))
	{
		PRINTF("ge_open failed!!\n");
		ASSERT(0);
	}

	MEMSET(&osd_cfg,0xFF,sizeof(osd_cfg));
	MEMSET(&osd_cfg.layer_color,GUI_INVALIDFORMAT,sizeof(&osd_cfg.layer_color));
	osd_cfg.hdev = (GUI_DEV)ge_dev;
	osd_cfg.layer_color[GUI_GMA2_SURF] = GUI_AYCBCR8888;//GUI_AYCBCR8888;//GUI_ARGB8888;//GUI_ARGB1555;
	osd_cfg.layer_rect[GUI_GMA2_SURF].uStartX = (1280 - OSD_MAX_WIDTH)>>1;;
	osd_cfg.layer_rect[GUI_GMA2_SURF].uStartY = (720-OSD_MAX_HEIGHT) >> 1;
	osd_cfg.layer_rect[GUI_GMA2_SURF].uWidth = OSD_MAX_WIDTH;
	osd_cfg.layer_rect[GUI_GMA2_SURF].uHeight = OSD_MAX_HEIGHT;
	//osd_cfg.layer_color[GUI_GMA2_SURF] = GUI_ARGB1555;
	//osd_cfg.layer_rect[GUI_GMA2_SURF].uStartX = (720 - OSD_MAX_WIDTH)>>1;;
	//osd_cfg.layer_rect[GUI_GMA2_SURF].uStartY = (576-OSD_MAX_HEIGHT) >> 1;
	//osd_cfg.layer_rect[GUI_GMA2_SURF].uWidth = OSD_MAX_WIDTH;
	//osd_cfg.layer_rect[GUI_GMA2_SURF].uHeight = OSD_MAX_HEIGHT;
	osd_cfg.layer_color[GUI_MEMVSCR_SURF] = GUI_AYCBCR8888;//GUI_ARGB1555;
	osd_cfg.layer_rect[GUI_MEMVSCR_SURF].uStartX = (1280 - OSD_MAX_WIDTH)>>1;;
	osd_cfg.layer_rect[GUI_MEMVSCR_SURF].uStartY = (720-OSD_MAX_HEIGHT) >> 1;
	osd_cfg.layer_rect[GUI_MEMVSCR_SURF].uWidth = OSD_MAX_WIDTH;
	osd_cfg.layer_rect[GUI_MEMVSCR_SURF].uHeight = OSD_MAX_HEIGHT;
//	osd_cfg.layer_color[GUI_MEM_SURF] = GUI_ARGB1555;
//	osd_cfg.layer_rect[GUI_MEM_SURF].uStartX = (720 - OSD_MAX_WIDTH)>>1;;
//	osd_cfg.layer_rect[GUI_MEM_SURF].uStartY = (576-OSD_MAX_HEIGHT) >> 1;
//	osd_cfg.layer_rect[GUI_MEM_SURF].uWidth = OSD_MAX_WIDTH;
//	osd_cfg.layer_rect[GUI_MEM_SURF].uHeight = OSD_MAX_HEIGHT;

	osd_cfg.direct_draw = FALSE;
	osd_cfg.animation_support = 0;

	MEMSET(rgn_list,0x0,sizeof(rgn_list));
	rgn_list[0].layer_idx = GUI_GMA2_SURF;
	rgn_list[0].rgn_idx = 0;
	rgn_list[0].rect.uStartX = (1280 - OSD_MAX_WIDTH)>>1;
	rgn_list[0].rect.uStartY = (720-OSD_MAX_HEIGHT) >> 1;
	rgn_list[0].rect.uWidth = OSD_MAX_WIDTH;
	rgn_list[0].rect.uHeight = OSD_MAX_HEIGHT;
/*	
	rgn_list[1].layer_idx = GUI_GMA2_SURF;
	rgn_list[1].rgn_idx = 0;
	rgn_list[1].rect.uStartX = 100;
	rgn_list[1].rect.uStartY = 100;
	rgn_list[1].rect.uWidth = 128;
	rgn_list[1].rect.uHeight = 128;
	
	rgn_list[2].layer_idx = GUI_GMA2_SURF;
	rgn_list[2].rgn_idx = 1;
	rgn_list[2].rect.uStartX = 256;
	rgn_list[2].rect.uStartY = 256;
	rgn_list[2].rect.uWidth = 128;
	rgn_list[2].rect.uHeight = 128;
*/	
	rgn_list[1].layer_idx = GUI_MEMVSCR_SURF;
	rgn_list[1].rgn_idx = 0;
	rgn_list[1].rect.uStartX = (1280 - OSD_MAX_WIDTH)>>1;
	rgn_list[1].rect.uStartY = (720-OSD_MAX_HEIGHT) >> 1;
	rgn_list[1].rect.uWidth = OSD_MAX_WIDTH;
	rgn_list[1].rect.uHeight = OSD_MAX_HEIGHT;//multi region info,rect & parent layer & region id etc.	

	osd_cfg.rgn_cnt = 2;
    
	GUI_INIT_PARAM gui_init;

	MEMSET(&gui_init,0x0,sizeof(gui_init));
	gui_init.ge_version = GE_MODULE_M3602;
	gui_init.gui_lines = GUI_720P_LINES;
    
	gelib_init((GUI_DEV)ge_dev,&gui_init);
	app_gui_init(rgn_list,(UINT32)&osd_cfg);

	MEMSET(&deo_lst,0x0,sizeof(deo_lst));
	deo_lst.layer_idx = GUI_SLVGMA_SURF;
	deo_lst.rgn_idx = 0;
	deo_lst.rect.uStartX = 0;
	deo_lst.rect.uStartY = 0;
	deo_lst.rect.uWidth = 720;
	deo_lst.rect.uHeight = 576;
	app_dualoutput_init(&deo_lst,0);

	OSD_RegionInit();//init for plugin compatible APIs
	rgnplug = gelib_getdstrgn(GUI_GMA2_SURF,0);
	OSD_SetDeviceHandle((HANDLE)(rgnplug->hsurf));//Switch to GMA2

	PGUI_VSCR pvsr = OSD_GetTaskVscr(osal_task_get_current_id());
	OSD_SetVscrLayer(pvsr,GUI_MEMVSCR_SURF,0,GUI_GMA2_SURF,0);
	OSD_SetVscrDst(GUI_GMA2_SURF,0);
}


SDBMP slvmute;
extern BITMAP win_mute_bmp;
DEF_BITMAP(win_mute_bmp, NULL, NULL, C_ATTR_ACTIVE, 0,  \
	1, 0, 0, 0, 0, MUTE_L, MUTE_T, MUTE_W, MUTE_H, WSTL_TRANS_IX, WSTL_TRANS_IX, 0, 0,  \
	NULL, NULL,  \
	C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, MUTE_ICON)
	
void ShowMuteOSDOnOff(UINT8 flag)
{
	OBJECT_HEAD *obj;
	obj = (OBJECT_HEAD*) &win_mute_bmp;

	win_mute_bmp.slave = &slvmute;
	slvmute.frame.uStartX = 100;
	slvmute.frame.uStartY = 100;
	slvmute.frame.uWidth = 64;
	slvmute.frame.uHeight = 64;
	slvmute.bX = 4;
	slvmute.bY = 4;
	slvmute.wIconID = MUTE_ICON;
	slvmute.style.bSelIdx = WSTL_BUTTON_2;
	slvmute.style.bGrayIdx = WSTL_BUTTON_2;
	slvmute.style.bHLIdx = WSTL_BUTTON_2;
	slvmute.style.bShowIdx = WSTL_BUTTON_2;
	if (flag)
		OSD_DrawObject(obj, C_UPDATE_ALL);
	else
		OSD_ClearObject(obj, 0);
}

#define __MM_DEO_LEN (720*576)
void gelib_guidev_attach()
{
	ge_driver_config_t para_config;
	struct ge_device *ge_dev = NULL;
	
	MEMSET(&para_config, 0, sizeof(ge_driver_config_t));
	para_config.ge_start_base = __MM_GE_START_ADDR;
	para_config.GMA1_buf_size = __MM_GMA2_LEN;//__MM_OSD1_LEN;
	para_config.GMA2_buf_size = __MM_GMA1_LEN;//__MM_OSD2_LEN;
	para_config.temp_buf_size = 0;
	para_config.cmd_buf_size = __MM_CMD_LEN;//__MM_GE_CMD_LEN;
	para_config.bpolling_cmd_status = TRUE;
	para_config.op_mutex_enable = TRUE;
	//para_config.st_on_osd_enable = TRUE;
	
	para_config.GMA_deo_buf_addr = ((UINT32)MALLOC(__MM_DEO_LEN+16)) & 0Xfffffff0; 
	para_config.GMA_deo_buf_size = __MM_DEO_LEN;
	ge_m36_attach(&para_config);    

	//TODO: add a ioctrl for SD_UI pallette setting
	vpo_ioctl(g_vpo_dev, VPO_IO_CB_UPDATE_PALLET, ge_io_ctrl_deo);
}
#endif

#define DEO_RSC_TEST
#ifdef DEO_RSC_TEST

#define vratio (0.8)
#define hratio (0.5625)
#define wstylsdoffset (125)
#define bmpidsdoffset (1024)
#define DEF_BITMAP(bmp,root,next,attr,font,ID,lID,rID,tID,dID,l,t,w,h,sh,hl,sel,gry,kmap,cb,align,ox,oy,iconID)   \
SDBMP bmp##_sd = \
{   \
        {(UINT16)((l)*hratio),(UINT16)((t)*vratio),(UINT16)((w)*hratio),(UINT16)((h)*vratio)},      \
        {sh + wstylsdoffset,hl + wstylsdoffset,sel + wstylsdoffset,gry + wstylsdoffset},    \
        (UINT8)((ox)*hratio),(UINT8)((oy)*vratio),(iconID + bmpidsdoffset),\
}; \
BITMAP bmp = \
{   \
    {OT_BITMAP, attr, font,ID,lID,rID,tID,dID,   \
        {l,t,w,h},      \
        {sh,hl,sel,gry},    \
        kmap,cb,    \
        (POBJECT_HEAD)next,(POBJECT_HEAD)root,  \
    },  \
    align,ox,oy,iconID,(UINT32)&(bmp##_sd) \
};

extern BITMAP testbmp;
extern SDBMP testbmp_sd;
DEF_BITMAP(testbmp,NULL,NULL,C_ATTR_ACTIVE,0, \
	0,0,0,0,0,20,20,400,400, 1,1,1,1,   \
	NULL,NULL,  \
	C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,0)

UINT32 gelib_deo_rsctest(UINT16 imgs,UINT32 imge,UINT32 param)
{
	UINT16 i,stylecnt;
	UINT8 *pBitMapData;
	ID_RSC	RscLibId;
	OBJECTINFO	ObjInfo;
	
	GUI_RECT rct;
	PGUI_REGION prgn;

	rct.uStartX = 20;
	rct.uStartY = 20;
	rct.uWidth = 400;
	rct.uHeight = 400;
	
	prgn = gelib_getdstrgn(GUI_SLVGMA_SURF,0);
	for(i = 0;i <= 0xff;i++)
	{
		gelib_fillrect(prgn,&rct,i);
		osal_task_sleep(80);
	}

	for(i = imgs;i <= imge;i++)
	{
		testbmp.wIconID = i;
		testbmp_sd.wIconID = i+1024;

		OSD_DrawObject((POBJECT_HEAD)&testbmp,C_UPDATE_ALL);
		osal_task_sleep(80);
	}

	for(i = imgs;i <= imge;i++)
	{
		testbmp.wIconID = i;
		testbmp_sd.wIconID = i+1024;

		OSD_DrawObject((POBJECT_HEAD)&testbmp,C_UPDATE_ALL);
		osal_task_sleep(80);
	}

	stylecnt = param;
	for(i = 0;i < stylecnt;i++)
	{
		testbmp.head.style.bShowIdx = i;
		testbmp_sd.style.bShowIdx = i;

		gelib_fillrect(prgn,&(testbmp_sd.frame),0xff);
		OSD_DrawObject((POBJECT_HEAD)&testbmp,C_UPDATE_ALL);
		osal_task_sleep(120);
	}
	
	return 0;
}
#endif

#ifdef GELIB_PALPHA_TEST
UINT32 gelib_pixelalpha_test(UINT32 fcolor,UINT32 param)
{
	UINT32 i,color;
	GUI_RECT rctfill;
	PGUI_REGION prgn;

	rctfill.uStartX = 10;
	rctfill.uStartY = 0;
	rctfill.uWidth = 2;
	rctfill.uHeight = 200;

	prgn = gelib_getdstrgn(GUI_GMA2_SURF,0);
         gelib_fillrect(prgn,NULL,0xffaabbcc);
	for(i=0;i<0xff;i++)
	{
		color = (0x8f3057) |(i<<24);
		rctfill.uStartX += 2;
		rctfill.uStartX = (rctfill.uStartX)%700;
		gelib_fillrect(prgn,&rctfill,color);
		osal_task_sleep(120);
	}
}
#endif

