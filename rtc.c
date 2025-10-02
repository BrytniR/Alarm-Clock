#include "rtc.h"

#include "print.h"
#include "configs.h"
#include "i2c.h"

#define MAX_SECONDS             60
#define MAX_MINUTES             60
#define MAX_HOURS               24

static uint8_t convert_time2raw(const uint8_t time) {
    // Convert to reg format
    uint8_t time_reg = time % 10;
    // Tens place
    time_reg |= ((time / 10) << 4);
    return time_reg;
}

static uint8_t convert_raw2time(const uint8_t time_reg) {
    // Convert to reg format
    uint8_t time = time_reg & 0xF;
    // Tens place
    time += ((time_reg & 0x70) >> 4) * 10;
    return time;
}

static void set_time(const uint8_t time, const uint8_t time_addr) {
    // Convert to reg format
    uint8_t time_reg = convert_time2raw(time);

    uint8_t data[] = {time_addr, time_reg};
    I2C_write(RTC_ADDR, data, sizeof(data) / sizeof(data[0]));
}

void RTC_init(void) {
    I2C_set_internal_pullups();
    I2C_init(RTC_CLOCK_SPEED);
}

void RTC_reset(void) {
    RTC_stop_alarm();

    RTC_time time_zero = {
        .seconds = 0,
        .minutes = 0,
        .hours = 0,
    };
    RTC_set_time(&time_zero);
    RTC_set_alarm(&time_zero);
    // only match alarms on h/m/s
    // A1M4 = 1
    uint8_t match_data[] = {RTC_ADDR_A1_MASK, (0x1 << RTC_OFFSET_A1_MASK)};
    I2C_write(RTC_ADDR, match_data, sizeof(match_data) / sizeof(match_data[0]));
}

bool RTC_time_is_valid(const RTC_time* time) {
    if (time->seconds >= MAX_SECONDS) {
        print("Invalid seconds input: %d >= %d", time, MAX_SECONDS);
        return false;
    }
    if (time->minutes >= MAX_MINUTES) {
        print("Invalid minutes input: %d >= %d", time, MAX_MINUTES);
        return false;
    }
    if (time->hours >= MAX_HOURS) {
        print("Invalid hours input: %d >= %d", time, MAX_HOURS);
        return false;
    }
    return true;
}

void RTC_set_time(const RTC_time* time) {
    if (time == NULL) {
        print("Invalid null ptr");
        return;
    }
    if (!RTC_time_is_valid(time)) {
        return;
    }

    set_time(time->seconds, RTC_ADDR_SECONDS);
    set_time(time->minutes, RTC_ADDR_MINUTES);
    set_time(time->hours, RTC_ADDR_HOURS);
}

void RTC_read_time(RTC_time* buffer) {
    if (buffer == NULL) {
        print("Invalid null ptr");
        return;
    }
    // Set pointer to time address
    uint8_t pointer_data[] = {RTC_ADDR_SECONDS};
    I2C_write(RTC_ADDR, pointer_data, sizeof(pointer_data) / sizeof(pointer_data[0]));
    uint8_t data[RTC_TIME_NUM_ELEMENTS];
    I2C_read(RTC_ADDR, data, RTC_TIME_NUM_ELEMENTS);
    buffer->seconds = convert_raw2time(data[0]);
    buffer->minutes = convert_raw2time(data[1]);
    buffer->hours = convert_raw2time(data[2]);
}
 
void RTC_set_alarm(const RTC_time* time) {
    if (time == NULL) {
        print("Invalid null ptr");
        return;
    }
    if (!RTC_time_is_valid(time)) {
        return;
    }

    set_time(time->seconds, RTC_ADDR_A1 + RTC_ADDR_SECONDS);
    set_time(time->minutes, RTC_ADDR_A1 + RTC_ADDR_MINUTES);
    set_time(time->hours, RTC_ADDR_A1 + RTC_ADDR_HOURS);

    // Set match
    uint8_t match_data[] = {RTC_ADDR_A1_MASK, (0x1 << RTC_OFFSET_A1_MASK)};
    I2C_write(RTC_ADDR, match_data, sizeof(match_data) / sizeof(match_data[0]));

    // Enable alarm1
    uint8_t control_state = 0x1c | (0x1 << RTC_OFFSET_INT) | (0x1 << RTC_OFFSET_A1_EN);
    uint8_t data[2] = {RTC_ADDR_CONTROL, control_state};
    I2C_write(RTC_ADDR, data, sizeof(data) / sizeof(data[0]));
}

void RTC_stop_alarm(void) {
    uint8_t control_state = 0x1c | (0x1 << RTC_OFFSET_INT);
    uint8_t data[2] = {RTC_ADDR_CONTROL, control_state};
    I2C_write(RTC_ADDR, data, sizeof(data) / sizeof(data[0]));

    // Set alarm flag to zero
    uint8_t status_state = 0x08;
    data[0] = RTC_ADDR_STATUS;
    data[1] = status_state;
    I2C_write(RTC_ADDR, data, sizeof(data) / sizeof(data[0]));
}

uint8_t RTC_read_control(void) {
    uint8_t pointer_data[] = {RTC_ADDR_CONTROL};
    I2C_write(RTC_ADDR, pointer_data, sizeof(pointer_data) / sizeof(pointer_data[0]));
    uint8_t data;
    I2C_read(RTC_ADDR, &data, 1);
    return data;
}

uint8_t RTC_read_status(void) {
    uint8_t pointer_data[] = {RTC_ADDR_STATUS};
    I2C_write(RTC_ADDR, pointer_data, sizeof(pointer_data) / sizeof(pointer_data[0]));
    uint8_t data;
    I2C_read(RTC_ADDR, &data, 1);
    return data;
}