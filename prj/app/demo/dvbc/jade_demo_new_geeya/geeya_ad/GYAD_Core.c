/*****************************************************************************

File Name  : GYAD_Core.c

Description: geeya advertisement core.

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
#include  "GYAD_Core.h"

//#define CRC32_ARITHMETIC_CCITT			0x21	/* CRC32 CCITT order */
#define CRC32_POLYNOMIAL_CCITT		0x04c11db7


static GYADU16 wGYADPid = 0;
static GYADU8 cTotalResrcNum = 0;
static GYADU8 cTotalAdIndxNum = 0;
static GYResrcInfo_t *psResrcInfo = NULL;
static GYAdRelativeInfo_t sProgRelativInfo;

static GYADU32 u32StartAddr = 0; /*NVRAM起始地址*/
static GYADU32 u32NvramSize = 0; /*NVRAM大小*/
static GYADU8 cBootVer = 0xff; /*NVRAM中的开机广告的版本号*/
static GYADU8 cTotalResrcNvram = 0; /*NVRAM中的资源的数量*/
static GYResrcInfo_t *psNvramResrcInfo = NULL; /*NVRAM中的资源的信息*/
static GYAdDataNvramInfo_t *psDataNvramInfo = NULL; /*NVRAM中的资源的数据分布偏移地址*/
static GYAdRelativeInfo_t *psRelativNvramInfo = NULL; /*NVRAM中资源节目关联信息*/

static GYResrcInfo_t *psCurResrcInfo = NULL;
static GYADU8 u8CurResrc = 0;
static GYADU32 u32RcvLen = 0;	/*当前资源已接收数据的长度*/

static GYADU8 cFinishFlag = 0; /*广告接收完毕标识*/
static GYADU8 cParseData[256]; /*数据已接收的标志*/

static GYADU32 crc_table_gyadv[256];

static GYADErrorType_e GYADCore_StartRecvCurResrc(GYADVOID);
static GYADErrorType_e GYADCore_SetFilter(GYADU8 cReqID, GYADU16 wPid, GYADU16 wTablId, GYADU16 wResrcId);
static GYADErrorType_e GYADCore_FreeNvramResrcData(GYADVOID);
static GYADErrorType_e GYADCore_StartRecvCurResrc(GYADVOID);


GYADU32 crc_setup_fast_lut(GYADU32 *crc_lut)
{
	GYADU32 count, crc;
	GYADU32 i;
	for (count = 0; count <= 255; count++) 
	{
		crc = (count << 24);
		for (i = 0; i < 8; i++) {
			if (crc & 0x80000000)			/*Highest bit procedure*/
				crc = (crc << 1) ^ CRC32_POLYNOMIAL_CCITT;
			else
				crc <<= 1;
		}
		crc_lut[count] = crc & 0xffffffff;	/*Get lower 32 bits FCS*/
	}
}
GYADU32 crc_fast_calculate(GYADU32 *crc_lut, GYADU8 *buf, GYADU32 len)
{
	GYADU32 i;
	GYADU32 crc = 0xFFFFFFFF;
	
	for (i = 0; i < len; i++)
		crc = crc_lut[((crc >> 24) & 0xff) ^ buf[i]] ^ (crc << 8);
	
	return crc;
}


static GYResrcInfo_t* GYADCore_SearchResrc(GYADU8 u8ResrcId)
{
	GYADU8 i = 0;
	
	for(i = 0; i < cTotalResrcNum; i++)
	{
		if(psResrcInfo[i].wResrcId == u8ResrcId)
		{
			return &psResrcInfo[i];
		}
	}
	GYAD_Print(("%s, src_id[%d], is invalid\n",__FUNCTION__, u8ResrcId));
	return NULL;
}

