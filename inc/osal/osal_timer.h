/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 - 2003 Copyright (C)
*
*    File:    osal_timer.h
*
*    Description:    This file contains all functions declare
*		             of OSAL timer.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Jun.5.2003       Justin Wu       Ver 0.1    Create file.
*	2.  Nov.4.2003       Justin Wu       Ver 1.0    Support OSAL MIS V0.92.
*	3.	Dec.23.2003      Justin Wu       Ver 2.0    Support TDS2 - wDVD TDS.
*****************************************************************************/

#ifndef __OSAL_TIMER_H__
#define __OSAL_TIMER_H__

#include <sys_config.h>
#include "osal.h"


#if (SYS_OS_MODULE == ALI_TDS2)

enum
{
	OSAL_TIMER_ALARM	= TIMER_ALARM,		/* Alarm, run only once */
	OSAL_TIMER_CYCLE	= TIMER_CYCLIC,		/* Cycle */
};

#define osal_timer_create					OS_CreateTimer
#define osal_timer_delete					OS_DeleteTimer
#define osal_timer_set						OS_SetTimer
#define osal_timer_activate					OS_ActivateTimer
#define osal_get_tick						OS_GetTickCount
#define osal_get_time						OS_GetTime
#define osal_delay							OS_Delay

#define OSAL_T_TIMER_FUNC_PTR				TMR_PROC
#define OSAL_T_CTIM							T_TIMER

#elif (SYS_OS_MODULE == LINUX_2_4_18)

enum
{
	OSAL_TIMER_ALARM,						/* Alarm, run only once */
	OSAL_TIMER_CYCLE,						/* Cycle */
};

#define osal_timer_create(...)				OSAL_INVALID_ID
#define osal_timer_delete(...)				OSAL_E_FAIL
#define osal_timer_set(...)					OSAL_E_FAIL
#define osal_timer_activate(...)			OSAL_E_FAIL
#define osal_get_tick(...)					0
#define osal_get_time(...)					0
#define osal_delay(...)						do{}while(0)

#else

enum
{
	OSAL_TIMER_ALARM,						/* Alarm, run only once */
	OSAL_TIMER_CYCLE,						/* Cycle */
};

#define osal_timer_create(...)				OSAL_INVALID_ID
#define osal_timer_delete(...)				OSAL_E_FAIL
#define osal_timer_set(...)					OSAL_E_FAIL
#define osal_timer_activate(...)			OSAL_E_FAIL
#define osal_get_tick(...)					0
#define osal_get_time(...)					0
#define osal_delay							osal_nos_delay

#endif


#endif /* __OSAL_TIMER_H__ */
