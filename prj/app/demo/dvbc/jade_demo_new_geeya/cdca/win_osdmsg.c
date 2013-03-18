#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

#if(defined(MIS_AD) || defined(MIS_AD_NEW))
#include "..\control.h"
#include "..\osd_rsc.h"
#include "win_cas_com.h"

#include <api/libge/osd_common.h>
#include <api/libcas/gy/cas_gy.h>
#if 0
#define OSDMSG_PRINT	libc_printf
#else
#define OSDMSG_PRINT(...) 	do{}while(0)
#endif

/*******************************************************************************
*	Objects definition
*******************************************************************************/
#define OSDMSG_REGION_TOP		0
#define OSDMSG_REGION_BTM		1

#define OSDMSG_BK_COLOR		0x0000//0x801F	//BLUE
#define OSDMSG_FONT_COLOR	0xFFFF	//WHITE

#define OSDMSG_SCROLL_TASK_STACKSIZE	(10 * 1024)
#define OSDMSG_SCROLL_TASK_QUANTUM		10

#define OSDMSG_DISP_BUF_SIZE	(720 * 28 * 2)

#define OSDMSG_BUFSIZE			(GY_OSDMSG_LEN+80)

static UINT16 osdmsg_buf_top[OSDMSG_BUFSIZE];
static UINT16 osdmsg_buf_btm[OSDMSG_BUFSIZE];

static UINT8 fontBuf_top[512];
static UINT8 fontBuf_btm[512];

static UINT8 osdmsg_disp_top_buf[OSDMSG_DISP_BUF_SIZE];
static UINT8 osdmsg_disp_btm_buf[OSDMSG_DISP_BUF_SIZE];

static BOOL osdmsg_top_disp_flag = FALSE;
static BOOL osdmsg_bottom_disp_flag = FALSE;

static OSAL_ID osdmsg_mutex_id;

static struct ge_device *osdmsg_ge_dev = NULL;

static ge_surface_desc_t *osdmsg_dst_surface = NULL, *osdmsg_src_surface = NULL;

static ge_surface_desc_t *ge_dst_surface = NULL, *ge_src_surface = NULL;

static ge_rect_t ge_dst_region = {0, 0, 720, 576};
OSAL_ID osdmsg_mbf_id = OSAL_INVALID_ID;
static ge_rect_t osdmsg_rect[2] = 
{
	{
		.left = 0,
		.top = 20,
		.width = 720,
		.height = 28,
	},
	{
		.left = 0,
		.top = 528,
		.width = 720,
		.height = 28,
	}
};

static UINT8 *osdmsg_content = "播放同方加密码流，显示滚动字幕时，进入系统设置菜单时，有时菜单上随机显示一个黑块，有时菜单上方显示一条绿色";

void osdmsg_test_set_msg_content(UINT8 *buf)
{
	convert_gb2312_to_unicode(osdmsg_content, 106, (UINT16 *)buf, 108);
}

//copy the osd data to local disp buff, and set the flag
BOOL osdmsg_display_start (UINT8 bystyle)
{
	ER		ret_val;
	UINT8 	style = bystyle,i;
	UINT8	temp[GY_OSDMSG_LEN+2] ={0,};
	UINT16 	len;
	
	if (CA_OSD_ON_TOP == style)
	{
		OSDMSG_PRINT("@@@@SHOW message on the top!\n");
		osal_mutex_lock(osdmsg_mutex_id, OSAL_WAIT_FOREVER_TIME);
		MEMSET(osdmsg_buf_top, 0, sizeof(osdmsg_buf_top));
		//CDSTBCA_GetOSDMessage(&osdmsg_buf_top);
		len = ComUniStrLen(osdmsg_buf_top);
		for(i = 0;i <100;i++)//68: how many space fill the whole line
		{
			osdmsg_buf_top[len+i] = 0x2000;
		}
		osal_mutex_unlock(osdmsg_mutex_id);
		osdmsg_top_disp_flag = TRUE;
	}
	else 
	{
		OSDMSG_PRINT("@@@@SHOW message on the Bottom!\n");
		osal_mutex_lock(osdmsg_mutex_id, OSAL_WAIT_FOREVER_TIME);
		MEMSET(osdmsg_buf_btm, 0, sizeof(osdmsg_buf_btm));
		//CDSTBCA_GetOSDMessage(&osdmsg_buf_btm);
		len = ComUniStrLen(osdmsg_buf_btm);
		for(i = 0;i <100;i++)
		{
			osdmsg_buf_btm[len+i] = 0x2000;//0x2000;
		}
		osal_mutex_unlock(osdmsg_mutex_id);
		osdmsg_bottom_disp_flag = TRUE;
	}
	return TRUE;
}

//clear local disp buff flag
BOOL osdmsg_display_stop(UINT8 bystyle)
{
	if (CA_OSD_ON_TOP == bystyle)
	{
		OSDMSG_PRINT("@@@@stop message on the top!\n");
		osdmsg_top_disp_flag = FALSE;
		osal_task_sleep(50);
	}
	else 
	{
		OSDMSG_PRINT("@@@@stop message on the Bottom!\n");
		osdmsg_bottom_disp_flag = FALSE;
		osal_task_sleep(50);
	}

	return TRUE;
}

