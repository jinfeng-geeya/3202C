/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    tun_tdccg1x1f.h
*
*    Description:    Header file for tdccg1x1f.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  20080520		Trueve Hu	Ver 0.1		Create file.
*****************************************************************************/

#ifndef __LLD_TUN_TDCCG1X1F_H__
#define __LLD_TUN_TDCCG1X1F_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>

#ifdef __cplusplus
extern "C"
{
#endif

INT32 tun_tdccg1x1f_init(UINT32 * ptrTun_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);

INT32 tun_tdccg1x1f_control(UINT32 Tun_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);

INT32 tun_tdccg1x1f_status(UINT32 Tun_id, UINT8 *lock);

#ifdef __cplusplus
}
#endif

#endif  /* __LLD_TUN_ALPSTDAE_H__ */


