/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 - 2003 Copyright (C)
*
*    File:    osal_flag.h
*
*    Description:    This file contains all functions declare
*		             of OSAL flag.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  Nov.4.2003       Justin Wu       Ver 1.0    Create file.
*													Support OSAL MIS V0.92.
*	2.	Dec.23.2003      Justin Wu       Ver 2.0    Support TDS2 - wDVD TDS.
*****************************************************************************/

#ifndef __OSAL_FLAG_H__
#define __OSAL_FLAG_H__

#include <sys_config.h>
#include "osal.h"


#if (SYS_OS_MODULE == ALI_TDS2)

enum
{
	OSAL_TWF_ANDW	= TWF_ANDW,	/* Release task until all specified bits are set */
	OSAL_TWF_ORW	= TWF_ORW,	/* Release task if one of specified bits are set */
	OSAL_TWF_CLR	= TWF_CLR,	/* Clear flag after the first waiting task released */
};

#define osal_flag_create					OS_CreateFlag
#define osal_flag_delete					OS_DelFlag
#define osal_flag_wait						OS_WaitFlagTimeOut
#define osal_flag_set						OS_SetFlag
#define osal_flag_clear						OS_ClearFlag

#elif (SYS_OS_MODULE == LINUX_2_4_18)

enum
{
	OSAL_TWF_ANDW,		/* Release task until all specified bits are set */
	OSAL_TWF_ORW,		/* Release task if one of specified bits are set */
	OSAL_TWF_CLR,		/* Clear flag after the first waiting task released */
};

#define osal_flag_create(...)				OSAL_INVALID_ID
#define osal_flag_delete(...)				OSAL_E_FAIL
#define osal_flag_wait(...)					OSAL_E_FAIL
#define osal_flag_set(...)					OSAL_E_FAIL
#define osal_flag_clear(...)				OSAL_E_FAIL

#else

enum
{
	OSAL_TWF_ANDW,		/* Release task until all specified bits are set */
	OSAL_TWF_ORW,		/* Release task if one of specified bits are set */
	OSAL_TWF_CLR,		/* Clear flag after the first waiting task released */
};

#define osal_flag_create(...)				OSAL_INVALID_ID
#define osal_flag_delete(...)				OSAL_E_FAIL
#define osal_flag_wait(...)					OSAL_E_FAIL
#define osal_flag_set(...)					OSAL_E_FAIL
#define osal_flag_clear(...)				OSAL_E_FAIL

#endif


#endif /* __OSAL_FLAG_H__ */
