/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2003 Copyright (C)
*
*    File:    sci_16550.c
*
*    Description:    This file contains all globe micros and functions declare
*		             of 16550 UART.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Sep.19.2004       Justin Wu      Ver 0.1    Create file.
*	2.  Jan.13.2006       Justin Wu      Ver 0.2    Make support multi UART.
*	3.	Feb.16.2006       Justin Wu      Ver 0.3    Support multi task.
*****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <osal/osal.h>
#include <bus/sci/sci.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>


#define SCI_16550_URBR				0
#define SCI_16550_UTBR				0
#define SCI_16550_UIER				1
#define SCI_16550_UIIR				2
#define SCI_16550_UFCR				2
#define SCI_16550_UDLL				0
#define SCI_16550_UDLM				1
#define SCI_16550_ULCR				3
#define SCI_16550_UMCR				4
#define SCI_16550_ULSR				5
#define SCI_16550_UMSR				6
#define SCI_16550_USCR				7
#define SCI_16550_DEVC				8
#define SCI_16550_RCVP				9

#define SCI_16550UART_RX_BUF_SIZE	256

/* uart_write_lock(unlock) is used to remove the mutex protection in sci_write(),
 *  for sci_write() might be used in interrupt handling fuction,
 *  while tds2 does not support synchronization object in interrupt function*/
#define uart_write_lock(...)			  do{}while(0)
#define uart_write_unlock(...)			  do{}while(0)
//#define uart_write_lock				  osal_mutex_lock
//#define uart_write_unlock				  osal_mutex_unlock


static struct sci_uart_st
{
	UINT8  rx_buf[SCI_16550UART_RX_BUF_SIZE];
	UINT32 rx_buf_head;
	UINT32 rx_buf_tail;
	UINT8  loopback_flag;
	UINT32 rd_mutex_id;
	UINT32 wt_mutex_id;
	UINT32 timeout;
} *sci_16550 = NULL;

/* For M3327/M3327C serial chip */
#define SCI_16550_NUM				2
static struct
{
	UINT32 reg_base;
	int    irq;
	UINT32 strap_ctrl;
#if (SYS_CHIP_MODULE == ALI_S3601)
} sci_16550_reg[SCI_16550_NUM] = {{0xb8018300, 24, 0}, {0xb8018400, 25, 0}};
#elif (SYS_CHIP_MODULE == ALI_S3602)
#ifdef SEE_CPU
} sci_16550_reg[SCI_16550_NUM] = {{0xB8018600, 25, 0}, {0xB8018300, 24, 0}};
#else
	#if((SYS_CPU_MODULE == CPU_M6303) && (SYS_PROJECT_FE==PROJECT_FE_DVBT))//for 3105
} sci_16550_reg[SCI_16550_NUM] = {{0xb8001300, 16, 0}, {0xb8001400, 14, 0x00180000}};
	#else
} sci_16550_reg[SCI_16550_NUM] = {{0xB8018300, 24, 0}, {0xB8018600, 25, 0}};
	#endif
#endif
#elif (SYS_CHIP_MODULE==ALI_M3329E) 
} sci_16550_reg[SCI_16550_NUM] = {{0xb8001300, 16, 0}, {0xb8001600, 14, 0}};
#else 
} sci_16550_reg[SCI_16550_NUM] = {{0xb8001300, 16, 0}, {0xb8001400, 14, 0x00180000}};
#endif

#define SCI_READ8(id, reg)			(*((volatile UINT8 *)(sci_16550_reg[id].reg_base + reg)))
#define SCI_WRITE8(id, reg, data)	(*((volatile UINT8 *)(sci_16550_reg[id].reg_base + reg)) = (data))


static void sci_16550uart_interrupt(UINT32 id);

__ATTRIBUTE_REUSE_
INT32 sci_16550uart_attach(int dev_num)
{
	int i;
	UINT32 chip_id,chip_ver;

	if (dev_num > SCI_16550_NUM)
		return ERR_FAILUE;

	chip_id = sys_ic_get_chip_id();
	chip_ver =sys_ic_get_rev_id();
	if ((ALI_M3329E == chip_id)&& (chip_ver>=IC_REV_5 ))
	{
		sci_16550_reg[SCI_16550_NUM-1].irq = 22;
	}
	if((1==sys_ic_is_M3202()) || sys_ic_is_M3101())
	{
		sci_16550_reg[0].reg_base= 0xb8001300;
		sci_16550_reg[0].irq = 16;
		sci_16550_reg[0].strap_ctrl = 0;
		
		sci_16550_reg[SCI_16550_NUM-1].reg_base = 0xb8001600;
		sci_16550_reg[SCI_16550_NUM-1].irq = 22;
		sci_16550_reg[SCI_16550_NUM-1].strap_ctrl = 0;
	}
	if (dev_num > 0)
	{
		dev_num = (dev_num > SCI_16550_NUM) ? SCI_16550_NUM : dev_num;
		sci_16550 = (struct sci_uart_st *)MALLOC(sizeof(struct sci_uart_st) * dev_num);
		if (sci_16550 == NULL)
			return ERR_FAILUE;
		MEMSET(sci_16550, 0, sizeof(struct sci_uart_st) * dev_num);

		for (i = 0; i < dev_num; i++)
		{
			sci_16550[i].rx_buf_head = sci_16550[i].rx_buf_tail = 0;
			sci_16550[i].loopback_flag = 0;
			sci_16550[i].rd_mutex_id = osal_mutex_create();
			sci_16550[i].wt_mutex_id = osal_mutex_create();
			osal_interrupt_register_lsr(sci_16550_reg[i].irq, sci_16550uart_interrupt, (UINT32)i);
			if (sci_16550_reg[i].strap_ctrl != 0)
				*(UINT32 *)0xb8000074 = sci_16550_reg[i].strap_ctrl;
		}
	}
	return SUCCESS;
}

