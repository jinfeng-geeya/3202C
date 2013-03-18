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
#include <hld/scart/scart.h>
#include <hld/osd/osddrv.h>
#include <api/libloader/ld_boot_info.h>
#include <hld/dis/vpo.h>
#include <hld/decv/decv.h>
#include <hld/decv/decv_avc.h>
#include <hld/nim/nim_dev.h>

#include <api/libstbinfo/stb_info_data.h>

#include "dev_handle.h"
#include "../copper_common/boot_system_data.h"
#include "../copper_common/stb_data.h"
#include "key.h"


/***********************************
VPO
************************************/
#ifndef MINI_DRIVER_SUPPORT
//for de_n
static struct tve_adjust g_tve_adjust_table[] = {
    {TVE_ADJ_COMPOSITE_Y_DELAY, SYS_625_LINE, 1}, {TVE_ADJ_COMPOSITE_Y_DELAY, SYS_525_LINE, 1}, 
    {TVE_ADJ_COMPOSITE_C_DELAY, SYS_625_LINE, 0}, {TVE_ADJ_COMPOSITE_C_DELAY, SYS_525_LINE, 0}, 
    {TVE_ADJ_COMPONENT_Y_DELAY, SYS_625_LINE, 3}, {TVE_ADJ_COMPONENT_Y_DELAY, SYS_525_LINE, 3}, 
    {TVE_ADJ_COMPONENT_CB_DELAY, SYS_625_LINE, 0}, {TVE_ADJ_COMPONENT_CB_DELAY, SYS_525_LINE, 0}, 
    {TVE_ADJ_COMPONENT_CR_DELAY, SYS_625_LINE, 2}, {TVE_ADJ_COMPONENT_CR_DELAY, SYS_525_LINE, 2}, 
    {TVE_ADJ_BURST_LEVEL_ENABLE, SYS_625_LINE, 1}, {TVE_ADJ_BURST_LEVEL_ENABLE, SYS_525_LINE, 0}, //8 bits
    {TVE_ADJ_BURST_CB_LEVEL, SYS_625_LINE, 0x8f}, {TVE_ADJ_BURST_CB_LEVEL, SYS_525_LINE, 0x68}, //8 bits
    {TVE_ADJ_BURST_CR_LEVEL, SYS_625_LINE, 0x53}, {TVE_ADJ_BURST_CR_LEVEL, SYS_525_LINE, 0}, //8 bits
    {TVE_ADJ_COMPOSITE_LUMA_LEVEL, SYS_625_LINE, 0x54}, {TVE_ADJ_COMPOSITE_LUMA_LEVEL, SYS_525_LINE, 0x4f}, //8 bits
#ifdef Enable_108MHz_Video_Sample_Rate    
    {TVE_ADJ_COMPOSITE_CHRMA_LEVEL, SYS_625_LINE, 0x10}, {TVE_ADJ_COMPOSITE_CHRMA_LEVEL, SYS_525_LINE, 0x10}, //attenuate chroma level
#else
    {TVE_ADJ_COMPOSITE_CHRMA_LEVEL, SYS_625_LINE, 0x1b}, {TVE_ADJ_COMPOSITE_CHRMA_LEVEL, SYS_525_LINE, 0x19}, //8 bits
#endif
    {TVE_ADJ_PHASE_COMPENSATION, SYS_625_LINE, 0x280}, {TVE_ADJ_PHASE_COMPENSATION, SYS_525_LINE, 0x48f}, //16 bits
 #ifdef Enable_108MHz_Video_Sample_Rate
    {TVE_ADJ_VIDEO_FREQ_RESPONSE, SYS_625_LINE, 0x0}, {TVE_ADJ_VIDEO_FREQ_RESPONSE, SYS_525_LINE, 0x0}, //disable freq response increase
#else
    {TVE_ADJ_VIDEO_FREQ_RESPONSE, SYS_625_LINE, 0x102}, {TVE_ADJ_VIDEO_FREQ_RESPONSE, SYS_525_LINE, 0x102}, //16 bits
#endif
};

struct tve_adjust g_tve_adjust_table_adv[] =
{
    {TVE_ADJ_ADV_PEDESTAL_ONOFF, SYS_625_LINE, 0},              {TVE_ADJ_ADV_PEDESTAL_ONOFF, SYS_525_LINE, 1},
    {TVE_ADJ_ADV_COMPONENT_LUM_LEVEL, SYS_625_LINE, 0x55},      {TVE_ADJ_ADV_COMPONENT_LUM_LEVEL, SYS_525_LINE, 0x50},
    {TVE_ADJ_ADV_COMPONENT_CHRMA_LEVEL, SYS_625_LINE, 0x52},    {TVE_ADJ_ADV_COMPONENT_CHRMA_LEVEL, SYS_525_LINE, 0x52},
    {TVE_ADJ_ADV_COMPONENT_PEDESTAL_LEVEL, SYS_625_LINE, 0x0},  {TVE_ADJ_ADV_COMPONENT_PEDESTAL_LEVEL, SYS_525_LINE, 0x5},
    {TVE_ADJ_ADV_COMPONENT_SYNC_LEVEL, SYS_625_LINE, 0x0},      {TVE_ADJ_ADV_COMPONENT_SYNC_LEVEL, SYS_525_LINE, 0x0},
    {TVE_ADJ_ADV_RGB_R_LEVEL, SYS_625_LINE, 0x87},              {TVE_ADJ_ADV_RGB_R_LEVEL, SYS_525_LINE, 0x8a},
    {TVE_ADJ_ADV_RGB_G_LEVEL, SYS_625_LINE, 0x8a},              {TVE_ADJ_ADV_RGB_G_LEVEL, SYS_525_LINE, 0x8b},
    {TVE_ADJ_ADV_RGB_B_LEVEL, SYS_625_LINE, 0x88},              {TVE_ADJ_ADV_RGB_B_LEVEL, SYS_525_LINE, 0x8b},
    {TVE_ADJ_ADV_COMPOSITE_PEDESTAL_LEVEL, SYS_625_LINE, 0x0},  {TVE_ADJ_ADV_COMPOSITE_PEDESTAL_LEVEL, SYS_525_LINE, 0x2b},
    {TVE_ADJ_ADV_COMPOSITE_SYNC_LEVEL, SYS_625_LINE, 0x0},      {TVE_ADJ_ADV_COMPOSITE_SYNC_LEVEL, SYS_525_LINE, 0x5},
};

