#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>

extern "C" {
#include "vtimer.h"
}

#include "ha_system.h"

/* configurable variables */
const int16_t stack_size = 1550;
const char thread_name[][5] = { "ep_0", "ep_1", "ep_2", "ep_3", "ep_4", "ep_5",
        "ep_6", "ep_7", };

char stack[ha_host_ns::max_end_point][stack_size];

int main(void)
{
    uint8_t i;
    ha_system_init();

    /* create end-point's threads */
    for (i = 0; i < ha_host_ns::max_end_point; i++) {
        ha_host_ns::end_point_pid[i] = thread_create(stack[i], stack_size,
        PRIORITY_MAIN - 1, CREATE_STACKTEST, end_point_handler, NULL,
                thread_name[i]);
    }

    /* read device list saved in flash and run devices */
    for (i = 0; i < ha_host_ns::max_end_point; i++) {
        run_endpoint(i);
    }

    /* Run shell */
    ha_shell_irun(NULL);
}