static GYADErrorType_e GYADCore_ParseAdIndxInfo(const GYADU8* pcData, GYADU32 dwLen)
{
	GYADU32 dwTmpLen = 0;
	GYADU8 u8TotalNonPfAdIndx = 0;
	GYADU8 u8TotalPfAdIndx = 0;
	GYADU8 u8PerAdIndxNum = 0;
	GYADU8 u8CurTotalResrcNum = 0;
	GYADU8 i = 0, j = 0;
	GYADU8 u8AdIndx = 0;
	GYADU8 u8AdVer = 0;
	GYADU8 u8AdTpye = 0;
	GYADU8 u8ResrcId = 0;
	GYADU16 u16X0 = 0;
	GYADU16 u16Y0 = 0;
	GYADU16 u16X1 = 0;
	GYADU16 u16Y1 = 0;

	GYAD_Print(("GYADCore_ParseAdIndxInfo GYADCore_ParseAdIndxInfo dwLen = 0x%x\n", dwLen));
	cTotalAdIndxNum = pcData[8];
	cTotalResrcNum = pcData[9];
	GYAD_Print(("GYADCore_ParseAdIndxInfo cTotalResrcNum = %d, cTotalAdIndxNum = %d\n", cTotalResrcNum, cTotalAdIndxNum));
	
	if(NULL == psResrcInfo)
	{
		psResrcInfo = (GYResrcInfo_t*)GYADAPI_Malloc(cTotalResrcNum * sizeof(GYResrcInfo_t));
		if(NULL == psResrcInfo)
		{
			return GYAD_ERROR_FAIL;
		}
		GYADAPI_Memset(psResrcInfo, 0, cTotalResrcNum * sizeof(GYResrcInfo_t));
	}

	/*解析非P/F 广告位*/
	dwTmpLen += 10;
	u8TotalNonPfAdIndx = pcData[dwTmpLen];
	dwTmpLen += 1;
	for(i = 0; i < u8TotalNonPfAdIndx; i++)
	{
		u8AdIndx = pcData[dwTmpLen];
		dwTmpLen += 1;
		u8AdVer = pcData[dwTmpLen];
		dwTmpLen += 1;
		u8AdTpye = pcData[dwTmpLen];
		dwTmpLen += 1;
		u16X0 = USHORT_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1]);
		dwTmpLen += 2;
		u16Y0 = USHORT_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1]);
		dwTmpLen += 2;
		u16X1 = USHORT_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1]);
		dwTmpLen += 2;
		u16Y1 = USHORT_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1]);
		dwTmpLen += 2;
		u8PerAdIndxNum = pcData[dwTmpLen];
		dwTmpLen += 1;
		
		GYAD_Print(("GYADCore_ParseAdIndxInfo no P/F ADV.. i = %d\n", i));
		GYAD_Print(("GYADCore_ParseAdIndxInfo no P/F ADV.. u8AdIndx = %d\n", u8AdIndx));
		GYAD_Print(("GYADCore_ParseAdIndxInfo no P/F ADV.. u8AdVer = %d\n", u8AdVer));
		GYAD_Print(("GYADCore_ParseAdIndxInfo no P/F ADV.. u8AdTpye = %d\n", u8AdTpye));
		GYAD_Print(("GYADCore_ParseAdIndxInfo no P/F ADV.. u16X0 = %d\n", u16X0));
		GYAD_Print(("GYADCore_ParseAdIndxInfo no P/F ADV.. u16Y0 = %d\n", u16Y0));
		GYAD_Print(("GYADCore_ParseAdIndxInfo no P/F ADV.. u16X1 = %d\n", u16X1));
		GYAD_Print(("GYADCore_ParseAdIndxInfo no P/F ADV.. u16Y1 = %d\n", u16Y1));
		
		for(j = 0; j < u8PerAdIndxNum; j++)
		{
			u8ResrcId = pcData[dwTmpLen];
			dwTmpLen += 1;
			GYAD_Print(("GYADCore_ParseAdIndxInfo no P/F ADV.. j = %d, psResrcInfo[%d] = %d\n", j, u8CurTotalResrcNum, u8ResrcId));
			psResrcInfo[u8CurTotalResrcNum].cAdIndx = u8AdIndx;
			psResrcInfo[u8CurTotalResrcNum].cAdVer = u8AdVer;
			psResrcInfo[u8CurTotalResrcNum].cRelativeFlag = GYAD_RESRC_NOT_RELATIVE_SERVICE;
			psResrcInfo[u8CurTotalResrcNum].wResrcId = u8ResrcId;
			psResrcInfo[u8CurTotalResrcNum].sAdInfo.cAdType = u8AdTpye;
			psResrcInfo[u8CurTotalResrcNum].sAdInfo.wAdX0 = u16X0;
			psResrcInfo[u8CurTotalResrcNum].sAdInfo.wAdY0 = u16Y0;
			psResrcInfo[u8CurTotalResrcNum].sAdInfo.wAdX1 = u16X1;
			psResrcInfo[u8CurTotalResrcNum].sAdInfo.wAdY1 = u16Y1;
			u8CurTotalResrcNum++;
		}
	}
	
	/*解析P/F 广告位*/
	u8TotalPfAdIndx = pcData[dwTmpLen];
	dwTmpLen += 1;
	for(i = 0; i < u8TotalPfAdIndx; i++)
	{
		u8AdIndx = pcData[dwTmpLen];
		dwTmpLen += 1;
		u8AdVer = pcData[dwTmpLen];
		dwTmpLen += 1;
		u8AdTpye = pcData[dwTmpLen];
		dwTmpLen += 1;
		u16X0 = USHORT_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1]);
		dwTmpLen += 2;
		u16Y0 = USHORT_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1]);
		dwTmpLen += 2;
		u16X1 = USHORT_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1]);
		dwTmpLen += 2;
		u16Y1 = USHORT_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1]);
		dwTmpLen += 2;
		u8PerAdIndxNum = pcData[dwTmpLen];
		dwTmpLen += 1;
		
		GYAD_Print(("GYADCore_ParseAdIndxInfo P/F ADV.. i = %d\n", i));
		GYAD_Print(("GYADCore_ParseAdIndxInfo P/F ADV.. u8AdIndx = %d\n", u8AdIndx));
		GYAD_Print(("GYADCore_ParseAdIndxInfo P/F ADV.. u8AdVer = %d\n", u8AdVer));
		GYAD_Print(("GYADCore_ParseAdIndxInfo P/F ADV.. u8AdTpye = %d\n", u8AdTpye));
		GYAD_Print(("GYADCore_ParseAdIndxInfo P/F ADV.. u16X0 = %d\n", u16X0));
		GYAD_Print(("GYADCore_ParseAdIndxInfo P/F ADV.. u16Y0 = %d\n", u16Y0));
		GYAD_Print(("GYADCore_ParseAdIndxInfo P/F ADV.. u16X1 = %d\n", u16X1));
		GYAD_Print(("GYADCore_ParseAdIndxInfo P/F ADV.. u16Y1 = %d\n", u16Y1));
	
		for(j = 0; j < u8PerAdIndxNum; j++)
		{
			u8ResrcId = pcData[dwTmpLen];
			dwTmpLen += 1;
			GYAD_Print(("GYADCore_ParseAdIndxInfo P/F ADV.. j = %d, psResrcInfo[%d] = %d\n", j, u8CurTotalResrcNum, u8ResrcId));
			psResrcInfo[u8CurTotalResrcNum].cAdIndx = u8AdIndx;
			psResrcInfo[u8CurTotalResrcNum].cAdVer = u8AdVer;
			psResrcInfo[u8CurTotalResrcNum].cRelativeFlag = GYAD_RESRC_RELATIVE_SERVICE;
			psResrcInfo[u8CurTotalResrcNum].wResrcId = u8ResrcId;
			psResrcInfo[u8CurTotalResrcNum].sAdInfo.cAdType = u8AdTpye;
			psResrcInfo[u8CurTotalResrcNum].sAdInfo.wAdX0 = u16X0;
			psResrcInfo[u8CurTotalResrcNum].sAdInfo.wAdY0 = u16Y0;
			psResrcInfo[u8CurTotalResrcNum].sAdInfo.wAdX1 = u16X1;
			psResrcInfo[u8CurTotalResrcNum].sAdInfo.wAdY1 = u16Y1;
			u8CurTotalResrcNum++;
		}
	}

	return GYAD_ERROR_SUCCESS;
}

static GYADErrorType_e GYADCore_ParseResrcInfo(const GYADU8* pcData, GYADU32 dwLen)
{
	GYADU32 dwTmpLen = 0;
	GYADU8 u8TotaPicResrcNum = 0;
	GYADU8 u8TotaOsdResrcNum = 0;
	GYADU8 i = 0, j = 0;
	GYResrcInfo_t *sResrcInfo = NULL;
	GYADU8 u8ResrcId = 0;
	GYADU8 u8SaveFlag = 0;
	GYADU32 u32Resrclen = 0;
	GYADU32 u32DisTime = 0;
	GYADU32 u32StartTime = 0;
	GYADU32 u32EndTime = 0;
	GYADU32 u32TxtColor = 0;
	GYADU32 u32BgColor = 0;

	GYAD_Print(("GYADCore_ParseResrcInfo dwLen = 0x%x\n", dwLen));

	dwTmpLen += 10;
	u8TotaPicResrcNum = pcData[dwTmpLen];
	dwTmpLen += 1;
	for(i = 0; i < u8TotaPicResrcNum; i++)
	{
		u8ResrcId = pcData[dwTmpLen];
		dwTmpLen += 1;
		u8SaveFlag = pcData[dwTmpLen];
		dwTmpLen += 1;
		u32Resrclen = ULONG_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1], pcData[dwTmpLen+2], pcData[dwTmpLen+3]);
		dwTmpLen += 4;
		u32DisTime = ULONG_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1], pcData[dwTmpLen+2], pcData[dwTmpLen+3]);
		dwTmpLen += 4;
		u32StartTime = ULONG_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1], pcData[dwTmpLen+2], pcData[dwTmpLen+3]);
		dwTmpLen += 4;
		u32EndTime = ULONG_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1], pcData[dwTmpLen+2], pcData[dwTmpLen+3]);
		dwTmpLen += 4;
		
		GYAD_Print(("GYADCore_ParseResrcInfo PicResr .. i = %d\n", i));
		GYAD_Print(("GYADCore_ParseResrcInfo PicResr .. u8ResrcId = %d\n", u8ResrcId));
		GYAD_Print(("GYADCore_ParseResrcInfo PicResr .. u8SaveFlag = %d\n", u8SaveFlag));
		GYAD_Print(("GYADCore_ParseResrcInfo PicResr .. u32Resrclen = %d\n", u32Resrclen));
		GYAD_Print(("GYADCore_ParseResrcInfo PicResr .. u32DisTime = %d\n", u32DisTime));
		GYAD_Print(("GYADCore_ParseResrcInfo PicResr .. u32StartTime = %d\n", u32StartTime));
		GYAD_Print(("GYADCore_ParseResrcInfo PicResr .. u32EndTime = %d\n", u32EndTime));
		
		sResrcInfo = GYADCore_SearchResrc(u8ResrcId);
		if(NULL != sResrcInfo)
		{
			sResrcInfo->cSaveFlag = u8SaveFlag;
			sResrcInfo->sAdInfo.dwAdLen = u32Resrclen;
			sResrcInfo->sAdInfo.dwAdDisTime = u32DisTime;
			sResrcInfo->sAdInfo.dwAdStartTime = u32StartTime;
			sResrcInfo->sAdInfo.dwAdEndTime = u32EndTime;
		}
		else
		{
			GYAD_Print(("GYADCore_ParseResrcInfo PicResr -------u8ResrcId = %d, is invalid\n", u8ResrcId));
		}
	}
	
	u8TotaOsdResrcNum = pcData[dwTmpLen];
	dwTmpLen += 1;
	for(i = 0; i < u8TotaOsdResrcNum; i++)
	{
		u8ResrcId = pcData[dwTmpLen];
		dwTmpLen += 1;
		u8SaveFlag = pcData[dwTmpLen];
		dwTmpLen += 1;
		u32Resrclen = ULONG_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1], pcData[dwTmpLen+2], pcData[dwTmpLen+3]);
		dwTmpLen += 4;
		u32DisTime = ULONG_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1], pcData[dwTmpLen+2], pcData[dwTmpLen+3]);
		dwTmpLen += 4;
		u32StartTime = ULONG_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1], pcData[dwTmpLen+2], pcData[dwTmpLen+3]);
		dwTmpLen += 4;
		u32EndTime = ULONG_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1], pcData[dwTmpLen+2], pcData[dwTmpLen+3]);
		dwTmpLen += 4;
		u32TxtColor = ULONG_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1], pcData[dwTmpLen+2], pcData[dwTmpLen+3]);
		dwTmpLen += 4;
		u32BgColor = ULONG_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1], pcData[dwTmpLen+2], pcData[dwTmpLen+3]);
		dwTmpLen += 4;
		
		GYAD_Print(("GYADCore_ParseResrcInfo OSDResr .. i = %d\n", i));
		GYAD_Print(("GYADCore_ParseResrcInfo OSDResr .. u8ResrcId = %d\n", u8ResrcId));
		GYAD_Print(("GYADCore_ParseResrcInfo OSDResr .. u8SaveFlag = %d\n", u8SaveFlag));
		GYAD_Print(("GYADCore_ParseResrcInfo OSDResr .. u32Resrclen = %d\n", u32Resrclen));
		GYAD_Print(("GYADCore_ParseResrcInfo OSDResr .. u32DisTime = %d\n", u32DisTime));
		GYAD_Print(("GYADCore_ParseResrcInfo OSDResr .. u32StartTime = %d\n", u32StartTime));
		GYAD_Print(("GYADCore_ParseResrcInfo OSDResr .. u32EndTime = %d\n", u32EndTime));
		GYAD_Print(("GYADCore_ParseResrcInfo OSDResr .. u32TxtColor = %d\n", u32TxtColor));
		GYAD_Print(("GYADCore_ParseResrcInfo OSDResr .. u32BgColor = %d\n", u32BgColor));
		
		sResrcInfo = GYADCore_SearchResrc(u8ResrcId);
		if(NULL != sResrcInfo)
		{
			sResrcInfo->cSaveFlag = u8SaveFlag;
			sResrcInfo->sAdInfo.dwAdLen = u32Resrclen;
			sResrcInfo->sAdInfo.dwAdDisTime = u32DisTime;
			sResrcInfo->sAdInfo.dwAdStartTime = u32StartTime;
			sResrcInfo->sAdInfo.dwAdEndTime = u32EndTime;
			sResrcInfo->sAdInfo.dwTxtColor = u32TxtColor;
			sResrcInfo->sAdInfo.dwBgColor = u32BgColor;
		}
		else
		{
			GYAD_Print(("GYADCore_ParseResrcInfo OSDResr -------u8ResrcId = %d, is invalid\n", u8ResrcId));
		}
	}

	return GYAD_ERROR_SUCCESS;
}

