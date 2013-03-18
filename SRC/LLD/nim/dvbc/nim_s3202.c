/*****************************************************************************
*    Copyright (C)2007 Ali Corporation. All Rights Reserved.
*
*    File:    nim_s3202.c
*
*    Description:    Source file in LLD.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.20070524       Penghui      Ver 0.1       Create file.
*	2.20071022	   Penghui      Ver 0.2       Delete the redundant code
*	3.20080418		Joey		Ver 1.0		Fromal release for S3202B V1.0
*	4.20100822		Joey		Ver 1.14. 	Optimize CR parameter for easy lock.
*	5.20110826		Joey		Ver 1.15. 	Optimize PN parameter for optimization.
*	6.20120406		Joey		Ver 1.16. 	for low-IF tuner(silicon tuner) support.
*	7.20120421		Artie		Ver 1.17. 	Add nim_s3202_register_RF_Level_cb function for Customize RF LEVEL
*	8.20120618		Joey		Ver 1.18. 	Fix the M3202C+mxl203 16/32 lock but can't search program issue, 
*										add nim_Tuner_Get_rf_levell Function which support tuner driver return  RF LEVEL value.
*	9.20120808		Joey		Ver 1.19. 	Fix the M3202C some time CR hard to lock, when modulator has burst carrier jitter.
*****************************************************************************/
#include <sys_config.h>
#include <retcode.h>
#include <types.h>
#include <osal/osal.h> 
#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <hal/hal_gpio.h>
#include <bus/i2c/i2c.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>

#include <hld/nim/nim_tuner.h>		
#include <bus/tsi/tsi.h>

#include "nim_s3202.h"


/* Need to modify here for different perpose */
//===============
#ifndef QAM_FPGA_USAGE
#define	QAM_FPGA_USAGE	SYS_FUNC_OFF
#endif

#ifndef QAM_ONLY_USAGE
#define QAM_ONLY_USAGE  SYS_FUNC_OFF
#endif

#if (QAM_FPGA_USAGE == SYS_FUNC_ON)
#define	QAM_WORK_MODE	QAM_ONLY
#define	QAM_TEST_FUNC	SYS_FUNC_ON
#elif (QAM_ONLY_USAGE == SYS_FUNC_ON)
#define	QAM_WORK_MODE	QAM_ONLY
#define	QAM_TEST_FUNC	SYS_FUNC_OFF
#else
#define	QAM_WORK_MODE	QAM_SOC
#define	QAM_TEST_FUNC	SYS_FUNC_OFF
#endif

#ifdef TUNER_I2C_BYPASS
#define	I2C_BYPASS
#endif
//===============


//joey 20080414 for 0x19/0x1a register debug.
#if (QAM_FPGA_USAGE == SYS_FUNC_ON)
#define sys_ic_get_rev_id()	(IC_REV_0 + 1)
#elif (QAM_ONLY_USAGE == SYS_FUNC_ON)
#if (SYS_DEM_M3200C_USAGE  ==SYS_FUNC_ON)
#define sys_ic_get_rev_id()	(IC_REV_0 + 3)
#else
#define sys_ic_get_rev_id()	(IC_REV_0 + 2)
#endif
#endif

/* Name for the tuner, the last character must be Number for index */
static char nim_s3202_name[HLD_MAX_NAME_SIZE] = "NIM_QAM_S3202";
static struct S3202_Lock_Info S3202_CurChannelInfo;
static OSAL_ID dem_s3202_mutex_id = OSAL_INVALID_ID;
static OSAL_ID dem_s3202_task_id = OSAL_INVALID_ID;

static UINT32 BER_COUNTS=0;
static UINT32 PER_COUNTS=0;
static BOOL    BER_VALID=FALSE;
//joey 20080504. add in ber&per ready count.
static UINT32 acc_ber_cnt = 0;

static UINT16 aci_delfreq =  0;
static BOOL channel_change_en = FALSE;

//joey 20080504. For ACI and max level AGC patch.
static UINT8 if_sml_val1; //for strong signal
static UINT8 if_def_val2;//for aci signal

static UINT16 rf_sml_th1;
static UINT16 rf_big_th2;

//joey 20080512. for RF-agc enable switch.
static BOOL rf_agc_en = TRUE;

//joey 20080504. move here for global usage.
//joey 20080422. for init freq offset usage.
//joey 20120406, for low-IF case, not add-in freq offset.
static INT16 init_foffset = 0;


#if (QAM_FPGA_USAGE == SYS_FUNC_ON)
static BOOL draw_osd_en = TRUE;
#else
static BOOL draw_osd_en = FALSE;
#endif

static nim_rf_level_func_t nim_s3202_RF_Level_cb = NULL;

//=============================================================
#if (QAM_WORK_MODE == QAM_ONLY)
static UINT32 dem_i2c_id = I2C_TYPE_SCB0;
INT32 nim_s3202_read(UINT8 bMemAdr, UINT8 *pData, UINT8 bLen)
{
	UINT8 chip_adr;
	INT32 bRet;

#ifndef SYS_DEM_BASE_ADDR
	chip_adr = 0x40;
#else
	chip_adr = SYS_DEM_BASE_ADDR;
#endif
	osal_mutex_lock(dem_s3202_mutex_id, OSAL_WAIT_FOREVER_TIME);
	bRet = i2c_write(dem_i2c_id, chip_adr, &bMemAdr, 1);
	if(bRet == SUCCESS)
		bRet = i2c_read(dem_i2c_id, chip_adr, pData, bLen);
	osal_mutex_unlock(dem_s3202_mutex_id);
	return bRet;
}
INT32 nim_s3202_write(UINT8 bMemAdr, UINT8 *pData, UINT8 bLen)
{
	UINT8 chip_adr;
	INT32 i,bRet;

#ifndef SYS_DEM_BASE_ADDR
	chip_adr = 0x40;
#else
	chip_adr = SYS_DEM_BASE_ADDR;
#endif
	UINT8 bTemp[bLen+1];

	for(i = 1;i < bLen+1;i++)
		bTemp[i] = pData[i-1];
	bTemp[0] = bMemAdr;

	osal_mutex_lock(dem_s3202_mutex_id, OSAL_WAIT_FOREVER_TIME);
	bRet = i2c_write(dem_i2c_id, chip_adr, bTemp, bLen+1);
	osal_mutex_unlock(dem_s3202_mutex_id);
	return bRet;
}

#elif (QAM_WORK_MODE == QAM_SOC)
INT32 nim_s3202_read(UINT8 bMemAdr, UINT8 *pData, UINT8 bLen)
{
	INT32 i;
	
	for (i = 0; i < bLen; i++)
	{
		*(pData+i) = NIM_S3202_GET_BYTE((S3202_QAM_SOC_BASE_ADDR + bMemAdr + i));
	}
	return SUCCESS;
}
INT32 nim_s3202_write(UINT8 bMemAdr, UINT8 *pData, UINT8 bLen)
{
	INT32 i;
	
	for (i = 0; i < bLen; i++)
	{
		NIM_S3202_SET_BYTE((S3202_QAM_SOC_BASE_ADDR + bMemAdr + i),  *(pData+i));
	}
	return SUCCESS;
}
#endif





//=============================================================
#if (QAM_TEST_FUNC==SYS_FUNC_ON)

/*These functions come from the win_nimreg.c file   */
extern void nimreg_ber_refresh (UINT32 err_count,UINT32 RsUbc);
extern void nimRegPrint();
extern void display_dynamic_vision();
extern BOOL nimRegFuncFlag(UINT8 indx, BOOL *val, BOOL dir);


/* These functions is software test or patch*/
static void nim_s3201_R2FFT(INT16 *FFT_I_256, INT16 *FFT_Q_256)
{
	 INT16	L,k,i,j;
	 INT16    Wn_num,p;

	 INT16    Wn_imag_index,Wn_real_index;
	 INT16     neg_flag;
	 INT16     Wn_imag,  Wn_real;
	 
        INT32     real_tmp_a1,real_tmp_b1,real_tmp_3;
        INT32      imag_tmp_a1,imag_tmp_b1,imag_tmp_3;

       INT16 TR,TI;
	INT16 tempR,tempI;
       INT16 dataR[256];
       INT16 dataI[256];

	for (i=0;i<256;i++)
	{
	  k=0;
	  for(j=0;j<8;j++)
	  {
	  	k=k<<1;
		k+=((i>>j) & 0x1);//address change for 256 FFT
	  }
	  dataR[k] =*(FFT_I_256+i);
	  dataI[k] =*(FFT_Q_256+i);
	}
	i =0;
	j =0;
	k =0;

/************** following code FFT *******************/
		
	for(L=1;L<=8;L++)  {
		Wn_num = 1;
        Wn_num  = Wn_num  <<(L-1); //unmber of Wn in this layer

		for(j=0;j<Wn_num;j++) {
			p=1;
			p=p<<(8-L);
			p=p*j;
			if(p>64)
				Wn_imag_index = 128-p;
			else
                Wn_imag_index = p;

            Wn_imag = sin_table[Wn_imag_index];

			if(p>64) {
				Wn_real_index = p-64;
				neg_flag = 1;
			} else {
                Wn_real_index = 64-p;
				neg_flag =0;
			}
				
			if(neg_flag)
               Wn_real = -sin_table[Wn_real_index];
			else
               Wn_real = sin_table[Wn_real_index];

			for(k=j;k<256;k=k+2*Wn_num)  {           //butterfly
      	  
				TR=dataR[k]; TI=dataI[k]; 
				tempR=dataR[k+Wn_num]; tempI=dataI[k+Wn_num];
                 
				real_tmp_a1 = tempR * Wn_real;

               		real_tmp_b1 = tempI *Wn_imag;
				real_tmp_3  = (real_tmp_a1 + real_tmp_b1 + 256)>>9;
                
				imag_tmp_a1 = tempR * Wn_imag;
				imag_tmp_b1 = tempI *Wn_real; 
				imag_tmp_3  =(imag_tmp_a1  - imag_tmp_b1 + 256)>>9;

				dataR[k]=TR + real_tmp_3;
                dataR[k+Wn_num]=TR- real_tmp_3;

				dataI[k]=TI - imag_tmp_3;                
                dataI[k+Wn_num]=TI+imag_tmp_3;

				 *(FFT_I_256+k) = dataR[k];
				 *(FFT_Q_256+k) = dataI[k];
				 *(FFT_I_256+k+Wn_num) = dataR[k+Wn_num];
				 *(FFT_Q_256+k+Wn_num) = dataI[k+Wn_num];
			}
		}

	}
}

static void nim_s3202_mon_catch_ad_data(struct nim_device *dev)
{
	UINT8 data;
	UINT16 i;
	UINT8 data_reg[3];
	INT16 DI[256];
	INT16 DQ[256];
	UINT32 Spectrum;
	UINT8 fft_buf_full =0;
	UINT8 time_out;
	UINT16 sum1,sum2;
	UINT8 lock = 0, Tuner_Retry = 0;
	
	UINT32 freq = S3202_CurChannelInfo.Frequency;
	struct nim_s3202_private *dev_priv;	
	dev_priv = (struct nim_s3202_private *)dev->priv;
	
//Step 1: set receiver to IDLE status, reset Interrupt indicator, and set WORK_MODE

	data = 0x80;
	nim_s3202_write(NIM_S3202_CONTROL1,&data,1);
	data = 0x00;
	nim_s3202_write(NIM_S3202_INTERRUPT_EVENTS,&data,1);
	data = 0x0F;
	nim_s3202_write(NIM_S3202_INTERRUPT_MASK,&data,1);

	nim_s3202_read(NIM_S3202_FSM1,&data,1);
	data = (data & 0xcf) | 0x10;
	nim_s3202_write(NIM_S3202_FSM1,&data,1); // set to catch mode.

	data = 0x40;
	nim_s3202_write(NIM_S3202_CONTROL1,&data,1);

//Setp 2: set tuner frequency 
	do
	{
		if(Tuner_Retry>2)
		{
			S3202_PRINTF("ERROR! Tuner Lock Fail\n");
			lock = 0;
			break;
		}
		Tuner_Retry++;
		// Fast config tuner
		if(dev_priv->nim_Tuner_Control(dev_priv->tuner_id,freq,0,FAST_TIMECST_AGC,_1st_i2c_cmd)==ERR_FAILUE)		
		{
			S3202_PRINTF("Fast Config tuner failed !\n");
		}

		// Slow config tuner
		if(dev_priv->nim_Tuner_Control(dev_priv->tuner_id,freq,0,SLOW_TIMECST_AGC, _1st_i2c_cmd)==ERR_FAILUE)
		{
			S3202_PRINTF("Slow Config tuner failed!\n");
		}

		// Read status
		if(dev_priv->nim_Tuner_Status(dev_priv->tuner_id,&lock)==ERR_FAILUE)
		{
			S3202_PRINTF("ERROR! Tuner Read Status Fail\n");
		}
		
		S3202_PRINTF("catch AD Tuner Lock Times=%d, *lock=%d !!\n",Tuner_Retry,lock);
	}while(0 == lock);

//Step 3: Wait buffer full interrupt

	// First, we should ensure that the AGC is locked
	time_out = 0;
	while(1)
	{
		osal_task_sleep(10);
		if(time_out > 100)
		{
			//S3202_PRINTF("AGC lock time_out !!\n");
			goto out;
		}
		time_out += 1;
		nim_s3202_read(NIM_S3202_MONITOR1, &data, 1);
		if((data & 0x01) == 0x01)
		{
			//S3202_PRINTF("AGC locked!\n");
			break;
		}
	}
	
	nim_s3202_read(NIM_S3202_FSM11,&data,1);
	data = (data & 0xE7) | 0x08;
	nim_s3202_write(NIM_S3202_FSM11,&data,1); 

	// Second, wait for the BUF is fulled
	time_out = 0;
	while(!fft_buf_full)
	{
		if(time_out > 50)
		{	
			//S3202_PRINTF("FFT buffer full time out!!\n");
			goto out;
		}
		time_out += 1;
		nim_s3202_read(NIM_S3202_MONITOR1,&data,1);
		fft_buf_full = (data >> 6) & 0x01;
		osal_task_sleep( 100 );
	}
	
//Step 4: Get data from registers
	for (i = 0; i< 256; i++ )
	{
		data = i;
		nim_s3202_write(NIM_S3202_MONITOR2,&data,1);
		nim_s3202_read(NIM_S3202_MONITOR3,data_reg,3);
		DI[i] = (INT16)((data_reg[2]&0x0f)<<6 | (data_reg[1]&0xfc)>>2);
		DQ[i] = (INT16)((data_reg[1]&0x03)<<8 | (data_reg[0]));
		//S3202_PRINTF("ram[%03d] I,Q= 0x%03x, 0x%03x \n",i,DI[i],DQ[i]);
		// DI , DQ  from 10bit extern to 16bit for sign data.
		if ((DI[i] & 0x200) == 0x200)	DI[i]  = DI[i]  | 0xfc00;
		if ((DQ[i] & 0x200) == 0x200)	DQ[i] = DQ[i] | 0xfc00;
		
	}

	nim_s3201_R2FFT(DI, DQ);

	sum1 = 0;
	sum2 = 0;
	if (draw_osd_en == TRUE)
	{
		drawSquare(100, 270, 256, 150, 87);
		for (i = 0 ; i < 150 ; i ++)
			OSD_DrawFill(228, 270 + i, 1, 1, 0xF1, NULL);
	}

	for (i = 0 ; i< 256; i++)
	{
		Spectrum =	Log10Times100_L ((UINT32)(DI[i] * DI[i]) + (UINT32)(DQ[i] *DQ[i]))/10;
		//S3202_PRINTF(" %03d, DI = 0x%03x, DQ = 0x%03x, Result = %d \n",i , DI[i], DQ[i],Spectrum);
		if (i >= 68 & i <= 87)
			sum1 = sum1 + Spectrum;
		if (i >= 167 & i <= 186) 
			sum2 = sum2 + Spectrum;
		if (draw_osd_en == TRUE)
			OSD_DrawFill(100+i, 245 + Spectrum*2, 1, 2, 0xF2, NULL);
	}

	sum1 = sum1/20;
	sum2 = sum2/20;

	if ((sum1 -sum2) > 10)
		aci_delfreq = 300;
	else	
		aci_delfreq = 0;

	S3202_PRINTF(" Sum1 = %d, Sum2 = %d \n", sum1, sum2);

out:
	S3202_PRINTF("\n");
//Step 5: After catch data, should set the mode from " catch mode" to "normal mode".
	nim_s3202_read(NIM_S3202_FSM1,&data,1);
	data = (data & 0xcf) | 0x00;
	nim_s3202_write(NIM_S3202_FSM1,&data,1);
	
	nim_s3202_read(NIM_S3202_FSM11,&data,1);
	data = (data & 0xE7) | 0x10;
	nim_s3202_write(NIM_S3202_FSM11,&data,1); 

	data = 0x80;
	nim_s3202_write(NIM_S3202_CONTROL1,&data,1);
	data = 0x40;
	nim_s3202_write(NIM_S3202_CONTROL1,&data,1);

	
	return;
}
#endif

