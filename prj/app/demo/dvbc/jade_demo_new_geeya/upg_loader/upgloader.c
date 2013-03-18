/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    bootloader.c
*
*    Description:    This file contains all globe micros and functions declare
*		             of Flash boot loader.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Oct.28.2003     Justin Wu       Ver 0.1    Create file.
 *  2.  2006.5.16       Justin Wu       Ver 0.2    STO chunk support & clean up.
*****************************************************************************/
#include <sys_config.h>
#include <sys_parameters.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libchunk/chunk.h>
#include <bus/sci/sci.h>
#include <bus/flash/flash.h>
#include <hal/hal_gpio.h>
#include <hal/hal_mem.h>
#include <hld/pan/pan_dev.h>
#include <hld/pan/pan.h>

#include <hld/nim/nim_tuner.h>
#include <hld/dmx/dmx.h>
#include <bus/i2c/i2c.h>
#include <bus/tsi/tsi.h>
#include <api/libota/lib_ota.h>
#include <hld/scart/scart.h>
#include <hld/osd/osddrv.h>
#include <api/libloader/ld_osd_lib.h>
#include <api/libloader/ld_boot_info.h>
#include <api/libosd/osd_vkey.h>
#include <hld/dis/vpo.h>
#include <hld/decv/decv.h>
#include <hld/decv/decv_avc.h>
#include <hld/nim/nim_dev.h>
#include <api/libpvr/lib_pvr.h>

#include <api/libstbinfo/stb_info_data.h>
#include "../copper_common/boot_system_data.h"
#include "../copper_common/stb_data.h"
#include "resource/pallette/palettes_array.h"
#include "key.h"
#include "boot_osd_rsc.h"
#include "dev_handle.h"

#ifdef SUPPORT_HDMI_OUTPUT
#include <bus/hdmi/m36/hdmi_api.h>
#endif

#ifdef HDTV_SUPPORT_OTA
#include <bus/hdmi/m36/hdmi_dev.h>
#include <bus/hdmi/m36/hdmi_api.h>
#endif
#include <bus/dog/dog.h>

#define CONFIGDATA_BASE 0xAFC00000


#ifdef HW_SET_BOOTLOGO
#undef HW_SET_BOOTLOGO
#endif
#ifdef HW_SET_GPIO
#undef HW_SET_GPIO
#endif
#ifdef HW_SET_BOOTTYPE
#undef HW_SET_BOOTTYPE
#endif
#ifdef HW_SET_PANTYPE
#undef HW_SET_PANTYPE
#endif

#define HW_SET_BOOTLOGO 0x9C
#define HW_SET_GPIO 0x104
#define HW_SET_BOOTTYPE 0x12D
#define BOOT_TYPE_MASK 0x80
#define HW_SET_PANTYPE 0x9F
#define PAN_TYPE_MASK 0xE0

#define US_TICKS        (SYS_CPU_CLOCK / 2000000)
#define WAIT_300MS      (300000 * US_TICKS)
#define WAIT_500MS		(500000 * US_TICKS)
#define WAIT_1MS        (1000   * US_TICKS)

struct pan_device *g_pan_dev = NULL;
struct sto_device *g_sto_dev = NULL;
struct nim_device*nim_dev = NULL;
struct dmx_device *dmx_dev = NULL;
struct scart_device* scart_dev = NULL;
struct osd_device *osd_dev = NULL;

struct vdec_device*  g_decv_dev = NULL;
struct vpo_device*   g_vpo_dev = NULL;

struct ge_device	*g_ge_dev = NULL;

BOOL g_upg_stb_info_ok;
extern BOOL g_upg_preburn_flag;

UINT8   pre_tv_mode = 0xFF;
BOOL g_resouce_err=FALSE;
UINT32  g_crc_bad;	

//define for libm
int __errno;

UINT32 ap_hk_to_vk(UINT32 start, UINT32 key_info, UINT32* vkey );

void osal_delay_ms(UINT32 ms)
{
    UINT32 i;
    for (i = 0; i < ms; i++)
        osal_delay(1000);
}

