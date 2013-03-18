/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2006 Copyright (C)
 *
 *  File: tf_os.c
 *
 *  Description: This file fills the OS related API of Tongfang CA library.
 *
 *  History:
 *      Date            Author            Version   Comment
 *      ====        ======      =======  =======
 *  1. 2006.3.19  Gushun Chen     0.1.000    Initial
 *  2. 2007.7.25  Grady Xu		 0.2.000	 re-org the Tongfang CA lib.
 ****************************************************************************/

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <api/libchunk/chunk.h>
#include <hld/dmx/dmx.h>

//#include <api/libcas/cdca/pub_st.h>
#include <api/libcas/cdca/CDCAS30.h>

#include "cd_os.h"

extern void CDSTBCA_Sleep(CDCA_U16 wMilliSeconds)
{
	osal_task_sleep(wMilliSeconds);
}

extern void* CDSTBCA_Malloc(CDCA_U32 byBufSize)
{
	void *p = NULL;
	
	p = MALLOC(byBufSize);
	if(p == NULL)
	{
		CAS_OSERR_PRINTF("OS_Malloc : failed!\n");
		return NULL;
	}
	CAS_OS_PRINTF("@@@@MALLOC size %d, addr:0x%x\n", byBufSize, p);
	return p;
}

extern void  CDSTBCA_Free(void* pBuf)
{
	CAS_OS_PRINTF("@@@@FREE addr:0x%x\n", pBuf);
	if(pBuf== NULL)
	{
		CAS_OSERR_PRINTF("OS_FREE : NULL pointer!\n");
		return;
	}
	FREE(pBuf);
}


extern void  CDSTBCA_Memset(void* pDestBuf, CDCA_U8 c, CDCA_U32 wSize)
{
	UINT16 i;
	//TFCAS_OS_PRINTF("@@@@MEMSET dest:0x%x, c:%d, size:%d\n", pDestBuf, c, wSize);
	/*for(i=0; i<wSize; i++)
	{
		((UINT8 *)pDestBuf)[i] = c;
	}*/
	MEMSET(pDestBuf, c, wSize);
	//return;
}

extern void  CDSTBCA_Memcpy( void* pDestBuf, const void* pSrcBuf, CDCA_U32 wSize )
{
	//TFCAS_OS_PRINTF("@@@@MEMCPY dest:0x%x, src:0x%x, size:%d\n", pDestBuf, pSrcBuf, wSize);
	MEMCPY(pDestBuf, pSrcBuf, wSize);
}

extern CDCA_U16 CDSTBCA_Strlen( char* pString)
{
	return STRLEN(pString);
}


extern void CDSTBCA_SemaphoreInit( CDCA_Semaphore* pSemaphore, CDCA_BOOL bInitVal)
{
#ifdef OSALSEMA_FOR_TFSEMA
	OSAL_ID sema_id = OSAL_INVALID_ID;
//	TFCAS_OS_PRINTF("TFSTBCA_SemaphoreInit:%02x\n", bInitVal);
	sema_id = osal_semaphore_create(bInitVal?1:0);
	if(sema_id == OSAL_INVALID_ID)
	{
		CAS_OSERR_PRINTF("TFSTBCA_SemaphoreInit: Can not create sema!!!\n");
		return;
	}
	*pSemaphore = (CDCA_Semaphore) sema_id;
#else
	OSAL_ID sema_id = OSAL_INVALID_ID;
//	TFCAS_OS_PRINTF("TFSTBCA_SemaphoreInit:%02x\n", bInitVal);
	sema_id = osal_mutex_create();
	if(sema_id == OSAL_INVALID_ID)
	{
		CAS_OSERR_PRINTF("CDSTBCA_SemaphoreInit: Can not create sema!!!\n");
		ASSERT(0);
	}
	*pSemaphore = (CDCA_Semaphore) sema_id;
	if(!bInitVal)
		osal_mutex_lock((OSAL_ID)(*pSemaphore), OSAL_WAIT_FOREVER_TIME);
#endif
	return;
}

extern void CDSTBCA_SemaphoreSignal( CDCA_Semaphore* pSemaphore )
{
#ifdef OSALSEMA_FOR_TFSEMA
	osal_semaphore_release((OSAL_ID)(*pSemaphore));
#else
	osal_mutex_unlock((OSAL_ID)(*pSemaphore));
#endif
}

extern void CDSTBCA_SemaphoreWait( CDCA_Semaphore* pSemaphore )
{
#ifdef OSALSEMA_FOR_TFSEMA
	osal_semaphore_capture((OSAL_ID)(*pSemaphore), OSAL_WAIT_FOREVER_TIME);
#else
	osal_mutex_lock((OSAL_ID)(*pSemaphore), OSAL_WAIT_FOREVER_TIME);
#endif
}


extern CDCA_BOOL CDSTBCA_RegisterTask( const char* szName, CDCA_U8 byPriority,  void* pTaskFun,  void* pParam, CDCA_U16 wStackSize)
{
	UINT32 thread_id;
	OSAL_T_CTSK t_ctsk;

	t_ctsk.task = (OSAL_T_TASK_FUNC_PTR)pTaskFun;	
	t_ctsk.itskpri = byPriority;	/*OSAL_PRI_NORMAL*/
	t_ctsk.stksz = wStackSize;
	t_ctsk.quantum = 10;
	t_ctsk.name[0] = szName[0];
	t_ctsk.name[1] = szName[1];
	t_ctsk.name[2] = szName[2];	/*ALi only has 3 character*/
	t_ctsk.para1 = (UINT32) pParam;
	t_ctsk.para2 = 0;	
	thread_id = osal_task_create(&t_ctsk);
	if (OSAL_INVALID_ID == thread_id)
	{
		CAS_OSERR_PRINTF("STBCA_RegisterTask: Can not create task!!!\n");
		return CDCA_FALSE;
	}
	return CDCA_TRUE;
}

#define STBCA_DEBUG_LEVEL	0x01

extern void CDSTBCA_Printf(CDCA_U8 byLevel,  const char* szMesssage)
{

	if (MEMCMP(szMesssage, "MessageManger task running!", 13) == 0)
		return;
	if(byLevel<STBCA_DEBUG_LEVEL)
		CAS_OS_PRINTF(szMesssage);

	return;
	
}

