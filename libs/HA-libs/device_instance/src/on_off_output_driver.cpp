/**
 * @file on_off_bulb_driver.cpp
 * @author  Nguyen Van Hien <nvhien1992@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 20-10-2014
 * @brief This is source file for on-off bulb device instance for HA system.
 */
#include "on_off_output_driver.h"

on_off_output_instance::on_off_output_instance(void) :
        gpio_dev_class(false)
{
    this->active_level = 0;
    this->is_turn_on = false;
}

void on_off_output_instance::device_configure(
        gpio_config_params_t *gpio_config_params)
{
    gpio_dev_configure(gpio_config_params->device_port,
            gpio_config_params->device_pin);
    dev_turn_off();
}

void on_off_output_instance::set_active_level(uint8_t active_level)
{
    this->active_level = active_level;
}

void on_off_output_instance::dev_turn_on(void)
{
    this->is_turn_on = true;
    if (active_level == 0) {
        gpio_dev_off();
    } else {
        gpio_dev_on();
    }
}

void on_off_output_instance::dev_turn_off(void)
{
    this->is_turn_on = false;
    if (active_level == 0) {
        gpio_dev_on();
    } else {
        gpio_dev_off();
    }
}

void on_off_output_instance::dev_toggle(void)
{
    if (this->is_turn_on) {
        dev_turn_off();
    } else {
        dev_turn_on();
    }
}

bool on_off_output_instance::dev_get_state(void)
{
    return this->is_turn_on;
}
