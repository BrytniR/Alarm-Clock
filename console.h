#ifndef CONSOLE_H
#define CONSOLE_H

#include "task.h"

Task_action RX_process_buffer(void);
void console_init(void);
void console_options_print(void);

#endif