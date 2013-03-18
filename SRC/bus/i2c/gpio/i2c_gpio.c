/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    i2c_gpio.c
*
*    Description:    This file contains all globe micros and functions declare
*		             of GPIO I2C driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Feb.12.2003       Justin Wu      Ver 0.1    Create file.
*	2.
*****************************************************************************/

#include <sys_config.h>

#include <types.h>
#include <retcode.h>
#include <hal/hal_gpio.h>
#include <bus/i2c/i2c.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>


#define I2C_GPIO_TIMES_OUT		10
//#define EXTERNAL_PULL_HIGH		TRUE
static struct i2c_gpio_st
{
	UINT32 mutex_id;
	UINT16 clock_period;
} *i2c_gpio = NULL;

/* For M3327/M3327C serial chip */
#if (SYS_MAIN_BOARD == BOARD_S3601_DEMO)
#define I2C_GPIO_NUM			2	//for hdmi - cmchen
static struct
{
	int sda;
	int scl;
} i2c_gpio_reg[I2C_GPIO_NUM] = {{SYS_I2C_SDA, SYS_I2C_SCL},{SYS_I2C_SDA1, SYS_I2C_SCL1}};	//cmchen
#elif defined(SYS_I2C_SCL2) && defined(SYS_I2C_SDA2)
#ifdef _ISDBT_ENABLE_
#define I2C_GPIO_NUM			3
static struct
{
	int sda;
	int scl;
} i2c_gpio_reg[I2C_GPIO_NUM] = {{SYS_I2C_SDA1, SYS_I2C_SCL1}, {SYS_I2C_SDA, SYS_I2C_SCL},{SYS_I2C_SDA2, SYS_I2C_SCL2}}; 
#else
#define I2C_GPIO_NUM			3
static struct
{
	int sda;
	int scl;
} i2c_gpio_reg[I2C_GPIO_NUM] = {{SYS_I2C_SDA, SYS_I2C_SCL}, {SYS_I2C_SDA, SYS_I2C_SCL},{SYS_I2C_SDA2, SYS_I2C_SCL2}}; 
#endif
#else
#define I2C_GPIO_NUM			2
static struct
{
	int sda;
	int scl;
} i2c_gpio_reg[I2C_GPIO_NUM] = {{SYS_I2C_SDA, SYS_I2C_SCL}, {SYS_I2C_SDA, SYS_I2C_SCL}}; 

#endif
#define SET_SDA_OUT(id)	HAL_GPIO_BIT_DIR_SET(i2c_gpio_reg[id].sda, HAL_GPIO_O_DIR)
#define SET_SDA_IN(id)	HAL_GPIO_BIT_DIR_SET(i2c_gpio_reg[id].sda, HAL_GPIO_I_DIR)
#if( EXTERNAL_PULL_HIGH == TRUE)
#define SET_SDA_HI(id)	SET_SDA_IN(id)
#define SET_SDA_LO(id)	SET_SDA_OUT(id);  HAL_GPIO_BIT_SET(i2c_gpio_reg[id].sda, 0)
#else
#define SET_SDA_HI(id)	HAL_GPIO_BIT_SET(i2c_gpio_reg[id].sda, 1)
#define SET_SDA_LO(id)	HAL_GPIO_BIT_SET(i2c_gpio_reg[id].sda, 0)
#endif
#define GET_SDA(id)		HAL_GPIO_BIT_GET(i2c_gpio_reg[id].sda)

#define SET_SCL_OUT(id)	HAL_GPIO_BIT_DIR_SET(i2c_gpio_reg[id].scl, HAL_GPIO_O_DIR)
#define SET_SCL_IN(id)	HAL_GPIO_BIT_DIR_SET(i2c_gpio_reg[id].scl, HAL_GPIO_I_DIR)
#if( EXTERNAL_PULL_HIGH == TRUE)
#define SET_SCL_HI(id)	SET_SCL_IN(id)
#define SET_SCL_LO(id)	SET_SCL_OUT(id); HAL_GPIO_BIT_SET(i2c_gpio_reg[id].scl, 0)
#else
#define SET_SCL_HI(id)	HAL_GPIO_BIT_SET(i2c_gpio_reg[id].scl, 1)
#define SET_SCL_LO(id)	HAL_GPIO_BIT_SET(i2c_gpio_reg[id].scl, 0)
#endif
#define GET_SCL(id)		HAL_GPIO_BIT_GET(i2c_gpio_reg[id].scl)

