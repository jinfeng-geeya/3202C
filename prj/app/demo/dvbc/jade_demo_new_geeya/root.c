/*****************************************************************************
 *	Copyrights(C) 2003 Acer Laboratries Inc. All Rights Reserved.
 *
 *	FILE NAME:		root.c
 *
 *	DESCRIPTION:	 This file contains application's root function - AppInit().
 *				 It inits all modules used in the application and creates all the tasks
 *				used. It also send a command to play the current tv program.
 *
 *
 *	HISTORY:
 *		Date 		Author 		Version 	Notes
 *		=========	=========	=========	=========
 *		2003-04-19	Xianyu Nie	Ver 0.1		Create File
 *		..............	......	.		....			.....
 *		2004-11-11 	Xionghua		Ver 0.2		modify for m3327
 *****************************************************************************/

#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <osal/osal_timer.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libchunk/chunk.h>
#include <osal/osal.h>
#include <hal/hal_mem.h>
#include <hal/hal_gpio.h>
#include <bus/i2c/i2c.h>
#include <bus/sci/sci.h>
#include <bus/tsi/tsi.h>
#include <hld/pan/pan.h>
#include <hld/pan/pan_key.h>
#include <hld/hld_dev.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <hld/deca/deca_dev.h>
#include <hld/deca/deca.h>
#include <hld/snd/snd_dev.h>
#include <hld/snd/snd.h>
#include <hld/dis/vpo.h>
#include <hld/decv/vdec_driver.h>
#include <hld/osd/osddrv.h>
#include <hld/ge/ge.h>
#include <hld/smc/smc.h>

#include <api/libpub/lib_pub.h>
#include <api/libpub/lib_as.h>
#include <hld/nim/Nim_tuner.h>
#include <sys_parameters.h>
//#include <api/libosd/osd_lib.h>
#if defined(CAS_TYPE) && (CAS_TYPE == CAS_CONAX)
#include <api/libcas/mcas.h>
#elif(defined(CAS_TYPE) && (CAS_TYPE == CAS_ABEL))
#include <api/libcas/abel/abel_mcas.h>
#include <api/libcas/abel/NN.h>
#endif
#include <api/libpub/lib_device_manage.h>

#include "win_com.h"
#include "copper_common/com_api.h"


#if (SUBTITLE_ON == 1)
#include <hld/sdec/sdec.h>
#include <api/libsubt/lib_subt.h>
#include <api/libsubt/Subt_osd.h>
#endif
#include "copper_common/stb_data.h"
#ifdef AD_SANZHOU
#include "sanzhou_ad/adprocess.h"
#endif
#include <../src/lib/libapplet/libbootupg3/lib_ash.h>
#include "copper_common/stb_hwinfo.h"
/*******************************************************
 * macro define
 ********************************************************/
//disable sgdma for 3202C test
//#define SGDMA_DISABLE
//enable TDS debug info
#ifdef _ALI_SYSDBG_ENABLE_
#define SYS_DEBUG_ENABLE
#endif


/*******************************************************
 * extern declare
 ********************************************************/
extern enum Output_Frame_Ret_Code VDec_Output_Frame(struct Display_Info *pDisplay_info);
extern BOOL VDec_Release_FrBuf(UINT8 top_index, UINT8 bottom_index);
extern INT32 sto_eeprom_init();
extern INT32 ota_table_request();

/*******************************************************
 * struct and variable
 ********************************************************/
struct vdec_device *g_decv_dev = NULL;
struct deca_device *g_deca_dev = NULL;
struct dmx_device *g_dmx_dev = NULL;
struct nim_device *g_nim_dev = NULL;
struct nim_device *g_nim_dev2 = NULL;
struct snd_device *g_snd_dev = NULL;
struct sdec_device *g_sdec_dev = NULL;
struct vbi_device *g_vbi_dev = NULL;
struct osd_device *g_osd_dev = NULL;
struct sto_device *g_sto_dev = NULL;
struct vpo_device *g_vpo_dev = NULL;
struct pan_device *g_pan_dev = NULL;
//add for libbootupg3 module compile error 
unsigned char g_crc_bad=0;
//struct sto_device *flash_dev=NULL;
//end
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_ABEL)
struct osd_device *g_osd_dev2 = NULL;
#endif
#endif

UINT8 *vbv_buffer_add = NULL;
T_VBIRequest VBIRequest;
struct QAM_TUNER_CONFIG_API g_QAM_Tuner_Cfg;

int __errno;

unsigned short blogo_switch = 1;	// 1: not off logo when bootloader-->maincode

unsigned short blogo_tvformat =0;
unsigned short blogo_height = 0;
static BOOL g_fastplay_flag = FALSE;
#define HW_BOOTLOGO_SIZE 0x9c
extern struct tve_adjust g_tve_adjust_table[];
extern struct tve_adjust g_tve_adjust_table_adv[];

extern UINT8 board_vdac_config[VDAC_NUM_MAX];

/*******************************************************
 * internal functions
 ********************************************************/
#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_ABEL)
#define MAX_STB_ID_SIZE 30 
//RSA Key 516byte + key reference 4byte + STB ID (MAX  30byte) = 550byte

UINT16 load_stb_identify(UINT8 *stb_id, UINT16 id_len)
{
	UINT8 buf[MAX_STB_ID_SIZE];
	UINT32 id = 0; //bootloader ID
	UINT8 i=0;
	unsigned long bl_offset;
	
	MEMSET(stb_id, 0, id_len);
	
	//sto_get_data( (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), buf,
	//					(UINT32)sto_chunk_goto(&id,0xFFFFFFFF,1) + CHUNK_HEADER_SIZE+520, MAX_STB_ID_SIZE);

	bl_offset = sto_fetch_long(0 + CHUNK_OFFSET);
	sto_get_data((struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), (UINT8 *)buf, bl_offset - MAX_STB_ID_SIZE, MAX_STB_ID_SIZE);


	while((buf[i] !=0) && (i<MAX_STB_ID_SIZE))
	{
		if(i<id_len-1)
			stb_id[i] = buf[i];
		i++;
	}
	
	return i;
}

