/*****************************************************************************
*    Copyright (C)2007 Ali Corporation. All Rights Reserved.
*
*    File:    nim_s3202.h
*
*    Description:    Header file in LLD.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.20070417       Penghui      Ver 0.1       Create file.
*	2.20080418		Joey		Ver 1.0		Fromal release for S3202B V1.0
*	4.20100822		Joey		Ver 1.14. 	Optimize CR parameter for easy lock.
*****************************************************************************/

#ifndef __LLD_NIM_S3202_H__
#define __LLD_NIM_S3202_H__

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim_tuner.h>

#define S3202_DEBUG_FLAG	0
#if(S3202_DEBUG_FLAG)
#define QAM_DEBUG			1
#define TUNER_DEBUG		1
#else
#define QAM_DEBUG			0
#define TUNER_DEBUG		0
#endif
#if(QAM_DEBUG)
	#define S3202_PRINTF		libc_printf
#else
	#define S3202_PRINTF(...)		do{}while(0)
#endif
#if(TUNER_DEBUG)
	#define TUNER_PRINTF		libc_printf
#else
	#define TUNER_PRINTF(...)		do{}while(0)
#endif

#define M3202_LOG_FUNC 0// add for printf berper and CR
#if(M3202_LOG_FUNC)
	#define M3202_PRINTF		libc_printf
#else
	#define M3202_PRINTF(...)		do{}while(0)
#endif




#define QAM_ONLY   0
#define QAM_SOC	1

#define NIM_S3202_GET_BYTE(i)             (*(volatile UINT8 *)(i))
#define NIM_S3202_SET_BYTE(i,d)          (*(volatile UINT8 *)(i)) = (d)

#define S3202_QAM_SOC_BASE_ADDR  	0xb8003000
#define S3202_QAM_ONLY_I2C_BASE_ADDR  	0x40 


/*Register Name*/

//common Register
#define NIM_S3202_CONTROL1							0x00
#define NIM_S3202_ADC_CONFIG						0x01
#define NIM_S3202_INTERRUPT_EVENTS				0x02
#define NIM_S3202_INTERRUPT_MASK					0x03
#define NIM_S3202_CONTROL2							0x04
#define NIM_S3202_I2C_CONTROL_GPIO				0x05

#define NIM_S3202_TIMEOUT_THRESHOLD				0x06
#define NIM_S3202_TS_OUT_FORMAT					0x07
#define NIM_S3202_PER_REGISTER1					0x08
#define NIM_S3202_PER_REGISTER2					0x09


//AGC Register
#define NIM_S3202_AGC1								0x0a
#define NIM_S3202_AGC2								0x0b
#define NIM_S3202_AGC3								0x0c
#define NIM_S3202_AGC4								0x0d
#define NIM_S3202_AGC5								0x0e
#define NIM_S3202_AGC6								0x0f
#define NIM_S3202_AGC7								0x10
#define NIM_S3202_AGC8								0x11
#define NIM_S3202_AGC9								0x12
#define NIM_S3202_AGC10							0x13
#define NIM_S3202_AGC11							0x14
#define NIM_S3202_AGC12							0x15
#define NIM_S3202_AGC13							0x16


//DCC Register
#define NIM_S3202_DCC								0x17

//Filter Bank
#define NIM_S3202_FILTER_BANK						0x18

//TR Loop register
#define NIM_S3202_TR_LOOP1							0x19
#define NIM_S3202_TR_LOOP2							0x1a
#define NIM_S3202_TR_LOOP3							0x1b
#define NIM_S3202_TR_LOOP4							0x1c
#define NIM_S3202_TR_LOOP5							0x1d
#define NIM_S3202_TR_LOOP6							0x1e
#define NIM_S3202_TR_LOOP7							0x1f
#define NIM_S3202_TR_LOOP8							0x20
#define NIM_S3202_TR_LOOP9							0x21
#define NIM_S3202_TR_LOOP10						0x22
#define NIM_S3202_TR_LOOP11						0x23
#define NIM_S3202_TR_LOOP12						0x24
#define NIM_S3202_TR_LOOP13						0x25
#define NIM_S3202_TR_LOOP14						0x26

