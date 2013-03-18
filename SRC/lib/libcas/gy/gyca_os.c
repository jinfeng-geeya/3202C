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

#include <api/libcas/gy/cas_gy.h>
#include <api/libcas/gy/gyca_pub.h>
#include <api/libcas/gy/gyca_osp.h>
#include "gyca_os.h"



/*** 生成一个随机数的函数接口*/
/**********************************************************************************************
** 函数名称：    GYCA_GetRandomNumber
***********************************************************************************************
**     提供者：           机顶盒
**     使用者：           CA模块
**     何时使用：         CA模块内部需要时使用
**     功能描述：         获取一个不大于限定数的随机数
**     输入参数：         limit   最大限定的数
**     输出参数：         无
**     返回值：           随机数
**     其他说明：         无
***********************************************************************************************/
GYU32 GYCA_GetRandomNumber(GYU32 limit)
{
	GYCASDRV_PRINTF("GYCA_GetRandomNumber---ENTER\n");
	return RAND(limit);
}

/*** 打印测试函数接口*/
/**********************************************************************************************
** 函数名称：    GYCA_DBG_Print
***********************************************************************************************
**     提供者：      机顶盒
**     使用者：      CA模块
**     何时使用： CA模块内部需要时调试测试时使用
**     功能描述： 通过串口打印格式化调试字符串
**     输入参数： fmt     c语言格式化字符串
**     输出参数： 无
**     返回值：      实际打印的字符数量.(字节数)
**     其他说明： 无
***********************************************************************************************/


//static volatile char test_num = 0;
GYU32 GYCA_DBG_Print (const char *fmt, ...)
{
	//test_num++;
	//osal_task_sleep(1);
#if 1
	if(gy_switch_debug_get())
	{
		va_list arg;
		va_start(arg, fmt);
		vprintf(fmt, arg);
		va_end(arg);
	}
#endif
	return;
}

/*********************************************************************************************
** 	函数名称:GYCA_OSPTaskCreate
**********************************************************************************************
**	提 供 者： 	机顶盒。
**	使 用 者：	 CA模块。
**	功能描述：	创建任务。
**	输入参数：	name[]，	4个字节的任务名称。
**	 				stackSize，	任务所使用的堆栈的大小。
**	 				entryPoint，任务的入口地址。
**	 				priority，	任务的优先级。
**	 				arg1，		传递给任务的第一个参数。
**	 				taskId，	任务的ID。	
**	输出参数：	无。
**	返 回 值：	SUCCESS：   成功；
**	 				GYFAILURE?   发生错误
**	其他说明：
**********************************************************************************************/
GYS32 GYCA_OSPTaskCreate
(
	char name[],
	GYU32 stackSize,
	void (*entryPoint)(void),
	GYS32 priority, 
	GYU32 arg1,
	GYU32 arg2,
	GYU32 * taskId
)
{
	gycas_info_priv_t *priv;
	OSAL_T_CTSK gy_task;
	OSAL_ID task = OSAL_INVALID_ID;
	GYU8 i =0;
	
	GYCASDRV_PRINTF("GYCA_OSPTaskCreate---ENTER--%s--%x\n",name,stackSize);

	priv = gycas_get_priv();
	
	for (i = 0; i < GYCAS_MAX_TASK_NUMBER; i++) 
	{
		if (priv->TaskTable[i] == 0) 
		{
			//priv->TaskTable[i] = *taskId;
			break;
		}
	}
	if (i >= GYCAS_MAX_TASK_NUMBER) 
	{
//		GYCAS_ERR("[Cas_gy_drv.c] GYCA_OSPTaskCreate: Fail! i = %d\n", i);
		return GYFAILURE;
	}

	gy_task.task = (OSAL_T_TASK_FUNC_PTR)entryPoint;
	gy_task.itskpri = OSAL_PRI_NORMAL;
	gy_task.stksz = stackSize;
	gy_task.quantum = 10;
	gy_task.name[0] = 'S';
	gy_task.name[1] = 'K';
	gy_task.name[2] = i+'0';
	gy_task.para1 = arg1;
	gy_task.para2 = arg2;	
	
	task = osal_task_create(&gy_task);
	if (OSAL_INVALID_ID == task)
	{
		GYCASDRV_PRINTF("[Cas_gy_drv.c] GYCA_OSPTaskCreate: Fail! i = %d\n", i);
		ASSERT(0);
		return GYFAILURE;
	}
	priv->TaskTable[i] = task;
	*taskId = task;
	return GYSUCCESS;
}

