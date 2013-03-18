/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002, 2003 Copyright (C)
*
*    File:    sys_define.h
*
*    Description:    This file contains all system parameter definations.
*
*    Note:    This file should be existed under "inc" directory.
*
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Jan.18.2003       Justin Wu       Ver 0.1    Create file.
*	2.	Feb.10.2003		  Justin Wu       Ver 0.2    Update
*	3.  2003.2.13         Liu Lan         Ver 0.3    Split def & cfg
*****************************************************************************/

#ifndef _SYS_DEFINE_H_
#define _SYS_DEFINE_H_

/****************************************************************************
 * Hardware and software type coding, used for component ID.
 ****************************************************************************/
#define HW_TYPE_CHIP			0x00010000	/* Chip */
#define HW_TYPE_CHIP_REV		0x00020000	/* Chip Revised */
#define HW_TYPE_CPU				0x00030000	/* CPU */
#define HW_TYPE_ENDIAN			0x00040000	/* CPU endian */
#define HW_TYPE_BOARD			0x00050000	/* Main board type */

#define SW_TYPE_PROJECT			0x00100000	/* Project */
#define SW_TYPE_OS				0x00110000	/* OS */
#define SW_TYPE_MW				0x00120000	/* Medlleware */
#define SW_TYPE_CA				0x00130000	/* Conditional access */
#define SW_TYPE_EPG				0x00140000	/* EPG */
#define SW_TYPE_DB              0x00150000  /* Database */

#define LLD_DEV_TYPE_DEC		0x02010000	/* Decoder */
#define LLD_DEV_TYPE_DMX		0x02020000	/* DeMUX */
#define LLD_DEV_TYPE_OSD		0x02030000	/* OSD */
#define LLD_DEV_TYPE_NET		0x02040000	/* Network */
#define LLD_DEV_TYPE_NIM		0x02050000	/* NIM */
#define LLD_DEV_TYPE_DEM		0x02050100	/* Demodulator */
#define LLD_DEV_TYPE_TUN		0x02050200	/* Tuner */
#define LLD_DEV_TYPE_IRC		0x02060000	/* IR Controller */
#define LLD_DEV_TYPE_PAN		0x02070000	/* Panel */
#define LLD_DEV_TYPE_SMC		0x02080000	/* Smart-card */
#define LLD_DEV_TYPE_SND		0x02090000	/* Sound card */
#define LLD_DEV_TYPE_DSC		0x020A0000	/* Descrambler */
#define LLD_DEV_TYPE_USB		0x020B0000	/* USB device */
#define LLD_DEV_TYPE_STO		0x020C0000	/* Character storage device */
#define LLD_DEV_TYPE_RFM		0x020D0000	/* RF modulator device */
#define LLD_DEV_TYPE_MST		0x020E0000	/* Block storage device */
#define LLD_DEV_TYPE_DIS		0x020F0000	/* Display device */
#define LLD_DEV_TYPE_CIC		0x02100000	/* CI controler device */
#define LLD_DEV_TYPE_SCART      0x02110000	/* SCART */

#define BUS_DEV_TYPE_GPIO		0x03010000	/* GPIO port */
#define BUS_DEV_TYPE_SCI		0x03020000	/* SCI interface */
#define BUS_DEV_TYPE_I2C		0x03030000	/* I2C interface */
#define BUS_DEV_TYPE_ATA		0x03040000	/* ATA interface */
#define BUS_DEV_TYPE_PCI		0x03050000	/* PCI interface */
#define BUS_DEV_TYPE_OHCI		0x03060000	/* OHCI interface */
#define BUS_DEV_TYPE_1394		0x03070000	/* 1394 interface */
#define BUS_DEV_TYPE_ISO7816	0x03080000	/* ISO7816 interface */
#define BUS_DEV_TYPE_TSI		0x03090000	/* TS input interface */

#define ACC_DEV_TYPE_IRP		0x04010000	/* IR Pad */
#define ACC_DEV_TYPE_JSP		0x04020000	/* Joy-stick Pad */

#define SYS_DEFINE_NULL			0x00000000	/* NULL define */

#define SYS_FUNC_ON				0x00000001	/* Function on */
#define SYS_FUNC_OFF			0x00000000	/* Function disable */

/****************************************************************************
 * Section for SW configuration.
 ****************************************************************************/
/* Project */
#define PROJECT_C3044	(SW_TYPE_PROJECT + 1)
#define PROJECT_C3026	(SW_TYPE_PROJECT + 2)
#define PROJECT_C3063	(SW_TYPE_PROJECT + 3)
#define PROJECT_C3075	(SW_TYPE_PROJECT + 4)
#define PROJECT_C3021	(SW_TYPE_PROJECT + 5)
#define PROJECT_C3060	(SW_TYPE_PROJECT + 6)
#define PROJECT_C3009	(SW_TYPE_PROJECT + 7)
#define PROJECT_C3016	(SW_TYPE_PROJECT + 8)
#define PROJECT_C3008_312	(SW_TYPE_PROJECT + 9)
#define PROJECT_C3008_0299	(SW_TYPE_PROJECT + 10)
#define PROJECT_C30632	(SW_TYPE_PROJECT + 11)
#define PROJECT_C3065	(SW_TYPE_PROJECT + 12)
#define PROJECT_C3062	(SW_TYPE_PROJECT + 13)
#define PROJECT_C30652	(SW_TYPE_PROJECT + 14)
#define PROJECT_C3015	(SW_TYPE_PROJECT + 15)
#define PROJECT_C3020	(SW_TYPE_PROJECT + 16)
#define PROJECT_C3057	(SW_TYPE_PROJECT + 17)
#define PROJECT_C3025	(SW_TYPE_PROJECT + 18)
#define PROJECT_C3059	(SW_TYPE_PROJECT + 19)
#define PROJECT_HDTV			(SW_TYPE_PROJECT + 20)
#define PROJECT_DVBC_C30651	(SW_TYPE_PROJECT + 21)
#define PROJECT_C3033	(SW_TYPE_PROJECT + 22)
#define PROJECT_C3052	(SW_TYPE_PROJECT + 23)
#define PROJECT_C3042	(SW_TYPE_PROJECT + 24)

#define PROJECT_FE_DVBS			(SW_TYPE_PROJECT + 0x101)
#define PROJECT_FE_DVBC			(SW_TYPE_PROJECT + 0x102)
#define PROJECT_FE_DVBT			(SW_TYPE_PROJECT + 0x103)
#define PROJECT_FE_DVBH			(SW_TYPE_PROJECT + 0x104)
#define PROJECT_FE_DVBS2			(SW_TYPE_PROJECT + 0x105)
#define PROJECT_FE_ATSC			(SW_TYPE_PROJECT + 0x106)
#define PROJECT_FE_ISDBT			(SW_TYPE_PROJECT + 0x107)


