#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "timey.h"

#define TIMEY_TIME_STR_LEN 9
#define TIMEY_DATE_STR_LEN 11

// obtain and format time
static void fmt_time(char *buffer, size_t size, const char *fmt)
{
	time_t raw = 0;
	struct tm *time_info;

	time(&raw);

	time_info = localtime(&raw);

	strftime(buffer, size, fmt, time_info);
}
// obtain full string after formatting
static void get_fmt_str(char *buffer, size_t size, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, size, fmt, args);
	va_end(args);
}
void timey_query_raw_time(timey_raw_time *t)
{
	if(!t)
		return;

	struct timespec ts;
	timespec_get(&ts, TIME_UTC);

	t->sec = ts.tv_sec;
	t->millisec = t->sec * 1000LL;
	t->microsec = t->sec * 1000000LL;
	t->nanosec = t->sec * 1000000000LL;
}
void timey_query_time(char *buffer, size_t buffer_size)
{
	if(!buffer)
		return;

	fmt_time(buffer, buffer_size, "%H:%M:%S");
}
void timey_query_date(char *buffer, size_t buffer_size)
{
	if(!buffer)
		return;

	fmt_time(buffer, buffer_size, "%m-%d-%Y");
}

// array of months, where the index corresponds to the month number - 1
static const char *const FULL_MONTH_NAMES[12] = {
	"January", "February", "March", "April", "May", "June",
	"July", "August", "September", "October", "November", "December"
};
static const char *const ABBREV_MONTH_NAMES[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
void timey_query_month_name(char *buffer, size_t buffer_size, unsigned char month_num, bool use_abbreviated_name)
{
	if(month_num < 1 || month_num > 12 || !buffer || buffer_size == 0)
		return;

	const char *month_name = NULL;
	if(use_abbreviated_name)
		month_name = ABBREV_MONTH_NAMES[month_num - 1];
	else
		month_name = FULL_MONTH_NAMES[month_num - 1];

	snprintf(buffer, buffer_size, "%s", month_name);
}
#define DAYS_IN_WEEK 7
static const char *const FULL_DAY_NAMES[DAYS_IN_WEEK] = {
	"Sunday", "Monday", "Tuesday",
	"Wednesday", "Thursday", "Friday", "Saturday"
};
static const char *const ABBREV_DAY_NAMES[DAYS_IN_WEEK] = {
	"Sun", "Mon", "Tue", "Wed",
	"Thu", "Fri", "Sat"
};
void timey_query_day_name(char *buffer, size_t buffer_size, unsigned char day_num, bool is_wday, bool use_abbreviated_name)
{
	if(is_wday && (day_num < 1 || day_num > DAYS_IN_WEEK))
		return;
	if(!is_wday && (day_num < 1 || day_num > 31))
		return;
	if(!buffer || buffer_size == 0)
		return;

	const char *day_name = NULL;

	// if using 1-7 for weekdays, just use the same logic from query_month_name(...)
	if(is_wday)
	{
		if(use_abbreviated_name)
			day_name = ABBREV_DAY_NAMES[day_num - 1];
		else
			day_name = FULL_DAY_NAMES[day_num - 1];
	}
	else // else if using 1-31 for actual days of the month, create a datetime object and obtain the day name from that instead
	{
		timey_datetime now = timey_curr_datetime();
		timey_set_day(&now, day_num);

		// now.day_name will be the full day name
		day_name = now.day_name;

		// see if the abbreviated name should be used instead
		if(use_abbreviated_name)
		{
			// find matching name in the ABBREV_DAY_NAMES array
			for(size_t i = 0; i < DAYS_IN_WEEK; ++i)
			{
				// when the first 3 chars match, use the name in the ABBREV_DAY_NAMES array at that index
				if(strncmp(ABBREV_DAY_NAMES[i], day_name, 3) == 0)
				{
					day_name = ABBREV_DAY_NAMES[i];
					break;
				}
			}
		}
	}

	snprintf(buffer, buffer_size, "%s", day_name);
}

timey_timestamp timey_curr_timestamp()
{
	timey_timestamp ts = {0};

	// obtain time
	char time[TIMEY_TIME_STR_LEN];
	timey_query_time(time, sizeof time);

	// obtain specific time components from string
	char hour[3], min[3], sec[3];

	memcpy(hour, time, 2);
	hour[2] = '\0';

	memcpy(min, time + 3, 2);
	min[2] = '\0';

	memcpy(sec, time + 6, 2);
	sec[2] = '\0';

	ts.hour24 = strtoul(hour, NULL, 10);
	// convert hour24 to hour12
	ts.hour12 = ts.hour24;
	if(ts.hour12 > 12)
	{
		ts.hour12 -= 12;
		strcpy(ts.period, "PM");
	}
	else
		strcpy(ts.period, "AM");
	ts.min = strtoul(min, NULL, 10);
	ts.sec = strtoul(sec, NULL, 10);

	return ts;
}
static void switch_period(timey_timestamp *ts)
{
	if(strcmp("AM", ts->period) == 0)
		strcpy(ts->period, "PM");
	else
		strcpy(ts->period, "AM");
}
static void goto_next_hour(timey_timestamp *ts)
{
	ts->min -= 60;
	ts->hour24 += 1;
	ts->hour12 += 1;
	if(ts->hour24 > 24)
		ts->hour24 -= 24;
	if(ts->hour12 > 12)
	{
		ts->hour12 -= 12;
		switch_period(ts);
	}
}
timey_timestamp timey_future_timestamp(timey_timestamp *now, unsigned int hours, unsigned int minutes, unsigned int seconds)
{
	timey_timestamp future = {0};

	if(!now)
		return future;

	future = *now;

	// add hours
	future.hour24 += hours;
	// continue to loop back 24 hours if necessary
	while(future.hour24 > 24)
		future.hour24 -= 24;
	// do the same for hour12
	future.hour12 += hours;
	while(future.hour12 > 12)
	{
		future.hour12 -= 12;
		switch_period(&future);
	}

	// add minutes
	future.min += minutes;
	// continue to loop back an hour (60 minutes) if necessary
	while(future.min > 60)
		goto_next_hour(&future);

	// add seconds
	future.sec += seconds;
	// continue to loop back a minute and hour if necessary
	while(future.sec > 60)
	{
		future.sec -= 60;
		future.min += 1;
		if(future.min > 60)
			goto_next_hour(&future);
	}

	return future;
}
timey_timestamp timey_future_timestamp_from_now(unsigned int hours, unsigned int minutes, unsigned int seconds)
{
	timey_timestamp now = timey_curr_timestamp();
	return timey_future_timestamp(&now, hours, minutes, seconds);
}
timey_timestamp timey_create_timestamp(unsigned char hours, unsigned char minutes, unsigned char seconds)
{
	struct tm time_result = {0};
	time_result.tm_hour = hours - 1;
	time_result.tm_min = minutes - 1;
	time_result.tm_sec = seconds - 1;

	if(mktime(&time_result) == -1)
		return (timey_timestamp) {0};

	timey_timestamp ts = {0};
	ts.min = minutes;
	ts.sec = seconds;
	ts.hour24 = hours;
	if(hours > 12)
	{
		ts.hour12 = hours - 12;
		strcpy(ts.period, "PM");
	}
	else
		strcpy(ts.period, "AM");

	return ts;
}

static void set_month_name(timey_datetime *dt)
{
	switch(dt->month)
	{
		case 1:
			strcpy(dt->month_name, "January");
			break;
		case 2:
			strcpy(dt->month_name, "February");
			break;
		case 3:
			strcpy(dt->month_name, "March");
			break;
		case 4:
			strcpy(dt->month_name, "April");
			break;
		case 5:
			strcpy(dt->month_name, "May");
			break;
		case 6:
			strcpy(dt->month_name, "June");
			break;
		case 7:
			strcpy(dt->month_name, "July");
			break;
		case 8:
			strcpy(dt->month_name, "August");
			break;
		case 9:
			strcpy(dt->month_name, "September");
			break;
		case 10:
			strcpy(dt->month_name, "October");
			break;
		case 11:
			strcpy(dt->month_name, "November");
			break;
		case 12:
			strcpy(dt->month_name, "December");
			break;
	}
}
static void set_day_name_dt(timey_datetime *dt, int wday)
{
	switch(wday)
	{
		case 0:
			strcpy(dt->day_name, "Sunday");
			break;
		case 1:
			strcpy(dt->day_name, "Monday");
			break;
		case 2:
			strcpy(dt->day_name, "Tuesday");
			break;
		case 3:
			strcpy(dt->day_name, "Wednesday");
			break;
		case 4:
			strcpy(dt->day_name, "Thursday");
			break;
		case 5:
			strcpy(dt->day_name, "Friday");
			break;
		case 6:
			strcpy(dt->day_name, "Saturday");
			break;
	}
}
static void update_datetime(timey_datetime *dt)
{
	struct tm time_result = {0};
	time_result.tm_mday = dt->day;
	time_result.tm_mon = dt->month - 1;
	time_result.tm_year = dt->year - 1900;

	if(mktime(&time_result) == -1)
		return;

	set_day_name_dt(dt, time_result.tm_wday);
}
static void set_curr_day_name_dt(timey_datetime *dt)
{
	time_t raw = 0;
	struct tm *time_info;

	time(&raw);

	time_info = localtime(&raw);

	int wday = time_info->tm_wday; // days since Sunday (0 - 6)

	set_day_name_dt(dt, wday);
}
timey_datetime timey_curr_datetime()
{
	timey_datetime dt = {0};

	// obtain date
	char date[TIMEY_DATE_STR_LEN];
	timey_query_date(date, sizeof date);

	// obtain specific date-time components from string
	char month[3], day[3], year[5];

	memcpy(month, date, 2);
	month[2] = '\0';

	memcpy(day, date + 4, 2);
	day[2] = '\0';

	memcpy(year, date + 6, 4);
	year[4] = '\0';

	dt.time = timey_curr_timestamp();

	dt.year = strtoul(year, NULL, 10);
	dt.month = strtoul(month, NULL, 10);
	set_month_name(&dt);
	dt.day = strtoul(day, NULL, 10);
	set_curr_day_name_dt(&dt);
	dt.is_leap_year = timey_is_leap_year(dt.year);

	return dt;
}
static void goto_next_year(timey_datetime *dt)
{
	dt->month -= 12;
	dt->year += 1;
	dt->is_leap_year = timey_is_leap_year(dt->year);
}
static void add_days_dt(timey_datetime *dt, unsigned int days)
{
	dt->day += days;
	unsigned int max_month_days = timey_num_days_in_month_dt(dt);
	while(dt->day > max_month_days)
	{
		dt->day -= max_month_days;
		dt->month += 1;
		if(dt->month > 12)
			goto_next_year(dt);
		max_month_days = timey_num_days_in_month_dt(dt);
	}
}
timey_datetime timey_future_datetime(timey_datetime *now, unsigned int years, unsigned int months, unsigned int days, unsigned int hours, unsigned int minutes, unsigned int seconds)
{
	timey_datetime future = {0};

	if(!now)
		return future;

	future = *now;

	// add years
	future.year += years;

	// add months
	future.month += months;
	// continue to increment year if necessary
	while(future.month > 12)
		goto_next_year(&future);

	// add days
	add_days_dt(&future, days);

	// add to timestamp (while tracking date-time components this time)
	timey_timestamp *ts = &future.time;

	ts->hour24 += hours;
	while(ts->hour24 > 24)
	{
		ts->hour24 -= 24;
		add_days_dt(&future, 1);
	}
	ts->hour12 += hours;
	if(ts->hour12 == 12)
	{
		switch_period(ts);
		add_days_dt(&future, 1);
	}
	while(ts->hour12 > 12)
	{
		ts->hour12 -= 12;
		switch_period(ts);
	}

	ts->min += minutes;
	while(ts->min > 60)
	{
		ts->min -= 60;
		ts->hour24 += 1;
		ts->hour12 += 1;
		if(ts->hour24 > 24)
		{
			ts->hour24 -= 24;
			add_days_dt(&future, 1);
		}
		if(ts->hour12 > 12)
		{
			ts->hour12 -= 12;
			switch_period(ts);
		}
	}

	ts->sec += seconds;
	while(ts->sec > 60)
	{
		ts->sec -= 60;
		ts->min += 1;
		if(ts->min > 60)
		{
			ts->min -= 60;
			ts->hour24 += 1;
			ts->hour12 += 1;
			if(ts->hour24 > 24)
			{
				ts->hour24 -= 24;
				add_days_dt(&future, 1);
			}
			if(ts->hour12 > 12)
			{
				ts->hour12 -= 12;
				switch_period(ts);
			}
		}
	}

	// update day and month names
	set_month_name(&future);
	update_datetime(&future);

	return future;
}
timey_datetime timey_future_datetime_from_now(unsigned int years, unsigned int months, unsigned int days, unsigned int hours, unsigned int minutes, unsigned int seconds)
{
	timey_datetime now = timey_curr_datetime();
	return timey_future_datetime(&now, years, months, days, hours, minutes, seconds);
}

bool timey_is_leap_year(unsigned int year)
{
	return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}
static const unsigned char DAY_NUMS[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
unsigned char timey_num_days_in_month(unsigned char month)
{
	if(month < 0 || month > 11)
		return 0;
	return DAY_NUMS[month - 1];
}
unsigned char timey_num_days_in_month_dt(timey_datetime *dt)
{
	if(!dt)
		return 0;

	if(dt->month == 1 || dt->month == 3 || dt->month == 5
			|| dt->month == 7 || dt->month == 8 || dt->month == 10
			|| dt->month == 12)
		return 31;
	else if(dt->month == 2)
		return dt->is_leap_year ? 29 : 28;
	else
		return 30;
}

void timey_set_day(timey_datetime *dt, unsigned char day)
{
	if(!dt || day < 1 || day > 31)
		return;

	dt->day = day;

	update_datetime(dt);
}
void timey_set_month(timey_datetime *dt, unsigned char month)
{
	if(!dt || month < 1 || month > 12)
		return;

	dt->month = month;

	update_datetime(dt);
}
void timey_set_year(timey_datetime *dt, unsigned char year)
{
	if(!dt)
		return;

	dt->year = year;

	update_datetime(dt);
}