//EQ Register
#define NIM_S3202_EQ1								0x28
#define NIM_S3202_EQ2								0x29
#define NIM_S3202_EQ3								0x2a
#define NIM_S3202_EQ4								0x2b
#define NIM_S3202_EQ5								0x2c
#define NIM_S3202_EQ6								0x2d
#define NIM_S3202_EQ7								0x2e
#define NIM_S3202_EQ8								0x2f
#define NIM_S3202_EQ9								0x30
#define NIM_S3202_EQ10								0x31
#define NIM_S3202_EQ11								0x32
#define NIM_S3202_EQ12								0x33
#define NIM_S3202_EQ13								0x34
#define NIM_S3202_EQ14								0x35

//Frame Sync
#define NIM_S3202_FRAME_SYNC1						0x36
#define NIM_S3202_FRAME_SYNC2						0x37

//FSM register
#define NIM_S3202_FSM1								0x38
#define NIM_S3202_FSM2								0x39
#define NIM_S3202_FSM3								0x3a
#define NIM_S3202_FSM4								0x3b
#define NIM_S3202_FSM5								0x3c
#define NIM_S3202_FSM6								0x3d
#define NIM_S3202_FSM7								0x3e
#define NIM_S3202_FSM8								0x3f
#define NIM_S3202_FSM9								0x40
#define NIM_S3202_FSM10							0x41
#define NIM_S3202_FSM11							0x42
#define NIM_S3202_FSM12							0x43
#define NIM_S3202_FSM13							0x44
#define NIM_S3202_FSM14							0x45
#define NIM_S3202_FSM15							0x46
#define NIM_S3202_FSM16							0x47
#define NIM_S3202_FSM17							0x48
#define NIM_S3202_FSM18							0x49
#define NIM_S3202_FSM19							0x4a

//ESTM register
#define NIM_S3202_ESTM1							0x4b
#define NIM_S3202_ESTM2							0x4c
#define NIM_S3202_ESTM3							0x4d
#define NIM_S3202_ESTM4							0x4e
#define NIM_S3202_ESTM5							0x4f

//PN
#define NIM_S3202_PN1								0x50
#define NIM_S3202_PN2								0x51


//TR Loop register
#define NIM_S3202_TR_LOOP15						0x52
#define NIM_S3202_TR_LOOP16						0x53
#define NIM_S3202_TR_LOOP17						0x54
#define NIM_S3202_TR_LOOP18						0x55

//Monitor Register
#define NIM_S3202_MONITOR1						0x56
#define NIM_S3202_MONITOR2						0x57
#define NIM_S3202_MONITOR3						0x58
#define NIM_S3202_MONITOR4						0x59
#define NIM_S3202_MONITOR5						0x5a
#define NIM_S3202_MONITOR6						0x5b


#define NIM_S3202_AGC15							0x5c
#define NIM_S3202_AGC16							0x5d


#define NIM_S3202_RS_BER1							0x5e
#define NIM_S3202_RS_BER2							0x5f
#define NIM_S3202_RS_BER3							0x60
#define NIM_S3202_RS_BER4							0x61
#define NIM_S3202_RS_BER5							0x62

#define NIM_S3202_EQ_COEF1							0x63
#define NIM_S3202_EQ_COEF2							0x64
#define NIM_S3202_EQ_COEF3							0x65
#define NIM_S3202_EQ_COEF4							0x66
#define NIM_S3202_EQ_COEF5							0x67

#define NIM_S3202_PLL_CFG							0x68
#define NIM_S3202_EADC_CFG1						0x69
#define NIM_S3202_EADC_CFG2						0x6a

#define NIM_S3202_QAM_DIAGNOSE					0x6b

#define NIM_S3202_SNR_MONI1						0x6c
#define NIM_S3202_SNR_MONI2						0x6d

#define NIM_S3202_I2C_REP							0x6e

