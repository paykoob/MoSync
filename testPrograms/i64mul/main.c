#include <time.h>
#include <limits.h>
#include <stdio.h>
#include <maassert.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include "../../libs/newlib/libc/time/local.h"

#define DIV(a, b) ((a) / (b) - ((a) % (b) < 0))
#define LEAPS_THRU_END_OF(y) (DIV (y, 4) - DIV (y, 100) + DIV (y, 400))

#define DUMPI(i) printf("%s: %lli\n", #i, i)
#if 0
static time_t test(time_t days, time_t y, time_t yg) {
	days -= ((yg - y) * 365
			+ LEAPS_THRU_END_OF (yg - 1)
			- LEAPS_THRU_END_OF (y - 1));
	return days;
}

static time_t test2(time_t days, time_t y, time_t yg) {
	time_t diff;
#if 0
	days -= ((yg - y) * 365
			+ LEAPS_THRU_END_OF (yg - 1)
			- LEAPS_THRU_END_OF (y - 1));
#endif
	DUMP((yg - y) * 365);
	DUMP(LEAPS_THRU_END_OF (yg - 1));
	DUMP(LEAPS_THRU_END_OF (y - 1));
	diff = ((yg - y) * 365
			+ LEAPS_THRU_END_OF (yg - 1)
			- LEAPS_THRU_END_OF (y - 1));
	DUMP(diff);
	DUMP(days - diff);
	return days - diff;
}
#endif
#if 0
static time_t test3(time_t days, time_t y, time_t yg) {
	DUMP((yg - y) * 365);
	return days;
}
static time_t test4(time_t days, time_t y, time_t yg) {
	DUMP((yg - y));
	return days;
}
#if 0
static time_t test5(time_t days, time_t y, time_t yg) {
	DUMP(y * 365);
	return days;
}
static time_t test6(time_t days, time_t y, time_t yg) {
	DUMP(yg * 365);
	return days;
}
#endif
static time_t test7(time_t days, time_t y, time_t yg) {
	return yg - y;
}
static time_t test8(time_t days, time_t y, time_t yg) {
	return yg + y;
}
#endif

#if 0
#define _ISLEAP(y) (((y) % 4) == 0 && (((y) % 100) != 0 || (((y)+1900) % 400) == 0))
static int isleap(time_t y) { return _ISLEAP(y); }
#endif

#if 1
#define LOG printf
#else
#define LOG(...)
#endif

#if 0
static time_t test0(time_t days, time_t y) {
	while (days < 0 || days >= (isleap (y) ? 366 : 365))
	{
		/* Guess a corrected year, assuming 365 days per year.  */
		time_t yg = y + days / 365 - (days % 365 < 0);
		LOG("days: %llx, y: %llx, yg: %llx\n", days, y, yg);

		/* Adjust DAYS and Y to match the guessed year.  */
		days -= ((yg - y) * 365
			+ LEAPS_THRU_END_OF (yg - 1)
			- LEAPS_THRU_END_OF (y - 1));
#define DUMPLLX(i) LOG("%s: 0x%llx\n", #i, (i))
		DUMPLLX((yg - y) * 365);
		DUMPLLX(LEAPS_THRU_END_OF (yg - 1));
		DUMPLLX(LEAPS_THRU_END_OF (y - 1));
		y = yg;
	}
	//DUMPI(y);
	return days;
}
#endif