#define PROJECT_SM_FTA			(0x00000001)
#define PROJECT_SM_CI			(0x00000002)
#define PROJECT_SM_CA			(0x00000004)
#define PROJECT_SM_PVR			(0x00000008)

#define PROJECT_UI_OLD			(SW_TYPE_PROJECT + 0x301)
#define PROJECT_UI_C3049			(SW_TYPE_PROJECT + 0x302)
#define PROJECT_UI_ALI			(SW_TYPE_PROJECT + 0x303)
#define PROJECT_UI_COM			(SW_TYPE_PROJECT + 0x304)

/* OS */
#define NO_OS					(SW_TYPE_OS + 0)
#define ALI_TDS2				(SW_TYPE_OS + 2)
#define LINUX_2_4_18			(SW_TYPE_OS + 3)

/* MW */
#define SJTU_MBT				(SW_TYPE_MW + 1)
#define IPANEL					(SW_TYPE_MW + 2)
#define MW_IPANEL				(SW_TYPE_MW + 3)
#define MW_ENREACH				(SW_TYPE_MW + 4)
#define MW_AVIT					(SW_TYPE_MW + 5)

#define DB_MANAGE_2LEVEL		(SW_TYPE_DB + 1)
#define DB_MANAGE_3LEVEL		(SW_TYPE_DB + 2)

/* CAS */
#define CAS_NONE				(SW_TYPE_CA)
#define CAS_TF					(SW_TYPE_CA + 1)
#define CAS_DVT					(SW_TYPE_CA + 2)
#define CAS_CTI					(SW_TYPE_CA + 3)
#define CAS_IRDETO				(SW_TYPE_CA + 4)
#define CAS_DVN					(SW_TYPE_CA + 5)
#define CAS_CONAX				(SW_TYPE_CA + 6)
#define CAS_MG					(SW_TYPE_CA + 7)
#define CAS_GOS					(SW_TYPE_CA + 8)
#define CAS_CDCA3				(SW_TYPE_CA + 9)
#define CAS_CDCA				(SW_TYPE_CA + 10)
#define CAS_GY				(SW_TYPE_CA + 11)

/* EPG */


/****************************************************************************
 * Section for HW configuration, include bus configuration.
 ****************************************************************************/
/* CHIP */
#define ALI_M3327				(HW_TYPE_CHIP + 4)
#define ALI_M3329E				(HW_TYPE_CHIP + 6)
#define ALI_M3327C				(HW_TYPE_CHIP + 7)
#define ALI_M3202				(HW_TYPE_CHIP + 10)

#define ALI_M3101				(HW_TYPE_CHIP + 30)

#define ALI_M3501				(HW_TYPE_CHIP + 50)

#define ALI_S3601				(HW_TYPE_CHIP + 60)

#define ALI_S3602				(HW_TYPE_CHIP + 70)

#define ALI_S3602F				(HW_TYPE_CHIP + 71)

/* CHIP Revised */
#define IC_REV_0				(HW_TYPE_CHIP_REV + 1)
#define IC_REV_1				(HW_TYPE_CHIP_REV + 2)
#define IC_REV_2				(HW_TYPE_CHIP_REV + 3)
#define IC_REV_3				(HW_TYPE_CHIP_REV + 4)
#define IC_REV_4				(HW_TYPE_CHIP_REV + 5)
#define IC_REV_5				(HW_TYPE_CHIP_REV + 6)
#define IC_REV_6				(HW_TYPE_CHIP_REV + 7)
#define IC_REV_7				(HW_TYPE_CHIP_REV + 8)
#define IC_REV_8				(HW_TYPE_CHIP_REV + 9)

/* CPU */
#define CPU_M6303				(HW_TYPE_CPU + 2)
#define CPU_MIPS24KE				(HW_TYPE_CPU + 3)

/* Endian */
#define ENDIAN_BIG				(HW_TYPE_ENDIAN + 1)
#define ENDIAN_LITTLE			(HW_TYPE_ENDIAN + 2)

/* Main board */
#define BOARD_M3327FPGA			(HW_TYPE_BOARD + 21)
#define BOARD_M3327SORT         (HW_TYPE_BOARD + 22)
#define BOARD_M3327DEMO         (HW_TYPE_BOARD + 23)
#define BOARD_M3327C30631_1	(HW_TYPE_BOARD + 24)
#define BOARD_M3327C30621_0	(HW_TYPE_BOARD + 25)
#define BOARD_M3327C30101_0	(HW_TYPE_BOARD + 26)
#define BOARD_M3327DEMO3        (HW_TYPE_BOARD + 27)
#define BOARD_M3327C30101_1	(HW_TYPE_BOARD + 28)
#define BOARD_M3327C30441_1	(HW_TYPE_BOARD + 29)
#define BOARD_M3327C30631_2	(HW_TYPE_BOARD + 30)
#define BOARD_M3327HDTV1        (HW_TYPE_BOARD + 31)
#define BOARD_M3327HDTV2        (HW_TYPE_BOARD + 32)
#define BOARD_M3327C30261_1       	(HW_TYPE_BOARD + 33)
#define BOARD_M3327HDTV3        (HW_TYPE_BOARD + 34)
#define BOARD_M3327DVR          (HW_TYPE_BOARD + 35)
#define BOARD_M3327C3059ME1       	(HW_TYPE_BOARD + 36)
#define BOARD_M3327C3059EP1       	(HW_TYPE_BOARD + 37)
#define BOARD_M3327C3020          	(HW_TYPE_BOARD + 38)
#define BOARD_M3327C3056           	(HW_TYPE_BOARD + 39)
#define BOARD_M3327C3025_01       	(HW_TYPE_BOARD + 40)
#define BOARD_M3327C3065_01        	(HW_TYPE_BOARD + 41)
#define BOARD_M3327C3053_00      	(HW_TYPE_BOARD + 42)
#define BOARD_M3330C30651_1	(HW_TYPE_BOARD + 43)
#define BOARD_M3327C3033_00 	(HW_TYPE_BOARD + 44)
#define BOARD_M3327C3018_00 	(HW_TYPE_BOARD + 45)
#define BOARD_M3327C30261_2	(HW_TYPE_BOARD + 46)
#define BOARD_M3329DB01V02      (HW_TYPE_BOARD + 47)
#define BOARD_M3329C_DEMO		(HW_TYPE_BOARD + 48)
#define BOARD_M3329C303401V01	(HW_TYPE_BOARD + 49)
#define BOARD_M3329C_DEMO_SF	(HW_TYPE_BOARD + 50)
#define BOARD_M3329_C300200	(HW_TYPE_BOARD + 51)
#define BOARD_M3329_DEMO_2CI	(HW_TYPE_BOARD + 52)
#define BOARD_M3329C_C3007_SF	(HW_TYPE_BOARD + 53)
#define BOARD_M3329_C3046600	(HW_TYPE_BOARD + 54)
#define BOARD_M3329DB01V06      (HW_TYPE_BOARD + 55)
#define BOARD_M3329C_C3059_A2	(HW_TYPE_BOARD + 56)
#define BOARD_M3327_C302000	(HW_TYPE_BOARD + 57)
#define BOARD_M3329C_C304200     	(HW_TYPE_BOARD + 58)
#define BOARD_M3329_DEMO_960		(HW_TYPE_BOARD + 59)

