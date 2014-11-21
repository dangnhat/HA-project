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
#include "controller.h"
#include "gff_mesg_id.h"
#include "cc_msg_id.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

void slp_received_GFF_handler(uint8_t *GFF_buffer)
{
    HA_DEBUG("slp_received_GFF_handler, forward to controller\n");

    /* Push data to queue */
    controller_ns::slp_to_controller_queue.add_data(
            GFF_buffer,
            ha_ns::GFF_CMD_SIZE + ha_ns::GFF_LEN_SIZE + GFF_buffer[ha_ns::GFF_LEN_POS]);
    /* send message to controller */
    msg_t mesg;
    mesg.type = ha_cc_ns::SLP_GFF_PENDING;
    mesg.content.ptr = (char *) &controller_ns::slp_to_controller_queue;

    msg_send(&mesg, controller_ns::controller_pid, false);

    return;
}
