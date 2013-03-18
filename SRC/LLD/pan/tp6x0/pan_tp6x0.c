/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2010 Copyright (C)
*
*    File:    pan_ptp6x0.c
*
*    Description:    This file contains all functions definition
*		             of Front Panel driver.
*
*    Note:
*    This driver is used to support tp6x0 front panel controller. 
*
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	2010.4.25      Trueve Hu        Ver 0.1     Create file.
*****************************************************************************/

#include <sys_config.h>

#include <retcode.h>
#include <types.h>

#include <api/libc/alloc.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hal/hal_gpio.h>
#include <hld/hld_dev.h>
#include <hld/pan/pan.h>
#include <hld/pan/pan_dev.h>


#define PW_CLK	2
#define T_WAIT	2

//#define ESC_CMD_TP6X0
//#define FD620_LED5_COL
/*this array defines number to digital conversion
*
 *      ###b0##
 *    b5#     #b1
 *      ###b6##
 *    b4#     #b2
 *      ###b3##  #b7#
 */
 #if 0
static struct led_bitmap bitmap_table[] =
{
	/* Let's put the dot bitmap into the table */
	/*为了快速查表,仅仅保留了需要显示的码值*/
	{'0', 0x3f}, {'1', 0x06}, {'2', 0x5b}, {'3', 0x4f},{'8', 0x7f},
	{'4', 0x66}, {'5', 0x6d}, {'6', 0x7d}, {'7', 0x7}, {'9', 0x6f},
	{'a', 0x5f}, {'C', 0x39}, {'n', 0x54}, {'d', 0x5e},{'o', 0x5c}, 
	{'t', 0x78}, {'u', 0x1c}, {':', 0x09}, {' ', 0x80},{'R', 0x77},
	/*河北样机没有的,为loader显示保留,建议毙掉,加快查表速度
	{'U', 0x3e},{'e', 0x7b}, {'i', 0x05}, {'z', 0x5b}, {'r', 0x50},
	{'p', 0x73}, */
	{'s', 0x6d},  {'c', 0x58},{'H', 0x76,},{'h', 0x74},{'-', 0x40},
	{'N', 0x37},{'O', 0x3f},{'D', 0x5e},{' ', 0x00},

	/*added for irdeto loader*/
	{'A', 0x77}, {'b', 0x7c}, {'E', 0x79}, {'F', 0x71}, {'L', 0x38},
	{'P', 0x73}, {'r', 0x50}, {'t', 0x78}, {'U', 0x3e}, {0x10, 0xe6},
	{0x11, 0x10}, {0x12, 0x20}, {0x13, 0x01}, {0x14, 0x02}, {0x15, 0x04},
	{0x16, 0x08},
};
#else 
static struct led_bitmap bitmap_table[] =
{
	{'.', 0x80}, /* Let's put the dot bitmap into the table */
	{'0', 0x3F}, {'1', 0x06}, {'2', 0x5B}, {'3', 0x4F},
	{'4', 0x66}, {'5', 0x6D}, {'6', 0x7D}, {'7', 0x07},
	{'8', 0x7F}, {'9', 0x6F}, {'a', 0x77}, {'A', 0x77},
	{'b', 0x7C}, {'B', 0x7C}, {'c', 0x58}, {'C', 0x39},
	{'d', 0x5E}, {'D', 0x5E}, {'e', 0x79}, {'E', 0x79},
	{'f', 0x71}, {'F', 0x71}, {'g', 0x6F}, {'G', 0x3D},
	{'h', 0x74}, {'H', 0x76}, {'i', 0x04}, {'I', 0x06},
	{'j', 0x1E}, {'J', 0x1E}, {'l', 0x30}, {'L', 0x38},
	{'n', 0x54}, {'N', 0x37}, {'o', 0x5C}, {'O', 0x3F},
	{'p', 0x73}, {'P', 0x73}, {'q', 0x67}, {'Q', 0x67},
	{'r', 0x50}, {'R', 0x77}, {'s', 0x6D}, {'S', 0x6D},
	{'t', 0x78}, {'T', 0x31}, {'u', 0x1C}, {'U', 0x3E},
	{'y', 0x6E}, {'Y', 0x6E}, {'z', 0x5B}, {'Z', 0x5B},
	{':', 0x80/*0xfe*/}, {'-', 0x40}, {'_', 0x08}, {' ', 0x00},
};
#endif

#define PAN_TP6X0_CHAR_LIST_NUM sizeof(bitmap_table)/sizeof(struct led_bitmap)