/*********************************************************************************************
** 	函数名称:GYCA_OSPTaskTemporarySleep
**********************************************************************************************
**	提 供 者： 	机顶盒。
**	使 用 者：	 CA模块。
**	功能描述：	暂停任务一段指定的时间。
**	输入参数：	milliSecsToWait，	终止任务的时间。（以毫秒为单位。）
**	输出参数：	无。
**	返 回 值：	SUCCESS：       	成功；
**	 				FAILURE：       	发生错误
**	其他说明：
**********************************************************************************************/
GYS32 GYCA_OSPTaskTemporarySleep(GYU32 milliSecsToWait)
{

	GYCASDRV_PRINTF("GYCA_OSPTaskTemporarySleep---ENTER milliSecsToWait[%d]\n",milliSecsToWait);
	osal_task_sleep(milliSecsToWait);
	return GYSUCCESS;
}

/*********************************************************************************************
** 	函数名称:GYCA_OSPSemaphoreCreate
**********************************************************************************************
**	提 供 者： 	机顶盒。
**	使 用 者：	 CA模块。
**	功能描述：	创建信号量。
**	输入参数：	name[],				4个字节的信号量名称。
**	 				initialTokenCount	初始化时信号量的数量。
**	 				taskWaitMode，		取得该信号量的方式。为以下两种之一:
**	 				GYCA_WAIT_FIFO， 按先等待者先得的方式取得。
** 					GYCA_WAIT_PRIO， 按优先级高者先得的方式取得。
**	 				semaphoreId，		该信号量的ID
**	输出参数：	无。
**	返 回 值：	SUCCESS：       	成功；
**	 				FAILURE：       	发生错误
**	其他说明：目前CA模块只用到了GYCA_WAIT_FIFO模式，如果无法实现
**	 				GYCA_WAIT_PRIO则可以暂时不实现。
**********************************************************************************************/
GYS32 GYCA_OSPSemaphoreCreate
(
	char name[4], 
	GYU32 initialTokenCount,
	GYU32 taskWaitMode,
	GYU32 *semaphoreId
)
{

	UINT32 i;
	gycas_info_priv_t *priv;

	GYCASDRV_PRINTF("GYCA_OSPSemaphoreCreate---ENTER--%d\n",initialTokenCount);
	OSAL_ID sema_id = OSAL_INVALID_ID;
	sema_id = osal_semaphore_create(initialTokenCount);
	while(sema_id == OSAL_INVALID_ID)
	{
		osal_task_sleep(10);
		GYCASDRV_PRINTF("[Cas_gy_drv.c] GYCA_OSPSemaphoreCreate: Fail\n");
		return GYFAILURE;

	}
	*semaphoreId = (GYU32)sema_id;
	GYCASDRV_PRINTF("%s: id = %d\n", __FUNCTION__, *semaphoreId);
	return GYSUCCESS;
}

