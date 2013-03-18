/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *
 *  File: dvt_sc.c
 *
 *  Description: This file fills the smart card related API of DVT CA library.
 *
 *  History:
 *           Date            Author         Version       Comment
 *      ========    ======      ======     =========
 *  1.  2008.01.24      Grady Xu       0.1.000       Create this file
 *
 ****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <hld/smc/smc.h>

#include <api/libcas/gy/cas_gy.h>
#include "gyca_sc.h"


#if 0
#define GYCASDRV_PRINTF	libc_printf
#else
#define GYCASDRV_PRINTF(param...)	do{}while(0)
#endif

#define GYCASDRV_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) \
                         {\
						 	GYCASDRV_PRINTF("0x%02x,",*((data)+i)); \
						 	if((i&0x0f) == 0x0f) GYCASDRV_PRINTF("\n");\
                         }\
                         GYCASDRV_PRINTF("\n"); }


/**********************************************************************************************
** 	函数名称:GYCA_SC_DRV_Initialize
***********************************************************************************************
** 	提 供 者： 	机顶盒。
**	使 用 者：	CAM
**	何时调用：	金亚CAS在做初始化工作时提供的一个接口给机顶盒
**                                初始化机顶盒中的智能卡模块驱动程序.
**	功能描述：	 初始化智能卡驱动。
**	输入参数：	 无。
**	输出参数：	 无。
**	返 回 值：	 SUCCESS：成功， FAILURE： 失败。
**	其他说明：
**********************************************************************************************/
GYS32	GYCA_SC_DRV_Initialize(void)
{
	GYCASDRV_PRINTF("GYCA_SC_DRV_Initialize---ENTER\n");
	return GYSUCCESS;
}

/**********************************************************************************************
** 	函数名称: GYCA_SCStatus_RegisterEN2Notify
***********************************************************************************************
** 	提 供 者： 	机顶盒。
**	使 用 者：	CAM
**	何时使用：	CAM 初始化的时候调用该函数。
**	功能描述：	注册智能卡状态改变时通知CAM任务的回调函数。
**	输入参数：	SCStatusNotify (GYU8 bCardNumber, GYU8 bStatus,GYU8* pReserved1, GYS32 iReserved2 )
**					其中:  bCardNumber：	卡座号；
**							bStatus：		卡座状态；
**							pReserved1            保留参数1;
**							iReserved2             保留参数2;
**	输出参数：	无。
**	返 回 值：	SUCCESS：成功， FAILURE： 失败。
**	其他说明：	机顶盒须在智能卡的状态改变后立即通过该回调函数
**                                通知CAM。
**					智能卡的状态必须为GYCA_SMCSTATUS所定义的几个值之一。
**                                详细请参考gyca_def.h GYCA_SMCSTATUS定义                                     
***********************************************************************************************/
GYS32 GYCA_SCStatus_RegisterEN2Notify(SC_Status_Notify_Fct_t SCStatusNotify)
{
	gycas_info_priv_t *priv;	
	GYCASDRV_PRINTF("GYCA_SCStatus_RegisterEN2Notify---ENTER\n");
	priv = gycas_get_priv();
	priv->smc_notify = SCStatusNotify;

	return GYSUCCESS;
}

/*********************************************************************************************
** 	函数名称:GYCA_SC_DRV_ResetCard
**********************************************************************************************
**	提 供 者：		机顶盒。
**	使 用 者：		CA模块。
**	功能描述： 	复位智能卡。
**	输入参数： 	bCardNumber，	为智能卡的卡座号。
**	输出参数： 	无。
**	返 回 值： 		无返回，总认为是成功的。
**	其他说明：
*********************************************************************************************/
void GYCA_SC_DRV_ResetCard (GYU8 bCardNumber)
{
	INT32 ret;
	UINT8 i;
	UINT8 buffer[33];
	UINT16 atr_size;
	struct smc_device *smc_dev = NULL;
	gycas_info_priv_t *priv;
	priv = gycas_get_priv();

	smc_dev = (struct smc_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SMC);
	if(NULL == smc_dev)
	{		
		GYCASDRV_PRINTF("Err: %s- Get Smart card reader failed\n",__FUNCTION__);
		return CAS_FAIL;
	}
	
	for(i=0; i<CAS_SC_RETRY_TIMES; i++)
	{
		ret = smc_reset(smc_dev, buffer, &atr_size);
		if(RET_SUCCESS == ret)
		{

			MEMCPY(priv->atr,buffer,atr_size);
			priv->atr_size = atr_size;
			UINT8 j;
			GYCASDRV_PRINTF("Get Smart card atr :");
			for(j=0; j<atr_size;j++)
				GYCASDRV_PRINTF("%02x, ",buffer[j]);
			GYCASDRV_PRINTF("\n");

			smc_io_control(smc_dev, SMC_DRIVER_SET_WWT, 2000);
			return CAS_SUCCESS;
		}
		else
		{
			GYCASDRV_PRINTF("Err: %s- smc_reset failed!!\n", __FUNCTION__);	
		}
	}	
	priv->atr_size = 0;
	return CAS_FAIL;	
}

