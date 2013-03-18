
#include <retcode.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <api/libsi/si_tdt.h>
#include <api/libsi/si_module.h>
#include <api/libtsi/sie.h>
#include <api/libtsi/si_types.h>

#define SUPPORT_TOT_PARSE
#define _SYNC_TIME_ENABLE_

#if 0
#define TDT_PRINTF libc_printf
#else
#define TDT_PRINTF(...) do{}while(0)
#endif

static BOOL need_sync_time();


struct time_data_section
{
	UINT8 table_id;
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 section_syntax_indicator			: 1;
	UINT8 reserved_future_use			: 1;
	UINT8 reserved					: 2;
	UINT8 section_length_high			: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 section_length_high			: 4;
	UINT8 reserved					: 2;
	UINT8 reserved_future_use			: 1;
	UINT8 section_syntax_indicator			: 1;
#endif
	UINT8 section_length_low;
	UINT8 UTC_time[5];
}__attribute__((packed));

struct tdt_filter_info
{
	UINT8 table_id;
	struct restrict mask_value;
	UINT8* buff;
	UINT32 bufflen;
};

/*
struct time_offset_section {
	UINT8 table_id;
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 section_syntax_indicator		: 1;
	UINT8 reserved_future_use		: 1;
	UINT8 reserved				: 2;
	UINT8 section_length_hi			: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 section_length_high			: 4;
	UINT8 reserved				: 2;
	UINT8 reserved_future_use		: 1;
	UINT8 section_syntax_indicator		: 1;
#endif
	UINT8 section_length_low;
	UINT8 UTC_time[5];
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 reserved2				: 4;
	UINT8 descriptor_loop_length_hi		: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 descriptor_loop_length_hi		: 4;
	UINT8 reserved2				: 4;
#endif
	UINT8 descriptor_loop_lenght_low;
	UINT8 descriptor[0];
	UINT8 crc32[4];
};
*/
/*
*	UTC -- The GMT Date and Time  from stream.
*	STC	-- The GMT Date and Time  from user.	
*/
static date_time UTC,STC;
static date_time Time_Of_Change = { 0xFFFFFFFF, 0xFFFF, 0xFF, 0xFF,0xFF,0xFF,0xFF, 0xFF }; 
static UINT32 toc_cntry_code = 0xFFFFFF; // (Time_of_Change, toc_cntry_code)
static INT32 next_hour_offset = 0;	// Corresponds to next_time_offset
static INT32 next_min_offset = 0;
static INT32 hour_offset;
static INT32 min_offset;
static INT32 sec_offset;

static INT32 time_started;
static INT32 time_inited;
static BOOL time_offset_inited = FALSE;
static BOOL time_parse_enable = TRUE;
#ifdef SUPPORT_TOT_PARSE
static INT8 tot_delay_flag =0 ;
static INT32 utc_hour_offset;
static INT32 utc_min_offset;
static INT32 utc_sec_offset;
#endif
static UINT32 sys_country_code;
static UINT8 summer_time_enabled = 0;

static time_callback call_back;

static TDT_CALL_BACK tdt_cb = NULL;

void mjd_to_ymd(UINT32 mjd, UINT16 * year, UINT8 * month, UINT8 * day, UINT8 * week_day)
{
	static UINT32 pre_mjd = 0xFFFFFFFF;
	static UINT16 pre_year;
	static UINT8  pre_month;
	static UINT8 pre_day;
	static UINT8 pre_week_day;
	UINT32 Y,M,D,K;

	if (mjd <= 15078)   //fix bug:stream's mjd error
        mjd = 53371;    //set to default:2005/1/1

	if (mjd == pre_mjd)
	{
		*year = pre_year;
		*month = pre_month;
		*day = pre_day;
		*week_day = pre_week_day;
	}
	else
	{//according to MJD formula, use integer instead of float point
		Y = (20*mjd - 301564) / 7305;
		UINT32 YY = (Y*365 + Y/4);	//(UINT32)(Y * 365.25)
		M = 1000*(10*mjd - 149561 - 10*YY) / 306001;
		D = mjd - 14956 - YY - M * 306001 / 10000;

		if(M == 14 || M == 15)
			K = 1;
		else 
			K = 0;
			
		*year = (UINT16)(Y + K + 1900);
		*month = M - 1 - K * 12;
		if (*month > 12)
			*month = 1;
		*day = D;
		if (*day > 31)
			*day = 1;	
		*week_day = ((mjd + 2) % 7) + 1;

		//record it
		pre_mjd = mjd;
		pre_year = *year;
		pre_month = *month;
		pre_day = *day;
		pre_week_day = *week_day;
	}
}

