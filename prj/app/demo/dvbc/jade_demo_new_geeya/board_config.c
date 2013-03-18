#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <hal/hal_gpio.h>
#include <hld/pan/pan.h>
#include <hld/pan/pan_dev.h>
#include <hld/smc/smc.h>
#include <hld/dis/vpo.h>
//#include <api/libosd/osd_vkey.h>
#include <bus/i2c/i2c.h>
#include <hld/snd/snd.h>
#include <hld/nim/Nim_tuner.h>
#include "key.h"
#include "control.h"
#include "win_com.h"
/********************************************************************************
 *																				*
 *							S3202C DEMO BOARD 01V01								*
 *																				*
 ********************************************************************************/
#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01)
/* TV ENC config */
struct tve_adjust g_tve_adjust_table[] =
{
    {TVE_ADJ_COMPOSITE_Y_DELAY, SYS_625_LINE, 1}, {TVE_ADJ_COMPOSITE_Y_DELAY, SYS_525_LINE, 1}, 
    {TVE_ADJ_COMPOSITE_C_DELAY, SYS_625_LINE, 0}, {TVE_ADJ_COMPOSITE_C_DELAY, SYS_525_LINE, 0}, 
    {TVE_ADJ_COMPONENT_Y_DELAY, SYS_625_LINE, 3}, {TVE_ADJ_COMPONENT_Y_DELAY, SYS_525_LINE, 3}, 
    {TVE_ADJ_COMPONENT_CB_DELAY, SYS_625_LINE, 2}, {TVE_ADJ_COMPONENT_CB_DELAY, SYS_525_LINE, 0}, 
    {TVE_ADJ_COMPONENT_CR_DELAY, SYS_625_LINE, 0}, {TVE_ADJ_COMPONENT_CR_DELAY, SYS_525_LINE, 2}, 
    {TVE_ADJ_BURST_LEVEL_ENABLE, SYS_625_LINE, 1}, {TVE_ADJ_BURST_LEVEL_ENABLE, SYS_525_LINE, 0}, //8 bits
    {TVE_ADJ_BURST_CB_LEVEL, SYS_625_LINE, 0x8f}, {TVE_ADJ_BURST_CB_LEVEL, SYS_525_LINE, 0x68}, //8 bits
    {TVE_ADJ_BURST_CR_LEVEL, SYS_625_LINE, 0x53}, {TVE_ADJ_BURST_CR_LEVEL, SYS_525_LINE, 0}, //8 bits
    {TVE_ADJ_COMPOSITE_LUMA_LEVEL, SYS_625_LINE, 0x51}, {TVE_ADJ_COMPOSITE_LUMA_LEVEL, SYS_525_LINE, 0x4c}, //8 bits
#ifdef Enable_108MHz_Video_Sample_Rate    
    {TVE_ADJ_COMPOSITE_CHRMA_LEVEL, SYS_625_LINE, 0x07}, {TVE_ADJ_COMPOSITE_CHRMA_LEVEL, SYS_525_LINE, 0x06}, //attenuate chroma level
#else
    {TVE_ADJ_COMPOSITE_CHRMA_LEVEL, SYS_625_LINE, 0x1b}, {TVE_ADJ_COMPOSITE_CHRMA_LEVEL, SYS_525_LINE, 0x19}, //8 bits
#endif
    {TVE_ADJ_PHASE_COMPENSATION, SYS_625_LINE, 0x290}, {TVE_ADJ_PHASE_COMPENSATION, SYS_525_LINE, 0x4bf}, //16 bits
 #ifdef Enable_108MHz_Video_Sample_Rate
    {TVE_ADJ_VIDEO_FREQ_RESPONSE, SYS_625_LINE, 0x0}, {TVE_ADJ_VIDEO_FREQ_RESPONSE, SYS_525_LINE, 0x0}, //disable freq response increase
#else
    {TVE_ADJ_VIDEO_FREQ_RESPONSE, SYS_625_LINE, 0x102}, {TVE_ADJ_VIDEO_FREQ_RESPONSE, SYS_525_LINE, 0x102}, //16 bits
#endif
};

struct tve_adjust g_tve_adjust_table_adv[] =
{
    {TVE_ADJ_ADV_PEDESTAL_ONOFF, SYS_625_LINE, 0},              {TVE_ADJ_ADV_PEDESTAL_ONOFF, SYS_525_LINE, 1},
    {TVE_ADJ_ADV_COMPONENT_LUM_LEVEL, SYS_625_LINE, 0x51},      {TVE_ADJ_ADV_COMPONENT_LUM_LEVEL, SYS_525_LINE, 0x51},
    {TVE_ADJ_ADV_COMPONENT_CHRMA_LEVEL, SYS_625_LINE, 0x4f},    {TVE_ADJ_ADV_COMPONENT_CHRMA_LEVEL, SYS_525_LINE, 0x50},
    {TVE_ADJ_ADV_COMPONENT_PEDESTAL_LEVEL, SYS_625_LINE, 0x0},  {TVE_ADJ_ADV_COMPONENT_PEDESTAL_LEVEL, SYS_525_LINE, 0x0},
    {TVE_ADJ_ADV_COMPONENT_SYNC_LEVEL, SYS_625_LINE, 0x1},      {TVE_ADJ_ADV_COMPONENT_SYNC_LEVEL, SYS_525_LINE, 0x1},
    {TVE_ADJ_ADV_RGB_R_LEVEL, SYS_625_LINE, 0x87},              {TVE_ADJ_ADV_RGB_R_LEVEL, SYS_525_LINE, 0x8a},
    {TVE_ADJ_ADV_RGB_G_LEVEL, SYS_625_LINE, 0x8a},              {TVE_ADJ_ADV_RGB_G_LEVEL, SYS_525_LINE, 0x8b},
    {TVE_ADJ_ADV_RGB_B_LEVEL, SYS_625_LINE, 0x88},              {TVE_ADJ_ADV_RGB_B_LEVEL, SYS_525_LINE, 0x8b},
    {TVE_ADJ_ADV_COMPOSITE_PEDESTAL_LEVEL, SYS_625_LINE, 0x0},  {TVE_ADJ_ADV_COMPOSITE_PEDESTAL_LEVEL, SYS_525_LINE, 0x2b},
    {TVE_ADJ_ADV_COMPOSITE_SYNC_LEVEL, SYS_625_LINE, 0x1},      {TVE_ADJ_ADV_COMPOSITE_SYNC_LEVEL, SYS_525_LINE, 0x7},
};

