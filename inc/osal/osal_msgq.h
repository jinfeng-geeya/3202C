/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 - 2003 Copyright (C)
*
*    File:    osal_msgq.h
*
*    Description:    This file contains all functions declare
*		             of OSAL message queue.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.  Nov.4.2003       Justin Wu       Ver 1.0    Create file.
*													Support OSAL MIS V0.92.
*	2.	Dec.23.2003      Justin Wu       Ver 2.0    Support TDS2 - wDVD TDS.
*****************************************************************************/

#ifndef __OSAL_MSGQ_H__
#define __OSAL_MSGQ_H__

#include <sys_config.h>
#include "osal.h"


#if (SYS_OS_MODULE == ALI_TDS2)

#define osal_msgqueue_create				OS_CreateMsgBuffer
#define osal_msgqueue_delete				OS_DelMessageBuffer
#define osal_msgqueue_send					OS_SendMsgTimeOut
#define osal_msgqueue_receive				OS_GetMsgTimeOut

#define OSAL_T_CMBF							T_CMBF

#elif (SYS_OS_MODULE == LINUX_2_4_18)

#define osal_msgqueue_create(...)			OSAL_INVALID_ID
#define osal_msgqueue_delete(...)			OSAL_E_FAIL
#define osal_msgqueue_send(...)				OSAL_E_FAIL
#define osal_msgqueue_receive(...)			OSAL_E_FAIL

#else

#define osal_msgqueue_create				osal_nos_msgqueue_create
#define osal_msgqueue_delete				osal_nos_msgqueue_delete
#define osal_msgqueue_send					osal_nos_msgqueue_send
#define osal_msgqueue_receive				osal_nos_msgqueue_receive

#endif


#endif /* __OSAL_MSGQ_H__ */
