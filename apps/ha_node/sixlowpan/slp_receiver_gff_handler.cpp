/**
 * @file slp_receiver_gff_handler.cpp
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 14-Nov-2014
 * @brief This is source file for 6lowpan slp_received_GFF_handler function.
 */

extern "C" {
#include "thread.h"
#include "msg.h"
}

#include "ha_sixlowpan.h"
#include "ha_gff_misc.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (1)
#include "ha_debug.h"

void slp_received_GFF_handler(void *GFF_buffer)
{
    /* TODO: implement this */
    HA_DEBUG("slp_received_GFF_handler: ret.\n");
    return;
}
