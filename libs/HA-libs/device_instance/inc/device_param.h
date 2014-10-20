/**
 * @file device_param.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is a header file containing common parameter for devices in HA system.
 */
#ifndef __HA_DEV_PARAM_H_
#define __HA_DEV_PARAM_H_

#include "MB1_System.h"

using namespace gpio_ns;

namespace dev_param_ns {
typedef struct {
    port_t device_port;
    uint8_t device_pin;
} config_params_t;
}

#endif //__HA_DEV_PARAM_H_
