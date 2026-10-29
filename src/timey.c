#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "timey.h"

#define TIME_STR_LEN 9
#define DATE_STR_LEN 11
#define TM_YEAR_START 1900

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
	char time[TIME_STR_LEN];
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

static void detect_period(timey_timestamp *ts)
{
	// see where hour24 is
	if(ts->hour24 < 12)
		strcpy(ts->period, "AM");
	else if(ts->hour24 >= 12)
		strcpy(ts->period, "PM");
}
static void goto_next_hour(timey_timestamp *ts)
{
	ts->min -= 60;
	ts->hour24 += 1;
	ts->hour12 += 1;
	if(ts->hour24 > 24)
		ts->hour24 -= 24;
	if(ts->hour12 > 12)
		ts->hour12 -= 12;
}
static void goto_last_hour(timey_timestamp *ts)
{
	ts->min += 60;
	ts->hour24 -= 1;
	ts->hour12 -= 1;
	if(ts->hour24 <= 0)
		ts->hour24 += 24;
	if(ts->hour12 <= 0)
		ts->hour12 += 12;
}
timey_timestamp timey_future_timestamp(const timey_timestamp *start, unsigned long long hours, unsigned long long minutes, unsigned long long seconds)
{
	timey_timestamp future = {0};

	if(!start)
		return future;

	future = *start;

	// add hours
	future.hour24 += hours;
	// continue to loop back 24 hours if necessary
	while(future.hour24 > 24)
		future.hour24 -= 24;
	// do the same for hour12
	future.hour12 += hours;
	while(future.hour12 > 12)
		future.hour12 -= 12;

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

	detect_period(&future);

	return future;
}
timey_timestamp timey_future_timestamp_from_now(unsigned long long hours, unsigned long long minutes, unsigned long long seconds)
{
	timey_timestamp now = timey_curr_timestamp();
	return timey_future_timestamp(&now, hours, minutes, seconds);
}
timey_timestamp timey_past_timestamp(const timey_timestamp *start, unsigned long long hours, unsigned long long minutes, unsigned long long seconds)
{
	timey_timestamp past = {0};

	if(!start)
		return past;

	past = *start;

	// work on hours first:
	past.hour24 -= hours;
	while(past.hour24 <= 0)
		past.hour24 += 24;
	past.hour12 -= hours;
	while(past.hour12 <= 0)
		past.hour12 += 12;

	// then minutes:
	past.min -= minutes;
	while(past.min <= 0)
		goto_last_hour(&past);

	// then seconds:
	past.sec -= seconds;
	while(past.sec <= 0)
	{
		past.sec += 60;
		past.min -= 1;
		if(past.min <= 0)
			goto_last_hour(&past);
	}

	detect_period(&past);

	return past;
}
timey_timestamp timey_past_timestamp_from_now(unsigned long long hours, unsigned long long minutes, unsigned long long seconds)
{
	timey_timestamp now = timey_curr_timestamp();
	return timey_past_timestamp(&now, hours, minutes, seconds);
}
timey_timestamp timey_create_timestamp(unsigned long long hour, unsigned long long minute, unsigned long long second)
{
	timey_timestamp ts = {0};

	if(hour > 24 || minute > 59 || second > 59)
		return ts;

	ts.min = minute;
	ts.sec = second;
	ts.hour24 = hour;
	if(hour > 12)
	{
		ts.hour12 = hour - 12;
		strcpy(ts.period, "PM");
	}
	else
		strcpy(ts.period, "AM");

	return ts;
}

