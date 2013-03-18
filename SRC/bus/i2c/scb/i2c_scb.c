/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    i2c_scb.c
*
*    Description:    This file contains all globe micros and functions declare
*		             of SCB I2C driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  Apr.13.2003       Justin Wu      Ver 0.1    Create file.
*	2.  Feb.20.2006       Justin Wu      Ver 0.2    Support new SCB(Dev detect).
*	3.  Mar.27.2007       Victor Chen    Ver 0.3	Support EDDC Read.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>


#define SCB_HCR		 			0x00
#define SCB_HSR					0x01
#define SCB_IER					0x02
#define SCB_ISR					0x03
#define SCB_SAR					0x04
#define SCB_SSAR				0x05
#define SCB_HPCC				0x06
#define SCB_LPCC				       0x07
#define SCB_PSUR				0x08
#define SCB_PHDR				0x09
#define SCB_RSUR				0x0A
#define SCB_SHDR				0x0B
#define SCB_FCR					0x0C
#define SCB_FDR					0x10
#define SCB_DDC_ADDR		       0x0E
#define SCB_SEG_PTR				0x0F
#define SCB_IER1					0x20
#define SCB_ISR1					0x21
#define SCB_FIFO_TRIG_LEVEL	0x22
#define SCB_BC          				0x23
#define SCB_SSAR_EN     			0x24
#define SCB_SSAR1					0x25
#define SCB_SSAR2					0x26
#define SCB_SSAR3					0x27
/* SCB_HCR bit define */
#define SCB_HCE					0x80
#define SCB_DNEE				0x40
#define SCB_CP_WT				0x00
#define SCB_CP_CAR				0x04
#define SCB_CP_SER				0x08
#define SCB_CP_STR				0x0c
#define SCB_ST					0x01
#define SCB_EDDC				0x20
/* SCB_HSR bit define */
#define SCB_DB					0x80
#define SCB_DNE					0x40
#define SCB_HB					0x20
#define SCB_FER					0x10
#define SCB_FO					0x08
#define SCB_FU					0x04
#define SCB_FF					0x02
#define SCB_FE					0x01
/* SCB_FCR bit define */
#define SCB_FLUSH				0x80
#define SCB_BC_VAL				0x1f
/* SCB_IER bit define */
#define SCB_IERE				       0xf0

#define I2C_SCB_TIMES_OUT		100
#define I2C_SCB_RETRY			10
UINT32 g_scb_sys_chip_id = 0;
UINT32 g_scb_sys_chip_ver =0;

static struct i2c_scb_st
{
	UINT32	mutex_id;
} *i2c_scb = NULL;


#if (SYS_CHIP_MODULE == ALI_S3602)
	#if((SYS_CPU_MODULE == CPU_M6303) && (SYS_PROJECT_FE==PROJECT_FE_DVBT))//for 3105
		#define I2C_SCB_NUM				2
	#else
		#define I2C_SCB_NUM				3
	#endif
#else
#define I2C_SCB_NUM				2
#endif

static struct
{
	UINT32 reg_base;
	int    irq;
#if (SYS_CHIP_MODULE == ALI_S3601)
} i2c_scb_reg[I2C_SCB_NUM] = {{0xB8018200, 26}};
#elif (SYS_CHIP_MODULE == ALI_S3602)
	#if((SYS_CPU_MODULE == CPU_M6303) && (SYS_PROJECT_FE==PROJECT_FE_DVBT))//for 3105
} i2c_scb_reg[I2C_SCB_NUM] = {{0xB8001200, 5},{0xB8001700, 15}};
	#else
} i2c_scb_reg[I2C_SCB_NUM] = {{0xB8018200, 26},{0xB8018700,33},{0xB8018b00,34}};
	#endif
#else
} i2c_scb_reg[I2C_SCB_NUM] = {{0xB8001200, 13},{0xB8001700,35}};
#endif

#define SCB_READ8(id, reg)			(*((volatile UINT8 *)(i2c_scb_reg[id].reg_base + reg)))
#define SCB_WRITE8(id, reg, data)	(*((volatile UINT8 *)(i2c_scb_reg[id].reg_base + reg)) = (data))

INT32 i2c_scb_mode_set(UINT32 id, int bps, int en);
INT32 i2c_scb_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);
INT32 i2c_scb_write(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);
INT32 i2c_scb_write_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen);
INT32 i2c_scb_write_read_std(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen);
INT32 i2c_scb_write_write_read(UINT32 id,UINT8 segment_ptr, UINT8 slv1_addr,UINT8 slv2_addr, UINT8 *data, int rlen);
INT32 i2c_scb_eddc_read(UINT32 id,UINT8 segment_ptr, UINT8 slv1_addr,UINT8 slv2_addr, UINT8 *data, int rlen);
INT32 i2c_scb_write_plus_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen);

