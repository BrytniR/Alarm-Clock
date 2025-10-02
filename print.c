#include "print.h"

#include <avr/io.h>
#include <stdarg.h>

#include "configs.h"

// Transmit data
static inline void tx_write(const uint8_t data) {
    uint16_t max_cycles = -1;
    while (!(UCSR0A & (0x1 << UDRE0)) && (--max_cycles > 0)); // transmit ready
    UDR0 = data;
}

void USART_init(const uint32_t baudRate) {
    // No double clock speed or multi-proc communication
    // Clear transmit buffer just in case
    UCSR0A = (0x1 << TXC0);
    // Synchronous & 8-bit data
    UCSR0C = (0x1 << UCSZ01) | (0x1 << UCSZ00);

    // Setting baud rate
    uint16_t ubbr_val = CPU_F / (baudRate * 16UL) - 1;
    if (baudRate == 115200) {
        // Calculation gets spotty for higher freq
        ubbr_val = 8;
    }
    UBRR0H = ubbr_val >> 8;
    UBRR0L = ubbr_val & 0xff;

    // Enable transmitter
    UCSR0B = (0x1 << TXEN0);
}

// Print function. Only some argument types are supported
void print(const char* str, ...) {
    va_list args;
    va_start(args, str);

    while (*str) {
        // Replace next with argument
        if (*str == '%') {
            ++str;
            char buffer[15];
            int len = 0;
            if (*str == 'x' || *str == 'u') { // max val is uint16_t
                const char print_type[] = {'%', *str, '\0'};
                uint16_t val = va_arg(args, uint16_t);
                len = sprintf(buffer, print_type, val);
            } else if (*str == 'l' && (*(++str) == 'u' || *str == 'x')) {
                // Double input chars of size long
                const char print_type[] = {'%', 'l', *str, '\0'};
                uint32_t val = va_arg(args, uint32_t);
                len = sprintf(buffer, print_type, val);
            } else if (*str == 'c' || *str == 'd') {
                // Smaller than int is promoted to int
                int val = va_arg(args, int);
                len = sprintf(buffer, "%d", val);
            }
            // Print out argument
            for (int i = 0; i < len; ++i) {
                tx_write(buffer[i]);
            }
        } else {
            // Normal letter
            tx_write(*str);
        }
        ++str;
    }
    va_end(args);

    // everything will get a newline
    // also flushes the buffer just in case
    tx_write('\n');
}

// Specifically for console echo
void print_char(const char letter) {
    tx_write(letter);
}