static void nim_s3202_mon_catch_ad_data_loop(struct nim_device *dev)
{
	UINT8 data;
	UINT16 i;
	UINT8 data_reg[3];
	INT16 DI[256];
	INT16 DQ[256];
	UINT32 Spectrum;
	UINT8 fft_buf_full =0;
	UINT8 time_out;
	UINT16 sum1,sum2;
	UINT8 lock = 0, Tuner_Retry = 0;
	
		
	UINT32 freq = S3202_CurChannelInfo.Frequency;
	struct nim_s3202_private *dev_priv;	
	dev_priv = (struct nim_s3202_private *)dev->priv;
	
//Step 1: set receiver to IDLE status, reset Interrupt indicator, and set WORK_MODE

	data = 0x80;
	nim_s3202_write(NIM_S3202_CONTROL1,&data,1);
	data = 0x00;
	nim_s3202_write(NIM_S3202_INTERRUPT_EVENTS,&data,1);
	data = 0x0F;
	nim_s3202_write(NIM_S3202_INTERRUPT_MASK,&data,1);

	nim_s3202_read(NIM_S3202_FSM1,&data,1);
	data = (data & 0xcf) | 0x10;
	nim_s3202_write(NIM_S3202_FSM1,&data,1); // set to catch mode.

	data = 0x40;
	nim_s3202_write(NIM_S3202_CONTROL1,&data,1);

//Setp 2: set tuner frequency 
#if 0
	do
	{
		if(Tuner_Retry>2)
		{
			S3202_PRINTF("ERROR! Tuner Lock Fail\n");
			lock = 0;
			break;
		}
		Tuner_Retry++;
		// Fast config tuner
		if(dev_priv->nim_Tuner_Control(dev_priv->tuner_id,freq,0,FAST_TIMECST_AGC,_1st_i2c_cmd)==ERR_FAILUE)		
		{
			S3202_PRINTF("Fast Config tuner failed !\n");
		}

		// Slow config tuner
		if(dev_priv->nim_Tuner_Control(dev_priv->tuner_id,freq,0,SLOW_TIMECST_AGC, _1st_i2c_cmd)==ERR_FAILUE)
		{
			S3202_PRINTF("Slow Config tuner failed!\n");
		}

		// Read status
		if(dev_priv->nim_Tuner_Status(dev_priv->tuner_id,&lock)==ERR_FAILUE)
		{
			S3202_PRINTF("ERROR! Tuner Read Status Fail\n");
		}
		
		S3202_PRINTF("catch AD Tuner Lock Times=%d, *lock=%d !!\n",Tuner_Retry,lock);
	}while(0 == lock);
#endif
//Step 3: Wait buffer full interrupt

	// First, we should ensure that the AGC is locked
	time_out = 0;
	while(1)
	{
		osal_task_sleep(10);
		if(time_out > 100)
		{
			//S3202_PRINTF("AGC lock time_out !!\n");
			goto out_1;
		}
		time_out += 1;
		nim_s3202_read(NIM_S3202_MONITOR1, &data, 1);
		if((data & 0x01) == 0x01)
		{
			//S3202_PRINTF("AGC locked!\n");
			break;
		}
	}
	
	nim_s3202_read(NIM_S3202_FSM11,&data,1);
	data = (data & 0xE7) | 0x08;
	nim_s3202_write(NIM_S3202_FSM11,&data,1); 

	// Second, wait for the BUF is fulled
	time_out = 0;
	while(!fft_buf_full)
	{
		if(time_out > 50)
		{	
			//S3202_PRINTF("FFT buffer full time out!!\n");
			goto out_1;
		}
		time_out += 1;
		nim_s3202_read(NIM_S3202_MONITOR1,&data,1);
		fft_buf_full = (data >> 6) & 0x01;
		osal_task_sleep( 100 );
	}

	
//Step 4: Get data from registers
	for (i = 0; i< 256; i++ )
	{
		data = i;
		nim_s3202_write(NIM_S3202_MONITOR2,&data,1);
		nim_s3202_read(NIM_S3202_MONITOR3,data_reg,3);
		DI[i] = (INT16)((data_reg[2]&0x0f)<<6 | (data_reg[1]&0xfc)>>2);
		DQ[i] = (INT16)((data_reg[1]&0x03)<<8 | (data_reg[0]));


		M3202_PRINTF("%d, %d \n",DI[i],DQ[i]);

		// DI , DQ  from 10bit extern to 16bit for sign data.
		if ((DI[i] & 0x200) == 0x200)	DI[i]  = DI[i]  | 0xfc00;
		if ((DQ[i] & 0x200) == 0x200)	DQ[i] = DQ[i] | 0xfc00;
		
	}
	
	
#if 0
	nim_s3201_R2FFT(DI, DQ);

	sum1 = 0;
	sum2 = 0;
	if (draw_osd_en == TRUE)
	{
		drawSquare(100, 270, 256, 150, 87);
		for (i = 0 ; i < 150 ; i ++)
			OSD_DrawFill(228, 270 + i, 1, 1, 0xF1, NULL);
	}

	for (i = 0 ; i< 256; i++)
	{
		Spectrum =	Log10Times100_L ((UINT32)(DI[i] * DI[i]) + (UINT32)(DQ[i] *DQ[i]))/10;
		//S3202_PRINTF(" %03d, DI = 0x%03x, DQ = 0x%03x, Result = %d \n",i , DI[i], DQ[i],Spectrum);
		if (i >= 68 & i <= 87)
			sum1 = sum1 + Spectrum;
		if (i >= 167 & i <= 186) 
			sum2 = sum2 + Spectrum;
		if (draw_osd_en == TRUE)
			OSD_DrawFill(100+i, 245 + Spectrum*2, 1, 2, 0xF2, NULL);
	}

	sum1 = sum1/20;
	sum2 = sum2/20;

	if ((sum1 -sum2) > 10)
		aci_delfreq = 300;
	else	
		aci_delfreq = 0;

	S3202_PRINTF(" Sum1 = %d, Sum2 = %d \n", sum1, sum2);
#endif

out_1:
	S3202_PRINTF("\n");
//Step 5: After catch data, should set the mode from " catch mode" to "normal mode".
	nim_s3202_read(NIM_S3202_FSM1,&data,1);
	data = (data & 0xcf) | 0x00;
	nim_s3202_write(NIM_S3202_FSM1,&data,1);
	
	nim_s3202_read(NIM_S3202_FSM11,&data,1);
	data = (data & 0xE7) | 0x10;
	nim_s3202_write(NIM_S3202_FSM11,&data,1); 

	data = 0x80;
	nim_s3202_write(NIM_S3202_CONTROL1,&data,1);
	data = 0x40;
	nim_s3202_write(NIM_S3202_CONTROL1,&data,1);

	
	return;
}
static INT32 nim_s3202_monitor_agc_status(struct nim_device *dev, UINT16 *cir)
{
	UINT8 i;
	UINT8 data[2];

	UINT32 cur_time;
	static UINT32 last_time;

	static UINT8 first_time = 1;

	UINT16 rf_gain_sum, tmp_rf;
	static BOOL reset_once= FALSE;

	if (channel_change_en == TRUE)
	{
		first_time = 1;
		reset_once = FALSE;
		return SUCCESS;
	}

	if (first_time == 1)
	{
		last_time = osal_get_tick();// the unit is 1ms.
		first_time = 0;
	}

	//check time first.
	cur_time = osal_get_tick();
	if (cur_time - last_time <200)
	{
		//time is not arrive, still polling.
		return SUCCESS;
	}
	else
	{
		//time is arrive, do work. update last time.
		last_time = cur_time;
	}

	i = 0;
	rf_gain_sum = 0;
	while (i <16)
	{
		if (channel_change_en == TRUE)
		{
			first_time = 1;
			reset_once = FALSE;
			return SUCCESS;
			
		}

		osal_task_sleep(10);
		nim_s3202_read(0x15, data, 2);
		tmp_rf = ((data[0]>>2 | (data[1]&0x0f)<<6) + 512) & 0x3ff;
		rf_gain_sum += tmp_rf;
		i++;
	}

	rf_gain_sum = rf_gain_sum>>6; //use this, we can reduce the complex.

	if (channel_change_en == TRUE)
	{
		first_time = 1;
		reset_once = FALSE;
		return SUCCESS;
		
	}
	
	if (rf_gain_sum< rf_sml_th1) //so-called TH1.
	{	
		nim_s3202_read(0x12, data, 1);
		if (data[0] > if_sml_val1)
		{
			data[0] = if_sml_val1;
			nim_s3202_write(0x12, data, 1);

			reset_once = FALSE;
			
			S3202_PRINTF("Reach Max sensitivity once!\n ");
		}
	}
	else if (rf_gain_sum > rf_big_th2) // so-called TH2.
	{
		nim_s3202_read(0x12, data, 1);
		if ((data[0] < if_def_val2) && (reset_once == FALSE))
		{
			data[0] = if_def_val2;
			nim_s3202_write(0x12, data, 1);

			data[0] = 0x80;
			nim_s3202_write(0x00, data, 1);

			data[0] = 0x40;
			nim_s3202_write(0x00, data, 1);

			reset_once = TRUE;

			S3202_PRINTF("take over point recovey! \n");
			
		}
	}
	

	return SUCCESS;
}
static INT32 nim_s3202_monitor_agc0a_loop(struct nim_device *dev, UINT16 *agc0a_loop)
{
	struct nim_s3202_private *dev_priv;
	UINT8 data;

	static UINT8 read_ber_cnt = 0;
	static UINT8 ber_k5_cnt = 0;
	static UINT8 ber_k1_cnt = 0;
	static UINT8 per_p5_cnt = 0;
	static UINT8 first_cal = 1;
	static UINT32 old_acc_ber_cnt = 0;
	static UINT8 tr_tout_cnt = 0;
	static UINT8 cma_tout_cnt = 0;
	static UINT8 tr_tout_flg = 0;
	static UINT8 cma_tout_flg = 0;
	UINT8 new_cr0a = 0;
	UINT8 agc_ref;
		
	UINT32 curt_time = 0;
	static UINT32 old_time = 0; 
	
	static UINT8 fsm_trans = 1;
	static UINT8 init_act = 1;

  	curt_time = osal_get_tick();
  	if (((curt_time >= old_time) && (curt_time - old_time >50)) \
		|| ((curt_time < old_time) && (((0xffffffff-old_time) + curt_time) > 50)))
  	{ 
		old_time = osal_get_tick();
  	}
	else
	{
		return SUCCESS;
	}

	//64Qam and above 6.5Msym 
	if ((S3202_CurChannelInfo.Modulation != 6) || (S3202_CurChannelInfo.SymbolRate < 6500))
	{
		if (first_cal ==0)
		{
			first_cal = 1;
		}
				
		return SUCCESS;
	}
	
	dev_priv = (struct nim_s3202_private *)dev->priv;
	agc_ref=dev_priv->tuner_config_data.AGC_REF;

	nim_s3202_read(0x56, &data, 1);
	data = data & 0x3f;
	
	if(data >= 0x1f)
	{
	
		if (fsm_trans == 1)	// flag to record and clear the time out cnt from unlock to lock 
		{
			
			tr_tout_cnt = 0;
			tr_tout_flg = 0;

			cma_tout_cnt = 0;
			cma_tout_flg = 0;

			fsm_trans = 0;
			
			nim_s3202_read(0x0a, &new_cr0a, 1);

			if (new_cr0a != agc_ref)
			{
				new_cr0a = agc_ref;
				nim_s3202_write(0x0a, &new_cr0a, 1);
			}
		}
		
	}
	else if (data == 0x00)//idle state.
	{	
		nim_s3202_read(0x0a, &new_cr0a, 1);
		if (new_cr0a != agc_ref)
		{
			new_cr0a = agc_ref;
			nim_s3202_write(0x0a, &new_cr0a, 1);
		}
		if (first_cal == 0)
		{
			first_cal = 1;
		}

		if (fsm_trans == 0)
		{
			fsm_trans = 1;
		}

		if (init_act == 1)
		{
			tr_tout_cnt = 0;
			tr_tout_flg = 0;

			cma_tout_cnt = 0;
			cma_tout_flg = 0;

			init_act = 0;

		}
	}
	else if (data < 0x03) //tr unlock.
	{
			
		tr_tout_cnt += 1;
		if (tr_tout_cnt > 7)
		{     
		       
			tr_tout_cnt = 0;
			cma_tout_cnt =0; 
			
			if (0 == tr_tout_flg)
			{
				nim_s3202_read(0x0a, &new_cr0a, 1);
				if (new_cr0a != 0x92)
				{
					new_cr0a = 0x92;
					nim_s3202_write(0x0a, &new_cr0a, 1);
				}
				
				tr_tout_flg=1;//modified by magic
			 }
			else if (1 == tr_tout_flg) 
			{
				nim_s3202_read(0x0a, &new_cr0a, 1);
				if (new_cr0a != 0x9a)
				{
					new_cr0a = 0x9a;
					nim_s3202_write(0x0a, &new_cr0a, 1);
				}
				tr_tout_flg=2;//modified by magic
			}
			else if (2 == tr_tout_flg) 
			{
				nim_s3202_read(0x0a, &new_cr0a, 1);
				if (new_cr0a != 0x9a)
				{
					new_cr0a = 0x9a;
					nim_s3202_write(0x0a, &new_cr0a, 1);
				}
				tr_tout_flg=3;//modified by magic
			}
			else if (3 == tr_tout_flg) 
			{
				nim_s3202_read(0x0a, &new_cr0a, 1);
				if (new_cr0a != 0x88)
				{
					new_cr0a = 0x88;
					nim_s3202_write(0x0a, &new_cr0a, 1);
				}
				tr_tout_flg=0;//modified by magic
			}

		}
		
		if (fsm_trans == FALSE)
		{
			fsm_trans = TRUE;
		}

		if (init_act == 0)
		{
			init_act = 1;
		}		
	}
	else if (data < 0x0f) //dd unlock.
	{
		
		cma_tout_cnt += 1;
		if (cma_tout_cnt > 6)
		{
			
			tr_tout_cnt = 0;
			cma_tout_cnt =0;
			
			if (0 == cma_tout_flg)
			{
				nim_s3202_read(0x0a, &new_cr0a, 1);
				if (new_cr0a != 0x92)
				{
					new_cr0a = 0x92;
					nim_s3202_write(0x0a, &new_cr0a, 1);
				}
				cma_tout_flg = 1;
			 }
			else if (1 == cma_tout_flg)	 
			{
				nim_s3202_read(0x0a, &new_cr0a, 1);
				if (new_cr0a != 0x9a)
				{
				new_cr0a = 0x9a;
				nim_s3202_write(0x0a, &new_cr0a, 1);
				}
				cma_tout_flg = 2;
			}
			else if (2== cma_tout_flg)	 	
			{
				nim_s3202_read(0x0a, &new_cr0a, 1);
				if (new_cr0a != 0x9a)
				{
				new_cr0a = 0x9a;
				nim_s3202_write(0x0a, &new_cr0a, 1);
				}
				cma_tout_flg = 3;
			}
			else if (3== cma_tout_flg)	 
			{
				nim_s3202_read(0x0a, &new_cr0a, 1);
				if (new_cr0a != 0x88)
				{
				new_cr0a = 0x88;
				nim_s3202_write(0x0a, &new_cr0a, 1);
				}
				cma_tout_flg = 0;
			}
			
		}
		
		if (fsm_trans == FALSE)
		{
			fsm_trans = TRUE;
		}

		if (init_act == 0)
		{
			init_act = 1;
		}		
			
	}

	return SUCCESS;
}
static INT32 nim_s3202_monitor_cr_loop(struct nim_device *dev, UINT16 *cr_loop)
{
	UINT8 data;
	static UINT8 read_ber_cnt = 0;
	static UINT8 ber_th_cnt_a = 0;
	static UINT8 ber_th_cnt_c = 0;
	static UINT8 per_1f_cnt = 0;
	static UINT8 first_cal = 1;
	static UINT32 old_acc_ber_cnt = 0;

	UINT32 cur_ber, cur_per;

	nim_s3202_read(0x56, &data, 1);
	if((data & 0x20) != 0x20)
	{
	/*
		nim_s3202_read(0xd8, &data, 1);
		if (data != 0x0a)
		{
			data = 0x0a;
			nim_s3202_write(0xd8, &data, 1);
		}
	*/
		if (first_cal == 0)
		{	
			first_cal = 1;
		}
		
		return SUCCESS;
	}

	if (first_cal == 1)
	{
		read_ber_cnt = 0;
		ber_th_cnt_a = 0;
		ber_th_cnt_c = 0;
		per_1f_cnt = 0;

		first_cal = 0;
	}
	
	if (old_acc_ber_cnt == acc_ber_cnt)
	{
		return SUCCESS;
	}
	else
	{
		old_acc_ber_cnt = acc_ber_cnt;
	}

	cur_ber = BER_COUNTS;
	cur_per = PER_COUNTS;
	read_ber_cnt += 1;


	if (cur_ber > 5000)
	{
		ber_th_cnt_a +=1;
	}
	else if (cur_ber < 1000)
	{
		ber_th_cnt_c +=1;
	}

	if (cur_per > 5)
	{
		per_1f_cnt += 1;
	}

	if (read_ber_cnt > 4)
	{
		if (ber_th_cnt_a > 3)
		{
			nim_s3202_read(0xd8, &data, 1);
			if (data != 0x0a)
			{
				data = 0x0a;
				nim_s3202_write(0xd8, &data, 1);
			}
				
			first_cal = 1;
			
			return SUCCESS;
		}
		else if ((read_ber_cnt <= 6) && (ber_th_cnt_c > 3) && (per_1f_cnt > 1)) //for Anti-Frequency Jitter
		{
				nim_s3202_read(0xd8, &data, 1);
				if (data != 0x0b)
				{
					data = 0x0b;
					nim_s3202_write(0xd8, &data, 1);
				}
				
				first_cal = 1;
			
			return SUCCESS;
		}
		else if (read_ber_cnt > 6)
		{
			if ((ber_th_cnt_c > 4) && (per_1f_cnt > 2))
			{
				nim_s3202_read(0xd8, &data, 1);
				if (data != 0x0b)
				{
					data = 0x0b;
					nim_s3202_write(0xd8, &data, 1);
				}
			}
			else
			{
			/*
				nim_s3202_read(0xd8, &data, 1);
				if (data != 0x0a)
				{
					data = 0x0a;
					nim_s3202_write(0xd8, &data, 1);
				}
			*/
			}

			first_cal =1;
			
			return SUCCESS;
		}
	}
	
	return SUCCESS;
}