/*
*Convert Y/M/D  to mjd code.  Y >= 1900
*/
UINT32 ymd_to_mjd(UINT16 y, UINT8 m, UINT8 d)
{
	INT8 l;
	INT16 Y=y;
	
	if (m == 1 || m== 2)
		l = 1;
	else
		l = 0;
	
	Y -= 1900;
	y=(Y < 0)?0:Y;
		
//	return (14956 + d + (UINT32)((y -l) * 365.25) + (UINT32)(((m + 1 + l * 12) * 30.6001)));
	UINT32 yl = (y -l);
	UINT32 yy = yl * 365 + yl / 4;

	return (14956 + d + yy + (UINT32)(((m + 1 + l * 12) * 306001)/10000));
}

/* Convert BCD code to hour, min, sec */
void bcd_to_hms(UINT8 * time,	UINT8 * hour, UINT8 * min, UINT8 * sec)
{
	*hour = (time[0] >> 4) * 10 + (time[0] & 0x0f);
	*min = (time[1] >> 4) * 10 + (time[1] & 0x0f);
	*sec = (time[2] >> 4) * 10 + (time[2] & 0x0f);
}
/* Convert BCD code to hour, min, */
void bcd_to_hm(UINT8 * time,	UINT8 * hour, UINT8 * min)
{
	*hour = (time[0] >> 4) * 10 + (time[0] & 0x0f);
	*min = (time[1] >> 4) * 10 + (time[1] & 0x0f);
}
/*
* return the relative days between date1 and date2
* return (date2-date1)
*/
INT32 relative_day(date_time *d1,date_time *d2)
{
	return (ymd_to_mjd(d2->year, d2->month, d2->day) - ymd_to_mjd(d1->year, d1->month, d1->day));

}
/*
if d1 > d2 return 1
else return 0
*/
INT8 date_time_compare(date_time *d1,date_time *d2)
{
	INT32 ret;

	ret = relative_day(d2,d1);
    if(ret > 0)
     {
        return 1;
     }
    else if(ret < 0)
     {
        return 0;
     }
    else //same date
     {
         if((d1->hour*3600+d1->min*60+d1->sec)>(d2->hour*3600+d2->min*60+d2->sec))
            return 1;
         else
            return 0;
     
     }       
}
/*
* return the offset between date1 and date2
* return (date2-date1)
*/
void get_time_offset(date_time*d1,date_time*d2,INT32*day,INT32*hour, INT32*min, INT32*sec)
{
	*day = relative_day(d1, d2);
	*hour = d2->hour - d1->hour;
	*min = d2->min - d1->min;
	*sec = 0;
}

void convert_time_by_offset (date_time *des,date_time *src,INT32 hoff, INT32 moff)
{
	INT32 h,m;
	INT32 doff=0;
	h=src->hour;
	m=src->min;	
	UINT32 mjd;

	/*deal with min offset*/	
	if ((m + moff) < 0)
		h --;
	else if ((m + moff) >=60)		//050728-cmchen
		h++;
	
	m=(m+moff+60)%60;		

	/*deal with hour offset*/	
	if ((h + hoff) < 0)
		doff--;
	else if ((h + hoff) >=24)	//GMI issue#1807 050728-cmchen
		doff++;
	h=(h+hoff+24)%24;
	
	/*update the ymd*/	
	mjd=ymd_to_mjd(src->year,src->month, src->day)+doff;
	
	mjd_to_ymd(mjd, &des->year,&des->month,&des->day,&des->weekday);
	des->mjd=mjd;
	des->min=m;
	des->hour=h;	
	des->sec=src->sec;	
}

