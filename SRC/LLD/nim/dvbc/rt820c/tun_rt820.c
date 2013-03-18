/*****************************************************************************
*    Copyright (C)2008 Ali Corporation. All Rights Reserved.
*
*    File:    tun_rt820.c
*
*    Description:    This file contains alpstdae basic function in LLD. 
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  20080520		Trueve Hu		 Ver 0.1	Create file.
*	
*****************************************************************************/

#include <sys_config.h>
#include <retcode.h>
#include <types.h>

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <bus/i2c/i2c.h>
#include <osal/osal_task.h>

#include "tun_rt820.h"
#include "R820C.h"


#define R828_PRINTF(...)	do{}while(0) //libc_printf

static struct QAM_TUNER_CONFIG_EXT * rt820_dev_id[MAX_TUNER_SUPPORT_NUM] = {NULL};
static UINT32 rt820_tuner_cnt = 0;
static UINT8 rt820_reg_ini[2]={0,0};
static UINT8 rt820_bw[2]={0,0};


INT32 tun_rt820_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config)
{
	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;
	/* check Tuner Configuration structure is available or not */
	if ((ptrTuner_Config == NULL) || (rt820_tuner_cnt>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;

	tuner_dev_ptr = (struct QAM_TUNER_CONFIG_EXT *)MALLOC(sizeof(struct QAM_TUNER_CONFIG_EXT));
	if((void*)tuner_dev_ptr == NULL)
		return ERR_FAILUE;

	MEMCPY(tuner_dev_ptr, ptrTuner_Config, sizeof(struct QAM_TUNER_CONFIG_EXT));	
	
	rt820_dev_id[rt820_tuner_cnt] = tuner_dev_ptr;
	*tuner_id = rt820_tuner_cnt;

	rt820_reg_ini[rt820_tuner_cnt]=0;		// status bit for initialized rt820 register
	rt820_tuner_cnt ++;

	return SUCCESS;
}

INT32 tun_rt820_status(UINT32 tuner_id, UINT8 *lock)
{
	INT32 result;
	UINT8 data =0;	

	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	if ((tuner_id>=rt820_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = rt820_dev_id[tuner_id];

	// rt820 has no PLL lock indicator, only need to wait 100ms, enough for lock
	//osal_task_sleep(100);

	//result = i2c_read(tuner_dev_ptr->i2c_type_id, tuner_dev_ptr->cTuner_Base_Addr, &data, 1);
	*lock = 1;

	return SUCCESS;
}

INT32 tun_rt820_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd)	
{	

	INT32 result;
	UINT8 tmp_bw=0;
	UINT8 bandwidth = 8;

	UINT32 tmp_tick_1 = 0;
	UINT32 tmp_tick_2 = 0;

	UINT32 tmp_dur = 0;
	


	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	if ((tuner_id>=rt820_tuner_cnt) || (tuner_id>=MAX_TUNER_SUPPORT_NUM))
		return ERR_FAILUE;
	tuner_dev_ptr = rt820_dev_id[tuner_id];


	if(0==rt820_reg_ini[tuner_id])
	{

		R828_Init(); // direct setting. should be same with the tuner driver.
		//R828_GPIO(HI_SIG);
		rt820_bw[tuner_id] = 0;

		rt820_reg_ini[tuner_id]=1;
	}
/*
	if (sym < 5200) //now, only devide as 2 mode, 6m & 8m.
	{
		bandwidth = 6;
	}
	else
	{
		bandwidth = 8;
	}
	
	
	if (bandwidth != rt820_bw[tuner_id]) // do standard setting.
	{
		//R828_Standard(DVB_C_8M);

		rt820_bw[tuner_id] = bandwidth;
	}
*/

	//R828_Standard(DVB_C_8M);
	//R828_SetFrequency(freq, 5070, tmp_bw, AIR_IN, DVB_C_5070, SIGLE_IN);

	R828_Set_Info tun_info;
	tun_info.RT_Input = SIGLE_IN;
	tun_info.RF_KHz = freq;


	if (tuner_dev_ptr->wTuner_IF_Freq > 4500) // 8m mode, for 6M/8M mode tmp code.
	{
		R828_SetStandard(DVB_C_8M);
		tun_info.R828_Standard = DVB_C_8M;
	}
	else
	{
		R828_SetStandard(DVB_C_6M);
		tun_info.R828_Standard = DVB_C_6M;
	}


		//R828_SetStandard(DVB_C_6M);
		//tun_info.R828_Standard = DVB_C_6M;

		//tmp_tick_1 = osal_get_tick();
		//R828_SetStandard(DVB_C_8M);
		//tun_info.R828_Standard = DVB_C_8M;
		//tmp_tick_2 = osal_get_tick();

		//tmp_dur = tmp_tick_2 -tmp_tick_1;

		//soc_printf("R820C step 1: %d !\n", tmp_dur);

	R828_SetFrequency(tun_info, NORMAL_MODE);

	//tmp_tick_1 = osal_get_tick();

	//tmp_dur = tmp_tick_1 - tmp_tick_2;
	//soc_printf("R820C step 2: %d !\n", tmp_dur);
	
	
	return SUCCESS;

}

/*
void R828_Delay_MS(int ms)
{
	osal_task_sleep(ms);
}
*/

void R828_Delay_MS(int ms)
{
	int i = 0;
	for (i=0; i<=ms; i++)
	{
		osal_delay(1000);		
	}
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
#define BURST_SZ 13

//joey, 20111207, for RT820C/R828.
typedef struct _I2C_OP_TYPE
{
	//UINT8 DevAddr;
	//UINT8 Data[50];
	UINT8 Data[51]; // 1 more than 50.
	UINT8 Len;
}I2C_OP_TYPE;

R828_ErrCode write_i2c(I2C_OP_TYPE *I2C_Info)
{
	INT32 result = 0;
	UINT8 data[BURST_SZ+1]; // every time, write 14 byte..

	INT32 RemainLen, BurstNum;
	INT32 i,j;

	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	tuner_dev_ptr = rt820_dev_id[0];


	//UINT8 u8_add = I2C_Info->DevAddr;
	UINT8 u8_add = 0x34;
	UINT8 len = (I2C_Info->Len)-1;
	UINT8 *buff = &(I2C_Info->Data[1]);

	RemainLen = len % BURST_SZ; 
	if (RemainLen)
	{
		BurstNum = len / BURST_SZ; 
	}
	else
	{
		BurstNum = len / BURST_SZ - 1;
		RemainLen = BURST_SZ;
	}

	for ( i = 0 ; i < BurstNum; i ++ )
	{
		for ( j = 0 ; j < BURST_SZ ; j++  )
		{
			data[j+1]   = buff[i * BURST_SZ + j ];
		}

		data[0] = I2C_Info->Data[0] + BURST_SZ*i;
		result |= i2c_write(tuner_dev_ptr->i2c_type_id, u8_add, data, BURST_SZ+1);
	}

	

	for ( i = 0 ; i < RemainLen ; i++ )
	{
		data[i+1]   = buff[BurstNum * BURST_SZ + i ];
	}

	data[0] = I2C_Info->Data[0] + BURST_SZ*BurstNum;
	
	result |= i2c_write(tuner_dev_ptr->i2c_type_id, u8_add, data, RemainLen+1);
	//libc_printf("rt820 i2c write: %d !\n", result);

	if (result == SUCCESS)
	{
		return RT_Success;
	}
	else
	{
		return RT_Fail;
	}
}

R828_ErrCode read_i2c(I2C_LEN_TYPE *I2C_Info)
{
	//UINT8 data[4]; // every time, write 14 byte..
	INT32 i,j;

	INT32 result = 0;

	struct QAM_TUNER_CONFIG_EXT * tuner_dev_ptr = NULL;

	tuner_dev_ptr = rt820_dev_id[0];
	
	//UINT8 u8_add = I2C_Info->DevAddr | 0x01;
	UINT8 u8_add = 0x34 | 0x01;
	UINT8 len = I2C_Info->Len;
	UINT8 *data = I2C_Info->Data;

//joey, 20111207, for RT820C(R828 support).
	if (len > 8)
	{
		return RT_Fail;
	}

	result |= i2c_scb_read(tuner_dev_ptr->i2c_type_id, u8_add, data, len);

/*
	libc_printf("rt820 i2c read: %d !\n", result);

	for (i=0;i<len;i++)
	{
		libc_printf("rt820 data[%d]: 0x%x !\n", i, data[i]);
		
	}
*/

	if (result == SUCCESS)
	{
		return RT_Success;
	}
	else
	{
		return RT_Fail;
	}
}


R828_ErrCode I2C_Write_Len(I2C_LEN_TYPE *I2C_Info)
{
	I2C_OP_TYPE tmp_info;
	UINT8 i;

	for (i=0;i< I2C_Info->Len;i++)
	{
		tmp_info.Data[i+1] = I2C_Info->Data[i];
	}
	//tmp_info.DevAddr = I2C_Info->DevAddr;
	tmp_info.Data[0] = I2C_Info->RegAddr;
	tmp_info.Len = I2C_Info->Len+1;
	
	return write_i2c(&tmp_info);
}

UINT8 R828_Convert(UINT8 InvertNum)
{
	UINT8 ReturnNum = 0;
	UINT8 AddNum    = 0x80;
	UINT8 BitNum    = 0x01;
	UINT8 CuntNum   = 0;

	for(CuntNum = 0;CuntNum < 8;CuntNum ++)
	{
		if(BitNum & InvertNum)
			ReturnNum += AddNum;

		AddNum = AddNum >> 1;
		BitNum = BitNum << 1;
	}

	//libc_printf("rt820 convert: from 0x%2x to 0x%2x !\n", InvertNum, ReturnNum);

	return ReturnNum;
}

R828_ErrCode I2C_Read_Len(I2C_LEN_TYPE *I2C_Info)
{
	INT32 result;
	UINT8 tmp_cnt;
	I2C_Info->RegAddr  = 0x00;

	result = read_i2c(I2C_Info);

	if(RT_Success != result)
		return RT_Fail;

	for(tmp_cnt = 0;tmp_cnt < I2C_Info->Len;tmp_cnt ++)
	{
		 I2C_Info->Data[tmp_cnt] = R828_Convert(I2C_Info->Data[tmp_cnt]);
	}
	
	return RT_Success;
}

R828_ErrCode I2C_Write(I2C_TYPE *I2C_Info)
{
	I2C_OP_TYPE tmp_info;

	//tmp_info.DevAddr = I2C_Info->DevAddr;
	tmp_info.Len       = 2;
	tmp_info.Data[0]      = I2C_Info->RegAddr;
	tmp_info.Data[1]      = I2C_Info->Data;

	return write_i2c(&tmp_info);
}