/* Smart card init */
void smartcard_init(UINT32 param1, UINT32 param2)
{
	struct smc_dev_config smc_config;
	UINT32 init_clk = 3570000;

	MEMSET(&smc_config, 0, sizeof(struct smc_dev_config));
	/***S3202B need not use GPIO detecting
	smc_config.gpio_cd_trigger = 1;
	smc_config.gpio_cd_pol = 0;
	smc_config.gpio_cd_io = HAL_GPIO_I_DIR;
	smc_config.gpio_cd_pos = 7;
	*/
	smc_config.default_etu = 372;
	smc_config.def_etu_trigger = 1;
	smc_config.force_tx_rx_trigger = 1;
	smc_config.apd_disable_trigger = 1;
	smc_config.invert_detect = 0;
#ifdef MULTI_CAS
#if(CAS_TYPE == CAS_DVN)  //for dvn card,can not do pps.
	smc_config.disable_pps = 1;
	init_clk = 5000000;
#elif((CAS_TYPE==CAS_CDCA) ||(CAS_TYPE == CAS_TF) || (CAS_TYPE == CAS_DVT) ||(CAS_TYPE==CAS_GY))
	//init_clk = 5000000;
#elif(CAS_TYPE == CAS_IRDETO)
	smc_config.def_etu_trigger = 0;
	smc_config.disable_pps = 1;
	init_clk = 6000000;
#elif(CAS_TYPE == CAS_CTI)
	init_clk = 3570000;
#endif

	smc_config.init_clk_trigger = 1;
	smc_config.init_clk_number = 1;
	smc_config.init_clk_array = &init_clk;

#endif
	smc_dev_attach(0, &smc_config);
}

/* pin mux */
void pin_mux_config(UINT32 param1, UINT32 param2)
{
#if 0
	*((volatile UINT32*)0xb8000100) |= ((1 << 2)|(1 << 3)|(1 << 6)|(1 << 12)|(1 << 13)|(1 << 14)|(1 << 18) ); /* RF_AGC_PDM[b17&b18],GPIO2[b1],GPIO7[b16] */
	*((volatile UINT32*)0xb8000104) = 0; 

	//enable gpio
	*((volatile UINT32*)0xb8000040) = ((1 << 0) | (1 << 26) | (1 << 27) | (1 << 28));
	*((volatile UINT32*)0xb80000A0) = ((1 << 1) | (1 << 17)); 
	*((volatile UINT32*)0xb80000D0) = 0; 
#else
	*((volatile UINT32*)0xb8000100) |= ((1 << 2)|(1 << 3)|(1 << 6)|(1 << 12)|(1 << 13)|(1 << 14)|(1 << 18) ); /* RF_AGC_PDM[b17&b18],GPIO2[b1],GPIO7[b16] */
	*((volatile UINT32*)0xb8000104) = 0; 

	//enable gpio
	*((volatile UINT32*)0xb8000040) = ((1 << 0) | (1 << 26) | (1 << 27) | (1 << 28));
	*((volatile UINT32*)0xb80000A0) = ((1 << 1) | (1 << 17)); 
	*((volatile UINT32*)0xb80000D0) = 0; 
	//enable GPIO[1] power	
	*((volatile UINT32*)0xb8000100) |= (1 << 12);	//pinmux spdif gpio[1]
	*((volatile UINT32*)0xb8000040) |= (1<<0);	// gpio enable
	*((volatile UINT32*)0xb8000058) |= (1<<0);	// output enable
	*((volatile UINT32*)0xb8000054) |= (1<<0);	// output 1
    //*((volatile UINT32*)0xb8000074) |= ((1<<4 )| (1<<25));//154M MEMERY
#if 1//i2c reset 
    *((volatile UINT32*)0xb8000060) |= (1<<7); //reset scb1
    osal_task_sleep(100);
    *((volatile UINT32*)0xb8000060) &= (~((UINT32)1<<7)); //clear reset scb1
#endif
#endif
}

