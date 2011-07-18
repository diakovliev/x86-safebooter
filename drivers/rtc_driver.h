#ifndef RTC_HEADER
#define RTC_HEADER

#include <common.h>

typedef struct tm_s {
	byte_t sec;
	byte_t min;
	byte_t hour;
	byte_t day;
	byte_t mon;
	byte_t year;
} tm_t, *tm_p;

extern void rtc_init(void);
extern tm_p rtc_current(void);

#endif /* RTC_HEADER */