static _CONST int mon_lengths[2][MONSPERYEAR] = {
  {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
  {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
} ;

static _CONST int year_lengths[2] = {
  365,
  366
} ;

static struct tm* s_mktm_r(_CONST time_t * tim_p, struct tm *res, int is_gmtime)
{
  time_t days, rem;
  time_t lcltime;
  time_t y;
  int yleap;
	int loopCount;
  _CONST int *ip;
   __tzinfo_type *tz = __gettzinfo ();

  /* base decision about std/dst time on current time */
  lcltime = *tim_p;

  days = (lcltime) / SECSPERDAY;
  rem = (lcltime) % SECSPERDAY;
	LOG("mktm_r lcltime: 0x%llx, SECSPERDAY %li, days: 0x%llx, rem: 0x%llx\n", lcltime, SECSPERDAY, days, rem);
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

  /* compute hour, min, and sec */
  res->tm_hour = (int) (rem / SECSPERHOUR);
  rem %= SECSPERHOUR;
  res->tm_min = (int) (rem / SECSPERMIN);
  res->tm_sec = (int) (rem % SECSPERMIN);

  /* compute day of week */
  if ((res->tm_wday = ((EPOCH_WDAY + days) % DAYSPERWEEK)) < 0)
    res->tm_wday += DAYSPERWEEK;

  /* compute year & day of year */
	// terribly slow if time is not close to zero.
	// also fails to set errno = EOVERFLOW.
  y = EPOCH_YEAR;
#if 0
  if (days >= 0)
    {
      for (;;)
	{
	  yleap = isleap(y);
	  if (days < year_lengths[yleap])
	    break;
	  y++;
	  days -= year_lengths[yleap];
	}
    }
  else
    {
      do
	{
	  --y;
	  yleap = isleap(y);
	  days += year_lengths[yleap];
	} while (days < 0);
    }
  res->tm_year = (int)(y - YEAR_BASE);
#else
	assert(sizeof(y) == sizeof(time_t));
#define DIV(a, b) ((a) / (b) - ((a) % (b) < 0))
#define LEAPS_THRU_END_OF(y) (DIV (y, 4) - DIV (y, 100) + DIV (y, 400))

		loopCount = 0;
	while (days < 0 || days >= (isleap (y) ? 366 : 365))
	{
		/* Guess a corrected year, assuming 365 days per year.  */
		time_t yg = y + days / 365 - (days % 365 < 0);
		//LOG("days: %llx, y: %llx, yg: %llx\n", days, y, yg);

		/* Adjust DAYS and Y to match the guessed year.  */
		days -= ((yg - y) * 365
			+ LEAPS_THRU_END_OF (yg - 1)
			- LEAPS_THRU_END_OF (y - 1));
		//printf("%s: 0x%llx\n", "foo", (yg - y) * 365);
#define DUMPLLX(i) LOG("%s: 0x%llx\n", #i, (i))
		DUMPLLX((yg - y) * 365);
		DUMPLLX(LEAPS_THRU_END_OF (yg - 1));
		//DUMPLLX(LEAPS_THRU_END_OF (y - 1));
		y = yg;
		loopCount++;
		assert(loopCount < 100);
	}
	res->tm_year = y - YEAR_BASE;
	yleap = isleap(y);
	// mktime has a limit of 10000 years.
	if (res->tm_year != y - YEAR_BASE || abs(y) > 10000)
	{
		/* The year cannot be represented due to overflow.  */
		errno = EOVERFLOW;
		return 0;
	}
#endif

  res->tm_yday = days;
  ip = mon_lengths[yleap];
  for (res->tm_mon = 0; days >= ip[res->tm_mon]; ++res->tm_mon)
    days -= ip[res->tm_mon];
  res->tm_mday = days + 1;

  if (!is_gmtime)
    {
      long offset;
      int hours, mins, secs;

      TZ_LOCK;
      if (_daylight)
	{
	  if (y == tz->__tzyear || __tzcalc_limits (y))
	    res->tm_isdst = (tz->__tznorth
			     ? (*tim_p >= tz->__tzrule[0].change
				&& *tim_p < tz->__tzrule[1].change)
			     : (*tim_p >= tz->__tzrule[0].change
				|| *tim_p < tz->__tzrule[1].change));
	  else
	    res->tm_isdst = -1;
	}
      else
	res->tm_isdst = 0;

      offset = (res->tm_isdst == 1
		  ? tz->__tzrule[1].offset
		  : tz->__tzrule[0].offset);

      hours = (int) (offset / SECSPERHOUR);
      offset = offset % SECSPERHOUR;

      mins = (int) (offset / SECSPERMIN);
      secs = (int) (offset % SECSPERMIN);

      res->tm_sec -= secs;
      res->tm_min -= mins;
      res->tm_hour -= hours;

      if (res->tm_sec >= SECSPERMIN)
	{
	  res->tm_min += 1;
	  res->tm_sec -= SECSPERMIN;
	}
      else if (res->tm_sec < 0)
	{
	  res->tm_min -= 1;
	  res->tm_sec += SECSPERMIN;
	}
      if (res->tm_min >= MINSPERHOUR)
	{
	  res->tm_hour += 1;
	  res->tm_min -= MINSPERHOUR;
	}
      else if (res->tm_min < 0)
	{
	  res->tm_hour -= 1;
	  res->tm_min += MINSPERHOUR;
	}
      if (res->tm_hour >= HOURSPERDAY)
	{
	  ++res->tm_yday;
	  ++res->tm_wday;
	  if (res->tm_wday > 6)
	    res->tm_wday = 0;
	  ++res->tm_mday;
	  res->tm_hour -= HOURSPERDAY;
	  if (res->tm_mday > ip[res->tm_mon])
	    {
	      res->tm_mday -= ip[res->tm_mon];
	      res->tm_mon += 1;
	      if (res->tm_mon == 12)
		{
		  res->tm_mon = 0;
		  res->tm_year += 1;
		  res->tm_yday = 0;
		}
	    }
	}
       else if (res->tm_hour < 0)
	{
	  res->tm_yday -= 1;
	  res->tm_wday -= 1;
	  if (res->tm_wday < 0)
	    res->tm_wday = 6;
	  res->tm_mday -= 1;
	  res->tm_hour += 24;
	  if (res->tm_mday == 0)
	    {
	      res->tm_mon -= 1;
	      if (res->tm_mon < 0)
		{
		  res->tm_mon = 11;
		  res->tm_year -= 1;
		  res->tm_yday = 365 + isleap(res->tm_year);
		}
	      res->tm_mday = ip[res->tm_mon];
	    }
	}
      TZ_UNLOCK;
    }
  else
    res->tm_isdst = 0;

  return (res);
}



int MAMain(void) GCCATTRIB(noreturn);
int MAMain(void) {
	struct tm t;
	time_t ti = LONG_LONG_MAX;
	struct tm* tp;
	tp = s_mktm_r(&ti, &t, 0);
	printf("%p\n", tp);
	if(tp) {
		DUMPI((time_t)t.tm_year);
	}
#if 0
	time_t days = LONG_LONG_MAX / 365;
	time_t y = 1970;
	time_t yg = y + days / 365 - (days % 365 < 0);
	DUMPI(days);
	DUMPI(y);
	DUMPI(yg);
	DUMPI(test0(days, y));
#endif
#if 0
	DUMP(test(days, y, yg));
	DUMP(test2(days, y, yg));
#endif
#if 0
	DUMP(test3(days, y, yg));
	test4(days, y, yg);
	DUMP(test7(days, y, yg));
	DUMP(test7(days, 2, 4));
	DUMP(test8(days, 2, 2));
	DUMP(test7(days, 0x200000000, 0x400000000));
	DUMP(test8(days, 0x200000000, 0x200000000));
	DUMP(test7(days, 0x7b2, 0x3ef739672594));
#endif
#if 0
	test5(days, yg, y);
	test6(days, yg, y);
#endif
	FREEZE;
}