INT32 i2c_gpio_mode_set(UINT32 id, int bps, int en);
INT32 i2c_gpio_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);
INT32 i2c_gpio_write(UINT32 id, UINT8 slv_addr, UINT8 *data, int len);
INT32 i2c_gpio_write_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen);
INT32 i2c_gpio_write_write_read(UINT32 id, UINT8 segment_ptr, UINT8 slv1_addr, UINT8 slv2_addr, UINT8 *data, int rlen);
INT32 i2c_gpio_write_read_std(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen);
INT32 i2c_gpio_write_plus_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen);

void  i2c_gpio_set(UINT32 id, UINT8 sda, UINT8 scl)
{
	if(id<I2C_GPIO_NUM)
	{
		i2c_gpio_reg[id].sda = sda;
		i2c_gpio_reg[id].scl = scl;
	}
}

INT32 i2c_gpio_attach(int dev_num)
{
	int i;

	if (dev_num > 0)
	{
		dev_num = (dev_num > I2C_GPIO_NUM) ? I2C_GPIO_NUM : dev_num;
		i2c_gpio = (struct i2c_gpio_st *)MALLOC(sizeof(struct i2c_gpio_st) * dev_num);
		if (i2c_gpio == NULL)
			return ERR_FAILUE;
		MEMSET(i2c_gpio, 0, sizeof(struct i2c_gpio_st) * dev_num);

		for (i = 0; i < dev_num; i++)
		{
			i2c_gpio[i].mutex_id = osal_mutex_create();
			i2c_gpio[i].clock_period = 0;
		}

		i2c_dev[I2C_TYPE_GPIO>>16].mode_set = i2c_gpio_mode_set;
		i2c_dev[I2C_TYPE_GPIO>>16].read = i2c_gpio_read;
		i2c_dev[I2C_TYPE_GPIO>>16].write = i2c_gpio_write;
		i2c_dev[I2C_TYPE_GPIO>>16].write_read = i2c_gpio_write_read;
		i2c_dev[I2C_TYPE_GPIO>>16].write_read_std = i2c_gpio_write_read_std;	
		i2c_dev[I2C_TYPE_GPIO>>16].write_write_read = i2c_gpio_write_write_read;
		i2c_dev_ext[I2C_TYPE_GPIO>>16].write_plus_read = i2c_gpio_write_plus_read;
	}
	return SUCCESS;
}

/* I2C band rate: 1/i2c_gpio[id].clock_period(uS). */

/*---------------------------------------------------
int i2c_gpio_phase_start(id);
	Generate i2c_gpio_phase_start Condition:
	Stream Format:
		SCL   _____/--------\___
		SDA   =---------\_____
		width (4.7u)4.7u|4.7u|
	Arguments:
		NONE
	Return value:
		int SUCCESS				0
		int ERR_I2C_SCL_LOCK	1
		int ERR_I2C_SDA_LOCK	2
----------------------------------------------------*/
static INT32 i2c_gpio_phase_start(UINT32 id)
{
	/* Make sure is out */
#if( EXTERNAL_PULL_HIGH != TRUE)	
	SET_SDA_OUT(id);
	SET_SCL_OUT(id);
#endif

	SET_SDA_HI(id);		/* Set SDA high */
	if (!GET_SCL(id))
	{
		osal_delay(i2c_gpio[id].clock_period);
	}

	SET_SCL_HI(id);		/* Set SCL high */
	osal_delay(i2c_gpio[id].clock_period);
	if(!GET_SCL(id))
	{
		return ERR_I2C_SCL_LOCK;
	}

	if(!GET_SDA(id))
	{
		return ERR_I2C_SDA_LOCK;
	}

	SET_SDA_LO(id);
	osal_delay(i2c_gpio[id].clock_period);
	SET_SCL_LO(id);

	return SUCCESS;
}

