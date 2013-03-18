/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 Copyright (C)
*
*    File:    hal_gpio.h
*
*    Description:    This file contains all functions about GPIO operations.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Jan.30.2003       Justin Wu       Ver 0.1    Create file.
*	2.	2003.6.26         Liu Lan         Ver 0.2    Exclusive access
*	3.  2005.5.12	      Justin Wu       Ver 0.3    Support M3327/M3327C only.
*	4.  2005.12.9	      Justin Wu       Ver 0.4    Support interrupt setting.
*****************************************************************************/

#ifndef	__HAL_GPIO_H__
#define __HAL_GPIO_H__

#include <types.h>
#include <osal/osal.h>
#include <sys_config.h>

#if (SYS_GPIO_MODULE == M6303GPIO)

/* GPIO input/output register in M3327/M3327C, etc:
 * 1 for output enable, 0 for disable.
 */
#define HAL_GPIO_IER_REG	0xb8000044
#define HAL_GPIO_REC_REG	0xb8000048
#define HAL_GPIO_FEC_REG	0xb800004c
#define HAL_GPIO_ISR_REG	0xb800005c
#define HAL_GPIO_DIR_REG	0xb8000058
#define HAL_GPIO_DI_REG		0xb8000050
#define HAL_GPIO_DO_REG		0xb8000054

#if(SYS_CHIP_MODULE == ALI_S3601)
#define HAL_GPIO1_IER_REG	0xb80000c4
#define HAL_GPIO1_REC_REG	0xb80000c8
#define HAL_GPIO1_FEC_REG	0xb80000cc
#define HAL_GPIO1_ISR_REG	0xb80000dc
#define HAL_GPIO1_DIR_REG	0xb80000d8
#define HAL_GPIO1_DI_REG	0xb80000d0
#define HAL_GPIO1_DO_REG	0xb80000d4

#define HAL_GPIO2_IER_REG	0xb8000000
#define HAL_GPIO2_REC_REG	0xb8000000
#define HAL_GPIO2_FEC_REG	0xb8000000
#define HAL_GPIO2_ISR_REG	0xb8000000
#define HAL_GPIO2_DIR_REG	0xb8000000
#define HAL_GPIO2_DI_REG	0xb8000000
#define HAL_GPIO2_DO_REG	0xb8000000

#define HAL_GPIO3_IER_REG	0xb8000000
#define HAL_GPIO3_REC_REG	0xb8000000
#define HAL_GPIO3_FEC_REG	0xb8000000
#define HAL_GPIO3_ISR_REG	0xb8000000
#define HAL_GPIO3_DIR_REG	0xb8000000
#define HAL_GPIO3_DI_REG	0xb8000000
#define HAL_GPIO3_DO_REG	0xb8000000

#define HAL_GPIO4_IER_REG	0xb8000000
#define HAL_GPIO4_REC_REG	0xb8000000
#define HAL_GPIO4_FEC_REG	0xb8000000
#define HAL_GPIO4_ISR_REG	0xb8000000
#define HAL_GPIO4_DIR_REG	0xb8000000
#define HAL_GPIO4_DI_REG	0xb8000000
#define HAL_GPIO4_DO_REG	0xb8000000

#elif ((SYS_CHIP_MODULE == ALI_S3602) && (SYS_CPU_MODULE != CPU_M6303))//for 3105

#define HAL_GPIO1_IER_REG	0xb80000c4
#define HAL_GPIO1_REC_REG	0xb80000c8
#define HAL_GPIO1_FEC_REG	0xb80000cc
#define HAL_GPIO1_ISR_REG	0xb80000dc
#define HAL_GPIO1_DIR_REG	0xb80000d8
#define HAL_GPIO1_DI_REG	0xb80000d0
#define HAL_GPIO1_DO_REG	0xb80000d4

