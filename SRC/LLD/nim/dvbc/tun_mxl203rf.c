/*****************************************************************************
*    Copyright (C)2007 Ali Corporation. All Rights Reserved.
*
*    File:    tun_mxl203rf.c
*
*    Description:    Source file of MAXLINEAR mxl203rf TUNER.
*    History:
*           Date          Author                 Version          Reason
*	    ============	=============	=========	=================
*   4.17.2008	     David.Deng	      Ver 0.1	     Create file.
*****************************************************************************/
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>


#include "tun_mxl203rf.h"
//#define MXL_DEBUG

#define NIM_PRINTF(...)
#define NIM_MXL203RF_PRINTF(...)  //libc_printf

static MxL203RF_TunerConfigS mxl203rf_TunerConfig[2];
static BOOL bMxl_Tuner_Inited[2];

#ifdef MXL_DEBUG
UINT8 reg_dump[256];
#endif

static struct QAM_TUNER_CONFIG_EXT * mxl203rf_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 mxl203rf_tuner_cnt = 0;


INT32 tun_mxl203rf_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	/* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (mxl203rf_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	tuner_dev_ptr = (struct QAM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QAM_TUNER_CONFIG_EXT));
	if((void*)tuner_dev_ptr == NULL)
		return ERR_FAILUE;

	MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct QAM_TUNER_CONFIG_EXT));	
	
	mxl203rf_dev_id[mxl203rf_tuner_cnt] = tuner_dev_ptr;
	*tuner_id = mxl203rf_tuner_cnt;
	mxl203rf_TunerConfig[mxl203rf_tuner_cnt].tuner_id = mxl203rf_tuner_cnt;
	bMxl_Tuner_Inited[mxl203rf_tuner_cnt] = FALSE;


	mxl203rf_tuner_cnt ++;

	return SUCCESS;
}

/*****************************************************************************
* INT32 tun_mxl203rf_status(UINT8 *lock)
*
* Tuner read operation
*
* Arguments:
*  Parameter1: UINT8 *lock		: Phase lock status
*
* Return Value: INT32			: Result
*****************************************************************************/

INT32 tun_mxl203rf_status(UINT32 tuner_id, UINT8 *lock)
{
	BOOL IfLock;

	if(tuner_id >= MAX_TUNER_SUPPORT_NUM)
		return ERR_FAILUE;
	
	MxL_RFSynth_Lock_Status(&mxl203rf_TunerConfig[tuner_id], &IfLock);
	
	//*lock = 1;
	*lock = ( IfLock == TRUE ) ? 1 : 0; 

	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_mxl203rf_control(UINT32 freq, UINT8 bandwidth,UINT8 AGC_Time_Const,UINT8 *data,UINT8 _i2c_cmd)
*
* Tuner write operation
*
* Arguments:
*  Parameter1: UINT32 freq		: Synthesiser programmable divider
*  Parameter2: UINT8 bandwidth		: channel bandwidth
*  Parameter3: UINT8 AGC_Time_Const	: AGC time constant
*  Parameter4: UINT8 *data		: 
*
* Return Value: INT32			: Result
*****************************************************************************/
INT32 tun_mxl203rf_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd)
{
	unsigned long dwRF_Freq_KHz;
	MxL203RF_BW_MHz mxl_BW;
	MxL_ERR_MSG Status;
	
	if(tuner_id >= MAX_TUNER_SUPPORT_NUM)
		return ERR_FAILUE;
	
	if ( bMxl_Tuner_Inited[tuner_id] == FALSE )
	{
		NIM_MXL203RF_PRINTF("MXL203RF CTL_INIT,ADDR=%x\n",mxl203rf_dev_id[tuner_id]->cTuner_Base_Addr);
		mxl203rf_TunerConfig[tuner_id].I2C_Addr =(MxL203RF_I2CAddr)(mxl203rf_dev_id[tuner_id]->cTuner_Base_Addr);//MxL_I2C_ADDR_96;
		mxl203rf_TunerConfig[tuner_id].Mode = MxL_MODE_CAB_STD;
		mxl203rf_TunerConfig[tuner_id].Xtal_Freq=MxL_XTAL_24_MHZ;		
		mxl203rf_TunerConfig[tuner_id].IF_Freq =MxL_IF_7_2_MHZ;// MxL_IF_4_57_MHZ;//MxL_IF_36_15_MHZ;
		mxl203rf_TunerConfig[tuner_id].IF_Spectrum = MxL_NORMAL_IF;
		mxl203rf_TunerConfig[tuner_id].ClkOut_Setting = MxL_CLKOUT_DISABLE;
		mxl203rf_TunerConfig[tuner_id].ClkOut_Amp = MxL_CLKOUT_AMP_10;
		mxl203rf_TunerConfig[tuner_id].Xtal_Cap = MxL_XTAL_CAP_12_PF;

		if((Status = MxL_Tuner_Init(&mxl203rf_TunerConfig[tuner_id])) != MxL_OK )
		{
			return ERR_FAILUE;
		}
		
		bMxl_Tuner_Inited[tuner_id] = TRUE;
	}

	dwRF_Freq_KHz = freq;
	mxl_BW = MxL_BW_8MHz; //not use 'sym'

	if(  (Status = MxL_Tuner_RFTune(&mxl203rf_TunerConfig[tuner_id], dwRF_Freq_KHz*1000, mxl_BW)) != MxL_OK )
	{
		return ERR_FAILUE;
	}

//?#define MXL_DEBUG
#ifdef MXL_DEBUG
	UINT8 i;
	for(i = 0; i < 255; i++)
	{
		//soc_printf(&mxl203rf_TunerConfig[tuner_id], i, &reg_dump[i]);
	}
#endif	
	return SUCCESS;	
}




