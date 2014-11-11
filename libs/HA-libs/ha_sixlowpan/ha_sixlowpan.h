/**
 * @file ha_sixlowpan.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 08-Nov-2014
 * @brief This is header holds common function definitions for 6lowpan network
 * of home automation nodes and cc.
 */

#ifndef HA_SIXLOWPAN_H_
#define HA_SIXLOWPAN_H_

/* Includes */
extern "C" {
#include "transceiver.h"
#include "kernel.h"
#include "socket_base/socket.h"
}

#include "common_msg_id.h"
#include "cir_queue.h"

/* Constants and definitions */
namespace ha_ns {

const uint16_t sixlowpan_pattern_maxsize = 128;
const char sixlowpan_config_file[] = "slp_conf";
const char sixlowpan_config_pattern[sixlowpan_pattern_maxsize] =
        "6LoWPAN\n"
        "64-bit prefix: %lx:%lx:%lx:%lx\n"
        "node id: %lu\n"
        "device type: %c\n"   /* type could be r (root router), n (node router), h (host) */
        "channel: %lu\n";

const uint8_t sixlowpan_default_interface = 0;
const transceiver_type_t sixlowpan_default_transceiver = TRANSCEIVER_DEFAULT;

/* 6LoWPAN sender and receiver threads */
extern kernel_pid_t* sixlowpan_sender_pid;
extern cir_queue* slp_sender_gff_queue_p;

extern kernel_pid_t* sixlowpan_receiver_pid;

/* Communications */
extern ipv6_addr_t sixlowpan_ipaddr;
extern uint16_t sixlowpan_node_id;
extern char sixlowpan_netdev_type;

const uint16_t sixlowpan_payload_maxsize = 256;
const uint16_t sixlowpan_receiving_port = 1001;

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

#endif /* HA_SIXLOWPAN_H_ */
