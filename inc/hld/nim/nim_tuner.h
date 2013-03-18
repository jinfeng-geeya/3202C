/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2005 Copyright (C)
*
*    File:    nim_tuner.h
*
*    Description:    This file contains QPSK Tuner Configuration AP Functions
*
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Aug.25.2005       Jun Zhu       Ver 0.1    Create file.
*****************************************************************************/

#ifndef _NIM_TUNER_H_
#define _NIM_TUNER_H_
/*
Nim_xxxx_attach
Function: INT32 nim_xxxx_attach (QPSK_TUNER_CONFIG_API * ptrQPSK_Tuner)
Description: QPSK Driver Attach function
Parameters:  QPSK_TUNER_CONFIG_API * ptrQPSK_Tuner, pointer to structure QPSK_TUNER_CONFIG_API
Return:         INT32, operation status code; configuration successful return with SUCCESS

Nim_Tuner _Init
Function: INT32 nim_Tuner_Init (UINT32* tuner_id,struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
Description: API function for QPSK Tuner Initialization
Parameters:  UINT32* tuner_id, return allocated tuner id value in same tuner type to demod driver.
		      struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config, parameter for tuner config
Return:         INT32, operation status code; configuration successful return with SUCCESS

Nim_Tuner_Control
Function: INT32 nim_Tuner _Control (UINT32 tuenr_id, UINT32 freq, UINT32 sym)
Description: API function for QPSK Tuner's Parameter Configuration
Parameters:  	UINT32 tuner_id, tuner device id in same tuner type
			UINT32 freq, Channel Frequency
			UINT32 sym, Channel Symbol Rate
Return:         INT32, operation status code; configuration successful return with SUCCESS

Nim_Tuner_Status
Function: INT32 nim_Tuner _Status (UINT32 tuner_id, UINT8 *lock)
Description: API function for QPSK Tuner's Parameter Configuration
Parameters:  UINT32 tuner_id, tuner device id in same tuner type
			UINT8 *lock, pointer to the place to write back the Tuner Current Status
Return:         INT32, operation status code; configuration successful return with SUCCESS
*/
#define MAX_TUNER_SUPPORT_NUM	2
#define FAST_TIMECST_AGC	1
#define SLOW_TIMECST_AGC	0

#define Tuner_Chip_SANYO        9
#define Tuner_Chip_CD1616LF_GIH	8
#define Tuner_Chip_NXP		7
#define Tuner_Chip_MAXLINEAR	6
#define Tuner_Chip_MICROTUNE	5
#define Tuner_Chip_QUANTEK	4
#define Tuner_Chip_RFMAGIC  3
#define Tuner_Chip_ALPS		2	//60120-01Angus
#define Tuner_Chip_PHILIPS	1
#define Tuner_Chip_INFINEON	0
/****************************************************************************/
#define Tuner_Chip_MAXLINEAR 6
#define Tuner_Chip_EN4020      9     

#define _1st_i2c_cmd		0
#define _2nd_i2c_cmd		1

/*Front End State*/
#define TUNER_INITIATING	0
#define TUNER_INITIATED		1
#define TUNER_TUNING		2
#define TUNER_TUNED		3

/*external demodulator config parameter*/
struct EXT_DM_CONFIG{
	UINT32 i2c_base_addr;
	UINT32 i2c_type_id;
	UINT32 dm_crystal;
	UINT32 dm_clock;
	UINT32 polar_gpio_num;
    UINT32 lock_polar_reverse;
};

#define LNB_CMD_BASE		0xf0
#define LNB_CMD_ALLOC_ID	(LNB_CMD_BASE+1)
#define LNB_CMD_INIT_CHIP	(LNB_CMD_BASE+2)
#define LNB_CMD_SET_POLAR	(LNB_CMD_BASE+3)
#define LNB_CMD_POWER_EN	(LNB_CMD_BASE+4)
/*external lnb controller config parameter*/
struct EXT_LNB_CTRL_CONFIG{
	UINT32 param_check_sum; //ext_lnb_control+i2c_base_addr+i2c_type_id = param_check_sum
	INT32 	(*ext_lnb_control) (UINT32, UINT32, UINT32);
	UINT32 i2c_base_addr;
	UINT32 i2c_type_id;
	UINT8 int_gpio_en;
	UINT8 int_gpio_polar;
	UINT8 int_gpio_num;
};