#define BOARD_M3327C_DEMO       (HW_TYPE_BOARD + 60)
#define BOARD_M3327C_C306300       	(HW_TYPE_BOARD + 65)
#define BOARD_M3327C_C306500       	(HW_TYPE_BOARD + 70)
#define BOARD_M3327C_C302600      	(HW_TYPE_BOARD + 75)

#define BOARD_DEMO_ZARLINK		(HW_TYPE_BOARD + 80)
#define BOARD_M3330_C3020v1	(HW_TYPE_BOARD + 81)
#define BOARD_M3330C_C305801	(HW_TYPE_BOARD + 82)
#define BOARD_M3330C_C302001	(HW_TYPE_BOARD + 83)
#define BOARD_M3330C_C305802	(HW_TYPE_BOARD + 84)
#define BOARD_M3330C_PNPEV		(HW_TYPE_BOARD + 85)
#define BOARD_ES_C3013           	(HW_TYPE_BOARD + 86)
#define BOARD_DEMO_353          (HW_TYPE_BOARD + 87)
#define BOARD_DEMO_M3101		(HW_TYPE_BOARD + 88)
#define BOARD_SORTING_M3101		(HW_TYPE_BOARD + 89)
#define BOARD_C3013_PNPEV	(HW_TYPE_BOARD + 90)


#define HW_ALiS00_ES_V5          (HW_TYPE_BOARD + 91)//Canal+
#define HW_ALiS00_ES_V1		(HW_TYPE_BOARD + 92)
#define HW_ALiS00_ES_V2		(HW_TYPE_BOARD + 93)//model 1
#define HW_ALiS00_ES_V3		(HW_TYPE_BOARD + 94)//model 2= model 1 + S-Video
#define HW_ALiS00_ES_V4		(HW_TYPE_BOARD + 95)//model 2= model 1 + S-Video
#define HW_ALI_DEMO_ZARLINK		(HW_TYPE_BOARD + 96)
#define HW_ALiS01_BOARD			(HW_TYPE_BOARD + 97)
#define HW_ALiG00_BOARD_v1			(HW_TYPE_BOARD + 98)
#define HW_ALiG00_BOARD_v2			(HW_TYPE_BOARD + 99)
#define HW_ALI_DEMO_AFA			(HW_TYPE_BOARD + 100)
#define HW_ALiM00_BOARD			(HW_TYPE_BOARD + 101)
#define HW_ALiI00_BOARD			(HW_TYPE_BOARD + 102)
#define HW_ALI_DVR			(HW_TYPE_BOARD + 103)
#define HW_ALiA00_BOARD_v1		(HW_TYPE_BOARD + 104) //060222 yuchun
#define HW_ALiP01_BOARD			(HW_TYPE_BOARD + 105)
#define HW_ALiH00_BOARD_v1		(HW_TYPE_BOARD + 106) //060302 hbchen
#define HW_ALiC00_BOARD			(HW_TYPE_BOARD + 107)
#define HW_ALiP00_BOARD			(HW_TYPE_BOARD + 108)	//060315 Angus
#define HW_ALiL00_BOARD			(HW_TYPE_BOARD + 109)	//060411 yuchun
#define HW_ALiD00_BOARD			(HW_TYPE_BOARD + 110)	//060419 Ming Yi
#define BOARD_M3329_P00			(HW_TYPE_BOARD + 111)	//060612 Tom Yang
#define HW_ALiA00_3101_BOARD	(HW_TYPE_BOARD + 112)	//060612 Tom Yang
#define BOARD_C3006_PNPES	(HW_TYPE_BOARD + 113)
#define HW_ALiG00_BOARD_M3101	(HW_TYPE_BOARD + 114)
#define BOARD_C3013_DV0618r0	(HW_TYPE_BOARD + 115)
#define BOARD_DEMO2_M3101		(HW_TYPE_BOARD + 116)
#define BOARD_M3330C_C302101      	(HW_TYPE_BOARD + 117)
#define BOARD_M3330C_C302102      	(HW_TYPE_BOARD + 118)
#define BOARD_M3329_SKYWORTH	(HW_TYPE_BOARD + 119)
#define BOARD_M3329_C3058	(HW_TYPE_BOARD + 119)
#define BOARD_M3330C_QIS01      (HW_TYPE_BOARD + 121)
#define BOARD_C3013_DV0626r0	(HW_TYPE_BOARD + 122)
#define BOARD_M3329C_C3013	(HW_TYPE_BOARD + 124)
#define BOARD_M3329C_ETEK		(HW_TYPE_BOARD + 124)
#define BOARD_M3330C_BDF02      (HW_TYPE_BOARD + 125)
#define BOARD_M3330_S2_B1SH     (HW_TYPE_BOARD + 126)
#define HW_ALiS00_M3101_ES_V1		(HW_TYPE_BOARD + 127)//M3101
#define BOARD_M3327CC3018_00	(HW_TYPE_BOARD + 128)
#define BOARD_M3329DB01V04 (HW_TYPE_BOARD + 129)
#define BOARD_M3329_C3074	(HW_TYPE_BOARD + 130)
#define BOARD_M3329C_C3033_SF	(HW_TYPE_BOARD + 131)

#define BOARD_M3329_C3059_DVR	(HW_TYPE_BOARD + 132)
#define BOARD_M3329_C3018_CICA	(HW_TYPE_BOARD + 133)
#define BOARD_M3329C_C3042_2CA	(HW_TYPE_BOARD + 134)
#define BOARD_M3329C_C3062	(HW_TYPE_BOARD + 135)

#define BOARD_S3202_DEMO_01V01		(HW_TYPE_BOARD + 5000) //DVBC board type
#define BOARD_S3202_C3069_HSC2000LvA	(HW_TYPE_BOARD + 5001) 
#define BOARD_S3202_C3058_C11	(HW_TYPE_BOARD + 5002) 
#define BOARD_S3202_DEMO_01V03		(HW_TYPE_BOARD + 5003) 
#define BOARD_S3202_C3050_CP8	(HW_TYPE_BOARD + 5004) 
#define BOARD_S3202_C3058_C12	(HW_TYPE_BOARD + 5005) 
#define BOARD_S3202_C3012_ECR5119	(HW_TYPE_BOARD + 5006) 
#define BOARD_S3202_C3021_DEBUG	(HW_TYPE_BOARD + 5007)
#define BOARD_S3202C_DEMO_01V01	(HW_TYPE_BOARD + 5008)
#define BOARD_S3202C_DEMO_02V01	(HW_TYPE_BOARD + 5009)
#define BOARD_S3202C_CS		(HW_TYPE_BOARD + 5010)
#define BOARD_S3202C_CSN3700		(HW_TYPE_BOARD + 5011)
#define BOARD_S3202C_NEWLAND_02V01		(HW_TYPE_BOARD + 5012)
#define BOARD_S3202C_SKY_5800_2A0C1A	(HW_TYPE_BOARD + 5013)
#define BOARD_S3202C_YINHE	(HW_TYPE_BOARD + 5014)