#define NIM_S3202_FSM_REG20						0x73
#define NIM_S3202_FSM_REG21						0x74
#define NIM_S3202_FSM_REG22						0x75
#define NIM_S3202_FSM_REG23						0x76

#define NIM_S3202_AGC17							0x77
#define NIM_S3202_AGC18							0x78

#define NIM_S3202_EQ15								0x79
#define NIM_S3202_EQ16								0x7a
#define NIM_S3202_EQ17								0x7b
#define NIM_S3202_EQ18								0x7c

#define NIM_S3202_MONITOR7						0x7d

#define NIM_S3202_Q16_1							0x80
#define NIM_S3202_Q16_2							0x81
#define NIM_S3202_Q32_MODE0_1						0x82
#define NIM_S3202_Q32_MODE0_2						0x83
#define NIM_S3202_Q32_MODE1_1						0x84
#define NIM_S3202_Q32_MODE1_2						0x85
#define NIM_S3202_Q32_MODE2_1						0x86
#define NIM_S3202_Q32_MODE2_2						0x87

#define NIM_S3202_Q64_MODE0_1						0x88
#define NIM_S3202_Q64_MODE0_2						0x89
#define NIM_S3202_Q64_MODE1_1						0x8a
#define NIM_S3202_Q64_MODE1_2						0x8b
#define NIM_S3202_Q64_MODE2_1						0x8c
#define NIM_S3202_Q64_MODE2_2						0x8d

#define NIM_S3202_Q128_MODE0_1					0x8e
#define NIM_S3202_Q128_MODE0_2					0x8f
#define NIM_S3202_Q128_MODE1_1					0x90
#define NIM_S3202_Q128_MODE1_2					0x91
#define NIM_S3202_Q128_MODE2_1					0x92
#define NIM_S3202_Q128_MODE2_2					0x93

#define NIM_S3202_Q256_MODE0_1					0x94
#define NIM_S3202_Q256_MODE0_2					0x95
#define NIM_S3202_Q256_MODE1_1					0x96
#define NIM_S3202_Q256_MODE1_2					0x97
#define NIM_S3202_Q256_MODE2_1					0x98
#define NIM_S3202_Q256_MODE2_2					0x99
#define NIM_S3202_Q256_MODE3_1					0x9a
#define NIM_S3202_Q256_MODE3_2					0x9b

#define NIM_S3202_CR_THREAD_POWER0				0x9c
#define NIM_S3202_CR_THREAD_POWER1				0x9d

#define NIM_S3202_CR_LOCK_THRD_0					0x9e
#define NIM_S3202_CR_LOCK_THRD_1					0x9f
#define NIM_S3202_CR_LOCK_THRD_2					0xa0
#define NIM_S3202_CR_LOCK_THRD_3					0xa1
#define NIM_S3202_CR_LOCK_THRD_4					0xa2
#define NIM_S3202_CR_LOCK_THRD_5					0xa3
#define NIM_S3202_CR_LOCK_THRD_6					0xa4
#define NIM_S3202_CR_LOCK_THRD_7					0xa5
#define NIM_S3202_CR_LOCK_THRD_8					0xa6
#define NIM_S3202_CR_LOCK_THRD_9					0xa7
#define NIM_S3202_CR_LOCK_THRD_10					0xa8
#define NIM_S3202_CR_LOCK_THRD_11					0xa9
#define NIM_S3202_CR_LOCK_THRD_12					0xaa
#define NIM_S3202_CR_LOCK_THRD_13					0xab
#define NIM_S3202_CR_LOCK_THRD_14					0xac
#define NIM_S3202_CR_LOCK_THRD_15					0xad
#define NIM_S3202_CR_LOCK_THRD_16					0xae
#define NIM_S3202_CR_LOCK_THRD_17					0xaf
#define NIM_S3202_CR_LOCK_THRD_18					0xb0
#define NIM_S3202_CR_LOCK_THRD_19					0xb1
#define NIM_S3202_CR_LOCK_THRD_20					0xb2
#define NIM_S3202_CR_LOCK_THRD_21					0xb3
#define NIM_S3202_CR_LOCK_THRD_22					0xb4