static void osdmsg_ge_draw(UINT8 id)
{
	ge_rect_t dst_rect, src_rect;
	
	dst_rect.left = osdmsg_rect[id].left;
	dst_rect.top = osdmsg_rect[id].top;
	dst_rect.width = osdmsg_rect[id].width;
	dst_rect.height = osdmsg_rect[id].height;

	ge_lock(osdmsg_ge_dev);

#if 0
	src_rect.left = osdmsg_rect[id].left;
	src_rect.top = osdmsg_rect[id].top;
	src_rect.width = osdmsg_rect[id].width;
	src_rect.height = osdmsg_rect[id].height;

	ge_lock(osdmsg_ge_dev);	
	ge_set_cur_region(osdmsg_ge_dev, osdmsg_dst_surface, 0);
	ge_set_cur_region(osdmsg_ge_dev, osdmsg_src_surface, id);
	if (sys_ic_get_rev_id()>= IC_REV_2)//m3202
		ge_blt(osdmsg_ge_dev, osdmsg_dst_surface, osdmsg_src_surface, &dst_rect, &src_rect, GE_BLT_SRC2_VALID);
	else
		ge_blt(osdmsg_ge_dev, osdmsg_dst_surface, osdmsg_src_surface, &dst_rect, &src_rect, GE_BLT_SRC2_VALID|GE_BLT_RGB2YUV);
	ge_unlock(osdmsg_ge_dev);
#else
	UINT16 *dst_buffer;
	UINT32 dst_pitch;
	UINT8 pixel_bytes = osdmsg_dst_surface->region[0].pixel_bits/8;

	dst_pitch = osdmsg_dst_surface->region[0].rect.width*pixel_bytes;

	dst_buffer = (UINT16 *)osdmsg_dst_surface->region[0].addr + (dst_rect.top*osdmsg_dst_surface->region[0].rect.width) + dst_rect.left;
	memcpy(dst_buffer, osdmsg_src_surface->region[id].addr, dst_rect.width*dst_rect.height*pixel_bytes);
#endif

	ge_unlock(osdmsg_ge_dev);
}

static void osdmsg_ge_fill(UINT8 id, UINT16 color)
{
	ge_rect_t dst_rect, src_rect;
	UINT32 i;
	UINT16 *buffer[2] = {osdmsg_buf_top, osdmsg_buf_btm};
	
	dst_rect.left = osdmsg_rect[id].left;
	dst_rect.top = osdmsg_rect[id].top;
	dst_rect.width = osdmsg_rect[id].width;
	dst_rect.height = osdmsg_rect[id].height;

	src_rect.left = osdmsg_rect[id].left;
	src_rect.top = osdmsg_rect[id].top;
	src_rect.width = osdmsg_rect[id].width;
	src_rect.height = osdmsg_rect[id].height;

	ge_lock(osdmsg_ge_dev);
	ge_set_cur_region(osdmsg_ge_dev, osdmsg_dst_surface, 0);
	ge_set_cur_region(osdmsg_ge_dev, osdmsg_src_surface, id);	
	ge_fill_rect(osdmsg_ge_dev, osdmsg_src_surface, &src_rect, color);
	ge_fill_rect(osdmsg_ge_dev, osdmsg_dst_surface, &dst_rect, color);
	ge_unlock(osdmsg_ge_dev);
}

static void osdmsg_ge_clear(UINT8 id)
{
	ge_rect_t dst_rect, src_rect;
	
	dst_rect.left = osdmsg_rect[id].left;
	dst_rect.top = osdmsg_rect[id].top;
	dst_rect.width = osdmsg_rect[id].width;
	dst_rect.height = osdmsg_rect[id].height;

	src_rect.left = osdmsg_rect[id].left;
	src_rect.top = osdmsg_rect[id].top;
	src_rect.width = osdmsg_rect[id].width;
	src_rect.height = osdmsg_rect[id].height;

	ge_lock(osdmsg_ge_dev);
	ge_set_cur_region(osdmsg_ge_dev, osdmsg_dst_surface, 0);
	ge_set_cur_region(osdmsg_ge_dev, osdmsg_src_surface, id);	
	ge_fill_rect(osdmsg_ge_dev, osdmsg_src_surface, &src_rect, 0x0000);
	ge_fill_rect(osdmsg_ge_dev, osdmsg_dst_surface, &dst_rect, 0x0000);
	ge_unlock(osdmsg_ge_dev);
}

static UINT16 COMMB16ToUINT16(UINT16 pVal)
{
	UINT16 wRes=0;
	UINT16 high,low;

	if(pVal == 0)
		return 0;
	high=((pVal&0xff00)>>8);
	low=((pVal&0x00ff)<<8);
	
	wRes=(low|high);
	return wRes;	
}