#define BOARD_S3601_DEMO		(HW_TYPE_BOARD + 10000)
#define BOARD_S3602_DEMO		(HW_TYPE_BOARD + 10001)
#define BOARD_S3602_SORT		(HW_TYPE_BOARD + 10002)
#define BOARD_ATSC_DEMO_00	(HW_TYPE_BOARD + 10003)
#define BOARD_DB_S3601C_01V01	(HW_TYPE_BOARD + 10004)
#define BOARD_CB_M3601C_F01V01 (HW_TYPE_BOARD + 10005)
#define BOARD_CB_M3601C_S01V01 (HW_TYPE_BOARD + 10006)
#define BOARD_DB_M3602_02V01	(HW_TYPE_BOARD + 10007)
#define BOARD_DB_S3601C_04V01	(HW_TYPE_BOARD + 10008)
#define BOARD_DB_M3602_04V01	(HW_TYPE_BOARD + 10009)
#define BOARD_A00_DB_M3601C_ATSC_PVR_01V01 (HW_TYPE_BOARD + 10010)
#define BOARD_P00_M3602_MBQ_157_A (HW_TYPE_BOARD + 10011)
#define BOARD_C00_M3602_YMB_A003_A (HW_TYPE_BOARD + 10012)
#define BOARD_DB_M3602_ATSC_02V01 (HW_TYPE_BOARD + 10013)
#define BOARD_DB_M3602_07V01 (HW_TYPE_BOARD + 10014)

#define BOARD_SB_S3602F_QFP_01V01   (HW_TYPE_BOARD + 10024)
#define BOARD_SB_S3602F_BGA_01V01   (HW_TYPE_BOARD + 10025)
#define BOARD_DB_M3603_01V01        (HW_TYPE_BOARD + 10026)
#define BOARD_DB_M3606_01V01        (HW_TYPE_BOARD + 10027)
#define BOARD_DB_M3601E_01V01       (HW_TYPE_BOARD + 10028)
#define BOARD_DB_M3601E_02V01       (HW_TYPE_BOARD + 10029)
#define BOARD_DB_M3603_02V01		(HW_TYPE_BOARD + 10030)
#define BOARD_M3383_UM00_01V01	(HW_TYPE_BOARD + 10031)
#define BOARD_M3329E_C3042	(HW_TYPE_BOARD + 20000)
#define BOARD_M3329E_DEMO01V01	(HW_TYPE_BOARD + 20001)
#define BOARD_M3329E_DEMO01V02	(HW_TYPE_BOARD + 20002)/*2CI & 2CA*/
#define BOARD_M3329E_DEMO01V03	(HW_TYPE_BOARD + 20003)/*v01 enhanced*/
#define BAORD_M3329E_C3020_00     	(HW_TYPE_BOARD + 20004)/*CI_USB_HDMI*/
#define BAORD_M3329E_C3067_00 	(HW_TYPE_BOARD + 20005)/*USB_CI_CA*/
#define BAORD_M3329E_C3028_00  	(HW_TYPE_BOARD + 20006)/*USB_2CA*/
#define BAORD_M3329E_C3018_00	(HW_TYPE_BOARD + 20007)/*USB_HDMI_CI_2CA*/
#define BOARD_M3329E_C3013	(HW_TYPE_BOARD + 20008)
#define BOARD_M3329E_C3034_00	(HW_TYPE_BOARD + 20009)/*USB_HDMI_NFLASH_CI_CA*/
#define BOARD_M3329E_C3052_Q	(HW_TYPE_BOARD + 20010)/*Q model:USB_HDMI_CI*/
#define BOARD_M3329E4_DEMO01V01	(HW_TYPE_BOARD + 20011)/*2T + 1CI + 1CA + HDMI */
#define BOARD_M3327E4_DEMO01V01 (HW_TYPE_BOARD + 20012)/*2CA+SDIO/1CA+HDMI*/
#define BOARD_M3329E_DEMO01V05	(HW_TYPE_BOARD + 20013)/*32bit RAM*/
#define BOARD_M3329E_C3042_HDMI	(HW_TYPE_BOARD + 20014)/*USB_HDMI*/
#define BOARD_M3329E_C3052_K	(HW_TYPE_BOARD + 20015)/*K model:USB_HDMI_CI_IDE*/
#define BOARD_M3329E_C3013_2TUNER	(HW_TYPE_BOARD + 20016)/*2tuner,1ci*/
#define BOARD_DB_M3381C_01V01     (HW_TYPE_BOARD + 20017)
#define BOARD_M3329E_C3038_00	(HW_TYPE_BOARD + 20018)/*32M SDRAM+2M PFlash+1Scart+CA+W5100*/
#define BOARD_M3329E_C3017_00     (HW_TYPE_BOARD + 20019)/*32M SDRAM+2M SFlash+CAt+CI+HDMI*/
#define BOARD_M3602_J00_00     (HW_TYPE_BOARD + 20020)
#define BOARD_DB_M3381C_02V01     (HW_TYPE_BOARD + 20021)
#define BOARD_M3381C_J00V01     (HW_TYPE_BOARD + 20022)
#define BOARD_M3329D_DM01V01     (HW_TYPE_BOARD + 20023)
#define BOARD_M3381R_J00V01     (HW_TYPE_BOARD + 20024)
#define BOARD_DB_M3602_05V01	(HW_TYPE_BOARD + 20025)
#define BOARD_M3381C_M00V01     (HW_TYPE_BOARD + 20026)
#define BOARD_M3329D_SB01V01     (HW_TYPE_BOARD + 20027)
#define BOARD_M3329F_SB01V01     (HW_TYPE_BOARD + 20028)
#define BOARD_DB_M3381C_S_01V01 (HW_TYPE_BOARD + 20029)
#define BOARD_M3329E_C3038_01	(HW_TYPE_BOARD + 20030)
#define BOARD_DB_M3381L_01V01 (HW_TYPE_BOARD + 20031)
#define BOARD_M3602_J01V01	(HW_TYPE_BOARD + 20032)
#define BOARD_DB_M3602_08V01	(HW_TYPE_BOARD + 20033)
#define BOARD_M3602_B00_00	(HW_TYPE_BOARD + 20034)
#define BOARD_M3381R_B00_00	(HW_TYPE_BOARD + 20035)
#define BOARD_M3381R_S05_00	(HW_TYPE_BOARD + 20036)
#define BOARD_M3381R_S05_01	(HW_TYPE_BOARD + 20037)
#define BOARD_DB_M3105_01V01  	(HW_TYPE_BOARD + 20038)
#define BOARD_M3381R_S05_02	(HW_TYPE_BOARD + 20039)
#define BOARD_M3381R_N00_CP8	(HW_TYPE_BOARD + 20040)
#define BOARD_DB_M3701C_01V01		(HW_TYPE_BOARD + 20041)
#define BOARD_DB_M3383_01V01		(HW_TYPE_BOARD + 20042)
#define BOARD_M3383_C3076_00	(HW_TYPE_BOARD + 20043)
#define BOARD_M3701C_128M    (HW_TYPE_BOARD + 20044)
#define BOARD_M3603_C3076_I_00	(HW_TYPE_BOARD + 20045)
#define BOARD_M3603_C3076_C_00	(HW_TYPE_BOARD + 20046)
#define BOARD_M3606_C3077_S_00  (HW_TYPE_BOARD + 20047) 