typedef struct COFDM_TUNER_CONFIG_API *PCOFDM_TUNER_CONFIG_API;


struct COFDM_TUNER_CONFIG_DATA
{
	UINT8 *ptMT352;
	UINT8 *ptMT353;
	UINT8 *ptST0360;	
	UINT8 *ptST0361;
	UINT8 *ptST0362;
	UINT8 *ptAF9003;
	UINT8  *ptNXP10048;
	UINT8  *ptSH1432;
	UINT16 *ptSH1409;

//for ddk and normal design.
	//for I/Q conncetion config. bit2: I/Q swap. bit1: I_Diff swap. bit0: Q_Diff swap.< 0: no, 1: swap>; 
	UINT8 Connection_config;
	//bit0: IF-AGC enable <0: disable, 1: enalbe>;bit1: IF-AGC slop <0: negtive, 1: positive>
	//bit2: RF-AGC enable <0: disable, 1: enalbe>;bit3: RF-AGC slop <0: negtive, 1: positive>
	//bit4: Low-if/Zero-if.<0: Low-if, 1: Zero-if>
	//bit5: RF-RSSI enable <0: disable, 1: enalbe>;bit6: RF-RSSI slop <0: negtive, 1: positive>
	//bit8: fft_gain function <0: disable, 1: enable>
	//bit9: "blank channel" searching function <0: accuate mode, 1: fast mode>
	//bit10~11: frequency offset searching range <0: +-166, 1: +-(166*2), 2: +-(166*3), 3: +-(166*4)>
	//bit12: RSSI monitor <0: disable, 1: enable>
	UINT16 Cofdm_Config;

	UINT8 AGC_REF;
	UINT8 RF_AGC_MAX;
	UINT8 RF_AGC_MIN;
	UINT8 IF_AGC_MAX;
	UINT8 IF_AGC_MIN;
	UINT32 i2c_type_sel;
	UINT32 i2c_type_sel_1;//for I2C_SUPPORT_MUTI_DEMOD	
	UINT8 demod_chip_addr;
	UINT8 demod_chip_addr1;
	UINT8 demod_chip_ver;
	UINT8 tnuer_id;
	UINT8  cTuner_Tsi_Setting_0;
	UINT8  cTuner_Tsi_Setting_1;
};

struct COFDM_TUNER_CONFIG_EXT
{
	UINT16  cTuner_Crystal;
	UINT8  cTuner_Base_Addr;		/* Tuner BaseAddress for Write Operation: (BaseAddress + 1) for Read */	
	UINT8  cChip;
	UINT8  cTuner_Ref_DivRatio;
	UINT16 wTuner_IF_Freq;
	UINT8  cTuner_AGC_TOP;
	UINT16 cTuner_Step_Freq;
	INT32  (*Tuner_Write)(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);		/* Write Tuner Program Register */
	INT32  (*Tuner_Read)(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);		/* Read Tuner Status Register */	
	INT32  (*Tuner_Write_Read)(UINT32 id, UINT8 slv_addr, UINT8 *data, UINT8 wlen,int len);
	UINT32 i2c_type_id;	/*i2c type and dev id select. bit16~bit31: type, I2C_TYPE_SCB/I2C_TYPE_GPIO. bit0~bit15:dev id, 0/1.*/				
        
        // copy from COFDM_TUNER_CONFIG_DATA struct in order to  let tuner knows whether the RF/IF AGC is enable or not.
	// esp for max3580, which uses this info to turn on/off internal power detection circuit. See max3580 user manual for detail.

