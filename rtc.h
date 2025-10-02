#ifndef RTC_H
#define RTC_H

#include <avr/io.h>
#include <stdbool.h>

#define RTC_CLOCK_SPEED         200000UL
#define RTC_ADDR                0x68

#define RTC_ADDR_SECONDS        0x0
#define RTC_ADDR_MINUTES        0x1
#define RTC_ADDR_HOURS          0x2
#define RTC_ADDR_A1             0x7
#define RTC_ADDR_A1_MASK        0xA // A1M4
#define RTC_OFFSET_A1_MASK      7
#define RTC_ADDR_CONTROL        0xE
#define RTC_OFFSET_INT          2
#define RTC_OFFSET_A1_EN        0
#define RTC_ADDR_STATUS         0xF
#define RTC_OFFSET_A1_FLAG      0 // active low

#define RTC_TIME_NUM_ELEMENTS   3
typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
} RTC_time;

void RTC_init(void);
void RTC_reset(void);
bool RTC_time_is_valid(const RTC_time* time);

void RTC_set_time(const RTC_time* time);
void RTC_read_time(RTC_time* buffer);
void RTC_set_alarm(const RTC_time* time);
void RTC_stop_alarm(void);

uint8_t RTC_read_control(void);
uint8_t RTC_read_status(void);

#endif