static void AbelConfig( )
{

	UINT8 	*link_encytption_keys;
	UINT8 	Key1[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
	UINT8 	Key2[8] = {0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01};
	UINT8 	Key3[8] = {0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x01, 0x23};
	UINT8	Plaintext[8];
	UINT16	 i;
	UINT32 id =0; //bootloader ID
	unsigned long bl_offset;
	
	link_encytption_keys = (UINT8 *)MALLOC(sizeof(UINT8)*520);
	
	//sto_get_data( (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), link_encytption_keys,
						//(UINT32)sto_chunk_goto(&id,0xFFFFFFFF,1) + CHUNK_HEADER_SIZE, 520);

	bl_offset = sto_fetch_long(0 + CHUNK_OFFSET);
	sto_get_data((struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0), (UINT8 *)link_encytption_keys, bl_offset - 550, 520);
		
	 for(i=0; i<520; i++)
	 {
	 	if(link_encytption_keys[i] != 0xFF)
	 	{
	 		abel_key_is_existed();
	 		break;
	 	}
	 }

	 for(i=0; i<520; i+=8)
	{
		TripleDES_Decryption(link_encytption_keys+i, Plaintext, Key1, Key2, Key3);       
		memcpy(link_encytption_keys+i, Plaintext, 8);            
	 } 

	 set_private_key((R_RSA_PUBLIC_KEY*)link_encytption_keys);
	 set_link_encryption_key_ref(&link_encytption_keys[516]);
	 	
	 free(link_encytption_keys);

	 api_set_inbox_chunk_id(0x08F70100);

}
#endif
#endif



BOOL init_vbv_buffer()
{
	#if(8 == SYS_SDRAM_SIZE)
		vbv_buffer_add = (UINT8*)__MM_VBV_START_ADDR;
    #else
		vbv_buffer_add = MALLOC(__MM_VBV_LEN+256);
    #endif

	if(vbv_buffer_add == NULL)
		return FALSE;
	else
		return TRUE;
}


static void system_hw_init()
{
	struct sto_device *eep = NULL;
	
	pin_mux_config(0, 0);
	
	sci_16550uart_attach(1); /* Use just 1 UART for RS232 */
	sci_mode_set(SCI_FOR_RS232, 115200, SCI_PARITY_EVEN); //SCI_PARITY_NONE

    board_i2c_set();

	/* Init front panel */
	extern struct pan_configuration pan_config;
	#if(FRONT_PANEL_TYPE == FP_CT1642)
		pan_ct1642_attach(&pan_config);
	#endif
	
    pan_attach();
	//pan_sm1668_attach(&pan_config);
	g_pan_dev = (struct pan_device*)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);

	pan_open(g_pan_dev);
	set_pan_display_type(PAN_DISP_LED_BUF);
	pan_display(g_pan_dev, " on ", 4);
    
    osal_task_sleep(10);//for 3202 boot up issue at the first time after burn flash
	/* Flash */
	flash_info_sl_init();
	sto_local_sflash_attach(NULL);
	g_sto_dev = (struct sto_device*)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	/*M3202*/
	if ((1 == sys_ic_is_M3202())&&(sys_ic_get_rev_id() <=IC_REV_2))
	{
		*((volatile UINT32*)0xb8008098) &= ~0xc0000000;
		if (g_sto_dev->totol_size <= 0x200000)
			*((volatile UINT32*)0xb8008098) |= 0;
		else if (g_sto_dev->totol_size > 0x200000 && g_sto_dev->totol_size <= 0x400000)
			*((volatile UINT32*)0xb8008098) |= 0x40000000;
		else if (g_sto_dev->totol_size > 0x400000 && g_sto_dev->totol_size <= 0x800000)
			*((volatile UINT32*)0xb8008098) |= 0x80000000;
		else
			*((volatile UINT32*)0xb8008098) |= 0xc0000000;
	}
	/*M3202C*/
	else if ((1 == sys_ic_is_M3202())&&(sys_ic_get_rev_id() >=IC_REV_4))
	{
		*((volatile UINT32*)0xb8000098) &= ~0xc0000000;
		if (g_sto_dev->totol_size <= 0x200000)
			*((volatile UINT32*)0xb8000098) |= 0;
		else if (g_sto_dev->totol_size > 0x200000 && g_sto_dev->totol_size <= 0x400000)
			*((volatile UINT32*)0xb8000098) |= 0x40000000;
		else if (g_sto_dev->totol_size > 0x400000 && g_sto_dev->totol_size <= 0x800000)
			*((volatile UINT32*)0xb8000098) |= 0x80000000;
		else
			*((volatile UINT32*)0xb8000098) |= 0xc0000000;	
	}
	sto_open ( g_sto_dev );
	sto_chunk_init(0, g_sto_dev->totol_size); //sto_eeprom_init();

	flash_identify();
	chunk_reg_bootloader_id(STB_CHUNK_BOOTLOADER_ID);

	//eeprom initialization.

#if 0//(CAS_TYPE==CAS_IRDETO)
	sto_eeprom_init();
	eep = (struct sto_device *)dev_get_by_name("STO_EEPROM_0");
	sto_open(eep);
#endif

	/* TSI */
	if(sys_ic_get_rev_id() >= IC_REV_2)
	{
#ifndef SSI_TS_OUT_IN_SET
		tsi_mode_set(TSI_SPI_0, 0x83);
		tsi_select(TSI_DMX_0,TSI_SPI_0);
#else
		tsi_mode_set(TSI_SSI_0, 0x83);
		tsi_select(TSI_DMX_0,TSI_SSI_0);
#endif
	}

	/* Demodulator & Tuner*/
	if (sys_ic_is_M3202() == 1)
	{
		MEMSET(&g_QAM_Tuner_Cfg, 0, sizeof(struct QAM_TUNER_CONFIG_API));
		nim_s3202_Tuner_Attatch(&g_QAM_Tuner_Cfg);
		nim_s3202_attach(&g_QAM_Tuner_Cfg);
	}
	g_nim_dev = dev_get_by_id ( HLD_DEV_TYPE_NIM, 0 );
	if (SUCCESS != nim_open(g_nim_dev))
		PRINTF("device: [nim 0] open failed!\n");
	/*default turn off nim test task to make fast lock*/
	//set_testThread_off(TRUE);
	/*set nim to risk lock mode to make fast lock*/
	//nim_ioctl_ext(g_nim_dev, NIM_DRIVER_SET_PERF_LEVEL, NIM_PERF_RISK);

	/****special patch for fast paly last service, read system_data from flash and tune nim****/
	UINT32 chunk_id, db_len, db_addr;
	INT32 ret, i;
	chunk_id = 0x04FB0100;
	api_get_chuck_addlen(chunk_id, &db_addr, &db_len);
	db_len = db_len - (64 *1024); 
	init_tmp_info(db_addr + db_len, 64 *1024);
	ret = load_tmp_data((UINT8*)&system_config, sizeof(SYSTEM_DATA));
	if (ret != SUCCESS)
	{
		sys_data_factroy_init();
	}	
	if(system_config.startup_service_valid!=0)
	{
		nim_quick_channel_change(g_nim_dev, system_config.startup_fre, system_config.startup_sym, system_config.startup_qam);
		//libc_printf("nim tune to %d, tick=%d\n",system_config.startup_fre,osal_get_tick());
	}
	/*********************************/

	struct deca_feature_config deca_config;
	MEMSET(&deca_config, 0, sizeof(struct deca_feature_config));
    deca_config.detect_sprt_change=1;
	deca_m33_attach(&deca_config);
/*#ifdef USB_MP_SUPPORT
	deca_ext_dec_enable((struct deca_device*)dev_get_by_id(HLD_DEV_TYPE_DECA, 0), NULL);
#endif*/
#ifdef AC3_DECODE_SUPPORT
	deca_dolby_decode_enable((struct deca_device * )dev_get_by_id(HLD_DEV_TYPE_DECA, 0));
#endif

	snd_config_set();
	

	struct dmx_feature_config dmx_config;
	MEMSET(&dmx_config, 0, sizeof(struct dmx_feature_config));
	dmx_config.mm_map.dmx0_dma_buf = __MM_DMX_AVP_START_ADDR;
	dmx_config.mm_map.dmx1_dma_buf = __MM_DMX_REC_START_ADDR;
	dmx_config.mm_map.ts_audio_cb_len = (__MM_DMX_AUDIO_LEN / 188);
	dmx_config.mm_map.ts_video_cb_len = (__MM_DMX_VIDEO_LEN / 188);
	dmx_config.mm_map.ts_pcr_cb_len = (__MM_DMX_PCR_LEN / 188);
	dmx_config.mm_map.ts_ttx_cb_len = (__MM_DMX_DATA_LEN / 188);
	dmx_config.mm_map.ts_common_cb_len = (__MM_DMX_SI_LEN / 188);
	dmx_config.mm_map.ts_united_cb_len = (__MM_DMX_REC_LEN / 188);
	dmx_config.mm_map.total_pid_filt_num = (4+__MM_DMX_SI_TOTAL_LEN / __MM_DMX_SI_LEN);
	/*M3202*/
	if((1==sys_ic_is_M3202())&&(sys_ic_get_rev_id()<=IC_REV_2))
	{
		dmx_config.video_bitrate_detect = 1; 
	}
	/*M3202C*/
#ifndef SGDMA_DISABLE
	else if((1==sys_ic_is_M3202())&&(sys_ic_get_rev_id()>=IC_REV_4))
	{
		//enable M3202C SGDMA function
		dmx_config.mm_map.xfer_ves_by_dma = TRUE; 
	}
#endif
	dmx_config.sync_times = 3;

#ifdef AV_SYNC_TEST
	//enable manually adjust A/V pts to solve some A/V issue
	dmx_config.adjust_av_pts = 1; 
#endif
	
	dmx_m33_attach(&dmx_config);
	
	dmx_ca_enable((struct dmx_device*)dev_get_by_id(HLD_DEV_TYPE_DMX, 0));

#ifndef SGDMA_DISABLE
	//enable M3202C SGDMA function
	if((1==sys_ic_is_M3202())&&(sys_ic_get_rev_id()>=IC_REV_4))
	{
	/* dma_init() MUST be called AFTER a calling to dmx_m33_attach(), since 
	* dma_init() uses dmx device pointer to init dmx's dma call back
	* functions.
	*/
		dma_init();
	}
#endif

	ge_driver_config_t para_config;
	struct ge_device *ge_dev = NULL;
	MEMSET(&para_config, 0, sizeof(ge_driver_config_t));
	para_config.ge_start_base = __MM_OSD_START_ADDR;
	para_config.GMA1_buf_size = __MM_OSD_LAYER0_LEN;
	para_config.GMA2_buf_size = __MM_OSD_LAYER1_LEN;//810K
	para_config.temp_buf_size = 0;
	para_config.cmd_buf_size = __MM_GE_CMD_LEN;//4K
	para_config.bpolling_cmd_status = TRUE;
	para_config.op_mutex_enable = 1;
	para_config.region_malloc_optimization = 1;
	ge_m33_attach(&para_config);//add ge driver to OS
	//ge device config for ipanel
	ge_dev = (struct ge_device*)dev_get_by_id(HLD_DEV_TYPE_GE, 0);
	/*if the osd buffer is between the 8M and 16M,__MM_OSD_BLOCK_ADDR should be allocated in the memory mapping */
	if (__MM_OSD_START_ADDR > 0xA0800000)
	{
		ge_m33e_enable_16M(ge_dev, (__MM_OSD_BLOCK_ADDR & 0x8FFFFFFF));
	}

	OSD_DRIVER_CONFIG osd_c_config;
	MEMSET(&osd_c_config, 0, sizeof(OSD_DRIVER_CONFIG));
	//osd_c_config.uMemBase = __MM_OSD_START_ADDR;
	//osd_c_config.uMemSize = __MM_OSD_LEN*2;
	osd_c_config.bStaticBlock = TRUE;
	osd_c_config.bDirectDraw = FALSE;
	osd_c_config.bCacheable = TRUE;
	osd_c_config.bVFilterEnable = FALSE;
	osd_c_config.bP2NScaleInNormalPlay = TRUE;
	osd_c_config.bP2NScaleInSubtitlePlay = TRUE;
	osd_c_config.uDViewScaleRatio[OSD_PAL][0] = 384;
	osd_c_config.uDViewScaleRatio[OSD_PAL][1] = 510;
	osd_c_config.uDViewScaleRatio[OSD_NTSC][0] = 384;
	osd_c_config.uDViewScaleRatio[OSD_NTSC][1] = 426;
	osd_c_config.af_par[0].id = 0;
	osd_c_config.af_par[0].vd = 0;
	osd_c_config.af_par[0].af = 0;
	/*open anti-flick for osd layer*/
	osd_c_config.af_par[1].id = 1;
	osd_c_config.af_par[1].vd = 1;
	osd_c_config.af_par[1].af = 1;
	osd_s36_attach("OSD_S36_1", &osd_c_config);
	osd_s36_attach("OSD_S36_0", &osd_c_config);

	struct VP_Feature_Config vp_config;
	struct Tve_Feature_Config tve_config;
	struct vdec_config_par vdec_config_par;

	MEMSET((void*) &vp_config, 0, sizeof(struct VP_Feature_Config));
	MEMSET((void*) &tve_config, 0, sizeof(struct Tve_Feature_Config));
	MEMSET((void*) &vdec_config_par, 0, sizeof(struct vdec_config_par));
	vp_config.bAvoidMosaicByFreezScr = FALSE; /* macro VDEC_AVOID_MOSAIC_BY_FREEZE_SCR */
	vp_config.bMHEG5Enable = FALSE; /* macro  _MHEG5_ENABLE_ */

	vp_config.bOsdMulitLayer = TRUE; /* macro OSD_MULTI_LAYER */
	
	vp_config.bOvershootSolution = FALSE; /* macro VIDEO_OVERSHOOT_SOLUTION */
	vp_config.bP2NDisableMAF = FALSE; /* macro SYS_M3327_TURNOFF_MAF_P2N */
	vp_config.bSupportExtraWin = TRUE; /* macro VIDEO_SUPPORT_EXTRA_DVIEW_WINDOW */
	vp_config.bADPCMEnable = FALSE; /* macro VIDEO_ADPCM_ONOFF(VIDEO_ADPCM_ON:TRUE VIDEO_ADPCM_OFF:FALSE) */
	vp_config.pMPGetMemInfo = vdec_m3327_get_mem_info;
	//vp_config.pSrcAspRatioChange_CallBack = api_Scart_Aspect_Switch;

	tve_config.config = YUV_SMPTE | TVE_TTX_BY_VBI | TVE_CC_BY_VBI;
	tve_config.tve_adjust = g_tve_adjust_table;
	vpo_set_logo_switch(TRUE);
	vpo_m33_attach(&vp_config, &tve_config);

	if (sys_ic_is_M3202() == 1)
		tve_advance_config(g_tve_adjust_table_adv);

	vdec_config_par.mem_map.frm0_Y_size = __MM_FB0_Y_LEN;
	vdec_config_par.mem_map.frm0_C_size = __MM_FB0_C_LEN;
	vdec_config_par.mem_map.frm1_Y_size = __MM_FB1_Y_LEN;
	vdec_config_par.mem_map.frm1_C_size = __MM_FB1_C_LEN;
	vdec_config_par.mem_map.frm2_Y_size = __MM_FB2_Y_LEN;
	vdec_config_par.mem_map.frm2_C_size = __MM_FB2_C_LEN;

	vdec_config_par.mem_map.frm0_Y_start_addr = __MM_FB0_Y_START_ADDR;
	vdec_config_par.mem_map.frm0_C_start_addr = __MM_FB0_C_START_ADDR;
	vdec_config_par.mem_map.frm1_Y_start_addr = __MM_FB1_Y_START_ADDR;
	vdec_config_par.mem_map.frm1_C_start_addr = __MM_FB1_C_START_ADDR;
	vdec_config_par.mem_map.frm2_Y_start_addr = __MM_FB2_Y_START_ADDR;
	vdec_config_par.mem_map.frm2_C_start_addr = __MM_FB2_C_START_ADDR;

	vdec_config_par.mem_map.dvw_frm_size = __MM_DVW_LEN;
	vdec_config_par.mem_map.dvw_frm_start_addr = __MM_DVW_START_ADDR;

	vdec_config_par.mem_map.maf_size = __MM_MAF_LEN;
	vdec_config_par.mem_map.maf_start_addr = __MM_MAF_START_ADDR;

	vdec_config_par.mem_map.vbv_size = ((__MM_VBV_LEN - 4) &0xFFFFFF00);
	vdec_config_par.mem_map.vbv_start_addr = (((UINT32)(vbv_buffer_add+256) &0xfffff00) | 0x80000000);
	vdec_config_par.mem_map.vbv_end_addr = ((vdec_config_par.mem_map.vbv_start_addr) + vdec_config_par.mem_map.vbv_size - 1);

	vdec_config_par.user_data_parsing = TRUE; /* macro USER_DATA_PARSING */
	vdec_config_par.dtg_afd_parsing = FALSE; /* macro DTG_AFD_PARSE */
	vdec_config_par.drop_freerun_pic_before_firstpic_show = FALSE; /* macro VDEC27_DROP_FREERUN_BEFORE_FIRSTSHOW */
	vdec_config_par.reset_hw_directly_when_stop = TRUE; /* macro VDEC27_STOP_REST_HW_DIRECTLY*/
	vdec_config_par.show_hd_service = TRUE; /* macro SUPPORT_SHOW_HD_SERVICE */
	vdec_config_par.still_frm_in_cc = FALSE; /* macro STILL_FRAME_IN_CC */
	vdec_config_par.not_show_mosaic = FALSE; /* macro VDEC_AVOID_MOSAIC_BY_FREEZE_SCR */
	vdec_config_par.adpcm.adpcm_mode = FALSE; /* macro VIDEO_ADPCM_ONOFF(VIDEO_ADPCM_ON:TRUE VIDEO_ADPCM_OFF:FALSE)*/
	vdec_config_par.adpcm.adpcm_ratio = 0;
	vdec_config_par.sml_frm.sml_frm_mode = FALSE; /* macro VDEC27_SML_FRM_ONOFF(VDEC27_SML_FRM_OFF: FALSE   VDEC27_SML_FRM_ON: TRUE)*/
	vdec_config_par.return_multi_freebuf = TRUE; /* macro VDEC27_SUPPORT_RETURN_MULTI_FREEBUF */
	vdec_config_par.sml_frm.sml_frm_size = 0; /* macro VDEC27_SML_FRM_SIZE*/
	vdec_config_par.lm_shiftthreshold = 2; /* macro VDEC27_LM_SHIFTTHRESHOLD*/
	vdec_config_par.vp_init_phase = 0; /* macro DEFAULT_MP_FILTER_ENABLE*/
	vdec_config_par.preview_solution = VDEC27_PREVIEW_DE_SCALE; /* macro VDEC27_PREVIEW_SOLUTION(VDEC27_PREVIEW_VE_SCALE or VDEC27_PREVIEW_DE_SCALE)*/
	vdec_m33_attach(&vdec_config_par);
	struct vdec_device *vdec_config_dev = (struct vdec_device*)dev_get_by_name("DECV_M3327");
	/* macro VIDEO_SUPPORT_EXTRA_DVIEW_WINDOW defined, if which is disable, call API vdec_disable_extra_dview_feature*/
	vdec_enable_extra_dview_feature(vdec_config_dev);

	smartcard_init(0, 0);

#if	(TTX_ON == 1)
	struct vbi_config_par vbi_config;
	MEMSET(&vbi_config,0,sizeof(struct vbi_config_par));

	vbi_config.cc_by_vbi = FALSE;
	vbi_config.erase_unknown_packet = FALSE;
	vbi_config.hamming_24_16_enable = FALSE;
	vbi_config.hamming_8_4_enable = FALSE;
#ifdef SUPPORT_PACKET_26
	vbi_config.parse_packet26_enable = TRUE;
	vbi_config.mem_map.p26_nation_buf_size = __MM_TTX_P26_NATION_LEN;
	vbi_config.mem_map.p26_nation_buf_start_addr = __MM_TTX_P26_NATION_BUF_ADDR;
	vbi_config.mem_map.p26_data_buf_size = __MM_TTX_P26_DATA_LEN;
	vbi_config.mem_map.p26_data_buf_start_addr = __MM_TTX_P26_DATA_BUF_ADDR;
#else
	vbi_config.parse_packet26_enable = FALSE;
#endif
	vbi_config.ttx_by_vbi = TRUE;
	vbi_config.ttx_sub_page = TRUE;
	vbi_config.user_fast_text = FALSE;
	vbi_config.vps_by_vbi = FALSE;
	vbi_config.wss_by_vbi = FALSE;

	vbi_config.mem_map.sbf_start_addr = __MM_TTX_BS_START_ADDR;
	vbi_config.mem_map.sbf_size = __MM_TTX_BS_LEN;
	vbi_config.mem_map.pbf_start_addr = __MM_TTX_PB_START_ADDR;

	vbi_m33_attach(&vbi_config);

	struct vbi_device *vbi_config_dev = NULL;
	vbi_config_dev = dev_get_by_id(HLD_DEV_TYPE_VBI,0);	
	vbi_enable_ttx_by_osd(vbi_config_dev);
    
#endif
#if (SUBTITLE_ON == 1)
	struct subt_config_par libsubt_cfg;
	MEMSET(&libsubt_cfg, 0, sizeof(struct subt_config_par));
	
	libsubt_cfg.osd_blocklink_enable = FALSE;
	libsubt_cfg.g_ps_buf_addr = (UINT8*)__SUBTITLE_BUF_ADDR;
	libsubt_cfg.g_ps_buf_len = __SUBTITLE_BUF_LEN;
	libsubt_cfg.max_subt_height = 576;
	libsubt_cfg.speed_up_subt_enable = TRUE;
	libsubt_cfg.osd_layer_id = 1;
	libsubt_cfg.hd_subtitle_support = 0;
	libsubt_cfg.subt_width = 0;
	libsubt_cfg.subt_height =0 ;
	libsubt_cfg.subt_hor_offset =0;
	libsubt_cfg.subt_ver_offset =0 ;
	lib_subt_attach(&libsubt_cfg);

	struct sdec_feature_config cfg_param;
	MEMSET(&cfg_param, 0, sizeof(struct sdec_feature_config));
	cfg_param.temp_buf_len = 0x2000; 
	cfg_param.temp_buf = NULL;
	cfg_param.bs_hdr_buf_len = 200;
	cfg_param.bs_hdr_buf = 0;
	cfg_param.bs_buf = 0;
	cfg_param.bs_buf_len = 0XA000;
	//cfg_param.pixel_buf = (UINT8*)__MM_SUB_PB_START_ADDR;

	cfg_param.tsk_qtm = 2;//10;	//fix BUG05435
	cfg_param.tsk_pri = OSAL_PRI_HIGH;//OSAL_PRI_NORMAL;//OSAL_PRI_HIGH
	cfg_param.transparent_color = OSD_TRANSPARENT_COLOR;
	cfg_param.support_hw_decode = 0;
	cfg_param.sdec_hw_buf = NULL;
	cfg_param.sdec_hw_buf_len = 0;    
    
	cfg_param.subt_draw_pixel = osd_subt_draw_pixel;
	cfg_param.region_is_created = osd_region_is_created;
	cfg_param.subt_create_region = osd_subt_create_region;
	cfg_param.subt_region_show = osd_subt_region_show;
	cfg_param.subt_delete_region = osd_subt_delete_region;
	cfg_param.subt_get_region_addr = osd_subt_get_region_addr;
//	cfg_param.draw_obj_hardware = draw_object_hardware;

	sdec_m33_attach(&cfg_param);
	subt_disply_bl_init((struct sdec_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SDEC));
#endif

#ifdef MULTI_CAS
#if (CAS_TYPE == CAS_ABEL)
	AbelConfig();
#endif
#endif

}