static unsigned char *databuf = 0;
__ATTRIBUTE_REUSE_
INT32 i2c_scb_attach(int dev_num)
{
	int i;
       g_scb_sys_chip_id = sys_ic_get_chip_id();
	g_scb_sys_chip_ver =sys_ic_get_rev_id();
	if (dev_num > 0)
	{
		dev_num = (dev_num > I2C_SCB_NUM) ? I2C_SCB_NUM : dev_num;
		//M3329E5 only contains 1 SCB
		if ((ALI_M3329E == g_scb_sys_chip_id)&& (g_scb_sys_chip_ver>=IC_REV_5 ))
		{
			dev_num = 1;
		}
		i2c_scb = (struct i2c_scb_st *)MALLOC(sizeof(struct i2c_scb_st) * dev_num);
		if (i2c_scb == NULL)
			return ERR_FAILUE;
		MEMSET(i2c_scb, 0, sizeof(struct i2c_scb_st) * dev_num);

		for (i = 0; i < dev_num; i++)
		{
			i2c_scb[i].mutex_id = osal_mutex_create();
		}

		i2c_dev[I2C_TYPE_SCB>>16].mode_set = i2c_scb_mode_set;
		i2c_dev[I2C_TYPE_SCB>>16].read = i2c_scb_read;
		i2c_dev[I2C_TYPE_SCB>>16].write = i2c_scb_write;
		i2c_dev[I2C_TYPE_SCB>>16].write_read = i2c_scb_write_read;
		i2c_dev[I2C_TYPE_SCB>>16].write_read_std = i2c_scb_write_read_std;
		i2c_dev[I2C_TYPE_SCB>>16].write_write_read = i2c_scb_write_write_read;
		i2c_dev_ext[I2C_TYPE_SCB>>16].write_plus_read=i2c_scb_write_plus_read;
	}
	if(1==sys_ic_is_M3202()&&I2C_SCB_NUM>1)
	{
		i2c_scb_reg[1].irq = 25;
	}
	return SUCCESS;
}

INT32 i2c_scb_mode_set(UINT32 id, int bps, int en)
{
	if (bps <= 0)
	{
		return ERR_FAILUE;
	}

	/*M3329E5 only contains on SCB*/
	if ((ALI_M3329E == g_scb_sys_chip_id)&&( g_scb_sys_chip_ver>=IC_REV_5)  && (id >0))
	{
		return ERR_FAILUE;
	}

	/* Disable interrupt */
	SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
	SCB_WRITE8(id, SCB_ISR, (UINT8)~SCB_IERE);

	/* Timing based on SB_CLK, for SB_CLK is 12MHz */
	SCB_WRITE8(id, SCB_HPCC, 6000000/bps);
	SCB_WRITE8(id, SCB_LPCC, 6000000/bps);

	SCB_WRITE8(id, SCB_PSUR, 6000000/bps);	/* 1/2 clock period */
	SCB_WRITE8(id, SCB_PHDR, 6000000/bps);	/* 1/2 clock period */
	SCB_WRITE8(id, SCB_RSUR, 6000000/bps);	/* 1/2 clock period */
	SCB_WRITE8(id, SCB_SHDR, 6000000/bps);	/* 1/2 clock period */

	SCB_WRITE8(id, SCB_FCR, 0x80);	/* Clear FIFO */

	if (en)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_HCE);	/* DNEE can set only when start transmit */
	} else
	{
		SCB_WRITE8(id, SCB_HCR, 0x00);
	}

	return SUCCESS;
}

static INT32 i2c_scb_wait_host_ready(UINT32 id)
{
	int i = I2C_SCB_TIMES_OUT;

	while (--i)
	{
		if ((SCB_READ8(id, SCB_HSR) & SCB_HB) == 0)
		{
			return SUCCESS;
		}
		osal_task_sleep(1);
	}

	//0x14 bit[2:0] == 0, master is idle, continue send data/cmd.
	if((SCB_READ8(id, 0x14)&0x07) == 0)
	{
		return SUCCESS;
	}
	
	PRINTF("i2c_scb_wait_host_ready: time out\n");
	return ERR_TIME_OUT;
}

