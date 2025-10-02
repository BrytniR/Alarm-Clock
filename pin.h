#ifndef PIN_H
#define PIN_H

#include <avr/io.h>

// Early debugging, but note that pin is
// Shared with the SPI clock
#define LED_DDR     DDRB
#define LED_PORT    PORTB
#define LED_PIN     PB5

// Notes:
// DDR: 1 = output, 0 = input
// PORT: output. If DDR=input, pullup for PORT = 1

#endif