	//bit0: IF-AGC enable <0: disable, 1: enalbe>;bit1: IF-AGC slop <0: negtive, 1: positive>
	//bit2: RF-AGC enable <0: disable, 1: enalbe>;bit3: RF-AGC slop <0: negtive, 1: positive>
	//bit4: Low-if/Zero-if.<0: Low-if, 1: Zero-if>
	//bit5: RF-RSSI enable <0: disable, 1: enalbe>;bit6: RF-RSSI slop <0: negtive, 1: positive>
	UINT16 Cofdm_Config;

	INT32  if_signal_target_intensity;
};

#if((SYS_PROJECT_FE == PROJECT_FE_DVBT)||(SYS_PROJECT_FE == PROJECT_FE_ISDBT))
struct COFDM_TUNER_CONFIG_API
{	
	struct COFDM_TUNER_CONFIG_DATA config_data;	
	INT32 (*nim_Tuner_Init) (UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
	INT32 (*nim_Tuner_Control) (UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 cmd_type);
	INT32 (*nim_Tuner_Status) (UINT32 tuner_id,UINT8 *lock);
    union
    {
	    INT32 (*nim_Tuner_Cal_Agc) (UINT32 tuner_id, UINT8 flag, UINT16 rf_val, UINT16 if_val, UINT8 *data);
        INT32 (*nim_Tuner_Command)(UINT32 tuner_id, INT32 cmd, UINT32 param);
    };
    void (*nim_lock_cb) (UINT8 lock);				
	struct COFDM_TUNER_CONFIG_EXT tuner_config;
	struct EXT_DM_CONFIG ext_dm_config;

    UINT32 tuner_type;
    UINT32 rev_id           : 8;
    UINT32 config_mode      : 1;
    UINT32 work_mode        : 1;    // NIM_COFDM_SOC_MODE or NIM_COFDM_ONLY_MODE
    UINT32 ts_mode          : 2;    // enum nim_cofdm_ts_mode, only for NIM_COFDM_ONLY_MODE
    UINT32 reserved         : 20;
};
// >=0: successful; 1: need to config pin mux
// <0: failed
//INT32 nim_config_tuner(UINT32 type, UINT32 base_addr, struct COFDM_TUNER_CONFIG_API *tuner);

#else
struct COFDM_TUNER_CONFIG_API
{	
	struct COFDM_TUNER_CONFIG_DATA config_data;	
	INT32 (*nim_Tuner_Init) (struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);		
	INT32 (*nim_Tuner_Control) (UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 cmd_type);	
	INT32 (*nim_Tuner_Status) (UINT8 *lock);				
	INT32 (*nim_Tuner_Cal_Agc) (UINT8 flag, UINT16 rf_val, UINT16 if_val, UINT8 *data);

    void (*nim_lock_cb) (UINT8 lock);				
	struct COFDM_TUNER_CONFIG_EXT tuner_config;
	struct EXT_DM_CONFIG ext_dm_config;
};
#endif

typedef enum//For  TSI  select
{
	NIM_0_SPI_0	  = 0,
	NIM_0_SPI_1	 ,
	NIM_1_SPI_0	 ,
	NIM_1_SPI_1	 ,
	NIM_0_SSI_0	 ,
	NIM_0_SSI_1	 ,
	NIM_1_SSI_0	 ,
	NIM_1_SSI_1	
} NIM_TSI_Setting ;
/* For customer to adjust RF level when it is necessary.	*/
typedef UINT32 (*nim_rf_level_func_t)(UINT16 if_agc_gain, UINT16 rf_agc_gain);

#if (SYS_TUN_MODULE == ANY_TUNER)
extern UINT32 SYS_TUN_TYPE;
extern UINT32 SYS_TUN_BASE_ADDR;

extern INT32 nim_mt2060_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_mt2060_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);	
extern INT32 nim_mt2060_status(UINT8 *lock);

extern INT32 tun_SCH201A_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_SCH201A_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32 tun_SCH201A_status(UINT8 *lock);


extern INT32 tun_TD1336_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_TD1336_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32 tun_TD1336_status(UINT8 *lock);

extern INT32 tun_DTT76806_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_DTT76806_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32 tun_DTT76806_status(UINT8 *lock);

extern INT32 tun_DTT76801_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_DTT76801_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32 tun_DTT76801_status(UINT8 *lock);