static struct pan_tp6x0_info
{
	struct pan_hw_info *hw_info;
	int bitmap_len;
	struct led_bitmap  *bitmap_list;
	UINT8 dis_buff[8];
	UINT32 dis_id;
	UINT8 led_buff;
	UINT8 dotmap;
	UINT8 colonmap;
	UINT8 blankmap;
	UINT32 cnt_repeat_first;
	UINT32 cnt_repeat;
	UINT32 key_cnt;			/* Continue press key times */
	UINT32 keypress_cnt;		/* Continue press key counter */
	UINT32 keypress_intv;		/* Continue press key interval */
	UINT32 keypress_num;		/* Pressed key COM index saver */
	UINT32 keypress_bak;		/* Pressed key saver */
	UINT8 cur_dis_mem[7][2];
	UINT8 new_dis_mem[7][2];
} pan_tp6x0_data;

static INT32 pan_tp6x0_open(struct pan_device *dev);
static INT32 pan_tp6x0_close(struct pan_device *dev);
static INT32 pan_tp6x0_ioctl(struct pan_device *dev, INT32 cmd, UINT32 param);
static void  pan_tp6x0_display(struct pan_device *dev, char *data, UINT32 len);
static void  pan_tp6x0_interrupt(UINT32 param);

/* Name for the panel, the last character must be Number for index */
static char  pan_tp6x0_name[HLD_MAX_NAME_SIZE] = "PAN_TP6X0_0";


/************************************************************************/
#define CMD1_PREFIX		0x00

#define MODE_4X13		(0)
#define MODE_5X12		(1)
#define MODE_6X11		(2)
#define MODE_7X10		(3)

/************************************************************************/
#define CMD2_PREFIX		0x40

#define NORMAL_MODE	(0<<3)
#define TEST_MODE		(1<<3)

#define INCRE_ADDR		(0<<2)
#define FIXED_ADDR		(1<<2)

#define WRITE_DATA		(0)
#define READ_KEY		(2)

#define CMD2_READ		(CMD2_PREFIX|NORMAL_MODE|INCRE_ADDR|READ_KEY)

#define CMD2_WRITE_INCRE		(CMD2_PREFIX|NORMAL_MODE|INCRE_ADDR|WRITE_DATA)
#define CMD2_WRITE_FIXED		(CMD2_PREFIX|NORMAL_MODE|FIXED_ADDR|WRITE_DATA)

/************************************************************************/
#define CMD3_PREFIX		0xc0
#define CMD3_ADDR(addr)	(CMD3_PREFIX|(addr&0xf))

/************************************************************************/
#define CMD4_PREFIX		0x80

#define DISPLAY_OFF		(0<<3)
#define DISPLAY_ON		(1<<3)

#define DIM_1_16		0
#define DIM_2_16		1
#define DIM_4_16		2
#define DIM_10_16		3
#define DIM_11_16		4
#define DIM_12_16		5
#define DIM_13_16		6
#define DIM_14_16		7

#define LED_OFF			(CMD4_PREFIX|DISPLAY_OFF|DIM_14_16)
#define LED_ON			(CMD4_PREFIX|DISPLAY_ON|DIM_14_16)


static UINT8 tp6x0_keys_map[2][8][5] = 
{
	{
		{0xff, 0xff, 0xff, 0xff, 0xff},
		{0x10, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x02, 0x00, 0x00, 0x00},
		{0xff, 0xff, 0xff, 0xff, 0xff},
		{0x00, 0x10, 0x00, 0x00, 0x00},
		{0x00, 0x08, 0x00, 0x00, 0x00},
		{0x00, 0x01, 0x00, 0x00, 0x00},
		{0x08, 0x00, 0x00, 0x00, 0x00},
	},
	{
		{0x01, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x08, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x01, 0x00},
		{0xff, 0xff, 0xff, 0xff, 0xff},
		{0x00, 0x00, 0x01, 0x00, 0x00},
		{0x00, 0x08, 0x00, 0x00, 0x00},
		{0x00, 0x01, 0x00, 0x00, 0x00},
		{0x08, 0x00, 0x00, 0x00, 0x00},
	},
};


static UINT8 clk_id;
static UINT8 data_id;
static UINT8 stb_id;

//#define GPIO_OD


#ifdef GPIO_OD

void set_gpio_high(int gpio_pin)
{
	HAL_GPIO_BIT_DIR_SET(gpio_pin, HAL_GPIO_I_DIR);
	HAL_GPIO_BIT_SET(gpio_pin, 1);
}

void set_gpio_low(int gpio_pin)
{
	HAL_GPIO_BIT_SET(gpio_pin, 0);
	HAL_GPIO_BIT_DIR_SET(gpio_pin, HAL_GPIO_O_DIR);
}