static INT32 nim_s3202_monitor_fff_len(struct nim_device *dev, UINT16 *fff_len)
{
	UINT8 data;
	static UINT8 eq_st_flg = 0;
	static UINT8 eq_unlock_cnt = 0;
	static UINT8 fff_len_ch = 0;
	static UINT8 first_cal = 1;


	nim_s3202_read(0x56, &data, 1);
	if((data & 0x3f) >= 0x3f)
	{
	
		if (first_cal == 0)
		{	
			first_cal = 1;
		}
		
		return SUCCESS;
	}

	if (first_cal == 1)
	{
		eq_st_flg = 0;
		eq_unlock_cnt = 0;
		fff_len_ch = 0;

		first_cal = 0;
	}
	


	if (((data&0x3f) == 0x0f) && (eq_st_flg == 0))//eq start and not lock yet
	{
		eq_st_flg = 1;
	}
	else
	{
		if (((data&0x3f) < 0x0f) && (eq_st_flg == 1))
		{
			eq_st_flg = 0;
			eq_unlock_cnt += 1;

			//if (eq_unlock_cnt > 3)
			if (eq_unlock_cnt > 2)
			{
				nim_s3202_read(0xd8, &data, 1);              
				if (data != 0x0a)
				{
					data = 0x0a;
					nim_s3202_write(0xd8, &data, 1);
				}
				else
				{
					data = 0x0b;
					nim_s3202_write(0xd8, &data, 1);				
				}// add for anti-unlock issue under larger frequency jitter or larger IQ imbalance issue

				// add by magic 20090813 for moniter the fff patch	
				
				M3202_PRINTF("%s,      CRd8  =   0x%x \n",__FUNCTION__, data);
				
				if (eq_unlock_cnt > 3)
				{
					eq_unlock_cnt = 0;   //reset this counter when (eq_unlock_cnt >3)
					if (fff_len_ch == 0)
					{
						nim_s3202_read(0x28, &data, 1);
						data = (data&0xf8) |0x03;
						nim_s3202_write(0x28, &data, 1);

						fff_len_ch = 1;
					}
					else
					{
						nim_s3202_read(0x28, &data, 1);
						data = (data&0xf8) |0x04;
						nim_s3202_write(0x28, &data, 1);

						fff_len_ch = 0;
					}
				// add by magic 20090813 for moniter the fff patch	
				     
				 M3202_PRINTF("%s,      CR28  =   0x%x \n",__FUNCTION__, data);
					
				}
			}
		}
	}
	
	return SUCCESS;
}

//=============================================================
static void nim_s3202_sw_test_thread()
{
	UINT8 data[4];
	UINT8 i;
	static UINT32 curt_time,last_time[10];

	UINT32 sym_rate,rtp_sym,freq;
	UINT8 qam_order;
	UINT16 time_cons=200;
	UINT8 lock;
	UINT8 cnt=0;
		
	
	struct nim_device *dev = (struct nim_device*)dev_get_by_type(NULL,HLD_DEV_TYPE_NIM);

	for(i=0;i<10;i++)
		last_time[i]=0;


#if (QAM_TEST_FUNC==SYS_FUNC_ON)
	for (i = 0; i<8 ; i++)
		nimRegFuncFlag(i,FALSE,TRUE);
#endif


	while(1)
	{
	  	curt_time = osal_get_tick();

		osal_task_sleep(20);

	  	if (curt_time - last_time[0] >300)
	  	{ 
			nim_s3202_monitor_berper(dev,&BER_VALID);
			last_time[0] = osal_get_tick();
			
#if (1 == M3202_LOG_FUNC)
			data[1]=0x56;
			nim_s3202_read(data[1],data,1);
			
			M3202_PRINTF("CR%2x   =   0x%x   ",data[1],data[0]);

			data[1]=0x0a;
			nim_s3202_read(data[1],data,1);
			M3202_PRINTF(" CR%2x   =   0x%x   ",data[1],data[0]);

			data[2]=0x14;
			nim_s3202_read(data[2],data,2);
			M3202_PRINTF(" CR%2x   =   0x%x   ",data[2], (((data[1]&0x03)<<8) |data[0]) );

			data[1]=0x28;
			nim_s3202_read(data[1],data,1);
			M3202_PRINTF("CR%2x   =   0x%x   ",data[1],data[0]);

			data[1]=0x30;
			nim_s3202_read(data[1],data,1);
			M3202_PRINTF("CR%2x   =   0x%x   ",data[1],data[0]);

			data[1]=0xd8;
			nim_s3202_read(data[1],data,1);
			M3202_PRINTF("CR%2x   =   0x%x   ",data[1],data[0]);

			data[2]=0x6c;
			nim_s3202_read(data[2],data,2);
			M3202_PRINTF("CR%2x   =   0x%x   ",data[2], ((data[1]<<8) |data[0]) );


			if(BER_VALID == TRUE)
			{
				M3202_PRINTF("BER = %6d, PER = %d ",BER_COUNTS,PER_COUNTS);
				BER_VALID = FALSE;
			}
			M3202_PRINTF(";\n");

			cnt ++;
			
			if (20==cnt)
			{
				freq=S3202_CurChannelInfo.Frequency;
				sym_rate=S3202_CurChannelInfo.SymbolRate;
				data[0]=S3202_CurChannelInfo.Modulation;

				M3202_PRINTF("%s,freq = %d, sym = %d, fec = %d\n",__FUNCTION__, freq,sym_rate,data[0]);

				cnt = 0;
			}
			/*
			nim_s3202_read(0x02,data,1);
		
			if (0x08==(data[0]&0x08))
				{
					M3202_PRINTF("QAM lock threshold timerout!\n");

					data[0]=data[0]&0xf7;
					nim_s3202_write(0x02,data,1);
				}
			*/
#endif
	  	}

//-----------------------------------
// This Part is For Test Perpose----------

#if (QAM_TEST_FUNC==SYS_FUNC_ON)		
		if (BER_VALID == TRUE)
		{
//joey. 20090224. To make the driver more reasonable and readable.			
			nim_s3202_get_lock(dev, &lock);

			if (lock ==1)
				S3202_PRINTF("Locked, BER = %6d, PER = %d ;\n",BER_COUNTS,PER_COUNTS);
			else
				S3202_PRINTF("Unlock, BER = %6d, PER = %d ;\n",BER_COUNTS,PER_COUNTS);

			if (nimRegFuncFlag(1,FALSE,FALSE) == TRUE)
				nimreg_ber_refresh (BER_COUNTS,PER_COUNTS);
			BER_VALID = FALSE;
		}
		
		if (nimRegFuncFlag(0,FALSE,FALSE) == TRUE)
		{ 
			data[0] = 0x80;
			nim_s3202_write(0x00,data,1);
			data[0] = 0x40;
			nim_s3202_write(0x00,data,1);
			nimRegFuncFlag(0,FALSE,TRUE) ;
		}
		if (nimRegFuncFlag(2,FALSE,FALSE) == TRUE)
		{ 
			if (curt_time - last_time[1] > 500)
			{
				display_dynamic_vision();
				last_time[1] = osal_get_tick();
			}
		}
		if (nimRegFuncFlag(3,FALSE,FALSE) == TRUE)
		{
			nim_s3202_mon_catch_ad_data(dev);
			nimRegFuncFlag(3,FALSE,TRUE) ;
		}

		if (nimRegFuncFlag(7,FALSE,FALSE) == TRUE)
			time_cons = 5;
		else
			time_cons = 300;

	  	if (curt_time - last_time[2] >time_cons)
	  	{ 
			nimRegPrint();
			last_time[2] = osal_get_tick();
	  	}
#else
		if (BER_VALID == TRUE)
		{
#ifndef SFU_AUTO_TEST			
			//libc_printf("BER = %6d, PER = %d ;\n",BER_COUNTS,PER_COUNTS);
#endif			
			BER_VALID = FALSE;
		}
		
#endif

//-----------------------------------
// This Part is For Patch Perpose----------

#if (QAM_FPGA_USAGE == SYS_FUNC_OFF)
	//joey 20080504. update according to program guide 20080430 by Dan.
		if ((sys_ic_get_rev_id() == IC_REV_1) && (rf_agc_en == TRUE))
		{
			nim_s3202_monitor_agc_status(dev, NULL);
		}

	//Trueve 20110526. for 3202C disable agc monitor.
		if (sys_ic_get_rev_id() < IC_REV_4)
		{
	//joey 20090225. for CCI pK with stv0297e.
			nim_s3202_monitor_agc0a_loop(dev, NULL);
	//joey 20080504. update according to program guide 20080430 by Dan.
			//nim_s3202_monitor_cr_loop(dev, NULL);
			//nim_s3202_monitor_fff_len(dev, NULL);
		}

#if (1 == M3202_LOG_FUNC)
		nim_s3202_read(0x05,data,1);

		if ( 0x80 == (data[0]&0x80) )
		{
			M3202_PRINTF("Capture data start \n");
			for(i=0;i<10;i++)
			{
				nim_s3202_mon_catch_ad_data_loop(dev);
			}
			M3202_PRINTF("Capture data end \n");
			data[0] =0x00;	
			nim_s3202_write(0x05,data,1);
		}
#endif
#endif
	}
}


/*****************************************************************************
* INT32 nim_s3202_open(struct nim_device *dev)
* Description: S3202 open
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3202_open(struct nim_device *dev)
{
	UINT8 Data;

	struct QAM_TUNER_CONFIG_DATA TunerConfig= ((struct nim_s3202_private*)dev->priv)->tuner_config_data;
	struct QAM_TUNER_CONFIG_EXT TunerConfigext= ((struct nim_s3202_private*)dev->priv)->tuner_config_ext;

	dem_s3202_mutex_id = osal_mutex_create();
	if (dem_s3202_mutex_id == OSAL_INVALID_ID)
	{
		S3202_PRINTF("nim_s3202_open: Create mutex failed!\n");
	}

//soft reset.	
	Data = 0x80;
	nim_s3202_write(NIM_S3202_CONTROL1, &Data, 1);

//AD mode set 
#if (QAM_FPGA_USAGE == SYS_FUNC_ON)
	Data = 0x80;
	nim_s3202_write(NIM_S3202_ADC_CONFIG, &Data, 1);
#else
	Data = 0x08;
	nim_s3202_write(NIM_S3202_ADC_CONFIG, &Data, 1);
#endif

	if (sys_ic_get_rev_id() < IC_REV_2)
	{
		Data = 0x07;
		nim_s3202_write(NIM_S3202_EADC_CFG1, &Data, 1); //By David Wang 2008/05/28
	}
	
// Set AGC config.
// Set special config. 
	Data = TunerConfigext.cTuner_special_config;
	if ((Data & 0x01) == 0x01) //disable rf_agc.
	{
		nim_s3202_read(NIM_S3202_AGC10,&Data,1);
		Data = Data & 0xDF;
		nim_s3202_write(NIM_S3202_AGC10,&Data,1);

		rf_agc_en = FALSE;
	}
	else
	{
		nim_s3202_read(NIM_S3202_AGC10,&Data,1);
		Data = Data | 0x20;
		nim_s3202_write(NIM_S3202_AGC10,&Data,1);

		Data = TunerConfig.RF_AGC_MAX;
		nim_s3202_write(0x0f,&Data,1);
		Data = TunerConfig.RF_AGC_MIN;
		nim_s3202_write(0x10,&Data,1);

		rf_agc_en = TRUE;
	}
	
	Data = TunerConfig.IF_AGC_MAX;
	nim_s3202_write(0x11,&Data,1);
	Data = TunerConfig.IF_AGC_MIN;
	nim_s3202_write(0x12,&Data,1);
	Data = TunerConfig.AGC_REF;
	nim_s3202_write(0x0a,&Data,1);

//joey 20080504. default we will care about aci performance.
//for agc patch usage.
	if_def_val2 = TunerConfig.IF_AGC_MIN;
//Set Down Mixer according to the IF freq

//joey, 20120406, for low-IF tuner(silicon tuner) support.
	if (TunerConfigext.wTuner_IF_Freq<=10000)
	{
		Data=(UINT8)(0xff&TunerConfigext.wTuner_IF_Freq);
		nim_s3202_write(INI_DM_FREQ_OFFSET_0,&Data,1);
		Data=(UINT8)(0xff&((TunerConfigext.wTuner_IF_Freq)>>8));
		nim_s3202_write(INI_DM_FREQ_OFFSET_1,&Data,1);

		init_foffset = 0;
	}
	else // traditional 36M IF tuner.
	{
		init_foffset = 150;
	}
		
//For ts output format (SSI)
#if (QAM_FPGA_USAGE == SYS_FUNC_ON)
	Data = 0x88;
	nim_s3202_write(0x07,&Data,1);
#else
    if (TunerConfig.cTuner_Tsi_Setting >= NIM_0_SSI_0)
	{
        Data = 0x88;
    	nim_s3202_write(0x07,&Data,1);
    }
#endif	

#if (QAM_ONLY_USAGE == SYS_FUNC_ON)
    Data = 0x00;
	nim_s3202_write(0xf1,&Data,1);
#endif

	if (sys_ic_get_rev_id() == IC_REV_0)
	{
// Set AGC config.
/*
		Data = TunerConfig.RF_AGC_MAX;
		nim_s3202_write(NIM_S3202_AGC6,&Data,1);
  		Data = TunerConfig.RF_AGC_MIN;
		nim_s3202_write(NIM_S3202_AGC7,&Data,1);
		Data = TunerConfig.IF_AGC_MAX;
		nim_s3202_write(NIM_S3202_AGC8,&Data,1);
		Data = TunerConfig.IF_AGC_MIN;
		nim_s3202_write(NIM_S3202_AGC9,&Data,1);
		Data = TunerConfig.AGC_REF;
		nim_s3202_write(NIM_S3202_AGC1,&Data,1);
*/

