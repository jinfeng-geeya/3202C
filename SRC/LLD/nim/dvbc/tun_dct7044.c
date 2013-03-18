/*****************************************************************************
*    Copyright (C)2006 Ali Corporation. All Rights Reserved.
*
*    File:    tun_dct7044.c
*
*    Description:    This file contains dct7044 basic function in LLD. For Xuguang Half Tuner DCT-7044.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  20070417	  Penghui		 Ver 0.1	Create file.
*	
*****************************************************************************/

#include <sys_config.h>
#include <retcode.h>
#include <types.h>

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal_task.h>

#include "tun_dct7044.h"


#define DCT7044_PRINTF(...) 		do{}while(0) //libc_printf

static UINT32 tun_dct7044_mutex_id;

static struct QAM_TUNER_CONFIG_EXT * stDCT7044_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 stDCT7044_tuner_cnt = 0;


INT32 tun_dct7044_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	/* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (stDCT7044_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	tuner_dev_ptr = (struct QAM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QAM_TUNER_CONFIG_EXT));
	if((void*)tuner_dev_ptr == NULL)
		return ERR_FAILUE;

	MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct QAM_TUNER_CONFIG_EXT));	
	
	stDCT7044_dev_id[stDCT7044_tuner_cnt] = tuner_dev_ptr;
	*tuner_id = stDCT7044_tuner_cnt;

	stDCT7044_tuner_cnt ++;

	return SUCCESS;
}

INT32 tun_dct7044_status(UINT32 tuner_id, UINT8 *lock)
{
	INT32 result;
	UINT8 data =0;	

	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	if ((tuner_id>=stDCT7044_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = stDCT7044_dev_id[tuner_id];

	//result = tuner_dev_ptr->Tuner_Read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->cTuner_Base_Addr, &data, 1);
	result = i2c_read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->cTuner_Base_Addr, &data, 1);
	*lock = (UINT8)((data>>6)&0x01);	

	return result;
}

INT32 tun_dct7044_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd)	
{	

	INT32   result;
	UINT16 Npro;
	UINT32 OscFreq;
	UINT8   StepFreq,RefDivRatio,TunCrystal;
	UINT8   data[4];
	UINT8   P2_0;
	UINT8   CP, RSAB;

	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	if ((tuner_id>=stDCT7044_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = stDCT7044_dev_id[tuner_id];


	DCT7044_PRINTF("tuner_id, feq, sym: %d, %d, %d !\n", tuner_id, freq, sym);
       OscFreq = freq + tuner_dev_ptr->wTuner_IF_Freq;
	StepFreq = tuner_dev_ptr->cTuner_Step_Freq;
	RefDivRatio = tuner_dev_ptr->cTuner_Ref_DivRatio;
	TunCrystal = tuner_dev_ptr->cTuner_Crystal ;


//==========DATA[1:0] for Npro==============

	// StepFreq also equal to (TunCrystal * 1000 / RefDivRatio)
	Npro=(OscFreq+StepFreq/2)* RefDivRatio/ (TunCrystal * 1000);	


//==========DATA[2]  for CP, RSA ,RSB=========
	CP = 0;//tuner_dev_ptr->cTuner_Charge_Pump;

	if (RefDivRatio == 128)
		{
		RSAB = 0x01;
		}
	else if (RefDivRatio == 80)
		{
		RSAB = 0x00;
		}
	else if (RefDivRatio == 64)
		{
		RSAB = 0x03;
		}
	else 
		{
		RSAB = 0x00;
		}
	
	//(Ref_divRatio*Step_Freq)=(80*50)=(64*62.5)=(24*166.7)=(31.25*128)
	
//==========DATA[3]  for P2_0=================

       if (OscFreq >= 87000 && OscFreq <185000)
	   	P2_0 = 0x02;
	   else if (OscFreq >= 185000 && OscFreq < 465000)
	   	P2_0 = 0x01;
	   else if (OscFreq >= 465000 && OscFreq <= 894000)
	   	P2_0 = 0x04;
	   else 
	   	P2_0 = 0x04;


//==========Evaluated to control byte=============
	data[0] = (UINT8) ((Npro>>8)&0x7F);
	data[1] = (UINT8) (Npro&0xFF); 
       data[2] = (UINT8)(0x80 | (CP <<6) |  (RSAB << 1));
	data[3] = (UINT8)(0x00|P2_0);

//==========Write to tuner register===============
       if(AGC_Time_Const == SLOW_TIMECST_AGC)
	  	osal_task_sleep(20);

       result = ERR_FAILUE;

	if(_i2c_cmd==_1st_i2c_cmd)
	{
		//result = tuner_dev_ptr->Tuner_Write(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->cTuner_Base_Addr, data,4);
		result = i2c_write(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->cTuner_Base_Addr, data,4);
	}

     return result;
}


