#pragma once

#include <time.h>
#include <stdbool.h>

/**
  Holds time data (hour, minute, and second).
*/
typedef struct timey_timestamp
{
	/**
	  The time's hour in 24-hour format.
	*/
	long long hour24;
	/**
	  The time's hour in 12-hour format.
	*/
	long long hour12;
	/**
	  The time's minute.
	*/
	long long min;
	/**
	  The time's second.
	*/
	long long sec;
	/**
	  Either AM or PM according to the current time.
	*/
	char period[3];
} timey_timestamp;

/**
  Holds raw time data.

  Raw time data is the number of seconds since
  the UNIX epoch (Jan 1, 1970).
*/
typedef struct timey_raw_time
{
	/**
	  The number of seconds since the epoch.
	*/
	time_t sec;
	/**
	  The number of milliseconds since the epoch.
	*/
	time_t millisec;
	/**
	  The number of microseconds since the epoch.
	*/
	time_t microsec;
	/**
	  The number of nanoseconds since the epoch.
	*/
	time_t nanosec;
} timey_raw_time;

/**
  Holds time data (hour, minute, and second),
  as well as date information (year, month, and day).
*/
typedef struct timey_datetime
{
	/**
	  The timestamp.
	*/
	timey_timestamp time;
	/**
	  The date's month name.
	*/
	char month_name[10];
	/**
	  The name of the current day.
	*/
	char day_name[10];
	/**
	  The date's year.
	*/
	long long year;
	/**
	  The date's month.
	*/
	long long month;
	/**
	  The date's day.
	*/
	long long day;
	/**
	  Whether or not the year of this date-time
	  is a leap year.
	*/
	bool is_leap_year;
} timey_datetime;

/**
  Used to track elapsed time during the program.
*/
typedef struct timey_stopwatch
{
	/**
	  The number of elapsed seconds.
	*/
	long double seconds;
	/**
	  The timestamp when the stopwatch was started.
	*/
	timey_timestamp start_time;
	/**
	  A target timestamp for when the stopwatch
	  should automatically stop.

	  @note Default stopwatches do not use this
	  timestamp. You must manually set this
	  timestamp if you want the stopwatch
	  to automatically stop at a target time.
	*/
	timey_timestamp end_time;
	/**
	  Whether or not the stopwatch is paused.
	*/
	bool paused;
	/**
	  Whether or not the stopwatch has reached
	  its target end time.

	  @see end_time
	*/
	bool end_time_reached;
} timey_stopwatch;

#ifdef _WIN32
	#ifdef TIMEY_DLL
		#ifdef TIMEY_EXPORTS
			#define TIMEY_API __declspec(dllexport)
		#else
			#define TIMEY_API __declspec(dllimport)
		#endif
	#else
		#define TIMEY_API
	#endif
#else
	#define TIMEY_API
#endif

/**
  Obtains the current raw time in seconds since the epoch
  and inserts the time into the given timey_raw_time.
*/
TIMEY_API void timey_query_raw_time(timey_raw_time *t);
/**
  Obtains the current time and inserts it into a string buffer.

  @note For the buffer to hold the full time, the buffer must
  have a size of at least 9 bytes. The buffer will be in the format
  of "hh:mm:ss."
*/
TIMEY_API void timey_query_time(char *buffer, size_t buffer_size);
/**
  Obtains the current date and inserts it into a string buffer.

  @note For the buffer to hold the full date, the buffer must
  have a size of at least 11 bytes. The buffer will be in the
  format of "mm-dd-yyyy."
*/
TIMEY_API void timey_query_date(char *buffer, size_t buffer_size);
/**
  Obtains the corresponding month name and inserts it into a string buffer.

  @note The longest possible month name is 9 characters, so the buffer
  should have a size of at least 10 to account for the null terminator.

  @param buffer Where the resulting month name should be placed.
  @param buffer_size The size of the buffer given.
  @param month_num The number of the month. This should range from 1 to 12.
  @param use_abbreviated_name Whether or not the month's name should be abbreviated.
*/
TIMEY_API void timey_query_month_name(char *buffer, size_t buffer_size, unsigned char month_num, bool use_abbreviated_name);
/**
  Obtains the corresponding day name and inserts it into a string buffer.

  @note The longest possible day name is 9 characters, so the buffer
  should have a size of at least 10 to account for the null terminator.

  @note This function assumes the current day, month, and year should
  be used. For past or future dates, refer to the timestamp and datetime
  functions.

  @param buffer Where the resulting day name should be placed.
  @param buffer_size The size of the buffer given.
  @param day_num The number of the day. This should range from 1 to 7, or 1 to 31
  depending on the 'is_wday' argument.
  @param is_wday Whether or not 'day_num' should be interpreted as a weekday
  value or a month value. If this is true, then 'day_num' should range from 1 to 7
  where 1 represents Sunday, and 7 represents Saturday. If this is false, then 'day_num'
  should range from 1 to 31, where 1 is the first day of the month, and 31 is the last
  possible day a month can have.
  @param use_abbreviated_name Whether or not the day's name should be abbreviated.
*/
TIMEY_API void timey_query_day_name(char *buffer, size_t buffer_size, unsigned char day_num, bool is_wday, bool use_abbreviated_name);