/*---------------------------------------------------
int i2c_gpio_phase_stop(id);
	Generate i2c_gpio_phase_stop Condition:
	Stream Format:
		SCL   _____/-------------------------------
		SDA   __________/--------------------------
		width  4.7u|4.7u|4.7u from next i2c_gpio_phase_start bit
	Arguments:
		NONE
	Return value:
		int SUCCESS				0
		int ERR_I2C_SCL_LOCK	1
		int ERR_I2C_SDA_LOCK	2
----------------------------------------------------*/
static INT32 i2c_gpio_phase_stop(UINT32 id)
{
	/* Make sure is out */
#if( EXTERNAL_PULL_HIGH != TRUE)	
	SET_SDA_OUT(id);
	SET_SCL_OUT(id);
#endif
	SET_SDA_LO(id);
	osal_delay(i2c_gpio[id].clock_period);
	SET_SCL_HI(id);
	osal_delay(i2c_gpio[id].clock_period);
	if (!GET_SCL(id))
	{
		return ERR_I2C_SCL_LOCK;
	}

	SET_SDA_HI(id);
	osal_delay(2);
	if (!GET_SDA(id))
	{
		return ERR_I2C_SDA_LOCK;
	}

	return SUCCESS;
}

/*---------------------------------------------------
void i2c_gpio_phase_set_bit(id, int val);
	Set a BIT (Hi or Low)
	Stream Format:
		SCL   _____/---\
		SDA   ??AAAAAAAA
		width  4.7u| 4u|
	Arguments:
		int i	: Set(1) or Clear(0) this bit on iic bus
	Return value:
		NONE
----------------------------------------------------*/
static void i2c_gpio_phase_set_bit(UINT32 id, int val)
{
	/* Make sure is out */
#if( EXTERNAL_PULL_HIGH != TRUE)	
	SET_SDA_OUT(id);
	SET_SCL_OUT(id);
#endif
	SET_SCL_LO(id);
	if(val)
	{
		SET_SDA_HI(id);
	}
	else
	{
		SET_SDA_LO(id);
	}
	osal_delay(i2c_gpio[id].clock_period);

	SET_SCL_HI(id);
	osal_delay(i2c_gpio[id].clock_period);
	SET_SCL_LO(id);

	return;
}

/*---------------------------------------------------
int i2c_gpio_phase_get_bit(id);
	Set a BIT (Hi or Low)
	Stream Format:
		SCL   _____/---\
		SDA   ??AAAAAAAA
		width  4.7u| 4u|
	Arguments:
		NONE
	Return value:
		int i	: Set(1) or Clear(0) this bit on iic bus
----------------------------------------------------*/
static int i2c_gpio_phase_get_bit(UINT32 id)
{
	int ret = 0;

	SET_SDA_IN(id);

	SET_SDA_HI(id);				/* Hi Ind */
	osal_delay(i2c_gpio[id].clock_period);
	SET_SCL_HI(id);
	osal_delay(i2c_gpio[id].clock_period);
	ret = GET_SDA(id);
	SET_SCL_LO(id);

	return ret;
}

/*---------------------------------------------------
int i2c_gpio_phase_set_byte(UINT32 id, UINT8 data);
	Perform a byte write process
	Stream Format:
		SCL   ___/-\___/-\___/-\___/-\___/-\___/-\___/-\___/-\__/-\
		SDA   --< B7>-< B6>-< B5>-< B4>-< B3>-< B2>-< B1>-< B0>-Check
		Clock Low: 4.7u, High: 4.0u.                            Ack
    	Data exchanged at CLK Low, ready at SCL High
	Arguments:
		char data	- Data to send on iic bus
	return value:
		The /ack signal returned from slave
----------------------------------------------------*/
static INT32 i2c_gpio_phase_set_byte(UINT32 id, UINT8 data)
{
	int i;

	for (i = 0; i < 8; i++)
	{
		if (data & 0x80)
		{
			i2c_gpio_phase_set_bit(id, 1);
		}
		else
		{
			i2c_gpio_phase_set_bit(id, 0);
		}

		data <<= 1;
	}

	return(i2c_gpio_phase_get_bit(id));
}

/*---------------------------------------------------
char i2c_gpio_phase_get_byte(UINT32 id, int ack);
	Perform a byte read process
			by Charlemagne Yue
	SCL   ___/-\___/-\___/-\___/-\___/-\___/-\___/-\___/-\___/-\
	SDA   --< B7>-< B6>-< B5>-< B4>-< B3>-< B2>-< B1>-< B0>-(Ack)
	Clock Low: 4.7u, High: 4.0u.
    Data exchanged at CLK Low, ready at SCL High
----------------------------------------------------*/
static UINT8 i2c_gpio_phase_get_byte(UINT32 id, int ack)
{
	UINT8 ret = 0;
	int i;

	for (i = 0; i < 8; i++)
	{
		ret <<= 1;
		ret |= i2c_gpio_phase_get_bit(id);
	}
	i2c_gpio_phase_set_bit(id, ack);

	return ret;
}


