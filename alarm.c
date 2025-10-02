#include "alarm.h"

#include <util/atomic.h>

#include "print.h"
#include "configs.h"
#include "rtc.h"

#define ALARM_NUM               15
#define ALARM_MAX_TIME_IN_SEC   (24UL * 60UL * 60UL)
#define ALARM_SET_BUFFER        1  // to accomodate alarm that might be immediately missed if code too slow

typedef enum {
    INVALID,
    ACTIVE,
} Alarm_state;

typedef struct {
    RTC_time time;
    Alarm_state state;
    uint32_t total_seconds; // complete time in seconds for fast comparisons
} Alarm;

// Task structure
typedef struct {
    Alarm alarm[ALARM_NUM];
    uint8_t size;
} Alarm_list;

static Alarm_list global_alarm_list;

static uint32_t get_total_seconds(RTC_time* time) {
    if (time == NULL) {
        return 0;
    }

    uint32_t total_seconds = time->seconds;
    total_seconds += 60 * (time->minutes);
    total_seconds += (uint32_t) 24 * 60 * (time->hours); // to prevent overflow
    return total_seconds;
}

static inline uint32_t compare_times(uint32_t now_time, uint32_t later_time) {
    return later_time - now_time;
}

void alarm_list_init(void) {
    global_alarm_list.size = 0;
    // Fill global array with invalid
    for (uint8_t i = 0; i < ALARM_NUM; ++i) {
        global_alarm_list.alarm[i].state = INVALID;
    }
}

static Result alarm_add(RTC_time* time) {
    if (time == NULL) {
        print("Null alarm");
        return ERROR;
    }
    if (!RTC_time_is_valid(time)) {
        return ERROR;
    }
    if (global_alarm_list.size >= ALARM_NUM) {
        print("Alarm list full");
        return ERROR;
    }
    // Find an empty spot to add new alarm
    for (uint8_t i = 0; i < ALARM_NUM; ++i) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            Alarm* alarm = &(global_alarm_list.alarm[i]);
            if (alarm->state == INVALID) {
                ++global_alarm_list.size;
                alarm->time = *time; // copied by value
                alarm->total_seconds = get_total_seconds(time);
                alarm->state = ACTIVE;
                return SUCCESS;
            }
        }
    }
    print("Unexpected failure adding alarm");
    return ERROR;
}

// Input: index of alarm to remove
static void alarm_remove(uint8_t idx) {
    if (global_alarm_list.alarm[idx].state == INVALID) {
        print("Removed already invalid alarm");
        return; // nothing to do
    }
    global_alarm_list.alarm[idx].state = INVALID;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        --global_alarm_list.size;
    }
}

// Set RTC with the latest alarm
void alarm_set_rtc(void) {
    RTC_time* best_time = NULL;
    uint32_t best_diff = ALARM_MAX_TIME_IN_SEC + 1;

    RTC_time now;
    RTC_read_time(&now);
    uint32_t seconds_now = get_total_seconds(&now);
    seconds_now = (seconds_now + ALARM_SET_BUFFER) % ALARM_MAX_TIME_IN_SEC;
    for (uint8_t i = 0; i < ALARM_NUM; ++i) {
        Alarm* alarm = &(global_alarm_list.alarm[i]);
        if (alarm->state == ACTIVE) {
            if (best_time == NULL) { // first found alarm
                best_time = &(alarm->time);
                best_diff = compare_times(seconds_now, alarm->total_seconds);
            } else {
                uint32_t new_diff = compare_times(seconds_now, alarm->total_seconds);
                if (new_diff < best_diff) {
                    best_time = &(alarm->time);
                    best_diff = new_diff;
                }
            }
        }
    }

    RTC_stop_alarm();
    if (best_time != NULL) {
        // Set this alarm
        RTC_set_alarm(best_time);
    }
}

void alarm_list_save(RTC_time* time) {
    print("Set alarm");
    alarm_add(time);
    alarm_set_rtc();
}

void alarm_list_print() {
    print("Index | HH:mm:ss");
    for (uint8_t i = 0; i < ALARM_NUM; ++i) {
        Alarm* alarm = &(global_alarm_list.alarm[i]);
        if (alarm->state != INVALID) {
            print(" %d    | %d:%d:%d", i, alarm->time.hours, alarm->time.minutes, alarm->time.seconds);
        }
    }
}

void alarm_list_rem(uint32_t idx) {
    print("Remove alarm %d", idx);
    alarm_remove(idx);
    alarm_set_rtc();
}