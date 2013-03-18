
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
#define SYS_FLASH_BASE_ADDR	0xafc00000
#define SYS_SDRAM_SIZE			32//8 //64//
#define SYS_FLASH_SIZE      		0x800000
#define SYS_MAIN_BOARD			BOARD_S3202C_DEMO_02V01//BOARD_S3202_DEMO_01V03//BOARD_S3202C_NEWLAND_02V01//

//#define USB_MP_SUPPORT
/***********************************************************************
System periphery related marco define
*************************************************************************/
#define SYS_GPIO_MODULE		M6303GPIO
#define SYS_I2C_MODULE			M6303I2C
#define SYS_I2C_SDA				4//42 
#define SYS_I2C_SCL				5//43
#define SYS_SCI_MODULE			UART16550

#define SYS_RFM_MODULE			SYS_DEFINE_NULL
#define SYS_RFM_BASE_ADDR		0xca

/*DCT70701 for BOARD_S3202C_NEWLAND_02V01*/
#define SYS_TUNER_MODULE 		DCT70701//DCT7044//

#define SYS_IRP_MOUDLE			ALI25C01
#define FE_DVBC_SUPPORT      
#define STO_PROTECT_BY_MUTEX		//protocting flashw wirte/erase by mutex        

/***********************************************************************
S.W module configration related marco define
*************************************************************************/
#define SYS_PROJECT					SYS_DEFINE_NULL
#define SYS_PROJECT_FE				PROJECT_FE_DVBC
#define SYS_MW_MODULE				SYS_DEFINE_NULL
#define SYS_CA_MODULE				SYS_DEFINE_NULL

/***********************************************************************
for stb_info_data structure
*************************************************************************/
// must be same between bootloader/upgloader/maincode!!
#define STB_MAX_PART_NUM 1
#define STB_PART_MAX_CHUNK_NUM 32
#define STB_MAX_CHUNK_NUM 32

/***********************************************************************
Chunk ID marco define
*************************************************************************/
#define STB_CHUNK_BOOTLOADER_ID 	0xE3000010
#define	LOGO_ID					0x02FD0100
#define	RADIO_LOGO_ID			LOGO_ID//0x02FD0200
#define	BOOT_LOGO_ID			LOGO_ID
#define	MENU_LOGO_ID			LOGO_ID//0x02FD0200//0x02FD0300
#define	MAINCODE_ID				0x01FE0100
#define	CADATA_ID				0x08F70100
#define	IRCA_PS_CHUNK_ID		0x08F70100

/***********************************************************************
Video related marco define
*************************************************************************/
#define VIDEO_SUPPORT_EXTRA_DVIEW_WINDOW
#define Enable_108MHz_Video_Sample_Rate
#define VDEC_AV_SYNC 
#define VIDEO_OUTPUT_BETTER_VISION

#define		VDAC_USE_CVBS_TYPE	CVBS_1
#define		VDAC_USE_SVIDEO_TYPE	SVIDEO_1
#define		VDAC_USE_YUV_TYPE	YUV_1

#if (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_01V01)
#define		CVBS_DAC  				DAC0
#define		SVIDEO_DAC_Y			DAC2
#define		SVIDEO_DAC_C			DAC3
#define		YUV_DAC_Y				DAC2
#define		YUV_DAC_U				DAC1
#define		YUV_DAC_V				DAC3
#elif (SYS_MAIN_BOARD == BOARD_S3202C_DEMO_02V01)
#define		CVBS_DAC  				DAC1
#define		SVIDEO_DAC_Y			DAC2
#define		SVIDEO_DAC_C			DAC3
#define		YUV_DAC_Y				DAC2
#define		YUV_DAC_U				DAC0
#define		YUV_DAC_V				DAC3
#elif (SYS_MAIN_BOARD == BOARD_S3202C_NEWLAND_02V01)
#define		CVBS_DAC  				DAC1
#define		SVIDEO_DAC_Y			DAC2
#define		SVIDEO_DAC_C			DAC3
#define		YUV_DAC_Y				DAC2
#define		YUV_DAC_U				DAC0
#define		YUV_DAC_V				DAC3
#elif (SYS_MAIN_BOARD == BOARD_S3202_DEMO_01V03)
#define		CVBS_DAC  				DAC3
#define		SVIDEO_DAC_Y			DAC0
#define		SVIDEO_DAC_C			DAC1
#define		YUV_DAC_Y				DAC2
#define		YUV_DAC_U				DAC1
#define		YUV_DAC_V				DAC0
#endif

