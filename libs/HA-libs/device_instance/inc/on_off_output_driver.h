/**
 * @file on_off_bulb_driver.h
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is header file for on-off bulb device instance for HA system.
 */
#ifndef __HA_ON_OFF_BULB_DRIVER_H_
#define __HA_ON_OFF_BULB_DRIVER_H_

#include "GPIO_device.h"

using namespace dev_param_ns;

class on_off_output_instance: public gpio_dev_class {
public:
    on_off_output_instance(void);

    void device_configure(gpio_config_params_t *gpio_config_params);
    void dev_turn_on(void);
    void dev_turn_off(void);
    void dev_toggle(void);
    void set_active_level(uint8_t active_level);
    bool dev_get_state(void);
private:
    uint8_t active_level;
    bool is_turn_on;
};

#endif //__HA_ON_OFF_BULB_DRIVER_H_