static void scroll_string(UINT8 style, UINT16* str,ge_rect_t rc,UINT16 fontColor,UINT16 backColor,UINT8 delay)
{
	UINT8 c,width,regionID;
	UINT8 fontArray[1024];
	UINT8 *fontbuf;
	UINT16 *tbuf;
	UINT8 *pDataBitmap;
	UINT16 ch, i,j,k,times=0;
	ID_RSC RscLibId;
	OBJECTINFO RscLibInfo;
	OBJECTINFO *pRscLibInfo = &RscLibInfo;

	if(str == NULL)
		return;

	if(delay == 0)
		delay = 30;  //default speed is 30 

	if (CA_OSD_ON_TOP == style)
	{
		tbuf = osdmsg_disp_top_buf;
		fontbuf = fontBuf_top;
		regionID = OSDMSG_REGION_TOP;
	}
	else
	{
		tbuf = osdmsg_disp_btm_buf;
		fontbuf = fontBuf_btm;
		regionID = OSDMSG_REGION_BTM;
	}

	while(*str != 0)
	{
		ch = COMMB16ToUINT16(*str);
		RscLibId = OSD_GetDefaultFontLib(ch);
		pDataBitmap = OSD_GetRscObjData_ext(RscLibId, ch, &RscLibInfo,fontbuf, 512);	
		if(NULL == pDataBitmap)
		{
			str++;
			continue;
		}
		
		//MEMCPY(fontArray,pDataBitmap,72);
		//copy ch bitmap to scroll fontArry, size should be ch's w*h, not 72
		MEMCPY(fontArray,pDataBitmap,(RscLibInfo.m_objAttr.m_wWidth*RscLibInfo.m_objAttr.m_wHeight)/8);
		width = RscLibInfo.m_objAttr.m_wActualWidth;
		for(i=0;i<width;i++)
		{   
			//OSDMSG_PRINT("loop s: %d\n", osal_get_tick());
			if(((FALSE == osdmsg_top_disp_flag)&& (CA_OSD_ON_TOP == style))
				||((FALSE == osdmsg_bottom_disp_flag)&& (CA_OSD_ON_BOTTOM== style)))
			{
				OSDMSG_PRINT("@@@CA_OSD_ON_TOP msg stop!\n");
				MEMSET(fontbuf, 0, 512);
				osdmsg_ge_clear(regionID);
				return;
			}
		
			for(k=0;k<rc.height;k++)
			{
				UINT8 cmask=0;
				UINT16 pos=k*rc.width;
				for(j=0;j<rc.width-1;j++)
				{
					tbuf[pos+j] = tbuf[pos+j+1];
				}	

				if( k <(rc.height - RscLibInfo.m_objAttr.m_wHeight))
				{
					tbuf[pos+rc.width-1]  = backColor;
					continue;
				}
				c=fontArray[(k-(rc.height -  RscLibInfo.m_objAttr.m_wHeight))*(RscLibInfo.m_objAttr.m_wWidth/8)+i/8];
				cmask = 0x80 >> (i%8);
				tbuf[pos+rc.width-1] = (c &cmask)?fontColor:backColor;
			}

			osdmsg_ge_draw(regionID);

			while(times<delay)
			{
				osal_task_sleep(1);
				times++;
				if(times >= delay)
				{
					times = 0;
					break;
				}
			}
//			OSDMSG_PRINT("loop e: %d\n", osal_get_tick());
		}
		str++;
	}
}

static void osdmsg_disp_top()
{
	UINT8 *buf = NULL;
	UINT32 msgcode = 0;

	if (FALSE == osdmsg_top_disp_flag)
		return;

	while(1)
	{
		if (TRUE == osdmsg_top_disp_flag)
		{
			osdmsg_ge_fill(OSDMSG_REGION_TOP, OSDMSG_BK_COLOR);
			scroll_string(CA_OSD_ON_TOP, osdmsg_buf_top, osdmsg_rect[OSDMSG_REGION_TOP], OSDMSG_FONT_COLOR, OSDMSG_BK_COLOR, 10);
		}
		else
		{
			osdmsg_ge_clear(OSDMSG_REGION_TOP);
			break;
		}
	}
	osdmsg_ge_clear(OSDMSG_REGION_TOP);
}

static void osdmsg_disp_bottom()
{
	UINT8 *buf = NULL;
	UINT32 msgcode = 0;

	if (FALSE == osdmsg_bottom_disp_flag)
		return;
	
	while(1)
	{
		if (TRUE == osdmsg_bottom_disp_flag)
		{
			scroll_string(CA_OSD_ON_BOTTOM, osdmsg_buf_btm, osdmsg_rect[OSDMSG_REGION_BTM], OSDMSG_FONT_COLOR, OSDMSG_BK_COLOR, 10);
		}
		else
		{
			osdmsg_ge_clear(OSDMSG_REGION_BTM);
			break;
		}
	}
	osdmsg_ge_clear(OSDMSG_REGION_BTM);
}

static void osdmsg_top_task(void)
{	
	while(1)
	{
		osdmsg_disp_top();
		osal_task_sleep(300);
	}
}

static void osdmsg_bottom_task(void)
{	
	while(1)
	{
		osdmsg_disp_bottom();
		osal_task_sleep(300);
	}
}

static BOOL cas_osdmsg_task_init(void)
{
	ID  			osdmsg_scroll_task_id = OSAL_INVALID_ID;
	OSAL_T_CMBF		t_cmbf;
	OSAL_T_CTSK		t_ctsk;

	osdmsg_mutex_id = osal_mutex_create();
	if (OSAL_INVALID_ID == osdmsg_mutex_id)
	{
		OSDMSG_PRINT("!!!osdmsg create mutex failed\n");
		//asm("sdbbp");
		return FALSE;
	}

	t_ctsk.stksz	= OSDMSG_SCROLL_TASK_STACKSIZE;
	t_ctsk.quantum	= OSDMSG_SCROLL_TASK_QUANTUM;
	t_ctsk.itskpri	= OSAL_PRI_NORMAL;
	t_ctsk.name[0]	= 's';
	t_ctsk.name[1]	= 'b';
	t_ctsk.name[2]	= '1';
	t_ctsk.task = (FP)osdmsg_top_task;
	osdmsg_scroll_task_id = osal_task_create(&t_ctsk);
	if(OSAL_INVALID_ID == osdmsg_scroll_task_id)
	{
		OSDMSG_PRINT("cre_tsk osdmsg_scroll_task_id failed\n");
		osal_mutex_delete(osdmsg_mutex_id);
		return FALSE;
	}

	t_ctsk.name[0]	= 's';
	t_ctsk.name[1]	= 'b';
	t_ctsk.name[2]	= '2';
	t_ctsk.task = (FP)osdmsg_bottom_task;
	osdmsg_scroll_task_id = osal_task_create(&t_ctsk);
	if(OSAL_INVALID_ID == osdmsg_scroll_task_id)
	{
		OSDMSG_PRINT("cre_tsk osdmsg_scroll_task_id failed\n");
		osal_mutex_delete(osdmsg_mutex_id);
		return FALSE;
	}
	return TRUE;
}

