/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    tun_cd1616lf.c
*
*    Description:    This file contains cd1616lf basic function in LLD. 
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  20090302		Magic Yang		 Ver 0.1	Create file.
*	
*****************************************************************************/

#include <sys_config.h>
#include <retcode.h>
#include <types.h>

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal_task.h>

#include "tun_cd1616lf.h"


#define CD1616LF_PRINTF(...) do{}while(0)//libc_printf

static struct QAM_TUNER_CONFIG_EXT * cd1616lf_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 cd1616lf_tuner_cnt = 0;


INT32 tun_cd1616lf_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	/* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (cd1616lf_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	tuner_dev_ptr = (struct QAM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QAM_TUNER_CONFIG_EXT));
	if((void*)tuner_dev_ptr == NULL)
		return ERR_FAILUE;

	MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct QAM_TUNER_CONFIG_EXT));	
	
	cd1616lf_dev_id[cd1616lf_tuner_cnt] = tuner_dev_ptr;
	*tuner_id = cd1616lf_tuner_cnt;

	cd1616lf_tuner_cnt ++;

	return SUCCESS;
}

INT32 tun_cd1616lf_status(UINT32 tuner_id, UINT8 *lock)
{
	INT32 result;
	UINT8 data =0;	

	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	if ((tuner_id>=cd1616lf_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = cd1616lf_dev_id[tuner_id];

	result = i2c_read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->cTuner_Base_Addr, &data, 1);
	*lock = (UINT8)((data>>6)&0x01);
	CD1616LF_PRINTF("%s ::Tuner Status ::POR %d :FL %d \n", __FUNCTION__, (UINT8)((data>>7)&0x01),*lock);

	return result;
}

INT32 tun_cd1616lf_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd)	
{	

	INT32   result;
	UINT16 Npro;
	UINT32 OscFreq;
	UINT8   StepFreq,RefDivRatio,TunCrystal;
	UINT8   RSab =0, AGCn, SP3_1=0; 
	UINT8   data[4];


	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	if ((tuner_id>=cd1616lf_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = cd1616lf_dev_id[tuner_id];


	CD1616LF_PRINTF("%s: feq, sym:  %d, %d !\n", __FUNCTION__, freq, sym);


//==========Calculate control bytes=============
	OscFreq = freq + tuner_dev_ptr->wTuner_IF_Freq;
	StepFreq = tuner_dev_ptr->cTuner_Step_Freq;
	RefDivRatio = tuner_dev_ptr->cTuner_Ref_DivRatio;
	TunCrystal = tuner_dev_ptr->cTuner_Crystal;

	Npro=(UINT16)((OscFreq+(StepFreq/2)) * RefDivRatio / (TunCrystal * 1000));	

	if (RefDivRatio == 80) //50.0
		RSab = 0x00;
	else if (RefDivRatio == 128)//31.25
		RSab = 0x01;
	else if (RefDivRatio == 64) //62.5
		RSab = 0x03;
	else
		CD1616LF_PRINTF("RS config error!!\n");

	AGCn = tuner_dev_ptr->cTuner_AGC_TOP;
	

	if (freq < 167000) 
		SP3_1 = 0x01;
	else if ((freq >= 167000) && (freq < 454000)) 
	 	SP3_1 = 0x02;
	else if  (freq >= 454000) 
	 	SP3_1 = 0x04;

//==========Evaluated to control bytes=============
	data[0] = (UINT8)((Npro>>8)&0x7F);
	data[1] = (UINT8)(Npro&0xFF); 
	data[2] = (0xc0 | (0x06&(RSab<<1)));
	data[3] = (((AGCn<<3)&0x18) | (SP3_1&0x07) );
	

//==========Write to tuner register===============


	result = i2c_write(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->cTuner_Base_Addr, data,4);
CD1616LF_PRINTF("DB1::0x%x DB2::0x%x CB::0x%x BB::0x%x\n",data[0],data[1],data[2],data[3]);
CD1616LF_PRINTF("result= %d!!\n",result);

	//wait for pll and agc lock.
	osal_task_sleep(50);

     return result;
}