/* Panel */
#define bitmap_list             NULL
#define bitmap_list_num     0
struct pan_hw_info pan_hw_info =
{
	0,  /* type_kb : 2; Key board (array) type */
	1,  /* type_scan : 1; 0: Slot scan, 1: Shadow scan */
	1,  /* type_key: 1; Key exit or not */
	1,  /* type_irp: 3; 0: not IRP, 1: NEC, 2: LAB */
	0,  /* type_mcu: 1; MCU exit or not */
	4,  /* num_com: 4; Number of com PIN, 0 to 8 */
	1,  /* Position of colon flag, 0 to 7 */
	1,  /* num_scan: 2; Number of scan PIN, 0 to 2 */
	0,  /* rsvd_bits:6; Reserved bits */
	0,  /* rsvd_byte; Reserved byte, for alignment */
    {0, HAL_GPIO_O_DIR, 28},		/* LATCH PIN */
    {1, HAL_GPIO_O_DIR, 27},		/* CLOCK PIN */
    {1, HAL_GPIO_O_DIR, 26},		/* DATA PIN */
    {{0, HAL_GPIO_I_DIR, 63},		/* SCAN1 PIN */
     {0, HAL_GPIO_I_DIR, 63}},		/* SCAN2 PIN */
    {{0, HAL_GPIO_O_DIR, 63},		/* COM1 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM2 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM3 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM4 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM5 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM6 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM7 PIN */
     {0, HAL_GPIO_O_DIR, 63}},		/* COM8 PIN */
    {{0, HAL_GPIO_O_DIR, 63},		/* POWER PIN */
     {1, HAL_GPIO_O_DIR, 49},		/* LOCK PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* Extend function LBD */
     {0, HAL_GPIO_O_DIR, 63}},		/* Extend function LBD */
    {0, HAL_GPIO_O_DIR, 63},		/* Reserve */
	300,  /* Intv repeat first */
	250,  /* Intv repeat */
	350,  /* Intv release */
	NULL,  /* hook_scan() callback */
	NULL,  /* hook_show() callback */
};

struct pan_configuration pan_config =
{
	 &pan_hw_info, bitmap_list_num, bitmap_list
};

/* VDAC */
UINT8 board_vdac_config[VDAC_NUM_MAX] = 
{VDAC_CVBS, VDAC_YUV_U, VDAC_YUV_Y, VDAC_YUV_V, VDAC_NULL, VDAC_NULL};
//{VDAC_CVBS, VDAC_CVBS, VDAC_CVBS, VDAC_CVBS, VDAC_NULL, VDAC_NULL};


/********************************************************************************
 *																				*
 *							S3202C DEMO BOARD 02V01							    *
 *																				*
 ********************************************************************************/
