/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 - 2003 Copyright (C)
*
*    File:    osal_mm.h
*
*    Description:    This file contains all functions declare
*		             of OSAL memory management.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Jul.15.2003      Justin Wu       Ver 0.1    Create file.
*	2.  Nov.3.2003       Justin Wu       Ver 1.0    Support OSAL MIS V0.92.
*	3.	Dec.23.2003      Justin Wu       Ver 2.0    Support TDS2 - wDVD TDS.
*****************************************************************************/

#ifndef __OSAL_MM_H__
#define __OSAL_MM_H__

#include <sys_config.h>
#include "osal.h"
#include <api/libc/alloc.h>


#if (SYS_OS_MODULE == ALI_TDS2)

#define osal_memory_allocate			MALLOC
#define osal_memory_free				FREE

#define osal_mm_pa_to_va(pa)			((void *)((UINT32)(pa) | 0xA0000000))
#define osal_mm_va_to_pa(va)			((void *)((UINT32)(va) & 0x1fffffff))

#elif (SYS_OS_MODULE == LINUX_2_4_18)

#define osal_memory_allocate(...)		NULL
#define osal_memory_free(...)			OSAL_E_FAIL

#define osal_mm_pa_to_va(pa)			(pa)
#define osal_mm_va_to_pa(va)			(va)

#else

#define osal_memory_allocate			MALLOC
#define osal_memory_free				FREE

#define osal_mm_pa_to_va(pa)			(pa)
#define osal_mm_va_to_pa(va)			(va)

#endif

#endif /* __OSAL_MM_H__ */
