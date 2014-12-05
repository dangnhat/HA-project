/**
 * @file ha_sixlowpan.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 08-Nov-2014
 * @brief This is header holds common function definitions for 6lowpan network
 * of home automation nodes and cc.
 *
 * Sixlowpan frame format with flow control: (for now, it will not be used)
 * | to node id (*) (2) | frame len (1) | flags (1) | index (2) | data |
 * (8) to node id: just a work around because without ND, we must send to multicast address.
 * flags: 0x00 if data, 0x01 if ack.
 */

#ifndef HA_SIXLOWPAN_H_
#define HA_SIXLOWPAN_H_

/* Includes */
extern "C" {
#include "transceiver.h"
#include "kernel.h"
#include "socket_base/socket.h"
}

#include "slp_sender.h"
#include "slp_receiver.h"
#include "common_msg_id.h"
#include "cir_queue.h"

#include "MB1_System.h"

/* Constants and definitions */
namespace ha_ns {

const uint16_t sixlowpan_pattern_maxsize = 90;
const char sixlowpan_config_file[] = "slp_conf";
const char sixlowpan_config_pattern[sixlowpan_pattern_maxsize] =
        "6LoWPAN\n"
        "64-bit prefix: %lx:%lx:%lx:%lx\n"
        "node id: 0x%lx\n"
        "device type: %c\n"   /* type could be r (root router), n (node router), h (host) */
        "channel: %lu\n";

const uint8_t sixlowpan_default_interface = 0;
const transceiver_type_t sixlowpan_default_transceiver = TRANSCEIVER_DEFAULT;

/* 6LoWPAN sender and receiver threads */
extern kernel_pid_t sixlowpan_sender_pid;
extern cir_queue sixlowpan_sender_gff_queue;

extern kernel_pid_t sixlowpan_receiver_pid;

/* Communications */
const uint16_t sixlowpan_ha_cc_node_id = 1;

extern ipv6_addr_t sixlowpan_ipaddr;
extern uint16_t sixlowpan_node_id;
extern char sixlowpan_netdev_type;

const uint16_t sixlowpan_payload_maxsize = 256;
const uint16_t sixlowpan_receiving_port = 1001;

/* Frame format (for now, it will not be used) */
const uint8_t sixlowpan_header_len = 4;
enum sixlowpan_header_flags_e {
    DATA = 0,
    ACK = 1,
};

}

/**
 * @brief   6lowpan read configurations from file.
 *          Using following global definitions.
 *
 * @param[in]   path, path to file holding configurations
 * @param[in]   pattern, format of configurations in file.
 * @param[in]   pattern_size, size of pattern.
 * @param[out]  prefixes_p, buffer which'll hold 64 bits prefix, this array
 *              must have at least 4 members. prefixes_p[3] will hold most significant
 *              half-word.
 * @param[out]  node_id, node id in 6LoWPAN network (should be always > 0).
 * @param[out]  netdev_type, type of a device in 6LoWPAN network with RPL. Can be
 *              r (root router), n (node router) or h (host).
 * @param[out]  channel, channel of a device to work on.
 *
 * @return      -1 if error. Error will occur when file doesn't exist, node id or prefixes are 0,
 *               or netdev_type is not h, r, n.
 */
int16_t ha_slp_readconfig(const char* path, const char* pattern, uint16_t pattern_size,
        uint16_t* prefixes_p, uint16_t &node_id, char &netdev_type, uint16_t &channel);

/**
 * @brief   Init sixlowpan network on a interface with given configurations.
 *          Short address will be used by default.
 *
 * @param[in]   interface,
 * @param[in]   transceiver,
 * @param[in]   prefixes_p, buffer which'll hold 64 bits prefix, this array
 *              must have at least 4 members. prefixes_p[3] will hold most significant
 *              half-word.
 * @param[in]   node_id, node id in 6LoWPAN network.
 * @param[in]   netdev_type, type of a device in 6LoWPAN network with RPL. Can be
 *              r (root router), n (node router) or h (host).
 * @param[in]   channel, channel of a device to work on.
 *
 * @return      -1 if error. Error will occur when file doesn't exist, node id or prefixes are 0,
 *               or netdev_type is not h, r, n.
 */
int16_t ha_slp_init(uint8_t interface, transceiver_type_t transceiver,
        uint16_t* prefixes_p, uint16_t node_id, char netdev_type, uint16_t channel);

/**
 * @brief   Reset sixlowpan network on reset if a given button has not been pressed in 3s.
 *
 * @param[in]   btn_p, pointer to a give button object.
 * @param[in]   btn_prompt, string will be printed so user will know the button needed to be pressed.
 */
void ha_slp_start_on_reset(Button *btn_p, const char *btn_prompt);

/**
 * @brief   Insert frame header to payload. Data in payload will move forward sixlowpan_header_len
 *          bytes to give the spaces for header.
 *
 * @param[in/out]   payload, buffer holding data payload. This is also used as a workspace
 *                  for frame.
 * @param[in]       data_len, len of the data in payload. Frame len = data_len + sixlowpan_header_len
 * @param[in]       flags, ACK or DATA.
 * @param[in]       index, frame index.
 */
void ha_slp_add_frame_header(uint8_t *payload, uint8_t data_len, uint8_t flags, uint16_t index);

/**
 * @brief   Remove frame header and retrieve payload. Data in payload will move backward
 *          sixlowpan_header_len bytes.
 *
 * @param[in/out]   frame, buffer holding frame. This is also used as a workspace
 *                  for payload.
 * @param[out]      frame_len, len of the frame. Data len = data_len - sixlowpan_header_len
 * @param[out]      flags, ACK or DATA.
 * @param[out]      index, frame index.
 */
void ha_slp_parse_frame_header(uint8_t *frame, uint8_t &frame_len, uint8_t &flags, uint16_t &index);

#endif /* HA_SIXLOWPAN_H_ */