void upg_sys_reboot()
{
	UINT32 expire_time = 500;		
#if(SYS_MAIN_BOARD == BOARD_S3202_DEMO_01V03)
    HAL_GPIO_BIT_DIR_SET(7, HAL_GPIO_O_DIR);
    HAL_GPIO_BIT_SET(7, 0);
    osal_task_sleep(500);
#elif(SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01)
    HAL_GPIO_BIT_DIR_SET(0, HAL_GPIO_O_DIR);
    HAL_GPIO_BIT_SET(0, 0);
    osal_task_sleep(500);
#elif(SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01||SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01)
    HAL_GPIO_BIT_DIR_SET(32, HAL_GPIO_O_DIR);
    HAL_GPIO_BIT_SET(32, 0);
    osal_task_sleep(500);
#endif
	dog_m3327e_attach(0);		
	dog_mode_set(0, DOG_MODE_WATCHDOG, expire_time, NULL);		
	while(1);
}

/***********************************
AV
************************************/
RET_CODE avInit()
{
	OSAL_ID tsk_id = OSAL_INVALID_ID;
	struct VP_InitInfo tVPInitInfo;

	dmx_dev = (struct dmx_device*)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);

#ifndef MINI_DRIVER_SUPPORT
	g_vpo_dev = (struct vpo_device*)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
	g_decv_dev = (struct vdec_device*)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
#endif    

	if (dmx_dev == NULL)
	{
		ASSERT(0);
	}

	if (RET_SUCCESS != dmx_open(dmx_dev))
	{
		UPG_PRINTF("system_hw_init: Open demux device failed!\n");
		return RET_FAILURE;
	}

	dmx_io_control(dmx_dev, DMX_SWITCH_TO_UNI_BUF, 0);

#ifndef MINI_DRIVER_SUPPORT
	if(RET_SUCCESS!=vdec_open(g_decv_dev))
		ASSERT(0);

	InitVPPara(&tVPInitInfo);

	if(RET_SUCCESS!=vpo_open(g_vpo_dev,&tVPInitInfo))
		ASSERT(0);

 	/* Vdec ignore hd service */
	vdec_io_control(g_decv_dev, VDEC_IO_DISCARD_HD_SERVICE, 1);
#endif

	return RET_SUCCESS;
}

void avStart()
{
	struct VDecPIPInfo vInitInfo;
	struct MPSource_CallBack vMPCallBack;
	struct PIPSource_CallBack vPIPCallBack;
	struct VDec_PullDown_Opr tPullDownCallBack;

	if (RET_SUCCESS != dmx_start(dmx_dev))
	{
		UPG_PRINTF("system_hw_init: Start demux device failed!\n");
		return;
	}

#ifndef MINI_DRIVER_SUPPORT
	vdec_set_output(g_decv_dev, MP_MODE, &vInitInfo, &vMPCallBack, &vPIPCallBack);
	vpo_win_mode(g_vpo_dev, VPO_MAINWIN, &vMPCallBack, &vPIPCallBack);

#ifdef VDEC_AV_SYNC
	vdec_sync_mode(g_decv_dev, 0x00, VDEC_SYNC_I | VDEC_SYNC_P | VDEC_SYNC_B);
#endif

#ifdef VIDEO_OUTPUT_BETTER_VISION
	vpo_ioctl(g_vpo_dev, VPO_IO_COMPENENT_UPSAMP_REPEAT, 1);
#endif

#ifdef CHANNEL_CHANGE_VIDEO_FAST_SHOW
	vdec_io_control(g_decv_dev, VDEC_IO_FAST_CHANNEL_CHANGE, 1);
#endif
#endif
}

/***********************************
TV MODE
************************************/
void api_osd_set_tv_system(BOOL ntsc)
{
	UINT32 eOSDSys;

	if(ntsc == FALSE)
		eOSDSys = OSD_PAL;
	else 
		eOSDSys = OSD_NTSC;
	
	OSDDrv_Scale((HANDLE)osd_dev,OSD_VSCALE_OFF,(UINT32)&eOSDSys);
}

UINT8 api_video_get_tvout(void)
{
	enum TVSystem out_sys;
	UINT8 tv_out;

	vpo_ioctl(g_vpo_dev, VPO_IO_GET_OUT_MODE, (UINT32)(&out_sys));

	switch (out_sys)
	{
		case PAL:
			tv_out = TV_MODE_PAL;
			break;
		case NTSC:
			tv_out = TV_MODE_NTSC358;
			break;
		case NTSC_443:
			tv_out = TV_MODE_NTSC443;
			break;
		case PAL_M:
			tv_out = TV_MODE_PAL_M;
			break;
		default:
			tv_out = TV_MODE_PAL;
	}

	return tv_out;
}

