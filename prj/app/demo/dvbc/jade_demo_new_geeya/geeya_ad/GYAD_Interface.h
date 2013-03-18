/*****************************************************************************

File Name  : GYAD_Interface.h

Description: geeya advertisement interface.

Copyright (C) 2009 Chengdu Geeya Technology Co.,Ltd

Author: Archer 

Create Date:2010.09.09

* Date				Name			    Modification
* ---------		-----------			------------
* 2010.09.09		      Archer			     Created
* 2010.09.16		      Archer			     Modified
*  
*****************************************************************************/

#ifndef __GYAD_INTERFACE_H__
#define __GYAD_INTERFACE_H__

#ifdef  __cplusplus
extern "C" {
#endif

#define	NULL 	((void*)0)

typedef	signed char	GYADS8;
typedef	signed short	GYAD16;
typedef	int			GYADS32;
typedef	unsigned char	GYADU8;
typedef	unsigned short	GYADU16;
typedef	unsigned int	GYADU32;
typedef	unsigned char	GYADBOOL;       
typedef	void			GYADVOID;

/*º¯Êý·µ»ØÖµ*/
typedef enum
{
	GYAD_ERROR_SUCCESS 	= 0, /*³É¹¦*/
	GYAD_ERROR_PARA		= 1, /*²ÎÊý´íÎó*/
	GYAD_ERROR_INVALID 	= 2, /*ÎÞÐ§Ö¸Õë*/
	GYAD_ERROR_UNLOCK 	= 3, /*¹ã¸æÆµµãÎ´Ëø¶¨*/
	GYAD_ERROR_FAIL 		= 4, /*¶Á/Ð´NVRAMÊ§°Ü£¬ÄÚ´æ·ÖÅäÊ§°Ü*/
	GYAD_ERROR_UNKNOWN 	= 5, /*Î´Öª´íÎó*/
}GYADErrorType_e;

/*¹ã¸æÀàÐÍ*/
typedef enum
{
	GYAD_RESRC_TYPE_IFRAME 	= 0, /*IÖ¡*/
	GYAD_RESRC_TYPE_JPG		= 1, /*JPG*/
	GYAD_RESRC_TYPE_GIF 		= 2, /*GIF*/
	GYAD_RESRC_TYPE_BMP 		= 3, /*BMP*/
	GYAD_RESRC_TYPE_TXT 		= 4, /*ÎÄ×Ö¹ã¸æ*/
}GYADResrcType_e;

typedef struct{
	GYADU8 cAdIndx;	/*¹ã¸æÎ»±àºÅ*/
	GYADU16 wSvcId;	/*·þÎñºÅID*/
	GYADU16 wTsId;	/*´«ÊäÁ÷ID*/
	GYADU32 wCurTime;	/*µ±Ç°Ê±¼ä*/
}sGYADRelativeInfo_t;

/*¹ã¸æÐÅÏ¢*/
typedef struct{
	GYADU32 dwAdDisTime;/*ÏÔÊ¾Ê±³¤*/
	GYADU32 dwAdStartTime;/*¿ªÊ¼Ê±¼ä*/
	GYADU32 dwAdEndTime;	/*½áÊøÊ±¼ä*/
	GYADU32 dwTxtColor;	/*ÎÄ±¾ÑÕÉ«*/
	GYADU32 dwBgColor;	/*±³¾°ÑÕÉ«*/
	GYADU32 dwAdLen; 	/*³¤¶È*/
	GYADU16 wAdX0;	/*¹ã¸æ×ø±êÐÅÏ¢*/
	GYADU16 wAdY0;
	GYADU16 wAdX1;
	GYADU16 wAdY1;
	GYADU8 cAdType;	/*ÀàÐÍ*/
	GYADU8* cAdData;	/*Êý¾Ý*/
}sGYAdInfo_t;

/**************************************************************
* Function: GYSTBAD_Malloc(GY_U32 dwMemSize)
* Descripe: Called by GYAD
* Para:
	dwMemSize: ÐèÒª·ÖÅäµÄÄÚ´æµÄ´óÐ¡
* Return: None
* Note: 
* Author: Archer
***************************************************************/
GYADVOID *GYSTBAD_Malloc(GYADU32 dwMemSize);


/**************************************************************
* Function: GYSTBAD_Free(GY_VOID* buffer)
* Descripe: Called by GYAD
* Para:
	buffer: ÄÚ´æµØÖ·
* Return: None
* Note: 
* Author: Archer
***************************************************************/
GYADVOID GYSTBAD_Free(GYADVOID *buffer);


/**************************************************************
* Function: GYSTBAD_Memset(GY_VOID* pDestBuf, GY_U8 c, GY_U32 dwSize)
* Descripe: Called by GYAD
* Para:
	pDestBuf: Ìî³äµØÖ·
	cData: Ìî³äµÄÊý¾Ý
	dwSize: Ìî³äµÄ³¤¶È
* Return: None
* Note: 
* Author: Archer
***************************************************************/
GYADVOID GYSTBAD_Memset(GYADVOID *pDestBuf, GYADU32 c, GYADU32 dwSize);


/**************************************************************
* Function: GYSTBAD_Memcpy(GY_VOID* pDestBuf, GY_VOID* pSrcBuf, GY_U32 dwSize)
* Descripe: Called by GYAD
* Para:
	pDestBuf: 
	pSrcBuf: 
	dwSize: 
* Return: None
* Note: 
* Author: Archer
***************************************************************/
GYADVOID GYSTBAD_Memcpy(GYADVOID *pDestBuf, GYADVOID *pSrcBuf, GYADU32 dwSize);


GYADVOID GYSTBAD_Print(const GYADU8 *fmt, ...);


/**************************************************************
* Function: GYSTBAD_RequestPrivateData(GYADU8 cReqID, const GYADU8 *pcFilter, const GYADU8* pcMask, 
									GYADU8 cLen, GYADU16 wPid, GYADU8 byWaitSeconds);
* Descripe: Called by GYAD
* Para:
	cReqID: ÊÕÈ¡Ë½ÓÐ±íµÄÇëÇóºÅ£¬ÓëGYADSTB_PrivateDataCallbackÖÐµÄcReqIDÏàÍ¬
	pcFilter: µ±Ç°¹ýÂËÆ÷µÄÖµ£¬8×Ö½Ú
	pcFilter: µ±Ç°¹ýÂËÆ÷µÄÑÚÂë£¬8×Ö½Ú
	wLen: È¡µ½µÄË½ÓÐÊý¾ÝµÄ³¤¶È
	wPid: Êý¾ÝÁ÷µÄPID£¬£¬ÓëGYADSTB_PrivateDataCallbackÖÐµÄwPidÏàÍ¬
	byWaitSeconds: ³¬Ê±µÈ´ýÊ±¼ä£¬µ¥Î»ÎªÃë£¬Îª0ÔòÒ»Ö±µÈ´ý
* Return: Refer to GYADErrorType_e
* Note: Ö»Òªµ÷ÓÃÁË±¾º¯Êý£¬¾Í±ØÐëµ÷ÓÃGYADSTB_PrivateDataGot£¬ËüÃÇÊÇÒ»Ò»¶ÔÓ¦µÄ
* Author: Archer
***************************************************************/
GYADErrorType_e GYSTBAD_RequestPrivateData(GYADU8 cReqID, const GYADU8 *pcFilter, 
											const GYADU8 *pcMask, GYADU8 cLen, 
											GYADU16 wPid, GYADU8 byWaitSeconds);


/**************************************************************
* Function: GYSTBAD_RequestFree(GYADU8 cReqID)
* Descripe: Called by GYAD
* Para:
	cReqID: 
* Return: None
* Note: ÊÍ·Åfilter
* Author: Archer
***************************************************************/
GYADVOID GYSTBAD_RequestFree(GYADU8 cReqID);

/**************************************************************
* Function: GYADSTB_SetTS(GYADVOID)
* Descripe: Called by GYAD
* Para:
	None
* Return: Refer to GYADErrorType_e
* Note: Ëø¶¨ËùÔÚÆµµã
* Author: Archer
***************************************************************/
GYADErrorType_e GYSTBAD_SetTS(GYADVOID);


/**************************************************************
* Function: GYSTBAD_SemaphoreLock(GYADVOID)
* Descripe: Called by GYAD
* Para:
	None
* Return: None
* Note: 
* Author: Archer
***************************************************************/
GYADVOID GYSTBAD_SemaphoreLock(GYADVOID);


/**************************************************************
* Function: GYSTBAD_SemaphoreUnlock(GYADVOID)
* Descripe: Called by GYAD
* Para:
	None
* Return: None
* Note: 
* Author: Archer
***************************************************************/
GYADVOID GYSTBAD_SemaphoreUnlock(GYADVOID);


/**************************************************************
* Function: GYSTBAD_SaveBootAd(sGYAdInfo_t *bootADInfo, GYADU8 bootADNum, GYADU8 bootADVer)
* Descripe: Called by GYAD
* Para:
	bootADInfo: ¿ª»ú¹ã¸æÐÅÏ¢£¬ÐèÒª±£´æ
	bootADNum:   ¿ª»ú¹ã¸æµÄ¸öÊý
	bootADVer:   ¿ª»ú¹ã¸æ°æ±¾£¬ÐèÒª±£´æ
* Return: Refer to GYADErrorType_e
* Note:STB×ÔÐÐ±£´æ
* Author: Archer
***************************************************************/
GYADErrorType_e GYSTBAD_SaveBootAd(sGYAdInfo_t *bootADInfo, GYADU8 bootADNum, GYADU8 bootADVer);


/**************************************************************
* Function: GYSTBAD_GetBootVer(GYADU8 *ver)
* Descripe: Called by GYAD
* Para:
	version: ¿ª»ú¹ã¸æµÄ°æ±¾
* Return: Refer to GYADErrorType_e
* Note: 
* Author: Archer
***************************************************************/
GYADErrorType_e GYSTBAD_GetBootVer(GYADU8 *version);


/**************************************************************
* Function: GYSTBAD_GetNvram(GYADU32 *pdwAddr, GYADU32 *len)
* Descripe: Called by GYAD
* Para:
	pdwAddr: NVRAMµÄÆðÊ¼µØÖ·
	len:   NVRAMµÄ´óÐ¡
* Return: Refer to GYADErrorType_e
* Note: ä
* Author: Archer
***************************************************************/
GYADVOID GYSTBAD_GetNvram(GYADU32 *pdwAddr, GYADU32 *len);


/**************************************************************
* Function: GYSTBAD_ReadNvram(GYADU32 dwAddr, GYADU8 *data, GYADU32 len)
* Descripe: Called by GYAD
* Para:
       dwAddr: ÆðÊ¼µØÖ·
	data: ÐèÒª¶ÁÈ¡µÄÊý¾Ý
	len:   Êý¾Ý³¤¶È
* Return: Refer to GYADErrorType_e
* Note: 
* Author: Archer
***************************************************************/
GYADErrorType_e GYSTBAD_ReadNvram(GYADU32 dwAddr, GYADU8 *data, GYADU32 len);


/**************************************************************
* Function: GYSTBAD_WriteNvram(GYADU32 dwAddr, GYADU8 *data, GYADU32 len)
* Descripe: Called by GYAD
* Para:
       dwAddr: ÆðÊ¼µØÖ·
	data: ÐèÒª±£´æµÄÊý¾Ý
	len:   Êý¾Ý³¤¶È
* Return: Refer to GYADErrorType_e
* Note: 
* Author: Archer
***************************************************************/
GYADErrorType_e GYSTBAD_WriteNvram(GYADU32 dwAddr, GYADU8 *data, GYADU32 len);


/**************************************************************
* Function: GYSTBAD_ADExit(GYADVOID)
* Descripe: Called by GYAD
* Para:
	None
* Return: None
* Note: GYADÍ¨ÖªSTB¹ã¸æ½ÓÊÕÍê±Ï
* Author: Archer
***************************************************************/
GYADVOID GYSTBAD_ADExit(GYADVOID);


/**************************************************************
* Function: GYADSTB_ADStart(GYADU16 wPid)
* Descripe: Called by STB
* Para:
	wPid: GYADµÄPID
* Return: refer to GYADErrorType_e
* Note: STBÍ¨ÖªGYAD¿ªÊ¼½ÓÊÕ¹ã¸æ
* Author: Archer
***************************************************************/
GYADErrorType_e GYADSTB_ADStart(GYADU16 wPid);


/**************************************************************
* Function: GYADSTB_PrivateDataGot(GYADU8 cReqID, GYADU8 bOK, GYADU16 wPid, const GYADU8 *pcReceiveData, GYADU32 dwLen)
* Descripe: Called by STB
* Para:
	cReqID: ÊÕÈ¡Ë½ÓÐ±íµÄÇëÇóºÅ£¬ÓëGYSTBAD_RequestPrivateDataÖÐµÄcReqIDÏàÍ¬
	bOK: ÊÕÈ¡Êý¾ÝÊÇ·ñ³É¹¦,   1:³É¹¦   0:Ê§°Ü
	wPid: Êý¾ÝÁ÷µÄPID£¬£¬ÓëGYADSTB_PrivateDataCallbackÖÐµÄwPidÏàÍ¬
	pcReceiveData: È¡µ½µÄË½ÓÐÊý¾Ý
	dwLen: È¡µ½µÄË½ÓÐÊý¾ÝµÄ³¤¶È
* Return: Refer to GYADErrorType_e
* Note: Ö»Òªµ÷ÓÃÁËGYSTBAD_RequestPrivateData£¬¾Í±ØÐëµ÷ÓÃ±¾º¯Êý£¬ËüÃÇÊÇÒ»Ò»¶ÔÓ¦µÄ
* Author: Archer
***************************************************************/
GYADErrorType_e GYADSTB_PrivateDataGot(GYADU8 cReqID, GYADU8 bOK, GYADU16 wPid, const GYADU8 *pcReceiveData, GYADU32 dwLen);


/**************************************************************
* Function: GYADSTB_GetAdInfo(sGYADRelativeInfo_t sAdRelInfo, sGYAdInfo_t *sAdInfo)
* Descripe: Called by STB
* Para:
	sAdRelInfo: ¹ã¸æ¹ØÁªÐÅÏ¢
	sAdInfo: ¹ã¸æÐÅÏ¢
* Return: Refer to GYADErrorType_e
* Note: 
* Author: Archer
***************************************************************/
GYADErrorType_e GYADSTB_GetAdInfo(sGYADRelativeInfo_t sAdRelInfo, sGYAdInfo_t *sAdInfo);

#ifdef  __cplusplus
}
#endif

#endif