//Set EQ default Value:
		Data = 0x56;
		nim_s3202_write(NIM_S3202_EQ1,&Data,1);
		Data = 0x09;
		nim_s3202_write(NIM_S3202_EQ3,&Data,1);
		Data = 0x09;
		nim_s3202_write(NIM_S3202_EQ4,&Data,1);
		Data = 0x07;
		nim_s3202_write(NIM_S3202_EQ5,&Data,1);
		Data = 0x07;
		nim_s3202_write(NIM_S3202_EQ6,&Data,1);
		Data = 0x08;
		nim_s3202_write(NIM_S3202_EQ7,&Data,1);
		Data = 0x08;
		nim_s3202_write(NIM_S3202_EQ8,&Data,1);
		Data = 0x0A;
		nim_s3202_write(NIM_S3202_EQ9,&Data,1);
		Data = 0x06;
		nim_s3202_write(NIM_S3202_EQ10,&Data,1);
		Data = 0x7D;
		nim_s3202_write(NIM_S3202_EQ12,&Data,1);
		Data = 0x3F;
		nim_s3202_write(NIM_S3202_EQ13,&Data,1);
		Data = 0x35;
		nim_s3202_write(NIM_S3202_EQ14,&Data,1);
		Data = 0x78;
		nim_s3202_write(NIM_S3202_EQ16,&Data,1);
		Data = 0xB6;
		nim_s3202_write(NIM_S3202_EQ17,&Data,1);
		Data = 0x88;
		nim_s3202_write(NIM_S3202_EQ18,&Data,1);
	
	//Set CR default Value:
		Data = 0x24;
		nim_s3202_write(NIM_S3202_CR_PRO_PATH_GAIN_3,&Data,1);
		Data = 0x11;
		nim_s3202_write(NIM_S3202_CR_PRO_PATH_GAIN_4,&Data,1);
		Data = 0x0A;
		nim_s3202_write(NIM_S3202_CR_INT_PATH_GAIN_4,&Data,1);
	
	//Add by David Wang for Nanning MMDS network on 2/26
		Data = 0x40;
		nim_s3202_write(NIM_S3202_TR_LOOP9,&Data,1); 
	
	// Add for ChenYu by Penghui_li 20080303
		Data = 0x00;
		nim_s3202_write(NIM_S3202_AGC4,&Data,1);
		Data = 0xC4;
		nim_s3202_write(NIM_S3202_AGC5,&Data,1);
		Data = 0x00;
		nim_s3202_write(NIM_S3202_AGC15,&Data,1);
		Data = 0x04;
		nim_s3202_write(NIM_S3202_AGC16,&Data,1);
	}
	else if (sys_ic_get_rev_id() == IC_REV_1)
	{
#if (QAM_FPGA_USAGE == SYS_FUNC_ON)
//joey 20080414 for FPGA quick usage. set some register directly. init AGC vlaue.
		Data = 0x77;
		nim_s3202_write(0x0d,&Data,1);
		Data = 0xc6;
		nim_s3202_write(0x0e,&Data,1);
		Data = 0x98;
		nim_s3202_write(0x5c,&Data,1);
		Data = 0x06;
		nim_s3202_write(0x5d,&Data,1);
#else
//joey 20080416, update according to register config s3202 by xian_chen.
//joey 20080418, update according to register config s3202 by xian_chen.
//joey 20080504, update according to Program guide 20080430 by Dan.
		Data = 0x99;
		nim_s3202_write(0x2a,&Data,1);
		Data = 0x77;
		nim_s3202_write(0x2b,&Data,1);
		Data = 0x99;
		nim_s3202_write(0x2c,&Data,1);
		Data = 0x88;
		nim_s3202_write(0x2d,&Data,1);

		Data = 0x08;
		nim_s3202_write(0x85,&Data,1);
		Data = 0x08;
		nim_s3202_write(0x86,&Data,1);

		Data = 0x14;
		nim_s3202_write(0x28,&Data,1);
		Data = 0x47;
		nim_s3202_write(0x29,&Data,1);
		Data = 0x40;
		nim_s3202_write(0x21,&Data,1);
		Data = 0x16;
		nim_s3202_write(0x19,&Data,1);
		Data = 0x0e;
		nim_s3202_write(0x1a,&Data,1);

		Data = 0x17;
		nim_s3202_write(0x1b,&Data,1);
		Data = 0x94;
		nim_s3202_write(0x1c,&Data,1);
		Data = 0x41;
		nim_s3202_write(0x1d,&Data,1);

		Data = 0xdf;
		nim_s3202_write(0x52,&Data,1);
#endif
	}
	else if (sys_ic_get_rev_id() == IC_REV_2)
	{
#if 0
	//joey 20080616, update according to Program guide 20080616 by Dan.
		Data = 0x77;
		nim_s3202_write(0x0d,&Data,1);
		Data = 0xc6;
		nim_s3202_write(0x0e,&Data,1);

		Data = 0xed;
		nim_s3202_write(0x0f,&Data,1);
		Data = 0x1b;
		nim_s3202_write(0x10,&Data,1);
		Data = 0x4e;
		nim_s3202_write(0x11,&Data,1);

		Data = 0x08;
		nim_s3202_write(0x5c,&Data,1);
		Data = 0x06;
		nim_s3202_write(0x5d,&Data,1);
#endif


	//#if (SYS_TUNER_MODULE == DCT7044)
	if (TunerConfigext.cChip == Tuner_Chip_PHILIPS)
	{
		if (TunerConfigext.cTuner_Base_Addr == 0xC0)
		{
			if_sml_val1 = 0x2c;
			rf_sml_th1 = 0x17;
			rf_big_th2 = 0x55;

			Data = 0x00;
			nim_s3202_write(0x0d,&Data,1);
			Data = 0xC4;
			nim_s3202_write(0x0e,&Data,1);
			Data = 0x00;
			nim_s3202_write(0x5c,&Data,1);
			Data = 0x04;
			nim_s3202_write(0x5d,&Data,1);
		}	
	//#elif (SYS_TUNER_MODULE == DCT70701)
		else 
		{
			if_sml_val1 = 0x8a;
			rf_sml_th1 = 0x14;
			rf_big_th2 = 0x56;

			Data = 0xff;
			nim_s3202_write(0x0d,&Data,1);
			Data = 0xC5;
			nim_s3202_write(0x0e,&Data,1);
			Data = 0x00;
			nim_s3202_write(0x5c,&Data,1);
			Data = 0x04;
			nim_s3202_write(0x5d,&Data,1);
		}
	}
	//#else	//default use dct7044
	else
	{
		if_sml_val1 = 0x2c;
		rf_sml_th1 = 0x17;
		rf_big_th2 = 0x55;

		Data = 0x00;
		nim_s3202_write(0x0d,&Data,1);
		Data = 0xC4;
		nim_s3202_write(0x0e,&Data,1);
		Data = 0x00;
		nim_s3202_write(0x5c,&Data,1);
		Data = 0x04;
		nim_s3202_write(0x5d,&Data,1);
	}
	//#endif


	//trueve 20080716, update according to Program guide 20080716 by Dan.

		if (TunerConfigext.cChip == Tuner_Chip_PHILIPS && TunerConfigext.cTuner_Base_Addr == 0xC0)
		{//trueve 20080801, only DCT7044 use below AGC config paremeters
			Data = 0x10;
			nim_s3202_write(0x10,&Data,1);
			Data = 0x25;
			nim_s3202_write(0x12,&Data,1);
		}
		
		Data = 0xaa;
		nim_s3202_write(0x2a,&Data,1);
		Data = 0x88;
		nim_s3202_write(0x2b,&Data,1);

		Data = 0xFF;
		nim_s3202_write(0x5c,&Data,1);
		Data = 0x05;
		nim_s3202_write(0x5d,&Data,1);


		// magic 20090817, modified the QAM unlock threshold to 1.2us
		Data	= 0x20;
		nim_s3202_write(NIM_S3202_TIMEOUT_THRESHOLD,&Data,1);

		//trueve 20080820, update according to Program guide 20080926 by Dan.
		nim_s3202_read(0x42,&Data,1);
		Data |= (1<<4);
		nim_s3202_write(0x42,&Data,1);
#if 0 /* trueve 20081202, comment this block according to Program guide 20081202 by Joey */
		nim_s3202_read(0x69,&Data,1);
		Data |= 0x07;
		nim_s3202_write(0x69,&Data,1);
#else
		nim_s3202_read(0x69,&Data,1);
		Data &= 0xF8;
		nim_s3202_write(0x69,&Data,1);
#endif
		Data = 0x56;
		nim_s3202_write(0xc6,&Data,1);
		//trueve 20080912, update according to Program guide 20080912 by Dan.
		Data = 0x12;
		nim_s3202_write(0xcf, &Data, 1);
		//trueve 20081224, update according to Program guide 20081223 by Joey.	
		Data = 0x2a;
		nim_s3202_write(0x21, &Data, 1);
		//trueve 20081226, update according to Program guide 20081226 by Joey.
		//Comment these lines to use setting in root.c (Trueve 090311)
		/*
		if ((TunerConfigext.cTuner_special_config & 0x02) == 0x00)
		{
			Data = 0x80;
			nim_s3202_write(0x0a, &Data, 1);	
		}
		*/
		// modified the AGC_TUN_DSMIV to middle level  for CD1616
		// magic 20090303
		if (TunerConfigext.cChip == Tuner_Chip_CD1616LF_GIH)
		{
			Data = 0x00;
			nim_s3202_write(0x5c,&Data,1);
			Data = 0x04;
			nim_s3202_write(0x5d,&Data,1);

			Data =0x04;
			nim_s3202_write(0x69,&Data,1); //config the AD Full Scale to 2Vpp
			
			Data =0x80;
			nim_s3202_write(0x0d,&Data,1); //config the IF_IV 
		
		}


	//modified the CR LOCK threshold by magic 20090813
		Data = 0x66;
		nim_s3202_write(0xb8,&Data,1);

		Data = 0xa6;
		nim_s3202_write(0xb9,&Data,1);


	//modifed the fff len by magic 20090823
		Data = 0x13;
		nim_s3202_write(0x28,&Data,1);
        //modifed the CR I Gain by magic 20090823
        //joey, 20100806, for shuozhou 522M issue. fix CRd8 to 0x0b.
		Data = 0x0b;
		nim_s3202_write(0xd8,&Data,1);	

	//joey, 20100810, modify CR register for easy lock. 	
		Data = 0x20;
		nim_s3202_write(0xb4,&Data,1);	
	
		Data = 0x40;
		nim_s3202_write(0xb5,&Data,1);	

		Data = 0x64;
		nim_s3202_write(0xb6,&Data,1);	

		Data = 0x32;
		nim_s3202_write(0xb7,&Data,1);	



		Data = 0x11;
		nim_s3202_write(0xd2,&Data,1);	
	
		Data = 0x0f;
		nim_s3202_write(0xd3,&Data,1);	

		Data = 0x0d;
		nim_s3202_write(0xd4,&Data,1);	
	
		Data = 0x0b;
		nim_s3202_write(0xd5,&Data,1);	

		Data = 0x08;
		nim_s3202_write(0xd6,&Data,1);	

		Data = 0x08;
		nim_s3202_write(0xd7,&Data,1);	
		
		/* For lock threshold of 128-QAM */
		Data = 0x3c;
		nim_s3202_write(0xbc,&Data,1);	
		Data = 0x7a;
		nim_s3202_write(0xbd,&Data,1);
		Data = 0x2d;
		nim_s3202_write(0xbe,&Data,1);
		Data = 0x56;
		nim_s3202_write(0xbf,&Data,1);
		Data = 0x5f;
		nim_s3202_write(0xc0,&Data,1);
		Data = 0x50;
		nim_s3202_write(0xc1,&Data,1);
		Data = 0xcc;
		nim_s3202_write(0xdc,&Data,1);
		
//Joey. 20110811. for black screen issue. Modify PN function.
		Data = 0x00;
		nim_s3202_write(0x50,&Data,1);	
		
	}
	else if (sys_ic_get_rev_id() >= IC_REV_4) // for 3202c, no content
	{
		Data = 0x3a; //bypass pre-echo, awgn still on.
		nim_s3202_write(0x0c,&Data,1);

		//Data = 0x30; // bypass dagc.
		//nim_s3202_write(0x82,&Data,1);	

		//nim_s3202_read(0x42,&Data,1); // disable watchdog.
		//Data &= ~(1<<4);
		//nim_s3202_write(0x42,&Data,1);


    //joey, 20120720, for skyworth 674M issue. modify CR register for 64Qam easy lock.
		Data = 0x11; // CR_P_GAIN_4
		nim_s3202_write(0xcd,&Data,1);		

		Data = 0x09; // CR_I_GAIN_4
		nim_s3202_write(0xd6,&Data,1);		

		Data = 0x13; // CR_P_GAIN_5
		nim_s3202_write(0xce,&Data,1);		

		Data = 0x7a; // CR_THRED_LOCK_64_4
		nim_s3202_write(0xb8,&Data,1);		

		Data = 0x8e; // CR_THRED_LOCK_64_5
		nim_s3202_write(0xb9,&Data,1);		

		Data = 0x64; // CR_THRED_LOCK_64_6
		nim_s3202_write(0xba,&Data,1);	

//joey, 20120724. CR register update.
		Data = 0x09; // CR_I_GAIN_6_0
		nim_s3202_write(0xd8,&Data,1);	

		//Data = 0x94; // Disable freq_jitter.
		//nim_s3202_write(0x45,&Data,1);	

		//Data = 0x0b; // Change B_gain to 0x0a. 
		//nim_s3202_write(0x91,&Data,1);	

		Data = 0x10;
		nim_s3202_write(0x5e,&Data,1);

		Data = 0x08;
		nim_s3202_write(0x5f,&Data,1);

		Data = 0x9f; // Change threshod to 0x9f. 
		nim_s3202_write(0x67,&Data,1);	
		
	}


// Add a new thread
#ifndef NOT_MONITOR_NIM
	{
		OSAL_T_CTSK t_ctsk_sw;

		t_ctsk_sw.name[0] = 'M';
		t_ctsk_sw.name[1] = 'O';
		t_ctsk_sw.name[2] = 'I';    
		t_ctsk_sw.stksz = 0x1000;
		t_ctsk_sw.quantum = 5;
		t_ctsk_sw.task = nim_s3202_sw_test_thread;
		t_ctsk_sw.itskpri = OSAL_PRI_LOW;
		dem_s3202_task_id = osal_task_create(&t_ctsk_sw);
		if (OSAL_INVALID_ID == dem_s3202_task_id)
		{
			S3202_PRINTF("nim_s3202_open: Create task for minitoring nim failed!\n");
			ASSERT(0);
		}
	}
#endif 
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_s3202_close(struct nim_device *dev)
* Description: S3202 close
*
* Arguments:
*  Parameter1: struct nim_device *dev
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3202_close(struct nim_device *dev)
{
	UINT8 Data = 0;
	INT32 ret = SUCCESS;

	//soft reset.	
	Data = 0x80;
	nim_s3202_write(NIM_S3202_CONTROL1, &Data, 1);

	if (RET_SUCCESS != osal_mutex_delete(dem_s3202_mutex_id))
	{
		S3202_PRINTF("nim_s3202_close: Delete mutex failed!\n");
		ret = ERR_FAILUE;
	}
	dem_s3202_mutex_id = OSAL_INVALID_ID;

#ifndef NOT_MONITOR_NIM
	if (RET_SUCCESS != osal_task_delete(dem_s3202_task_id))
	{
		S3202_PRINTF("nim_s3202_close: Delete minitoring task failed!\n");
		ret = ERR_FAILUE;
	}
	dem_s3202_task_id = OSAL_INVALID_ID;
#endif    

	return ret;
}


/*****************************************************************************
* INT32 nim_s3202_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
*
*  device input/output operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: INT32 cmd
*  Parameter3: UINT32 param
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3202_ioctl(struct nim_device *dev, INT32 cmd, UINT32 param)
{
	INT32 rtn;
	switch( cmd )
	{
	case NIM_DRIVER_READ_QPSK_BER:
	    rtn =  nim_s3202_get_BER(dev, (UINT32 *)param);
	    break;
	case NIM_DRIVER_READ_RSUB:
	    rtn =  nim_s3202_get_PER(dev, (UINT32 *)param);
	    break;
    case NIM_DRIVER_GET_AGC:
        return nim_s3202_get_AGC(dev, (UINT8 *)param);
	default:
	    rtn = SUCCESS;
	    break;
	}
	return rtn;
}
static INT32 nim_s3202_ioctl_ext(struct nim_device *dev, INT32 cmd, void* param_list)
{
	INT32 rtn;
	switch( cmd )
	{
	case NIM_DRIVER_AUTO_SCAN:			/* Do AutoScan Procedure */
		//nim_s3202_AutoScan(dev, (struct NIM_Auto_Scan *) (param_list));
		rtn = SUCCESS;
		break;
	case NIM_DRIVER_CHANNEL_CHANGE:		/* Do Channel Change */
		rtn = nim_s3202_channel_change(dev, (struct NIM_Channel_Change *) (param_list));
		break;
	case NIM_DRIVER_QUICK_CHANNEL_CHANGE:	/* Do Quick Channel Change without waiting lock */
		rtn = nim_s3202_quick_channel_change(dev, (struct NIM_Channel_Change *) (param_list));
		break;
	case NIM_DRIVER_CHANNEL_SEARCH:	/* Do Channel Search */
		rtn= SUCCESS;
		break;
	case NIM_DRIVER_GET_RF_LEVEL:
		rtn = nim_s3202_get_RF_Level(dev, (UINT16 *)param_list);
		break;
	case NIM_DRIVER_GET_CN_VALUE:
		rtn = nim_s3202_get_CN_value(dev, (UINT16 *)param_list);
		break;
	case NIM_DRIVER_GET_BER_VALUE:
		rtn = nim_s3202_get_BER(dev, (UINT32 *)param_list);
		break;
	case NIM_DRIVER_SET_PERF_LEVEL:
		rtn = nim_s3202_set_perf_level(dev, (UINT32)param_list);
		break;
	case NIM_DRIVER_GET_I2C_INFO:
		{			
			struct NIM_I2C_Info *i2c_info = (struct NIM_I2C_Info *)param_list;
#if (QAM_WORK_MODE == QAM_ONLY)
			i2c_info->i2c_type = dem_i2c_id;
#ifndef SYS_DEM_BASE_ADDR
			i2c_info->i2c_addr = 0x40;
#else
			i2c_info->i2c_addr = SYS_DEM_BASE_ADDR;
#endif
			rtn = SUCCESS;
#else
			i2c_info->i2c_type = 0xFF;
			i2c_info->i2c_addr = 0xFF;
			rtn = ERR_FAILUE;
#endif
			break;
 		}
	default:
		rtn = SUCCESS;
	       break;
	}

	return rtn;
}



static INT32 nim_s3202_channel_search(struct nim_device *dev, UINT32 freq)
{
	return SUCCESS;
}

static INT32 nim_s3202_get_lock(struct nim_device *dev, UINT8 *lock)
{
	UINT8 data;
	
	//CR56.
	nim_s3202_read(NIM_S3202_MONITOR1, &data, 1);
	if ((data & 0x20) == 0x20)
	{
		*lock = 1;
	}
	else
	{
		*lock = 0;
	}

#ifdef FRONT_PANEL_SINO
#include <hld/pan/pan_dev.h>
extern struct pan_device  *g_pan_dev;
       //libc_printf("\n[CC_TRACE]________%s, g_pan_dev = 0x%X\n", __FUNCTION__, g_pan_dev);
       if (g_pan_dev != NULL)
       {
	       pan_io_control(g_pan_dev, PAN_DRIVER_SIGNAL_LOCK, *lock);
		//libc_printf("[CC_TRACE]________%s, lock_status = %d\n", __FUNCTION__, *lock);
       }
#endif
	
	return SUCCESS;
}

static INT32 nim_s3202_get_freq(struct nim_device *dev, UINT32 *freq)
{
	INT32 freq_off;
	UINT8 data[2];

	//CR48.CR49
	nim_s3202_read(NIM_S3202_FSM17, data, 2);

	if(0x00 == (data[1]&0x10))//positive value.
	{
		freq_off = (INT32)(data[0] | ((data[1]&0x1f)<<8)) ;
	}
	else//negtive.
	{
		freq_off = (INT32)(data[0] | ((data[1]&0x1f)<<8) | 0xFFFFE000);
	}

	S3202_PRINTF("data is 0x%x 0x%x !\n", data[0], data[1]);
	
	//actually is /1024*1000/10. unit is KHz*10.
	freq_off = (INT32)(freq_off*1000/1024);

	//here I think it should be refer the I/Q status. 
	//*freq -= freq_off;

	S3202_PRINTF("offset is %d !\n", freq_off);
	
	//*freq = (S3202_CurChannelInfo.Frequency - freq_off)/10;

//joey 20080504. Use persudo "freq offset" for auto-scan xxx.000 display. not xxx.150/130 display.
	*freq = (S3202_CurChannelInfo.Frequency - init_foffset)/10;

	S3202_PRINTF("*freq is %d !\n", *freq);
	
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_s3202_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate)
* Read S3202 symbol rate
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 *sym_rate			: Symbol rate in kHz
*
* Return Value: void
*****************************************************************************/
static INT32 nim_s3202_get_symbol_rate(struct nim_device *dev, UINT32 *sym_rate)
{
	UINT8 data[2];
	UINT32 rtp_sym;

	nim_s3202_read(NIM_S3202_FSM15, data, 2);

	rtp_sym = (UINT32)(((0x1f&data[1])<<8) | data[0]);

	*sym_rate = (UINT32)(rtp_sym*1000/1024);

	return SUCCESS;
}

