#ifndef TIME_HEADER
#define TIME_HEADER

#include <common.h>
#include <loader_types.h>
#include <stdio.h>
#include <drivers/rtc_driver.h>

static inline quad_t ctime(void) {
	tm_p tm = rtc_current();
	quad_t start = tm->sec + (tm->min * 60) + (tm->hour * 3600);
	return start;
}

static inline void print_tm(tm_p tm) {
	printf("%02d:%02d:%02d %02d.%02d.%02d",
			tm->hour, tm->min, tm->sec,
			tm->day, tm->mon, tm->year);
}

static inline void print_current_time(void) {
	print_tm(rtc_current());
	//printf("\n\r");
}

/* !!!  !!! */
/* Converts Gregorian date to seconds since 1970-01-01 00:00:00.
 * Assumes input in normal date format, i.e. 1980-12-31 23:59:59
 * => year=1980, mon=12, day=31, hour=23, min=59, sec=59.
 *
 * [For the Julian calendar (which was used in Russia before 1917,
 * Britain & colonies before 1752, anywhere else before 1582,
 * and is still in use by some communities) leave out the
 * -year/100+year/400 terms, and add 10.]
 *
 * This algorithm was first published by Gauss (I think).
 *
 * WARNING: this function will overflow on 2106-02-07 06:28:16 on
 * machines where long is 32-bit! (However, as time_t is signed, we
 * will already get problems at other places on 2038-01-19 03:14:08)
 */
unsigned long
mktime(const unsigned int year0, const unsigned int mon0,
       const unsigned int day, const unsigned int hour,
       const unsigned int min, const unsigned int sec)
{
    unsigned int mon = mon0, year = year0;

    /* 1..12 -> 11,12,1..10 */
    if (0 >= (int) (mon -= 2)) {
        mon += 12;  /* Puts Feb last since it has leap day */
        year -= 1;
    }

    return ((((unsigned long)
          (year/4 - year/100 + year/400 + 367*mon/12 + day) +
          year*365 - 719499
        )*24 + hour /* now have hours */
      )*60 + min /* now have minutes */
    )*60 + sec; /* finally seconds */
}


static inline void ssleep(quad_t timeout) {
	quad_t ctm 		= ctime();
	quad_t s 		= ctm;
	quad_t delta 	= 0;
	//printf("%d\t", s);
	do {
		//idle();
		ctm = ctime();
		delta = ctm - s;
		//printf("%d - %d = %d\n\r", ctm, s, delta);
	} while (delta < timeout);
}

#endif /* TIME_HEADER */
