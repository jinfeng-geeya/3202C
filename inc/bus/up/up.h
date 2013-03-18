/****************************************************************************
*
*  ALi (Shanghai) Corporation, All Rights Reserved. 2007 Copyright (C)
*
*  File: uP.h
*
*  Description: Header file of the management of uP bus.
*  History:
*      Date        Author         Version   Comment
*      ====        ======         =======   =======
*  1.  2007.11.07  Trueve Hu     0.1.000   Initial
****************************************************************************/

#ifndef __UP_H__
#define __UP_H__

#include <basic_types.h>


#define up_mode_config(ale, tcpwh, tcph, tcs, twh, enwr) \
	*((volatile UINT32 *)0xb8009090) = (ale<<28) | (tcpwh<<24) | \
	(tcph<<16) | (tcs<<12) | (twh<<4) | enwr


#define up_read8(a)			(*(volatile UINT8 *)(a))
#define up_read16(a)			(*(volatile UINT16 *)(a))
#define up_read32(a)			(*(volatile UINT32 *)(a))

#define up_write8(a, d)		(*(volatile UINT8 *)(a) = (d))
#define up_write16(a, d)		(*(volatile UINT16 *)(a) = (d))
#define up_write32(a, d)		(*(volatile UINT32 *)(a) = (d))

#define pinmux_set_up()		(*(volatile UINT32 *)(0xb800002C) |= (1<<4))
#define pinmux_set_flash()	(*(volatile UINT32 *)(0xb800002C) &= ~(1<<4))
#define pinmux_check_up()	(*(volatile UINT32 *)(0xb800002C) & (1<<4))

#define UP_DMA_BUSY		(1<<3)
#define UP_DMA_IDLE			(1<<4)
#define UP_DMA_START		(1<<5)
#define UP_DMA_FIXED		(1<<6)
#define UP_DMA_FLASH_MEM	(0<<7)
#define UP_DMA_MEM_FLASH	(1<<7)
#define UP_DMA_INVERT		(1<<8)
#define UP_DMA_FINISH		(1<<9)

#define UP_DMA_CONTINUE	(1<<28)
#define UP_DMA_ETHER		(1<<29)
#define UP_DMA_FLASH		(1<<30)
#define UP_DMA_SYNC		(1<<31)


typedef struct
{
	struct hld_device *pdev;
	UINT32 flag;
	UINT32 mem_addr;
	UINT32 flash_addr;
	UINT32 len;
	void (*pre_callback) (UINT32);
	void (*post_callback) (UINT32);
} udcb;

void up_mutex_enter(void);
void up_mutex_leave(void);
RET_CODE up_dma_init(void);
RET_CODE up_dma_close(void);
void up_dma_add_task(udcb *ptsk);

#endif /* __UP_H__ */

