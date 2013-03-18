/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 - 2003 Copyright (C)
*
*    File:    osal_task.h
*
*    Description:    This file contains all functions declare
*		             of OSAL task management.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Jun.5.2003       Justin Wu       Ver 0.1    Create file.
*	2.  Nov.3.2003       Justin Wu       Ver 1.0    Support OSAL MIS V0.92.
*	3.	Dec.23.2003      Justin Wu       Ver 2.0    Support TDS2 - wDVD TDS.
*****************************************************************************/

#ifndef __OSAL_TASK_H__
#define __OSAL_TASK_H__

#include <sys_config.h>
#include "osal.h"


#if (SYS_OS_MODULE == ALI_TDS2)

/* Type define for task priority */
enum
{
	OSAL_PRI_LOW		= LOW_PRI,			/* Lowest,  for idle task */
	OSAL_PRI_NORMAL		= DEF_PRI,			/* Normal,  for user task */
	OSAL_PRI_HIGH		= 17,				/* High,    for system task */
	OSAL_PRI_CRITICL	= HSR_PRI,			/* Highest, for HSR scheduler */
};

#define	OSAL_TICK_SIZE		10

#define osal_task_get_current_id			OS_GetCurrentThreadID
#define osal_task_create					OS_CreateThread
#define osal_task_sleep						OS_TaskSleep
#define osal_task_exit						OS_Exit
#define osal_task_delete					OS_DeleteThread
#define osal_task_dispatch_off				OS_DisableDispatch
#define osal_task_dispatch_on				OS_EnableDispatch

#define OSAL_T_TASK_FUNC_PTR				FP
#define OSAL_T_CTSK							T_CTSK

#elif (SYS_OS_MODULE == LINUX_2_4_18)

/* Type define for task priority */
enum
{
	OSAL_PRI_LOW,							/* Lowest,  for idle task */
	OSAL_PRI_NORMAL,						/* Normal,  for user task */
	OSAL_PRI_HIGH,							/* High,    for system task */
	OSAL_PRI_CRITICL,						/* Highest, for HSR scheduler */
};

#define	OSAL_TICK_SIZE		0

#define osal_task_get_current_id(...)		OSAL_INVALID_ID
#define osal_task_create(...)				OSAL_INVALID_ID
#define osal_task_sleep(...)				OSAL_E_FAIL
#define osal_task_exit(...)					do{}while(0)
#define osal_task_delete(...)				do{}while(0)
#define osal_task_dispatch_off(...)			do{}while(0)
#define osal_task_dispatch_on(...)			do{}while(0)

#else

/* Type define for task priority */
enum
{
	OSAL_PRI_LOW,							/* Lowest,  for idle task */
	OSAL_PRI_NORMAL,						/* Normal,  for user task */
	OSAL_PRI_HIGH,							/* High,    for system task */
	OSAL_PRI_CRITICL,						/* Highest, for HSR scheduler */
};

#define	OSAL_TICK_SIZE		0

#define osal_task_get_current_id(...)		OSAL_INVALID_ID
#define osal_task_create(...)				OSAL_INVALID_ID
#define osal_task_sleep						osal_nos_task_sleep
#define osal_task_exit(...)					do{}while(0)
#define osal_task_delete(...)				do{}while(0)
#define osal_task_dispatch_off(...)			do{}while(0)
#define osal_task_dispatch_on(...)			do{}while(0)

#endif


#endif /* __OSAL_TASK_H__ */
