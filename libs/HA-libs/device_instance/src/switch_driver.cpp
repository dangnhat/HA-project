/**
 * @file switch_driver.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is source file for switch device instance for HA system.
 */
#include "switch_driver.h"

const bool switch_active = true;

switch_instance::switch_instance(void) :
        gpio_dev_class(true)
{
    this->is_turn_on = false;
}

void switch_instance::device_configure(gpio_config_params_t *gpio_config_params)
{
    gpio_dev_configure(gpio_config_params->device_port,
            gpio_config_params->device_pin);
}

bool switch_instance::get_switch_status(void)
{
    return this->is_turn_on;
}
