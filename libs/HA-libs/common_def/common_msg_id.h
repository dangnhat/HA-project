/**
 * @file common_msg_id.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 08-Nov-2014
 * @brief This is header holds message ids for communications among threads of both CC and node.
 */

#ifndef COMMON_MSG_ID_H_
#define COMMON_MSG_ID_H_

namespace ha_ns {

enum common_msg_id: uint16_t {
    /* Threads communications */
    GFF_PENDING,

    /* 6lowpan communications */
    SIXLOWPAN_RESTART,

    /* this will be used to chain with other enum of CC and node */
    /* ALWAYS KEEP it at THE END */
    COMMON_MSG_ID_END,
};

}

#endif /* COMMON_MSG_ID_H_ */