extern ge_surface_desc_t *mis_ge_sf_dst;
BOOL cas_osdmsg_init(void)
{
	ge_surface_desc_t surface_para;
	ge_region_t region_para;
	BOOL ret;
	int i;
	
	osdmsg_ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
	if (osdmsg_ge_dev == NULL)
		return FALSE;

	osdmsg_dst_surface = mis_ge_sf_dst;

	#if 1
	/* Create source surface */
	MEMSET((UINT8 *)&surface_para, 0, sizeof(ge_surface_desc_t));
	surface_para.width = 720;
	surface_para.height = 576;
	surface_para.pitch = 720*2;
	surface_para.endian = GE_PIXEL_LITTLE_ENDIAN;
	surface_para.alpha_range = GE_LARGE_ALPHA;
	osdmsg_src_surface = ge_create_surface(osdmsg_ge_dev, &surface_para, 0, GMA_MEM_SURFACE);
	if(NULL == osdmsg_src_surface){
		return FALSE;
	}
	for (i=0; i<2; i++){
		region_para.pixel_fmt = GE_PF_ARGB1555;
		region_para.glob_alpha_en = 0;
		region_para.glob_alpha = 0x7f;
		region_para.pallet_seg = 0;
		region_para.pitch = 720*2;
		if (SUCCESS != ge_create_region(osdmsg_ge_dev, osdmsg_src_surface, 0, &osdmsg_rect[i], &region_para)){
			return FALSE;
		}  
	}
	osdmsg_src_surface->region[OSDMSG_REGION_TOP].addr = osdmsg_disp_top_buf;
	osdmsg_src_surface->region[OSDMSG_REGION_BTM].addr = osdmsg_disp_btm_buf;	
	
	for (i=0; i<2; i++){
		osdmsg_ge_clear(i);
	}
	ge_show_onoff(osdmsg_ge_dev, osdmsg_dst_surface, 1);
	#endif

	ret = cas_osdmsg_task_init();
	
	return ret;
}
UINT8 creat_ge_surface(void)
{
	ge_surface_desc_t surface_para;
	ge_region_t region_para;
	BOOL ret;
	int i;

	struct ge_device *ge_dev;

	struct osd_device  *g_osd_layer2_dev;
    g_osd_layer2_dev =(struct osd_device*)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
    if (g_osd_layer2_dev == NULL)
	{
		libc_printf("%s() OSD_layer2 is not be opened!\n", __FUNCTION__);
	}
    else
    {
		OSDDrv_ShowOnOff((HANDLE)g_osd_layer2_dev,0);
        OSDDrv_Close((HANDLE)g_osd_layer2_dev);
        libc_printf("%s() OSD_layer2 is closed!\n", __FUNCTION__);
		//osal_task_sleep(100);
    }    

	
	ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
	if (ge_dev == NULL)
	{
		libc_printf("%s() dev_get_by_id for dev failed!\n", __FUNCTION__);
		return FALSE;
	}

	if (SUCCESS != ge_open(ge_dev))
	{
		libc_printf("%s() ge_open failed!\n", __FUNCTION__);
		return FALSE;
	}		
	
	MEMSET((UINT8 *)&surface_para, 0, sizeof(ge_surface_desc_t));
	surface_para.width = 720;
	surface_para.height = 576;
	surface_para.pitch = 720*2;
	surface_para.endian = GE_PIXEL_LITTLE_ENDIAN;
	surface_para.alpha_range = GE_LARGE_ALPHA;
	ge_dst_surface = ge_create_surface(ge_dev, &surface_para, 0, GMA_HW_SURFACE_1);
	if (ge_dst_surface == NULL)
	{
		libc_printf("%s() ge_create_surface for dst failed!\n", __FUNCTION__);
		return FALSE;
	}

	region_para.pixel_fmt = GE_PF_ARGB1555;
	region_para.glob_alpha_en = 0;
	region_para.glob_alpha = 0x7f;
	region_para.pallet_seg = 0;
	region_para.pitch = 720*2;
   
	if (SUCCESS != ge_create_region(ge_dev, ge_dst_surface, 0, &ge_dst_region, &region_para))
	{
		libc_printf("%s() ge_create_region for dst failed!\n", __FUNCTION__);
		return FALSE;
	}	
	ge_fill_rect(ge_dev, ge_dst_surface, &ge_dst_region,0); 
	return TRUE;
}

ge_surface_desc_t * get_ge_dst_surface(void)
{
	return ge_dst_surface;
}
BOOL cas_osdmsg_send (UINT32 msg_type, UINT32 msg_code )
{
	ER		ret_val;
	UINT32	msg = msg_type;

	ret_val = osal_msgqueue_send(osdmsg_mbf_id, &msg, sizeof(msg), 0);
	if ( OSAL_E_OK != ret_val )
	{
		OSDMSG_PRINT("Err: %s-failed!\n", __FUNCTION__);
		return FALSE;
	}

	return TRUE;
}

#else
//#include <api/libosd/osd_lib.h>
#include <api/libge/osd_lib.h>
#include <api/libge/osd_common.h>
#include <api/libcas/gy/cas_gy.h>

#include "win_cas_com.h"

#define OSDMSG_DEBUG
#ifdef OSDMSG_DEBUG
#define OSDMSG_PRINT	 libc_printf
#else
#define OSDMSG_PRINT(...) do{}while(0)
#endif
/*******************************************************************************
*	Objects definition
*******************************************************************************/
#define OSDMSG_BUFSIZE (GY_OSDMSG_LEN+160)

static UINT16 osdmsg_buf_top[OSDMSG_BUFSIZE];
static UINT16 osdmsg_buf_btm[OSDMSG_BUFSIZE];

