#include "print.h"
#include "button.h"

#include <util/delay.h>

int main (void)
{
    USART_init(USART_BAUD_RATE);
    button_init();

    while (1) {
        _delay_ms(100);
        print("%x", button_read());
    }
}