void api_video_set_tvout(UINT8 tv_mode)
{
	UINT8 TV_Src, TV_Out;
	enum TVSystem tvsys, out_sys;
	BOOL ntsc;
	
	switch (tv_mode)
	{
		case TV_MODE_PAL:
			tvsys = PAL;
			break;
		case TV_MODE_PAL_M:
			tvsys = PAL_M;
			break;
		case TV_MODE_PAL_N:
			tvsys = PAL_N;
			break;
		case TV_MODE_NTSC358:
			tvsys = NTSC;
			break;
		case TV_MODE_SECAM:
			tvsys = SECAM;
			break;
		default:
			tvsys = PAL;
	}
	//if( eTVSys != NTSC_443)
	vpo_tvsys(g_vpo_dev, tvsys);

	vpo_ioctl(g_vpo_dev, VPO_IO_GET_OUT_MODE, (UINT32)(&out_sys));


	if (out_sys == NTSC || out_sys == PAL_M || out_sys == PAL_60 || out_sys == NTSC_443)
		ntsc = TRUE;
	else
		ntsc = FALSE;

	api_osd_set_tv_system(ntsc);
}

void boot_osd_init(void)
{
	struct OSD_InitInfo osd_init_info;
	BOOL	bProgressive = FALSE;
	union UPG_INFO *upg_info = NULL;

	osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 0);

	osd_init_info.p_handle = osd_dev;
	osd_init_info.tOpenPara.eMode = OSD_256_COLOR;
	osd_init_info.tOpenPara.uGAlphaEnable = IF_GLOBAL_ALPHA;
	osd_init_info.tOpenPara.uGAlpha = 0x0F;
	osd_init_info.tOpenPara.uPalletteSel = 0;
	osd_init_info.osdrect.uLeft = (720-OSD_MAX_WIDTH)>>1;
	osd_init_info.osdrect.uTop  = (576-OSD_MAX_HEIGHT)>>1;
	osd_init_info.osdrect.uWidth = OSD_MAX_WIDTH;
	osd_init_info.osdrect.uHeight = OSD_MAX_HEIGHT;
	
	osal_task_sleep(10);
	OSD_init(&osd_init_info,palette_array[0]);
}

static void system_hw_init(void)
{
	//struct QPSK_TUNER_CONFIG_API	Tuner_API;
	struct scart_init_param scart_init;
	UINT8 pan_type;
	UINT8 *t1, *t2;
 	UINT32 i;
	UINT32 data;

	/* pin mux */
	pinmux_init();

	/* Uart */
	uart_init();
	
	/* Panel */
	front_panel_init((UINT8*)(SYS_FLASH_BASE_ADDR+HW_SET_GPIO));

	/* Flash */
	flash_init();

	extern char* get_core_ver(void);
	FIXED_PRINTF("%s\r\n", get_core_ver());

	/* I2C */
	i2c_init();

	/* tuner */
	nim_tsi_config();

	/* dmx */
	dmx_dev_attach();

	nim_dev = dev_get_by_id(HLD_DEV_TYPE_NIM, 0);
	if(SUCCESS != nim_open(nim_dev))
	{
		UPG_PRINTF("device: [nim 0] open failed!\n");	
		int reset_count = 0;
		do
		{
			UPG_PRINTF("Reset tuner %d time!!!\n", reset_count + 2);
			//M3602_DB02V01_reset_tuner();
			reset_count++;
			if(SUCCESS == nim_open(nim_dev))
				break;
			if (reset_count >= 10)
			{
				UPG_PRINTF("ERROR: reset nim failed!!!\n");
				break;
			}
			osal_task_sleep(100);
		} while (1);
	}
	//nim_quick_channel_change(nim_dev, 47400, 6875, 6); //set as tuner default value.

#ifndef MINI_DRIVER_SUPPORT
	/* ge */
	ge_dev_attach();

	/* vpo */
	vpo_dev_attach();

	/* vedio dec */	
	vdec_dev_attach();

#else
	/* osd */
	osd_dev_attach();
#endif

 }

void load_rsc_data()
{
	UINT32 pre_crc_chk = 0;	
	INT32 ret;	

//	if(RSC_font_init()==FALSE)
	{
//		UPG_PRINTF("load font fail, start to load backup font.\n");
		OSD_SetLangEnvironment(CHINESE_ENV);
		RSC_font_bak_init();
	}
}