/* GPIO */
#define M6303GPIO				(BUS_DEV_TYPE_GPIO + 2)
#define M3602F_GPIO				(BUS_DEV_TYPE_GPIO + 3)

/* I2C */
#define GPIO_I2C				(BUS_DEV_TYPE_I2C + 1)
#define M6303I2C				(BUS_DEV_TYPE_I2C + 2)

/* PCI */
#define M6303PCI				(BUS_DEV_TYPE_PCI + 1)

/* SCI */
#define GPIO_SCI				(BUS_DEV_TYPE_SCI + 1)
#define M6303GSI				(BUS_DEV_TYPE_SCI + 2)
#define UART16550				(BUS_DEV_TYPE_SCI + 4)

/* TSI */
#define M3327TSI				(BUS_DEV_TYPE_TSI + 1)
#define M3327CTSI				(BUS_DEV_TYPE_TSI + 2)


/****************************************************************************
 * Section for HLD configuration.
 ****************************************************************************/


/****************************************************************************
 * Section for LLD configuration.
 ****************************************************************************/
/* Storage */
#define	SST39016                (LLD_DEV_TYPE_STO + 1)
#define	SST39080                (LLD_DEV_TYPE_STO + 2)
#define	SST39040                (LLD_DEV_TYPE_STO + 3)
#define AM29160B				(LLD_DEV_TYPE_STO + 4)
#define AM29160T				(LLD_DEV_TYPE_STO + 5)
#define AM29800B				(LLD_DEV_TYPE_STO + 6)
#define AM29800T				(LLD_DEV_TYPE_STO + 7)
#define AM29400B				(LLD_DEV_TYPE_STO + 8)
#define AM29400T				(LLD_DEV_TYPE_STO + 9)
#define AM29016					(LLD_DEV_TYPE_STO + 10)
#define AM29080					(LLD_DEV_TYPE_STO + 11)
#define AM29040					(LLD_DEV_TYPE_STO + 12)
#define	AT24C16                 (LLD_DEV_TYPE_STO + 13)

/* NET */
#define RTL8100B				(LLD_DEV_TYPE_NET + 1)
#define NET_ENC28J60				(LLD_DEV_TYPE_NET + 2)
#define SMSC9220				(LLD_DEV_TYPE_NET + 3)
#define SMSC9500				(LLD_DEV_TYPE_NET + 4)
#define AX88796B				(LLD_DEV_TYPE_NET + 5)
#define NET_ALIETHMAC			(LLD_DEV_TYPE_NET + 6)
#define NET_USB_WIFI			(LLD_DEV_TYPE_NET + 7)   // WiFi Support

/* CIC */
#define M3327CIC				(LLD_DEV_TYPE_CIC + 1)
#define CIMAX_27				(LLD_DEV_TYPE_CIC + 2)

/* DMX */
#define M3323DMX				(LLD_DEV_TYPE_DMX + 1)
#define M6307DMX				(LLD_DEV_TYPE_DMX + 2)
#define M3327DMX				(LLD_DEV_TYPE_DMX + 3)
#define M3357DMX				(LLD_DEV_TYPE_DMX + 4)

/* Demodulator */
#define MT312					(LLD_DEV_TYPE_DEM + 1)
#define MT10312					(LLD_DEV_TYPE_DEM + 2)
#define ST0299					(LLD_DEV_TYPE_DEM + 3)
#define M3327QPSK				(LLD_DEV_TYPE_DEM + 4)
#define ST0297					(LLD_DEV_TYPE_DEM + 5)
#define MT352						(LLD_DEV_TYPE_DEM + 6)
#define ST0360					(LLD_DEV_TYPE_DEM + 7)
#define AF9003					(LLD_DEV_TYPE_DEM + 8)
#define MT353						(LLD_DEV_TYPE_DEM + 9)
#define ST0361					(LLD_DEV_TYPE_DEM + 10)
#define PN2020					(LLD_DEV_TYPE_DEM + 11)
#define COFDM_READ_EEPROM		(LLD_DEV_TYPE_DEM + 12)
#define DRX3975					(LLD_DEV_TYPE_DEM + 13)
#define COFDM_M3101 		(LLD_DEV_TYPE_DEM + 14)
#define QAM_S3201 			(LLD_DEV_TYPE_DEM + 15)

#define SH1409				(LLD_DEV_TYPE_DEM + 16) //ATSC
#define NEX2004				(LLD_DEV_TYPE_DEM + 17) //ATSC
#define SH1411				(LLD_DEV_TYPE_DEM + 18) //ATSC
#define AU8524				 (LLD_DEV_TYPE_DEM + 19) //ATSC
#define MICRONAS_DRX3933J	(LLD_DEV_TYPE_DEM + 20) //ATSC
#define MTK_MT5112EE		(LLD_DEV_TYPE_DEM + 21) //ATSC

#define ALI_S3501				(LLD_DEV_TYPE_DEM + 22) //dvbs2
#define ST0362					(LLD_DEV_TYPE_DEM + 23)//dvbt
#define SUNPLUS210               (LLD_DEV_TYPE_DEM + 24) //dvbt
#define CX24116				(LLD_DEV_TYPE_DEM + 25) //dvbs2
#define  SI2165				(LLD_DEV_TYPE_DEM + 26) //dvbt
#define  DIB7070				(LLD_DEV_TYPE_DEM + 27) //dvbt
#define   TC90517			(LLD_DEV_TYPE_DEM + 28) //ISDBT
#define   DIB8000			(LLD_DEV_TYPE_DEM + 29) //ISDBT
#define   MXL101			(LLD_DEV_TYPE_DEM + 30) //DVBT
#define TC90512			(LLD_DEV_TYPE_DEM + 31) //ISDBT
#define COFDM_M3100             (LLD_DEV_TYPE_DEM + 32)//DVBT
#define  FUJIA21                      (LLD_DEV_TYPE_DEM + 33)//ISDBT
#define   TC90527			(LLD_DEV_TYPE_DEM + 34) //ISDBT

