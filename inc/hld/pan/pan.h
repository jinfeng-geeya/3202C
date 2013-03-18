/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    pan.h
*
*    Description:    This file contains all functions definition
*		             of Front panel driver.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Apr.21.2003      Justin Wu       Ver 0.1    Create file.
* 	2.  Dec.19.2003		 Justin Wu		 Ver 0.2    Add ESC CMD macros.
*	3.  Sep.23.2005		 Justin Wu		 Ver 0.3    Add pan information.
*****************************************************************************/

#ifndef __HLD_PAN_H__
#define __HLD_PAN_H__

#include <sys_config.h>

#define PAN_KEY_INVALID			0xFFFFFFFF

/* Key value of Front keypad. */
#define PAN_KEY_01				0xFFFF0001
#define	PAN_KEY_02				0xFFFF0002
#define PAN_KEY_03				0xFFFF0004
#define PAN_KEY_04				0xFFFF0008
#define PAN_KEY_05				0xFFFF0010
#define PAN_KEY_06				0xFFFF0020
#define PAN_KEY_07				0xFFFF0040
#define PAN_KEY_08				0xFFFF0080

//multi-panel key pressed
#define PAN_COMPOUND_KEY		0x1A2B3C4D

/* ESC command: 27 (ESC code), PAN_ESC_CMD_xx (CMD type), param1, param2 */
#define	PAN_ESC_CMD_LBD			'L'		/* LBD operate command */
#define PAN_ESC_CMD_LBD_POWER	0		/* Power status LBD */
#define PAN_ESC_CMD_LBD_LOCK	1		/* Lock status LBD */
#define PAN_ESC_CMD_LBD_FUNCA	2		/* Extend function LBD */
#define PAN_ESC_CMD_LBD_FUNCB	3		/* Extend function LBD */
#define PAN_ESC_CMD_LBD_LEVEL	5		/* Level status LBD, no used */
#define PAN_ESC_CMD_LBD_MAIL    6
#define PAN_ESC_CMD_LBD_WIFI    7

#define PAN_ESC_CMD_LED			'E'		/* LED operate command */
#define PAN_ESC_CMD_LED_LEVEL	0		/* Level status LED */

#define	PAN_ESC_CMD_LBD_ON		1		/* Set LBD to turn on status */
#define	PAN_ESC_CMD_LBD_OFF		0		/* Set LBD to turn off status */

#if ((UPGRADE_FORMAT & BOOT_UPG) == BOOT_UPG)
#define PAN_ESC_CMD_STANDBY_LED			'P'		/* LED operate command */
#endif

#ifndef __ASSEMBLER__

#include <hld/pan/pan_dev.h>

/* Note:
 * Please don't change follow structure for it will be used in HW information
 * field in FLASH chunk head and some ASM code.
 */

struct pan_gpio_info		/* Total 2 byte */
{
	UINT16	polar	: 1;	/* Polarity of GPIO, 0 or 1 active(light) */
	UINT16	io		: 1;	/* HAL_GPIO_I_DIR or HAL_GPIO_O_DIR in hal_gpio.h */
	UINT16	position: 14;	/* GPIO index, upto over 64 GPIO */
} __attribute__((packed));