void convert_time_by_offset2 (date_time *des,date_time *src,INT32 hoff, INT32 moff, INT32 soff)
{
	INT32 h,m,s;
	INT32 doff=0;
	h=src->hour;
	m=src->min;	
	s=src->sec;
	UINT32 mjd;

	/*deal with second offset*/	
	if ((s + soff) < 0)
		m --;
	else if ((s + soff) >=60)		
		m++;
	
	s=(s+soff+60)%60;		

	/*deal with min offset*/	
	if ((m + moff) < 0)
		h --;
	else if ((m + moff) >=60)		//050728-cmchen
		h++;
	
	m=(m+moff+60)%60;		

	/*deal with hour offset*/	
	if ((h + hoff) < 0)
		doff--;
	else if ((h + hoff) >=24)	//GMI issue#1807 050728-cmchen
		doff++;
	h=(h+hoff+24)%24;
	
	/*update the ymd*/	
	mjd=ymd_to_mjd(src->year,src->month, src->day)+doff;
	
	mjd_to_ymd(mjd, &des->year,&des->month,&des->day,&des->weekday);
	des->mjd=mjd;
	des->min=m;
	des->hour=h;	
	des->sec=s;	
}

static void add_second(date_time *dt, INT32 sec)
{
	dt->sec+=sec;
	if (dt->sec>= 60)
	{
        	dt->sec -= 60;
		if (++dt->min== 60)
		{
			dt->min = 0;
			if (++dt->hour == 24)
			{
				UINT16 year;
				UINT8 month;
				UINT8 day;
				UINT8 week_day;
				
				dt->hour = 0;
				mjd_to_ymd(++dt->mjd, &year,&month,&day,&week_day);
				dt->year    = year;
				dt->month   = month;
				dt->day     = day;
				dt->weekday = week_day;
			}
		}
	}	
}

static void clock_handler(UINT32 no_use)
{
	date_time utc,stc;
	INT32 sec;
	static UINT32 old_tick = 0;
	UINT32 new_tick;
	INT32 tmp;

	new_tick = osal_get_tick();
	if (time_started==0)
	{
		old_tick=new_tick;
		time_started=1;
		return;
	}
	else
	{
		tmp=new_tick-old_tick;
		if (tmp<1000 && tmp >=0)
			return;
		else if (tmp>=1000)
			sec=(tmp/1000);			
		else //<0
		{
			tmp=new_tick+(0xFFFFFFFF-old_tick);
			if (tmp<1000)
				return;
			sec=(tmp/1000);
		}
        
		/* To avoid date_time sec(one byte) field overflow .*/
		if(sec > 60) {
			sec = 60;
		}
		old_tick+=1000*sec;
	}			

	utc=UTC;
	stc=STC;
	add_second(&stc, sec);
	STC=stc;
	add_second(&utc, sec);
	UTC=utc;
	
	if (call_back != NULL)
	{
		call_back();
	}
}

void init_UTC(date_time *dt)
{
	dt->mjd=ymd_to_mjd(dt->year, dt->month,dt->day);
	osal_interrupt_disable();
	UTC=*dt;
	osal_interrupt_enable();
}

void get_UTC(date_time *dt)
{
	*dt=UTC;
}

void set_STC_time(date_time *dt)
{
	dt->mjd=ymd_to_mjd(dt->year, dt->month,dt->day);
	dt->weekday=((dt->mjd + 2) % 7) + 1;
	osal_interrupt_disable();
	STC=*dt;
	osal_interrupt_enable();
}

void set_STC_offset(INT32 hour, INT32 min, INT32 sec)
{
	hour_offset = hour;
	min_offset = min;
	sec_offset = sec;
	TDT_PRINTF("%s, offset(%d:%d:%d)\n", __FUNCTION__, \
				hour_offset, min_offset, sec_offset );
}

void get_STC_time(date_time *dt)
{
	*dt=STC;
	TDT_PRINTF("%s, (%d-%d-%d %d:%d:%d)\n", __FUNCTION__, \
				STC.year, STC.month, STC.day, \
				STC.hour, STC.min, STC.sec );
}

void get_STC_offset(INT32 *hour_off,INT32* min_off,INT32 *sec_off)
{
    #ifdef SUPPORT_TOT_PARSE
	/*
        *hour_off=hour_offset+utc_hour_offset;
        if(summer_time_enabled)
            (*hour_off)++;
        *min_off=min_offset+utc_min_offset;
        *sec_off=sec_offset+utc_sec_offset;
    */
	date_time utc;
	*hour_off = hour_offset;
	if(summer_time_enabled)
		(*hour_off)++;
	*min_off = min_offset;
	*sec_off= sec_offset;

	get_UTC( &utc );
	if( toc_cntry_code == sys_country_code
		&& !date_time_compare( &Time_Of_Change, &utc ) 
		&& time_offset_inited == FALSE ) // utc>=Time_Of_Change, then
	{
		if( *hour_off != next_hour_offset || *min_off != next_min_offset )
		{
			*hour_off = next_hour_offset;
			*min_off  = next_min_offset;
			TDT_PRINTF("%s, use next time offset\n", __FUNCTION__ );
		}
	}

    #else
        *hour_off=hour_offset;
        if(summer_time_enabled)
            (*hour_off)++;
        *min_off=min_offset;
        *sec_off=sec_offset;
    #endif
}