#define NIM_S3202_CR_LOCK_THRD_23					0xb5
#define NIM_S3202_CR_LOCK_THRD_24					0xb6
#define NIM_S3202_CR_LOCK_THRD_25					0xb7
#define NIM_S3202_CR_LOCK_THRD_26					0xb8
#define NIM_S3202_CR_LOCK_THRD_27					0xb9
#define NIM_S3202_CR_LOCK_THRD_28					0xba
#define NIM_S3202_CR_LOCK_THRD_29					0xbb
#define NIM_S3202_CR_LOCK_THRD_30					0xbc
#define NIM_S3202_CR_LOCK_THRD_31					0xbd
#define NIM_S3202_CR_LOCK_THRD_32					0xbe
#define NIM_S3202_CR_LOCK_THRD_33					0xbf
#define NIM_S3202_CR_LOCK_THRD_34					0xc0
#define NIM_S3202_CR_LOCK_THRD_35					0xc1
#define NIM_S3202_CR_LOCK_THRD_36					0xc2
#define NIM_S3202_CR_LOCK_THRD_37					0xc3
#define NIM_S3202_CR_LOCK_THRD_38					0xc4
#define NIM_S3202_CR_LOCK_THRD_39					0xc5
#define NIM_S3202_CR_LOCK_THRD_40					0xc6
#define NIM_S3202_CR_LOCK_THRD_41					0xc7
#define NIM_S3202_CR_LOCK_THRD_42					0xc8
#define NIM_S3202_CR_LOCK_THRD_43					0xc9

#define NIM_S3202_CR_PRO_PATH_GAIN_0				0xca
#define NIM_S3202_CR_PRO_PATH_GAIN_1				0xCb
#define NIM_S3202_CR_PRO_PATH_GAIN_2				0xCc
#define NIM_S3202_CR_PRO_PATH_GAIN_3				0xCD
#define NIM_S3202_CR_PRO_PATH_GAIN_4				0xCE

#define NIM_S3202_CR_INT_PATH_GAIN_0				0xcf
#define NIM_S3202_CR_INT_PATH_GAIN_1				0xd0
#define NIM_S3202_CR_INT_PATH_GAIN_2				0xd1
#define NIM_S3202_CR_INT_PATH_GAIN_3				0xd2
#define NIM_S3202_CR_INT_PATH_GAIN_4				0xd3

#define NIM_S3202_CR_TIME_OUT_FOR_ACQ_0			0xd4
#define NIM_S3202_CR_TIME_OUT_FOR_ACQ_1			0xd5
#define NIM_S3202_CR_TIME_OUT_FOR_ACQ_2			0xd6
#define NIM_S3202_CR_TIME_OUT_FOR_ACQ_3			0xd7

#define NIM_S3202_CR_TIME_OUT_FOR_DRT_0			0xd8
#define NIM_S3202_CR_TIME_OUT_FOR_DRT_1			0xd9
#define NIM_S3202_CR_TIME_OUT_FOR_DRT_2			0xda
#define NIM_S3202_CR_TIME_OUT_FOR_DRT_3			0xdb

#define NIM_S3202_CR_WORK_MODE_CTL				0xdc

//magic add for zero IF config 20090224.
#define INI_DM_FREQ_OFFSET_0						0xeb
#define INI_DM_FREQ_OFFSET_1						0xec

#define NIM_S3202_QAM_ORDER_KNOWN		1
#define NIM_S3202_QAM_ORDER_UNKNOWN		0

#define SWITCH_NIM_S3202_DEBUG	0



static const INT16 sin_table[65] = {
	0, 13, 25, 38, 50, 63, 75, 87,
	100, 112, 124, 136, 148, 160, 172, 184, 
	196, 207, 218, 230, 241, 252, 263, 273,
	284, 294, 304, 314, 324, 334, 343, 352,
	361, 370, 379, 387, 395, 403, 410, 418,
	425, 432, 438, 445, 451, 456, 462, 467,
	472, 477, 481, 485, 489, 492, 496, 499,
	501, 503, 505, 507, 509, 510, 510, 511,
	511
};