static INT32 i2c_scb_wait_dev_ready(UINT32 id)
{
	int i = I2C_SCB_TIMES_OUT;
#if 0
	/* According the IC spec, we wait the SCB_HB be 1, then wait be 0 */
	while (--i)
	{
		if ((SCB_READ8(id, SCB_HSR) & SCB_HB) == 1)
		{
			break;
		}
		osal_task_sleep(1);
	}
    i = I2C_SCB_TIMES_OUT;
#endif
	while (--i)
	{
		if ((SCB_READ8(id, SCB_HSR) & (SCB_DB | SCB_DNE | SCB_HB)) == 0)
		{
			return SUCCESS;
		} else if ((SCB_READ8(id, SCB_ISR) & 0x0e) != 0)
		{
			return ERR_FAILUE;
		}
		osal_task_sleep(1);
	}

	PRINTF("i2c_scb_wait_dev_ready: time out\n");
	return ERR_TIME_OUT;
}

static INT32 i2c_scb_wait_fifo_ready(UINT32 id, int len)
{
	int i = I2C_SCB_TIMES_OUT;

	while (--i)
	{
		if ((SCB_READ8(id, SCB_FCR) & SCB_BC_VAL) == (len > 16 ? 16: len))
		{
			return SUCCESS;
		}
		osal_task_sleep(1);
	}

	PRINTF("i2c_scb_wait_fifo_ready: time out\n");
	return ERR_TIME_OUT;
}

static INT32 i2c_scb_wait_trans_done(UINT32 id)
{
	int i = I2C_SCB_TIMES_OUT;
	UINT8 status;

	while (--i)
	{
		status = SCB_READ8(id, SCB_ISR);
		if ((status & 1) != 0)
		{
			return SUCCESS;
		} else if ((status & 0x0e) != 0)
		{
			return ERR_FAILUE;
		}
		osal_task_sleep(1);
	}

	PRINTF("i2c_scb_wait_trans_done: time out\n");
	return ERR_TIME_OUT;
}

/*---------------------------------------------------
INT32 i2c_scb_read(UINT32 id, UINT8 slv_addr, UINT8 *data, UINT32 len);
	Perform a byte read process
	Stream Format:
		S<SLV_R><Read>P
		S		: Start
		P		: Stop
		<SLV_R>	: Set Slave addr & Read Mode
		<Read>	: Read Data
	Arguments:
		BYTE slv_addr - Slave Address
		BYTE reg_addr - Data address
	Return value:
		Data returned
----------------------------------------------------*/
INT32 i2c_scb_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int len)
{
	int i;
	int timeout;

	osal_mutex_lock(i2c_scb[id].mutex_id, OSAL_WAIT_FOREVER_TIME);
	if (i2c_scb_wait_host_ready(id) != SUCCESS)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}

	for (i = I2C_SCB_RETRY; i > 0; i--)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (UINT8)~SCB_IERE);
		SCB_WRITE8(id, SCB_SAR, slv_addr);

	if ((ALI_M3329E == g_scb_sys_chip_id)&& (g_scb_sys_chip_ver>=IC_REV_5 )\
		 ||(ALI_S3602F == g_scb_sys_chip_id))
	{
		SCB_WRITE8(id, SCB_BC, ((len>>5) & 0xff));
	}

		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (len & 0x1f));
		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		  SCB_CP_CAR | SCB_ST);

		if (i2c_scb_wait_dev_ready(id) == SUCCESS)
		{
			break;
		}
		osal_task_sleep(1); 			/* wait for 1mS */
	}
	if (i == 0)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}

	if (i2c_scb_wait_fifo_ready(id, len) != SUCCESS)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}

	for (i = 0; i < len; i++)
	{
		for (timeout = I2C_SCB_TIMES_OUT; timeout > 0; timeout--)
		{
		 	if (SCB_READ8(id, SCB_FCR) & 0x1F)
		 	{
		 		break;
		 	}
			osal_task_sleep(1);
		}
		if (timeout == 0)
		{
			osal_mutex_unlock(i2c_scb[id].mutex_id);
			return ERR_FAILUE;
		}
		data[i] = SCB_READ8(id, SCB_FDR);
	}
	osal_mutex_unlock(i2c_scb[id].mutex_id);

	return SUCCESS;
}