/***********************************************************************
Audio related marco define
*************************************************************************/
#define AUDIO_SPECIAL_EFFECT	/* play mpeg audio file*/
#define MULTI_AUDIO_PID 
#define DVBT_BEE_TONE
#define CODEC_I2S						(0x0<<1)
#define CODEC_LEFT						(0x1<<1)
#define CODEC_RIGHT						(0x2<<1)
#define AC3DEC							0
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
/****************************************************************************
  macro for AD
*****************************************************************************/
#define AD_ENREACH 1
#define AD_MIS 2
//#define AD_TYPE AD_MIS
#if(AD_TYPE == AD_MIS) 
#define MIS_AD_NEW
#elif(AD_TYPE == AD_ENREACH)
#define ENRICH_AD
#endif
/****************************************************************************
  macro for MIS
*****************************************************************************/
//#define MIS_AD
#if(defined(MIS_AD) || defined(MIS_AD_NEW))
#define	OSD_VSRC_MEM_MAX_SIZE		(450*1024)//(280*1024)//(94*1024)	
#else
#define	OSD_VSRC_MEM_MAX_SIZE		(94*1024)	
#endif


/***********************************************************************
DB4 related marco define
*************************************************************************/
#define DB_CAS_SUPPORT
#define DB_RAM_BACKUP
#define DB_USE_UNICODE_STRING 
#define DB_VERSION					40
#define MAX_PROG_NUM				500
#define MAX_TP_NUM					100
#define MAX_SAT_NUM					1

/***********************************************************************
EPG related marco define
*************************************************************************/
#define SYS_EPG_MODULE SYS_DEFINE_NULL
#define EPG_MULTI_TP
//#define EPG_OTH_SCH_SUPPORT

/***********************************************************************
NVOD related marco define
*************************************************************************/
#define NVOD_FEATURE

/***********************************************************************
ts monitor marco define
*************************************************************************/
//#define TS_MONITOR_FEATURE

/***********************************************************************
no channel auto search 
*************************************************************************/
//#define NOCHANNEL_AUTOSEARCH
/***********************************************************************
  Macro for win_reg.c
************************************************************************/
//#define REG_ENABLE


/***********************************************************************
  Macro for channel change time print
************************************************************************/
//#define CHCHG_TICK_PRINTF
/****************************************************************************
  macro for AV SYNC MODE
*****************************************************************************/
//#define AV_SYNC_TEST

/***********************************************************************
  Macro for 16bit OSD
************************************************************************/
#define  USE_LIB_GE

#define USE_16BIT_OSD

#ifdef USE_16BIT_OSD
#ifndef USE_LIB_GE
  #error"if need show 16bit osd ,please use lib GE!!"
#endif
#endif
/***********************************************************************
  Macro for win_nimreg.c
************************************************************************/
//#define NIM_REG_ENABLE
//#define SFU_AUTO_TEST     //打开该宏请取消LIB_BOOTUPG3的勾选

#ifndef	SFU_AUTO_TEST
#define BURN_SERIAL_NUM             //需要勾选LIB_BOOTUPG3
#else
#undef  BURN_SERIAL_NUM
#endif
/****************************************************************************
show 32bit or 16 bit pic when showing logo, hange pic format from yuv to rgb
*****************************************************************************/
//#define SHOW_LOGO_ON_GE
//#define OUT_32BITS_OSD  //choose 32bit or 16 bit

/***********************************************************************
 Macro for NETWORK
************************************************************************/
//#define NETWORK_SUPPORT


/***********************************************************************
Macro for SSI TS IN/OUT
***********************************************************************/
//#define SSI_TS_OUT_IN_SET


/***********************************************************************
 Macro for AC3 support
************************************************************************/
#define AC3_DECODE_SUPPORT


/***********************************************************************
  show multi dynamic Gifs,
  could not show multi dynamic gifs in Jade_demo_2+8 
************************************************************************/
//#define SHOW_MULTI_GIF  1

/***********************************************************************
  Macro for Pic AD,ex.bmp,jpg,static gif and png. 
  The macro is added in mode of 8+2.
  If use the macro, please disable the macro for multi dynamic gifs
************************************************************************/
#define SHOW_SINGLE_PIC 2

