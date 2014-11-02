#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "ha_system.h"

int main(void)
{
    /* Init HA System */
    ha_system_init(cc);

    ha_shell_irun();

    return 0;
}