/**
void vp_vdac_init(SYSTEM_DATA *sys_data, struct VP_DacInfo *dac_config, UINT32 max_cnt)
{
	UINT8 soc_vdac_num, vdac_index = 0;
	UINT8 cur_vdac_type, cur_vdac_sub_type, cur_vdac_cnt= 0;
	UINT8 vdac_type_cnt[VDAC_TYPE_NUM] = {0,};
	UINT8 vdac_sub_type_cnt[VDAC_TYPE_NUM] = {0,};
	UINT8 vdac_config_got[VDAC_NUM_MAX] = {0,};
	//UINT8 vdac_index[] = {CVBS, YC_Y, CAV_Y, RGB_R, SECAM_YC, SECAM_Y};
	UINT8 vdac_config_index[] = {CVBS_1, SVIDEO_1, YUV_1, RGB_1, SECAM_CVBS1, SECAM_SVIDEO1};
	UINT8 vdac_type_max_num[] = {YC_Y-CVBS, CAV_Y-YC_Y, RGB_R-CAV_Y, \
		SECAM_YC-RGB_R-1, SECAM_Y-SECAM_YC, SECAM_C-SECAM_Y+1};
	UINT8 vdac_config_max_num[] = {SVIDEO_1-CVBS_1, YUV_1-SVIDEO_1, RGB_1-YUV_1, \
		SECAM_CVBS1-RGB_1-1, SECAM_SVIDEO1-SECAM_CVBS1, SECAM_SVIDEO3-SECAM_SVIDEO1+1};
	UINT8 vdac_config = 0;
	int i = 0, j = 0;
	
	if (dac_config == NULL)
		return;
	MEMSET(dac_config, 0, sizeof(struct VP_DacInfo)*max_cnt);

	if (sys_ic_is_M3202() == 1)
		soc_vdac_num = 4;
	else
		soc_vdac_num = 4;

	do
	{
		if (vdac_config_got[i] == 0)
		{
			vdac_index++;
			vdac_config_got[i] = 1;
			cur_vdac_type = (sys_data->avset.vdac_out[i] >> 2) & 0x3F;
			if (cur_vdac_type < VDAC_TYPE_MAX)
			{
				UINT8 vdac_sub_type[4]={0,};
				vdac_sub_type_cnt[cur_vdac_type]++;
				vdac_sub_type[sys_data->avset.vdac_out[i]&0x03] = 1<<i;

				vdac_config = vdac_config_index[cur_vdac_type]+vdac_type_cnt[cur_vdac_type];
				if (vdac_config >= max_cnt)
					return;
				
				for (j=0; j<soc_vdac_num && vdac_sub_type_cnt[cur_vdac_type] < vdac_type_max_num[cur_vdac_type]; j++)
				{
					if (vdac_config_got[j] == 0)
					{
						UINT8 vdac_type = (sys_data->avset.vdac_out[j] >> 2) & 0x3F;
						if (cur_vdac_type == vdac_type)
						{
							if (vdac_sub_type[sys_data->avset.vdac_out[j]&0x03] == 0)
							{
								vdac_sub_type_cnt[cur_vdac_type]++;
								vdac_index++;
								vdac_config_got[j] = 1;
								vdac_sub_type[sys_data->avset.vdac_out[j]&0x03] = 1<<j;
							}
						}
					}
				}
				vdac_type_cnt[cur_vdac_type]++;
				dac_config[vdac_config].bEnable = TRUE;
				for (j=0; j<vdac_type_max_num[cur_vdac_type]; j++)
					((UINT8*)&dac_config[vdac_config].tDacIndex)[j] = vdac_sub_type[j];
				//MEMCPY(&dac_config[vdac_config].tDacIndex, vdac_sub_type, vdac_type_max_num[cur_vdac_type]);
				dac_config[vdac_config].eVGAMode = VGA_NOT_USE;
				dac_config[vdac_config].bProgressive = FALSE;
			}
		}
		i++;
	}
	while (vdac_index < soc_vdac_num);
}
*/