/***********************************************************************
  The macro for showing pictures
************************************************************************/
//#define ALI_DEMO_AD

//#define AD_SHARE_UI_OSD_LAYER
#if(defined(ALI_DEMO_AD))
    #define GIF_MAX_W               360
    #define GIF_MAX_H               240
    #define GIF_MAX_NUM             (1)
	#define __MM_GIF_BIT_LEN		(0x8000*GIF_MAX_NUM)
	#define __MM_GIF_DEC_LEN		((GIF_MAX_W*GIF_MAX_H*8+0x5000)*GIF_MAX_NUM)	//h*w*4+0x5000 for static, h*w*8+0x5000 for animation
	#define __MM_IMAGEDEC_MEM_LEN	(0x100000)//+0xca800) //0x40000->decoder buffer,0xca800->display buffer:720*576*2
#else

#define SHOW_PICS  SHOW_SINGLE_PIC //SHOW_MULTI_GIF

#ifdef SHOW_PICS
#if(SHOW_PICS == SHOW_SINGLE_PIC)
#define	AD_PNG_ID				0x0AF50100 //0x02FD0200
#define	AD_GIF_ID	            0x0AF50200 //0x02FD0300
#define	AD_JPG_ID	            0x0AF50300 //0x02FD0400
#define	AD_BMP_ID	            0x0AF50400 //0x02FD0500
#endif
#endif

#if(SHOW_MULTI_GIF == 1)
	#define __MM_GIF_BIT_LEN		(0x8000*3)			//fix size ,for 3 gifs
	#define __MM_GIF_DEC_LEN		(0xEF600*3)	//h*w*4+0x5000 for static, h*w*8+0x5000 for animation 
#else
	#define __MM_GIF_BIT_LEN		0	
	#define __MM_GIF_DEC_LEN		0
#endif

#if(SHOW_SINGLE_PIC == 2)
	#define __MM_IMAGEDEC_MEM_LEN	0x90000 //0x8E800//image size must<=360*240
#else
	#define __MM_IMAGEDEC_MEM_LEN	0
#endif

#endif

/***********************************************************************
  The macro for sanzhouxunchi AD
************************************************************************/
#ifdef SHOW_PICS
#if(SHOW_PICS == SHOW_SINGLE_PIC)
//#define AD_SANZHOU		//三洲迅驰广告
#endif
#endif

/***********************************************************************
Character set related marco define
*************************************************************************/
#define GB2312_SUPPORT
#define ISO8859_SUPPORT 
#define ISO_5937_SUPPORT 

/***********************************************************************
OTA related marco define for data backup
*************************************************************************/
#define OTA_POWEROFF_SAFE
#define OTA_START_BACKUP_FLAG			0xFF
#define OTA_START_END_FLAG				0xFE
#define RESTORE_DATA_ADDR				0xAFD20000
#define BACKUP_START_FLAG				0x55aa5a5a

#define MAIN_FREQ_DEFAULT				29900
#define MAIN_SYM_DEFAULT				6875
#define MAIN_QAM_DEFAULT				6

/*ota buffer map(usb upgrad reused), should be modified according to sdram size and to be upgraded abs size*/
#define OTA_SWAP_BUF_LEN           (256*1024)
#define OTA_COMPRESSED_BUF_LEN     0x500000
#define OTA_UMCOMPRESSED_BUF_LEN   0xA00000
#define OTA_UNCOMPRESSED_BUF_ADDR   0XA1600000
#define OTA_COMPRESSED_BUF_ADDR    (OTA_UNCOMPRESSED_BUF_ADDR-OTA_COMPRESSED_BUF_LEN)

//#define USB_UPG_DVBC
/***********************************************************************
SI/PSI and program search related marco define
*************************************************************************/
#define LIB_TSI3_FULL
#define PSI_MONITOR_SUPPORT 
#define SEARCH_DEFAULT_FTAONLY			0
#define SYS_PSEARCH_SCAN_TP_ALL			SYS_FUNC_ON

//TDT
#define _SYNC_TIME_ENABLE_
/***********************************************************************
Miscellaneous marco define
*************************************************************************/
#define EASY_DM_SWITCH
#define SELECT_SAT_ONLY
#define ENABLE_FEED_POWER

#define NEW_IR_DRIVER		// To support multi IRP

//#define REFACTOR_PRINTF// this macro for sprintf() bug
#define WANT_FLOATING_POINT_IN_PRINTF