#define HAL_GPIO2_IER_REG	0xb800000c
#define HAL_GPIO2_REC_REG	0xb8000010
#define HAL_GPIO2_FEC_REG	0xb8000014
#define HAL_GPIO2_ISR_REG	0xb80000b4
#define HAL_GPIO2_DIR_REG	0xb80000ec
#define HAL_GPIO2_DI_REG	0xb80000e4
#define HAL_GPIO2_DO_REG	0xb80000e8

#define HAL_GPIO3_DIR_REG	0xb80000f8
#define HAL_GPIO3_DI_REG	0xb80000f0
#define HAL_GPIO3_DO_REG	0xb80000f4
#define HAL_GPIO3_IER_REG	0xb8000000
#define HAL_GPIO3_REC_REG	0xb8000000
#define HAL_GPIO3_FEC_REG	0xb8000000
#define HAL_GPIO3_ISR_REG	0xb8000000

#define HAL_GPIO4_IER_REG	0xb8000000
#define HAL_GPIO4_REC_REG	0xb8000000
#define HAL_GPIO4_FEC_REG	0xb8000000
#define HAL_GPIO4_ISR_REG	0xb8000000
#define HAL_GPIO4_DIR_REG	0xb8000000
#define HAL_GPIO4_DI_REG	0xb8000000
#define HAL_GPIO4_DO_REG	0xb8000000

#else

#define HAL_GPIO1_IER_REG	0xb80000a4
#define HAL_GPIO1_REC_REG	0xb80000a8
#define HAL_GPIO1_FEC_REG	0xb80000ac
#define HAL_GPIO1_ISR_REG	0xb80000bc
#define HAL_GPIO1_DIR_REG	0xb80000b8
#define HAL_GPIO1_DI_REG	0xb80000b0
#define HAL_GPIO1_DO_REG	0xb80000b4

#define HAL_GPIO2_IER_REG	0xb80000c0
#define HAL_GPIO2_REC_REG	0xb80000c4
#define HAL_GPIO2_FEC_REG	0xb80000c8
#define HAL_GPIO2_ISR_REG	0xb80000d8
#define HAL_GPIO2_DIR_REG	0xb80000d4
#define HAL_GPIO2_DI_REG	0xb80000cc
#define HAL_GPIO2_DO_REG	0xb80000d0

#define HAL_GPIO3_IER_REG	0xb8000000
#define HAL_GPIO3_REC_REG	0xb8000000
#define HAL_GPIO3_FEC_REG	0xb8000000
#define HAL_GPIO3_ISR_REG	0xb8000000
#define HAL_GPIO3_DIR_REG	0xb8000000
#define HAL_GPIO3_DI_REG	0xb8000000
#define HAL_GPIO3_DO_REG	0xb8000000

#define HAL_GPIO4_IER_REG	0xb8000000
#define HAL_GPIO4_REC_REG	0xb8000000
#define HAL_GPIO4_FEC_REG	0xb8000000
#define HAL_GPIO4_ISR_REG	0xb8000000
#define HAL_GPIO4_DIR_REG	0xb8000000
#define HAL_GPIO4_DI_REG	0xb8000000
#define HAL_GPIO4_DO_REG	0xb8000000

#endif

#define HAL_GPIO_I_DIR		0
#define HAL_GPIO_O_DIR		1

#define HAL_GPIO_INT_EN		0
#define HAL_GPIO_INT_DIS	1
#define HAL_GPIO_EDG_EN		1
#define HAL_GPIO_EDG_DIS	0

#elif (SYS_GPIO_MODULE == M3602F_GPIO)

#define HAL_GPIO_IER_REG	0xb8000044
#define HAL_GPIO_REC_REG	0xb8000048
#define HAL_GPIO_FEC_REG	0xb800004c
#define HAL_GPIO_ISR_REG	0xb800005c
#define HAL_GPIO_DIR_REG	0xb8000058
#define HAL_GPIO_DI_REG		0xb8000050
#define HAL_GPIO_DO_REG		0xb8000054