/**
	Name: vp_vdac_init
	Author: jinfeng
	Desc:
		VDAC, which is an ouput device and Can be configured with some format: CVBS, YCbCr, YC, YPbPr, RGB, SVIDEO ...
		How to configure is an important problem before reading this function. Below I Show it for you simply:
			DacInfo.bEnable = TRUE;	// Enabel this configuration
			// if you choose CVBS, the second and third is NULL
			DacInfo.tDacIndex.uDacFirst = YUV_Y;	// Y
			DacInfo.tDacIndex.uDacSecond = YUV_U;	// U
			DacInfo.tDacIndex.uDacThird = YUV_V;	// V
			DacInfo.eVGAMode = VGA_NOT_USE;
			DacInfo.bProgressive = FALSE;
		This function reads configuration data which stored in system data. How does it stored ? For Example:
			#define VDAC_TYPE_YUV		2
			#define VDAC_YUV_Y			(VDAC_TYPE_YUV<<2|0)
			#define VDAC_YUV_U			(VDAC_TYPE_YUV<<2|1)
			#define VDAC_YUV_V			(VDAC_TYPE_YUV<<2|2)
		So, we find the main type in the first loop, if it is CVBS, the second loop can't do anything; if it is YUV, then YUV_U & YUV_V would be found in the next loop.
			
*/
/**
void vp_vdac_init(SYSTEM_DATA *sys_data, struct VP_DacInfo *dac_config, UINT32 max_cnt)
{
	UINT8 vdac_num;
	
	UINT8 ReadFlag[VDAC_TYPE_MAX] = {0,};
	
	// type = Main Type, subTypeCnt count the really elements in subType[]
	UINT8 type, subType[3], subTypeCnt;
	
	UINT8 i, j;
	
	// Param Error Check
	if(NULL == dac_config) return;
	
	// On-board VDAC number
	if(TRUE == sys_ic_is_M3202())
		vdac_num = 4;
	else 
		vdac_num = 4;
	
	for(i=0; i<vdac_num; i++)
	{
		// Init temp variables
		type = 0;
		subType[0] = 0;
		subType[1] = 0;
		subType[2] = 0;
		subTypeCnt = 0;
		
		// Skip
		if(TRUE == ReadFlag[i]) continue;
		
		ReadFlag[i] = TRUE;
		
		// Get Main Type
		type = (sys_data->avset.vdac_out[i]>>2) & 0x3F;
		
		// Get Sub Types
		subType[sys_data->avset.vdac_out[i] & 0x3] = 1<<i;
		subTypeCnt++;
		
		// This loop finds the sub types which has the same Main Type!
		for(j=0; j<vdac_num; j++)
		{
			if(TRUE == ReadFlag[j]) continue;
			// If not same, continue...
			if(type != ((sys_data->avset.vdac_out[j]>>2) & 0x3F)) continue;
			// If sub type has already existed, Note: it means an error stored in system data! Just Continue!
			if(0 != (sys_data->avset.vdac_out[j] & 0x3)) continue;
			
			// Set Flag, the main loop will skip this.
			ReadFlag[j] = TRUE;
			subType[sys_data->avset.vdac_out[j] & 0x3] = 1<<j;
			subTypeCnt++;
			ASSERT(subTypeCnt <= 3);
		}
		
		// Configure, Note: VGA NOT USE !!!
		dac_config[i].bEnable = TRUE;
		for(j=0; j<subTypeCnt; j++)
			((UINT8 *) &dac_config[i].tDacIndex)[j] = subType[j];
		dac_config[i].eVGAMode = VGA_NOT_USE;
		dac_config[i].bProgressive = FALSE;
	}
}
*/

