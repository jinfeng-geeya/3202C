/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2002 - 2003 Copyright (C)
*
*    File:    osal_int.h
*
*    Description:    This file contains all functions declare
*		             of OSAL interrupt management.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Jun.5.2003       Justin Wu       Ver 0.1    Create file.
*	2.  Nov.4.2003       Justin Wu       Ver 1.0    Support OSAL MIS V0.92.
*	3.	Dec.23.2003      Justin Wu       Ver 2.0    Support TDS2 - wDVD TDS.
*****************************************************************************/

#ifndef __OSAL_INT_H__
#define __OSAL_INT_H__

#include <sys_config.h>
#include "osal.h"


#if (SYS_OS_MODULE == ALI_TDS2)

#define osal_interrupt_register_lsr			OS_RegisterISR
#define osal_interrupt_unregister_lsr		OS_DeleteISR
#define osal_interrupt_register_hsr			OS_RegisterHSR
#define osal_interrupt_disable				OS_DisableInterrupt
#define osal_interrupt_enable				OS_EnableInterrupt

#elif (SYS_OS_MODULE == LINUX_2_4_18)

#define osal_interrupt_register_lsr(...)	OSAL_E_FAIL
#define osal_interrupt_unregister_lsr(...)	OSAL_E_FAIL
#define osal_interrupt_register_hsr(...)	OSAL_E_FAIL
#define osal_interrupt_disable(...)			do{}while(0)
#define osal_interrupt_enable(...)			do{}while(0)

#else

#define osal_interrupt_register_lsr			osal_nos_register_interrupt
#define osal_interrupt_unregister_lsr		osal_nos_remove_interrupt
#define osal_interrupt_register_hsr			osal_nos_interrupt_register_hsr
#define osal_interrupt_disable				osal_nos_disable_interrupt
#define osal_interrupt_enable				osal_nos_enable_interrupt

#endif


typedef void (*OSAL_T_LSR_PROC_FUNC_PTR)(UINT32);
typedef void (*OSAL_T_HSR_PROC_FUNC_PTR)(UINT32);


#endif /* __OSAL_INT_H__ */
