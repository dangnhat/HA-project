/**
 * @file ha_node.h
 * @author  Nguyen Van Hien  <nvhien1992@gmail.com>.
 * @version 1.0
 * @date 11-Nov-2014
 * @brief This is header file for HA node initialization in HA system.
 */
#ifndef __HA_NODE_H
#define __HA_NODE_H

#include "ha_device_handler.h"

const uint8_t max_end_point = 1;
extern kernel_pid_t end_point_pid[max_end_point];

void ha_node_init(void);

#endif //__HA_NODE_H