void usbloader(UINT32 param1, UINT32 param2)
{
#ifdef USB_UPG_SUPPORT
	INT32 usb_ret = 0;
	union UPG_INFO *upg_info = NULL;
	upg_info = (union UPG_INFO *)stb_info_data_get_upginfo();

	stb_pan_display(" USB", 4);

	//MG_Setup_CRC_Table();

	//TODO: clear trigger_flag
	upg_info->c_info.upg_trigger_type = STB_INFO_UPG_TRIGGER_NONE;
	stb_info_data_save();

	usb_ret = usb_upgrade();
	UPG_PRINTF("usbloader: USB-upgrade finished(%d)!\n", usb_ret);

	while(1);
#endif
}

void otaloader(UINT32 param1, UINT32 param2)
{
	UINT32 boot_tick;
	UINT8 i;
	UINT32 chid;
	UINT32 offset;
	UINT8 *main_entry;
	INT32 ota_ret = 0;
	UINT8 force_ota = 0;
	UINT8 ch;
	UINT32 index;

	stb_pan_display(" OTA", 4);

	ota_ret = ota_upgrade(force_ota);
	UPG_PRINTF("otaloader: OTA finished(%d)!\n", ota_ret);

	if (RET_SUCCESS != dmx_stop(dmx_dev))
	{
		UPG_PRINTF("otaloader: Stop dmx device failed!\n");
	}
	if (RET_SUCCESS != dmx_close(dmx_dev))
	{
		UPG_PRINTF("otaloader: Close dmx device failed!\n");
	}

	if (RET_SUCCESS != nim_close(nim_dev))
	{
		UPG_PRINTF("otaloader: Close nim device failed!\n");
	}	

	g_crc_bad = sto_chunk_check();

	UPG_PRINTF("%s:%d: g_crc_bad:0x%08x\n", __FUNCTION__, __LINE__,g_crc_bad);

	if( (0 == g_crc_bad) || (2 == ota_ret) )
	{
#ifdef PANEL_DISPLAY
		pan_close(g_pan_dev);
#endif
		//sys_watchdog_reboot();
		upg_sys_reboot();
	}

	stb_pan_display("FAIL", 4);

	while(1);
}

#define UPG_TYPE_USB	1
#define UPG_TYPE_OTA	2

void upgloader(UINT32 param1, UINT32 param2)
{
	struct pan_key *pan_key=NULL;
	UINT32 tick;
	UINT32 hkey;
	BOOL enter_menu = FALSE;
	UINT8 upg_type = UPG_TYPE_OTA;
	UINT32 pre_crc_chk = 0;	
	INT32 ret;	
	STB_INFO_DATA *stb_data = NULL;

	//MG_Setup_CRC_Table();

#if 0
	/* if get front panel key, enter upg menu */
	tick = read_tsc();
	do
	{
		osal_task_sleep(50);
		pan_key = pan_get_key(g_pan_dev, 1);
		if(pan_key != NULL)
		{
			if(pan_key->type == PAN_KEY_TYPE_PANEL)
			{
				if(pan_key->count > 2)
				{	
					enter_menu = TRUE;
					break;
				}
			}
		}
	}while (read_tsc() - tick < WAIT_500MS);	

	stb_pan_display(enter_menu ? " UPG" : " OTA", 4);

	key_task_resume();
#endif

	stb_data = stb_info_data_get();
	if((stb_data != NULL) && (stb_data->state & STB_INFO_DATA_STATE_RESET))
	{
		UPG_PRINTF("not complete stb data, now fullfil it!\n");
		if(stb_data_factory_complete() != 0)
		{
			UPG_PRINTF("%s:%d, ERROR fullfil stb_info_data!\n", __FUNCTION__, __LINE__);
			stb_data->state = STB_INFO_DATA_STATE_LIMIT;			
		}
		else
		{
			stb_data->state = STB_INFO_DATA_STATE_NORMAL;			
		}
		stb_info_data_save();
	}
	if((stb_data != NULL) && (stb_data->state & STB_INFO_DATA_STATE_LIMIT))
	{
		g_upg_stb_info_ok = FALSE;		
	}

	if((stb_data != NULL) && (stb_data->state & STB_INFO_DATA_STATE_PREBURN))
	{
		g_upg_preburn_flag = TRUE;
	}

	/* OSD init*/
	boot_osd_init();

	/* load font data */
	load_rsc_data();

#ifdef USB_UPG_SUPPORT	
	if((stb_data == NULL) || (stb_data->upg_info.c_info.upg_trigger_type & STB_INFO_UPG_TRIGGER_BOOTKEY)
		|| (stb_data->upg_info.c_info.upg_trigger_type == STB_INFO_UPG_TRIGGER_NONE))
	{
		upg_type = upg_menu_proc();
	}
#endif

	switch(upg_type)
	{
	case UPG_TYPE_USB:
		usbloader(0,0);
		break;
	case UPG_TYPE_OTA:
		otaloader(0,0);
		break;
	}

}


