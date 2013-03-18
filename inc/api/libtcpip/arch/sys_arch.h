/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
 
#ifndef __ARCH_SYS_ARCH_H__
#define __ARCH_SYS_ARCH_H__

#define SYS_MBOX_NULL	NULL
#define SYS_SEM_NULL	NULL

#define OS_TDS2_SYS_ARCH    

#ifdef OS_TDS2_SYS_ARCH  

#include <osal/osal.h>

#undef SYS_MBOX_NULL
#undef SYS_SEM_NULL
#define SYS_THREAD_NULL	OSAL_INVALID_ID
#define SYS_MBOX_NULL		OSAL_INVALID_ID
#define SYS_SEM_NULL		OSAL_INVALID_ID

typedef OSAL_ID sys_thread_t;
typedef OSAL_ID sys_sem_t;
typedef OSAL_ID sys_mbox_t;
typedef OSAL_ID sys_prot_t;

#else  
struct sys_sem;
typedef struct sys_sem * sys_sem_t;

struct sys_mbox;
typedef struct sys_mbox *sys_mbox_t;

struct sys_thread;
typedef struct sys_thread * sys_thread_t;
#endif /* OS_TDS2_SYS_ARCH */


//Config structure for lwip stack.
typedef struct {
	BOOL	enable;
	UINT8	*start;	
	UINT32	length;	
} LWIP_MEMP_CFG;

typedef struct {
	BOOL	enable;
	UINT8	*start;	
	UINT32	length;	
} LWIP_MEM_CFG;

typedef struct {
	LWIP_MEMP_CFG	memp;	
	LWIP_MEM_CFG	mem;

} LWIP_CONFIG;


#endif /* __ARCH_SYS_ARCH_H__ */