void tp6x0_write(UINT8 * byte, UINT32 len)
{
	UINT32 i;
	UINT8 j;
	
	set_gpio_high(stb_id);//STB high
	osal_delay(1);
	set_gpio_high(clk_id);//clk high
	osal_delay(1);
	set_gpio_low(stb_id);//STB low
	osal_delay(1);
	for(j = 0; j<len; j++)
	{
		for(i=0; i<8; i++)
		{
			set_gpio_low(clk_id);//clk low
			if ((byte[j]>>i)&0x01)
				set_gpio_high(data_id);
			else
				set_gpio_low(data_id);
			osal_delay(PW_CLK);
			set_gpio_high(clk_id);//clk high, latch data
			osal_delay(PW_CLK);
		}
		osal_delay(T_WAIT);
	}
	set_gpio_high(stb_id);//STB high
}

void tp6x0_write_read(UINT8 * byte, UINT32 len_in, UINT32 len_out)
{
	UINT32 i;
	UINT8 j;

	set_gpio_high(stb_id);//STB high
	osal_delay(1);
	set_gpio_high(clk_id);//clk high
	osal_delay(1);
	set_gpio_low(stb_id);//STB low
	osal_delay(1);
	for(j = 0; j<len_in; j++)
	{
		for(i=0; i<8; i++)
		{
			set_gpio_low(clk_id);//clk low
			if ((byte[j]>>i)&0x01)
				set_gpio_high(data_id);
			else
				set_gpio_low(data_id);
			osal_delay(PW_CLK);
			set_gpio_high(clk_id);//clk high, latch data
			osal_delay(PW_CLK);
		}
		osal_delay(T_WAIT);
	}
	
	HAL_GPIO_BIT_DIR_SET(data_id, HAL_GPIO_I_DIR);
	for(j = 0; j<len_out; j++)
	{
		byte[j] = 0;
		for(i=0; i<8; i++)
		{
			set_gpio_low(clk_id);//clk low
			osal_delay(PW_CLK);
			byte[j] |= HAL_GPIO_BIT_GET(data_id)<<i;
			set_gpio_high(clk_id);//clk high, latch data
			osal_delay(PW_CLK);
		}
		osal_delay(T_WAIT);
	}
	set_gpio_high(stb_id);//STB high
}

#else
void tp6x0_write(UINT8 * byte, UINT32 len)
{
	UINT32 i;
	UINT8 j;
	
	HAL_GPIO_BIT_SET(clk_id, 1);//clk high
	
	HAL_GPIO_BIT_DIR_SET(clk_id, HAL_GPIO_O_DIR);
	HAL_GPIO_BIT_DIR_SET(data_id, HAL_GPIO_O_DIR);
	HAL_GPIO_BIT_DIR_SET(stb_id, HAL_GPIO_O_DIR);
	HAL_GPIO_BIT_SET(stb_id, 1);//STB high
	osal_delay(1);
	HAL_GPIO_BIT_SET(clk_id, 1);//clk high
	osal_delay(1);
	HAL_GPIO_BIT_SET(stb_id, 0);//STB low
	osal_delay(1);
	for(j = 0; j<len; j++)
	{
		for(i=0; i<8; i++)
		{
			HAL_GPIO_BIT_SET(clk_id, 0);//clk low
			HAL_GPIO_BIT_SET(data_id, (byte[j]>>i)&0x01);
			osal_delay(PW_CLK);
			HAL_GPIO_BIT_SET(clk_id, 1);//clk high, latch data
			osal_delay(PW_CLK);
		}
		osal_delay(T_WAIT);
	}
	HAL_GPIO_BIT_SET(stb_id, 1);//STB high
}

void tp6x0_write_read(UINT8 * byte, UINT32 len_in, UINT32 len_out)
{
	UINT32 i;
	UINT8 j;

	HAL_GPIO_BIT_SET(clk_id, 1);//clk high
	
	HAL_GPIO_BIT_DIR_SET(clk_id, HAL_GPIO_O_DIR);
	HAL_GPIO_BIT_DIR_SET(data_id, HAL_GPIO_O_DIR);
	HAL_GPIO_BIT_DIR_SET(stb_id, HAL_GPIO_O_DIR);
	
	HAL_GPIO_BIT_SET(stb_id, 1);//STB high
	osal_delay(1);
	HAL_GPIO_BIT_SET(clk_id, 1);//clk high
	osal_delay(1);
	HAL_GPIO_BIT_SET(stb_id, 0);//STB low
	osal_delay(1);
	for(j = 0; j<len_in; j++)
	{
		for(i=0; i<8; i++)
		{
			HAL_GPIO_BIT_SET(clk_id, 0);//clk low
			HAL_GPIO_BIT_SET(data_id, (byte[j]>>i)&0x01);
			osal_delay(PW_CLK);
			HAL_GPIO_BIT_SET(clk_id, 1);//clk high, latch data
			osal_delay(PW_CLK);
		}
		osal_delay(T_WAIT);
	}
	HAL_GPIO_BIT_DIR_SET(data_id, HAL_GPIO_I_DIR);
	for(j = 0; j<len_out; j++)
	{
		byte[j] = 0;
		for(i=0; i<8; i++)
		{
			HAL_GPIO_BIT_SET(clk_id, 0);//clk low
			osal_delay(PW_CLK);
			byte[j] |= HAL_GPIO_BIT_GET(data_id)<<i;
			HAL_GPIO_BIT_SET(clk_id, 1);//clk high, latch data
			osal_delay(PW_CLK);
		}
		osal_delay(T_WAIT);
	}
	HAL_GPIO_BIT_SET(stb_id, 1);//STB high
}

