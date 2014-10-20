/**
 * @file on_off_bulb_driver.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is source file for on-off bulb device instance for HA system.
 */
#include "on_off_bulb_driver.h"

on_off_bulb_instance::on_off_bulb_instance(void) :
        gpio_dev_class(false)
{
    this->is_turn_on = false;
}

void on_off_bulb_instance::device_configure(config_params_t *config_params) {
    gpio_dev_configure(config_params->device_port, config_params->device_pin);
    bulb_turn_off();
}

void on_off_bulb_instance::bulb_turn_on(void)
{
    this->is_turn_on = true;
    gpio_dev_on();
}

void on_off_bulb_instance::bulb_turn_off(void)
{
    this->is_turn_on = false;
    gpio_dev_off();
}

bool on_off_bulb_instance::bulb_get_state(void)
{
    return this->is_turn_on;
}