/****************************************************************************
fast playing free program when startup
When using IRDETO CA, Subtitle or Teletext, Please disable the Marco
*****************************************************************************/
#if(!defined(MIS_AD) && !defined(MIS_AD_NEW))
#define FAST_FREE_PLAY
#endif


 // close the CHANNEL_CHANGE_VIDEO_FAST_SHOW MACRO because of new AV SYNC and PCR sync issue. close this macor if need to use new libcore ( 9#) 

/****************************************************************************
  macro for data broadcast
*****************************************************************************/
#define DATA_BROADCAST_IPANEL   1
#define DATA_BROADCAST_ENREACH  2
//#define DATA_BROADCAST  DATA_BROADCAST_IPANEL

#if (DATA_BROADCAST==DATA_BROADCAST_IPANEL)
#define IPANEL20    1
#define IPANEL28    2
#define IPANEL_VER  IPANEL20//IPANEL28//use ipanel 2.8 please close CA  AD..
#endif
//macro for some addional c functions, enrich need these functions, but ipanel2.1 don's need
//#define NEW_CLIB_FUNCTION

/***************
**@jingjin
**CT1642 front panel driver
***************/
#define FP_NONE 0
#define FP_CT1642 (FP_NONE + 3)
#define FRONT_PANEL_TYPE FP_CT1642

/****************************************************************************
  macro for CAS and CAS type
*****************************************************************************/
#define MULTI_CAS
#define CAS_TYPE	CAS_GY//CAS_CDCA //CAS_ABEL//CAS_DVN//for 2+8 mode, //CAS_DVT//
						   //CAS_IRDETO//CAS_DVT//CAS_TF//CAS_DVN//CAS_MG//CAS_CONAX

#ifdef MULTI_CAS
#if(CAS_TYPE==CAS_IRDETO)
//#define	IRD_OTA_SUPPORT
#define IRD_FLSH_SIZE	 8 //  4  //2 //

/*macro define for eeprom*/
//#define IRCA_PS_BY_E2P
#define IRCA_LOADER_NVRAM_SIZE 0x4000
#define SYS_EEPROM_BASE_ADDR	0xA0			/* EEPROM base address */
#define SYS_EEPROM_SIZE (8192)
#define I2C_FOR_EEPROM  I2C_TYPE_GPIO0
#define TWO_BYTE_WORD_ADDRESS
#elif(CAS_TYPE==CAS_CONAX)
#define CONAX_NEW_PUB
#define CONAX_AUDIO_LAN_USE
#elif(CAS_TYPE==CAS_ABEL)
#define CONAX_NEW_PUB
#define CC_POST_CALLBACK
#elif(CAS_TYPE==CAS_DVT)
#define	DVTCA_VERSION_NEW
#endif
#endif

/***********************************************************************
  show subtitle & teletext
************************************************************************/
//#define SUBTITLE_ON   1
//#define TTX_ON        1

#if(SUBTITLE_ON == 1)
#define __SUBTITLE_BUF_LEN    0X19000
#else
#define __SUBTITLE_BUF_LEN    0
#endif

#if ((SUBTITLE_ON == 1)||(TTX_ON == 1)) 
#define  TTX_COLOR_NUMBER   256
#endif

#if(TTX_ON==1)
#define  TTX_BY_OSD
#define  TTX_SUB_PAGE
#define  SUPPORT_PACKET_26
//#define  TTX_COLOR_NUMBER   256
//the extended supportd languages
#define  TTX_CYRILLIC_1_SUPPORT
#define  TTX_CYRILLIC_2_SUPPORT
#define  TTX_CYRILLIC_3_SUPPORT
#define  TTX_GREEK_SUPPORT
#define  TTX_ARABIC_SUPPORT
#define  TTX_HEBREW_SUPPORT
#ifdef SUPPORT_PACKET_26
#define  TTX_CYRILLIC_G2_SUPPORT
#define  TTX_GREEK_G2_SUPPORT
#define  TTX_ARABIC_G2_SUPPORT
#define  TTX_G3_SUPPORT
#endif
#endif
/***********************************************************************
Memory map for hardware buffer related marco define
*************************************************************************/
#if(SYS_SDRAM_SIZE==8)
#define __MM_BUF_TOP_ADDR		0XA0800000
#elif(SYS_SDRAM_SIZE==16)
#define __MM_BUF_TOP_ADDR		0XA1000000
#elif(SYS_SDRAM_SIZE==32)
#define __MM_BUF_TOP_ADDR		0XA2000000
#elif(SYS_SDRAM_SIZE==64)
#define __MM_BUF_TOP_ADDR		0XA4000000 
#else
#define __MM_BUF_TOP_ADDR		0XA1000000  //For databroadcast,mem should bigger than 16M. 
#endif

