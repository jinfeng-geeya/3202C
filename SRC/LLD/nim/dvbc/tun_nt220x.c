/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    tun_nt220x.c
*
*    Description:    This file contains nt220x basic function in LLD. 
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  20100902		Joey.gao		 Ver 0.1	Create file.
*	
*****************************************************************************/

/*
  Copyright (C) 2006-2009 NXP B.V., All Rights Reserved.
  This source code and any compilation or derivative thereof is the proprietary
  information of NXP B.V. and is confidential in nature. Under no circumstances
  is this software to be  exposed to or placed under an Open Source License of
  any type without the expressed written permission of NXP B.V.
 *
 * \file          main.c
 *
 *                3
 *
 * \date          %modify_time%
 *
 * \brief         Sample application code of the NXP NT220X driver.
 *
 * REFERENCE DOCUMENTS :
 *                NT220X_Driver_User_Guide.pdf
 *
 * Detailed description may be added here.
 *
 * \section info Change Information
 *
*/


//*--------------------------------------------------------------------------------------
//* Include Standard files
//*--------------------------------------------------------------------------------------
#include "tmNxTypes.h"
#include "tmCompId.h"
#include "tmFrontEnd.h"
#include "tmbslFrontEndTypes.h"
#include "tmsysFrontEndTypes.h"

//*--------------------------------------------------------------------------------------
//* Include Driver files
//*--------------------------------------------------------------------------------------
#include "tmbslNT220HN.h"

#include <sys_config.h>
#include <retcode.h>
#include <types.h>

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal_task.h>

#include "tun_nt220x.h"

#define NT220X_PRINTF(...) //libc_printf

//#define NT220X_PRINTF libc_printf

static struct QAM_TUNER_CONFIG_EXT * nt220x_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 nt220x_tuner_cnt = 0;

static OSAL_ID tun_nt220x_mutex_arr[2] = {0,1};


//*--------------------------------------------------------------------------------------
//* Prototype of function to be provided by customer
//*--------------------------------------------------------------------------------------
tmErrorCode_t 	UserWrittenI2CRead(tmUnitSelect_t tUnit,UInt32 AddrSize, UInt8* pAddr,UInt32 ReadLen, UInt8* pData);
tmErrorCode_t 	UserWrittenI2CWrite (tmUnitSelect_t tUnit, UInt32 AddrSize, UInt8* pAddr,UInt32 WriteLen, UInt8* pData);
tmErrorCode_t 	UserWrittenWait(tmUnitSelect_t tUnit, UInt32 tms);
tmErrorCode_t 	UserWrittenPrint(UInt32 level, const char* format, ...);
tmErrorCode_t  	UserWrittenMutexInit(ptmbslFrontEndMutexHandle *ppMutexHandle);
tmErrorCode_t  	UserWrittenMutexDeInit( ptmbslFrontEndMutexHandle pMutex);
tmErrorCode_t  	UserWrittenMutexAcquire(ptmbslFrontEndMutexHandle pMutex, UInt32 timeOut);
tmErrorCode_t  	UserWrittenMutexRelease(ptmbslFrontEndMutexHandle pMutex);


//*--------------------------------------------------------------------------------------
//* Template of function to be provided by customer
//*--------------------------------------------------------------------------------------

