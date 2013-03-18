#ifndef _COMMON_CONFIG_H_
#define _COMMON_CONFIG_H_

/***********************************************************************
 OS object number maro define
*************************************************************************/
#define SYS_OS_MODULE			ALI_TDS2
#define SYS_OS_TASK_NUM			32
#define SYS_OS_SEMA_NUM			30
#define SYS_OS_FLAG_NUM			30
#define SYS_OS_MSBF_NUM			32

/***********************************************************************
System related marco define
*************************************************************************/
#define SYS_CHIP_MODULE			ALI_M3327C
#define SYS_CPU_MODULE			CPU_M6303
#define SYS_CPU_ENDIAN			ENDIAN_LITTLE
#define SYS_CHIP_CLOCK			27000000
#define SYS_CPU_CLOCK			216000000
#define SYS_FLASH_BASE_ADDR		0xafc00000
#define SYS_SDRAM_SIZE			32//8 //
#define SYS_FLASH_SIZE			0x800000		/* Flash size, in byte */
#define SYS_MAIN_BOARD			BOARD_S3202C_DEMO_02V01//BOARD_S3202_C3069_HSC2000LvA//BOARD_S3202_C3012_ECR5119//

/***********************************************************************
System periphery related marco define
*************************************************************************/
#define SYS_GPIO_MODULE			M6303GPIO
#define SYS_I2C_MODULE			M6303I2C
#define SYS_I2C_SDA				4//42 
#define SYS_I2C_SCL				5//43
#define SYS_SCI_MODULE			UART16550

#define SYS_RFM_MODULE			SYS_DEFINE_NULL//MCBS373
#define SYS_RFM_BASE_ADDR		0xca

#define SYS_TUNER_MODULE 		DCT7044//ALPSTDQE

#define SYS_IRP_MOUDLE			ALI25C00//C3069RC8A//
#define FE_DVBC_SUPPORT      
#define STO_PROTECT_BY_MUTEX		//protocting flashw wirte/erase by mutex        

/***********************************************************************
S.W module configration related marco define
*************************************************************************/
#define SYS_PROJECT					SYS_DEFINE_NULL
#define SYS_PROJECT_FE				PROJECT_FE_DVBC
#define SYS_MW_MODULE				SYS_DEFINE_NULL
#define SYS_CA_MODULE				SYS_DEFINE_NULL

/****************************************************************************
 * Section for HW configuration, include bus configuration.
 ****************************************************************************/

/* customer loader build related */
/************** common data define ****************/
#define STB_CHUNK_BOOTLOADER_ID 	0xE3000010
#define MAINCODE_ID 0x01FE0100
#define LOGO_ID		0x02FD0100
#define	RSC_FONT_ID			0x10EF0100
#define	RSC_BMP_ID			0x11EE0100

/*SSI TS IN/OUT */
//#define SSI_TS_OUT_IN_SET


/***********************************************************************
for stb_info_data structure
*************************************************************************/
// must be same between bootloader/upgloader/maincode!!
#define STB_MAX_PART_NUM 1
#define STB_PART_MAX_CHUNK_NUM 32
#define STB_MAX_CHUNK_NUM 32

#define PAN_KEY_INVALID	0xFFFFFFFF

#define PANEL_DISPLAY

/***********************************************************************
Video related marco define
*************************************************************************/
/* NOTE: open macro DECVIDEO_SUPPORT, will link vdec and avc module, size:40K*/
//#define DECVIDEO_SUPPORT
#define VIDEO_SUPPORT_EXTRA_DVIEW_WINDOW
#define Enable_108MHz_Video_Sample_Rate
#define VDEC_AV_SYNC 
#define VIDEO_OUTPUT_BETTER_VISION

#define		VDAC_USE_CVBS_TYPE	CVBS_1
#define		VDAC_USE_SVIDEO_TYPE	SVIDEO_1
#define		VDAC_USE_YUV_TYPE	YUV_1

#define		CVBS_DAC  				DAC0
#define		SVIDEO_DAC_Y			DAC2
#define		SVIDEO_DAC_C			DAC3
#define		YUV_DAC_Y				DAC2
#define		YUV_DAC_U				DAC1
#define		YUV_DAC_V				DAC3

/***********************************************************************
OSD related marco define
*************************************************************************/
#define COLOR_N							256
#define OSD_MAX_WIDTH					720
#define OSD_MAX_HEIGHT					576
#define OSD_STARTCOL					((720 - OSD_MAX_WIDTH)>>1)
#define OSD_STARTROW_N				((480 - OSD_MAX_HEIGHT)>>1)
#define OSD_STARTROW_P				((576 - OSD_MAX_HEIGHT)>>1)

#define	BIT_PER_PIXEL				8
#define OSD_TRANSPARENT_COLOR		0xFF
#define OSD_TRANSPARENT_COLOR_BYTE 	0xFF
#define	IF_GLOBAL_ALPHA				FALSE
#define	FACTOR						0					
#define	OSD_VSRC_MEM_MAX_SIZE		(94*1024)	

#define GPIO_NULL                   127

#define SYS_LNB_POWER_CUT           127
#define SYS_POWER_GPIO_NUM          (24) 
#define DEMO_RST_GPIO_NUM           42
#define DEMO_RST_GPIO_NUM2          41


#define DB_VERSION					40
#define MINI_DRIVER_SUPPORT

#define OTA_DEBUG_ON	1
#define OTA_DEBUG_OFF	0

#define OTA_DEBUG OTA_DEBUG_OFF

#if (OTA_DEBUG==OTA_DEBUG_ON)
//#define STO_CHUNK_PRINTF libc_printf	// chunk_sto.c
//#define CHUNK_PRINTF libc_printf	// chunk.c
#define UPG_PRINTF libc_printf		// ota app
#define UPG4_DEBUG  libc_printf		// lib_upg4_com.c upg_ota_api.c upg_usb_api.c
#define STB_DATA_PRINTF libc_printf	// stb_info_data.c
#define STBINFO_PRINTF libc_printf	// stb_info.c
#endif

#endif	/* _SYS_CONFIG_H_ */

