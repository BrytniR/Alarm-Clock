#include "rtc.h"
#include "i2c.h"
#include "print.h"

#include <util/delay.h>

int main (void)
{
    USART_init(USART_BAUD_RATE);
    print("Initial Reading");
    print("Expected Control upon power-up: 0x1c");
    print("Expected Status upon power-up: 0x88 or 0x89");
    I2C_set_internal_pullups();
    I2C_init(RTC_CLOCK_SPEED);

    uint8_t data = RTC_ADDR_CONTROL;
    uint8_t resp[2];
    I2C_write(RTC_ADDR, &data, 1);
    I2C_read(RTC_ADDR, resp, 2);
    print("Control reg: %x", resp[0]);
    print("Status reg: %x", resp[1]);

    data = RTC_ADDR_CONTROL;
    I2C_write(RTC_ADDR, &data, 1);
    I2C_read(RTC_ADDR, resp, 1);
    data = RTC_ADDR_STATUS;
    I2C_write(RTC_ADDR, &data, 1);
    I2C_read(RTC_ADDR, resp + 1, 1);
    print("Control reg: %x", resp[0]);
    print("Status reg: %x", resp[1]);

    RTC_time time_test = {
        .seconds = 33,
        .minutes = 5,
        .hours = 20,
    };
    RTC_set_time(&time_test);
    RTC_time time_ret = {
        .seconds = 0,
        .minutes = 0,
        .hours = 0,
    };
    RTC_read_time(&time_ret);
    print("\nSet time test: ~Exp | Act");
    print("Seconds:       %d     %d", time_test.seconds, time_ret.seconds);
    print("Minutes:       %d     %d", time_test.minutes, time_ret.minutes);
    print("Hours:         %d     %d", time_test.hours, time_ret.hours);

    _delay_ms(2000);
    RTC_read_time(&time_ret);
    print("2 Seconds later");
    print("Seconds:       %d     %d", time_test.seconds + 2, time_ret.seconds);
    print("Minutes:       %d     %d", time_test.minutes, time_ret.minutes);
    print("Hours:         %d     %d", time_test.hours, time_ret.hours);

    print("\nReset RTC");
    RTC_reset();
    data = RTC_ADDR_SECONDS;
    I2C_write(RTC_ADDR, &data, 1);
    RTC_read_time(&time_ret);
    print("Seconds: %d", time_ret.seconds);
    print("Minutes: %d", time_ret.minutes);
    print("Hours: %d", time_ret.hours);
    print("Alarm time");
    data = RTC_ADDR_A1;
    I2C_write(RTC_ADDR, &data, 1);
    RTC_read_time(&time_ret);
    print("Seconds: %d", time_ret.seconds);
    print("Minutes: %d", time_ret.minutes);
    print("Hours: %d", time_ret.hours);

    RTC_init();
    RTC_reset();
    print("Control %x", RTC_read_control());
    print("Status %x", RTC_read_status());

    RTC_time time = {
        .seconds = 33,
        .minutes = 5,
        .hours = 5,
    };
    RTC_set_time(&time);
    time.seconds += 6;
    print("Set the alarm");
    RTC_set_alarm(&time);
    _delay_ms(8000);
    print("Control %x", RTC_read_control());
    print("Status %x", RTC_read_status());
}