/* Tuner */
#define ANY_TUNER               (LLD_DEV_TYPE_TUN + 0)
#define SL1925					(LLD_DEV_TYPE_TUN + 1)
#define SL1935					(LLD_DEV_TYPE_TUN + 2)
#define MAX2118					(LLD_DEV_TYPE_TUN + 3)
#define IX2360					(LLD_DEV_TYPE_TUN + 4)
#define IX2410					(LLD_DEV_TYPE_TUN + 5)
#define TUA6120					(LLD_DEV_TYPE_TUN + 6)
#define ZL10036					(LLD_DEV_TYPE_TUN + 7)
#define SL1935D					(LLD_DEV_TYPE_TUN + 8)
#define ZL10039         (LLD_DEV_TYPE_TUN + 9)
#define STB6000         (LLD_DEV_TYPE_TUN + 10)
#define IX2476          (LLD_DEV_TYPE_TUN + 11)
#define ED5056					(LLD_DEV_TYPE_TUN + 12)  // for DVBT
#define TD1300AL				(LLD_DEV_TYPE_TUN + 13)  // for DVBT
#define G151D						(LLD_DEV_TYPE_TUN + 14)  // for DVBT
#define H10D8           (LLD_DEV_TYPE_TUN + 15)  // for DVBT
#define DTF8570					(LLD_DEV_TYPE_TUN + 16)  // for DVBT
#define C9251D				  (LLD_DEV_TYPE_TUN + 17)  // for DVBT
#define DCQ_1D					(LLD_DEV_TYPE_TUN + 18)
#define MT2050					(LLD_DEV_TYPE_TUN + 19)
#define ED6055					(LLD_DEV_TYPE_TUN + 20)
#define SAMSUNG886			(LLD_DEV_TYPE_TUN + 21)
#define TD1311ALF				(LLD_DEV_TYPE_TUN + 22)
#define ED6265					(LLD_DEV_TYPE_TUN + 23)
#define ALP504A					(LLD_DEV_TYPE_TUN + 24)
#define RF4000					(LLD_DEV_TYPE_TUN + 25)
#define DTT7596					(LLD_DEV_TYPE_TUN + 26)
#define QT1010	        (LLD_DEV_TYPE_TUN + 27) //for DVBT
#define DTT73000				(LLD_DEV_TYPE_TUN + 28)
#define DTT75300				(LLD_DEV_TYPE_TUN + 29)
#define DCT2A 					(LLD_DEV_TYPE_TUN + 30)
#define MT2060					(LLD_DEV_TYPE_TUN + 31)

#define TSA5059					(LLD_DEV_TYPE_TUN + 32)
#define SP5769					(LLD_DEV_TYPE_TUN + 33)
#define G101D						(LLD_DEV_TYPE_TUN + 34)  // for DVBT

#define TD1311ALF_G				(LLD_DEV_TYPE_TUN + 35)
#define TD1611ALF				(LLD_DEV_TYPE_TUN + 36)
#define DTT75101				(LLD_DEV_TYPE_TUN + 37)
#define ED5065					(LLD_DEV_TYPE_TUN + 38)
#define GX1001					(LLD_DEV_TYPE_TUN + 39) //for DVB-C GuoXin 1001 Tuner

#define DTT4A111                (LLD_DEV_TYPE_TUN + 40)
#define DTT73001                (LLD_DEV_TYPE_TUN + 41)
#define EDT1022B                (LLD_DEV_TYPE_TUN + 42)
#define RADIO2004               (LLD_DEV_TYPE_TUN + 43)
#define UM_ZL               	(LLD_DEV_TYPE_TUN + 44)

#define MAX3580                 (LLD_DEV_TYPE_TUN + 45)
#define QT3010                  (LLD_DEV_TYPE_TUN + 46)
#define SH201A					(LLD_DEV_TYPE_TUN + 47) //ATSC
#define TD1336					(LLD_DEV_TYPE_TUN + 48) //ATSC
#define ALPS510					(LLD_DEV_TYPE_TUN + 49) //ATSC
#define DTT76806				(LLD_DEV_TYPE_TUN + 50) //ATSC
#define DTT76801				(LLD_DEV_TYPE_TUN + 51) //ATSC
#define UBA00AP 				(LLD_DEV_TYPE_TUN + 52) //ATSC SYNYO
#define HZ6306                           	(LLD_DEV_TYPE_TUN + 53)
#define MT2131                          	(LLD_DEV_TYPE_TUN + 54)
#define DCT70701				(LLD_DEV_TYPE_TUN + 55)//DVBC
#define DCT7044					(LLD_DEV_TYPE_TUN + 56)//DVBC
#define ALPSTDQE				(LLD_DEV_TYPE_TUN + 57)//DVBC
#define SAMSUNG_DPH261D		                (LLD_DEV_TYPE_TUN + 58)//atsc
#define MXL5005                                 (LLD_DEV_TYPE_TUN + 59) //ATSC Maxlinear
#define UBD00AL 			   (LLD_DEV_TYPE_TUN + 60) 
#define DTT76852 			   (LLD_DEV_TYPE_TUN + 61) 
#define ALPSTDAE				(LLD_DEV_TYPE_TUN + 62) //DVBC
#define DTT75411				(LLD_DEV_TYPE_TUN + 63)//DVBT
#define MT2063					(LLD_DEV_TYPE_TUN + 64)//DVBT
#define TDA18211				(LLD_DEV_TYPE_TUN + 65)//DVBT
#define TDCCG0X1F				(LLD_DEV_TYPE_TUN + 66)//DVBC
#define DNOD44QZV102A		                (LLD_DEV_TYPE_TUN + 67)//DVBT
#define DNOD404PH105A		                (LLD_DEV_TYPE_TUN + 68)//DVBT
#define DTT76809                               (LLD_DEV_TYPE_TUN + 69)//ATSC
#define MXL5007                                (LLD_DEV_TYPE_TUN + 70)//DVBT
#define DBCTE702F1				(LLD_DEV_TYPE_TUN + 71)//DVBC
#define STV6110				(LLD_DEV_TYPE_TUN + 72)//DVBS2 half-nim
#define CD1616LF				(LLD_DEV_TYPE_TUN + 73)//DVBC
#define EN4020					(LLD_DEV_TYPE_TUN + 74)//DVBT
#define TDA18218				(LLD_DEV_TYPE_TUN + 75)//DVBT
#define DIB0070					(LLD_DEV_TYPE_TUN + 76)//DVBT
#define RADIO3432               (		LLD_DEV_TYPE_TUN + 77)//DVBT
#define TDA18212			(LLD_DEV_TYPE_TUN + 78)
#define FC0012			(LLD_DEV_TYPE_TUN + 79)
#define DIB0090			(LLD_DEV_TYPE_TUN + 80) //ISDBT
#define  BF6009			(LLD_DEV_TYPE_TUN + 81) //ISDBT
#define ALPSTDAC		(LLD_DEV_TYPE_TUN + 82) //DVBC/DTMB
#define ZL10037		(LLD_DEV_TYPE_TUN + 83) //DVB-S2
#define ED6092B					(LLD_DEV_TYPE_TUN + 84)//DVBT
#define	NT220x		(LLD_DEV_TYPE_TUN + 85) //DVB-C
#define	MXL136		(LLD_DEV_TYPE_TUN + 86) //ISDBT
#define TDA18219				(LLD_DEV_TYPE_TUN + 87)//DVBT
#define	MXL241		(LLD_DEV_TYPE_TUN + 88) //DVB-C SOC
#define FC2226		(LLD_DEV_TYPE_TUN + 89) //DVB-C
#define TDA18250		(LLD_DEV_TYPE_TUN + 89) //DVB-C
#define RT820C		(LLD_DEV_TYPE_TUN + 90) //DVB-C