/*********************************************************************************************
** 	函数名称:GYCA_SC_DRV_SendDataEx
**********************************************************************************************
**	提 供 者： 		机顶盒。
**	使 用 者：	 	CA模块。
**	何时使用：		向智能卡发送数据时。
**	功能描述：		CA模块调用此函数向智能卡发送数据。
**	输入参数：		bCardNumber：	暂时保留不用；
**						bLength：		pabMessage的长度；
**						pabMessage：		发送命令的消息指针；
**	输出参数：		pabResponse：	响应结果的数据块；
**						bRLength：		响应结果的数据块长度。
**                              pbSW1：         智能卡状态字节1
**				pbSW2：         智能卡状态字节2
**	返 回 值：		SMC_COM_OK				(0)发送命令成功
**						GYSUCCESS 成功  GYFAILURE 失败
**	其他说明：if *pbSW1=0x61 then buf[0] = pabMessage[0]&0x7f
**********************************************************************************************/
GYS32 GYCA_SC_DRV_SendDataEx
(
	GYU8 bCardNumber, 
	GYU8 bLength,
	GYU8* pabMessage, 
	GYU8* pabResponse,
	GYU8* bRLength,
	GYU8 *pbSW1,
	GYU8 *pbSW2
)
{
	UINT8 status[2];
	UINT16 size;
	UINT8 atr[33];
	UINT16 atr_size, histo_offset;
	UINT8 i;
	//gycas_info_priv_t *priv;
	struct smc_device *smc_dev;
	UINT8 g_cmd_get[5] = {0x00,0xc0,0x00,0x00,0x10};
	INT8 result = GYFAILURE;
	UINT8  rcvbuf[256+2];

	
	GYCASDRV_PRINTF("GYCA_SC_DRV_SendDataEx---ENTER\n");
	GYCASDRV_DUMP(pabMessage, bLength);

	smc_dev = (struct smc_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SMC);
	if (smc_dev == NULL) 
	{
		GYCASDRV_PRINTF("[Cas_gy_drv.c] GYCA_SC_DRV_SendDataEx: Get SMC Fail\n");
		return GYFAILURE;
	}
	if (pabMessage == NULL || bLength ==0 || pabResponse == NULL || bRLength == NULL) 
	{
		GYCASDRV_PRINTF("[Cas_gy_drv.c] GYCA_SC_DRV_SendDataEx: Param Error\n");
		return GYFAILURE;
	}
	
	for(i=0; i<3; i++)
	{
		result = smc_iso_transfer(smc_dev, (UINT8 *)pabMessage, (INT16)bLength, rcvbuf, 255, &size); 
		status[0] = rcvbuf[size-2];
		status[1] = rcvbuf[size-1];
		//GYCASDRV_PRINTF("GYCA_SC_DRV_SendDataEx-1-- result[%x] pbSW1[%x]  pbSW2[%x]\n",result,status[0],status[1]);

		if(result != SUCCESS)// && rcvbuf[size-2]!=0x61)
		{
			GYCASDRV_PRINTF("GYCA_SC_DRV_SendDataEx->the 1 smc_transfer fail\n");
			GYCA_OSPTaskTemporarySleep(5);
	        continue;
		}
		//if((status[0] == 0x61) || (status[0]==0x6c))
		if((status[0] == 0x61))// || (rcvbuf[size-2]==0x6c))
		{
			g_cmd_get[0] = pabMessage[0]&0x7f;
			g_cmd_get[4] = status[1];
			result = smc_iso_transfer(smc_dev, (UINT8 *)g_cmd_get, 5, rcvbuf, 255, &size);
			if(result != SUCCESS)
			{
				GYCASDRV_PRINTF("GYCA_SC_DRV_SendDataEx->the 2 smc_transfer fail\n");
				GYCA_OSPTaskTemporarySleep(5);
				return GYFAILURE;
	   			 //continue;
				//return GYFAILURE;
			}	
			status[0] = rcvbuf[size-2];
			status[1] = rcvbuf[size-1];
			//GYCASDRV_PRINTF("GYCA_SC_DRV_SendDataEx-2-- result[%x] pbSW1[%x]  pbSW2[%x]\n",result,status[0],status[1]);
			
		}
		break;
	}
	if(i == 3)
    {
        return GYFAILURE;
    }
	
	*bRLength = size-2;//pabMessage[4];
	*pbSW1 = status[0];//rcvbuf[size-2];
	*pbSW2 = status[1];//rcvbuf[size-1];
	 MEMCPY(pabResponse,rcvbuf,size-2);
	//GYCASDRV_PRINTF("after GYCA_SC_DRV_SendDataEx---SUCCESS-pbSW1[%x]  pbSW2[%x]\n",*pbSW1,*pbSW2);
	return GYSUCCESS;
}