void vp_vdac_init(SYSTEM_DATA *sys_data, struct VP_DacInfo *dac_config, UINT32 max_cnt)
{
	dac_config[CVBS_1].bEnable = TRUE;
	dac_config[CVBS_1].tDacIndex.uDacFirst = CVBS_DAC;
	dac_config[CVBS_1].eVGAMode = VGA_NOT_USE;
	dac_config[CVBS_1].bProgressive = FALSE;
	
	dac_config[YUV_1].bEnable = TRUE;
	dac_config[YUV_1].tDacIndex.uDacFirst = YUV_DAC_Y;
	dac_config[YUV_1].tDacIndex.uDacSecond = YUV_DAC_U;
	dac_config[YUV_1].tDacIndex.uDacThird = YUV_DAC_V;
	dac_config[YUV_1].eVGAMode = VGA_NOT_USE;
	dac_config[YUV_1].bProgressive = FALSE;
}



/********************Tmp Example for Application Begin********************/

static void InitVPPara(struct VP_InitInfo *pVPInitInfo)
{
	int i;
	SYSTEM_DATA *sys_data;
	UINT32 chunk_id, db_len, db_addr;
	INT32 ret;
	UINT8 pre_tv_mode = 0xFF;
	UINT32 chid;
	UINT32 offset;
	UINT16 blogo_para;
	UINT8  data[2];
	/*
	chunk_id = 0x04FB0100;
	api_get_chuck_addlen(chunk_id, &db_addr, &db_len);
	db_len = db_len - (64 *1024); 
	init_tmp_info(db_addr + db_len, 64 *1024);
	sys_data = &system_config;
	ret = load_tmp_data((UINT8*)sys_data, sizeof(SYSTEM_DATA));
	if (ret != SUCCESS)
	{
		for (i=0; i<VDAC_NUM_MAX; i++)
			sys_data->avset.vdac_out[i] = board_vdac_config[i];
		sys_data->avset.tv_mode = TV_MODE_PAL;
	}
	*/
	sys_data = &system_config;

	//api set backgound color]
	pVPInitInfo->tInitColor.uY = 0x10;
	pVPInitInfo->tInitColor.uCb = 0x80;
	pVPInitInfo->tInitColor.uCr = 0x80;

	//set advanced control
	pVPInitInfo->bBrightnessValue = 0;
	pVPInitInfo->fBrightnessValueSign = TRUE;
	pVPInitInfo->wContrastValue = 0;
	pVPInitInfo->fContrastSign = TRUE;
	pVPInitInfo->wSaturationValue = 0;
	pVPInitInfo->fSaturationValueSign = TRUE;
	pVPInitInfo->wSharpness = 0;
	pVPInitInfo->fSharpnessSign = TRUE;
	pVPInitInfo->wHueSin = 0;
	pVPInitInfo->fHueSinSign = TRUE;
	pVPInitInfo->wHueCos = 0;
	pVPInitInfo->fHueCosSign = TRUE;
	pVPInitInfo->bCCIR656Enable = FALSE;
	//VPO_Zoom
	pVPInitInfo->tSrcRect.uStartX = 0;
	pVPInitInfo->tSrcRect.uWidth = PICTURE_WIDTH;
	pVPInitInfo->tSrcRect.uStartY = 0;
	pVPInitInfo->tSrcRect.uHeight = PICTURE_HEIGHT;
	pVPInitInfo->DstRect.uStartX = 0;
	pVPInitInfo->DstRect.uWidth = SCREEN_WIDTH;
	pVPInitInfo->DstRect.uStartY = 0;
	pVPInitInfo->DstRect.uHeight = SCREEN_HEIGHT;

	//VPO_SetAspect
	pVPInitInfo->eTVAspect = TV_4_3;
	pVPInitInfo->eDisplayMode = NORMAL_SCALE; //LETTERBOX;
	pVPInitInfo->uNonlinearChangePoint = 111;
	pVPInitInfo->uPanScanOffset = 90;
	//VPO_SetOutput
	for (i = 0; i < VP_DAC_TYPENUM; i++)
		pVPInitInfo->pDacConfig[i].bEnable = FALSE;
	
	/* Init video dac config */
	vp_vdac_init(sys_data, pVPInitInfo->pDacConfig, 23);

	switch (sys_data->avset.tv_mode)
	{
		case TV_MODE_PAL:
			pre_tv_mode = PAL;
			break;
		case TV_MODE_PAL_M:
			pre_tv_mode = PAL_M;
			break;
		case TV_MODE_PAL_N:
			pre_tv_mode = PAL_N;
			break;
		case TV_MODE_NTSC358:
			pre_tv_mode = NTSC;
			break;
		default:
			pre_tv_mode = PAL;
	}

	//add to get logo format
	chid = BOOT_ID_2;
	offset = sto_chunk_goto(&chid, 0xFFFFFFFF, 1);
	sto_get_data(g_sto_dev, data, offset + HW_BOOTLOGO_SIZE, 2);
	blogo_para = ((data[1] << 8) | data[0]);
	blogo_tvformat = blogo_para & LOGO_FORMAT_MASK; 
	
	if (blogo_switch)
	{
		if (blogo_tvformat == LOGO_FORMAT_PAL)
			pVPInitInfo->eTVSys = PAL;
		else if (blogo_tvformat == LOGO_FORMAT_PAL_N)
			pVPInitInfo->eTVSys = PAL_N;
		else if (blogo_tvformat == LOGO_FORMAT_NTSC)
			pVPInitInfo->eTVSys = NTSC;
		else if (blogo_tvformat == LOGO_FORMAT_PAL_M)
			pVPInitInfo->eTVSys = PAL_M;
		else if (blogo_tvformat == LOGO_FORMAT_PAL_60)
			pVPInitInfo->eTVSys = PAL_60;
		else if (blogo_tvformat == LOGO_FORMAT_NTSC_443)
			pVPInitInfo->eTVSys = NTSC_443;
		pVPInitInfo->bWinOnOff = TRUE;
	}
	else
	{
		pVPInitInfo->eTVSys = pre_tv_mode;
		pVPInitInfo->bWinOnOff = FALSE;
	}

	pVPInitInfo->uWinMode = VPO_MAINWIN; //|VPO_PIPWIN;
	pVPInitInfo->tPIPCallBack.RequestCallback = NULL;
	pVPInitInfo->tPIPCallBack.ReleaseCallback = NULL;
#ifndef USE_NEW_VDEC
	pVPInitInfo->tMPCallBack.RequestCallback = VDec_Output_Frame;
	pVPInitInfo->tMPCallBack.ReleaseCallback = VDec_Release_FrBuf;
#else
	pVPInitInfo->tMPCallBack.RequestCallback = vdec_m3327_de_mp_request;
	pVPInitInfo->tMPCallBack.ReleaseCallback = vdec_m3327_de_mp_release;
#endif
	pVPInitInfo->pSrcChange_CallBack = NULL;
}