OSAL_ID boot_mbf_id;
#define BOOT_MBF_SIZE		24

extern void __SEE_ROM_START();
extern void __SEE_ROM_END();

void  see_boot(UINT32 addr)
{
	if(*(volatile UINT32 *)(0xb8000200) != 0xb8000280)
		return;

	*(volatile UINT32 *)(0xb8000200) = addr; 
	*(volatile UINT32 *)(0xb8000220) |= 0x2; 
	if(addr&0x80000000)
	return;	
boot_code:	
	asm volatile("                  \
		lui   $8, 0xb800; \
		addiu $8, 0x0200; \
		jr    $8;         \
		nop;               \
		");

}

void* upg_memcpy(void *dest, const void *src, unsigned int len)
{
	//the source and destination overlap, this function does not ensure that the original source UINT8s in the overlapping region are copied before being overwritten.
	UINT32 dwCount;
	UINT8 *DestBuf, *SrcBuf;

	DestBuf = (UINT8 *)dest,
			  SrcBuf = (UINT8 *)src;

	for (dwCount = 0;dwCount < len;dwCount++)
		*(DestBuf + dwCount) = *(SrcBuf + dwCount);

	return dest;
}

void AppInit(void)
{
	OSAL_T_CMBF task_mbf;
	OSAL_T_CTSK task_param;
	OSAL_ID tsk_id = OSAL_INVALID_ID;
	union UPG_INFO *upg_info = NULL;
	STB_INFO_DATA *stb_data;
	STB_INFO_DATA stb_data_temp;

	MG_Setup_CRC_Table();
	
	system_hw_init();

	/* load stb info data */
	if(stb_data_load() != SUCCESS)
	{
		set_pan_display_type(PAN_DISP_LED_ONLY);

		UPG_PRINTF("no valid stb data, now need init it!\n");
		if(stb_data_factory_init() != 0)
		{
			g_upg_stb_info_ok = FALSE;
		}
		else
		{
			g_upg_stb_info_ok = TRUE;
		}

		stb_pan_display_revert();
		set_pan_display_type(PAN_DISP_LED_BUF);
	}
	else
	{
		g_upg_stb_info_ok = TRUE;
	}

	if(g_upg_stb_info_ok == FALSE)
	{
		stb_info_data_init(&stb_data_temp);
		stb_data = &stb_data_temp;		
	}
	else
	{
		stb_data = stb_info_data_get();
	}
	
	upg_info = (union UPG_INFO *)&stb_data->upg_info;

#ifdef MINI_DRIVER_SUPPORT
	/* VPO */
	vpo_init(stb_data);
#endif
	

	OSD_rsc_reg_cb(OSD_GetLangEnvironment,OSD_GetObjInfo,\
	OSD_GetRscObjData,OSD_GetDefaultFontLib,OSDExt_GetMsgLibId);
	OSD_SetLangEnvironment(upg_info->c_info.osd_lang);

	pre_tv_mode = upg_info->c_info.boot_tv_mode;

	avInit();
	avStart();

#ifndef MINI_DRIVER_SUPPORT
	api_video_set_tvout(pre_tv_mode);
#endif

	task_mbf.bufsz	=  BOOT_MBF_SIZE * sizeof(BootMsg_t);
	task_mbf.maxmsz	= sizeof(BootMsg_t);
	task_mbf.name[0]		= 'u';
	task_mbf.name[1]		= 'p';
	task_mbf.name[2]		= 'g';

	boot_mbf_id = OSAL_INVALID_ID;
	boot_mbf_id = osal_msgqueue_create(&task_mbf);
	if(OSAL_INVALID_ID == boot_mbf_id)
	{
		UPG_PRINTF("cre_mbf boot_mbf_id failed\n");
		return;
	}
	
	task_param.task = upgloader;
	task_param.name[0] = 'U';
	task_param.name[1] = 'P';
	task_param.name[2] = 'G';
	task_param.stksz=0x8000;	

	task_param.itskpri=OSAL_PRI_NORMAL;
	task_param.quantum=50 ;
	task_param.para1=0;
	task_param.para2=0;
	tsk_id = osal_task_create(&task_param);
	if (OSAL_INVALID_ID == tsk_id)
	{
		UPG_PRINTF("Create task <otaloader> failed!\n");
		return;
	}

	key_Init();
	//key_task_suspend();
		/*enable M3202C POK for flash power loss protection*/
	if((1==sys_ic_is_M3202())&&(sys_ic_get_rev_id()>=IC_REV_4))
  		pdd_init(0, 0);
}