#endif


void pan_tp6x0_set_mode(UINT8 mode)
{
	UINT8 cmd;
	
	cmd = CMD1_PREFIX|mode;
	osal_interrupt_disable();
	tp6x0_write(&cmd, 1);
	osal_interrupt_enable();	
		
}

void pan_tp6x0_led_onoff(UINT8 on_off) // 1: on, 0: off
{
	UINT8 cmd;
	if(on_off)
		cmd = LED_ON;
	else
		cmd = LED_OFF;
	osal_interrupt_disable();
	tp6x0_write(&cmd, 1);
	osal_interrupt_enable();
}

 void pan_tp6x0_update_led(UINT8 addr, UINT8 * byte, UINT8 len)
 {
 
 	UINT8 cmd;	
 	UINT8 cmd_seq[15]; 
	UINT32 i;

	if(len<1||len>14)
		return;
	if(len==1)
		cmd = CMD2_WRITE_FIXED;
	else 
		cmd = CMD2_WRITE_INCRE;
	cmd_seq[0] = CMD3_ADDR(addr);
	MEMCPY(&cmd_seq[1], byte, len);	
	osal_interrupt_disable();	
	tp6x0_write(&cmd, 1);
	tp6x0_write(cmd_seq, 1+len);
	osal_interrupt_enable();
 }

void pan_tp6x0_led_init(struct pan_device *dev)
 {
 	struct pan_tp6x0_info *tp = (struct pan_tp6x0_info *)dev->priv;
 	UINT8 i; 
	for(i = 0; i<7; i++)
	{
		tp->new_dis_mem[i][0] |= tp->blankmap;
		tp->new_dis_mem[i][1] |= tp->blankmap;
		tp->cur_dis_mem[i][0] |= tp->blankmap;
		tp->cur_dis_mem[i][1] |= tp->blankmap;
	}
#ifndef FD620_LED5_COL
	pan_tp6x0_set_mode(MODE_4X13);
#else
	pan_tp6x0_set_mode(MODE_5X12);
#endif
	pan_tp6x0_update_led(0x00, &(tp->cur_dis_mem[0][0]), 14);
	pan_tp6x0_led_onoff(1);
 }

 void pan_tp6x0_read_key(UINT8 * byte, UINT8 len)
 {
 	UINT8 cmd_seq[5];

	if(len>5)
		return;
	cmd_seq[0] = CMD2_READ;
	osal_interrupt_disable();
	tp6x0_write_read(cmd_seq, 1, 5);
	osal_interrupt_enable();
	MEMCPY(byte, cmd_seq, len);
 }
/*
 * 	Name		:   pan_tp6x0_attach()
 *	Description	:   Panel init funciton should be called in
 *                  system boot up.
 *	Parameter	:	None
 *	Return		:	INT32				: return value
 *
 */