void vpo_dev_attach(void)
{
	struct VP_Feature_Config vp_config;
	struct Tve_Feature_Config tve_config;

	MEMSET((void*) &vp_config, 0, sizeof(struct VP_Feature_Config));
	MEMSET((void*) &tve_config, 0, sizeof(struct Tve_Feature_Config));
	vp_config.bAvoidMosaicByFreezScr = FALSE; /* macro VDEC_AVOID_MOSAIC_BY_FREEZE_SCR */
	vp_config.bMHEG5Enable = FALSE; /* macro  _MHEG5_ENABLE_ */
	vp_config.bOsdMulitLayer = FALSE; /* macro OSD_MULTI_LAYER */
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
}
#else
UINT16 g_blogo_format = 0;	/* Bootloader logo parameters */

void vdac_init(STB_INFO_DATA *stb_data)
{
	UINT8 soc_vdac_num = 0;
	UINT8 cur_vdac_type, cur_vdac = 0;
	UINT8 vdac_type_index[] = {CVBS, YC_Y, CAV_Y, RGB_R, SECAM_YC, SECAM_Y};
	int i = 0;

	soc_vdac_num = 4;

	for (i=0; i<soc_vdac_num; i++)
	{
		cur_vdac_type = (stb_data->upg_info.c_info.vdac_out[i] >> 2) & 0x3F;
		cur_vdac = vdac_type_index[cur_vdac_type] + (stb_data->upg_info.c_info.vdac_out[i] & 0x03);
		tvenc_set_dac(i, cur_vdac);
	}
}

void vpo_init(STB_INFO_DATA *stb_data)
{
	UINT32 boot_tick;
	
        //---reset video core: tve and vpo
        *(volatile UINT8 *)0xB8000060 = 0x09;
	osal_delay(5000);
	*(volatile UINT8 *)0xB8000060 = 0x00;

	switch(stb_data->upg_info.c_info.boot_tv_mode)
	{
		case TV_MODE_PAL:
			g_blogo_format = LOGO_FORMAT_PAL;
			break;
		case TV_MODE_PAL_M:
			g_blogo_format = LOGO_FORMAT_PAL_M;
			break;
		case TV_MODE_PAL_N:
			g_blogo_format = LOGO_FORMAT_PAL_N;
			break;
		case TV_MODE_NTSC358:
			g_blogo_format = LOGO_FORMAT_NTSC;
			break;
		case TV_MODE_NTSC443:
			g_blogo_format = LOGO_FORMAT_NTSC_443;
			break;
		default:
			g_blogo_format = LOGO_FORMAT_PAL;
	}
	
	declogo_attach_min(__MM_BUF_TOP_ADDR,__MM_FB0_Y_LEN+__MM_FB0_C_LEN);

	//first vpo init
	vpoInitGlobalValue();
	vpoInitRegs();

	//second vdac init
	vdac_init(stb_data);
}
#endif

/***********************************
VEDIO DEC
************************************/
#ifndef MINI_DRIVER_SUPPORT
extern UINT8   pre_tv_mode;
unsigned short blogo_switch=1;
unsigned short blogo_tvformat=0;

