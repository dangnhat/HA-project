#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

extern "C" {
#include "vtimer.h"
}

#include "ha_system.h"

/* configurable variables */
const int16_t stack_size = 1400;
const char thread_name_hdr[] = "endpoint %d";

char stack[ha_node_ns::max_end_point][stack_size];

int main(void)
{
    ha_system_init();

    /* create end-point's threads */
    char thread_name[16];
    for (int8_t i = 0; i < ha_node_ns::max_end_point; i++) {
        snprintf(thread_name, sizeof(thread_name), thread_name_hdr, i);
        ha_node_ns::end_point_pid[i] = thread_create(stack[i], stack_size,
        PRIORITY_MAIN - 1, CREATE_STACKTEST, end_point_handler, NULL,
                thread_name);
    }

    /* read device list saved in flash and run devices */
    for (int8_t i = 0; i < ha_node_ns::max_end_point; i++) {
        run_endpoint(i);
    }

    /* Run shell */
    ha_shell_irun(NULL);
}