extern INT32 tun_DTT76809_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_DTT76809_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32 tun_DTT76809_status(UINT8 *lock);

extern INT32 tun_uba00ap_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_uba00ap_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32 tun_uba00ap_status(UINT8 *lock);

extern INT32 tun_DPH261D_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_DPH261D_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32 tun_DPH261D_status(UINT8 *lock);

extern INT32 nim_mt2131_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_mt2131_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);	
extern INT32 nim_mt2131_status(UINT8 *lock);

extern INT32 tun_dtt75300_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32  tun_dtt75300_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);	
extern INT32  tun_dtt75300_status(UINT32 tuner_id,UINT8 *lock);

extern INT32 nim_DTT7596_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32  nim_DTT7596_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);	
extern INT32  nim_DTT7596_status(UINT32 tuner_id,UINT8 *lock);

extern INT32	nim_DTF8570_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32   nim_DTF8570_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32   nim_DTF8570_status(UINT32 tuner_id,UINT8 *lock);

extern INT32	nim_DTT75411_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32   nim_DTT75411_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32   nim_DTT75411_status(UINT32 tuner_id,UINT8 *lock);

extern INT32   tun_edt1022_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32   tun_edt1022_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32   tun_edt1022_status(UINT32 tuner_id,UINT8 *lock);

extern INT32   tun_td1611alf_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32   tun_td1611alf_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32   tun_td1611alf_status(UINT32 tuner_id,UINT8 *lock);

 extern INT32 tun_ed5065_init(UINT8 tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
  extern INT32 tun_ed5065_control(UINT8 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
  extern INT32 tun_ed5065_status(UINT8 tuner_id,UINT8 *lock);     


extern INT32	nim_TDA18211_init(UINT8 tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32   nim_TDA18211_control(UINT8 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32   nim_TDA18211_status(UINT8 tuner_id,UINT8 *lock);

extern INT32	tun_tda18218_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32   tun_tda18218_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32   tun_tda18218_status(UINT32 tuner_id,UINT8 *lock);
extern INT32 tun_tda18218_command(UINT32 tuner_id, INT32 cmd, UINT32 param);


extern INT32	tun_tda18212_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32   tun_tda18212_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32   tun_tda18212_status(UINT32 tuner_id,UINT8 *lock);

extern INT32	tun_bf6009_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32   tun_bf6009_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32   tun_bf6009_status(UINT32 tuner_id,UINT8 *lock);


extern INT32 tun_rd3432_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_rd3432_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_rd3432_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);	

extern INT32 tun_max3580_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_max3580_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32 tun_max3580_status(UINT32  tuner_id, UINT8 *lock);
extern INT32 tun_max3580_command(UINT32 tuner_id, INT32 cmd, UINT32 param);


#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)
extern INT32	tun_mxl5005_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32   tun_mxl5005_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32   tun_mxl5005_status(UINT32 tuner_id,UINT8 *lock);

extern INT32	tun_mxl5007_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32  tun_mxl5007_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32  tun_mxl5007_status(UINT32 tuner_id,UINT8 *lock);
extern INT32 tun_mxl5007_command(UINT32 tuner_id, INT32 cmd, UINT32 param);


#else
extern INT32	tun_mxl5005_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32   tun_mxl5005_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32   tun_mxl5005_status(UINT8 *lock);

extern INT32  tun_mxl5007_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32  tun_mxl5007_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32  tun_mxl5007_status(UINT8 *lock);

#endif
/*extern INT32 tun_mxl5005_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32  tun_mxl5005_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);	
extern INT32  tun_mxl5005_status(UINT8 *lock);*/

extern INT32 tun_ed6092b_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_ed6092b_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)	;
extern INT32 tun_ed6092b_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 tun_mxl136_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_mxl136_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_mxl136_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);

#else

#if (SYS_TUN_MODULE==SH201A)
extern INT32 tun_SCH201A_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_SCH201A_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32 tun_SCH201A_status(UINT8 *lock);

#elif (SYS_TUN_MODULE==TD1336)
extern INT32 tun_TD1336_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_TD1336_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32 tun_TD1336_status(UINT8 *lock);

#elif (SYS_TUN_MODULE==DTT76806)
extern INT32 tun_DTT76806_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_DTT76806_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32 tun_DTT76806_status(UINT8 *lock);

#elif ((SYS_TUN_MODULE==DTT76801)||(SYS_TUN_MODULE==DTT76852))
extern INT32 tun_DTT76801_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_DTT76801_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32 tun_DTT76801_status(UINT8 *lock);
#elif (SYS_TUN_MODULE==DTT76809)
extern INT32 tun_DTT76809_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_DTT76809_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32 tun_DTT76809_status(UINT8 *lock);

#elif( (SYS_TUN_MODULE==UBA00AP)||(SYS_TUN_MODULE==UBD00AL))
extern INT32 tun_uba00ap_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_uba00ap_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32 tun_uba00ap_status(UINT8 *lock);

#elif (SYS_TUN_MODULE==SAMSUNG_DPH261D)
extern INT32 tun_DPH261D_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_DPH261D_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32 tun_DPH261D_status(UINT8 *lock);

#elif (SYS_TUN_MODULE==MXL5005)
#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)
extern INT32	tun_mxl5005_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32   tun_mxl5005_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32   tun_mxl5005_status(UINT32 tuner_id,UINT8 *lock);
#else
extern INT32	tun_mxl5005_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32   tun_mxl5005_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32   tun_mxl5005_status(UINT8 *lock);
#endif
#elif (SYS_TUN_MODULE==MXL5007)
#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)
extern INT32	tun_mxl5007_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32  tun_mxl5007_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32  tun_mxl5007_status(UINT32 tuner_id,UINT8 *lock);
extern INT32 tun_mxl5007_command(UINT32 tuner_id, INT32 cmd, UINT32 param);

#else
extern INT32  tun_mxl5007_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32  tun_mxl5007_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32  tun_mxl5007_status(UINT8 *lock);
#endif
#elif (SYS_TUN_MODULE==MT2060)
extern INT32 nim_mt2060_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_mt2060_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);	
extern INT32 nim_mt2060_status(UINT8 *lock);
#elif (SYS_TUN_MODULE==MT2131)
extern INT32 nim_mt2131_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_mt2131_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);	
extern INT32 nim_mt2131_status(UINT8 *lock);