static void set_month_name_dt(timey_datetime *dt)
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
static void set_curr_day_name_dt(timey_datetime *dt)
{
	time_t raw = 0;
	struct tm *time_info;

	time(&raw);

	time_info = localtime(&raw);

	int wday = time_info->tm_wday; // days since Sunday (0 - 6)

	set_day_name_dt(dt, wday);
}
static void update_datetime(timey_datetime *dt)
{
	struct tm time_result = {0};
	time_result.tm_mday = dt->day;
	time_result.tm_mon = dt->month - 1;
	time_result.tm_year = dt->year - TM_YEAR_START;
	time_result.tm_hour = dt->time.hour24 - 1;
	time_result.tm_min = dt->time.min - 1;
	time_result.tm_sec = dt->time.sec - 1;

	if(mktime(&time_result) == -1)
	{
		// invalidate datetime
		*dt = (timey_datetime) {0};
		return;
	}

	set_day_name_dt(dt, time_result.tm_wday);
	set_month_name_dt(dt);
}
timey_datetime timey_curr_datetime()
{
	timey_datetime dt = {0};

	// obtain date
	char date[DATE_STR_LEN];
	timey_query_date(date, sizeof date);

	// obtain specific date-time components from string
	char month[3], day[3], year[5];

	memcpy(month, date, 2);
	month[2] = '\0';

	memcpy(day, date + 3, 2);
	day[2] = '\0';

	memcpy(year, date + 6, 4);
	year[4] = '\0';

	dt.time = timey_curr_timestamp();

	dt.year = strtoul(year, NULL, 10);
	dt.month = strtoul(month, NULL, 10);
	set_month_name_dt(&dt);
	dt.day = strtoul(day, NULL, 10);
	set_curr_day_name_dt(&dt);
	dt.is_leap_year = timey_is_leap_year(dt.year);

	return dt;
}
static void goto_next_year(timey_datetime *dt)
{
	dt->month -= 12;
	dt->year += 1;
}
static void goto_last_year(timey_datetime *dt)
{
	dt->month += 12;
	dt->year -= 1;
}
static void add_days_dt(timey_datetime *dt, unsigned long long days)
{
	dt->day += days;
	unsigned char max_month_days = timey_num_days_in_month_dt(dt);
	while(dt->day > max_month_days)
	{
		dt->day -= max_month_days;
		dt->month += 1;
		if(dt->month > 12)
			goto_next_year(dt);
		max_month_days = timey_num_days_in_month_dt(dt);
	}
}
static void minus_days_dt(timey_datetime *dt, unsigned long long days)
{
	dt->day -= days;
	unsigned char max_month_days = timey_num_days_in_month_dt(dt);
	while(dt->day <= 0)
	{
		// get the new max number of days for the new month
		dt->month -= 1;
		if(dt->month <= 0)
			goto_last_year(dt);
		max_month_days = timey_num_days_in_month_dt(dt);

		// now add back days
		dt->day += max_month_days;
	}
}
timey_datetime timey_future_datetime(const timey_datetime *start, unsigned long long years, unsigned long long months, unsigned long long days, unsigned long long hours, unsigned long long minutes, unsigned long long seconds)
{
	timey_datetime future = {0};

	if(!start)
		return future;

	future = *start;

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
		add_days_dt(&future, 1);
	while(ts->hour12 > 12)
		ts->hour12 -= 12;

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
			ts->hour12 -= 12;
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
				ts->hour12 -= 12;
		}
	}

	// update day and month names, as well as period
	update_datetime(&future);
	detect_period(ts);
	future.is_leap_year = timey_is_leap_year(future.year);

	return future;
}
timey_datetime timey_future_datetime_from_now(unsigned long long years, unsigned long long months, unsigned long long days, unsigned long long hours, unsigned long long minutes, unsigned long long seconds)
{
	timey_datetime now = timey_curr_datetime();
	return timey_future_datetime(&now, years, months, days, hours, minutes, seconds);
}
timey_datetime timey_past_datetime(const timey_datetime *start, unsigned long long years, unsigned long long months, unsigned long long days, unsigned long long hours, unsigned long long minutes, unsigned long long seconds)
{
	timey_datetime past = {0};

	if(!start)
		return past;

	past = *start;

	// just take away years
	past.year -= years;

	// work on months:
	past.month -= months;
	while(past.month <= 0)
		goto_last_year(&past);

	// take away days
	minus_days_dt(&past, days);

	// work on timestamp
	timey_timestamp *ts = &past.time;

	// hours:
	ts->hour24 -= hours;
	while(ts->hour24 <= 0)
	{
		ts->hour24 += 24;
		minus_days_dt(&past, 1);
	}
	ts->hour12 -= hours;
	if(ts->hour12 == 12)
		minus_days_dt(&past, 1);
	while(ts->hour12 <= 0)
		ts->hour12 += 12;

	// minutes:
	ts->min -= minutes;
	while(ts->min <= 0)
	{
		ts->min += 60;
		ts->hour24 -= 1;
		ts->hour12 -= 1;
		if(ts->hour24 <= 0)
		{
			ts->hour24 += 24;
			minus_days_dt(&past, 1);
		}
		if(ts->hour12 <= 0)
			ts->hour12 += 12;
	}

	// seconds:
	ts->sec -= seconds;
	while(ts->sec <= 0)
	{
		ts->sec += 60;
		ts->min -= 1;
		if(ts->min <= 0)
		{
			ts->min += 60;
			ts->hour24 -= 1;
			ts->hour12 -= 1;
			if(ts->hour24 <= 0)
			{
				ts->hour24 += 24;
				minus_days_dt(&past, 1);
			}
			if(ts->hour12 <= 0)
				ts->hour12 += 12;
		}
	}

	update_datetime(&past);
	detect_period(ts);
	past.is_leap_year = timey_is_leap_year(past.year);

	return past;
}
timey_datetime timey_past_datetime_from_now(unsigned long long years, unsigned long long months, unsigned long long days, unsigned long long hours, unsigned long long minutes, unsigned long long seconds)
{
	timey_datetime now = timey_curr_datetime();
	return timey_past_datetime(&now, years, months, days, hours, minutes, seconds);
}
timey_datetime timey_create_datetime(unsigned long long year, unsigned long long month, unsigned long long day, unsigned long long hour, unsigned long long minute, unsigned long long second)
{
	timey_datetime dt = {0};

	if(month > 12 || day > 31 || hour > 24 || minute > 59 || second > 59)
		return dt;

	dt.time = timey_create_timestamp(hour, minute, second);

	dt.day = day;
	dt.month = month;
	dt.year = year;
	dt.is_leap_year = timey_is_leap_year(year);

	update_datetime(&dt);

	return dt;
}

