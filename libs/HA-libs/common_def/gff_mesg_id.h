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
    SET_ACT_SCENE_NAME_WITH_INDEXS = 0x0004,
    SET_INACT_SCENE_NAME_WITH_INDEXS = 0x0005,
    SET_NUM_OF_RULES = 0x0006,
    SET_RULE_WITH_INDEXS = 0x0007,
    SET_ZONE_NAME = 0x0008,
    SET_NEW_SCENE = 0x0009,
    SET_REMOVE_SCENE = 0x000A,
    SET_RENAME_INACT_SCENE = 0x000B,

    GET_DEV_VAL = 0x0100,
    GET_NUM_OF_DEVS = 0x0101,
    GET_DEV_WITH_INDEXS = 0x0102,
    GET_NUM_OF_SCENES = 0x0103,
    GET_ACT_SCENE_NAME_WITH_INDEXS = 0x0104,
    GET_INACT_SCENE_NAME_WITH_INDEXS = 0x0105,
    GET_NUM_OF_RULES = 0x0106,
    GET_RULE_WITH_INDEXS = 0x0107,
    GET_ZONE_NAME = 0x0108,

    ALIVE = 0x0200,
};

const uint16_t GFF_MAX_DATA_SIZE = 255;
const uint16_t GFF_CMD_SIZE = 2;
const uint16_t GFF_LEN_SIZE = 1;
const uint16_t GFF_MAX_FRAME_SIZE = GFF_MAX_DATA_SIZE + GFF_CMD_SIZE + GFF_LEN_SIZE;

const uint8_t GFF_LEN_POS = 0;
const uint8_t GFF_CMD_POS = GFF_LEN_POS + GFF_LEN_SIZE;
const uint8_t GFF_DATA_POS = GFF_CMD_POS + GFF_CMD_SIZE;

enum gff_data_len_e: uint8_t {
    SET_DEV_VAL_DATA_LEN = 6, /* device_id + value */
    ALIVE_DATA_LEN = 4, /* device_id */

    SET_NUM_OF_DEVS_DATA_LEN = 4,
    SET_DEVICE_WITH_INDEX_DATA_LEN = 10,

    GET_ZONE_NAME_DATA_LEN = 1,
    SET_ZONE_NAME_DATA_LEN = 17,

    GET_NUM_OF_SCENES_DATA_LEN = 0,
    SET_NUM_OF_SCENES_DATA_LEN = 2,

    SET_ACT_SCENE_NAME_WITH_INDEXS_DATA_LEN = 9,
    SET_INACT_SCENE_NAME_WITH_INDEXS_DATA_LEN = 9,

    GET_NUM_OF_RULES_DATA_LEN = 8,
    SET_NUM_OF_RULES_DATA_LEN = 10,

    SET_RULE_WITH_INDEXS_DATA_LEN = 27,

    SET_NEW_SCENE_DATA_LEN = 8,
    SET_REMOVE_SCENE_DATA_LEN = 8,
    SET_RENAME_INACT_SCENE_DATA_LEN = 16,
};

const uint32_t SET_DEV_WITH_INDEX_ALL_DEVS = 0xFFFFFFFF;

};

#endif /* MESG_ID_H_ */