/*********************************************************************************************
** 	函数名称:GYCAS_GYCA_OSPSemaphoreGetToken
**********************************************************************************************
**	提 供 者： 	机顶盒。
**	使 用 者：	 CA模块。
**	何时使用：	创建任务。
**	功能描述：	取得信号量。
**	输入参数：	semaphoreId,	等待信号量的方式，为以下两种之一:
**	 				GYCA_TASK_WAIT，等待信号量直到超时。
** 					GYCA_TASK_NOWAIT，不管信号量存在与否立即返回。
**	 				milliSecsToWait,等待的时间。如果waitMode为GYCA_TASK_NOWAIT，则该参数无效。
**					如果为GYCA_TASK_WAIT，则代表时间。
**	输出参数：	无。
**	返 回 值：	SUCCESS：       成功；
**	 				FAILURE：       发生错误
**	其他说明：注意区别这里的等待模式和创建信号量的等待模式之间的区别。
**					创建信号量的是指同时有多个任务在等待同一个信号量时谁优先取
**  					得信号量。到取得信号量为止还是不做任何的等待，不管是否可以
**					取得信号量，立即返回。
**********************************************************************************************/
GYS32 GYCA_OSPSemaphoreGetToken (	
	GYU32 semaphoreId,
 	GYU32 waitMode,
	GYU32 milliSecsToWait
)
{
	INT32 sema_result = 0;
	
	GYCASDRV_PRINTF("GYCA_OSPSemaphoreGetToken---ENTER waitMode=%d semaphoreId[%x]\n",waitMode,semaphoreId);
	if(waitMode== GYCA_TASK_WAIT)
		sema_result = osal_semaphore_capture((OSAL_ID)(semaphoreId), OSAL_WAIT_FOREVER_TIME);
	else if(waitMode== GYCA_TASK_NOWAIT)
		sema_result = osal_semaphore_capture((OSAL_ID)(semaphoreId), 1);

	if(sema_result == OSAL_E_OK)
		return GYSUCCESS;
	else
	{
		GYCASDRV_PRINTF("GYCA_OSPSemaphoreGetToken--FAIL\n");
		return GYFAILURE;
	}
}

/*********************************************************************************************
** 	函数名称:GYCA_OSPSemaphoreReturnToken
**********************************************************************************************
**	提 供 者： 	机顶盒。
**	使 用 者：	 CA模块。
**	功能描述：	增加一个信号量
**	输入参数：	semaphoreId,	该信号量的ID
**	输出参数：	无。
**	返 回 值：	SUCCESS：       成功；
**	 				FAILURE：       发生错误
**	其他说明：              
**********************************************************************************************/
GYS32 GYCA_OSPSemaphoreReturnToken(	GYU32 semaphoreId )
{
	INT32 sema_result = 0;

	sema_result = osal_semaphore_release((OSAL_ID)(semaphoreId));
	if(sema_result == OSAL_E_OK)
		return GYSUCCESS;
	else
	{
		GYCASDRV_PRINTF("GYCA_OSPSemaphoreReturnToken---FAIL\n");
		return GYFAILURE;
	}
	return GYSUCCESS;
}

/*********************************************************************************************
** 	函数名称:GYCA_OSPMalloc
**********************************************************************************************
**	提 供 者： 	机顶盒。
**	使 用 者：	 CA模块。
**	功能描述：	分配内存。
**	输入参数：	numberOfBytes，	分配的内存空间的大小（以字节为单位）。
**	输出参数：	无。
**	返 回 值：	如果为NULL，则表示分配不到内存,其他的均为成功分配。
**	其他说明：              
**********************************************************************************************/
void * GYCA_OSPMalloc(GYU32 numberOfBytes)
{
	void *ptr = NULL;
	gycas_buf_link_t *buflink = NULL;
	gycas_info_priv_t *priv;

	
	GYCASDRV_PRINTF("GYCA_OSPMalloc---ENTER\n");

	ptr = MALLOC(numberOfBytes);
#if 1	
	if (ptr== NULL) 
	{
		ASSERT(0);
	} 
	else
	{
		priv = gycas_get_priv();
		buflink = MALLOC(sizeof(gycas_buf_link_t));
		if (buflink == NULL) 
		{
			ASSERT(0);
		}
		buflink->ptr = ptr;
		osal_mutex_lock(priv->mutex,OSAL_WAIT_FOREVER_TIME);
		list_add_tail(&buflink->list, &priv->buf_list);
		osal_mutex_unlock(priv->mutex);
	}
#endif
	return ptr;
}