/******************************************************************************
**
**  Name: MxL_I2C_Write
**
**  Description:    I2C write operations
**
**  Parameters:    	
**					DeviceAddr	- mxl203rf Device address
**					pArray		- Write data array pointer
**					count		- total number of array
**
**  Returns:        0 if success
**
**  Revision History:
**
**   SCR      Date      Author  Description
**  -------------------------------------------------------------------------
**   N/A   12-16-2007   khuang initial release.
**
******************************************************************************/
#if defined(BURST_PAIR_SZ)
#error " BURST_PAIR_SZ already defined!"
#else
//#define BURST_PAIR_SZ   6
#define BURST_SZ 12
#endif
UINT32 MxL_I2C_Write(UINT8 DeviceAddr, UINT8* pArray, UINT32 count)
{

	int status = 0;
	int i,j;

	UINT32 tuner_id = 0;
	
	UINT8 szBuffer[BURST_SZ];
	UINT16 BurstNum, RemainLen;

	NIM_MXL203RF_PRINTF("Write count =%d\n",count);

	for (i=0;i<count; i++)
	{
		NIM_MXL203RF_PRINTF("Write MXL203RF[%x]=%x\n",i, pArray[i]);
	}
//	mxl203rf_dev_id[tuner_id]->i2c_type_id=0;
//	mxl203rf_dev_id[tuner_id]->cTuner_Base_Addr=MxL_I2C_ADDR_96;

	if ( !count ) return 0;


	RemainLen = count % BURST_SZ; 
	if ( RemainLen )
	{
		BurstNum = count / BURST_SZ; 
	}
	else
	{
		BurstNum = (count / BURST_SZ) - 1;
		RemainLen = BURST_SZ;
	}

	for ( i = 0 ; i < BurstNum; i ++ )
	{
		for ( j = 0 ; j < BURST_SZ ; j++  )
		{
			szBuffer[j]   = pArray[i * BURST_SZ + j ];             // addr
			//szBuffer[j+1] = pArray[i * BURST_PAIR_SZ + j +1];	// value

		}
		
		status+= i2c_write(mxl203rf_dev_id[tuner_id]->i2c_type_id, mxl203rf_dev_id[tuner_id]->cTuner_Base_Addr, szBuffer, BURST_SZ);
		if(status!=0)
		{	
			NIM_MXL203RF_PRINTF("MXL203RF WR1 err=%d\n",status);
			;
		}
	}

	

	for ( i = 0 ; i < RemainLen ; i++ )
	{
		szBuffer[i]   = pArray[BurstNum * BURST_SZ + i ];         // addr 
	}

	status += i2c_write(mxl203rf_dev_id[tuner_id]->i2c_type_id, mxl203rf_dev_id[tuner_id]->cTuner_Base_Addr, szBuffer, RemainLen);
	if(status!=0)
	{		
			NIM_MXL203RF_PRINTF("MXL203RF WR2 err=%d\n",status);
			;
	}
	
	return status;

}

/******************************************************************************
**
**  Name: MxL_I2C_Read
**
**  Description:    I2C read operations
**
**  Parameters:    	
**					DeviceAddr	- mxl203rf Device address
**					Addr		- register address for read
**					*Data		- data return
**
**  Returns:        0 if success
**
**  Revision History:
**
**   SCR      Date      Author  Description
**  -------------------------------------------------------------------------
**   N/A   12-16-2007   khuang initial release.
**
******************************************************************************/
UINT32 MxL_I2C_Read(UINT8 DeviceAddr, UINT8 Addr, UINT8* mData)
{
	int status = 0;

	UINT8 Read_Cmd[2]; 

	UINT32 tuner_id = 0;

	/* read step 1. accroding to mxl203rf driver API user guide. */
	Read_Cmd[0] = 0xFB;
	Read_Cmd[1] = Addr;
	status = i2c_write(mxl203rf_dev_id[tuner_id]->i2c_type_id, mxl203rf_dev_id[tuner_id]->cTuner_Base_Addr, Read_Cmd, 2);

	status += i2c_read(mxl203rf_dev_id[tuner_id]->i2c_type_id, mxl203rf_dev_id[tuner_id]->cTuner_Base_Addr, mData, 1);

	NIM_MXL203RF_PRINTF("Read MXL203RF[%x]=%x\n",Addr, *mData);

	return status;
}

/******************************************************************************
**
**  Name: MxL_Delay
**
**  Description:    Delay function in milli-second
**
**  Parameters:    	
**					mSec		- milli-second to delay
**
**  Returns:        0
**
**  Revision History:
**
**   SCR      Date      Author  Description
**  -------------------------------------------------------------------------
**   N/A   12-16-2007   khuang initial release.
**
******************************************************************************/
void MxL_Delay(UINT32 mSec)
{
	osal_task_sleep(mSec);
}