//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenI2CRead
//* Object              : 
//* Input Parameters    : 	tmUnitSelect_t tUnit
//* 						UInt32 AddrSize,
//* 						UInt8* pAddr,
//* 						UInt32 ReadLen,
//* 						UInt8* pData
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
#define BURST_SZ 14
tmErrorCode_t UserWrittenI2CRead(tmUnitSelect_t tUnit,	UInt32 AddrSize, UInt8* pAddr,
UInt32 ReadLen, UInt8* pData)
{
   /* Variable declarations */
   tmErrorCode_t err = TM_OK;

/* Customer code here */
/* ...*/
	UINT8 data[BURST_SZ]; // every time, write 14 byte..
	INT32 RemainLen, BurstNum;
	INT32 i,j;

	INT32 result = 0;
	UINT8 u8_add = (0xc0 & 0xff);

	if (AddrSize != 1)
	{
		NT220X_PRINTF("NT220X error 1 !\n");
		
	}

	RemainLen = ReadLen % BURST_SZ; 
	if (RemainLen)
	{
		BurstNum = ReadLen / BURST_SZ; 
	}
	else
	{
		BurstNum = ReadLen / BURST_SZ - 1;
		RemainLen = BURST_SZ;
	}

	for ( i = 0 ; i < BurstNum; i ++ )
	{
		data[0] = pAddr[0]+i*BURST_SZ;

		result |= i2c_write_read(I2C_TYPE_SCB, u8_add, data,1, BURST_SZ);

		for ( j = 0 ; j < BURST_SZ ; j++  )
		{
			 pData[i * BURST_SZ + j ] = data[j];
		}

	}

	

	data[0] = pAddr[0]+BurstNum*BURST_SZ;
	result |= i2c_write_read(I2C_TYPE_SCB, u8_add, data, 1, RemainLen);
	for ( i = 0 ; i < RemainLen ; i++ )
	{
		pData[BurstNum * BURST_SZ + i ] = data[i];
	}

	if (result == SUCCESS)
	{
		err = TM_OK;
	}
	else
	{
		NT220X_PRINTF("NT220X error 2 !\n");
		err = !TM_OK;
	}

/* ...*/
/* End of Customer code here */

   return err;
}

//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenI2CWrite
//* Object              : 
//* Input Parameters    : 	tmUnitSelect_t tUnit
//* 						UInt32 AddrSize,
//* 						UInt8* pAddr,
//* 						UInt32 WriteLen,
//* 						UInt8* pData
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t UserWrittenI2CWrite (tmUnitSelect_t tUnit, 	UInt32 AddrSize, UInt8* pAddr,
UInt32 WriteLen, UInt8* pData)
{
   /* Variable declarations */
   tmErrorCode_t err = TM_OK;

/* Customer code here */
/* ...*/
	INT32 result = 0;
	UINT8 data[BURST_SZ+1]; // every time, write 14 byte..

	INT32 RemainLen, BurstNum;
	INT32 i,j;

	UINT8 u8_add = (0xc0 & 0xff);

	if (AddrSize != 1)
	{
		NT220X_PRINTF("NT220X error 3 !\n");
		
	}
	
	RemainLen = WriteLen % BURST_SZ; 
	if (RemainLen)
	{
		BurstNum = WriteLen / BURST_SZ; 
	}
	else
	{
		BurstNum = WriteLen / BURST_SZ - 1;
		RemainLen = BURST_SZ;
	}

	for ( i = 0 ; i < BurstNum; i ++ )
	{
		for ( j = 0 ; j < BURST_SZ ; j++  )
		{
			data[j+1]   = pData[i * BURST_SZ + j ];
		}
		data[0] = pAddr[0]+i*BURST_SZ;

		result |= i2c_write(I2C_TYPE_SCB, u8_add, data, BURST_SZ+1);

	}

	

	for ( i = 0 ; i < RemainLen ; i++ )
	{
		data[i+1]   = pData[BurstNum * BURST_SZ + i ];
		data[0] = pAddr[0]+BurstNum*BURST_SZ;
	}
	result |= i2c_write(I2C_TYPE_SCB, u8_add, data, RemainLen+1);

	if (result == SUCCESS)
	{
		err = TM_OK;
	}
	else
	{
		NT220X_PRINTF("NT220X error 4 !\n");
		err = !TM_OK;
	}

	
/* ...*/
/* End of Customer code here */

   return err;
}

//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenWait
//* Object              : 
//* Input Parameters    : 	tmUnitSelect_t tUnit
//* 						UInt32 tms
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t UserWrittenWait(tmUnitSelect_t tUnit, UInt32 tms)
{
   /* Variable declarations */
   tmErrorCode_t err = TM_OK;

/* Customer code here */
/* ...*/
	osal_task_sleep(tms);
/* ...*/
/* End of Customer code here */

   return err;
}

//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenPrint
//* Object              : 
//* Input Parameters    : 	UInt32 level, const char* format, ...
//* 						
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t 			UserWrittenPrint(UInt32 level, const char* format, ...)
{
   /* Variable declarations */
   tmErrorCode_t err = TM_OK;

/* Customer code here */
/* ...*/

/* ...*/
/* End of Customer code here */

   return err;
}

