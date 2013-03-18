/*****************************************************************************
*    Copyright (C)2007 Ali Corporation. All Rights Reserved.
*
*    File:    Tun_MxL5005.h
*
*    Description:    Header file of MAXLINEAR MxL5005 TUNER.
*    History:
*           Date          Author                 Version          Reason
*	    ============	=============	=========	=================
*   4.17.2008	     David.Deng	      Ver 0.1	     Create file.
*****************************************************************************/

#ifndef __TUN_MxL203RF_H__
#define __TUN_MxL203RF_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>


#ifdef __cplusplus
extern "C"
{
#endif

#include "MxL203RF_API.h"
#include "MxL203RF_Common.h"


INT32 tun_mxl203rf_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
INT32 tun_mxl203rf_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);
INT32 tun_mxl203rf_status(UINT32 tuner_id, UINT8 *lock);


#ifdef __cplusplus
}
#endif

#endif  /* __TUN_MxL5005_H__ */



