/*
 * Buffer handling functions 
 *
 */

#ifndef __SMC_T1_BUFFER_H__
#define __SMC_T1_BUFFER_H__

#include <types.h>

typedef struct t1_buf 
{
	UINT8*		base;
	UINT32		head, tail, size;
	UINT32		overrun;
} t1_buf_t;

extern void 	t1_buf_init(t1_buf_t *, void *, UINT32);
extern void 	t1_buf_set(t1_buf_t *, void *, UINT32);
extern INT32 	t1_buf_get(t1_buf_t *, void *, UINT32);
extern INT32 	t1_buf_put(t1_buf_t *, const void *, UINT32);
extern INT32 	t1_buf_putc(t1_buf_t *, INT32  );
extern UINT32 	t1_buf_avail(t1_buf_t * );
extern void *	t1_buf_head(t1_buf_t * );

#endif