#define HAL_GPIO1_IER_REG	0xb80000c4
#define HAL_GPIO1_REC_REG	0xb80000c8
#define HAL_GPIO1_FEC_REG	0xb80000cc
#define HAL_GPIO1_ISR_REG	0xb80000dc
#define HAL_GPIO1_DIR_REG	0xb80000d8
#define HAL_GPIO1_DI_REG	0xb80000d0
#define HAL_GPIO1_DO_REG	0xb80000d4

#define HAL_GPIO2_IER_REG	0xb80000e4
#define HAL_GPIO2_REC_REG	0xb80000e8
#define HAL_GPIO2_FEC_REG	0xb80000ec
#define HAL_GPIO2_ISR_REG	0xb80000fc
#define HAL_GPIO2_DIR_REG	0xb80000f8
#define HAL_GPIO2_DI_REG	0xb80000f0
#define HAL_GPIO2_DO_REG	0xb80000f4

#define HAL_GPIO3_IER_REG	0xb8000344
#define HAL_GPIO3_REC_REG	0xb8000348
#define HAL_GPIO3_FEC_REG	0xb800034c
#define HAL_GPIO3_ISR_REG	0xb800035c
#define HAL_GPIO3_DIR_REG	0xb8000358
#define HAL_GPIO3_DI_REG	0xb8000350
#define HAL_GPIO3_DO_REG	0xb8000354

#define HAL_GPIO4_IER_REG	0xb8000444
#define HAL_GPIO4_REC_REG	0xb8000448
#define HAL_GPIO4_FEC_REG	0xb800044c
#define HAL_GPIO4_ISR_REG	0xb800045c
#define HAL_GPIO4_DIR_REG	0xb8000458
#define HAL_GPIO4_DI_REG	0xb8000450
#define HAL_GPIO4_DO_REG	0xb8000454

#define HAL_GPIO_I_DIR		0
#define HAL_GPIO_O_DIR		1

#define HAL_GPIO_INT_EN		0
#define HAL_GPIO_INT_DIS	1
#define HAL_GPIO_EDG_EN		1
#define HAL_GPIO_EDG_DIS	0

#else

#define HAL_GPIO_IER_REG	0
#define HAL_GPIO_REC_REG	0
#define HAL_GPIO_FEC_REG	0
#define HAL_GPIO_DIR_REG	0
#define HAL_GPIO_DI_REG		0
#define HAL_GPIO_DO_REG		0

#define HAL_GPIO1_IER_REG	0
#define HAL_GPIO1_REC_REG	0
#define HAL_GPIO1_FEC_REG	0
#define HAL_GPIO1_DIR_REG	0
#define HAL_GPIO1_DI_REG	0
#define HAL_GPIO1_DO_REG	0

#define HAL_GPIO_I_DIR		0
#define HAL_GPIO_O_DIR		1

#define HAL_GPIO_INT_EN		0
#define HAL_GPIO_INT_DIS	1
#define HAL_GPIO_EDG_EN		1
#define HAL_GPIO_EDG_DIS	0

#endif

/* The first set of GPIO */
#define HAL_GPIO_READ()				(*(volatile UINT32 *)HAL_GPIO_DI_REG)
#define HAL_GPIO_WRITE(val)			(*(volatile UINT32 *)HAL_GPIO_DO_REG \
									 = (val))

#define HAL_GPIO_DIR_GET()			(*(volatile UINT32 *)HAL_GPIO_DIR_REG)
#define HAL_GPIO_DIR_SET(mode)		(*(volatile UINT32 *)HAL_GPIO_DIR_REG \
									 = (mode))

#define HAL_GPIO_IER_SET(val)		(*(volatile UINT32 *)HAL_GPIO_IER_REG \
									 = (val))
#define HAL_GPIO_RER_SET(val)		(*(volatile UINT32 *)HAL_GPIO_REC_REG \
									 = (val))
#define HAL_GPIO_FER_SET(val)		(*(volatile UINT32 *)HAL_GPIO_FEC_REG \
									 = (val))
