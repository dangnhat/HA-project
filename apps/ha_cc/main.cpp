#include "ha_system.h"

int main(void)
{
    /* Init HA System */
    ha_system_init();

    /* Run shell */
    ha_shell_irun(NULL);

    return 0;
}
