/**
 * @file ha_node_glb.h
 * @author  Nguyen Van Hien  <nvhien1992@gmail.com>.
 * @version 1.0
 * @date 14-Nov-2014
 * @brief This file contains global variables in entire node.
 */
#ifndef __HA_NODE_GLB_H
#define __HA_NODE_GLB_H

extern "C" {
#include "thread.h"
}

namespace ha_node_ns {
const uint8_t max_end_point = 12;
extern kernel_pid_t end_point_pid[max_end_point];
}

#endif //__HA_NODE_GLB_H