static void avStart()
{
	struct VDecPIPInfo vInitInfo;
	struct MPSource_CallBack vMPCallBack;
	struct PIPSource_CallBack vPIPCallBack;
	struct VDec_PullDown_Opr tPullDownCallBack;

	dmx_start(g_dmx_dev);
	vdec_set_output(g_decv_dev, MP_MODE, &vInitInfo, &vMPCallBack, &vPIPCallBack);
	vpo_win_mode(g_vpo_dev, VPO_MAINWIN, &vMPCallBack, &vPIPCallBack);
#ifdef VDEC_AV_SYNC
	//vdec_sync_mode(g_decv_dev, VDEC_SYNC_PTS, VDEC_SYNC_I | VDEC_SYNC_P | VDEC_SYNC_B);
	deca_set_sync_mode(g_deca_dev, ADEC_SYNC_PTS);
#endif
	/*to speedup video first I frame show time, we can set  first I frame freerun, then av sync step by step*/
	vdec_sync_mode(g_decv_dev, VDEC_SYNC_PTS, VDEC_SYNC_FIRSTFREERUN|VDEC_SYNC_I | VDEC_SYNC_P | VDEC_SYNC_B);
	
#ifdef VIDEO_OUTPUT_BETTER_VISION
	vpo_ioctl(g_vpo_dev, VPO_IO_COMPENENT_UPSAMP_REPEAT, 1);
#endif
#ifdef CHANNEL_CHANGE_VIDEO_FAST_SHOW
	vdec_io_control(g_decv_dev, VDEC_IO_FAST_CHANNEL_CHANGE, 1);
#endif

#if	(TTX_ON == 1)
	vbi_setoutput(g_vbi_dev, &VBIRequest);
	vpo_ioctl(g_vpo_dev, VPO_IO_SET_VBI_OUT, (UINT32)VBIRequest);
#endif
}

