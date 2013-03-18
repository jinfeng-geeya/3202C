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
**  ¶ÓÁÐÏûÏ¢½á¹¹
*/
typedef struct 
{
	GYU32 q1stWordOfMsg;                /* First word of a queue message.  */
	GYU32 q2ndWordOfMsg;                /* Second word of a queue message. */
	GYU32 q3rdWordOfMsg;                /* Third word of a queue message.  */
	GYU32 q4thWordOfMsg;                /* Fourth word of a queue message. */
}GYCA_QUEUE_MESSAGE;


/*
** ÐÅºÅÁ¿¶ÔÏó¼°ÏûÏ¢¶ÓÁÐ¶ÔÏóµÈ´ýËùÓÃµÄºê¶¨Òå
*/
#define GYCA_WAIT_FIFO		(0)	/* ±íÊ¾È¡µÃÏûÏ¢/ÐÅºÅÁ¿µÄË³ÐòÊÇÏÈµÈ´ýµÄÏÈµÃ
								*/
#define GYCA_WAIT_PRIO		(1)	/* ±íÊ¾È¡µÃÏûÏ¢/ÐÅºÅÁ¿µÄË³ÐòÊÇ¸ù¾ÝÓÅÏÈ¼¶À´µÄ
								*/	
#define GYCA_TASK_NOWAIT	(2)	/* ±íÊ¾Èç¹ûµ±Ç°ÏûÏ¢¶ÓÁÐÖÐÃ»ÓÐÏûÏ¢»òÕßµ±Ç°ÐÅºÅÁ¿
								** Îªoff×´Ì¬µÈ´ýµÄ³ÌÐòÁ¢¼´·µ»Ø
								*/
#define GYCA_TASK_WAIT		(3)	/* ±íÊ¾Èç¹ûµ±Ç°ÏûÏ¢¶ÓÁÐÖÐÃ»ÓÐÏûÏ¢»òÕßµ±Ç°ÐÅºÅÁ¿
								** Îªoff×´Ì¬µÈ´ýµÄ³ÌÐò²»Á¢¼´·µ»Ø£¬¶øÊÇ¹ÒÆð£¬ÖªµÀ
								** µÈ´ýµ½ÏàÓ¦µÄ¶ÔÏó£¬»òÕßµÈ´ýµÄÊ±¼ä³¬Ê±
								*/

/*********************************************************************************************
** 	º¯ÊýÃû³Æ:GYCA_OSPTaskCreate
**********************************************************************************************
**	Ìá ¹© Õß£º 	»ú¶¥ºÐ¡£
**	Ê¹ ÓÃ Õß£º	 CAÄ£¿é¡£
**	¹¦ÄÜÃèÊö£º	´´½¨ÈÎÎñ¡£
**	ÊäÈë²ÎÊý£º	name[]£¬	4¸ö×Ö½ÚµÄÈÎÎñÃû³Æ¡£
**	 				stackSize£¬	ÈÎÎñËùÊ¹ÓÃµÄ¶ÑÕ»µÄ´óÐ¡¡£
**	 				entryPoint£¬ÈÎÎñµÄÈë¿ÚµØÖ·¡£
**	 				priority£¬	ÈÎÎñµÄÓÅÏÈ¼¶¡£
**	 				arg1£¬		´«µÝ¸øÈÎÎñµÄµÚÒ»¸ö²ÎÊý¡£
**	 				taskId£¬	ÈÎÎñµÄID¡£	
**	Êä³ö²ÎÊý£º	ÎÞ¡£
**	·µ »Ø Öµ£º	SUCCESS£º   ³É¹¦£»
**	 				FAILURE£    ·¢Éú´íÎó
**	ÆäËûËµÃ÷£º
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
);

/*********************************************************************************************
** 	º¯ÊýÃû³Æ:GYCA_OSPTaskTemporarySleep
**********************************************************************************************
**	Ìá ¹© Õß£º 	»ú¶¥ºÐ¡£
**	Ê¹ ÓÃ Õß£º	 CAÄ£¿é¡£
**	¹¦ÄÜÃèÊö£º	ÔÝÍ£ÈÎÎñÒ»¶ÎÖ¸¶¨µÄÊ±¼ä¡£
**	ÊäÈë²ÎÊý£º	milliSecsToWait£¬	ÖÕÖ¹ÈÎÎñµÄÊ±¼ä¡££¨ÒÔºÁÃëÎªµ¥Î»¡££©
**	Êä³ö²ÎÊý£º	ÎÞ¡£
**	·µ »Ø Öµ£º	SUCCESS£º       	³É¹¦£»
**	 				FAILURE£º       	·¢Éú´íÎó
**	ÆäËûËµÃ÷£º
**********************************************************************************************/
extern GYS32 GYCA_OSPTaskTemporarySleep(GYU32 milliSecsToWait);