#elif (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)
/* TV ENC config */
struct tve_adjust g_tve_adjust_table[] =
{
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

/* Smart card init */
void smartcard_init(UINT32 param1, UINT32 param2)
{
	struct smc_dev_config smc_config;
	UINT32 init_clk = 3570000;

	MEMSET(&smc_config, 0, sizeof(struct smc_dev_config));
	/***S3202B need not use GPIO detecting
	smc_config.gpio_cd_trigger = 1;
	smc_config.gpio_cd_pol = 0;
	smc_config.gpio_cd_io = HAL_GPIO_I_DIR;
	smc_config.gpio_cd_pos = 7;
	*/
	smc_config.default_etu = 372;
	smc_config.def_etu_trigger = 1;
	smc_config.force_tx_rx_trigger = 1;
	smc_config.apd_disable_trigger = 1;
	smc_config.invert_detect = 0;
#ifdef MULTI_CAS
#if(CAS_TYPE == CAS_DVN)  //for dvn card,can not do pps.
	smc_config.disable_pps = 1;
	init_clk = 5000000;
#elif(CAS_TYPE==CAS_CDCA3 ||CAS_TYPE == CAS_TF || CAS_TYPE == CAS_DVT||(CAS_TYPE==CAS_GY))
	//init_clk = 5000000;
#elif(CAS_TYPE == CAS_IRDETO)
	smc_config.def_etu_trigger = 0;
	smc_config.disable_pps = 1;
	init_clk = 6000000;
#elif(CAS_TYPE == CAS_CTI)
	init_clk = 3570000;
#endif

	smc_config.init_clk_trigger = 1;
	smc_config.init_clk_number = 1;
	smc_config.init_clk_array = &init_clk;

#endif
	smc_dev_attach(0, &smc_config);
}

/* pin mux */
void pin_mux_config(UINT32 param1, UINT32 param2)
{
#if 0
	*((volatile UINT32*)0xb8000100) |= ((1 << 2)|(1 << 3)|(1 << 6)|(1 << 12)|(1 << 13)|(1 << 14)|(1 << 18) ); /* RF_AGC_PDM[b17&b18],GPIO2[b1],GPIO7[b16] */
	*((volatile UINT32*)0xb8000104) = 0; 

	//enable gpio
	*((volatile UINT32*)0xb8000040) = ((1 << 0) | (1 << 26) | (1 << 27) | (1 << 28));
	*((volatile UINT32*)0xb80000A0) = ((1 << 1) | (1 << 17)); 
	*((volatile UINT32*)0xb80000D0) = 0; 
#else
	*((volatile UINT32*)0xb8000100) |= ((1 << 2)|(1 << 3)|(1 << 6)|(1 << 12)|(1 << 13)|(1 << 14)|(1 << 18) ); /* RF_AGC_PDM[b17&b18],GPIO2[b1],GPIO7[b16] */
#ifdef SSI_TS_OUT_IN_SET
	*((volatile UINT32*)0xb8000100)&=~((1 << 18)|(1 << 19)|(1 << 21));
	*((volatile UINT32*)0xb8000100) |= ((1 << 20)|(1 << 24));
#endif
	*((volatile UINT32*)0xb8000104) = 0; 

	//enable gpio
	*((volatile UINT32*)0xb8000040) = ((1 << 23) | (1 << 24) | (1 << 25)|(1 << 26) | (1 << 27) | (1 << 28));
	*((volatile UINT32*)0xb80000A0) = ((1 << 0) | (1 << 1) | (1 << 17)); 
	*((volatile UINT32*)0xb80000D0) = 0; 
#if 0 //loader will open the power
	//enable GPIO[32] power	
	*((volatile UINT32*)0xb80000A0) |= (1<<0);	//enable gpio 32
	*((volatile UINT32*)0xb80000B8) |= (1<<0);	// set gpio 32 to be output
	*((volatile UINT32*)0xb80000B4) |= (1<<0);	// set gpio 32 output high level
#endif
#if 1//i2c reset 
    *((volatile UINT32*)0xb8000060) |= (1<<7); //reset scb1
    osal_task_sleep(100);
    *((volatile UINT32*)0xb8000060) &= (~((UINT32)1<<7)); //clear reset scb1
#endif
#endif
}

/* Panel */
#define bitmap_list             NULL
#define bitmap_list_num     0
struct pan_hw_info pan_hw_info =
{
	0,  /* type_kb : 2; Key board (array) type */
	1,  /* type_scan : 1; 0: Slot scan, 1: Shadow scan */
	1,  /* type_key: 1; Key exit or not */
	1,  /* type_irp: 3; 0: not IRP, 1: NEC, 2: LAB */
	0,  /* type_mcu: 1; MCU exit or not */
	4,  /* num_com: 4; Number of com PIN, 0 to 8 */
	1,  /* Position of colon flag, 0 to 7 */
	1,  /* num_scan: 2; Number of scan PIN, 0 to 2 */
	0,  /* rsvd_bits:6; Reserved bits */
	0,  /* rsvd_byte; Reserved byte, for alignment */
    {0, HAL_GPIO_O_DIR, 28},		/* LATCH PIN */
    {1, HAL_GPIO_O_DIR, 27},		/* CLOCK PIN */
    {1, HAL_GPIO_O_DIR, 26},		/* DATA PIN */
    {{0, HAL_GPIO_I_DIR, 63},		/* SCAN1 PIN */
     {0, HAL_GPIO_I_DIR, 63}},		/* SCAN2 PIN */
    {{0, HAL_GPIO_O_DIR, 63},		/* COM1 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM2 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM3 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM4 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM5 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM6 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM7 PIN */
     {0, HAL_GPIO_O_DIR, 63}},		/* COM8 PIN */
    {{0, HAL_GPIO_O_DIR, 32},		/* POWER PIN */
     {1, HAL_GPIO_O_DIR, 49},		/* LOCK PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* Extend function LBD */
     {0, HAL_GPIO_O_DIR, 63}},		/* Extend function LBD */
    {0, HAL_GPIO_O_DIR, 63},		/* Reserve */
	300,  /* Intv repeat first */
	250,  /* Intv repeat */
	350,  /* Intv release */
	NULL,  /* hook_scan() callback */
	NULL,  /* hook_show() callback */
};

struct pan_configuration pan_config =
{
	 &pan_hw_info, bitmap_list_num, bitmap_list
};

/* VDAC */
UINT8 board_vdac_config[VDAC_NUM_MAX] = 
{VDAC_YUV_V, VDAC_CVBS , VDAC_YUV_U, VDAC_YUV_Y, VDAC_NULL, VDAC_NULL};
//{VDAC_CVBS, VDAC_YUV_U, VDAC_YUV_Y, VDAC_YUV_V, VDAC_NULL, VDAC_NULL};
//{VDAC_CVBS, VDAC_CVBS, VDAC_CVBS, VDAC_CVBS, VDAC_NULL, VDAC_NULL};

/********************************************************************************

/********************************************************************************
 *																				*
 *							S3202C NEWLAND BOARD 02V01							*
 *																				*
 ********************************************************************************/
#elif (SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01)
/* TV ENC config */
struct tve_adjust g_tve_adjust_table[] =
{
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

/* Smart card init */
void smartcard_init(UINT32 param1, UINT32 param2)
{
	struct smc_dev_config smc_config;
	UINT32 init_clk = 3570000;

	MEMSET(&smc_config, 0, sizeof(struct smc_dev_config));
	/***S3202B need not use GPIO detecting
	smc_config.gpio_cd_trigger = 1;
	smc_config.gpio_cd_pol = 0;
	smc_config.gpio_cd_io = HAL_GPIO_I_DIR;
	smc_config.gpio_cd_pos = 7;
	*/
	smc_config.default_etu = 372;
	smc_config.def_etu_trigger = 1;
	smc_config.force_tx_rx_trigger = 1;
	smc_config.apd_disable_trigger = 1;
	smc_config.invert_detect = 1;
#ifdef MULTI_CAS
#if(CAS_TYPE == CAS_DVN)  //for dvn card,can not do pps.
	smc_config.disable_pps = 1;
	init_clk = 5000000;
#elif(CAS_TYPE==CAS_CDCA3 ||CAS_TYPE == CAS_TF || CAS_TYPE == CAS_DVT||(CAS_TYPE==CAS_GY))
	//init_clk = 5000000;
#elif(CAS_TYPE == CAS_IRDETO)
	smc_config.def_etu_trigger = 0;
	smc_config.disable_pps = 1;
	init_clk = 6000000;
#elif(CAS_TYPE == CAS_CTI)
	init_clk = 3570000;
#endif

	smc_config.init_clk_trigger = 1;
	smc_config.init_clk_number = 1;
	smc_config.init_clk_array = &init_clk;

#endif
	smc_dev_attach(0, &smc_config);
}

/* pin mux */
void pin_mux_config(UINT32 param1, UINT32 param2)
{
#if 0
	*((volatile UINT32*)0xb8000100) |= ((1 << 2)|(1 << 3)|(1 << 6)|(1 << 12)|(1 << 13)|(1 << 14)|(1 << 18) ); /* RF_AGC_PDM[b17&b18],GPIO2[b1],GPIO7[b16] */
	*((volatile UINT32*)0xb8000104) = 0; 

	//enable gpio
	*((volatile UINT32*)0xb8000040) = ((1 << 0) | (1 << 26) | (1 << 27) | (1 << 28));
	*((volatile UINT32*)0xb80000A0) = ((1 << 1) | (1 << 17)); 
	*((volatile UINT32*)0xb80000D0) = 0; 
#else
	*((volatile UINT32*)0xb8000100) |= ((1 << 2)|(1 << 3)|(1 << 6)|(1 << 12)|(1 << 13)|(1 << 14)|(1 << 18) ); /* RF_AGC_PDM[b17&b18],GPIO2[b1],GPIO7[b16] */
	*((volatile UINT32*)0xb8000104) = 0; 

	//enable gpio
	*((volatile UINT32*)0xb8000040) = ((1 << 23) | (1 << 24) | (1 << 25));
	*((volatile UINT32*)0xb80000A0) = ((1 << 0) | (1 << 1) | (1 << 12)|(1 << 13) | (1 << 14) | (1 << 17)); 
	*((volatile UINT32*)0xb80000D0) = 0; 
#if 0 //loader will open the power
	//enable GPIO[32] power	
	*((volatile UINT32*)0xb80000A0) |= (1<<0);	//enable gpio 32
	*((volatile UINT32*)0xb80000B8) |= (1<<0);	// set gpio 32 to be output
	*((volatile UINT32*)0xb80000B4) |= (1<<0);	// set gpio 32 output high level
#endif

#if 1//i2c reset 
    *((volatile UINT32*)0xb8000060) |= (1<<7); //reset scb1
    osal_task_sleep(100);
    *((volatile UINT32*)0xb8000060) &= (~((UINT32)1<<7)); //clear reset scb1
#endif

#endif
}

/* Panel */
#define bitmap_list             NULL
#define bitmap_list_num     0
struct pan_hw_info pan_hw_info =
{
	0,  /* type_kb : 2; Key board (array) type */
	1,  /* type_scan : 1; 0: Slot scan, 1: Shadow scan */
	1,  /* type_key: 1; Key exit or not */
	1,  /* type_irp: 3; 0: not IRP, 1: NEC, 2: LAB */
	0,  /* type_mcu: 1; MCU exit or not */
	4,  /* num_com: 4; Number of com PIN, 0 to 8 */
	1,  /* Position of colon flag, 0 to 7 */
	1,  /* num_scan: 2; Number of scan PIN, 0 to 2 */
	0,  /* rsvd_bits:6; Reserved bits */
	0,  /* rsvd_byte; Reserved byte, for alignment */
    {0, HAL_GPIO_O_DIR, 46},		/* LATCH PIN */
    {1, HAL_GPIO_O_DIR, 45},		/* CLOCK PIN */
    {1, HAL_GPIO_O_DIR, 44},		/* DATA PIN */
    {{0, HAL_GPIO_I_DIR, 63},		/* SCAN1 PIN */
     {0, HAL_GPIO_I_DIR, 63}},		/* SCAN2 PIN */
    {{0, HAL_GPIO_O_DIR, 63},		/* COM1 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM2 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM3 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM4 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM5 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM6 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM7 PIN */
     {0, HAL_GPIO_O_DIR, 63}},		/* COM8 PIN */
    {{0, HAL_GPIO_O_DIR, 32},		/* POWER PIN */
     {0, HAL_GPIO_O_DIR, 49},		/* LOCK PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* Extend function LBD */
     {0, HAL_GPIO_O_DIR, 63}},		/* Extend function LBD */
    {0, HAL_GPIO_O_DIR, 63},		/* Reserve */
	300,  /* Intv repeat first */
	250,  /* Intv repeat */
	350,  /* Intv release */
	NULL,  /* hook_scan() callback */
	NULL,  /* hook_show() callback */
};

struct pan_configuration pan_config =
{
	 &pan_hw_info, bitmap_list_num, bitmap_list
};

/* VDAC */
UINT8 board_vdac_config[VDAC_NUM_MAX] = 
{VDAC_YUV_U, VDAC_CVBS, VDAC_YUV_Y, VDAC_YUV_V, VDAC_NULL, VDAC_NULL};
//{VDAC_CVBS, VDAC_YUV_U, VDAC_YUV_Y, VDAC_YUV_V, VDAC_NULL, VDAC_NULL};
//{VDAC_CVBS, VDAC_CVBS, VDAC_CVBS, VDAC_CVBS, VDAC_NULL, VDAC_NULL};

/********************************************************************************


/********************************************************************************
 *																				*
 *							S3202 DEMO BOARD 01V03								*
 *																				*
 ********************************************************************************/
#elif (SYS_MAIN_BOARD == BOARD_S3202_DEMO_01V03)
/* TV ENC config */
struct tve_adjust g_tve_adjust_table[] =
{
    {TVE_ADJ_COMPOSITE_Y_DELAY, SYS_625_LINE, 1}, {TVE_ADJ_COMPOSITE_Y_DELAY, SYS_525_LINE, 1}, 
    {TVE_ADJ_COMPOSITE_C_DELAY, SYS_625_LINE, 0}, {TVE_ADJ_COMPOSITE_C_DELAY, SYS_525_LINE, 0}, 
    {TVE_ADJ_COMPONENT_Y_DELAY, SYS_625_LINE, 3}, {TVE_ADJ_COMPONENT_Y_DELAY, SYS_525_LINE, 3}, 
    {TVE_ADJ_COMPONENT_CB_DELAY, SYS_625_LINE, 2}, {TVE_ADJ_COMPONENT_CB_DELAY, SYS_525_LINE, 2}, 
    {TVE_ADJ_COMPONENT_CR_DELAY, SYS_625_LINE, 0}, {TVE_ADJ_COMPONENT_CR_DELAY, SYS_525_LINE, 0}, 
    {TVE_ADJ_BURST_LEVEL_ENABLE, SYS_625_LINE, 1}, {TVE_ADJ_BURST_LEVEL_ENABLE, SYS_525_LINE, 0}, //8 bits
    {TVE_ADJ_BURST_CB_LEVEL, SYS_625_LINE, 0x8f}, {TVE_ADJ_BURST_CB_LEVEL, SYS_525_LINE, 0x68}, //8 bits
    {TVE_ADJ_BURST_CR_LEVEL, SYS_625_LINE, 0x53}, {TVE_ADJ_BURST_CR_LEVEL, SYS_525_LINE, 0}, //8 bits
    {TVE_ADJ_COMPOSITE_LUMA_LEVEL, SYS_625_LINE, 0x53}, {TVE_ADJ_COMPOSITE_LUMA_LEVEL, SYS_525_LINE, 0x4c}, //8 bits
#ifdef Enable_108MHz_Video_Sample_Rate    
    {TVE_ADJ_COMPOSITE_CHRMA_LEVEL, SYS_625_LINE, 0xa}, {TVE_ADJ_COMPOSITE_CHRMA_LEVEL, SYS_525_LINE, 0x10}, //attenuate chroma level
#else
    {TVE_ADJ_COMPOSITE_CHRMA_LEVEL, SYS_625_LINE, 0x1b}, {TVE_ADJ_COMPOSITE_CHRMA_LEVEL, SYS_525_LINE, 0x19}, //8 bits
#endif
    {TVE_ADJ_PHASE_COMPENSATION, SYS_625_LINE, 0x2b0}, {TVE_ADJ_PHASE_COMPENSATION, SYS_525_LINE, 0x4c8}, //16 bits
 #ifdef Enable_108MHz_Video_Sample_Rate
    {TVE_ADJ_VIDEO_FREQ_RESPONSE, SYS_625_LINE, 0x0}, {TVE_ADJ_VIDEO_FREQ_RESPONSE, SYS_525_LINE, 0x0}, //disable freq response increase
#else
    {TVE_ADJ_VIDEO_FREQ_RESPONSE, SYS_625_LINE, 0x102}, {TVE_ADJ_VIDEO_FREQ_RESPONSE, SYS_525_LINE, 0x102}, //16 bits
#endif
};

struct tve_adjust g_tve_adjust_table_adv[] =
{
    {TVE_ADJ_ADV_PEDESTAL_ONOFF, SYS_625_LINE, 0},              {TVE_ADJ_ADV_PEDESTAL_ONOFF, SYS_525_LINE, 1},
    {TVE_ADJ_ADV_COMPONENT_LUM_LEVEL, SYS_625_LINE, 0x50},      {TVE_ADJ_ADV_COMPONENT_LUM_LEVEL, SYS_525_LINE, 0x50},
    {TVE_ADJ_ADV_COMPONENT_CHRMA_LEVEL, SYS_625_LINE, 0x4d},    {TVE_ADJ_ADV_COMPONENT_CHRMA_LEVEL, SYS_525_LINE, 0x4d},
    {TVE_ADJ_ADV_COMPONENT_PEDESTAL_LEVEL, SYS_625_LINE, 0x0},  {TVE_ADJ_ADV_COMPONENT_PEDESTAL_LEVEL, SYS_525_LINE, 0x9},
    {TVE_ADJ_ADV_COMPONENT_SYNC_LEVEL, SYS_625_LINE, 0x5},      {TVE_ADJ_ADV_COMPONENT_SYNC_LEVEL, SYS_525_LINE, 0xa},
    {TVE_ADJ_ADV_RGB_R_LEVEL, SYS_625_LINE, 0x87},              {TVE_ADJ_ADV_RGB_R_LEVEL, SYS_525_LINE, 0x8a},
    {TVE_ADJ_ADV_RGB_G_LEVEL, SYS_625_LINE, 0x8a},              {TVE_ADJ_ADV_RGB_G_LEVEL, SYS_525_LINE, 0x8b},
    {TVE_ADJ_ADV_RGB_B_LEVEL, SYS_625_LINE, 0x88},              {TVE_ADJ_ADV_RGB_B_LEVEL, SYS_525_LINE, 0x8b},
    {TVE_ADJ_ADV_COMPOSITE_PEDESTAL_LEVEL, SYS_625_LINE, 0x0},  {TVE_ADJ_ADV_COMPOSITE_PEDESTAL_LEVEL, SYS_525_LINE, 0x2b},
    {TVE_ADJ_ADV_COMPOSITE_SYNC_LEVEL, SYS_625_LINE, 0x0},      {TVE_ADJ_ADV_COMPOSITE_SYNC_LEVEL, SYS_525_LINE, 0x5},
};

/* Smart card init */
void smartcard_init(UINT32 param1, UINT32 param2)
{
	struct smc_dev_config smc_config;
	UINT32 init_clk = 3570000;

	MEMSET(&smc_config, 0, sizeof(struct smc_dev_config));
	/***S3202B need not use GPIO detecting
	smc_config.gpio_cd_trigger = 1;
	smc_config.gpio_cd_pol = 0;
	smc_config.gpio_cd_io = HAL_GPIO_I_DIR;
	smc_config.gpio_cd_pos = 7;
	*/
	smc_config.default_etu = 372;
	smc_config.def_etu_trigger = 1;
	smc_config.force_tx_rx_trigger = 1;
	smc_config.apd_disable_trigger = 1;
#ifdef MULTI_CAS
#if(CAS_TYPE == CAS_DVN)  //for dvn card,can not do pps.
	smc_config.disable_pps = 1;
	init_clk = 5000000;
#elif(CAS_TYPE==CAS_CDCA || CAS_TYPE == CAS_TF || CAS_TYPE == CAS_DVT||(CAS_TYPE==CAS_GY))
	init_clk = 5000000;
#elif(CAS_TYPE == CAS_IRDETO)
	smc_config.def_etu_trigger = 0;
	smc_config.disable_pps = 1;
	init_clk = 6000000;
#elif(CAS_TYPE == CAS_CTI)
	init_clk = 3570000;
#endif

	smc_config.init_clk_trigger = 1;
	smc_config.init_clk_number = 1;
	smc_config.init_clk_array = &init_clk;

#endif
	smc_dev_attach(1, &smc_config);
}

/* pin mux */
void pin_mux_config(UINT32 param1, UINT32 param2)
{
	*((volatile UINT32*)0xb8000028) |= ((1 << 18) | (1 << 17) | (1 << 1) | (1 << 16)); /* RF_AGC_PDM[b17&b18],GPIO2[b1],GPIO7[b16] */
	*((volatile UINT32*)0xb800002c) |= (1 << 6); //SPIDF[b6]

#if(CAS_TYPE==CAS_IRDETO)
	*((volatile UINT32*)0xb8000028) |= ( (1 << 21)|(1 << 22));//gpio18,17 as i2c[b22,21]	
#endif

	//enable hw7816 1, reserve for future use
	//*(volatile unsigned long*)(0xb800002c) |=(1<<2);

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
}

/* Panel */
#define bitmap_list             NULL
#define bitmap_list_num     0
struct pan_hw_info pan_hw_info =
{
	0,  /* type_kb : 2; Key board (array) type */
	0,  /* type_scan : 1; 0: Slot scan, 1: Shadow scan */
	1,  /* type_key: 1; Key exit or not */
	1,  /* type_irp: 3; 0: not IRP, 1: NEC, 2: LAB */
	0,  /* type_mcu: 1; MCU exit or not */
	4,  /* num_com: 4; Number of com PIN, 0 to 8 */
	1,  /* Position of colon flag, 0 to 7 */
	2,  /* num_scan: 2; Number of scan PIN, 0 to 2 */
	0,  /* rsvd_bits:6; Reserved bits */
	0,  /* rsvd_byte; Reserved byte, for alignment */
    {0, HAL_GPIO_O_DIR, 63},		/* LATCH PIN */
    {1, HAL_GPIO_O_DIR, 2},		/* CLOCK PIN */
    {1, HAL_GPIO_O_DIR, 21},		/* DATA PIN */
    {{0, HAL_GPIO_I_DIR, 32},		/* SCAN1 PIN */
     {0, HAL_GPIO_I_DIR, 35}},		/* SCAN2 PIN */
    {{0, HAL_GPIO_O_DIR, 12},		/* COM1 PIN */
     {0, HAL_GPIO_O_DIR, 13},		/* COM2 PIN */
     {0, HAL_GPIO_O_DIR, 14},		/* COM3 PIN */
     {0, HAL_GPIO_O_DIR, 15},		/* COM4 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM5 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM6 PIN */
     {0, HAL_GPIO_O_DIR, 63},		/* COM7 PIN */
     {0, HAL_GPIO_O_DIR, 63}},		/* COM8 PIN */
    {{1, HAL_GPIO_O_DIR, 7},		/* POWER PIN */
     {0, HAL_GPIO_O_DIR, 29},		/* LOCK PIN */
     {0, HAL_GPIO_O_DIR, 7},		/* Extend function LBD */
     {0, HAL_GPIO_O_DIR, 63}},		/* Extend function LBD */
    {0, HAL_GPIO_O_DIR, 63},		/* Reserve */
	300,  /* Intv repeat first */
	250,  /* Intv repeat */
	350,  /* Intv release */
	NULL,  /* hook_scan() callback */
	NULL,  /* hook_show() callback */
};

struct pan_configuration pan_config =
{
	 &pan_hw_info, bitmap_list_num, bitmap_list
};

/* VDAC */
UINT8 board_vdac_config[VDAC_NUM_MAX] = 
{VDAC_YUV_V, VDAC_YUV_U, VDAC_YUV_Y, VDAC_CVBS, VDAC_NULL, VDAC_NULL};


/********************************************************************************
 *																				*
 *							For Future extension									*
 *																				*
 ********************************************************************************/
#else
void smartcard_init(UINT32 param1, UINT32 param2)
{
}
void pin_mux_config(UINT32 param1, UINT32 param2)
{

}

struct pan_configuration pan_config =
{
	NULL, bitmap_list_num, bitmap_list
};

UINT8 board_vdac_config[VDAC_NUM_MAX] = 
{VDAC_NULL, VDAC_NULL, VDAC_NULL, VDAC_NULL, VDAC_NULL, VDAC_NULL};

#endif
/********************************************************************************
 *																				*
 *							other board related function						*
 *																				*
 ********************************************************************************/

void snd_config_set()
{
    struct snd_feature_config snd_config;
	MEMSET(&snd_config, 0, sizeof(struct snd_feature_config));
#if (SYS_MAIN_BOARD == BOARD_S3202_DEMO_01V03)
	snd_config.output_config.mute_num = 19;
#elif (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01\
	||SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01\
	||SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)
	snd_config.output_config.mute_num = 33;
#else
	snd_config.output_config.mute_num = 43;
#endif

#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01\
	||SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01\
	||SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)
	snd_config.output_config.mute_polar = 0;//1//xc.liu for new board
#else
	snd_config.output_config.mute_polar = 0;
#endif
	snd_config.output_config.dac_precision = 24;
	snd_config.output_config.dac_format = CODEC_LEFT;
	snd_config.output_config.is_ext_dac = 0;
#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01\
	||SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01\
	||SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)
	snd_config.output_config.gpio_mute_circuit = TRUE;
