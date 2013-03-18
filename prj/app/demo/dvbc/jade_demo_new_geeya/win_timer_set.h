#ifndef _WIN_TIMER_SET_H_
#define _WIN_TIMER_SET_H_

typedef enum
{
	TIMER_TYPE_EPG =0,
	TIMER_TYPE_NVOD,
	TIMER_TYPE_AUTO_REBOOT,
}TYPE_TIMER;

BOOL win_timerset_open(TIMER_SET_CONTENT *settimer, TIMER_SET_CONTENT *rettimer, BOOL check_starttime);
INT32 IsValidTimer(TIMER_SET_CONTENT *timer);
INT32 find_suited_timer(UINT32 prog_id, date_time* start_dt, date_time* end_dt);
UINT8 find_available_timer();
INT32 insert_timer(TYPE_TIMER type,TIMER_SET_CONTENT  *t_insert, UINT32 pos);
void resort_timer(TYPE_TIMER type,TIMER_SET_CONTENT* base);
INT32 find_timer_with_st(date_time * start_dt);
#endif //_WIN_TIMER_SET_H_