/*********************************************************************************************
** 	函数名称:GYCA_OSPFree
**********************************************************************************************
**	提 供 者： 	机顶盒。
**	使 用 者：	 CA模块。
**	功能描述：	释放掉分配的内存空间。
**	输入参数：	ptrToSegment,	要释放的内存空间指针。
**	输出参数：	无。
**	返 回 值：	SUCCESS：       成功释放；
**	 				FAILURE：       释放不成功，可能是因为所传递进来的指针无效。
**	其他说明：              
**********************************************************************************************/
GYS32 GYCA_OSPFree(void *ptrToSegment)
{
	gycas_buf_link_t *buflink;
	gycas_info_priv_t *priv;
	struct list_head *ptr, *temp;
	
	GYCASDRV_PRINTF("GYCA_OSPFree---ENTER\n");

	if (ptrToSegment != NULL) 
	{
	#if 1
		priv = gycas_get_priv();
		osal_mutex_lock(priv->mutex,OSAL_WAIT_FOREVER_TIME);
		list_for_each_safe(ptr, temp, &priv->buf_list) 
		{
			buflink = list_entry(ptr, gycas_buf_link_t, list);
			if (buflink->ptr == ptrToSegment)
			{
				list_del(&buflink->list);
				FREE(buflink);
				buflink = NULL;
				break;
			}
		}
		osal_mutex_unlock(priv->mutex);
		FREE(ptrToSegment);
		ptrToSegment = NULL;
	#endif
	}
	return GYSUCCESS;
}

/*********************************************************************************************
** 	函数名称:GYCA_OSPQueueCreate
**********************************************************************************************
**	提 供 者： 	机顶盒。
**	使 用 者：	 CA模块。
**	功能描述：	创建一个消息队列。
**	输入参数：	name[],	4个字节的队列名称。
**	 				MaxQueueLength，消息队列中可以存放的消息的数量。当消息队列中该
**                                     	数量达到该数量时，再往该消息队列发消息将会失败。
**	 				taskWaitMode，	取得消息队列中消息的方式。为以下两种之一:
**	 					GYCA_WAIT_FIFO， 按先等待者先得的方式取得。
** 						GYCA_WAIT_PRIO， 按优先级高者先得的方式取得。
**	 				queueId，消息队列的ID。
**	输出参数：	无。
**	返 回 值：	SUCCESS：       	成功；
**	 				FAILURE：       	发生错误
**	其他说明：              
**********************************************************************************************/
GYS32 GYCA_OSPQueueCreate
(
	char name[],
	GYU32 maxQueueLength,
	GYU32 taskWaitMode,
	GYU32 *queueId
)
{
	OSAL_T_CMBF msg_q;
	OSAL_ID msq = OSAL_INVALID_ID;
	gycas_info_priv_t *priv;
	GYU8 i = 0;

	priv = gycas_get_priv();
	
	for(i=0;i<GYCAS_MAX_MSGQ_NUMBER;i++)
	{
		if(priv->msgq_table[i] == 0)
		{
			//priv->msgq_table[i] = msg_q;
			break;
		}
	}

	msg_q.bufsz = maxQueueLength*sizeof(GYCA_QUEUE_MESSAGE);
	msg_q.maxmsz = sizeof(GYCA_QUEUE_MESSAGE);
	msg_q.name[0] = 'S';
	msg_q.name[1] = 'Q';
	msg_q.name[2] = i+'0';
	msq = osal_msgqueue_create(&msg_q);
	if (OSAL_INVALID_ID == msq)
	{
		GYCASDRV_PRINTF("GYCA_OSPQueueCreate---FAIL\n");
		ASSERT(0);
		return GYFAILURE;
	}
	priv->msgq_table[i] = msq;
	*queueId = msq;
	GYCASDRV_PRINTF("GYCA_OSPQueueCreate- SUCCESS queueId=%x\n",msq);
	return GYSUCCESS;
}

