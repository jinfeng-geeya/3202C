/*****************************************************************************
*    Copyright (C)2003 Ali Corporation. All Rights Reserved.
*
*    File:    nim.c
*
*    Description:    This file contains mt312 basic function in HLD.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Feb.16.2003      Justin Wu       Ver 0.1    Create file.
*	2.	Jun.12.2003      George jiang    Ver 0.2    Porting NIM.
*   3.  Aug.22.2003      Justin Wu       Ver 0.3    Update.
*	4. July 17.2006	Joey Gao		Ver 0.4 		Add Diseqc 2.X support.
*****************************************************************************/


#ifndef	__HLD_NIM_H__
#define __HLD_NIM_H__

#include <types.h>
#include <hld/nim/nim_dev.h>

/* DiSEqC mode */
#define NIM_DISEQC_MODE_22KOFF		0	/* 22kHz off */
#define	NIM_DISEQC_MODE_22KON		1	/* 22kHz on */
#define	NIM_DISEQC_MODE_BURST0		2	/* Burst mode, on for 12.5mS = 0 */
#define	NIM_DISEQC_MODE_BURST1		3	/* Burst mode, modulated 1:2 for 12.5mS = 1 */
#define	NIM_DISEQC_MODE_BYTES		4	/* Modulated with bytes from DISEQC INSTR */
#define	NIM_DISEQC_MODE_ENVELOP_ON	5	/* Envelop enable*/
#define	NIM_DISEQC_MODE_ENVELOP_OFF	6	/* Envelop disable, out put 22K wave form*/
#define	NIM_DISEQC_MODE_OTHERS		7	/* Undefined mode */
#define	NIM_DISEQC_MODE_BYTES_EXT_STEP1		8	/*Split NIM_DISEQC_MODE_BYTES to 2 steps to improve the speed,*/
#define	NIM_DISEQC_MODE_BYTES_EXT_STEP2		9	/*(30ms--->17ms) to fit some SPEC */

/* Polarization */
#define NIM_PORLAR_HORIZONTAL	0x00
#define NIM_PORLAR_VERTICAL		0x01
#define NIM_PORLAR_LEFT			0x02
#define NIM_PORLAR_RIGHT		0x03

#define NIM_PORLAR_REVERSE		0x01
#define NIM_PORLAR_SET_BY_22K	0x02

#define DISEQC2X_ERR_NO_REPLY			0x01
#define DISEQC2X_ERR_REPLY_PARITY		0x02
#define DISEQC2X_ERR_REPLY_UNKNOWN	0x03
#define DISEQC2X_ERR_REPLY_BUF_FUL	0x04


/* QAM Mode */
//for DVB-C
#define QAM16	4
#define QAM32	5
#define QAM64	6
#define QAM128	7
#define QAM256	8

//ATSC
#if (SYS_PROJECT_FE == PROJECT_FE_ATSC)	//dedicate for DVBT
#define SYS_ATSC		0x00
//#define SYS_CATV		0x01
#define CABLE_STD		0x01
#define CABLE_HRC		0x02
#define CABLE_IRC		0x03

#define MOD_8VSB		0x00
#define MOD_16VSB		0x01
#define MOD_64QAM		0x02
#define MOD_256QAM		0x03
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* Socket management functions */
INT32 nim_open(struct nim_device *dev);
INT32 nim_close(struct nim_device *dev);
INT32 nim_io_control(struct nim_device *dev, INT32 cmd, UINT32 param);
INT32 nim_get_lock(struct nim_device *dev, UINT8 *lock);
INT32 nim_get_freq(struct nim_device *dev, UINT32 *freq);
INT32 nim_get_FEC(struct nim_device *dev, UINT8 *fec);
INT32 nim_get_SNR(struct nim_device *dev, UINT8 *snr);
INT32 nim_ioctl_ext(struct nim_device *dev, INT32 cmd, void * param_list);

#if ( SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2 )	//dedicate for DVBS

INT32 nim_set_polar(struct nim_device *dev, UINT8 polar);
INT32 nim_set_12v(struct nim_device *dev, UINT8 flag);
INT32 nim_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec);
INT32 nim_channel_search(struct nim_device *dev, UINT32 freq);
INT32 nim_DiSEqC_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt);
INT32 nim_DiSEqC2X_operate(struct nim_device *dev, UINT32 mode, UINT8* cmd, UINT8 cnt, UINT8 *rt_value, UINT8 *rt_cnt);

INT32 nim_get_sym(struct nim_device *dev, UINT32 *sym);
INT32 nim_get_BER(struct nim_device *dev, UINT32 *ber);
INT32 nim_get_fft_result(struct nim_device *dev, UINT32 freq, UINT32* start_adr);
INT32 nim_get_AGC(struct nim_device *dev, UINT8 *agc);

#elif (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ATSC) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT) //dedicate for DVBT

INT32 nim_disable(struct nim_device *dev);//Sam_chen 20050616
INT32 nim_channel_change(struct nim_device * dev, UINT32 freq, UINT32 bandwidth,UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse, UINT8 priority);
INT32 nim_channel_search(struct nim_device *dev, UINT32 freq,UINT32 bandwidth,UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse,UINT16 freq_offset, UINT8 priority);
INT32 nim_get_gi(struct nim_device *dev, UINT8 *guard_interval);
INT32 nim_get_fftmode(struct nim_device *dev, UINT8 *fft_mode);
INT32 nim_get_modulation(struct nim_device *dev, UINT8 *modulation);
INT32 nim_get_spec_inv(struct nim_device *dev, UINT8 *inv);
INT32 nim_get_AGC(struct nim_device *dev, UINT16 *agc);
INT32 nim_get_HIER_mode(struct nim_device *dev, UINT8*hier);
INT8 nim_get_priority(struct nim_device *dev, UINT8*priority);
#ifdef SMART_ANT_SUPPORT
INT32 nim_Set_Smartenna(struct nim_device *dev, UINT8 position,UINT8 gain,UINT8 pol,UINT8 channel);
INT32 nim_Get_SmartennaSetting(struct nim_device *dev,UINT8 *pPosition,UINT8 *pGain,UINT8 *pPol,UINT8 *pChannel);
INT32 nim_Get_SmartennaMetric(struct nim_device *dev, UINT8 metric,UINT16 *pMetric);
INT32 nim_get_VSB_AGC(struct nim_device *dev, UINT16 *agc);
INT32 nim_get_VSB_SNR(struct nim_device *dev, UINT16 *snr);
INT32 nim_get_VSB_PER(struct nim_device *dev, UINT32 *per);
#endif




#elif ( SYS_PROJECT_FE == PROJECT_FE_DVBC )	//dedicate for DVBC
INT32 nim_get_sym(struct nim_device *dev, UINT32 *sym);
INT32 nim_get_BER(struct nim_device *dev, UINT32 *ber);
INT32 nim_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec);
INT32 nim_quick_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec);

#endif

#ifdef __cplusplus
}
#endif

#endif  /* __HLD_NIM_H__ */
