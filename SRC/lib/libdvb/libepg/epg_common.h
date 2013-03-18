/*========================================================================================
 *
 * Copyright (C) 2011 ALi Shanghai Corp.
 *
 * Description:
 *
 * History
 *
 *   Date	    Author		Comment
 *   ========	===========	========================================
 *   ......
 *   2011/7/5    Steven     Re-arrange print message.
 *
 *=========================================================================================*/

#ifndef _EPG_COMMON_H_
#define _EPG_COMMON_H_

#include <types.h>
#include <api/libc/printf.h>
#include <api/libsi/si_tdt.h>


#define	EPG_PRINT				PRINTF
#define EIT_PRINT				PRINTF
#define DB_PRINT				PRINTF
#define	X_PRINT					PRINTF

#define ERR_PRINT				PRINTF
#define MSG_PRINT				PRINTF

//test cases:
//1.only can store defined events
//#define EPG_TEST_CASE_DB_FULL
//max event count of one tp, shall 1tp < MAX_TP_EVENT_COUNT < 2tp.
//#define MAX_TP_EVENT_COUNT	1400

//2.only 100 events should have names/details
//#define EPG_TEST_CASE_NO_MEM
//3.when receive 20 sch sections, then version update
//#define EPG_TEST_CASE_SEC_VER_UPDATE


//error code
#define ERR_DB_FULL		-43


//#define SCHEDULE_TABLE_ID_NUM 	4 /*(0x50~0x53 or 0x60~0x63)*/
#define SCHEDULE_DAY			(SCHEDULE_TABLE_ID_NUM*4)

#define SI_EIT_SECTION_LEN		4096
//#define PSI_EIT_PID				0x0012


#define EPGDB_FLAG_MUTEX		0x80000000UL
#define ALLOC_FLAG_MUTEX		0x40000000UL
#define EPG_FLAG_MUTEX			0x20000000UL


#define SI_EIT_TABLE_ID_APF		0x4E
#define SI_EIT_TABLE_ID_OPF		0x4F

#define SI_EIT_TABLE_ID_ASCH	0x50
#define SI_EIT_TABLE_ID_OSCH	0x60

#define IS_SCH(tid)			(((tid&0xf0) == SI_EIT_TABLE_ID_ASCH)||((tid&0xf0) == SI_EIT_TABLE_ID_OSCH))
#define IS_OTH_SCH(tid)		((tid&0xf0) == SI_EIT_TABLE_ID_OSCH)
#define IS_PF(tid)			((tid == SI_EIT_TABLE_ID_APF)||(tid == SI_EIT_TABLE_ID_OPF))
#define IS_OTH_PF(tid)		(tid == SI_EIT_TABLE_ID_OPF)

enum EPG_STATUS
{
	STATUS_AVAILABLE   = 0,
	STATUS_UNAVAILABLE = -1,
	STATUS_NOT_READY   = 0,
	STATUS_OFF		   = 1,
	STATUS_ON		   = 2,
};

//event condition struct for creating view
//example:
//1. if need create view including present & following events, set event_mask to be
//	(PRESENT_EVENT_MASK | FOLLOWING_EVENT_MASK)
//2. if need create view of schedule events, set event_mask to be
//	SCHEDULE_EVENT_MASK
struct view_condition
{
	UINT32 tp_id;
	UINT16 service_id;
	UINT32 event_mask;	//PRESENT_EVENT_MASK,FOLLOWING_EVENT_MASK,SCHEDULE_EVENT_MASK

	date_time start_dt;	//time restriction for sch events
	date_time end_dt;

	void* priv;			//extend
};

#endif //_EPG_COMMON_H_

