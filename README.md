# Timey

**Timey** is a simple time and date library written in C.

## Features

- Functions for obtaining the current time and date, as well as future and past times and dates
- Tools like stopwatches and timers

> [!NOTE]
> Timey is not thread-safe.

## Example

Here is a simple example of Timey's features:

```c
/*
   raw time objects contain 4 fields, 'sec,' 'millisec,'
   'microsec,' and 'nanosec,' which all represent the number
   of seconds elapsed since the UNIX epoch (Jan 1, 1970).

   you can obtain the raw time like so:
*/
timey_raw_time raw_time = {0};
timey_query_raw_time(&raw_time);

// you can query the raw time string like this (in the format of HOUR:MIN:SEC)
char time[TIMEY_TIME_STR_LEN];
timey_query_time(time, TIMEY_TIME_STR_LEN);

printf("The current time is %s!\n", time);

// you can query the raw date string like this (in the format of MONTH-DAY-YEAR)
char date[TIMEY_DATE_STR_LEN];
timey_query_date(date, TIMEY_DATE_STR_LEN);

printf("The current date is %s!\n", date);

// for times or dates with more information, use timestamps and datetimes:

timey_timestamp ts = timey_curr_timestamp();
// note that hour12 or hour24 can be used here
printf("Current timestamp: %lld:%lld:%lld %s!\n", ts.hour12, ts.min, ts.sec, ts.period);

timey_datetime dt = timey_curr_datetime();
// datetimes have multiple fields in which you can use to customize how the date is printed:
printf("The day of the month and its name: %s %lld\n", dt.day_name, dt.day);
printf("The month number and name: %lld %s\n", dt.month, dt.month_name);
printf("The year and leap year status: %lld %d\n", dt.year, dt.is_leap_year);

// to obtain time information from a datetime, access its 'time' field, which is just a normal timey_timestamp object
```

## Documenation

Please refer to the Timey docs <a href="docs/html" targets="_blank">here</a>. (Open 'index.html' in file explorer)