void vp_vdac_init(UINT8 *vdac_out, struct VP_DacInfo *dac_config, UINT32 max_cnt)
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
			cur_vdac_type = (vdac_out[i] >> 2) & 0x3F;
			if (cur_vdac_type < VDAC_TYPE_MAX)
			{
				UINT8 vdac_sub_type[4]={0,};
				vdac_sub_type_cnt[cur_vdac_type]++;
				vdac_sub_type[vdac_out[i]&0x03] = 1<<i;

				vdac_config = vdac_config_index[cur_vdac_type]+vdac_type_cnt[cur_vdac_type];
				if (vdac_config >= max_cnt)
					return;
				
				for (j=0; j<soc_vdac_num && vdac_sub_type_cnt[cur_vdac_type] < vdac_type_max_num[cur_vdac_type]; j++)
				{
					if (vdac_config_got[j] == 0)
					{
						UINT8 vdac_type = (vdac_out[j] >> 2) & 0x3F;
						if (cur_vdac_type == vdac_type)
						{
							if (vdac_sub_type[vdac_out[j]&0x03] == 0)
							{
								vdac_sub_type_cnt[cur_vdac_type]++;
								vdac_index++;
								vdac_config_got[j] = 1;
								vdac_sub_type[vdac_out[j]&0x03] = 1<<j;
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

void InitVPPara(struct VP_InitInfo *pVPInitInfo)
{
	int i;
	INT32 ret;
	UINT8 tv_out = 0xFF;
	UINT32 chid;
	UINT32 offset;
	UINT16 blogo_para;
	UINT8  data[2];
	STB_INFO_DATA* stb_data;

	MEMSET((void *)pVPInitInfo, 0, sizeof(*pVPInitInfo));

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
#if 1
	/* Init video dac config */
	stb_data = stb_info_data_get();
	vp_vdac_init(stb_data->upg_info.c_info.vdac_out, pVPInitInfo->pDacConfig, 23);

	switch (pre_tv_mode)
	{
		case TV_MODE_PAL:
			tv_out = PAL;
			break;
		case TV_MODE_PAL_M:
			tv_out = PAL_M;
			break;
		case TV_MODE_PAL_N:
			tv_out = PAL_N;
			break;
		case TV_MODE_NTSC358:
			tv_out = NTSC;
			break;
		default:
			tv_out = PAL;
	}
#else
		vp_vdac_init(board_vdac_config, pVPInitInfo->pDacConfig, 23);
		tv_out = PAL;
#endif
		
	pVPInitInfo->eTVSys = tv_out;
	pVPInitInfo->bWinOnOff = FALSE;

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

UINT8 *vbv_buffer_add = NULL;

BOOL init_vbv_buffer()
{
	#if(8 == SYS_SDRAM_SIZE)
		vbv_buffer_add = (UINT8*)__MM_VBV_START_ADDR;
    #else
		vbv_buffer_add = MALLOC(__MM_VBV_LEN);
    #endif

	if(vbv_buffer_add == NULL)
		return FALSE;
	else
		return TRUE;
}
void vdec_dev_attach(void)
{
	/* !!!!!Note !!!!!
		If you copy this code to other project, please check feature configuration firstly
	     !!!!!Note !!!!!
	*/
	struct vdec_config_par vdec_config_par;

	MEMSET((void *)&vdec_config_par,0,sizeof(struct vdec_config_par));

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

	init_vbv_buffer();
	
	vdec_config_par.mem_map.vbv_size = ((__MM_VBV_LEN - 4) &0xFFFFFF00);
	vdec_config_par.mem_map.vbv_start_addr = (((UINT32)vbv_buffer_add &0xfffffff) | 0x80000000);
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
}
#endif

/***********************************
TUNER
************************************/
#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01||SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01||SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)
static INT32 nim_s3202_Tuner_Attatch(struct QAM_TUNER_CONFIG_API *ptrQAM_Tuner)
{
	UINT8 data = 0;
	INT32 ret = SUCCESS;
	ret = i2c_read(I2C_TYPE_SCB1, 0xC0, &data, 1);

	if (ret == SUCCESS)
	{
	    	ptrQAM_Tuner->tuner_config_data.RF_AGC_MAX		= 0xb8;
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MIN		= 0x14;
	//joey 20080504. change IF agc setting according to Jack's test result.	
	//trueve 20080806. restore IF AGC setting to max limitation according to Joey's suggestion.
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MAX		= 0xFF;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MIN		= 0x00;
		ptrQAM_Tuner->tuner_config_data.AGC_REF 			= 0x80;
	
		ptrQAM_Tuner->tuner_config_ext.cTuner_special_config = 0x01; // RF AGC is disabled
		ptrQAM_Tuner->tuner_config_ext.cChip				= Tuner_Chip_PHILIPS;
		ptrQAM_Tuner->tuner_config_ext.cTuner_AGC_TOP	= 1;//7; /*1 for single AGC, 7 for dual AGC */
		ptrQAM_Tuner->tuner_config_ext.cTuner_Base_Addr	= 0xC0;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Crystal		= 4;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Ref_DivRatio	= 64; 
		ptrQAM_Tuner->tuner_config_ext.cTuner_Step_Freq	= 62.5;
		//(Ref_divRatio*Step_Freq)=(80*50)=(64*62.5)=(24*166.7)
		ptrQAM_Tuner->tuner_config_ext.wTuner_IF_Freq		= 36000;
		//ptrQAM_Tuner->tuner_config_ext.cTuner_Charge_Pump= 0;  
		ptrQAM_Tuner->tuner_config_ext.i2c_type_id 			= I2C_TYPE_SCB1;
	
	
	extern INT32 tun_dct70701_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
	extern INT32 tun_dct70701_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);	
	extern INT32 tun_dct70701_status(UINT32 tuner_id, UINT8 *lock);
	
	
	       ptrQAM_Tuner->nim_Tuner_Init					= tun_dct70701_init;
		ptrQAM_Tuner->nim_Tuner_Control					= tun_dct70701_control;
		ptrQAM_Tuner->nim_Tuner_Status					= tun_dct70701_status;
	}	
	else
	{
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MAX = 0xBA;
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MIN = 0x2A;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MAX = 0xFF;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MIN = 0x00;
		ptrQAM_Tuner->tuner_config_data.AGC_REF = 0x80;

		ptrQAM_Tuner->tuner_config_ext.cTuner_special_config = 0x01; // RF AGC is disabled
		ptrQAM_Tuner->tuner_config_ext.cChip = Tuner_Chip_ALPS;
		ptrQAM_Tuner->tuner_config_ext.cTuner_AGC_TOP	= 3;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Base_Addr = 0xC2; // C0, C2, C4,C6
		ptrQAM_Tuner->tuner_config_ext.cTuner_Crystal = 4;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Ref_DivRatio = 64;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Step_Freq = 62.5;
		//(Ref_divRatio*Step_Freq)=(80*50)=(64*62.5)=(24*166.7)=(31.25*128)
		ptrQAM_Tuner->tuner_config_ext.wTuner_IF_Freq = 36125;
		//ptrQAM_Tuner->tuner_config_ext.cTuner_Charge_Pump= 0;
		ptrQAM_Tuner->tuner_config_ext.i2c_type_id = I2C_TYPE_SCB1;
		//ptrQAM_Tuner->tuner_config_ext.Tuner_Read			= i2c_read;
		//ptrQAM_Tuner->tuner_config_ext.Tuner_Write			= i2c_write;

		extern INT32 tun_alpstdae_init(UINT32 *tuner_id, struct QAM_TUNER_CONFIG_EXT *ptrTuner_Config);
		extern INT32 tun_alpstdae_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);
		extern INT32 tun_alpstdae_status(UINT32 tuner_id, UINT8 *lock);
		ptrQAM_Tuner->nim_Tuner_Init = tun_alpstdae_init;
		ptrQAM_Tuner->nim_Tuner_Control = tun_alpstdae_control;
		ptrQAM_Tuner->nim_Tuner_Status = tun_alpstdae_status;
	}

}
#elif (SYS_MAIN_BOARD == BOARD_S3202_DEMO_01V01)
static INT32 nim_s3202_Tuner_Attatch(struct QAM_TUNER_CONFIG_API *ptrQAM_Tuner)
{
#if (SYS_TUNER_MODULE == DCT7044)
	{
//joey 20080418 update according to register config by xian_chen	
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MAX = 0xa8;
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MIN = 0x23;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MAX = 0x5d;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MIN = 0x33;
		ptrQAM_Tuner->tuner_config_data.AGC_REF = 0x80;

		ptrQAM_Tuner->tuner_config_ext.cTuner_special_config = 0x00;
		ptrQAM_Tuner->tuner_config_ext.cChip = Tuner_Chip_PHILIPS;
		ptrQAM_Tuner->tuner_config_ext.cTuner_AGC_TOP = 2;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Base_Addr = 0xC0;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Crystal = 4;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Ref_DivRatio = 64;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Step_Freq = 62.5;
		//(Ref_divRatio*Step_Freq)=(80*50)=(64*62.5)=(24*166.7)
		ptrQAM_Tuner->tuner_config_ext.wTuner_IF_Freq = 36000;
		//ptrQAM_Tuner->tuner_config_ext.cTuner_Charge_Pump= 0;  // 0, 50uA; 1, 250uA
		ptrQAM_Tuner->tuner_config_ext.i2c_type_id = I2C_TYPE_SCB;
		//ptrQAM_Tuner->tuner_config_ext.Tuner_Read			= i2c_read;
		//ptrQAM_Tuner->tuner_config_ext.Tuner_Write			= i2c_write;

		extern INT32 tun_dct7044_init(UINT32 *tuner_id, struct QAM_TUNER_CONFIG_EXT *ptrTuner_Config);
		extern INT32 tun_dct7044_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);
		extern INT32 tun_dct7044_status(UINT32 tuner_id, UINT8 *lock);
		ptrQAM_Tuner->nim_Tuner_Init = tun_dct7044_init;
		ptrQAM_Tuner->nim_Tuner_Control = tun_dct7044_control;
		ptrQAM_Tuner->nim_Tuner_Status = tun_dct7044_status;

	}
#elif(SYS_TUNER_MODULE == DCT70701)
		{
	    	ptrQAM_Tuner->tuner_config_data.RF_AGC_MAX		= 0xb8;
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MIN		= 0x14;
	//joey 20080504. change IF agc setting according to Jack's test result.	
	//trueve 20080806. restore IF AGC setting to max limitation according to Joey's suggestion.
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MAX		= 0xFF;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MIN		= 0x00;
		ptrQAM_Tuner->tuner_config_data.AGC_REF 			= 0x80;
	
		ptrQAM_Tuner->tuner_config_ext.cTuner_special_config = 0x01; // RF AGC is disabled
		ptrQAM_Tuner->tuner_config_ext.cChip				= Tuner_Chip_PHILIPS;
		ptrQAM_Tuner->tuner_config_ext.cTuner_AGC_TOP	= 1;//7; /*1 for single AGC, 7 for dual AGC */
		ptrQAM_Tuner->tuner_config_ext.cTuner_Base_Addr	= 0xC2;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Crystal		= 4;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Ref_DivRatio	= 64; 
		ptrQAM_Tuner->tuner_config_ext.cTuner_Step_Freq	= 62.5;
		//(Ref_divRatio*Step_Freq)=(80*50)=(64*62.5)=(24*166.7)
		ptrQAM_Tuner->tuner_config_ext.wTuner_IF_Freq		= 36000;
		//ptrQAM_Tuner->tuner_config_ext.cTuner_Charge_Pump= 0;  
		ptrQAM_Tuner->tuner_config_ext.i2c_type_id 			= I2C_TYPE_SCB;
	
	
	extern INT32 tun_dct70701_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
	extern INT32 tun_dct70701_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);	
	extern INT32 tun_dct70701_status(UINT32 tuner_id, UINT8 *lock);
	
	
	       ptrQAM_Tuner->nim_Tuner_Init					= tun_dct70701_init;
		ptrQAM_Tuner->nim_Tuner_Control					= tun_dct70701_control;
		ptrQAM_Tuner->nim_Tuner_Status					= tun_dct70701_status;
	}	
#elif (SYS_TUNER_MODULE == ALPSTDQE)
	{
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MAX = 0xBA;
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MIN = 0x2A;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MAX = 0xFF;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MIN = 0x00;
		ptrQAM_Tuner->tuner_config_data.AGC_REF = 0x82;

		ptrQAM_Tuner->tuner_config_ext.cTuner_special_config = 0x01; // RF AGC is disabled
		ptrQAM_Tuner->tuner_config_ext.cChip = Tuner_Chip_ALPS;
		//ptrQAM_Tuner->tuner_config_ext.cTuner_AGC_TOP	= 3;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Base_Addr = 0xC0; // C0, C2, C4,C6
		ptrQAM_Tuner->tuner_config_ext.cTuner_Crystal = 4;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Ref_DivRatio = 64;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Step_Freq = 62.5;
		//(Ref_divRatio*Step_Freq)=(80*50)=(64*62.5)=(24*166.7)=(31.25*128)
		ptrQAM_Tuner->tuner_config_ext.wTuner_IF_Freq = 36125;
		//ptrQAM_Tuner->tuner_config_ext.cTuner_Charge_Pump= 0;
		ptrQAM_Tuner->tuner_config_ext.i2c_type_id = I2C_TYPE_SCB;
		//ptrQAM_Tuner->tuner_config_ext.Tuner_Read			= i2c_read;
		//ptrQAM_Tuner->tuner_config_ext.Tuner_Write			= i2c_write;

		extern INT32 tun_alpstdqe_init(UINT32 *tuner_id, struct QAM_TUNER_CONFIG_EXT *ptrTuner_Config);
		extern INT32 tun_alpstdqe_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);
		extern INT32 tun_alpstdqe_status(UINT32 tuner_id, UINT8 *lock);
		ptrQAM_Tuner->nim_Tuner_Init = tun_alpstdqe_init;
		ptrQAM_Tuner->nim_Tuner_Control = tun_alpstdqe_control;
		ptrQAM_Tuner->nim_Tuner_Status = tun_alpstdqe_status;
	}
#elif (SYS_TUNER_MODULE == ALPSTDAE)
	{
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MAX = 0xBA;
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MIN = 0x2A;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MAX = 0xFF;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MIN = 0x00;
		//trueve 20081224, update according to Program guide 20081223 by Joey.	
		ptrQAM_Tuner->tuner_config_data.AGC_REF = 0x80;

		ptrQAM_Tuner->tuner_config_ext.cTuner_special_config = 0x01; // RF AGC is disabled
		ptrQAM_Tuner->tuner_config_ext.cChip = Tuner_Chip_ALPS;
		ptrQAM_Tuner->tuner_config_ext.cTuner_AGC_TOP	= 3;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Base_Addr = 0xC2; // C0, C2, C4,C6
		ptrQAM_Tuner->tuner_config_ext.cTuner_Crystal = 4;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Ref_DivRatio = 64;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Step_Freq = 62.5;
		//(Ref_divRatio*Step_Freq)=(80*50)=(64*62.5)=(24*166.7)=(31.25*128)
		ptrQAM_Tuner->tuner_config_ext.wTuner_IF_Freq = 36125;
		//ptrQAM_Tuner->tuner_config_ext.cTuner_Charge_Pump= 0;
		ptrQAM_Tuner->tuner_config_ext.i2c_type_id = I2C_TYPE_SCB;
		//ptrQAM_Tuner->tuner_config_ext.Tuner_Read			= i2c_read;
		//ptrQAM_Tuner->tuner_config_ext.Tuner_Write			= i2c_write;

		extern INT32 tun_alpstdae_init(UINT32 *tuner_id, struct QAM_TUNER_CONFIG_EXT *ptrTuner_Config);
		extern INT32 tun_alpstdae_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);
		extern INT32 tun_alpstdae_status(UINT32 tuner_id, UINT8 *lock);
		ptrQAM_Tuner->nim_Tuner_Init = tun_alpstdae_init;
		ptrQAM_Tuner->nim_Tuner_Control = tun_alpstdae_control;
		ptrQAM_Tuner->nim_Tuner_Status = tun_alpstdae_status;
	}
#elif (SYS_TUNER_MODULE == TDCCG0X1F)
	{
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MAX = 0xBA;
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MIN = 0x2A;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MAX = 0xE7;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MIN = 0x10;
		ptrQAM_Tuner->tuner_config_data.AGC_REF = 0x80;

		ptrQAM_Tuner->tuner_config_ext.cTuner_special_config = 0x01; // RF AGC is disabled
		ptrQAM_Tuner->tuner_config_ext.cChip = Tuner_Chip_ALPS;
		ptrQAM_Tuner->tuner_config_ext.cTuner_AGC_TOP	= 3;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Base_Addr = 0xC4; // C0, C2, C4,C6
		ptrQAM_Tuner->tuner_config_ext.cTuner_Crystal = 4;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Ref_DivRatio = 64;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Step_Freq = 62.5;
		//(Ref_divRatio*Step_Freq)=(80*50)=(64*62.5)=(24*166.7)=(31.25*128)
		ptrQAM_Tuner->tuner_config_ext.wTuner_IF_Freq = 36125;
		//ptrQAM_Tuner->tuner_config_ext.cTuner_Charge_Pump= 0;
		ptrQAM_Tuner->tuner_config_ext.i2c_type_id = I2C_TYPE_SCB;
		//ptrQAM_Tuner->tuner_config_ext.Tuner_Read			= i2c_read;
		//ptrQAM_Tuner->tuner_config_ext.Tuner_Write			= i2c_write;

		extern INT32 tun_tdccg0x1f_init(UINT32 *tuner_id, struct QAM_TUNER_CONFIG_EXT *ptrTuner_Config);
		extern INT32 tun_tdccg0x1f_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);
		extern INT32 tun_tdccg0x1f_status(UINT32 tuner_id, UINT8 *lock);
		ptrQAM_Tuner->nim_Tuner_Init = tun_tdccg0x1f_init;
		ptrQAM_Tuner->nim_Tuner_Control = tun_tdccg0x1f_control;
		ptrQAM_Tuner->nim_Tuner_Status = tun_tdccg0x1f_status;
	}
