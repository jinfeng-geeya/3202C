/*****************************************************************************
*    Ali Corp. All Rights Reserved. 2006 Copyright (C)
*
*    File:    time.c
*
*    Description:    This file contains all functions definition
*		             of memory operations.
*    History:
*           Date            Athor        Version          Reason
*	    ============	=============	=========	=================
*	1.	Feb.10.2006       Justin Wu       Ver 0.1    Create file.
*	2.
*****************************************************************************/

#include <osal/osal.h>
#include <api/libc/time.h>


static long time_base = 0;

time_t time(time_t *t)
{
	time_t time_sec;

	time_sec = time_base + osal_get_time();
	if (t != NULL)
		*t = time_sec;

	return time_sec;
}


void setime(time_t t)
{
	time_base = t - osal_get_time();
}

/* Function: struct tm *gmtime_r(const time_t *clock, struct tm * res) */
#define SECSPERMIN	60L
#define MINSPERHOUR	60L
#define HOURSPERDAY	24L
#define SECSPERHOUR	(SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY	(SECSPERHOUR * HOURSPERDAY)

#define DAYSPERWEEK	7
#define MONSPERYEAR	12
#define YEAR_BASE	1900
#define EPOCH_YEAR	1970
#define EPOCH_WDAY	4

#define isleap(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)

static const int mon_lengths[2][MONSPERYEAR] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}} ;

static const int year_lengths[2] = {365, 366};

struct tm *gmtime_r(const time_t *tim_p, struct tm * res)
{
	long days, rem;
	time_t lcltime;
	int i, y, yleap;
	const int *ip;

	/* Base decision about std/dst time on current time */
	lcltime = *tim_p;

	days = ((long)lcltime) / SECSPERDAY;
	rem = ((long)lcltime) % SECSPERDAY;
	while (rem < 0)
	{
		rem += SECSPERDAY;
		--days;
	}
	while (rem >= SECSPERDAY)
	{
		rem -= SECSPERDAY;
		++days;
	}

	/* Compute hour, min, and sec */
	res->tm_hour = (int) (rem / SECSPERHOUR);
	rem %= SECSPERHOUR;
	res->tm_min = (int) (rem / SECSPERMIN);
	res->tm_sec = (int) (rem % SECSPERMIN);

	/* Compute day of week */
	if ((res->tm_wday = ((EPOCH_WDAY + days) % DAYSPERWEEK)) < 0)
		res->tm_wday += DAYSPERWEEK;

	/* Compute year & day of year */
	y = EPOCH_YEAR;
	if (days >= 0)
	{
		while (1)
		{
			yleap = isleap(y);
			if (days < year_lengths[yleap])
				break;
			y++;
			days -= year_lengths[yleap];
		}
	} else
    {
		do
		{
			--y;
			yleap = isleap(y);
			days += year_lengths[yleap];
		} while (days < 0);
	}

	res->tm_year = y - YEAR_BASE;
	res->tm_yday = days;
	ip = mon_lengths[yleap];
	for (res->tm_mon = 0; days >= ip[res->tm_mon]; ++res->tm_mon)
		days -= ip[res->tm_mon];
	res->tm_mday = days + 1;

	res->tm_isdst = 0;

	return (res);
}

struct tm cur_gm_tm;
struct tm *gmtime(const time_t *tim_p)
{
	gmtime_r(tim_p, &cur_gm_tm);
	return (&cur_gm_tm);
}


/* Function: time_t mktime(struct tm * tim_p) */
#define _SEC_IN_MINUTE	60L
#define _SEC_IN_HOUR	3600L
#define _SEC_IN_DAY		86400L

static const int DAYS_IN_MONTH[12] =
{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define _DAYS_IN_MONTH(x) ((x == 1) ? days_in_feb : DAYS_IN_MONTH[x])

static const int _DAYS_BEFORE_MONTH[12] =
{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

#define _ISLEAP(y) (((y) % 4) == 0 && (((y) % 100) != 0 || (((y)+1900) % 400) == 0))
#define _DAYS_IN_YEAR(year) (_ISLEAP(year) ? 366 : 365)

time_t mktime(struct tm * tim_p)
{
	time_t tim = 0;
	long days = 0;
	int year, isdst;

	/* Compute hours, minutes, seconds */
	tim += tim_p->tm_sec + (tim_p->tm_min * _SEC_IN_MINUTE) + (tim_p->tm_hour * _SEC_IN_HOUR);

	/* Compute days in year */
	days += tim_p->tm_mday - 1;
	if(tim_p->tm_mon>=12)
	{
		tim_p->tm_year += tim_p->tm_mon/12;
		tim_p->tm_mon %= 12;
	}
	days += _DAYS_BEFORE_MONTH[tim_p->tm_mon];
	if (tim_p->tm_mon > 1 && _DAYS_IN_YEAR (tim_p->tm_year) == 366)
		days++;

	/* Compute day of the year */
	tim_p->tm_yday = days;

	if (tim_p->tm_year > 10000 || tim_p->tm_year < -10000)
	{
		return (time_t) -1;
	}

	/* Compute days in other years */
	if (tim_p->tm_year > 70)
	{
		for (year = 70; year < tim_p->tm_year; year++)
			days += _DAYS_IN_YEAR (year);
	} else if (tim_p->tm_year < 70)
	{
		for (year = 69; year > tim_p->tm_year; year--)
			days -= _DAYS_IN_YEAR (year);
		days -= _DAYS_IN_YEAR (year);
    }

	/* Compute day of the week */
	if ((tim_p->tm_wday = (days + 4) % 7) < 0)
		tim_p->tm_wday += 7;

	//check overflow
	if((unsigned long)tim + (unsigned long)(days * _SEC_IN_DAY)>0x1FFFFFFFu)
		return -1;

	/* Compute total seconds */
	tim += (days * _SEC_IN_DAY);

	return tim;
}
struct tm cur_local_tm;
struct tm *localtime(const time_t *timer)
{
	*((long*)timer) -= 8 * 3600;
    gmtime_r(timer, &cur_local_tm);
	return (&cur_local_tm);
}

void tzset()
{
    ;
}