#ifdef SUPPORT_TOT_PARSE
/*
 *  deprecate: It is recommened that do not use any more
 */
void set_UTC_offset(INT32 hour, INT32 min, INT32 sec)
{
	utc_hour_offset = hour;
	utc_min_offset = min;
	utc_sec_offset = sec;
	TDT_PRINTF("%s, offset(%d:%d:%d)\n", __FUNCTION__, \
				utc_hour_offset, utc_min_offset, utc_sec_offset );
}
/*
 *  deprecate: It is recommened that do not use any more
 */
void get_UTC_offset(INT32 *hour_off,INT32* min_off,INT32 *sec_off)
{
	*hour_off=utc_hour_offset;
	*min_off=utc_min_offset;
	*sec_off=utc_sec_offset;
}
#endif

/*
 * country code, used by TOT, reference to ISO3166-1, example: GBR, cntry_code = 'G'<<16|'B'<<8|'R'
 * should set when system boot up, and change region setting
 */
void tdt_set_country_code( UINT32 cntry_code )
{
	TDT_PRINTF("%s, set cntry code: 0x%x\n", __FUNCTION__, cntry_code );
	sys_country_code = cntry_code;
}

/*
 * STB Time, STC + STC_Offset
 */
void get_local_time(date_time *dt)
{
	date_time stc;
	INT32 h,m,s;
	get_STC_time(&stc);
	get_STC_offset(&h,&m,&s);
	convert_time_by_offset(dt, &stc, h,m);
	TDT_PRINTF("%s, (%d-%d-%d %d:%d:%d)\n", __FUNCTION__, \
				dt->year, dt->month, dt->day, \
				dt->hour, dt->min, dt->sec );	
}

void enable_summer_time(UINT8 flag)
{
    summer_time_enabled = flag;
}

INT32 init_clock(time_callback callback)
{
	OSAL_ID os_id;
	OSAL_T_CTIM osal_t_ctim;
	osal_t_ctim.param = 0;
	osal_t_ctim.type = OSAL_TIMER_CYCLE;
	osal_t_ctim.callback = (OSAL_T_TIMER_FUNC_PTR)clock_handler;
	osal_t_ctim.time = 1000;		
	os_id= osal_timer_create(&osal_t_ctim);
	PRINTF("timer is %s : 0x%x\n", os_id != OSAL_INVALID_ID? "OK!" : "Error!", os_id);
	call_back = callback;
	osal_timer_activate(os_id, 1);		
	return SUCCESS;
}


INT32 is_time_inited(void)
{
	return time_inited;
}

BOOL is_time_parse_enable()
{
	return time_parse_enable;
}
void enable_time_parse(void)
{
    	time_parse_enable = TRUE;
}

void disable_time_parse(void)
{
    	time_parse_enable = FALSE;
}

void init_time_from_stream(UINT8* buff)
{
	struct time_data_section *tds_buff;	
	tds_buff = (struct time_data_section *)buff;
	
	if ((is_time_parse_enable()!=FALSE)&&(tds_buff->table_id==PSI_TDT_TABLE_ID||
	    tds_buff->table_id == PSI_TOT_TABLE_ID))
	{
		date_time utc;

		utc.mjd= tds_buff->UTC_time[0]<<8 | tds_buff->UTC_time[1];
		
		bcd_to_hms(&(tds_buff->UTC_time[2]),&utc.hour, &utc.min, &utc.sec);			
		mjd_to_ymd(utc.mjd,&utc.year,&utc.month,&utc.day,&utc.weekday);

		set_STC_time(&utc);		// only sync UTC time from stream here, User may use its manual time (GMT: off) as STC time, instead of using UTC time as STC time all the time.
		init_UTC(&utc);
		time_inited = TRUE;

		if(tdt_cb)
			tdt_cb();
		//libc_printf("\n%s(): time[%d-%d-%d, %d:%d:%d]\n",__FUNCTION__, 
		//	utc.year,utc.month,utc.day,utc.hour,utc.min,utc.sec);
	}
}


