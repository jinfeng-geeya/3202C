/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2006 Copyright (C)
*
*    File:    dog_m3327e.c
*
*    Description:    This file contains all globe micros and functions declare
*		             of watchdog timer.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	May.2.2006       Justin Wu      Ver 0.1    Create file.
*	2.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <osal/osal.h>
#include <bus/dog/dog.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>


#define DOG_M3327E_WDTCNT			0
#define DOG_M3327E_WDTCTRL			4

#define DOG_M3327E_NUM				1

static struct
{
	UINT32 reg_base;
	int    irq;
	UINT32 timebase;			/* The init value of cnt (we need config dog in us) */
#if(SYS_CHIP_MODULE == ALI_S3601||SYS_CHIP_MODULE == ALI_S3602)	
} dog_m3327e_reg[DOG_M3327E_NUM] = {{0xb8018500, 31, 0}};
#else	
} dog_m3327e_reg[DOG_M3327E_NUM] = {{0xb8001500, 12, 0}};
#endif	


#define DOG_WRITE32(id, reg, data)	(*((volatile UINT32 *)(dog_m3327e_reg[id].reg_base + reg)) = (data))
#define DOG_READ32(id, reg)			(*((volatile UINT32 *)(dog_m3327e_reg[id].reg_base + reg)))
#define DOG_WRITE8(id, reg, data)	(*((volatile UINT8 *)(dog_m3327e_reg[id].reg_base + reg)) = (data))
#define DOG_READ8(id, reg)			(*((volatile UINT8 *)(dog_m3327e_reg[id].reg_base + reg)))

static UINT32 sys_mem_clk = 135;

__ATTRIBUTE_REUSE_
INT32 dog_m3327e_attach(int dev_num)
{
	int i;

	if (dev_num > DOG_M3327E_NUM)
		return ERR_FAILUE;
	if (dev_num > 0)
	{
		for (i = 0; i < dev_num; i++)
		{
			if(0==i&&ALI_M3329E==sys_ic_get_chip_id()&&sys_ic_get_rev_id()<IC_REV_5)
				dog_m3327e_reg[i].irq = 34;
		}
	}
	sys_mem_clk = sys_ic_get_dram_clock();
	
	return SUCCESS;
}

void dog_m3327e_set_mode(UINT32 id, UINT32 mode, UINT32 duration_us, void (*callback)(UINT32))
{
	UINT16 div;
	UINT32 a;

    // Old code was incorrect:
	// a = 0xffffffff / (sys_mem_clk * 1000000);

	a = 0xffffffff / sys_mem_clk;
	if (duration_us < (a << 5))
		div = 0;
	else if (duration_us < (a << 6))
		div = 1;
	else if (duration_us < (a << 7))
		div = 2;
	else
		div = 3;

	osal_interrupt_disable();

	dog_m3327e_reg[id].timebase = 0xffffffff - (duration_us / (1 << (5 + div)) * sys_mem_clk);
	/* Init timebase, it should always check */
	DOG_WRITE32(id, DOG_M3327E_WDTCNT, dog_m3327e_reg[id].timebase);
	/* Enable timer at sametime of set dog mode */
	if (mode & DOG_MODE_TIMER)			/* It is timer mode */
	{
		osal_interrupt_register_lsr(dog_m3327e_reg[id].irq, callback, id);
		DOG_WRITE8(id, DOG_M3327E_WDTCTRL, 0x14 | div);
	}
	else								/* It is watchdog mode */
	{
		DOG_WRITE8(id, DOG_M3327E_WDTCTRL, 0x64 | div);
	}

	osal_interrupt_enable();
}


UINT32 dog_m3327e_get_time(UINT32 id)
{
	UINT16 div;

	div = DOG_READ8(id, DOG_M3327E_WDTCTRL) & 3;
	div = (1 << (5 + div));
    return ((DOG_READ32(id, DOG_M3327E_WDTCNT) - dog_m3327e_reg[id].timebase) /
    	(sys_mem_clk * div));
}

void dog_m3327e_set_time(UINT32 id, UINT32 us)
{
	UINT16 div;

	osal_interrupt_disable();
	div = DOG_READ8(id, DOG_M3327E_WDTCTRL) & 3;
	div = (1 << (5 + div));
	DOG_WRITE32(id, DOG_M3327E_WDTCNT, (us / div * sys_mem_clk) + dog_m3327e_reg[id].timebase);
	osal_interrupt_enable();

    return;
}

void dog_m3327e_pause(UINT32 id, int en)
{
	osal_interrupt_disable();
	DOG_WRITE8(id, DOG_M3327E_WDTCTRL, (DOG_READ8(id, DOG_M3327E_WDTCTRL) & (~0x04)) | (en == 1 ? 0 : 0x04));
	osal_interrupt_enable();

    return;
}

void dog_m3327e_stop(UINT32 id)
{
	osal_interrupt_disable();
	DOG_WRITE32(id, DOG_M3327E_WDTCTRL, 0);
	DOG_WRITE32(id, DOG_M3327E_WDTCNT, 0);
	osal_interrupt_enable();

    return;
}