/*---------------------------------------------------
INT32 i2c_gpio_read_no_stop(UINT32 id, UINT8 slv_addr, UINT8 *data, UINT32 len);
	Perform bytes read process but no stop
	Stream Format:
		S<SLV_R><Read>
		S		: Start
		<SLV_R>	: Set Slave addr & Read Mode
		<Read>	: Read Data
	Arguments:
		BYTE slv_addr - Slave Address
		BYTE reg_addr - Data address
	Return value:
		Data returned
----------------------------------------------------*/
static INT32 i2c_gpio_read_no_stop(UINT32 id, UINT8 slv_addr, UINT8 *data, int len)
{
	int i = I2C_GPIO_TIMES_OUT;

	slv_addr |= 1;						/* Read */
	while (--i)							/* Ack polling !! */
	{
		i2c_gpio_phase_start(id);
		/* has /ACK => i2c_gpio_phase_start transfer */
		if(!i2c_gpio_phase_set_byte(id, slv_addr))
		{
			break;
		}
		/* device is busy, issue i2c_gpio_phase_stop and chack again later */
		i2c_gpio_phase_stop(id);
		osal_task_sleep(1); 			/* wait for 1mS */
	}

	if (i == 0)
	{
		return ERR_TIME_OUT;
	}

	for (i = 0; i < (len - 1); i++)
	{
		/*with no /ack to stop process */
		data[i] = i2c_gpio_phase_get_byte(id, 0);
	}
	data[len - 1] = i2c_gpio_phase_get_byte(id, 1);

	return SUCCESS;
}

/*---------------------------------------------------
INT32 i2c_gpio_write_no_stop(UINT32 id, UINT8 slv_addr, UINT8 *data, UINT32 len);
	Perform bytes write process but no stop
	Stream Format:
		S<SLV_W><Write>
		S		: Start
		<SLV_W>	: Set Slave addr & Write Mode
		<Write>	: Send Data
	Arguments:
		BYTE slv_addr - Slave Address
		BYTE value    - data to write
	Return value:
		NONE
----------------------------------------------------*/
INT32 i2c_gpio_write_no_stop(UINT32 id, UINT8 slv_addr, UINT8 *data, int len)
{
	int i = I2C_GPIO_TIMES_OUT;

	slv_addr &= 0xFE;					/*Write*/
	while (--i)							/* Ack polling !! */
	{
		i2c_gpio_phase_start(id);
		/* has /ACK => i2c_gpio_phase_start transfer */
		if(!i2c_gpio_phase_set_byte(id, slv_addr))
		{
			break;
		}
		/* device is busy, issue i2c_gpio_phase_stop and chack again later */
		i2c_gpio_phase_stop(id);
		osal_task_sleep(1); 			/* wait for 1mS */
	}

	if (i == 0)
	{
		return ERR_TIME_OUT;
	}

	for (i = 0; i < len; i++){
		i2c_gpio_phase_set_byte(id, data[i]);
	}

	return SUCCESS;
}

static INT32 i2c_gpio_write_no_stop_no_ack(UINT32 id, UINT8 slv_addr, UINT8 *data, int len)
{
	int i = I2C_GPIO_TIMES_OUT;

	slv_addr &= 0xFE;					/*Write*/
//	while (--i)							/* Ack polling !! */
//	{
		i2c_gpio_phase_start(id);
		/* has /ACK => i2c_gpio_phase_start transfer */
//		if(!i2c_gpio_phase_set_byte(id, slv_addr))
		i2c_gpio_phase_set_byte(id, slv_addr);
		{
//			break;
		}
		/* device is busy, issue i2c_gpio_phase_stop and chack again later */
		//i2c_gpio_phase_stop(id);
		//osal_task_sleep(1); 			/* wait for 1mS */
//	}

	if (i == 0)
	{
		return ERR_TIME_OUT;
	}

	for (i = 0; i < len; i++){
		i2c_gpio_phase_set_byte(id, *data);
	}

	return SUCCESS;
}

/*---------------------------------------------------
INT32 i2c_gpio_mode_set(int bps, int en);
----------------------------------------------------*/
INT32 i2c_gpio_mode_set(UINT32 id, int bps, int en)
{
	if (bps <= 0)
	{
		i2c_gpio[id].clock_period = 5;	/* 100K bps */
		return ERR_FAILUE;
	}
	i2c_gpio[id].clock_period = 500000 / bps;
	SET_SDA_HI(id);
	SET_SCL_HI(id);
#if( EXTERNAL_PULL_HIGH != TRUE)
	if (en)
	{
		SET_SCL_OUT(id);
	} else
	{
		SET_SCL_IN(id);
	}
#endif
	return SUCCESS;
}

