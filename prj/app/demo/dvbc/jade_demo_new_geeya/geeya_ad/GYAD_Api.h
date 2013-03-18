/*****************************************************************************

File Name  : GYAD_Api.h

Description: geeya advertisement API.

Copyright (C) 2009 Chengdu Geeya Technology Co.,Ltd

Author: Archer 

Create Date:2010.09.09

* Date				Name			    Modification
* ---------		-----------			------------
* 2010.09.09		      Archer			     Created
*  
*****************************************************************************/

#ifndef __GYAD_API_H__
#define __GYAD_API_H__

#define GYAD_Print(fmt)	GYSTBAD_Print fmt
//#define GYAD_Print libc_printf

GYADVOID *GYADAPI_Malloc(GYADU32 size);

GYADVOID GYADAPI_Free(GYADVOID *p);

GYADVOID GYADAPI_Memset(GYADVOID *dst, GYADU32 val, GYADU32 size);

GYADVOID GYADAPI_Memcpy(GYADVOID *dst, GYADVOID *src, GYADU32 size);

GYADErrorType_e GYADAPI_RequestPrivateData(GYADU8 cReqID, const GYADU8 *pcFilter, 
											const GYADU8 *pcMask, GYADU8 cLen, 
											GYADU16 wPid, GYADU8 byWaitSeconds);

GYADVOID GYADAPI_RequestFree(GYADU8 cReqID);

GYADErrorType_e GYADAPI_SetTS(GYADVOID);

GYADVOID GYADAPI_SemaphoreLock(GYADVOID);

GYADVOID GYADAPI_SemaphoreUnlock(GYADVOID);

GYADErrorType_e GYADAPI_SaveBootAd(sGYAdInfo_t *bootADInfo, GYADU8 bootADNum, GYADU8 bootADVer);

GYADErrorType_e GYADAPI_GetBootVer(GYADU8 *version);

GYADVOID GYADAPI_GetNvram(GYADU32 *pdwAddr, GYADU32 *len);

GYADErrorType_e GYADAPI_ReadNvram(GYADU32 dwAddr, GYADU8 *data, GYADU32 len);

GYADErrorType_e GYADAPI_EraseNvram(GYADU32 dwAddr, GYADU32 len);

GYADErrorType_e GYADAPI_WriteNvram(GYADU32 dwAddr, GYADU8 *data, GYADU32 len);

GYADVOID GYADAPI_Exit(GYADVOID);

GYADErrorType_e GYADCore_Start(GYADU16 wPid);

GYADErrorType_e GYADCore_PrivateDataGot(GYADU8 cReqID, GYADU8 bOK, GYADU16 wPid, const GYADU8 *pcReceiveData, GYADU32 dwLen);

GYADErrorType_e GYADCore_GetAdInfo(sGYADRelativeInfo_t sAdRelInfo, sGYAdInfo_t *sAdInfo);

#endif