static UINT16 osdmsg_buf_top_b[OSDMSG_BUFSIZE];
static UINT16 osdmsg_buf_btm_b[OSDMSG_BUFSIZE];

static BOOL osd_top_new = FALSE;
static BOOL osd_btm_new = FALSE;

static UINT8 fontBuf_top[512];
static UINT8 fontBuf_btm[512];

static BOOL osdmsg_top_disp_flag = FALSE;
static BOOL osdmsg_bottom_disp_flag = FALSE;

static VSCR vscr_osdmsg_top;
static VSCR vscr_osdmsg_btm;
UINT32 osdmsg_mutex_id;

static struct osd_device  *g_osd_dev2;

static ge_surface_desc_t *ge_dst_surface = NULL, *ge_src_surface = NULL;

static ge_rect_t ge_dst_region = {0, 0, 720, 576};
OSAL_ID osdmsg_mbf_id = OSAL_INVALID_ID;
enum
{
	OSDMSG_REGION_TOP = 0,
	OSDMSG_REGION_BTM = 1,
};

enum
{
	OSDMSG_SCROLL_TASK_STACKSIZE = 10*1024,
	OSDMSG_SCROLL_TASK_QUANTUM = 10,
};


//copy the osd data to local disp buff, and set the flag
BOOL osdmsg_display_start (UINT8 bystyle)
{
	ER		ret_val;
	UINT8 	style = bystyle,i;
	UINT8	temp[GY_OSDMSG_LEN+2] ={0,};
	UINT16 	len;
	
	if (CA_OSD_ON_TOP == style)
	{
		OSDMSG_PRINT("@@@@SHOW message on the top!\n");
		osal_mutex_lock(osdmsg_mutex_id, OSAL_WAIT_FOREVER_TIME);
		
		if(TRUE == osdmsg_top_disp_flag)
			osd_top_new = TRUE;		
		
		MEMSET(osdmsg_buf_top, 0, sizeof(osdmsg_buf_top));
		//CDSTBCA_GetOSDMessage(&osdmsg_buf_top);
		len = ComUniStrLen(osdmsg_buf_top);
		for(i = 0;i <100;i++) //68: how many space fill the whole line
		{
			osdmsg_buf_top[len+i] = 0x2000;
		}
		osal_mutex_unlock(osdmsg_mutex_id);
		osdmsg_top_disp_flag = TRUE;
	}
	else 
	{
		OSDMSG_PRINT("@@@@SHOW message on the Bottom!\n");
		osal_mutex_lock(osdmsg_mutex_id, OSAL_WAIT_FOREVER_TIME);
		
		if(TRUE == osdmsg_bottom_disp_flag)
			osd_btm_new = TRUE;
			
		MEMSET(osdmsg_buf_btm, 0, sizeof(osdmsg_buf_btm));
		//CDSTBCA_GetOSDMessage(&osdmsg_buf_btm);
		len = ComUniStrLen(osdmsg_buf_btm);
		for(i = 0;i <100;i++)
		{
			osdmsg_buf_btm[len+i] = 0x2000;//0x2000;
		}
		osal_mutex_unlock(osdmsg_mutex_id);
		osdmsg_bottom_disp_flag = TRUE;
	}

	return TRUE;
}

//clear local disp buff flag
BOOL osdmsg_display_stop(UINT8 bystyle)
{
	if (CA_OSD_ON_TOP == bystyle)
	{
		OSDMSG_PRINT("@@@@stop message on the top!\n");
		osdmsg_top_disp_flag = FALSE;
		osal_task_sleep(50);
	}
	else 
	{
		OSDMSG_PRINT("@@@@stop message on the Bottom!\n");
		osdmsg_bottom_disp_flag = FALSE;
		osal_task_sleep(50);
	}

	return TRUE;
}


static void osdmsg_updatevscr(VSCR* pVscr,UINT8 regionID)
{	
	if(pVscr->lpbScr!=NULL)
	{
		if(pVscr->updatePending)
		{
			OSDDrv_RegionWrite((HANDLE)g_osd_dev2, regionID, pVscr,&pVscr->vR);
			pVscr->updatePending = 0;
		}
	}
}

static void osdmsg_clear_msg(UINT8 style, VSCR* pVscr, OSD_RECT prect)
{
	if (CA_OSD_ON_TOP == style)
	{
		MEMSET(vscr_osdmsg_top.lpbScr,OSD_TRANSPARENT_COLOR_BYTE,prect.uHeight*prect.uWidth);
		OSD_SetRect2 (&vscr_osdmsg_top.vR,&prect);
		vscr_osdmsg_top.updatePending = 1;
		osdmsg_updatevscr(&vscr_osdmsg_top,OSDMSG_REGION_TOP);   
	}
	else
	{
		MEMSET(vscr_osdmsg_btm.lpbScr,OSD_TRANSPARENT_COLOR_BYTE,prect.uHeight*prect.uWidth);
		OSD_SetRect2 (&vscr_osdmsg_btm.vR,&prect);
		vscr_osdmsg_btm.updatePending = 1;
		osdmsg_updatevscr(&vscr_osdmsg_btm,OSDMSG_REGION_BTM);  
	}
}

static UINT16 COMMB16ToUINT16(UINT16 pVal)
{
	UINT16 wRes=0;
	UINT16 high,low;

	if(pVal == 0)
		return 0;
	high=((pVal&0xff00)>>8);
	low=((pVal&0x00ff)<<8);
	
	wRes=(low|high);
	return wRes;	
}