void sci_16550uart_set_mode(UINT32 id, UINT32 bps, int parity)
{
	unsigned int div;

    if (sci_16550 == NULL)
        return;
	
	osal_mutex_lock(sci_16550[id].wt_mutex_id, OSAL_WAIT_FOREVER_TIME);
	sci_16550[id].timeout = 20000;//10000000 / bps;	/* Tout = 10*10^6/Rband *//*while write first char after mode set, takes 5ms*/
	/* Disable all interrupt */
	SCI_WRITE8(id, SCI_16550_UIER, 0);

	/* Set baud rate and transmit format */
	if (bps > 2000000)
	{
		SCI_WRITE8(id, SCI_16550_DEVC, SCI_READ8(id, SCI_16550_DEVC) | 0x08);
		div = (bps / 90000) * 1843;
	} else if (bps > 115200)
	{
		SCI_WRITE8(id, SCI_16550_DEVC, SCI_READ8(id, SCI_16550_DEVC) | 0x08);
		div = (bps / 32000) * 1843;
	} else
	{
		if (ALI_M3327C == sys_ic_get_chip_id() && sys_ic_get_rev_id() >= IC_REV_3)
			SCI_WRITE8(id, SCI_16550_DEVC, SCI_READ8(id, SCI_16550_DEVC) & 0xf7);
	}
	div = 115200 / bps;
	SCI_WRITE8(id, SCI_16550_ULCR, 0x9b);	/* Enable setup baud rate */
	SCI_WRITE8(id, SCI_16550_UDLL, (div & 0xff));
	SCI_WRITE8(id, SCI_16550_UDLM, ((div >> 8) & 0xff));

	div = (((parity >> 6) & 0x04) | ((~(parity >> 4)) & 0x03));
	switch (parity & 0x03)
	{
	case SCI_PARITY_EVEN:
		SCI_WRITE8(id, SCI_16550_ULCR, 0x18 | div);	/* even parity */
		break;
	case SCI_PARITY_ODD:
		SCI_WRITE8(id, SCI_16550_ULCR, 0x08 | div);	/* odd parity */
		break;
	default :
		SCI_WRITE8(id, SCI_16550_ULCR, 0x00 | div);	/* none parity */
		break;
	};

	/* Enable FIFO, threshold is 4 bytes */
	UINT32 chip_id;
	UINT32 rev_id;

	chip_id = sys_ic_get_chip_id();
	rev_id = sys_ic_get_rev_id();
	if ((chip_id == ALI_M3327 && rev_id < IC_REV_4) || (chip_id == ALI_M3327C && rev_id < IC_REV_1))
	{
		SCI_WRITE8(id, SCI_16550_UFCR, 0x41);	/* Don't reset FIFO for fix IC bug */
	} else
	{
		SCI_WRITE8(id, SCI_16550_UFCR, 0x47);	/* Reset FIFO */
	}

	SCI_WRITE8(id, SCI_16550_ULSR, 0x00);		/* Reset line status */
	SCI_WRITE8(id, SCI_16550_UMCR, 0x03);		/* Set modem control */

	/* Enable receiver interrupt */
	SCI_WRITE8(id, SCI_16550_UIER, 0x05);		/* Enable RX & timeout interrupt */

	/* signon message or measure TIMEOUT */
	osal_mutex_unlock(sci_16550[id].wt_mutex_id);
}

static void sci_16550uart_interrupt(UINT32 id)
{
	UINT8 istatus;
	int count;

	ASSERT(sci_16550 != NULL);
	/* It seems not need read UIIR but LSR for interrupt processing, but need
	 * to read UIIR for clear interrupt. If ULSR error ocured, read ULSR and
	 * clear it. */
	while (((istatus = (SCI_READ8(id, SCI_16550_UIIR) & 0x0f)) & 1) == 0)
	{
		switch (istatus)
		{
			case 0x06:	/* LSR error: OE, PE, FE, or BI */
				if (SCI_READ8(id, SCI_16550_ULSR) & 0x9e)
				{
					PRINTF("sci_16550uart_interrupt: lstatus error!\n");
				}
				/* We continue receive data at this condition */
			case 0x0c:	/* Character Timer-outIndication */
			case 0x04:	/* Received Data Available */
				count = 0;
				while (SCI_READ8(id, SCI_16550_ULSR) & 1)
				{
					/* Patch for M3327 AXX/BXA */
					if ((sys_ic_get_rev_id() < IC_REV_4) && (++count >= SCI_16550UART_RX_BUF_SIZE))
					{
						PRINTF("sci_16550uart_interrupt: chip error!\n");
						osal_interrupt_unregister_lsr(sci_16550_reg[id].irq, sci_16550uart_interrupt);
						return;
					}
					sci_16550[id].rx_buf[sci_16550[id].rx_buf_head++] = SCI_READ8(id, SCI_16550_URBR);
					sci_16550[id].rx_buf_head %= SCI_16550UART_RX_BUF_SIZE;
					/* Patch for M3327 AXA */
					if (sys_ic_get_rev_id() == IC_REV_0)
					{
						osal_delay(1);
					}
				}
				break;
			case 0x02:	/* TransmitterHoldingRegister Empty */
			case 0x00:	/* Modem Status */
			default:
				break;
		}
	}

	return;
}

