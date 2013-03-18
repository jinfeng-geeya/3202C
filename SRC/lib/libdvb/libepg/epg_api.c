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

#include <api/libc/string.h>
#include <api/libsi/lib_epg.h>
#include <api/libsi/si_tdt.h>
#include "epg_common.h"
#include "epg_db.h"

static UINT8 epg_view_buf[8192];		//db view buffer
static UINT8 epg_text_buf[8192];		//event name + short text char + extended text char
static struct DB_NODE p_event_node;			//present event node
static struct DB_NODE f_event_node;			//following event node
static struct DB_NODE s_event_node;			//schedule event node

//event condition struct for creating view
static struct view_condition local_view_con;

//local event check func & private data
static struct DB_CHECK service_check;

extern void epg_enter_mutex();
extern void epg_leave_mutex();

extern UINT32 epg_get_cur_tp_id();

/*if d2 > d1 , return 1,
   if d2== d1, return 0,
   if d2 < d1, return -1*/
INT32 eit_compare_time(date_time *d1, date_time *d2)
{
	if (d1 == NULL || d2 == NULL)
	{
#ifdef _DEBUG
		ASSERT(0);
#endif
		return 0;
	}

//	if (d2->mjd-d1->mjd !=0)
//		return d2->mjd-d1->mjd;
	if (d2->year-d1->year !=0)
		return d2->year-d1->year;
	else if (d2->month -d1->month !=0)
		return d2->month -d1->month;
	else if (d2->day -d1->day !=0)
		return d2->day -d1->day;
	else if (d2->hour -d1->hour !=0)
		return d2->hour -d1->hour;
	else if((d2->min - d1->min) != 0)
		return d2->min-d1->min;
    else
        return d2->sec - d1->sec;
}

date_time *get_event_start_time(struct DB_NODE *event, date_time *start_dt)
{
	if (event == NULL || start_dt == NULL)
		return NULL;

	start_dt->mjd = event->mjd_num;
	start_dt->hour = event->time.hour;
	start_dt->min = event->time.minute;
	start_dt->sec = event->time.second;
	mjd_to_ymd(start_dt->mjd, &start_dt->year, &start_dt->month, &start_dt->day, &start_dt->weekday);

	return start_dt;
}

date_time *get_event_end_time(struct DB_NODE *event, date_time *end_dt)
{
	if (event == NULL || end_dt == NULL)
		return NULL;

	end_dt->mjd = event->mjd_num;
	end_dt->hour = event->time.hour;
	end_dt->min = event->time.minute;
	end_dt->sec = event->time.second;

	end_dt->sec += event->duration.second;
	end_dt->min += event->duration.minute;
	end_dt->hour += event->duration.hour;

	end_dt->min += (end_dt->sec / 60);
	end_dt->sec = end_dt->sec % 60;

	end_dt->hour += (end_dt->min / 60);
	end_dt->min %= 60;

	end_dt->mjd += (end_dt->hour / 24);
	end_dt->hour %= 24;
	mjd_to_ymd(end_dt->mjd, &end_dt->year, &end_dt->month, &end_dt->day, &end_dt->weekday);

	return end_dt;
}

//present & following events maybe has no mjd data, and its duration is zero,
//so need fix them here
static struct DB_NODE *fix_pf_event(struct DB_NODE *event)
{
	date_time dt;
	if (event != NULL
		&& (event->event_type == PRESENT_EVENT || event->event_type == FOLLOWING_EVENT))
	{
		if (event->mjd_num != 0xFFFF
			&& event->mjd_num >= 53371/*2005/1/1*/)
		{
			return event;
		}
		//fix PF event has no MJD
		get_STC_time(&dt);
		//if the time across the 00:00, do fix mjd +1 or -1.
		if (event->event_type == PRESENT_EVENT
			&& (event->time.hour*3600+event->time.minute*60+event->time.second)
				> (dt.hour*3600+dt.min*60+dt.sec))
		{
			event->mjd_num = dt.mjd - 1;
		}
		else if (event->event_type == FOLLOWING_EVENT
			&& (event->time.hour*3600+event->time.minute*60+event->time.second)
				< (dt.hour*3600+dt.min*60+dt.sec))
		{
			event->mjd_num = dt.mjd + 1;
		}
		else
			event->mjd_num = dt.mjd;

		//fix PF event duration = 0
		if (event->duration.hour == 0
			&& event->duration.minute == 0
			&& event->duration.second == 0)
			event->duration.hour = 24;
	}

	return event;
}