bool timey_is_leap_year(unsigned long long year)
{
	/*
	   a year is a leap year if any one of the following
	   conditions are satisfied: 

	   the year is multiple of 400.
	   the year is a multiple of 4 and not a multiple of 100.
	*/
	return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
}
static const unsigned char DAY_NUMS[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
unsigned char timey_num_days_in_month(unsigned char month)
{
	if(month < 0 || month > 11)
		return 0;
	return DAY_NUMS[month - 1];
}
unsigned char timey_num_days_in_month_dt(const timey_datetime *dt)
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

bool timey_timestamps_are_equal(const timey_timestamp *ts1, const timey_timestamp *ts2)
{
	if(!ts1 || !ts2)
		return false;

	// if same pointer, they are equal by default
	if(ts1 == ts2)
		return true;

	// check each field on both timestamps:
	return strcmp(ts1->period, ts2->period) == 0 && ts1->hour24 == ts2->hour24 &&
		ts1->hour12 == ts2->hour12 && ts1->min == ts2->min && ts1->sec == ts2->sec;
}
bool timey_datetimes_are_equal(const timey_datetime *dt1, const timey_datetime *dt2)
{
	if(!dt1 || !dt2)
		return false;

	// if same pointer, they are equal by default
	if(dt1 == dt2)
		return true;

	// check each field on both datetimes:
	return timey_timestamps_are_equal(&dt1->time, &dt2->time) &&
		strcmp(dt1->month_name, dt2->month_name) == 0 &&
		strcmp(dt1->day_name, dt2->day_name) == 0 &&
		dt1->year == dt2->year && dt1->month == dt2->month &&
		dt1->day == dt2->day && dt1->is_leap_year == dt2->is_leap_year;
}

timey_stopwatch timey_new_stopwatch()
{
	timey_stopwatch ts = {0};
	timey_reset_stopwatch(&ts);
	return ts;
}
void timey_start_stopwatch(timey_stopwatch *ts)
{
	if(!ts)
		return;
	ts->paused = false;

	// the start time of the stopwatch is the current time
	ts->start_time = timey_curr_timestamp();

	ts->end_time_reached = false;
}
void timey_pause_stopwatch(timey_stopwatch *ts)
{
	if(!ts)
		return;
	ts->paused = true;
}
void timey_reset_stopwatch(timey_stopwatch *ts)
{
	if(!ts)
		return;
	ts->paused = true;
	ts->seconds = 0;
	ts->start_time = (timey_timestamp) {0};
	ts->end_time = (timey_timestamp) {0};
	ts->end_time_reached = false;
}
void timey_update_stopwatch(timey_stopwatch *ts, long double delta_time)
{
	if(!ts || ts->paused)
		return;
	ts->seconds += delta_time;

	// see if stopwatch is using a target time (must be a valid timestamp)
	if(ts->end_time.hour12 > 0 && ts->end_time.min > 0 && ts->end_time.sec > 0)
	{
		// get a new timestamp based on the start time and the number of seconds elapsed
		// !TODO! future_timestamp expects 'seconds' to be unsigned long long, but ts->seconds is a long double
		// !TODO! > make a _prec_ version of the timestamp struct and functions, used for high precision time math
		timey_timestamp future = timey_future_timestamp(&ts->start_time, 0, 0, ts->seconds);

		// compare new timestamp and target timestamp
		if(timey_timestamps_are_equal(&future, &ts->end_time))
		{
			timey_pause_stopwatch(ts);
			ts->end_time_reached = true;
		}
	}
}

// TODO make _past_ versions of timestamp and datetime functions
