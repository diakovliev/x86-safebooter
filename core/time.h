#ifndef TIME_HEADER
#define TIME_HEADER

#include <common.h>
#include <loader_types.h>
#include <drivers/rtc_driver.h>
#include <drivers/console_iface.h>

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
	printf("\n\r");
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
