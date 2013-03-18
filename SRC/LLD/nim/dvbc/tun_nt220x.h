/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    Tun_nt220x.h
*
*    Description:    Header file for alpstdae.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  20100902	Joey.gao	Ver 0.1		Create file.
*****************************************************************************/

#ifndef __LLD_TUN_NT220X_H__
#define __LLD_TUN_NT220X_H__

#include <types.h>
#include <hld/nim/nim_tuner.h>

#ifdef __cplusplus
extern "C"
{
#endif

INT32 tun_nt220x_init(UINT32 * ptrTun_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);

INT32 tun_nt220x_control(UINT32 Tun_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);

INT32 tun_nt220x_status(UINT32 Tun_id, UINT8 *lock);

unsigned int  tun_nt220xwrite	(UINT32  uAddress, UINT32  uSubAddress, UINT32  uNbData, UINT32* pDataBuff);
/*Bool	tun_nt220xwritebit(UInt32  uAddress,
                                 UInt32  uSubAddress,
                                 UInt32  uMaskValue,
                                 UInt32  uValue);*/
unsigned int	tun_nt220xread(UINT32  uAddress, UINT32  uSubAddress,UINT32  uNbData,UINT32* pDataBuff);

#ifdef __cplusplus
}
#endif

#endif  /* __LLD_TUN_NT220X_H__ */