//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenMutexInit
//* Object              : 
//* Input Parameters    : 	ptmbslFrontEndMutexHandle *ppMutexHandle
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t  UserWrittenMutexInit(ptmbslFrontEndMutexHandle *ppMutexHandle)
{
   /* Variable declarations */
   tmErrorCode_t err = TM_OK;


/* Customer code here */
/* ...*/

	static UINT8 mux_cnt = 0;
/*	
	OSAL_ID tun_nt220x_mutex_id = OSAL_INVALID_ID;	//for i2c write and read mutec protection.

	tun_nt220x_mutex_id = osal_mutex_create();
	if(tun_nt220x_mutex_id == OSAL_INVALID_ID)	
	{
		NT220X_PRINTF("i2c mutex error\n");
	}

	tun_nt220x_mutex_arr[mux_cnt] = tun_nt220x_mutex_id;
*/	
	*ppMutexHandle = (void*)&tun_nt220x_mutex_arr[mux_cnt];

	mux_cnt += 1;
/* ...*/
/* End of Customer code here */

   return err;
}


//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenMutexDeInit
//* Object              : 
//* Input Parameters    : 	 ptmbslFrontEndMutexHandle pMutex
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t  UserWrittenMutexDeInit( ptmbslFrontEndMutexHandle pMutex)
{
   /* Variable declarations */
   tmErrorCode_t err = TM_OK;

/* Customer code here */
/* ...*/
//	osal_mutex_delete(*((OSAL_ID*)pMutex));
/* ...*/
/* End of Customer code here */

   return err;
}



//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenMutexAcquire
//* Object              : 
//* Input Parameters    : 	ptmbslFrontEndMutexHandle pMutex, UInt32 timeOut
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t  UserWrittenMutexAcquire(ptmbslFrontEndMutexHandle pMutex, UInt32 timeOut)
{
   /* Variable declarations */
   tmErrorCode_t err = TM_OK;

/* Customer code here */
/* ...*/
	//osal_mutex_lock(*((OSAL_ID*)pMutex), timeOut);

/* ...*/
/* End of Customer code here */

   return err;
}

//*--------------------------------------------------------------------------------------
//* Function Name       : UserWrittenMutexRelease
//* Object              : 
//* Input Parameters    : 	ptmbslFrontEndMutexHandle pMutex
//* Output Parameters   : tmErrorCode_t.
//*--------------------------------------------------------------------------------------
tmErrorCode_t  UserWrittenMutexRelease(ptmbslFrontEndMutexHandle pMutex)
{
   /* Variable declarations */
   tmErrorCode_t err = TM_OK;

/* Customer code here */
/* ...*/
	//osal_mutex_unlock(*((OSAL_ID*)pMutex));

/* ...*/
/* End of Customer code here */

   return err;
}

	
INT32 tun_nt220x_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	/* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (nt220x_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	tuner_dev_ptr = (struct QAM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QAM_TUNER_CONFIG_EXT));
	if((void*)tuner_dev_ptr == NULL)
		return ERR_FAILUE;

	MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct QAM_TUNER_CONFIG_EXT));	
	
	nt220x_dev_id[nt220x_tuner_cnt] = tuner_dev_ptr;
	*tuner_id = nt220x_tuner_cnt;

	nt220x_tuner_cnt ++;

	NT220X_PRINTF("%s start !\n", __FUNCTION__);
	
	return SUCCESS;
}

INT32 tun_nt220x_status(UINT32 tuner_id, UINT8 *lock)
{
	INT32 result = 0;
	tmErrorCode_t err;
	tmbslFrontEndState_t PLLLockMaster = tmbslFrontEndStateUnknown;	
	
	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	if ((tuner_id>=nt220x_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = nt220x_dev_id[tuner_id];

   /* Get NT220X Master PLL Lock status */
	err = tmbslNT220xGetLockStatus(0, &PLLLockMaster);
	if(err != TM_OK)
	{
		*lock = 0;
		result = !SUCCESS;
	}
	else
	{
		if (tmbslFrontEndStateLocked == PLLLockMaster)
		{
			*lock = 1;
		}
		else
		{
			*lock = 0;
		}
		result = SUCCESS;
	}
   
	return result;
}