#else
	snd_config.output_config.gpio_mute_circuit = FALSE;
#endif
	snd_config.support_spdif_mute = 1;
	snd_config.output_config.enable_hw_accelerator = TRUE;
    snd_m33_attach(&snd_config);
}
void pan_attach()
{
    extern struct pan_configuration pan_config;
#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01\
	||SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01\
	||SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)
#ifndef FRONT_PANEL_SINO
	pan_tp6x0_attach(&pan_config);
#else
	*((volatile UINT32*)0xb8000040) |= (1<<26);	// gpio enable
	*((volatile UINT32*)0xb8000040) |= (1<<27);	// gpio enable
	//*((volatile UINT32*)0xb8000040) |= (1<<28);	// gpio enable
	pan_sino_attach(&pan_config);
	
#endif
#else
	pan_hwscan_l_attach(&pan_config);
#endif
}
void board_i2c_set()
{
#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01\
	||SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01\
	||SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)
	i2c_scb_attach(2);
#else
	i2c_scb_attach(1);
#endif
	i2c_gpio_attach(1);
	i2c_mode_set(I2C_TYPE_SCB, 40000, 1);
#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01\
	||SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01\
	||SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)
	i2c_mode_set(I2C_TYPE_SCB1, 40000, 1);
#endif	
	i2c_mode_set(I2C_TYPE_GPIO, 40000, 1);