extern UINT8 *OSD_GetRscObjData_ext(UINT16 wLibClass, UINT16 uIndex , OBJECTINFO *pObjectInfo, UINT8 *font_bitmap_ext, UINT32 font_bitmap_size);
void scroll_string(UINT8 style, UINT16* str,OSD_RECT rc,UINT8 fontColor,UINT8 backColor,UINT8 delay)
{
	UINT8 c,width,regionID;
	UINT8 fontArray[1024];
	UINT8 *fontbuf;
	UINT8 *pDataBitmap;
	UINT16 ch, i,j,k,times=0;
	ID_RSC RscLibId;
	VSCR* vScr;
	OBJECTINFO RscLibInfo;
	OBJECTINFO *pRscLibInfo = &RscLibInfo;

    
	if(str == NULL)
		return;

	if(delay == 0)
		delay = 30;  //default speed is 30 

	if (CA_OSD_ON_TOP == style)
	{
		vScr = &vscr_osdmsg_top;
		fontbuf = fontBuf_top;
		regionID = OSDMSG_REGION_TOP;
	}
	else
	{
		vScr = &vscr_osdmsg_btm;
		fontbuf = fontBuf_btm;
		regionID = OSDMSG_REGION_BTM;
	}

	while(*str != 0)
	{
		ch = COMMB16ToUINT16(*str);
		RscLibId = OSD_GetDefaultFontLib(ch);
		pDataBitmap = OSD_GetRscObjData_ext(RscLibId, ch, &RscLibInfo,fontbuf,512);	
		if(NULL == pDataBitmap)
		{
			str++;
			continue;
		}
		
		//MEMCPY(fontArray,pDataBitmap,72);
		//copy ch bitmap to scroll fontArry, size should be ch's w*h, not 72
		MEMCPY(fontArray,pDataBitmap,(RscLibInfo.m_objAttr.m_wWidth*RscLibInfo.m_objAttr.m_wHeight)/8);
		width = RscLibInfo.m_objAttr.m_wActualWidth;
		for(i=0;i<width;i++)
		{   
			//OSDMSG_PRINT("loop s: %d\n", osal_get_tick());
			if(((FALSE == osdmsg_top_disp_flag) && (CA_OSD_ON_TOP == style))
				||((FALSE == osdmsg_bottom_disp_flag) && (CA_OSD_ON_BOTTOM== style)))
			{
				OSDMSG_PRINT("@@@CA_OSD_ON_TOP msg stop!\n");
				MEMSET(fontbuf, 0, 512);
				osdmsg_clear_msg(style, vScr, rc);
				return;
			}
		
			for(k=0;k<rc.uHeight;k++)
			{
				UINT8 cmask=0;
				UINT16 pos=k*rc.uWidth;
				for(j=0;j<rc.uWidth-1;j++)
				{
					vScr->lpbScr[pos+j] = vScr->lpbScr[pos+j+1];
				}	

				if( k <(rc.uHeight -  RscLibInfo.m_objAttr.m_wHeight))
				{
					vScr->lpbScr[pos+rc.uWidth-1]  = backColor;
					continue;
				}
				c=fontArray[(k-(rc.uHeight -  RscLibInfo.m_objAttr.m_wHeight))*(RscLibInfo.m_objAttr.m_wWidth/8)+i/8];
				cmask = 0x80 >> (i%8) ;
				vScr->lpbScr[pos+rc.uWidth-1] = (c &cmask)?fontColor:backColor;	
			}

			OSD_SetRect2 ( &vScr->vR,&rc);
			vScr->updatePending = 1;
			osdmsg_updatevscr(vScr,regionID);

			while(times<delay)
			{
				osal_task_sleep(1);
				times++;
				if(times >= delay)
				{
					times = 0;
					break;
				}
			} 	
			//OSDMSG_PRINT("loop e: %d\n", osal_get_tick());
		}

		str++;
	}
}


void osdmsg_disp_top()
{
	UINT8 *pscr = NULL;
	UINT8 bg_color = OSD_TRANSPARENT_COLOR, font_color = 0xef;
	UINT32 msgcode = 0;

	OSD_RECT rect = {12,0, 700, 28};

	if (FALSE == osdmsg_top_disp_flag)
		return;
	
	if(FALSE == osd_top_new)	
	{
		pscr =MALLOC(rect.uHeight*rect.uWidth);
		if (NULL == pscr)
		{
			ASSERT(0);
		}
		vscr_osdmsg_top.lpbScr = pscr;

		//disp background
		MEMSET(vscr_osdmsg_top.lpbScr,bg_color,rect.uHeight*rect.uWidth);
		MEMCPY(osdmsg_buf_top_b, osdmsg_buf_top, sizeof(osdmsg_buf_top));
	}
	while(1)
	{
		if (TRUE == osdmsg_top_disp_flag)
		{
			scroll_string(CA_OSD_ON_TOP, osdmsg_buf_top_b, rect, font_color, bg_color, 6);//10);//speed up scroll
			osal_mutex_lock(osdmsg_mutex_id, OSAL_WAIT_FOREVER_TIME);
			if(TRUE == osd_top_new)
			{
				//disp background
				MEMSET(vscr_osdmsg_top.lpbScr,bg_color,rect.uHeight*rect.uWidth);
				MEMCPY(osdmsg_buf_top_b, osdmsg_buf_top, sizeof(osdmsg_buf_top));
				osd_top_new = FALSE;
			}
			osal_mutex_unlock(osdmsg_mutex_id);
		}
		else
		{
			osdmsg_clear_msg(CA_OSD_ON_TOP, &vscr_osdmsg_top, rect);
			break;
		}
	}
	
	osdmsg_clear_msg(CA_OSD_ON_TOP, &vscr_osdmsg_top, rect);
	//msgcode = ( UINT32 ) ((CA_OSD_ON_TOP&0xffffff) | (TFCA_MSG_UPDATE_WINDOW << 24));
	//ap_send_msg ( CTRL_MSG_SUBTYPE_STATUS_MCAS, msgcode, FALSE );
	
	if (NULL != pscr)
	{
		FREE(pscr);		
		pscr = NULL;
	}
}