INT32 tun_nt220x_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd)	
{	

	INT32   result;
	tmErrorCode_t err = TM_OK;
	static INT8 tun_status=0;
	tmNT220xStandardMode_t stdModeMaster;

	UInt32 uRFMaster = freq*1000;
	UInt32 uIFMaster = 0;

	if (0==tun_status)
	{
	   /* Variable declarations */
	   tmbslFrontEndDependency_t sSrvTunerFunc;
	/* Low layer struct set-up to link with user written functions */
	   sSrvTunerFunc.sIo.Write             = UserWrittenI2CWrite;
	   sSrvTunerFunc.sIo.Read              = UserWrittenI2CRead;
	   sSrvTunerFunc.sTime.Get             = Null;
	   sSrvTunerFunc.sTime.Wait            = UserWrittenWait;
	   sSrvTunerFunc.sDebug.Print          = UserWrittenPrint;
	   sSrvTunerFunc.sMutex.Init           = UserWrittenMutexInit;
	   sSrvTunerFunc.sMutex.DeInit         = UserWrittenMutexDeInit;
	   sSrvTunerFunc.sMutex.Acquire        = UserWrittenMutexAcquire;
	   sSrvTunerFunc.sMutex.Release        = UserWrittenMutexRelease;
	   sSrvTunerFunc.dwAdditionalDataSize  = 0;
	   sSrvTunerFunc.pAdditionalData       = Null;
	   
	   /* NT220X Master Driver low layer setup */
	   err = tmbslNT220xInit(0, &sSrvTunerFunc);
	   if(err != TM_OK)
	   {
	   		NT220X_PRINTF("tmNT220x init failed\n");
	       return err;
	   }
	   /* NT220X Master Hardware initialization */
	   do
	   {
	   err = tmbslNT220xReset(0);
	   if(err != TM_OK)
	   		{
	   			NT220X_PRINTF("tmNT220x reset again\n");
				osal_task_sleep(100);
	   		}
	   }while(err != TM_OK);
	   
	   if(err != TM_OK)
	   {
	   		NT220X_PRINTF("tmNT220x reset failed\n");
	       return err;
	   }
	   
	   /* NT220X Master Hardware power state */
	   
	   err = tmbslNT220xSetPowerState(0, tmNT220x_PowerNormalMode);

	   if(err != TM_OK)
	    {
	    
		NT220X_PRINTF("tmNT220x_error 1\n");
	       return err;
	    }

	#if (1)		
	   /* NT220X Master standard mode */
	#ifdef NT220x_IF5M_BW8M
	   stdModeMaster = tmNT220x_QAM_8MHz;
	#else
	  stdModeMaster = tmNT220x_QAM_6MHz;		
	#endif
	   err = tmbslNT220xSetStandardMode(0, stdModeMaster);

	   if(err != TM_OK)
	   {
	   	NT220X_PRINTF("tmNT220x_error 2\n");
	       return err;
	   }
	#endif
		tun_status = 1;
	}
#if (0)
/*
   if(sym<= 4500)
   {
		stdModeMaster = tmNT220x_QAM_6MHz;	
		NT220X_PRINTF("tmNT220x_QAM_6MHz\n");
			
   }
   else
   {	
   		stdModeMaster = tmNT220x_QAM_8MHz;
		NT220X_PRINTF("tmNT220x_QAM_8MHz\n");
   }
*/   
   stdModeMaster = tmNT220x_QAM_6MHz;	
	NT220X_PRINTF("tmNT220x_QAM_6MHz\n");
	
   /* NT220X Master standard mode */
   err = tmbslNT220xSetStandardMode(0, stdModeMaster);
   NT220X_PRINTF("tmNT220x_QAM,stdModeMaster=%d\n",stdModeMaster);	

   if(err != TM_OK)
   {
   		NT220X_PRINTF("set tmNT220x_QAM failed\n");
       return err;
	}
#endif
	
   /* NT220X Master RF frequency */
   err = tmbslNT220xSetRf(0, uRFMaster);// tUnit-> 0: master 1:slaver,uRF(hz)

   if(err != TM_OK)
   {
   		NT220X_PRINTF("tmbslNT220xSetRf failed! uRFMaster=%d\n",uRFMaster);
       return err;
   }

	osal_task_sleep(100); // we don't know wait how long, just give a norminal value.   
   /* Get NT220X Master IF */
/*
   err = tmbslNT220xGetIF(0, &uIFMaster);
   if(err != TM_OK)
       return err;
*/
	if (err !=TM_OK) 
	{
		result= ERR_FAILED;
	}
	else
	{
		result = SUCCESS;
	}
	return result;

}

