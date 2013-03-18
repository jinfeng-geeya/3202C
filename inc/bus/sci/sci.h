/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: sci.h
 *
 *  Description: This file provide common Serial Communication Interface.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2003.2.13   Liu Lan     0.1.000  Initial
 *	2.  2003.3.12	Justin Wu	0.1.001  Add M6303GSI support
*	3.  2006.1.13   Justin Wu   0.2.000  Make support multi UART; remove MDM.
 ****************************************************************************/

#ifndef	__LLD_SCI_H__
#define __LLD_SCI_H__

#include <sys_config.h>

/* sci_mode_set(): parity bit alocate:
 *    byte1 half low      byte0 half high     byte0 half low
 *  (stop bit setting) (word length setting) (parity setting)
 */
/* Parity setting */
#define SCI_PARITY_NONE		0x0000
#define SCI_PARITY_EVEN		0x0001
#define SCI_PARITY_ODD		0x0002

/* Word length setting */
#define SCI_WORDLEN_8		0x0000
#define SCI_WORDLEN_7		0x0010
#define SCI_WORDLEN_6		0x0020
#define SCI_WORDLEN_5		0x0030

/* Stop bit setting */
#define SCI_STOPBIT_1		0x0000
#define SCI_STOPBIT_2		0x0100

/* Device ID */
#define SCI_FOR_RS232		0
#define SCI_FOR_MDM			1

#if (SYS_SCI_MODULE == UART16550)

#define sci_mode_set(id, bps, parity, ...)	sci_16550uart_set_mode(id, bps, parity)
#define sci_clear_buff(id)					sci_16550uart_clear_buff(id)
#define sci_loopback(id, mode)				sci_16550uart_loopback(id, mode)
#define sci_read(id)						sci_16550uart_read(id)
#define sci_read_tm(id, ch, tm)				sci_16550uart_read_tm(id, ch, tm)
#define sci_write(id, ch)					sci_16550uart_write(id, ch)
#define sci_fifowrite(id, buf,len)			sci_16550uart_fifowrite(id, buf, len)

#else

#define sci_mode_set(...)					do{}while(0)
#define sci_clear_buff(...)					do{}while(0)
#define sci_loopback(...)					do{}while(0)
#define sci_read(...)						0
#define sci_read_tm(...)					ERR_FAILUE
#define sci_write(...)						do{}while(0)

#endif


#endif	/* __LLD_SCI_H__ */
