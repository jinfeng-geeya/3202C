/*****************************************************************************

File Name  : GYAD_Api.c

Description: geeya advertisement API.

Copyright (C) 2009 Chengdu Geeya Technology Co.,Ltd

Author: Archer 

Create Date:2010.09.09

* Date				Name			    Modification
* ---------		-----------			------------
* 2010.09.09		      Archer			     Created
*  
*****************************************************************************/

#include  "GYAD_Interface.h"
#include  "GYAD_Api.h"

GYADVOID *GYADAPI_Malloc(GYADU32 size)
{
	return GYSTBAD_Malloc(size);
}

GYADVOID GYADAPI_Free(GYADVOID *p)
{
	GYSTBAD_Free(p);
}

GYADVOID GYADAPI_Memset(GYADVOID *dst, GYADU32 val, GYADU32 size)
{
	GYSTBAD_Memset(dst, val, size);
}

GYADVOID GYADAPI_Memcpy(GYADVOID *dst, GYADVOID *src, GYADU32 size)
{
	GYSTBAD_Memcpy(dst, src, size);
}

GYADErrorType_e GYADAPI_RequestPrivateData(GYADU8 cReqID, const GYADU8 *pcFilter, 
											const GYADU8 *pcMask, GYADU8 cLen, 
											GYADU16 wPid, GYADU8 byWaitSeconds)
{
	return GYSTBAD_RequestPrivateData(cReqID, pcFilter, pcMask, cLen, wPid, byWaitSeconds);
}

GYADVOID GYADAPI_RequestFree(GYADU8 cReqID)
{
	GYSTBAD_RequestFree(cReqID);
}

/*********************
*set freq to lock
*********************/

GYADErrorType_e GYADAPI_SetTS(GYADVOID)
{
	return GYSTBAD_SetTS();
}

GYADVOID GYADAPI_SemaphoreLock(GYADVOID)
{
	GYSTBAD_SemaphoreLock();
}

GYADVOID GYADAPI_SemaphoreUnlock(GYADVOID)
{
	GYSTBAD_SemaphoreUnlock();
}

GYADErrorType_e GYADAPI_SaveBootAd(sGYAdInfo_t *bootADInfo, GYADU8 bootADNum, GYADU8 bootADVer)
{
	return GYSTBAD_SaveBootAd(bootADInfo, bootADNum, bootADVer);

}

GYADErrorType_e GYADAPI_GetBootVer(GYADU8 *version)
{
	return GYSTBAD_GetBootVer(version);

}

GYADVOID GYADAPI_GetNvram(GYADU32 *pdwAddr, GYADU32 *len)
{
	GYSTBAD_GetNvram(pdwAddr, len);
}

GYADErrorType_e GYADAPI_ReadNvram(GYADU32 dwAddr, GYADU8 *data, GYADU32 len)
{
	return GYSTBAD_ReadNvram(dwAddr, data, len);

}

GYADErrorType_e GYADAPI_EraseNvram(GYADU32 dwAddr, GYADU32 len)
{
	GYSTBAD_EraseNvram(dwAddr, len);
}

GYADErrorType_e GYADAPI_WriteNvram(GYADU32 dwAddr, GYADU8 *data, GYADU32 len)
{
	return GYSTBAD_WriteNvram(dwAddr, data, len);
}

GYADVOID GYADAPI_Exit(GYADVOID)
{
	GYSTBAD_ADExit();
}

GYADErrorType_e GYADSTB_ADStart(GYADU16 wPid)
{
	return GYADCore_Start(wPid);
}

GYADErrorType_e GYADSTB_PrivateDataGot(GYADU8 cReqID, GYADU8 bOK, GYADU16 wPid, const GYADU8 *pcReceiveData, GYADU32 dwLen)
{
	return GYADCore_PrivateDataGot(cReqID, bOK, wPid, pcReceiveData, dwLen);
}

GYADErrorType_e GYADSTB_GetAdInfo(sGYADRelativeInfo_t sAdRelInfo, sGYAdInfo_t *sAdInfo)
{
	return GYADCore_GetAdInfo(sAdRelInfo, sAdInfo);
}


