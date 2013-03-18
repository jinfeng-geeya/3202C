/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    This is the demo implenment of the base Porting APIs needed by iPanel MiddleWare. 
    Maybe you should modify it accorrding to Platform.
    
    Note: the "int" in the file is 32bits
    
    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/
#include <stdio.h>
#include <stdarg.h>
//#include <memory.h>
#include <time.h>
#include <string.h>
#include <sys_config.h>
#include <basic_types.h>
#include <osal/osal_timer.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <bus/sci/sci.h>


#include <udi/ipanel/ipanel_base.h>

//#define DEBUG_OUT  /*this is the log output key, if you #undef, no log output*/

static unsigned int  LEFT_MEMORY = IPANEL_MW_BUF_SIZE;
static unsigned int M_IPANEL_MEM = IPANEL_MW_BUF_START;
static unsigned int msec = 0;
static unsigned int base_msec = 0;
/******************************************************************/
/*Alloc memory function.                                          */
/*Input:  memory size                                             */
/*Output&return: memory address pointer if success, other NULL    */
/******************************************************************/
void *ipanel_porting_malloc(int memsize)
{
    void *pointer = NULL;

#if 0

    pointer = (void *)MALLOC(memsize);
    if (pointer)
    {
        ipanel_porting_dprintf("[ipanel_porting_malloc]malloc 0x%x bytes start from %p.\n", memsize, pointer);
        memset(pointer,0,memsize);
    }else
        ipanel_porting_dprintf("[ipanel_porting_malloc]malloc fail.\n");
    
#else

    if ((unsigned int)memsize > LEFT_MEMORY) 
    {
    	ipanel_porting_dprintf("[ipanel_porting_malloc] no space for malloc!!!\n");
    	return NULL;
    }    
    pointer = (void *)M_IPANEL_MEM;    
    M_IPANEL_MEM += memsize;
    LEFT_MEMORY -= memsize; 
    if (pointer)
        ipanel_porting_dprintf("[ipanel_porting_malloc]malloc 0x%x bytes start from %p.\n", memsize, pointer);
    else
        ipanel_porting_dprintf("[ipanel_porting_malloc]malloc fail.\n");
	
#endif   
    return pointer;
}

/******************************************************************/
/*Free memory function.                                           */
/*Input:  memory address pointer                                  */
/*Output&return: No                                               */
/******************************************************************/
void ipanel_porting_free(void* memptr)
{
    ipanel_porting_dprintf("[ipanel_porting_free]free 0x%x.\n",memptr);
#if 0    
    FREE(memptr);
#endif    
}

/******************************************************************/
/*log output, description and format is same as ANSI printf       */
/******************************************************************/
int ipanel_porting_dprintf(const char* fmt, ...)
  {
  
    int ret = 0;
#ifdef  DEBUG_OUT
    va_list args;
    va_start(args,fmt);
    ret = vprintf(fmt,args);
    va_end(args);
#endif
    return ret;
  
   }
/******************************************************************/
/*Description: iPanel Browser Runtime Base time.                  */
/*             and Unit is millisecond (1/1000 second).           */
/*Input      : No                                                 */
/*Output     : No                                                 */
/*Return     : counts of millisecond.                             */
/******************************************************************/
unsigned int ipanel_porting_time_ms(void)
{   
	msec = osal_get_tick();
    return msec;    
}


void ipanel_porting_sleep_ms(unsigned int ms)
{
	if (ms > 0)
    {
        osal_task_sleep(ms);
    }
}

typedef struct tagEisTime {
	UINT32 tm_sec;
	UINT32 tm_usec;
} EisTime;

int	ipanel_porting_get_utc_time(EisTime *tm)
{
	dprintf("[ipanel_porting_get_utc_time]called.\n");
	tm->tm_sec = 0x20000000 + (msec/1000);
	tm->tm_usec = 0;
	return 0;
}

int	ipanel_porting_set_utc_time(UINT16 year,UINT16 month,UINT16 day,UINT8 hour,UINT8 minute,UINT8 second)
{
	dprintf("[ipanel_porting_set_utc_time] called.\n");
	if(year>=1970 && (month>=1 && month<=12))
		return 0;
	else
		return -1;
}

int ipanel_base_init(void)
{
	LEFT_MEMORY = IPANEL_MW_BUF_SIZE;
	M_IPANEL_MEM = IPANEL_MW_BUF_START;
	return 0;	
}

/*
return semaphore handle if success;
return 0 if fail;
*/
UINT32 ipanel_porting_sem_create(const char *name, INT32 initialTokenCount, UINT32 taskWaitMode)
{
	OSAL_ID sema_id;

	dprintf("%s(), sema name=%s, count=%d, mode=%d,",__FUNCTION__, name,initialTokenCount,taskWaitMode);
	if(initialTokenCount < 0)
		return 0;
	
	sema_id = osal_semaphore_create(initialTokenCount);
	if (sema_id==OSAL_INVALID_ID)
	{
		dprintf("FAIL!\n");
		return 0;
	}
    	dprintf("SUCCESS, id=%d!\n",sema_id);
	return ((UINT32)sema_id);
}

/*
return 0 if success;
return -1 if fail;
*/
INT32 ipanel_porting_sem_destroy(UINT32 sem_handle)
{
	if (sem_handle == OSAL_INVALID_ID)
		return -1;
	if(OSAL_E_OK!=osal_semaphore_delete((OSAL_ID)sem_handle))
	{
		
		return -1;
	}
	return 0;
}

INT32 ipanel_porting_sem_wait(UINT32 sem_handle, UINT32 waitMode, INT32 milliSecsToWait)
{
	TMO time;
	INT32 ret = EIS_OS_QUEUE_SEM_STATUS_UNAVAILABLE;
		
	if(waitMode==EIS_QUEUE_SEM_WAIT)
	{
		if(milliSecsToWait==0)
			time = OSAL_WAIT_FOREVER_TIME;
		else
			time = milliSecsToWait;
		if(OSAL_E_OK==osal_semaphore_capture((OSAL_ID)sem_handle, time))
			return EIS_OS_QUEUE_SEM_STATUS_AVAILABLE;
	}
	else if(waitMode==EIS_QUEUE_SEM_NOWAIT)
	{
		if(OSAL_E_OK==osal_semaphore_capture((OSAL_ID)sem_handle, 0))
			return EIS_OS_QUEUE_SEM_STATUS_AVAILABLE;
	}
	return ret;
}

/*
return 0 if success;
return -1 if fail;
*/
INT32 ipanel_porting_sem_release(UINT32 sem_handle)
{
	INT32 ercd;

	if (sem_handle == OSAL_INVALID_ID)
		return -1;
	
	ercd = osal_semaphore_release(sem_handle);
	if(OSAL_E_OK == ercd)
		return  0 ;
	else
		return -1;

}