static GYADErrorType_e GYADCore_ParseResrcRelativeInfo(const GYADU8* pcData, GYADU32 dwLen)
{
	GYADU32 dwTmpLen = 0;
	GYADU16 i = 0;
	GYADU16 u16TotalProgNum = 0;
	GYADU16 u16ServicId = 0;
	GYADU16 u16TsId = 0;
	GYADU8 u8ResrcNum = 0;
	GYADU8 j = 0;

	GYAD_Print(("GYADCore_ParseResrcRelativeInfo dwLen = 0x%x\n", dwLen));
	GYADAPI_Memset(&sProgRelativInfo, 0, sizeof(GYAdRelativeInfo_t));
	
	dwTmpLen += 10;
	u16TotalProgNum = USHORT_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1]);
	dwTmpLen += 2;
	GYAD_Print(("GYADCore_ParseResrcRelativeInfo u16TotalProgNum = %d\n", u16TotalProgNum));
	sProgRelativInfo.wSvcNum = u16TotalProgNum;

	if(NULL == sProgRelativInfo.psAdProgInfo)
	{
		sProgRelativInfo.psAdProgInfo = (GYAdProg_t*)GYADAPI_Malloc(sProgRelativInfo.wSvcNum * sizeof(GYAdProg_t));
		if(NULL == sProgRelativInfo.psAdProgInfo)
		{
			return GYAD_ERROR_FAIL;
		}
		GYADAPI_Memset(sProgRelativInfo.psAdProgInfo, 0, sProgRelativInfo.wSvcNum * sizeof(GYAdProg_t));
	}

	for(i = 0; i < u16TotalProgNum; i++)
	{
		u16ServicId = USHORT_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1]);;
		dwTmpLen += 2;
		u16TsId = USHORT_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1]);;
		dwTmpLen += 2;
		u8ResrcNum = pcData[dwTmpLen];
		dwTmpLen += 1;
		GYAD_Print(("GYADCore_ParseResrcRelativeInfo i = %d\n", i));
		GYAD_Print(("GYADCore_ParseResrcRelativeInfo u16ServicId = %d\n", u16ServicId));
		GYAD_Print(("GYADCore_ParseResrcRelativeInfo u16TsId = %d\n", u16TsId));
		GYAD_Print(("GYADCore_ParseResrcRelativeInfo u8ResrcNum = %d\n", u8ResrcNum));

		sProgRelativInfo.psAdProgInfo[i].wSvcId = u16ServicId;
		sProgRelativInfo.psAdProgInfo[i].wTsId = u16TsId;
		sProgRelativInfo.psAdProgInfo[i].cResrcNum = u8ResrcNum;
		if(NULL == sProgRelativInfo.psAdProgInfo[i].pwResrcId)
		{
			sProgRelativInfo.psAdProgInfo[i].pwResrcId = (GYADU16*)GYADAPI_Malloc(sProgRelativInfo.psAdProgInfo[i].cResrcNum * 2);
			if(NULL == sProgRelativInfo.psAdProgInfo[i].pwResrcId)
			{
				return GYAD_ERROR_FAIL;
			}
			GYADAPI_Memset(sProgRelativInfo.psAdProgInfo[i].pwResrcId, 0, sProgRelativInfo.psAdProgInfo[i].cResrcNum * 2);
		}
		for(j = 0; j < sProgRelativInfo.psAdProgInfo[i].cResrcNum; j++)
		{
			sProgRelativInfo.psAdProgInfo[i].pwResrcId[j] = pcData[dwTmpLen];
			dwTmpLen += 1;
			GYAD_Print(("GYADCore_ParseResrcRelativeInfo sProgRelativInfo.psAdProgInfo[%d].pwResrcId[%d] = %d\n", i, j, sProgRelativInfo.psAdProgInfo[i].pwResrcId[j]));
		}
	}

	return GYAD_ERROR_SUCCESS;
}

static GYADErrorType_e GYADCore_ParseRelativeInfo(GYADU8 cReqID, const GYADU8* pcData, GYADU32 dwLen)
{
	GYADU32 dwRtn = GYAD_ERROR_SUCCESS;
	static GYADU8 cParseAdIndxDes = 0;
	static GYADU8 cParseResrcDes = 0;

	switch(pcData[6])
	{
		case GYAD_AD_INDX_DES_SECTION:
			if(cParseAdIndxDes == 0)
			{
				dwRtn = GYADCore_ParseAdIndxInfo(pcData, dwLen);
				cParseAdIndxDes = 1;
			}
			break;
		case GYAD_RESRC_DES_SECTION:
			if((cParseAdIndxDes == 1)&&(cParseResrcDes == 0))
			{
				dwRtn = GYADCore_ParseResrcInfo(pcData, dwLen);
				cParseResrcDes = 1;
			}
			break;
		case GYAD_RESRC_INFO_SECTION:
			if((cParseAdIndxDes == 1)&&(cParseResrcDes == 1))
			{
				dwRtn = GYADCore_ParseResrcRelativeInfo(pcData, dwLen);
				GYADAPI_RequestFree(cReqID);
				dwRtn = GYADCore_StartRecvCurResrc();
			}
			break;
		default:
			GYAD_Print(("GYADCore_ParseRelativeInfo UNKNOWN Section\n"));
			dwRtn = GYAD_ERROR_UNKNOWN;
			break;
	}
	
	return dwRtn;
}

