#include "console.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "print.h"
#include "rtc.h"
#include "alarm.h"

#define RX_MASK         0x7F
#define RX_BUFFER_SIZE  64

typedef struct {
    char buffer[RX_BUFFER_SIZE];
    uint8_t tail;
    uint8_t size;
} Rx_buffer;

static volatile Rx_buffer read_buffer;

ISR(USART_RX_vect) {
    char received_char = (UDR0 & RX_MASK); // mask leftmost value

    uint8_t buffer_idx = (read_buffer.tail + read_buffer.size) % RX_BUFFER_SIZE;
    read_buffer.buffer[buffer_idx] = received_char;

    if (read_buffer.size == RX_BUFFER_SIZE) {
        // Lose last char
        read_buffer.tail = (read_buffer.tail + 1) % RX_BUFFER_SIZE;
    } else {
        ++read_buffer.size;
    }

    if (received_char == '\n') {
        // Command finished, process buffer
        // Could get race condition unless I turn off RX
        task_add(RX_process_buffer);
    } else {
        print_char(received_char); // note: is slow
    }
}

static bool get_verify_time(const char * string, RTC_time* time_buffer) {
    // Verify that string input time is valid and return as an RTC_time struct
    if (string == NULL) {
        print("Empty time string");
    }

    uint8_t number_arg = sscanf(string, "%hhu:%hhu:%hhu", &(time_buffer->hours), &(time_buffer->minutes), &(time_buffer->seconds));
    if (number_arg == 3) {
        return true;
    }
    
    print("Invalid time format");
    return false;
}

Task_action RX_process_buffer(void) {
    // Get command
    char command[RX_BUFFER_SIZE];
    for (uint8_t i = 0; i < read_buffer.size; ++i) {
        command[i] = read_buffer.buffer[(i + read_buffer.tail) % RX_BUFFER_SIZE];
    }
    command[(read_buffer.size + read_buffer.tail) % RX_BUFFER_SIZE] = '\0'; // terminate string
    read_buffer.size = 0; // consume string

    RTC_time time_buffer;

    char *complete_cmd = strtok(command, "\r\n");
    if (complete_cmd == NULL) {
        printf("Missing return");
    }
    // Tasks that do not require an option
    if (strcmp(complete_cmd, "get time") == 0) {
        RTC_read_time(&time_buffer);
        print("HH:mm:ss = %d:%d:%d", time_buffer.hours, time_buffer.minutes, time_buffer.seconds);
        return REMOVE;
    } else if (strcmp(complete_cmd, "get alarm") == 0) {
        // Return list of current alarms
        alarm_list_print();
        return REMOVE;
    } 

    // Figure out task option
    const char *delim = "/";
    char *token = strtok(command, delim);
    if (token == NULL) {
        print("Invalid option, use /");
        return REMOVE;
    }

    if (strcmp(token, "set time") == 0) {
        token = strtok(NULL, delim);
        if (get_verify_time(token, &time_buffer)) {
            RTC_set_time(&time_buffer);
        }
    } else if (strcmp(token, "set alarm") == 0) {
        token = strtok(NULL, delim);
        if (get_verify_time(token, &time_buffer)) {
            alarm_list_save(&time_buffer);
        }
    } else if (strcmp(token, "rem alarm") == 0) {
        // remove alarm at index token
        token = strtok(NULL, delim);
        int alarm_idx;
        if (sscanf(token, "%d", &alarm_idx) == 1) {
            alarm_list_rem(alarm_idx);
        }
    } else {
        print("Invalid option");
    }
    return REMOVE;
}

void console_init(void) {
    // Set up receiver
    read_buffer.tail = 0;
    read_buffer.size = 0;

    // Enable receiver interrupts
    UCSR0B |= (0x1 << RXEN0) | (0x1 << RXCIE0);
}

void console_options_print(void) {
    print("Option / Input");
    print("set time/hh:mm:ss");
    print("get time");
    print("set alarm/hh:mm:ss");
    print("get alarm");
    print("rem alarm/indexOfAlarm");
}