/*---------------------------------------------------
INT32 i2c_scb_write(UINT32 id, UINT8 slv_addr, UINT8 *data, UINT32 len);
	Perform bytes write process
	Stream Format:
		S<SLV_W><Write>P
		S		: Start
		P		: Stop
		<SLV_W>	: Set Slave addr & Write Mode
		<Write>	: Send Data
	Arguments:
		BYTE slv_addr - Slave Address
		BYTE value    - data to write
	Return value:
		NONE
----------------------------------------------------*/
INT32 i2c_scb_write(UINT32 id, UINT8 slv_addr, UINT8 *data, int len)
{
	int i, j;

	osal_mutex_lock(i2c_scb[id].mutex_id, OSAL_WAIT_FOREVER_TIME);
	if (i2c_scb_wait_host_ready(id) != SUCCESS)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}

	for (j = I2C_SCB_RETRY; j > 0; j--)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (UINT8)~SCB_IERE);
		SCB_WRITE8(id, SCB_SAR, slv_addr);
		SCB_WRITE8(id, SCB_SSAR, data[0]);

  	if ((ALI_M3329E == g_scb_sys_chip_id)&& (g_scb_sys_chip_ver>=IC_REV_5 )\
		 ||(ALI_S3602F == g_scb_sys_chip_id))
	 {
		SCB_WRITE8(id, SCB_BC, ((len>>5) & 0xff));
	 }
		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (len & 0x1f));

		for (i = 0; i < len; i++)
		{
			if (SCB_READ8(id, SCB_HSR) & SCB_FF)
			{
				osal_mutex_unlock(i2c_scb[id].mutex_id);
				return ERR_FAILUE;
			}
			SCB_WRITE8(id, SCB_FDR, data[i]);
		}

		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		  SCB_CP_WT | SCB_ST);

		if (i2c_scb_wait_dev_ready(id) == SUCCESS)
		{
			break;
		}
		osal_task_sleep(1); 			/* wait for 1mS */
	}
	if (j == 0)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}

	if (i2c_scb_wait_trans_done(id) != SUCCESS)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}
	osal_mutex_unlock(i2c_scb[id].mutex_id);

	return SUCCESS;
}

/*---------------------------------------------------
INT32 i2c_scb_write_read(UINT32 id, UINT8 slv_addr, UINT8 *data, UINT32 wlen, UINT32 rlen);
	Perform bytes write-read process
	Stream Format:
		S<SLV_W><Write>S<SLV_W><Read>P
		S		: Start
		P		: Stop
		<SLV_W>	: Set Slave addr & Write Mode
		<Write>	: Send Data
		<Read>	: Read Data
	Arguments:
		BYTE slv_addr - Slave Address
		BYTE value    - data to write and read
	Return value:
		NONE
----------------------------------------------------*/
INT32 i2c_scb_write_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen)
{
	int i,j;
	int timeout;

	if (wlen == 0)
	{
		return i2c_scb_read(id, slv_addr, data, rlen);
	}

	osal_mutex_lock(i2c_scb[id].mutex_id, OSAL_WAIT_FOREVER_TIME);
	if (i2c_scb_wait_host_ready(id) != SUCCESS)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}

	for (i = I2C_SCB_RETRY; i > 0; i--)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (UINT8)~SCB_IERE);
		SCB_WRITE8(id, SCB_SAR, slv_addr);
		SCB_WRITE8(id, SCB_SSAR, data[0]);

	if ((ALI_M3329E == g_scb_sys_chip_id)&& (g_scb_sys_chip_ver>=IC_REV_5 )\
		 ||(ALI_S3602F == g_scb_sys_chip_id))
	 {
		SCB_WRITE8(id, SCB_SSAR_EN, ((wlen - 1)&0x3));
		for(j = 0; j < ((wlen - 1)&0x3); j++)
			 SCB_WRITE8(id, (SCB_SSAR1 + j), data[j+1]);
		SCB_WRITE8(id, SCB_BC, ((rlen>>5) & 0xff));
	 }

		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (rlen & 0x1f));
		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		  SCB_CP_SER | SCB_ST);

		if (i2c_scb_wait_dev_ready(id) == SUCCESS)
		{
			break;
		}
		osal_task_sleep(1); 			/* wait for 1mS */
	}
	if (i == 0)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}

	if (i2c_scb_wait_fifo_ready(id, rlen) != SUCCESS)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}

	for (i = 0; i < rlen; i++)
	{
		for (timeout = I2C_SCB_TIMES_OUT; timeout > 0; timeout--)
		{
		 	if (SCB_READ8(id, SCB_FCR) & 0x1F)
		 	{
		 		break;
		 	}
			osal_task_sleep(1);
		}
		if (timeout == 0)
		{
			osal_mutex_unlock(i2c_scb[id].mutex_id);
			return ERR_FAILUE;
		}
		data[i] = SCB_READ8(id, SCB_FDR);
	}
	osal_mutex_unlock(i2c_scb[id].mutex_id);

	return SUCCESS;
}

//  Note: the following function i2c_isr() ,i2c_scb_write_multi() and i2c_scb_write_read_multi() are
//for the S3329E5 new feature
static volatile int inx = 0;
static UINT8 *ubuf;
void i2c_isr(UINT32 param)
{
	UINT8 cnt,j;
	UINT8 id = (UINT8)param;
	
	if(SCB_READ8(id, SCB_ISR1) & 0x01)
		SCB_WRITE8(id, SCB_ISR1, 0x1);
	else
		SCB_WRITE8(id, SCB_ISR, 0x1);
	cnt = (SCB_READ8(id, SCB_FCR) & 0x1f);
	//static int ii = 0;
	//*(UINT8 *)(0xa0710000 + ii++) =  cnt;
	//*(UINT8 *)(0xa0710000 + ii++) =  inx;
	
	for(j = 0; j <cnt; j++)
		ubuf[inx + j] = SCB_READ8(id, SCB_FDR);
	
	inx += cnt;
}


