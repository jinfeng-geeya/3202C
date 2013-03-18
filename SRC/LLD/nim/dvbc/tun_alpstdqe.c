/*****************************************************************************
*    Copyright (C)2007 Ali Corporation. All Rights Reserved.
*
*    File:    tun_alpstdqe.c
*
*    Description:    This file contains alpstdqe basic function in LLD. 
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  20071022	  Penghui		 Ver 0.1	Create file.
*	
*****************************************************************************/

#include <sys_config.h>
#include <retcode.h>
#include <types.h>

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal_task.h>

#include "tun_alpstdqe.h"


#define ALPSTDQE_PRINTF libc_printf

static UINT32 tun_alpstdqe_mutex_id;

static struct QAM_TUNER_CONFIG_EXT * ALPSTDQE_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 ALPSTDQE_tuner_cnt = 0;


INT32 tun_alpstdqe_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	/* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (ALPSTDQE_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	tuner_dev_ptr = (struct QAM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QAM_TUNER_CONFIG_EXT));
	if((void*)tuner_dev_ptr == NULL)
		return ERR_FAILUE;

	MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct QAM_TUNER_CONFIG_EXT));	
	
	ALPSTDQE_dev_id[ALPSTDQE_tuner_cnt] = tuner_dev_ptr;
	*tuner_id = ALPSTDQE_tuner_cnt;

	ALPSTDQE_tuner_cnt ++;

	return SUCCESS;
}

INT32 tun_alpstdqe_status(UINT32 tuner_id, UINT8 *lock)
{
	INT32 result;
	UINT8 data =0;	

	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	if ((tuner_id>=ALPSTDQE_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = ALPSTDQE_dev_id[tuner_id];

	result = i2c_read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->cTuner_Base_Addr, &data, 1);
	*lock = (UINT8)((data>>6)&0x01);	

	return result;
}

INT32 tun_alpstdqe_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd)	
{	

	INT32   result;
	UINT16 Npro;
	UINT8   data[5];


	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	if ((tuner_id>=ALPSTDQE_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = ALPSTDQE_dev_id[tuner_id];


	ALPSTDQE_PRINTF("%s: feq, sym:  %d, %d !\n", __FUNCTION__, freq, sym);


//==========Evaluated to control byte=============
	Npro=(freq+36125)*10/625; 
	data[0] = (UINT8) ((Npro>>8)&0x7F);
	data[1] = (UINT8) (Npro&0xFF); 
	data[2] = 0x8b;	/*62.5KHz*/

	 if (freq < 153000) 
		data[3] = 0x60;
	 if ((freq >= 153000 )&&(freq < 430000)) 
	 	data[3] = 0xa2;
	 if (freq >= 430000 ) 
	 	data[3] = 0xaa; 

	 data[4] = 0xc6;


//==========Write to tuner register===============
       if(AGC_Time_Const == SLOW_TIMECST_AGC)
	  	osal_task_sleep(50);

       result = ERR_FAILUE;

	if(_i2c_cmd==_1st_i2c_cmd)
	{
		result = i2c_write(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->cTuner_Base_Addr, data,5);
	}

     return result;



}

