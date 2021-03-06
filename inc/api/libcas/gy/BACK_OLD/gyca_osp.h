/*******************************************************************************
** Notice: Copyright (c) 2008 Geeya corporation - All Rights Reserved 
********************************************************************************
** Description:
**                 the head file of STB RTOS porting layer.
**
** File Rev     $Revision: \main\20060819_1.0_integration_branch\1 $
**
** Date         $Date:     Saturday Augest 19 17:54:04 2006 $
** 
** File Name: gycas_osp.h
**
********************************************************************************/
#ifndef _GYCAS_OSP_H_
#define _GYCAS_OSP_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include "gyca_pub.h"

/*****************************************************************************
** These macros are used by the application in conjunction with queue, and 
** semaphore system calls to determine whether or not the task will block,
** when a requested resource is not available.
** TASK_NOWAIT - Do not wait for the resource.
** TASK_WAIT - Wait for the resource.
*****************************************************************************/
/*
**  队列消息结构
*/
typedef struct 
{
	GYU32 q1stWordOfMsg;                /* First word of a queue message.  */
	GYU32 q2ndWordOfMsg;                /* Second word of a queue message. */
	GYU32 q3rdWordOfMsg;                /* Third word of a queue message.  */
	GYU32 q4thWordOfMsg;                /* Fourth word of a queue message. */
}GYCA_QUEUE_MESSAGE;


/*
** 信号量对象及消息队列对象等待所用的宏定义
*/
#define GYCA_WAIT_FIFO		(0)	/* 表示取得消息/信号量的顺序是先等待的先得
								*/
#define GYCA_WAIT_PRIO		(1)	/* 表示取得消息/信号量的顺序是根据优先级来的
								*/	
#define GYCA_TASK_NOWAIT	(2)	/* 表示如果当前消息队列中没有消息或者当前信号量
								** 为off状态等待的程序立即返回
								*/
#define GYCA_TASK_WAIT		(3)	/* 表示如果当前消息队列中没有消息或者当前信号量
								** 为off状态等待的程序不立即返回，而是挂起，知道
								** 等待到相应的对象，或者等待的时间超时
								*/

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
**	 				FAILURE�    发生错误
**	其他说明：
**********************************************************************************************/
extern GYS32 GYCA_OSPTaskCreate
(
	char name[],
	GYU32 stackSize,
	void (*entryPoint)(void),
	GYS32 priority, 
	GYU32 arg1,
	GYU32 arg2,
	GYU32 * taskId
)CA_FUNCPOSTFIX;

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
extern GYS32 GYCA_OSPTaskTemporarySleep(GYU32 milliSecsToWait)CA_FUNCPOSTFIX;

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
extern GYS32 GYCA_OSPSemaphoreCreate
(
	char name[4], 
	GYU32 initialTokenCount,
	GYU32 taskWaitMode,
	GYU32 *semaphoreId
)CA_FUNCPOSTFIX;

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
extern GYS32 GYCA_OSPSemaphoreGetToken (	
	GYU32 semaphoreId,
 	GYU32 waitMode,
	GYU32 milliSecsToWait
)CA_FUNCPOSTFIX;

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
extern GYS32 GYCA_OSPSemaphoreReturnToken(	GYU32 semaphoreId )CA_FUNCPOSTFIX;

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
extern void * GYCA_OSPMalloc(GYU32 numberOfBytes)CA_FUNCPOSTFIX;

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
extern GYS32 GYCA_OSPFree(void *ptrToSegment)CA_FUNCPOSTFIX;

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
extern GYS32 GYCA_OSPQueueCreate
(
	char name[],
	GYU32 maxQueueLength,
	GYU32 taskWaitMode,
	GYU32 *queueId
)CA_FUNCPOSTFIX;

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
extern GYS32 GYCA_OSPQueueGetMessage
(
	GYU32 queueId,
	GYCA_QUEUE_MESSAGE *message ,
	GYU32 waitMode,
	GYU32 milliSecsToWait
)CA_FUNCPOSTFIX;

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
extern GYS32  GYCA_OSPQueueSendMessage(GYU32 queueId,GYCA_QUEUE_MESSAGE * message)CA_FUNCPOSTFIX;


#ifdef  __cplusplus
}
#endif

#endif		//_GYCAS_OSP_H_


