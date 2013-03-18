/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: tf_sc.c
 *
 *  Description: This file fills the smart card related API of Tongfang CA library.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  1. 2006.3.19  Gushun Chen     0.1.000    Initial
 *  2. 2007.10.22  Grady Xu		 0.2.000	 re-org the Tongfang CA lib.
 ****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <api/libchunk/chunk.h>
#include <hld/dmx/dmx.h>
#include <hld/smc/smc.h>

//#include <api/libcas/cdca/pub_st.h>
#include <api/libcas/cdca/CDCAS30.h>
#include "cd_sc.h"

extern CDCA_BOOL CDSTBCA_SCReset(CDCA_U8* pbyATR,  CDCA_U8* pbyLen)
{
	INT32 ret;
	UINT8 i;
	UINT8 buffer[33];
	UINT16 atr_size;
	struct smc_device *smc_dev = NULL;

	smc_dev = (struct smc_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SMC);
	if(smc_dev == NULL)
	{		
		CAS_SCERR_PRINTF("STBCA_SCReset: Get Smart card reader failed\n");
		return CDCA_FALSE;
	}
	for(i=0; i<2; i++)
	{
		ret = smc_reset(smc_dev, buffer, &atr_size);
		if(ret == RET_SUCCESS)
		{
			CAS_SC_PRINTF("STBCA_SCReset: smc_reset ok! ATR size=%d\n", atr_size);
			//TFCAS_SC_DUMP(buffer, atr_size);
			//according to TF porting doc, set the wwt to 2000 ms
			smc_io_control(smc_dev, SMC_DRIVER_SET_WWT, 2000);
			*pbyLen = atr_size;
			MEMCPY(pbyATR, buffer, atr_size);
			return CDCA_TRUE;
		}
		else
		{
			CAS_SCERR_PRINTF("!!!STBCA_SCReset: smc_reset failed!!\n");	
		}
	}	
	return CDCA_FALSE;	
}

extern CDCA_BOOL CDSTBCA_SCPBRun( const CDCA_U8* pbyCommand, CDCA_U16 wCommandLen,  CDCA_U8* pbyReply,  CDCA_U16* pwReplyLen)
{
	UINT8 i;
	INT32 ret;
	UINT8 buffer[33];
	UINT16 atr_size;
	INT16 len = 0xff;
	struct smc_device *smc_dev = NULL;

	if((pbyCommand == NULL)||(wCommandLen == 0)||(pbyReply == NULL))
	{
		CAS_SCERR_PRINTF("STBCA_SCPBRun: param error!\r\n");
		return CDCA_FALSE;
	}

	smc_dev = (struct smc_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_SMC);
	if(smc_dev == NULL)
	{		
		CAS_SCERR_PRINTF("STBCA_SCPBRun: Get Smart card reader failed\n");
		return CDCA_FALSE;
	}
	//TFCAS_SC_PRINTF("CMD: ");
	//TFCAS_SC_DUMP(pbyCommand, wCommandLen);
	for(i=0; i<3; i++)
	{
		ret = smc_iso_transfer(smc_dev,(UINT8*) pbyCommand, (INT16)wCommandLen, pbyReply, len , (INT16*)pwReplyLen);
		if(ret == RET_SUCCESS)
		{
			//TFCAS_SC_PRINTF("ANS: ");
			//TFCAS_SC_DUMP(pbyReply, *pwReplyLen);			
			return CDCA_TRUE;
		}
	}
	CAS_SCERR_PRINTF("!!!STBCA_SCPBRun: transfer failed!!\n");	

	for(i=0; i<3; i++)
	{
		ret = smc_reset(smc_dev, buffer, &atr_size);
		if(ret == RET_SUCCESS)
			return CDCA_TRUE;
	}
	CAS_SCERR_PRINTF("STBCA_SCPBRun: reset failed!!\n");	
	return CDCA_FALSE;
}


