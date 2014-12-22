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
#include "gff_mesg_id.h"
#include "ha_gff_misc.h"
#include "ha_host_glb.h"

#define HA_NOTIFICATION (1)
#define HA_DEBUG_EN (0)
#include "ha_debug.h"

void slp_received_GFF_handler(uint8_t *GFF_buffer)
{
    HA_DEBUG("slp_received_GFF_handler:\n");
    if (!GFF_buffer) {
        HA_NOTIFY("GFF buffer is null.\n");
        return;
    }

    /* |1byte length|2byte cmd|4byte dev_id|2byte value| */
    uint16_t gff_msg_cmd = buf2uint16((GFF_buffer + ha_ns::GFF_CMD_POS));
    uint32_t dev_id = buf2uint32((GFF_buffer + ha_ns::GFF_DATA_POS));
    uint16_t value = buf2uint16((GFF_buffer + ha_ns::GFF_DATA_POS + 4));

    if (gff_msg_cmd != ha_ns::SET_DEV_VAL) {
        HA_NOTIFY("SET_DEV_VAL message only.\n");
        return;
    }

    uint8_t ep_id = parse_ep_deviceid(dev_id);
    if (ep_id > ha_host_ns::max_end_point) {
        HA_NOTIFY("End point id is invalid.\n");
        return;
    }
    uint32_t data_send = (dev_id << 16) | value;

    msg_t msg_to_endpoint;
    msg_to_endpoint.type = gff_msg_cmd;
    msg_to_endpoint.content.value = data_send;

    msg_send(&msg_to_endpoint, ha_host_ns::end_point_pid[ep_id], false);

    return;
}
