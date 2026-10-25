#pragma once

#include <time.h>
#include <stdbool.h>

/**
  Holds time data (hour, minute, and second).

  Each time component is an unsigned integer.
*/
typedef struct timey_timestamp
{
	/**
	  The time's hour in 24-hour format.
	*/
	unsigned int hour24;
	/**
	  The time's hour in 12-hour format.
	*/
	unsigned int hour12;
	/**
	  The time's minute.
	*/
	unsigned int min;
	/**
	  The time's second.
	*/
	unsigned int sec;
	/**
	  Either AM or PM according to the current time.
	*/
	char period[3];
} timey_timestamp;

/**
  Holds raw time data.

  Raw time data is simply the number of seconds since
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

  Each component is an unsigned integer.
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
	unsigned int year;
	/**
	  The date's month.
	*/
	unsigned int month;
	/**
	  The date's day.
	*/
	unsigned int day;
	/**
	  Whether or not the year of this date-time
	  is a leap year.
	*/
	bool is_leap_year;
} timey_datetime;

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
TIMEY_API void timey_query_month_name(char *buffer, size_t buffer_size, int month_num, bool use_abbreviated_name);
/**
  Obtains the corresponding day name and inserts it into a string buffer.

  @note The longest possible day name is 9 characters, so the buffer
  should have a size of at least 10 to account for the null terminator.

  @param buffer Where the resulting day name should be placed.
  @param buffer_size The size of the buffer given.
  @param day_num The number of the day. This should range from 1 to 7.
  @param use_abbreviated_name Whether or not the day's name should be abbreviated.
*/
TIMEY_API void timey_query_day_name(char *buffer, size_t buffer_size, int day_num, bool use_abbreviated_name);

/**
  Obtains a timestamp with the current time.
*/
TIMEY_API timey_timestamp timey_curr_timestamp(void);
/**
  Obtains a timestamp in the future using another
  timestamp as a starting point.
*/
TIMEY_API timey_timestamp timey_future_timestamp(timey_timestamp *now, unsigned int hours, unsigned int minutes, unsigned int seconds);
/**
  Obtains a timestamp in the future using the
  current timestamp as the starting point.
*/
TIMEY_API timey_timestamp timey_future_timestamp_from_now(unsigned int hours, unsigned int minutes, unsigned int seconds);

/**
  Obtains a datetime with the current date and time.
*/
TIMEY_API timey_datetime timey_curr_datetime(void);
/**
  Obtains a datetime in the future using another
  datetime as a starting point.
*/
TIMEY_API timey_datetime timey_future_datetime(timey_datetime *now, unsigned int years, unsigned int months, unsigned int days, unsigned int hours, unsigned int minutes, unsigned int seconds);
/**
  Obtains a datetime in the future using the current
  datetime as the starting point.
*/
TIMEY_API timey_datetime timey_future_datetime_from_now(unsigned int years, unsigned int months, unsigned int days, unsigned int hours, unsigned int minutes, unsigned int seconds);

/**
  Determines whether or not a year is a leap year.
*/
TIMEY_API bool timey_is_leap_year(unsigned int year);
/**
  Returns the number of days in a month.

  For February, 28 is always returned by this function.
*/
TIMEY_API unsigned int timey_num_days_in_month(unsigned int month);
/**
  Returns the number of days in the given date-time's
  month.

  For February, based on if the datetime's year is leap year,
  either 29 or 28 is returned.
*/
TIMEY_API unsigned int timey_num_days_in_month_dt(timey_datetime *dt);
