extern "C" {
#include "vtimer.h"
}

#include "ha_system.h"
#include "shell_cmds_sixlowpan.h"
#include "ha_sixlowpan.h"

const char* argv[4] = {"6lowpan", "-x", "20000", "12345"};
const int argc = 4;

int main(void)
{
    /* Init HA System */
    ha_system_init();

    /* Send */
    if (ha_ns::sixlowpan_node_id == 1) {
        while (1) {
            sixlowpan_config((int)argc, (char**)argv);

            vtimer_usleep(200000);
        }
    }

    return 0;
}
