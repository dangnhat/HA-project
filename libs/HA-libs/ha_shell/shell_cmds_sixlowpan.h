/**
 * @file shell_cmds_sixlowpan.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>.
 * @version 1.0
 * @date 10-Nov-2014
 * @brief Header files for sixlowpan network stack related shell commands.
 *
 * Shell (6lowpan command)
 * -- store data ---> ha_ns::sixlowpan_config_file (with option -p, -n, -t, -c)
 * -- send ha_ns::SIXLOWPAN_RESTART (-s) --> 6LoWPAN threads
 *                                          (ha_ns::sixlowpan_sender_pid)
 *                                          (ha_ns::sixlowpan_receiver_pid)
 * 6LoWPAN threads read configurations from ha_ns::sixlowpan_config_file and
 * initialize network stack.
 */

#ifndef SHELL_CMDS_SIXLOWPAN_H_
#define SHELL_CMDS_SIXLOWPAN_H_

/**
 * @brief   6lowpan configuring command.
 *          Data will be saved/written to ha_ns::sixlowpan_config_file by following
 *          ha_ns::sixlowpan_config_pattern.
 *
 * @details Usage:  6lowpan, show current 6lowpan configurations.
 *                  6lowpan -p prefix3:prefix2:prefix1:prefix0, set 64bit prefixes.
 *                  6lowpan -n node_id, set node id (should always be > 0).
 *                  6lowpan -t netdev_type, set network device type,
 *                      can be r (root router), n (node router), or h (host).
 *                  6lowpan -c channel, set channel.
 *                  6lowpan -s channel, send restart message to 6lowpan threads.
 *                  6lowpan -x device_id value, send SET-DEV_VAL message to a device.
 *                  6lowpan -h, get help
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void sixlowpan_config(int argc, char** argv);

#endif /* SHELL_CMDS_SIXLOWPAN_H_ */