static RET_CODE avInit()
{
	struct VP_InitInfo tVPInitInfo;

	g_deca_dev = (struct deca_device*)dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
	g_dmx_dev = (struct dmx_device*)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
	g_snd_dev = (struct snd_device*)dev_get_by_id(HLD_DEV_TYPE_SND, 0);
	g_vpo_dev = (struct vpo_device*)dev_get_by_id(HLD_DEV_TYPE_DIS, 0);
	g_decv_dev = (struct vdec_device*)dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
#if	(SUBTITLE_ON == 1)
	g_sdec_dev = (struct sdec_device *)dev_get_by_id(HLD_DEV_TYPE_SDEC, 0);
#endif
#if	(TTX_ON == 1)
	g_vbi_dev = (struct vbi_device *)dev_get_by_id(HLD_DEV_TYPE_VBI, 0);
#endif
	if (g_deca_dev == NULL || g_dmx_dev == NULL || g_snd_dev == NULL)
	{
		PRINTF("Get device point failed: g_deca_dev=%08x g_dmx_dev=%08x g_snd_dev=%08x\n", g_deca_dev, g_dmx_dev, g_snd_dev);
		ASSERT(0);
	}

	deca_init_tone_voice(g_deca_dev);
	snd_init_tone_voice(g_snd_dev);
	snd_init_spectrum(g_snd_dev);
	if (RET_SUCCESS != deca_open(g_deca_dev, AUDIO_MPEG2, AUDIO_SAMPLE_RATE_48, AUDIO_QWLEN_24, 2, 0))
	{
		PRINTF("deca_open failed!!\n");
		ASSERT(0);
	}

#ifdef AV_SYNC_TEST
        struct deca_avsync_drop_threshold avsync_thres;
	 avsync_thres.thres1 = 2;
    	 avsync_thres.thres2 = 3;
	 deca_io_control(g_deca_dev, DECA_SET_AVSYNC_DROP_THRESHOLD, &avsync_thres);
	deca_io_control(g_deca_dev, DECA_SET_AV_SYNC_LEVEL, 2);
#endif

	if (RET_SUCCESS != dmx_open(g_dmx_dev))
	{
		PRINTF("dmx_open failed!!\n");
		ASSERT(0);
	}


	struct pes_retrieve_param pes_service;

	//Binding Video Decoder to DMX
	pes_service.device = dev_get_by_id(HLD_DEV_TYPE_DECV, 0);
	pes_service.filter_idx = 0;
	pes_service.retrieve_fmt = PES_HEADER_DISCARDED;
	pes_service.str_type = VIDEO_STR;
	pes_service.request_write = vdec_vbv_request;
	pes_service.update_write = vdec_vbv_update;
	dmx_io_control(g_dmx_dev, DMX_BINDING_PES_RETRIEVE, (UINT32)(&pes_service));

	//Binding Audio Decoder to DMX
	pes_service.device = dev_get_by_id(HLD_DEV_TYPE_DECA, 0);
	pes_service.filter_idx = 1;
	pes_service.retrieve_fmt = PES_HEADER_DISCARDED;
	pes_service.str_type = AUDIO_STR;
	pes_service.request_write = deca_request_write;
	pes_service.update_write = deca_update_write;
	dmx_io_control(g_dmx_dev, DMX_BINDING_PES_RETRIEVE, (UINT32)(&pes_service));
	dmx_io_control(g_dmx_dev, DMX_SWITCH_TO_UNI_BUF, 0);
	dmx_io_control(g_dmx_dev, DMX_CHANGE_THLD, (0 << 16) | (16));

#if (TTX_ON == 1 )
    //Binding TTX Decoder to DMX
	pes_service.device = dev_get_by_id(HLD_DEV_TYPE_VBI, 0);
	pes_service.filter_idx = 3;
	pes_service.retrieve_fmt = PES_HEADER_DISCARDED;
	pes_service.str_type = PRIV_STR_1;
	pes_service.request_write = vbi_request_write;
	pes_service.update_write = vbi_update_write ;
	dmx_io_control(g_dmx_dev, DMX_BINDING_PES_RETRIEVE, (UINT32)(&pes_service));

	if(RET_SUCCESS!=vbi_open(g_vbi_dev))
	{
		PRINTF("vbi_open failed!!\n");
		ASSERT(0);
	}
#endif
#if	(SUBTITLE_ON == 1)
	//Binding SUB Decoder to DMX
	pes_service.device = dev_get_by_id(HLD_DEV_TYPE_SDEC, 0);
	pes_service.filter_idx = 4;
	pes_service.retrieve_fmt = PES_HEADER_DISCARDED;
	pes_service.str_type = PRIV_STR_1;
	pes_service.request_write = sdec_request_write;
	pes_service.update_write = sdec_update_write ;
	dmx_io_control(g_dmx_dev, DMX_BINDING_PES_RETRIEVE, (UINT32)(&pes_service));
    
	if(RET_SUCCESS!=sdec_open(g_sdec_dev))
	{
		PRINTF("sdec_open failed!!\n");
		ASSERT(0);
	}
#endif

	if (RET_SUCCESS != snd_open(g_snd_dev))
	{
		PRINTF("snd_open failed!!\n");
		ASSERT(0);
    }
    deca_init_ase ( g_deca_dev );

	if (RET_SUCCESS != vdec_open(g_decv_dev))
		ASSERT(0);
	InitVPPara(&tVPInitInfo);
	if (RET_SUCCESS != vpo_open(g_vpo_dev, &tVPInitInfo))
		ASSERT(0);
	/* Vdec ignore hd service */
	vdec_io_control(g_decv_dev, VDEC_IO_DISCARD_HD_SERVICE, 1);

	return RET_SUCCESS;
}



void regist_device_to_manage()
{
    struct nim_config nim_config_info;

    MEMSET(&nim_config_info, 0, sizeof(nim_config_info));
    if(g_nim_dev!=NULL)
    {
       dev_register_device((void *)g_nim_dev, HLD_DEV_TYPE_NIM, 0, FRONTEND_TYPE_C);
    }
    if(g_nim_dev2!=NULL)
        dev_register_device((void *)g_nim_dev2, HLD_DEV_TYPE_NIM, 1, FRONTEND_TYPE_C);
}

/*******************************************************
 * external APIs
 ********************************************************/

/*fast play last free service after power on*/
void api_fastplay_service()
{
	struct io_param io_parameter;
  	UINT16 pid_list[3];
	UINT32 create_stream = 0;
	UINT32 enable_stream = 0;
	UINT32 av_sync_mode = ADEC_SYNC_PTS;
	INT32 ret = SUCCESS;
	UINT8 volumn = 70;
	UINT32 audio_stream_type = AUDIO_MPEG2;
	UINT8 audio_chan = AUDIO_CH_STEREO;


	MEMSET(&pid_list[0], 0x1fff, sizeof(pid_list));

	/*read system data to get last time servie info: pid(a,v,pcr), audio chan, volumn*/
	SYSTEM_DATA *psys_data;
	
	psys_data = sys_data_get();
	if(psys_data->startup_service_valid==0||psys_data->startup_lock==1)
	{
		//libc_printf("%s(): system data invalid\n",__FUNCTION__);
		return;
	}
	
	MEMCPY(&pid_list[0], psys_data->startup_pid_list, sizeof(pid_list));
	volumn = psys_data->startup_volumn;
	audio_chan = psys_data->startup_audio_chan;
	//libc_printf("%s(): af read tmp data, tick=%d\n",__FUNCTION__, osal_get_tick());

	if(pid_list[1]&0x2000)
		audio_stream_type = AUDIO_AC3;
	/*set dmx, vdec, deca, snd */
	//Video PID, Audio PID and PCR PID
    	if((pid_list[0]!=INVALID_PID)&&(pid_list[0]!=0)&&(pid_list[1]!=INVALID_PID)&&(pid_list[1]!=0))
	{
		create_stream = IO_CREATE_AV_STREAM;
		enable_stream = IO_STREAM_ENABLE;
	}
/*	else if(((pid_list[0]==INVALID_PID)||(pid_list[0]==0))&&((pid_list[1]!=INVALID_PID)&&(pid_list[1]!=0)))
	{
		create_stream = IO_CREATE_AUDIO_STREAM;
		enable_stream = AUDIO_STREAM_ENABLE;
		av_sync_mode = ADEC_SYNC_FREERUN;
	}
*/
	//not normal tv or radio service
	else
	{
		//libc_printf("%s(): pid[%d][%d][%d] not normal tv or radio service\n",__FUNCTION__,
			//pid_list[0], pid_list[1], pid_list[2]);
		return;
	}
	io_parameter.io_buff_in = (UINT8 *)pid_list;
	dmx_io_control(g_dmx_dev, create_stream, (UINT32)&io_parameter);
	deca_set_sync_mode(g_deca_dev, av_sync_mode);
	dmx_io_control(g_dmx_dev, enable_stream, (UINT32)&io_parameter);

	//special patch for first video I fram free run
	#if 0
	vpo_ioctl(g_vpo_dev,VPO_IO_CHANGE_CHANNEL, 1); 
	#endif

	//start vdec
	if((pid_list[0]!=INVALID_PID)&&(pid_list[0]!=0))
		vdec_start(g_decv_dev);
	//Set Audio Stream Type in Deca
	deca_io_control(g_deca_dev, DECA_SET_STR_TYPE, audio_stream_type);
	//Start Deca
	ret = deca_start(g_deca_dev, 0);
	//Set Audio Channel
	enum SndDupChannel chan;
	switch(audio_chan)
	{
	case AUDIO_CH_L:
		chan = SND_DUP_L;
		break;
	case AUDIO_CH_R:
        	chan = SND_DUP_R;
        	break;
       case AUDIO_CH_MONO:
        	chan = SND_DUP_MONO;
        	break;
       case AUDIO_CH_STEREO:
       default:
        	chan = SND_DUP_NONE;
        	break;
	}
	snd_set_duplicate(g_snd_dev, chan);
	//set volumn
	if(SUCCESS !=  snd_io_control(g_snd_dev, IS_SND_MUTE, 0))
	{
		snd_set_volume(g_snd_dev, SND_SUB_OUT, volumn);
	}
	//libc_printf("%s(): finsish, tick=%d\n",__FUNCTION__, osal_get_tick());

	g_fastplay_flag = TRUE;
}