/*---------------------------------------------------
INT32 i2c_scb_write_multi(UINT32 id, UINT8 slv_addr, UINT8 *data, UINT32 len);
	Perform bytes write process, initially create for S3329E5 new I2C feature,
	can finish more than 16 bytes writing one time
	
	Stream Format:
		S<SLV_W><Write>P
		S		: Start
		P		: Stop
		<SLV_W>	: Set Slave addr & Write Mode
		<Write>	: Send Data
	Arguments:
		BYTE slv_addr - Slave Address
		BYTE value    - data to write
	Return value:
		NONE
----------------------------------------------------*/
INT32 i2c_scb_write_multi(UINT32 id, UINT8 slv_addr, UINT8 *data, int len)
{
	int i, j;
	int timeout;
	
	osal_mutex_lock(i2c_scb[id].mutex_id, OSAL_WAIT_FOREVER_TIME);
	if (i2c_scb_wait_host_ready(id) != SUCCESS)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}

	for (j = I2C_SCB_RETRY; j > 0; j--)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (UINT8)~SCB_IERE);
		SCB_WRITE8(id, SCB_SAR, slv_addr);
		SCB_WRITE8(id, SCB_SSAR, data[0]);
////
    		SCB_WRITE8(id, SCB_BC, ((len>>5) & 0xff));
////    
		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (len & 0x1f));


		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		  SCB_CP_WT | SCB_ST);

		for (i = 0; i < len; i++)
		{
			for (timeout = I2C_SCB_TIMES_OUT; timeout > 0; timeout--)
			{
				if (!(SCB_READ8(id, SCB_HSR) & SCB_FF))
					break;
				osal_task_sleep(1);
			}
			if (timeout == 0)
			{
				osal_mutex_unlock(i2c_scb[id].mutex_id);
				return ERR_FAILUE;
			}
			SCB_WRITE8(id, SCB_FDR, data[i]);
			//*((UINT8 *)(0xa0700000 + i)) =  data[i];
		}

		if (i2c_scb_wait_dev_ready(id) == SUCCESS)
		{
			break;
		}
		osal_task_sleep(1); 			/* wait for 1mS */
	}
	if (j == 0)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}

	if (i2c_scb_wait_trans_done(id) != SUCCESS)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}
	osal_mutex_unlock(i2c_scb[id].mutex_id);

	return SUCCESS;
}

/*---------------------------------------------------
INT32 i2c_scb_write_read_multi(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen);
	Perform bytes write-read process, initially create for S3329E5 to read more than 16 bytes
	one time
	Stream Format:
		S<SLV_W><Write>S<SLV_R><Read>P
		S		: Start
		P		: Stop
		<SLV_W>	: Set Slave addr & Write Mode
		<SLV_R>       : Set Slave addr & Read  Mode
		<Write>	: Send Data
		<Read>	: Read Data
	Arguments:
		BYTE slv_addr - Slave Address
		BYTE value    - data to write and read
	Return value:
		NONE
----------------------------------------------------*/
INT32 i2c_scb_write_read_multi(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen)
{
	int i, j;
	int timeout;

	if (wlen == 0)
	{
		return i2c_scb_read(id, slv_addr, data, rlen);
	}

	osal_mutex_lock(i2c_scb[id].mutex_id, OSAL_WAIT_FOREVER_TIME);
	if (i2c_scb_wait_host_ready(id) != SUCCESS)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}

	//for (i = I2C_SCB_RETRY; i > 0; i--)
	//{
  	inx = 0;
  	ubuf = data;
		SCB_WRITE8(id, SCB_FIFO_TRIG_LEVEL, 0x8);
		osal_interrupt_register_lsr(i2c_scb_reg[id].irq, i2c_isr, id);
  	SCB_WRITE8(id, SCB_IER1, 0x1);
  
	
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, (SCB_READ8(id, SCB_IER) & SCB_IERE)|0x01);
		SCB_WRITE8(id, SCB_ISR, (UINT8)(~(SCB_IERE | 0x01)));
		SCB_WRITE8(id, SCB_SAR, slv_addr);
		SCB_WRITE8(id, SCB_SSAR, data[0]);

		SCB_WRITE8(id, SCB_SSAR_EN, ((wlen - 1)&0x3));
		for(j = 0; j < ((wlen - 1)&0x3); j++)
			 SCB_WRITE8(id, (SCB_SSAR1 + j), data[j+1]);
		SCB_WRITE8(id, SCB_BC, ((rlen>>5) & 0xff));

		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (rlen & 0x1f));
		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		  SCB_CP_SER | SCB_ST);

	
		for (timeout = I2C_SCB_TIMES_OUT; timeout > 0; timeout--)
		{
			if(inx >= rlen)
				break;
		
			if ((SCB_READ8(id, SCB_ISR) & 0x0e) != 0)
			{
				SCB_WRITE8(id, SCB_IER1, 0x0);
				osal_interrupt_unregister_lsr(i2c_scb_reg[id].irq, i2c_isr);
				osal_mutex_unlock(i2c_scb[id].mutex_id);
				return ERR_FAILUE;
				//continue;
			}
			osal_task_sleep(1);
		}
	
		if (timeout == 0)
		{
			SCB_WRITE8(id, SCB_IER1, 0x0);
			osal_interrupt_unregister_lsr(i2c_scb_reg[id].irq, i2c_isr);
			osal_mutex_unlock(i2c_scb[id].mutex_id);
			return ERR_FAILUE;
			//continue;
		}
