#ifndef ALARM_H
#define ALARM_H

#include "rtc.h"

void alarm_list_init(void);

// For maneuvering between multiple alarms
void alarm_list_save(RTC_time* time);
void alarm_list_print();
void alarm_list_rem(uint32_t idx);
void alarm_set_rtc(void);

#endif