__ATTRIBUTE_REUSE_
INT32 pan_tp6x0_attach(struct pan_configuration *config)
{
	struct pan_device *dev;
	struct pan_tp6x0_info *tp;
	int i, j;

	if (config == NULL || config->hw_info == NULL)
	{
		return ERR_FAILUE;
	}
	dev = dev_alloc(pan_tp6x0_name, HLD_DEV_TYPE_PAN, sizeof(struct pan_device));
	if (dev == NULL)
	{
		PRINTF("Error: Alloc front panel device error!\n");
		return ERR_NO_MEM;
	}
	/* Alloc structure space of private */
	MEMSET(&pan_tp6x0_data, 0, sizeof(struct pan_tp6x0_info));

	if (config->bitmap_list == NULL)
	{
		pan_tp6x0_data.bitmap_len = PAN_TP6X0_CHAR_LIST_NUM;
		pan_tp6x0_data.bitmap_list = &(bitmap_table[0]);
	}
	else
	{
		pan_tp6x0_data.bitmap_len = config->bitmap_len;
		pan_tp6x0_data.bitmap_list = config->bitmap_list;
	}
	pan_tp6x0_data.hw_info = config->hw_info;

	dev->priv = &pan_tp6x0_data;

	dev->led_num = pan_tp6x0_data.hw_info->num_com;
	dev->key_num = 0;

	/* Function point init */
	dev->init = pan_tp6x0_attach;
	dev->open = pan_tp6x0_open;
	dev->stop = pan_tp6x0_close;
	dev->do_ioctl = pan_tp6x0_ioctl;
	dev->display =pan_tp6x0_display;
	dev->send_data = NULL;
	dev->receive_data = NULL;

	/* Add this device to queue */
	if (dev_register(dev) != SUCCESS)
	{
		PRINTF("Error: Register panel device error!\n");
		dev_free(dev);
		return ERR_NO_DEV;
	}

	tp = (struct pan_tp6x0_info *)dev->priv;
	for(i=0; i<4; i++)
	{
		HAL_GPIO_BIT_DIR_SET(tp->hw_info->lbd[i].position, tp->hw_info->lbd[i].io);
		HAL_GPIO_BIT_SET(tp->hw_info->lbd[i].position, !tp->hw_info->lbd[i].polar);
	}
	/* Set default bitmap in buffer. At same time get the dot bitmap. */
	for (i = 0; i < tp->bitmap_len; i++)
	{
		if (' ' == tp->bitmap_list[i].character)
		{
			tp->blankmap = tp->bitmap_list[i].bitmap;
			for (j = 0; j < tp->hw_info->num_com; j++)
				tp->dis_buff[j] = tp->blankmap;
		}
		else if ('.' == tp->bitmap_list[i].character)
		{
			tp->dotmap = tp->bitmap_list[i].bitmap;
		}
		else if (':' == tp->bitmap_list[i].character)
		{
			tp->colonmap = tp->bitmap_list[i].bitmap;
		}
	}
	/* If is shadow scan, all key scan per 1mS interrupt, else... */
	if (tp->hw_info->type_scan == 1)
	{
		tp->cnt_repeat_first = tp->hw_info->intv_repeat_first;
		tp->cnt_repeat = tp->hw_info->intv_repeat;
	} else
	{
		tp->cnt_repeat_first = tp->hw_info->intv_repeat_first / tp->hw_info->num_com;
		tp->cnt_repeat = tp->hw_info->intv_repeat / tp->hw_info->num_com;
	}

	stb_id = tp->hw_info->flatch.position;
	clk_id = tp->hw_info->fclock.position;
	data_id = tp->hw_info->fdata.position;
	return SUCCESS;
}

static INT32 pan_tp6x0_ioctl(struct pan_device *dev, INT32 cmd,
								UINT32 param)
{
	switch (cmd)
	{
		case PAN_DRIVER_ATTACH:
			break;
		case PAN_DRIVER_SUSPEND:
			break;
		case PAN_DRIVER_RESUME:
			break;
		case PAN_DRIVER_DETACH:
			break;
		case PAN_DRIVER_READ_LNB_POWER:
			break;
		default:
			break;
	}

	return SUCCESS;
}

/*
* Name          :   pan_tp6x0_open()
* Description   :   Open front panel
* Parameter     :   struct pan_device *dev
* Return        :   void
*/
static INT32  pan_tp6x0_open(struct pan_device *dev)
{
	struct pan_tp6x0_info *tp = (struct pan_tp6x0_info *)dev->priv;
	struct pan_hw_info *hp = tp->hw_info;
	UINT8 led_data = 0;
	int i;

	tp->keypress_intv = tp->cnt_repeat_first/10;
	tp->keypress_bak = PAN_KEY_INVALID;
	tp->keypress_num = tp->hw_info->num_com;
	tp->keypress_cnt = 0;
	tp->key_cnt = 0;

	/* Init LBD to invalid state */
	if (hp->type_kb & 0x02)
	{
		for (i = 0; i < 4; i++)
		{
			if (hp->lbd[i].polar == 0)	/* Set to invalidate status */
				led_data |= (1 << hp->lbd[i].position);
		}
		led_data &= 0xff;
	}
	pan_tp6x0_data.led_buff = led_data;
	pan_tp6x0_led_init(dev);
#ifdef _DEBUG_VERSION_
	if (tp->hw_info->type_irp != 0)
		irc_m6303irc_init(tp->hw_info);
#endif
	/* Register an 1mS cycle interrupt ISR */
	osal_interrupt_register_lsr(7, pan_tp6x0_interrupt, (UINT32)dev);

	return SUCCESS;
}