/**
  Obtains a timestamp with the current time.
*/
TIMEY_API timey_timestamp timey_curr_timestamp(void);
/**
  Obtains a timestamp in the future using another
  timestamp as a starting point.
*/
TIMEY_API timey_timestamp timey_future_timestamp(const timey_timestamp *start, unsigned long long hours, unsigned long long minutes, unsigned long long seconds);
/**
  Obtains a timestamp in the future using the
  current timestamp as the starting point.
*/
TIMEY_API timey_timestamp timey_future_timestamp_from_now(unsigned long long hours, unsigned long long minutes, unsigned long long seconds);
/**
  Obtains a timestamp in the past using another timestamp
  as a starting point.
*/
TIMEY_API timey_timestamp timey_past_timestamp(const timey_timestamp *start, unsigned long long hours, unsigned long long minutes, unsigned long long seconds);
/**
  Obtains a timestamp in the past using the
  current timestamp as the starting point.
*/
TIMEY_API timey_timestamp timey_past_timestamp_from_now(unsigned long long hours, unsigned long long minutes, unsigned long long seconds);
/**
  Creates a timestamp using a specific hour, minute, and second.

  @note If this function fails, the returned timestamp object will have
  all fields set to 0.

  @important This function expects the timestamp to be within the
  current day's current time. For past or future timestamps,
  refer to timey_future_timestamp(...) and timey_past_timestamp(...).

  @note The 'hours' argument should be in 24-hour format.
*/
TIMEY_API timey_timestamp timey_create_timestamp(unsigned long long hour, unsigned long long minute, unsigned long long second);

/**
  Obtains a datetime with the current date and time.
*/
TIMEY_API timey_datetime timey_curr_datetime(void);
/**
  Obtains a datetime in the future using another
  datetime as a starting point.
*/
TIMEY_API timey_datetime timey_future_datetime(const timey_datetime *start, unsigned long long years, unsigned long long months, unsigned long long days, unsigned long long hours, unsigned long long minutes, unsigned long long seconds);
/**
  Obtains a datetime in the future using the current
  datetime as the starting point.
*/
TIMEY_API timey_datetime timey_future_datetime_from_now(unsigned long long years, unsigned long long months, unsigned long long days, unsigned long long hours, unsigned long long minutes, unsigned long long seconds);
/**
  Obtains a datetime in the past using another
  datetime as the starting point.

  @important This function uses mktime(...) which fails
  for years before 1970.
*/
TIMEY_API timey_datetime timey_past_datetime(const timey_datetime *start, unsigned long long years, unsigned long long months, unsigned long long days, unsigned long long hours, unsigned long long minutes, unsigned long long seconds);
/**
  Obtains a datetime in the past using the current
  datetime as the starting point.
*/
TIMEY_API timey_datetime timey_past_datetime_from_now(unsigned long long years, unsigned long long months, unsigned long long days, unsigned long long hours, unsigned long long minutes, unsigned long long seconds);
/**
  Creates a datetime using a specific year, month, day,
  hour, minute, and second.

  @note If this function fails, the returned datetime object will
  have all fields set to 0.

  @note The 'hours' argument should be in 24-hour format.
  And the 'day' argument should be the day of the month
  (range of 1 to 31).

  @note This function uses mktime(...) which does not
  support years before 1970.
*/
TIMEY_API timey_datetime timey_create_datetime(unsigned long long year, unsigned long long month, unsigned long long day, unsigned long long hour, unsigned long long minute, unsigned long long second);

/**
  Determines whether or not a year is a leap year.
*/
TIMEY_API bool timey_is_leap_year(unsigned long long year);
/**
  Returns the number of days in a month.

  For February, 28 is always returned by this function.

  If the given month is invalid, 0 is returned.
*/
TIMEY_API unsigned char timey_num_days_in_month(unsigned char month);
/**
  Returns the number of days in the given date-time's
  month.

  For February, based on if the datetime's year is leap year,
  either 29 or 28 is returned.

  If the datetime is invalid in any way, 0 is returned.
*/
TIMEY_API unsigned char timey_num_days_in_month_dt(const timey_datetime *dt);

/**
  Modifies a datetime object by changing its current day.
  The other fields on the datetime will automatically
  be updated.
*/
TIMEY_API void timey_set_day(timey_datetime *dt, unsigned char day);
/**
  Modifies a datetime object by changing its current month.
  The other fields on the datetime will automatically
  be updated.
*/
TIMEY_API void timey_set_month(timey_datetime *dt, unsigned char month);
/**
  Modifies a datetime object by changing its current year.
  The other fields on the datetime will automatically
  be updated.
*/
TIMEY_API void timey_set_year(timey_datetime *dt, unsigned char year);

/**
  Determines if two timestamps are equal.
*/
TIMEY_API bool timey_timestamps_are_equal(const timey_timestamp *ts1, const timey_timestamp *ts2);
/**
  Determines if two datetimes are equal.
*/
TIMEY_API bool timey_datetimes_are_equal(const timey_datetime *dt1, const timey_datetime *dt2);

/**
  Creates a stopwatch.
*/
TIMEY_API timey_stopwatch timey_new_stopwatch(void);
/**
  Starts a stopwatch.
*/
TIMEY_API void timey_start_stopwatch(timey_stopwatch *ts);
/**
  Pauses a stopwatch.

  Pausing a stopwatch does not reset
  the amount of elapsed seconds the stopwatch
  has currently tracked.
*/
TIMEY_API void timey_pause_stopwatch(timey_stopwatch *ts);
/**
  Resets a stopwatch.

  Resetting a stopwatch results in its
  elapsed seconds being reset to 0,
  and its 'start_time' timestamp being reset
  as well.

  @note This function also resets the 'end_time'
  timestamp on the stopwatch.
*/
TIMEY_API void timey_reset_stopwatch(timey_stopwatch *ts);
/**
  Updates a stopwatch.

  Use this function in the program's main loop
  using the delta time value to update the
  stopwatch's time.

  @note If the stopwatch is using a target end time,
  this function automatically checks to see if
  the end time has been reached, and if it has,
  the stopwatch is paused, and the 'end_time_reached'
  field on the stopwatch will be true.
*/
TIMEY_API void timey_update_stopwatch(timey_stopwatch *ts, long double delta_time);
