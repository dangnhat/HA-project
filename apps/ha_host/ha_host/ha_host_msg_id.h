/**
 * @file ha_host_mesg_id.h
 * @author  Nguyen Van Hien  <nvhien1992@gmail.com>.
 * @version 1.0
 * @date 3-Nov-2014
 * @brief This is the header file for message-related definitions.
 */

#ifndef __HA_HOST_MESG_ID_H_
#define __HA_HOST_MESG_ID_H_

#include <stdint.h>

namespace ha_host_ns {

enum mesg_type_e
    : uint16_t { /* in GFF format */
        NEW_DEVICE = 0x0300,
    SEND_ALIVE = 0x0201
};

}

#endif //__HA_HOST_MESG_ID_H_