BOOL api_get_fastplay_flag()
{
#if(CAS_TYPE == CAS_CONAX ||CAS_TYPE == CAS_ABEL)      //bug #1896. 
        g_fastplay_flag = FALSE;
#endif
	return g_fastplay_flag;
}


#if(CAS_TYPE== CAS_IRDETO)
#define ADV_DMX_LEN	(160*1024)
UINT8 adv_dmx_buf[ADV_DMX_LEN];
#endif


void stb_info_check()
{
	STB_INFO_DATA *stb_data;
	if(stb_data_load() != SUCCESS)
	{
		set_pan_display_type(PAN_DISP_LED_ONLY);
		PRINTF("no valid stb data, now need init it!\n");
		if(stb_data_factory_init() != 0)
		{
			PRINTF("%s:%d, ERROR stb_info_data!\n", __FUNCTION__, __LINE__);
			stb_pan_display("ErON", 4);	
			osal_task_sleep(2000);
			sys_watchdog_reboot();
		}
		else
		{
			/*backup upgloader for first power on*/
			stb_data = stb_info_data_get();
			stb_pan_display("b1-2", 4);	
			backup_upgloader(stb_data->param1,stb_data->param2,stb_data->param3);
		}
		stb_pan_display_revert();	
		set_pan_display_type(PAN_DISP_LED_BUF);
	}

	stb_data = stb_info_data_get();
	if(stb_data->state & STB_INFO_DATA_STATE_RESET)
	{
		set_pan_display_type(PAN_DISP_LED_ONLY);
		
		PRINTF("not complete stb data, now fullfil it!\n");
		if(stb_data_factory_complete() != 0)
		{
			PRINTF("%s:%d, ERROR fullfil stb_info_data!\n", __FUNCTION__, __LINE__);
			stb_data->state = STB_INFO_DATA_STATE_LIMIT;			
		}
		else
		{
			stb_data->state = STB_INFO_DATA_STATE_NORMAL;			
		}
		stb_info_data_save();

		stb_pan_display_revert();	
		set_pan_display_type(PAN_DISP_LED_BUF);
	}
	if(stb_data->state & STB_INFO_DATA_STATE_LIMIT)
	{
		PRINTF("%s:%d, ERROR stb_info_data!\n", __FUNCTION__, __LINE__);
		pan_display(g_pan_dev, "ErON", 4);	
		osal_task_sleep(2000);
		sys_watchdog_reboot();
	}
}

#ifdef AV_SYNC_TEST
void av_sync_certificate_patch()
{
	/*具体mode, time_shift要根据认证测试码流确定*/
	UINT8 mode = 1;
	UINT32 time_shift = 0;
	switch(mode)
	{
	case 1:     // Make audio stream play earlier xxxx ms
	{
		time_shift = 20;  // in ms unit.
		dmx_io_control(g_dmx_dev, DMX_ADJUST_AV_PTS,  (0xa0<<24)|time_shift);
		break;
	}
	case 2:    //  Make audio stream play delay xxxx ms
	{
		//time_shift= 0 ;  // in ms unit.
		dmx_io_control(g_dmx_dev, DMX_ADJUST_AV_PTS,  (0xa8<<24)|time_shift);
		break;
	}
	case 3:     // Make video stream play earlier xxxx ms
	{
		//time_shift= xxx ;  // in ms unit.
		dmx_io_control(g_dmx_dev, DMX_ADJUST_AV_PTS,  (0x50<<24)|time_shift);
		break;
	}
	case 4:    //  Make video stream play delay xxxx ms
	{
		//time_shift= xxx ;  // in ms unit.
		dmx_io_control(g_dmx_dev, DMX_ADJUST_AV_PTS,  (0x58<<24)|time_shift);
		break;
	}
	default:
		break;
	}
}
#endif

void AppInit()
{
	if (init_vbv_buffer()==FALSE)
	{
		ASSERT(0);
	}

	system_hw_init();
#ifdef MULTI_CAS
	#if(CAS_TYPE== CAS_IRDETO)
	adv_dmx_init(32, adv_dmx_buf, ADV_DMX_LEN);
	#endif
#endif
	avInit();
	avStart();

/*irdeto ota use FTA service for ota, that need cat and emm, must call api_play_channel(),
so not use fast play*/	
//#if(CAS_TYPE!= CAS_IRDETO)
#ifdef AD_SANZHOU
	ad_init();
	if(!szxc_ad_show_mpg(AD_STARTUP))
	{
	#ifdef FAST_FREE_PLAY
		/*fast play last service*/
		api_fastplay_service();
	#endif
	}
#else
	#ifdef FAST_FREE_PLAY
		/*fast play last service*/
		//api_fastplay_service();  防止出现启动后先播台后跳台的BUG(广告中重新设置了频点->nim_channel_change 导致换台)
	#endif
#endif

	regist_device_to_manage();
    
	struct pub_module_config config;
	MEMSET(&config, 0, sizeof(config));
	config.dm_enable = FALSE;

#if	(TTX_ON == 1)
	TTXEng_Init();
    TTXEng_set_g0_set();
	ttx_enable(TRUE);
#endif 
#if (SUBTITLE_ON == 1)
	subt_enable(TRUE);
#endif
    
	if ( libpub_init(&config) != RET_SUCCESS )
		PRINTF("libpub27_init Init failed\n");
	/*loader4, need stb_info CHUNK data*/
	stb_info_check();
	
	ap_task_init();
	key_Init();

#ifdef SFU_AUTO_TEST
	SFUTestInit();
#endif

	ota_init();
#ifdef MULTI_CAS
#if ((CAS_TYPE == CAS_CONAX)||(CAS_TYPE == CAS_ABEL))
	void ap_mcas_display(mcas_disp_type_t type, UINT32 param);
	api_mcas_register(ap_mcas_display);
#else
	cas_init();
#endif
#endif
	ota_reg_parse_callback(ota_table_request);

	/*enable M3202C POK for flash power loss protection*/
	if((1==sys_ic_is_M3202())&&(sys_ic_get_rev_id()>=IC_REV_4))
  		pdd_init(0, 0);


	PRINTF("APP init finished...\n");
#ifdef AV_SYNC_TEST
	av_sync_certificate_patch();
#endif
	
//burn stbid by com

#ifdef BURN_SERIAL_NUM
	extern UINT32 cmd_stbid(unsigned int argc, unsigned char *argv[]);
	extern UINT32 cmd_process_monitor(unsigned int argc, unsigned char *argv[]);
	struct ash_cmd ash_cmd_stbid = {"serial", cmd_stbid};
	struct ash_cmd ash_cmd_pm = {"pmonitor", cmd_process_monitor};
	UINT32 cmd_process_monitor(unsigned int argc, unsigned char *argv[]);
	if(cmd_reg_callback(ash_cmd_stbid)==FALSE)
	{
		return;
	}
	if(cmd_reg_callback(ash_cmd_pm)==FALSE)
	{
		return;
	}
	api_set_com_check_flag(COM_MONITOR_CHECK_STBID);
//
#endif

#ifdef	SYS_DEBUG_ENABLE 
	//Sys debug tool.
	#include <osal/osal_sysdbg.h>
 	sysdbg_display_interval_set(5000);
	sysdbg_output_set(libc_printf);
	sysdbg_task_info_enable(TRUE, TSK_EXTRA_DURA);
#endif

#ifdef _DMX_AV_SYNC_API_MODIFY_
/*RD attention please!! Don't define _DMX_AV_SYNC_API_MODIFY_ in any file of your porj.
This macro should be defined in compiler.def by QT, and only valid in QT testing!*/
	dmx_io_control(g_dmx_dev, ALI_SET_AVSYNC_MODE, ALI_AVSYNC_AUDIO_MASTER);
#endif/*end of _DMX_AV_SYNC_API_MODIFY_*/

	return ;
}

void kill()
{
}
void getpid()
{
}