static GYADU8 crc_err_cnt = 0;

static GYADErrorType_e GYADCore_ParseResrcData(GYADU8 cReqID, const GYADU8* pcData, GYADU32 dwLen)
{
	GYADU32 dwRtn = GYAD_ERROR_SUCCESS;
	GYADU32 dwTmpLen = 0;
	GYADU32 u32DataLen = 0;
	GYADU8 u8TabId;
	GYADU8	u8SecCur = 0;
	GYADU8 u8SumSecNum = 0;
	GYADU8* tempData = pcData;
	GYADU32 crc = 0, crc_table;
	
	u8TabId = pcData[0];
	if(u8TabId != GYAD_DATA_TABLE_ID)
	{
		return GYAD_ERROR_PARA;
	}
	
	crc_table = (((GYADU32)tempData[dwLen-4])<<24) | (((GYADU32)tempData[dwLen-3])<<16) | (((GYADU32)tempData[dwLen-2])<<8) | ((GYADU32)tempData[dwLen-1]);
	crc = crc_fast_calculate(crc_table_gyadv, tempData, dwLen-4);
	if(crc_table != crc)
	{	
		GYAD_Print(("\n\n\n\n===================================================== sec_num = 0x%x, crc_table = 0x%x, crc = 0x%x\n\n\n\n",tempData[6],crc_table,crc));
		crc_err_cnt++;
		if(crc_err_cnt < 3)
		{
			GYAD_Print(("\n===================================================== data crc err, so try again\n"));
			GYADAPI_RequestFree(cReqID);
			u32RcvLen = 0;
			GYSTBAD_Memset(cParseData, 0, sizeof(cParseData));
			dwRtn = GYADCore_StartRecvCurResrc();
			return dwRtn;
		}
		else
		{
			if(psCurResrcInfo->cAdIndx == GYAD_BOOT_LOGO_INDX)
			{
				GYAD_Print(("\n===================================================== Three consecutive receiving LOGO data failed, so no receive err LOGO data\n"));
				psCurResrcInfo->cAdVer = cBootVer;
				GYADAPI_RequestFree(cReqID);
				u8CurResrc++;
				u32RcvLen = 0;
				GYSTBAD_Memset(cParseData, 0, sizeof(cParseData));
				crc_err_cnt=0;
				dwRtn = GYADCore_StartRecvCurResrc();
				return dwRtn;
			}
			GYAD_Print(("\n===================================================== banner data crc err, but continue receive\n"));
		}
	}
	
	dwTmpLen = 6;
	u8SecCur = pcData[dwTmpLen];
	dwTmpLen += 1;
	u8SumSecNum = pcData[dwTmpLen];
	dwTmpLen += 1;
	if(0 != cParseData[u8SecCur])
	{
		return GYAD_ERROR_SUCCESS;
	}
	cParseData[u8SecCur] = 1;
	
	u32DataLen = ULONG_LENGTH(pcData[dwTmpLen], pcData[dwTmpLen+1], pcData[dwTmpLen+2], pcData[dwTmpLen+3]);
	dwTmpLen += 4;
	GYADAPI_Memcpy((GYADU8*)(psCurResrcInfo->sAdInfo.cAdData + u8SecCur*GYAD_RESRC_PER_SEC_DATA_LEN), (GYADU8*)&pcData[dwTmpLen], u32DataLen);
	u32RcvLen += u32DataLen;
	GYAD_Print(("GYADCore_ParseResrcData srcID=%d, u32DataLen=%d\n", psCurResrcInfo->wResrcId,u32DataLen));
	GYAD_Print(("GYADCore_ParseResrcData srcID=%d, u32RcvLen=%d\n", psCurResrcInfo->wResrcId,u32RcvLen));
	GYAD_Print(("GYADCore_ParseResrcData srcID=%d, u32ResrcLen=%d\n", psCurResrcInfo->wResrcId,psCurResrcInfo->sAdInfo.dwAdLen));

	
	if(u32RcvLen != psCurResrcInfo->sAdInfo.dwAdLen)
	{
		GYAD_Print(("GYADCore_ParseResrcData, rsc_num = 0x%02x, section_num = 0x%02x, return GYAD_ERROR_SUCCESS\n", u8CurResrc,u8SecCur));
		return GYAD_ERROR_SUCCESS;
	}
	else
		GYAD_Print(("GYADCore_ParseResrcData, rsc_num = 0x%02x, last section_num = 0x%02x, receive finish, ready receive next adv data\n", u8CurResrc,u8SecCur));

	GYADAPI_RequestFree(cReqID);
	u8CurResrc++;
	u32RcvLen = 0;
	GYSTBAD_Memset(cParseData, 0, sizeof(cParseData));
	crc_err_cnt=0;
	dwRtn = GYADCore_StartRecvCurResrc();
	return dwRtn;
}

static GYADErrorType_e GYADCore_SetFilter(GYADU8 cReqID, GYADU16 wPid, GYADU16 wTablId, GYADU16 wResrcId)
{
	GYADU32 dwRtn = GYAD_ERROR_SUCCESS;
	GYADU8 Filter[8];
	GYADU8 Mask[8];
	libc_printf("cReqID=%d, wPid=0x%x, wTablId=0x%x, wResrcId=0x%x\n", cReqID, wPid, wTablId, wResrcId);
	//GYAD_Print(("GYADCore_SetFilter Enter\n"));
	//libc_printf("cReqID=%d, wPid=0x%x, wTablId=0x%x, wResrcId=0x%x\n", cReqID, wPid, wTablId, wResrcId);
	//GYAD_Print(("cReqID=%d, wPid=0x%x, wTablId=0x%x, wResrcId=0x%x\n", cReqID, wPid, wTablId, wResrcId));
	
	GYADAPI_Memset(Filter, 0, 8);
	GYADAPI_Memset(Mask, 0, 8);

	switch(wTablId)
	{
		case GYAD_RELATE_TABLE_ID:
			Filter[0] = GYAD_RELATE_TABLE_ID;
			Mask[0] = 0xff;
			break;
		case GYAD_DATA_TABLE_ID:
			Filter[0] = GYAD_DATA_TABLE_ID;
			Mask[0] = 0xff;
			Filter[1] = (GYADU8)((wResrcId>>8)&0x00ff);
			Mask[1] = 0xff;
			Filter[2] = (GYADU8)(wResrcId&0x00ff);
			Mask[2] = 0xff;
			break;
		default:
			return GYAD_ERROR_UNKNOWN;
	}

	dwRtn = GYADAPI_RequestPrivateData(cReqID, Filter, Mask, 8, wPid, GYAD_GET_DATA_TIMEOUT);
	GYAD_Print(("GYADCore_SetFilter Eixt\n"));
	
	return dwRtn;
}

static GYADVOID GYADCore_GetBootAdNum(GYADU8 *u8Num)
{
	GYADU8 i = 0;
	GYADU8 u8BootAdNum = 0;
	
	for(i = 0; i < cTotalResrcNum; i++)
	{
		if(psResrcInfo[i].cAdIndx == GYAD_BOOT_LOGO_INDX)
		{
			u8BootAdNum++;
		}
	}

	*u8Num = u8BootAdNum;
}