//check: service id + event type + date time
static BOOL check_event(struct DB_NODE *node, void *priv)
{
	struct view_condition *condition = (struct view_condition*)priv;
	date_time event_start_dt;
	date_time event_end_dt;
	BOOL flag = FALSE;
	struct nibble_service_t* nibble_service = NULL;

	if (node != NULL
		&& priv != NULL
		&& node->status == NODE_ACTIVE)
	{
		if (NIBBLE_MASK&condition->event_mask)
		{
			nibble_service = (struct nibble_service_t*)(condition->priv);
			if (nibble_service != NULL
				&& ((1<<node->nibble) & nibble_service->nibble_mask) != 0
				&& (nibble_service->get_channel_num == NULL ||
					-1 != nibble_service->get_channel_num(
#ifdef EPG_MULTI_TP
                    node->tp_id,
#else
                    0,
#endif
                    node->service_id, nibble_service)))
				flag = TRUE;
		}
		else if (
			node->service_id == condition->service_id
#ifdef EPG_MULTI_TP
			&& node->tp_id == condition->tp_id
#endif
			&& ((0x01<<node->event_type)&condition->event_mask))
		{
			flag = TRUE;
		}
	}

	if (flag)
	{
		if(node->mjd_num == 0)
			fix_pf_event(node);

		if (node->event_type == SCHEDULE_EVENT
			|| (condition->start_dt.month != 0 && condition->start_dt.day != 0))
		{
			//event start time
			get_event_start_time(node, &event_start_dt);

			if (NIBBLE_MASK&condition->event_mask)
			{
				//only compare start time
				if (eit_compare_time(&condition->start_dt,&event_start_dt) >= 0
					&& eit_compare_time(&condition->end_dt,&event_start_dt) <= 0)
					return TRUE;
				else
					return FALSE;
			}
			else
			{
				//event end time
				get_event_end_time(node, &event_end_dt);

				//[start_dt, end_dt]
				// -> event_start_dt <= end_dt
				//	&& event_end_dt > start_dt
				if (eit_compare_time(&condition->end_dt, &event_start_dt) <= 0
					&& eit_compare_time(&condition->start_dt, &event_end_dt) > 0)
					return TRUE;
				else
					return FALSE;
			}
		}
		else
			return TRUE;
	}
	else
		return FALSE;
}

//create a view with some condition
//update: IN, if pf or sch event update, do create view any more
static INT32 epg_create_view(struct view_condition *condition, BOOL update)
{
	static INT32 ret = 0;

	epg_enter_mutex();

#ifndef EPG_MULTI_TP
	if (condition == NULL
		|| condition->tp_id != epg_get_cur_tp_id())
	{
		//reset restriction
		MEMSET(&local_view_con, 0, sizeof(local_view_con));
		reset_epg_view();
		epg_leave_mutex();
		return 0;
	}
#endif

	if (MEMCMP(&local_view_con, condition, sizeof(local_view_con)) != 0
		|| update)
	{
		MEMCPY(&local_view_con, condition, sizeof(local_view_con));

		service_check.check_func = check_event;
		service_check.priv = (void*)&local_view_con;

		ret = create_view((UINT32*)epg_view_buf, sizeof(epg_view_buf), &service_check);
	}

	epg_leave_mutex();
	return ret;
}

static struct DB_NODE *epg_get_present_event()
{
	//maybe there is no present event or following event
	if (SUCCESS == get_event_by_pos(0, &p_event_node, epg_text_buf, 4096)
		&& p_event_node.event_type == PRESENT_EVENT)
	{
		return &p_event_node;
	}
	else
		return NULL;
}

static struct DB_NODE *epg_get_following_event()
{
	int i;

	//maybe there is no present event or following event
	for (i=1; i>=0; i--)
	{
		if (SUCCESS == get_event_by_pos(i, &f_event_node, epg_text_buf+4096, 4096)
			&& f_event_node.event_type == FOLLOWING_EVENT)
			return &f_event_node;
	}

	return NULL;
}

struct DB_NODE *epg_get_schedule_event(INT32 index)
{
	if (SUCCESS == get_event_by_pos(index, &s_event_node, epg_text_buf, 8192))
		return &s_event_node;
	else
		return NULL;
}

