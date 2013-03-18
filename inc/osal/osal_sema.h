/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 - 2003 Copyright (C)
*
*    File:    osal_sema.h
*
*    Description:    This file contains all functions declare
*		             of OSAL semaphore.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  Nov.4.2003       Justin Wu       Ver 1.0    Create file.
*													Support OSAL MIS V0.92.
*	2.	Dec.23.2003      Justin Wu       Ver 2.0    Support TDS2 - wDVD TDS.
*****************************************************************************/

#ifndef __OSAL_SEMA_H__
#define __OSAL_SEMA_H__

#include <sys_config.h>
#include "osal.h"


#if (SYS_OS_MODULE == ALI_TDS2)

#define osal_semaphore_create				OS_CreateSemaphore
#define osal_semaphore_delete				OS_DelSemaphore
#define osal_semaphore_capture				OS_AcquireSemaphoreTimeOut
#define osal_semaphore_release				OS_FreeSemaphore

#elif (SYS_OS_MODULE == LINUX_2_4_18)

#define osal_semaphore_create(...)			OSAL_INVALID_ID
#define osal_semaphore_delete(...)			OSAL_E_FAIL
#define osal_semaphore_capture(...)			OSAL_E_FAIL
#define osal_semaphore_release(...)			OSAL_E_FAIL

#else

#define osal_semaphore_create(...)			OSAL_INVALID_ID
#define osal_semaphore_delete(...)			OSAL_E_FAIL
#define osal_semaphore_capture(...)			OSAL_E_FAIL
#define osal_semaphore_release(...)			OSAL_E_FAIL

#endif


#endif /* __OSAL_SEMA_H__ */
