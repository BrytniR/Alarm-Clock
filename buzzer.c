#include "buzzer.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "task.h"
#include "button.h"

#define INT0_DDR     DDRD
#define INT0_BIT     2
#define INT0_PORT    PORTD

#define BUZZER_DDR     DDRD
#define BUZZER_BIT     7
#define BUZZER_PORT    PORTD

// alarm triggered, turn on buzzer and wait for button press
ISR(INT0_vect) {
    BUZZER_PORT |= (0x1 << BUZZER_BIT);
    task_add(button_wait_press);
}

void buzzer_off(void) {
    BUZZER_PORT &= ~(0x1 << BUZZER_BIT);
}

void buzzer_on(void) {
    BUZZER_PORT |= (0x1 << BUZZER_BIT);
}

void buzzer_init(void) {
    // Buzzer IO pins
    BUZZER_PORT &= ~(0x1 << BUZZER_BIT);
    BUZZER_DDR |= (0x1 << BUZZER_BIT);

    // Set interrupt pins
    INT0_DDR &= ~(0x1 << INT0_BIT);
    // No pullup
    INT0_PORT &= ~(0x1 << INT0_BIT);

    // Activate on falling edge
    EICRA = (0x1 << ISC01);
    // Enable INT0
    EIMSK = (0x1 << INT0);
}