//	};
	SCB_WRITE8(id, SCB_IER1, 0x0);
	osal_interrupt_unregister_lsr(i2c_scb_reg[id].irq, i2c_isr);
  //UINT8 cnt;
	
	/*inx = 0;
	bbuf = data;
	for (timeout = I2C_SCB_TIMES_OUT; timeout > 0; timeout--)
	{
		if(inx >= rlen)
			break;
		osal_task_sleep(1);
	}
	if (timeout == 0)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_FAILUE;
	}
*/
	

	
  /*UINT8 cnt;
	static 		UINT8 index = 0;
	for (i = 0; i < rlen; i++)
	{
		for (timeout = I2C_SCB_TIMES_OUT; timeout > 0; timeout--)
		{
			
		 	if ((cnt = SCB_READ8(id, SCB_FCR)) & 0x1F)
		 	{
		 		*(UINT8 *)(0xa0680000 + index++) = cnt; 
		 		break;
		 	}
			osal_task_sleep(1);
		}
		if (timeout == 0)
		{
			osal_mutex_unlock(i2c_scb[id].mutex_id);
			return ERR_FAILUE;
		}
		data[i] = SCB_READ8(id, SCB_FDR);
	}*/
	osal_mutex_unlock(i2c_scb[id].mutex_id);
	return SUCCESS;
}


INT32 i2c_scb_write_read_std(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen)
{
	int i,j;
	int timeout;
	
	if (wlen == 0)
	{
		return i2c_scb_read(id, slv_addr, data, rlen);
	}
	
	osal_mutex_lock(i2c_scb[id].mutex_id, OSAL_WAIT_FOREVER_TIME);
	if (i2c_scb_wait_host_ready(id) != SUCCESS)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}
	
	for (i = I2C_SCB_RETRY; i > 0; i--)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (UINT8)~SCB_IERE);
		SCB_WRITE8(id, SCB_SAR, slv_addr);
		SCB_WRITE8(id, SCB_SSAR, data[0]);

	if ((ALI_M3329E == g_scb_sys_chip_id)&& (g_scb_sys_chip_ver>=IC_REV_5 )\
		 ||(ALI_S3602F == g_scb_sys_chip_id))
	 {
		SCB_WRITE8(id, SCB_SSAR_EN, ((wlen - 1)&0x3));
		for(j = 0; j < ((wlen - 1)&0x3); j++)
			 SCB_WRITE8(id, (SCB_SSAR1 + j), data[j+1]);
		SCB_WRITE8(id, SCB_BC, ((rlen>>5) & 0xff));
	 }
		
		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (rlen & 0x1f));
		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
			SCB_CP_STR | SCB_ST);
		
		if (i2c_scb_wait_dev_ready(id) == SUCCESS)
		{
			break;
		}
		osal_task_sleep(1); 			/* wait for 1mS */
	}
	if (i == 0)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}
	
	if (i2c_scb_wait_fifo_ready(id, rlen) != SUCCESS)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}
	
	for (i = 0; i < rlen; i++)
	{
		for (timeout = I2C_SCB_TIMES_OUT; timeout > 0; timeout--)
		{
			if (SCB_READ8(id, SCB_FCR) & 0x1F)
			{
				break;
			}
			osal_task_sleep(1);
		}
		if (timeout == 0)
		{
			osal_mutex_unlock(i2c_scb[id].mutex_id);
			return ERR_FAILUE;
		}
		data[i] = SCB_READ8(id, SCB_FDR);
	}
	osal_mutex_unlock(i2c_scb[id].mutex_id);
	
	return SUCCESS;
}