UINT8 sci_16550uart_read(UINT32 id)
{
	UINT8 data;

    if (sci_16550 == NULL)
        return 0;

	ASSERT(sci_16550 != NULL);
	osal_mutex_lock(sci_16550[id].rd_mutex_id, OSAL_WAIT_FOREVER_TIME);
    while (sci_16550[id].rx_buf_head == sci_16550[id].rx_buf_tail)
    {
    	osal_task_sleep(1);
	}

    data = sci_16550[id].rx_buf[sci_16550[id].rx_buf_tail++];
    sci_16550[id].rx_buf_tail %= SCI_16550UART_RX_BUF_SIZE;
	osal_mutex_unlock(sci_16550[id].rd_mutex_id);

    return data;
}

INT32 sci_16550uart_read_tm(UINT32 id, UINT8* data,INT32 timeout)
{
    	if (sci_16550 == NULL||timeout<0)
        	return RET_FAILURE;
	ASSERT(sci_16550 != NULL);
	osal_mutex_lock(sci_16550[id].rd_mutex_id, OSAL_WAIT_FOREVER_TIME);
	while(1)
	{
		if (sci_16550[id].rx_buf_head == sci_16550[id].rx_buf_tail)
		{
			if (timeout <= 0)
			{	
				osal_mutex_unlock(sci_16550[id].rd_mutex_id);
				return ERR_FAILUE;
			}
			osal_delay(1);
			timeout--;
		}
		else
			break;
	}

	*data = sci_16550[id].rx_buf[sci_16550[id].rx_buf_tail++];
	sci_16550[id].rx_buf_tail %= SCI_16550UART_RX_BUF_SIZE;
	osal_mutex_unlock(sci_16550[id].rd_mutex_id);

	return SUCCESS;
}

void sci_16550uart_loopback(UINT32 id, UINT8 mode)
{
    if (sci_16550 == NULL)
        return;
	ASSERT(sci_16550 != NULL);
	sci_16550[id].loopback_flag = mode;
}

void sci_16550uart_write(UINT32 id, UINT8 ch)
{
	int i;
	int retry = 3;

    if (sci_16550 == NULL)
        return;

	ASSERT(sci_16550 != NULL);
	if (sci_16550[id].loopback_flag)
	{
		osal_interrupt_disable();
		sci_16550[id].rx_buf[sci_16550[id].rx_buf_head++] = ch;
		sci_16550[id].rx_buf_head %= SCI_16550UART_RX_BUF_SIZE;
		osal_interrupt_enable();
		return;
	}

	//osal_mutex_lock(sci_16550[id].wt_mutex_id, OSAL_WAIT_FOREVER_TIME);
	uart_write_lock(sci_16550[id].wt_mutex_id, OSAL_WAIT_FOREVER_TIME);
	while (retry)
	{
		/* Send character. */
		SCI_WRITE8(id, SCI_16550_UTBR, ch);

		/* wait for transmission finished */
		i = sci_16550[id].timeout;
		while (--i)
		{
			osal_delay(1);
			if (SCI_READ8(id, SCI_16550_ULSR) & 0x20)
			{
#if (SYS_CHIP_MODULE == ALI_S3602)
				// When CPU is too fast, UART output will have repeated character,
				// Add a delay is a temporary solution, wait for IC team check it.
				osal_delay(10);
#endif
				break;
			}
		}
		if (0 != i)
			break;

		/* Timeout, reset XMIT FIFO */
		SCI_WRITE8(id, SCI_16550_UFCR, SCI_READ8(id, SCI_16550_UFCR) | 0x04);
		osal_delay(10);
		retry --;
	}
	//osal_mutex_unlock(sci_16550[id].wt_mutex_id);
	uart_write_unlock(sci_16550[id].wt_mutex_id);
	return;
}

INT32 sci_16550uart_fifowrite(UINT32 id, UINT8 *buf,int len)
{
	//null to avoid compile error in shuttle time
}
void sci_16550uart_clear_buff(UINT32 id)
{
    if (sci_16550 == NULL)
        return;
	ASSERT(sci_16550 != NULL);
	sci_16550[id].rx_buf_head = sci_16550[id].rx_buf_tail = 0;
}
