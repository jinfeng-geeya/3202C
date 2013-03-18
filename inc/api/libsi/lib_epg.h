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
 *   2011/7/5    Steven     Add API of epg_get_service_event_ext() to support creating epg
 *                          view by nibble.
 *
 *=========================================================================================*/

#ifndef __LIB_EPG_H__
#define  __LIB_EPG_H__

#include <sys_config.h>
#include <types.h>
#include <api/libsi/si_tdt.h>
#include <api/libsi/si_config.h>

//libg_epg config
//#define EPG_MULTI_TP
//#define EPG_OTH_SCH_SUPPORT
//#define PARENTAL_SUPPORT
//#define COMPONENT_SUPPORT
//#define EPG_CACHE_ENABLE
#ifndef SCHEDULE_TABLE_ID_NUM
#define SCHEDULE_TABLE_ID_NUM 	2 /*(0x50~0x53 or 0x60~0x63), 2(8days) for cable, 4 for sat*/
#endif
#ifndef MAX_EPG_LANGUAGE_COUNT
	#define MAX_EPG_LANGUAGE_COUNT	1	//1 for cable, 3 for sat
#endif

//for most tp, 64 is enough. but some maybe > 64
#ifdef EPG_MULTI_TP
#define MAX_TP_SERVICE_COUNT	400	//max service count of all tps.
#else
#define MAX_TP_SERVICE_COUNT	64	//max service count of one tp
#endif
#define MAX_TP_COUNT			128
#define MAX_ACTIVE_SERVICE_CNT	5
//end config


enum NODE_STATUS
{
	NODE_FREE	= 0,
	NODE_ACTIVE = 1,
	NODE_DIRTY	= 2,
};

enum DB_EVENT_TYPE
{
	PRESENT_EVENT 	= 0,
	FOLLOWING_EVENT = 1,
	SCHEDULE_EVENT 	= 2,
	NIBBLE_TYPE 	= 3,	//for create view by nibble
	EPG_FULL_EVENT	= 0xFF
};

enum DB_EVENT_TYPE_MASK
{
	PRESENT_EVENT_MASK	 = (0X01<<PRESENT_EVENT),
	FOLLOWING_EVENT_MASK = (0X01<<FOLLOWING_EVENT),
	SCHEDULE_EVENT_MASK	 = (0X01<<SCHEDULE_EVENT),
	NIBBLE_MASK = (0X01<<NIBBLE_TYPE),
};

struct TIME
{
	UINT8 hour: 5;		//0-23
	UINT8 minute: 6;	//0-59
	UINT8 second: 6;	//0-59
} __attribute__((packed));

//epg event fixed length node
struct DB_NODE
{

#ifdef EPG_MULTI_TP
	UINT32 tp_id;
#endif
	UINT16 service_id;
	UINT16 event_id;

	UINT8 tab_num: 4;		//table_id & 0x0F
	UINT8 reserved: 4;

	UINT8 event_type: 2;	//DB_EVENT_TYPE
	UINT8 status: 2;		//NODE_STATUS

	UINT8  nibble: 4;		//content descriptor: content_nibble1

#ifdef PARENTAL_SUPPORT
//	struct EPG_MULTI_RATING
//	{
//		UINT8 country_code[3];	//ISO 3166
		UINT8 rating: 4;		//0-0xF, parent rating descriptor: rating & 0x0F
//	} parental[1];
#endif

#if (defined(COMPONENT_SUPPORT) || defined(_ISDBT_ENABLE_))
	UINT8 stream_content: 4;	//Component descriptor
	UINT8 component_type;		//
#endif

	UINT16 mjd_num;
	struct TIME time;
	struct TIME duration;

//	UINT8  lang_cnt;
	struct EPG_MULTI_LANG
	{
		UINT8 *text_char;	//[event name] + [short event text char] + [ext event text char]
		UINT16 text_length;
//		UINT8 lang_code[3];	//ISO_639_language_code of event name, etc.
		UINT16 lang_code2;	//!!!NOTE: converted from 3 byte of ISO_639_language_code
	}lang[MAX_EPG_LANGUAGE_COUNT];
	/*
	{
		UINT8 event_name_length;
		UINT8 event_name[0];
		UINT8 short_text_length;
		UINT8 short_text_char[0];
		UINT16 extend_text_length;
		UINT8 extend_text_char[0];
	}
	*/

} __attribute__((packed));

typedef struct DB_NODE eit_event_info_t;
typedef BOOL (*epg_db_filter_t)(struct DB_NODE *, void *);

//active service info
struct ACTIVE_SERVICE_INFO
{
	UINT32 tp_id;
	UINT16 service_id;
};
#define active_service_t	ACTIVE_SERVICE_INFO

//for create epg view by nibble & service list(eg: TV only, not Radio)
struct nibble_service_t
{
	UINT16 nibble_mask;

	struct
	{
		int count;
		struct
		{
			UINT32 tp_id;
			UINT16 service_id;
			UINT16 channel_num;
		} service[MAX_TP_SERVICE_COUNT];
	} service_list;

	//-1, if not find
	INT32 (*get_channel_num)(UINT32,UINT16,void*);
};

typedef void (*EIT_CALL_BACK)(UINT32 tp_id, UINT16 service_id, UINT8 event_type);


#ifdef __cplusplus
extern "C"{
#endif

INT32 epg_init(UINT8 mode, UINT8* buf, UINT32 len, EIT_CALL_BACK call_back);
INT32 epg_release();
INT32 epg_reset();
INT32 epg_del_service(UINT32 tp_id, UINT16 service_id);

void epg_on(UINT16 sat_id, UINT32 tp_id, UINT16 service_id);
void epg_on_ext(void *dmx, UINT16 sat_id, UINT32 tp_id, UINT16 service_id);
void epg_off();
void epg_off_ext();

//UINT32 epg_get_cur_tp_id();
//UINT16 epg_get_cur_service_id();

struct DB_NODE *epg_get_schedule_event(INT32 index);
//create view & get some events of some service from the epg DB
//type: IN, see enum DB_EVENT_TYPE
//update: IN, TRUE - force to update the event view.
struct DB_NODE *epg_get_service_event(UINT32 tp_id, UINT16 service_id, UINT32 type,
							date_time *start_dt, date_time *end_dt,
							INT32 *num,
							BOOL update);
//type: IN, see enum DB_EVENT_TYPE, if = NIBBLE_TYPE, priv shall be struct nibble_service_t
//		to transfer nibble_mask parameter etc.
struct DB_NODE *epg_get_service_event_ext(UINT32 tp_id, UINT16 service_id, UINT32 type,
							date_time *start_dt, date_time *end_dt,
							INT32 *num,
							BOOL update,
                            void* priv);
//create epg event view by user's filter & private data.
INT32 epg_create_view_by_user(epg_db_filter_t *filter, void *priv);

INT32 eit_compare_time(date_time *d1, date_time *d2);
date_time *get_event_start_time(struct DB_NODE *event, date_time *start_dt);
date_time *get_event_end_time(struct DB_NODE *event, date_time *end_dt);

UINT8* lang2tolang3(UINT16 lang_code2);	//2 byte to 3 byte

INT32 epg_set_active_service(struct ACTIVE_SERVICE_INFO *service, UINT8 cnt);
#define api_epg_set_active_service	epg_set_active_service
BOOL  epg_check_active_service(UINT32 tp_id, UINT16 service_id);

#ifdef __cplusplus
}
#endif

#endif //__LIB_EPG_H__

