/**
 * @file cc_msg_id.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 08-Nov-2014
 * @brief This is header holds message ids for communications among threads of CC.
 */

#ifndef CC_MSG_ID_H_
#define CC_MSG_ID_H_

#include "common_msg_id.h"

namespace ha_cc_ns {

enum cc_msg_id: uint16_t {
    SLP_GFF_PENDING = ha_ns::COMMON_MSG_ID_END,
    BLE_GFF_PENDING,
    ONE_SEC_INTERRUPT,
    NEW_SCENE_TIMEOUT,
    NEW_SCENE_SET_RULE_TIMEOUT,

    /* BLE message */
    BLE_USART_REC,
    BLE_SERVER_RESET,
    BLE_CLIENT_CONNECT,
    BLE_CLIENT_DISCONNECT,
    BLE_CLIENT_WRITE,
};

}

#endif /* CC_MSG_ID_H_ */
