#include "button.h"

#include <avr/interrupt.h>
#include <util/delay.h>

#include "task.h"
#include "alarm.h"
#include "rtc.h"
#include "buzzer.h"
#include "print.h"

#define BUTTON_DDR      DDRD
#define BUTTON_BIT      5
#define BUTTON_PORT     PORTD
#define BUTTON_PIN      PIND

#define BUTTON_DEBOUNCE_MS    50

static inline bool button_is_pressed(void) {
    return ((BUTTON_PIN & (0x1 << BUTTON_BIT)) >> BUTTON_BIT);
}

void button_init(void) {
    BUTTON_DDR &= ~(0x1 << BUTTON_BIT);
    BUTTON_PORT &= ~(0x1 << BUTTON_BIT); // no pullup
}

// Mainly for testing
bool button_read(void) {
    return button_is_pressed();
}

// Potential TODO: make this a constant task in case task list full
Task_action button_wait_press(void) {
    // Wait for button press
    if (button_is_pressed()) {
        // Debounce wait
        _delay_ms(BUTTON_DEBOUNCE_MS);
        if (button_is_pressed()) {
            // Turn off buzzer
            buzzer_off();
            // Set up new alarm
            alarm_set_rtc();
            return REMOVE;
        }
    }
    return CONTINUE;
}