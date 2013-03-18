#ifndef __LD_BOOT_INFO_H__
#define __LD_BOOT_INFO_H__

#include <sys_config.h>
#include <api/libpub/lib_frontend.h>


#define BOOT_INFO_S			0
#define BOOT_INFO_C			1
#define BOOT_INFO_T			2

#define LD_BOOT_OTA_TYPE_AUTO   1
#define LD_BOOT_OTA_TYPE_MANUAL 2

#define LD_BOOT_OTA_CHK_SW_VER	0
#define LD_BOOT_OTA_UNCHK_SW_VER	1

/* Define for VDAC configuration */
#define VDAC_NUM_MAX		6
#define VDAC_TYPE_NUM		6
//Type
#define VDAC_TYPE_CVBS		0
#define VDAC_TYPE_SVIDEO	1
#define VDAC_TYPE_YUV		2
#define VDAC_TYPE_RGB		3
#define VDAC_TYPE_SCVBS	4
#define VDAC_TYPE_SSV		5
#define VDAC_TYPE_MAX		6
//Detail
#define VDAC_CVBS			(VDAC_TYPE_CVBS<<2|0)
#define VDAC_SVIDEO_Y		(VDAC_TYPE_SVIDEO<<2|0)
#define VDAC_SVIDEO_C		(VDAC_TYPE_SVIDEO<<2|1)
#define VDAC_YUV_Y			(VDAC_TYPE_YUV<<2|0)
#define VDAC_YUV_U			(VDAC_TYPE_YUV<<2|1)
#define VDAC_YUV_V			(VDAC_TYPE_YUV<<2|2)
#define VDAC_RGB_R			(VDAC_TYPE_RGB<<2|0)
#define VDAC_RGB_G			(VDAC_TYPE_RGB<<2|1)
#define VDAC_RGB_B			(VDAC_TYPE_RGB<<2|2)
#define VDAC_SCVBS			(VDAC_TYPE_SCVBS<<2|0)
#define VDAC_SSV_Y			(VDAC_TYPE_SSV<<2|0)
#define VDAC_SSV_C			(VDAC_TYPE_SSV<<2|1)
#define VDAC_NULL			0xFF
//Number of sub item
#define TYPE_CVBS_NUM		1
#define TYPE_SVIDEO_NUM	2
#define TYPE_YUV_NUM		3
#define TYPE_RGB_NUM		3
#define TYPE_SCVBS_NUM		1
#define TYPE_SSV_NUM		2

//dac config
#define CVBS  		0x00
#define YC_Y  		0x01
#define YC_C  		0x02
#define CAV_Y  		0x03
#define CAV_Pb  		0x04
#define CAV_Pr  		0x05
#define RGB_R  		0x06
#define RGB_G  		0x07
#define RGB_B  		0x08
#define RGB_SYNC  	0x09
#define SECAM_YC      0x0A
#define SECAM_Y		0x0B
#define SECAM_C        0x0C

#define DAC_DEFAULT  	0x0A

#define SYS_E2P_START_ADDR			0x400
#define LOADER_OPK_SIZE     		( 256 )
#define SYS_E2P_OPK_ADDR			SYS_E2P_START_ADDR
#define SYS_E2P_VER_ADDR			(SYS_E2P_START_ADDR+LOADER_OPK_SIZE)

#define LD_VALID_FLAG	0x1A2B3C4D
#define STB_SYS_OUI				0x10

#define LD_INVALID_LEN_TAG_FOR_WRITE 0x2B1A0000
#define LD_INVALIS_LEN_TAG_FOR_READ	0x00001A2B

#define	LDR_OTA_IMAGE_LOADER 0
#define	LDR_OTA_IMAGE_MAINPROG 1
#define	LDR_OTA_IMAGE_LOGO 2
#define	LDR_OTA_IMAGE_FONT 3

/*Define for OTA Freq Parameters*/
#define OTA_PID_DEFAULT					209
#define OTA_FREQ_DEFAULT				58600
#define OTA_SYM_DEFAULT				6875
#define OTA_QAM_DEFAULT				6