void boot_clear_all_message(void)
{
	ER				retVal;
	OSAL_ID temp;
	UINT32	i,msgSize,msg_num;
	BootMsg_t	msg,*p_msg;
	OSAL_T_CMBF		t_cmbf;
	BootMsg_t p_msgbuffer[BOOT_MBF_SIZE];

	msg_num = 0;
	p_msg = p_msgbuffer;
	while(1)
	{
		retVal = osal_msgqueue_receive((VP)&msg, &msgSize, boot_mbf_id,0);
	        if(retVal != E_OK)
	            break;
		if(msg.priority == 1)
		{
			memcpy(p_msg,&msg,sizeof(BootMsg_t));//copy to temporary buffer
			msg_num++;
			p_msg++;
		}
	}	

	t_cmbf.bufsz	=  BOOT_MBF_SIZE * sizeof(BootMsg_t);
	t_cmbf.maxmsz	= sizeof(BootMsg_t);
	t_cmbf.name[0]		= 'o';
	t_cmbf.name[1]		= 't';
	t_cmbf.name[2]		= 'a';
	temp = OSAL_INVALID_ID;
	temp = osal_msgqueue_create(&t_cmbf);
	if(temp == OSAL_INVALID_ID)
	{
		return;
	}
	
	p_msg = p_msgbuffer;
	for(i = 0;i < msg_num;i++)
	{
		if(OSAL_E_OK == osal_msgqueue_send(temp,p_msg,sizeof(BootMsg_t), 0))
		{
	        	p_msg++;
		}
	}
	
	osal_task_dispatch_off();
	osal_msgqueue_delete(boot_mbf_id);
	boot_mbf_id = temp;
	osal_task_dispatch_on();

}

BOOL boot_send_msg(BootMsgType_t msg_type, UINT32 msg_code, BOOL if_clear_buffer)
{
	ER				ret_val;
	BootMsg_t	boot_msg;

	boot_msg.msgType     = msg_type;
	boot_msg.msgCode     = msg_code;
	boot_msg.priority 	= if_clear_buffer;

	ret_val = osal_msgqueue_send(boot_mbf_id,&boot_msg,sizeof(BootMsg_t), 0);		
	if(OSAL_E_OK != ret_val)
	{
		if(if_clear_buffer)
		{
			boot_clear_all_message();
			ret_val = osal_msgqueue_send(boot_mbf_id,&boot_msg,sizeof(BootMsg_t), 0);
		}
		else
			return FALSE;
	}
	else
		return TRUE;

	return ret_val;
}

UINT32 boot_get_key_msg(void)
{
	OSAL_ER			retVal;
	BootMsg_t	msg;
	UINT32			msgSize;
	UINT32 hkey = INVALID_HK;
	UINT32  vkey;
		
	retVal = osal_msgqueue_receive(&msg,&msgSize,boot_mbf_id,100);
	if(OSAL_E_OK != retVal)
		return INVALID_MSG;
	
	if( msg.msgType <= CTRL_MSG_TYPE_KEY)
	{
		ap_hk_to_vk(0,msg.msgCode, &vkey);
		hkey = msg.msgCode;
		return hkey;
	}
		
	return hkey;
}

extern IR_Key_Map_t ir_key_maps[];

UINT32 ap_hk_to_vk(UINT32 start, UINT32 key_info, UINT32* vkey )
{
	UINT16 i, j, n;
	UINT16 key_count;

	*vkey = V_KEY_NULL;

	for (j = 0; j < IRP_NUM; j++)
	{
		for (i = start; i < ir_key_maps[j].irkey_num; i++)
		{
			key_count = (ir_key_maps[j].key_map[i].key_info & 0xFF0000);
			if (((ir_key_maps[j].key_map[i].key_info & 0xFF00FFFF) == (key_info & 0xFF00FFFF))
			        && (key_count == (key_info & 0xFF0000) || (0 == key_count)))
			{
				*vkey = ir_key_maps[j].key_map[i].ui_vkey;
				break;
			}
		}
	}
	
	return (i + 1);
}