static date_time stream_utc;
//same with init_time_from_stream(), except no set STC time
void set_time_from_stream(UINT8* buff)
{
	struct time_data_section *tds_buff;	
	tds_buff = (struct time_data_section *)buff;
	//if ((tds_buff->table_id==PSI_TDT_TABLE_ID)||(tds_buff->table_id == PSI_TOT_TABLE_ID))
	if ((tds_buff->table_id==PSI_TDT_TABLE_ID))
	{

		stream_utc.mjd= tds_buff->UTC_time[0]<<8 | tds_buff->UTC_time[1];
		
		bcd_to_hms(&(tds_buff->UTC_time[2]),&stream_utc.hour, &stream_utc.min, &stream_utc.sec);			
		mjd_to_ymd(stream_utc.mjd,&stream_utc.year,&stream_utc.month,&stream_utc.day,&stream_utc.weekday);
		TDT_PRINTF("%s, (%d-%d-%d %d:%d:%d)\n", __FUNCTION__, \
					stream_utc.year, stream_utc.month, stream_utc.day, \
					stream_utc.hour, stream_utc.min, stream_utc.sec );
	}
}

//alway return tdt/tot's utc time
void get_stream_UTC_time(date_time*dt)
{
	MEMCPY(dt, &stream_utc, sizeof(stream_utc));
}
#ifdef SUPPORT_TOT_PARSE

/*
 *  deprecate: It is recommened that do not use any more
 */
INT8 is_time_offset_delay_flag(void)
{
	return tot_delay_flag;
}

/*
 *  deprecate: It is recommened that do not use any more
 */

/*
 *  deprecate: It is recommened that do not use any more
 */
void set_time_offset_delay_flag(void)
{
    tot_delay_flag = TRUE;
}

/*
 *  deprecate: It is recommened that do not use any more
 */
void reset_time_offset_delay_flag(void)
{
    tot_delay_flag = FALSE;
}

INT32 is_time_offset_inited(void)
{
	return time_offset_inited;
}

void enable_time_offset_parse(void)
{
    time_offset_inited = FALSE;
}

void disable_time_offset_parse(void)
{
    time_offset_inited = TRUE;
}

// this part makes the time change accuratly
void set_time_of_change( date_time * tm )
{
	Time_Of_Change = *tm;
}

void set_next_time_offset( INT32 hour, INT32 min )
{
	next_hour_offset = hour;
	next_min_offset = min;
}

static INT32 _get_time_offset_from_desc( UINT8 *buffer, date_time TOT_utc, UINT32 country_code)
{
	INT8 flag;
	UINT8 h, m, s;
	UINT8 country_region_id,local_time_offset_polarity;
	UINT16 local_time_offset;
	INT32 local_offset_hour, local_offset_min;
	INT32 next_offset_hour, next_offset_min;
	date_time utc;
	date_time time_of_change;

	country_region_id = (*(buffer+3) & 0xFC)>>2;
	local_time_offset_polarity = *(buffer+3) & 0x01;
	local_time_offset = (*(buffer+4)<<8) | (*(buffer+5));
	bcd_to_hm(buffer+4,&h, &m );
	local_offset_hour = (INT32)h;
	local_offset_min  = (INT32)m;

	time_of_change.mjd =  (*(buffer+6))<<8 | (*(buffer+7));
	mjd_to_ymd( time_of_change.mjd, &time_of_change.year, &time_of_change.month,\
				&time_of_change.day,&time_of_change.weekday );
	bcd_to_hms( buffer+8, &time_of_change.hour, &time_of_change.min, \
				&time_of_change.sec ); 

	bcd_to_hm( buffer+11, &h, &m );
	next_offset_hour = (INT32)h;
	next_offset_min  = (INT32)m;

	if(country_region_id>60)
		return !SUCCESS;
	flag = (local_time_offset_polarity == 1) ? -1 : 1;
	
	// if tot_utc<time_of_change, means local_time_offset valid here, and should use it
	// tot_utc>=time_of_change, means we should use the next_time_offset
	if( date_time_compare(&time_of_change, &TOT_utc) ) // time_of_change - ToT_utc
	{
		set_STC_offset( local_offset_hour*flag, local_offset_min*flag, 0 );
		set_UTC_offset( local_offset_hour*flag, local_offset_min*flag, 0 );	// compatible with old code	
	}
	else 
	{
		set_STC_offset( next_offset_hour*flag, next_offset_min*flag, 0 );
		set_UTC_offset( next_offset_hour*flag, next_offset_min*flag, 0 );// compatible with old code
	}
	get_UTC( &utc );
	set_STC_time( &utc );

	toc_cntry_code = country_code;
	set_time_of_change( &time_of_change );
	set_next_time_offset( next_offset_hour*flag, next_offset_min*flag );
	return SUCCESS;
}

