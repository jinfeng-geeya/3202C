/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2005 Copyright (C)
 *
 *  File: smc.h
 *
 *  Description: This file contains all functions definition
 *		             of smart card reader interface driver.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  0.                 Victor Chen            Ref. code
 *  1. 2005.9.8  Gushun Chen     0.1.000    Initial
 *  2. 2006.12.5  Victor Chen    0.2.000    Update interface for s3602
 ****************************************************************************/

#ifndef __HLD_SMC_H__
#define __HLD_SMC_H__

#include <types.h>
#include <hld/hld_dev.h>

#define SMC_IO_ON		1
#define SMC_IO_OFF		0

#define SMC_STATUS_OK			0
#define SMC_STATUS_NOT_EXIST	1
#define SMC_STATUS_NOT_RESET	2

#define BOARD_SUPPORT_CLASS_A		(1<<0)
#define BOARD_SUPPORT_CLASS_B		(1<<1)
#define BOARD_SUPPORT_CLASS_C		(1<<2)

enum class_selection
{
	SMC_CLASS_NONE_SELECT = 0,
	SMC_CLASS_A_SELECT,
	SMC_CLASS_B_SELECT,
	SMC_CLASS_C_SELECT	
};

enum smc_atr_result
{
	SMC_ATR_NONE = 0,
	SMC_ATR_WRONG,
	SMC_ATR_OK
};

enum smc_device_ioctrl_command
{
	SMC_DRIVER_SET_IO_ONOFF = 0,	/*en/dis smart card io*/	
	SMC_DRIVER_SET_ETU,			/*set working etu*/
	SMC_DRIVER_SET_WWT,			/*set block waiting time, in unit of ms*/			
	SMC_DRIVER_SET_GUARDTIME,	/*to do*/
	SMC_DRIVER_SET_BAUDRATE,		/*to do*/
	SMC_DRIVER_CHECK_STATUS,		
	SMC_DRIVER_CLKCHG_SPECIAL,	/*to do*/
	SMC_DRIVER_FORCE_SETTING,	/*to do*/
	SMC_DRIVER_SET_CWT,			/*set character waiting time, in unit of ms*/
	SMC_DRIVER_GET_F,				/*get F factor value*/
	SMC_DRIVER_GET_D,				/*get D factor value*/
	SMC_DRIVER_GET_ATR_RESULT, 	/*check ATR status*/
	SMC_DRIVER_GET_HB	,			/*get History Bytes*/
	SMC_DRIVER_GET_PROTOCOL,		/*get card current protocol*/
	SMC_DRIVER_SET_WCLK,			/*set the working clock of smc, */
									/*the new setting value will be used from */
									/*next time reset*/
	SMC_DRIVER_GET_CLASS,			/*return the currently selected classs*/
	SMC_DRIVER_SET_CLASS			/*setting new class selection if previous select fail*/
};

/*sub command set for SMC_DRIVER_FORCE_SETTING */
#define SMC_FORCE_BASE		0x1
#define SMC_FORCE_TX_RX	(SMC_FORCE_BASE + 1)
/***********************************************/
struct smc_hb_t		//Struct to retrieve Historical Byte from ATR
{
	UINT8 hb[33];
	UINT8 hbn;
};

struct smc_gpio_info		/* Total 1 byte */
{
	UINT8	polar;	/* Polarity of GPIO, 0 or 1  is enable or for smart card */
	UINT8	io;	/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT8	position;	/* GPIO index, upto 64 GPIO */
};

struct smc_hw_info
{
	UINT8 io_type;		/*0 uart io; 1 iso_7816 io. Only can be 0 now*/
	UINT8 uart_id;		/*SCI_FOR_RS232 0, SCI_FOR_MDM 1*/
	UINT8 gpio_control;	/*1 gpio control*/
	UINT8 ext_clock;	/*0 internal clock; 1 external clock*/
	UINT8 shared_uart;	/*0 ; 1 shared with rs232 with uart_switch gpio*/
	UINT8 uart_with_appinit : 1;
	UINT8 reserved : 7;
	
	struct smc_gpio_info prest;
	struct smc_gpio_info power;
	struct smc_gpio_info reset;
	struct smc_gpio_info uart_switch;
	//add for external 6MHz clock
	struct smc_gpio_info clock_switch;
	UINT32 clock;			/*external clock*/
	//add for external 6MHz clock
	UINT32 clock_ext;		/*external clock 2*/
	UINT32 to_for_atr;		/*timeout for atr in ms*/
	UINT32 to_for_cmd;	/*timeout for command in ms*/
};