/*********************************************************************************************
** 	º¯ÊýÃû³Æ:GYCA_OSPSemaphoreCreate
**********************************************************************************************
**	Ìá ¹© Õß£º 	»ú¶¥ºÐ¡£
**	Ê¹ ÓÃ Õß£º	 CAÄ£¿é¡£
**	¹¦ÄÜÃèÊö£º	´´½¨ÐÅºÅÁ¿¡£
**	ÊäÈë²ÎÊý£º	name[],				4¸ö×Ö½ÚµÄÐÅºÅÁ¿Ãû³Æ¡£
**	 				initialTokenCount	³õÊ¼»¯Ê±ÐÅºÅÁ¿µÄÊýÁ¿¡£
**	 				taskWaitMode£¬		È¡µÃ¸ÃÐÅºÅÁ¿µÄ·½Ê½¡£ÎªÒÔÏÂÁ½ÖÖÖ®Ò»:
**	 				GYCA_WAIT_FIFO£¬ °´ÏÈµÈ´ýÕßÏÈµÃµÄ·½Ê½È¡µÃ¡£
** 					GYCA_WAIT_PRIO£¬ °´ÓÅÏÈ¼¶¸ßÕßÏÈµÃµÄ·½Ê½È¡µÃ¡£
**	 				semaphoreId£¬		¸ÃÐÅºÅÁ¿µÄID
**	Êä³ö²ÎÊý£º	ÎÞ¡£
**	·µ »Ø Öµ£º	SUCCESS£º       	³É¹¦£»
**	 				FAILURE£º       	·¢Éú´íÎó
**	ÆäËûËµÃ÷£ºÄ¿Ç°CAÄ£¿éÖ»ÓÃµ½ÁËGYCA_WAIT_FIFOÄ£Ê½£¬Èç¹ûÎÞ·¨ÊµÏÖ
**	 				GYCA_WAIT_PRIOÔò¿ÉÒÔÔÝÊ±²»ÊµÏÖ¡£
**********************************************************************************************/
extern GYS32 GYCA_OSPSemaphoreCreate
(
	char name[4], 
	GYU32 initialTokenCount,
	GYU32 taskWaitMode,
	GYU32 *semaphoreId
);

/*********************************************************************************************
** 	º¯ÊýÃû³Æ:GYCAS_GYCA_OSPSemaphoreGetToken
**********************************************************************************************
**	Ìá ¹© Õß£º 	»ú¶¥ºÐ¡£
**	Ê¹ ÓÃ Õß£º	 CAÄ£¿é¡£
**	ºÎÊ±Ê¹ÓÃ£º	´´½¨ÈÎÎñ¡£
**	¹¦ÄÜÃèÊö£º	È¡µÃÐÅºÅÁ¿¡£
**	ÊäÈë²ÎÊý£º	semaphoreId,	µÈ´ýÐÅºÅÁ¿µÄ·½Ê½£¬ÎªÒÔÏÂÁ½ÖÖÖ®Ò»:
**	 				GYCA_TASK_WAIT£¬µÈ´ýÐÅºÅÁ¿Ö±µ½³¬Ê±¡£
** 					GYCA_TASK_NOWAIT£¬²»¹ÜÐÅºÅÁ¿´æÔÚÓë·ñÁ¢¼´·µ»Ø¡£
**	 				milliSecsToWait,µÈ´ýµÄÊ±¼ä¡£Èç¹ûwaitModeÎªGYCA_TASK_NOWAIT£¬Ôò¸Ã²ÎÊýÎÞÐ§¡£
**					Èç¹ûÎªGYCA_TASK_WAIT£¬Ôò´ú±íÊ±¼ä¡£
**	Êä³ö²ÎÊý£º	ÎÞ¡£
**	·µ »Ø Öµ£º	SUCCESS£º       ³É¹¦£»
**	 				FAILURE£º       ·¢Éú´íÎó
**	ÆäËûËµÃ÷£º×¢ÒâÇø±ðÕâÀïµÄµÈ´ýÄ£Ê½ºÍ´´½¨ÐÅºÅÁ¿µÄµÈ´ýÄ£Ê½Ö®¼äµÄÇø±ð¡£
**					´´½¨ÐÅºÅÁ¿µÄÊÇÖ¸Í¬Ê±ÓÐ¶à¸öÈÎÎñÔÚµÈ´ýÍ¬Ò»¸öÐÅºÅÁ¿Ê±Ë­ÓÅÏÈÈ¡
**  					µÃÐÅºÅÁ¿¡£µ½È¡µÃÐÅºÅÁ¿ÎªÖ¹»¹ÊÇ²»×öÈÎºÎµÄµÈ´ý£¬²»¹ÜÊÇ·ñ¿ÉÒÔ
**					È¡µÃÐÅºÅÁ¿£¬Á¢¼´·µ»Ø¡£
**********************************************************************************************/
extern GYS32 GYCA_OSPSemaphoreGetToken (	
	GYU32 semaphoreId,
 	GYU32 waitMode,
	GYU32 milliSecsToWait
);