#define HAL_GPIO_ISR_GET()			(*(volatile UINT32 *)HAL_GPIO_ISR_REG)

#define HAL_GPIO_ISR_SET(val)		(*(volatile UINT32 *)HAL_GPIO_ISR_REG \
									 = (val))
									 
/* The second set of GPIO */
#define HAL_GPIO1_READ()			(*(volatile UINT32 *)HAL_GPIO1_DI_REG)
#define HAL_GPIO1_WRITE(val)		(*(volatile UINT32 *)HAL_GPIO1_DO_REG \
									 = (val))

#define HAL_GPIO1_DIR_GET()			(*(volatile UINT32 *)HAL_GPIO1_DIR_REG)
#define HAL_GPIO1_DIR_SET(mode)		(*(volatile UINT32 *)HAL_GPIO1_DIR_REG \
									 = (mode))

#define HAL_GPIO1_IER_SET(val)		(*(volatile UINT32 *)HAL_GPIO1_IER_REG \
									 = (val))
#define HAL_GPIO1_RER_SET(val)		(*(volatile UINT32 *)HAL_GPIO1_REC_REG \
									 = (val))
#define HAL_GPIO1_FER_SET(val)		(*(volatile UINT32 *)HAL_GPIO1_FEC_REG \
									 = (val))
#define HAL_GPIO1_ISR_GET()			(*(volatile UINT32 *)HAL_GPIO1_ISR_REG)

#define HAL_GPIO1_ISR_SET(val)		(*(volatile UINT32 *)HAL_GPIO1_ISR_REG \
									 = (val))

/* The third set of GPIO */
#define HAL_GPIO2_READ()			(*(volatile UINT32 *)HAL_GPIO2_DI_REG)
#define HAL_GPIO2_WRITE(val)		(*(volatile UINT32 *)HAL_GPIO2_DO_REG \
									= (val))

#define HAL_GPIO2_DIR_GET()			(*(volatile UINT32 *)HAL_GPIO2_DIR_REG)
#define HAL_GPIO2_DIR_SET(mode)		(*(volatile UINT32 *)HAL_GPIO2_DIR_REG \
									= (mode))

#define HAL_GPIO2_IER_SET(val)		(*(volatile UINT32 *)HAL_GPIO2_IER_REG \
									 = (val))
#define HAL_GPIO2_RER_SET(val)		(*(volatile UINT32 *)HAL_GPIO2_REC_REG \
									 = (val))
#define HAL_GPIO2_FER_SET(val)		(*(volatile UINT32 *)HAL_GPIO2_FEC_REG \
									 = (val))
#define HAL_GPIO2_ISR_GET()			(*(volatile UINT32 *)HAL_GPIO2_ISR_REG)

#define HAL_GPIO2_ISR_SET(val)		(*(volatile UINT32 *)HAL_GPIO2_ISR_REG \
									 = (val))

/* The fourth set of GPIO */
#define HAL_GPIO3_READ()			(*(volatile UINT32 *)HAL_GPIO3_DI_REG)
#define HAL_GPIO3_WRITE(val)		(*(volatile UINT32 *)HAL_GPIO3_DO_REG \
									= (val))

#define HAL_GPIO3_DIR_GET()			(*(volatile UINT32 *)HAL_GPIO3_DIR_REG)
#define HAL_GPIO3_DIR_SET(mode)		(*(volatile UINT32 *)HAL_GPIO3_DIR_REG \
									= (mode))

//WHH10100311 start								
#define HAL_GPIO3_IER_SET(val)		(*(volatile UINT32 *)HAL_GPIO3_IER_REG \
									 = (val))
#define HAL_GPIO3_RER_SET(val)		(*(volatile UINT32 *)HAL_GPIO3_REC_REG \
									 = (val))
#define HAL_GPIO3_FER_SET(val)		(*(volatile UINT32 *)HAL_GPIO3_FEC_REG \
									 = (val))
#define HAL_GPIO3_ISR_GET()			(*(volatile UINT32 *)HAL_GPIO3_ISR_REG)

