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
#include <api/libcas/dvt/DVTSTB_CASInf.h>
#include <api/libcas/dvt_dvt.h>
#include "dvt_sc.h"

//#include "../gy/cas_gy.h"

static UINT8 atrs[][4] =
{
	{0x3b, 0x02, 0x06, 0x62},
	{0x3b, 0x02, 0x1c, 0x90},
};

extern gycas_info_priv_t* gycas_get_priv(void);
HRESULT DVTSTBCA_SCReset(void)
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
		CAS_SCERR_PRINTF("Err: %s- Get Smart card reader failed\n",__FUNCTION__);
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
			CAS_SCERR_PRINTF("Get Smart card atr :");
			for(j=0; j<atr_size;j++)
				CAS_SCERR_PRINTF("%02x, ",buffer[j]);
			CAS_SCERR_PRINTF("\n");

			smc_io_control(smc_dev, SMC_DRIVER_SET_WWT, 2000);
			return CAS_SUCCESS;
		#if 0
			CAS_SC_PRINTF("smc_reset ok! ATR size=%d\n", atr_size);
			CAS_SC_DUMP(buffer, atr_size);

			//Check ATR size and content
			//if (atr_size == 4)//gaca size unknow
			{
				//{
					{
						//set the wwt to 2 seconds
						smc_io_control(smc_dev, SMC_DRIVER_SET_WWT, 2000);
						return CAS_SUCCESS;
					}
				//}
			}

			CAS_SCERR_PRINTF("Err: %s- ATR size or content invalid!\n", __FUNCTION__);	

			return CAS_FAIL;
		#endif
		}
		else
		{
			CAS_SCERR_PRINTF("Err: %s- smc_reset failed!!\n", __FUNCTION__);	
		}
	}	
	priv->atr_size = 0;
	return CAS_FAIL;	
}

HRESULT DVTSTBCA_SCAPDU(BYTE byReaderNo, BYTE * pbyLen, const BYTE * byszCommand, BYTE * byszReply)
{
	UINT8 i;
	UINT8 buffer[33];
	INT16 size = 0;
	INT32 ret;
	
	struct smc_device *smc_dev = NULL;

	if((NULL == byszCommand) ||(*pbyLen == 0) || (NULL == pbyLen))
	{
		CAS_SCERR_PRINTF("Err: %s- param error!\n", __FUNCTION__);
		return CAS_FAIL;
	}

	smc_dev = (struct smc_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SMC);
	if(NULL == smc_dev)
	{		
		CAS_SCERR_PRINTF("Err: %s- Get Smart card reader failed\n",__FUNCTION__);
		return CAS_FAIL;
	}
	
	CAS_SC_PRINTF("CMD: ");
	CAS_SC_DUMP(byszCommand, *pbyLen);
	for(i=0; i<CAS_SC_RETRY_TIMES; i++)
	{
		ret = smc_iso_transfer(smc_dev, byszCommand, (INT16)*pbyLen, byszReply, 0xff, &size);
		if(ret == RET_SUCCESS)
		{
			if ((NULL != byszReply) && (0 != size))
			{
				CAS_SC_PRINTF("ANS: ");
				CAS_SC_DUMP(byszReply, size);			
			}
			*pbyLen = size;
			return CAS_SUCCESS;
		}
	}
	CAS_SCERR_PRINTF("Err: %s transfer failed!!\n",__FUNCTION__);		
	return CAS_FAIL;
}


