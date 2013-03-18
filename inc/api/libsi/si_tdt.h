#ifndef  _SEC_TDT_H_
#define  _SEC_TDT_H_

#include <types.h>


#ifdef __cplusplus
extern "C"
{
#endif


typedef struct 
{
       UINT32 mjd;
	UINT16 year;
	UINT8   month;
	UINT8   day;
	UINT8   weekday;
	UINT8   hour;
	UINT8   min;
	UINT8   sec;
}date_time;

typedef void (* time_callback)(void);
typedef void (*TDT_CALL_BACK)(void);


void mjd_to_ymd(UINT32 mjd,UINT16 * year,UINT8 * month,UINT8 * day,UINT8 * week_day);
UINT32 ymd_to_mjd(UINT16 y, UINT8 m, UINT8 d);
void bcd_to_hms(UINT8 * time,UINT8 * hour,UINT8 * min,UINT8 * sec);
INT32 relative_day(date_time *d1,date_time *d2);
void convert_time_by_offset (date_time *des,date_time *src,INT32 hoff, INT32 moff);
void convert_time_by_offset2 (date_time *des,date_time *src,INT32 hoff, INT32 moff, INT32 soff);
void get_time_offset(date_time*d1,date_time*d2,INT32*day,INT32*hour, INT32*min, INT32*sec);
void get_stream_UTC_time(date_time*dt);

void init_UTC(date_time *dt);
void get_UTC(date_time *dt);
void set_STC_time(date_time *dt);
void set_STC_offset(INT32 hour, INT32 min, INT32 sec);
void get_STC_time(date_time *dt);
void get_STC_offset(INT32 *hour_off,INT32* min_off,INT32 *sec_off);
void tdt_set_country_code( UINT32 cntry_code );

void get_local_time(date_time *dt);
INT32 init_clock(time_callback callback);
INT32 is_time_inited(void);
BOOL is_time_parse_enable();
void enable_time_parse(void);
void disable_time_parse(void);

void init_time_from_stream(UINT8* buff);

#ifdef SUPPORT_TOT_PARSE
INT32 is_time_offset_inited(void);
void enable_time_offset_parse(void);
void disable_time_offset_parse(void);
void enable_summer_time(UINT8 flag);
void init_time_offset_from_stream(UINT8* buff);
#endif

INT32 start_tdt();
INT32 stop_tdt();

void tdt_register_callback(TDT_CALL_BACK cb);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* _SEC_TDT_H_ */