#elif (SYS_TUNER_MODULE == DBCTE702F1)
	{
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MAX = 0xBA;
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MIN = 0x2A;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MAX = 0xE7;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MIN = 0x10;
		ptrQAM_Tuner->tuner_config_data.AGC_REF = 0x80;

		ptrQAM_Tuner->tuner_config_ext.cTuner_special_config = 0x01; // RF AGC is disabled
		ptrQAM_Tuner->tuner_config_ext.cChip = Tuner_Chip_ALPS;
		ptrQAM_Tuner->tuner_config_ext.cTuner_AGC_TOP	= 1;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Base_Addr = 0xC0; // C0, C2, C4,C6
		ptrQAM_Tuner->tuner_config_ext.cTuner_Crystal = 4;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Ref_DivRatio = 64;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Step_Freq = 62.5;
		//(Ref_divRatio*Step_Freq)=(80*50)=(64*62.5)=(24*166.7)=(31.25*128)
		ptrQAM_Tuner->tuner_config_ext.wTuner_IF_Freq = 36000;
		//ptrQAM_Tuner->tuner_config_ext.cTuner_Charge_Pump= 0;
		ptrQAM_Tuner->tuner_config_ext.i2c_type_id = I2C_TYPE_SCB;
		//ptrQAM_Tuner->tuner_config_ext.Tuner_Read			= i2c_read;
		//ptrQAM_Tuner->tuner_config_ext.Tuner_Write			= i2c_write;

		extern INT32 tun_dbcte702f1_init(UINT32 *tuner_id, struct QAM_TUNER_CONFIG_EXT *ptrTuner_Config);
		extern INT32 tun_dbcte702f1_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);
		extern INT32 tun_dbcte702f1_status(UINT32 tuner_id, UINT8 *lock);
		ptrQAM_Tuner->nim_Tuner_Init = tun_dbcte702f1_init;
		ptrQAM_Tuner->nim_Tuner_Control = tun_dbcte702f1_control;
		ptrQAM_Tuner->nim_Tuner_Status = tun_dbcte702f1_status;
	}
