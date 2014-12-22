/**
 * @file ha_host_glb.h
 * @author  Nguyen Van Hien  <nvhien1992@gmail.com>.
 * @version 1.0
 * @date 14-Nov-2014
 * @brief This file contains global variables in entire host.
 */
#ifndef __HA_HOST_GLB_H
#define __HA_HOST_GLB_H

extern "C" {
#include "thread.h"
}

namespace ha_host_ns {
const uint8_t max_end_point = 8;
extern kernel_pid_t end_point_pid[max_end_point];
}

#endif //__HA_HOST_GLB_H