#define __MM_FB0_Y_LEN			0X65400
#define __MM_FB0_C_LEN			0X32C00
#define __MM_FB1_Y_LEN			0X65400
#define __MM_FB1_C_LEN			0X32C00
#define __MM_FB2_Y_LEN			0X65400
#define __MM_FB2_C_LEN			0X32C00
#define __MM_MAF_LEN			0XC00
#define	__MM_EXTRA_LEN			0 //0x25200// not use now
#define __MM_DVW_LEN			0//0X25FA0 //only used in multi-view, can be 0 if no multi-view
/*osd layer buf len should be modified according to osd region width,hight and bytes per pixel*/
#ifdef SHOW_LOGO_ON_GE
    #ifdef OUT_32BITS_OSD
	   #define __MM_OSD_LAYER0_LEN	(0XCA800*2)  //720*576*2*2,32bit
    #else
       #define __MM_OSD_LAYER0_LEN	0XCA800  //720*576*2,16bit
    #endif
#else
	#if (SYS_SDRAM_SIZE == 8)
		#define __MM_OSD_LAYER0_LEN	0X68100  //for 2+8 mode,  720*(28+28)*2+720*240*2,16bit
    #else
		#define __MM_OSD_LAYER0_LEN	0XCA800  //720*576*2,16bit
    #endif
#endif

#ifdef USE_16BIT_OSD
#define __MM_OSD_LAYER1_LEN	0XCA800 //720*576*2,16bit
#else
#define __MM_OSD_LAYER1_LEN	0X65400  //720*576,clut8
#endif
#define __MM_GE_CMD_LEN		0x1000
#define __MM_OSD_BLK_LEN		0x1200 // 0X480
#if (SYS_SDRAM_SIZE == 8)
	#define __MM_VBV_LEN	0X48000 //for 2+8 mode, if memory enough, 0x6D000 better
#else
	#define __MM_VBV_LEN	0X6D000
#endif
#if(SYS_SDRAM_SIZE>=16)
#define __MM_DMX_SI_LEN			(80*188)
#else
#define __MM_DMX_SI_LEN			(1*188) ////if memory enough, (20*188) better
#endif
#define __MM_DMX_SI_TOTAL_LEN	(__MM_DMX_SI_LEN*28)
#define __MM_SI_VBV_OFFSET		__MM_DMX_SI_TOTAL_LEN
#define __MM_DMX_DATA_LEN		(30*188)
#define __MM_DMX_PCR_LEN		(10*188)
#if(SYS_SDRAM_SIZE>=16)
#define __MM_DMX_AUDIO_LEN		(256*188)
#define __MM_DMX_VIDEO_LEN		(1024*188)
#else
#define __MM_DMX_AUDIO_LEN		(32*188)
#define __MM_DMX_VIDEO_LEN		(384*188) //if memory enough, (1024*188) better
#endif
#define __MM_DMX_AVP_LEN		(__MM_DMX_VIDEO_LEN+__MM_DMX_AUDIO_LEN+__MM_DMX_PCR_LEN)
#define __MM_DMX_REC_LEN		0
#ifdef USB_MP_SUPPORT
#define __MM_TTX_BS_LEN			(5*1024)
#define __MM_TTX_PB_LEN			0
#define __MM_SUB_BS_LEN		(10*1024)
#define __MM_SUB_PB_LEN		(64*1024)
#else
#define __MM_TTX_BS_LEN			0
#define __MM_TTX_PB_LEN			0
#define __MM_SUB_BS_LEN		0
#define __MM_SUB_PB_LEN		0
#endif
#if(TTX_ON==1)
#undef __MM_TTX_BS_LEN	
#undef __MM_TTX_PB_LEN	
#define __MM_TTX_BS_LEN			0XA000
#define __MM_TTX_PB_LEN			0XCA800
#endif

#ifdef TTX_SUB_PAGE
#define __MM_TTX_SUB_PAGE_BUF_LEN	0X19000   //The macro is used in Lib
#else
#define __MM_TTX_SUB_PAGE_BUF_LEN	0  //The macro is used in Lib
#endif