static GYADVOID GYADCore_SaveBootAd(GYADVOID)
{
	GYADU8 i = 0;
	GYADU8 j = 0;
	GYADU8 u8BootAdVer = 0;
	GYADU8 u8BootAdNum = 0;
	sGYAdInfo_t *sbootADInfo = NULL;

	GYADCore_GetBootAdNum(&u8BootAdNum);
	if(u8BootAdNum == 0)
		return ;
	if(NULL == sbootADInfo)
	{
		sbootADInfo = (sGYAdInfo_t*)GYADAPI_Malloc(u8BootAdNum*sizeof(sGYAdInfo_t));
		if(NULL == sbootADInfo)
		{
			sbootADInfo = NULL;
			return;
		}
		GYADAPI_Memset(sbootADInfo, 0, u8BootAdNum*sizeof(sGYAdInfo_t));
	}
	for(i = 0; i < cTotalResrcNum; i++)
	{
		if(psResrcInfo[i].cAdIndx == GYAD_BOOT_LOGO_INDX)
		{
			u8BootAdVer = psResrcInfo[i].cAdVer;
			if(u8BootAdVer != cBootVer)
			{
				GYADAPI_Memcpy(&sbootADInfo[j], &psResrcInfo[i].sAdInfo, sizeof(sGYAdInfo_t));
				//u8BootAdVer = psResrcInfo[i].cAdVer;
				j++;
			}
			else 
				u8BootAdNum--;
		}
	}

	if(u8BootAdVer != cBootVer)
	{
		GYADAPI_SaveBootAd(sbootADInfo, u8BootAdNum, u8BootAdVer);
	}

	GYADAPI_Free(sbootADInfo);
}

static GYADU8 GYADCore_GetNvramResrcVer(GYADU16 u16ResrcId)
{
	GYADU8 i = 0;
	GYADU8 j = 0;
	GYADU32 u32Len = 0;
	GYADU32 u32Offset = 0;

	for(i = 0; i < cTotalResrcNvram; i++)
	{
		if(psNvramResrcInfo[i].wResrcId == u16ResrcId)
		{
			return psNvramResrcInfo[i].cAdVer;
		}
	}
	
	return GYAD_ERROR_SUCCESS;
}

static GYADVOID GYADCore_NeedWriteResrc(GYADU8 *num, GYADU32 *len)
{
	GYADU8 i = 0;
	GYADU8 u8Num = 0;
	GYADU8 u8NeedUpdateNum = 0;
	GYADU32 u32Len = GYAD_NVRAM_HEAD_INFO_SIZE;
	
	for(i = 0; i < cTotalResrcNum; i++)
	{
//		if((psResrcInfo[i].cSaveFlag == GYAD_NEED_SAVE_TO_NVRAM) && (psResrcInfo[i].cAdIndx != GYAD_BOOT_LOGO_INDX))  //save one
		if(psResrcInfo[i].cAdIndx != GYAD_BOOT_LOGO_INDX)  // save all
		{
			u8Num++;
			u32Len += sizeof(GYAdDataNvramInfo_t) + sizeof(GYResrcInfo_t) + psResrcInfo[i].sAdInfo.dwAdLen;

			/*和NVRAM中保存的资源的版本是否一样*/
			if(psResrcInfo[i].cAdVer != GYADCore_GetNvramResrcVer(psResrcInfo[i].wResrcId))
			{
				u8NeedUpdateNum++;
			}
		}
	}

	if(u8NeedUpdateNum == 0)/*和NVRAM中保存的数据相比无更新*/
	{
		*num = 0;
		*len = 0;
	}
	else
	{
		u32Len += u8Num*sizeof(GYAdDataNvramInfo_t);
		*num = u8Num;
		*len = u32Len;
	}
}

static GYADVOID GYADCore_WriteNvram(GYADVOID)
{
	GYADU8 i = 0;
	GYADU8 j = 0;
	GYADU8 *data = NULL;
	GYADU8 u8Num = 0;
	GYADU32 u32Len = 0;
	GYADU32 u32DataNvramInfoLen = 0;
	GYADU32 u32TmpLen = 0;
	GYAdDataNvramInfo_t *psDataNvramInfo = NULL;

	GYADCore_NeedWriteResrc(&u8Num, &u32Len);
	//GYADAPI_EraseNvram(u32StartAddr, u32NvramSize);

	/*NVRAM太小或者无资源需要保存*/
	if((u32Len > u32NvramSize) ||(u8Num == 0))
	{
		u8Num = 0;
		if(u32Len > u32NvramSize)
			GYAD_Print(("\n\nGYADCore_WriteNvram ADV_banner_len(0x%x) > Flash_Max_len(0x%x), so ADV_banner don't save \n\n",u32Len, u32NvramSize));
		//GYSTBAD_WriteNvram(u32StartAddr, &u8Num, GYAD_NVRAM_HEAD_INFO_SIZE);
		return;
	}
	
#if 1
    u32Len += 2; //save sProgRelativInfo.wSvcNum
	u32Len += sProgRelativInfo.wSvcNum*sizeof(GYAdProg_t);
	GYADU32 k;
	for(k=0; k<sProgRelativInfo.wSvcNum; k++)
	{
		u32Len += sProgRelativInfo.psAdProgInfo[k].cResrcNum*2; 
	}
#endif

	

	if(NULL == data)
	{
		data = (GYADU8*)GYADAPI_Malloc(u32Len);
		if(NULL == data)
		{
			u8Num = 0;
			GYSTBAD_WriteNvram(u32StartAddr, &u8Num, GYAD_NVRAM_HEAD_INFO_SIZE);
			return;
		}
	}

	if(NULL == psDataNvramInfo)
	{
		u32DataNvramInfoLen = u8Num*sizeof(GYAdDataNvramInfo_t);
		psDataNvramInfo = (GYAdDataNvramInfo_t*)GYADAPI_Malloc(u32DataNvramInfoLen);
		if(NULL == psDataNvramInfo)
		{
			u8Num = 0;
			GYSTBAD_WriteNvram(u32StartAddr, &u8Num, GYAD_NVRAM_HEAD_INFO_SIZE);
			return;
		}
	}

	u32TmpLen = GYAD_NVRAM_HEAD_INFO_SIZE + u32DataNvramInfoLen;
	for(i = 0; i < cTotalResrcNum; i++)
	{
	//	if((psResrcInfo[i].cSaveFlag == GYAD_NEED_SAVE_TO_NVRAM) && (psResrcInfo[i].cAdIndx != GYAD_BOOT_LOGO_INDX))
		if(psResrcInfo[i].cAdIndx != GYAD_BOOT_LOGO_INDX)  // save all
		{
			GYADAPI_Memcpy(&data[u32TmpLen], &psResrcInfo[i], sizeof(GYResrcInfo_t));
			u32TmpLen += sizeof(GYResrcInfo_t);
		}
	}
	
#if 1			
		GYADU32 sProgsrcIdLen=0;
		data[u32TmpLen] = (GYADU8)(sProgRelativInfo.wSvcNum & 0x00ff);
		u32TmpLen+= 1;
		data[u32TmpLen] = (GYADU8)((sProgRelativInfo.wSvcNum>>8) & 0x00ff);
		u32TmpLen+= 1;

		GYADAPI_Memcpy(&data[u32TmpLen], &sProgRelativInfo.psAdProgInfo[0], sProgRelativInfo.wSvcNum*sizeof(GYAdProg_t));
		u32TmpLen+= sProgRelativInfo.wSvcNum*sizeof(GYAdProg_t);

		for(k=0; k<sProgRelativInfo.wSvcNum; k++)
		{
			sProgsrcIdLen = sProgRelativInfo.psAdProgInfo[k].cResrcNum*2; 
			GYADAPI_Memcpy(&data[u32TmpLen], &sProgRelativInfo.psAdProgInfo[k].pwResrcId[0], sProgsrcIdLen);
			u32TmpLen += sProgsrcIdLen;
			
		}

#endif

	
	
	for(i = 0; i < cTotalResrcNum; i++)
	{
	//	if((psResrcInfo[i].cSaveFlag == GYAD_NEED_SAVE_TO_NVRAM) && (psResrcInfo[i].cAdIndx != GYAD_BOOT_LOGO_INDX))
		if(psResrcInfo[i].cAdIndx != GYAD_BOOT_LOGO_INDX)  // save all
		{
			psDataNvramInfo[j].wResrcId = psResrcInfo[i].wResrcId;
			psDataNvramInfo[j].dwOffset = u32TmpLen;
			GYADAPI_Memcpy(&data[u32TmpLen], psResrcInfo[i].sAdInfo.cAdData, psResrcInfo[i].sAdInfo.dwAdLen);
			u32TmpLen += psResrcInfo[i].sAdInfo.dwAdLen;
			j++;
		}
	}

	GYADAPI_Memcpy(data, &u8Num, GYAD_NVRAM_HEAD_INFO_SIZE);
	GYADAPI_Memcpy(&data[GYAD_NVRAM_HEAD_INFO_SIZE], psDataNvramInfo, u32DataNvramInfoLen);
	GYADAPI_WriteNvram(u32StartAddr, data, u32Len);
}