void init_time_offset_from_stream(UINT8* buff)
{
	UINT8 *p_tmp_buffer;
	UINT8 low, high, des_tag, des_len;
	UINT16 length=0;
	UINT32 country_code;
	date_time TOT_utc;

	if ((is_time_offset_inited()!=TRUE)&&((*buff) == PSI_TOT_TABLE_ID))
	{
		TOT_utc.mjd= (*(buff+3))<<8 | (*(buff+4));
		bcd_to_hms((buff+5),&TOT_utc.hour, &TOT_utc.min, &TOT_utc.sec);			
		mjd_to_ymd(TOT_utc.mjd,&TOT_utc.year,&TOT_utc.month,&TOT_utc.day,&TOT_utc.weekday);

		high=(*(buff+8)) & 0x0F;
		low=*(buff+9);
		length=high<<8 | low; // desc loop length
		p_tmp_buffer = (buff+10); // desc loop
		while(length >0) //for mutilple desc parser
		{
			des_tag = *p_tmp_buffer;
			des_len = *(p_tmp_buffer+1);
			p_tmp_buffer += 2;
			length -= (des_len+2);
			while( des_len >= 13 ) // one desc
			{
				country_code = (*(p_tmp_buffer)<<16) | (*(p_tmp_buffer+1)<<8) | (*(p_tmp_buffer+2));
				TDT_PRINTF("%s, cntry code:0x%x, sys:0x%x\n", __FUNCTION__, country_code, sys_country_code );
				if( country_code == sys_country_code )
				{
					_get_time_offset_from_desc( p_tmp_buffer, TOT_utc, country_code );
				}
				des_len -= 13;
				p_tmp_buffer += 13;
			}
		}
	}
}
#endif


static BOOL  tdt_in_parsing = FALSE;
static UINT8 tdt_buffer[PSI_SHORT_SECTION_LENGTH];
#ifdef SUPPORT_TOT_PARSE
static UINT8 tot_buffer[PSI_SHORT_SECTION_LENGTH];
#endif
const struct tdt_filter_info filter_info[] = {
	{
		.table_id 	=	PSI_TDT_TABLE_ID,
		.mask_value	=	{
							.mask		=	{ 0xFF, },
							.value[0][0]=	PSI_TDT_TABLE_ID,
							.mask_len	=	1,
							.value_num	=	1,
							.multi_mask	=	{ 0, },
							.tb_flt_msk	=	0,
						},
		.buff		=	tdt_buffer,
		.bufflen	=	PSI_SHORT_SECTION_LENGTH,
	},
#ifdef SUPPORT_TOT_PARSE
	{
		.table_id 	=	PSI_TOT_TABLE_ID,
		.mask_value	=	{
							.mask		=	{ 0xFF, },
							.value[0][0]=	PSI_TOT_TABLE_ID,
							.mask_len	=	1,
							.value_num	=	1,
							.multi_mask	=	{ 0, },
							.tb_flt_msk	=	0,
						},
		.buff		=	tot_buffer,
		.bufflen	=	PSI_SHORT_SECTION_LENGTH,
	},
#endif
};

static sie_status_t tdt_parser(UINT16 pid,struct si_filter_t *filter, UINT8 reason, UINT8 *buffer, INT32 length)
{

	if((buffer == NULL) || (reason == SIE_REASON_FILTER_TIMEOUT))
		return sie_started;

	set_time_from_stream(buffer);
	if( (need_sync_time() || (time_inited == FALSE)) 
		&& is_time_parse_enable() != FALSE )
	{
		enable_time_parse();
		init_time_from_stream(buffer);
		TDT_PRINTF("Update Time from Stream\n");
	}
#ifdef SUPPORT_TOT_PARSE
	init_time_offset_from_stream(buffer);
#endif
	return sie_started;
}



