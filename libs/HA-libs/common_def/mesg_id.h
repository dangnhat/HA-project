/**
 * @file mesg_id.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 3-Nov-2014
 * @brief This is the header file for message-related definitions.
 */

#ifndef MESG_ID_H_
#define MESG_ID_H_

#include <stdint.h>

namespace ha_ns {

enum mesg_type_e: uint16_t { /* in GFF format */
    SET_DEV_VAL = 0x0000,
    SET_NUM_OF_DEVS = 0x0001,
    SET_DEV_WITH_INDEXS = 0x0002,
    SET_NUM_OF_SCENES = 0x0003,
    SET_ACT_SCENE_WITH_INDEXS = 0x0004,
    SET_INACT_SCENE_WITH_INDEXS = 0x0005,
    SET_NUM_OF_RULES = 0x0006,
    SET_RULE_WITH_INDEXS = 0x0007,
    SET_ZONE_NAME = 0x0008,

    GET_DEV_VAL = 0x0100,
    GET_NUM_OF_DEVS = 0x0101,
    GET_DEV_WITH_INDEXS = 0x0102,
    GET_NUM_OF_SCENES = 0x0103,
    GET_ACT_SCENE_WITH_INDEXS = 0x0104,
    GET_INACT_SCENE_WITH_INDEXS = 0x0105,
    GET_NUM_OF_RULES = 0x0106,
    GET_RULE_WITH_INDEXS = 0x0107,
    GET_ZONE_NAME = 0x0108,

    ALIVE = 0x0200,
};

};

#endif /* MESG_ID_H_ */