static GYADErrorType_e GYADCore_StartRecvCurResrc(GYADVOID)
{
	GYADU8 i = 0;
	GYADU8 u8AdVer = 0;
	GYADU8 bBootAd = 0;
	
ReCheck:
	if(u8CurResrc == cTotalResrcNum)/*完成接收，保存数据后退出*/
	{
		//GYADAPI_RequestFree(0);
		//GYADCore_FreeNvramResrcData();
#if 1		
		/*保存数据*/
		GYADCore_SaveBootAd();
		GYADCore_WriteNvram();
		GYADCore_FreeNvramResrcData();
#endif		
		cFinishFlag = 1;
		GYADAPI_Exit();
	}

	if(u8CurResrc < cTotalResrcNum)
	{
		if(NULL == psResrcInfo[u8CurResrc].sAdInfo.cAdData)
		{
			if(0 == psResrcInfo[u8CurResrc].sAdInfo.dwAdLen)
			{
				libc_printf("%s, adv data err, u8CurResrc = %d, len == 0, so stop receive\n",__FUNCTION__, u8CurResrc);
				return GYAD_ERROR_FAIL;
			}
			psResrcInfo[u8CurResrc].sAdInfo.cAdData = (GYADU8*)GYADAPI_Malloc(psResrcInfo[u8CurResrc].sAdInfo.dwAdLen);
			if(NULL == psResrcInfo[u8CurResrc].sAdInfo.cAdData)
			{
				libc_printf("%s, adv mem err, u8CurResrc = %d, so stop receive\n",__FUNCTION__, u8CurResrc);
				return GYAD_ERROR_FAIL;
			}
			GYADAPI_Memset(psResrcInfo[u8CurResrc].sAdInfo.cAdData, 0, psResrcInfo[u8CurResrc].sAdInfo.dwAdLen);
		}
		
		psCurResrcInfo = &psResrcInfo[u8CurResrc];


		bBootAd = 0;
		u8AdVer = 0;
		if(psCurResrcInfo->cAdIndx == GYAD_BOOT_LOGO_INDX)
		{
			u8AdVer = cBootVer;
			bBootAd = 1;
		}
		else

	//	if(psCurResrcInfo->cSaveFlag == GYAD_NEED_SAVE_TO_NVRAM)
		
		{
			for(i = 0; i < cTotalResrcNvram; i++)
			{
				bBootAd = 0;
				u8AdVer = 0;
				//if(psCurResrcInfo->cAdIndx == GYAD_BOOT_LOGO_INDX)
				//{
				//	u8AdVer = cBootVer;
				//	bBootAd = 1;
				//	break;
				//}
				//else
				{
					if(psCurResrcInfo->wResrcId == psNvramResrcInfo[i].wResrcId)
					{
						u8AdVer = psNvramResrcInfo[i].cAdVer;
						break;
					}
				}
			#if 0
				if(psCurResrcInfo->cAdVer == u8AdVer)
				{
					if(0 == bBootAd)
					{
						GYADAPI_Memcpy(psCurResrcInfo->sAdInfo.cAdData, psNvramResrcInfo[i].sAdInfo.cAdData, psCurResrcInfo->sAdInfo.dwAdLen);
					}
					bBootAd = 0;
					u8AdVer = 0;
					u8CurResrc++;
					goto ReCheck;
				}
			#endif
			
			}
		}
		if(psCurResrcInfo->cAdVer == u8AdVer)
		{
			if(0 == bBootAd)
			{
				GYADAPI_Memcpy(psCurResrcInfo->sAdInfo.cAdData, psNvramResrcInfo[i].sAdInfo.cAdData, psCurResrcInfo->sAdInfo.dwAdLen);
			}
			bBootAd = 0;
			u8AdVer = 0;
			u8CurResrc++;
			goto ReCheck;
		}
		
		if(psCurResrcInfo->cAdIndx == GYAD_BOOT_LOGO_INDX)
			GYAD_Print(("GYADCore_StartRecvCurResrc  Request LOGO PrivateData psCurResrcInfo->cAdVer = %d, old_adver = %d,u8AdVer = %d \n",psCurResrcInfo->cAdVer, cBootVer,u8AdVer));
		else 
			GYAD_Print(("GYADCore_StartRecvCurResrc  Request rsc PrivateData srcID: %d ..............\n",psCurResrcInfo->wResrcId));

		if(GYAD_ERROR_SUCCESS != GYADCore_SetFilter(0, wGYADPid, GYAD_DATA_TABLE_ID, psCurResrcInfo->wResrcId))
		{
			GYAD_Print(("%s, adv set filter fail, so stop recevie\n",__FUNCTION__));
			return GYAD_ERROR_UNKNOWN;
		}
		return GYAD_ERROR_SUCCESS;
	}
	return GYAD_ERROR_UNKNOWN;
}

/*释放从NVRAM中读取数据时所占用的内存*/
static GYADErrorType_e GYADCore_FreeNvramResrcData(GYADVOID)
{
	GYADU8 i = 0;

	if(NULL != psDataNvramInfo)
	{
		GYADAPI_Free(psDataNvramInfo);
		psDataNvramInfo = NULL;
	}
	
#if 1
    if(NULL != psRelativNvramInfo)
	{
		if(psRelativNvramInfo->psAdProgInfo	!= NULL)
		{
			for(i = 0; i<psRelativNvramInfo->wSvcNum; i++)
			{
				if(psRelativNvramInfo->psAdProgInfo->pwResrcId != NULL)
				{
					GYADAPI_Free(psRelativNvramInfo[i].psAdProgInfo->pwResrcId);
					psRelativNvramInfo[i].psAdProgInfo->pwResrcId = NULL;
				}
			}
			GYADAPI_Free(psRelativNvramInfo->psAdProgInfo);
		}
		GYADAPI_Free(psRelativNvramInfo);
		psRelativNvramInfo = NULL;
	}
#endif

	for(i = 0; i < cTotalResrcNvram; i++)
	{
	    if(psNvramResrcInfo[i].sAdInfo.cAdData != NULL)
			GYADAPI_Free(psNvramResrcInfo[i].sAdInfo.cAdData);
		psNvramResrcInfo[i].sAdInfo.cAdData = NULL;
	}

	if(NULL != psNvramResrcInfo)
	{
		GYADAPI_Free(psNvramResrcInfo);
		psNvramResrcInfo = NULL;
	}
}