void scb_get_data(UINT32 id)
{
	unsigned char count;
	unsigned int i;

	if(databuf == 0)
	{
		libc_printf("buf address equal zero!\n");
		SDBBP();
	}
	
	count = SCB_READ8(id,SCB_FCR);

	for(i=0;i<count;i++)
	{
		*databuf = SCB_READ8(id,SCB_FDR);
		databuf++;
	}
}
INT32 i2c_scb_eddc_read(UINT32 id, UINT8 segment_ptr, UINT8 slv1_addr, UINT8 slv2_addr, UINT8 *data, int rlen)
{
	
	int i,done_flag,isr_status,isr1_status;
	int timeout;

	osal_mutex_lock(i2c_scb[id].mutex_id, OSAL_WAIT_FOREVER_TIME);
	if (i2c_scb_wait_host_ready(id) != SUCCESS)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}

	for (i = I2C_SCB_RETRY; i > 0; i--)
	{
		SCB_WRITE8(id, SCB_FIFO_TRIG_LEVEL, 0x8);
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (UINT8)~SCB_IERE);
		SCB_WRITE8(id, SCB_DDC_ADDR, slv1_addr);
		SCB_WRITE8(id, SCB_SEG_PTR, segment_ptr);
		SCB_WRITE8(id, SCB_SAR, slv2_addr);
		SCB_WRITE8(id, SCB_SSAR, data[0]);

		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (rlen & 0x1f));
		SCB_WRITE8(id, SCB_BC, (rlen>>5)&0xff);    //set the byte number expect transfer one time
#if 0                                               //fix bug edid cannt be read by C303 DB. open this code and then mask below codes when using to read edid.
		if (i2c_scb_wait_dev_ready(id) == SUCCESS)
		{
		    SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		    SCB_CP_SER | SCB_EDDC | SCB_ST);
			break;
		}
#endif

#if 1
		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		SCB_CP_SER | SCB_EDDC | SCB_ST);  
		
		if (i2c_scb_wait_dev_ready(id) == SUCCESS)
		{
			break;
		}
#endif
		//osal_task_sleep(1); 			// wait for 1mS
	}
	if (i == 0)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}

	/*if (i2c_scb_wait_fifo_ready(id, rlen) != SUCCESS)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}*/
	databuf = data;
	done_flag = 0;
	while(!done_flag){
		isr_status = SCB_READ8(id,SCB_ISR);
		isr1_status = SCB_READ8(id,SCB_ISR1);
		if(isr_status & ISR_TDI)
		{			
			scb_get_data(id);
			SCB_WRITE8(id,SCB_ISR,isr_status);
			done_flag = 1;
		}
		if(isr1_status & ISR1_TRIG)
		{
			
			scb_get_data(id);
			SCB_WRITE8(id,SCB_ISR1,isr1_status);
		}
 		if ((isr_status & 0x0e)!= 0)
		{
			osal_mutex_unlock(i2c_scb[id].mutex_id);
			return ERR_FAILUE;
		}
//		delay(1); // delay could make i2c read lost data.
	}
	
/*	for (i = 0; i < rlen; i++)
	{
		for (timeout = I2C_SCB_TIMES_OUT; timeout > 0; timeout--)
		{
		 	if (SCB_READ8(id, SCB_FCR) & 0x1F)
		 	{
		 		break;
		 	}
			osal_task_sleep(1);
		}
		if (timeout == 0)
		{
			osal_mutex_unlock(i2c_scb[id].mutex_id);
			return ERR_FAILUE;
		}
		data[i] = SCB_READ8(id, SCB_FDR);
		libc_printf("0x%2x",data[i]);
	}*/
	osal_mutex_unlock(i2c_scb[id].mutex_id);

	return SUCCESS;
}