#if(CAS_TYPE==CAS_IRDETO)
	i2c_mode_set(I2C_TYPE_GPIO, 40000, 1);
	i2c_gpio_set(0, 17, 18);
#endif

}

#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01\
	||SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01\
	||SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)

INT32 nim_s3202_Tuner_Attatch(struct QAM_TUNER_CONFIG_API *ptrQAM_Tuner)
{
	UINT8 data = 0;
	INT32 ret = SUCCESS;
    INT32 ret_0x34 = ERR_FAILUE;
	INT32 ret_0xc0 = ERR_FAILUE;
	INT32 ret_0xc2 = ERR_FAILUE;
	INT32 ret_0xc4 = ERR_FAILUE;
	INT32 ret_0xc6 = ERR_FAILUE;

	ret_0xc0 = i2c_read(I2C_TYPE_SCB1, 0xC0, &data, 1);

	if(SUCCESS != ret_0xc0)
	{
		osal_task_sleep(10);
		
		ret_0xc2 = i2c_read(I2C_TYPE_SCB1, 0xC2, &data, 1);

		if(SUCCESS != ret_0xc2)
		{

			osal_task_sleep(10);
			
			ret_0xc4 = i2c_read(I2C_TYPE_SCB1, 0xC4, &data, 1);

			if(SUCCESS != ret_0xc4)
			{
				osal_task_sleep(10);
				
				ret_0xc6 = i2c_read(I2C_TYPE_SCB1, 0xC6, &data, 1);
                if(SUCCESS != ret_0xc6)
			    {
				    osal_task_sleep(10);
				
				    ret_0x34 = i2c_read(I2C_TYPE_SCB1, 0x34, &data, 1);
			    }

			}
		}
	
	}

#ifdef SSI_TS_OUT_IN_SET
		ptrQAM_Tuner->tuner_config_data.cTuner_Tsi_Setting=NIM_0_SSI_0;
#endif
    if(SUCCESS == ret_0x34)
    {
        ptrQAM_Tuner->tuner_config_data.RF_AGC_MAX = 0xBA;
		ptrQAM_Tuner->tuner_config_data.RF_AGC_MIN = 0x2A;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MAX = 0xFE;
		ptrQAM_Tuner->tuner_config_data.IF_AGC_MIN = 0x01;
		ptrQAM_Tuner->tuner_config_data.AGC_REF 			= 0x80;
	
		ptrQAM_Tuner->tuner_config_ext.cTuner_special_config = 0x01; // RF AGC is disabled
		ptrQAM_Tuner->tuner_config_ext.cChip				= Tuner_Chip_NXP;
		ptrQAM_Tuner->tuner_config_ext.cTuner_AGC_TOP	= 1;//7; /*1 for single AGC, 7 for dual AGC */
		ptrQAM_Tuner->tuner_config_ext.cTuner_Base_Addr	= 0x34;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Crystal		= 4;
		ptrQAM_Tuner->tuner_config_ext.cTuner_Ref_DivRatio	= 64; 
		ptrQAM_Tuner->tuner_config_ext.cTuner_Step_Freq	= 62.5;
		//(Ref_divRatio*Step_Freq)=(80*50)=(64*62.5)=(24*166.7)
		ptrQAM_Tuner->tuner_config_ext.wTuner_IF_Freq		= 5070; //4063; 
		//ptrQAM_Tuner->tuner_config_ext.cTuner_Charge_Pump= 0;  
		ptrQAM_Tuner->tuner_config_ext.i2c_type_id 			= I2C_TYPE_SCB1;//I2C_TYPE_SCB0;

		extern INT32 tun_rt820_init(UINT32 * ptrTun_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
		extern INT32 tun_rt820_control(UINT32 Tun_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);
		extern INT32 tun_rt820_status(UINT32 Tun_id, UINT8 *lock);

		ptrQAM_Tuner->nim_Tuner_Init = tun_rt820_init;
		ptrQAM_Tuner->nim_Tuner_Control = tun_rt820_control;
		ptrQAM_Tuner->nim_Tuner_Status = tun_rt820_status;
    }
    else if ((ret_0xc0 == SUCCESS)||(ret_0xc6 == SUCCESS))
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
#elif ((SYS_MAIN_BOARD == BOARD_S3202_DEMO_01V03) || \
	(SYS_MAIN_BOARD == BOARD_S3202_C3069_HSC2000LvA))
INT32 nim_s3202_Tuner_Attatch(struct QAM_TUNER_CONFIG_API *ptrQAM_Tuner)
{
	UINT8 data = 0;
	INT32 ret = SUCCESS;
	ret = i2c_read(I2C_TYPE_SCB, 0xC0, &data, 1);

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