union BOOT_INFO
{
	struct
	{
		UINT8 type;	// 0:s_info; 1:c_info; 2:t_info
	}common;
	struct
	{
		// 0B
		UINT8 type;
		// 1B
		UINT8 	standby_flag	:1;/*it will saved in temp_info head position,then bootloader will judge the flag if need sleep*/
		UINT8	ota_flag			:1;
		UINT8	reserved6b		:6;
		// 2B
		UINT8	boot_tv_mode 		: 4;
		UINT8	boot_tv_ratio 		: 4;
		// 3B
		UINT8	boot_display_mode 	: 4;
		UINT8	boot_scart_out 		: 4;
		// 4B
		struct ft_antenna antenna_info;
		// 32B
		UINT32	ota_frequency;
		// 36B
		UINT32	ota_symbol;
		// 40B
		UINT16	ota_pid;
		// 42B
		UINT16	sw_version;
		// 44B
		UINT8	ota_modulation;
		// 45B
		UINT8	vdac_out[VDAC_NUM_MAX];
		// 51B
		UINT8	osd_lang;
		// 52B
		UINT8	reserved8[12];
	}s_info;
	struct
	{
		// 0B
		UINT8	type;
		// 1B
		UINT8 	standby_flag	:1;/*it will saved in temp_info head position,then bootloader will judge the flag if need sleep*/
		UINT8	reserved3b		:3;
		UINT8	ota_flag		:2;
		UINT8	ota_count		:2;
		// 2B
		UINT8	boot_tv_mode 		: 4;
		UINT8	boot_tv_ratio 		: 4;
		// 3B
		UINT8	boot_display_mode 	: 4;
		UINT8	boot_scart_out 		: 4;
		// 4B
		UINT32	ota_frequency;//xxx00
		// 8B
		UINT32	ota_symbol;
		// 12B
		UINT16	ota_pid;
		// 14B
		UINT8	ota_qam;
		// 15B - 20B
		UINT8	vdac_out[VDAC_NUM_MAX];
		// 21B
		UINT8 main_qam;
		// 22B - 25B
		UINT32 main_frequency;
		// 26B - 29B
		UINT32 main_symbol;
		// 30B
		UINT8	osd_lang;
		// 31B
		UINT8	rf_mode;
		// 32B
		UINT8	rf_channel;
		// 33B
		UINT8	boot_counter;
		// 34B
		UINT8	standby_ota_flag;
		// 35B
		UINT8	reserved8[29];
	}c_info;
	struct
	{
		// 0B
		UINT8 type;
		// 1B
		UINT8 	standby_flag	:1;/*it will saved in temp_info head position,then bootloader will judge the flag if need sleep*/
		UINT8	ota_flag			:1;
		UINT8	auto_ota_flag	:1;
		UINT8	reserved5b		:5;
		// 2B
		UINT8	boot_tv_mode 		: 4;
		UINT8	boot_tv_ratio 		: 4;
		// 3B
		UINT8	boot_display_mode 	: 4;
		UINT8	boot_scart_out 		: 4;
		// 4B
        	UINT32   band_width;
	 	// 8B
	        UINT8    modulation;
		// 9B
       	 UINT8    fft_mode;
		// 10B		
	        UINT8   guard_intl;
		// 11B
	        UINT8   fec;
		// 12B
	        UINT8   usage_type;
		// 13B
       	 UINT8   inverse;
		// 14B
        	UINT8    priority;
        	// 15B
		UINT8    reverse;
		// 16B
		UINT32	ota_frequency;
		// 20B
		UINT32	ota_symbol;
		// 24B
		UINT16	ota_pid;
		// 26B
		UINT16	sw_version;
		// 28B
		UINT8	ota_modulation;
		// 29B
		UINT8	vdac_out[VDAC_NUM_MAX];
		// 35B
		UINT8	rf_mode;
		// 36B
		UINT8	rf_channel;
		// 37B
		UINT8	boot_counter;
		// 38B
		UINT8	reserved8[26];
	}t_info;

};

typedef	union 
{
   UINT32	 Ver;
   struct 
   {
		UINT16  Model;
		UINT16	 MainVer;
   } unShort;
} HARDWARE_VERSION_STRUCT;

typedef	union
{
	UINT32  Ver;
	struct
	{
		UINT8	MajorVer;	
		UINT8	MinorVer;
		UINT8	LoaderVer;
		UINT8	PicVer;	
	} byte;
} SOFTWARE_VERSION_STRUCT;

typedef struct cpcb_info_t
{
	UINT32 ifValid;
	
	HARDWARE_VERSION_STRUCT   HwVer;
	SOFTWARE_VERSION_STRUCT   SwVer;

	UINT32		SN;//24b
	UINT8		ManufacturerCode;//7b

	UINT8		BackLoaderVer;

	UINT8		Reserved1;
	UINT8       Reserved2;
	UINT32		Reserved3;
} STB_INFO_STRUCT;

#endif /* __LD_BOOT_INFO_H__ */