/*********************************************************************************************
** 	º¯ÊýÃû³Æ:GYCA_OSPSemaphoreReturnToken
**********************************************************************************************
**	Ìá ¹© Õß£º 	»ú¶¥ºÐ¡£
**	Ê¹ ÓÃ Õß£º	 CAÄ£¿é¡£
**	¹¦ÄÜÃèÊö£º	Ôö¼ÓÒ»¸öÐÅºÅÁ¿
**	ÊäÈë²ÎÊý£º	semaphoreId,	¸ÃÐÅºÅÁ¿µÄID
**	Êä³ö²ÎÊý£º	ÎÞ¡£
**	·µ »Ø Öµ£º	SUCCESS£º       ³É¹¦£»
**	 				FAILURE£º       ·¢Éú´íÎó
**	ÆäËûËµÃ÷£º              
**********************************************************************************************/
extern GYS32 GYCA_OSPSemaphoreReturnToken(	GYU32 semaphoreId );

/*********************************************************************************************
** 	º¯ÊýÃû³Æ:GYCA_OSPMalloc
**********************************************************************************************
**	Ìá ¹© Õß£º 	»ú¶¥ºÐ¡£
**	Ê¹ ÓÃ Õß£º	 CAÄ£¿é¡£
**	¹¦ÄÜÃèÊö£º	·ÖÅäÄÚ´æ¡£
**	ÊäÈë²ÎÊý£º	numberOfBytes£¬	·ÖÅäµÄÄÚ´æ¿Õ¼äµÄ´óÐ¡£¨ÒÔ×Ö½ÚÎªµ¥Î»£©¡£
**	Êä³ö²ÎÊý£º	ÎÞ¡£
**	·µ »Ø Öµ£º	Èç¹ûÎªNULL£¬Ôò±íÊ¾·ÖÅä²»µ½ÄÚ´æ,ÆäËûµÄ¾ùÎª³É¹¦·ÖÅä¡£
**	ÆäËûËµÃ÷£º              
**********************************************************************************************/
extern void * GYCA_OSPMalloc(GYU32 numberOfBytes);

/*********************************************************************************************
** 	º¯ÊýÃû³Æ:GYCA_OSPFree
**********************************************************************************************
**	Ìá ¹© Õß£º 	»ú¶¥ºÐ¡£
**	Ê¹ ÓÃ Õß£º	 CAÄ£¿é¡£
**	¹¦ÄÜÃèÊö£º	ÊÍ·Åµô·ÖÅäµÄÄÚ´æ¿Õ¼ä¡£
**	ÊäÈë²ÎÊý£º	ptrToSegment,	ÒªÊÍ·ÅµÄÄÚ´æ¿Õ¼äÖ¸Õë¡£
**	Êä³ö²ÎÊý£º	ÎÞ¡£
**	·µ »Ø Öµ£º	SUCCESS£º       ³É¹¦ÊÍ·Å£»
**	 				FAILURE£º       ÊÍ·Å²»³É¹¦£¬¿ÉÄÜÊÇÒòÎªËù´«µÝ½øÀ´µÄÖ¸ÕëÎÞÐ§¡£
**	ÆäËûËµÃ÷£º              
**********************************************************************************************/
extern GYS32 GYCA_OSPFree(void *ptrToSegment);

