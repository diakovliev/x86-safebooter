/*
 * rtc_driver.c
 *
 *  Created on: Jul 13, 2011
 *      Author: D.Iakovliev
 */

#include "rtc_driver.h"

#define RTC_SECONDS 0x00
#define RTC_MINUTES 0x02
#define RTC_HOURS 0x04
#define RTC_DAY 0x07
#define RTC_MONTH 0x08
#define RTC_YEAR 0x09

void rtc_init(void) {
	outb(0x70, 0x0B);
	outb(0x71, 1 << 2);

	rtc_current();
}

inline byte_t rtc_get(byte_t reg) {
	outb(0x70, reg);
	return inb(0x71);
}

tm_p rtc_current(void) {
	static tm_t tm;
	memset(&tm, 0, sizeof(tm));
	tm.sec		= rtc_get(RTC_SECONDS);
	tm.min		= rtc_get(RTC_MINUTES);
	tm.hour		= rtc_get(RTC_HOURS) % 24;
	tm.day		= rtc_get(RTC_DAY);
	tm.mon		= rtc_get(RTC_MONTH);
	tm.year		= rtc_get(RTC_YEAR);
	return &tm;
}
