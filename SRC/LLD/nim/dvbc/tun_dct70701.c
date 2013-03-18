/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    tun_dct70701.c
*
*    Description:    This file contains dct70701 basic function in LLD. For Tuner DCT-70701.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  20080111	  Penghui		 Ver 0.1	Create file.
*	
*****************************************************************************/

#include <sys_config.h>
#include <retcode.h>
#include <types.h>

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal_task.h>

#include "tun_dct70701.h"


#define DCT70701_PRINTF(...) do{}while(0)//libc_printf

static UINT32 tun_dct70701_mutex_id;

static struct QAM_TUNER_CONFIG_EXT * stDCT70701_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 stDCT70701_tuner_cnt = 0;


INT32 tun_dct70701_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	/* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (stDCT70701_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	tuner_dev_ptr = (struct QAM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QAM_TUNER_CONFIG_EXT));
	if((void*)tuner_dev_ptr == NULL)
		return ERR_FAILUE;

	MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct QAM_TUNER_CONFIG_EXT));	
	
	stDCT70701_dev_id[stDCT70701_tuner_cnt] = tuner_dev_ptr;
	*tuner_id = stDCT70701_tuner_cnt;

	stDCT70701_tuner_cnt ++;

	return SUCCESS;
}

INT32 tun_dct70701_status(UINT32 tuner_id, UINT8 *lock)
{
	INT32 result;
	UINT8 data =0;	

	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	if ((tuner_id>=stDCT70701_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = stDCT70701_dev_id[tuner_id];

	//result = tuner_dev_ptr->Tuner_Read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->cTuner_Base_Addr, &data, 1);
	result = i2c_read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->cTuner_Base_Addr, &data, 1);
	*lock = (UINT8)((data>>6)&0x01);	

	return result;
}

INT32 tun_dct70701_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd)	
{	

	INT32   result;
	UINT16 Npro;
	UINT32 OscFreq;
	UINT8   StepFreq,RefDivRatio,TunCrystal;
	UINT8   data[5];

	UINT8 ATP2_0, RS2_0,BS4_1;

	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	if ((tuner_id>=stDCT70701_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = stDCT70701_dev_id[tuner_id];


	DCT70701_PRINTF("Tuner feq, sym: %d, %d !\n",freq, sym);
       OscFreq = freq + tuner_dev_ptr->wTuner_IF_Freq;
	StepFreq = tuner_dev_ptr->cTuner_Step_Freq;
	RefDivRatio = tuner_dev_ptr->cTuner_Ref_DivRatio;
	TunCrystal = tuner_dev_ptr->cTuner_Crystal ;


//==========DATA[1:0] for Npro==============

	// StepFreq also equal to (TunCrystal * 1000 / RefDivRatio)
	Npro=(OscFreq+StepFreq/2)* RefDivRatio/ (TunCrystal * 1000);	


//==========DATA[2]  for ATP2_0, RS2_0=========

	if (RefDivRatio == 24) //166.667
		RS2_0 = 0x00;
	else if (RefDivRatio == 28)//142.857
		RS2_0 = 0x01;
	else if (RefDivRatio == 50) //80
		RS2_0 = 0x02;
	else if (RefDivRatio == 64) // 62.5
		RS2_0 = 0x03;
	else if (RefDivRatio == 128) // 31.25
		RS2_0 = 0x04;
	else if (RefDivRatio == 80) // 50
		RS2_0 = 0x05;
	else
		RS2_0 = 0x03;

	//ATP2_0 = 0x01;
	//ATP2_0 = 0x07; // two AGC
//joey 20080504. for ATP value from app layer.
	ATP2_0 = tuner_dev_ptr->cTuner_AGC_TOP;

//==========DATA[3]  for BS4_1=================
       if (OscFreq >= 85000 && OscFreq <185000)
	   	BS4_1 = 0x01;
	   else if (OscFreq >= 185000 && OscFreq < 465000)
	   	BS4_1 = 0x06;
	   else if (OscFreq >= 465000 && OscFreq <= 896000)
	   	BS4_1 = 0x0C;
	   else 
	   	BS4_1 = 0x0C;


//==========Evaluated to control byte=============
	data[0] = (UINT8) ((Npro>>8)&0x7F);
	data[1] = (UINT8) (Npro&0xFF); 
       data[2] = (UINT8)(0x80 | (ATP2_0 <<3) | RS2_0);
	data[3] = (UINT8)(0x00|BS4_1);
	data[4] = (UINT8)(0xC3);

//==========Write to tuner register===============
//       if(AGC_Time_Const == SLOW_TIMECST_AGC)
	  

       result = ERR_FAILUE;

	if(_i2c_cmd==_1st_i2c_cmd)
	{
		result = i2c_write(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->cTuner_Base_Addr, data,5);
	}
	/* for tuner lock stable 20100811(shuozhou)*/
	//osal_task_sleep(200);

	/*for faster lock, sleep at least 50ms, normal 100ms*/
	osal_task_sleep(50);
	
    return result;
}


