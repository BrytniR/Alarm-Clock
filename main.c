#include "print.h"
#include "configs.h"
#include "task.h"
#include "rtc.h"
#include "button.h"
#include "buzzer.h"
#include "console.h"
#include "alarm.h"

#include <avr/interrupt.h>

int main (void)
{
    // Init hardware and queues
    USART_init(USART_BAUD_RATE);
    task_list_init();
    RTC_init();
    RTC_reset();
    buzzer_init();
    alarm_list_init();
    button_init();

    console_init();
    console_options_print();

    // Enable global interrupts
    sei();

    while (1) {
        // Round robin available tasks
        for (uint8_t idx = 0; idx < TASK_NUM; ++idx) {
            task_service(idx);
        }
    }
}
