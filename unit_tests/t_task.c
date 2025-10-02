#include "print.h"
#include "configs.h"
#include "task.h"

#include <avr/io.h>

Task_action test_task1(void) {
    print("1");
    return CONTINUE;
}
Task_action test_task2(void) {
    print("2");
    return REMOVE;
}

int main (void)
{
    // Init hardware and queues
    USART_init(USART_BAUD_RATE);
    task_list_init();

    task_add(test_task1);
    // Main loop will run tasks
    for (uint8_t i = 0; i < 15; ++i) {
        print("Loop %d", i);
        // Round robin available tasks
        for (uint8_t idx = 0; idx < TASK_NUM; ++idx) {
            if (global_task_list.task[idx] != NULL) {
                Task_action action = global_task_list.task[idx]();
                if (action == REMOVE) {
                    print("Removing task index: %d", idx);
                    task_remove(idx);
                }
            }
        }

        // add second task every 5 runs, will be removed on next run
        if (i % 5 == 0) {
            task_add(test_task2);
        }
    }
}
