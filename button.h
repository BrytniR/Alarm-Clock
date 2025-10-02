#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

#include "task.h"

void button_init(void);
bool button_read(void);
Task_action button_wait_press(void);

#endif