/* RF modulator */
#define TA1243					(LLD_DEV_TYPE_RFM + 1)
#define MCBS373					(LLD_DEV_TYPE_RFM + 2)
#define	RMUP74055				(LLD_DEV_TYPE_RFM + 3)
#define SHARP5056				(LLD_DEV_TYPE_RFM + 4)
#define SM0268					(LLD_DEV_TYPE_RFM + 5)
#define MC44BS374T1             (LLD_DEV_TYPE_RFM + 6)
#define V8060                   (LLD_DEV_TYPE_RFM + 7)

/* RF modulator */
#define AK4707                  (LLD_DEV_TYPE_SCART + 1)
#define MX9671                  (LLD_DEV_TYPE_SCART + 2)

/****************************************************************************
 * Section for ACC (accessory) configuration.
 ****************************************************************************/
/* IRP */
#define ALI25C00				(ACC_DEV_TYPE_IRP + 1)
#define MPNET_04				(ACC_DEV_TYPE_IRP + 2)
#define C304401	(ACC_DEV_TYPE_IRP + 3)
#define C304402	(ACC_DEV_TYPE_IRP + 4)
#define C302600	(ACC_DEV_TYPE_IRP + 5)
#define C307500	(ACC_DEV_TYPE_IRP + 6)
#define C306301	(ACC_DEV_TYPE_IRP + 7)
#define C302100	(ACC_DEV_TYPE_IRP + 8)
#define C300500	(ACC_DEV_TYPE_IRP + 9)
#define C300901	(ACC_DEV_TYPE_IRP + 10)
#define C300800	(ACC_DEV_TYPE_IRP + 11)
#define C304403	(ACC_DEV_TYPE_IRP + 12)
#define C306500	(ACC_DEV_TYPE_IRP + 13)
#define C306200	(ACC_DEV_TYPE_IRP + 14)
#define C301600	(ACC_DEV_TYPE_IRP + 15)
#define C301500	(ACC_DEV_TYPE_IRP + 16)
#define C304900	(ACC_DEV_TYPE_IRP + 17)
#define C306400	(ACC_DEV_TYPE_IRP + 18)
#define C301501	(ACC_DEV_TYPE_IRP + 19)
#define C3010	(ACC_DEV_TYPE_IRP + 20)
#define C3020_00	(ACC_DEV_TYPE_IRP + 21)
#define C303500	(ACC_DEV_TYPE_IRP + 22)
#define C3059_00	(ACC_DEV_TYPE_IRP + 23)
#define C306302	(ACC_DEV_TYPE_IRP + 24)
#define C301400	(ACC_DEV_TYPE_IRP + 25)
#define ROCK00					(ACC_DEV_TYPE_IRP + 26)
#define C3056				(ACC_DEV_TYPE_IRP + 27)
#define C301401	(ACC_DEV_TYPE_IRP + 28)
#define C306501	(ACC_DEV_TYPE_IRP + 29)
#define C302500	(ACC_DEV_TYPE_IRP + 30)
#define C306303	(ACC_DEV_TYPE_IRP + 31)
#define C301402	(ACC_DEV_TYPE_IRP + 32)
#define C3053_00	(ACC_DEV_TYPE_IRP + 33)
#define C3053_01	(ACC_DEV_TYPE_IRP + 34)
#define C3059_01	(ACC_DEV_TYPE_IRP + 35)
#define C3059_02	(ACC_DEV_TYPE_IRP + 36)
#define C3025_00	(ACC_DEV_TYPE_IRP + 37)
#define C3025_01	(ACC_DEV_TYPE_IRP + 38)
#define C3059_03	(ACC_DEV_TYPE_IRP + 39)
#define C303100	(ACC_DEV_TYPE_IRP + 40)
#define C306503	(ACC_DEV_TYPE_IRP + 41)
#define C3033_00	(ACC_DEV_TYPE_IRP + 42)
#define ALI25C01				(ACC_DEV_TYPE_IRP + 43)
#define C305801				(ACC_DEV_TYPE_IRP + 44)