static INT32 nim_s3202_get_qam_order(struct nim_device *dev, UINT8 *qam_order)
{
	UINT8 data;

	nim_s3202_read(NIM_S3202_FSM19, &data, 1);

	*qam_order = (UINT8)(data&0x0f);

	return SUCCESS;
}

static INT32 nim_s3202_get_fft_result(struct nim_device *dev, UINT32 freq, UINT32* start_adr )
{
	return SUCCESS;
}


/*****************************************************************************
* INT32 nim_s3202_get_AGC(struct nim_device *dev, UINT8 *agc)
*
*  This function will access the NIM to determine the AGC feedback value
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT8* agc
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3202_get_AGC(struct nim_device *dev, UINT8 *agc)
{
	UINT8 data[4];
	UINT16 temp;
	UINT8 if_agc_gain, tun_agc_gain;
	UINT8 RF_EN, IF_EN;

	nim_s3202_read(NIM_S3202_AGC6,data,4);
	UINT8	RF_AGC_MAX_priv=data[0];
	UINT8	RF_AGC_MIN_priv=data[1] ;
	UINT8	IF_AGC_MAX_priv=data[2];
	UINT8	IF_AGC_MIN_priv=data[3];

	//CR56.
	nim_s3202_read(NIM_S3202_MONITOR1, data, 1);
	if((data[0]&0x01) == 0x01)	//AGC is locked
	{
		nim_s3202_read(NIM_S3202_AGC10, data, 4);
		RF_EN = (data[0] & 0x20)>> 5;
		IF_EN = (data[0] & 0x02) >> 1;
		
		//IF_AGC_GAIN operation.
		temp = (UINT16)(((data[2]&0x03)<<8) | data[1]);
		if_agc_gain = (UINT8)((temp>>2) + 0x80);

		//TUN_AGC_GAIN operation.
		temp = (UINT16)(((data[3]&0x0f)<<6) | ((data[2]>>2)&0x3f));
		tun_agc_gain = (UINT8)((temp>>2) + 0x80);  

		if (RF_EN && IF_EN)  // RF agc & IF agc are all opened 
			{
			if (tun_agc_gain < RF_AGC_MIN_priv)// we not use 100 and 0 for indicator.
				*agc = 99;
			else if (tun_agc_gain <= RF_AGC_MAX_priv)
			{
				if(0!=(RF_AGC_MAX_priv -RF_AGC_MIN_priv))
				{
				*agc = (99 - (tun_agc_gain-RF_AGC_MIN_priv)*40/(RF_AGC_MAX_priv -RF_AGC_MIN_priv));
				}
			}
			else if (if_agc_gain <= IF_AGC_MAX_priv)
			{
				if(0!=(IF_AGC_MAX_priv -IF_AGC_MIN_priv))
				{
				*agc = (60 - (if_agc_gain-IF_AGC_MIN_priv)*57/(IF_AGC_MAX_priv -IF_AGC_MIN_priv));
				}
			}
			else 
				*agc = 1;
			}
		else if (RF_EN && !IF_EN) // RF agc opened & IF agc closed
			{
			if (tun_agc_gain <RF_AGC_MIN_priv)
				*agc = 99;
			else if(tun_agc_gain < RF_AGC_MAX_priv)
			{
				if(0!=(RF_AGC_MAX_priv - RF_AGC_MIN_priv))
				{
				*agc = 99 -99* (tun_agc_gain - RF_AGC_MIN_priv)/(RF_AGC_MAX_priv - RF_AGC_MIN_priv);
				}
			}
			else 
				*agc = 1;
			}
		else if (!RF_EN && IF_EN) // RF agc closed & IF agc opened
			{
			if (if_agc_gain < IF_AGC_MIN_priv)
				*agc = 99;
			else if (if_agc_gain < IF_AGC_MAX_priv)
			{
				if(0!=(IF_AGC_MAX_priv - IF_AGC_MIN_priv))
				{
				*agc = 99 -99* (if_agc_gain - IF_AGC_MIN_priv) / (IF_AGC_MAX_priv - IF_AGC_MIN_priv);
				}
			}
			else 
				*agc = 1;
			}
		else  // RF agc & IF agc are all closed
			*agc = 1;

	}
	else
		*agc = 0x00;
	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3202_get_SNR(struct nim_device *dev, UINT8 *snr)
*
* This function returns an approximate estimation of the SNR from the NIM
*  The Eb No is calculated using the SNR from the NIM, using the formula:
*     Eb ~     13312- M_SNR_H
*     -- =    ----------------  dB.
*     NO           683
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3202_get_SNR(struct nim_device *dev, UINT8 *snr)
{
	UINT8 data[2];
	UINT32 rpt_power=0;

	//CR56.
	nim_s3202_read(NIM_S3202_MONITOR1, data, 1);
	if((data[0] & 0x20) == 0x20)	//dem is locked.
	{
		//CR6C,CR6D.
		nim_s3202_read(NIM_S3202_SNR_MONI1, data, 2);

		rpt_power = (UINT32)(data[1]<<8 | data[0]);

		if(rpt_power>307)
			*snr = 5;
		else if(rpt_power>204)
			*snr = 5+(307-rpt_power)*55/103;
		else  if(rpt_power>88)
			*snr = 60 + (204-rpt_power)*35/116;
		else
			*snr = 95;
	}
	else
		*snr = 0x00;
	return SUCCESS;
}

/*****************************************************************************
* INT32 nim_s3202_get_RF_Level(struct nim_device *dev, UINT16 *RfLevel)
*
*  This function will access the NIM to determine the RF level feedback value
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 *RfLevel
*  Real_RF_level and RfLevel relation is : RfLevel = -(Real_RF_level * 10)
*  eg.  if Real_RF_level = -30.2dBm then , RfLevel = -(Real_RF_level * 10) = 302
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3202_get_RF_Level(struct nim_device *dev, UINT16 *RfLevel)
{
	UINT8 data[3];
	UINT16 Temp16, if_agc_gain,rf_agc_gain;
	UINT32 Temp32 = 0, type;
	UINT32 freq = 0;
	UINT16 x;

//joey, 20120608. for RF_level get.
	struct nim_s3202_private *dev_priv;
	dev_priv = (struct nim_s3202_private *)dev->priv;

	UINT32 rf_level = 0;
	if (NULL != (dev_priv->nim_Tuner_Get_rf_level))	// means there is registered tuner RF level dealing function.	
	{
		if(dev_priv->nim_Tuner_Get_rf_level(dev_priv->tuner_id, &type, &rf_level) != SUCCESS)		
		{
			rf_level = 0;
		}
		else
		{
			if(type == 0)		/* When type is 0, rf_level value is Signal level in dBuv , add by Artie 20120618 */
			{
				*RfLevel = (UINT16)(rf_level & 0xffff);
				return SUCCESS;
			}
		}
	}
	
	nim_s3202_read(NIM_S3202_AGC11, data, 3);
	Temp16 = (UINT16)(((data[1]&0x03)<<8) | data[0]);
	if_agc_gain = (UINT16)((Temp16+0x200)&0x3FF);
	Temp16 = (UINT16)(((data[2]&0x0f)<<6) | ((data[1]>>2)&0x3f));
	rf_agc_gain = (UINT16)((Temp16+0x200)&0x3FF); 

	//libc_printf("if_agc_gain = %d \n", if_agc_gain);

	/* If  nim_s3202_RF_Level_cb is not NULL, we adjust RF level here using customer registered callback instance of default way.*/
	if(nim_s3202_RF_Level_cb != NULL)
	{
		Temp32 = (*nim_s3202_RF_Level_cb)(if_agc_gain, rf_agc_gain);
	}
	else
	{
#if (SYS_TUNER_MODULE == DCT7044)
		if (if_agc_gain > 208)
			Temp32 = (UINT32)((179 * if_agc_gain + 30205 + 50 )/100) ; 
		else if (if_agc_gain > 142)
			Temp32 = (UINT32)((159 * if_agc_gain + 34309 + 50)/100);
		else if (if_agc_gain > 114)
			Temp32 = (UINT32) ((114 * if_agc_gain + 40671 + 50)/100);
		else if (if_agc_gain > 91 )
			Temp32 = (UINT32 )((52 * if_agc_gain + 47752 + 50)/100);
		else if ((if_agc_gain <= 91) && (rf_agc_gain >967 ))
			Temp32 = (UINT32) 510;
		else if (rf_agc_gain > 967 )
			Temp32 = (UINT32)((13 * rf_agc_gain + 38793 + 50)/100);
		else if (rf_agc_gain > 940)
			Temp32 = (UINT32 )((26 * rf_agc_gain + 26030 + 50)/100);
		else if (rf_agc_gain > 914 )
			Temp32 = (UINT32 )((58 * rf_agc_gain - 3831 + 50)/100);
		else if (rf_agc_gain > 878 )
			Temp32 = (UINT32)(( 75 * rf_agc_gain - 19650 + 50)/100);
		else if (rf_agc_gain > 806)
			Temp32 = (UINT32)(( 83 * rf_agc_gain - 26967 + 50)/100);
		else if (rf_agc_gain > 767)
			Temp32 = (UINT32)((67 * rf_agc_gain - 13533 + 50)/100);
		else if (rf_agc_gain > 694)
			Temp32 = (UINT32)((49 * rf_agc_gain - 225 + 50)/100);
		else if (rf_agc_gain > 532)
			Temp32 = (UINT32) ((28 * rf_agc_gain + 14294 + 50)/100);
		else if(rf_agc_gain > 387 )
			Temp32 = (UINT32)(( 21 * rf_agc_gain + 18026 + 50)/100);
		else if (rf_agc_gain > 315)
			Temp32 = (UINT32)(( 33 * rf_agc_gain + 13400 + 50)/100);
		else if (rf_agc_gain > 245)
			Temp32 = (UINT32)((64 * rf_agc_gain+ 3650 + 50)/100);
		else if (rf_agc_gain > 201)
			Temp32 = (UINT32)((141 * rf_agc_gain - 15123 + 50)/100);
		else if (rf_agc_gain > 155)
			Temp32 = (UINT32)((570 * rf_agc_gain - 101283 + 50)/100);
		else 
			Temp32 = 0;
#elif (SYS_TUNER_MODULE == DCT70701)
		if (if_agc_gain > 721)
			Temp32 = 770;
		else if (if_agc_gain > 704)
			Temp32 = (UINT32)((176 * if_agc_gain - 49896)/100);
		else if (if_agc_gain > 685)
			Temp32 = (UINT32)((211 * if_agc_gain - 74544)/100);
		else if (if_agc_gain > 667)
			Temp32 = (UINT32)((167 * if_agc_gain - 44395)/100);
		else if (if_agc_gain > 650)
			Temp32 = (UINT32)((294 * if_agc_gain - 129098)/100);
		else if (if_agc_gain > 628)
			Temp32 = (UINT32)((227 * if_agc_gain - 85550)/100);
		else if (if_agc_gain > 608)
			Temp32 = (UINT32)((250 * if_agc_gain - 100000)/100);
		else if (if_agc_gain > 583)
			Temp32 = (UINT32)((200 * if_agc_gain - 69600)/100);
		else
			Temp32 = 470;
 #elif (SYS_TUNER_MODULE == TDCCG0X1F)
		if (if_agc_gain > 456)
			Temp32 = 850;
		else if (if_agc_gain > 430)
			Temp32 = (UINT32)((222 * if_agc_gain - 16460)/100);
		else if (if_agc_gain > 407)
			Temp32 = (UINT32)((217 * if_agc_gain - 14319)/100);
		else if (if_agc_gain > 373)
			Temp32 = (UINT32)((205 * if_agc_gain - 9465)/100);
		else if (if_agc_gain > 350)
			Temp32 = (UINT32)((434 * if_agc_gain - 94878)/100);
		else if (if_agc_gain > 341)
			Temp32 = (UINT32)((1111 * if_agc_gain - 331851)/100);
		else
			Temp32 = 460;
#elif (SYS_TUNER_MODULE == CD1616LF)
		if (if_agc_gain > 1021) 
				Temp32 = 795; 
		else if (if_agc_gain > 718) 
				Temp32 = (UINT32)((69580 + 10*if_agc_gain)/101); 
		else if (if_agc_gain > 573) 
				Temp32 = (UINT32)((14860 + 10*if_agc_gain)/29); 
		else if (if_agc_gain > 318) 
				Temp32 = (UINT32)((68450 + 200*if_agc_gain)/255); 
		else if (if_agc_gain > 288) 
				Temp32 = (UINT32)(15*if_agc_gain - 4260); 
		else
				Temp32 = 50;
#elif (SYS_TUNER_MODULE == TDA18250)
		if (if_agc_gain > 208)
			Temp32 = (UINT32)((160 * if_agc_gain + 30205 + 50 )/100) ; 
		else if (if_agc_gain > 142)
			Temp32 = (UINT32)((159 * if_agc_gain + 34309 + 50)/100);
		else if (if_agc_gain > 114)
			Temp32 = (UINT32) ((114 * if_agc_gain + 40671 + 50)/100);
		else if (if_agc_gain > 91 )
			Temp32 = (UINT32 )((52 * if_agc_gain + 47752 + 50)/100);
		else if ((if_agc_gain <= 91) && (rf_agc_gain >967 ))
			Temp32 = (UINT32) 510;
		else if (rf_agc_gain > 967 )
			Temp32 = (UINT32)((13 * rf_agc_gain + 38793 + 50)/100);
		else if (rf_agc_gain > 940)
			Temp32 = (UINT32 )((26 * rf_agc_gain + 26030 + 50)/100);
		else if (rf_agc_gain > 914 )
			Temp32 = (UINT32 )((58 * rf_agc_gain - 3831 + 50)/100);
		else if (rf_agc_gain > 878 )
			Temp32 = (UINT32)(( 75 * rf_agc_gain - 19650 + 50)/100);
		else if (rf_agc_gain > 806)
			Temp32 = (UINT32)(( 83 * rf_agc_gain - 26967 + 50)/100);
		else if (rf_agc_gain > 767)
			Temp32 = (UINT32)((67 * rf_agc_gain - 13533 + 50)/100);
		else if (rf_agc_gain > 694)
			Temp32 = (UINT32)((49 * rf_agc_gain - 225 + 50)/100);
		else if (rf_agc_gain > 532)
			Temp32 = (UINT32) ((28 * rf_agc_gain + 14294 + 50)/100);
		else if(rf_agc_gain > 387 )
			Temp32 = (UINT32)(( 21 * rf_agc_gain + 18026 + 50)/100);
		else if (rf_agc_gain > 315)
			Temp32 = (UINT32)(( 33 * rf_agc_gain + 13400 + 50)/100);
		else if (rf_agc_gain > 245)
			Temp32 = (UINT32)((64 * rf_agc_gain+ 3650 + 50)/100);
		else if (rf_agc_gain > 201)
			Temp32 = (UINT32)((141 * rf_agc_gain - 15123 + 50)/100);
		else if (rf_agc_gain > 155)
			Temp32 = (UINT32)((570 * rf_agc_gain - 101283 + 50)/100);
		else 
			Temp32 = 0;
#elif (SYS_TUNER_MODULE == ALPSTDAE)
		if(if_agc_gain > 845)
				Temp32 = 1070; 
		else if (if_agc_gain > 832) 
				Temp32 = (UINT32)((233857 - 142*if_agc_gain)/100); 
		else if (if_agc_gain > 589) 
				Temp32 = (UINT32)((176629 - 74*if_agc_gain)/100); 
		else if (if_agc_gain > 313) 
				Temp32 = (UINT32)((184403 - 87*if_agc_gain)/100); 
		else
			Temp32 = 570;
		
#elif (SYS_TUNER_MODULE == RT820C)
		nim_s3202_get_freq(dev, &freq);
		freq /=100;
		if(if_agc_gain > 1016)
			Temp32 = 1070;
		else if(if_agc_gain > 823)
			Temp32 = (UINT32)((77923 + 20*if_agc_gain)/100);
		else if(if_agc_gain > 649)
			Temp32 = (UINT32)((38173 + 68*if_agc_gain)/100);
		else if(if_agc_gain > 559)
			Temp32 = (UINT32)((10778 + 111*if_agc_gain)/100);
		else if(if_agc_gain > 385)
			Temp32 = (UINT32)((34380 + 68*if_agc_gain)/100);
		else if(if_agc_gain > 190)
			Temp32 = (UINT32)((41206 + 51*if_agc_gain)/100);
		else if(if_agc_gain > 3)
			Temp32 = (UINT32)((48969 + 10*if_agc_gain)/100);
		else
			Temp32 = 490;
		
		if(3 < if_agc_gain && if_agc_gain < 1016)
		{
			if(282 <= freq && freq <= 365)
				Temp32 += 20;	/* shift -2dB */
			else if(550 <= freq && freq <= 634)
				Temp32 -= 20;	/* shift 2dB */
			else if(635 <= freq && freq <= 720)
				Temp32 -= 40;	/* shift 4dB */
			else if(freq > 720)
				Temp32 -= 60;	/* shift 6dB */
		}

#endif
	}
	*RfLevel = (UINT16)Temp32;

	return SUCCESS;	

}

