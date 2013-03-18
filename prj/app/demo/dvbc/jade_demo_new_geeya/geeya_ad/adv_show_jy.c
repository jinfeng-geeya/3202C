#include <sys_types.h>
#include <sys_media.h>
#include <mdl/libc/stdio.h>
#include <mdl/libc/string.h>
#include <mdl/nadv/adv_pic_fmt_decoder.h>
#include "adv_geeya.h"
#include "sys_config.h"
#include "mdl/logo/logo.h"
#include <mdl/fs/vsffs.h>
#include "mdl/dynamic/gdi_bitmap.h"
#include "../../../../prj/solutions/dvbc/public/ap_include.h"
/*
void Gy_get_time (UINT32 *Curtime)
{
	UINT32 temp;
	UINT32 cur_time;
	struct tm timer;
	
	cur_time = ap_tm_get_time();
	temp = cur_time + ap_get_time_zone_shift();
	GMTTIME_R(&temp, &timer);
	
//	LOG_PRINTF("__________________________________________time  %d:%d:%d\n",timer.tm_hour, timer.tm_min, timer.tm_sec);
	*Curtime = timer.tm_hour*3600 + timer.tm_min*60 + timer.tm_sec;
}	


UINT8 GetAdType_Gy(UINT16 ad_type, UINT16 sid)
{
	sGYAdInfo_t 			*pGetAdInfo = NULL;
	sGYADRelativeInfo_t 	sGetAdRelInfo;
	UINT32	u32CurTime = 0;
	UINT8 	u8AdType = 0;
	UINT8	i = 0;
	
	if(NULL == pGetAdInfo)
	{
		pGetAdInfo = MALLOC(50000);
	}
	switch(ad_type)
	{
		case SHOW_ADV_LOGO: 
			sGetAdRelInfo.cAdIndx	= SHOW_GYADV_LOGO;
			sGetAdRelInfo.wSvcId	= 0;
			break;
		case SHOW_ADV_MAIN_MENU:
			sGetAdRelInfo.cAdIndx	= SHOW_GYADV_MAIN_MENU;
			sGetAdRelInfo.wSvcId	= 0;
			break;
		case SHOW_ADV_ROLL_TXT:
			sGetAdRelInfo.cAdIndx	= SHOW_GYADV_ROLL_TXT;
			sGetAdRelInfo.wSvcId	= sid;
			break;
		case SHOW_ADV_CHANNEL_CHANGE:
			sGetAdRelInfo.cAdIndx	= SHOW_GYADV_CHANNEL_CHANGE;
			sGetAdRelInfo.wSvcId	= sid;	
			break;
	}
	Gy_get_time(&u32CurTime);
	sGetAdRelInfo.wCurTime	= u32CurTime;
	sGetAdRelInfo.wTsId 	= 2;

	GYADSTB_GetAdInfo(sGetAdRelInfo, pGetAdInfo);	
	if(pGetAdInfo == NULL)
	{
		return ADV_PIC_TYPE_UNKNOWN;
	}
	switch(ad_type)
	{
		case SHOW_ADV_LOGO: 
			u8AdType = ADV_PIC_TYPE_IFRAME;
			break;
		case SHOW_ADV_MAIN_MENU:
		case SHOW_ADV_CHANNEL_CHANGE:
			if((pGetAdInfo->cAdData[0] == 0x47) && (pGetAdInfo->cAdData[1] == 0x49) && (pGetAdInfo->cAdData[2] == 0x46))
			{
				u8AdType = ADV_PIC_TYPE_GIF;
			}
			else if((pGetAdInfo->cAdData[0] == 0xff) && (pGetAdInfo->cAdData[1] == 0xd8) && (pGetAdInfo->cAdData[2] == 0xff))
			{
				u8AdType = ADV_PIC_TYPE_JPG;
			}
			else if((pGetAdInfo->cAdData[0] == 0x42) && (pGetAdInfo->cAdData[1] == 0x4d) && (pGetAdInfo->cAdData[2] == 0x48))
			{
				u8AdType = ADV_PIC_TYPE_BMP;
			}
			break;
		case SHOW_ADV_ROLL_TXT:
			sGetAdRelInfo.cAdIndx	= SHOW_GYADV_ROLL_TXT;
			sGetAdRelInfo.wSvcId	= sid;
			break;
	}
//	u8AdType = (UINT8)pGetAdInfo->cAdType;
	FREE(pGetAdInfo);
	return u8AdType;
}



void clear_still_buffer(void)
{
	UINT8 *buf;
	UINT32 i;

	buf = SYS_MAP_SFRM_ADDR;
	for(i=0;i<SYS_MAP_SFRM_LEN;i++)
	{
		if((i%2) == 0)
		{
			*(buf+i) = 0x10;
		}
		else
		{
			*(buf+i) = 0x80;
		}
	}
}


RET_CODE ap_gyadlogo_show()
{
    extern void func_1st_pic(void* para1, UINT32 rid);
	sGYAdInfo_t *pBootADInfo = NULL;
	UINT32		u32BootAddr = 0;
	UINT32		len = 0;
	UINT32		u32InfoLen = 0;
	UINT8		u8VerBootAdv = 0;
	UINT8		u8BootADNum = 1;
	INT8		i = 0;
	UINT8		u8Data0 = 0;

	get_vsf_info(CHUNKID_BOOTAD_ID, &u32BootAddr, &len);
	GYSTBAD_ReadNvram(u32BootAddr+2*sizeof(GYADU8)+sizeof(GYADU32)*(u8BootADNum-1), &u32InfoLen, sizeof(GYADU32));
	
	GYSTBAD_GetBootVer(&u8VerBootAdv);
	if(u8VerBootAdv == 0xff)
	{
		return ERR_FAILURE;
	}

	pBootADInfo = (UINT8*)MALLOC(u32InfoLen);
	
	GYSTBAD_GetBootAd(pBootADInfo, u8BootADNum);
	
	LOG_PRINTF("logo_adv_len = %d\n",pBootADInfo->dwAdLen);
	for(i=0; i<100; i++)
	{
		LOG_PRINTF("0x%02x  ",pBootADInfo->cAdData[i]);
		if((i+1)%30 == 0)
		{
			LOG_PRINTF("\n");
		}
		if(0 == pBootADInfo->cAdData[i])
		{
			u8Data0++;
		}
	}
	LOG_PRINTF("\n");

	if(u8Data0 > 98)
	{
		return ERR_FAILURE;
	}

	if(pBootADInfo->cAdType == ADV_PIC_TYPE_GIF) 		
	{
		clear_still_buffer();
//		ap_gif_logo(pBootADInfo->cAdData, pBootADInfo->dwAdLen);
		logo_init(LOGO_GIF, pBootADInfo->cAdData, pBootADInfo->dwAdLen, 0, func_1st_pic);
		logo_show();
		FREE(pBootADInfo);
		return SUCCESS;
	}	
	else if(pBootADInfo->cAdType == ADV_PIC_TYPE_JPG)	
	{
		clear_still_buffer();
		logo_init(LOGO_JPG, pBootADInfo->cAdData, pBootADInfo->dwAdLen, 0, func_1st_pic);
		logo_show();
		FREE(pBootADInfo);
		return SUCCESS;
	}
	else if(pBootADInfo->cAdType == ADV_PIC_TYPE_IFRAME)
	{
	//	ap_show_logo_gyIframe(pBootADInfo->cAdData, pBootADInfo->dwAdLen);
		FREE(pBootADInfo);
		return SUCCESS;
	}
	
	return ERR_FAILURE;
}
*/
int adv_read_pf(UINT16 sid,struct bitmap *bmp)
{
 sGYAdInfo_t	*pGetAdInfo = NULL;
 sGYADRelativeInfo_t  sGetAdRelInfo;
 UINT8 u8AdType = ADV_PIC_TYPE_UNKNOWN;
 
 pGetAdInfo = MALLOC(50000);
 
 sGetAdRelInfo.cAdIndx = SHOW_GYADV_CHANNEL_CHANGE;
 sGetAdRelInfo.wSvcId = sid; 
 sGetAdRelInfo.wCurTime= 0;
 sGetAdRelInfo.wTsId  = 2;
 if(GYADSTB_GetAdInfo(sGetAdRelInfo, pGetAdInfo) != SUCCESS)
 {
  FREE(pGetAdInfo);
  pGetAdInfo = NULL;
  return ERR_FAILURE;
 }
 
 if((pGetAdInfo->cAdData[0] == 0x47) && (pGetAdInfo->cAdData[1] == 0x49) && (pGetAdInfo->cAdData[2] == 0x46))
 {
  u8AdType = ADV_PIC_FMT_GIF;
 }
 else if((pGetAdInfo->cAdData[0] == 0xff) && (pGetAdInfo->cAdData[1] == 0xd8) && (pGetAdInfo->cAdData[2] == 0xff))
 {
  u8AdType = ADV_PIC_FMT_JPG;
 }
 else if((pGetAdInfo->cAdData[0] == 0x42) && (pGetAdInfo->cAdData[1] == 0x4d) && (pGetAdInfo->cAdData[2] == 0x48))
 {
  u8AdType = ADV_PIC_FMT_BMP;
 }
 else
 {
  FREE(pGetAdInfo);
  pGetAdInfo = NULL;
  return ERR_FAILURE;
 }

 adv_pic_fmt_decoder(u8AdType, pGetAdInfo->cAdData, pGetAdInfo->dwAdLen, bmp);
 
 FREE(pGetAdInfo);
 pGetAdInfo = NULL;
 
 return SUCCESS;
 
}
	