/*********************************************************************************************
** 	º¯ÊýÃû³Æ:GYCA_OSPQueueCreate
**********************************************************************************************
**	Ìá ¹© Õß£º 	»ú¶¥ºÐ¡£
**	Ê¹ ÓÃ Õß£º	 CAÄ£¿é¡£
**	¹¦ÄÜÃèÊö£º	´´½¨Ò»¸öÏûÏ¢¶ÓÁÐ¡£
**	ÊäÈë²ÎÊý£º	name[],	4¸ö×Ö½ÚµÄ¶ÓÁÐÃû³Æ¡£
**	 				MaxQueueLength£¬ÏûÏ¢¶ÓÁÐÖÐ¿ÉÒÔ´æ·ÅµÄÏûÏ¢µÄÊýÁ¿¡£µ±ÏûÏ¢¶ÓÁÐÖÐ¸Ã
**                                     	ÊýÁ¿´ïµ½¸ÃÊýÁ¿Ê±£¬ÔÙÍù¸ÃÏûÏ¢¶ÓÁÐ·¢ÏûÏ¢½«»áÊ§°Ü¡£
**	 				taskWaitMode£¬	È¡µÃÏûÏ¢¶ÓÁÐÖÐÏûÏ¢µÄ·½Ê½¡£ÎªÒÔÏÂÁ½ÖÖÖ®Ò»:
**	 					GYCA_WAIT_FIFO£¬ °´ÏÈµÈ´ýÕßÏÈµÃµÄ·½Ê½È¡µÃ¡£
** 						GYCA_WAIT_PRIO£¬ °´ÓÅÏÈ¼¶¸ßÕßÏÈµÃµÄ·½Ê½È¡µÃ¡£
**	 				queueId£¬ÏûÏ¢¶ÓÁÐµÄID¡£
**	Êä³ö²ÎÊý£º	ÎÞ¡£
**	·µ »Ø Öµ£º	SUCCESS£º       	³É¹¦£»
**	 				FAILURE£º       	·¢Éú´íÎó
**	ÆäËûËµÃ÷£º              
**********************************************************************************************/
extern GYS32 GYCA_OSPQueueCreate
(
	char name[],
	GYU32 maxQueueLength,
	GYU32 taskWaitMode,
	GYU32 *queueId
);

/*********************************************************************************************
** 	º¯ÊýÃû³Æ:GYCA_OSPQueueGetMessage
**********************************************************************************************
**	Ìá ¹© Õß£º 	»ú¶¥ºÐ¡£
**	Ê¹ ÓÃ Õß£º	 CAÄ£¿é¡£
**	¹¦ÄÜÃèÊö£º	´ÓÏûÏ¢¶ÓÁÐ»ñÈ¡ÏûÏ¢¡£
**	ÊäÈë²ÎÊý£º	queueId£¬	ËùÒªÈ¡µÃµÄÏûÏ¢µÄID¡£
**	 				waitMode£¬	µÈ´ýÏûÏ¢µÄ·½Ê½£¬ÎªÒÔÏÂÁ½ÖÖÖ®Ò»:
**	 					GYCA_TASK_WAIT£¬µÈ´ýÏûÏ¢Ö±µ½³¬Ê±¡£
** 						GYCA_TASK_NOWAIT£¬²»¹ÜÏûÏ¢´æÔÚÓë·ñÁ¢¼´·µ»Ø¡£
**	 				milliSecsToWait£¬	µÈ´ýµÄÊ±¼ä¡£Èç¹ûwaitModeÎªGYCA_TASK_NOWAIT£¬Ôò¸Ã
**		 				²ÎÊýÎÞÐ§¡£Èç¹ûÎªGYCA_TASK_WAIT£¬Ôò´ú±íÊ±¼ä¡£
**	Êä³ö²ÎÊý£º	messsage£¬	ÎªÏûÏ¢µÄ¸ñÊ½¡£²ÎÕÕQUEUE_MESSAGE¡£
**	·µ »Ø Öµ£º	SUCCESS£º ³É¹¦£»
**	 				FAILURE£º   ·¢Éú´íÎó
**	ÆäËûËµÃ÷£º              
**********************************************************************************************/
extern GYS32 GYCA_OSPQueueGetMessage
(
	GYU32 queueId,
	GYCA_QUEUE_MESSAGE *message ,
	GYU32 waitMode,
	GYU32 milliSecsToWait
);

/*********************************************************************************************
** 	º¯ÊýÃû³Æ: GYCA_OSPQueueSendMessage
**********************************************************************************************
**	Ìá ¹© Õß£º 	»ú¶¥ºÐ¡£
**	Ê¹ ÓÃ Õß£º	 CAÄ£¿é¡£
**	¹¦ÄÜÃèÊö£º	·¢ËÍÏûÏ¢µ½ÏûÏ¢¶ÓÁÐ¡£
**	ÊäÈë²ÎÊý£º	queueId£¬ÏûÏ¢¶ÓÁÐID¡£
**	 				message£¬Òª·¢ËÍµÄÏûÏ¢¡£Æä¸ñÊ½¼ûQUEUE_MESSAGE½á¹¹¡£
**	Êä³ö²ÎÊý£º	ÎÞ¡£
**	·µ »Ø Öµ£º	SUCCESS£º       	³É¹¦£»
**	 				FAILURE£º       	·¢Éú´íÎó
**	ÆäËûËµÃ÷£º              
**********************************************************************************************/
extern GYS32  GYCA_OSPQueueSendMessage(GYU32 queueId,GYCA_QUEUE_MESSAGE * message);


#ifdef  __cplusplus
}
#endif

#endif		//_GYCAS_OSP_H_