/*读取NVRAM中的广告数据*/
static GYADErrorType_e GYADCore_GetNvramResrcData(GYADVOID)
{
	GYADU8 i = 0;
	GYADU8 j = 0;
	GYADU32 u32Len = 0;
	GYADU32 u32Offset = 0;

	for(i = 0; i < cTotalResrcNvram; i++)
	{
		u32Len = psNvramResrcInfo[i].sAdInfo.dwAdLen;
		psNvramResrcInfo[i].sAdInfo.cAdData = (GYADU8*)GYADAPI_Malloc(u32Len);
		GYADAPI_Memset(psNvramResrcInfo[i].sAdInfo.cAdData, 0, u32Len);
		for(j = 0; j < cTotalResrcNvram; j++)
		{
			if(psNvramResrcInfo[i].wResrcId == psDataNvramInfo[j].wResrcId)
			{
				u32Offset = psDataNvramInfo[j].dwOffset;
				break;
			}
		}
		GYADAPI_ReadNvram(u32StartAddr+u32Offset, psNvramResrcInfo[i].sAdInfo.cAdData, u32Len);
	}
	
	return GYAD_ERROR_SUCCESS;
}

/*读取NVRAM中的广告信息*/
static GYADErrorType_e GYADCore_GetNvramResrcInfo(GYADVOID)
{
	GYADU32 u32DataInfoLen = 0;
	GYADU32 u32ResrcInfoLen = 0;
	GYADU32 u32Offset = 0;
	
	GYADAPI_GetNvram(&u32StartAddr, &u32NvramSize);
	GYADAPI_GetBootVer(&cBootVer);
	GYADAPI_ReadNvram(u32StartAddr, &cTotalResrcNvram, GYAD_NVRAM_HEAD_INFO_SIZE);
	if(0xff == cTotalResrcNvram)
	{
		cTotalResrcNvram = 0;
	}
	if(0 != cTotalResrcNvram)
	{
		if(NULL == psDataNvramInfo)
		{
			u32DataInfoLen = cTotalResrcNvram * sizeof(GYAdDataNvramInfo_t);
			psDataNvramInfo = (GYAdDataNvramInfo_t*)GYADAPI_Malloc(u32DataInfoLen);
			if(NULL == psDataNvramInfo)
			{
				return GYAD_ERROR_FAIL;
			}
			GYADAPI_Memset(psDataNvramInfo, 0, u32DataInfoLen);
			u32Offset = GYAD_NVRAM_HEAD_INFO_SIZE;
			GYADAPI_ReadNvram(u32StartAddr+u32Offset, (GYADU8*)psDataNvramInfo, u32DataInfoLen);
		}
		
		if(NULL == psNvramResrcInfo)
		{
			u32ResrcInfoLen = cTotalResrcNvram * sizeof(GYResrcInfo_t);
			psNvramResrcInfo = (GYResrcInfo_t*)GYADAPI_Malloc(u32ResrcInfoLen);
			if(NULL == psNvramResrcInfo)
			{
				return GYAD_ERROR_FAIL;
			}
			GYADAPI_Memset(psNvramResrcInfo, 0xff, u32ResrcInfoLen);
			u32Offset = GYAD_NVRAM_HEAD_INFO_SIZE + u32DataInfoLen;
			GYADAPI_ReadNvram(u32StartAddr+u32Offset, (GYADU8*)psNvramResrcInfo, u32ResrcInfoLen);
		}

#if 1
		GYADU32  RelativInfoLen = 0, Relativ_len=0;
        GYADU16  PelativProgNum = 0,cnt=0;
		GYADU8   temp=0;
		
		u32Offset+= u32ResrcInfoLen;
		GYADAPI_ReadNvram(u32StartAddr+u32Offset, &temp, 1);
		PelativProgNum = (GYADU16)temp;
		GYADAPI_ReadNvram(u32StartAddr+u32Offset+1, &temp, 1);
		PelativProgNum |= ((GYADU16)temp)<<8;
		u32Offset += 2;
		if(PelativProgNum == 0)
			return GYAD_ERROR_SUCCESS;
		
		if(psRelativNvramInfo == NULL)
		{
			psRelativNvramInfo = (GYAdRelativeInfo_t *)GYADAPI_Malloc(sizeof(GYAdRelativeInfo_t));
			if(NULL == psRelativNvramInfo)
			{
				return GYAD_ERROR_FAIL;
			}
			psRelativNvramInfo->wSvcNum = PelativProgNum;
			RelativInfoLen = sizeof(GYAdProg_t)*PelativProgNum;
			psRelativNvramInfo->psAdProgInfo = (GYAdProg_t*)GYADAPI_Malloc(RelativInfoLen);
			if(NULL == psRelativNvramInfo->psAdProgInfo)
			{
				return GYAD_ERROR_FAIL;
			}
			GYADAPI_Memset(psRelativNvramInfo->psAdProgInfo, 0x00, RelativInfoLen);
			u32Offset = GYAD_NVRAM_HEAD_INFO_SIZE + u32DataInfoLen + u32ResrcInfoLen + 2;
			GYADAPI_ReadNvram(u32StartAddr+u32Offset, (GYADU8*)psRelativNvramInfo->psAdProgInfo, RelativInfoLen);

			u32Offset = GYAD_NVRAM_HEAD_INFO_SIZE + u32DataInfoLen + u32ResrcInfoLen + RelativInfoLen + 2;
			
			for(cnt=0; cnt<psRelativNvramInfo->wSvcNum; cnt++)
			{
				if(psRelativNvramInfo->psAdProgInfo[cnt].cResrcNum!=0)
				{
					Relativ_len = 2*psRelativNvramInfo->psAdProgInfo[cnt].cResrcNum;
					psRelativNvramInfo->psAdProgInfo[cnt].pwResrcId = (GYADU16*)GYADAPI_Malloc(Relativ_len);

					if(psRelativNvramInfo->psAdProgInfo[cnt].pwResrcId == NULL)
						return GYAD_ERROR_FAIL;
					GYADAPI_Memset(&psRelativNvramInfo->psAdProgInfo[cnt].pwResrcId[0], 0x00, Relativ_len);
                    GYADAPI_ReadNvram(u32StartAddr+u32Offset, (GYADU8*)&psRelativNvramInfo->psAdProgInfo[cnt].pwResrcId[0], Relativ_len);
					u32Offset += Relativ_len;
				}
					
			}
		}						
#endif
	}

	return GYAD_ERROR_SUCCESS;
}

GYADErrorType_e GYADCore_Start(GYADU16 wPid)
{
	static GYADU8 cEnterFlag = 0;
	
	GYAD_Print(("GYAD_Start Enter\n"));

	if(0 != cEnterFlag)
	{
		return GYAD_ERROR_UNKNOWN;
	}
	cEnterFlag = 1;
	GYSTBAD_Memset(crc_table_gyadv, 0, sizeof(crc_table_gyadv));
	crc_setup_fast_lut(crc_table_gyadv);

	/*读取NVRAM中的广告*/
	GYADCore_GetNvramResrcInfo();
	GYADCore_GetNvramResrcData();
	
	wGYADPid = wPid;
	GYSTBAD_Memset(cParseData, 0, sizeof(cParseData));
	
	if(GYAD_ERROR_SUCCESS != GYADAPI_SetTS())
	{
		
		GYADAPI_Exit();
		GYAD_Print(("GYADCore_Start: not lock ts\n"));
		return GYAD_ERROR_UNLOCK;
	}
	
	if(GYAD_ERROR_SUCCESS !=GYADCore_SetFilter(0, wGYADPid, GYAD_RELATE_TABLE_ID, 0))
	{
		GYAD_Print(("GYAD_Start setfilter fail\n"));
		return GYAD_ERROR_UNKNOWN;
	}
	
	GYAD_Print(("GYAD_Start Eixt\n"));
	
	return GYAD_ERROR_SUCCESS;
}