#elif (SYS_TUNER_MODULE == CD1616LF)
	{
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MAX = 0xBB;
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MIN = 0x2A;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MAX = 0xFF;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MIN = 0x86;//0x72
		ptrQAM_Tuner->tuner_config_data.AGC_REF = 0x80;

		ptrQAM_Tuner->tuner_config_ext.cTuner_special_config = 0x00; // 0x01-->RF AGC is disabled,other enable RF AGC
		ptrQAM_Tuner->tuner_config_ext.cChip = Tuner_Chip_CD1616LF_GIH;
		ptrQAM_Tuner->tuner_config_ext.cTuner_AGC_TOP	= 0;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Base_Addr = 0xC2; // C0, C2, C4,C6
		ptrQAM_Tuner->tuner_config_ext.cTuner_Crystal = 4;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Ref_DivRatio = 64;
		
		ptrQAM_Tuner->tuner_config_ext.cTuner_Step_Freq = 63;//round from 62.5
		//(Ref_divRatio*Step_Freq)=(80*50)=(64*62.5)=(24*166.7)=(31.25*128)
		ptrQAM_Tuner->tuner_config_ext.wTuner_IF_Freq = 36130;
		//ptrQAM_Tuner->tuner_config_ext.cTuner_Charge_Pump= 0;
		ptrQAM_Tuner->tuner_config_ext.i2c_type_id = I2C_TYPE_SCB;
		//ptrQAM_Tuner->tuner_config_ext.Tuner_Read= i2c_read;
		//ptrQAM_Tuner->tuner_config_ext.Tuner_Write= i2c_write;
		extern INT32 tun_cd1616lf_init(UINT32 * tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
		extern INT32 tun_cd1616lf_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);
		extern INT32 tun_cd1616lf_status(UINT32 tuner_id, UINT8 *lock);

		ptrQAM_Tuner->nim_Tuner_Init = tun_cd1616lf_init;
		ptrQAM_Tuner->nim_Tuner_Control = tun_cd1616lf_control;
		ptrQAM_Tuner->nim_Tuner_Status = tun_cd1616lf_status;
	}