void osdmsg_disp_bottom()
{
	UINT8 *pscr = NULL;
	UINT8 bg_color = OSD_TRANSPARENT_COLOR, font_color = 0xef;
	UINT32 msgcode = 0;

	OSD_RECT rect = {12,0, 700, 28};

	if (FALSE == osdmsg_bottom_disp_flag)
		return;
	
	if(FALSE == osd_btm_new)
	{
		pscr =MALLOC(rect.uHeight*rect.uWidth);
		if (NULL == pscr)
		{
			ASSERT(0);
		}
		vscr_osdmsg_btm.lpbScr = pscr;

		//disp background
		MEMSET(vscr_osdmsg_btm.lpbScr,bg_color,rect.uHeight*rect.uWidth);
		MEMCPY(osdmsg_buf_btm_b, osdmsg_buf_btm, sizeof(osdmsg_buf_btm));
	}
	
	while(1)
	{
		if (TRUE == osdmsg_bottom_disp_flag)
		{
			scroll_string(CA_OSD_ON_BOTTOM, osdmsg_buf_btm_b, rect, font_color, bg_color,6);//10);//speed up scroll
			osal_mutex_lock(osdmsg_mutex_id, OSAL_WAIT_FOREVER_TIME);
			if(TRUE == osd_btm_new)
			{
				osd_btm_new = FALSE;
				//disp background
				MEMSET(vscr_osdmsg_btm.lpbScr,bg_color,rect.uHeight*rect.uWidth);
				MEMCPY(osdmsg_buf_btm_b, osdmsg_buf_btm, sizeof(osdmsg_buf_btm));
			}
			osal_mutex_unlock(osdmsg_mutex_id);
		}
		else
		{
			osdmsg_clear_msg(CA_OSD_ON_BOTTOM, &vscr_osdmsg_btm, rect);
			break;
		}
	}

	osdmsg_clear_msg(CA_OSD_ON_BOTTOM, &vscr_osdmsg_btm, rect);
	//msgcode = ( UINT32 ) ((CA_OSD_ON_BOTTOM&0xffffff) | (TFCA_MSG_UPDATE_WINDOW << 24));
	//ap_send_msg ( CTRL_MSG_SUBTYPE_STATUS_MCAS, msgcode, FALSE );
	
	if (NULL != pscr)
	{
		FREE(pscr);	
		pscr = NULL;
	}
}

void osdmsg_top_task(void)
{	
	while(1)
	{
		osdmsg_disp_top();
		osal_task_sleep(300);
	}
}

void osdmsg_bottom_task(void)
{	
	while(1)
	{
		osdmsg_disp_bottom();
		osal_task_sleep(300);
	}
}

static UINT8 g_osd_layer2_inited = 0;

