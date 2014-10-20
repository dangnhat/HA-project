/**
 * @file switch_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is header file for switch device instance for HA system.
 */
#ifndef __HA_SWITCH_DRIVER_H_
#define __HA_SWITCH_DRIVER_H_

#include "GPIO_devices.h"
#include "device_param.h"

using namespace dev_param_ns;

class switch_instance: public gpio_dev_class {
public:
    switch_instance(void);

    void device_configure(config_params_t *config_params);
    void switch_set_state(bool is_turn_on);
    bool switch_state(void);
private:
    bool is_turn_on;
};

#endif //__HA_SWITCH_DRIVER_H_