#elif (SYS_TUN_MODULE==DTT75300)
extern INT32 tun_dtt75300_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32  tun_dtt75300_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);	
extern INT32  tun_dtt75300_status(UINT32 tuner_id,UINT8 *lock);
/*#elif (SYS_TUN_MODULE==MXL5005)
extern INT32 tun_mxl5005_init(struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32  tun_mxl5005_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);	
extern INT32  tun_mxl5005_status(UINT8 *lock);*/
#elif (SYS_TUN_MODULE==DTT7596)
extern INT32 nim_DTT7596_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32  nim_DTT7596_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);	
extern INT32  nim_DTT7596_status(UINT32 tuner_id,UINT8 *lock);

#elif (SYS_TUN_MODULE==DTF8570)
extern INT32	nim_DTF8570_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32  nim_DTF8570_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32  nim_DTF8570_status(UINT32 tuner_id,UINT8 *lock);

/*#elif (SYS_TUN_MODULE==MXL5005)
extern INT32	tun_mxl5005_init(UINT8 tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32  tun_mxl5005_control(UINT8 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32  tun_mxl5005_status(UINT8 tuner_id,UINT8 *lock);
#elif (SYS_TUN_MODULE==MXL5007)
extern INT32	tun_mxl5007_init(UINT32 tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32  tun_mxl5007_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32  tun_mxl5007_status(UINT32 tuner_id,UINT8 *lock);*/
#elif (SYS_TUN_MODULE==DTT75411)
extern INT32	nim_DTT75411_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32   nim_DTT75411_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32   nim_DTT75411_status(UINT32 tuner_id,UINT8 *lock);

