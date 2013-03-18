/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2007 Copyright (C)
 *
 *  File: smc_t1_buffer.c
 *
 *  Description: This file contains functions to manage T1 send/rcv buffer
 *		             
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  1. 2007.3.15  Grady Xu        0.1.000       Initial
 *  
 ****************************************************************************/

#include <types.h>
#include <api/libc/string.h>
#include "smc_t1_buffer.h"

void t1_buf_init(t1_buf_t *bp, void *mem, UINT32 len)
{
	MEMSET(bp, 0, sizeof(*bp));
	bp->base = (UINT8*) mem;
	bp->size = len;
}

void t1_buf_set(t1_buf_t *bp, void *mem, UINT32 len)
{
	t1_buf_init(bp, mem, len);
	bp->tail = len;
}

INT32 t1_buf_get(t1_buf_t *bp, void *mem, UINT32 len)
{
	if (len > bp->tail - bp->head)
		return -1;
	if (mem)
		MEMCPY(mem, bp->base + bp->head, len);
	bp->head += len;
	return len;
}

INT32 t1_buf_put(t1_buf_t *bp, const void *mem, UINT32 len)
{
	if (len > bp->size - bp->tail) {
		bp->overrun = 1;
		return -1;
	}
	if (mem)
		MEMCPY(bp->base + bp->tail, mem, len);
	bp->tail += len;
	return len;
}

INT32 t1_buf_putc(t1_buf_t *bp, INT32 byte)
{
	UINT8 c = byte;

	return t1_buf_put(bp, &c, 1);
}

UINT32 t1_buf_avail(t1_buf_t *bp)
{
	return bp->tail - bp->head;
}

void *t1_buf_head(t1_buf_t *bp)
{
	return bp->base + bp->head;
}

