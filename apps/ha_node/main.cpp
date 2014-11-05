#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

extern "C" {
#include "vtimer.h"
}

#include "ha_system.h"

/* configurable variables */
const uint8_t max_end_point = 1;
const int16_t stack_size = 1024;
//const char* thread_name[max_end_point] = { "thread1", "thread2", "thread3",
//        "thread4", "thread5", "thread6", "thread7", "thread8", "thread9",
//        "thread10", "thread11", "thread12", "thread13", "thread14", "thread15",
//        "thread16" };

char stack[max_end_point][stack_size];
kernel_pid_t end_point_pid[max_end_point];

int main(void)
{
    ha_system_init(ha_ns::node);

    /* create end-point's threads */
    for (int i = 0; i < max_end_point; i++) {
        end_point_pid[i] = thread_create(stack[i], stack_size,
        PRIORITY_MAIN - 1, 0, end_point_handler, NULL, (const char*) i);
    }

    /* read device list saved in flash */

    while (1) {

    }
}