GYADErrorType_e GYADCore_PrivateDataGot(GYADU8 cReqID, GYADU8 bOK, GYADU16 wPid, const GYADU8 *pcReceiveData, GYADU32 dwLen)
{
	GYADU32 dwRtn = GYAD_ERROR_SUCCESS;

	GYADAPI_SemaphoreLock();

	if((NULL == pcReceiveData) || (0 == dwLen))
	{
		GYADAPI_RequestFree(cReqID);
		GYADAPI_SemaphoreUnlock();
		GYADAPI_Exit();
		return GYAD_ERROR_UNKNOWN;
	}

	GYAD_Print(("\n\nGYAD_PrivateDataGot Receive Data\n"));
	/*获取数据失败*/
	if(0 == bOK)
	{
		GYAD_Print(("GYAD_PrivateDataGot bOK Failed\n"));
		GYADAPI_RequestFree(cReqID);
		GYADAPI_SemaphoreUnlock();
		GYADAPI_Exit();
		return GYAD_ERROR_UNKNOWN;
	}

	if(wPid != wGYADPid)
	{
		GYAD_Print(("GYAD_PrivateDataGot wGYADPid Failed\n"));
		GYADAPI_RequestFree(cReqID);
		GYADAPI_SemaphoreUnlock();
		GYADAPI_Exit();
		return GYAD_ERROR_UNKNOWN;
	}

	switch(pcReceiveData[0])
	{
		case GYAD_RELATE_TABLE_ID:
			dwRtn = GYADCore_ParseRelativeInfo(cReqID, pcReceiveData, dwLen);
			break;
		case GYAD_DATA_TABLE_ID:
			dwRtn = GYADCore_ParseResrcData(cReqID, pcReceiveData, dwLen);
			break;
		default:
			GYAD_Print(("GYAD_PrivateDataGot UNKNOWN TABLE_ID\n"));
			dwRtn = GYAD_ERROR_UNKNOWN;
			break;
	}

	GYADAPI_SemaphoreUnlock();
	
	if(GYAD_ERROR_SUCCESS != dwRtn)
	{
		GYADAPI_Exit();
	}
	
	return dwRtn;
}

static GYADErrorType_e GYADCore_ResrcRelativeServic(GYADU16 u16ResrcId, GYADU16 u16SvcId, GYADU16 u16TsId)
{
	GYADU8 i = 0;
	GYADU8 j = 0;

	for(i = 0; i < sProgRelativInfo.wSvcNum; i++)
	{
	//	if((u16SvcId == sProgRelativInfo.psAdProgInfo[i].wSvcId)&&(u16TsId == sProgRelativInfo.psAdProgInfo[i].wTsId))
		if(u16SvcId == sProgRelativInfo.psAdProgInfo[i].wSvcId)
		{
			for(j = 0; j < sProgRelativInfo.psAdProgInfo[i].cResrcNum; j++)
			{
				if(u16ResrcId == sProgRelativInfo.psAdProgInfo[i].pwResrcId[j])
				{
					return GYAD_ERROR_SUCCESS;
				}
			}
		}
	}

	return GYAD_ERROR_UNKNOWN;
}

static GYADErrorType_e GYADCore_ResrcRelativeNVRAMServic(GYADU16 u16ResrcId, GYADU16 u16SvcId, GYADU16 u16TsId)
{
	GYADU8 i = 0;
	GYADU8 j = 0;
    if(psRelativNvramInfo == NULL)
    {
		return GYAD_ERROR_UNKNOWN;
    }
	for(i = 0; i < psRelativNvramInfo->wSvcNum; i++)
	{
		if(psRelativNvramInfo->psAdProgInfo == NULL)
		{
			return GYAD_ERROR_UNKNOWN;
    		}
		
		if(u16SvcId == psRelativNvramInfo->psAdProgInfo[i].wSvcId)
		{
			if(psRelativNvramInfo->psAdProgInfo[i].pwResrcId== NULL)
			{
				return GYAD_ERROR_UNKNOWN;
    			}
			
			for(j = 0; j < psRelativNvramInfo->psAdProgInfo[i].cResrcNum; j++)
			{
				if(u16ResrcId == psRelativNvramInfo->psAdProgInfo[i].pwResrcId[j])
				{
					return GYAD_ERROR_SUCCESS;
				}
			}
		}
	}

	return GYAD_ERROR_UNKNOWN;
}

GYADErrorType_e GYADCore_GetAdInfo(sGYADRelativeInfo_t sAdRelInfo, sGYAdInfo_t *sAdInfo)
{
	GYADU8 i = 0;

	if(sAdRelInfo.cAdIndx == GYAD_BOOT_LOGO_INDX)
	{
		return GYAD_ERROR_PARA;
	}
	
	if(0 == cFinishFlag)
	{
		if(0 == cTotalResrcNvram)
		{
			return GYAD_ERROR_UNKNOWN;
		}

		/*在从NVRAM读取的数据中查找*/
		for(i = 0; i < cTotalResrcNvram; i++)
		{
			if(sAdRelInfo.cAdIndx == psNvramResrcInfo[i].cAdIndx)
			{
			
///////////////////////////////////////////////////////////////////////////////////////////
				if(psNvramResrcInfo[i].cRelativeFlag == GYAD_RESRC_RELATIVE_SERVICE)
					{
						if(GYAD_ERROR_SUCCESS == GYADCore_ResrcRelativeNVRAMServic(psNvramResrcInfo[i].wResrcId, sAdRelInfo.wSvcId, sAdRelInfo.wTsId))
						{
							GYADAPI_Memcpy(sAdInfo, &psNvramResrcInfo[i].sAdInfo, sizeof(sGYAdInfo_t));
							return GYAD_ERROR_SUCCESS;
						}
						else
						{
							//GYADAPI_Memcpy(sAdInfo, &psNvramResrcInfo[i].sAdInfo, sizeof(sGYAdInfo_t));
							//return GYAD_ERROR_SUCCESS;
	//						return GYAD_ERROR_UNKNOWN;
						}
					}
					else
					{
						GYADAPI_Memcpy(sAdInfo, &psNvramResrcInfo[i].sAdInfo, sizeof(sGYAdInfo_t));
						return GYAD_ERROR_SUCCESS;
					}

///////////////////////////////////////////////////////////////////////////////////////////////////////////



			
				//GYADAPI_Memcpy(sAdInfo, &psNvramResrcInfo[i].sAdInfo, sizeof(sGYAdInfo_t));
				//return GYAD_ERROR_SUCCESS;
			}
		}
	}
	else
	{
		/*在从码流解析下来的数据中查找*/
		for(i = 0; i < cTotalResrcNum; i++)
		{
			if(sAdRelInfo.cAdIndx == psResrcInfo[i].cAdIndx)
			{
		//		if((sAdRelInfo.wCurTime >= psResrcInfo[i].sAdInfo.dwAdStartTime)&&(sAdRelInfo.wCurTime <= psResrcInfo[i].sAdInfo.dwAdEndTime))
				{
					if(psResrcInfo[i].cRelativeFlag == GYAD_RESRC_RELATIVE_SERVICE)
					{
						if(GYAD_ERROR_SUCCESS == GYADCore_ResrcRelativeServic(psResrcInfo[i].wResrcId, sAdRelInfo.wSvcId, sAdRelInfo.wTsId))
						{
							GYADAPI_Memcpy(sAdInfo, &psResrcInfo[i].sAdInfo, sizeof(sGYAdInfo_t));
							return GYAD_ERROR_SUCCESS;
						}
						else
						{
	//						return GYAD_ERROR_UNKNOWN;
						}
					}
					else
					{
						GYADAPI_Memcpy(sAdInfo, &psResrcInfo[i].sAdInfo, sizeof(sGYAdInfo_t));
						return GYAD_ERROR_SUCCESS;
					}
				}
			}
		}
	}
	
	return GYAD_ERROR_UNKNOWN;
}

