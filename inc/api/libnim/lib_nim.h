/*****************************************************************************
*    Copyright (C)2003 Ali Corporation. All Rights Reserved.
*
*    File:    lib_nim.h
*
*    Description:    This file contains head file for NIM library.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Sep.9.2003       Justin Wu       Ver 0.1    Create file.
*	2.	Nov.1,2004	Zhengdao	Ver 0.2		Porting to M3327
*****************************************************************************/


#ifndef	__LIB_NIM_H__
#define __LIB_NIM_H__

#include <types.h>
#include <retcode.h>
#include <sys_config.h>

#ifdef __cplusplus
extern "C"
{
#endif

INT32 api_nim_register(char *nim_name);
INT32 api_nim_unregister(char *nim_name);
UINT8 api_nim_get_lock();
UINT32 api_nim_get_freq();
UINT8 api_nim_get_SNR();
UINT8 api_nim_get_FEC();

#if (SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2)

	INT32 api_nim_get_fft_result(UINT32 freq, UINT32* start_adr);
	INT32 api_nim_fastscan(UINT32 sfreq, UINT32 efreq, INT32 (*callback)(UINT8 status, UINT8 polar, UINT32 freq, UINT32 sym, UINT8 fec));
	#if 0
	INT32 api_nim_autoscan(UINT32 sfreq, UINT32 efreq, INT32 (*callback)(UINT8 status, UINT8 polar, UINT32 freq, UINT32 sym, UINT8 fec));
	#endif
	INT32 api_nim_channel_change(UINT32 freq, UINT32 sym, UINT8 fec);
	UINT32 api_nim_get_BER();
	UINT32 api_nim_get_sym();
	UINT8 api_nim_get_AGC();
	
#elif (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ATSC) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)

INT32 api_nim_disable();//Sam_chen 20050616
INT32 api_nim_channel_change(UINT32 freq, UINT32 bandwidth, UINT8 guard_interval, UINT8 fft_mode,UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse, UINT8 priority);	
INT32 api_nim_channel_search(UINT32 freq,UINT32 bandwidth,UINT8 guard_interval, UINT8 fft_mode, UINT8 modulation, UINT8 fec, UINT8 usage_type, UINT8 inverse,UINT16 freq_offset, UINT8 priority);	
UINT8 api_nim_get_fftmode();
UINT8 api_nim_get_gi();
UINT8 api_nim_get_modulation();
UINT8 api_nim_get_freqinv();
UINT16 api_nim_get_AGC();
INT8 api_nim_get_priority();
INT8 api_nim_HIER_mode();

#ifdef SMART_ANT_SUPPORT
typedef void (*sm_status_update)(UINT16 value);

INT8 api_nim_Set_Smartenna(UINT8 position,UINT8 gain,	UINT8 pol,UINT8 channel);
INT8 api_nim_Get_SmartennaSetting(UINT8 *pPosition,UINT8 *pGain,UINT8 *pPol,UINT8 *pChannel);
INT8 api_nim_Get_SmartennaMetric(UINT8 metric,UINT16 *pMetric);
INT32  api_nim_AutoSeek_Smartenna(UINT32 freq, UINT16 *antval,UINT8 *detect, sm_status_update status_update );

UINT16 api_nim_get_VSB_AGC();
UINT16 api_nim_get_VSB_SNR();
UINT32 api_nim_get_VSB_PER();
#endif


#if (SYS_SDRAM_SIZE == 8 || GET_BER == SYS_FUNC_ON )
UINT32 api_nim_get_BER();
#endif

#elif (SYS_PROJECT_FE == PROJECT_FE_DVBC)
INT32  api_nim_fastscan(UINT32 sfreq, UINT32 efreq, INT32 (*callback)(UINT8 status, UINT8 polar, UINT32 freq, UINT32 sym, UINT8 fec));
#if 0
INT32 api_nim_autoscan(UINT32 sfreq, UINT32 efreq, INT32 (*callback)(UINT8 status, UINT8 polar, UINT32 freq, UINT32 sym, UINT8 fec));
#endif
INT32 api_nim_channel_change(UINT32 freq, UINT32 sym, UINT8 fec);
UINT32 api_nim_get_BER();
UINT32 api_nim_get_sym();
UINT32 api_nim_get_freq();
#endif




#ifdef __cplusplus
}
#endif

#endif  /* __LIB_NIM_H__ */