struct smc_dev_config			//Struct for smc driver configure
{
	UINT32	init_clk_trigger : 1;	/*0, use default initial clk 3.579545MHz. 
								1, use configed initial clk.*/
	UINT32	def_etu_trigger : 1;	/*0, use HW detected ETU as initial ETU. 
								1, use configed ETU as initial ETU.*/
	UINT32	sys_clk_trigger : 1; 	/*Currently, useless*/
	UINT32	gpio_cd_trigger : 1;	/*Current down detecting, , while power off, a gpio int will notify
								CPU to do deactivation*/
	UINT32	gpio_cs_trigger : 1;	/*Currently, useless*/
	UINT32    force_tx_rx_trigger: 1; /*Support TX/RX timely switch*/
	UINT32	parity_disable_trigger: 1; /*0, disable parity check while get ATR. 
									1, enable parity check while get ATR.*/
	UINT32	parity_odd_trigger: 1; /*0, use even parity while get ATR. 
									1, use odd parity while get ATR.*/
	UINT32	apd_disable_trigger: 1;/*0, enable auto pull down function while get ATR. 
									1, disable auto pull down while get ATR.*/
	UINT32	type_chk_trigger : 1;		/*0, don't care card type check, 1, check card type
									is A, B or AB type according to interface device setting*/	
	UINT32 	warm_reset_trigger: 1;	/*0, all the reset are cold reset. 1, all the reset are warm 
									reset except the first one.*/	
	UINT32	gpio_vpp_trigger : 1;		/*Use a gpio pin to provide Vpp signal*/
	UINT32 	disable_pps: 1;		
	UINT32	invert_power: 1;
	UINT32	invert_detect: 1;
	UINT32	class_selection_supported: 1;	 /*indicate current board support more than one class*/
	UINT32 	board_supported_class: 6;	/*indicate classes supported by current board*/
	UINT32	en_power_open_drain:1;		/*1: enable power pin open drain*/
	UINT32	en_clk_open_drain:1;		/*1: enable clk pin open drain*/
	UINT32	en_data_open_drain:1;		/*1: enable data pin open drain*/
	UINT32	en_rst_open_drain:1;		/*1: enable reset pin open drain*/
	UINT32	reserved : 6;									
	UINT32	init_clk_number;
	UINT32 *	init_clk_array;
	UINT32	default_etu;
	UINT32	smc_sys_clk;
	UINT16 	gpio_cd_pol:1;		/* Polarity of GPIO, 0 or 1 active */
	UINT16 	gpio_cd_io:1;		/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT16 	gpio_cd_pos: 14;	
	UINT16 	gpio_cs_pol:1;		/* Polarity of GPIO, 0 or 1 active */
	UINT16 	gpio_cs_io:1;		/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT16 	gpio_cs_pos: 14;	
	UINT8     force_tx_rx_cmd;
	UINT8     force_tx_rx_cmd_len;
	UINT8    	intf_dev_type;
	UINT8    	reserved1;
	UINT16 	gpio_vpp_pol:1;		/* Polarity of GPIO, 0 or 1 active */
	UINT16 	gpio_vpp_io:1;		/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT16 	gpio_vpp_pos: 14;	
	UINT32	ext_cfg_tag;
	void * 	ext_cfg_pointer;
	void 	(*class_select)(enum class_selection );	/*call back function for class selection operation*/
};

/*
 *  Structure smc_device, the basic structure between HLD and LLD smart card reader device.
 */

struct smc_device
{
	struct hld_device *next;		/* Next device structure */
	UINT32 type;					/* Interface hardware type */
	INT8 name[HLD_MAX_NAME_SIZE];	/* Device name */

	UINT16 flags;					/* Interface flags, status and ability */
	UINT32 base_addr;			/* Device IO base addr */

	UINT8  irq;					/*interrupt number using for s3602*/

	/* Hardware privative structure */
	void *priv;					/* pointer to private data */

/*
 *  Functions of this smart card reader
 */
	INT32	(*open)(struct smc_device *dev, void (*callback)(UINT32 param));

	INT32	(*close)(struct smc_device *dev);

	INT32	(*card_exist)(struct smc_device *dev);

	INT32	(*reset)(struct smc_device *dev, UINT8 *buffer, UINT16 *atr_size);

	INT32	(*deactive)(struct smc_device *dev);

	INT32	(*raw_read)(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen);

	INT32	(*raw_write)(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen);
	INT32	(*raw_fifo_write)(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen);

	INT32 	(*iso_transfer)(struct smc_device *dev, UINT8 *command, INT16 num_to_write, UINT8 *response, INT16 num_to_read, INT16 *actual_size);
	INT32	(*t1_transfer)(struct smc_device*dev, UINT8 dad, const void *snd_buf, UINT32 snd_len, void *rcv_buf, UINT32 rcv_len);
	INT32 	(*t1_xcv)(struct smc_device *dev, UINT8*sblock, UINT32 slen, UINT8 *rblock, UINT32 rmax, UINT32 *ractual);
	INT32 	(*t1_negociate_ifsd)(struct smc_device*dev, UINT32 dad, INT32 ifsd);

    	INT32 	(*iso_transfer_t1)(struct smc_device *dev, UINT8 *command, INT16 num_to_write, 
						UINT8 *response, INT16 num_to_read,INT32 *actual_size);

	INT32	(*do_ioctl)(struct smc_device *dev, INT32 cmd, UINT32 param);

	void	(*callback)(UINT32 param);
};

#ifdef __cplusplus
extern "C"
{
#endif

INT32 smc_open(struct smc_device *dev, void (*callback)(UINT32 param));
INT32 smc_close(struct smc_device *dev);
INT32 smc_card_exist(struct smc_device *dev);
INT32 smc_reset(struct smc_device *dev, UINT8 *buffer, UINT16 *atr_size);
INT32 smc_deactive(struct smc_device *dev);
INT32 smc_raw_read(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen);
INT32 smc_raw_write(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen);
INT32 smc_raw_fifo_write(struct smc_device *dev, UINT8 *buffer, INT16 size, INT16 *actlen);
INT32 smc_iso_transfer(struct smc_device *dev, UINT8 *command, INT16 num_to_write, UINT8 *response, INT16 num_to_read, INT16 *actual_size);
INT32 smc_iso_transfer_t1(struct smc_device *dev, UINT8 *command, INT16 num_to_write, UINT8 *response, INT16 num_to_read,INT32 *actual_size);
INT32 smc_iso_transfer_t14(struct smc_device *dev, UINT8 *command, INT16 num_to_write, UINT8 *response, INT16 num_to_read,INT32 *actual_size);
INT32 smc_io_control(struct smc_device *dev, INT32 cmd, UINT32 param);
INT32 smc_t1_transfer(struct smc_device*dev, UINT8 dad, const void *snd_buf, UINT32 snd_len, void *rcv_buf, UINT32 rcv_len);

#ifdef __cplusplus
}
#endif

#endif /* __HLD_SMC_H__ */