/*********************************************************************************************
** 	函数名称:GYCA_OSPQueueGetMessage
**********************************************************************************************
**	提 供 者： 	机顶盒。
**	使 用 者：	 CA模块。
**	功能描述：	从消息队列获取消息。
**	输入参数：	queueId，	所要取得的消息的ID。
**	 				waitMode，	等待消息的方式，为以下两种之一:
**	 					GYCA_TASK_WAIT，等待消息直到超时。
** 						GYCA_TASK_NOWAIT，不管消息存在与否立即返回。
**	 				milliSecsToWait，	等待的时间。如果waitMode为GYCA_TASK_NOWAIT，则该
**		 				参数无效。如果为GYCA_TASK_WAIT，则代表时间。
**	输出参数：	messsage，	为消息的格式。参照QUEUE_MESSAGE。
**	返 回 值：	SUCCESS： 成功；
**	 				FAILURE：   发生错误
**	其他说明：              
**********************************************************************************************/
GYS32 GYCA_OSPQueueGetMessage
(
	GYU32 queueId,
	GYCA_QUEUE_MESSAGE *message ,
	GYU32 waitMode,
	GYU32 milliSecsToWait
)
{
	UINT32 wait_time;
	INT32 result;
	UINT32 msg_size=0;
	
	if(waitMode == GYCA_TASK_WAIT)
	{
		if(milliSecsToWait == 0)
			wait_time = OSAL_WAIT_FOREVER_TIME;
		else
			wait_time = milliSecsToWait;
	}
	else
		wait_time = 1;
	msg_size = sizeof(GYCA_QUEUE_MESSAGE) ;
	result = osal_msgqueue_receive((void *)message, (UINT32 *)(&msg_size), (OSAL_ID)queueId, wait_time);

	if(result == OSAL_E_OK)
	{
		GYCASDRV_PRINTF("GYCA_OSPQueueGetMessage-->SUCCESS;\n");
		return GYSUCCESS;
	}
	else
	{
		GYCASDRV_PRINTF("GYCA_OSPQueueGetMessage---FAIL--queueId[%x] waitMode[%d] milliSecsToWait[%x]\n",queueId,waitMode,milliSecsToWait);
		return GYFAILURE;
	}
}

/*********************************************************************************************
** 	函数名称: GYCA_OSPQueueSendMessage
**********************************************************************************************
**	提 供 者： 	机顶盒。
**	使 用 者：	 CA模块。
**	功能描述：	发送消息到消息队列。
**	输入参数：	queueId，消息队列ID。
**	 				message，要发送的消息。其格式见QUEUE_MESSAGE结构。
**	输出参数：	无。
**	返 回 值：	SUCCESS：       	成功；
**	 				FAILURE：       	发生错误
**	其他说明：              
**********************************************************************************************/
GYS32  GYCA_OSPQueueSendMessage(GYU32 queueId,GYCA_QUEUE_MESSAGE * message)
{
	INT32 result;

	result = osal_msgqueue_send((OSAL_ID)queueId, message, sizeof(GYCA_QUEUE_MESSAGE), 0);
	if(result == OSAL_E_OK)
	{
		GYCASDRV_PRINTF("GYCA_OSPQueueSendMessage--SUCCESS\n");
		return GYSUCCESS;
	}
	else
	{
		GYCASDRV_PRINTF("GYCA_OSPQueueSendMessage---FAIL  queueId[%x]\n",queueId);
		return GYFAILURE;
	}
}

void GYCA_GetStbSn(GYU8 * sn, GYS16 *len)
{
	GYU8 i;
	GYCASDRV_PRINTF("%s,+++++++++++++++++++++++++++\n",__FUNCTION__);
	gycas_info_priv_t* priv = NULL;
	priv= gycas_get_priv();
	*len = 0;
	for(i = 0; i<20; i++)
	{
		if(priv->stb_id[i] != '\0')
		{
			sn[*len] = priv->stb_id[i];
			*len++;
		}
		else
			break;
	}
	//GYCASDRV_PRINTF("%s,  STB SN: %s \n",__FUNCTION__, sn);
}

/* 消隐当前会话框 */
/**********************************************************************************************
** 函数名称：   GYCA_HideIppvDlg
***********************************************************************************************
**     提供者：           机顶盒。
**     使用者：           CAM模块。
**     何时使用：      CAM需要消隐当前会话框的时候
**     功能描述：      消隐当前会话框
**     输入参数：      wEcmPid        当前节目的ECM PID。
**     输出参数：     无。                                                
**     返回值：           无。                                  
**     其他说明：     无
***********************************************************************************************/
extern GYU8 ap_cas_ipp_popup_proc(GYU8 open_close);
void GYCA_HideIppvDlg(T_CAServiceInfo *serviceInfo)
{
	ap_cas_ipp_popup_proc(0);
	return ;
}

