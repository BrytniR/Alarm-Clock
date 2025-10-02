#include "rtc.h"
#include "print.h"
#include "buzzer.h"

#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

int main (void)
{
    USART_init(USART_BAUD_RATE);
    RTC_init();
    RTC_reset();
    buzzer_init();
    print("Start");
    _delay_ms(1000);

    buzzer_on();
    print("Buzzer on");
    _delay_ms(1000);
    buzzer_off();
    print("Buzzer off");

    RTC_time time = {
        .seconds = 33,
        .minutes = 5,
        .hours = 5,
    };
    RTC_set_time(&time);
    time.seconds += 5;
    RTC_set_alarm(&time);
    print("Buzzer in 5 seconds");
    while(1);
}