INT32 i2c_scb_write_write_read(UINT32 id, UINT8 segment_ptr, UINT8 slv1_addr, UINT8 slv2_addr, UINT8 *data, int rlen)
{
	UINT32 count;
	UINT8 i;
	UINT8 segment_point = segment_ptr;
	UINT32 word_offset = data[0], r_lens;
	UINT32 compare_len = 128;
	UINT8 *data_buffer = data;

	count = rlen;
	
	while(count > 0)
	{
		r_lens = (count > compare_len)? compare_len : count;
		
		if(word_offset == 256)
		{
			segment_point++;
			word_offset = 0;
		}
		else if((word_offset + r_lens) > 256)
		{
			r_lens = 256 - word_offset;
		}

		*data_buffer = word_offset;
				
		if(i2c_scb_eddc_read(id, segment_point, slv1_addr, slv2_addr, data_buffer, r_lens) != SUCCESS)
		{
			PRINTF("read error!\n");
			return !SUCCESS;
		}
		
		word_offset += r_lens;
		data_buffer += r_lens;
		count -= r_lens;
	}

	return SUCCESS;
}
static INT32 i2c_scb_write_internal(UINT32 id, UINT8 slv_addr, UINT8 *data, int len)
{
	int i, j;
	
	if (i2c_scb_wait_host_ready(id) != SUCCESS)
	{
		//osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}

	for (j = I2C_SCB_RETRY; j > 0; j--)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (UINT8)~SCB_IERE);
		SCB_WRITE8(id, SCB_SAR, slv_addr);
		SCB_WRITE8(id, SCB_SSAR, data[0]);


		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (len & 0x1f));

		for (i = 0; i < len; i++)
		{
			if (SCB_READ8(id, SCB_HSR) & SCB_FF)
			{
				//osal_mutex_unlock(i2c_scb[id].mutex_id);
				return ERR_FAILUE;
			}
			SCB_WRITE8(id, SCB_FDR, data[i]);
		}

		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		  SCB_CP_WT | SCB_ST);

		if (i2c_scb_wait_dev_ready(id) == SUCCESS)
		{
			break;
		}
		osal_task_sleep(1); 			/* wait for 1mS */
	}
	if (j == 0)
	{
		//osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}

	if (i2c_scb_wait_trans_done(id) != SUCCESS)
	{
		//osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}
	return SUCCESS;
}


static INT32 i2c_scb_read_internal(UINT32 id, UINT8 slv_addr, UINT8 *data, int len)
{
	int i;
	int timeout;
		
	if (i2c_scb_wait_host_ready(id) != SUCCESS)
	{
		//osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}

	for (i = I2C_SCB_RETRY; i > 0; i--)
	{
		SCB_WRITE8(id, SCB_HCR, SCB_READ8(id, SCB_HCR) & 0x80);	/* Clear */
		SCB_WRITE8(id, SCB_HSR, 0x00);
		SCB_WRITE8(id, SCB_IER, SCB_READ8(id, SCB_IER) & SCB_IERE);
		SCB_WRITE8(id, SCB_ISR, (UINT8)~SCB_IERE);
		SCB_WRITE8(id, SCB_SAR, slv_addr);

		SCB_WRITE8(id, SCB_FCR, SCB_FLUSH | (len & 0x1f));
		SCB_WRITE8(id, SCB_HCR, (SCB_READ8(id, SCB_HCR) & 0x80) | SCB_DNEE | \
		  SCB_CP_CAR | SCB_ST);

		if (i2c_scb_wait_dev_ready(id) == SUCCESS)
		{
			break;
		}
		osal_task_sleep(1); 			/* wait for 1mS */
	}
	if (i == 0)
	{
		//osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}

	if (i2c_scb_wait_fifo_ready(id, len) != SUCCESS)
	{
		//osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ERR_TIME_OUT;
	}

	for (i = 0; i < len; i++)
	{
		for (timeout = I2C_SCB_TIMES_OUT; timeout > 0; timeout--)
		{
		 	if (SCB_READ8(id, SCB_FCR) & 0x1F)
		 	{
		 		break;
		 	}
			osal_task_sleep(1);
		}
		if (timeout == 0)
		{
			//osal_mutex_unlock(i2c_scb[id].mutex_id);
			return ERR_FAILUE;
		}
		data[i] = SCB_READ8(id, SCB_FDR);
	}
	return SUCCESS;
}
/*---------------------------------------------------
INT32 i2c_scb_write_plus_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen);
	Perform bytes read by the following 2 steps: 
	1. write a register address to device which we want to read from
	Start<Write SLV_ADDR><Write register addr>STOP
	2. read bytes
	Start<Write SLV_ADDR><Read byte0><Read byte1>.....<Read byten>STOP

	Arguments:
		slv_addr - Slave Address
		data    - data[0]:register address we want to read from
		rlen - data length we want to read
	Return value:
		SUCCESS 
		ERR_FAILUE
		ERR_TIME_OUT
----------------------------------------------------*/

INT32 i2c_scb_write_plus_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen)
{	
	INT32 ret;
	
	osal_mutex_lock(i2c_scb[id].mutex_id, OSAL_WAIT_FOREVER_TIME);
	ret = i2c_scb_write_internal(id, slv_addr, data, wlen);
	if(ret != SUCCESS)
	{
		osal_mutex_unlock(i2c_scb[id].mutex_id);
		return ret;
	}
	ret = i2c_scb_read_internal(id, slv_addr, data, rlen);
	osal_mutex_unlock(i2c_scb[id].mutex_id);

	return ret;
}

