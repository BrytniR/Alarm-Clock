#ifndef TASK_H
#define TASK_H

#include <avr/io.h>

#include "configs.h"

// Max number of tasks
#define TASK_NUM    10

typedef enum {
    CONTINUE, // Keep task
    REMOVE, // Remove/stop task
} Task_action;

// Service event
// return whether to remove this task from the list
typedef Task_action (*Task_callback) (void);

// Task structure
typedef struct {
    Task_callback task[TASK_NUM];
    uint8_t size; // to a degree, needs to be atomic
} Task_list;

extern volatile Task_list global_task_list;

void task_list_init(void);
Result task_add(Task_callback cb);
void task_remove(const uint8_t idx);
void task_service(const uint8_t idx);

#endif