void ap_osd_layer2_exit()
{
	struct OSDRect rct = {0, 0, 720, 28};

    if(!g_osd_layer2_inited)
        return;
    
	osdmsg_display_stop(CA_OSD_ON_TOP);
	osdmsg_display_stop(CA_OSD_ON_BOTTOM);	
	g_osd_dev2 = (struct osd_device*)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
	OSDDrv_RegionFill((HANDLE)g_osd_dev2,OSDMSG_REGION_TOP,&rct, 0);	
	OSDDrv_RegionFill((HANDLE)g_osd_dev2,OSDMSG_REGION_BTM,&rct, 0);
    OSDDrv_DeleteRegion((HANDLE)g_osd_dev2, OSDMSG_REGION_TOP);
    OSDDrv_DeleteRegion((HANDLE)g_osd_dev2, OSDMSG_REGION_BTM);
	OSDDrv_Close((HANDLE)g_osd_dev2);
	g_osd_layer2_inited = 0;

}
void ap_osd_layer2_init()
{
	UINT8 *p_pallette;
	struct OSDPara tOpenPara;
	struct OSDRect region1, region2, rc1, rc2;

	g_osd_dev2 = (struct osd_device*)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
	if(g_osd_layer2_inited)
		return;

	tOpenPara.eMode = OSD_256_COLOR;
	tOpenPara.uGAlphaEnable = 0;
	tOpenPara.uGAlpha = 0x0F;
	tOpenPara.uPalletteSel = 0;
	OSDDrv_Open((HANDLE)g_osd_dev2, &tOpenPara);
	osal_task_sleep(20);
	OSDDrv_IoCtl((HANDLE)g_osd_dev2, OSD_IO_SET_TRANS_COLOR, OSD_TRANSPARENT_COLOR);

	//create region 0 for Top osd msg
	region1.uLeft = 0;
	region1.uTop = 20; 
	region1.uWidth = 720;
	region1.uHeight = 28;
	rc1.uLeft = 0;
	rc1.uTop= 0;
	rc1.uWidth = region1.uWidth;
	rc1.uHeight = region1.uHeight;
	OSDDrv_CreateRegion((HANDLE)g_osd_dev2, OSDMSG_REGION_TOP, &region1, NULL);   

	//create region 1 for bottom osd msg
	region2.uLeft = 0;
	region2.uTop = 530; 
	region2.uWidth = 720;
	region2.uHeight = 28;
	rc2.uLeft = 0;
	rc2.uTop = 0;
	rc2.uWidth = region2.uWidth;
	rc2.uHeight = region2.uHeight;
	OSDDrv_CreateRegion((HANDLE)g_osd_dev2, OSDMSG_REGION_BTM, &region2, NULL);   

	p_pallette = OSD_GetRscPallette(0x4080 | 0);
	OSDDrv_SetPallette((HANDLE)g_osd_dev2, p_pallette, COLOR_N, OSDDRV_YCBCR);
	OSDDrv_RegionFill((HANDLE)g_osd_dev2,OSDMSG_REGION_TOP,&rc1,OSD_TRANSPARENT_COLOR);
	OSDDrv_RegionFill((HANDLE)g_osd_dev2,OSDMSG_REGION_BTM,&rc2,OSD_TRANSPARENT_COLOR);	
	OSDDrv_ShowOnOff((HANDLE)g_osd_dev2,OSDDRV_ON);
	
	g_osd_layer2_inited = 1;
}
BOOL cas_osdmsg_task_init(void)
{
	ID  			osdmsg_scroll_task_id = OSAL_INVALID_ID;
	OSAL_T_CMBF		t_cmbf;
	OSAL_T_CTSK		t_ctsk;

	osdmsg_mutex_id = osal_mutex_create();
	if (OSAL_INVALID_ID == osdmsg_mutex_id)
	{
		OSDMSG_PRINT("!!!osdmsg create mutex failed\n");
		//asm("sdbbp");
		return FALSE;
	}

	t_ctsk.stksz	= OSDMSG_SCROLL_TASK_STACKSIZE;
	t_ctsk.quantum = OSDMSG_SCROLL_TASK_QUANTUM;
	t_ctsk.itskpri	= OSAL_PRI_NORMAL;
	t_ctsk.name[0]	= 's';
	t_ctsk.name[1]	= 'b';
	t_ctsk.name[2]	= '1';
	t_ctsk.task = (FP)osdmsg_top_task;
	osdmsg_scroll_task_id = osal_task_create(&t_ctsk);
	if(OSAL_INVALID_ID == osdmsg_scroll_task_id)
	{
		OSDMSG_PRINT("cre_tsk osdmsg_scroll_task_id failed\n");
		osal_mutex_delete(osdmsg_mutex_id);
		return FALSE;
	}

	t_ctsk.name[0]	= 's';
	t_ctsk.name[1]	= 'b';
	t_ctsk.name[2]	= '2';
	t_ctsk.task = (FP)osdmsg_bottom_task;
	osdmsg_scroll_task_id = osal_task_create(&t_ctsk);
	if(OSAL_INVALID_ID == osdmsg_scroll_task_id)
	{
		OSDMSG_PRINT("cre_tsk osdmsg_scroll_task_id failed\n");
		osal_mutex_delete(osdmsg_mutex_id);
		return FALSE;
	}
	//ap_osd_layer2_init();
	return TRUE;
}
UINT8 creat_ge_surface(void)
{
	ge_surface_desc_t surface_para;
	ge_region_t region_para;
	BOOL ret;
	int i;

	struct ge_device *ge_dev;

	struct osd_device  *g_osd_layer2_dev;
    g_osd_layer2_dev =(struct osd_device*)dev_get_by_id(HLD_DEV_TYPE_OSD, 1);
    if (g_osd_layer2_dev == NULL)
	{
		libc_printf("%s() OSD_layer2 is not be opened!\n", __FUNCTION__);
	}
    else
    {
		OSDDrv_ShowOnOff((HANDLE)g_osd_layer2_dev,0);
        OSDDrv_Close((HANDLE)g_osd_layer2_dev);
        libc_printf("%s() OSD_layer2 is closed!\n", __FUNCTION__);
		//osal_task_sleep(100);
    }    

	
	ge_dev = (struct ge_device *)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
	if (ge_dev == NULL)
	{
		libc_printf("%s() dev_get_by_id for dev failed!\n", __FUNCTION__);
		return FALSE;
	}

	if (SUCCESS != ge_open(ge_dev))
	{
		libc_printf("%s() ge_open failed!\n", __FUNCTION__);
		return FALSE;
	}		
	
	MEMSET((UINT8 *)&surface_para, 0, sizeof(ge_surface_desc_t));
	surface_para.width = 720;
	surface_para.height = 576;
	surface_para.pitch = 720*2;
	surface_para.endian = GE_PIXEL_LITTLE_ENDIAN;
	surface_para.alpha_range = GE_LARGE_ALPHA;
	ge_dst_surface = ge_create_surface(ge_dev, &surface_para, 0, GMA_HW_SURFACE_1);
	if (ge_dst_surface == NULL)
	{
		libc_printf("%s() ge_create_surface for dst failed!\n", __FUNCTION__);
		return FALSE;
	}

	region_para.pixel_fmt = GE_PF_ARGB1555;
	region_para.glob_alpha_en = 0;
	region_para.glob_alpha = 0x7f;
	region_para.pallet_seg = 0;
	region_para.pitch = 720*2;
   
	if (SUCCESS != ge_create_region(ge_dev, ge_dst_surface, 0, &ge_dst_region, &region_para))
	{
		libc_printf("%s() ge_create_region for dst failed!\n", __FUNCTION__);
		return FALSE;
	}	
	ge_fill_rect(ge_dev, ge_dst_surface, &ge_dst_region,0); 
	return TRUE;
}

ge_surface_desc_t * get_ge_dst_surface(void)
{
	return ge_dst_surface;
}
BOOL cas_osdmsg_send (UINT32 msg_type, UINT32 msg_code )
{
	ER		ret_val;
	UINT32	msg = msg_type;

	ret_val = osal_msgqueue_send(osdmsg_mbf_id, &msg, sizeof(msg), 0);
	if ( OSAL_E_OK != ret_val )
	{
		OSDMSG_PRINT("Err: %s-failed!\n", __FUNCTION__);
		return FALSE;
	}

	return TRUE;
}

#endif