/*---------------------------------------------------
INT32 i2c_gpio_read(UINT32 id, UINT8 slv_addr, UINT8 *data, UINT32 len);
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
INT32 i2c_gpio_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int len)
{
	INT32 ret;

	osal_mutex_lock(i2c_gpio[id].mutex_id, OSAL_WAIT_FOREVER_TIME);
	if ((ret = i2c_gpio_read_no_stop(id, slv_addr, data, len)) != SUCCESS)
	{
		osal_mutex_unlock(i2c_gpio[id].mutex_id);
		return ret;
	}
	i2c_gpio_phase_stop(id);
	osal_mutex_unlock(i2c_gpio[id].mutex_id);

	return SUCCESS;
}

/*---------------------------------------------------
INT32 i2c_gpio_write(UINT8 slv_addr, UINT8 *data, UINT32 len);
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
INT32 i2c_gpio_write(UINT32 id, UINT8 slv_addr, UINT8 *data, int len)
{
	INT32 ret;

	osal_mutex_lock(i2c_gpio[id].mutex_id, OSAL_WAIT_FOREVER_TIME);
	if ((ret = i2c_gpio_write_no_stop(id, slv_addr, data, len)) != SUCCESS)
	{
		osal_mutex_unlock(i2c_gpio[id].mutex_id);
		return ret;
	}
	i2c_gpio_phase_stop(id);
	osal_mutex_unlock(i2c_gpio[id].mutex_id);

	return SUCCESS;
}

/*---------------------------------------------------
INT32 i2c_gpio_write_read(UINT8 slv_addr, UINT8 *data, UINT32 wlen, UINT32 rlen);
	Perform bytes write-read process
	Stream Format:
		S<SLV_W><Write>S<SLV_W><Read>
		S		: Start
		<SLV_W>	: Set Slave addr & Write Mode
		<Write>	: Send Data
		<Read>	: Read Data
	Arguments:
		BYTE slv_addr - Slave Address
		BYTE value    - data to write and read
	Return value:
		NONE
----------------------------------------------------*/
INT32 i2c_gpio_write_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen)
{
	INT32 i, ret;

	if (wlen == 0)
	{
		return i2c_gpio_read(id, slv_addr, data, rlen);
	}

	osal_mutex_lock(i2c_gpio[id].mutex_id, OSAL_WAIT_FOREVER_TIME);
	if ((ret = i2c_gpio_write_no_stop(id, slv_addr, data, wlen)) != SUCCESS)
	{
		osal_mutex_unlock(i2c_gpio[id].mutex_id);
		return ret;
	}
#ifdef SPECIAL_I2C_REQUEST
	if (id == 2)
	{
		for (i = 0; i < (rlen - 1); i++)
		{
			/*with ack to stop process */
			data[i] = i2c_gpio_phase_get_byte(id, 0);
		}
		data[rlen - 1] = i2c_gpio_phase_get_byte(id, 1);

	}	
	else
#endif		
	{
		if ((ret = i2c_gpio_read_no_stop(id, slv_addr, data, rlen)) != SUCCESS)
		{
			osal_mutex_unlock(i2c_gpio[id].mutex_id);
			return ret;
		}
	}
	i2c_gpio_phase_stop(id);
	osal_mutex_unlock(i2c_gpio[id].mutex_id);

	return SUCCESS;
}

