/**
 * @file on_off_bulb_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is header file for on-off bulb device instance for HA system.
 */
#ifndef __HA_ON_OFF_BULB_DRIVER_H_
#define __HA_ON_OFF_BULB_DRIVER_H_

#include "GPIO_devices.h"
#include "device_param.h"

using namespace dev_param_ns;

class on_off_bulb_instance: public gpio_dev_class {
public:
    on_off_bulb_instance(void);

    void device_configure(config_params_t *config_params);
    void bulb_turn_on(void);
    void bulb_turn_off(void);
    bool bulb_get_state(void);
private:
    bool is_turn_on;
};

#endif //__HA_ON_OFF_BULB_DRIVER_H_