#endif

}
#elif ((SYS_MAIN_BOARD == BOARD_S3202_DEMO_01V03) || \
	(SYS_MAIN_BOARD == BOARD_S3202_C3069_HSC2000LvA) || \
	(SYS_MAIN_BOARD == BOARD_S3202_C3058_C11)||\
	(SYS_MAIN_BOARD == BOARD_S3202_C3012_ECR5119))
static INT32 nim_s3202_Tuner_Attatch(struct QAM_TUNER_CONFIG_API *ptrQAM_Tuner)
{
	UINT8 data = 0;
	INT32 ret = SUCCESS;
	ret = i2c_read(I2C_TYPE_SCB, 0xC0, &data, 1);
#ifdef SSI_TS_OUT_IN_SET
		ptrQAM_Tuner->tuner_config_data.cTuner_Tsi_Setting=NIM_0_SSI_0;
#endif
	if (ret != SUCCESS)
	{
	    	ptrQAM_Tuner->tuner_config_data.RF_AGC_MAX		= 0xb8;
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MIN		= 0x14;
	//joey 20080504. change IF agc setting according to Jack's test result.	
	//trueve 20080806. restore IF AGC setting to max limitation according to Joey's suggestion.
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MAX		= 0xFF;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MIN		= 0x00;
		ptrQAM_Tuner->tuner_config_data.AGC_REF 			= 0x80;
	
		ptrQAM_Tuner->tuner_config_ext.cTuner_special_config = 0x01; // RF AGC is disabled
		ptrQAM_Tuner->tuner_config_ext.cChip				= Tuner_Chip_PHILIPS;
		ptrQAM_Tuner->tuner_config_ext.cTuner_AGC_TOP	= 1;//7; /*1 for single AGC, 7 for dual AGC */
		ptrQAM_Tuner->tuner_config_ext.cTuner_Base_Addr	= 0xC2;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Crystal		= 4;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Ref_DivRatio	= 64; 
		ptrQAM_Tuner->tuner_config_ext.cTuner_Step_Freq	= 62.5;
		//(Ref_divRatio*Step_Freq)=(80*50)=(64*62.5)=(24*166.7)
		ptrQAM_Tuner->tuner_config_ext.wTuner_IF_Freq		= 36000;
		//ptrQAM_Tuner->tuner_config_ext.cTuner_Charge_Pump= 0;  
		ptrQAM_Tuner->tuner_config_ext.i2c_type_id 			= I2C_TYPE_SCB;
	
	
	extern INT32 tun_dct70701_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
	extern INT32 tun_dct70701_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);	
	extern INT32 tun_dct70701_status(UINT32 tuner_id, UINT8 *lock);
	
	
	       ptrQAM_Tuner->nim_Tuner_Init					= tun_dct70701_init;
		ptrQAM_Tuner->nim_Tuner_Control					= tun_dct70701_control;
		ptrQAM_Tuner->nim_Tuner_Status					= tun_dct70701_status;
	}	
	else
	{
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MAX = 0xBA;
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MIN = 0x2A;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MAX = 0xFF;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MIN = 0x00;
		ptrQAM_Tuner->tuner_config_data.AGC_REF = 0x80;

		ptrQAM_Tuner->tuner_config_ext.cTuner_special_config = 0x01; // RF AGC is disabled
		ptrQAM_Tuner->tuner_config_ext.cChip = Tuner_Chip_ALPS;
		ptrQAM_Tuner->tuner_config_ext.cTuner_AGC_TOP	= 3;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Base_Addr = 0xC0; // C0, C2, C4,C6
		ptrQAM_Tuner->tuner_config_ext.cTuner_Crystal = 4;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Ref_DivRatio = 64;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Step_Freq = 62.5;
		//(Ref_divRatio*Step_Freq)=(80*50)=(64*62.5)=(24*166.7)=(31.25*128)
		ptrQAM_Tuner->tuner_config_ext.wTuner_IF_Freq = 36125;
		//ptrQAM_Tuner->tuner_config_ext.cTuner_Charge_Pump= 0;
		ptrQAM_Tuner->tuner_config_ext.i2c_type_id = I2C_TYPE_SCB;
		//ptrQAM_Tuner->tuner_config_ext.Tuner_Read			= i2c_read;
		//ptrQAM_Tuner->tuner_config_ext.Tuner_Write			= i2c_write;

		extern INT32 tun_alpstdae_init(UINT32 *tuner_id, struct QAM_TUNER_CONFIG_EXT *ptrTuner_Config);
		extern INT32 tun_alpstdae_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);
		extern INT32 tun_alpstdae_status(UINT32 tuner_id, UINT8 *lock);
		ptrQAM_Tuner->nim_Tuner_Init = tun_alpstdae_init;
		ptrQAM_Tuner->nim_Tuner_Control = tun_alpstdae_control;
		ptrQAM_Tuner->nim_Tuner_Status = tun_alpstdae_status;
	}

}
#endif