/*
* Name          :   pan_tp6x0_close()
* Description   :   close front panel
* Parameter     :   struct pan_device *dev
* Return        :   void
*/
static INT32  pan_tp6x0_close(struct pan_device *dev)
{
	struct pan_tp6x0_info *tp = (struct pan_tp6x0_info *)dev->priv;
	int i;

#ifdef _DEBUG_VERSION_
	if (tp->hw_info->type_irp != 0)
		irc_m6303irc_close();
#endif

	/* Un-register an 1mS cycle interrupt ISR */
	osal_interrupt_unregister_lsr(7, pan_tp6x0_interrupt);
	
	/* Clear last displayed char map. */
	for(i = 0; i<7; i++)
	{
		tp->cur_dis_mem[i][0] = tp->cur_dis_mem[i][1] = 0;
		tp->cur_dis_mem[i][0] |= tp->blankmap;
		tp->cur_dis_mem[i][1] |= tp->blankmap;		
	}
	pan_tp6x0_update_led(0x00, &(tp->cur_dis_mem[0][0]), 14);
	pan_tp6x0_led_onoff(0);
	return SUCCESS;
}


/*
* Name          :   pan_tp6x0_interrupt()
* Description   :   front panel interrupt function
* Parameter     :   None
* Return        :   void
*/
static void  pan_tp6x0_interrupt(UINT32 param)
{
	struct pan_device *dev = (struct pan_device *)param;
	struct pan_tp6x0_info *tp = (struct pan_tp6x0_info *)dev->priv;
	UINT32 keypress = PAN_KEY_INVALID;
	UINT8 key_6x0[5] = {0,0,0,0,0};
	UINT8 *key_map;
	struct pan_key key;
	static UINT8 cnt = 0;
	static UINT8 tmpcnt = 0;
	UINT32 i;

	if (tmpcnt++<10)
		return;
	tmpcnt = 0;
#ifndef FD620_LED5_COL
	pan_tp6x0_set_mode(MODE_4X13);
#else
	pan_tp6x0_set_mode(MODE_5X12);
#endif
	pan_tp6x0_update_led(0, &tp->cur_dis_mem[0][0], 14);
	pan_tp6x0_led_onoff(1);
	pan_tp6x0_read_key((UINT8 *)(&key_6x0), 5);

	// tp620 panel
	if (tp->hw_info->num_scan == 1)
	{
		key_map = &tp6x0_keys_map[1][0][0];
	}
	else //tp610
	{
		key_map = &tp6x0_keys_map[0][0][0];
	}

	for (i = 0; i < 8; i++)
	{
		if (0 == MEMCMP(key_6x0, key_map + i * 5, 5))
			break;
	}

	if (i == 8)
		keypress = PAN_KEY_INVALID;
	else
		keypress = 0xffff0000 | (1 << i);
	
	if (tp->hw_info->hook_scan)
		keypress = tp->hw_info->hook_scan(dev, keypress);

	/* Some key input */
	if (keypress != PAN_KEY_INVALID)
	{
		/* If is the same key, count it */
		if (tp->keypress_bak == keypress)
		{
			tp->keypress_cnt++;
			if (tp->keypress_cnt == 2)
			{
	    			key.type = PAN_KEY_TYPE_PANEL;
	    			key.state = PAN_KEY_PRESSED;
				key.count = tp->key_cnt;
				key.code = tp->keypress_bak;
				pan_buff_queue_tail(&key);
				tp->key_cnt++;
			}
			else if (tp->keypress_cnt == tp->keypress_intv)
			{
				tp->keypress_intv = tp->cnt_repeat/10;
				tp->keypress_cnt = 0;	/* Support continue press key */
			}
		}
		/* Else is a new key, backup it */
		else
		{
			if (tp->hw_info->intv_release & 1)	/* If intv_release is odd, pan key repeat enable */
			{
				if (tp->keypress_bak != PAN_KEY_INVALID)
				{
			   		key.type = PAN_KEY_TYPE_PANEL;
			   		key.state = PAN_KEY_RELEASE;
					key.count = 0;
					key.code = tp->keypress_bak;
					pan_buff_queue_tail(&key);
				}
			}
			tp->keypress_intv = tp->cnt_repeat_first/10;
			tp->keypress_bak = keypress;
			tp->keypress_num = tp->dis_id;
			tp->keypress_cnt = 1;
			tp->key_cnt = 0;
		}
	}
	/* No key input, if the same key switch to invalid, reset it */
	else if (/*tp->keypress_num == tp->dis_id &&*/ tp->keypress_bak != PAN_KEY_INVALID)
	{
		if (cnt++ > 5)
		{
			if (tp->hw_info->intv_release & 1)	/* If intv_release is odd, pan key repeat enable */
			{
		   		key.type = PAN_KEY_TYPE_PANEL;
		   		key.state = PAN_KEY_RELEASE;
				key.count = 0;
				key.code = tp->keypress_bak;
				pan_buff_queue_tail(&key);
			}
			tp->keypress_intv = tp->cnt_repeat_first/10;
			tp->keypress_bak = PAN_KEY_INVALID;
			tp->keypress_num = tp->hw_info->num_com;
			tp->keypress_cnt = 0;
			tp->key_cnt = 0;
			cnt = 0;
		}
	}
}