/*****************************************************************************
* INT32 nim_s3202_get_CN_value(struct nim_device *dev, UINT16 *CNValue)
*
*  This function will access the NIM to determine the C/N  feedback value
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16 *CNValue
*  Real_CN_value and CNValue relation is : CNValue = Real_CN_value * 10
*  eg.  if Real_CN_value = 28.3dB then , CNValue = Real_CN_value * 10 = 283
* Return Value: INT32
*****************************************************************************/

static INT32 nim_s3202_get_CN_value(struct nim_device *dev, UINT16 *CNValue)
{
	UINT16 EQ_MSE;
	UINT8 data[2];
	UINT8 qam_order;
	UINT16 const_modules;
	UINT16 log_cstmod_x100_add_100;//  100 * log10(const_modules) + 100
	UINT16 SNR_offset = 25;
	UINT16 cnr = 0;

	nim_s3202_read(NIM_S3202_SNR_MONI1, data, 2);
	EQ_MSE = (UINT16)(data[1]<<8 | data[0]);

	qam_order = S3202_CurChannelInfo.Modulation;
	switch (qam_order)
	{
		case 4:const_modules = 40960; log_cstmod_x100_add_100 = 561;break;
		case 5:const_modules = 20480; log_cstmod_x100_add_100 = 531;break;
		case 6:const_modules = 43008; log_cstmod_x100_add_100 = 563;break;
		case 7:const_modules = 20992; log_cstmod_x100_add_100 = 532;break;
		case 8:const_modules = 43520; log_cstmod_x100_add_100 = 564;break;
		default:const_modules = 43008; log_cstmod_x100_add_100 = 563;break;
	}
	/*
	SNR = 10*log10(const_modules/EQ_MSE);
		= 10*log10(const_modules)-10*log10(EQ_MSE);
		
	SNR10 = 10 * SNR
		= 100 * log10(const_modules) - 100*log10(EQ_MSE)
		= 100 * log10(const_modules) + 100  - 100*log10(EQ_MSE *10);
		= log_cstmod_x100_add_100 - Log10Times100_L(EQ_MSE*10);
	*/

	cnr = log_cstmod_x100_add_100 - Log10Times100_L(EQ_MSE*10) - SNR_offset;

	*CNValue = cnr;

	return SUCCESS;	
	
}



/*****************************************************************************
* INT32 nim_s3202_get_PER(struct nim_device *dev, UINT32 *RsUbc)
* Reed Solomon Uncorrected block count
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT16* RsUbc
*
* Return Value: INT32
*****************************************************************************/
static INT32 nim_s3202_get_PER(struct nim_device *dev, UINT32 *RsUbc)
{
	*RsUbc = PER_COUNTS;
	return SUCCESS;
}

static INT32 nim_s3202_get_BER(struct nim_device *dev, UINT32 *err_count)
{
	*err_count = BER_COUNTS;
	return SUCCESS;
}

static INT32 nim_s3202_monitor_berper(struct nim_device *dev, BOOL *bervalid)
{
	UINT8 data;
	UINT8 data_b[3];

	UINT32 bTemp;
	static UINT32 shBER=0;
	static UINT32 shPER=0;

	//CR62.
	nim_s3202_read(NIM_S3202_RS_BER5,&data,1);
	if(0 == (data&0x80))
	{
		data_b[2] = data & 0x7f;
		nim_s3202_read(NIM_S3202_RS_BER3, &data_b[0], 2);
		BER_COUNTS = (UINT32)((data_b[2]<<16 | data_b[1]<<8 | data_b[0])>>1);

		data = 0x80;
		nim_s3202_write(NIM_S3202_RS_BER5,&data,1);
/*
		data_b[0] = 0xde;
		data_b[1] = 0x2f;
		nim_s3202_write(NIM_S3202_RS_BER1,data_b,2);
*/
		nim_s3202_read(NIM_S3202_PER_REGISTER2,&data,1);
		if((data&0x80) == 0x00)
		{
			bTemp = (data&0x7f)<<8;
			nim_s3202_read(NIM_S3202_PER_REGISTER1,&data,1);
			bTemp |= data;

			if (bTemp >= shPER )
				PER_COUNTS = bTemp - shPER;

			if (bTemp > 0x2fde) // See above data of BER_count number of static.
			{
				data = 0x80;
				nim_s3202_write(NIM_S3202_PER_REGISTER2,&data,1);
				
				shPER = 0;
			}
			else
			{
				shPER = bTemp;
			}
		}
		else	
		{
			PER_COUNTS = 0;
		}

//joey 20080504. add in acc_ber_cnt.
		acc_ber_cnt += 1;

		*bervalid = TRUE;
		return SUCCESS;
		
	} 
	else
	{
		*bervalid = FALSE;
		return ERR_FAILUE;
	}
}



INT32 nim_s3202_attach(struct QAM_TUNER_CONFIG_API * ptrQAM_Tuner)
{

	struct nim_device *dev;
	struct nim_s3202_private *priv_mem;

       /* Attatch the private Tuner Configuration */
#if (QAM_FPGA_USAGE == SYS_FUNC_ON)
	nim_s3202_Tuner_Attatch(ptrQAM_Tuner);
#endif

       /* Alloc structure space of tuner devive*/
	dev = (struct nim_device *)dev_alloc(nim_s3202_name, HLD_DEV_TYPE_NIM, sizeof(struct nim_device));
	if (dev == NULL)
	{
		S3202_PRINTF("Error: Alloc nim device error!\n");
		return ERR_NO_MEM;
	}

	/* Alloc structure space of private */
	priv_mem = (struct nim_s3202_private *)MALLOC(sizeof(struct nim_s3202_private));	
	if ((void*)priv_mem == NULL)
	{
		dev_free(dev);
		S3202_PRINTF("Alloc nim device prive memory error!/n");
		return ERR_NO_MEM;
	}
	MEMSET((void*)priv_mem, 0, sizeof(struct nim_s3202_private));

	/* tuner configuration function */
	MEMCPY((void*)&(priv_mem->tuner_config_data), (void*)&(ptrQAM_Tuner->tuner_config_data), sizeof(struct QAM_TUNER_CONFIG_DATA));
       MEMCPY((void*)&(priv_mem->tuner_config_ext), (void*)&(ptrQAM_Tuner->tuner_config_ext), sizeof(struct QAM_TUNER_CONFIG_EXT));
	priv_mem->nim_Tuner_Init = ptrQAM_Tuner->nim_Tuner_Init;
	priv_mem->nim_Tuner_Control = ptrQAM_Tuner->nim_Tuner_Control;
	priv_mem->nim_Tuner_Status = ptrQAM_Tuner->nim_Tuner_Status;

    #ifdef I2C_BYPASS
	dem_i2c_id = ptrQAM_Tuner->tuner_config_ext.i2c_type_id;
    #endif
	
	dev->priv = (void*)priv_mem;
	/* Function point init */
	dev->base_addr = S3202_QAM_ONLY_I2C_BASE_ADDR;	//0x40
	dev->init = nim_s3202_attach;
	dev->open = nim_s3202_open;
	dev->stop = nim_s3202_close;
	dev->do_ioctl = nim_s3202_ioctl;
	dev->do_ioctl_ext = nim_s3202_ioctl_ext;
	dev->get_lock = nim_s3202_get_lock;
	dev->get_freq = nim_s3202_get_freq;
	dev->get_FEC = nim_s3202_get_qam_order;	
	dev->get_SNR = nim_s3202_get_SNR;
	dev->get_sym = nim_s3202_get_symbol_rate;
	dev->get_BER = nim_s3202_get_BER;
	dev->get_AGC = nim_s3202_get_AGC;
	dev->get_fft_result = nim_s3202_get_fft_result;
	dev->channel_search = nim_s3202_channel_search;
	
	/* Add this device to queue */
	if (dev_register(dev) != SUCCESS)
	{
		S3202_PRINTF("Error: Register nim device error!\n");
		FREE(priv_mem);
		dev_free(dev);
		return ERR_NO_DEV;
	}
	/* Tuner Initial */
	if (((struct nim_s3202_private*)dev->priv)->nim_Tuner_Init != NULL)	
	{
		if (((struct nim_s3202_private*)dev->priv)->nim_Tuner_Init(&(((struct nim_s3202_private*)dev->priv)->tuner_id), &(ptrQAM_Tuner->tuner_config_ext)) != SUCCESS)
		{
			S3202_PRINTF("Error: Init Tuner Failure!\n");
			return ERR_NO_DEV;
		}
	}
	return SUCCESS;
}



/*****************************************************************************
* INT32 nim_s3202_channel_change(struct nim_device *dev, UINT32 freq, UINT32 sym, UINT8 fec);
* Description: S3202 channel change operation
*
* Arguments:
*  Parameter1: struct nim_device *dev
*  Parameter2: UINT32 freq				: Frequence
*  Parameter3: UINT32 sym				: Symbol rate
*  Parameter4: UINT8 fec				: Code rate
*
* Return Value: INT32
*****************************************************************************/
#if (QAM_FPGA_USAGE == SYS_FUNC_ON)
static INT32 nim_s3202_channel_change(struct nim_device *dev, struct NIM_Channel_Change* pstChl_Change)
{
	INT32 result;
   	UINT8 lock = 0, Tuner_Retry = 0;
	UINT8 data,data1;
	UINT8 try_time;
	UINT8 try_256QAM;
	struct nim_s3202_private *dev_priv;	
	UINT32	freq = pstChl_Change->freq;
	UINT32	sym = pstChl_Change->sym;
	UINT8	fec   = pstChl_Change->modulation;
	
	dev_priv = (struct nim_s3202_private *)dev->priv;
	freq *= 10;

//joey 20080417. add in "+150Khz" according to register config by xian_chen.
	freq = freq + 150;

	// Just whther it is Catching Data Mode 
	nim_s3202_read(NIM_S3202_FSM1,&data,1);
	nim_s3202_read(NIM_S3202_FSM11,&data1,1);
	
	if(((data&0x10) == 0x10) ||(( data1&0x08) == 0x08))
	{
		return SUCCESS;
	}
	else
	{
	
//step 1: set receiver to IDLE status, reset Interrupt indicator
		S3202_PRINTF("%s,freq = %d, sym = %d, fec = %d \n",__FUNCTION__, freq,sym,fec);
		data = 0x80;
		nim_s3202_write(NIM_S3202_CONTROL1, &data, 1);
		data = 0x00;
		nim_s3202_write(NIM_S3202_INTERRUPT_EVENTS, &data, 1);		

	// step3: set QAM_ORDER_KNOWN = Unknow
		//nim_s3202_read(0x38, &data, 1);	
		//data = (UINT8)(data&0xbf);
		//nim_s3202_write(0x38, &data, 1);

		//I2c for  confige tuner use by-pass mode of dimodulator. enable the function first.
		//nim_s3202_read(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
		//data |= 0x01; 
		//nim_s3202_write(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
	
		do
			{
				if(Tuner_Retry>5)
				{
					S3202_PRINTF("ERROR! Tuner Lock Fail\n");
					lock = 0;
					return ERR_FAILUE;
				}

				Tuner_Retry++;
				// Fast config tuner
				if(dev_priv->nim_Tuner_Control(dev_priv->tuner_id,freq,sym,FAST_TIMECST_AGC,_1st_i2c_cmd)==ERR_FAILUE)		
				{
					S3202_PRINTF("Fast Config tuner failed step 1!\n");
				}

				if((dev_priv->tuner_config_ext.cChip)==Tuner_Chip_INFINEON)		
				{	
					if(dev_priv->nim_Tuner_Control(dev_priv->tuner_id,freq,sym,FAST_TIMECST_AGC,_2nd_i2c_cmd)==ERR_FAILUE)		
					{
						S3202_PRINTF("Fast Config tuner failed step 2!\n");
					}	
				}
				// Slow config tuner
				if(dev_priv->nim_Tuner_Control(dev_priv->tuner_id,freq,sym,SLOW_TIMECST_AGC, _1st_i2c_cmd)==ERR_FAILUE)
				{
					S3202_PRINTF("Slow Config tuner failed step 1!\n");
				}

				if((dev_priv->tuner_config_ext.cChip)==Tuner_Chip_INFINEON)
				{	
					if(dev_priv->nim_Tuner_Control(dev_priv->tuner_id,freq,sym,SLOW_TIMECST_AGC,_2nd_i2c_cmd)==ERR_FAILUE)
					{	
						S3202_PRINTF("Slow Config tuner failed step 2!\n");
					}		
				}
				// Read status
				if(dev_priv->nim_Tuner_Status(dev_priv->tuner_id,&lock)==ERR_FAILUE)
				{
					S3202_PRINTF("ERROR! Tuner Read Status Fail\n");
				}

				S3202_PRINTF("Tuner Lock Times=0x%d,*lock=0x%d !!\n",Tuner_Retry,lock);
				
			}while(0 == lock);

		//I2c for  confige tuner use by-pass mode of dimodulator. disable the function after config.
		//nim_s3202_read(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
		//data &= 0xfe;
		//nim_s3202_write(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);

//joey 20080417 add in rs swap according to register update by xian_chen.

//step 3: set symbol rate and symbol rate sweep ranger
UINT32 rs_max, rs_min;
	rs_max = (UINT32)((sym*1076)/1024.0 + 0.5);
	rs_min = (UINT32)((sym*973)/1024.0 + 0.5);
	nim_s3202_set_rs(sym);  // eg. sym = 6900
	nim_s3202_set_search_rs(rs_min, rs_max);


	//step 5:start capture.
		data = 0x40;
		nim_s3202_write(NIM_S3202_CONTROL1, &data, 1);

		S3202_CurChannelInfo.Frequency = freq;
		S3202_CurChannelInfo.SymbolRate = sym;
		S3202_CurChannelInfo.Modulation = fec;

	//step 6: check dem lock or not.
		try_time = 0;
		while(1)
		{

			nim_s3202_read(NIM_S3202_MONITOR1, &data, 1);
			if ((data & 0x20) == 0x20)
			{
				return SUCCESS;
			}

			osal_task_sleep(20);	
			
			try_time++;
			if(try_time >50)
			{
				return ERR_FAILED;
			}
		}
	}
}