struct QAM_TUNER_CONFIG_API g_QAM_Tuner_Cfg;

void nim_tsi_config(void)
{
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
}

/***********************************
I2C
************************************/
void i2c_init()
{
#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01||SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01||SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)
	i2c_scb_attach(2);
#else
	i2c_scb_attach(1);
#endif
	i2c_gpio_attach(1);
	i2c_mode_set(I2C_TYPE_SCB, 40000, 1);
#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01||SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01||SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)
	i2c_mode_set(I2C_TYPE_SCB1, 40000, 1);
#endif	
	i2c_mode_set(I2C_TYPE_GPIO, 40000, 1);
}

/***********************************
DMX
************************************/
void dmx_dev_attach(void)
{
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
	dmx_config.video_bitrate_detect = 1;
	dmx_config.sync_times = 3;
#ifndef MINI_DRIVER_SUPPORT	
	dmx_m33_attach(&dmx_config);
#else
	dmx_m33_dev_attach(&dmx_config);
	dmx_m33_sec_enable(NULL);
#endif	
}

/***********************************
GE & OSD
************************************/
#ifndef MINI_DRIVER_SUPPORT
void ge_dev_attach(void)
{
	ge_driver_config_t para_config;
	struct ge_device *ge_dev = NULL;

	MEMSET(&para_config, 0, sizeof(ge_driver_config_t));
	para_config.ge_start_base = __MM_OSD_START_ADDR;
	para_config.GMA1_buf_size = __MM_OSD_LAYER0_LEN;
	para_config.GMA2_buf_size = __MM_OSD_LAYER1_LEN;
	para_config.temp_buf_size = 0;
	para_config.cmd_buf_size = __MM_GE_CMD_LEN;
	para_config.bpolling_cmd_status = TRUE;
	para_config.op_mutex_enable = 1;
	para_config.region_malloc_optimization = 1;
	ge_m33_attach(&para_config);

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
	
	osd_s36_attach("OSD_S36_0", &osd_c_config);
}
#else
void osd_dev_attach(void)
{
	OSD_DRIVER_CONFIG osd_c_config;
	
	MEMSET(&osd_c_config, 0, sizeof(OSD_DRIVER_CONFIG));
	osd_c_config.uMemBase = __MM_OSD_START_ADDR;
	osd_c_config.uMemSize = __MM_OSD_LAYER0_LEN;
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

	osd_m33_attach_mini("OSD_M33_MINI_0", &osd_c_config);

	osd_dev = (struct osd_device *)dev_get_by_id(HLD_DEV_TYPE_OSD, 0);
	//(SYS_SDRAM_SIZE > 8)
       osd_m3327c_enable_16M(osd_dev, __MM_OSD_BLOCK_ADDR);
}
#endif

/***********************************
UART
************************************/
void uart_init()
{
	libc_printf_uart_id_set(SCI_FOR_RS232);
	sci_16550uart_attach(1);
	sci_mode_set(SCI_FOR_RS232, 115200, SCI_PARITY_EVEN);
	FIXED_PRINTF("UPG  init ok\r\n");
}