/*
* Name          :   pan_tp6x0_esc_command()
* Description   :   Do ESC command
* Parameter     :   UINT8* data			: Command data
*                   UINT32 limit_len	: Length limit
* Return        :   UINT32				: command length
*/
static UINT32 pan_tp6x0_esc_command(struct pan_device *dev, UINT8 *data, UINT32 limit_len)
{
	struct pan_tp6x0_info *tp = (struct pan_tp6x0_info *)dev->priv;
	struct pan_gpio_info *gp;
	UINT32 dp;

	/* Search ESC command untill no-ESC or reached data limit */
	for (dp = 0; dp < limit_len && data[dp] == 27; dp += 4)
	{	/* LBD operate command */
		if (PAN_ESC_CMD_LBD == data[dp + 1] || 'l' == data[dp + 1])
		{
#ifndef ESC_CMD_TP6X0
			gp = &tp->hw_info->lbd[data[dp + 2]];
			if (tp->hw_info->type_kb & 0x02)	/* LED PIN is controlled by shifter */
			{
				tp->led_buff = ((tp->led_buff & ~(1 << gp->position)) |
					((~(gp->polar ^ data[dp + 3]) & 1) << gp->position));
			} else
			{
				HAL_GPIO_BIT_SET(gp->position, data[dp + 3]==0?(!gp->polar):gp->polar);
				//pan_common_gpio_set_bit(*(UINT8 *)(gp), data[dp + 3]);
			}
#else
			int i;
			switch(data[dp+2])
			{
				case PAN_ESC_CMD_LBD_POWER	:
					
					break;
					
				case PAN_ESC_CMD_LBD_LOCK		:
					if (data[dp+3] == PAN_ESC_CMD_LBD_ON)
					{
                      
						tp->new_dis_mem[4][0] &= 0x0f;
						tp->new_dis_mem[4][0] |= 0x10;
                        tp->new_dis_mem[4][1] = tp->new_dis_mem[4][0];
						tp->cur_dis_mem[4][0] = tp->new_dis_mem[4][0];
                        tp->cur_dis_mem[4][1] = tp->new_dis_mem[4][0];
					}
					else if (data[dp+3] == PAN_ESC_CMD_LBD_OFF)
					{
						tp->new_dis_mem[4][0] &= 0x0f;
						tp->new_dis_mem[4][0] |= 0x20;
                        tp->new_dis_mem[4][1] = tp->new_dis_mem[4][0];
						tp->cur_dis_mem[4][0] = tp->new_dis_mem[4][0];
                        tp->cur_dis_mem[4][1] = tp->new_dis_mem[4][0];
					}
					break;
					
   				case PAN_ESC_CMD_LBD_FUNCA	://standby
					/*if (data[dp+3] == PAN_ESC_CMD_LBD_ON)
					{
						tp->cur_dis_mem[1][1] |= 0x08;
						tp->new_dis_mem[1][1] |= 0x08;
						
					}
					else if (data[dp+3] == PAN_ESC_CMD_LBD_OFF)
					{
						tp->cur_dis_mem[1][1] &= ~0x08;
						tp->new_dis_mem[1][1] &= ~0x08;
					}*/				
					break;
					
				case PAN_ESC_CMD_LBD_FUNCB	:
					break;
					
				case PAN_ESC_CMD_LBD_LEVEL		:
					break;
					
				default :
					break;
			}
			pan_tp6x0_update_led(0, &tp->cur_dis_mem[0][0], 14);
#endif
		} else if (PAN_ESC_CMD_LED == data[dp + 1] || 'e' == data[dp + 1])
		{
			tp->dis_buff[data[dp + 2]] = data[dp + 3];
		}
	}

	return dp;
}