#elif (SYS_TUN_MODULE==TDA18211)
extern INT32	nim_TDA18211_init(UINT8 tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32   nim_TDA18211_control(UINT8 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32   nim_TDA18211_status(UINT8 tuner_id,UINT8 *lock);
#elif (SYS_TUN_MODULE==EDT1022B)
extern INT32	tun_edt1022_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32   tun_edt1022_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32   tun_edt1022_status(UINT32 tuner_id,UINT8 *lock);
#elif (SYS_TUN_MODULE==TD1611ALF)
  extern INT32 tun_td1611alf_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
  extern INT32 tun_td1611alf_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
  extern INT32 tun_td1611alf_status(UINT32 tuner_id,UINT8 *lock);
#elif (SYS_TUN_MODULE==ED5065)
 extern INT32 tun_ed5065_init(UINT8 tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
  extern INT32 tun_ed5065_control(UINT8 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
  extern INT32 tun_ed5065_status(UINT8 tuner_id,UINT8 *lock);     
#elif (SYS_TUN_MODULE==QT3010)
extern INT32 tuner_3010_init(UINT8 tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32  tuner_3010_control(UINT8 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);	
extern INT32  tuner_3010_status(UINT8 tuner_id,UINT8 *lock);   
#elif (SYS_TUN_MODULE==TDA18218)
extern INT32	tun_tda18218_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32   tun_tda18218_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32   tun_tda18218_status(UINT32 tuner_id,UINT8 *lock);
extern INT32 tun_tda18218_command(UINT32 tuner_id, INT32 cmd, UINT32 param);
#elif (SYS_TUN_MODULE==RADIO3432)
extern INT32 tun_rd3432_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_rd3432_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_rd3432_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);	
#elif (SYS_TUN_MODULE==MAX3580)
extern INT32 tun_max3580_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_max3580_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32 tun_max3580_status(UINT32  tuner_id, UINT8 *lock);
extern INT32 tun_max3580_command(UINT32 tuner_id, INT32 cmd, UINT32 param);
#elif (SYS_TUN_MODULE==ED6092B)
extern INT32 tun_ed6092b_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_ed6092b_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)	;
extern INT32 tun_ed6092b_status(UINT32 tuner_id, UINT8 *lock);
#elif (SYS_TUN_MODULE==TDA18212)
extern INT32	tun_tda18212_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32   tun_tda18212_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32   tun_tda18212_status(UINT32 tuner_id,UINT8 *lock);
#elif (SYS_TUN_MODULE==BF6009)
extern INT32	tun_bf6009_init(UINT32 *tuner_id,struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32   tun_bf6009_control(UINT32 tuner_id,UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
extern INT32   tun_bf6009_status(UINT32 tuner_id,UINT8 *lock);
#elif(SYS_TUN_MODULE==MXL136)
extern INT32 tun_mxl136_init(UINT32 *tuner_id, struct COFDM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_mxl136_status(UINT32 tuner_id, UINT8 *lock);
extern INT32 tun_mxl136_control(UINT32 tuner_id, UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd);
#endif
#endif

struct QPSK_TUNER_CONFIG_DATA
{
	UINT16 Recv_Freq_Low;
	UINT16 Recv_Freq_High;
	UINT16 Ana_Filter_BW;
	UINT8 Connection_config;
	UINT8 Reserved_byte;
	UINT8 AGC_Threshold_1;
	UINT8 AGC_Threshold_2;
	UINT16 QPSK_Config;/*bit0:QPSK_FREQ_OFFSET,bit1:EXT_ADC,bit2:IQ_AD_SWAP,bit3:I2C_THROUGH,bit4:polar revert bit5:NEW_AGC1,bit6bit7:QPSK bitmode:
	                                00:1bit,01:2bit,10:4bit,11:8bit*/
};

struct QPSK_TUNER_CONFIG_EXT
{
	UINT16 wTuner_Crystal;			/* Tuner Used Crystal: in KHz unit */
	UINT8  cTuner_Base_Addr;		/* Tuner BaseAddress for Write Operation: (BaseAddress + 1) for Read */
	UINT8  cTuner_Out_S_D_Sel;		/* Tuner Output mode Select: 1 --> Single end, 0 --> Differential */
	UINT32 i2c_type_id;	/*i2c type and dev id select. bit16~bit31: type, I2C_TYPE_SCB/I2C_TYPE_GPIO. bit0~bit15:dev id, 0/1.*/	
};

struct QPSK_TUNER_CONFIG_API
{
	/* struct for QPSK Configuration */
	struct   QPSK_TUNER_CONFIG_DATA config_data;

	/* Tuner Initialization Function */
	INT32 (*nim_Tuner_Init) (UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);

	/* Tuner Parameter Configuration Function */
	INT32 (*nim_Tuner_Control) (UINT32 tuner_id, UINT32 freq, UINT32 sym);

	/* Get Tuner Status Function */
	INT32 (*nim_Tuner_Status) (UINT32 tuner_id, UINT8 *lock);

	/* Extension struct for Tuner Configuration */
	struct QPSK_TUNER_CONFIG_EXT tuner_config;
	struct EXT_DM_CONFIG ext_dm_config;
	struct EXT_LNB_CTRL_CONFIG ext_lnb_config;
	UINT32 device_type;	//current chip type. only used for M3501A
};

/*--- Tuner MAX2118 Callback Functions --- */
extern INT32 nim_max2118_init (UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_max2118_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_max2118_status(UINT32 tuner_id, UINT8 *lock);

/*--- Tuner IX2410 Callback Functions --- */
extern INT32 nim_ix2410_init(UINT32* tuner_id,  struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_ix2410_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_ix2410_status(UINT32 tuner_id, UINT8 *lock);

/*--- Tuner SL1935 Callback Functions --- */
extern INT32 nim_sl1935_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_sl1935_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_sl1935_status(UINT32 tuner_id, UINT8 * lock);

/*--- Tuner ZL10036 Callback Functions --- */
extern INT32 nim_zl10036_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_zl10036_control(UINT32 tuner_id, UINT32 freq, UINT32 bs);
extern INT32 nim_zl10036_status(UINT32 tuner_id, UINT8 *lock);

/*--- Tuner ZL10039 Callback Functions --- */
extern INT32 nim_zl10039_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_zl10039_control(UINT32 tuner_id, UINT32 freq , UINT32 sym );
extern INT32 nim_zl10039_status(UINT32 tuner_id, UINT8 *lock);

/*--- Tuner STB6000 Callback Functions --- */
extern INT32 nim_st6000_init(UINT32* tuner_id,  struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_st6000_control(UINT32 tuner_id, UINT32 freq, UINT16 sym );
extern INT32 nim_st6000_status(UINT32 tuner_id, UINT8 *lock);

/*--- Tuner IX2476 Callback Functions --- */
extern INT32 nim_ix2476_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_ix2476_control(UINT32 tuner_id, UINT32 freq, UINT16 bs);
extern INT32 nim_ix2476_status(UINT32 tuner_id, UINT8 *lock);

/*--- Tuner Tda8262 Callback Functions --- */
extern INT32 tun_tda8262_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_tda8262_control(UINT32 tuner_id, UINT32 freq , UINT32 sym );
extern INT32 tun_tda8262_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_hz6306_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_hz6306_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_hz6306_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_vz7306_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_vz7306_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_vz7306_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_vz7803_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_vz7803_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_vz7803_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_stv6110_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_stv6110_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_stv6110_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_rda5812_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_rda5812_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_rda5812_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_av2011_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_av2011_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_av2011_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_zl10037_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_zl10037_control(UINT32 tuner_id, UINT32 freq , UINT32 sym );
extern INT32 nim_zl10037_status(UINT32 tuner_id, UINT8 *lock);

extern INT32 nim_vz7851_init(UINT32* tuner_id, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_vz7851_control(UINT32 tuner_id, UINT32 freq, UINT32 sym);
extern INT32 nim_vz7851_status(UINT32 tuner_id, UINT8 *lock);
/*
#if (SYS_TUN_MODULE == MAX2118)
extern INT32 nim_max2118_init (UINT32 para, struct QPSK_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 nim_max2118_control(UINT32 freq,UINT32 sym);
extern INT32 nim_max2118_status(UINT8 *lock);
#elif (SYS_TUN_MODULE == IX2410)
extern INT32 nim_ix2410_control(UINT32 freq, UINT32 sym);
extern INT32 nim_ix2410_status(UINT8 *lock);
#elif (SYS_TUN_MODULE == SL1935)
extern INT32 nim_sl1935_control(UINT32 freq, UINT32 sym);
extern INT32 nim_sl1935_status(UINT8 *lock);
#elif( SYS_TUN_MODULE == ZL10039 )
extern INT32 nim_zl10039_init(UINT32 para);
extern INT32 nim_zl10039_control(UINT32 freq, UINT32 sym);
extern INT32 nim_zl10039_status(UINT8 *lock);
#elif( SYS_TUN_MODULE == IX2476 )
extern INT32 nim_ix2476_init(UINT32 para);
extern INT32 nim_ix2476_control(UINT32 freq, UINT32 sym);
extern INT32 nim_ix2476_status(UINT8 *lock);
#elif( SYS_TUN_MODULE == STB6000 )
extern INT32 nim_st6000_control(UINT32 freq, UINT32 sym);
extern INT32 nim_st6000_status(UINT8 *lock);
#endif
*/

struct QAM_TUNER_CONFIG_DATA
{
	UINT8 RF_AGC_MAX;//x.y V to xy value, 5.0v to 50v(3.3v to 33v)Qam then use it configue register.
	UINT8 RF_AGC_MIN;//x.y V to xy value, 5.0v to 50v(3.3v to 33v)Qam then use it configue register.
	UINT8 IF_AGC_MAX;//x.y V to xy value, 5.0v to 50v(3.3v to 33v)Qam then use it configue register.
	UINT8 IF_AGC_MIN;//x.y V to xy value, 5.0v to 50v(3.3v to 33v)Qam then use it configue register.
	UINT8 AGC_REF; //the average amplitude to full scale of A/D. % percentage rate.
    UINT8 cTuner_Tsi_Setting;
};

struct QAM_TUNER_CONFIG_EXT
{
	UINT8  cTuner_Crystal;
	UINT8  cTuner_Base_Addr;		/* Tuner BaseAddress for Write Operation: (BaseAddress + 1) for Read */
	UINT8  cChip;
	UINT8  cTuner_special_config;		/*0x01, RF AGC is disabled*/
	UINT8  cTuner_Ref_DivRatio;
	UINT16 wTuner_IF_Freq;
	UINT8  cTuner_AGC_TOP;
	UINT8  cTuner_Step_Freq;
//	INT32  (*Tuner_Write)(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);		/* Write Tuner Program Register */
//	INT32  (*Tuner_Read)(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);		/* Read Tuner Status Register */	
	UINT32 i2c_type_id;	/*i2c type and dev id select. bit16~bit31: type, I2C_TYPE_SCB/I2C_TYPE_GPIO. bit0~bit15:dev id, 0/1.*/	
};

struct QAM_TUNER_CONFIG_API
{
	/* struct for QAM Configuration */
	struct QAM_TUNER_CONFIG_DATA tuner_config_data;

	/* Tuner Initialization Function */
	INT32 (*nim_Tuner_Init)(UINT32 * ptrTun_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);

	/* Tuner Parameter Configuration Function */
	INT32 (*nim_Tuner_Control)(UINT32 Tun_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);//since there will no bandwidth demand, so pass "sym" for later use.

	/* Get Tuner Status Function */
	INT32 (*nim_Tuner_Status)(UINT32 Tun_id, UINT8 *lock);

	/* Extension struct for Tuner Configuration */
	struct QAM_TUNER_CONFIG_EXT tuner_config_ext;
	
	struct EXT_DM_CONFIG ext_dem_config;

};

/*--- Tuner DCT-2A Callback Functions --- */

extern INT32 tun_dct2a_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_dct2a_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);
extern INT32 tun_dct2a_status(UINT32 tuner_id, UINT8 *lock);

#endif // _NIM_TUNER_H_