//#if (((SYS_CHIP_MODULE==ALI_S3601)||(SYS_CHIP_MODULE==ALI_S3602)) && (!defined(PAN_INFO_ALIGN)))
#if 0
struct pan_hw_info			/* Total 16 bytes */
{
	/* Offset 0: Panal device type */
	UINT8					type_kb	: 2;	/* Key board (array) type */
	UINT8					type_scan:1;	/* 0: slot scan, 1: shadow scan */
	UINT8					type_key: 1;	/* Key exit or not */
	UINT8					type_irp: 3;	/* 0: not IRP, 1: NEC, 2: LAB */
	UINT8					type_mcu: 1;	/* MCU exit or not */
	/* Offset 1: GPIO number */
	UINT8					num_com	: 4;	/* Number of COM PIN, 0: no com; <= 8 */
	UINT8					pos_colon:4;	/* Position of colon flag, 0 to 7, 8 no colon */
    /* Offset 2: */
	UINT8					num_scan: 2;	/* Number of scan PIN, 0: no scan; <= 2 */
    UINT8					rsvd_bits:6;	/* in M3101: 0:show time,1:show "off ",2:show blank, 3:show " off"*/
	/* Offset 3: Panel shift control */
	struct pan_gpio_info	flatch;			/* Shifter latch PIN */
	struct pan_gpio_info	fclock;			/* Shifter clock PIN */
	struct pan_gpio_info	fdata;			/* Shifter data PIN */
	/* Offset 6: Panel scan control */
	struct pan_gpio_info	scan[2];		/* Panel scan PIN */
	/* Offset 8: Panel com PIN */
	struct pan_gpio_info	com[8];			/* COM PIN */
	/* Offset 16: Panel LBD control */
	struct pan_gpio_info	lbd[4];			/* LBD GPIO PIN */
	/* Offset 20: Panel input attribute */
	UINT32	intv_repeat_first;				/* Repeat interval first in mS */
    //24
	UINT32	intv_repeat;					/* Repeat interval in mS */
    //28
	UINT32	intv_release;					/* Release interval in mS */
    //32
	UINT32	(*hook_scan)(struct pan_device *dev, UINT32 key);
    //36
	UINT32	(*hook_show)(struct pan_device *dev, char *data, UINT32 len);
    //40
} __attribute__((packed));
#else
struct pan_hw_info			/* Total 16 bytes */
{
	/* Offset 0: Panel device type */
	UINT8					type_kb	: 2;	/* Key board (array) type */
	UINT8					type_scan:1;	/* 0: slot scan, 1: shadow scan */
	UINT8					type_key: 1;	/* Key exit or not */
	UINT8					type_irp: 3;	/* 0: not IRP, 1: NEC, 2: LAB */
	UINT8					type_mcu: 1;	/* MCU exit or not */
	/* Offset 1: GPIO number */
	UINT8					num_com : 4;	/* Number of COM PIN, 0: no com; <= 8 */
	UINT8					pos_colon:4;	/* Position of colon flag, 0 to 7, 8 no colon */
	/* Offset 2: */
	UINT8					num_scan: 2;	/* Number of scan PIN, 0: no scan; <= 2 */
	UINT8					rsvd_bits:6;	/* in M3101: 0:show time,1:show "off ",2:show blank, 3:show " off"*/
	/* Offset 3: */
	UINT8 					rsvd_byte;	/* Reserved for alignment */
	/* Offset 4: Panel shift latch */
	struct pan_gpio_info	flatch;			/* Shifter latch PIN */
	struct pan_gpio_info	fclock;			/* Shifter clock PIN */
	struct pan_gpio_info	fdata;			/* Shifter data PIN */
	/* Offset 10: Panel scan control */
	struct pan_gpio_info	scan[2];			/* Panel scan PIN */
	/* Offset 14: Panel com PIN */
	struct pan_gpio_info	com[8];			/* COM PIN */
	/* Offset 30: Panel LBD control */
	struct pan_gpio_info	lbd[4];			/* LBD GPIO PIN */
	struct pan_gpio_info	rsvd_hw;			/* Reserved for alignment */
	
	/* Offset 40: Panel input attribute */
	UINT32	intv_repeat_first;				/* Repeat interval first in mS */
    //44
	UINT32	intv_repeat;					/* Repeat interval in mS */
    //48
	UINT32	intv_release;					/* Release interval in mS */
    //52
	UINT32	(*hook_scan)(struct pan_device *dev, UINT32 key);
    //56
	UINT32	(*hook_show)(struct pan_device *dev, char *data, UINT32 len);
    //60
} __attribute__((packed));
#endif

#ifdef PAN_HT_16315_COMPATIBLE
/* LED bitmap */
struct led_bitmap
{
	UINT8 character;
	UINT16 bitmap;
};
#else
/* LED bitmap */
struct led_bitmap
{
	UINT8 character;
	UINT8 bitmap;
};
#endif

struct led_bitmap32
{
	UINT8 character;
	UINT32 bitmap;
};

/* Panel driver configuration structure */
struct pan_configuration
{
	struct pan_hw_info *hw_info;			/* Panel GPIO information */
	int					bitmap_len;			/* Bitmap list length */
	struct led_bitmap  *bitmap_list;		/* Bitmap list */
};

struct pan_configuration32
{
	struct pan_hw_info *hw_info;			/* Panel GPIO information */
	int					bitmap_len;			/* Bitmap list length */
	struct led_bitmap32  *bitmap_list;		/* Bitmap list */
};

#ifdef __cplusplus
extern "C"
{
#endif

INT32 pan_open(struct pan_device *dev);
INT32 pan_close(struct pan_device *dev);
INT32 pan_display(struct pan_device *dev, char *data, UINT32 len);
INT32 pan_io_control(struct pan_device *dev, INT32 cmd, UINT32 param);

struct pan_key * pan_get_key(struct pan_device *dev, UINT32 timeout);
INT32 pan_send_data(struct pan_device *dev, UINT8 *data, UINT32 len, UINT32 timeout);
INT32 pan_receive_data(struct pan_device *dev, UINT8 *data, UINT32 len, UINT32 timeout);

INT32 pan_buff_clear();
INT32 pan_buff_queue_tail(struct pan_key *key);

#ifdef __cplusplus
}
#endif
#endif

#endif /* __HLD_PAN_H__ */
