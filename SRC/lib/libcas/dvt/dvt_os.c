/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *
 *  File: dvt_os.c
 *
 *  Description: This file fills the OS related API of DVT CA library.
 *
 *  History:
 *           Date            Author         Version       Comment
 *      ========    ======      ======     =========
 *  1.  2008.01.22      Grady Xu       0.1.000       Create this file
 *
 ****************************************************************************/
#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>

#include <api/libcas/dvt/DVTSTB_CASInf.h>
#include <api/libcas/dvt/cas_dvt.h>
#include "dvt_os.h"

HRESULT DVTSTBCA_Sleep( int dwMicroseconds )
{
	osal_task_sleep(dwMicroseconds);
	return CAS_SUCCESS;
}
HRESULT DVTSTBCA_SemaphoreInit( DVTCA_Semaphore * pSemaphore )
{
	OSAL_ID sema_id = OSAL_INVALID_ID;
	sema_id = osal_semaphore_create(1);
	if(sema_id == OSAL_INVALID_ID)
	{
		CAS_OSERR_PRINTF("Err: %s failed!\n", __FUNCTION__);
		return CAS_FAIL;
	}
	*pSemaphore = (DVTCA_Semaphore)sema_id;
	//intial status is no semaphore
	if (0 != osal_semaphore_capture(sema_id, OSAL_WAIT_FOREVER_TIME))
	{
		CAS_OSERR_PRINTF("ERR: %s- id = %d\n", __FUNCTION__, *pSemaphore);
		ASSERT(0);
	}
	CAS_OSERR_PRINTF("%s: id = %d\n", __FUNCTION__, *pSemaphore);
	return CAS_SUCCESS;
}
HRESULT DVTSTBCA_SemaphoreSignal( DVTCA_Semaphore * pSemaphore )
{
	ER ret; 
	ret = osal_semaphore_release((OSAL_ID)(*pSemaphore));
	
	if (E_OK != ret)
	{
		CAS_OSERR_PRINTF("ERR: %s- id = %d, ret = %d\n", __FUNCTION__, *pSemaphore, ret);
		SDBBP();
	}
	return CAS_SUCCESS;
}
HRESULT DVTSTBCA_SemaphoreWait( DVTCA_Semaphore * pSemaphore )
{
	ER ret;
	ret = osal_semaphore_capture((OSAL_ID)(*pSemaphore), OSAL_WAIT_FOREVER_TIME);
	if (E_OK != ret)
	{
		CAS_OSERR_PRINTF("ERR: %s- id = %d, ret = %d\n", __FUNCTION__, *pSemaphore, ret);
		SDBBP();
	}
	return CAS_SUCCESS;
}
HRESULT DVTSTBCA_RegisterTask(const char * szName, pThreadFunc pTaskFun)
{
	UINT32 thread_id;
	OSAL_T_CTSK t_ctsk;
	
	CAS_OSERR_PRINTF("%s: --%s\n", __FUNCTION__, szName);
	t_ctsk.task = (OSAL_T_TASK_FUNC_PTR)pTaskFun;	
	t_ctsk.itskpri = OSAL_PRI_NORMAL;	/*OSAL_PRI_NORMAL*/
	t_ctsk.stksz = CAS_STACK_SIZE;
	t_ctsk.quantum = 10;
	t_ctsk.name[0] = szName[0];
	t_ctsk.name[1] = szName[1];
	t_ctsk.name[2] = szName[2];	/*ALi only has 3 character*/
	t_ctsk.para1 = 0;
	t_ctsk.para2 = 0;	
	thread_id = osal_task_create(&t_ctsk);
	if (OSAL_INVALID_ID == thread_id)
	{
		CAS_OSERR_PRINTF("Err: %s create task failed!\n",__FUNCTION__);
		return CAS_FAIL;
	}
	return CAS_SUCCESS;
}

void DVTSTBCA_AddDebugMsg(const char * pszMsg)
{
	if (pszMsg)
		libc_printf("%s", pszMsg);
}