//event_type: IN, see enum DB_EVENT_TYPE
//update: IN, force to update the view?
//return: event count of this view
static INT32 epg_create_event_view(UINT32 tp_id, UINT16 service_id, UINT8 event_type,
							date_time *start_dt, date_time *end_dt,
							BOOL update,
							void* priv)
{
	struct view_condition condition;

	MEMSET(&condition, 0, sizeof(condition));
	condition.tp_id = tp_id;
	condition.service_id = service_id;
	
	if (event_type == PRESENT_EVENT || event_type == FOLLOWING_EVENT)
		condition.event_mask = PRESENT_EVENT_MASK | FOLLOWING_EVENT_MASK;
	else if (event_type == SCHEDULE_EVENT)	//schedule
	{
		condition.event_mask = PRESENT_EVENT_MASK | FOLLOWING_EVENT_MASK | SCHEDULE_EVENT_MASK;
//		condition.event_mask = SCHEDULE_EVENT_MASK;
	}
	else if (event_type == NIBBLE_TYPE)
	{
		condition.event_mask = NIBBLE_MASK;
		condition.priv = priv;
	}
	else
	{
		return 0;
	}

	if (start_dt != NULL && end_dt != NULL)
	{
		condition.start_dt = *start_dt;
		condition.end_dt = *end_dt;
	}

	INT32 cnt = epg_create_view(&condition, update);
	//if no schedule event, get pf event
	/*if (event_type == SCHEDULE_EVENT
		&& cnt == 0)
	{
		condition.event_mask = PRESENT_EVENT_MASK | FOLLOWING_EVENT_MASK;
		cnt = epg_create_view(&condition, update);
	}*/

	return cnt;
}

#ifdef EPG_CACHE_ENABLE
static struct DB_NODE *epg_get_cache_present_event(UINT32 tp_id, UINT16 service_id)
{
	//maybe there is no present event or following event
	if (SUCCESS == get_event_from_cache(tp_id, service_id,PRESENT_EVENT, &p_event_node, epg_text_buf, 4096)
		&& p_event_node.event_type == PRESENT_EVENT)
	{
		return &p_event_node;
	}
	else
		return NULL;
}

static struct DB_NODE *epg_get_cache_following_event(UINT32 tp_id, UINT16 service_id)
{
	//maybe there is no present event or following event
	if (SUCCESS == get_event_from_cache(tp_id, service_id, FOLLOWING_EVENT, &f_event_node, epg_text_buf+4096, 4096)
		&& f_event_node.event_type == FOLLOWING_EVENT)
	{
		return &f_event_node;
	}
	else
		return NULL;
}
#endif

//create view & get some events of some service from the epg DB
//type: IN, see enum DB_EVENT_TYPE
//		extended for support NIBBLE view - 2011/6/30
//		if = NIBBLE_TYPE, priv shall be struct nibble_service_t
//		to transfer nibble_mask parameter etc.
//update: IN, TRUE - force to update the event view.
struct DB_NODE *epg_get_service_event_ext(UINT32 tp_id, UINT16 service_id, UINT32 type,
							date_time *start_dt, date_time *end_dt,
							INT32 *num,
							BOOL update,
							void* priv)
{
	INT32 event_num;
	UINT8 event_type = type&0xFF;

	//first create view
	event_num = epg_create_event_view(tp_id, service_id, event_type, start_dt, end_dt, update, priv);
	if (num != NULL)
		*num = event_num;

	if (event_num == 0)
	{
#ifdef EPG_CACHE_ENABLE
		if (event_type == PRESENT_EVENT)
			return epg_get_cache_present_event(tp_id, service_id);
		else if (event_type == FOLLOWING_EVENT)
			return epg_get_cache_following_event(tp_id, service_id);
		else
#endif
			return NULL;
	}
	else
	{
		//then can get node from view
		if (event_type == PRESENT_EVENT)
			return epg_get_present_event();
		else if (event_type == FOLLOWING_EVENT)
			return epg_get_following_event();
		else
			return epg_get_schedule_event(0);
	}
}

struct DB_NODE *epg_get_service_event(UINT32 tp_id, UINT16 service_id, UINT32 type,
							date_time *start_dt, date_time *end_dt,
							INT32 *num,
							BOOL update)
{
	return epg_get_service_event_ext(tp_id, service_id, type, start_dt, end_dt, num, update, NULL);
}

//create epg event view by user restriction
INT32 epg_create_view_by_user(epg_db_filter_t *filter, void *priv)
{
	static INT32 ret = 0;

	epg_enter_mutex();

	service_check.check_func = (check_func_t)filter;
	service_check.priv = priv;

	ret = create_view((UINT32*)epg_view_buf, sizeof(epg_view_buf), &service_check);

	epg_leave_mutex();
	return ret;
}