#ifdef SUPPORT_PACKET_26
#define __MM_TTX_P26_DATA_LEN       0x3E8	//The macro is used in Lib
#define __MM_TTX_P26_NATION_LEN		0x61A80	//The macro is used in Lib
#else
#define __MM_TTX_P26_DATA_LEN       0
#define __MM_TTX_P26_NATION_LEN     0
#endif

#ifdef NETWORK_SUPPORT
#define __MM_LWIP_MEMP_LEN		(1532*256)//Lwcp need this buffer.Set to 0 if lwcp don't need.
#else
#define __MM_LWIP_MEMP_LEN		0
#endif


#define __MM_FB0_Y_START_ADDR		((__MM_BUF_TOP_ADDR - __MM_FB0_Y_LEN)&0XFFFFFF00)
#define __MM_FB0_C_START_ADDR		((__MM_FB0_Y_START_ADDR - __MM_FB0_C_LEN)&0XFFFFFF00)
#define __MM_FB1_Y_START_ADDR		((__MM_FB0_C_START_ADDR - __MM_FB1_Y_LEN)&0XFFFFFF00)
#define __MM_FB1_C_START_ADDR		((__MM_FB1_Y_START_ADDR - __MM_FB1_C_LEN)&0XFFFFFF00)
#define __MM_FB2_Y_START_ADDR		((__MM_FB1_C_START_ADDR - __MM_FB2_Y_LEN)&0XFFFFFF00)
#define __MM_FB2_C_START_ADDR		((__MM_FB2_Y_START_ADDR - __MM_FB2_C_LEN)&0XFFFFFF00)
#define __MM_MAF_START_ADDR			((__MM_FB2_C_START_ADDR - __MM_MAF_LEN)&0XFFFFFC00)
#define __MM_DVW_START_ADDR			((__MM_MAF_START_ADDR - __MM_DVW_LEN)&0XFFFFFFF0)
#if(8==SYS_SDRAM_SIZE)
	#define __MM_VBV_START_ADDR			((__MM_DVW_START_ADDR - __MM_VBV_LEN)&0XFFFFFF00)
	#define __MM_OSD_START_ADDR			((__MM_VBV_START_ADDR - __MM_OSD_LAYER0_LEN -__MM_OSD_LAYER1_LEN - __MM_GE_CMD_LEN)&0XFFFFFFF0)
#else
	//#define __MM_VBV_START_ADDR			((__MM_DVW_START_ADDR - __MM_VBV_LEN)&0XFFFFFF00)
	#define __MM_OSD_START_ADDR			((__MM_DVW_START_ADDR - __MM_OSD_LAYER0_LEN -__MM_OSD_LAYER1_LEN - __MM_GE_CMD_LEN)&0XFFFFFFF0)
#endif
#define __MM_OSD_BLOCK_ADDR			((__MM_OSD_START_ADDR - __MM_OSD_BLK_LEN)&0XFFFFFFF0)
#define __MM_DMX_AVP_START_ADDR		((__MM_OSD_BLOCK_ADDR - __MM_SI_VBV_OFFSET - __MM_DMX_DATA_LEN - __MM_DMX_AVP_LEN)&0XFFFFFFFC)
#define __MM_DMX_REC_START_ADDR		((__MM_DMX_AVP_START_ADDR - __MM_DMX_REC_LEN)&0XFFFFFFFC)
#define __MM_TTX_BS_START_ADDR		((__MM_DMX_REC_START_ADDR - __MM_TTX_BS_LEN)&0XFFFFFFFC)
#define __MM_TTX_PB_START_ADDR		((__MM_TTX_BS_START_ADDR - __MM_TTX_PB_LEN)&0XFFFFFFFC)
#define __MM_TTX_SUB_PAGE_BUF_ADDR  ((__MM_TTX_PB_START_ADDR - __MM_TTX_SUB_PAGE_BUF_LEN)&0XFFFFFFFC)
#define __MM_TTX_P26_DATA_BUF_ADDR	((__MM_TTX_SUB_PAGE_BUF_ADDR - __MM_TTX_P26_DATA_LEN)&0XFFFFFFFC)
#define __MM_TTX_P26_NATION_BUF_ADDR	((__MM_TTX_P26_DATA_BUF_ADDR - __MM_TTX_P26_NATION_LEN)&0XFFFFFFFC)
#define __MM_SUB_BS_START_ADDR		__MM_DVW_START_ADDR
#define __MM_SUB_PB_START_ADDR		(__MM_SUB_BS_START_ADDR + __MM_SUB_BS_LEN)
#define __MM_LWIP_MEMP_ADDR         (__MM_TTX_PB_START_ADDR - __MM_LWIP_MEMP_LEN)
#ifdef USB_MP_SUPPORT
#define __MM_MP_BUFFER_LEN			0x300000
#define __MM_MP_BUFFER_ADDR		(__MM_LWIP_MEMP_ADDR - __MM_MP_BUFFER_LEN)
#define __MM_PVR_VOB_BUFFER_ADDR 	0
#define __MM_PVR_VOB_BUFFER_LEN 	0
#else
#define __MM_MP_BUFFER_LEN			0
#define __MM_MP_BUFFER_ADDR		__MM_LWIP_MEMP_ADDR
#endif