/*---------------------------------------------------
INT32 i2c_gpio_write_read(UINT8 slv_addr, UINT8 *data, UINT32 wlen, UINT32 rlen);
	Perform bytes write-read process
	Stream Format:
		S<SLV_W><Write>S<SLV_W><Read>
		S		: Start
		<SLV_W>	: Set Slave addr & Write Mode
		<Write>	: Send Data
		<Read>	: Read Data
	Arguments:
		BYTE slv_addr - Slave Address
		BYTE value    - data to write and read
	Return value:
		NONE
----------------------------------------------------*/
INT32 i2c_gpio_mbyte_write_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen)
{
	INT32 i, ret;
	UINT8 msection[2];

	if (wlen == 0)
	{
		return i2c_gpio_read(id, slv_addr, data, rlen);
	}

	osal_mutex_lock(i2c_gpio[id].mutex_id, OSAL_WAIT_FOREVER_TIME);
	if ((ret = i2c_gpio_write_no_stop(id, slv_addr, data, wlen)) != SUCCESS)
	{
		osal_mutex_unlock(i2c_gpio[id].mutex_id);
		return ret;
	}	

	msection[0]=0xFE;		
	msection[1]=0xC0|0x1;

	if ((ret = i2c_gpio_write_no_stop(id, slv_addr, msection, 2)) != SUCCESS)
	{
		osal_mutex_unlock(i2c_gpio[id].mutex_id);
		return ret;
	}	
	
	//{
		if ((ret = i2c_gpio_read_no_stop(id, slv_addr, data, rlen)) != SUCCESS)
		{
			osal_mutex_unlock(i2c_gpio[id].mutex_id);
			return ret;
		}
	//}
	i2c_gpio_phase_stop(id);
	osal_mutex_unlock(i2c_gpio[id].mutex_id);

	return SUCCESS;
}


/*---------------------------------------------------
INT32 i2c_gpio_write_write_read(UINT8 slv_addr, UINT8 *data, UINT32 wlen, UINT32 rlen);
	Perform bytes write-write-read process, specailly for E-DDC
	Stream Format:
		S<SLV_W><Write>S<SLV_W><Read>
		S		: Start
		<SLV_W>	: Set Slave addr & Write Mode
		<Write>	: Send Data
		<Read>	: Read Data
	Arguments:
		BYTE slv_addr - Slave Address
		BYTE value    - data to write and read
	Return value:
		NONE
----------------------------------------------------*/
INT32 i2c_gpio_write_write_read(UINT32 id,UINT8 segment_ptr, UINT8 slv1_addr,UINT8 slv2_addr, UINT8 *data, int rlen)
{
	INT32 ret;

	osal_mutex_lock(i2c_gpio[id].mutex_id, OSAL_WAIT_FOREVER_TIME);
#if 1
	if ((ret = i2c_gpio_write_no_stop_no_ack(id, slv1_addr, &segment_ptr, 1)) != SUCCESS)
	{
		osal_mutex_unlock(i2c_gpio[id].mutex_id);
		return ret;
	}
#endif
	if ((ret = i2c_gpio_write_no_stop(id, slv2_addr, data, 1)) != SUCCESS)
	{
		osal_mutex_unlock(i2c_gpio[id].mutex_id);
		return ret;
	}

	if ((ret = i2c_gpio_read_no_stop(id, slv2_addr, data, rlen)) != SUCCESS)
	{
		osal_mutex_unlock(i2c_gpio[id].mutex_id);
		return ret;
	}
	i2c_gpio_phase_stop(id);
	osal_mutex_unlock(i2c_gpio[id].mutex_id);

	return SUCCESS;
}

INT32 i2c_gpio_write_read_std(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen)
{
	INT32 i, ret;

	if (wlen == 0)
	{
		return i2c_gpio_read(id, slv_addr, data, rlen);
	}
	osal_mutex_lock(i2c_gpio[id].mutex_id, OSAL_WAIT_FOREVER_TIME);
	if ((ret = i2c_gpio_write_no_stop(id, slv_addr, data, wlen)) != SUCCESS)
	{
		osal_mutex_unlock(i2c_gpio[id].mutex_id);
		return ret;
	}
	i2c_gpio_phase_stop(id);
	if ((ret = i2c_gpio_read_no_stop(id, slv_addr, data, rlen)) != SUCCESS)
	{
		osal_mutex_unlock(i2c_gpio[id].mutex_id);
		return ret;
	}
	i2c_gpio_phase_stop(id);
	osal_mutex_unlock(i2c_gpio[id].mutex_id);

	return SUCCESS;
}

INT32 i2c_gpio_write_plus_read(UINT32 id, UINT8 slv_addr, UINT8 *data, int wlen, int rlen)
{	
	INT32 ret;
	
	osal_mutex_lock(i2c_gpio[id].mutex_id, OSAL_WAIT_FOREVER_TIME);
	ret = i2c_gpio_write_no_stop(id, slv_addr, data, wlen);
	if(ret != SUCCESS)
	{
		osal_mutex_unlock(i2c_gpio[id].mutex_id);
		return ret;
	}
	ret = i2c_gpio_read_no_stop(id, slv_addr, data, rlen);
	osal_mutex_unlock(i2c_gpio[id].mutex_id);

	return ret;
}