/*
* Name          :   pan_tp6x0_char_map()
* Description   :   Do string display
* Parameter     :   UINT8* data			: Command data
*                   UINT32 limit_len	: Length limit
* Return        :   UINT32				: string length
*/
static UINT32 pan_tp6x0_char_map(struct pan_device *dev,
											 UINT8 *data, UINT32 len)
{
	struct pan_tp6x0_info *tp = (struct pan_tp6x0_info *)dev->priv;
	UINT32 pdata, pbuff;
	int i, j;
	UINT8 dot_flag = 0;
	UINT8 col_flag = 0;

	pdata = 0;
	pbuff = 0;
	tp->dis_buff[tp->hw_info->pos_colon] = tp->blankmap;/* Clear colon flag */
	/* Search all string sector in data untill ESC or pbuf full */
	while (pdata < len && data[pdata] != 27 && pbuff < dev->led_num)
	{
		if (data[pdata] == ':')		/* Process colon charactor */
		{
			col_flag = 1;
			pdata++;
			continue;
		}
		if (data[pdata] == '.')		/* Process dot charactor */
		{
			tp->dis_buff[pbuff] = tp->dotmap;
			pdata++;
			pbuff++;
			continue;
		}
		/* Generate the bitmap */
		tp->dis_buff[pbuff] = tp->blankmap;	/* Pre-set buffer to NULL */
		for (j = 0; j < tp->bitmap_len; j++)
		{
			if(data[pdata] == tp->bitmap_list[j].character)
			{
				tp->dis_buff[pbuff] = tp->bitmap_list[j].bitmap;
#if 0
				if (data[pdata + 1] == '.' && (pdata + 1) < len)
				{

					UINT32 k, l = 0;
					for(k=0; k<8; k++)
						if(tp->dotmap&(1<<k))
							l++;
					if(l==7)
						tp->dis_buff[pbuff] &= tp->dotmap;
					else
						tp->dis_buff[pbuff] |= tp->dotmap;
				}
#endif
				break;

			}
		}
		/* Move to next position */
		pbuff++;
		if (data[pdata + 1] == '.' && (pdata + 1) < len)
		{
			pdata += 2;
		} else
		{
			pdata += 1;
		}
	}
	//if (dot_flag == 1)
		//tp->dis_buff[tp->hw_info->pos_colon] &= tp->colonmap;


	//generate new display matrix
#ifndef FD620_LED5_COL
	if(tp->hw_info->num_com > 4)
		tp->hw_info->num_com = 4;
#else
	if(tp->hw_info->num_com > 5)
		tp->hw_info->num_com = 4;
#endif
	for(i=0; i<4;i++)
	{
		tp->new_dis_mem[i][0] = tp->dis_buff[i] & 0x7f;
	}
#ifndef FD620_LED5_COL
	if (col_flag)
	{
		tp->new_dis_mem[1][0] |= 0x80;
		tp->new_dis_mem[1][1] |= 0x20;
	}
#else
	if (col_flag)
	{
		tp->new_dis_mem[4][0] |= 0x03;
		tp->new_dis_mem[4][1] |= 0x03;
	}
	else
	{
		tp->new_dis_mem[4][0] &= 0xFC;
		tp->new_dis_mem[4][1] &= 0xFC;
	}
#endif
	//update current display matrix
	for(i=0; i<tp->hw_info->num_com;i++)
	{
		if(tp->new_dis_mem[i][0] != tp->cur_dis_mem[i][0])
		{
			tp->cur_dis_mem[i][0] = tp->new_dis_mem[i][0] ;
			pan_tp6x0_update_led(i*2, &tp->cur_dis_mem[i][0], 1);
		}
		if(tp->new_dis_mem[i][1] != tp->cur_dis_mem[i][1])
		{
			tp->cur_dis_mem[i][1] = tp->new_dis_mem[i][1] ;
			pan_tp6x0_update_led(i*2+1, &tp->cur_dis_mem[i][1], 1);
		}
	}
	/* Display buffer full, stop display process */
	if (data[pdata] != 27 && pbuff == dev->led_num)
	{
		return len;
	}
	return pdata;
}

/*
* Name          :   pan_tp6x0_display()
* Description   :   Set display data
* Parameter     :   char* disp
* Return        :   void
*/
static void pan_tp6x0_display(struct pan_device *dev, char *data, UINT32 len)
{
	struct pan_tp6x0_info *tp = (struct pan_tp6x0_info *)dev->priv;
	UINT32 pdata = 0;

	if (tp->hw_info->hook_show)
		len = tp->hw_info->hook_show(dev, data, len);

	while (pdata < len)
	{
		if (data[pdata] == 27)			/* ESC command */
		{
			pdata += pan_tp6x0_esc_command(dev, &(data[pdata]), len - pdata);
		} else							/* String display */
		{
			pdata += pan_tp6x0_char_map(dev, &(data[pdata]), len - pdata);
		}
	}
}