#define HAL_GPIO3_ISR_SET(val)		(*(volatile UINT32 *)HAL_GPIO3_ISR_REG \
									 = (val))
//WHH10100311 end									 


#define HAL_GPIO_BIT_GET(pos)			\
			((pos < 32) ? ((HAL_GPIO_READ() >> (pos)) & 1) \
			: ((pos < 64) ? ((HAL_GPIO1_READ() >> (pos - 32)) & 1) \
			: ((pos < 96) ? ((HAL_GPIO2_READ() >> (pos - 64)) & 1) \
			: ((HAL_GPIO3_READ() >> (pos - 96)) & 1))))

#define HAL_GPIO_BIT_DIR_GET(pos)		\
			((pos < 32) ? ((HAL_GPIO_DIR_GET() >> (pos)) & 1) \
			: ((pos < 64) ? ((HAL_GPIO1_DIR_GET() >> (pos - 32)) & 1) \
			: ((pos < 96) ? ((HAL_GPIO2_DIR_GET() >> (pos - 64)) & 1) \
			: ((HAL_GPIO3_DIR_GET() >> (pos - 96)) & 1)))

#define HAL_GPIO_BIT_DIR_SET(pos, val)	\
		do {osal_interrupt_disable();	\
			((pos < 32) ? HAL_GPIO_DIR_SET((HAL_GPIO_DIR_GET() & ~(1 << (pos))) | ((val) << (pos))) \
			: ((pos < 64) ? HAL_GPIO1_DIR_SET((HAL_GPIO1_DIR_GET() & ~(1 << (pos - 32))) | ((val) << (pos - 32))) \
			: ((pos < 96) ? HAL_GPIO2_DIR_SET((HAL_GPIO2_DIR_GET() & ~(1 << (pos - 64))) | ((val) << (pos - 64))) \
			: HAL_GPIO3_DIR_SET((HAL_GPIO3_DIR_GET() & ~(1 << (pos - 96))) | ((val) << (pos - 96))))));\
		    osal_interrupt_enable();	\
		} while (0)

/* For M3327/27C output attribute GPIO, the data read from DO and DI
 * are different, so we read D0 instead of DI for bit set operation */
#if (SYS_CHIP_MODULE ==	ALI_M3327C && (defined POS_GPIO_LATCH))
#define HAL_GPIO_BIT_SET(pos, val)		\
		do {osal_interrupt_disable();	\
			((pos < 32)					\
			? HAL_GPIO_WRITE(((*(volatile UINT32 *)HAL_GPIO_DO_REG) & ~(1 << (pos))) | ((val) << (pos))) \
			: HAL_GPIO1_WRITE(((*(volatile UINT32 *)HAL_GPIO1_DO_REG) & ~(1 << (pos - 32))) | ((val) << (pos - 32)))); \
			HAL_GPIO_BIT_DIR_SET(POS_GPIO_LATCH, HAL_GPIO_O_DIR);	\
			*(volatile UINT32 *)0xb8000028 = (*(volatile UINT32 *)0xb8000028 | 1);	\
			((POS_GPIO_LATCH < 32)		\
			? HAL_GPIO_WRITE(((*(volatile UINT32 *)HAL_GPIO_DO_REG) & ~(1 << (POS_GPIO_LATCH))) | ((0) << (POS_GPIO_LATCH))) \
			: HAL_GPIO1_WRITE(((*(volatile UINT32 *)HAL_GPIO1_DO_REG) & ~(1 << (POS_GPIO_LATCH - 32))) | ((0) << (POS_GPIO_LATCH - 32)))); \
			((POS_GPIO_LATCH < 32)					\
			? HAL_GPIO_WRITE(((*(volatile UINT32 *)HAL_GPIO_DO_REG) & ~(1 << (POS_GPIO_LATCH))) | ((1) << (POS_GPIO_LATCH))) \
			: HAL_GPIO1_WRITE(((*(volatile UINT32 *)HAL_GPIO1_DO_REG) & ~(1 << (POS_GPIO_LATCH - 32))) | ((1) << (POS_GPIO_LATCH - 32)))); \
			((POS_GPIO_LATCH < 32)					\
			? HAL_GPIO_WRITE(((*(volatile UINT32 *)HAL_GPIO_DO_REG) & ~(1 << (POS_GPIO_LATCH))) | ((0) << (POS_GPIO_LATCH))) \
			: HAL_GPIO1_WRITE(((*(volatile UINT32 *)HAL_GPIO1_DO_REG) & ~(1 << (POS_GPIO_LATCH - 32))) | ((0) << (POS_GPIO_LATCH - 32)))); \
			*(volatile UINT32 *)0xb8000028 = (*(volatile UINT32 *)0xb8000028 & ~1);	\
		    osal_interrupt_enable();	\
		} while (0)
#else
#define HAL_GPIO_BIT_SET(pos, val)		\
		do {osal_interrupt_disable();	\
			((pos < 32)	? HAL_GPIO_WRITE(((*(volatile UINT32 *)HAL_GPIO_DO_REG) & ~(1 << (pos))) | ((val) << (pos))) \
			: ((pos < 64) ? HAL_GPIO1_WRITE(((*(volatile UINT32 *)HAL_GPIO1_DO_REG) & ~(1 << (pos - 32))) | ((val) << (pos - 32))) \
			: ((pos < 96) ? HAL_GPIO2_WRITE(((*(volatile UINT32 *)HAL_GPIO2_DO_REG) & ~(1 << (pos - 64))) | ((val) << (pos - 64))) \
			: HAL_GPIO3_WRITE(((*(volatile UINT32 *)HAL_GPIO3_DO_REG) & ~(1 << (pos - 96))) | ((val) << (pos - 96)))))); \
		    osal_interrupt_enable();	\
		} while (0)
#endif

#define HAL_GPIO_INT_SET(pos, en)		\
		do {osal_interrupt_disable();	\
			((pos < 32)					\
			? HAL_GPIO_IER_SET(((*(volatile UINT32 *)HAL_GPIO_IER_REG) & ~(1 << (pos))) | ((en) << (pos))) \
			: ((pos < 64) 				\
			? HAL_GPIO1_IER_SET(((*(volatile UINT32 *)HAL_GPIO1_IER_REG) & ~(1 << (pos - 32))) | ((en) << (pos - 32))) \
			: ((pos < 96) 				\
			? HAL_GPIO2_IER_SET(((*(volatile UINT32 *)HAL_GPIO2_IER_REG) & ~(1 << (pos - 64))) | ((en) << (pos - 64))) \
			: HAL_GPIO3_IER_SET(((*(volatile UINT32 *)HAL_GPIO3_IER_REG) & ~(1 << (pos - 96))) | ((en) << (pos - 96)))))); \
		    osal_interrupt_enable();	\
		} while (0)

#define HAL_GPIO_INT_REDG_SET(pos, rise)	\
		do {osal_interrupt_disable();	\
			((pos < 32)					\
			? HAL_GPIO_RER_SET(((*(volatile UINT32 *)HAL_GPIO_REC_REG) & ~(1 << (pos))) | ((rise) << (pos))) \
			: ((pos < 64) 				\
			? HAL_GPIO1_RER_SET(((*(volatile UINT32 *)HAL_GPIO1_REC_REG) & ~(1 << (pos - 32))) | ((rise) << (pos - 32))) \
			: ((pos < 96) 				\
			? HAL_GPIO2_RER_SET(((*(volatile UINT32 *)HAL_GPIO2_REC_REG) & ~(1 << (pos - 64))) | ((rise) << (pos - 64))) \
			: HAL_GPIO3_RER_SET(((*(volatile UINT32 *)HAL_GPIO3_REC_REG) & ~(1 << (pos - 96))) | ((rise) << (pos - 96)))))); \
		    osal_interrupt_enable();	\
		} while (0)

#define HAL_GPIO_INT_FEDG_SET(pos, fall)	\
		do {osal_interrupt_disable();	\
			((pos < 32)					\
			? HAL_GPIO_FER_SET(((*(volatile UINT32 *)HAL_GPIO_FEC_REG) & ~(1 << (pos))) | ((fall) << (pos))) \
			: ((pos < 64)				\
			? HAL_GPIO1_FER_SET(((*(volatile UINT32 *)HAL_GPIO1_FEC_REG) & ~(1 << (pos - 32))) | ((fall) << (pos - 32))) \
			: ((pos < 96) 				\
			? HAL_GPIO2_FER_SET(((*(volatile UINT32 *)HAL_GPIO2_FEC_REG) & ~(1 << (pos - 64))) | ((fall) << (pos - 64))) \
			: HAL_GPIO3_FER_SET(((*(volatile UINT32 *)HAL_GPIO3_FEC_REG) & ~(1 << (pos - 96))) | ((fall) << (pos - 96)))))); \
		    osal_interrupt_enable();	\
		} while (0)

#define HAL_GPIO_INT_EDG_SET(pos, rise, fall)	\
		do {osal_interrupt_disable();	\
			((pos < 32)					\
			? (HAL_GPIO_RER_SET(((*(volatile UINT32 *)HAL_GPIO_REC_REG) & ~(1 << (pos))) | ((rise) << (pos))), \
			  HAL_GPIO_FER_SET(((*(volatile UINT32 *)HAL_GPIO_FEC_REG) & ~(1 << (pos))) | ((fall) << (pos)))) \
			: ((pos < 64)				\
			? (HAL_GPIO1_RER_SET(((*(volatile UINT32 *)HAL_GPIO1_REC_REG) & ~(1 << (pos - 32))) | ((rise) << (pos - 32))), \
			  HAL_GPIO1_FER_SET(((*(volatile UINT32 *)HAL_GPIO1_FEC_REG) & ~(1 << (pos - 32))) | ((fall) << (pos - 32)))) \
			: ((pos < 96)				\
			?  (HAL_GPIO2_RER_SET(((*(volatile UINT32 *)HAL_GPIO2_REC_REG) & ~(1 << (pos - 64))) | ((rise) << (pos - 64))), \
			  HAL_GPIO2_FER_SET(((*(volatile UINT32 *)HAL_GPIO2_FEC_REG) & ~(1 << (pos - 64))) | ((fall) << (pos - 64)))) \
			:  (HAL_GPIO3_RER_SET(((*(volatile UINT32 *)HAL_GPIO3_REC_REG) & ~(1 << (pos - 96))) | ((rise) << (pos - 96))), \
			  HAL_GPIO3_FER_SET(((*(volatile UINT32 *)HAL_GPIO3_FEC_REG) & ~(1 << (pos - 96))) | ((fall) << (pos - 96))))))); \
		    osal_interrupt_enable();	\
		} while (0)

#define HAL_GPIO_INT_STA_GET(pos)		\
			((pos < 32) ? ((HAL_GPIO_ISR_GET() >> (pos)) & 1) \
			: ((pos < 64) ? ((HAL_GPIO1_ISR_GET() >> (pos - 32)) & 1) \
			: ((pos < 96) ? ((HAL_GPIO2_ISR_GET() >> (pos - 64)) & 1) \
			: ((HAL_GPIO3_ISR_GET() >> (pos - 96)) & 1))))

#define HAL_GPIO_INT_CLEAR(pos)		\
			((pos < 32) ? (HAL_GPIO_ISR_SET(1 << (pos))) \
			: ((pos < 64) ? (HAL_GPIO1_ISR_SET(1 << (pos-32))) \
			: ((pos < 96) ? (HAL_GPIO2_ISR_SET(1 << (pos-64))) \
			: (HAL_GPIO3_ISR_SET(1 << (pos-96))))))

#endif	/* __HAL_GPIO_H__ */