#else 
static INT32 nim_s3202_channel_change(struct nim_device *dev, struct NIM_Channel_Change* pstChl_Change)
{
	struct nim_s3202_private *dev_priv;	
	INT32 rtn = SUCCESS;
	INT32 result;
   	UINT8 lock = 0, Tuner_Retry = 0;
	UINT8 data,data1;
	UINT8 try_time;
	UINT16 cons_time;

	static UINT8 if_agc_min_ch = 0;

	
	UINT32	freq = pstChl_Change->freq;
	UINT32	sym = pstChl_Change->sym;
	UINT8	fec   = pstChl_Change->modulation;


	UINT8 lockget;
	nim_s3202_get_lock(dev, &lockget);
	
	// Add a comparametion, when the the new parameter and the existed parameter is the same ,
	// then return success directly without set it to the tuner and demod.
	// For the request of Mark_Li 2007/11/07
	if ((((freq - S3202_CurChannelInfo.Frequency/10)<= 40)||\
		((S3202_CurChannelInfo.Frequency/10 - freq)<= 40))&&\
	    ((sym - S3202_CurChannelInfo.SymbolRate <=40)||(S3202_CurChannelInfo.SymbolRate - sym <=40))&&\
	    (S3202_CurChannelInfo.Modulation == fec)&&(lockget ==1))
	{
	    return SUCCESS;
	}

	//Up_layer send freq para is not "KHz", should be multipier 10 time to "KHz".
	freq = freq*10;

//joey 20080418. for add in initial "+150Khz" offset according to register config by xian_chen.
//joey 20080422. add init_foffset;

	freq = freq + init_foffset;

	//Patch for S3202
	if (sys_ic_get_rev_id() == IC_REV_0)
	{
		if ((fec == 0x04 && sym <= 1200) ||\
		    (fec == 0x05 && sym <= 1300) ||\
		    (fec == 0x06 && sym <= 1500) ||\
		    (fec == 0x07 && sym <= 1500) ||\
		    (fec == 0x08 && sym <= 1800) )
		{
			data = 0x26;
			data1 = 0x12;
			nim_s3202_write(NIM_S3202_CR_PRO_PATH_GAIN_3, &data, 1);
			nim_s3202_write(NIM_S3202_CR_PRO_PATH_GAIN_4, &data1, 1);
		}
		else
		{
			data = 0x24;
			data1 = 0x11;
			nim_s3202_write(NIM_S3202_CR_PRO_PATH_GAIN_3, &data, 1);
			nim_s3202_write(NIM_S3202_CR_PRO_PATH_GAIN_4, &data1, 1);
		}
	}


	// Just whther it is Catching Data Mode 
	nim_s3202_read(NIM_S3202_FSM1,&data,1);
	nim_s3202_read(NIM_S3202_FSM11,&data1,1);
	
	if(((data&0x10) == 0x10) ||(( data1&0x08) == 0x08))
	{
		return SUCCESS;
	}

	channel_change_en = TRUE;
	
	// To watch the input parameter	
	char  *char_qam;
	switch(fec)
		{
		case 4: char_qam = "16QAM";break;
		case 5: char_qam = "32QAM";break;
		case 6: char_qam = "64QAM";break;
		case 7: char_qam = "128QAM";break;
		case 8: char_qam = "256QAM";break;
		default: char_qam = "NONE"; break;
		}

	
		M3202_PRINTF("%s,freq = %d, sym = %d, fec = %s \n",__FUNCTION__, freq,sym,char_qam);

	
	// Just the input parameter's correction
	// If given an out bound data, then set it to an imposible state, to make it unlock. instead of return to ERR_FAILED
	// Because, if return a ERR_FAILED, the original set is still valid, and the demod is still in original state(eg. LOCKED)
	// even the new parameter will make it UNLOCK. so give it an imposible data to set it to realy UNLOCK is reasonable.
	// For the request of Robbin_Han 2007/12/19
	// fec == 0 means unknown qam order. Trueve 090415
#if defined (UNKNOWN_QAM_SUPPORT)
	if(((fec != 0)&&( fec != QAM16 )&& ( fec != QAM32 )&&( fec != QAM64 )&&( fec != QAM128 ) &&( fec != QAM256 ) )\
		||( (sym < 1000 ) || ( sym  > 7000) ) ||((freq < 48000 ) || ( freq > 859000)) )
#elif defined (DVBC_FREQ_EXTEND)
	if((( fec != QAM16 )&& ( fec != QAM32 )&&( fec != QAM64 )&&( fec != QAM128 ) &&( fec != QAM256 ) )\
		||( (sym < 1000 ) || ( sym  > 7000) ) ||((freq < 48000 ) || ( freq > 863000)) )
#else
	if((( fec != QAM16 )&& ( fec != QAM32 )&&( fec != QAM64 )&&( fec != QAM128 ) &&( fec != QAM256 ) )\
		||( (sym < 1000 ) || ( sym  > 7000) ) ||((freq < 48000 ) || ( freq > 859000)) )
#endif
	{	
		fec = QAM16;
		sym = 1000;
		freq = 10000;
	} 
	
	dev_priv = (struct nim_s3202_private *)dev->priv;

	
//step 1: set receiver to IDLE status, reset Interrupt indicator, and set WORK_MODE.
	data = 0x80;
	nim_s3202_write(NIM_S3202_CONTROL1, &data, 1);
	
	data = 0x00;
	nim_s3202_write(NIM_S3202_INTERRUPT_EVENTS, &data, 1);
	data = 0x0f;
	nim_s3202_write(NIM_S3202_INTERRUPT_MASK, &data, 1);


// modified by magic ( move from 2590 to here , to protect the agc0a patch)
	S3202_CurChannelInfo.Frequency = freq;
	S3202_CurChannelInfo.SymbolRate = sym;
	S3202_CurChannelInfo.Modulation = fec;

#if 1
// step 2: set tuner frequency
	//because I2c for  confige tuner use by-pass mode of demodulator. enable the function first.
	#ifdef	I2C_BYPASS
		#if (SYS_DEM_M3200C_USAGE == SYS_FUNC_ON)
		nim_s3202_read(NIM_S3202_CONTROL2, &data, 1);
		data |= 0x04; 
		nim_s3202_write(NIM_S3202_CONTROL2, &data, 1);
		#else
		nim_s3202_read(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
		data |= 0x01; 
		nim_s3202_write(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
		#endif
	#endif
	do
		{
			if(Tuner_Retry>2)
			{
				TUNER_PRINTF("ERROR! Tuner Lock Fail\n");
				lock = 0;
				break;
			}
			Tuner_Retry++;
			// Fast config tuner
			if(dev_priv->nim_Tuner_Control(dev_priv->tuner_id,freq,sym,FAST_TIMECST_AGC,_1st_i2c_cmd) != SUCCESS)		
			{
				TUNER_PRINTF("Fast Config tuner failed !\n");
			}

			// Slow config tuner
//			if(dev_priv->nim_Tuner_Control(dev_priv->tuner_id,freq,sym,SLOW_TIMECST_AGC, _1st_i2c_cmd)==ERR_FAILUE)
//			{
//				S3202_PRINTF("Slow Config tuner failed!\n");/
//			}

			// Read status
			if(dev_priv->nim_Tuner_Status(dev_priv->tuner_id,&lock) != SUCCESS)
			{
				lock = 0;
				TUNER_PRINTF("ERROR! Tuner Read Status Fail\n");
			}
			
			M3202_PRINTF("Tuner Lock Times=%d, *lock=%d !!\n",Tuner_Retry,lock);
		}while(0 == lock);
	//because I2c for  confige tuner use by-pass mode of dimodulator. disable the function after config.
	#ifdef	I2C_BYPASS
		#if (SYS_DEM_M3200C_USAGE == SYS_FUNC_ON)
		nim_s3202_read(NIM_S3202_CONTROL2, &data, 1);
		data &= 0xfb;
		nim_s3202_write(NIM_S3202_CONTROL2, &data, 1);
		#else
		nim_s3202_read(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
		data &= 0xfe;
		nim_s3202_write(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
		#endif
	#endif
	//if (lock == 0)		return ERR_FAILUE;
	if (lock == 0)		TUNER_PRINTF("tuner unlock...\n");// sometime TCL tuner will lock failure, I don't know why ^_^ 

#endif // do not control tuner

//step 3: set symbol rate and symbol rate sweep ranger
/*
	nim_s3202_set_rs(sym);  // eg. sym = 6900
	if(sym<1500)
		nim_s3202_set_search_rs(1000, sym+500);
	else if(sym>6500)
		nim_s3202_set_search_rs(sym-500, 7000);
	else
		nim_s3202_set_search_rs(sym-900, sym+300);
*/
//step 3: set symbol rate and symbol rate sweep ranger
UINT32 rs_max, rs_min;
	rs_max = (UINT32)((sym*1076)/1024.0 + 0.5);
	rs_min = (UINT32)((sym*973)/1024.0 + 0.5);
	nim_s3202_set_rs(sym);  // eg. sym = 6900
	nim_s3202_set_search_rs(rs_min, rs_max);


//step 4: set delat frequency and delata frequency sweep range
	//set delta freq and sweep range.//unit is KHz.( range from  -4096 to  +4095 KHz)
//joey 20080418. update according to register config xian_chen.
//joey 20080422. add init_foffset;
	nim_s3202_set_delfreq(init_foffset);	
	//sweep range is "0" means disable sweep.//unit is KHz. +/- range is include.
	nim_s3202_set_search_freq(1000);



//step 5: set QAM_OREDER
#ifdef UNKNOWN_QAM_SUPPORT
	// Add for unknown qam order, set default 64-QAM. Trueve 090415
	if (fec == 0)
		nim_s3202_set_qam_order(NIM_S3202_QAM_ORDER_UNKNOWN, 6);
	else
#endif
	nim_s3202_set_qam_order(NIM_S3202_QAM_ORDER_KNOWN, fec);

	//clear all interupt.
	data = 0x00;						
	nim_s3202_write(NIM_S3202_INTERRUPT_EVENTS, &data, 1);


//step 6: start capture.

	// For agc patch
	if ((sys_ic_get_rev_id() == IC_REV_1) && (rf_agc_en == TRUE))
	{
		data = if_def_val2;
		nim_s3202_write(0x12, &data, 1);
	}

// pretect for agc0a patch

	nim_s3202_read(NIM_S3202_AGC1, &data, 1);
	if ((dev_priv->tuner_config_data.AGC_REF)!=data)
	{
		data=(dev_priv->tuner_config_data.AGC_REF);
		nim_s3202_write(NIM_S3202_AGC1, &data, 1);
	}
	
//patch for 64/256 diffirent IF_min patch.
	if (dev_priv->tuner_config_ext.cChip == Tuner_Chip_CD1616LF_GIH)
	{
		if ((QAM256 == fec) && (if_agc_min_ch == 0))
		{
			data = (dev_priv->tuner_config_data.IF_AGC_MIN) - 0x0c;//0x7a=0x86-0x0c.
			nim_s3202_write(0x12,&data,1);
			if_agc_min_ch = 1;
		}
		else if ((QAM256 != fec) && (if_agc_min_ch == 1))
		{
			data = (dev_priv->tuner_config_data.IF_AGC_MIN);
			nim_s3202_write(0x12,&data,1);
			if_agc_min_ch = 0;
		}
	}

	data = 0x40;
	nim_s3202_write(NIM_S3202_CONTROL1, &data, 1);

// 
/* Modify for meeting minutes @ 2010-02-09 for fast cc */
#ifdef UNKNOWN_QAM_SUPPORT
//step 6: check dem lock or not.
	try_time = 0;
#if 0
	if (sym>6500)
		cons_time = 10;
	else if(sym > 5500)
		cons_time = 12;
	else if (sym > 4500)
		cons_time = 15;
	else if (sym > 3500)
		cons_time = 18;
	else if (sym > 2500)
		cons_time = 23;
	else if (sym > 1500)
		cons_time = 40;
	else if (sym >=1000)
		cons_time = 50;
	else
		cons_time = 60;
#endif
//Joey 20080504. According to Program guide 20080430. for the worst case lock time.
	cons_time = 40;
	
	while(1)
	{
		nim_s3202_read(NIM_S3202_MONITOR1, &data, 1);
		if ((data & 0x20) == 0x20)
		{

			// Add for unknown qam order. Trueve 090415
			if (fec == 0)
			{
				UINT8 qam = 6;
				nim_s3202_get_qam_order(dev, &qam);
				S3202_CurChannelInfo.Modulation = qam;
				//libc_printf("channel_change locked qam-order is %d\n", qam);
			}
			M3202_PRINTF("Demod Locked! Try times = %d!\n",try_time);
			rtn = SUCCESS;
			break;
		}
/*
		if (((data&0x3f) > 0) && (channel_change_en == TRUE))
		{
			channel_change_en = FALSE;
		}
*/		
		if(try_time >cons_time)// If 200ms no lock , then retrun failed.
		{
			M3202_PRINTF("Demod Unlock! Try times = %d!\n",try_time);
			rtn = ERR_FAILED;
			break;
		}
		
		osal_task_sleep(10);
		try_time++;
		
	}
#endif	
	channel_change_en = FALSE;

	return rtn;
}
#endif

static INT32 nim_s3202_quick_channel_change(struct nim_device *dev, struct NIM_Channel_Change* pstChl_Change)
{
	struct nim_s3202_private *dev_priv;	
	INT32 rtn = SUCCESS;
	INT32 result;
   	UINT8 lock = 0, Tuner_Retry = 0;
	UINT8 data,data1;
	UINT8 try_time;
	UINT16 cons_time;

	static UINT8 if_agc_min_ch = 0;

	
	UINT32	freq = pstChl_Change->freq;
	UINT32	sym = pstChl_Change->sym;
	UINT8	fec   = pstChl_Change->modulation;


	UINT8 lockget;
	nim_s3202_get_lock(dev, &lockget);
	
	// Add a comparametion, when the the new parameter and the existed parameter is the same ,
	// then return success directly without set it to the tuner and demod.
	// For the request of Mark_Li 2007/11/07
	if ((((freq - S3202_CurChannelInfo.Frequency/10)<= 40)||\
		((S3202_CurChannelInfo.Frequency/10 - freq)<= 40))&&\
	    ((sym - S3202_CurChannelInfo.SymbolRate <=40)||(S3202_CurChannelInfo.SymbolRate - sym <=40))&&\
	    (S3202_CurChannelInfo.Modulation == fec)&&(lockget ==1))
	{
	    return SUCCESS;
	}

	//Up_layer send freq para is not "KHz", should be multipier 10 time to "KHz".
	freq = freq*10;

//joey 20080418. for add in initial "+150Khz" offset according to register config by xian_chen.
//joey 20080422. add init_foffset;

	freq = freq + init_foffset;

	//Patch for S3202
	if (sys_ic_get_rev_id() == IC_REV_0)
	{
		if ((fec == 0x04 && sym <= 1200) ||\
		    (fec == 0x05 && sym <= 1300) ||\
		    (fec == 0x06 && sym <= 1500) ||\
		    (fec == 0x07 && sym <= 1500) ||\
		    (fec == 0x08 && sym <= 1800) )
		{
			data = 0x26;
			data1 = 0x12;
			nim_s3202_write(NIM_S3202_CR_PRO_PATH_GAIN_3, &data, 1);
			nim_s3202_write(NIM_S3202_CR_PRO_PATH_GAIN_4, &data1, 1);
		}
		else
		{
			data = 0x24;
			data1 = 0x11;
			nim_s3202_write(NIM_S3202_CR_PRO_PATH_GAIN_3, &data, 1);
			nim_s3202_write(NIM_S3202_CR_PRO_PATH_GAIN_4, &data1, 1);
		}
	}


	// Just whther it is Catching Data Mode 
	nim_s3202_read(NIM_S3202_FSM1,&data,1);
	nim_s3202_read(NIM_S3202_FSM11,&data1,1);
	
	if(((data&0x10) == 0x10) ||(( data1&0x08) == 0x08))
	{
		return SUCCESS;
	}

	channel_change_en = TRUE;
	
	// To watch the input parameter	
	char  *char_qam;
	switch(fec)
		{
		case 4: char_qam = "16QAM";break;
		case 5: char_qam = "32QAM";break;
		case 6: char_qam = "64QAM";break;
		case 7: char_qam = "128QAM";break;
		case 8: char_qam = "256QAM";break;
		default: char_qam = "NONE"; break;
		}

	
		M3202_PRINTF("%s,freq = %d, sym = %d, fec = %s \n",__FUNCTION__, freq,sym,char_qam);

	
	// Just the input parameter's correction
	// If given an out bound data, then set it to an imposible state, to make it unlock. instead of return to ERR_FAILED
	// Because, if return a ERR_FAILED, the original set is still valid, and the demod is still in original state(eg. LOCKED)
	// even the new parameter will make it UNLOCK. so give it an imposible data to set it to realy UNLOCK is reasonable.
	// For the request of Robbin_Han 2007/12/19
	// fec == 0 means unknown qam order. Trueve 090415
#if defined (UNKNOWN_QAM_SUPPORT)
	if(((fec != 0)&&( fec != QAM16 )&& ( fec != QAM32 )&&( fec != QAM64 )&&( fec != QAM128 ) &&( fec != QAM256 ) )\
		||( (sym < 1000 ) || ( sym  > 7000) ) ||((freq < 48000 ) || ( freq > 859000)) )
#elif defined (DVBC_FREQ_EXTEND)
	if((( fec != QAM16 )&& ( fec != QAM32 )&&( fec != QAM64 )&&( fec != QAM128 ) &&( fec != QAM256 ) )\
		||( (sym < 1000 ) || ( sym  > 7000) ) ||((freq < 48000 ) || ( freq > 863000)) )
#else
	if((( fec != QAM16 )&& ( fec != QAM32 )&&( fec != QAM64 )&&( fec != QAM128 ) &&( fec != QAM256 ) )\
		||( (sym < 1000 ) || ( sym  > 7000) ) ||((freq < 48000 ) || ( freq > 859000)) )
#endif
	{	
		fec = QAM16;
		sym = 1000;
		freq = 10000;
	} 
	
	dev_priv = (struct nim_s3202_private *)dev->priv;

	
//step 1: set receiver to IDLE status, reset Interrupt indicator, and set WORK_MODE.
	data = 0x80;
	nim_s3202_write(NIM_S3202_CONTROL1, &data, 1);
	
	data = 0x00;
	nim_s3202_write(NIM_S3202_INTERRUPT_EVENTS, &data, 1);
	data = 0x0f;
	nim_s3202_write(NIM_S3202_INTERRUPT_MASK, &data, 1);


// modified by magic ( move from 2590 to here , to protect the agc0a patch)
	S3202_CurChannelInfo.Frequency = freq;
	S3202_CurChannelInfo.SymbolRate = sym;
	S3202_CurChannelInfo.Modulation = fec;

#if 1
// step 2: set tuner frequency
	//because I2c for  confige tuner use by-pass mode of demodulator. enable the function first.
	#ifdef	I2C_BYPASS
		nim_s3202_read(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
		data |= 0x01; 
		nim_s3202_write(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
	#endif
	do
		{
			if(Tuner_Retry>2)
			{
				TUNER_PRINTF("ERROR! Tuner Lock Fail\n");
				lock = 0;
				break;
			}
			Tuner_Retry++;
			// Fast config tuner
			if(dev_priv->nim_Tuner_Control(dev_priv->tuner_id,freq,sym,FAST_TIMECST_AGC,_1st_i2c_cmd) != SUCCESS)		
			{
				TUNER_PRINTF("Fast Config tuner failed !\n");
			}

			// Slow config tuner
//			if(dev_priv->nim_Tuner_Control(dev_priv->tuner_id,freq,sym,SLOW_TIMECST_AGC, _1st_i2c_cmd)==ERR_FAILUE)
//			{
//				S3202_PRINTF("Slow Config tuner failed!\n");/
//			}

			// Read status
			if(dev_priv->nim_Tuner_Status(dev_priv->tuner_id,&lock) != SUCCESS)
			{
				lock = 0;
				TUNER_PRINTF("ERROR! Tuner Read Status Fail\n");
			}
			
			M3202_PRINTF("Tuner Lock Times=%d, *lock=%d !!\n",Tuner_Retry,lock);
		}while(0 == lock);
	//because I2c for  confige tuner use by-pass mode of dimodulator. disable the function after config.
	#ifdef	I2C_BYPASS
		nim_s3202_read(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
		data &= 0xfe;
		nim_s3202_write(NIM_S3202_I2C_CONTROL_GPIO, &data, 1);
	#endif
	//if (lock == 0)		return ERR_FAILUE;
	if (lock == 0)		TUNER_PRINTF("tuner unlock...\n");// sometime TCL tuner will lock failure, I don't know why ^_^ 

#endif // do not control tuner

//step 3: set symbol rate and symbol rate sweep ranger
/*
	nim_s3202_set_rs(sym);  // eg. sym = 6900
	if(sym<1500)
		nim_s3202_set_search_rs(1000, sym+500);
	else if(sym>6500)
		nim_s3202_set_search_rs(sym-500, 7000);
	else
		nim_s3202_set_search_rs(sym-900, sym+300);
*/
//step 3: set symbol rate and symbol rate sweep ranger
UINT32 rs_max, rs_min;
	rs_max = (UINT32)((sym*1076)/1024.0 + 0.5);
	rs_min = (UINT32)((sym*973)/1024.0 + 0.5);
	nim_s3202_set_rs(sym);  // eg. sym = 6900
	nim_s3202_set_search_rs(rs_min, rs_max);


//step 4: set delat frequency and delata frequency sweep range
	//set delta freq and sweep range.//unit is KHz.( range from  -4096 to  +4095 KHz)
//joey 20080418. update according to register config xian_chen.
//joey 20080422. add init_foffset;
	nim_s3202_set_delfreq(init_foffset);	
	//sweep range is "0" means disable sweep.//unit is KHz. +/- range is include.
	nim_s3202_set_search_freq(1000);



//step 5: set QAM_OREDER
#ifdef UNKNOWN_QAM_SUPPORT
	// Add for unknown qam order, set default 64-QAM. Trueve 090415
	if (fec == 0)
		nim_s3202_set_qam_order(NIM_S3202_QAM_ORDER_UNKNOWN, 6);
	else
#endif
	nim_s3202_set_qam_order(NIM_S3202_QAM_ORDER_KNOWN, fec);

	//clear all interupt.
	data = 0x00;						
	nim_s3202_write(NIM_S3202_INTERRUPT_EVENTS, &data, 1);


//step 6: start capture.

	// For agc patch
	if ((sys_ic_get_rev_id() == IC_REV_1) && (rf_agc_en == TRUE))
	{
		data = if_def_val2;
		nim_s3202_write(0x12, &data, 1);
	}

// pretect for agc0a patch

	nim_s3202_read(NIM_S3202_AGC1, &data, 1);
	if ((dev_priv->tuner_config_data.AGC_REF)!=data)
	{
		data=(dev_priv->tuner_config_data.AGC_REF);
		nim_s3202_write(NIM_S3202_AGC1, &data, 1);
	}
	
//patch for 64/256 diffirent IF_min patch.
	if (dev_priv->tuner_config_ext.cChip == Tuner_Chip_CD1616LF_GIH)
	{
		if ((QAM256 == fec) && (if_agc_min_ch == 0))
		{
			data = (dev_priv->tuner_config_data.IF_AGC_MIN) - 0x0c;//0x7a=0x86-0x0c.
			nim_s3202_write(0x12,&data,1);
			if_agc_min_ch = 1;
		}
		else if ((QAM256 != fec) && (if_agc_min_ch == 1))
		{
			data = (dev_priv->tuner_config_data.IF_AGC_MIN);
			nim_s3202_write(0x12,&data,1);
			if_agc_min_ch = 0;
		}
	}
	
	data = 0x40;
	nim_s3202_write(NIM_S3202_CONTROL1, &data, 1);

	channel_change_en = FALSE;

	return rtn;
}


static INT32 nim_s3202_set_perf_level(struct nim_device *dev, UINT32 level)
{
	UINT8 data[3];
	
	switch(level)
	{
		case NIM_PERF_RISK:
			data[0] = 0x32;
			data[1] = 0x32;
			data[2] = 0x46;
			break;
		case NIM_PERF_SAFER:
			data[0] = 0x64;
			data[1] = 0x32;
			data[2] = 0x47;
			break;
		case NIM_PERF_DEFAULT:
		default:
			data[0] = 0xb6;
			data[1] = 0x88;
			data[2] = 0x47;
			break;	
	}

	nim_s3202_write(NIM_S3202_EQ2, &data[2], 1);
	nim_s3202_write(NIM_S3202_EQ17, &data[0], 2);

	return SUCCESS;
}

void nim_s3202_set_qam_order(UINT8 known, UINT8 qam_order)
{
	UINT8 data;

	//CR38.
	nim_s3202_read(NIM_S3202_FSM1, &data, 1);//since there is a "work_mode" bit, in this function, should be normal mode.
	if(NIM_S3202_QAM_ORDER_KNOWN == known)
	{
		data = (UINT8)((data&0x80) | 0x40 | (qam_order&0x0f));
	} 
	else 
	{
		data = (UINT8)((data&0x80) | 0x00 | (qam_order&0x0f));
	}
	nim_s3202_write(NIM_S3202_FSM1, &data, 1);		
}

void nim_s3202_set_rs(UINT32 sym)	//Unit:KBaud
{
	//eg. sym = 6900 . SYMBOL_RATE_SET[12:0] = 6900*1024/1000
	UINT8 data;
	UINT32 sym_cal;

	sym_cal = (UINT32)((sym*1024)/1000.0 + 0.5); //0.5 for precision reasion.

	//CR39
	data = (UINT8)(sym_cal&0xFF);
	nim_s3202_write(NIM_S3202_FSM2, &data, 1);

	//CR3A
	nim_s3202_read(NIM_S3202_FSM3, &data, 1);
	data &= 0xe0;
	data |= (UINT8)((sym_cal>>8)&0x1f) ;
	nim_s3202_write(NIM_S3202_FSM3, &data, 1);
}

void nim_s3202_set_search_freq(UINT16 freq_range)	//+/-KHz
{
	UINT8 data;
	UINT16 read_data, fr_range_cal;

	fr_range_cal = (UINT16)(freq_range*1024/1000.0 + 0.5);

	//CR44.
	data = (UINT8)(fr_range_cal&0xff);
	nim_s3202_write(NIM_S3202_FSM13, &data, 1);
	//CR45.
	nim_s3202_read(NIM_S3202_FSM14, &data, 1);
	data = (UINT8)((data&0xf0) | ((fr_range_cal>>8)&0x0f));
	nim_s3202_write(NIM_S3202_FSM14, &data, 1);

	//CR42
	nim_s3202_read(NIM_S3202_FSM11, &data, 1);
	if (fr_range_cal == 0)//"0" means no freq sweep need. disable the sweep function.
	{
		data &= 0x7f;
	}
	else//enable the tr_freq sweep function.
	{
		data |= 0x80;		
	}
	nim_s3202_write(NIM_S3202_FSM11, &data, 1);
}

void nim_s3202_set_search_rs(UINT16 rs_min, UINT16 rs_max)	//KBaud
{
	UINT8 data;
	UINT16 read_data, rs_min_cal, rs_max_cal;

	rs_min_cal = (UINT16)((rs_min*1024)/1000.0+0.5);
	rs_max_cal = (UINT16)((rs_max*1024)/1000.0+0.5);
	
	//CR40
	data = (UINT8)(rs_min_cal&0xff);
	nim_s3202_write(NIM_S3202_FSM9, &data, 1);
	//CR41
	nim_s3202_read(NIM_S3202_FSM10, &data, 1);
	data = (UINT8)((data&0xe0) | ((rs_min_cal>>8)&0x1f));
	nim_s3202_write(NIM_S3202_FSM10, &data, 1 );

	//CR3E
	data = (UINT8)(rs_max_cal&0xff);
	nim_s3202_write(NIM_S3202_FSM7, &data, 1);
	//CR3F
	nim_s3202_read(NIM_S3202_FSM8, &data, 1);
	data = (UINT8)((data&0xe0) | ((rs_max_cal>>8)&0x1f));
	nim_s3202_write(NIM_S3202_FSM8, &data, 1);
}

void nim_s3202_set_delfreq(INT16 delfreq)	//+/-KHz
{
	UINT8 data;
	INT16 delfrq_cal;

	//spec need *1024 not *1000, so do the change.
	delfrq_cal = (INT16)(delfreq*1024/1000.0); 

	if(delfrq_cal > 4095) //13bit signed value, max positive is 4095.
		delfrq_cal = 4095;
	else if(delfrq_cal < -4096)
		delfrq_cal = -4096;
	
	//CR3B
	data = (UINT8)(delfrq_cal&0xff);
	nim_s3202_write(NIM_S3202_FSM4, &data, 1);
	//CR3C
	nim_s3202_read(NIM_S3202_FSM5, &data, 1);
	data = (UINT8)((data&0xe0) | ((delfrq_cal>>8)&0x1f));
	nim_s3202_write(NIM_S3202_FSM5, &data, 1);
}

#if (QAM_FPGA_USAGE == SYS_FUNC_ON)

INT32 nim_s3202_Tuner_Attatch(struct QAM_TUNER_CONFIG_API * ptrQAM_Tuner)
{


#ifdef I2C_FOR_TUNER
#undef I2C_FOR_TUNER
#endif
#if (QAM_WORK_MODE ==QAM_ONLY)
#define	I2C_FOR_TUNER  I2C_TYPE_GPIO
#elif (QAM_WORK_MODE==QAM_SOC)
#define	I2C_FOR_TUNER  I2C_TYPE_SCB
#endif


 #if (SYS_TUNER_MODULE == DCT7044)
    	{
    	ptrQAM_Tuner->tuner_config_data.RF_AGC_MAX		= 0xED;
	ptrQAM_Tuner->tuner_config_data.RF_AGC_MIN		= 0x1B;
	ptrQAM_Tuner->tuner_config_data.IF_AGC_MAX		= 0x4E;
	ptrQAM_Tuner->tuner_config_data.IF_AGC_MIN		= 0x1C;
	ptrQAM_Tuner->tuner_config_data.AGC_REF 			= 0x70;

	ptrQAM_Tuner->tuner_config_ext.cTuner_special_config = 0x00; 
	ptrQAM_Tuner->tuner_config_ext.cChip				= Tuner_Chip_PHILIPS;
	ptrQAM_Tuner->tuner_config_ext.cTuner_AGC_TOP	= 2;
	ptrQAM_Tuner->tuner_config_ext.cTuner_Base_Addr	= 0xC0;
	ptrQAM_Tuner->tuner_config_ext.cTuner_Crystal		= 4;
	ptrQAM_Tuner->tuner_config_ext.cTuner_Ref_DivRatio	= 64; 
	ptrQAM_Tuner->tuner_config_ext.cTuner_Step_Freq	= 62.5;
	//(Ref_divRatio*Step_Freq)=(80*50)=(64*62.5)=(24*166.7)
	ptrQAM_Tuner->tuner_config_ext.wTuner_IF_Freq		= 36000;
	//ptrQAM_Tuner->tuner_config_ext.cTuner_Charge_Pump= 0;  // 0, 50uA; 1, 250uA
	ptrQAM_Tuner->tuner_config_ext.i2c_type_id 			= I2C_FOR_TUNER;
	//ptrQAM_Tuner->tuner_config_ext.Tuner_Read			= i2c_read;
	//ptrQAM_Tuner->tuner_config_ext.Tuner_Write			= i2c_write;

extern INT32 tun_dct7044_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_dct7044_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);	
extern INT32 tun_dct7044_status(UINT32 tuner_id, UINT8 *lock);


       ptrQAM_Tuner->nim_Tuner_Init					= tun_dct7044_init;
	ptrQAM_Tuner->nim_Tuner_Control					= tun_dct7044_control;
	ptrQAM_Tuner->nim_Tuner_Status					= tun_dct7044_status;
	
    	}

  #elif (SYS_TUNER_MODULE == ALPSTDQE)
 	{
    	ptrQAM_Tuner->tuner_config_data.RF_AGC_MAX		= 0xBA;
	ptrQAM_Tuner->tuner_config_data.RF_AGC_MIN		= 0x2A;
	ptrQAM_Tuner->tuner_config_data.IF_AGC_MAX		= 0xFF;
	ptrQAM_Tuner->tuner_config_data.IF_AGC_MIN		= 0x00;
	ptrQAM_Tuner->tuner_config_data.AGC_REF 			= 0x82;

	ptrQAM_Tuner->tuner_config_ext.cTuner_special_config = 0x01; // RF AGC is disabled
	ptrQAM_Tuner->tuner_config_ext.cChip				= Tuner_Chip_ALPS;
	//ptrQAM_Tuner->tuner_config_ext.cTuner_AGC_TOP	= 3;
	ptrQAM_Tuner->tuner_config_ext.cTuner_Base_Addr	= 0xC0; // C0, C2, C4,C6
	ptrQAM_Tuner->tuner_config_ext.cTuner_Crystal		= 4;
	ptrQAM_Tuner->tuner_config_ext.cTuner_Ref_DivRatio	= 64;
	ptrQAM_Tuner->tuner_config_ext.cTuner_Step_Freq	= 62.5;
	//(Ref_divRatio*Step_Freq)=(80*50)=(64*62.5)=(24*166.7)=(31.25*128)
	ptrQAM_Tuner->tuner_config_ext.wTuner_IF_Freq		= 36125;   
	//ptrQAM_Tuner->tuner_config_ext.cTuner_Charge_Pump= 0;
	ptrQAM_Tuner->tuner_config_ext.i2c_type_id 			= I2C_FOR_TUNER;
	//ptrQAM_Tuner->tuner_config_ext.Tuner_Read			= i2c_read;
	//ptrQAM_Tuner->tuner_config_ext.Tuner_Write			= i2c_write;

extern INT32 tun_alpstdqe_init(UINT32* tuner_id, struct QAM_TUNER_CONFIG_EXT * ptrTuner_Config);
extern INT32 tun_alpstdqe_control(UINT32 tuner_id, UINT32 freq, UINT32 sym, UINT8 AGC_Time_Const, UINT8 _i2c_cmd);	
extern INT32 tun_alpstdqe_status(UINT32 tuner_id, UINT8 *lock);

       ptrQAM_Tuner->nim_Tuner_Init					= tun_alpstdqe_init;
	ptrQAM_Tuner->nim_Tuner_Control					= tun_alpstdqe_control;
	ptrQAM_Tuner->nim_Tuner_Status					= tun_alpstdqe_status;
 	}

 #else 
 	{
 	 S3202_PRINTF(NULL);
 	}
#endif
}
#endif

/*============================================================================*/
/**
* \fn UINT32 Log10Times100( UINT32 x)
* \brief Compute: 100*log10(x)
* \param x 32 bits
* \return 100*log10(x)
*
* 100*log10(x)
* = 100*(log2(x)/log2(10)))
* = (100*(2^15)*log2(x))/((2^15)*log2(10))
* = ((200*(2^15)*log2(x))/((2^15)*log2(10)))/2
* = ((200*(2^15)*(log2(x/y)+log2(y)))/((2^15)*log2(10)))/2
* = ((200*(2^15)*log2(x/y))+(200*(2^15)*log2(y)))/((2^15)*log2(10)))/2
*
* where y = 2^k and 1<= (x/y) < 2
*/

UINT32 Log10Times100_L( UINT32 x)
{
   static const UINT8 scale=15;
   static const UINT8 indexWidth=5;
   /*
   log2lut[n] = (1<<scale) * 200 * log2( 1.0 + ( (1.0/(1<<INDEXWIDTH)) * n ))
   0 <= n < ((1<<INDEXWIDTH)+1)
   */

   static const UINT32 log2lut[] = {
      0, 290941,  573196,  847269,1113620, 1372674, 1624818, 
      1870412, 2109788, 2343253, 2571091, 2793569,3010931, 
      3223408, 3431216, 3634553, 3833610, 4028562, 4219576, 
      4406807, 4590402, 4770499, 4947231, 5120719, 5291081, 
      5458428, 5622864, 5784489, 5943398,  6099680, 6253421, 
      6404702,  6553600  };
	  
   UINT8  i = 0;
   UINT32 y = 0;
   UINT32 d = 0;
   UINT32 k = 0;
   UINT32 r = 0;

   if (x==0) return (0);

   /* Scale x (normalize) */
   /* computing y in log(x/y) = log(x) - log(y) */
   if ( (x & (((UINT32)(-1))<<(scale+1)) ) == 0 )
   {
      for (k = scale; k>0 ; k--)
      {
        if (x & (((UINT32)1)<<scale)) break;
        x <<= 1;
      }
   } else {
      for (k = scale; k<31 ; k++)
      {
        if ((x & (((UINT32)(-1))<<(scale+1)))==0) break;
        x >>= 1;
      }
   }
   /*
     Now x has binary point between bit[scale] and bit[scale-1]
     and 1.0 <= x < 2.0 */

   /* correction for divison: log(x) = log(x/y)+log(y) */
   y = k * ( ( ((UINT32)1) << scale ) * 200 );

   /* remove integer part */
   x &= ((((UINT32)1) << scale)-1);
   /* get index */
   i = (UINT8) (x >> (scale -indexWidth));
   /* compute delta (x-a) */
   d = x & ((((UINT32)1) << (scale-indexWidth))-1);
   /* compute log, multiplication ( d* (.. )) must be within range ! */
   y += log2lut[i] + (( d*( log2lut[i+1]-log2lut[i] ))>>(scale-indexWidth));
   /* Conver to log10() */
   y /= 108853; /* (log2(10) << scale) */
   r = (y>>1);
   /* rounding */
   if (y&((UINT32)1)) r++;

   return (r);

}

void nim_s3202_register_RF_Level_cb(nim_rf_level_func_t rf_level_func)
{
	nim_s3202_RF_Level_cb = rf_level_func;
}

