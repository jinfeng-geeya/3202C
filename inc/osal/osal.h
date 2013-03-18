/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 - 2003 Copyright (C)
*
*    File:    osal.h
*
*    Description:    This file contains all functions and structure declare
*		             of OSAL.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Nov.3.2003       Justin Wu       Ver 0.1    Create file.
*	2.	Dec.23.2003      Justin Wu       Ver 0.2    Support TDS2 - wDVD TDS.
*****************************************************************************/


#ifndef __OSAL_H__
#define __OSAL_H__

#include <sys_config.h>
#include <retcode.h>
#include <types.h>

#if	(SYS_OS_MODULE == ALI_TDS2)

	#include <os/tds2/itron.h>

	#define OSAL_E_OK					E_OK
	#define OSAL_E_FAIL					E_FAILURE
	#define OSAL_E_TIMEOUT				E_TIMEOUT

	#define OSAL_INVALID_ID				INVALID_ID
	#define OSAL_WAIT_FOREVER_TIME		0xFFFFFFFF

#elif (SYS_OS_MODULE == LINUX_2_4_18)

	#define OSAL_E_OK					SUCCESS
	#define OSAL_E_FAIL					ERR_FAILUE
	#define OSAL_E_TIMEOUT				ERR_TIME_OUT

	#define OSAL_INVALID_ID				0
	#define OSAL_WAIT_FOREVER_TIME		0xFFFFFFFF

#else

	#define OSAL_E_OK					SUCCESS
	#define OSAL_E_FAIL					ERR_FAILUE
	#define OSAL_E_TIMEOUT				ERR_TIME_OUT

	#define OSAL_INVALID_ID				0xFFFFFFFF
	#define OSAL_WAIT_FOREVER_TIME		0xFFFFFFFF

	#define INLINE inline

	typedef long               ER;
	typedef	long	INT;
	typedef unsigned short	WORD;
	typedef WORD               ID;

	/* System call Error Code */
	#define E_OK               0
	#define E_FAILURE		(-1)
	#define E_TIMEOUT	(-2)

struct os_config{

	UINT16 task_num;
	UINT16 sema_num;
	UINT16 flg_num;
	UINT16 mbf_num;
	UINT16 mutex_num;
	UINT16 parity_value;
};
#endif

typedef INT32  OSAL_ER;
typedef UINT32 OSAL_ID;

#include "osal_task.h"
#include "osal_int.h"
#include "osal_timer.h"
#include "osal_msgq.h"
#include "osal_sema.h"
#include "osal_flag.h"
#include "osal_mm.h"
#include "osal_cache.h"
#include "osal_mutex.h"

#ifdef __cplusplus
extern "C"
{
#endif


#ifdef __cplusplus
}
#endif

#endif /* __OSAL_H__ */
