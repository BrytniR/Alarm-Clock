#include "task.h"

#include <util/atomic.h>
#include <stddef.h>

#include "print.h"

volatile Task_list global_task_list;

void task_list_init(void) {
    global_task_list.size = 0;
    // Fill global array with invalid
    for (uint8_t i = 0; i < TASK_NUM; ++i) {
        global_task_list.task[i] = NULL;
    }
}

// Non-preemptive: Nested interrupts do not work
Result task_add(Task_callback cb) {
    if (cb == NULL) {
        print("Null callback");
        return ERROR;
    }

    // Compare against TASK_NUM - 1 for case where nested task_add() with ISR/normal
    if (global_task_list.size >= TASK_NUM - 1) {
        print("Task list full");
        return ERROR;
    }
    // Find an empty spot to add new task
    for (uint8_t i = 0; i < TASK_NUM; ++i) {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            if (global_task_list.task[i] == NULL) {
                ++global_task_list.size;
                global_task_list.task[i] = cb;
                return SUCCESS;
            }
        }
    }
    print("Unexpected failure adding task");
    return ERROR;
}

// Input: index of task to remove
void task_remove(const uint8_t idx) {
    if (global_task_list.task[idx] == NULL) {
        print("Removed already null task");
        return; // nothing to do
    }
    global_task_list.task[idx] = NULL;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        --global_task_list.size;
    }
}

void task_service(const uint8_t idx) {
    if (global_task_list.task[idx] != NULL) {
        Task_action action = global_task_list.task[idx]();
        if (action == REMOVE) {
            task_remove(idx);
        }
    }
}