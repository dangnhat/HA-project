/**
 * @file device_common.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is a header file containing common parameter for devices in HA system.
 */
#ifndef __HA_DEVICE_COMMON_H_
#define __HA_DEVICE_COMMON_H_

#include "MB1_System.h"

using namespace gpio_ns;
using namespace adc_ns;
using namespace pwm_ns;

namespace dev_param_ns {
typedef struct {
    port_t device_port;
    uint8_t device_pin;
    uint8_t mode;
} gpio_config_params_t;

typedef struct {
    port_t device_port;
    uint8_t device_pin;
    adc_t adc_x;
    uint8_t adc_channel;
} adc_config_params_t;

typedef struct {
    port_t device_port;
    uint8_t device_pin;
    pwm_timer_t timer_x;
    uint8_t pwm_channel;
} pwm_config_params_t;
}

#endif //__HA_DEVICE_COMMON_H_