INT32 start_tdt()
{
	INT32 ret;
	//struct dmx_device *dev = NULL;
	struct si_filter_t *filter = NULL;
	struct si_filter_param fparam;
	UINT8 i;
#ifdef SI_SUPPORT_MUTI_DMX	
	UINT8 dmx_id=0;		
	UINT8 nim_id=0;	
	if(lib_nimng_get_nim_play()==0)
		dmx_id=0;
	else	
		dmx_id=(lib_nimng_get_nim_play()-1);
	
	struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
#endif	

	if (tdt_in_parsing)
       	return SUCCESS;

	MEMSET(&tdt_buffer,0,PSI_SHORT_SECTION_LENGTH);
	MEMSET(&fparam,0,sizeof(struct si_filter_param));

	//dev = dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
	//if (NULL==dev)
	//	return !SUCCESS;
	
	//ret = sie_open(dev, PSI_MODULE_CONCURRENT_FILTER, NULL, 0);
	for( i=0; i<sizeof(filter_info)/sizeof(struct tdt_filter_info); i++)
	{
		#ifdef SI_SUPPORT_MUTI_DMX
		filter = sie_alloc_filter_ext(dmx,PSI_TDT_PID, filter_info[i].buff, filter_info[i].bufflen, PSI_SHORT_SECTION_LENGTH,0);
		#else
		filter = sie_alloc_filter(PSI_TDT_PID, filter_info[i].buff, filter_info[i].bufflen, PSI_SHORT_SECTION_LENGTH);
		#endif
		if(NULL == filter)
		{
			PRINTF("%s(): alloc filter failed!\n", __FUNCTION__);
			return !SUCCESS;
		}
		fparam.timeout = OSAL_WAIT_FOREVER_TIME;
		fparam.mask_value = filter_info[i].mask_value;
		fparam.section_event = NULL;
		fparam.section_parser = (si_handler_t)tdt_parser;
		sie_config_filter(filter, &fparam);
		ret = sie_enable_filter(filter);
		if(ret != SI_SUCCESS)
		{
			PRINTF("%s(): enable filter failed!\n", __FUNCTION__);
			return !SUCCESS;
		}
	}
	tdt_in_parsing=TRUE;
	return SUCCESS;
}


INT32 stop_tdt()
{
	UINT8 i;
	INT32 ret;
	#ifdef SI_SUPPORT_MUTI_DMX
	UINT8 dmx_id=0;
	if(lib_nimng_get_nim_play()==0)
		dmx_id=0;
	else	
		dmx_id=(lib_nimng_get_nim_play()-1);

	struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, dmx_id);
	#endif	

	if (!tdt_in_parsing)
		return SUCCESS;

	ret = SUCCESS;
	for( i=0; i<sizeof(filter_info)/sizeof(struct tdt_filter_info); i++)
	{
		#ifdef SI_SUPPORT_MUTI_DMX
		 	if( SI_SUCCESS != sie_abort_ext( dmx, NULL, PSI_TDT_PID, (struct restrict*)(&(filter_info[i].mask_value)) ) )
		#else
	   		if( SI_SUCCESS != sie_abort( PSI_TDT_PID, (struct restrict*)(&(filter_info[i].mask_value) )) )
		#endif
			ret = !SUCCESS;
	}

	if( ret == !SUCCESS )
		return ret;
	
	tdt_in_parsing=FALSE;
	return SUCCESS;
}


void tdt_register_callback(TDT_CALL_BACK cb)
{
	tdt_cb = cb;
}

/* if the interval of time between UTC and stream_utc is larger than 1 min, we should  
 * call init_time_from_stream to sync these two time.
 */
static BOOL need_sync_time()
{
#ifdef _SYNC_TIME_ENABLE_
	if(UTC.year != stream_utc.year)  return TRUE;
	if(UTC.month != stream_utc.month)  return TRUE;
	if(UTC.day != stream_utc.day)  return TRUE;
	if(UTC.hour != stream_utc.hour)  return TRUE;
	if((UTC.min > stream_utc.min) && (UTC.min - stream_utc.min >= 1))
		return TRUE;
	if((UTC.min < stream_utc.min) && (stream_utc.min - UTC.min >= 1))
		return TRUE;
#endif
	return FALSE;
		
}

