#ifndef PRINT_H
#define PRINT_H

#include <stdio.h>

#define USART_BAUD_RATE 14400
//#define USART_BAUD_RATE 115200

void USART_init(const uint32_t baudRate);
void print(const char* str, ...);
void print_char(const char letter);

#endif