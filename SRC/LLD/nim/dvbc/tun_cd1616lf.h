/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    tun_cd1616lf.h
*
*    Description:    Header file for  cd1616lf.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  20090302		Magic Yang	Ver 0.1		Create file.
*****************************************************************************/

#ifndef __LLD_TUN_CD1616LF_H__
#define __LLD_TUN_CD1616LF_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>

#ifdef __cplusplus
extern "C"
{
#endif

INT32 tun_cd1616lf_init(UINT32 * ptrTun_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);

INT32 tun_cd1616lf_control(UINT32 Tun_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);

INT32 tun_cd1616lf_status(UINT32 Tun_id, UINT8 *lock);

#ifdef __cplusplus
}
#endif

#endif  /* __LLD_TUN_CD1616LF_H__ */


