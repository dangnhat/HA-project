/**
 * @file switch_driver.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is source file for switch device instance for HA system.
 */
#include "switch_driver.h"

switch_instance::switch_instance(void) :
        gpio_dev_class(true)
{
    this->is_turn_on = false;
}

void switch_instance::device_configure(config_params_t *config_params)
{
    gpio_dev_configure(config_params->device_port, config_params->device_pin);
    gpio_dev_int_both_edge();
}

void switch_instance::switch_set_state(bool turn_on)
{
    this->is_turn_on = turn_on;
}

bool switch_instance::switch_state(void)
{
    return this->is_turn_on;
}