struct nim_s3202_private
{

	/* struct for QAM Configuration */
	struct   QAM_TUNER_CONFIG_DATA tuner_config_data;

	/* Tuner Initialization Function */
	INT32 (*nim_Tuner_Init)(UINT32 * ptrTun_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);

	/* Tuner Parameter Configuration Function */
	INT32 (*nim_Tuner_Control)(UINT32 Tun_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);//since there will no bandwidth demand, so pass "sym" for later use.

	/* Get Tuner Status Function */
	INT32 (*nim_Tuner_Status)(UINT32 Tun_id, UINT8 *lock);

//joey, 20120608. for RF level get.
	/* Get Tuner RF level Function */
	INT32 (*nim_Tuner_Get_rf_level)(UINT32 Tun_id, UINT32 *type, UINT32 *rf_level); //type = 0, rf_level dbuV unit, type = 1, rf_level AGC unit.

	/* Extension struct for Tuner Configuration */
	struct QAM_TUNER_CONFIG_EXT tuner_config_ext;
	
	//struct QAM_TUNER_CONFIG_API TUNER_PRIV;	

	UINT32 tuner_id;
};


struct S3202_Lock_Info
{	
	UINT32	Frequency;
	UINT32	SymbolRate;
	UINT8	Modulation;
};


INT32 nim_s3202_read(UINT8 bMemAdr, UINT8 *pData, UINT8 bLen);
INT32 nim_s3202_write(UINT8 bMemAdr, UINT8 *pData, UINT8 bLen);
INT32 nim_s3202_Tuner_Attatch(struct QAM_TUNER_CONFIG_API * ptrQAM_Tuner);
INT32 nim_s3202_attach();

static INT32 nim_s3202_open(struct nim_device *dev);
static INT32 nim_s3202_close(struct nim_device *dev);
static INT32 nim_s3202_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param);
static INT32 nim_s3202_ioctl_ext(struct nim_device *dev, INT32 cmd, void* param_list);
static INT32 nim_s3202_channel_change(struct nim_device *dev, struct NIM_Channel_Change* pstChl_Change);
static INT32 nim_s3202_quick_channel_change(struct nim_device *dev, struct NIM_Channel_Change* pstChl_Change);
static INT32 nim_s3202_channel_search(struct nim_device *dev, UINT32 freq);

static INT32 nim_s3202_get_BER(struct nim_device *dev, UINT32 *err_count);
static INT32 nim_s3202_get_lock(struct nim_device *dev, UINT8 *lock);
static INT32 nim_s3202_get_freq(struct nim_device *dev, UINT32 *freq);
static INT32 nim_s3202_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate);
static INT32 nim_s3202_get_qam_order(struct nim_device *dev, UINT8 *QAM_order);
static INT32 nim_s3202_get_AGC(struct nim_device *dev, UINT8 *agc);
static INT32 nim_s3202_get_SNR(struct nim_device *dev, UINT8 *snr);
static INT32 nim_s3202_get_PER(struct nim_device *dev, UINT32 *RsUbc);

static INT32 nim_s3202_set_perf_level(struct nim_device *dev, UINT32 level);

static void nim_s3202_sw_test_thread();
static void nim_s3202_set_qam_order(UINT8 known, UINT8 qam_order );
static void nim_s3202_set_rs(UINT32 sym);
static void nim_s3202_set_search_freq(UINT16 freq_range);
static void nim_s3202_set_search_rs(UINT16 rs_min, UINT16 rs_max );
static void nim_s3202_set_delfreq(INT16 delfreq );

static INT32 nim_s3202_monitor_berper(struct nim_device *dev, BOOL *bervalid);
static INT32 nim_s3202_get_RF_Level(struct nim_device *dev, UINT16 *RfLevel);
static INT32 nim_s3202_get_CN_value(struct nim_device *dev, UINT16 *CNValue);

static UINT32 Log10Times100_L( UINT32 x);

#endif	/* __LLD_NIM_S3202_H__ */