#define C303000	(ACC_DEV_TYPE_IRP + 45)
#define C303200                 	(ACC_DEV_TYPE_IRP + 46)
#define C307201              	(ACC_DEV_TYPE_IRP + 47)
#define C301101               	(ACC_DEV_TYPE_IRP + 48)
#define C301201                	(ACC_DEV_TYPE_IRP + 49)
#define C302601	(ACC_DEV_TYPE_IRP + 50)
#define C302602	(ACC_DEV_TYPE_IRP + 51)
#define C3020_30C_00	(ACC_DEV_TYPE_IRP + 52)
#define C301301                  	(ACC_DEV_TYPE_IRP + 53)
#define C301302                  	(ACC_DEV_TYPE_IRP + 54)
#define C3059_012	(ACC_DEV_TYPE_IRP + 55)
#define C3059_013	(ACC_DEV_TYPE_IRP + 56)
#define C3059_006	(ACC_DEV_TYPE_IRP + 57)
#define C3059_008	(ACC_DEV_TYPE_IRP + 58)
#define C300701	(ACC_DEV_TYPE_IRP + 59)
#define C303401	(ACC_DEV_TYPE_IRP + 60)
#define C302300				(ACC_DEV_TYPE_IRP + 61)
#define C305400	(ACC_DEV_TYPE_IRP + 62)
#define ALiL00_00				(ACC_DEV_TYPE_IRP + 63)
#define C303101	(ACC_DEV_TYPE_IRP + 64)
#define C300200	(ACC_DEV_TYPE_IRP + 65)
#define ALI_00					(ACC_DEV_TYPE_IRP + 66)
#define ALiG00_01				(ACC_DEV_TYPE_IRP + 67)
#define ALiS01_008				(ACC_DEV_TYPE_IRP + 68)
#define ALiP00_00				(ACC_DEV_TYPE_IRP + 69)
#define ALiA00_00				(ACC_DEV_TYPE_IRP + 70)
#define C3006_01	(ACC_DEV_TYPE_IRP + 71)
#define ALiG00_04				(ACC_DEV_TYPE_IRP + 72)
#define C3006_02	(ACC_DEV_TYPE_IRP + 73)
#define ALiD00_00               (ACC_DEV_TYPE_IRP + 74)
#define C3046600                	(ACC_DEV_TYPE_IRP + 75)
#define C3021_01                  	(ACC_DEV_TYPE_IRP + 76)
#define QIS_01					(ACC_DEV_TYPE_IRP + 77)
#define C305401	(ACC_DEV_TYPE_IRP + 78)
#define C305402	(ACC_DEV_TYPE_IRP + 79)
#define IRP_SDT_TDT1000         (ACC_DEV_TYPE_IRP + 80)
#define BDF_01					(ACC_DEV_TYPE_IRP + 81)
#define C3042_00                 	(ACC_DEV_TYPE_IRP + 82)
#define C3047_00	(ACC_DEV_TYPE_IRP + 83)
#define C3069RC8A	(ACC_DEV_TYPE_IRP + 84)
#define ALI_01					(ACC_DEV_TYPE_IRP + 85)
#define C3069_ZJ	(ACC_DEV_TYPE_IRP + 86)
#define ARC_020					(ACC_DEV_TYPE_IRP + 87)
#define C3050_HB	(ACC_DEV_TYPE_IRP + 88)
#define C3050_COM	(ACC_DEV_TYPE_IRP + 89)
#define C3050_HEBEI	(ACC_DEV_TYPE_IRP + 90)
#define ALIG00_02				(ACC_DEV_TYPE_IRP + 91)
#define ALIJ00_00				(ACC_DEV_TYPE_IRP + 92)
#define C3050_BZ	  (ACC_DEV_TYPE_IRP + 93)
#define ALIM00_01	  (ACC_DEV_TYPE_IRP + 94)
#define ERLING00    (ACC_DEV_TYPE_IRP + 95)
#define ALIB00_00	 	(ACC_DEV_TYPE_IRP + 96)
#define ALIS05_00	 	(ACC_DEV_TYPE_IRP + 97)
#define C3021_00		(ACC_DEV_TYPE_IRP + 98)
#define C3021_NC		(ACC_DEV_TYPE_IRP + 99)
#define UM_RC8A				(ACC_DEV_TYPE_IRP + 100)
#define LIAOCHENG_IR				(ACC_DEV_TYPE_IRP + 101)
#define INSPUR_IR				(ACC_DEV_TYPE_IRP + 102)
#define NEWLAND_COM		(ACC_DEV_TYPE_IRP + 103)

/* Joy-Stick Pad */
#define JSP_ALI26C00			(ACC_DEV_TYPE_JSP + 1)
#define JSP_C306300			(ACC_DEV_TYPE_JSP + 2)
#define JSP_C304400	(ACC_DEV_TYPE_JSP + 3)
#define JSP_C301500	(ACC_DEV_TYPE_JSP + 4)

/* front panel */
#define C3063_PAN01			(LLD_DEV_TYPE_PAN + 1)
#define C3063_PAN02			(LLD_DEV_TYPE_PAN + 2)
#define PAN_PT6311			    (LLD_DEV_TYPE_PAN + 3)
#define PAN_PT6964			    (LLD_DEV_TYPE_PAN + 4)
#define PAN_CH455               (LLD_DEV_TYPE_PAN + 5)
#define PAN_GPIO                (LLD_DEV_TYPE_PAN + 6)
#define PAN_FD650               (LLD_DEV_TYPE_PAN + 7)
#define PAN_SM1668              (LLD_DEV_TYPE_PAN + 8)

/****************************************************************************
 * Section for compiler and linker configuration.
 ****************************************************************************/
/* Memory mapping option */
#define __ATTRIBUTE_RAM_		/* Code section running in RAM */
#define __ATTRIBUTE_ROM_		/* Code section running in ROM */
#define __ATTRIBUTE_REUSE_		/* Code section overlay in RAM */
#define __ATTRIBUTE_ICON16_		/* Icon section */
#define __ATTRIBUTE_FONTMAP_	/* Font section */


#define SYS_COFDM_ST0360_CHIP_ADRRESS		0x38
#define SYS_COFDM_ST0361_CHIP_ADRRESS		0x38
#define SYS_COFDM_ST0362_CHIP_ADRRESS             0x38
#define SYS_COFDM_MT352_CHIP_ADRRESS		0x1e//050714 yuchun for zarlink
#define SYS_COFDM_MT353_CHIP_ADRRESS		0x1e//051014 sam for zarlink 353
#define SYS_COFDM_AF9003_CHIP_ADRRESS		0x38
#define SYS_COFDM_PN2020_CHIP_ADRRESS       0x86
#define SYS_COFDM_DRX3975_CHIP_ADRRESS	0xe0
#define SYS_COFDM_SI2165_CHIP_ADRRESS      0xc8
//#define SYS_COFDM_M3101_CHIP_ADRRESS 0x40


//atsc
#define  SYS_COFDM_MT1409_CHIP_ADRRESS 0x32
#define  SYS_COFDM_MT1411_CHIP_ADRRESS 0x32
#define SYS_COFDM_au8524_CHIP_ADRRESS		0x8e
#define SYS_COFDM_DRX3933J_CHIP_ADRRESS	0x52
#define SYS_COFDM_MT5112EE_CHIP_ADRRESS  0x92 //HH

/****************************************************************************
 * chunk id
 ****************************************************************************/
#define SECONDLOASER_CHUNID_TYPE	0x00FF0000
#define MAINCODE_CHUNID_TYPE		0x01FE0000
#define LOGO_CHUNID_TYPE		0x02FD0000
#define DEFAULTDB_CHUNID_TYPE		0x03FC0000
#define USERDB_CHUNID_TYPE		0x04FB0000
#define HWCONFIG_CHUNID_TYPE		0x05FA0000

#define COUNTRYBAND_CHUNID_TYPE		0x06F90000
#define BEEP_CHUNID_TYPE		0x06F90000

#define EROM_CHUNID_TYPE		0x07F80000
#define CASKEY_CHUNID_TYPE		0x08F70000
#define HDCPKEY_CHUNID_TYPE		0x09F60000

#endif	/* _SYS_DEFINE_H_ */