/***********************************
FLASH
************************************/
void flash_init()
{
	flash_info_sl_init();
	sto_local_sflash_attach(NULL);	
	g_sto_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	if (g_sto_dev == NULL)
	{
		PRINTF("Can't find FLASH device!\n");
	}

	if (ALI_M3329E == sys_ic_get_chip_id() || 1 == sys_ic_is_M3202())
	{
#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01||SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01||SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)
		*((volatile UINT32*)0xb8008098) &= ~0xc0000000;
		if (g_sto_dev->totol_size <= 0x200000)
			*((volatile UINT32*)0xb8008098) |= 0;
		else if (g_sto_dev->totol_size > 0x200000 && g_sto_dev->totol_size <= 0x400000)
			*((volatile UINT32*)0xb8008098) |= 0x40000000;
		else if (g_sto_dev->totol_size > 0x400000 && g_sto_dev->totol_size <= 0x800000)
			*((volatile UINT32*)0xb8008098) |= 0x80000000;
		else
			*((volatile UINT32*)0xb8008098) |= 0xc0000000;
#else	
		*((volatile UINT32*)0xb8000098) &= ~0xc0000000;
		if (g_sto_dev->totol_size <= 0x200000)
			*((volatile UINT32*)0xb8000098) |= 0;
		else if (g_sto_dev->totol_size > 0x200000 && g_sto_dev->totol_size <= 0x400000)
			*((volatile UINT32*)0xb8000098) |= 0x40000000;
		else if (g_sto_dev->totol_size > 0x400000 && g_sto_dev->totol_size <= 0x800000)
			*((volatile UINT32*)0xb8000098) |= 0x80000000;
		else
			*((volatile UINT32*)0xb8000098) |= 0xc0000000;
#endif		
	}
	sto_open(g_sto_dev);
	sto_chunk_init(0, g_sto_dev->totol_size);
	flash_identify();

	chunk_reg_bootloader_id(STB_CHUNK_BOOTLOADER_ID);
}


/***********************************
PANEL
************************************/
/*this array defines number to digital conversion
 *
 *      ###b4##
 *    b2#     #b6
 *      ###b5##
 *    b1#     #b3
 *      ###b0##  #b7#
 */
struct pan_hw_info pan_hw_info_cp;

#define bitmap_list             NULL
#define bitmap_list_num     0

struct pan_configuration pan_config = {&pan_hw_info_cp, bitmap_list_num, bitmap_list};

void front_panel_init(UINT8* addr)
{
#ifdef PANEL_DISPLAY
	UINT32 i;
	UINT8 *t1, *t2;
	t1 = (UINT8*)&pan_hw_info_cp;
	t2 = addr;//(UINT8*)(SYS_FLASH_BASE_ADDR+HW_SET_GPIO);//&pan_hw_info;

	for(i=0; i<sizeof(struct pan_hw_info); i++)
		*(t1+i) = *(t2+i);
	
#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01||SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01||SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)
	pan_tp6x0_attach(&pan_config);
#else
	pan_hwscan_l_attach(&pan_config);
#endif

	g_pan_dev = (struct pan_device *)dev_get_by_id(HLD_DEV_TYPE_PAN, 0);   
	pan_open(g_pan_dev);
	set_pan_display_type(PAN_DISP_LED_BUF);
	stb_pan_display(" UPG", 4);	
#endif
}

/***********************************
GPIO
************************************/
void GPIO_onoff(unsigned int gpio,BOOL onoff)
{
	HAL_GPIO_BIT_DIR_SET(gpio, HAL_GPIO_O_DIR);
	HAL_GPIO_BIT_SET(gpio,onoff);
}

void GPIO_lan_phy_rst(BOOL onoff)
{// GPIO9: LAN_PHY_RST, 0:reset active; 1:work on
	GPIO_onoff(7,onoff);
}

void pinmux_init()
{
	*((volatile UINT32*)0xb8000100) |= ((1 << 2)|(1 << 3)|(1 << 6)|(1 << 12)|(1 << 13)|(1 << 14)|(1 << 18) ); /* RF_AGC_PDM[b17&b18],GPIO2[b1],GPIO7[b16] */
#ifdef SSI_TS_OUT_IN_SET
	*((volatile UINT32*)0xb8000100)&=~((1 << 18)|(1 << 19)|(1 << 21));
	*((volatile UINT32*)0xb8000100) |= ((1 << 20)|(1 << 24));
#endif
	*((volatile UINT32*)0xb8000104) = 0; 
#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01)
	//enable gpio
	*((volatile UINT32*)0xb8000040) = ((1 << 0) | (1 << 26) | (1 << 27) | (1 << 28));
	*((volatile UINT32*)0xb80000A0) = ((1 << 1) | (1 << 17)); 
	*((volatile UINT32*)0xb80000D0) = 0; 
#elif(SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)   
    	//enable gpio
	*((volatile UINT32*)0xb8000040) = ((1 << 23) | (1 << 24) | (1 << 25)|(1 << 26) | (1 << 27) | (1 << 28));
	*((volatile UINT32*)0xb80000A0) = ((1 << 0) | (1 << 1) | (1 << 17)); 
	*((volatile UINT32*)0xb80000D0) = 0; 
#elif(SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01)   
    	//enable gpio
	*((volatile UINT32*)0xb8000040) = ((1 << 23) | (1 << 24) | (1 << 25));
	*((volatile UINT32*)0xb80000A0) = ((1 << 0) | (1 << 1) | (1 << 12)|(1 << 13) | (1 << 14) | (1 << 17)); 
	*((volatile UINT32*)0xb80000D0) = 0; 
#elif(SYS_MAIN_BOARD == BOARD_S3202_DEMO_01V03)
    *((volatile UINT32*)0xb8000028) |= ((1 << 18) | (1 << 17) | (1 << 1) | (1 << 16)); /* RF_AGC_PDM[b17&b18],GPIO2[b1],GPIO7[b16] */
	*((volatile UINT32*)0xb800002c) |= (1 << 6); //SPIDF[b6]

	//enable hw7816 2
	*(volatile unsigned long*)(0xb800002c) |= (1 << 3);

    if(IC_REV_0!=sys_ic_get_rev_id())
    {
        /*uart1     tx/eeprom_addr[13], pin52, (2ch[1:0]  1:tx, 2:rx, 3:txrx )
                       rx/eeprom_addr[12], pin53, (28h[10:9] 0:tx, 1:rx, 2:txrx )*/
        *((volatile UINT8 *)0xb800002c) &= (~0x03);
        *((volatile UINT8 *)0xb800002c) |= 0x01;
        *((volatile UINT8 *)0xb8000029) &= (~0x06);
        *((volatile UINT8 *)0xb8000029) |= 0x02;
    }  
#endif
}