#define __SUBTITLE_BUF_ADDR   (__MM_MP_BUFFER_ADDR - __SUBTITLE_BUF_LEN ) 

#define __MM_GIF_BIT_ADDR		(__SUBTITLE_BUF_ADDR - __MM_GIF_BIT_LEN)
#define __MM_GIF_DEC_ADDR		(__MM_GIF_BIT_ADDR - __MM_GIF_DEC_LEN)


#define __MM_IMAGEDEC_BUF_ADDR	((__MM_GIF_DEC_ADDR -__MM_IMAGEDEC_MEM_LEN)&0x8FFFFFFF)

/*set heap top address under data broadcast buffer. 
avoid data broadcast crushing heap data*/
#ifndef DATA_BROADCAST
#define __MM_HEAP_TOP_ADDR			__MM_IMAGEDEC_BUF_ADDR
#elif(DATA_BROADCAST==DATA_BROADCAST_IPANEL)
#define IPANEL_MW_BUF_START     ((__MM_IMAGEDEC_BUF_ADDR - IPANEL_MW_BUF_SIZE)&0X8FFFFF00)
#if(defined(MIS_AD) || defined(MIS_AD_NEW))
#define IPANEL_MW_BUF_SIZE		0 //0xE00000//0X1000000
#else 
#if(IPANEL_VER==IPANEL28)
#define IPANEL_MW_BUF_SIZE		0xF33500//0X1000000//
#else
#define IPANEL_MW_BUF_SIZE		0xE00000//0X1000000//
#endif
#endif
#define DMX_PORTING_BUF_START   ((IPANEL_MW_BUF_START - DMX_PORTING_BUF_SIZE)&0X8FFFFF00)
#if(IPANEL_VER==IPANEL28)
#define DMX_PORTING_BUF_SIZE    0X180000
#else
#define DMX_PORTING_BUF_SIZE    0X180000
#endif

#define __MM_HEAP_TOP_ADDR			DMX_PORTING_BUF_START
#elif(DATA_BROADCAST==DATA_BROADCAST_ENREACH)
#define ENREACH_BROWSER_BUF_START     	((__MM_IMAGEDEC_BUF_ADDR - ENREACH_BROWSER_BUF_SIZE)&0X8FFFFF00)
#define ENREACH_BROWSER_BUF_SIZE		0x400000
#define ENREACH_CACHE_START  			((ENREACH_BROWSER_BUF_START - ENREACH_CACHE_SIZE)&0X8FFFFF00)
#define ENREACH_CACHE_SIZE      		0x100000
#define ENREACH_OCBUF_START   			((ENREACH_CACHE_START - ENREACH_OCBUF_SIZE)&0X8FFFFF00)
#define ENREACH_OCBUF_SIZE    			0x800000
#define ENREACH_DMXBUF_START    		((ENREACH_OCBUF_START - ENREACH_DMXBUF_SIZE)&0X8FFFFF00)
#define ENREACH_DMXBUF_SIZE     		0x300000

#define __MM_HEAP_TOP_ADDR			ENREACH_DMXBUF_START
#endif
#define NAND_DEBUG
//#define FRONT_PANEL_SINO

/*为保证入网认证AV同步测试，须定义此宏，并调整A/V PTS 的相对偏移量，以符合测试标准，量产软件不需定义此宏*/
//#define AV_SYNC_TEST
#endif	/* _COMMON_CONFIG_H_ */

